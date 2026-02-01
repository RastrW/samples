# ============================================================================
# Plugin: ti (Temperature Index)
# ============================================================================
project(ti)
cmake_minimum_required(VERSION 3.23) # 3.20 min version for "astra" build
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
include_directories(${COMCK_INCLUDE_DIR})

# ============================================================================
# Project Sources
# ============================================================================
set(PROJECT_SOURCES
    plugin_ti_interfaces.h
    pluginti.h
    pluginti.cpp
)

# ============================================================================
# Target Creation
# ============================================================================
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_plugin(ti
        CLASS_NAME PluginTI
        ${PROJECT_SOURCES}
    )
else()
    add_library(ti SHARED
        ${PROJECT_SOURCES}
    )
endif()

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(ti PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    spdlog::spdlog
    fmt::fmt
)

# ============================================================================
# Compiler Options
# ============================================================================
if(DEFINED COMPILE_OPTIONS)
    target_compile_options(ti PRIVATE ${COMPILE_OPTIONS})
endif()

# Добавляем include директорию головного проекта для доступа к общим заголовкам
target_include_directories(ti PRIVATE
    "../../"
)
