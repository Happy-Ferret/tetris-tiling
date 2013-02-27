#include "common.h"

#include <stdlib.h>

static inline board_t
add_piece_to_board(bpiece_t piece, board_t board, unsigned int pos)
{
    if (pos < PIECE_VERTICAL_OFFSET)
        return board | piece >> (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        return board | piece << (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;
}

static inline board_t
check_piece_placement(bpiece_t piece, board_t board, unsigned int pos)
{
    board_t shifted_board;
    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board = board << (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        shifted_board = board >> (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;

    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board |= (board_t)0x3f
                << (PIECE_VERTICAL_OFFSET - pos - 1) * PIECE_WIDTH;

    if (pos > PIECE_VERTICAL_OFFSET)
        shifted_board |= (board_t)0x3f
                << (BOARD_HEIGHT + PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    
    return shifted_board & piece;
}

