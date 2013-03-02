#include "common.h"
#include "adjacency.h"
#include "pieces.h"
#include "crossing_list.h"
#include "crossings.h"

static inline void
build_adjacent_crossings_kernel(struct crossing_list *adjacent_crossings,
        crossing_t crossing, board_t board, piece_t *pieces, unsigned int pos)
{
    unsigned int i;
    board_t mask;

    if (pos == BOARD_HEIGHT) {
        /* Compute the between-crossings mask */
        mask = 0;
        for (i = 0; i < BOARD_HEIGHT; ++i)
            mask |= (board_t)1 << ((i * PIECE_WIDTH) + PIECE_WIDTH / 2 - 1);

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

static void
build_adjacent_crossings(struct crossing_list *adjacent_crossings,
        crossing_t crossing, piece_t *binary_pieces)
{
    unsigned int i, j, p;
    board_t board, left_edge;
    crossing_t future;

    left_edge = 0;
    future = 0;
    /* Compute a default crossing and a left board edge */
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        left_edge |= (board_t)1 << (i * PIECE_WIDTH);
        future = set_crossing_value_unsafe(future, CROSSING_INVALID, i);
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

    /* Reset the list */
    adjacent_crossings->count = 0;
    build_adjacent_crossings_kernel(adjacent_crossings, future, board,
            binary_pieces, 0);
}

static void
build_adjacency_list(struct adjacency_list *list,
        const struct crossing_list *all_crossings,
        const struct crossing_list *adjacent_crossings)
{
    unsigned int i;
    ssize_t idx;

    list->count = 0;
    list->indices = malloc(adjacent_crossings->count * sizeof(*list->indices));

    for (i = 0; i < adjacent_crossings->count; ++i) {
        idx = crossing_list_find(all_crossings,
                adjacent_crossings->crossings[i]);
        if (idx >= 0) {
            list->indices[i] = idx;
            ++list->count;
        }
    }

    list->indices = realloc(list->indices, list->count * sizeof(*list->indices));
}

struct adjacency_list *
build_adjacency_matrix(const struct crossing_list *crossings,
        unsigned int threads)
{
    unsigned int i, zeros;
    piece_t *binary_pieces;
    struct adjacency_list *matrix;
    struct crossing_list tmp_list;

    /* Load the pieces as binary */
    binary_pieces = malloc(piece_data_count * sizeof(*binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        binary_pieces[i] = piece_data_get_piece(&piece_data[i]);

    crossing_list_init(&tmp_list);
    matrix = malloc(crossings->count * sizeof(*matrix));

    zeros = 0;
    for (i = 0; i < crossings->count; ++i) {
        build_adjacent_crossings(&tmp_list, crossings->crossings[i],
                binary_pieces);
        build_adjacency_list(&matrix[i], crossings, &tmp_list);

        if (matrix[i].count == 0) {
            ++zeros;
            printf("Crossing %d has no neighbors:\n", i);
            print_crossing(stdout, crossings->crossings[i]);
        }
    }
    fprintf(stderr, "%d crossings have no neighbors\n", zeros);

    crossing_list_destroy(&tmp_list);

    return matrix;
}

