#include "common.h"
#include "crossing_list.h"
#include "crossings.h"

static inline void
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

static void
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

struct adjacency_list *
build_adjacency_matrix(const struct crossing_list *crossings,
        const struct piece_data *pieces, unsigned int piece_count,
        unsigned int threads)
{
}

