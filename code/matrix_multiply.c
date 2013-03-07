#include "matrix_multiply.h"

#include <pthread.h>

#include "crossing_list.h"
#include "pieces.h"
#include "crossings.h"

#define THREAD_CHUNK_SIZE 1024

static void
matrix_multiply_row(mpz_t out_z, const mpz_t *in_vec, crossing_t crossing,
        const struct crossing_list *crossings, struct crossing_list *tmp_list,
        piece_t *binary_pieces)
{
    size_t i;
    ssize_t idx;

    /* Reset the list */
    tmp_list->count = 0;
    build_adjacent_crossings(tmp_list, crossing, binary_pieces);

    /* Start at 0 */
    mpz_set_ui(out_z, 0);

    for (i = 0; i < tmp_list->count; ++i) {
        idx = crossing_list_find(crossings, tmp_list->crossings[i]);

        if (idx >= 0)
            mpz_add(out_z, out_z, in_vec[idx]);
    }
}

struct matrix_multiply_data
{
    pthread_mutex_t lock;
    mpz_t *out_vec;
    const mpz_t *in_vec;
    const struct crossing_list *crossings;
    piece_t *binary_pieces;
    volatile size_t count;
};

static void *
matrix_multiply_thread(void *data_)
{
    size_t pos, end;
    struct matrix_multiply_data *data;
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

        for (; pos < end; ++pos) {
            matrix_multiply_row(data->out_vec[pos], data->in_vec,
                    data->crossings->crossings[pos], data->crossings,
                    &tmp_list, data->binary_pieces);
        }

        pthread_mutex_lock(&data->lock);
    }

    pthread_mutex_unlock(&data->lock);

    crossing_list_destroy(&tmp_list);

    return NULL;
}

void
adjacency_matrix_multiply(const struct crossing_list *crossings,
        const mpz_t *in_vec, mpz_t* out_vec)
{
    unsigned int i;
    struct matrix_multiply_data data;
    pthread_t *thread_list;

    pthread_mutex_init(&data.lock, NULL);
    data.out_vec = out_vec;
    data.in_vec = in_vec;
    data.crossings = crossings;

    data.binary_pieces = malloc(piece_data_count * sizeof(*data.binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        data.binary_pieces[i] = piece_data_get_piece(&piece_data[i]);

    data.count = 0;

    /* Start threads */
    if (NUM_THREADS <= 1) {
        matrix_multiply_thread(&data);
    } else {
        thread_list = malloc(NUM_THREADS * sizeof(*thread_list));

        for (i = 0; i < NUM_THREADS; ++i) {
            pthread_create(&thread_list[i], NULL,
                    &matrix_multiply_thread, &data);
        }

        for (i = 0; i < NUM_THREADS; ++i) {
            pthread_join(thread_list[i], NULL);
        }

        free(thread_list);
    }

    free(data.binary_pieces);
}

