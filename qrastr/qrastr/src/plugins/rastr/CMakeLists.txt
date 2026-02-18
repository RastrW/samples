project(rastr)
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
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//$<CONFIG>//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//$<CONFIG>//plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//$<CONFIG>//plugins")
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//$<CONFIG>//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//$<CONFIG>/plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//$<CONFIG>//plugins")
endif()

# ============================================================================
# Получаем THIRDPARTY_DIR из главного проекта
# ============================================================================
get_directory_property(THIRDPARTY_DIR DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITION THIRDPARTY_DIR)

if(NOT THIRDPARTY_DIR)
    message(FATAL_ERROR "THIRDPARTY_DIR not defined in parent project!")
endif()

set(INCLUDES
    "${THIRDPARTY_DIR}/spdlog/include/"
    "${THIRDPARTY_DIR}/fmt/include/"
)
# ============================================================================
# Используем include_directories для Astra
# ============================================================================
include_directories(${Astra_INCLUDE_DIR})
message(STATUS "Rastr include Astra: ${Astra_INCLUDE_DIR}")

set(PROJECT_SOURCES
    plugin_interfaces.h
    pluginrastr.h
    pluginrastr.cpp
)

add_library(rastr MODULE ${PROJECT_SOURCES})
target_include_directories(rastr PRIVATE ${INCLUDES})

# ============================================================================
# Линковка ТОЛЬКО Qt
# ============================================================================
target_link_libraries(rastr PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
