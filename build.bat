@echo off
chcp 65001 >nul

echo ============================================================
echo   yaw_cc build script
echo ============================================================
echo.
echo Working directory: %cd%
echo.

:: === 1) Git sync ===
echo [1/3] Syncing with remote...
git pull --rebase
if errorlevel 1 (
    echo [!] Pull failed! Trying without rebase...
    git pull
)
echo.

:: === 2) Find MSBuild ===
echo [2/3] Building Release x64...

set "MSBUILD_PATH="

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    echo Found MSBuild: Community 2022
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    echo Found MSBuild: Professional 2022
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    echo Found MSBuild: Enterprise 2022
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    echo Found MSBuild: Community 2019
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    echo Found MSBuild: Professional 2019
)

if "%MSBUILD_PATH%"=="" (
    where msbuild >nul 2>nul
    if errorlevel 0 (
        set "MSBUILD_PATH=msbuild"
        echo Found MSBuild in PATH
    )
)

if "%MSBUILD_PATH%"=="" (
    echo [!] MSBuild not found!
    exit /b 1
)

:: === 3) Build ===
echo.
"%MSBUILD_PATH%" "ImGui DirectX 11 Kiero Hook.sln" /p:Configuration=Release /p:Platform=x64 /v:minimal

if errorlevel 1 (
    echo [!] Build failed!
    exit /b 1
)

echo.
echo Build finished successfully!
:: НЕТ PAUSE!