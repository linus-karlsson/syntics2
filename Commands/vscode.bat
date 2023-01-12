@echo off

call .\Commands\Ninja\configDebug.bat
call .\Commands\build.bat
code .
