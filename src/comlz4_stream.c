#include "comlz4.h"

void init_stream(void)
{
    LZ4_stream_t *stream = LZ4_createStream();
    NULL_CHECK(stream);
    AS3_Return((uint32_t)stream);

_err_cleanup:
    AS3_Return(0);
}

void init_stream_decode(void)
{
    LZ4_streamDecode_t *stream_decode = LZ4_createStreamDecode();
    NULL_CHECK(stream_decode);
    AS3_Return((uint32_t)stream_decode);

_err_cleanup:
    AS3_Return(0);
}

void free_stream(void)
{
    LZ4_stream_t *stream = NULL;
    inline_as3("%0 = handlePtr;" : "=r"(stream));
    AS3_Return(LZ4_freeStream(stream));
}

void free_stream_decode(void)
{
    LZ4_streamDecode_t *stream_decode = NULL;
    inline_as3("%0 = handlePtr;" : "=r"(stream_decode));
    AS3_Return(LZ4_freeStreamDecode(stream_decode));
}

void compress_stream(void)
{
    LZ4_stream_t *stream;
    inline_as3("%0 = handlePtr;" : "=r"(stream));
    if (!stream)
    {
        HANDLE_ERROR("invalid stream handle");
    }

    unsigned int src_len;
    inline_as3("%0 = src.bytesAvailable;" : "=r"(src_len));
    if (!src_len)
    {
        HANDLE_ERROR("empty src");
    }

    int max_compressed_size = LZ4_compressBound(src_len);
    if (max_compressed_size <= 0)
    {
        HANDLE_ERROR("compress bound check failed");
    }

    char *buf = buffer_reserve((size_t)src_len + (size_t)max_compressed_size);
    NULL_CHECK(buf);

    char *src_data = buf;
    char *dest_data = src_data + src_len;

    int acceleration;
    inline_as3("%0 = acceleration;" : "=r"(acceleration));

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(src_data), "r"(src_len));

    int compressed_size = LZ4_compress_fast_continue(
        stream,
        src_data,
        dest_data,
        src_len,
        max_compressed_size,
        acceleration);

    if (compressed_size <= 0)
    {
        HANDLE_ERROR("compress fast failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));

    AS3_Return(compressed_size);

_err_cleanup:
    AS3_Return(0);
}

void decompress_stream(void)
{
    LZ4_streamDecode_t *stream_decode;
    inline_as3("%0 = handlePtr;" : "=r"(stream_decode));
    if (!stream_decode)
    {
        HANDLE_ERROR("invalid stream handle");
    }

    unsigned int src_len;
    inline_as3("%0 = src.bytesAvailable;" : "=r"(src_len));
    if (!src_len)
    {
        HANDLE_ERROR("empty src");
    }

    unsigned int original_size;
    inline_as3("%0 = originalSize;" : "=r"(original_size));
    if (!original_size)
    {
        HANDLE_ERROR("read max must be greater than zero");
    }

    char *buf = buffer_reserve(src_len + original_size);
    NULL_CHECK(buf);

    char *src_data = buf;
    char *dest_data = src_data + src_len;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(src_data), "r"(src_len));

    int decompressed_size = LZ4_decompress_safe_continue(
        stream_decode,
        src_data,
        dest_data,
        src_len,
        original_size);

    if (decompressed_size <= 0)
    {
        HANDLE_ERROR("decompression failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(decompressed_size));

    AS3_Return(decompressed_size);

_err_cleanup:
    AS3_Return(0);
}