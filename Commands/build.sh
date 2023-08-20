#! /bin/sh

#./Commands/buildOperator.sh 
./Operator_Overload/bin/operator ./Syntics/src/game_internal.c ./Syntics/src/game.c
./Commands/buildClang.sh 
