cmake_minimum_required(VERSION 3.16)
project(qmcr VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# from https://github.com/Vatsinator/vatsinator-legacy/blob/develop/CMakeLists.txt
# set configuration types (msvc/xcode)
if (CMAKE_CONFIGURATION_TYPES)
    set (CMAKE_CONFIGURATION_TYPES Debug Release)
    set (CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we actually need" FORCE
    )
endif()

set(SCI_DIR "${DEPENDENCIES_LOCATION}")

# For Mingw:
if (WIN32 AND NOT MSVC)
    set(Python3_EXECUTABLE "C:/msys64/mingw64/bin/python3.exe" CACHE FILEPATH "" FORCE)
    set(Python3_ROOT_DIR   "C:/msys64/mingw64"                 CACHE PATH "" FORCE)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # visual studio not generate .lib without exported from .dll!

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
# Линковка Scintilla/Lexilla для разных платформ
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
    set(PLATFORM_DIR "linux")
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

# Добавляем старый путь для обратной совместимости
link_directories(${SCI_DIR}/bin)

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

# Python
if(NOT MSVC)
    target_link_libraries(qmcr PRIVATE Python3::Python)
endif()

# ============================================================================
# Линковка Scintilla/Lexilla
# ============================================================================
if(MSVC)
    # MSVC: использует суффиксы _d5 для Debug, 5 для Release
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(qmcr PRIVATE ScintillaEdit_d5 lexilla)
    else()
        target_link_libraries(qmcr PRIVATE ScintillaEdit5 lexilla)
    endif()

elseif(MINGW)
    # MinGW: использует .a статические библиотеки или .dll.a импорт-библиотеки
    # Пытаемся найти библиотеки в порядке приоритета

    # ScintillaEdit
    find_library(SCINTILLA_EDIT_LIB
        NAMES ScintillaEdit5 libScintillaEdit5
        PATHS "${SCINTILLA_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    # ScintillaEditBase (запасной вариант)
    find_library(SCINTILLA_EDIT_BASE_LIB
        NAMES ScintillaEditBase5 libScintillaEditBase5
        PATHS "${SCINTILLA_BASE_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    # Lexilla
    find_library(LEXILLA_LIB
        NAMES lexilla Lexilla liblexilla
        PATHS "${LEXILLA_DIR}/lib" "${SCI_DIR}/bin"
        NO_DEFAULT_PATH
    )

    # Выводим информацию о найденных библиотеках
    message(STATUS "Scintilla libraries for MinGW:")
    message(STATUS "  ScintillaEdit: ${SCINTILLA_EDIT_LIB}")
    message(STATUS "  ScintillaEditBase: ${SCINTILLA_EDIT_BASE_LIB}")
    message(STATUS "  Lexilla: ${LEXILLA_LIB}")

    # Линкуем то, что нашли
    if(SCINTILLA_EDIT_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_LIB})
    elseif(SCINTILLA_EDIT_BASE_LIB)
        target_link_libraries(qmcr PRIVATE ${SCINTILLA_EDIT_BASE_LIB})
        message(WARNING "ScintillaEdit not found, using ScintillaEditBase")
    else()
        message(FATAL_ERROR "Neither ScintillaEdit nor ScintillaEditBase found!")
    endif()

    if(LEXILLA_LIB)
        target_link_libraries(qmcr PRIVATE ${LEXILLA_LIB})
    else()
        message(FATAL_ERROR "Lexilla library not found!")
    endif()

elseif(UNIX)
    # Linux: использует .so shared libraries
    target_link_libraries(qmcr PRIVATE ScintillaEdit Lexilla)

else()
    message(FATAL_ERROR "Unsupported platform for Scintilla/Lexilla linking")
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

if(QT_VERSION_MAJOR EQUAL 6)
    if(NOT QRASTR_BUILD_WITH_MCR)
        qt_finalize_executable(qmcr)
    endif()
endif()
