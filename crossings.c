#include "common.h"

#include <stdlib.h>
#include <string.h>

inline board_t
add_piece_to_board(piece_t piece, board_t board, unsigned int pos)
{
    if (pos < PIECE_VERTICAL_OFFSET)
        return board | piece >> (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        return board | piece << (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;
}

inline board_t
check_piece_conflict(piece_t piece, board_t board, unsigned int pos)
{
    board_t shifted_board;
    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board = board << (PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    else
        shifted_board = board >> (pos - PIECE_VERTICAL_OFFSET) * PIECE_WIDTH;

    if (pos < PIECE_VERTICAL_OFFSET)
        shifted_board |= (board_t)0x3f
                << (PIECE_VERTICAL_OFFSET - pos - 1) * PIECE_WIDTH;

    if (pos > PIECE_VERTICAL_OFFSET)
        shifted_board |= (board_t)0x3f
                << (BOARD_HEIGHT + PIECE_VERTICAL_OFFSET - pos) * PIECE_WIDTH;
    
    return shifted_board & piece;
}

inline void
crossing_list_init(struct crossing_list *list)
{
    list->crossings = malloc(1024 * sizeof(*list->crossings));
    list->count = 0;
    list->alloc = 1024;
}

static inline void
crossing_list_ensure_capacity(struct crossing_list *list, size_t capacity)
{
    if (list->alloc < capacity) {
        list->alloc = list->alloc * 2 < capacity ? capacity : list->alloc * 2;
        list->crossings = realloc(list->crossings,
                list->alloc * sizeof(*list->crossings));
    }
}

inline void
crossing_list_append(struct crossing_list *list, crossing_t crossing)
{
    crossing_list_ensure_capacity(list, list->count + 1);
    list->crossings[list->count] = crossing;
    ++list->count;
}

inline void
crossing_list_append_all(struct crossing_list *list,
        struct crossing_list *other)
{
    crossing_list_ensure_capacity(list, list->count + other->count);
    memcpy(list->crossings + list->count, other->crossings,
            other->count * sizeof(*other->crossings));
}

inline ssize_t
crossing_list_find(struct crossing_list *list, crossing_t crossing)
{
    crossing_t *start, *end, *middle;

    start = list->crossings;
    end = list->crossings + list->count;

    while (start < end) {
        middle = start + (end - start) / 2;
        if (crossing > *middle)
            start = middle + 1;
        else if (crossing < *middle)
            end = middle;
        else
            return middle - list->crossings;
    }
    return -1;
}

inline void
crossing_list_destroy(struct crossing_list *list)
{
    free(list->crossings);
    crossing_list_init(list);
}

/* Note that crossings are storeed in reverse-order.  This way, they come out
 * sorted from the crossing-generation algorithm. */
inline crossing_t
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

inline unsigned int
get_crossing_value(crossing_t crossing, unsigned int pos)
{
    return (crossing >> (BOARD_HEIGHT - pos - 1) * 6) & (crossing_t)0x3f;
}

void
print_crossing(FILE *file, crossing_t crossing, piece_t *pieces)
{
    int r, c, pos, parity;
    piece_t piece;
    char board[BOARD_HEIGHT][PIECE_WIDTH + 1];

    for (r = 0; r < BOARD_HEIGHT; ++r) {
        for (c = 0; c < PIECE_WIDTH; ++c)
            board[r][c] = '.';

        board[r][PIECE_WIDTH] = '\0';
    }

    parity = 0;
    for (pos = 0; pos < BOARD_HEIGHT; ++pos) {
        if (get_crossing_value(crossing, pos) == CROSSING_INVALID)
            continue;

        piece = pieces[get_crossing_value(crossing, pos)];
        for (r = 0; r < PIECE_HEIGHT; ++r) {
            for (c = 0; c < PIECE_WIDTH; ++c) {
                if (r + pos - PIECE_VERTICAL_OFFSET >= 0
                        && r + pos - PIECE_VERTICAL_OFFSET < BOARD_HEIGHT
                        && piece & (1 << (r * PIECE_WIDTH + c))) {
                    board[r + pos - PIECE_VERTICAL_OFFSET][c] = 'A' + pos;

                    if (c < PIECE_WIDTH / 2)
                        ++parity;
                }
            }
        }
    }

    fprintf(file, "Parity: %d\n", parity);
    for (r = BOARD_HEIGHT - 1; r >= 0; --r) {
        fprintf(file, "%s  %02u\n", board[r], get_crossing_value(crossing, r));
    }
}

inline void
build_adjacent_crossings_kernel(struct crossing_list *clist,
        crossing_t crossing, board_t board, piece_t *pieces,
        unsigned int piece_count, unsigned int pos)
{
    unsigned int i;

    if (pos == BOARD_HEIGHT) {
        crossing_list_append(clist, crossing);
        return;
    }

    if (get_crossing_value(crossing, pos) != CROSSING_INVALID) {
        build_adjacent_crossings_kernel(clist, crossing,
                board, pieces, piece_count, pos + 1);
        return;
    }
        
    for (i = 0; i < piece_count; ++i) {
        if (! check_piece_conflict(pieces[i], board, pos)) {
            build_adjacent_crossings_kernel(clist,
                    set_crossing_value(crossing, i, pos),
                    add_piece_to_board(pieces[i], board, pos),
                    pieces, piece_count, pos + 1);
        }
    }
}

void
build_adjacent_crossings(struct crossing_list *clist, crossing_t crossing,
        piece_t *pieces, unsigned int piece_count)
{
    int i;
    board_t board, left_edge;

    left_edge = 0;
    for (i = 0; i < BOARD_HEIGHT; ++i)
        left_edge |= 1 << (i * PIECE_WIDTH);

    board = 0;
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        board = add_piece_to_board(
                pieces[get_crossing_value(crossing, i)], board, i);
    }
    board = (board & ~left_edge) >> 1;
}

inline void
find_all_crossings_kernel(struct crossing_list *clist,
        crossing_t crossing, board_t board, piece_t *pieces,
        unsigned int piece_count, uint64_t parity_data, unsigned int parity,
        unsigned int pos)
{
    unsigned int i;

    if (pos == BOARD_HEIGHT) {
        if (BOARD_HEIGHT % 4 == 0) {
            /* Divisible by 4 */
            if (parity % 4)
                return;
        } else if (BOARD_HEIGHT % 2 == 0) {
            /* Divisible by 2 */
            if (parity % 2)
                return;
        }

        crossing_list_append(clist, crossing);
        return;
    }
        
    for (i = 0; i < piece_count; ++i) {
        if (! check_piece_conflict(pieces[i], board, pos)) {
            find_all_crossings_kernel(clist,
                    set_crossing_value(crossing, i, pos),
                    add_piece_to_board(pieces[i], board, pos),
                    pieces, piece_count, parity_data,
                    parity + ((parity_data >> (i * 2) & 0x3)),
                    pos + 1);
        }
    }
}

void
find_all_crossings(struct crossing_list *clist, piece_t *pieces,
        unsigned int piece_count)
{
    uint64_t parity, parity_data;
    unsigned int i, r, c;

    parity_data = 0;
    for (i = 0; i < piece_count; ++i) {
        parity = 0;
        for (r = 0; r < PIECE_HEIGHT; ++r)
            for (c = 0; c < PIECE_WIDTH / 2; ++c)
                parity += (pieces[i] >> (r * PIECE_WIDTH + c)) & 0x1;

        parity_data |= (parity & 0x3) << (i * 2);
    }

    find_all_crossings_kernel(clist, 0, 0, pieces, piece_data_count,
            parity_data, 0, 0);
}

