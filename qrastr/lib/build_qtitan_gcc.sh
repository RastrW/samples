#!/bin/bash
set -e

# ============================================================================
# Build Qtitan DataGrid for Astra Linux (gcc-12)
# Updated for GCC 12.2.0 and Qt 5.15.2
# ============================================================================

echo "============================================================================"
echo "Building Qtitan DataGrid for Astra Linux (gcc-12)"
echo "============================================================================"
echo ""

# ---------------------------------------------------------------------------
# Paths - adjust these as needed
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QTITAN_ROOT="${SCRIPT_DIR}"

# Qt path - adjust for your system
QT_PATH="${QT_PATH:-${HOME}/Qt/5.15.2/gcc_64}"

# Output directories
OUTPUT_BASE="${QTITAN_ROOT}/lib"

# Build type: debug, release, or both (default)
BUILD_TYPE="${1:-release}"  # Default to release only (for trial version)

echo "Configuration:"
echo "  QTITAN_ROOT: ${QTITAN_ROOT}"
echo "  QT_PATH:     ${QT_PATH}"
echo "  OUTPUT_BASE: ${OUTPUT_BASE}"
echo "  BUILD_TYPE:  ${BUILD_TYPE}"
echo ""

# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------
echo "Checking prerequisites..."

if [ ! -d "${QTITAN_ROOT}" ]; then
    echo "ERROR: Qtitan root not found: ${QTITAN_ROOT}"
    exit 1
fi

if [ ! -d "${QTITAN_ROOT}/src" ]; then
    echo "ERROR: src directory not found in: ${QTITAN_ROOT}"
    exit 1
fi

if [ ! -f "${QTITAN_ROOT}/src/shared/qtitangrid.pri" ]; then
    echo "ERROR: qtitangrid.pri not found at: ${QTITAN_ROOT}/src/shared/qtitangrid.pri"
    exit 1
fi

if [ ! -f "${QT_PATH}/bin/qmake" ]; then
    echo "ERROR: qmake not found at: ${QT_PATH}/bin/qmake"
    echo "Please adjust QT_PATH in the script or set it as environment variable"
    echo "Example: QT_PATH=/path/to/Qt/5.15.2/gcc_64 $0"
    exit 1
fi

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

echo "[OK] Qtitan root found"
echo "[OK] Qt 5.15.2 found: ${QT_PATH}"
echo "[OK] Compilers found: CC=${CC} (${GCC_VERSION}), CXX=${CXX}"
echo ""

# ---------------------------------------------------------------------------
# Set QTITANDIR environment variable (as per README)
# ---------------------------------------------------------------------------
export QTITANDIR="${QTITAN_ROOT}"
export LD_LIBRARY_PATH="${QTITAN_ROOT}/lib:${LD_LIBRARY_PATH}"

echo "Environment variables set:"
echo "  QTITANDIR=${QTITANDIR}"
echo "  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
echo ""

# Add Qt to PATH
export PATH="${QT_PATH}/bin:${PATH}"

# Verify Qt version
QT_VERSION=$(qmake -query QT_VERSION)
echo "Detected Qt version: ${QT_VERSION}"

if [[ ! "${QT_VERSION}" =~ ^5\.15\. ]]; then
    echo "WARNING: Qt version ${QT_VERSION} detected, but 5.15.x expected"
    echo "         Build may fail or produce incompatible binaries"
fi
echo ""

# ---------------------------------------------------------------------------
# Build function
# ---------------------------------------------------------------------------
build_qtitan() {
    local CONFIG="$1"
    local BUILD_DIR="${QTITAN_ROOT}/build-gcc-${CONFIG}"

    echo "==========================================================================="
    echo "Building Qtitan (${CONFIG})"
    echo "==========================================================================="
    echo "Build dir:  ${BUILD_DIR}"
    echo "Output dir: ${OUTPUT_BASE}"
    echo ""

    # Create build directory
    if [ -d "${BUILD_DIR}" ]; then
        echo "Removing old build directory..."
        rm -rf "${BUILD_DIR}"
    fi
    mkdir -p "${BUILD_DIR}"

    # Create output directory
    mkdir -p "${OUTPUT_BASE}"

    # Enter build directory
    pushd "${BUILD_DIR}" > /dev/null

    # Create a minimal .pro file that includes qtitangrid.pri
    # This follows the README recommendation
    cat > QtitanBuild.pro << 'EOF'
TEMPLATE = subdirs
SUBDIRS = src
src.file = $$QTITANDIR/src/grid/grid.pro
EOF

    # Configure with qmake
    echo "Running qmake..."
    qmake QtitanBuild.pro \
        "CONFIG+=${CONFIG}" \
        "QMAKE_CC=${CC}" \
        "QMAKE_CXX=${CXX}" \
        "QMAKE_LINK=${CXX}" \
        "QMAKE_LINK_SHLIB=${CXX}"

    if [ $? -ne 0 ]; then
        echo "ERROR: qmake failed"
        popd > /dev/null
        exit 1
    fi

    # Build
    echo ""
    echo "Building with make..."
    make -j$(nproc)

    if [ $? -ne 0 ]; then
        echo "ERROR: make failed"
        popd > /dev/null
        exit 1
    fi

    popd > /dev/null

    echo ""
    echo "[OK] ${CONFIG} build completed successfully"
    echo ""
}

# ---------------------------------------------------------------------------
# Alternative: Build using the main project file if it exists
# ---------------------------------------------------------------------------
build_qtitan_direct() {
    local CONFIG="$1"
    local BUILD_DIR="${QTITAN_ROOT}/build-gcc-${CONFIG}"

    echo "==========================================================================="
    echo "Building Qtitan (${CONFIG}) - Direct method"
    echo "==========================================================================="
    echo "Build dir:  ${BUILD_DIR}"
    echo ""

    # Create build directory
    if [ -d "${BUILD_DIR}" ]; then
        echo "Removing old build directory..."
        rm -rf "${BUILD_DIR}"
    fi
    mkdir -p "${BUILD_DIR}"
    mkdir -p "${OUTPUT_BASE}"

    # Enter build directory
    pushd "${BUILD_DIR}" > /dev/null

    # Find the main .pro file
    MAIN_PRO=""
    if [ -f "${QTITAN_ROOT}/QtitanDataGrid.pro" ]; then
        MAIN_PRO="${QTITAN_ROOT}/QtitanDataGrid.pro"
    elif [ -f "${QTITAN_ROOT}/src/grid/grid.pro" ]; then
        MAIN_PRO="${QTITAN_ROOT}/src/grid/grid.pro"
    else
        echo "ERROR: Cannot find main .pro file"
        popd > /dev/null
        exit 1
    fi

    echo "Using project file: ${MAIN_PRO}"

    # Configure with qmake
    echo "Running qmake..."
    qmake "${MAIN_PRO}" \
        "CONFIG+=${CONFIG}" \
        "QMAKE_CC=${CC}" \
        "QMAKE_CXX=${CXX}" \
        "QMAKE_LINK=${CXX}" \
        "QMAKE_LINK_SHLIB=${CXX}"

    if [ $? -ne 0 ]; then
        echo "ERROR: qmake failed"
        popd > /dev/null
        exit 1
    fi

    # Build
    echo ""
    echo "Building with make..."
    make -j$(nproc)

    if [ $? -ne 0 ]; then
        echo "ERROR: make failed"
        popd > /dev/null
        exit 1
    fi

    popd > /dev/null

    echo ""
    echo "[OK] ${CONFIG} build completed successfully"
    echo ""
}

# ---------------------------------------------------------------------------
# Main build process
# ---------------------------------------------------------------------------
cd "${QTITAN_ROOT}"

# Check if this is a trial version
echo "Note: For trial version, only release build is available."
echo ""

case "${BUILD_TYPE}" in
    debug)
        build_qtitan_direct "debug"
        ;;
    release)
        build_qtitan_direct "release"
        ;;
    both)
        build_qtitan_direct "release"
        build_qtitan_direct "debug"
        ;;
    *)
        echo "ERROR: Invalid build type: ${BUILD_TYPE}"
        echo "Usage: $0 [debug|release|both]"
        echo "   or: QT_PATH=/path/to/Qt/5.15.2/gcc_64 $0 [debug|release|both]"
        exit 1
        ;;
esac

echo "==========================================================================="
echo "Qtitan build completed successfully"
echo "==========================================================================="
echo ""
echo "Library location: ${OUTPUT_BASE}/"
echo ""
echo "To use in your Qt projects:"
echo "1. Set environment variable: export QTITANDIR=${QTITAN_ROOT}"
echo "2. Add to your LD_LIBRARY_PATH: export LD_LIBRARY_PATH=${OUTPUT_BASE}:\$LD_LIBRARY_PATH"
echo "3. Add to your .pro file: include(\$\$QTITANDIR/src/shared/qtitangrid.pri)"
echo "4. Run: qmake -r"
echo ""
echo "For permanent setup, you can create ${QTITAN_ROOT}/qtitanvars.sh:"
cat << 'VAREOF'
#!/bin/bash
export QTITANDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${QTITANDIR}/lib:${LD_LIBRARY_PATH}"
echo "Qtitan environment configured:"
echo "  QTITANDIR=${QTITANDIR}"
VAREOF

echo ""
echo "Then source it with: source ${QTITAN_ROOT}/qtitanvars.sh"
echo ""
echo "Build directories (can be removed):"
find "${QTITAN_ROOT}" -maxdepth 1 -type d -name "build-gcc-*" -exec echo "  {}" \;
echo ""
echo "Done!"
