#include "comlz4.h"

// Tests //
static void test_lib_stream_ops(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "function repeatString(str:String, times:int):String {"
        "return new Array(times + 1).join(str);"
        "}"

        "var handlePtr:uint = com.lz4.initStream();"
        "var streamDecodeHandle:uint = com.lz4.initStreamDecode();"

        "var a:ByteArray = new ByteArray();"
        "var b:ByteArray = new ByteArray();"
        "var c:ByteArray = new ByteArray();"

        "var input:String = repeatString('hello world! ', 10000);"
        "a.writeUTFBytes(input);"
        "a.position = 0;"

        "var compressedSize:int = com.lz4.compressStream(handlePtr, a, b, 5);"
        "var decompressedSize:int = com.lz4.decompressStream(streamDecodeHandle, b, c, a.length);"

        "trace('raw size: ' + a.length);"
        "trace('compressed size: ' + compressedSize);"
        "trace('decompressed length: ' + decompressedSize);"
        "trace('data equals: ' + (c.toString() == input));"

        "com.lz4.freeStream(handlePtr);"
        "com.lz4.freeStreamDecode(streamDecodeHandle);");
}

static void test_lib_frame_ops(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "function repeatString(str:String, times:int):String {"
        "return new Array(times + 1).join(str);"
        "}"

        "var frameCompressorHandle:uint = com.lz4.initFrameCompressor();"

        "var a:ByteArray = new ByteArray();"
        "var b:ByteArray = new ByteArray();"

        "var input:String = repeatString('hello world! ', 10000);"
        "a.writeUTFBytes(input);"

        "var compressedSize:int = 0;"
        "var size:int;"

        "size = com.lz4.compressFrameBegin(frameCompressorHandle, b);"
        "if (size <= 0) trace('invalid (size <= 0), size:' + size);"
        "compressedSize += size;"

        "a.position = 0;"
        "var size:int = com.lz4.compressFrameUpdate(frameCompressorHandle, a, b);"
        "if (size <= 0) trace('invalid (size <= 0), size:' + size);"
        "compressedSize += size;"

        "size = com.lz4.compressFrameEnd(frameCompressorHandle, b);"
        "if (size <= 0) trace('invalid (size <= 0), size:' + size);"
        "compressedSize += size;"

        "trace('raw size: ' + a.length);"
        "trace('compressed size: ' + compressedSize);"

        "com.lz4.freeFrameCompressor(frameCompressorHandle);");
}

static void test_lib_block_ops(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "function repeatString(str:String, times:int):String {"
        "return new Array(times + 1).join(str);"
        "}"

        "var a:ByteArray = new ByteArray();"
        "var b:ByteArray = new ByteArray();"
        "var c:ByteArray = new ByteArray();"

        "var input:String = repeatString('hello world! ', 10000);"
        "a.writeUTFBytes(input);"
        "a.position = 0;"

        "var compressedSize:int = com.lz4.compress(a, b, 5);"
        "var decompressedSize:int = com.lz4.decompress(b, c, a.length);"

        "trace('raw size: ' + a.length);"
        "trace('compressed size: ' + compressedSize);"
        "trace('decompressed length: ' + decompressedSize);"
        "trace('data equals: ' + (c.toString() == input));");
}

// Loader //
int main(void)
{
#ifdef DOTEST
    inline_as3("trace('== BLOCK TEST ==');");
    test_lib_block_ops();
    inline_as3("trace('== STREAM TEST ==');");
    test_lib_stream_ops();
    inline_as3("trace('== FRAME TEST ==');");
    test_lib_frame_ops();
#endif

    AS3_GoAsync();
}