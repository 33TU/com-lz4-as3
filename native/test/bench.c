#include <AS3/AS3.h>
#include <stdio.h>

/* LZ4 block compression against the player's built-in deflate.

   This is not an apples-to-apples implementation comparison. ByteArray
   .compress() is native player code, while this library is C compiled to
   AVM2 bytecode -- except that memcpy and memmove are preserved as
   crossbridge built-ins (see exports.txt) and map to native ByteArray
   operations. LZ4's copy-heavy paths therefore run at native speed while
   only its scan logic is interpreted, which is why incompressible input,
   where LZ4 emits one long literal run, measures close to raw memcpy. */

#define BENCH_ITERATIONS 20
#define BENCH_TARGET_SIZE 2097152

static void bench_dataset(const char *name, int kind)
{
    int raw_size;
    int lz4_size;
    int deflate_size;
    int lz4_compress_ms;
    int lz4_decompress_ms;
    int deflate_compress_ms;
    int deflate_decompress_ms;

    inline_as3(
        "import flash.utils.ByteArray;"
        "import flash.utils.CompressionAlgorithm;"
        "import flash.utils.getTimer;"

        "var kind:int = %0;"
        "var iterations:int = %1;"
        "var target:int = %2;"
        "var seed:uint = 0x12345678;"
        "var source:ByteArray = new ByteArray();"

        /* 0: prose, 1: incompressible noise, 2: sparse records. */
        "if (kind == 0) {"
        "var chunk:String = 'the quick brown fox jumps over the lazy dog 0123456789 ';"
        "while (source.length < target) source.writeUTFBytes(chunk);"
        "} else if (kind == 1) {"
        "while (source.length < target) {"
        "seed = seed * 1664525 + 1013904223;"
        "source.writeByte(seed >>> 24);"
        "}"
        "} else {"
        "while (source.length < target) {"
        "seed = seed * 1664525 + 1013904223;"
        "source.writeInt(int(seed >>> 28));"
        "source.writeInt(0);"
        "source.writeInt(0);"
        "source.writeInt(int(seed >>> 30));"
        "}"
        "}"
        : : "r"(kind), "r"(BENCH_ITERATIONS), "r"(BENCH_TARGET_SIZE));

    inline_as3(
        /* LZ4 block, which reads and writes without consuming its input. */
        "var lz4Compressed:ByteArray = new ByteArray();"
        "var start:int = getTimer();"
        "for (var n:int = 0; n < iterations; ++n) {"
        "lz4Compressed.length = 0;"
        "lz4Compressed.position = 0;"
        "source.position = 0;"
        "com.lz4._native.compressBlock(source, lz4Compressed, 1);"
        "}"
        "var lz4CompressMs:int = getTimer() - start;"

        "var lz4Restored:ByteArray = new ByteArray();"
        "start = getTimer();"
        "for (n = 0; n < iterations; ++n) {"
        "lz4Restored.length = 0;"
        "lz4Restored.position = 0;"
        "lz4Compressed.position = 0;"
        "com.lz4._native.decompressBlock(lz4Compressed, lz4Restored, source.length);"
        "}"
        "var lz4DecompressMs:int = getTimer() - start;"
        "if (lz4Restored.length != source.length) throw new Error('lz4 bench length differs');"
        "source.position = 0;"
        "lz4Restored.position = 0;"
        "for (n = 0; n < source.length; ++n) {"
        "if (source.readUnsignedByte() != lz4Restored.readUnsignedByte()) {"
        "throw new Error('lz4 bench round trip differs at ' + n);"
        "}"
        "}");

    inline_as3(
        /* Deflate works in place, so the copy each iteration needs is timed
           separately and subtracted rather than charged to the codec. */
        "var work:ByteArray = new ByteArray();"
        "start = getTimer();"
        "for (n = 0; n < iterations; ++n) {"
        "work.length = 0;"
        "work.position = 0;"
        "work.writeBytes(source);"
        "}"
        "var copyMs:int = getTimer() - start;"

        "start = getTimer();"
        "for (n = 0; n < iterations; ++n) {"
        "work.length = 0;"
        "work.position = 0;"
        "work.writeBytes(source);"
        "work.compress(CompressionAlgorithm.DEFLATE);"
        "}"
        "var deflateCompressMs:int = getTimer() - start - copyMs;"
        "if (deflateCompressMs < 0) deflateCompressMs = 0;"

        "var deflateCompressed:ByteArray = new ByteArray();"
        "deflateCompressed.writeBytes(work);"

        "start = getTimer();"
        "for (n = 0; n < iterations; ++n) {"
        "work.length = 0;"
        "work.position = 0;"
        "work.writeBytes(deflateCompressed);"
        "}"
        "copyMs = getTimer() - start;"

        "start = getTimer();"
        "for (n = 0; n < iterations; ++n) {"
        "work.length = 0;"
        "work.position = 0;"
        "work.writeBytes(deflateCompressed);"
        "work.uncompress(CompressionAlgorithm.DEFLATE);"
        "}"
        "var deflateDecompressMs:int = getTimer() - start - copyMs;"
        "if (deflateDecompressMs < 0) deflateDecompressMs = 0;"
        "if (work.length != source.length) throw new Error('deflate bench length differs');"
        "source.position = 0;"
        "work.position = 0;"
        "for (n = 0; n < source.length; ++n) {"
        "if (source.readUnsignedByte() != work.readUnsignedByte()) {"
        "throw new Error('deflate bench round trip differs at ' + n);"
        "}"
        "}");

    inline_as3(
        "%0 = source.length;"
        "%1 = lz4Compressed.length;"
        "%2 = deflateCompressed.length;" : "=r"(raw_size), "=r"(lz4_size),
        "=r"(deflate_size));

    inline_as3(
        "%0 = lz4CompressMs;"
        "%1 = lz4DecompressMs;"
        "%2 = deflateCompressMs;"
        "%3 = deflateDecompressMs;" : "=r"(lz4_compress_ms),
        "=r"(lz4_decompress_ms), "=r"(deflate_compress_ms),
        "=r"(deflate_decompress_ms));

    {
        double total_mb =
            (double)raw_size * (double)BENCH_ITERATIONS / (1024.0 * 1024.0);

        printf("%-9s %7.1f%% %7.1f%% %9.0f %9.0f %9.0f %9.0f\n",
               name,
               100.0 * (double)lz4_size / (double)raw_size,
               100.0 * (double)deflate_size / (double)raw_size,
               lz4_compress_ms ? total_mb / (lz4_compress_ms / 1000.0) : 0.0,
               deflate_compress_ms ? total_mb / (deflate_compress_ms / 1000.0) : 0.0,
               lz4_decompress_ms ? total_mb / (lz4_decompress_ms / 1000.0) : 0.0,
               deflate_decompress_ms ? total_mb / (deflate_decompress_ms / 1000.0) : 0.0);
    }
}

int main(void)
{
    printf("LZ4 block vs ByteArray deflate, %d MiB x %d iterations\n",
           BENCH_TARGET_SIZE / (1024 * 1024), BENCH_ITERATIONS);
    printf("throughput in MiB/s, ratio as %% of the original size\n\n");
    printf("%-9s %8s %8s %9s %9s %9s %9s\n",
           "dataset", "lz4", "deflate",
           "lz4-comp", "def-comp", "lz4-dec", "def-dec");

    bench_dataset("prose", 0);
    bench_dataset("noise", 1);
    bench_dataset("records", 2);

    return 0;
}
