@echo off

call .\Commands\buildNoCrash.bat
if %errorlevel% neq 0 exit /b %errorlevel%
.\build\syntics.exe
