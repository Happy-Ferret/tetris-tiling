#ifndef __TETRIS_MATRIX_MULTIPLY_H__
#define __TETRIS_MATRIX_MULTIPLY_H__

#include "common.h"

#include <gmp.h>

/* Multiply in_vec by the adjacency matrix and store the value in out_vec */
void adjacency_matrix_multiply(const struct crossing_list *crossings,
        const mpz_t *in_vec, mpz_t* out_vec);

#endif /* ! defined __TETRIS_MATRIX_MULTIPLY_H__ */

