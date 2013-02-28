#include "common.h"

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    unsigned int i;
    piece_t *binary_pieces;
    struct crossing_list crossing_list;

    /* Load the pieces as binary */
    binary_pieces = malloc(piece_data_count * sizeof(*binary_pieces));
    for (i = 0; i < piece_data_count; ++i)
        binary_pieces[i] = tetris_piece_get_binary(&piece_data[i]);

    crossing_list_init(&crossing_list);
    compute_crossings(&crossing_list, 0, 0, binary_pieces, piece_data_count, 0);
    fprintf(stderr, "Crossings: %lu\n", crossing_list.count);

    for (i = 0; i < crossing_list.count; ++i) {
        print_crossing(stdout, crossing_list.crossings[i], binary_pieces);
        fprintf(stdout, "\n");
    }

    return 0;
}

