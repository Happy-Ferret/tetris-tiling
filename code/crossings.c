#include "common.h"
#include "crossings.h"
#include "pieces.h"
#include "crossing_list.h"

#include <stdlib.h>
#include <string.h>

void
print_crossing(FILE *file, crossing_t crossing)
{
    int r, c, pos, parity, piece;
    char board[BOARD_HEIGHT][PIECE_WIDTH + 1];

    for (r = 0; r < BOARD_HEIGHT; ++r) {
        for (c = 0; c < PIECE_WIDTH; ++c)
            board[r][c] = '.';

        board[r][PIECE_WIDTH] = '\0';
    }

    parity = 0;
    for (pos = 0; pos < BOARD_HEIGHT; ++pos) {
        piece = get_crossing_value(crossing, pos);

        if (piece == CROSSING_INVALID)
            continue;

        for (r = 0; r < PIECE_HEIGHT; ++r) {
            for (c = 0; c < PIECE_WIDTH; ++c) {
                if (r + pos - PIECE_VERTICAL_OFFSET >= 0
                        && r + pos - PIECE_VERTICAL_OFFSET < BOARD_HEIGHT
                        && piece_data[piece].board[PIECE_HEIGHT - r - 1][c]) {
                    board[r + pos - PIECE_VERTICAL_OFFSET][c] = 'A' + pos;

                    if (c < PIECE_WIDTH / 2)
                        ++parity;
                }
            }
        }
    }

    fprintf(file, "Parity: %d\n", parity);
    for (r = BOARD_HEIGHT - 1; r >= 0; --r) {
        fprintf(file, "%s", board[r]);

        if (get_crossing_value(crossing, r) != CROSSING_INVALID)
            fprintf(file, "  %02d\n", get_crossing_value(crossing, r));
        else
            fprintf(file, "  --\n");
    }
}

static inline void
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
find_all_crossings(struct crossing_list *clist)
{
    uint64_t parity, parity_data;
    piece_t *binary_pieces;
    unsigned int i, r, c;

    /* Load the pieces as binary */
    binary_pieces = malloc(piece_data_count * sizeof(*binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        binary_pieces[i] = piece_data_get_piece(&piece_data[i]);

    /* Load the parity_data */
    parity_data = 0;
    for (i = 0; i < piece_data_count; ++i) {
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

static inline void
build_adjacent_crossings_kernel(struct crossing_list *adjacent_crossings,
        crossing_t crossing, board_t board, piece_t *pieces, unsigned int pos)
{
    unsigned int i;
    board_t mask;
    crossing_t vert_four;

    if (pos == BOARD_HEIGHT) {
        /* Compute the between-crossings mask */
        mask = 0;
        for (i = 0; i < BOARD_HEIGHT; ++i)
            mask |= (board_t)1 << ((i * PIECE_WIDTH) + PIECE_WIDTH / 2 - 1);

        /* Compute a vertical bar piece */
        vert_four = 0;
        for (i = 0; i < 4; ++i)
            vert_four |= (crossing_t)1 << ((i * PIECE_WIDTH) + PIECE_WIDTH / 2 - 1);

        /* The vertical bar piece is a special case. */
        for (i = 0; i < BOARD_HEIGHT; ++i)
            if (! check_piece_conflict(vert_four, board, i))
                board = add_piece_to_board(vert_four, board, i);

        if (~board & mask)
            return;

        crossing_list_append(adjacent_crossings, crossing);
        return;
    }

    if (get_crossing_value(crossing, pos) != CROSSING_INVALID) {
        build_adjacent_crossings_kernel(adjacent_crossings, crossing,
                board, pieces, pos + 1);
        return;
    }
        
    for (i = 0; i < piece_data_count; ++i) {
        if (! check_piece_conflict(pieces[i], board, pos)) {
            build_adjacent_crossings_kernel(adjacent_crossings,
                    set_crossing_value(crossing, i, pos),
                    add_piece_to_board(pieces[i], board, pos),
                    pieces, pos + 1);
        }
    }
}

void
build_adjacent_crossings(struct crossing_list *adjacent_crossings,
        crossing_t crossing, piece_t *binary_pieces)
{
    unsigned int i, j, p;
    board_t board, left_edge;
    crossing_t future;

    left_edge = 0;
    future = 0;
    /* Compute a default (all invalid) crossing and a left board edge */
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        left_edge |= (board_t)1 << (i * PIECE_WIDTH);
        future = set_crossing_value(future, CROSSING_INVALID, i);
    }

    /* Re-build the board and fill in the future crossing */
    board = 0;
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        p = get_crossing_value(crossing, i);
        board = add_piece_to_board(binary_pieces[p], board, i);

        for (j = 0; j < 5; ++j) {
            if (piece_data[p].future[j] >= 0) {
                future = set_crossing_value(future, piece_data[p].future[j],
                        i + j - 2);
            }
        }
    }
    board = (board & ~left_edge) >> 1;

    /* Fill in the left-most two columns */
    board |= left_edge << (PIECE_WIDTH / 2 - 2);

    build_adjacent_crossings_kernel(adjacent_crossings, future, board,
            binary_pieces, 0);
}

