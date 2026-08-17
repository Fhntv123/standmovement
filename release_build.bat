@echo off
setlocal

call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
if errorlevel 1 exit /b 1

msbuild "ImGui DirectX 11 Kiero Hook.sln" ^
  /m ^
  /t:Rebuild ^
  /p:Configuration=Release ^
  /p:Platform=x64 ^
  /p:Optimization=MaxSpeed ^
  /p:WholeProgramOptimization=true ^
  /p:LinkTimeCodeGeneration=UseLinkTimeCodeGeneration ^
  /p:FunctionLevelLinking=true ^
  /p:EnableCOMDATFolding=true ^
  /p:OptimizeReferences=true ^
  /p:DebugSymbols=false ^
  /p:DebugType=None ^
  /p:GenerateDebugInformation=false ^
  /p:GenerateMapFile=false ^
  /p:LinkIncremental=false ^
  /v:minimal

if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)

echo BUILD COMPLETE
endlocal