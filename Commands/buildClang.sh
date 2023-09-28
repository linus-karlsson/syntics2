#! /bin/sh

CompilerFlags="-Wno-null-dereference -g3 -O0 -DDEBUG -DCRASH_DEREF -DLINUX"
Libraries="-lvulkan -lxcb -lxcb-xfixes -lX11 -lX11-xcb -lxcb-cursor -lm"
Files="./Syntics/src/syntics.c"
IncludeDirs="-I./Syntics/headers -I./Syntics/src -I./Syntics/vendor"
LibraryDirs=""

BuildDirectory="build/bin"
 
if [ ! -d "$BuildDirectory" ]; then
    mkdir -p "$BuildDirectory"
fi

if [ ! -f "build/stb_image.o" ]; then
    clang $CompilerFlags $IncludeDirs -c ./Syntics/vendor/stb/stb_image.c -o ./build/stb_image.o
fi

if [ ! -f "build/stb_truetype.o" ]; then
    clang $CompilerFlags $IncludeDirs -c ./Syntics/vendor/stb/stb_truetype.c -o ./build/stb_truetype.o
fi

echo Clang compile $Files
clang $CompilerFlags $IncludeDirs -c ./Syntics/src/syntics.c -o ./build/syntics.o
clang $LibraryDirs $Libraries ./build/syntics.o ./build/stb_image.o ./build/stb_truetype.o -o ./build/bin/Syntics

#clang $CompilerFlags $IncludeDirs ./Syntics/src/syntics.c $LibraryDirs $Libraries ./build/stb_image.o -o ./build/bin/syntics

if [ $? -eq 0 ]
then
    echo Build Completed Successfully
else
    echo ERROR Build Stopped
fi

#rm ./build/CMakeFiles/Syntics.dir/Syntics/src/*.o
#cmake --build build
