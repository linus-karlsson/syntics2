#! /bin/sh

cmake -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DINCLUDE_TEST=OFF -B build
