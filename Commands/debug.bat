@echo off

call .\Commands\build.bat
..\gdb --args .\build\bin\Syntics.exe
