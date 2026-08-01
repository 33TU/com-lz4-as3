#include "scratch.h"

#include <stdint.h>
#include <stdlib.h>

int comlz4_size_add(size_t left, size_t right, size_t *result)
{
    if (left > SIZE_MAX - right)
        return 0;

    *result = left + right;
    return 1;
}

char *comlz4_scratch_reserve(comlz4_scratch *scratch, size_t required)
{
    size_t capacity;
    char *resized;

    if (required <= scratch->capacity)
        return scratch->data;

    capacity = scratch->capacity ? scratch->capacity : 4096;
    while (capacity < required)
    {
        if (capacity > SIZE_MAX / 2)
        {
            capacity = required;
            break;
        }
        capacity *= 2;
    }

    resized = (char *)realloc(scratch->data, capacity);
    if (!resized)
        return NULL;

    scratch->data = resized;
    scratch->capacity = capacity;
    return resized;
}

void comlz4_scratch_dispose(comlz4_scratch *scratch)
{
    free(scratch->data);
    scratch->data = NULL;
    scratch->capacity = 0;
}
