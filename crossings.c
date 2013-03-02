#include "common.h"
#include "crossings.h"
#include "pieces.h"
#include "crossing_list.h"

#include <stdlib.h>
#include <string.h>

void
print_crossing(FILE *file, crossing_t crossing, piece_t *pieces)
{
    int r, c, pos, parity;
    piece_t piece;
    char board[BOARD_HEIGHT][PIECE_WIDTH + 1];

    for (r = 0; r < BOARD_HEIGHT; ++r) {
        for (c = 0; c < PIECE_WIDTH; ++c)
            board[r][c] = '.';

        board[r][PIECE_WIDTH] = '\0';
    }

    parity = 0;
    for (pos = 0; pos < BOARD_HEIGHT; ++pos) {
        if (get_crossing_value(crossing, pos) == CROSSING_INVALID)
            continue;

        piece = pieces[get_crossing_value(crossing, pos)];
        for (r = 0; r < PIECE_HEIGHT; ++r) {
            for (c = 0; c < PIECE_WIDTH; ++c) {
                if (r + pos - PIECE_VERTICAL_OFFSET >= 0
                        && r + pos - PIECE_VERTICAL_OFFSET < BOARD_HEIGHT
                        && piece & (1 << (r * PIECE_WIDTH + c))) {
                    board[r + pos - PIECE_VERTICAL_OFFSET][c] = 'A' + pos;

                    if (c < PIECE_WIDTH / 2)
                        ++parity;
                }
            }
        }
    }

    fprintf(file, "Parity: %d\n", parity);
    for (r = BOARD_HEIGHT - 1; r >= 0; --r) {
        fprintf(file, "%s  %02u\n", board[r], get_crossing_value(crossing, r));
    }
}

inline void
build_adjacent_crossings_kernel(struct crossing_list *adjacent_clist,
        crossing_t crossing, board_t board, piece_t *pieces,
        unsigned int piece_count, unsigned int pos)
{
    unsigned int i;

    if (pos == BOARD_HEIGHT) {
        crossing_list_append(adjacent_clist, crossing);
        return;
    }

    if (get_crossing_value(crossing, pos) != CROSSING_INVALID) {
        build_adjacent_crossings_kernel(adjacent_clist, crossing,
                board, pieces, piece_count, pos + 1);
        return;
    }
        
    for (i = 0; i < piece_count; ++i) {
        if (! check_piece_conflict(pieces[i], board, pos)) {
            build_adjacent_crossings_kernel(adjacent_clist,
                    set_crossing_value(crossing, i, pos),
                    add_piece_to_board(pieces[i], board, pos),
                    pieces, piece_count, pos + 1);
        }
    }
}

void
build_adjacent_crossings(struct crossing_list *clist, crossing_t crossing,
        piece_t *pieces, unsigned int piece_count)
{
    unsigned int i, p;
    board_t board, left_edge;
    crossing_t future;

    left_edge = 0;
    for (i = 0; i < BOARD_HEIGHT; ++i)
        left_edge |= 1 << (i * PIECE_WIDTH);

    board = 0;
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        p = get_crossing_value(crossing, i);
        board = add_piece_to_board(pieces[p], board, i);

    }
    board = (board & ~left_edge) >> 1;
}

inline void
find_all_crossings_kernel(struct crossing_list *clist,
        crossing_t crossing, board_t board, piece_t *pieces,
        unsigned int piece_count, uint64_t parity_data, unsigned int parity,
        unsigned int pos)
{
    unsigned int i;

    if (pos == BOARD_HEIGHT) {
        if (BOARD_HEIGHT % 4 == 0) {
            /* Divisible by 4 */
            if (parity % 4)
                return;
        } else if (BOARD_HEIGHT % 2 == 0) {
            /* Divisible by 2 */
            if (parity % 2)
                return;
        }

        crossing_list_append(clist, crossing);
        return;
    }
        
    for (i = 0; i < piece_count; ++i) {
        if (! check_piece_conflict(pieces[i], board, pos)) {
            find_all_crossings_kernel(clist,
                    set_crossing_value(crossing, i, pos),
                    add_piece_to_board(pieces[i], board, pos),
                    pieces, piece_count, parity_data,
                    parity + ((parity_data >> (i * 2) & 0x3)),
                    pos + 1);
        }
    }
}

void
find_all_crossings(struct crossing_list *clist,
        const struct piece_data *pieces, unsigned int piece_count)
{
    uint64_t parity, parity_data;
    piece_t *binary_pieces;
    unsigned int i, r, c;

    /* Load the pieces as binary */
    binary_pieces = malloc(piece_data_count * sizeof(*binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        binary_pieces[i] = piece_data_get_piece(&pieces[i]);

    /* Load the parity_data */
    parity_data = 0;
    for (i = 0; i < piece_count; ++i) {
        parity = 0;
        for (r = 0; r < PIECE_HEIGHT; ++r)
            for (c = 0; c < PIECE_WIDTH / 2; ++c)
                parity += (binary_pieces[i] >> (r * PIECE_WIDTH + c)) & 0x1;

        parity_data |= (parity & 0x3) << (i * 2);
    }

    find_all_crossings_kernel(clist, 0, 0, binary_pieces, piece_data_count,
            parity_data, 0, 0);

    free(binary_pieces);
}

