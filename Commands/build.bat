@echo off

del .\build\CMakeFiles\Syntics.dir\Syntics\src\*.obj
del .\build\CMakeFiles\Syntics.dir\Syntics\src\math\*.obj
del .\build\CMakeFiles\Syntics.dir\Syntics\src\win32\*.obj
cmake --build build

