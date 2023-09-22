@echo off

set CompilerFlags=-m64 -Wno-null-dereference -g -DDEBUG -DCRASH_DEREF
set Libraries=vulkan-1.lib user32.lib Winmm.lib stb_image.lib
set Files=.\Syntics\src\syntics.c 
set IncludeDirs=-I.\Syntics\headers -I.\Syntics\src -I.\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include 
set LibraryDirs="C:\VulkanSDK\1.3.236.0\Lib"

IF NOT EXIST build\bin mkdir build\bin
IF NOT EXIST build\stb_image.obj (clang %IncludeDirs% %CompilerFlags% -c .\build\stb_image.c -o .\build\stb_image.obj.obj)

echo Clang compile %Files%
clang %IncludeDirs% %CompilerFlags% -c %Files% -o .\build\syntics.obj
clang -LC:\VulkanSDK\1.3.236.0\Lib -lvulkan-1 -luser32 -lwinmm .\build\syntics.obj .\build\stb_image.obj -o build\bin\Syntics.exe 

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
