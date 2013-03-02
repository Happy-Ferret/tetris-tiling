#ifndef __TETRIS_CROSSING_LIST_H__
#define __TETRIS_CROSSING_LIST_H__

#include "common.h"

#include <stdlib.h>
#include <string.h>

struct crossing_list {
    crossing_t *crossings;
    size_t count;
    size_t alloc;
};

static inline void
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

static inline void
crossing_list_append(struct crossing_list *list, crossing_t crossing)
{
    crossing_list_ensure_capacity(list, list->count + 1);
    list->crossings[list->count] = crossing;
    ++list->count;
}

static inline void
crossing_list_append_all(struct crossing_list *list,
        struct crossing_list *other)
{
    crossing_list_ensure_capacity(list, list->count + other->count);
    memcpy(list->crossings + list->count, other->crossings,
            other->count * sizeof(*other->crossings));
}

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

static inline void
crossing_list_destroy(struct crossing_list *list)
{
    free(list->crossings);
    crossing_list_init(list);
}

#endif /* ! defined __TETRIS_CROSSING_LIST_H__ */
