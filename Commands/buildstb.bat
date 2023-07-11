@echo off

set StandardCompilerFlags= -c -EHsc -nologo -Fo:stb_image.obj

IF NOT EXIST build mkdir build
pushd build

cl %StandardCompilerFlags% ..\Syntics\vendor\stb\stb_image.c 

lib -nologo stb_image.obj
del stb_image.obj
popd

