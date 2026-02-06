cmake_minimum_required(VERSION 3.16)
project(qmcr VERSION 0.1 LANGUAGES CXX)

# ============================================================================
# Build Configuration
# ============================================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# from https://github.com/Vatsinator/vatsinator-legacy/blob/develop/CMakeLists.txt
# set configuration types (msvc/xcode)
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we actually need" FORCE
    )
endif()

set(SCI_DIR "${DEPENDENCIES_LOCATION}")

# ============================================================================
# Python Configuration (for codegen)
# ============================================================================
# For MinGW: specify Python3 paths
if(WIN32 AND NOT MSVC)
    set(Python3_EXECUTABLE "C:/msys64/mingw64/bin/python3.exe" CACHE FILEPATH "" FORCE)
    set(Python3_ROOT_DIR   "C:/msys64/mingw64"                 CACHE PATH "" FORCE)
endif()

# Visual Studio exports all symbols by default when building DLL
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ============================================================================
# Package Dependencies
# ============================================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickWidgets)
find_package(Astra REQUIRED)

set(PYTHON_MIN 3.10)
find_package(Python3 ${PYTHON_MIN} REQUIRED COMPONENTS Interpreter Development)
message("Python3_FOUND:             ${Python3_FOUND}")
message("Python3_VERSION:           ${Python3_VERSION}")
message("Python3_Development_FOUND: ${Python3_Development_FOUND}")
message("Python3_LIBRARIES:         ${Python3_LIBRARIES}")
message("Python3_LIBRARY_DIRS:      ${Python3_LIBRARY_DIRS}")
message("Python3_INCLUDE_DIR:       ${Python3_INCLUDE_DIRS}")
message("Python3_EXECUTABLE:        ${Python3_EXECUTABLE}")

link_directories(${Python3_LIBRARY_DIRS})
include_directories(${Python3_INCLUDE_DIRS})
include_directories(${Astra_INCLUDE_DIR})

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${SCI_DIR}/scintilla/include
    ${SCI_DIR}/scintilla/src
    ${SCI_DIR}/scintilla/qt/ScintillaEditBase
    ${SCI_DIR}/scintilla/qt/ScintillaEdit
    ${SCI_DIR}/lexilla/include
    include
)

if(QRASTR_BUILD_WITH_MCR)
    include_directories(
        # Add qrastr-specific includes if needed
    )
endif()

# ============================================================================
# Platform Detection для Scintilla/Lexilla
# ============================================================================
# Определяем CONFIG_DIR для путей к precompiled библиотекам
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CONFIG_DIR "Debug")
else()
    set(CONFIG_DIR "Release")
endif()

# Определяем PLATFORM_DIR
if(WIN32)
    if(MSVC)
        set(PLATFORM_DIR "win/msvc")
    elseif(MINGW)
        set(PLATFORM_DIR "win/mingw")
    endif()
elseif(UNIX AND NOT APPLE)
        set(PLATFORM_DIR "linux/gcc")
endif()

# Базовый путь к precompiled библиотекам
set(PRECOMPILED_BASE "${SCI_DIR}/compile")

# Путь к Scintilla
set(SCINTILLA_DIR "${PRECOMPILED_BASE}/ScintillaEdit/${PLATFORM_DIR}/${CONFIG_DIR}")
set(SCINTILLA_BASE_DIR "${PRECOMPILED_BASE}/ScintillaEditBase/${PLATFORM_DIR}/${CONFIG_DIR}")

# Путь к Lexilla
set(LEXILLA_DIR "${PRECOMPILED_BASE}/lexilla/${PLATFORM_DIR}/${CONFIG_DIR}")

# Добавляем include директории
if(EXISTS "${SCINTILLA_DIR}/include")
    include_directories("${SCINTILLA_DIR}/include")
endif()
if(EXISTS "${SCINTILLA_BASE_DIR}/include")
    include_directories("${SCINTILLA_BASE_DIR}/include")
endif()
if(EXISTS "${LEXILLA_DIR}/include")
    include_directories("${LEXILLA_DIR}/include")
endif()

# Добавляем lib директории для поиска библиотек
if(EXISTS "${SCINTILLA_DIR}/lib")
    link_directories("${SCINTILLA_DIR}/lib")
endif()
if(EXISTS "${SCINTILLA_BASE_DIR}/lib")
    link_directories("${SCINTILLA_BASE_DIR}/lib")
endif()
if(EXISTS "${LEXILLA_DIR}/lib")
    link_directories("${LEXILLA_DIR}/lib")
endif()

# Добавляем bin директории для поиска DLL (Windows)
if(WIN32)
    if(EXISTS "${SCINTILLA_DIR}/bin")
        link_directories("${SCINTILLA_DIR}/bin")
    endif()
    if(EXISTS "${SCINTILLA_BASE_DIR}/bin")
        link_directories("${SCINTILLA_BASE_DIR}/bin")
    endif()
    if(EXISTS "${LEXILLA_DIR}/bin")
        link_directories("${LEXILLA_DIR}/bin")
    endif()
endif()

# Добавляем старый путь для обратной совместимости
link_directories(${SCI_DIR}/bin)

message(STATUS "Scintilla/Lexilla paths:")
message(STATUS "  SCINTILLA_DIR: ${SCINTILLA_DIR}")
message(STATUS "  SCINTILLA_BASE_DIR: ${SCINTILLA_BASE_DIR}")
message(STATUS "  LEXILLA_DIR: ${LEXILLA_DIR}")

# ============================================================================
# Project Sources
# ============================================================================
set(TS_FILES qmcr_ru_RU.ts)
set(PROJECT_SOURCES)
list(APPEND PROJECT_SOURCES
    scihlp.h
    scihlp.cpp
    mcrwnd.h
    mcrwnd.cpp
    ${TS_FILES}
    forms/dlgfindrepl.h
    forms/dlgfindrepl.cpp
    forms/dlgfindrepl.ui
    pyhlp.h
    pyhlp.cpp
)

if(QRASTR_BUILD_WITH_MCR)
    list(APPEND PROJECT_SOURCES
        # qrastr-specific sources
    )
else()
    list(APPEND PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
    )
endif()

# ============================================================================
# Target Creation
# ============================================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(QRASTR_BUILD_WITH_MCR)
        add_library(qmcr SHARED
            ${PROJECT_SOURCES}
            tst_toolbox.h tst_toolbox.cpp tst_toolbox.ui
            tst2_dialog.h tst2_dialog.cpp tst2_dialog.ui
            tst.qml
        )
    else()
        qt_add_executable(qmcr
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            tst_toolbox.h tst_toolbox.cpp tst_toolbox.ui
            tst2_dialog.h tst2_dialog.cpp tst2_dialog.ui
            tst.qml
        )
    endif()
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(QRASTR_BUILD_WITH_MCR)
        add_library(qmcr SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(qmcr
            ${PROJECT_SOURCES}
            tst_toolbox.h tst_toolbox.cpp tst_toolbox.ui
            tst2_dialog.h tst2_dialog.cpp tst2_dialog.ui
            tst.qml
        )
        qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
    endif()
endif()

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(qmcr PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(qmcr PRIVATE Qt${QT_VERSION_MAJOR}::QuickWidgets)

# Python - не линкуем для MSVC (используем embeddable Python)
if(NOT MSVC)
    target_link_libraries(qmcr PRIVATE Python3::Python)
endif()

# ============================================================================
# Линковка Scintilla/Lexilla для разных платформ
# ============================================================================
if(MSVC)
    # MSVC: используем import libraries (.lib) и копируем DLL
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        # Debug build - ищем ScintillaEdit с суффиксом _d
        find_library(SCINTILLA_EDIT_LIB
            NAMES ScintillaEdit_d5 ScintillaEdit5
            PATHS "${SCINTILLA_DIR}/lib" "${SCI_DIR}/bin"
            NO_DEFAULT_PATH
        )

        if(SCINTILLA_EDIT_LIB)
            target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
            message(STATUS "Linked Scintilla (Debug): ${SCINTILLA_EDIT_LIB}")
        else()
            message(FATAL_ERROR "ScintillaEdit library not found for Debug configuration")
        endif()
    else()
        # Release build
        find_library(SCINTILLA_EDIT_LIB
            NAMES ScintillaEdit5
            PATHS "${SCINTILLA_DIR}/lib" "${SCI_DIR}/bin"
            NO_DEFAULT_PATH
        )

        if(SCINTILLA_EDIT_LIB)
            target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
            message(STATUS "Linked Scintilla (Release): ${SCINTILLA_EDIT_LIB}")
        else()
            message(FATAL_ERROR "ScintillaEdit library not found for Release configuration")
        endif()
    endif()

    # Lexilla: ищем import library для DLL (lexilla.lib находится в lib/)
    find_library(LEXILLA_IMPORT_LIB
        NAMES lexilla
        PATHS "${LEXILLA_DIR}/lib"
        NO_DEFAULT_PATH
    )

    if(LEXILLA_IMPORT_LIB)
        target_link_libraries(qmcr PRIVATE ${LEXILLA_IMPORT_LIB})
        message(STATUS "Using Lexilla import lib: ${LEXILLA_IMPORT_LIB}")

        # Копируем lexilla5.dll
        if(EXISTS "${LEXILLA_DIR}/bin/lexilla5.dll")
            file(COPY "${LEXILLA_DIR}/bin/lexilla5.dll"
                 DESTINATION "${CMAKE_BINARY_DIR}")
            message(STATUS "Copied lexilla5.dll to ${CMAKE_BINARY_DIR}")
        else()
            message(WARNING "lexilla5.dll not found at ${LEXILLA_DIR}/bin/")
        endif()
    else()
        message(FATAL_ERROR "Lexilla import library (lexilla.lib) not found in ${LEXILLA_DIR}/lib")
    endif()

elseif(MINGW)
    # MinGW: используем статические библиотеки (.a)
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit5 libScintillaEdit5
        PATHS "${SCINTILLA_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    find_library(SCINTILLA_EDIT_BASE_LIB
        NAMES ScintillaEditBase5 libScintillaEditBase5
        PATHS "${SCINTILLA_BASE_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    # Статическая библиотека Lexilla для MinGW
    find_library(LEXILLA_STATIC_LIB
        NAMES liblexilla.a liblexilla
        PATHS "${LEXILLA_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    message(STATUS "Scintilla libraries for MinGW:")
    message(STATUS "  ScintillaEdit: ${SCINTILLA_EDIT_LIB}")
    message(STATUS "  ScintillaEditBase: ${SCINTILLA_EDIT_BASE_LIB}")
    message(STATUS "  Lexilla (static): ${LEXILLA_STATIC_LIB}")

    if(SCINTILLA_EDIT_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
    elseif(SCINTILLA_EDIT_BASE_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_BASE_LIB})
        message(WARNING "ScintillaEdit not found, using ScintillaEditBase")
    else()
        message(FATAL_ERROR "Neither ScintillaEdit nor ScintillaEditBase found!")
    endif()

    if(LEXILLA_STATIC_LIB)
        target_link_libraries(qmcr PRIVATE ${LEXILLA_STATIC_LIB})
    else()
        message(FATAL_ERROR "Lexilla static library not found!")
    endif()

elseif(UNIX)
    # Linux: используем shared library (.so) для динамической загрузки

    # Для Scintilla - статическая линковка
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit libScintillaEdit
        PATHS "${SCINTILLA_DIR}/lib" "${SCINTILLA_DIR}/bin" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    if(NOT SCINTILLA_EDIT_LIB)
        find_library(SCINTILLA_EDIT_BASE_LIB
            NAMES ScintillaEditBase libScintillaEditBase
            PATHS "${SCINTILLA_BASE_DIR}/lib" "${SCINTILLA_BASE_DIR}/bin" "${SCI_DIR}/bin"
            NO_DEFAULT_PATH
        )
    endif()

    # Для Lexilla - НЕ линкуем напрямую, только добавляем в RPATH
    # Lexilla будет загружаться динамически через QLibrary

    message(STATUS "Scintilla libraries for Linux:")
    message(STATUS "  ScintillaEdit: ${SCINTILLA_EDIT_LIB}")
    message(STATUS "  ScintillaEditBase: ${SCINTILLA_EDIT_BASE_LIB}")
    message(STATUS "  Lexilla will be loaded dynamically from: ${LEXILLA_DIR}/bin")

    # Линкуем только Scintilla
    if(SCINTILLA_EDIT_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
    elseif(SCINTILLA_EDIT_BASE_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_BASE_LIB})
        message(WARNING "ScintillaEdit not found, using ScintillaEditBase")
    else()
        message(FATAL_ERROR "Neither ScintillaEdit nor ScintillaEditBase found!")
    endif()

    # Добавляем RPATH чтобы приложение могло найти lexilla5.so
    set_target_properties(qmcr PROPERTIES
        INSTALL_RPATH "${LEXILLA_DIR}/bin"
        BUILD_RPATH "${LEXILLA_DIR}/bin"
    )

    # Копируем lexilla5.so в output directory
    if(EXISTS "${LEXILLA_DIR}/bin/lexilla5.so")
        file(COPY "${LEXILLA_DIR}/bin/lexilla5.so"
             DESTINATION "${CMAKE_BINARY_DIR}")
        message(STATUS "Copied lexilla5.so to ${CMAKE_BINARY_DIR}")
    else()
        message(WARNING "lexilla5.so not found at ${LEXILLA_DIR}/bin/")
    endif()
endif()
# ============================================================================
# Target Properties
# ============================================================================
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qmcr)
endif()

if(NOT QRASTR_BUILD_WITH_MCR)
    set_target_properties(qmcr PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
endif()

# ============================================================================
# Installation
# ============================================================================
include(GNUInstallDirs)
install(TARGETS qmcr
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ============================================================================
# Qt Finalization
# ============================================================================
if(QT_VERSION_MAJOR EQUAL 6)
    if(NOT QRASTR_BUILD_WITH_MCR)
        qt_finalize_executable(qmcr)
    endif()
endif()
