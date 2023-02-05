@echo off

call .\Commands\compile_shader.bat
cmake --build build
.\build\bin\Syntics.exe
