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
# Получаем PLUGIN_OUTPUT_DIR из родительского проекта, если уже определена
if(NOT PLUGIN_OUTPUT_DIR)
    set(PLUGIN_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/plugins")
endif()

# Нормализуем путь (убираем /../)
cmake_path(NORMAL_PATH PLUGIN_OUTPUT_DIR)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")

message(STATUS "Plugin ${PROJECT_NAME} output dir: ${PLUGIN_OUTPUT_DIR}")
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

add_library(rastr SHARED ${PROJECT_SOURCES})
target_include_directories(rastr PRIVATE ${INCLUDES})

# ============================================================================
# Линковка ТОЛЬКО Qt
# ============================================================================
target_link_libraries(rastr PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
