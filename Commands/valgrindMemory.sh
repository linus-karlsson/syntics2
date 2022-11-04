#! /bin/sh

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-Syntics.txt  ./build/bin/Syntics
