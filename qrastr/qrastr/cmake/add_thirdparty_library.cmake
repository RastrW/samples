# ============================================================================
# Helper Function for Third-Party Libraries
# ============================================================================
function(add_thirdparty_library LIB_NAME LIB_PATH)
    set(options USE_DEBUG_SUFFIX)
    set(oneValueArgs LIB_REAL_NAME)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_LIB_REAL_NAME)
        set(ARG_LIB_REAL_NAME "${LIB_NAME}")
    endif()

    # Добавляем debug суффикс если требуется
    if(ARG_USE_DEBUG_SUFFIX AND CMAKE_BUILD_TYPE MATCHES Debug)
        set(ARG_LIB_REAL_NAME "${ARG_LIB_REAL_NAME}d")
    endif()

    set(PRECOMPILED_DIR "${THIRDPARTY_DIR}/compile/${LIB_NAME}/${PLATFORM_DIR}/${CONFIG_DIR}")

    find_library(${LIB_NAME}_LIBRARY
        NAMES ${ARG_LIB_REAL_NAME}
        PATHS ${PRECOMPILED_DIR}/lib
        NO_DEFAULT_PATH
    )

    set(${LIB_NAME}_INCLUDE_DIR "${PRECOMPILED_DIR}/include")
    if(NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
        set(${LIB_NAME}_INCLUDE_DIR "${LIB_NAME}_INCLUDE_DIR-NOTFOUND")
    endif()

    if(NOT ${LIB_NAME}_LIBRARY OR NOT EXISTS "${${LIB_NAME}_INCLUDE_DIR}")
        message(FATAL_ERROR
            "Precompiled ${LIB_NAME} not found in ${PRECOMPILED_DIR}\n"
            "  Library: ${${LIB_NAME}_LIBRARY}\n"
            "  Include: ${${LIB_NAME}_INCLUDE_DIR}\n"
            "  Looking for: ${ARG_LIB_REAL_NAME}")
    endif()

    add_library(${LIB_NAME} UNKNOWN IMPORTED)
    set_target_properties(${LIB_NAME} PROPERTIES
        IMPORTED_LOCATION "${${LIB_NAME}_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${${LIB_NAME}_INCLUDE_DIR}"
    )

    message(STATUS "Using precompiled ${LIB_NAME}: ${${LIB_NAME}_LIBRARY}")
endfunction()
