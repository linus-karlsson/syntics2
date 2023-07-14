@echo off

set WarningEliminations= -wd4100 -wd4201 
set CompilerFlags= -WL -nologo -W4 %WarningEliminations% -Od -Oi -Zi -DDEBUG -DCRASH_DEREF -Fe"bin\Syntics"
set LinkerFlags= vulkan-1.lib user32.lib Winmm.lib stb_image.lib
REM set Files=..\Syntics\src\syntics.c
REM set IncludeDirs=-I..\Syntics\headers -I..\Syntics\src -I..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include 
set Files=..\Definition_Generator\src\main.c
set IncludeDirs=-I..\Definition_Generator\src
set LibraryDirs="C:\VulkanSDK\1.3.236.0\Lib"

IF NOT EXIST build (mkdir build) 
IF NOT EXIST build\stb_image.lib (call .\Commands\buildstb.bat)

pushd build
IF NOT EXIST bin mkdir bin

cl %CompilerFlags% %IncludeDirs% %Files% /link /LIBPATH:%LibraryDirs% %LinkerFlags%

popd

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
