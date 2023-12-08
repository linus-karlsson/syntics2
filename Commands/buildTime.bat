@echo off

REM call .\Commands\time.bat .\Commands\buildMSVC.bat
call .\Commands\buildMSVC.bat
REM call .\Commands\time.bat cmake --build build
REM del .\build\CMakeFiles\Syntics.dir\Syntics\src\*.obj
REM del .\build\CMakeFiles\Syntics.dir\Syntics\src\math\*.obj
REM del .\build\CMakeFiles\Syntics.dir\Syntics\src\win32\*.obj
REM call cmake --build build
