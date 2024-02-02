@echo off

REM call .\Commands\time.bat .\Commands\buildMSVC.bat
REM call .\Commands\buildMSVC.bat
del .\build\CMakeFiles\Syntics.dir\Syntics\src\*.obj
del .\build\CMakeFiles\Syntics.dir\Syntics\src\math\*.obj
del .\build\CMakeFiles\Syntics.dir\Syntics\src\win32\*.obj
REM call .\Commands\time.bat cmake --build build
call cmake --build build
