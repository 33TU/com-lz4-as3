#include <AS3/AS3.h>
#include <limits.h>
#include <lz4.h>
#include <stdint.h>

#include "scratch.h"

void comlz4_block_compress(void)
    __attribute__((used,
                   annotate("as3sig:public function compressBlock(src:ByteArray, dest:ByteArray, acceleration:int = 1):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

void comlz4_block_decompress(void)
    __attribute__((used,
                   annotate("as3sig:public function decompressBlock(src:ByteArray, dest:ByteArray, originalSize:uint):void"),
                   annotate("as3import:flash.utils.ByteArray"),
                   annotate("as3package:com.lz4._native")));

static comlz4_scratch comlz4_block_scratch = {NULL, 0};

void comlz4_block_compress(void)
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

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = src.bytesAvailable;"
        "%1 = src.position;"
        "%2 = dest.position;" : "=r"(source_length),
        "=r"(source_position), "=r"(destination_position));

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
        inline_as3("throw new RangeError('src is too large for LZ4 block compression.');");
        return;
    }

    bound = LZ4_compressBound((int)source_length);
    if (bound <= 0 || !comlz4_size_add(source_length, (size_t)bound, &required))
    {
        inline_as3("throw new RangeError('Required compression buffer is too large.');");
        return;
    }
    if ((uint32_t)bound > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    buffer = comlz4_scratch_reserve(&comlz4_block_scratch, required);
    if (!buffer)
    {
        inline_as3("throw new Error('Unable to allocate the compression buffer.');");
        return;
    }
    output = buffer + source_length;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(buffer), "r"(source_length));

    compressed_size = LZ4_compress_fast(
        buffer,
        output,
        (int)source_length,
        bound,
        acceleration_value);

    if (compressed_size <= 0)
    {
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new Error('LZ4 block compression failed.');");
        return;
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));
    inline_as3("dest.position += %0;" : : "r"(compressed_size));
}

void comlz4_block_decompress(void)
{
    uint32_t source_length;
    uint32_t source_position;
    uint32_t destination_position;
    uint32_t original_size;
    int decompressed_size;
    size_t required;
    char *buffer;
    char *output;

    inline_as3(
        "if (src == null) throw new ArgumentError('src must not be null.');"
        "if (dest == null) throw new ArgumentError('dest must not be null.');"
        "%0 = src.bytesAvailable;"
        "%1 = src.position;"
        "%2 = dest.position;" : "=r"(source_length),
        "=r"(source_position), "=r"(destination_position));

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
        inline_as3("throw new RangeError('Block size exceeds the LZ4 API limit.');");
        return;
    }
    if (original_size > UINT32_MAX - destination_position)
    {
        inline_as3("throw new RangeError('dest does not have enough addressable space.');");
        return;
    }

    if (!comlz4_size_add(source_length, original_size, &required))
    {
        inline_as3("throw new RangeError('Required decompression buffer is too large.');");
        return;
    }

    buffer = comlz4_scratch_reserve(&comlz4_block_scratch, required);
    if (!buffer)
    {
        inline_as3("throw new Error('Unable to allocate the decompression buffer.');");
        return;
    }
    output = buffer + source_length;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(buffer), "r"(source_length));

    decompressed_size = LZ4_decompress_safe(
        buffer,
        output,
        (int)source_length,
        (int)original_size);

    if (decompressed_size != (int)original_size)
    {
        inline_as3("src.position = %0;" : : "r"(source_position));
        inline_as3("throw new Error('LZ4 block decompression failed or originalSize is incorrect.');");
        return;
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(output));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(decompressed_size));
    inline_as3("dest.position += %0;" : : "r"(decompressed_size));
}
