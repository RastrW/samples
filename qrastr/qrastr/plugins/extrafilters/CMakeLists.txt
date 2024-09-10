# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )

qt_add_plugin(pnp_extrafilters
    CLASS_NAME ExtraFiltersPlugin
    extrafiltersplugin.cpp extrafiltersplugin.h
)

#set_target_properties(pnp_extrafilters PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/app"
#)

target_include_directories(pnp_extrafilters PRIVATE
    ../../app
)

target_link_libraries(pnp_extrafilters PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

install(TARGETS pnp_extrafilters
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}/plugins"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}/plugins"
)
