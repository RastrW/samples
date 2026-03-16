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
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING
        "Reset the configurations to what we actually need" FORCE)
endif()

set(SCI_DIR "${DEPENDENCIES_LOCATION}")
# ============================================================================
# Package Dependencies
# ============================================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(PYTHON_MIN 3.10)
# Для Windows python 3.12 (в соответствии с astra_py)
# Для работы python на Windows необходимо указать путь к нему в системной переменной PATH
# Для Linux python 3.11 (версия пакета в системе)
find_package(Python3 ${PYTHON_MIN} REQUIRED COMPONENTS Interpreter Development)
message("Python3_FOUND:             ${Python3_FOUND}")
message("Python3_VERSION:           ${Python3_VERSION}")
message("Python3_Development_FOUND: ${Python3_Development_FOUND}")
message("Python3_LIBRARIES:         ${Python3_LIBRARIES}")
message("Python3_LIBRARY_DIRS:      ${Python3_LIBRARY_DIRS}")
message("Python3_INCLUDE_DIR:       ${Python3_INCLUDE_DIRS}")
message("Python3_EXECUTABLE:        ${Python3_EXECUTABLE}")

link_directories(${Python3_LIBRARY_DIRS})
# ============================================================================
# Scintilla/Lexilla
# ============================================================================
# Базовый путь к precompiled библиотекам
set(PRECOMPILED_BASE "${SCI_DIR}/compile")
# Путь к Scintilla
set(SCINTILLA_DIR "${PRECOMPILED_BASE}/ScintillaEdit/${PLATFORM_DIR}/${CONFIG_DIR}")
# Путь к Lexilla
set(LEXILLA_DIR "${PRECOMPILED_BASE}/lexilla/${PLATFORM_DIR}/${CONFIG_DIR}")

message(STATUS "  SCINTILLA_DIR: ${SCINTILLA_DIR}")
message(STATUS "  LEXILLA_DIR: ${LEXILLA_DIR}")
# ============================================================================
# Project Sources
# ============================================================================
set(TS_FILES qmcr_ru_RU.ts)

set(PROJECT_SOURCES
    scihlp.h
    scihlp.cpp
    mcrwnd.h
    mcrwnd.cpp
    ${TS_FILES}
    forms/dlgfindrepl.h
    forms/dlgfindrepl.cpp
    pyhlp.h
    pyhlp.cpp
    protocolLogWidget.h
    protocolLogWidget.cpp
    qmcr_api.h
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
        add_library(qmcr SHARED ${PROJECT_SOURCES})
    else()
        qt_add_executable(qmcr MANUAL_FINALIZATION ${PROJECT_SOURCES})
    endif()
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(QRASTR_BUILD_WITH_MCR)
        add_library(qmcr SHARED ${PROJECT_SOURCES})
    else()
        add_executable(qmcr ${PROJECT_SOURCES})
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()
# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(qmcr PRIVATE
    ${SCI_DIR}/scintilla/include
    ${SCI_DIR}/scintilla/src
    ${SCI_DIR}/lexilla/include
    ${SCI_DIR}/scintilla/qt/ScintillaEditBase
    ${SCI_DIR}/scintilla/qt/ScintillaEdit
    ${Astra_INCLUDE_DIR}
    ${Python3_INCLUDE_DIRS}
)

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(qmcr PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Python3::Python           # работает и на MSVC начиная с CMake 3.18+
)
# ============================================================================
# Линковка Scintilla/Lexilla
# ============================================================================
if(MSVC)
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit5
        PATHS "${SCINTILLA_DIR}/lib"
        NO_DEFAULT_PATH
    )
    if(NOT SCINTILLA_EDIT_LIB)
        message(FATAL_ERROR "ScintillaEdit5 library not found in ${SCINTILLA_DIR}/lib")
    endif()
    target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})

    # Копируем ScintillaEdit5.dll
    set(_sci_dll "${SCINTILLA_DIR}/lib/ScintillaEdit5.dll")
    if(EXISTS "${_sci_dll}")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_sci_dll}" "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying ScintillaEdit5.dll"
        )
    else()
        message(FATAL_ERROR "ScintillaEdit5.dll not found at ${_sci_dll}")
    endif()

    # Копируем lexilla5.dll (динамическая загрузка через QLibrary)
    set(_lex_dll "${LEXILLA_DIR}/bin/lexilla5.dll")
    if(EXISTS "${_lex_dll}")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_lex_dll}" "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying lexilla5.dll"
        )
    else()
        message(FATAL_ERROR "lexilla5.dll not found at ${_lex_dll}")
    endif()

elseif(UNIX)
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit libScintillaEdit
        PATHS "${SCINTILLA_DIR}/lib"
        NO_DEFAULT_PATH
    )
    if(NOT SCINTILLA_EDIT_LIB)
        message(FATAL_ERROR "ScintillaEdit library not found in ${SCINTILLA_DIR}/lib")
    endif()
    target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})

    # Копируем liblexilla.so (динамическая загрузка через QLibrary)
    set(_lex_so "${LEXILLA_DIR}/lib/liblexilla.so")
    if(EXISTS "${_lex_so}")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_lex_so}" "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying liblexilla.so"
        )
    else()
        message(FATAL_ERROR "liblexilla.so not found at ${_lex_so}")
    endif()

    set_target_properties(qmcr PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_RPATH   "$ORIGIN"
    )
endif()

# ============================================================================
# Target Properties
# ============================================================================
if(NOT QRASTR_BUILD_WITH_MCR)
    if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(_bundle_id MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qmcr)
    endif()
    set_target_properties(qmcr PROPERTIES
        ${_bundle_id}
        MACOSX_BUNDLE_BUNDLE_VERSION       ${PROJECT_VERSION}
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
if(QT_VERSION_MAJOR EQUAL 6 AND NOT QRASTR_BUILD_WITH_MCR)
    qt_finalize_executable(qmcr)
endif()
