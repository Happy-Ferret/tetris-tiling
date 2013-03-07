#ifndef __TETRIS_CROSSING_LIST_H__
#define __TETRIS_CROSSING_LIST_H__

#include "common.h"

#include <stdlib.h>
#include <string.h>

/* Represents a list of crossings */
struct crossing_list {
    crossing_t *crossings;
    size_t count;
    size_t alloc;
};

/* Initialize a crossing list */
static inline void
crossing_list_init(struct crossing_list *list)
{
    list->crossings = malloc(1024 * sizeof(*list->crossings));
    list->count = 0;
    list->alloc = 1024;
}

/* Ensure that the list has room for least capacity crossings */
static inline void
crossing_list_ensure_capacity(struct crossing_list *list, size_t capacity)
{
    if (list->alloc < capacity) {
        list->alloc = list->alloc * 2 < capacity ? capacity : list->alloc * 2;
        list->crossings = realloc(list->crossings,
                list->alloc * sizeof(*list->crossings));
    }
}

/* Append a single crossing to the list */
static inline void
crossing_list_append(struct crossing_list *list, crossing_t crossing)
{
    crossing_list_ensure_capacity(list, list->count + 1);
    list->crossings[list->count] = crossing;
    ++list->count;
}

/* Find a crossing in a sorted crossing list.  If the crossing is found, its
 * index is returned; otherwise -1 is returned.
 *
 * Note: The list *must* be sorted for this algorithm to work. */
static inline ssize_t
crossing_list_find(const struct crossing_list *list, crossing_t crossing)
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

/* Destroy a crossing list */
static inline void
crossing_list_destroy(struct crossing_list *list)
{
    free(list->crossings);
    crossing_list_init(list);
}

#endif /* ! defined __TETRIS_CROSSING_LIST_H__ */
