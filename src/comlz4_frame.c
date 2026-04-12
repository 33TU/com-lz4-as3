#include <string.h>
#include "comlz4.h"

typedef struct
{
    LZ4F_cctx *cctx;
    LZ4F_preferences_t prefs;
} CompressionContext;

static void default_preference(LZ4F_preferences_t *preferences, int compressionLevel)
{
    memset(preferences, 0, sizeof(LZ4F_preferences_t));
    preferences->compressionLevel = compressionLevel;
    preferences->autoFlush = 1;
    preferences->frameInfo.blockMode = LZ4F_blockIndependent;
}

void init_frame_compressor(void)
{
    int compression_level;
    inline_as3("%0 = compressionLevel;" : "=r"(compression_level));

    CompressionContext *ctx = malloc(sizeof(CompressionContext));
    NULL_CHECK(ctx);

    if (LZ4F_isError(LZ4F_createCompressionContext(&ctx->cctx, LZ4F_VERSION)))
    {
        HANDLE_ERROR("failed to create LZ4 compression context");
    }

    default_preference(&ctx->prefs, compression_level);
    AS3_Return((uint32_t)ctx);

_err_cleanup:
    if (ctx)
        free(ctx);
    AS3_Return(0);
}

void free_frame_compressor(void)
{
    CompressionContext *ctx;
    inline_as3("%0 = handlePtr;" : "=r"(ctx));
    NULL_CHECK(ctx);

    LZ4F_freeCompressionContext(ctx->cctx);
    free(ctx);

    AS3_Return(1);

_err_cleanup:
    AS3_Return(0);
}

void frame_compress_begin(void)
{
    CompressionContext *ctx;
    inline_as3("%0 = handlePtr;" : "=r"(ctx));
    NULL_CHECK(ctx);

    char *dest_data = buffer_reserve(LZ4F_HEADER_SIZE_MAX);
    NULL_CHECK(dest_data);

    size_t compressed_size =
        LZ4F_compressBegin(ctx->cctx, dest_data, LZ4F_HEADER_SIZE_MAX, &ctx->prefs);

    if (LZ4F_isError(compressed_size))
    {
        HANDLE_ERROR("compress begin frame failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));

    AS3_Return(compressed_size);

_err_cleanup:
    AS3_Return(0);
}

void frame_compress_update(void)
{
    CompressionContext *ctx;
    inline_as3("%0 = handlePtr;" : "=r"(ctx));
    NULL_CHECK(ctx);

    unsigned int src_len;
    inline_as3("%0 = src.bytesAvailable;" : "=r"(src_len));
    if (!src_len)
    {
        HANDLE_ERROR("empty src");
    }

    size_t max_compressed_size = LZ4F_compressBound(src_len, &ctx->prefs);
    if (max_compressed_size <= 0)
    {
        HANDLE_ERROR("compress bound check failed");
    }

    char *buf = buffer_reserve(src_len + max_compressed_size);
    NULL_CHECK(buf);

    char *src_data = buf;
    char *dest_data = src_data + src_len;

    inline_as3("src.readBytes(CModule.ram, %0, %1);" : : "r"(src_data), "r"(src_len));

    size_t compressed_size =
        LZ4F_compressUpdate(ctx->cctx, dest_data, max_compressed_size,
                            src_data, src_len, NULL);

    if (LZ4F_isError(compressed_size))
    {
        HANDLE_ERROR("compress update frame failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));

    AS3_Return(compressed_size);

_err_cleanup:
    AS3_Return(0);
}

void frame_compress_end(void)
{
    CompressionContext *ctx;
    inline_as3("%0 = handlePtr;" : "=r"(ctx));
    NULL_CHECK(ctx);

    size_t max_compressed_size = LZ4F_compressBound(0, &ctx->prefs);
    if (max_compressed_size <= 0)
    {
        HANDLE_ERROR("compress bound check failed");
    }

    char *dest_data = buffer_reserve(max_compressed_size);
    NULL_CHECK(dest_data);

    size_t compressed_size =
        LZ4F_compressEnd(ctx->cctx, dest_data, max_compressed_size, NULL);

    if (LZ4F_isError(compressed_size))
    {
        HANDLE_ERROR("compress end frame failed");
    }

    inline_as3("CModule.ram.position = %0;" : : "r"(dest_data));
    inline_as3("CModule.ram.readBytes(dest, dest.position, %0);" : : "r"(compressed_size));

    AS3_Return(compressed_size);

_err_cleanup:
    AS3_Return(0);
}