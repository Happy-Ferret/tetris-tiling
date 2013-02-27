#ifndef __TETRIS_COMMON_H__
#define __TETRIS_COMMON_H__

#include <stdint.h>
#include <stdio.h>

#define PIECE_WIDTH 6
#define PIECE_HEIGHT 6
#define BOARD_HEIGHT 10

#define PIECE_VERTICAL_OFFSET 2

typedef uint64_t piece_t;
typedef uint64_t board_t;
typedef uint64_t crossing_t;

struct piece_data {
    char board[PIECE_HEIGHT][PIECE_WIDTH];
};

extern const struct piece_data piece_data[];
extern const unsigned int num_pieces;
extern piece_t *binary_pieces;

piece_t tetris_piece_get_binary(const struct piece_data *piece);
void print_board(FILE *file, board_t board);
void print_piece(FILE *file, piece_t piece);

unsigned int compute_crossings(board_t board, unsigned int pos);

#endif /* __TETRIS_COMMON_H__ */

