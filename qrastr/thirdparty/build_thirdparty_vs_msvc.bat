@echo off
setlocal enabledelayedexpansion

echo ===========================================================================
echo Building precompiled libraries for Windows (MSVC)
echo ===========================================================================
echo.

REM ---------------------------------------------------------------------------
REM Paths - adjust these as needed
REM ---------------------------------------------------------------------------
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
set "CMAKE_BIN=C:\Qt\Tools\CMake_64\bin\cmake.exe"
set "QT_DIR=C:\Qt\6.9.3\msvc2022_64"
set "PYTHON_BIN=C:\Users\kappes-ad\AppData\Local\Programs\Python\Python312\python.exe"
set "POSTGRESQL_ROOT=C:\Program Files\PostgreSQL\18"
REM Необходимо установить nasm: https://www.nasm.us/pub/nasm/releasebuilds/3.01/win64/
REM Для правильной сборки SDL3_image под msvc
set PATH=C:\nasm-3.01-win64\nasm-3.01;%PATH% 
set "NINJA_BIN=C:\Qt\Tools\Ninja\ninja.exe"

REM Script directory and THIRDPARTY_DIR (no trailing backslash)
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" (
    set "THIRDPARTY_DIR=%SCRIPT_DIR:~0,-1%"
) else (
    set "THIRDPARTY_DIR=%SCRIPT_DIR%"
)

REM Directories used by script (use explicit backslashes when joining)
set "COMPILE_DIR=%THIRDPARTY_DIR%\compile"
REM ВАЖНО: Устанавливаем PLATFORM_NAME до инициализации vcvarsall
set "PLATFORM_NAME=win\msvc"

if "%~1"=="" (
    set "BUILD_TYPES=Release Debug"
) else (
    set "BUILD_TYPES=%~1"
)

if "%KEEP_BUILD%"=="" set "KEEP_BUILD=0"

echo Configuration:
echo   THIRDPARTY_DIR: %THIRDPARTY_DIR%
echo   COMPILE_DIR:    %COMPILE_DIR%
echo   VCVARS:         %VCVARS%
echo   QT_DIR:         %QT_DIR%
echo   PLATFORM_NAME:  %PLATFORM_NAME%
echo   BUILD_TYPES:    %BUILD_TYPES%
echo   KEEP_BUILD:     %KEEP_BUILD%
echo.

REM ---------------------------------------------------------------------------
REM Basic checks
REM ---------------------------------------------------------------------------
if not exist "%VCVARS%" (
    echo ERROR: vcvarsall.bat not found: %VCVARS%
    exit /b 1
)
call "%VCVARS%" x64
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment
    exit /b 1
)
echo [OK] MSVC environment initialized

set "USE_NINJA=0"
if exist "%NINJA_BIN%" (
    "%NINJA_BIN%" --version >nul 2>&1
    if not errorlevel 1 (
        set "USE_NINJA=1"
        echo [OK] Ninja found: %NINJA_BIN%
    ) else (
        echo [WARNING] Ninja found but failed to run - will use NMake instead
    )
) else (
    echo [WARNING] Ninja not found at %NINJA_BIN% - will use NMake Makefiles instead
)

REM Восстанавливаем PLATFORM_NAME после vcvarsall
set "PLATFORM_NAME=win\msvc"

if not exist "%CMAKE_BIN%" (
    echo ERROR: CMake not found at %CMAKE_BIN%
    exit /b 1
)
"%CMAKE_BIN%" --version >nul 2>&1 || ( echo ERROR: Failed to run CMake at %CMAKE_BIN% & exit /b 1 )
echo [OK] CMake found: %CMAKE_BIN%

if not exist "%QT_DIR%\bin\qmake.exe" (
    echo ERROR: Qt not found at: %QT_DIR%
    exit /b 1
)
echo [OK] Qt found: %QT_DIR%

if defined PYTHON_BIN (
    %PYTHON_BIN% --version >nul 2>&1
    if errorlevel 1 (
        set "PYTHON_BIN="
    )
)
if not defined PYTHON_BIN (
    echo [WARNING] Python not found - ScintillaEdit codegen will be skipped
)

if not exist "%COMPILE_DIR%" md "%COMPILE_DIR%"

REM ---------------------------------------------------------------------------
REM перепрыгиваем к main чтобы пропустить определения функций
goto :main


REM ---------------------------------------------------------------------------
REM Helper: check source dir exists
REM ---------------------------------------------------------------------------
:assert_src
if "%~1"=="" (
    echo ERROR: Internal: expected source path but parameter empty
    exit /b 1
)
if not exist "%~1" (
    echo ERROR: Source directory not found: %~1
    exit /b 1
)
exit /b 0

REM ---------------------------------------------------------------------------
REM build_library (CMake + Ninja/Nmake)
REM Usage: call :build_library <name> "<source_dir>" <build_type> "<extra_flags>"
REM ---------------------------------------------------------------------------
:build_library
set "LIB_NAME=%~1"
set "LIB_SRC=%~2"
set "BUILD_TYPE=%~3"
set "EXTRA_FLAGS=%~4"

if "%LIB_NAME%"=="" (
    echo ERROR: build_library: LIB_NAME empty
    exit /b 1
)
if "%LIB_SRC%"=="" (
    echo ERROR: build_library: LIB_SRC empty for %LIB_NAME%
    exit /b 1
)

call :assert_src "%LIB_SRC%"

set "INSTALL_DIR=%COMPILE_DIR%\%LIB_NAME%\%PLATFORM_NAME%\%BUILD_TYPE%"
set "BUILD_DIR=%THIRDPARTY_DIR%\_build_%LIB_NAME%_%BUILD_TYPE%"

echo ===========================================================================
echo Building %LIB_NAME% (%BUILD_TYPE%) via CMake
echo ===========================================================================
echo Source:  %LIB_SRC%
echo Install: %INSTALL_DIR%
echo Build:   %BUILD_DIR%
echo.

if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"

set "CMAKE_PREFIX_PATH=%QT_DIR%"
if exist "%COMPILE_DIR%\fmt\%PLATFORM_NAME%\%BUILD_TYPE%" set "CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!;%COMPILE_DIR%\fmt\%PLATFORM_NAME%\%BUILD_TYPE%"
if exist "%COMPILE_DIR%\SDL3\%PLATFORM_NAME%\%BUILD_TYPE%" set "CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!;%COMPILE_DIR%\SDL3\%PLATFORM_NAME%\%BUILD_TYPE%"
if exist "%POSTGRESQL_ROOT%" set "CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!;%POSTGRESQL_ROOT%"

if "%USE_NINJA%"=="1" (
    "%CMAKE_BIN%" -B "%BUILD_DIR%" -S "%LIB_SRC%" ^
        -G "Ninja" ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
        -DCMAKE_PREFIX_PATH="!CMAKE_PREFIX_PATH!" ^
        -DCMAKE_MAKE_PROGRAM="%NINJA_BIN%" ^
        %EXTRA_FLAGS%
) else (
    "%CMAKE_BIN%" -B "%BUILD_DIR%" -S "%LIB_SRC%" ^
        -G "NMake Makefiles" ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
        -DCMAKE_PREFIX_PATH="!CMAKE_PREFIX_PATH!" ^
        %EXTRA_FLAGS%
)

if errorlevel 1 (
    echo ERROR: Configuration failed for %LIB_NAME%
    exit /b 1
)

"%CMAKE_BIN%" --build "%BUILD_DIR%" --parallel
if errorlevel 1 (
    echo ERROR: Build failed for %LIB_NAME%
    exit /b 1
)

"%CMAKE_BIN%" --install "%BUILD_DIR%"
if errorlevel 1 (
    echo ERROR: Install failed for %LIB_NAME%
    exit /b 1
)

if "%KEEP_BUILD%"=="0" rd /s /q "%BUILD_DIR%"
echo [OK] %LIB_NAME% (%BUILD_TYPE%) installed at %INSTALL_DIR%
exit /b 0

REM ---------------------------------------------------------------------------
REM build_scintilla (nmake)
REM Usage: call :build_scintilla "<scintilla_src_dir>" <build_type>
REM ---------------------------------------------------------------------------
:build_scintilla
set "LIB_SRC=%~1"
set "BUILD_TYPE=%~2"
if "%LIB_SRC%"=="" (
    echo ERROR: build_scintilla: LIB_SRC empty
    exit /b 1
)
call :assert_src "%LIB_SRC%"

set "SCINTILLA_MAK=%LIB_SRC%\win32\scintilla.mak"
if not exist "%SCINTILLA_MAK%" (
    echo ERROR: scintilla.mak not found at %SCINTILLA_MAK%
    exit /b 1
)

set "INSTALL_DIR=%COMPILE_DIR%\scintilla\%PLATFORM_NAME%\%BUILD_TYPE%"

echo ===========================================================================
echo Building Scintilla (%BUILD_TYPE%) via nmake
echo ===========================================================================
echo Source: %LIB_SRC%
echo Makefile: %SCINTILLA_MAK%
echo Install: %INSTALL_DIR%
echo.

pushd "%LIB_SRC%\win32"

REM Очистка предыдущей сборки
nmake /f scintilla.mak clean
if errorlevel 1 ( popd & echo ERROR: scintilla clean failed & exit /b 1 )

REM Сборка
if /i "%BUILD_TYPE%"=="Debug" (
    nmake /f scintilla.mak DEBUG=1
) else (
    nmake /f scintilla.mak
)
if errorlevel 1 ( popd & echo ERROR: scintilla build failed & exit /b 1 )

REM Создание директорий для установки
if not exist "%INSTALL_DIR%\lib" md "%INSTALL_DIR%\lib"
if not exist "%INSTALL_DIR%\include" md "%INSTALL_DIR%\include"
if not exist "%INSTALL_DIR%\bin" md "%INSTALL_DIR%\bin"

REM Копируем библиотеки (.lib файлы)
if exist "..\bin\libscintilla.lib" (
    copy /Y "..\bin\libscintilla.lib" "%INSTALL_DIR%\lib\" >nul 2>&1
    echo   Copied libscintilla.lib
)
if exist "..\bin\Scintilla.lib" (
    copy /Y "..\bin\Scintilla.lib" "%INSTALL_DIR%\lib\" >nul 2>&1
    echo   Copied Scintilla.lib
)

REM Копируем DLL
if exist "..\bin\Scintilla.dll" (
    copy /Y "..\bin\Scintilla.dll" "%INSTALL_DIR%\bin\" >nul 2>&1
    echo   Copied Scintilla.dll
) else (
    echo   WARNING: Scintilla.dll not found in ..\bin\
)

REM Копируем заголовки из include/
if exist "..\include\*.h" (
    copy /Y "..\include\*.h" "%INSTALL_DIR%\include\" >nul 2>&1
    echo   Copied headers from include\
)

REM Копируем заголовки из src/ (если нужны)
if exist "..\src\*.h" (
    copy /Y "..\src\*.h" "%INSTALL_DIR%\include\" >nul 2>&1
    echo   Copied headers from src\
)

popd

echo [OK] Scintilla (%BUILD_TYPE%) installed at %INSTALL_DIR%
echo   - Static library: %INSTALL_DIR%\lib\libscintilla.lib
echo   - Import library: %INSTALL_DIR%\lib\Scintilla.lib
echo   - DLL file: %INSTALL_DIR%\bin\Scintilla.dll
exit /b 0

REM ---------------------------------------------------------------------------
REM build_lexilla (nmake)
REM Usage: call :build_lexilla "<lexilla_src_dir>" <build_type>
REM ---------------------------------------------------------------------------
:build_lexilla
set "LIB_SRC=%~1"
set "BUILD_TYPE=%~2"
if "%LIB_SRC%"=="" (
    echo ERROR: build_lexilla: LIB_SRC empty
    exit /b 1
)
call :assert_src "%LIB_SRC%"

set "LEXILLA_MAK=%LIB_SRC%\lexilla.mak"
if not exist "%LEXILLA_MAK%" (
    echo ERROR: lexilla.mak not found at %LEXILLA_MAK%
    exit /b 1
)

set "INSTALL_DIR=%COMPILE_DIR%\lexilla\%PLATFORM_NAME%\%BUILD_TYPE%"

echo ===========================================================================
echo Building Lexilla (%BUILD_TYPE%) via nmake
echo ===========================================================================
echo Source: %LIB_SRC%
echo Makefile: %LEXILLA_MAK%
echo Install: %INSTALL_DIR%
echo.

pushd "%LIB_SRC%"

REM Очистка предыдущей сборки
nmake /f "%LEXILLA_MAK%" clean
if errorlevel 1 ( popd & echo ERROR: lexilla clean failed & exit /b 1 )

REM Сборка
if /i "%BUILD_TYPE%"=="Debug" (
    nmake /f "%LEXILLA_MAK%" DEBUG=1
) else (
    nmake /f "%LEXILLA_MAK%"
)
if errorlevel 1 ( popd & echo ERROR: lexilla build failed & exit /b 1 )

REM Создание директорий для установки
if not exist "%INSTALL_DIR%\lib" md "%INSTALL_DIR%\lib"
if not exist "%INSTALL_DIR%\include" md "%INSTALL_DIR%\include"
if not exist "%INSTALL_DIR%\bin" md "%INSTALL_DIR%\bin"

REM Копируем библиотеки (.lib файлы)
if exist "..\bin\lexilla.lib" (
    copy /Y "..\bin\lexilla.lib" "%INSTALL_DIR%\lib\" >nul 2>&1
    echo   Copied lexilla.lib
)
if exist "..\bin\liblexilla.lib" (
    copy /Y "..\bin\liblexilla.lib" "%INSTALL_DIR%\lib\" >nul 2>&1
    echo   Copied liblexilla.lib
)

REM Переименовываем DLL в lexilla5.dll для совместимости с кодом
if exist "..\bin\lexilla.dll" (
    copy /Y "..\bin\lexilla.dll" "%INSTALL_DIR%\bin\lexilla5.dll" >nul 2>&1
    echo   Created lexilla5.dll from lexilla.dll
) else (
    echo   WARNING: lexilla.dll not found in ..\bin\
)

REM Копируем заголовки из include/
if exist "..\include\*.h" (
    copy /Y "..\include\*.h" "%INSTALL_DIR%\include\" >nul 2>&1
    echo   Copied headers from include\
)

REM Копируем заголовки из access/ (если есть)
if exist "..\access\*.h" (
    copy /Y "..\access\*.h" "%INSTALL_DIR%\include\" >nul 2>&1
    echo   Copied headers from access\
)

popd

echo [OK] Lexilla (%BUILD_TYPE%) installed at %INSTALL_DIR%
echo   - Import library: %INSTALL_DIR%\lib\lexilla.lib
echo   - Static library: %INSTALL_DIR%\lib\liblexilla.lib
echo   - DLL file: %INSTALL_DIR%\bin\lexilla5.dll
exit /b 0
REM ---------------------------------------------------------------------------
REM build_qmake_library (qmake + nmake)
REM Usage: call :build_qmake_library "<name>" "<src_dir>" <build_type> <need_codegen>
REM ---------------------------------------------------------------------------
:build_qmake_library
set "LIB_NAME=%~1"
set "LIB_SRC=%~2"
set "BUILD_TYPE=%~3"
set "NEED_CODEGEN=%~4"

if "%LIB_SRC%"=="" (
    echo ERROR: build_qmake_library: LIB_SRC empty
    exit /b 1
)
call :assert_src "%LIB_SRC%"

set "INSTALL_DIR=%COMPILE_DIR%\%LIB_NAME%\%PLATFORM_NAME%\%BUILD_TYPE%"
set "BUILD_DIR=%THIRDPARTY_DIR%\_build_%LIB_NAME%_%BUILD_TYPE%"

echo ===========================================================================
echo Building %LIB_NAME% (%BUILD_TYPE%) via qmake
echo ===========================================================================
echo Source:  %LIB_SRC%
echo Install: %INSTALL_DIR%
echo Build:   %BUILD_DIR%
echo.

if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
md "%BUILD_DIR%"
pushd "%BUILD_DIR%"

if /i "%BUILD_TYPE%"=="Debug" (
    set "QMAKE_CONFIG=CONFIG+=debug CONFIG-=release"
) else (
    set "QMAKE_CONFIG=CONFIG+=release CONFIG-=debug"
)

if "%NEED_CODEGEN%"=="1" (
    if not defined PYTHON_BIN (
        echo ERROR: Python required for codegen
        popd
        exit /b 1
    )
    pushd "%LIB_SRC%"
    %PYTHON_BIN% WidgetGen.py
    if errorlevel 1 ( popd & popd & echo ERROR: codegen failed & exit /b 1 )
    popd
)

for %%f in ("%LIB_SRC%\*.pro") do set "PRO_FILE=%%f"
if not defined PRO_FILE (
    popd
    echo ERROR: .pro file not found in %LIB_SRC%
    exit /b 1
)

echo Running qmake with: %QMAKE_CONFIG%
"%QT_DIR%\bin\qmake.exe" "%PRO_FILE%" %QMAKE_CONFIG% ^
    "QMAKE_CXXFLAGS+=/FImemory /FIstring /FIvector /FIoptional"
if errorlevel 1 ( popd & echo ERROR: qmake failed & exit /b 1 )

nmake
if errorlevel 1 ( popd & echo ERROR: nmake failed & exit /b 1 )

if not exist "%INSTALL_DIR%\lib" md "%INSTALL_DIR%\lib"
if not exist "%INSTALL_DIR%\include" md "%INSTALL_DIR%\include"

set "OUTPUT_BIN=%BUILD_DIR%\..\..\bin"
echo Looking for output files in: %OUTPUT_BIN%

if exist "%OUTPUT_BIN%" (
    echo Copying from output directory: %OUTPUT_BIN%
    for %%f in ("%OUTPUT_BIN%\%LIB_NAME%*.lib" "%OUTPUT_BIN%\%LIB_NAME%*.dll") do (
        if exist "%%f" (
            echo   Copying %%~nxf
            copy /Y "%%f" "%INSTALL_DIR%\lib\" >nul 2>&1
        )
    )
)

REM Копируем заголовки
for %%f in ("%LIB_SRC%\*.h") do copy /Y "%%f" "%INSTALL_DIR%\include\" >nul 2>&1

if "%LIB_NAME%"=="ScintillaEdit" if exist "%LIB_SRC%\ScintillaDocument.h" copy /Y "%LIB_SRC%\ScintillaDocument.h" "%INSTALL_DIR%\include\" >nul 2>&1

popd
if "%KEEP_BUILD%"=="0" rd /s /q "%BUILD_DIR%"
echo [OK] %LIB_NAME% (%BUILD_TYPE%) installed at %INSTALL_DIR%
exit /b 0

REM ---------------------------------------------------------------------------
REM build_metakit (cl + lib, без CMake)
REM Usage: call :build_metakit "<src_dir>" <build_type>
REM ---------------------------------------------------------------------------
:build_metakit
set "LIB_SRC=%~1"
set "BUILD_TYPE=%~2"

if "%LIB_SRC%"=="" (
    echo ERROR: build_metakit: LIB_SRC empty
    exit /b 1
)
call :assert_src "%LIB_SRC%"

set "INSTALL_DIR=%COMPILE_DIR%\metakit\%PLATFORM_NAME%\%BUILD_TYPE%"
set "OBJ_DIR=%THIRDPARTY_DIR%\_build_metakit_%BUILD_TYPE%"

echo ===========================================================================
echo Building metakit (%BUILD_TYPE%) via cl.exe (no CMake)
echo ===========================================================================
echo Source:  %LIB_SRC%
echo Install: %INSTALL_DIR%
echo ObjDir:  %OBJ_DIR%
echo.

if exist "%OBJ_DIR%" rd /s /q "%OBJ_DIR%"
md "%OBJ_DIR%"

REM --- Базовые флаги ---
set "CL_FLAGS=/nologo /EHsc /std:c++14 /D_CRT_SECURE_NO_WARNINGS"

REM --- CRT и оптимизация ---
REM Debug:   CMake по умолчанию → /MDd
REM Release: -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded → /MT
if /i "%BUILD_TYPE%"=="Debug" (
    set "CL_FLAGS=%CL_FLAGS% /Od /Zi /MDd /DDEBUG"
) else (
    set "CL_FLAGS=%CL_FLAGS% /O2 /MT /DNDEBUG"
)

REM --- Include пути (идентично CMake: win/ = config.h, include/, src/) ---
set "CL_INCLUDES=/I"%LIB_SRC%\win" /I"%LIB_SRC%\include" /I"%LIB_SRC%\src""

REM --- Компилируем все .cpp из src/ ---
pushd "%OBJ_DIR%"
for %%f in ("%LIB_SRC%\src\*.cpp") do (
    echo   Compiling %%~nxf ...
    cl.exe %CL_FLAGS% %CL_INCLUDES% /c "%%f" /Fo"%%~nf.obj"
    if errorlevel 1 (
        popd
        echo ERROR: Compilation failed for %%~nxf
        exit /b 1
    )
)

REM --- Собираем статическую библиотеку с именем mk4.lib (как в CMake) ---
if not exist "%INSTALL_DIR%\lib" md "%INSTALL_DIR%\lib"
if not exist "%INSTALL_DIR%\include" md "%INSTALL_DIR%\include"

lib.exe /nologo /OUT:"%INSTALL_DIR%\lib\mk4.lib" *.obj
if errorlevel 1 (
    popd
    echo ERROR: lib.exe failed for metakit
    exit /b 1
)

REM --- Копируем публичные заголовки (идентично CMake: install DIRECTORY include/) ---
copy /Y "%LIB_SRC%\include\*.h" "%INSTALL_DIR%\include\" >nul 2>&1

popd
if "%KEEP_BUILD%"=="0" rd /s /q "%OBJ_DIR%"

echo [OK] metakit (%BUILD_TYPE%) installed at %INSTALL_DIR%
echo   - Static library: %INSTALL_DIR%\lib\mk4.lib
exit /b 0
REM ---------------------------------------------------------------------------
REM Main
REM ---------------------------------------------------------------------------
:main
echo Starting build...
cd /d "%THIRDPARTY_DIR%"

for %%T in (%BUILD_TYPES%) do (
    echo.
    echo ===========================================================================
    echo Building configuration: %%T
    echo ===========================================================================
    
    if /i "%%T"=="Debug" (
        call :build_library "fmt" "%THIRDPARTY_DIR%\fmt" "%%T" "-DFMT_TEST=OFF -DBUILD_SHARED_LIBS=OFF"
        call :build_library "spdlog" "%THIRDPARTY_DIR%\spdlog" "%%T" "-DSPDLOG_FMT_EXTERNAL=ON -DBUILD_SHARED_LIBS=OFF"
    ) else (
        call :build_library "fmt" "%THIRDPARTY_DIR%\fmt" "%%T" "-DFMT_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
        call :build_library "spdlog" "%THIRDPARTY_DIR%\spdlog" "%%T" "-DSPDLOG_FMT_EXTERNAL=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
    )
    
	call :build_scintilla "%THIRDPARTY_DIR%\scintilla" "%%T"   
    call :build_library "SDL3" "%THIRDPARTY_DIR%\SDL" "%%T" "-DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF -DSDL_INSTALL=ON -DSDL_SHARED=ON -DSDL_STATIC=OFF"
    call :build_library "SDL3_image" "%THIRDPARTY_DIR%\SDL_image" "%%T" "-DSDL3IMAGE_SAMPLES=OFF -DBUILD_SHARED_LIBS=ON"
    call :build_lexilla "%THIRDPARTY_DIR%\lexilla\src" "%%T"
    call :build_library "qtadvanceddocking" "%THIRDPARTY_DIR%\Qt-Advanced-Docking-System" "%%T" "-DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON"
    REM call :build_metakit "%THIRDPARTY_DIR%\metakit" "%%T"
    call :build_qmake_library "ScintillaEditBase" "%THIRDPARTY_DIR%\scintilla\qt\ScintillaEditBase" "%%T" "0"
    call :build_qmake_library "ScintillaEdit" "%THIRDPARTY_DIR%\scintilla\qt\ScintillaEdit" "%%T" "1"
   
    REM Очищаем временную папку bin
    if exist "%THIRDPARTY_DIR%\..\bin" (
        echo Cleaning temporary bin directory: %THIRDPARTY_DIR%\..\bin
        rd /s /q "%THIRDPARTY_DIR%\..\bin"
    )
    
    REM Optional libpqxx
    if exist "%POSTGRESQL_ROOT%" (
        set "PostgreSQL_INCLUDE_DIR=%POSTGRESQL_ROOT%\include"
        set "PostgreSQL_LIBRARY=%POSTGRESQL_ROOT%\lib\libpq.lib"
        if exist "!PostgreSQL_INCLUDE_DIR!" if exist "!PostgreSQL_LIBRARY!" (
            call :build_library "libpqxx" "%THIRDPARTY_DIR%\libpqxx" "%%T" "-DSKIP_BUILD_TEST=ON -DBUILD_SHARED_LIBS=OFF -DPostgreSQL_INCLUDE_DIR=!PostgreSQL_INCLUDE_DIR! -DPostgreSQL_LIBRARY=!PostgreSQL_LIBRARY!"
        ) else (
            echo [SKIP] libpqxx - expected headers/libs not found
        )
    ) else (
        echo [SKIP] libpqxx - PostgreSQL not installed
    )
)

echo ===========================================================================
echo Build completed
echo ===========================================================================
goto :finish

:finish
endlocal
echo All libraries built under MSVC successfully.
exit /b 0