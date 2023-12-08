@echo off

REM call .\Commands\buildNoCrash.bat
if %errorlevel% neq 0 exit /b %errorlevel%
.\build\bin\Syntics.exe
