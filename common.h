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
extern const unsigned int piece_data_count;

struct crossing_list {
    crossing_t *crossings;
    size_t count;
    size_t alloc;
};

void crossing_list_init(struct crossing_list *list);
void crossing_list_append(struct crossing_list *list, crossing_t crossing);
void crossing_list_append_all(struct crossing_list *list,
        struct crossing_list *other);
void crossing_list_destroy(struct crossing_list *list);

piece_t tetris_piece_get_binary(const struct piece_data *piece);
void print_board(FILE *file, board_t board);
void print_piece(FILE *file, piece_t piece);
void print_crossing(FILE *file, crossing_t crossing, piece_t *pieces);

board_t add_piece_to_board(piece_t piece, board_t board, unsigned int pos);
board_t check_piece_conflict(piece_t piece, board_t board, unsigned int pos);

void compute_crossings(struct crossing_list *crossing_list, crossing_t crossing,
        board_t board, piece_t *pieces, unsigned int piece_count,
        unsigned int pos);

void print_piece(FILE *file, piece_t piece);

#endif /* __TETRIS_COMMON_H__ */

