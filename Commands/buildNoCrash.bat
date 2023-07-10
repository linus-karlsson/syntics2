@echo off

set StandardCompilerFlags= -diagnostics:column -WL -nologo -Gm- -GR- -EHa- -W4 -wd4100 -wd4201 -wd4505 -Od -Oi -Zi -DDEBUG
set StandardLinkerFlags= vulkan-1.lib user32.lib Winmm.lib pdh.lib stb_image.lib

IF NOT EXIST build mkdir build
pushd build

cl %StandardCompilerFlags% -IC:..\Syntics\headers -IC:..\Syntics\src -IC:..\Syntics\vendor -IC:\VulkanSDK\1.3.236.0\Include -I"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\include" ..\Syntics\src\syntics.c /DWIN32 /D_WINDOWS /link /LIBPATH:"C:\VulkanSDK\1.3.236.0\Lib" %StandardLinkerFlags%
popd

If NOT %errorlevel% neq 0 echo Build Completed
