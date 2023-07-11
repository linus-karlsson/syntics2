@echo off

set WarningEliminations= -wd4820 -wd4100 -wd4201 
set StandardCompilerFlags= -WL -nologo -Gm- -GR- -W4 %WarningEliminations% -Od -Oi -Zi -DDEBUG -DCRASH_DEREF -Fe"bin\Syntics"
set StandardLinkerFlags= vulkan-1.lib user32.lib Winmm.lib pdh.lib stb_image.lib

IF NOT EXIST build (mkdir build) 
IF NOT EXIST build\stb_image.lib (call .\Commands\buildstb.bat)

pushd build
IF NOT EXIST bin mkdir bin

cl %StandardCompilerFlags% -IC:..\Syntics\headers -IC:..\Syntics\src -IC:..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include -I"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\include" ..\Syntics\src\syntics.c /link /LIBPATH:"C:\VulkanSDK\1.3.236.0\Lib" %StandardLinkerFlags%

popd

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
