
# Определяем пути к библиотекам Qtitan в зависимости от платформы и компилятора
if(NOT IS_DIRECTORY "${QTITAN_ROOT}")
    message(FATAL_ERROR "Qtitan not found at: ${QTITAN_ROOT}")
else()
    message(STATUS "QTITAN_ROOT: ${QTITAN_ROOT}")
endif()

if(WIN32)
     set(QTITAN_LIB_DIR "${QTITAN_ROOT}/lib/${COMPILER_NAME}/${CONFIG_DIR}")
else()
    # Linux - оставляем без изменений для совместимости с Qtitan
    set(QTITAN_LIB_DIR "${QTITAN_ROOT}/bin")
endif()

if(NOT IS_DIRECTORY ${QTITAN_LIB_DIR})
    message(FATAL_ERROR "Qtitan library directory not found: ${QTITAN_LIB_DIR}")
endif()
message(STATUS "Qtitan configuration:")
message(STATUS "  Library dir: ${QTITAN_LIB_DIR}")
# ============================================================================
# Универсальная функция для добавления Qtitan библиотек
# ============================================================================
function(add_qtitan_library LIB_NAME_BASE)
    if(WIN32)
        # Windows: определяем имена для текущей конфигурации
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
        # Linux код остаётся без изменений
        string(REGEX REPLACE "[0-9]+$" "" LIB_NAME_NO_VERSION "${LIB_NAME_BASE}")
        set(LIB_FILE "${QTITAN_LIB_DIR}/lib${LIB_NAME_BASE}.so")

        if(NOT EXISTS ${LIB_FILE})
            file(GLOB LIB_FILE_VERSIONED "${QTITAN_LIB_DIR}/lib${LIB_NAME_NO_VERSION}.so*")
            if(LIB_FILE_VERSIONED)
                list(SORT LIB_FILE_VERSIONED)
                list(REVERSE LIB_FILE_VERSIONED)
                list(GET LIB_FILE_VERSIONED 0 LIB_FILE)
                message(STATUS "  Found versioned: ${LIB_FILE}")
            else()
                message(FATAL_ERROR "Qtitan library not found: ${QTITAN_LIB_DIR}/lib${LIB_NAME_NO_VERSION}.so*")
            endif()
        endif()

        add_library(Qtitan::${LIB_NAME_BASE} SHARED IMPORTED)
        set_target_properties(Qtitan::${LIB_NAME_BASE} PROPERTIES
            IMPORTED_LOCATION "${LIB_FILE}"
            INTERFACE_INCLUDE_DIRECTORIES "${QTITAN_ROOT}/include"
        )
        message(STATUS "  Added Qtitan::${LIB_NAME_BASE}")
    endif()
endfunction()
