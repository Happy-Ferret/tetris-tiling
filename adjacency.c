#include "common.h"
#include "adjacency.h"
#include "pieces.h"
#include "crossing_list.h"
#include "crossings.h"

#include <pthread.h>

#define THREAD_CHUNK_SIZE 1024

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

struct adjacency_matrix_data
{
    pthread_mutex_t lock;
    struct adjacency_list *matrix;
    const struct crossing_list *crossings;
    piece_t *binary_pieces;
    volatile size_t count;
    volatile size_t zeros;
};

void *
build_adjacency_matrix_thread(void *data_)
{
    unsigned int zeros;
    size_t pos, end;
    struct adjacency_matrix_data *data;
    struct crossing_list tmp_list;

    data = data_;

    crossing_list_init(&tmp_list);

    pthread_mutex_lock(&data->lock);

    while(data->count < data->crossings->count) {
        pos = data->count;
        end = pos + THREAD_CHUNK_SIZE;
        if (end > data->crossings->count)
            end = data->crossings->count;
        data->count = end;

        pthread_mutex_unlock(&data->lock);

        zeros = 0;
        for (; pos < end; ++pos) {
            build_adjacent_crossings(&tmp_list,
                    data->crossings->crossings[pos], data->binary_pieces);
            build_adjacency_list(&data->matrix[pos],
                    data->crossings, &tmp_list);

            if (data->matrix[pos].count == 0)
                ++zeros;
        }

        pthread_mutex_lock(&data->lock);

        data->zeros += zeros;
    }

    pthread_mutex_unlock(&data->lock);

    crossing_list_destroy(&tmp_list);

    return NULL;
}

struct adjacency_list *
build_adjacency_matrix(const struct crossing_list *crossings,
        unsigned int threads)
{
    unsigned int i;
    struct adjacency_matrix_data data;
    pthread_t *thread_list;

    /* Initialize adjacency matrix data */
    pthread_mutex_init(&data.lock, NULL);
    data.matrix = malloc(crossings->count * sizeof(*data.matrix));
    data.crossings = crossings;

    data.binary_pieces = malloc(piece_data_count * sizeof(*data.binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        data.binary_pieces[i] = piece_data_get_piece(&piece_data[i]);

    data.count = 0;
    data.zeros = 0;

    /* Start threads */
    if (threads <= 1) {
        build_adjacency_matrix_thread(&data);
    } else {
        thread_list = malloc(threads * sizeof(*thread_list));

        for (i = 0; i < threads; ++i) {
            pthread_create(&thread_list[i], NULL,
                    &build_adjacency_matrix_thread, &data);
        }

        for (i = 0; i < threads; ++i) {
            pthread_join(thread_list[i], NULL);
        }
    }

    fprintf(stderr, "%d crossings have no neighbors\n", (int)data.zeros);

    return data.matrix;
}

