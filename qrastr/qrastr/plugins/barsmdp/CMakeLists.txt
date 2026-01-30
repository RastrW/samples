project(barsmdp)
cmake_minimum_required(VERSION 3.23) # 3.20 min version for "astra" build
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package( QT NAMES Qt6 Qt5 COMPONENTS REQUIRED )
find_package( Qt${QT_VERSION_MAJOR} COMPONENTS
             Widgets
             Core
             Gui
             REQUIRED
)

if(MSVC)
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins" )
else()
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//${CMAKE_BUILD_TYPE}//plugins" )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//${CMAKE_BUILD_TYPE}//plugins" )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//${CMAKE_BUILD_TYPE}//plugins" )
endif ()

#set( INCLUDES
#    "${DEPENDENCIES_LOCATION}/spdlog/include/"
    #"${ASTRA_LOCATION}/"
    #"${ASTRA_LOCATION}/../comck/"
#    "../../"
#)
include_directories ( ${Astra_INCLUDE_DIR} )

set(PROJECT_SOURCES
  plugin_barsmdp.cpp
  plugin_barsmdp.h
  plugin_barsmdp_interfaces.h
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_plugin( barsmdp
        CLASS_NAME PluginBarsMDP
        ${PROJECT_SOURCES}
    )
else()
#    add_executable( rastr-exexc
#        ${PROJECT_SOURCES}
#    )
    add_library( barsmdp SHARED
        ${PROJECT_SOURCES}
    )
endif()


target_link_libraries( barsmdp PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    spdlog::spdlog
    fmt::fmt
)

target_compile_options(barsmdp PRIVATE ${COMPILE_OPTIONS})
target_include_directories( barsmdp PRIVATE
  "../../"
)
