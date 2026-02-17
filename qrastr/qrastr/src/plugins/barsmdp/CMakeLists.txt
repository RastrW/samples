project(barsmdp)
cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    Core
    Gui
    REQUIRED
)

# ============================================================================
# Output Directories
# ============================================================================
if(MSVC)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
endif()

# ============================================================================
# Получаем THIRDPARTY_DIR из главного проекта
# ============================================================================
get_directory_property(THIRDPARTY_DIR DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITION THIRDPARTY_DIR)

if(NOT THIRDPARTY_DIR)
    message(FATAL_ERROR "THIRDPARTY_DIR not defined in parent project!")
endif()

# ============================================================================
# те же defines что и в главном проекте
# ============================================================================
add_compile_definitions(
    SPDLOG_HEADER_ONLY
    SPDLOG_FMT_EXTERNAL
    FMT_HEADER_ONLY
)

set(INCLUDES
    "${THIRDPARTY_DIR}/spdlog/include/"
    "${THIRDPARTY_DIR}/fmt/include/"
    "${Astra_INCLUDE_DIR}/comck"
)

# ============================================================================
# Используем include_directories для Astra
# ============================================================================
include_directories(${Astra_INCLUDE_DIR})
message(STATUS "Barsmdp include Astra: ${Astra_INCLUDE_DIR}")

set(PROJECT_SOURCES
    plugin_barsmdp.cpp
    plugin_barsmdp.h
    plugin_barsmdp_interfaces.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_plugin(barsmdp
        CLASS_NAME PluginBarsMDP
        ${PROJECT_SOURCES}
    )
else()
    add_library(barsmdp SHARED
        ${PROJECT_SOURCES}
    )
endif()

target_include_directories(barsmdp PRIVATE ${INCLUDES})

# ============================================================================
# Линковка Qt
# ============================================================================
target_link_libraries(barsmdp PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
