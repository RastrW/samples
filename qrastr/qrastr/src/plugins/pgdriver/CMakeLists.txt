cmake_minimum_required(VERSION 3.16)

project(pgdriver LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

# ============================================================================
# Output Directories
# ============================================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}")

message("PLUGIN_OUTPUT_DIR barsmdp:        ${PLUGIN_OUTPUT_DIR}")

set(INCLUDES
    "${THIRDPARTY_DIR}/spdlog/include/"
    "${THIRDPARTY_DIR}/fmt/include/"
    "${Astra_INCLUDE_DIR}/comck"
)

add_library(pgdriver SHARED
  plugin_pgdriver.cpp
  plugin_pgdriver.h
  #pgdriver.json
  plugin_pgdriver_interfaces.h
)
target_include_directories(pgdriver PRIVATE ${INCLUDES})

target_link_libraries(pgdriver PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

target_compile_definitions(pgdriver PRIVATE PGDRIVER_LIBRARY)
