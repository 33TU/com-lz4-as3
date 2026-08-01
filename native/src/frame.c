#include <AS3/AS3.h>
#include <lz4frame.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "scratch.h"

void comlz4_frame_encoder_create(void)
    __attribute__((used,
                   annotate("as3sig:public function createFrameEncoder(compressionLevel:int = 0):uint"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_encoder_dispose(void)
    __attribute__((used,
                   annotate("as3sig:public function disposeFrameEncoder(handle:uint):void"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_begin(void)
    __attribute__((used,
                   annotate("as3sig:public function beginFrame(handle:uint, dest:ByteArray):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_update(void)
    __attribute__((used,
                   annotate("as3sig:public function updateFrame(handle:uint, src:ByteArray, dest:ByteArray):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_end(void)
    __attribute__((used,
                   annotate("as3sig:public function endFrame(handle:uint, dest:ByteArray):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_decoder_create(void)
    __attribute__((used,
                   annotate("as3sig:public function createFrameDecoder():uint"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_decoder_dispose(void)
    __attribute__((used,
                   annotate("as3sig:public function disposeFrameDecoder(handle:uint):void"),
                   annotate("as3package:com.lz4._native")));

void comlz4_frame_decompress(void)
    __attribute__((used,
                   annotate("as3sig:public function decompressFrame(handle:uint, src:ByteArray, dest:ByteArray):Boolean"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

typedef enum
{
    COMLZ4_FRAME_READY = 0,
    COMLZ4_FRAME_ACTIVE = 1,
    COMLZ4_FRAME_FAILED = 2
} comlz4_frame_state;

typedef struct
{
    LZ4F_cctx *lz4;
    LZ4F_preferences_t preferences;
    comlz4_scratch scratch;
    comlz4_frame_state state;
} comlz4_frame_encoder;

typedef struct
{
    LZ4F_dctx *lz4;
    comlz4_scratch input;
    comlz4_scratch output;
} comlz4_frame_decoder;

#define COMLZ4_FRAME_OUTPUT_CHUNK 65536

static void comlz4_frame_encoder_destroy(comlz4_frame_encoder *encoder)
{
    if (!encoder)
        return;
    if (encoder->lz4)
        LZ4F_freeCompressionContext(encoder->lz4);
    comlz4_scratch_dispose(&encoder->scratch);
    free(encoder);
}

static void comlz4_frame_decoder_destroy(comlz4_frame_decoder *decoder)
{
    if (!decoder)
        return;
    if (decoder->lz4)
        LZ4F_freeDecompressionContext(decoder->lz4);
    comlz4_scratch_dispose(&decoder->input);
    comlz4_scratch_dispose(&decoder->output);
    free(decoder);
}

void comlz4_frame_encoder_create(void)
{
    int compression_level;
    size_t result;
    comlz4_frame_encoder *encoder;

    inline_as3("%0 = compressionLevel;" : "=r"(compression_level));

    encoder = (comlz4_frame_encoder *)calloc(1, sizeof(*encoder));
    if (!encoder)
    {
        inline_as3("throw new Error('Unable to allocate the frame encoder.');");
        return;
    }

    result = LZ4F_createCompressionContext(&encoder->lz4, LZ4F_VERSION);
    if (LZ4F_isError(result))
    {
        comlz4_frame_encoder_destroy(encoder);
        inline_as3("throw new Error('Unable to create the LZ4 frame encoder.');");
        return;
    }

    memset(&encoder->preferences, 0, sizeof(encoder->preferences));
    encoder->preferences.compressionLevel = compression_level;
    encoder->preferences.autoFlush = 1;
    encoder->preferences.frameInfo.blockMode = LZ4F_blockIndependent;
    encoder->state = COMLZ4_FRAME_READY;

    AS3_Return((uint32_t)(uintptr_t)encoder);
}

void comlz4_frame_encoder_dispose(void)
{
    comlz4_frame_encoder *encoder;

    inline_as3("%0 = handle;" : "=r"(encoder));
    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame encoder.');");
        return;
    }

    comlz4_frame_encoder_destroy(encoder);
}

void comlz4_frame_begin(void)
{
    uint32_t destination_position;
    size_t produced;
    char *output;
    comlz4_frame_encoder *encoder;

    inline_as3(
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = handle;"
        "%1 = dest.position;" : "=r"(encoder),
        "=r"(destination_position));

    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame encoder.');");
        return;
    }
    if (encoder->state == COMLZ4_FRAME_ACTIVE)
    {
        inline_as3("throw new IllegalOperationError('The frame has already been started.');");
        return;
    }
    if (encoder->state == COMLZ4_FRAME_FAILED)
    {
        inline_as3("throw new IllegalOperationError('The frame encoder cannot be reused after an LZ4 error.');");
        return;
    }
    if (LZ4F_HEADER_SIZE_MAX > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    output = comlz4_scratch_reserve(&encoder->scratch, LZ4F_HEADER_SIZE_MAX);
    if (!output)
    {
        inline_as3("throw new Error('Unable to allocate the frame header buffer.');");
        return;
    }

    produced = LZ4F_compressBegin(
        encoder->lz4,
        output,
        LZ4F_HEADER_SIZE_MAX,
        &encoder->preferences);
    if (LZ4F_isError(produced))
    {
        encoder->state = COMLZ4_FRAME_FAILED;
        inline_as3("throw new Error('LZ4 frame initialization failed.');");
        return;
    }

    encoder->state = COMLZ4_FRAME_ACTIVE;
    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(produced));
    inline_as3("dest.position += %0;" : : "r"(produced));
}

void comlz4_frame_update(void)
{
    uint32_t source_length;
    uint32_t source_position;
    uint32_t destination_position;
    size_t bound;
    size_t required;
    size_t produced;
    char *buffer;
    char *output;
    comlz4_frame_encoder *encoder;

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = handle;"
        "%1 = src.bytesAvailable;"
        "%2 = src.position;"
        "%3 = dest.position;" : "=r"(encoder),
        "=r"(source_length),
        "=r"(source_position),
        "=r"(destination_position));

    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame encoder.');");
        return;
    }
    if (encoder->state != COMLZ4_FRAME_ACTIVE)
    {
        inline_as3("throw new IllegalOperationError('begin() must be called before update().');");
        return;
    }
    if (source_length == 0)
    {
        inline_as3("throw new ArgumentError('src must contain readable data.');");
        return;
    }

    bound = LZ4F_compressBound(source_length, &encoder->preferences);
    if (LZ4F_isError(bound) ||
        !comlz4_size_add(source_length, bound, &required))
    {
        inline_as3("throw new RangeError('Required frame buffer is too large.');");
        return;
    }
    if (bound > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    buffer = comlz4_scratch_reserve(&encoder->scratch, required);
    if (!buffer)
    {
        inline_as3("throw new Error('Unable to allocate the frame compression buffer.');");
        return;
    }
    output = buffer + source_length;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(buffer), "r"(source_length));

    produced = LZ4F_compressUpdate(
        encoder->lz4,
        output,
        bound,
        buffer,
        source_length,
        NULL);
    if (LZ4F_isError(produced))
    {
        encoder->state = COMLZ4_FRAME_FAILED;
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new Error('LZ4 frame compression failed.');");
        return;
    }

    if (produced > 0)
    {
        inline_as3("CModule.ram.position = %0;" : : "r"(output));
        inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(produced));
        inline_as3("dest.position += %0;" : : "r"(produced));
    }
}

void comlz4_frame_end(void)
{
    uint32_t destination_position;
    size_t bound;
    size_t produced;
    char *output;
    comlz4_frame_encoder *encoder;

    inline_as3(
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = handle;"
        "%1 = dest.position;" : "=r"(encoder),
        "=r"(destination_position));

    if (!encoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame encoder.');");
        return;
    }
    if (encoder->state != COMLZ4_FRAME_ACTIVE)
    {
        inline_as3("throw new IllegalOperationError('begin() must be called before end().');");
        return;
    }

    bound = LZ4F_compressBound(0, &encoder->preferences);
    if (LZ4F_isError(bound))
    {
        inline_as3("throw new Error('Unable to calculate the frame footer size.');");
        return;
    }
    if (bound > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    output = comlz4_scratch_reserve(&encoder->scratch, bound);
    if (!output)
    {
        inline_as3("throw new Error('Unable to allocate the frame footer buffer.');");
        return;
    }

    produced = LZ4F_compressEnd(encoder->lz4, output, bound, NULL);
    if (LZ4F_isError(produced))
    {
        encoder->state = COMLZ4_FRAME_FAILED;
        inline_as3("throw new Error('LZ4 frame finalization failed.');");
        return;
    }

    encoder->state = COMLZ4_FRAME_READY;
    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(produced));
    inline_as3("dest.position += %0;" : : "r"(produced));
}

void comlz4_frame_decoder_create(void)
{
    size_t result;
    comlz4_frame_decoder *decoder;

    decoder = (comlz4_frame_decoder *)calloc(1, sizeof(*decoder));
    if (!decoder)
    {
        inline_as3("throw new Error('Unable to allocate the frame decoder.');");
        return;
    }

    result = LZ4F_createDecompressionContext(&decoder->lz4, LZ4F_VERSION);
    if (LZ4F_isError(result))
    {
        comlz4_frame_decoder_destroy(decoder);
        inline_as3("throw new Error('Unable to create the LZ4 frame decoder.');");
        return;
    }

    AS3_Return((uint32_t)(uintptr_t)decoder);
}

void comlz4_frame_decoder_dispose(void)
{
    comlz4_frame_decoder *decoder;

    inline_as3("%0 = handle;" : "=r"(decoder));
    if (!decoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame decoder.');");
        return;
    }

    comlz4_frame_decoder_destroy(decoder);
}

void comlz4_frame_decompress(void)
{
    uint32_t source_length;
    uint32_t source_position;
    uint32_t destination_position;
    size_t consumed_total = 0;
    size_t output_total = 0;
    size_t remaining;
    int frame_complete = 0;
    char *input;
    comlz4_frame_decoder *decoder;

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = handle;"
        "%1 = src.bytesAvailable;"
        "%2 = src.position;"
        "%3 = dest.position;" : "=r"(decoder),
        "=r"(source_length),
        "=r"(source_position),
        "=r"(destination_position));

    if (!decoder)
    {
        inline_as3("throw new ArgumentError('Invalid or disposed frame decoder.');");
        return;
    }
    if (source_length == 0)
        AS3_Return(0);

    input = comlz4_scratch_reserve(&decoder->input, source_length);
    if (!input)
    {
        inline_as3("throw new Error('Unable to allocate the frame input buffer.');");
        return;
    }

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(input), "r"(source_length));
    remaining = source_length;

    while (1)
    {
        size_t required;
        size_t consumed = remaining;
        size_t produced = COMLZ4_FRAME_OUTPUT_CHUNK;
        size_t hint;
        char *output;

        if (output_total > UINT32_MAX - COMLZ4_FRAME_OUTPUT_CHUNK ||
            !comlz4_size_add(output_total, COMLZ4_FRAME_OUTPUT_CHUNK, &required))
        {
            LZ4F_resetDecompressionContext(decoder->lz4);
            inline_as3("src.position = %0;" : : "r"(source_position));
            inline_as3("throw new RangeError('Decompressed frame output is too large.');");
            return;
        }

        output = comlz4_scratch_reserve(&decoder->output, required);
        if (!output)
        {
            LZ4F_resetDecompressionContext(decoder->lz4);
            inline_as3("src.position = %0;" : : "r"(source_position));
            inline_as3("throw new Error('Unable to allocate the frame output buffer.');");
            return;
        }

        hint = LZ4F_decompress(
            decoder->lz4,
            output + output_total,
            &produced,
            input + consumed_total,
            &consumed,
            NULL);
        if (LZ4F_isError(hint))
        {
            LZ4F_resetDecompressionContext(decoder->lz4);
            inline_as3("src.position = %0;" : : "r"(source_position));
            inline_as3("throw new Error('Invalid or corrupt LZ4 frame data.');");
            return;
        }

        consumed_total += consumed;
        remaining -= consumed;
        output_total += produced;

        if (hint == 0)
        {
            frame_complete = 1;
            break;
        }
        if (consumed == 0 && produced == 0)
            break;
        if (remaining == 0 && produced < COMLZ4_FRAME_OUTPUT_CHUNK)
            break;
    }

    if (output_total > UINT32_MAX - destination_position)
    {
        LZ4F_resetDecompressionContext(decoder->lz4);
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    inline_as3("src.position = %0 + %1;" : : "r"(source_position), "r"(consumed_total));
    if (output_total > 0)
    {
        inline_as3("CModule.ram.position = %0;" : : "r"(decoder->output.data));
        inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(output_total));
        inline_as3("dest.position += %0;" : : "r"(output_total));
    }

    AS3_Return(frame_complete);
}
