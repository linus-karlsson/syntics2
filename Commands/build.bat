@echo off

del .\build\CMakeFiles\Syntics.dir\Syntics\src\*.obj
REM del .\build\Syntics.dir\Debug\syntics.obj
cmake --build build
rem cmake --build build -- --quiet
