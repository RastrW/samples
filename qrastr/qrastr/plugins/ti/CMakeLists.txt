project(ti)
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
# Получаем THIRDPARTY_DIR из главного проекта
# ============================================================================
get_directory_property(THIRDPARTY_DIR DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITION THIRDPARTY_DIR)

if(NOT THIRDPARTY_DIR)
    message(FATAL_ERROR "THIRDPARTY_DIR not defined in parent project!")
endif()

set(INCLUDES
    "${THIRDPARTY_DIR}/spdlog/include/"
    "../../"
)

# ============================================================================
# Используем include_directories для COMCK, как в оригинале!
# ============================================================================
include_directories(${COMCK_INCLUDE_DIR})

set(PROJECT_SOURCES
    plugin_ti_interfaces.h
    pluginti.h
    pluginti.cpp
)

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

target_include_directories(ti PRIVATE ${INCLUDES})
# ============================================================================
# Линковка библиотек
# ============================================================================
target_link_libraries(ti PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

message(STATUS "IMPORTED_LOCATION FMT = ${THIRDPARTY_DIR}/compile/fmt/${PLATFORM_DIR}/${CONFIG_DIR}/bin/fmtd.dll")
message(STATUS "IMPORTED_LOCATION SPDLOG = ${THIRDPARTY_DIR}/compile/spdlog/${PLATFORM_DIR}/${CONFIG_DIR}/bin/spdlogd.dll")

# Создаем imported targets для fmt и spdlog (такой же код)
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

target_link_libraries(ti PRIVATE fmt::fmt spdlog::spdlog)
