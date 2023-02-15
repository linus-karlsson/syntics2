@echo off

cmake --build build
if %errorlevel% neq 0 exit /b %errorlevel%
.\build\bin\Syntics.exe
