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
        #"${ASTRA_LOCATION}/"
        #"${ASTRA_LOCATION}/../../astra/"
    )
endif()

# ============================================================================
# Scintilla/Lexilla
# ============================================================================

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
    set(SCINTILLA_LIB_NAME "ScintillaEdit5")

    find_library(SCINTILLA_EDIT_LIB
        NAMES ${SCINTILLA_LIB_NAME}
        PATHS "${SCINTILLA_DIR}/lib"
        NO_DEFAULT_PATH
    )

    if(SCINTILLA_EDIT_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
        message(STATUS "Linked Scintilla: ${SCINTILLA_EDIT_LIB}")
    else()
        message(FATAL_ERROR "ScintillaEdit library not found")
    endif()

    # Копируем lexillaв output directory для динамической загрузки через QLibrary::resolve()
    if(EXISTS "${LEXILLA_DIR}/bin/lexilla5.dll")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LEXILLA_DIR}/bin/lexilla5.dll"
                "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying lexilla5.dll for dynamic loading"
        )
        message(STATUS "Will copy lexilla5.dll from: ${LEXILLA_DIR}/bin/lexilla5.dll")
    else()
        message(FATAL_ERROR "lexilla5.dll not found at ${LEXILLA_DIR}/bin/")
    endif()

elseif(UNIX)
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit libScintillaEdit
        PATHS "${SCINTILLA_DIR}/lib"
        NO_DEFAULT_PATH
    )

    # Линкуем только Scintilla
    if(SCINTILLA_EDIT_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
        message(STATUS "Linked Scintilla: ${SCINTILLA_EDIT_LIB}")
    else()
        message(FATAL_ERROR "ScintillaEdit library not found!")
    endif()

    # Копируем lexillaв output directory для динамической загрузки через QLibrary::resolve()
    if(EXISTS "${LEXILLA_DIR}/lib/liblexilla.so")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LEXILLA_DIR}/lib/liblexilla.so"
                "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying liblexilla.so for dynamic loading"
        )
        message(STATUS "Will copy liblexilla.so from: ${LEXILLA_DIR}/lib/liblexilla.so")
    else()
        message(FATAL_ERROR "liblexilla.so not found at ${LEXILLA_DIR}/lib/")
    endif()

    # Добавляем RPATH чтобы приложение могло найти библиотеки
    set_target_properties(qmcr PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_RPATH "$ORIGIN"
    )
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

# ============================================================================
# Copy Python DLLs
# ============================================================================
if(WIN32)
    if(Python3_FOUND)
        # Получаем директорию с Python
        get_filename_component(PYTHON_DIR "${Python3_EXECUTABLE}" DIRECTORY)

        message(STATUS "PYTHON_DIR: ${PYTHON_DIR}")

        # Копируем python3.dll и pythonXX.dll
        set(PYTHON_DLLS
            "${PYTHON_DIR}/python3.dll"
            "${PYTHON_DIR}/python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.dll"
        )

        foreach(DLL_PATH ${PYTHON_DLLS})
            if(EXISTS "${DLL_PATH}")
                add_custom_command(TARGET qmcr POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL_PATH}"
                        "$<TARGET_FILE_DIR:qmcr>/"
                    COMMENT "Copying Python DLL: ${DLL_PATH}"
                )
            endif()
        endforeach()
    endif()
    # Копирование ScintillaEdit DLL
    set(SCINTILLA_DLL_NAME "ScintillaEdit5.dll")

    if(EXISTS "${SCINTILLA_DIR}/lib/${SCINTILLA_DLL_NAME}")
        add_custom_command(TARGET qmcr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SCINTILLA_DIR}/lib/${SCINTILLA_DLL_NAME}"
                "$<TARGET_FILE_DIR:qmcr>/"
            COMMENT "Copying ${SCINTILLA_DLL_NAME}"
        )
        message(STATUS "Will copy ${SCINTILLA_DLL_NAME} from: ${SCINTILLA_DIR}/bin/")
    else()
        message(WARNING "${SCINTILLA_DLL_NAME} not found at ${SCINTILLA_DIR}/bin/")
    endif()
endif()
