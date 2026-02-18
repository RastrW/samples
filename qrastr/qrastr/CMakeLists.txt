# ============================================================================
# Универсальный CMakeLists.txt для qrastr
# Поддерживаемые платформы:
#   - Windows (MSVC)
#   - Ubuntu 24.04 (gcc)
#   - AstraLinux 1.8 (gcc)
#
# Все платформы  используют библиотеки из thirdparty
# ============================================================================

cmake_minimum_required(VERSION 3.23)
message(STATUS "CMake version: ${CMAKE_VERSION}")

# ============================================================================
# Инициализация проекта
# ============================================================================
cmake_policy(SET CMP0167 NEW) # In CMake 3.30+, the FindBoost module has been removed. Using BoostConfig.cmake
project(qrastr VERSION 0.1 LANGUAGES CXX C)

# ============================================================================
# Build Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Автоматическая обработка Qt-специфичных файлов
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Output directories - все бинарники идут в ../Debug или ../Release
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}")

# ============================================================================
# Compile Definitions
# ============================================================================
add_compile_definitions(SDL_NO) #Воспроизведение окон SDL
add_compile_definitions(QTN_NAMESPACE)

add_compile_definitions(
    $<$<CONFIG:Debug>:q4_INLINE=0>  # Отключает inline функции в Debug
    $<$<CONFIG:Debug>:YYDEBUG>      # Включает отладочный вывод для yacc/bison парсеров
)

if(MSVC)
    add_compile_definitions( COMPILE_WIN )
    # Подавление предупреждений о deprecated Microsoft extensions
    add_compile_definitions( _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS )
    # Отключение макросов min/max из windows.h (конфликт с std::min/max)
    add_compile_definitions( NOMINMAX )
endif()
# ============================================================================
# Определение компилятора и соответствующих путей
# ============================================================================
include(cmake/compiler_and_platform.cmake)
include(cmake/add_thirdparty_library.cmake)

add_compile_definitions(
    LIB_PREFIX="${LIB_PREFIX}"
    SHLIB_SUFFIX="${SHLIB_SUFFIX}"
)
# ============================================================================
# Options
# ============================================================================
option(QRASTR_BUILD_WITH_MCR "Build qmcr as library for qrastr" ON)
option(BUILD_WITH_PQ "Build with libpq" OFF)

message(STATUS "Build options:")
message(STATUS "  QRASTR_BUILD_WITH_MCR: ${QRASTR_BUILD_WITH_MCR}")
message(STATUS "  BUILD_WITH_PQ: ${BUILD_WITH_PQ}")

# ============================================================================
# Path Configuration
# ============================================================================
# Thirdparty directory - сторонние библиотеки
set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty")
# Передаем THIRDPARTY_DIR в дочерние проекты
set(THIRDPARTY_DIR "${THIRDPARTY_DIR}")

if(NOT IS_DIRECTORY ${THIRDPARTY_DIR})
    message(FATAL_ERROR "THIRDPARTY_DIR not found: ${THIRDPARTY_DIR}")
else()
    message(STATUS "THIRDPARTY_DIR: ${THIRDPARTY_DIR}")
endif()

# Путь к библиотеке ASTRA с бинарными файлами
set(ASTRA_BIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/astra")

if(NOT IS_DIRECTORY ${ASTRA_BIN_ROOT})
    message(FATAL_ERROR "ASTRA binary directory not found: ${ASTRA_BIN_ROOT}")
endif()

message(STATUS "ASTRA_BIN_ROOT: ${ASTRA_BIN_ROOT}")

# Ищем конфигурацию ASTRA
# CONFIG означает: FindCOMCK.cmake — даже не ищется, ищется comck-config.cmake
# HINTS вместо list(APPEND CMAKE_PREFIX_PATH "${PATH}")
find_package(COMCK CONFIG HINTS "${ASTRA_BIN_ROOT}/lib/cmake/COMCK")
find_package(Astra CONFIG HINTS "${ASTRA_BIN_ROOT}/lib/cmake/Astra")

if(NOT COMCK_FOUND)
    message(FATAL_ERROR "COMCK not found")
endif()

if(NOT Astra_FOUND)
    message(FATAL_ERROR "Astra not found")
endif()

message(STATUS "COMCK found: ${COMCK_FOUND}")
message(STATUS "Astra found: ${Astra_FOUND}")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(APP_DIR ${SRC_DIR}/app)
# ============================================================================
# Qtitan Configuration
# ============================================================================
# Путь к библиотеке Qtitan - оставляем по-старому для Astra Linux
if(WIN32)
    set(QTITAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/QtitanDataGrid8.2.0_win_qt_6_7")
else()
    set(QTITAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/qtitan8_lin_5_15")
endif()

# ============================================================================
# System Libraries
# - header-only библиотеки из thirdparty: Boost, Eigen3, fmt, spdlog
# - libpqxx: опционально собирается через add_thirdparty_library (зависит от наличия PostgreSQL)
# ============================================================================
add_library(Boost::boost INTERFACE IMPORTED)
target_include_directories(Boost::boost INTERFACE "${THIRDPARTY_DIR}/boost_1_80_0")

add_library(Eigen3::Eigen INTERFACE IMPORTED)
target_include_directories(Eigen3::Eigen INTERFACE "${THIRDPARTY_DIR}/eigen")

add_library(Fmt::fmt INTERFACE IMPORTED)
target_include_directories(Fmt::fmt INTERFACE "${THIRDPARTY_DIR}/fmt/include")
target_compile_definitions(Fmt::fmt INTERFACE FMT_HEADER_ONLY)

add_library(Spdlog::spdlog INTERFACE IMPORTED)
target_include_directories(Spdlog::spdlog INTERFACE "${THIRDPARTY_DIR}/spdlog/include")

# Создаём алиасы для совместимости
add_library(fmt::fmt ALIAS Fmt::fmt)
add_library(spdlog::spdlog ALIAS Spdlog::spdlog)
# ============================================================================
# libpqxx - опционально собирается из thirdparty на всех платформах
# ============================================================================
if(BUILD_WITH_PQ)
    add_thirdparty_library(libpqxx "${THIRDPARTY_DIR}/libpqxx")
    add_library(libpqxx::pqxx ALIAS libpqxx)
endif()

# ============================================================================
# Qt
# ============================================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Widgets
    PrintSupport
    Xml
    Test
    LinguistTools
    SvgWidgets
    Svg
)

# Qt6 требует дополнительный модуль для совместимости с Qt5
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat)
    # qt_standard_project_setup() настраивает проект для Qt6
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19" AND QT_VERSION VERSION_GREATER_EQUAL "6.3")
        qt_standard_project_setup()
    endif()
endif()

message(STATUS "Qt found: ${QT_VERSION} at ${Qt${QT_VERSION_MAJOR}_DIR}")

# ============================================================================
# Plugins (ASTRA-dependent)
# ============================================================================
set(PLUGIN_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/plugins")

add_subdirectory(${SRC_DIR}/plugins/rastr)
add_subdirectory(${SRC_DIR}/plugins/ti)
add_subdirectory(${SRC_DIR}/plugins/barsmdp)

message(STATUS "Plugins output directory: ${PLUGIN_OUTPUT_DIR}")

# qmcr subdirectory
set(DEPENDENCIES_LOCATION "${THIRDPARTY_DIR}")

set(QMCR_SOURCE_DIR "${SRC_DIR}/qmcr")
add_subdirectory(
    "${QMCR_SOURCE_DIR}"
)
message(STATUS "qmcr subproject added")

# ============================================================================
# Third-Party Libraries
# ----------------------------------------------------------------------------
add_thirdparty_library(metakit "${THIRDPARTY_DIR}/metakit"
    LIB_REAL_NAME "mk4")

add_thirdparty_library(qtadvanceddocking "${THIRDPARTY_DIR}/Qt-Advanced-Docking-System"
    LIB_REAL_NAME "qtadvanceddocking-qt${QT_VERSION_MAJOR}"
    USE_DEBUG_SUFFIX)

add_thirdparty_library(SDL3 "${THIRDPARTY_DIR}/SDL")
add_thirdparty_library(SDL3_image "${THIRDPARTY_DIR}/SDL_image")

add_thirdparty_library(scintilla "${THIRDPARTY_DIR}/scintilla")

# Scintilla требует дополнительные include директории
target_include_directories(scintilla INTERFACE
    ${THIRDPARTY_DIR}/scintilla/qt/ScintillaEditBase
    ${THIRDPARTY_DIR}/scintilla/qt/ScintillaEdit
)
# ============================================================================
# Include Directories
# ============================================================================
set(PROJECT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTITAN_ROOT}/include
    ${SRC_DIR}/
    ${APP_DIR}/astra/
    ${APP_DIR}/data/
    ${APP_DIR}/delegates/
    ${APP_DIR}/formatting/
    ${APP_DIR}/forms/
    ${APP_DIR}/main/
    ${APP_DIR}/mdi/
    ${APP_DIR}/protocol_tree/
    ${APP_DIR}/sdl/
    ${APP_DIR}/test/
    ${APP_DIR}/ui_components/
    ${APP_DIR}/utilities/
    ${APP_DIR}/astra/astra_headers
)

# Добавляем header-only include directories
get_target_property(BOOST_INCLUDES Boost::boost INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND PROJECT_INCLUDES ${BOOST_INCLUDES})

get_target_property(EIGEN3_INCLUDES Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND PROJECT_INCLUDES ${EIGEN3_INCLUDES})

get_target_property(SPDLOG_INCLUDES Spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND PROJECT_INCLUDES ${SPDLOG_INCLUDES})

get_target_property(FMT_INCLUDES Fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND PROJECT_INCLUDES ${FMT_INCLUDES})

# Добавляем ASTRA include директории
list(APPEND PROJECT_INCLUDES
    ${ASTRA_BIN_ROOT}/include
    ${ASTRA_BIN_ROOT}/include/comck
)
# ============================================================================
# Source Files
# ============================================================================
file(GLOB_RECURSE PROJECT_SOURCES
    "${APP_DIR}/main/*.cpp"
    "${APP_DIR}/main/*.h"
    "${APP_DIR}/data/*.cpp"
    "${APP_DIR}/data/*.h"
    "${APP_DIR}/ui_components/*.cpp"
    "${APP_DIR}/ui_components/*.h"
    "${APP_DIR}/delegates/*.cpp"
    "${APP_DIR}/delegates/*.h"
    "${APP_DIR}/forms/*.cpp"
    "${APP_DIR}/forms/*.h"
    "${APP_DIR}/forms/*.ui"
    "${APP_DIR}/protocol_tree/*.cpp"
    "${APP_DIR}/protocol_tree/*.h"
    "${APP_DIR}/formatting/*.cpp"
    "${APP_DIR}/formatting/*.h"
    "${APP_DIR}/utilities/*.cpp"
    "${APP_DIR}/utilities/*.h"
    "${APP_DIR}/sdl/*.cpp"
    "${APP_DIR}/sdl/*.h"
    "${APP_DIR}/test/*.cpp"
    "${APP_DIR}/test/*.h"
    "${APP_DIR}/astra/*.cpp"
    "${APP_DIR}/astra/*.h"
    "${APP_DIR}/astra/astra_headers/*.h"
    "${SRC_DIR}/plugins/rastr/plugin_interfaces.h"
    "${APP_DIR}/main.cpp"
)

# Translation files
set(TS_FILES
    recources/qrastr_ru_RU.ts
)

# Ресурсы
set(RESOURCES
    resources/qrastr.qrc
)

# ============================================================================
# Executable - создание исполняемого файла
# ============================================================================
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    # Qt6: используем qt_add_executable
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION  # Вызовем qt_finalize_executable() вручную в конце
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )
    # Создание скомпилированных переводов (.qm файлы из .ts файлов)
    qt_create_translation(QM_FILES
        ${PROJECT_SOURCES}  # Исходники для извлечения строк перевода
        ${TS_FILES}         # XML файлы переводов
    )
else()
    # Qt5: используем add_executable
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )

    qt5_create_translation(QM_FILES
        ${PROJECT_SOURCES}
        ${TS_FILES}
    )
endif()

# ============================================================================
# Link Libraries - линковка библиотек к исполняемому файлу
# ============================================================================
# 1) Определяем списки библиотек по "блокам"
set(QT_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::Svg
)

if(QT_VERSION_MAJOR EQUAL 6)
    list(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

set(THIRD_PARTY_LIBS
    metakit
    qtadvanceddocking
    SDL3
    SDL3_image
    scintilla
    qmcr
)

if(BUILD_WITH_PQ)
    list(APPEND THIRD_PARTY_LIBS libpqxx::pqxx)
endif()

# Astra
set(ASTRA_LIBS)
list(APPEND ASTRA_LIBS
    "${ASTRA_BIN_ROOT}/lib/${LIB_PREFIX}astra${LIB_SUFFIX}"
    "${ASTRA_BIN_ROOT}/lib/${LIB_PREFIX}COMCK${LIB_SUFFIX}"
)

# 2) Составляем окончательный список с учётом опций
set(PROJECT_LINK_LIBS)
list(APPEND PROJECT_LINK_LIBS ${QT_LIBS})
list(APPEND PROJECT_LINK_LIBS ${THIRD_PARTY_LIBS})
list(APPEND PROJECT_LINK_LIBS ${ASTRA_LIBS})

# 3) Применяем один вызов для link'ов
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LINK_LIBS})
# ============================================================================
# Qtitan Libraries Configuration
# ============================================================================
if(WIN32)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" QTITAN_CONFIG_DIR)
    set(QTITAN_LIB_DIR "${QTITAN_ROOT}/lib/msvc/${QTITAN_CONFIG_DIR}")
else()
    set(QTITAN_LIB_DIR "${QTITAN_ROOT}/lib")
endif()

# Нормализуем путь — убираем /../ чтобы линкер не спотыкался
cmake_path(NORMAL_PATH QTITAN_LIB_DIR)

if(NOT IS_DIRECTORY "${QTITAN_LIB_DIR}")
    message(FATAL_ERROR "Qtitan library directory not found: ${QTITAN_LIB_DIR}")
endif()
message(STATUS "Qtitan library dir: ${QTITAN_LIB_DIR}")

# Debug суффикс: QtitanBased2, QtitanGridd8 (d перед цифрой)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(QTN_D "d")
else()
    set(QTN_D "")
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanBase${QTN_D}2${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanGrid${QTN_D}8${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanStyle${QTN_D}2${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanFastInfoset${QTN_D}2${LIB_SUFFIX}"
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanBase${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanGrid${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanStyle${LIB_SUFFIX}"
        "${QTITAN_LIB_DIR}/${LIB_PREFIX}QtitanFastInfoset${LIB_SUFFIX}"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${QTITAN_ROOT}/include")
message(STATUS "Qtitan lib dir (normalized): ${QTITAN_LIB_DIR}")
# ============================================================================
# Копируем ASTRA.DLL на Windows
# ============================================================================
if(WIN32)
    set(ASTRA_FOLDER "bin")
else()
    set(ASTRA_FOLDER "lib")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASTRA_BIN_ROOT}/${ASTRA_FOLDER}/${LIB_PREFIX}astra${SHLIB_SUFFIX}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASTRA_BIN_ROOT}/${ASTRA_FOLDER}/${LIB_PREFIX}COMCK${SHLIB_SUFFIX}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/"
    COMMENT "Copying ASTRA"
)

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDES}
    ${QMCR_SOURCE_DIR}
    ${QMCR_SOURCE_DIR}/forms
)

# ============================================================================
# Target Properties
# ============================================================================
# WIN32_EXECUTABLE создаёт GUI приложение на Windows (без консольного окна)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

# ============================================================================
# Finalization
# ============================================================================
if(QT_VERSION_MAJOR EQUAL 6)
    # qt_finalize_executable() выполняет:
    # - Копирование Qt DLL в папку с .exe (Windows)
    # - Настройку QML ресурсов
    # - Добавление Windows manifest
    qt_finalize_executable(${PROJECT_NAME})
endif()
