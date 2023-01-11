@echo off

del ./build/CMakeFiles/Syntics.dir/Syntics/src/*.obj
del ./build/CMakeFiles/Syntics.dir/Syntics/src/math/*.obj
cmake --build build
