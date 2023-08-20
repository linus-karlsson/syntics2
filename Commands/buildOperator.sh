#! /bin/sh

#CompilerFlags="-Wno-null-dereference -g3 -O0 -DDEBUG -DLINUX"
CompilerFlags="-Wno-null-dereference -O2 -DNDEBUG -DLINUX"
Libraries=""
Files="./Operator_Overload/src/main.c"
IncludeDirs="-I./Syntics/headers -I./Syntics/src"
LibraryDirs=""

BuildDirectory="build/bin"
 
if [ ! -d "$BuildDirectory" ]; then
    mkdir -p "$BuildDirectory"
fi

echo Clang compile $Files
clang $CompilerFlags $IncludeDirs -c $Files -o ./build/operator.o
clang $LibraryDirs $Libraries ./build/operator.o -o ./build/bin/operator

if [ $? -eq 0 ]
then
    echo Build Completed Successfully
else
    echo ERROR Build Stopped
fi

#rm ./build/CMakeFiles/Syntics.dir/Syntics/src/*.o
#cmake --build build
