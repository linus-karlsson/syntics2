@echo off

set CompilerFlags=-m64 -Wno-null-dereference -g -DDEBUG -DCRASH_DEREF
set Libraries=vulkan-1.lib user32.lib Winmm.lib stb_image.lib
set Files=..\Syntics\src\syntics.c 
set IncludeDirs=-I..\Syntics\headers -I..\Syntics\src -I..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include 
set LibraryDirs="C:\VulkanSDK\1.3.236.0\Lib"

IF NOT EXIST build (mkdir build) 
IF NOT EXIST build\stb_image.lib (call .\Commands\buildstb.bat)

pushd build
IF NOT EXIST bin mkdir bin

echo Clang compile %Files%
clang %IncludeDirs% %CompilerFlags% %Files% -L..\build -LC:\VulkanSDK\1.3.236.0\Lib -lvulkan-1 -luser32 -lwinmm -lstb_image -o bin\Syntics.exe 

popd

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
