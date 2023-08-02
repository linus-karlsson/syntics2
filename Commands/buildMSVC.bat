@echo off

set WarningEliminations=-wd4100 -wd4201 -wd4189 -wd4101
set CompilerFlags=-WL -nologo -Gm- -WX -W4 %WarningEliminations% -Od -Oi -Z7 -DDEBUG -DCRASH_DEREF -Fe"build/bin/Syntics" -Fo"build/"
set Libraries=vulkan-1.lib user32.lib Winmm.lib stb_image.lib
set Files=./Syntics/src/syntics.c 
set IncludeDirs=-I./Syntics/headers -I./Syntics/src -I./Syntics/vendor -IC:/VulkanSDK/1.3.236.0/Include 
set LibraryDirs=/LIBPATH:"C:/VulkanSDK/1.3.236.0/Lib" /LIBPATH:./build

IF NOT EXIST build/bin mkdir build\bin
IF NOT EXIST build/stb_image.lib (call ./Commands/buildstb.bat)

cl %CompilerFlags% %IncludeDirs% %Files% /link /SUBSYSTEM:windows %LibraryDirs% %Libraries%

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
