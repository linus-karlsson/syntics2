@echo off

set WarningEliminations=-wd4100 -wd4201 -wd4189 -wd4101 -wd4127
set CompilerFlags=-WL -nologo -Gm- -WX -W4 %WarningEliminations% -Od -Oi -Z7 -DDEBUG -DCRASH_DEREF 
set OutputPath=-Fe"build/bin/Syntics" -Fo"build/"
set Libraries=vulkan-1.lib user32.lib Winmm.lib
set Files=./Syntics/src/syntics.c 
set IncludeDirs=-I./Syntics/headers -I./Syntics/src -I./Syntics/vendor -IC:/VulkanSDK/1.3.236.0/Include 
set LibraryDirs=/LIBPATH:"C:/VulkanSDK/1.3.236.0/Lib" /LIBPATH:./build

IF NOT EXIST build/bin mkdir build\bin
IF NOT EXIST build/stb_image.obj (cl %CompilerFlags% -c ./Syntics/vendor/stb/stb_image.c -Fo"build/")
IF NOT EXIST build/stb_truetype.obj (cl %CompilerFlags% -c ./Syntics/vendor/stb/stb_truetype.c -Fo"build/")
set StbLibs=./build/stb_image.obj ./build/stb_truetype.obj

cl %CompilerFlags% %OutputPath% %IncludeDirs% %Files% /link /SUBSYSTEM:windows %StbLibs% %LibraryDirs% %Libraries%

IF NOT %errorlevel% neq 0 (echo Build Completed Successfully) ELSE (echo ERROR Build Stopped)
set HHHHHHH=1
