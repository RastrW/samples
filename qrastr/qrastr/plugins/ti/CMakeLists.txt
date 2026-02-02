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

target_link_libraries(ti PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(DEFINED COMPILE_OPTIONS)
    target_compile_options(ti PRIVATE ${COMPILE_OPTIONS})
endif()

target_include_directories(ti PRIVATE ${INCLUDES})

# Добавляем compile definitions для header-only библиотек
target_compile_definitions(ti PRIVATE
    SPDLOG_HEADER_ONLY
    FMT_HEADER_ONLY
)
