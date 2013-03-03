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

    fprintf(stderr, "Computing crossings with %d pieces.\n", piece_data_count);

    crossing_list_init(&crossing_list);
    find_all_crossings(&crossing_list);

    fprintf(stderr, "Crossings: %lu\n", crossing_list.count);

    build_adjacency_matrix(&crossing_list, 1);

/*
    for (i = 0; i < crossing_list.count; ++i) {
        print_crossing(stdout, crossing_list.crossings[i], binary_pieces);
        fprintf(stdout, "\n");
        if (crossing_list_find(&crossing_list, crossing_list.crossings[i]) != i)
            fprintf(stderr, "Cannot find crossing %d\n", i);
    }
*/

    return 0;
}

