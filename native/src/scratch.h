#ifndef COMLZ4_SCRATCH_H
#define COMLZ4_SCRATCH_H

#include <stddef.h>

typedef struct
{
    char *data;
    size_t capacity;
} comlz4_scratch;

int comlz4_size_add(size_t left, size_t right, size_t *result);
char *comlz4_scratch_reserve(comlz4_scratch *scratch, size_t required);
void comlz4_scratch_dispose(comlz4_scratch *scratch);

#endif
