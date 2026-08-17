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
git pull
if errorlevel 1 (
    echo [!] Pull failed! Trying without rebase...
    git pull
)
echo.

:: === 2) Build ===
echo [2/3] Building Release x64...
msbuild "ImGui DirectX 11 Kiero Hook.sln" /p:Configuration=Release /p:Platform=x64 /v:minimal

echo.
echo Build finished with code: %errorlevel%
pause