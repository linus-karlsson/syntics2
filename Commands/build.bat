@echo off

rem del .\build\CMakeFiles\Syntics.dir\Syntics\src\*.obj
rem del .\build\CMakeFiles\Syntics.dir\Syntics\src\math\*.obj
rem del .\build\CMakeFiles\Syntics.dir\Syntics\src\win32\*.obj
cmake --build build
rem cmake --build build -- --quiet
