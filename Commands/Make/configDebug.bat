@echo off

cmake -S . 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug -B build
