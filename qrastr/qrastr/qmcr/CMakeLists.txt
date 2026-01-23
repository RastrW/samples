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

#if(MSVC)
#    set(SCI_DIR C:/projects/compile/sci/)
#    #set (SCI_DIR C:/Projects/compile/sci/)
#else()
#    set(SCI_DIR "${DEPENDENCIES_LOCATION}/sci")
#endif()

set(SCI_DIR "${DEPENDENCIES_LOCATION}/sci")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)#visual studio not generate .lib without exported from .dll!

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package( Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickWidgets)
find_package( Astra   REQUIRED)         # need install Astra: ex. cmake --install build --config Debug from C:\Projects\tfs\rastr\RastrWin

set(PYTHON_MIN 3.10)
#old CMake if(CMAKE_VERSION VERSION_LESS "3.12.0")
#find_package(PythonLibs ${PYTHON_MIN} REQUIRED)
#message("PYTHON_INCLUDE_DIRS:${PYTHON_INCLUDE_DIRS}")
#message("PYTHON_LIBRARIES:${PYTHON_LIBRARIES}")
#include_directories(${PYTHON_INCLUDE_DIRS})

#new CMake
find_package (Python3 ${PYTHON_MIN} REQUIRED COMPONENTS Interpreter Development)
message( "Python3_FOUND:             ${Python3_FOUND}")
message( "Python3_VERSION:           ${Python3_VERSION}")
message( "Python3_Development_FOUND: ${Python3_Development_FOUND}")
message( "Python3_LIBRARIES:         ${Python3_LIBRARIES}")
message( "Python3_LIBRARY_DIRS:      ${Python3_LIBRARY_DIRS}")
message( "Python3_INCLUDE_DIR:       ${Python3_INCLUDE_DIRS}")
message( "Python3_EXECUTABLE:        ${Python3_EXECUTABLE}")
link_directories    ( ${Python3_LIBRARY_DIRS} )
include_directories ( ${Python3_INCLUDE_DIRS} )
include_directories ( ${Astra_INCLUDE_DIR} )

include_directories("${ASTRA_LOCATION}/fmt/include/")

include_directories(
        ${SCI_DIR}/scintilla/include
        ${SCI_DIR}/scintilla/src #ustas
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

link_directories(${SCI_DIR}/bin)

set(TS_FILES qmcr_ru_RU.ts)
set(PROJECT_SOURCES)
list (APPEND PROJECT_SOURCES
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
    )
else()
    list(APPEND PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
    )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(QRASTR_BUILD_WITH_MCR)
        add_library( qmcr SHARED
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
        add_library( qmcr SHARED
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

target_link_libraries( qmcr PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries( qmcr PRIVATE Qt${QT_VERSION_MAJOR}::QuickWidgets)
#target_link_libraries( qmcr PRIVATE ${PYTHON_LIBRARIES})
##target_link_libraries( qmcr PRIVATE Python3::Python)
#target_link_libraries( qmcr PRIVATE python3)
#target_compile_definitions( qmcr PRIVATE Py_LIMITED_API=0x030A0000 ) #minimal version Python 3.10
if(NOT MSVC)
  target_link_libraries(qmcr PRIVATE Python3::Python) # lin
endif()

if(MSVC)
    #set_target_properties(Python3::Python PROPERTIES MAP_IMPORTED_CONFIG_DEBUG ";RELEASE")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries( qmcr PRIVATE ScintillaEdit_d5 Lexilla_d5 )
    else()
        target_link_libraries( qmcr PRIVATE ScintillaEdit5   Lexilla5   )
    endif()
else()
    target_link_libraries( qmcr PRIVATE libScintillaEdit.so libLexilla.so )
endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qmcr)
endif()

if(QRASTR_BUILD_WITH_MCR)
else()
    set_target_properties(qmcr PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
endif()

include(GNUInstallDirs)
install(TARGETS qmcr
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    if(QRASTR_BUILD_WITH_MCR)
    else()
        qt_finalize_executable(qmcr)
    endif()
endif()
