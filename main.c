#include "common.h"

#include <stdlib.h>

bpiece_t
tetris_piece_get_binary(const struct tetris_piece *piece)
{
    int r, c;
    bpiece_t p;

    p = 0;
    for (r = 0; r < PIECE_HEIGHT; ++r) {
        for (c = 0; c < PIECE_WIDTH; ++c) {
            if (piece->board[PIECE_HEIGHT - r][c]) {
                p |= 1u << (r * PIECE_WIDTH + c);
            }
        }
    }

    return p;
}

int
main(int argc, char **argv)
{
    int i;

    /* Load the pieces as binary */
    binary_pieces = malloc(num_pieces * sizeof(*binary_pieces));
    for (i = 0; i < num_pieces; ++i)
        binary_pieces[i] = tetris_piece_get_binary(&pieces[i]);

    return 0;
}

