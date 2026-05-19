#!/bin/bash
# deploy_appimage.sh — сборка дистрибутива QRastr в формате AppImage.
# Теория и обоснование каждого шага — в DEPLOY_INSTRUCTION.md.
#
# ПРЕДУСЛОВИЯ:
#   - Система: AltLinux 10.4 (GLIBC 2.32)
#   - Проект собран в Qt Creator (конфигурация Release)
#   - build_thirdparty_gcc.sh выполнен с BUILD_LIBPQXX=1
#   - linuxdeploy и linuxdeploy-plugin-qt скачаны в LINUXDEPLOY_DIR
#
# ИСПОЛЬЗОВАНИЕ:
#   ./deploy_appimage.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# =============================================================================
# Конфигурация — скорректируйте пути под вашу машину
# =============================================================================

QT_ROOT="${HOME}/Qt/5.15.2/gcc_64"
PROJ="${HOME}/projects/rastr/qrastr"
THIRDPARTY="${PROJ}/thirdparty/compile"
ASTRA_ROOT="${HOME}/projects/astra"

# Инструменты linuxdeploy
LINUXDEPLOY_DIR="${HOME}/tools/linuxdeploy"

# Куда положить итоговый .AppImage
OUTPUT_DIR="${HOME}/dist"

# =============================================================================
# Внутренние переменные
# =============================================================================
APPDIR="${HOME}/AppDir"
PLATFORM="linux/gcc/Release"
LIBPQXX_INSTALL="${THIRDPARTY}/libpqxx/${PLATFORM}"

# Файл для лога
LOG_FILE="${OUTPUT_DIR}/deploy.log"

# =============================================================================
# Функция для логирования
# =============================================================================
log() {
    echo "$@" | tee -a "${LOG_FILE}"
}

# =============================================================================
echo ""
echo "============================================================================="
echo "  QRastr AppImage deploy"
echo "  Хост: $(uname -n)  |  GLIBC: $(ldd --version | head -1 | grep -oP '[0-9]+\.[0-9]+$')"
echo "  Дата: $(date '+%Y-%m-%d %H:%M')"
echo "============================================================================="
echo "" | tee "${LOG_FILE}"

# =============================================================================
# Шаг 0: Проверки окружения
# =============================================================================
log "[ 0 / 5 ]  Проверка окружения..."

ERRORS=0
check() {
    if [ ! -e "$2" ] && ! command -v "$2" &>/dev/null; then
        log "  [FAIL] $1: не найдено — $2"
        ERRORS=$((ERRORS + 1))
    else
        log "  [ OK ] $1"
    fi
}

check "Qt qmake"              "${QT_ROOT}/bin/qmake"
check "qrastr (Release)"      "${PROJ}/Release/qrastr"
check "libqmcr.so"            "${PROJ}/Release/libqmcr.so"
check "plugins/"              "${PROJ}/Release/plugins"
check "libpqxx (prebuilt)"    "${LIBPQXX_INSTALL}"
check "linuxdeploy"           "${LINUXDEPLOY_DIR}/linuxdeploy-x86_64.AppImage"
check "linuxdeploy-plugin-qt" "${LINUXDEPLOY_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"
check "qrastr.png"            "${PROJ}/qrastr/resources/images/qrastr.png"

if [ "${ERRORS}" -gt 0 ]; then
    log ""
    log "  Устраните ошибки выше и повторите запуск."
    exit 1
fi
log ""

# =============================================================================
# Шаг 1: Подготовка AppDir
# =============================================================================
log "[ 1 / 5 ]  Подготовка AppDir..."

rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/lib"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/256x256/apps"

log "  [ OK ] ${APPDIR}"
log ""

# =============================================================================
# Шаг 2: Копирование файлов в AppDir
# Логика размещения по папкам — DEPLOY_INSTRUCTION.md, Часть 2.
# =============================================================================
log "[ 2 / 5 ]  Копирование файлов..."

# --- usr/bin/ : бинарник и библиотеки, загружаемые через QLibrary ---
log "  → usr/bin/"

cp "${PROJ}/Release/qrastr"     "${APPDIR}/usr/bin/"
cp "${PROJ}/Release/libqmcr.so" "${APPDIR}/usr/bin/"

LEXILLA_SO="${THIRDPARTY}/lexilla/${PLATFORM}/lib/liblexilla.so"
[ -f "${LEXILLA_SO}" ] || { log "  ОШИБКА: не найдена ${LEXILLA_SO}"; exit 1; }
cp "${LEXILLA_SO}" "${APPDIR}/usr/bin/"

SCINTILLA_EDIT_SO="${THIRDPARTY}/ScintillaEdit/${PLATFORM}/lib/libScintillaEdit.so.5"
[ -f "${SCINTILLA_EDIT_SO}" ] || { log "  ОШИБКА: не найдена ${SCINTILLA_EDIT_SO}"; exit 1; }
cp "${SCINTILLA_EDIT_SO}" "${APPDIR}/usr/bin/"
ln -sf libScintillaEdit.so.5 "${APPDIR}/usr/bin/libScintillaEdit.so"

# --- usr/bin/plugins/ : плагины QPluginLoader + их прямые зависимости ---
log "  → usr/bin/plugins/"

cp -r "${PROJ}/Release/plugins" "${APPDIR}/usr/bin/"

# libmk4.so — критичная зависимость libastra.so
LIBMK4=$(find "${ASTRA_ROOT}" /usr/lib -name "libmk4.so" 2>/dev/null | head -1 || true)
if [ -z "${LIBMK4}" ]; then
    log "  ОШИБКА: libmk4.so не найдена. Проверьте ASTRA_ROOT=${ASTRA_ROOT}"
    exit 1
fi
cp "${LIBMK4}" "${APPDIR}/usr/bin/plugins/"
log "  libmk4.so  ← ${LIBMK4}"

# libpqxx — зависимость libCOMCK.so, собрана build_thirdparty_gcc.sh
LIBPQXX_SO=$(find "${LIBPQXX_INSTALL}" -name "libpqxx*.so*" 2>/dev/null | head -1 || true)
if [ -z "${LIBPQXX_SO}" ]; then
    log "  ОШИБКА: libpqxx.so не найдена в ${LIBPQXX_INSTALL}"
    log "  Запустите: BUILD_LIBPQXX=1 ./thirdparty/build_thirdparty_gcc.sh"
    exit 1
fi
find "${LIBPQXX_INSTALL}" -name "libpqxx*.so*" -exec cp {} "${APPDIR}/usr/bin/plugins/" \;
log "  libpqxx    ← ${LIBPQXX_INSTALL}"

# --- usr/lib/ : библиотеки, подхватываемые через RPATH ---
log "  → usr/lib/"

copy_if_exists() {
    if [ -f "$1" ]; then
        cp "$1" "${APPDIR}/usr/lib/" && log "  [ OK ] $2"
    else
        log "  [SKP] $2  ($1)"
    fi
}
copy_glob() {
    local FILES; FILES=$(ls $1 2>/dev/null || true)
    if [ -n "${FILES}" ]; then
        cp ${FILES} "${APPDIR}/usr/lib/" && log "  [ OK ] $2"
    else
        log "  [SKP] $2  ($1)"
    fi
}

copy_if_exists "${THIRDPARTY}/qtadvanceddocking/${PLATFORM}/lib/libqtadvanceddocking-qt5.so.4" "libqtadvanceddocking"
copy_if_exists "${THIRDPARTY}/SDL3/${PLATFORM}/lib/libSDL3.so.0"                               "libSDL3"
copy_glob      "${THIRDPARTY}/SDL3_image/${PLATFORM}/lib/libSDL3_image.so*"                    "libSDL3_image"
copy_if_exists "${PROJ}/lib/qtitan8_lin_5_15/lib/libQtitanBase.so.2"                           "libQtitanBase"
copy_if_exists "${PROJ}/lib/qtitan8_lin_5_15/lib/libQtitanGrid.so.8"                           "libQtitanGrid"
copy_glob      "${PROJ}/lib/qtitan8_lin_5_15/lib/libQtitanStyle.so*"                           "libQtitanStyle"
copy_glob      "${PROJ}/lib/qtitan8_lin_5_15/lib/libQtitanFastInfoset.so*"                     "libQtitanFastInfoset"

# --- Иконка и desktop-файл ---
log "  → ресурсы"

cp "${PROJ}/qrastr/resources/images/qrastr.png" \
    "${APPDIR}/usr/share/icons/hicolor/256x256/apps/qrastr.png"

cat > "${APPDIR}/usr/share/applications/qrastr.desktop" << 'EOF'
[Desktop Entry]
Name=QRastr
Exec=qrastr
Icon=qrastr
Type=Application
Categories=Utility;
EOF
cp "${APPDIR}/usr/share/applications/qrastr.desktop" "${APPDIR}/"

log ""

# =============================================================================
# Шаг 3: Интеграция Python 3.11
# =============================================================================
log "[ 3 / 5 ]  Интеграция Python 3.11..."

PYTHON_VERSION="3.11"
PYTHON_BIN=$(command -v python${PYTHON_VERSION} 2>/dev/null || true)

if [ -z "${PYTHON_BIN}" ]; then
    log "  [WARN] python${PYTHON_VERSION} не найден — пропускаем."
    log "         Установите: apt-get install -y python${PYTHON_VERSION}"
else
    cp "${PYTHON_BIN}" "${APPDIR}/usr/bin/python"

    PYTHON_LIBDIR=""
    for CANDIDATE in "/usr/lib64/python${PYTHON_VERSION}" "/usr/lib/python${PYTHON_VERSION}"; do
        [ -d "${CANDIDATE}" ] && { PYTHON_LIBDIR="${CANDIDATE}"; break; }
    done

    if [ -n "${PYTHON_LIBDIR}" ]; then
        cp -r "${PYTHON_LIBDIR}" "${APPDIR}/usr/lib/"
        log "  stdlib: ${PYTHON_LIBDIR}"
    else
        log "  [WARN] stdlib python${PYTHON_VERSION} не найдена"
    fi

    find /usr/lib64 /usr/lib -maxdepth 1 \
        -name "libpython${PYTHON_VERSION}*.so*" \
        -exec cp {} "${APPDIR}/usr/lib/" \; 2>/dev/null || true

    log "  [ OK ] Python ${PYTHON_VERSION}"
fi
log ""

# =============================================================================
# Шаг 4: linuxdeploy
# =============================================================================
log "[ 4 / 5 ]  Запуск linuxdeploy..."

export QTDIR="${QT_ROOT}"
export PATH="${QT_ROOT}/bin:${PATH}"
export QMAKE="${QT_ROOT}/bin/qmake"
export LD_LIBRARY_PATH="${QT_ROOT}/lib:${APPDIR}/usr/lib:${APPDIR}/usr/bin:${LD_LIBRARY_PATH:-}"
export LINUXDEPLOY_PLUGIN_QT_PATH="${LINUXDEPLOY_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"

log "  qmake: $(qmake --version | head -1)"

mkdir -p "${OUTPUT_DIR}"
cd "${OUTPUT_DIR}"

# Весь вывод linuxdeploy теперь идет в лог (без tee, так как log уже использует tee)
"${LINUXDEPLOY_DIR}/linuxdeploy-x86_64.AppImage" \
    --appdir "${APPDIR}" \
    --executable "${APPDIR}/usr/bin/qrastr" \
    --desktop-file "${APPDIR}/qrastr.desktop" \
    --plugin qt \
    --output appimage >> "${LOG_FILE}" 2>&1

log ""

# =============================================================================
# Шаг 5: Итог
# =============================================================================
log "[ 5 / 5 ]  Результат..."

APPIMAGE_FILE=$(ls "${OUTPUT_DIR}"/QRastr-*.AppImage 2>/dev/null | head -1 || true)

if [ -z "${APPIMAGE_FILE}" ]; then
    log ""
    log "  ОШИБКА: AppImage не создан. Подробности: ${LOG_FILE}"
    exit 1
fi

log ""
log "============================================================================="
log "  ГОТОВО: $(basename "${APPIMAGE_FILE}")"
log "  Размер: $(du -sh "${APPIMAGE_FILE}" | cut -f1)"
log "  Путь:   ${APPIMAGE_FILE}"
log "============================================================================="
log ""
log "  На целевой машине рядом с .AppImage должна лежать папка Data/"
log ""
log "  Запуск:      ./$(basename "${APPIMAGE_FILE}")"
log "  Без FUSE:    ./$(basename "${APPIMAGE_FILE}") --appimage-extract-and-run"
log "  Диагностика: DEPLOY_INSTRUCTION.md, Часть 5"
log ""
