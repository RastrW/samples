# ============================================================================
# Определение компилятора
# ============================================================================
if(MSVC)
    set(COMPILER_NAME "msvc")
elseif(MINGW)
    set(COMPILER_NAME "mingw")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(COMPILER_NAME "gcc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER_NAME "clang")
else()
    set(COMPILER_NAME "unknown")
endif()

# ============================================================================
# Platform Detection for Libraries
# ============================================================================
if(WIN32)
    set(OS_NAME "Windows")
    set(SHLIB_SUFFIX ".dll")
    if(MSVC)
        set(COMPILER "msvc")
        set(LIB_PREFIX "")
        set(LIB_SUFFIX ".lib")
    elseif(MINGW)
        set(COMPILER "mingw")
        set(LIB_PREFIX "lib")
        set(LIB_SUFFIX ".a")
    else()
        message(WARNING "Unknown Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
        set(COMPILER "unknown")
    endif()
    set(PLATFORM_DIR "win/${COMPILER}")

elseif(UNIX AND NOT APPLE)
    set(OS_NAME "Linux")

    set(SHLIB_SUFFIX ".so")

    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".so")
    # Определяем компилятор для выбора правильных precompiled библиотек
    # Библиотеки скомпилированные разными компиляторами могут быть несовместимы
    set(COMPILER "gcc")
    set(PLATFORM_DIR "linux/${COMPILER}")
else()
    set(OS_NAME "macOS")
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Определяем CONFIG_DIR для путей
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CONFIG_DIR "Debug")
else()
    set(CONFIG_DIR "Release")
endif()

message(STATUS "Platform configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${COMPILER}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform directory: ${PLATFORM_DIR}")
