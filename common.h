#ifndef __TETRIS_COMMON_H__
#define __TETRIS_COMMON_H__

#include <stdint.h>
#include <stdio.h>

#define PIECE_WIDTH 6
#define PIECE_HEIGHT 6
#define BOARD_HEIGHT 8

#define PIECE_VERTICAL_OFFSET 2

typedef uint64_t bpiece_t;
typedef uint64_t board_t;

struct tetris_piece {
    char board[PIECE_HEIGHT][PIECE_WIDTH];
};

struct crossing {
    board_t board;
    uint8_t pieces[BOARD_HEIGHT];
};

extern const struct tetris_piece pieces[];
extern const unsigned int num_pieces;
extern bpiece_t *binary_pieces;

bpiece_t tetris_piece_get_binary(const struct tetris_piece *piece);
void print_board(FILE *file, board_t board);
void print_piece(FILE *file, bpiece_t piece);

board_t check_piece_placement(bpiece_t piece, board_t board, unsigned int pos);

#endif /* __TETRIS_COMMON_H__ */

