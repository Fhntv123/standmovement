@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

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
    echo [!] Pull failed!
    pause
    exit /b 1
)
git push
if errorlevel 1 (
    echo [!] Push failed!
    pause
    exit /b 1
)
echo.

:: === 2) Build ===
echo [2/3] Building Release x64...

set "MSBUILD="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
)

if "%MSBUILD%"=="" (
    echo [!] MSBuild not found! Trying PATH...
    msbuild "ImGui DirectX 11 Kiero Hook.sln" /p:Configuration=Release /p:Platform=x64 /v:minimal
) else (
    "%MSBUILD%" "ImGui DirectX 11 Kiero Hook.sln" /p:Configuration=Release /p:Platform=x64 /v:minimal
)

if errorlevel 1 (
    echo [!] Build failed!
    pause
    exit /b 1
)
echo.

:: === 3) Copy DLL ===
echo [3/3] Copying DLL to cheat.dll...

for /r "ImGui DirectX 11 Kiero Hook\x64\Release" %%f in (*.dll) do (
    echo   Found: %%f
    copy /Y "%%f" "cheat.dll"
    echo   [OK] Copied to cheat.dll
    goto :done
)

echo [ERROR] No DLL found!
pause
exit /b 1

:done
echo.
echo ============================================================
echo   BUILD COMPLETE
echo ============================================================
pause