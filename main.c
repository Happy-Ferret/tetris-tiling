#include "common.h"
#include "crossings.h"
#include "adjacency.h"
#include "crossing_list.h"

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    struct crossing_list crossing_list;
    struct adjacency_list *matrix;
    unsigned long nonzeroes, empty_rows;
    size_t i;

    fprintf(stderr, "Computing crossings with %d pieces.\n", piece_data_count);

    crossing_list_init(&crossing_list);
    find_all_crossings(&crossing_list);

    fprintf(stderr, "Crossings: %lu\n", crossing_list.count);

    matrix = build_adjacency_matrix(&crossing_list, 1);
    nonzeroes = 0;
    empty_rows = 0;
    for (i = 0; i < crossing_list.count; ++i) {
        nonzeroes += matrix[i].count;
        if (matrix[i].count == 0)
            ++empty_rows;
    }
    
    fprintf(stderr,
            "Matrix has %lu nonzero entries and %lu zero rows\n",
            nonzeroes, empty_rows);

    return 0;
}

