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
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//..//..//${CMAKE_BUILD_TYPE}//plugins")
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//${CMAKE_BUILD_TYPE}//plugins")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//${CMAKE_BUILD_TYPE}//plugins")
endif()

# ============================================================================
# Получаем переменные из главного проекта
# ============================================================================
if(NOT THIRDPARTY_DIR)
    message(FATAL_ERROR "THIRDPARTY_DIR not defined in parent project!")
endif()

if(NOT PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR not defined in parent project!")
endif()

if(NOT CONFIG_DIR)
    message(FATAL_ERROR "CONFIG_DIR not defined in parent project!")
endif()

set(INCLUDES
    "${THIRDPARTY_DIR}/spdlog/include/"
    "${THIRDPARTY_DIR}/fmt/include/"
    "../../"
)

# ============================================================================
# Используем include_directories для Astra
# ============================================================================
include_directories(${Astra_INCLUDE_DIR})

set(PROJECT_SOURCES
    plugin_interfaces.h
    pluginrastr.h
    pluginrastr.cpp
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_plugin(rastr
        CLASS_NAME PluginRastr
        ${PROJECT_SOURCES}
    )
else()
    add_library(rastr SHARED
        ${PROJECT_SOURCES}
    )
endif()

target_include_directories(rastr PRIVATE ${INCLUDES})
# ============================================================================
# Линковка библиотек
# ============================================================================
target_link_libraries(rastr PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

message(STATUS "IMPORTED_LOCATION FMT = ${THIRDPARTY_DIR}/compile/fmt/${PLATFORM_DIR}/${CONFIG_DIR}/bin/fmtd.dll")
message(STATUS "IMPORTED_LOCATION SPDLOG = ${THIRDPARTY_DIR}/compile/spdlog/${PLATFORM_DIR}/${CONFIG_DIR}/bin/spdlogd.dll")

# Создаем imported targets для fmt и spdlog
if(NOT TARGET fmt::fmt)
    message(STATUS "Creating imported target fmt::fmt")
    add_library(fmt::fmt SHARED IMPORTED)
    set_target_properties(fmt::fmt PROPERTIES
        IMPORTED_LOCATION "${THIRDPARTY_DIR}/compile/fmt/${PLATFORM_DIR}/${CONFIG_DIR}/bin/fmtd.dll"
        IMPORTED_IMPLIB "${THIRDPARTY_DIR}/compile/fmt/${PLATFORM_DIR}/${CONFIG_DIR}/lib/fmtd.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/compile/fmt/${PLATFORM_DIR}/${CONFIG_DIR}/include"
    )
else()
    message(STATUS "Target fmt::fmt already exists")
endif()

if(NOT TARGET spdlog::spdlog)
    message(STATUS "Creating imported target spdlog::spdlog")
    add_library(spdlog::spdlog SHARED IMPORTED)
    set_target_properties(spdlog::spdlog PROPERTIES
        IMPORTED_LOCATION "${THIRDPARTY_DIR}/compile/spdlog/${PLATFORM_DIR}/${CONFIG_DIR}/bin/spdlogd.dll"
        IMPORTED_IMPLIB "${THIRDPARTY_DIR}/compile/spdlog/${PLATFORM_DIR}/${CONFIG_DIR}/lib/spdlogd.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/compile/spdlog/${PLATFORM_DIR}/${CONFIG_DIR}/include"
    )
else()
    message(STATUS "Target spdlog::spdlog already exists")
endif()

target_link_libraries(rastr PRIVATE fmt::fmt spdlog::spdlog)
