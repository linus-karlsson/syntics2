#! /bin/sh

cmake -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug -DINCLUDE_TEST=ON -B build
