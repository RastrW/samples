# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )

set( INCLUDES
    "${DEPENDENCIES_LOCATION}/spdlog/include/"
#    "${ASTRA_LOCATION}/../../flytfs/IPlainRastrTest/"
    "${ASTRA_LOCATION}/"
    "../../"
)
set(PROJECT_SOURCES
    pluginrastr.h
    pluginrastr.cpp
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_plugin( rastr
        CLASS_NAME PluginRastr
        ${PROJECT_SOURCES}
    )
else()
    add_executable( rastr
        ${PROJECT_SOURCES}
    )
endif()
target_link_libraries( rastr PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
#${PROJECT_NAME} == qrastr
target_include_directories( rastr PRIVATE
  ${INCLUDES}
)
install(TARGETS rastr
    BUNDLE DESTINATION  "${INSTALL_EXAMPLEDIR}/plugins"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}/plugins"
)
