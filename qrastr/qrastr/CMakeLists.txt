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
# Platform Detection and Compiler Configuration
# ============================================================================
include(cmake/platform_detection.cmake)

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
set(THIRDPARTY_DIR "${THIRDPARTY_DIR}" CACHE INTERNAL "")

if(NOT IS_DIRECTORY ${THIRDPARTY_DIR})
    message(FATAL_ERROR "THIRDPARTY_DIR not found: ${THIRDPARTY_DIR}")
else()
    message(STATUS "THIRDPARTY_DIR: ${THIRDPARTY_DIR}")
endif()

# Путь к библиотеке ASTRA с бинарными файлами
set(ASTRA_BIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/astra" CACHE PATH "Path to precompiled ASTRA library")

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
    set(QTITAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/QtitanDataGrid8.2.0_win_qt_6_7" CACHE PATH "Path to Qtitan")
else()
    set(QTITAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lib/qtitan8_lin_5_15" CACHE PATH "Path to Qtitan")
endif()
# ============================================================================
# Helper Function for Third-Party Libraries
# ============================================================================
function(add_thirdparty_library LIB_NAME LIB_PATH)
    set(options USE_DEBUG_SUFFIX)
    set(oneValueArgs LIB_REAL_NAME)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_LIB_REAL_NAME)
        set(ARG_LIB_REAL_NAME "${LIB_NAME}")
    endif()

    # Добавляем debug суффикс если требуется
    if(ARG_USE_DEBUG_SUFFIX AND CMAKE_BUILD_TYPE MATCHES Debug)
        set(ARG_LIB_REAL_NAME "${ARG_LIB_REAL_NAME}d")
    endif()

    set(PRECOMPILED_DIR "${THIRDPARTY_DIR}/compile/${LIB_NAME}/${PLATFORM_DIR}/${CONFIG_DIR}")

    find_library(${LIB_NAME}_LIBRARY
        NAMES ${ARG_LIB_REAL_NAME}
        PATHS ${PRECOMPILED_DIR}/lib
        NO_DEFAULT_PATH
    )

    set(${LIB_NAME}_INCLUDE_DIR "${PRECOMPILED_DIR}/include")
    if(NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
        set(${LIB_NAME}_INCLUDE_DIR "${LIB_NAME}_INCLUDE_DIR-NOTFOUND")
    endif()

    if(NOT ${LIB_NAME}_LIBRARY OR NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
        message(FATAL_ERROR
            "Precompiled ${LIB_NAME} not found in ${PRECOMPILED_DIR}\n"
            "  Library: ${${LIB_NAME}_LIBRARY}\n"
            "  Include: ${${LIB_NAME}_INCLUDE_DIR}\n"
            "  Looking for: ${ARG_LIB_REAL_NAME}")
    endif()

    add_library(${LIB_NAME} UNKNOWN IMPORTED)
    set_target_properties(${LIB_NAME} PROPERTIES
        IMPORTED_LOCATION "${${LIB_NAME}_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${${LIB_NAME}_INCLUDE_DIR}"
    )

    # Специальные настройки для spdlog
    if("${LIB_NAME}" STREQUAL "spdlog")
        set_target_properties(spdlog PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS SPDLOG_FMT_EXTERNAL
        )
        message(STATUS "spdlog uses SPDLOG_FMT_EXTERNAL")
    endif()

    message(STATUS "Using precompiled ${LIB_NAME}: ${${LIB_NAME}_LIBRARY}")

    # Специальные настройки для qtadvanceddocking
    if("${LIB_NAME}" STREQUAL "qtadvanceddocking")
        target_include_directories(qtadvanceddocking INTERFACE
            ${THIRDPARTY_DIR}/Qt-Advanced-Docking-System/src
        )
    endif()
endfunction()

# ============================================================================
# System Libraries
# - Boost и Eigen3: header-only библиотеки из thirdparty
# - fmt, spdlog: собираются через add_thirdparty_library
# - libpqxx: опционально собирается через add_thirdparty_library (зависит от наличия PostgreSQL)
# ============================================================================
# ============================================================================
# Boost (header-only) - одинаково для всех платформ
# ============================================================================
add_library(Boost::boost INTERFACE IMPORTED)
target_include_directories(Boost::boost INTERFACE "${THIRDPARTY_DIR}/boost_1_80_0")
message(STATUS "Boost (header-only): ${THIRDPARTY_DIR}/boost_1_80_0")

# ============================================================================
# Eigen3 (header-only) - одинаково для всех платформ
# ============================================================================
add_library(Eigen3::Eigen INTERFACE IMPORTED)
target_include_directories(Eigen3::Eigen INTERFACE "${THIRDPARTY_DIR}/eigen")
message(STATUS "Eigen3 (header-only): ${THIRDPARTY_DIR}/eigen")

# ============================================================================
# fmt и spdlog - header-only для всех модулей
# ============================================================================
add_library(Fmt::fmt INTERFACE IMPORTED)
target_include_directories(Fmt::fmt INTERFACE "${THIRDPARTY_DIR}/fmt/include")
target_compile_definitions(Fmt::fmt INTERFACE FMT_HEADER_ONLY)

add_library(Spdlog::spdlog INTERFACE IMPORTED)
target_include_directories(Spdlog::spdlog INTERFACE "${THIRDPARTY_DIR}/spdlog/include")
target_compile_definitions(Spdlog::spdlog INTERFACE
    SPDLOG_HEADER_ONLY
    SPDLOG_FMT_EXTERNAL
    FMT_HEADER_ONLY
)

# Создаём алиасы для совместимости
add_library(fmt::fmt ALIAS Fmt::fmt)
add_library(spdlog::spdlog ALIAS Spdlog::spdlog)

message(STATUS "fmt (header-only): ${THIRDPARTY_DIR}/fmt/include")
message(STATUS "spdlog (header-only with external fmt): ${THIRDPARTY_DIR}/spdlog/include")
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
# Передаем переменные в дочерние проекты через CACHE
set(THIRDPARTY_DIR "${THIRDPARTY_DIR}" CACHE INTERNAL "Thirdparty directory")
set(PLATFORM_DIR "${PLATFORM_DIR}" CACHE INTERNAL "Platform directory")
set(CONFIG_DIR "${CONFIG_DIR}" CACHE INTERNAL "Config directory")

add_subdirectory(${SRC_DIR}/plugins/rastr)
add_subdirectory(${SRC_DIR}/plugins/ti)
add_subdirectory(${SRC_DIR}/plugins/barsmdp)
message(STATUS "ASTRA plugins enabled")

# qmcr subdirectory
set(DEPENDENCIES_LOCATION "${THIRDPARTY_DIR}" CACHE PATH "Passes the path to the thirdparty to the qmcr subproject")

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

# Добавляем Boost и Eigen3 include directories
if(TARGET Boost::boost)
    get_target_property(BOOST_INCLUDES Boost::boost INTERFACE_INCLUDE_DIRECTORIES)
    if(BOOST_INCLUDES)
        list(APPEND PROJECT_INCLUDES ${BOOST_INCLUDES})
    endif()
endif()

if(TARGET Eigen3::Eigen)
    get_target_property(EIGEN3_INCLUDES Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
    if(EIGEN3_INCLUDES)
        list(APPEND PROJECT_INCLUDES ${EIGEN3_INCLUDES})
    endif()
endif()

# Добавляем ASTRA include директории
list(APPEND PROJECT_INCLUDES
    ${ASTRA_BIN_ROOT}/include
    ${ASTRA_BIN_ROOT}/include/comck
)
# ============================================================================
# Source Files
# ============================================================================
set(PROJECT_SOURCES
    ${APP_DIR}/main.cpp

    # Main
    ${APP_DIR}/main/app.h
    ${APP_DIR}/main/app.cpp
    ${APP_DIR}/main/mainwindow.cpp
    ${APP_DIR}/main/mainwindow.h

    # Data
    ${APP_DIR}/data/rtablesdatamanager.h
    ${APP_DIR}/data/rtablesdatamanager.cpp
    ${APP_DIR}/data/rmodel.cpp
    ${APP_DIR}/data/rmodel.h
    ${APP_DIR}/data/rdata.h
    ${APP_DIR}/data/rdata.cpp
    ${APP_DIR}/data/IDataBlocksWrappers.h
    ${APP_DIR}/data/IDataBlocksWrappers.cpp
    ${APP_DIR}/data/QDataBlocks.h
    ${APP_DIR}/data/QDataBlocks.cpp

    # UI Components
    ${APP_DIR}/ui_components/rtabwidget.h
    ${APP_DIR}/ui_components/rtabwidget.cpp
    ${APP_DIR}/ui_components/rtableview.h
    ${APP_DIR}/ui_components/rtableview.cpp
    ${APP_DIR}/ui_components/filtertableheader.h
    ${APP_DIR}/ui_components/filtertableheader.cpp
    ${APP_DIR}/ui_components/filterlineedit.h
    ${APP_DIR}/ui_components/filterlineedit.cpp
    ${APP_DIR}/ui_components/qtitangrid.h
    ${APP_DIR}/ui_components/qtitangrid.cpp

    # Delegates
    ${APP_DIR}/delegates/delegatecombobox.cpp
    ${APP_DIR}/delegates/delegatecombobox.h
    ${APP_DIR}/delegates/delegatedoubleitem.h
    ${APP_DIR}/delegates/delegatedoubleitem.cpp
    ${APP_DIR}/delegates/delegatecheckbox.h
    ${APP_DIR}/delegates/delegatecheckbox.cpp

    # Forms
    ${APP_DIR}/forms/ColPropForm.h
    ${APP_DIR}/forms/ColPropForm.cpp
    ${APP_DIR}/forms/ColPropForm.ui
    ${APP_DIR}/forms/formsettings.h
    ${APP_DIR}/forms/formsettings.cpp
    ${APP_DIR}/forms/formsettingsdatas.h
    ${APP_DIR}/forms/formsettingsdatas.cpp
    ${APP_DIR}/forms/formsettingsdatas.ui
    ${APP_DIR}/forms/formsettingsforms.h
    ${APP_DIR}/forms/formsettingsforms.cpp
    ${APP_DIR}/forms/formsettingsforms.ui
    ${APP_DIR}/forms/formsettingsonloadfiles.h
    ${APP_DIR}/forms/formsettingsonloadfiles.cpp
    ${APP_DIR}/forms/formsettingsonloadfiles.ui
    ${APP_DIR}/forms/formsettingsstackeditem.h
    ${APP_DIR}/forms/formsettingsstackeditem.cpp
    ${APP_DIR}/forms/formsettingsonloadtemplates.h
    ${APP_DIR}/forms/formsettingsonloadtemplates.cpp
    ${APP_DIR}/forms/formsettingsonloadtemplates.ui
    ${APP_DIR}/forms/formsettingsonloadforms.h
    ${APP_DIR}/forms/formsettingsonloadforms.cpp
    ${APP_DIR}/forms/formsettingsonloadforms.ui
    ${APP_DIR}/forms/formfilenew.h
    ${APP_DIR}/forms/formfilenew.cpp
    ${APP_DIR}/forms/formfilenew.ui
    ${APP_DIR}/forms/formselection.h
    ${APP_DIR}/forms/formselection.cpp
    ${APP_DIR}/forms/formselection.ui
    ${APP_DIR}/forms/formprotocol.h
    ${APP_DIR}/forms/formprotocol.cpp
    ${APP_DIR}/forms/formprotocol.ui
    ${APP_DIR}/forms/formexportcsv.h
    ${APP_DIR}/forms/formexportcsv.cpp
    ${APP_DIR}/forms/formexportcsv.ui
    ${APP_DIR}/forms/formgroupcorrection.h
    ${APP_DIR}/forms/formgroupcorrection.cpp
    ${APP_DIR}/forms/formgroupcorrection.ui
    ${APP_DIR}/forms/formimportcsv2.h
    ${APP_DIR}/forms/formimportcsv2.cpp
    ${APP_DIR}/forms/formimportcsv2.ui
    ${APP_DIR}/forms/formcalcidop.h
    ${APP_DIR}/forms/formcalcidop.cpp
    ${APP_DIR}/forms/formcalcidop.ui
    ${APP_DIR}/forms/formsaveall.h
    ${APP_DIR}/forms/formsaveall.cpp
    ${APP_DIR}/forms/formsaveall.ui

    # Protocol tree
    ${APP_DIR}/protocol_tree/protocoltreeitem.h
    ${APP_DIR}/protocol_tree/protocoltreeitem.cpp
    ${APP_DIR}/protocol_tree/protocoltreemodel.h
    ${APP_DIR}/protocol_tree/protocoltreemodel.cpp

    # Conditional formatting
    ${APP_DIR}/formatting/CondFormat.h
    ${APP_DIR}/formatting/CondFormat.cpp
    ${APP_DIR}/formatting/CondFormatManager.h
    ${APP_DIR}/formatting/CondFormatManager.cpp
    ${APP_DIR}/formatting/CondFormatManager.ui
    ${APP_DIR}/formatting/Palette.h
    ${APP_DIR}/formatting/Palette.cpp
    ${APP_DIR}/formatting/condformatjson.h
    ${APP_DIR}/formatting/condformatjson.cpp

    # Utilities
    ${APP_DIR}/utilities/utils.h
    ${APP_DIR}/utilities/singleton_dclp.hpp
    ${APP_DIR}/utilities/params.h
    ${APP_DIR}/utilities/params.cpp
    ${APP_DIR}/utilities/common_qrastr.h
    ${APP_DIR}/utilities/common_qrastr.cpp
    ${APP_DIR}/utilities/rastrhlp.h
    ${APP_DIR}/utilities/rastrhlp.cpp
    ${APP_DIR}/utilities/Settings.h
    ${APP_DIR}/utilities/Settings.cpp
    ${APP_DIR}/utilities/string_bool.h
    ${APP_DIR}/utilities/string_bool.cpp
    ${APP_DIR}/utilities/linkedform.h
    ${APP_DIR}/utilities/linkedform.cpp

    # SDL
    ${APP_DIR}/sdl/SDLChild.h
    ${APP_DIR}/sdl/SDLChild.cpp

    # Test hints
    ${APP_DIR}/test/tsthints.h
    ${APP_DIR}/test/tsthints.cpp
    ${APP_DIR}/test/tsthintsmvc.h
    ${APP_DIR}/test/tsthintsmvc.cpp
    ${APP_DIR}/test/testmodel.h
    ${APP_DIR}/test/testmodel.cpp

    # Plugin
    ${SRC_DIR}/plugins/rastr/plugin_interfaces.h

    # ASTRA
    ${APP_DIR}/astra/astra_shared.h
    ${APP_DIR}/astra/qastra_events_data.h
    ${APP_DIR}/astra/qastra.h
    ${APP_DIR}/astra/qastra.cpp
    ${APP_DIR}/astra/qti.h
    ${APP_DIR}/astra/qti.cpp
    ${APP_DIR}/astra/qbarsmdp.h
    ${APP_DIR}/astra/qbarsmdp.cpp
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
)

if(QT_VERSION_MAJOR EQUAL 6)
    list(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

set(THIRD_PARTY_LIBS
    Eigen3::Eigen
    fmt::fmt
    spdlog::spdlog
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

set(QTITAN_LIBS
    Qtitan::QtitanBase2
    Qtitan::QtitanGrid8
    Qtitan::QtitanStyle2
    Qtitan::QtitanFastInfoset2
)

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
list(APPEND PROJECT_LINK_LIBS ${QTITAN_LIBS})
list(APPEND PROJECT_LINK_LIBS ${ASTRA_LIBS})

# 3) Применяем один вызов для link'ов
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LINK_LIBS})
# ============================================================================
# Qtitan Libraries Configuration
# ============================================================================
# Универсальная функция для добавления Qtitan библиотек
include(cmake/add_qtitan_library.cmake)

# Использование - одинаково для всех платформ!
add_qtitan_library(QtitanBase2)
add_qtitan_library(QtitanGrid8)
add_qtitan_library(QtitanStyle2)
add_qtitan_library(QtitanFastInfoset2)

# Link Qtitan Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qtitan::QtitanBase2
    Qtitan::QtitanGrid8
    Qtitan::QtitanStyle2
    Qtitan::QtitanFastInfoset2
)

# ============================================================================
# ASTRA Library Linking
# ============================================================================
# Копируем DLL на Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ASTRA_BIN_ROOT}/bin/astra.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ASTRA_BIN_ROOT}/bin/COMCK.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/"
        COMMENT "Copying ASTRA DLLs"
    )
endif()

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
