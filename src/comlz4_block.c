#include "comlz4.h"

void compress(void)
{
    char *data = NULL;

    unsigned int src_len;
    inline_as3("%0 = src.bytesAvailable;" : "=r"(src_len));
    if (!src_len)
    {
        HANDLE_ERROR("empty src");
    }

    int acceleration;
    inline_as3("%0 = acceleration;" : "=r"(acceleration));

    int max_compressed_size = LZ4_compressBound(src_len);
    if (max_compressed_size <= 0)
    {
        HANDLE_ERROR("compress bound check failed");
    }

    data = (char *)malloc(src_len + max_compressed_size);
    NULL_CHECK(data);
    char *src_data = data;
    char *dest_data = data + src_len;
    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(src_data), "r"(src_len));

    int compressed_size = LZ4_compress_fast(src_data, dest_data, src_len, max_compressed_size, acceleration);
    if (compressed_size <= 0)
    {
        HANDLE_ERROR("compression failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));

    free(data);
    AS3_Return(compressed_size);

_err_cleanup:
    if (data)
    {
        free(data);
    }
    AS3_Return(0);
}

void decompress(void)
{
    char *data = NULL;

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
        HANDLE_ERROR("invalid original size");
    }

    data = (char *)malloc(src_len + original_size);
    NULL_CHECK(data);
    char *src_data = data;
    char *dest_data = data + src_len;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(src_data), "r"(src_len));
    int decompressed_size = LZ4_decompress_safe(src_data, dest_data, src_len, original_size);
    if (decompressed_size <= 0)
    {
        HANDLE_ERROR("decompression failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(decompressed_size));

    free(data);
    AS3_Return(decompressed_size);

_err_cleanup:
    if (data)
    {
        free(data);
    }
    AS3_Return(0);
}