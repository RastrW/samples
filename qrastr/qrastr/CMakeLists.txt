# ============================================================================
# Универсальный CMakeLists.txt для qrastr
# Поддерживаемые платформы:
#   - *Windows (MSVC, MinGW)
#   - Ubuntu 24.04 (gcc, системные библиотеки через apt)
#   - AstraLinux 1.7 (clang-13, библиотеки собраны в ~/.local)
# ============================================================================

cmake_minimum_required(VERSION 3.23)
message(STATUS "CMake version: ${CMAKE_VERSION}")

# ============================================================================
# Определение дистрибутива Linux и настройка компилятора ДО project()
# ============================================================================
# Эта секция должна быть ДО project(), т.к. выбор компилятора
# должен произойти до инициализации проекта

if(UNIX AND NOT APPLE AND NOT WIN32)
    # Читаем /etc/os-release для определения дистрибутива
    if(EXISTS "/etc/os-release")
        file(STRINGS "/etc/os-release" OS_RELEASE_CONTENTS)

        # Ищем строку ID="astra" или NAME="Astra Linux"
        set(IS_ASTRA_LINUX FALSE)
        foreach(line ${OS_RELEASE_CONTENTS})
            if(line MATCHES "ID=\"?astra\"?" OR line MATCHES "NAME=.*[Aa]stra")
                set(IS_ASTRA_LINUX TRUE)
                message(STATUS "Detected AstraLinux distribution")
                break()
            endif()
        endforeach()

        if(NOT IS_ASTRA_LINUX)
            # Проверяем, возможно это Ubuntu
            foreach(line ${OS_RELEASE_CONTENTS})
                if(line MATCHES "ID=\"?ubuntu\"?" OR line MATCHES "NAME=.*[Uu]buntu")
                    message(STATUS "Detected Ubuntu distribution")
                    break()
                endif()
            endforeach()
        endif()
    else()
        message(WARNING "/etc/os-release not found, cannot auto-detect Linux distribution")
        set(IS_ASTRA_LINUX FALSE)
    endif()

    # Для AstraLinux принудительно используем clang-13
    # На AstraLinux встроенные gcc8 и clang11 слишком старые для C++17 features
    if(IS_ASTRA_LINUX)
        if(NOT CMAKE_C_COMPILER)
            set(CMAKE_C_COMPILER "/usr/bin/clang-13" CACHE FILEPATH "C compiler" FORCE)
        endif()
        if(NOT CMAKE_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER "/usr/bin/clang++-13" CACHE FILEPATH "C++ compiler" FORCE)
        endif()
        message(STATUS "Using Clang-13 compiler for AstraLinux")
        message(STATUS "  CC:  ${CMAKE_C_COMPILER}")
        message(STATUS "  CXX: ${CMAKE_CXX_COMPILER}")
    else()
        # Для Ubuntu используем системный компилятор (gcc по умолчанию)
        message(STATUS "Using default system compiler (gcc for Ubuntu)")
    endif()
endif()

# ============================================================================
# Инициализация проекта
# ============================================================================
cmake_policy(SET CMP0167 NEW)  # In CMake 3.30+, the FindBoost module has been removed. Using BoostConfig.cmake
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
add_compile_definitions(QICSGRID_NO)
add_compile_definitions(SDL_NO)
add_compile_definitions(ASTRA_NIX)
add_compile_definitions(QTN_NAMESPACE)

# Debug-specific definitions
add_compile_definitions(
    $<$<CONFIG:Debug>:q4_INLINE=0>  # Отключает inline функции в Debug
    $<$<CONFIG:Debug>:YYDEBUG>       # Включает отладочный вывод для yacc/bison парсеров
)

# Windows-specific definitions
if(WIN32)
    add_compile_definitions(COMPILE_WIN)

    if(MSVC)
        # Подавление предупреждений о deprecated Microsoft extensions (?)
        add_compile_definitions(_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
        # Отключение макросов min/max из windows.h (конфликт с std::min/max)
        add_compile_definitions(NOMINMAX)
    endif()

    # Настройка CRT (C Runtime Library) для Release сборок
    # Используем статическую линковку CRT для уменьшения зависимостей
    if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# ============================================================================
# Platform Detection
# ============================================================================
if(WIN32)
    # Windows: определяем компилятор (MSVC или MinGW)
    if(MSVC)
        set(COMPILER "msvc")
        set(LIB_PREFIX "")      # MSVC библиотеки без префикса
        set(LIB_SUFFIX ".lib")  # MSVC статические библиотеки
    elseif(MINGW)
        set(COMPILER "mingw")
        set(LIB_PREFIX "lib")   # MinGW использует Unix-style префикс
        set(LIB_SUFFIX ".a")    # MinGW статические библиотеки
    else()
        message(WARNING "Unknown Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
        set(COMPILER "unknown")
    endif()
    set(PLATFORM_DIR "win/${COMPILER}")

elseif(UNIX AND NOT APPLE)
    # Linux: используем префикс "lib" и суффикс ".so" для динамических библиотек
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".so")
    set(PLATFORM_DIR "linux")

    # Определяем компилятор для выбора правильных precompiled библиотек
    # Библиотеки скомпилированные разными компиляторами могут быть несовместимы
    if(IS_ASTRA_LINUX)
        set(COMPILER "clang")  # AstraLinux использует clang-13
    else()
        set(COMPILER "gcc")    # Ubuntu использует gcc
    endif()

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Определяем суффикс для Debug версий Qtitan библиотек
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(LIBEND_SUFFIX "d")
    set(CONFIG_DIR "Debug")
else()
    set(LIBEND_SUFFIX "")
    set(CONFIG_DIR "Release")
endif()

message(STATUS "Platform configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${COMPILER}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform directory: ${PLATFORM_DIR}")

# ============================================================================
# Options
# ============================================================================
option(QRASTR_BUILD_WITH_MCR "Build qmcr as library for qrastr" ON)
option(BUILD_WITH_ASTRA "Build with ASTRA library" OFF)
option(USE_PRECOMPILED_LIBS "Use precompiled libraries instead of building from source" OFF)

message(STATUS "Build options:")
message(STATUS "  QRASTR_BUILD_WITH_MCR: ${QRASTR_BUILD_WITH_MCR}")
message(STATUS "  BUILD_WITH_ASTRA: ${BUILD_WITH_ASTRA}")
message(STATUS "  USE_PRECOMPILED_LIBS: ${USE_PRECOMPILED_LIBS}")

# ============================================================================
# Path Configuration - пути к внешним зависимостям
# ============================================================================

# Thirdparty directory - сторонние библиотеки
set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty")

if(NOT IS_DIRECTORY ${THIRDPARTY_DIR})
    message(FATAL_ERROR "THIRDPARTY_DIR not found: ${THIRDPARTY_DIR}")
else()
    message(STATUS "THIRDPARTY_DIR: ${THIRDPARTY_DIR}")
endif()

# ASTRA library location
if(BUILD_WITH_ASTRA)
    set(ASTRA_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../astra" CACHE PATH "Path to ASTRA library")

    if(NOT IS_DIRECTORY ${ASTRA_LOCATION})
        message(FATAL_ERROR "ASTRA_LOCATION not found: ${ASTRA_LOCATION}")
    else()
        message(STATUS "ASTRA_LOCATION: ${ASTRA_LOCATION}")
    endif()
endif()

# Qtitan location - коммерческая библиотека UI компонентов
if(WIN32)
    set(QTITAN_ROOT "C:/Qt/QtitanComponents2025.2.0" CACHE PATH "Path to Qtitan")
else()
    set(QTITAN_ROOT "$ENV{HOME}/Qt/Developer Machines/QtitanComponents2025.2.0 Demo" CACHE PATH "Path to Qtitan")
endif()

if(NOT IS_DIRECTORY ${QTITAN_ROOT})
    message(FATAL_ERROR "Qtitan not found at: ${QTITAN_ROOT}")
else()
    message(STATUS "QTITAN_ROOT: ${QTITAN_ROOT}")
endif()

# ============================================================================
# Helper Function for Third-Party Libraries
# Эта функция управляет подключением сторонних библиотек в двух режимах:
# 1. USE_PRECOMPILED_LIBS=ON: использовать предсобранные .so/.lib файлы
# 2. USE_PRECOMPILED_LIBS=OFF: собрать библиотеку из исходников через add_subdirectory
# ============================================================================
function(add_thirdparty_library LIB_NAME LIB_PATH)
    # Парсим опциональные аргументы
    set(options "")
    set(oneValueArgs LIB_REAL_NAME)  # Реальное имя библиотеки (если отличается от LIB_NAME)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Если LIB_REAL_NAME не указан, используем LIB_NAME
    if(NOT ARG_LIB_REAL_NAME)
        set(ARG_LIB_REAL_NAME "${LIB_NAME}")
    endif()

    if(USE_PRECOMPILED_LIBS)
        # ====================================================================
        # MODE 1: Using precompiled libraries
        # Ищем готовые .so/.lib файлы в thirdparty/compile/
        # ====================================================================
        set(PRECOMPILED_DIR "${THIRDPARTY_DIR}/compile/${LIB_NAME}/${PLATFORM_DIR}/${CONFIG_DIR}")

        # 1. Ищем скомпилированную библиотеку
        find_library(${LIB_NAME}_LIBRARY
            NAMES ${ARG_LIB_REAL_NAME}
            PATHS ${PRECOMPILED_DIR}/lib
            NO_DEFAULT_PATH  # Не искать в системных путях
        )
        # find_library автоматически определяет приоритет:
        # - Shared (.so/.dll) — если BUILD_SHARED_LIBS=ON (по умолчанию)
        # - Static (.a/.lib) — если BUILD_SHARED_LIBS=OFF

        # 2. Include directory - проверяем существование директории напрямую
        set(${LIB_NAME}_INCLUDE_DIR "${PRECOMPILED_DIR}/include")
        if(NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
            set(${LIB_NAME}_INCLUDE_DIR "${LIB_NAME}_INCLUDE_DIR-NOTFOUND")
        endif()

        # 3. Проверяем, что оба компонента найдены
        if(NOT ${LIB_NAME}_LIBRARY OR NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
            message(FATAL_ERROR
                "Precompiled ${LIB_NAME} not found in ${PRECOMPILED_DIR}\n"
                "  Library: ${${LIB_NAME}_LIBRARY}\n"
                "  Include: ${${LIB_NAME}_INCLUDE_DIR}")
        endif()

        # 4. Создаём IMPORTED target (виртуальную цель CMake для внешней библиотеки)
        add_library(${LIB_NAME} UNKNOWN IMPORTED)

        # 5. Устанавливаем свойства (путь к .so/.lib и include директории)
        set_target_properties(${LIB_NAME} PROPERTIES
            IMPORTED_LOCATION "${${LIB_NAME}_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${${LIB_NAME}_INCLUDE_DIR}"
        )
        message(STATUS "Using precompiled ${LIB_NAME}: ${${LIB_NAME}_LIBRARY}")

    else()
        # ====================================================================
        # MODE 2: Build from source
        # Добавляем подпроект через add_subdirectory и собираем из исходников
        # ====================================================================
        if(NOT EXISTS "${LIB_PATH}/CMakeLists.txt")
            message(FATAL_ERROR "${LIB_NAME} source not found at: ${LIB_PATH}")
        endif()

        message(STATUS "Building ${LIB_NAME} from source")

        # Добавляем подпроект
        # LIB_PATH - путь к исходникам библиотеки
        # CMAKE_BINARY_DIR/${LIB_NAME} - куда складывать build файлы
        add_subdirectory(${LIB_PATH} ${CMAKE_BINARY_DIR}/${LIB_NAME})
    endif()
endfunction()

# ============================================================================
# System Libraries - поиск системных библиотек
# Стратегия зависит от платформы:
# - Ubuntu: библиотеки установлены через apt, ищем в стандартных путях
# - AstraLinux: библиотеки собраны вручную в ~/.local, добавляем этот путь
# - Windows: используем предсобранные версии из thirdparty
# ============================================================================

if(UNIX AND NOT APPLE)
    if(IS_ASTRA_LINUX)
        # Для AstraLinux добавляем ~/.local в начало списка поиска
        # PREPEND = добавить в начало (высший приоритет)
        list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")
        message(STATUS "AstraLinux: Added ~/.local to CMAKE_PREFIX_PATH")
    endif()

    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
endif()

# ----------------------------------------------------------------------------
# Boost
# ----------------------------------------------------------------------------
# Ubuntu: boost установлен через apt
# AstraLinux: boost собран вручную в ~/.local
# Windows: из thirdparty
find_package(Boost 1.80 REQUIRED COMPONENTS filesystem system)
message(STATUS "Boost found: ${Boost_VERSION} at ${Boost_INCLUDE_DIRS}")

# ----------------------------------------------------------------------------
# Eigen3 - header-only библиотека линейной алгебры
# ----------------------------------------------------------------------------
# Ubuntu: eigen3-dev через apt
# AstraLinux: установлен через apt (версия 3.3.7)
find_package(Eigen3 3.3 REQUIRED)
message(STATUS "Eigen3 found: ${EIGEN3_VERSION} at ${EIGEN3_INCLUDE_DIR}")

# ----------------------------------------------------------------------------
# fmt - библиотека форматирования строк (аналог std::format для C++17)
# ----------------------------------------------------------------------------
# Ubuntu: libfmt-dev через apt
# AstraLinux: fmt 10.2.1 собран вручную в ~/.local
find_package(fmt 10.0 REQUIRED)
message(STATUS "fmt found: ${fmt_VERSION}")

# ----------------------------------------------------------------------------
# spdlog - библиотека логирования
# ----------------------------------------------------------------------------
# Ubuntu: libspdlog-dev через apt
# AstraLinux: spdlog 1.12.0 собран вручную в ~/.local
find_package(spdlog 1.12 REQUIRED)
message(STATUS "spdlog found: ${spdlog_VERSION}")

# ----------------------------------------------------------------------------
# libpqxx - C++ клиент для PostgreSQL
# ----------------------------------------------------------------------------
# Ubuntu: libpqxx-dev через apt
# AstraLinux: libpqxx 7.7.5 собран вручную в ~/.local
# Проблема: libpqxxConfig.cmake может отсутствовать в старых версиях

# Сначала пробуем найти через CMake config
find_package(libpqxx 7.0 QUIET)
message(STATUS "libpqxx found: ${libpqxx_VERSION}")
# ============================================================================
# Qt
# ============================================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core            # Базовые классы Qt
    Widgets         # GUI виджеты
    PrintSupport    # Поддержка печати
    Xml             # XML парсер
    Test            # Тестовый фреймворк
    LinguistTools   # Инструменты для интернационализации (i18n)
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
if(BUILD_WITH_ASTRA)
    add_subdirectory(plugins/rastr)
    add_subdirectory(plugins/ti)
    add_subdirectory(plugins/barsmdp)
    message(STATUS "ASTRA plugins enabled")
endif()

# ============================================================================
# qmcr subdirectory
# ============================================================================
set(DEPENDENCIES_LOCATION "${THIRDPARTY_DIR}" CACHE PATH "Passes the path to the thirdparty to the qmcr subproject")
add_subdirectory(qmcr ${CMAKE_BINARY_DIR}/generated/qmcr)
message(STATUS "qmcr subproject added")

# ----------------------------------------------------------------------------
# Metakit
# ----------------------------------------------------------------------------
if(USE_PRECOMPILED_LIBS)
    # Metakit собирается только из precompiled (проблемы с новыми компиляторами)
    add_thirdparty_library(metakit "${THIRDPARTY_DIR}/metakit"
        LIB_REAL_NAME "mk4")  # Реальное имя библиотеки - libmk4.so
endif()

# ----------------------------------------------------------------------------
# Qt-Advanced-Docking-System
# ----------------------------------------------------------------------------
add_thirdparty_library(qtadvanceddocking "${THIRDPARTY_DIR}/Qt-Advanced-Docking-System"
    LIB_REAL_NAME "qtadvanceddocking-qt6${LIBEND_SUFFIX}")

# ----------------------------------------------------------------------------
# SDL3 и SDL3_image
# ----------------------------------------------------------------------------
# Временно уменьшаем уровень логирования CMake при сборке SDL
if(NOT USE_PRECOMPILED_LIBS)
    set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
endif()

add_thirdparty_library(SDL3 "${THIRDPARTY_DIR}/SDL")
add_thirdparty_library(SDL3_image "${THIRDPARTY_DIR}/SDL_image")

# Восстанавливаем уровень логирования
if(NOT USE_PRECOMPILED_LIBS)
    set(CMAKE_MESSAGE_LOG_LEVEL STATUS)
endif()

# ----------------------------------------------------------------------------
# Scintilla
# ----------------------------------------------------------------------------
add_thirdparty_library(scintilla "${THIRDPARTY_DIR}/scintilla")

# ============================================================================
# Include Directories
# ============================================================================
set(PROJECT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}  # Текущая директория проекта
    ${Boost_INCLUDE_DIRS}        # Boost заголовки
    ${EIGEN3_INCLUDE_DIR}        # Eigen3 заголовки
    ${QTITAN_ROOT}/include       # Qtitan заголовки
)

# Добавляем ASTRA include директории, если ASTRA включена
if(BUILD_WITH_ASTRA)
    list(APPEND PROJECT_INCLUDES
        ${ASTRA_LOCATION}
        ${ASTRA_LOCATION}/../comck
        ${ASTRA_LOCATION}/fmt/include/
        ${ASTRA_LOCATION}/../../flytfs/IPlainRastrTest/
    )
endif()

# ============================================================================
# Source Files
# ============================================================================
set(PROJECT_SOURCES
    # Main
    app.h
    app.cpp
    main.cpp
    mainwindow.cpp
    mainwindow.h

    # MDI
    mdiChildTable.h
    mdiChildTable.cpp
    mdiChildGrid.h
    mdiChildGrid.cpp
    mdiChildHeaderGrid.h
    mdiChildHeaderGrid.cpp

    # Data
    rastrdatamodel.h
    rastrdatamodel.cpp
    rmodel.cpp
    rmodel.h
    testmodel.h
    testmodel.cpp
    rdata.h
    rdata.cpp
    rtablesdatamanager.h
    rtablesdatamanager.cpp
    IDataBlocksWrappers.h
    IDataBlocksWrappers.cpp
    QDataBlocks.h
    QDataBlocks.cpp

    # UI Components
    rtabwidget.h
    rtabwidget.cpp
    rtableview.h
    rtableview.cpp
    filtertableheader.h
    filtertableheader.cpp
    filterlineedit.h
    filterlineedit.cpp
    qtitangrid.h
    qtitangrid.cpp

    # Delegates
    delegatecombobox.cpp
    delegatecombobox.h
    delegatedoubleitem.h
    delegatedoubleitem.cpp
    delegatecheckbox.h
    delegatecheckbox.cpp

    # Forms
    ColPropForm.h
    ColPropForm.cpp
    ColPropForm.ui
    formsettings.h
    formsettings.cpp
    formsettingsdatas.h
    formsettingsdatas.cpp
    formsettingsdatas.ui
    formsettingsforms.h
    formsettingsforms.cpp
    formsettingsforms.ui
    formsettingsonloadfiles.h
    formsettingsonloadfiles.cpp
    formsettingsonloadfiles.ui
    formsettingsstackeditem.h
    formsettingsstackeditem.cpp
    formsettingsonloadtemplates.h
    formsettingsonloadtemplates.cpp
    formsettingsonloadtemplates.ui
    formsettingsonloadforms.h
    formsettingsonloadforms.cpp
    formsettingsonloadforms.ui
    formfilenew.h
    formfilenew.cpp
    formfilenew.ui
    formselection.h
    formselection.cpp
    formselection.ui
    formprotocol.h
    formprotocol.cpp
    formprotocol.ui
    formexportcsv.h
    formexportcsv.cpp
    formexportcsv.ui
    formgroupcorrection.h
    formgroupcorrection.cpp
    formgroupcorrection.ui
    formimportcsv2.h
    formimportcsv2.cpp
    formimportcsv2.ui
    formcalcidop.h
    formcalcidop.cpp
    formcalcidop.ui
    formsaveall.h
    formsaveall.cpp
    formsaveall.ui

    # Protocol tree
    protocoltreeitem.h
    protocoltreeitem.cpp
    protocoltreemodel.h
    protocoltreemodel.cpp

    # Conditional formatting
    CondFormat.h
    CondFormat.cpp
    CondFormatManager.h
    CondFormatManager.cpp
    CondFormatManager.ui
    Palette.h
    Palette.cpp
    condformatjson.h
    condformatjson.cpp

    # Utilities
    utils.h
    singleton_dclp.hpp
    params.h
    params.cpp
    common_qrastr.h
    common_qrastr.cpp
    rastrhlp.h
    rastrhlp.cpp
    Settings.h
    Settings.cpp
    string_bool.h
    string_bool.cpp
    linkedform.h
    linkedform.cpp

    # SDL
    SDLChild.h
    SDLChild.cpp

    # Test hints
    tsthints.h
    tsthints.cpp
    tsthintsmvc.h
    tsthintsmvc.cpp

    # Plugin
    plugins/rastr/plugin_interfaces.h

    # ASTRA
    astra_shared.h
    qastra_events_data.h

    # Resources
    qrastr.qrc
    appsettings.json
)

# ASTRA-specific sources
if(BUILD_WITH_ASTRA)
    list(APPEND PROJECT_SOURCES
        qastra.h
        qastra.cpp
        qti.h
        qti.cpp
        qbarsmdp.h
        qbarsmdp.cpp
        ${ASTRA_LOCATION}/ResourceManager.cpp
    )
endif()

# Translation files
set(TS_FILES qrastr_ru_RU.ts)

# ============================================================================
# Executable - создание исполняемого файла
# ============================================================================
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    # Qt6: используем qt_add_executable
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION  # Вызовем qt_finalize_executable() вручную в конце
        ${PROJECT_SOURCES}
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
    )

    qt5_create_translation(QM_FILES
        ${PROJECT_SOURCES}
        ${TS_FILES}
    )
endif()

# ============================================================================
# Link Libraries - линковка библиотек к исполняемому файлу
# ============================================================================
# ----------------------------------------------------------------------------
# System libraries
# ----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    fmt::fmt
    spdlog::spdlog
    ${PQXX_LIBRARIES}
)

# ----------------------------------------------------------------------------
# Qt libraries
# ----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Test
)

# Qt6 требует Core5Compat для обратной совместимости с Qt5 API
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

# ----------------------------------------------------------------------------
# Third-party libraries
# ----------------------------------------------------------------------------
# Metakit линкуется только если используются precompiled библиотеки
if(USE_PRECOMPILED_LIBS)
    target_link_libraries(${PROJECT_NAME} PRIVATE metakit)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    qtadvanceddocking
    SDL3
    SDL3_image
    scintilla
    qmcr
)

# ----------------------------------------------------------------------------
# Qtitan libraries
# ----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${QTITAN_ROOT}/bin/${LIB_PREFIX}QtitanBase${LIB_SUFFIX}
    ${QTITAN_ROOT}/bin/${LIB_PREFIX}QtitanGrid${LIB_SUFFIX}
)

# ----------------------------------------------------------------------------
# ASTRA library
# ----------------------------------------------------------------------------
if(BUILD_WITH_ASTRA)
    target_link_libraries(${PROJECT_NAME} PRIVATE astra)
endif()

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_INCLUDES}
    ${PQXX_INCLUDE_DIRS}
)

# ============================================================================
# Target Properties
# ============================================================================
# WIN32_EXECUTABLE создаёт GUI приложение на Windows (без консольного окна)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

# ============================================================================
# Finalization - финальные настройки для Qt6
# ============================================================================
if(QT_VERSION_MAJOR EQUAL 6)
    # qt_finalize_executable() выполняет:
    # - Копирование Qt DLL в папку с .exe (Windows)
    # - Настройку QML ресурсов
    # - Добавление Windows manifest
    qt_finalize_executable(${PROJECT_NAME})
endif()
