# ============================================================================
# Определяем пути к библиотекам Qtitan в зависимости от платформы и компилятора
# ============================================================================
if(NOT IS_DIRECTORY "${QTITAN_ROOT}")
    message(FATAL_ERROR "Qtitan not found at: ${QTITAN_ROOT}")
else()
    message(STATUS "QTITAN_ROOT: ${QTITAN_ROOT}")
endif()

if(WIN32)
    # Windows: Qt6, структура lib/msvc|mingw/Debug|Release
    set(QTITAN_LIB_DIR "${QTITAN_ROOT}/lib/${COMPILER_NAME}/${CONFIG_DIR}")
else()
    # Linux: Qt5, структура lib/
    set(QTITAN_LIB_DIR "${QTITAN_ROOT}/lib")
endif()

if(NOT IS_DIRECTORY ${QTITAN_LIB_DIR})
    message(FATAL_ERROR "Qtitan library directory not found: ${QTITAN_LIB_DIR}")
endif()

message(STATUS "Qtitan configuration:")
message(STATUS "  Library dir: ${QTITAN_LIB_DIR}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Универсальная функция для добавления Qtitan библиотек
# ============================================================================
function(add_qtitan_library LIB_NAME_BASE)
    if(WIN32)
        # ====================================================================
        # Windows: Qt6, MSVC/MinGW
        # ====================================================================
        # Определяем имена для текущей конфигурации
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            # Qtitan 8.2.0 использует формат: QtitanBased2 вместо QtitanBase2d
            set(LIB_NAME "${LIB_NAME_BASE}d")
            # Исправляем позицию 'd' для Qtitan 8.2.0+
            string(REPLACE "2d" "d2" LIB_NAME "${LIB_NAME}")
            string(REPLACE "8d" "d8" LIB_NAME "${LIB_NAME}")
            set(CONFIG_SUBDIR "Debug")
        else()
            set(LIB_NAME "${LIB_NAME_BASE}")
            set(CONFIG_SUBDIR "Release")
        endif()

        if(MSVC)
            set(LIB_FILE "${QTITAN_ROOT}/lib/msvc/${CONFIG_SUBDIR}/${LIB_NAME}.lib")
            set(DLL_FILE "${QTITAN_ROOT}/lib/msvc/${CONFIG_SUBDIR}/${LIB_NAME}.dll")
        elseif(MINGW)
            set(LIB_FILE "${QTITAN_ROOT}/lib/mingw/${CONFIG_SUBDIR}/lib${LIB_NAME}.a")
            set(DLL_FILE "${QTITAN_ROOT}/lib/mingw/${CONFIG_SUBDIR}/${LIB_NAME}.dll")
        endif()

        if(NOT EXISTS ${LIB_FILE})
            message(FATAL_ERROR "Qtitan library not found: ${LIB_FILE}")
        endif()

        add_library(Qtitan::${LIB_NAME_BASE} SHARED IMPORTED)
        set_target_properties(Qtitan::${LIB_NAME_BASE} PROPERTIES
            IMPORTED_IMPLIB "${LIB_FILE}"
            IMPORTED_LOCATION "${DLL_FILE}"
            INTERFACE_INCLUDE_DIRECTORIES "${QTITAN_ROOT}/include"
        )
        message(STATUS "  Added Qtitan::${LIB_NAME_BASE} (${CONFIG_SUBDIR}: ${LIB_NAME})")

    else()
        # ====================================================================
        # Linux: Qt5, GCC
        # ====================================================================
        # Удаляем версию из конца имени библиотеки для поиска
        string(REGEX REPLACE "[0-9]+$" "" LIB_NAME_NO_VERSION "${LIB_NAME_BASE}")

        # Основной файл библиотеки
        set(LIB_FILE "${QTITAN_LIB_DIR}/lib${LIB_NAME_BASE}.so")

        # Если точное имя не найдено, ищем с версией
        if(NOT EXISTS ${LIB_FILE})
            # Ищем файлы вида: libQtitanDataGrid2.so* или libQtitanBase8.so*
            file(GLOB LIB_FILE_VERSIONED "${QTITAN_LIB_DIR}/lib${LIB_NAME_NO_VERSION}.so*")
if(LIB_FILE_VERSIONED)
                # Сортируем и берём самую свежую версию
                list(SORT LIB_FILE_VERSIONED)
                list(REVERSE LIB_FILE_VERSIONED)
                list(GET LIB_FILE_VERSIONED 0 LIB_FILE)
                message(STATUS "  Found versioned library: ${LIB_FILE}")
            else()
                message(FATAL_ERROR "Qtitan library not found: ${QTITAN_LIB_DIR}/lib/${LIB_NAME_NO_VERSION}.so*
  Searched in: ${QTITAN_LIB_DIR}
  Pattern: lib${LIB_NAME_NO_VERSION}.so*")
            endif()
        endif()

        # Проверяем существование файла
        if(NOT EXISTS ${LIB_FILE})
            message(FATAL_ERROR "Qtitan library file does not exist: ${LIB_FILE}")
        endif()

        # Создаём импортируемую библиотеку
        add_library(Qtitan::${LIB_NAME_BASE} SHARED IMPORTED)
        set_target_properties(Qtitan::${LIB_NAME_BASE} PROPERTIES
            IMPORTED_LOCATION "${LIB_FILE}"
            IMPORTED_NO_SONAME FALSE
            INTERFACE_INCLUDE_DIRECTORIES "${QTITAN_ROOT}/include"
        )

        # Для Linux также добавляем runtime путь
        set_target_properties(Qtitan::${LIB_NAME_BASE} PROPERTIES
            INTERFACE_LINK_DIRECTORIES "${QTITAN_LIB_DIR}"
        )

        message(STATUS "  Added Qtitan::${LIB_NAME_BASE}")

    endif()
endfunction()
