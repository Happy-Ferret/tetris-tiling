#include "common.h"

struct adjacency_list {
    unsigned int count;
    index_t *indices;
};

struct adjacency_list *
build_adjacency_matrix(const struct crossing_list *crossings,
        unsigned int threads);

