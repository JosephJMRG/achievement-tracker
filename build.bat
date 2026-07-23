@echo off
setlocal enabledelayedexpansion

set REPO_DIR=%~dp0
set BUILD_DIR=%REPO_DIR%build

echo ============================================
echo  Achievement Tracker - Builder
echo ============================================
echo.

REM Check for vcvars64.bat (VS 2026 / 18)
set VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%VCVARS%" (
    echo [ERROR] vcvars64.bat not found at:
    echo   %VCVARS%
    echo.
    echo Make sure Visual Studio 2026 Community is installed.
    pause
    exit /b 1
)

REM Check GEODE_SDK
if not defined GEODE_SDK (
    echo [WARNING] GEODE_SDK is not set. Assuming default...
    set GEODE_SDK=%USERPROFILE%\Documents\Geode
    if not exist "!GEODE_SDK!" (
        echo [ERROR] Geode SDK not found at !GEODE_SDK!
        echo   Set the GEODE_SDK environment variable to your Geode SDK path.
        pause
        exit /b 1
    )
)

echo [OK] Geode SDK: %GEODE_SDK%
echo.

REM Enable MSVC environment
call "%VCVARS%" >nul
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment.
    pause
    exit /b 1
)
echo [OK] MSVC environment initialized
echo.

REM Create build directory if needed
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    echo [INFO] Created build directory
)

REM Configure CMake (only if not configured or CMakeLists changed)
cd /d "%BUILD_DIR%"
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)
echo [OK] CMake configured
echo.

REM Build
echo Building...
cmake --build . 2>&1
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo.
echo ============================================
echo  Build SUCCESS
echo  Output: %BUILD_DIR%josephjmrg.achievement_tracker.geode
echo ============================================
echo.
echo The mod has been installed automatically.
echo.
pause
