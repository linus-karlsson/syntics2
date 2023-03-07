@echo off

call .\Commands\build.bat
if %errorlevel% neq 0 exit /b %errorlevel%
.\build\bin\Syntics.exe
