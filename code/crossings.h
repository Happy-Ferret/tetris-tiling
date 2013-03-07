#ifndef __TETRIS_CROSSINGS_H__
#define __TETRIS_CROSSINGS_H__

#include "common.h"

#define CROSSING_INVALID 0x3f

void print_crossing(FILE *file, crossing_t crossing);

/* Find all of the possible crossings */
void find_all_crossings(struct crossing_list *crossing_list);

/* Find all possible crossings adjacent to the given crossing.  The
 * bianary_pieces array must be a binary version of piece_data */
void build_adjacent_crossings(struct crossing_list *adjacent_crossings,
        crossing_t crossing, piece_t *binary_pieces);

/* Set the value for the given position in crossing and return the new
 * crossing.
 *
 * Note that crossings are storeed in reverse-order.  This way, they come out
 * sorted from the crossing-generation algorithm. */
static inline crossing_t
set_crossing_value(crossing_t crossing, unsigned int value, unsigned int pos)
{
    crossing &= ~((crossing_t)0x3f << (BOARD_HEIGHT - pos - 1) * 6);
    crossing |= (crossing_t)value << (BOARD_HEIGHT - pos - 1) * 6;
    return crossing;
}

/* Get a particular crossing value */
static inline unsigned int
get_crossing_value(crossing_t crossing, unsigned int pos)
{
    return (crossing >> (BOARD_HEIGHT - pos - 1) * 6) & (crossing_t)0x3f;
}

#endif /* ! defined __TETRIS_CROSSINGS_H__ */

