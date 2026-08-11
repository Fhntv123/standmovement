@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
title yaw_cc build

echo.
echo ============================================================
echo   yaw_cc build script
echo ============================================================
echo.

REM Anchor to the bat's own directory — works with cyrillic in path
cd /d "%~dp0"
if errorlevel 1 (
    echo [ERROR] failed to cd into %~dp0
    exit /b 1
)

echo Working directory: %CD%
echo.

echo [1/3] git pull...
git pull
if errorlevel 1 (
    echo [ERROR] git pull failed
    exit /b 1
)
echo.

echo [2/3] MSBuild Release x64...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    "ImGui DirectX 11 Kiero Hook\ImGui DirectX 11 Kiero Hook.vcxproj" ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /nologo ^
    /v:minimal
if errorlevel 1 (
    echo.
    echo [ERROR] build failed - see errors above
    exit /b 1
)
echo.

echo [3/3] copy DLL to cheat.dll...
set "SRC=%CD%\ImGui-DirectX-11-Kiero-Hook-master\x64\Release\ze0move2.05OBTStandChillow.dll"
set "DST_DIR=%CD%\x64\Release"
set "DST=%DST_DIR%\ze0move2.05OBTStandChillow.dll"

if not exist "!SRC!" (
    echo [ERROR] built DLL not found at:
    echo         !SRC!
    echo         check vcxproj TargetName / OutDir settings
    exit /b 1
)

if not exist "%DST_DIR%" mkdir "%DST_DIR%"

copy /y "!SRC!" "!DST!" >nul
if errorlevel 1 (
    echo [ERROR] copy failed
    exit /b 1
)

echo.
echo ============================================================
echo   DONE. cheat.dll ready at:
echo   !DST!
echo ============================================================
echo.
exit /b 0
