#! /bin/sh

./Commands/test.sh && ./Commands/build.sh && cd build && ninja test && cd ..

