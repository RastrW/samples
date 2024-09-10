# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(CMAKE_CXX_STANDARD 17) #20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )

if(MSVC)
    set( DEPENDENCIES_LOCATION "c:/projects/compile/" )
    set( ASTRA_LOCATION        "c:/projects/rastr/RastrWin/astra" )
else()
    set( DEPENDENCIES_LOCATION "/home/ustas/projects/compile/" )
    set( ASTRA_LOCATION        "/home/ustas/projects/git_r/rastr/RastrWin/astra/" )
    set( DEPENDENCIES_GIT      "/home/ustas/projects/git_r/" )
endif()
set(INCLUDES
    "${ASTRA_LOCATION}/"
    #"${DEPENDENCIES_LOCATION}/fmt/include"
    "${ASTRA_LOCATION}/fmt/include/"
#    "${DEPENDENCIES_LOCATION}/qicstable/include"
#    "${DEPENDENCIES_LOCATION}/metakit/include" #-win
#    "${DEPENDENCIES_GIT}/metakit/include"      #-lin
#    "${DEPENDENCIES_LOCATION}/eigen-eigen"
#    "${DEPENDENCIES_LOCATION}/boost/boost_1_76_0" # in_nix
#    "${DEPENDENCIES_LOCATION}/boost/boost_1_80_0" # in_win
#    "${DEPENDENCIES_LOCATION}/sci/scintilla/include"
#    "${DEPENDENCIES_LOCATION}/sci/scintilla/src"
#    "${DEPENDENCIES_LOCATION}/sci/scintilla/qt/ScintillaEditBase"
#    "${DEPENDENCIES_LOCATION}/sci/scintilla/qt/ScintillaEdit"
#    "${DEPENDENCIES_LOCATION}/Qt-Advanced-Docking-System/src"
    "${DEPENDENCIES_LOCATION}/spdlog/include/"
)

set(PROJECT_SOURCES
 #   ${ASTRA_LOCATION}/ResourceManager.cpp
)

qt_add_plugin(rastr
    CLASS_NAME PluginRastr
    pluginrastr.h
    pluginrastr.cpp
     ${PROJECT_SOURCES}
)

#set_target_properties(pnp_extrafilters PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/app"
#)

include_directories(
    "C:/projects/rastr/flytfs/IPlainRastrTest/"
    "C:/projects/rastr/RastrWin/astra/"
    "../../"
)

target_include_directories( rastr PRIVATE
    ../../app
)

target_link_libraries( rastr PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

#${PROJECT_NAME}
target_include_directories( rastr PRIVATE
  ${INCLUDES}
)

install(TARGETS rastr
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}/plugins"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}/plugins"
)
