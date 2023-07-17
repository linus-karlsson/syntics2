@echo off

set WarningEliminations=-wd4100 -wd4201 -wd4189
set CompilerFlags=-WL -nologo -WX -W4 %WarningEliminations% -Od -Oi -Zi -DDEBUG -DCRASH_DEREF -Fe"bin\Syntics"
set Libraries=vulkan-1.lib user32.lib Winmm.lib stb_image.lib
set Files=..\Syntics\src\syntics.c 
REM set Files=..\Syntics\src\syntics.c ..\Syntics\vendor\stb\stb_image.c
set IncludeDirs=-I..\Syntics\headers -I..\Syntics\src -I..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include 
REM set Files=..\Definition_Generator\src\main.c
REM set IncludeDirs=-I..\Definition_Generator\src
set LibraryDirs="C:\VulkanSDK\1.3.236.0\Lib"

IF NOT EXIST build (mkdir build) 
IF NOT EXIST build\stb_image.lib (call .\Commands\buildstb.bat)

pushd build
IF NOT EXIST bin mkdir bin

REM tcc %IncludeDirs% -g -DDEBUG -DCRASH_DEREF -mwindows %Files% -L..\ -LC:\VulkanSDK\1.3.236.0\Lib -lvulkan-1 -luser32 -lwinmm -o bin\Syntics.exe 

cl %CompilerFlags% %IncludeDirs% %Files% /link /SUBSYSTEM:windows /LIBPATH:%LibraryDirs% %Libraries% 

popd

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
