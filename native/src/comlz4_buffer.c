#include <stdlib.h>
#include <stdint.h>
#include "comlz4.h"

static char *scratch_buf = NULL;
static size_t scratch_cap = 0;

static size_t next_pow2(size_t x)
{
    if (x <= 1)
        return 1;

    x--;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

#if SIZE_MAX > UINT32_MAX
    x |= x >> 32;
#endif

    return x + 1;
}

char *buffer_reserve(size_t size)
{
    if (size > scratch_cap)
    {
        size_t new_cap = next_pow2(size);

        char *tmp = realloc(scratch_buf, new_cap);
        if (!tmp)
        {
            return NULL;
        }

        scratch_buf = tmp;
        scratch_cap = new_cap;
    }

    return scratch_buf;
}

size_t buffer_capacity(void)
{
    return scratch_cap;
}

void buffer_release(void)
{
    if (scratch_buf)
    {
        free(scratch_buf);
        scratch_buf = NULL;
    }
    scratch_cap = 0;
}