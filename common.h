#ifndef __TETRIS_COMMON_H__
#define __TETRIS_COMMON_H__

#include <stdint.h>

#define PIECE_WIDTH 6
#define PIECE_HEIGHT 6

typedef uint64_t bpiece_t;
typedef uint64_t board_t;

struct tetris_piece {
    char board[PIECE_HEIGHT][PIECE_WIDTH];
};

extern const struct tetris_piece pieces[];
extern const unsigned int num_pieces;
extern bpiece_t *binary_pieces;

#endif /* __TETRIS_COMMON_H__ */

