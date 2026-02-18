#!/bin/bash
set -e

# ============================================================================
# Build all thirdparty libraries for Astra Linux (gcc-12)
# Updated for GCC 12.2.0 and Qt 5.15.2
# ============================================================================

echo "============================================================================"
echo "Building precompiled libraries for Astra Linux (gcc-12)"
echo "============================================================================"
echo ""

# ---------------------------------------------------------------------------
# Paths and configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="${SCRIPT_DIR}"
COMPILE_DIR="${THIRDPARTY_DIR}/compile"
PLATFORM_NAME="linux/gcc"

# Parse arguments
if [ -z "$1" ]; then
    BUILD_TYPES="Release Debug"
else
    BUILD_TYPES="$1"
fi

KEEP_BUILD=${KEEP_BUILD:-0}

# PostgreSQL paths (adjust as needed)
POSTGRESQL_ROOT="/usr"

# Qt paths
QT_PATH="${QT_PATH:-${HOME}/Qt/5.15.2/gcc_64}"

echo "Configuration:"
echo "  THIRDPARTY_DIR: ${THIRDPARTY_DIR}"
echo "  COMPILE_DIR:    ${COMPILE_DIR}"
echo "  PLATFORM_NAME:  ${PLATFORM_NAME}"
echo "  BUILD_TYPES:    ${BUILD_TYPES}"
echo "  KEEP_BUILD:     ${KEEP_BUILD}"
echo "  QT_PATH:        ${QT_PATH}"
echo ""

# ---------------------------------------------------------------------------
# Basic checks
# ---------------------------------------------------------------------------
echo "Checking required tools..."

# Check gcc
if ! command -v gcc &> /dev/null; then
    echo "ERROR: gcc not found!"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ not found!"
    exit 1
fi

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

GCC_VERSION=$(gcc --version | head -n1 | awk '{print $3}')
echo "[OK] Compiler found:"
echo "     CC:  ${CC} (${GCC_VERSION})"
echo "     CXX: ${CXX}"

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "Please install CMake 3.16+"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
CMAKE_MAJOR=$(echo ${CMAKE_VERSION} | cut -d. -f1)
CMAKE_MINOR=$(echo ${CMAKE_VERSION} | cut -d. -f2)

if [ ${CMAKE_MAJOR} -lt 3 ] || ([ ${CMAKE_MAJOR} -eq 3 ] && [ ${CMAKE_MINOR} -lt 16 ]); then
    echo "ERROR: CMake version is ${CMAKE_VERSION}, but 3.16+ is required"
    exit 1
fi

echo "[OK] CMake found: ${CMAKE_VERSION}"

# Check Ninja (optional, fallback to make)
if command -v ninja &> /dev/null; then
    GENERATOR="Ninja"
    echo "[OK] Ninja found: $(ninja --version)"
else
    GENERATOR="Unix Makefiles"
    echo "[INFO] Ninja not found, using Make"
fi

# Check Python (for ScintillaEdit codegen)
if command -v python3 &> /dev/null; then
    PYTHON_BIN=$(which python3)
    echo "[OK] Python found: ${PYTHON_BIN}"
else
    PYTHON_BIN=""
    echo "[WARNING] Python not found - ScintillaEdit codegen will be skipped"
fi

# Check GTK (for Scintilla)
echo ""
echo "Checking GTK libraries for Scintilla..."
GTK_VERSION="gtk+-3.0"
if pkg-config --exists ${GTK_VERSION}; then
    echo "[OK] GTK+ 3.0 found"
    GTK3_FLAG="GTK3=1"
elif pkg-config --exists gtk+-2.0; then
    echo "[OK] GTK+ 2.0 found"
    GTK_VERSION="gtk+-2.0"
    GTK3_FLAG=""
else
    echo "ERROR: GTK+ not found!"
    echo "Please install: sudo apt install libgtk-3-dev or libgtk2.0-dev"
    exit 1
fi

# Check X11 libraries
echo ""
echo "Checking X11 libraries for SDL..."
MISSING_X11_LIBS=()

for lib in libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxinerama-dev libxi-dev; do
    if ! dpkg -s ${lib} &> /dev/null 2>&1; then
        MISSING_X11_LIBS+=("${lib}")
    fi
done

if [ ${#MISSING_X11_LIBS[@]} -gt 0 ]; then
    echo "[WARNING] Some X11 libraries are missing:"
    for lib in "${MISSING_X11_LIBS[@]}"; do
        echo "          ${lib}"
    done
    echo "          SDL3 build may fail."
else
    echo "[OK] All required X11 libraries found"
fi

# Check Qt
if [ -f "${QT_PATH}/bin/qmake" ]; then
    export PATH="${QT_PATH}/bin:${PATH}"
    echo "[OK] Qt found: ${QT_PATH}"
    QMAKE_BIN="${QT_PATH}/bin/qmake"
else
    echo "[WARNING] Qt not found at ${QT_PATH}"
    echo "          qmake-based builds will be skipped"
    QMAKE_BIN=""
fi

echo ""

# Create compile directory
mkdir -p "${COMPILE_DIR}"

# ---------------------------------------------------------------------------
# Helper function: assert source directory exists
# ---------------------------------------------------------------------------
assert_src() {
    local SRC_PATH="$1"
    if [ -z "${SRC_PATH}" ]; then
        echo "ERROR: Internal: expected source path but parameter empty"
        exit 1
    fi
    if [ ! -d "${SRC_PATH}" ]; then
        echo "ERROR: Source directory not found: ${SRC_PATH}"
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# build_library (CMake)
# Usage: build_library <name> <source_dir> <build_type> <extra_flags>
# ---------------------------------------------------------------------------
build_library() {
    local LIB_NAME="$1"
    local LIB_SRC="$2"
    local BUILD_TYPE="$3"
    local EXTRA_FLAGS="$4"

    if [ -z "${LIB_NAME}" ]; then
        echo "ERROR: build_library: LIB_NAME empty"
        exit 1
    fi

    if [ -z "${LIB_SRC}" ]; then
        echo "ERROR: build_library: LIB_SRC empty for ${LIB_NAME}"
        exit 1
    fi

    assert_src "${LIB_SRC}"

    local INSTALL_DIR="${COMPILE_DIR}/${LIB_NAME}/${PLATFORM_NAME}/${BUILD_TYPE}"
    local BUILD_DIR="${THIRDPARTY_DIR}/_build_${LIB_NAME}_${BUILD_TYPE}"

    echo "==========================================================================="
    echo "Building ${LIB_NAME} (${BUILD_TYPE}) via CMake"
    echo "==========================================================================="
    echo "Source:  ${LIB_SRC}"
    echo "Install: ${INSTALL_DIR}"
    echo "Build:   ${BUILD_DIR}"
    echo ""

    # Remove old build directory
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
    fi

    # Build CMAKE_PREFIX_PATH with already compiled libraries
    local CMAKE_PREFIX_PATH=""

    if [ -d "${COMPILE_DIR}/fmt/${PLATFORM_NAME}/${BUILD_TYPE}" ]; then
        CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH};${COMPILE_DIR}/fmt/${PLATFORM_NAME}/${BUILD_TYPE}"
    fi

    if [ -d "${COMPILE_DIR}/SDL3/${PLATFORM_NAME}/${BUILD_TYPE}" ]; then
        CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH};${COMPILE_DIR}/SDL3/${PLATFORM_NAME}/${BUILD_TYPE}"
    fi

    if [ -d "${POSTGRESQL_ROOT}/include/postgresql" ]; then
        CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH};${POSTGRESQL_ROOT}"
    fi

    # Remove leading semicolon
    CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH#;}"

    # Configure
    cmake -B "${BUILD_DIR}" -S "${LIB_SRC}" \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        ${CMAKE_PREFIX_PATH:+-DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}"} \
        ${EXTRA_FLAGS}

    if [ $? -ne 0 ]; then
        echo "ERROR: Configuration failed for ${LIB_NAME}"
        exit 1
    fi

    # Build
    if [ "${GENERATOR}" = "Ninja" ]; then
        cmake --build "${BUILD_DIR}" --parallel $(nproc)
    else
        cmake --build "${BUILD_DIR}" -- -j$(nproc)
    fi

    if [ $? -ne 0 ]; then
        echo "ERROR: Build failed for ${LIB_NAME}"
        exit 1
    fi

    # Install
    cmake --install "${BUILD_DIR}"

    if [ $? -ne 0 ]; then
        echo "ERROR: Install failed for ${LIB_NAME}"
        exit 1
    fi

    # Cleanup
    if [ ${KEEP_BUILD} -eq 0 ]; then
        rm -rf "${BUILD_DIR}"
    fi

    echo "[OK] ${LIB_NAME} (${BUILD_TYPE}) installed at ${INSTALL_DIR}"
    echo ""
}

# ---------------------------------------------------------------------------
# build_scintilla (make)
# Usage: build_scintilla <scintilla_src_dir> <build_type>
# ---------------------------------------------------------------------------
build_scintilla() {
    local LIB_SRC="$1"
    local BUILD_TYPE="$2"

    if [ -z "${LIB_SRC}" ]; then
        echo "ERROR: build_scintilla: LIB_SRC empty"
        exit 1
    fi

    assert_src "${LIB_SRC}"

    local SCINTILLA_MAK="${LIB_SRC}/gtk/makefile"
    if [ ! -f "${SCINTILLA_MAK}" ]; then
        echo "ERROR: makefile not found at ${SCINTILLA_MAK}"
        exit 1
    fi

    local INSTALL_DIR="${COMPILE_DIR}/scintilla/${PLATFORM_NAME}/${BUILD_TYPE}"

    echo "==========================================================================="
    echo "Building Scintilla (${BUILD_TYPE}) via make"
    echo "==========================================================================="
    echo "Source:   ${LIB_SRC}"
    echo "Makefile: ${SCINTILLA_MAK}"
    echo "Install:  ${INSTALL_DIR}"
    echo ""

    pushd "${LIB_SRC}/gtk" > /dev/null

    # Clean previous build
    make clean || true

    # Build with GCC
    local MAKE_FLAGS="${GTK3_FLAG}"
    if [ "${BUILD_TYPE}" = "Debug" ]; then
        MAKE_FLAGS="${MAKE_FLAGS} DEBUG=1"
    fi

    echo "Building with: make ${MAKE_FLAGS}"
    make ${MAKE_FLAGS}

    if [ $? -ne 0 ]; then
        popd > /dev/null
        echo "ERROR: scintilla build failed"
        exit 1
    fi

    # Create installation directories
    mkdir -p "${INSTALL_DIR}/lib"
    mkdir -p "${INSTALL_DIR}/include"
    mkdir -p "${INSTALL_DIR}/bin"

    # Copy static library
    if [ -f "../bin/scintilla.a" ]; then
        cp -f "../bin/scintilla.a" "${INSTALL_DIR}/lib/"
        echo "  Copied scintilla.a"
    fi

    # Copy shared library
    if [ -f "../bin/libscintilla.so" ]; then
        cp -f "../bin/libscintilla.so" "${INSTALL_DIR}/lib/"
        echo "  Copied libscintilla.so"
        # Also copy to bin for runtime linking
        cp -f "../bin/libscintilla.so" "${INSTALL_DIR}/bin/"
    fi

    # Copy headers from include/
    if [ -d "../include" ]; then
        cp -f ../include/*.h "${INSTALL_DIR}/include/" 2>/dev/null || true
        echo "  Copied headers from include/"
    fi

    # Copy headers from src/ (if needed)
    if [ -d "../src" ]; then
        cp -f ../src/*.h "${INSTALL_DIR}/include/" 2>/dev/null || true
        echo "  Copied headers from src/"
    fi

    popd > /dev/null

    echo "[OK] Scintilla (${BUILD_TYPE}) installed at ${INSTALL_DIR}"
    echo "  - Static library: ${INSTALL_DIR}/lib/scintilla.a"
    echo "  - Shared library: ${INSTALL_DIR}/lib/libscintilla.so"
    echo ""
}

# ---------------------------------------------------------------------------
# build_lexilla (make)
# Usage: build_lexilla <lexilla_src_dir> <build_type>
# ---------------------------------------------------------------------------
build_lexilla() {
    local LIB_SRC="$1"
    local BUILD_TYPE="$2"

    if [ -z "${LIB_SRC}" ]; then
        echo "ERROR: build_lexilla: LIB_SRC empty"
        exit 1
    fi

    assert_src "${LIB_SRC}"

    local LEXILLA_MAK="${LIB_SRC}/makefile"
    if [ ! -f "${LEXILLA_MAK}" ]; then
        echo "ERROR: makefile not found at ${LEXILLA_MAK}"
        exit 1
    fi

    local INSTALL_DIR="${COMPILE_DIR}/lexilla/${PLATFORM_NAME}/${BUILD_TYPE}"

    echo "==========================================================================="
    echo "Building Lexilla (${BUILD_TYPE}) via make"
    echo "==========================================================================="
    echo "Source:   ${LIB_SRC}"
    echo "Makefile: ${LEXILLA_MAK}"
    echo "Install:  ${INSTALL_DIR}"
    echo ""

    pushd "${LIB_SRC}" > /dev/null

    # Clean previous build
    make clean || true

    # Build with GCC
    local MAKE_FLAGS=""
    if [ "${BUILD_TYPE}" = "Debug" ]; then
        MAKE_FLAGS="${MAKE_FLAGS} DEBUG=1"
    fi

    echo "Building with: make ${MAKE_FLAGS}"
    make ${MAKE_FLAGS}

    if [ $? -ne 0 ]; then
        popd > /dev/null
        echo "ERROR: lexilla build failed"
        exit 1
    fi

    # Create installation directories
    mkdir -p "${INSTALL_DIR}/lib"
    mkdir -p "${INSTALL_DIR}/include"
    mkdir -p "${INSTALL_DIR}/bin"

    # Copy library files
    if [ -f "../bin/liblexilla.so" ]; then
        cp -f "../bin/liblexilla.so" "${INSTALL_DIR}/lib/"
        echo "  Copied liblexilla.so"
    fi

    if [ -f "../bin/liblexilla.a" ]; then
        cp -f "../bin/liblexilla.a" "${INSTALL_DIR}/lib/"
        echo "  Copied liblexilla.a"
    fi

    # Create lexilla5.so symlink for compatibility (in bin directory)
    if [ -f "${INSTALL_DIR}/lib/liblexilla.so" ]; then
        cp -f "${INSTALL_DIR}/lib/liblexilla.so" "${INSTALL_DIR}/bin/lexilla5.so"
        echo "  Created lexilla5.so copy in bin/"
    fi

    # Copy headers from include/
    if [ -d "../include" ]; then
        cp -f ../include/*.h "${INSTALL_DIR}/include/" 2>/dev/null || true
        echo "  Copied headers from include/"
    fi

    # Copy headers from access/ (if exists)
    if [ -d "../access" ]; then
        cp -f ../access/*.h "${INSTALL_DIR}/include/" 2>/dev/null || true
        echo "  Copied headers from access/"
    fi

    popd > /dev/null

    echo "[OK] Lexilla (${BUILD_TYPE}) installed at ${INSTALL_DIR}"
    echo "  - Shared library: ${INSTALL_DIR}/lib/liblexilla.so"
    echo "  - Static library: ${INSTALL_DIR}/lib/liblexilla.a"
    echo "  - Runtime DLL:    ${INSTALL_DIR}/bin/lexilla5.so"
    echo ""
}

# ---------------------------------------------------------------------------
# build_qmake_library (qmake + make)
# Usage: build_qmake_library <name> <src_dir> <build_type> <need_codegen>
# ---------------------------------------------------------------------------
build_qmake_library() {
    local LIB_NAME="$1"
    local LIB_SRC="$2"
    local BUILD_TYPE="$3"
    local NEED_CODEGEN="$4"

    if [ -z "${QMAKE_BIN}" ]; then
        echo "[SKIP] ${LIB_NAME} - qmake not available"
        return
    fi

    if [ -z "${LIB_SRC}" ]; then
        echo "ERROR: build_qmake_library: LIB_SRC empty"
        exit 1
    fi

    assert_src "${LIB_SRC}"

    local INSTALL_DIR="${COMPILE_DIR}/${LIB_NAME}/${PLATFORM_NAME}/${BUILD_TYPE}"
    local BUILD_DIR="${THIRDPARTY_DIR}/_build_${LIB_NAME}_${BUILD_TYPE}"

    echo "==========================================================================="
    echo "Building ${LIB_NAME} (${BUILD_TYPE}) via qmake"
    echo "==========================================================================="
    echo "Source:  ${LIB_SRC}"
    echo "Install: ${INSTALL_DIR}"
    echo "Build:   ${BUILD_DIR}"
    echo ""

    # Remove old build directory
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
    fi
    mkdir -p "${BUILD_DIR}"

    pushd "${BUILD_DIR}" > /dev/null

    # Set qmake CONFIG based on build type
    if [ "${BUILD_TYPE}" = "Debug" ]; then
        QMAKE_CONFIG="CONFIG+=debug CONFIG-=release"
    else
        QMAKE_CONFIG="CONFIG+=release CONFIG-=debug"
    fi

    # Run codegen if needed
    if [ "${NEED_CODEGEN}" = "1" ]; then
        if [ -z "${PYTHON_BIN}" ]; then
            echo "ERROR: Python required for codegen"
            popd > /dev/null
            exit 1
        fi

        pushd "${LIB_SRC}" > /dev/null
        ${PYTHON_BIN} WidgetGen.py
        if [ $? -ne 0 ]; then
            popd > /dev/null
            popd > /dev/null
            echo "ERROR: codegen failed"
            exit 1
        fi
        popd > /dev/null
    fi

    # Find .pro file
    PRO_FILE=$(find "${LIB_SRC}" -maxdepth 1 -name "*.pro" | head -n 1)
    if [ -z "${PRO_FILE}" ]; then
        popd > /dev/null
        echo "ERROR: .pro file not found in ${LIB_SRC}"
        exit 1
    fi

    echo "Running qmake with: ${QMAKE_CONFIG}"
    echo "Using compilers: CC=${CC}, CXX=${CXX}"

    # Set compilers via qmake variables
    "${QMAKE_BIN}" "${PRO_FILE}" ${QMAKE_CONFIG} \
        "QMAKE_CC=${CC}" \
        "QMAKE_CXX=${CXX}" \
        "QMAKE_LINK=${CXX}" \
        "QMAKE_LINK_SHLIB=${CXX}"

    if [ $? -ne 0 ]; then
        popd > /dev/null
        echo "ERROR: qmake failed"
        exit 1
    fi

    # Build
    make -j$(nproc)

    if [ $? -ne 0 ]; then
        popd > /dev/null
        echo "ERROR: make failed"
        exit 1
    fi

    popd > /dev/null

    # Create installation directories
    mkdir -p "${INSTALL_DIR}/lib"
    mkdir -p "${INSTALL_DIR}/include"

    # .pro files set DESTDIR = ../../bin relative to BUILD_DIR
    OUTPUT_BIN="$(cd "${BUILD_DIR}/../.." && pwd)/bin"

    echo "Looking for output files in: ${OUTPUT_BIN}"

    if [ -d "${OUTPUT_BIN}" ]; then
        echo "Copying from output directory: ${OUTPUT_BIN}"
        # Copy libraries (shared and static)
        if ls "${OUTPUT_BIN}/${LIB_NAME}"*.so* 1> /dev/null 2>&1; then
            cp -f "${OUTPUT_BIN}/${LIB_NAME}"*.so* "${INSTALL_DIR}/lib/" 2>/dev/null
            echo "  Copied shared libraries (${LIB_NAME}*.so*)"
        fi
        if ls "${OUTPUT_BIN}/${LIB_NAME}"*.a 1> /dev/null 2>&1; then
            cp -f "${OUTPUT_BIN}/${LIB_NAME}"*.a "${INSTALL_DIR}/lib/" 2>/dev/null
            echo "  Copied static libraries (${LIB_NAME}*.a)"
        fi
        if ls "${OUTPUT_BIN}/lib${LIB_NAME}"*.so* 1> /dev/null 2>&1; then
            cp -f "${OUTPUT_BIN}/lib${LIB_NAME}"*.so* "${INSTALL_DIR}/lib/" 2>/dev/null
            echo "  Copied lib-prefixed shared libraries (lib${LIB_NAME}*.so*)"
        fi
        if ls "${OUTPUT_BIN}/lib${LIB_NAME}"*.a 1> /dev/null 2>&1; then
            cp -f "${OUTPUT_BIN}/lib${LIB_NAME}"*.a "${INSTALL_DIR}/lib/" 2>/dev/null
            echo "  Copied lib-prefixed static libraries (lib${LIB_NAME}*.a)"
        fi
    else
        echo "WARNING: Output directory not found: ${OUTPUT_BIN}"
    fi

    # Copy headers
    cp -f "${LIB_SRC}"/*.h "${INSTALL_DIR}/include/" 2>/dev/null || true

    # Special case for ScintillaEdit
    if [ "${LIB_NAME}" = "ScintillaEdit" ] && [ -f "${LIB_SRC}/ScintillaDocument.h" ]; then
        cp -f "${LIB_SRC}/ScintillaDocument.h" "${INSTALL_DIR}/include/"
    fi

    # Cleanup build directory
    if [ ${KEEP_BUILD} -eq 0 ]; then
        rm -rf "${BUILD_DIR}"
    fi

    echo "[OK] ${LIB_NAME} (${BUILD_TYPE}) installed at ${INSTALL_DIR}"
    echo ""
}

# ---------------------------------------------------------------------------
# build_metakit (g++ + ar/g++ -shared, без CMake)
# Usage: build_metakit <src_dir> <build_type> <shared: 0|1>
# ---------------------------------------------------------------------------
build_metakit() {
    local LIB_SRC="$1"
    local BUILD_TYPE="$2"
    local BUILD_SHARED="${3:-0}"

    assert_src "${LIB_SRC}"

    local LIB_LABEL="metakit"
    if [ "${BUILD_SHARED}" = "1" ]; then
        LIB_LABEL="metakit_shared"
    fi

    local INSTALL_DIR="${COMPILE_DIR}/${LIB_LABEL}/${PLATFORM_NAME}/${BUILD_TYPE}"
    local OBJ_DIR="${THIRDPARTY_DIR}/_build_${LIB_LABEL}_${BUILD_TYPE}"
    local AUTOCONF_DIR="${OBJ_DIR}/autoconf"

    echo "==========================================================================="
    echo "Building ${LIB_LABEL} (${BUILD_TYPE}) via g++ (no CMake)"
    echo "==========================================================================="
    echo "Source:  ${LIB_SRC}"
    echo "Install: ${INSTALL_DIR}"
    echo "ObjDir:  ${OBJ_DIR}"
    echo ""

    rm -rf "${OBJ_DIR}"
    mkdir -p "${OBJ_DIR}"
    mkdir -p "${AUTOCONF_DIR}"

    # --- Запуск unix/configure для генерации config.h ---
    if [ ! -f "${AUTOCONF_DIR}/config.h" ]; then
        echo "  Running unix/configure to generate config.h..."
        pushd "${AUTOCONF_DIR}" > /dev/null
        "${LIB_SRC}/unix/configure" \
            CC="${CC}" CXX="${CXX}" \
            > "${AUTOCONF_DIR}/configure.log" 2>&1

        if [ $? -ne 0 ]; then
            popd > /dev/null
            echo "ERROR: unix/configure failed. Log: ${AUTOCONF_DIR}/configure.log"
            exit 1
        fi
        popd > /dev/null
        echo "  config.h generated"
    fi

    # --- Флаги компилятора ---
    local CXX_FLAGS="-std=c++14 -D_GNU_SOURCE -DCXX11_ONLY"
    local INCLUDE_FLAGS="-I${AUTOCONF_DIR} -I${LIB_SRC}/include -I${LIB_SRC}/src"

    if [ "${BUILD_TYPE}" = "Debug" ]; then
        CXX_FLAGS="${CXX_FLAGS} -g -O0 -DDEBUG"
    else
        CXX_FLAGS="${CXX_FLAGS} -O2 -DNDEBUG"
    fi

    if [ "${BUILD_SHARED}" = "1" ]; then
        CXX_FLAGS="${CXX_FLAGS} -fPIC"
    fi

    # Подавляем предупреждения характерные для старого кода metakit
    CXX_FLAGS="${CXX_FLAGS} -Wno-deprecated -Wno-unused-parameter -Wno-sign-compare"

    # --- Компиляция всех .cpp из src/ ---
    local OBJ_FILES=()
    for SRC_FILE in "${LIB_SRC}/src/"*.cpp; do
        local BASE
        BASE=$(basename "${SRC_FILE}" .cpp)
        local OBJ_FILE="${OBJ_DIR}/${BASE}.o"

        echo "  Compiling ${BASE}.cpp ..."
        "${CXX}" ${CXX_FLAGS} ${INCLUDE_FLAGS} -c "${SRC_FILE}" -o "${OBJ_FILE}"
        if [ $? -ne 0 ]; then
            echo "ERROR: Compilation failed for ${BASE}.cpp"
            exit 1
        fi
        OBJ_FILES+=("${OBJ_FILE}")
    done

    # --- Установка ---
    mkdir -p "${INSTALL_DIR}/lib"
    mkdir -p "${INSTALL_DIR}/include"

    if [ "${BUILD_SHARED}" = "1" ]; then
        # Shared library
        echo "  Linking libmk4.so ..."
        "${CXX}" -shared -o "${INSTALL_DIR}/lib/libmk4.so" "${OBJ_FILES[@]}"
        if [ $? -ne 0 ]; then
            echo "ERROR: Linking libmk4.so failed"
            exit 1
        fi
        echo "  - Shared library: ${INSTALL_DIR}/lib/libmk4.so"
    else
        # Static library
        echo "  Archiving mk4.a ..."
        ar rcs "${INSTALL_DIR}/lib/libmk4.a" "${OBJ_FILES[@]}"
        if [ $? -ne 0 ]; then
            echo "ERROR: ar failed for metakit"
            exit 1
        fi
        echo "  - Static library: ${INSTALL_DIR}/lib/libmk4.a"
    fi

    # --- Публичные заголовки ---
    cp -f "${LIB_SRC}/include/"*.h "${INSTALL_DIR}/include/" 2>/dev/null || true

    # --- Cleanup ---
    if [ ${KEEP_BUILD} -eq 0 ]; then
        rm -rf "${OBJ_DIR}"
    fi

    echo "[OK] ${LIB_LABEL} (${BUILD_TYPE}) installed at ${INSTALL_DIR}"
    echo ""
}
# ---------------------------------------------------------------------------
# Main build process
# ---------------------------------------------------------------------------
echo "Starting build..."
cd "${THIRDPARTY_DIR}"

for BUILD_TYPE in ${BUILD_TYPES}; do
    echo ""
    echo "==========================================================================="
    echo "Building configuration: ${BUILD_TYPE}"
    echo "==========================================================================="
    echo ""

    build_metakit "${THIRDPARTY_DIR}/metakit" "${BUILD_TYPE}" "0"   # static → libmk4.a
    build_metakit "${THIRDPARTY_DIR}/metakit" "${BUILD_TYPE}" "1"   # shared → libmk4.soN"

    # Build libraries in dependency order
    build_library "fmt" "${THIRDPARTY_DIR}/fmt" "${BUILD_TYPE}" \
        "-DFMT_TEST=OFF -DBUILD_SHARED_LIBS=OFF"

    build_library "spdlog" "${THIRDPARTY_DIR}/spdlog" "${BUILD_TYPE}" \
        "-DSPDLOG_FMT_EXTERNAL=ON -DBUILD_SHARED_LIBS=OFF"

    # Build Scintilla via makefile
    build_scintilla "${THIRDPARTY_DIR}/scintilla" "${BUILD_TYPE}"

    build_library "SDL3" "${THIRDPARTY_DIR}/SDL" "${BUILD_TYPE}" \
        "-DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF -DSDL_INSTALL=ON -DSDL_SHARED=ON -DSDL_STATIC=OFF"

    build_library "SDL3_image" "${THIRDPARTY_DIR}/SDL_image" "${BUILD_TYPE}" \
        "-DSDL3IMAGE_SAMPLES=OFF -DBUILD_SHARED_LIBS=ON"

    build_lexilla "${THIRDPARTY_DIR}/lexilla/src" "${BUILD_TYPE}"

    build_library "qtadvanceddocking" "${THIRDPARTY_DIR}/Qt-Advanced-Docking-System" "${BUILD_TYPE}" \
        "-DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON"

    build_qmake_library "ScintillaEditBase" "${THIRDPARTY_DIR}/scintilla/qt/ScintillaEditBase" "${BUILD_TYPE}" "0"

    if [ -n "${PYTHON_BIN}" ]; then
        build_qmake_library "ScintillaEdit" "${THIRDPARTY_DIR}/scintilla/qt/ScintillaEdit" "${BUILD_TYPE}" "1"
    fi

    # Clean temporary bin directory
    if [ -d "${THIRDPARTY_DIR}/../bin" ]; then
        echo "Cleaning temporary bin directory: ${THIRDPARTY_DIR}/../bin"
        rm -rf "${THIRDPARTY_DIR}/../bin"
    fi

    # Optional libpqxx
    if [ -d "${POSTGRESQL_ROOT}/include/postgresql" ]; then
        PostgreSQL_INCLUDE_DIR="${POSTGRESQL_ROOT}/include/postgresql"
        PostgreSQL_LIBRARY="${POSTGRESQL_ROOT}/lib/libpq.so"

        if [ -d "${PostgreSQL_INCLUDE_DIR}" ] && [ -f "${PostgreSQL_LIBRARY}" ]; then
            build_library "libpqxx" "${THIRDPARTY_DIR}/libpqxx" "${BUILD_TYPE}" \
                "-DSKIP_BUILD_TEST=ON -DBUILD_SHARED_LIBS=OFF -DPostgreSQL_INCLUDE_DIR=${PostgreSQL_INCLUDE_DIR} -DPostgreSQL_LIBRARY=${PostgreSQL_LIBRARY}"
        else
            echo "[SKIP] libpqxx - expected headers/libs not found"
        fi
    else
        echo "[SKIP] libpqxx - PostgreSQL not installed"
    fi
done

echo ""
echo "==========================================================================="
echo "Build completed"
echo "==========================================================================="
echo ""
echo "All libraries built with GCC 12.2.0 successfully."
echo ""
