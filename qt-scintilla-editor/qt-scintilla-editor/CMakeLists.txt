cmake_minimum_required(VERSION 3.9)
#https://github.com/mavroprovato/qt-scintilla-editor
project(qt-scintilla-editor LANGUAGES CXX)

#set(SCINTILLA_CODE ../scintilla-code)
set(SCINTILLA_CODE C:/Qt/Examples/qt_6/scintilla550/scintilla)

#set(LEXILLA_CODE C:/Qt/Examples/qt_6/lexilla532/lexilla)
set(LEXILLA_CODE C:/Qt/Examples/qt_6/lexilla532/lexilla/)

option( BUILD_WITH_EXP_PY    "Build with Python"    ON  )

if(MSVC)
else()
    add_compile_options( -fpermissive )
endif()

if(BUILD_WITH_EXP_PY)
    message( TRACE "BUILD_WITH_EXP_PY= ${BUILD_WITH_EXP_PY}"  )
    add_compile_definitions( BUILD_WITH_EXP_PY )
    #set (PYTHON_EXECUTABLE "C:/Python311_x64/")
    set(PYBIND11_PYTHON_VERSION 3.11 CACHE STRING "")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

include_directories(
        ${SCINTILLA_CODE}/include
        ${SCINTILLA_CODE}/src #ustas
        ${LEXILLA_CODE}/include
        ${SCINTILLA_CODE}/qt/ScintillaEditBase
        ${SCINTILLA_CODE}/qt/ScintillaEdit
        include
)

link_directories(${SCINTILLA_CODE}/bin)
#link_directories(${LEXILLA_CODE}/bin) //ustas moved to ${SCINTILLA_CODE}

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

#find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

add_executable( qt-scintilla-editor
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

if(BUILD_WITH_EXP_PY)
  include(cmake/pybind11.cmake)
  include_directories(${pybind11_INCLUDE_DIRS})
  message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
  message(STATUS " pybind11_INCLUDE_DIRS = ${pybind11_INCLUDE_DIRS}")
  message(STATUS " pybind11_LIBRARIES    = ${pybind11_LIBRARIES}")
endif(BUILD_WITH_EXP_PY)

#target_link_libraries(qt-scintilla-editor PRIVATE Qt5::Widgets ScintillaEdit)
target_link_libraries(qt-scintilla-editor PRIVATE Qt6::Widgets ScintillaEdit5 Qt6::Core5Compat lexilla5)

if(BUILD_WITH_EXP_PY)
  add_compile_definitions(PY_ASTRA)
   #   list(APPEND SOURCES astra_exp_py.cpp)
     # pybind11_add_module(qt-scintilla-editor SHARED ${SOURCES})
      target_link_libraries(qt-scintilla-editor PRIVATE pybind11::embed)
endif()
