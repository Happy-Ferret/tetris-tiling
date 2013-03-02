#include "common.h"

struct adjacency_list {
    unsigned int count;
    index_t *indices;
};

struct adjacency_list *
build_adjacency_matrix(const struct crossing_list *crossings,
        const struct piece_data *pieces, unsigned int piece_count,
        unsigned int threads);

