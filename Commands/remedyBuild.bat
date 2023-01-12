@echo off

call .\Commands\Ninja\configDebug.bat
call .\Commands\build.bat
..\remedybg_0_3_8_6\remedybg.exe Remedy\Syntics.rdbg 
