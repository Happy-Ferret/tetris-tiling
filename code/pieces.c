#include "common.h"

#include <stdlib.h>
#include <stdio.h>

piece_t
piece_data_get_piece(const struct piece_data *piece)
{
    int r, c;
    piece_t p;

    p = 0;
    for (r = 0; r < PIECE_HEIGHT; ++r) {
        for (c = 0; c < PIECE_WIDTH; ++c) {
            if (piece->board[PIECE_HEIGHT - r - 1][c]) {
                p |= (piece_t)1u << (r * PIECE_WIDTH + c);
            }
        }
    }

    return p;
}

static inline void
print_board_helper(FILE *file, board_t board, unsigned int height)
{
    int r, c;
    char row_str[PIECE_WIDTH + 1];

    row_str[PIECE_WIDTH] = '\0';

    for (r = height - 1; r >= 0; --r) {
        for (c = 0; c < PIECE_WIDTH; ++c) {
            if (board & (board_t)1u << (r * PIECE_WIDTH + c))
                row_str[c] = 'X';
            else
                row_str[c] = '.';
        }

        fprintf(file, "%s\n", row_str);
    }
}

void
print_board(FILE *file, board_t board)
{
    print_board_helper(file, board, BOARD_HEIGHT);
}

void
print_piece(FILE *file, piece_t piece)
{
    print_board_helper(file, piece, PIECE_HEIGHT);
}

