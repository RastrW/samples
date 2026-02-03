# Определяем OS
if(WIN32)
    set(OS_NAME "Windows")
elseif(APPLE)
    set(OS_NAME "macOS")
elseif(UNIX)
    set(OS_NAME "Linux")

    # Определяем дистрибутив Linux
    if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" OS_RELEASE)
        if(OS_RELEASE MATCHES "ID=astra")
            set(LINUX_DISTRO "AstraLinux")
            set(IS_ASTRA_LINUX TRUE)

            # Устанавливаем компилятор для Astra
            if(NOT CMAKE_C_COMPILER)
                set(CMAKE_C_COMPILER "/usr/bin/clang-13" CACHE FILEPATH "C compiler" FORCE)
            endif()
            if(NOT CMAKE_CXX_COMPILER)
                set(CMAKE_CXX_COMPILER "/usr/bin/clang++-13" CACHE FILEPATH "C++ compiler" FORCE)
            endif()
        elseif(OS_RELEASE MATCHES "ID=ubuntu")
            set(LINUX_DISTRO "Ubuntu")
            set(IS_ASTRA_LINUX FALSE)
        endif()
    endif()
endif()
