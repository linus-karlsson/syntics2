@echo off

set CommonCompilerFlags= -nologo -EHa- -W4 -wd4100 -wd4201 -wd4505 -Od -Oi -Zi -DDEBUG
set CommonLinkerFlags= vulkan-1.lib user32.lib Winmm.lib pdh.lib

IF NOT EXIST build mkdir build
pushd build

cl %CommonCompilerFlags% -IC:..\Syntics\headers -IC:..\Syntics\src -IC:..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include -I"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\include" ..\Syntics\src\syntics.c ..\Syntics\vendor\stb\stb_image.c /DWIN32 /D_WINDOWS /link /LIBPATH:"C:\VulkanSDK\1.3.236.0\Lib" %CommonLinkerFlags%

lib stb_image.obj
popd

