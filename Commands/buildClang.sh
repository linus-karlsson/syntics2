#! /bin/sh

CompilerFlags="-m64 -Wno-null-dereference -g -DDEBUG -DCRASH_DEREF -DLINUX"
Libraries="-lvulkan -lxcb -lxcb-xfixes -lX11 -lX11-xcb -lxcb-cursor -l:stb_image.a -lm"
Files="./Syntics/src/syntics.c"
IncludeDirs="-I./Syntics/headers -I./Syntics/src -I./Syntics/vendor"
LibraryDirs="-L./build"

BuildDirectory="build/bin"
 
if [ ! -d "$BuildDirectory" ]; then
    mkdir -p "$BuildDirectory"
fi

if [ ! -f "build/stb_image.a" ]; then
    ./Commands/buildstb.sh
fi

echo Clang compile $Files
clang $IncludeDirs $CompilerFlags $Files $LibraryDirs $Libraries -o build\bin\Syntics
echo Finished
