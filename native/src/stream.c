#include <AS3/AS3.h>
#include <limits.h>
#include <lz4.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "scratch.h"

void comlz4_stream_encoder_create(void)
    __attribute__((used,
                   annotate("as3sig:public function createStreamEncoder():uint"),
                   annotate("as3package:com.lz4._native")));

void comlz4_stream_encoder_dispose(void)
    __attribute__((used,
                   annotate("as3sig:public function disposeStreamEncoder(handle:uint):void"),
                   annotate("as3package:com.lz4._native")));

void comlz4_stream_compress(void)
    __attribute__((used,
                   annotate("as3sig:public function compressStream(handle:uint, src:ByteArray, dest:ByteArray, acceleration:int = 1):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

void comlz4_stream_decoder_create(void)
    __attribute__((used,
                   annotate("as3sig:public function createStreamDecoder():uint"),
                   annotate("as3package:com.lz4._native")));

void comlz4_stream_decoder_dispose(void)
    __attribute__((used,
                   annotate("as3sig:public function disposeStreamDecoder(handle:uint):void"),
                   annotate("as3package:com.lz4._native")));

void comlz4_stream_decompress(void)
    __attribute__((used,
                   annotate("as3sig:public function decompressStream(handle:uint, src:ByteArray, dest:ByteArray, originalSize:uint):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

#define COMLZ4_HISTORY_CAPACITY 65536

typedef struct
{
    LZ4_stream_t *lz4;
    comlz4_scratch scratch;
    char history[COMLZ4_HISTORY_CAPACITY];
    int history_size;
} comlz4_stream_encoder;

typedef struct
{
    LZ4_streamDecode_t *lz4;
    comlz4_scratch scratch;
    char history[COMLZ4_HISTORY_CAPACITY];
    int history_size;
} comlz4_stream_decoder;

static void comlz4_history_append(
    char history[COMLZ4_HISTORY_CAPACITY],
    int *history_size,
    const char *data,
    size_t data_size)
{
    size_t retained;
    size_t discard;

    if (data_size >= COMLZ4_HISTORY_CAPACITY)
    {
        memcpy(
            history,
            data + data_size - COMLZ4_HISTORY_CAPACITY,
            COMLZ4_HISTORY_CAPACITY);
        *history_size = COMLZ4_HISTORY_CAPACITY;
        return;
    }

    retained = (size_t)*history_size;
    if (retained + data_size > COMLZ4_HISTORY_CAPACITY)
    {
        discard = retained + data_size - COMLZ4_HISTORY_CAPACITY;
        memmove(history, history + discard, retained - discard);
        retained -= discard;
    }

    memcpy(history + retained, data, data_size);
    *history_size = (int)(retained + data_size);
}

static void comlz4_stream_encoder_destroy(comlz4_stream_encoder *encoder)
{
    if (!encoder)
        return;
    LZ4_freeStream(encoder->lz4);
    comlz4_scratch_dispose(&encoder->scratch);
    free(encoder);
}

static void comlz4_stream_decoder_destroy(comlz4_stream_decoder *decoder)
{
    if (!decoder)
        return;
    LZ4_freeStreamDecode(decoder->lz4);
    comlz4_scratch_dispose(&decoder->scratch);
    free(decoder);
}

void comlz4_stream_encoder_create(void)
{
    comlz4_stream_encoder *encoder;

    encoder = (comlz4_stream_encoder *)calloc(1, sizeof(*encoder));
    if (!encoder)
    {
        inline_as3("throw new Error('Unable to allocate the stream encoder.');");
        return;
    }

    encoder->lz4 = LZ4_createStream();
    if (!encoder->lz4)
    {
        comlz4_stream_encoder_destroy(encoder);
        inline_as3("throw new Error('Unable to create the LZ4 stream encoder.');");
        return;
    }

    AS3_Return((uint32_t)(uintptr_t)encoder);
}

void comlz4_stream_encoder_dispose(void)
{
    comlz4_stream_encoder *encoder;

    inline_as3("%0 = handle;" : "=r"(encoder));
    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed stream encoder.');");
        return;
    }

    comlz4_stream_encoder_destroy(encoder);
}

void comlz4_stream_compress(void)
{
    uint32_t source_length;
    uint32_t source_position;
    uint32_t destination_position;
    int acceleration_value;
    int bound;
    int compressed_size;
    size_t required;
    char *buffer;
    char *output;
    comlz4_stream_encoder *encoder;

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "if (src === dest) throw new ArgumentError('src and dest must be different ByteArray instances.');"
        "%0 = handle;"
        "%1 = src.bytesAvailable;"
        "%2 = src.position;"
        "%3 = dest.position;" : "=r"(encoder),
        "=r"(source_length),
        "=r"(source_position),
        "=r"(destination_position));

    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed stream encoder.');");
        return;
    }
    if (source_length == 0)
    {
        inline_as3("throw new ArgumentError('src must contain readable data.');");
        return;
    }

    inline_as3("%0 = acceleration;" : "=r"(acceleration_value));
    if (acceleration_value < 1)
    {
        inline_as3("throw new ArgumentError('acceleration must be at least 1.');");
        return;
    }
    if (source_length > LZ4_MAX_INPUT_SIZE || source_length > INT_MAX)
    {
        inline_as3("throw new RangeError('src is too large for LZ4 stream compression.');");
        return;
    }

    bound = LZ4_compressBound((int)source_length);
    if (bound <= 0 || !comlz4_size_add(source_length, (size_t)bound, &required))
    {
        inline_as3("throw new RangeError('Required stream buffer is too large.');");
        return;
    }
    if ((uint32_t)bound > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    buffer = comlz4_scratch_reserve(&encoder->scratch, required);
    if (!buffer)
    {
        inline_as3("throw new Error('Unable to allocate the stream compression buffer.');");
        return;
    }
    output = buffer + source_length;

    LZ4_loadDict(encoder->lz4, encoder->history, encoder->history_size);
    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(buffer), "r"(source_length));

    compressed_size = LZ4_compress_fast_continue(
        encoder->lz4,
        buffer,
        output,
        (int)source_length,
        bound,
        acceleration_value);
    if (compressed_size <= 0)
    {
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new Error('LZ4 stream compression failed.');");
        return;
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));
    inline_as3("dest.position += %0;" : : "r"(compressed_size));

    comlz4_history_append(
        encoder->history,
        &encoder->history_size,
        buffer,
        source_length);
}

void comlz4_stream_decoder_create(void)
{
    comlz4_stream_decoder *decoder;

    decoder = (comlz4_stream_decoder *)calloc(1, sizeof(*decoder));
    if (!decoder)
    {
        inline_as3("throw new Error('Unable to allocate the stream decoder.');");
        return;
    }

    decoder->lz4 = LZ4_createStreamDecode();
    if (!decoder->lz4)
    {
        comlz4_stream_decoder_destroy(decoder);
        inline_as3("throw new Error('Unable to create the LZ4 stream decoder.');");
        return;
    }

    AS3_Return((uint32_t)(uintptr_t)decoder);
}

void comlz4_stream_decoder_dispose(void)
{
    comlz4_stream_decoder *decoder;

    inline_as3("%0 = handle;" : "=r"(decoder));
    if (!decoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed stream decoder.');");
        return;
    }

    comlz4_stream_decoder_destroy(decoder);
}

void comlz4_stream_decompress(void)
{
    uint32_t source_length;
    uint32_t source_position;
    uint32_t destination_position;
    uint32_t original_size;
    int decompressed_size;
    size_t required;
    char *buffer;
    char *output;
    comlz4_stream_decoder *decoder;

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "if (src === dest) throw new ArgumentError('src and dest must be different ByteArray instances.');"
        "%0 = handle;"
        "%1 = src.bytesAvailable;"
        "%2 = src.position;"
        "%3 = dest.position;" : "=r"(decoder),
        "=r"(source_length),
        "=r"(source_position),
        "=r"(destination_position));

    if (!decoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed stream decoder.');");
        return;
    }
    if (source_length == 0)
    {
        inline_as3("throw new ArgumentError('src must contain readable data.');");
        return;
    }

    inline_as3("%0 = originalSize;" : "=r"(original_size));
    if (original_size == 0)
    {
        inline_as3("throw new ArgumentError('originalSize must be greater than 0.');");
        return;
    }
    if (source_length > INT_MAX || original_size > INT_MAX)
    {
        inline_as3("throw new RangeError('Stream block size exceeds the LZ4 API limit.');");
        return;
    }
    if (original_size > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }
    if (!comlz4_size_add(source_length, original_size, &required))
    {
        inline_as3("throw new RangeError('Required stream buffer is too large.');");
        return;
    }

    buffer = comlz4_scratch_reserve(&decoder->scratch, required);
    if (!buffer)
    {
        inline_as3("throw new Error('Unable to allocate the stream decompression buffer.');");
        return;
    }
    output = buffer + source_length;

    if (!LZ4_setStreamDecode(decoder->lz4, decoder->history, decoder->history_size))
    {
        inline_as3("throw new Error('Unable to configure the LZ4 stream dictionary.');");
        return;
    }

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(buffer), "r"(source_length));

    decompressed_size = LZ4_decompress_safe_continue(
        decoder->lz4,
        buffer,
        output,
        (int)source_length,
        (int)original_size);
    if (decompressed_size != (int)original_size)
    {
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new Error('LZ4 stream decompression failed or originalSize is incorrect.');");
        return;
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(decompressed_size));
    inline_as3("dest.position += %0;" : : "r"(decompressed_size));

    comlz4_history_append(
        decoder->history,
        &decoder->history_size,
        output,
        original_size);
}
