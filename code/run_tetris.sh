#!/bin/sh

BOARD_WIDTH=20
MAX_BOARD_HEIGHT=10
NUM_THREADS=20

for i in $(seq ${MAX_BOARD_HEIGHT})
do
    make -s clean
    export TETRIS_DEFINES="NUM_THREADS=$NUM_THREADS BOARD_WIDTH=$BOARD_WIDTH BOARD_HEIGHT=$i"
    make -s -j4
    ./tetris
    echo
done

