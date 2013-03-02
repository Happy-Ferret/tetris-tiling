#ifndef __TETRIS_CROSSINGS_H__
#define __TETRIS_CROSSINGS_H__

#include "common.h"
#include "pieces.h"
#include "crossing_list.h"

#define CROSSING_INVALID 0x3f

void print_crossing(FILE *file, crossing_t crossing, piece_t *pieces);
void find_all_crossings(struct crossing_list *crossing_list,
        const struct piece_data *pieces, unsigned int piece_count);

/* Note that crossings are storeed in reverse-order.  This way, they come out
 * sorted from the crossing-generation algorithm. */
static inline crossing_t
set_crossing_value(crossing_t crossing, unsigned int value, unsigned int pos)
{
    crossing &= ~((crossing_t)0x3f << (BOARD_HEIGHT - pos - 1) * 6);
    crossing |= (crossing_t)value << (BOARD_HEIGHT - pos - 1) * 6;
    return crossing;
}

static inline crossing_t
set_crossing_value_unsafe(crossing_t crossing, unsigned int value,
        unsigned int pos)
{
    return crossing | (crossing_t)value << (BOARD_HEIGHT - pos - 1) * 6;
}

static inline unsigned int
get_crossing_value(crossing_t crossing, unsigned int pos)
{
    return (crossing >> (BOARD_HEIGHT - pos - 1) * 6) & (crossing_t)0x3f;
}

#endif /* ! defined __TETRIS_CROSSINGS_H__ */

