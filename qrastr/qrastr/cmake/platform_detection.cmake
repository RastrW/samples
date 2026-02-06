# Определяем OS
if(WIN32)
    set(OS_NAME "Windows")
elseif(APPLE)
    set(OS_NAME "macOS")
elseif(UNIX)
    set(OS_NAME "Linux")
endif()
