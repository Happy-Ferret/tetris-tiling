#ifndef __TETRIS_MATRIX_MULTIPLY_H__
#define __TETRIS_MATRIX_MULTIPLY_H__

#include "common.h"

#include <gmp.h>

void adjacency_matrix_multiply(const struct crossing_list *crossings,
        const mpz_t *in_vec, mpz_t* out_vec);

#endif /* ! defined __TETRIS_MATRIX_MULTIPLY_H__ */

