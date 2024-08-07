cmake_minimum_required(VERSION 3.16)
#https://github.com/mavroprovato/qt-scintilla-editor
project(qt-scintilla-editor LANGUAGES C CXX)


if(MSVC)
    #set(SCINTILLA_CODE C:/Qt/Examples/qt_6/scintilla550/scintilla)
    #set(SCINTILLA_CODE C:/projects/compile/sci/bin)
    set(SCI_DIR C:/projects/compile/sci/)
else()
    #set(SCINTILLA_CODE /home/ustas/projects/compile/scintilla/scintilla551/)
    set(SCI_DIR C:/projects/compile/sci/)
endif()

#it not use CMake include(cmake/scintilla.cmake)

#if(MSVC)
#    set(LEXILLA_CODE C:/Qt/Examples/qt_6/lexilla532/lexilla/)
#else()
#    set(LEXILLA_CODE /home/ustas/projects/compile/scintilla/lexilla533)
#endif()

option( BUILD_WITH_EXP_PY    "Build with Python"    ON )

find_package(Python COMPONENTS Interpreter Development)

message("Python_FOUND:${Python_FOUND}")
message("Python_VERSION:${Python_VERSION}")
message("Python_Development_FOUND:${Python_Development_FOUND}")
message("Python_LIBRARIES:${Python_LIBRARIES}")

if(MSVC)
else()
    add_compile_options( -fpermissive )
endif()

if(BUILD_WITH_EXP_PY)
    message( TRACE "BUILD_WITH_EXP_PY= ${BUILD_WITH_EXP_PY}"  )
    add_compile_definitions( BUILD_WITH_EXP_PY )
    #set (PYTHON_EXECUTABLE "C:/Python311_x64/")
    #set(PYBIND11_PYTHON_VERSION 3.11 CACHE STRING "")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

include_directories(
        ${SCI_DIR}/scintilla/include
        ${SCI_DIR}/scintilla/src #ustas
        ${SCI_DIR}/scintilla/qt/ScintillaEditBase
        ${SCI_DIR}/scintilla/qt/ScintillaEdit
        ${SCI_DIR}/lexilla/include
        include
)

link_directories(${SCI_DIR}/bin)
#link_directories(${LEXILLA_CODE}/bin) //ustas moved to ${SCINTILLA_CODE}
#link_directories(${LEXILLA_CODE}/bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

#find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(QT NAMES Qt5 Qt6 COMPONENTS Widgets REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat)    
endif()

#find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(QT_USE_QTGUI TRUE)

if (NOT TARGET Qt5::moc)
    add_executable(Qt5::moc IMPORTED)

    set(imported_location "${_qt5Core_install_prefix}/bin/moc")
    #_qt5_Core_check_file_exists(${imported_location})

    set_target_properties(Qt5::moc PROPERTIES
        IMPORTED_LOCATION ${imported_location}
    )
    # For CMake automoc feature
    get_target_property(QT_MOC_EXECUTABLE Qt5::moc LOCATION)
endif()

set(SOURCES
    src/aboutdialog.cpp
    src/qscintillaeditor.cpp
    src/buffer.cpp
    src/colorscheme.cpp
    src/configuration.cpp
    src/encoding.cpp
    src/encodingdialog.cpp
    src/findreplacedialog.cpp
    src/icondb.cpp
    src/language.cpp
    src/languagedialog.cpp
    src/main.cpp
    src/qscintillaeditor.cpp
    src/styleinfo.cpp
    src/util.cpp
    include/aboutdialog.h
    include/buffer.h
    include/colorscheme.h
    include/configuration.h
    include/encoding.h
    include/encodingdialog.h
    include/findreplacedialog.h
    include/icondb.h
    include/language.h
    include/languagedialog.h
    include/qscintillaeditor.h
    include/styleinfo.h
    include/util.h
    include/version.h
    forms/aboutdialog.ui
    forms/encodingdialog.ui
    forms/findreplacedialog.ui
    forms/languagedialog.ui
    forms/qscintillaeditor.ui
    resources/qtscitntillaeditor.qrc
    include/py_fun.h
    src/py_fun.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
else()
    #list (APPEND SOURCES resources/qtscintillaeditor.qrc)
    qt5_add_resources ( SOURCES resources/qtscitntillaeditor.qrc )
endif()

add_executable( qt-scintilla-editor
    ${SOURCES}
)

if(BUILD_WITH_EXP_PY)
  include(cmake/pybind11.cmake)
  include_directories(${pybind11_INCLUDE_DIRS})
  message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
  message(STATUS " pybind11_INCLUDE_DIRS = ${pybind11_INCLUDE_DIRS}")
  message(STATUS " pybind11_LIBRARIES    = ${pybind11_LIBRARIES}")
endif(BUILD_WITH_EXP_PY)

target_link_libraries(qt-scintilla-editor PRIVATE Qt::Widgets )
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(qt-scintilla-editor PRIVATE  Qt::Core5Compat )
endif()

# from https://github.com/Vatsinator/vatsinator-legacy/blob/develop/CMakeLists.txt
# set configuration types (msvc/xcode)
if (CMAKE_CONFIGURATION_TYPES)
    set (CMAKE_CONFIGURATION_TYPES Debug Release)
    set (CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we actually need" FORCE
)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (CMAKE_BUILD_TYPE Debug)
else ()
    set (CMAKE_BUILD_TYPE Release)
endif ()

if(MSVC)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(qt-scintilla-editor PRIVATE ScintillaEdit_d5 Lexilla_d5 )
    else()
        target_link_libraries(qt-scintilla-editor PRIVATE ScintillaEdit5   Lexilla5   )
    endif()
else()
    target_link_libraries(qt-scintilla-editor libScintillaEdit.so libLexilla.so)
endif()

if(BUILD_WITH_EXP_PY)
  add_compile_definitions(PY_ASTRA)
   #   list(APPEND SOURCES astra_exp_py.cpp)
     # pybind11_add_module(qt-scintilla-editor SHARED ${SOURCES})
      target_link_libraries(qt-scintilla-editor PRIVATE pybind11::embed)
endif()
