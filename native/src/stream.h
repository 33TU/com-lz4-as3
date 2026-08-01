#ifndef COMLZ4_STREAM_H
#define COMLZ4_STREAM_H

#include <stddef.h>

#define COMLZ4_HISTORY_CAPACITY 65536

/* Retains the trailing COMLZ4_HISTORY_CAPACITY bytes of the block sequence.
   Exposed so the smoke tests can assert the retained bytes directly; a round
   trip cannot, because the encoder and the decoder share this function and any
   layout bug therefore applies to both sides and cancels out. */
void comlz4_history_append(
    char history[COMLZ4_HISTORY_CAPACITY],
    int *history_size,
    const char *data,
    size_t data_size);

#endif
