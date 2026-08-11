@echo off
echo.
echo === building solution (Release x64) ===

set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if not exist %MSBUILD% set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
if not exist %MSBUILD% set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
if not exist %MSBUILD% set MSBUILD="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if not exist %MSBUILD% set MSBUILD=msbuild

%MSBUILD% "ImGui DirectX 11 Kiero Hook.sln" /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal /p:IncludePath="%CD%\ImGui DirectX 11 Kiero Hook\imgui;%CD%\ImGui DirectX 11 Kiero Hook\kiero;%CD%\ImGui DirectX 11 Kiero Hook\kiero\minhook\include"

echo.
echo === build finished ===
pause