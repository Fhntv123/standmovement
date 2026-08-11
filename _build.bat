@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul
cd /d "ImGui DirectX 11 Kiero Hook"
MSBuild "ImGui DirectX 11 Kiero Hook.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /nologo
