@echo off

set WarningEliminations=-wd4100 -wd4201
set CompilerFlags=-WL -nologo -Gm- -WX -W4 %WarningEliminations% -O2 -DNDEBUG -Fe"build/bin/definition_generator" -Fo"build/"
set Libraries= user32.lib Winmm.lib
set Files=./Definition_Generator/src/main.c 
set IncludeDirs=-I./Definition_Generator/src

IF NOT EXIST build/bin mkdir build\bin

cl %CompilerFlags% %IncludeDirs% %Files% /link %Libraries%

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
