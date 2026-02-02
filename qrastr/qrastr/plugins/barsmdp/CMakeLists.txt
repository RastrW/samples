project(barsmdp)
cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Build Configuration
# ============================================================================
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# ============================================================================
# Package Dependencies
# ============================================================================
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
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/plugins")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/plugins")
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(${Astra_INCLUDE_DIR})

# ============================================================================
# Project Sources
# ============================================================================
set(PROJECT_SOURCES
    plugin_barsmdp.cpp
    plugin_barsmdp.h
    plugin_barsmdp_interfaces.h
)

# ============================================================================
# Target Creation
# ============================================================================
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

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(barsmdp PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

# ============================================================================
# Compiler Options
# ============================================================================
if(DEFINED COMPILE_OPTIONS)
    target_compile_options(barsmdp PRIVATE ${COMPILE_OPTIONS})
endif()

# ============================================================================
# Получаем THIRDPARTY_DIR из родительского проекта
# ============================================================================
get_directory_property(THIRDPARTY_DIR DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITION THIRDPARTY_DIR)

# Добавляем include директории
target_include_directories(barsmdp PRIVATE
    "../../"
    "${THIRDPARTY_DIR}/spdlog/include"
    "${THIRDPARTY_DIR}/fmt/include"
)

target_compile_definitions(barsmdp PRIVATE
    SPDLOG_HEADER_ONLY
    FMT_HEADER_ONLY
)
