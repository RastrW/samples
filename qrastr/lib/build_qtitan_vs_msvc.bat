@echo off
setlocal enabledelayedexpansion

echo ===========================================================================
echo Building Qtitan DataGrid for Windows ^(MSVC^)
echo ===========================================================================
echo.

REM ---------------------------------------------------------------------------
REM Paths - adjust these as needed
REM ---------------------------------------------------------------------------
REM Get script directory
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Qtitan root is the script directory
set "QTITAN_ROOT=%SCRIPT_DIR%"

REM Qt path
set "QT_PATH=C:\Qt\6.9.3\msvc2022_64"

REM MSVC environment
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

REM Output directories
set "OUTPUT_BASE=%QTITAN_ROOT%\lib\msvc"

REM Build type: debug | release | both (default)
if "%~1"=="" (
    set "BUILD_TYPE=both"
) else (
    set "BUILD_TYPE=%~1"
)

echo Configuration:
echo   QTITAN_ROOT: %QTITAN_ROOT%
echo   QT_PATH:     %QT_PATH%
echo   OUTPUT_BASE: %OUTPUT_BASE%
echo   BUILD_TYPE:  %BUILD_TYPE%
echo.

REM ---------------------------------------------------------------------------
REM Checks
REM ---------------------------------------------------------------------------
echo Checking prerequisites...

if not exist "%QTITAN_ROOT%" (
    echo ERROR: Qtitan root not found: %QTITAN_ROOT%
    exit /b 1
)

if not exist "%QTITAN_ROOT%\QtitanDataGrid.pro" (
    echo ERROR: QtitanDataGrid.pro not found in: %QTITAN_ROOT%
    exit /b 1
)

if not exist "%QT_PATH%\bin\qmake.exe" (
    echo ERROR: qmake not found at: %QT_PATH%\bin\qmake.exe
    echo Please adjust QT_PATH in the script
    exit /b 1
)

if not exist "%VCVARS%" (
    echo ERROR: vcvarsall.bat not found: %VCVARS%
    exit /b 1
)

echo [OK] Qtitan root found
echo [OK] Qt found: %QT_PATH%
echo [OK] MSVC environment found
echo.

REM ---------------------------------------------------------------------------
REM Initialize MSVC environment
REM ---------------------------------------------------------------------------
echo Initializing MSVC environment...
call "%VCVARS%" x64
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment
    exit /b 1
)
echo [OK] MSVC environment initialized
echo.

REM Add Qt to PATH
set "PATH=%QT_PATH%\bin;%PATH%"

REM ---------------------------------------------------------------------------
REM Jump to main to skip function definitions
REM ---------------------------------------------------------------------------
goto :main

REM ---------------------------------------------------------------------------
REM Build function
REM Usage: call :build_qtitan <config>
REM ---------------------------------------------------------------------------
:build_qtitan
set "CONFIG=%~1"
set "BUILD_DIR=%QTITAN_ROOT%\build-msvc2022-%CONFIG%"
set "OUTPUT_DIR=%OUTPUT_BASE%\%CONFIG%"

echo ===========================================================================
echo Building Qtitan ^(%CONFIG%^)
echo ===========================================================================
echo Build dir:  %BUILD_DIR%
echo Output dir: %OUTPUT_DIR%
echo.

REM Create build directory
if exist "%BUILD_DIR%" (
    echo Removing old build directory...
    rd /s /q "%BUILD_DIR%"
)
md "%BUILD_DIR%"

REM Create output directory
if not exist "%OUTPUT_DIR%" md "%OUTPUT_DIR%"

REM Enter build directory
pushd "%BUILD_DIR%"

REM Configure with qmake
echo Running qmake...
"%QT_PATH%\bin\qmake.exe" "%QTITAN_ROOT%\QtitanDataGrid.pro" ^
    CONFIG+=%CONFIG% ^
    CONFIG+=dll ^
    CONFIG+=qtquickcompiler

if errorlevel 1 (
    echo ERROR: qmake failed
    popd
    exit /b 1
)

REM Build
echo.
echo Building with nmake...
nmake

if errorlevel 1 (
    echo ERROR: nmake failed
    popd
    exit /b 1
)

popd

REM Copy binaries to output directory
echo.
echo Copying binaries to output directory...

REM Qtitan typically outputs to bin/ directory
if exist "%QTITAN_ROOT%\bin" (
    echo   Found bin directory, copying libraries...
    
    REM Copy DLL files
    if exist "%QTITAN_ROOT%\bin\*.dll" (
        copy /Y "%QTITAN_ROOT%\bin\*.dll" "%OUTPUT_DIR%\" >nul 2>&1
        echo   Copied DLL files
    )
    
    REM Copy LIB files (import libraries)
    if exist "%QTITAN_ROOT%\bin\*.lib" (
        copy /Y "%QTITAN_ROOT%\bin\*.lib" "%OUTPUT_DIR%\" >nul 2>&1
        echo   Copied LIB files
    )
    
    REM Copy PDB files (debug symbols)
    if exist "%QTITAN_ROOT%\bin\*.pdb" (
        copy /Y "%QTITAN_ROOT%\bin\*.pdb" "%OUTPUT_DIR%\" >nul 2>&1
        echo   Copied PDB files
    )
    
    echo   Libraries copied to: %OUTPUT_DIR%
) else (
    echo   WARNING: bin directory not found at %QTITAN_ROOT%\bin
    echo   Checking build directory for libraries...
    
    REM Try to find libraries in build directory
    for /r "%BUILD_DIR%" %%f in (*.dll *.lib *.pdb) do (
        copy /Y "%%f" "%OUTPUT_DIR%\" >nul 2>&1
    )
)

REM Copy headers if needed (usually in include/ or src/)
if exist "%QTITAN_ROOT%\include" (
    if not exist "%OUTPUT_BASE%\include" (
        echo.
        echo Copying headers...
        xcopy /E /I /Y "%QTITAN_ROOT%\include" "%OUTPUT_BASE%\include" >nul 2>&1
        echo   Headers copied to: %OUTPUT_BASE%\include
    )
)

echo.
echo [OK] %CONFIG% build completed successfully
echo.
exit /b 0

REM ---------------------------------------------------------------------------
REM Main
REM ---------------------------------------------------------------------------
:main
cd /d "%QTITAN_ROOT%"

if /i "%BUILD_TYPE%"=="debug" (
    call :build_qtitan debug
    if errorlevel 1 goto :error
) else if /i "%BUILD_TYPE%"=="release" (
    call :build_qtitan release
    if errorlevel 1 goto :error
) else if /i "%BUILD_TYPE%"=="both" (
    call :build_qtitan debug
    if errorlevel 1 goto :error
    call :build_qtitan release
    if errorlevel 1 goto :error
) else (
    echo ERROR: Invalid build type: %BUILD_TYPE%
    echo Usage: %~nx0 [debug^|release^|both]
    exit /b 1
)

echo ===========================================================================
echo Qtitan build completed successfully
echo ===========================================================================
echo.
echo Output locations:
echo   Debug libraries:   %OUTPUT_BASE%\debug\
echo   Release libraries: %OUTPUT_BASE%\release\
if exist "%OUTPUT_BASE%\include" echo   Headers:           %OUTPUT_BASE%\include\
echo.
echo Build directories ^(can be removed^):
echo   %QTITAN_ROOT%\build-msvc2022-debug
echo   %QTITAN_ROOT%\build-msvc2022-release
echo.

REM Optional: Clean up bin directory if it exists
if exist "%QTITAN_ROOT%\bin" (
    echo Temporary bin directory found: %QTITAN_ROOT%\bin
    set /p CLEAN_BIN="Remove temporary bin directory? (y/N): "
    if /i "!CLEAN_BIN!"=="y" (
        rd /s /q "%QTITAN_ROOT%\bin"
        echo   Removed: %QTITAN_ROOT%\bin
    )
)

echo.
echo Done!
exit /b 0

:error
echo Build failed
exit /b 1
