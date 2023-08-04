#! /bin/sh

BuildDirectory="build"
 
if [ ! -d "$BuildDirectory" ]; then
    mkdir -p "$BuildDirectory"
fi

clang -c -o build/stb_image.o ./Syntics/vendor/stb/stb_image.c \
    && llvm-ar rc ./build/stb_image.a ./build/stb_image.o

