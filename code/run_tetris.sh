#!/bin/sh

BOARD_WIDTH=20
MAX_BOARD_HEIGHT=4

for i in $(seq ${MAX_BOARD_HEIGHT})
do
    make clean
    export TETRIS_DEFINES="BOARD_WIDTH=$BOARD_WIDTH BOARD_HEIGHT=$i"
    make -j4
    ./tetris
done

