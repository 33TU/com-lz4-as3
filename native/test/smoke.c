#include <AS3/AS3.h>
#include <stdio.h>

// Block compression and decompression
static void test_block(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "var input:String = new Array(10001).join('hello world! ');"
        "var source:ByteArray = new ByteArray();"
        "var compressed:ByteArray = new ByteArray();"
        "var restored:ByteArray = new ByteArray();"
        "source.writeUTFBytes(input);"
        "source.position = 0;"

        "com.lz4._native.compressBlock(source, compressed, 5);"
        "compressed.position = 0;"
        "com.lz4._native.decompressBlock(compressed, restored, source.length);"

        "if (restored.length != source.length || restored.toString() != input) {"
        "throw new Error('block round trip failed');"
        "}"

        "var rejectedEmpty:Boolean = false;"
        "try {"
        "com.lz4._native.compressBlock(new ByteArray(), new ByteArray());"
        "} catch (error:ArgumentError) {"
        "rejectedEmpty = true;"
        "}"
        "if (!rejectedEmpty) throw new Error('empty block input was accepted');");

    inline_as3(
        "source.position = 0;"
        "var rejectedAlias:Boolean = false;"
        "try {"
        "com.lz4._native.compressBlock(source, source);"
        "} catch (aliasError:ArgumentError) {"
        "rejectedAlias = true;"
        "}"
        "if (!rejectedAlias || source.position != 0) throw new Error('aliased block buffers were accepted');"

        "compressed.position = 0;"
        "var originalPosition:uint = compressed.position;"
        "var rejectedWrongSize:Boolean = false;"
        "try {"
        "com.lz4._native.decompressBlock(compressed, new ByteArray(), source.length + 1);"
        "} catch (error:Error) {"
        "rejectedWrongSize = true;"
        "}"
        "if (!rejectedWrongSize) throw new Error('incorrect block size was accepted');"
        "if (compressed.position != originalPosition) throw new Error('failed block call consumed src');");
}

// Streaming compression and decompression
static void test_stream(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "var encoder:uint = com.lz4._native.createStreamEncoder();"
        "var decoder:uint = com.lz4._native.createStreamDecoder();"
        "var first:ByteArray = new ByteArray();"
        "var second:ByteArray = new ByteArray();"
        "var firstCompressed:ByteArray = new ByteArray();"
        "var secondCompressed:ByteArray = new ByteArray();"
        "var firstRestored:ByteArray = new ByteArray();"
        "var secondRestored:ByteArray = new ByteArray();"
        "var seed:uint = 0x12345678;"
        "for (var index:int = 0; index < 65536; ++index) {"
        "seed = seed * 1664525 + 1013904223;"
        "first.writeByte(seed >>> 24);"
        "}"
        "first.position = 32768;"
        "for (index = 0; index < 4096; ++index) second.writeByte(first.readUnsignedByte());"
        "first.position = 0;"
        "second.position = 0;"

        "com.lz4._native.compressStream(encoder, first, firstCompressed, 5);"
        "com.lz4._native.compressStream(encoder, second, secondCompressed, 5);"
        "firstCompressed.position = 0;"
        "secondCompressed.position = 0;"
        "com.lz4._native.decompressStream(decoder, firstCompressed, firstRestored, first.length);"
        "com.lz4._native.decompressStream(decoder, secondCompressed, secondRestored, second.length);"

        "if (secondCompressed.length >= second.length) throw new Error('stream dictionary was not used');"
        "first.position = 0;"
        "firstRestored.position = 0;"
        "for (index = 0; index < first.length; ++index) {"
        "if (first.readUnsignedByte() != firstRestored.readUnsignedByte()) throw new Error('first stream block differs');"
        "}"
        "second.position = 0;"
        "secondRestored.position = 0;"
        "for (index = 0; index < second.length; ++index) {"
        "if (second.readUnsignedByte() != secondRestored.readUnsignedByte()) throw new Error('second stream block differs');"
        "}"

        "com.lz4._native.disposeStreamEncoder(encoder);"
        "com.lz4._native.disposeStreamDecoder(decoder);");
}

// Frame compression and decompression
static void test_frame(void)
{
    inline_as3(
        "import flash.utils.ByteArray;"

        "var encoder:uint = com.lz4._native.createFrameEncoder();"
        "var input:String = new Array(10001).join('hello framed world! ');"
        "var source:ByteArray = new ByteArray();"
        "var compressed:ByteArray = new ByteArray();"
        "var restored:ByteArray = new ByteArray();"
        "source.writeUTFBytes(input);"
        "source.position = 0;"

        /* Reject frame updates before the header is written. */
        "var rejectedBeforeBegin:Boolean = false;"
        "try {"
        "com.lz4._native.updateFrame(encoder, source, compressed);"
        "} catch (error:Error) {"
        "rejectedBeforeBegin = true;"
        "}"
        "if (!rejectedBeforeBegin) throw new Error('frame update before begin was accepted');"

        /* Encode one complete frame. */
        "com.lz4._native.beginFrame(encoder, compressed);"
        "var rejectedAlias:Boolean = false;"
        "try {"
        "com.lz4._native.updateFrame(encoder, source, source);"
        "} catch (aliasError:ArgumentError) {"
        "rejectedAlias = true;"
        "}"
        "if (!rejectedAlias) throw new Error('aliased frame buffers were accepted');"
        "com.lz4._native.updateFrame(encoder, source, compressed);"
        "com.lz4._native.endFrame(encoder, compressed);"
        "if (compressed.length <= 11) throw new Error('frame output is incomplete');"
        "com.lz4._native.disposeFrameEncoder(encoder);"

        /* Enforce the configured output limit before writing any result. */
        "var limitedDecoder:uint = com.lz4._native.createFrameDecoder(32);"
        "compressed.position = 0;"
        "var rejectedLargeFrame:Boolean = false;"
        "try {"
        "com.lz4._native.decompressFrame(limitedDecoder, compressed, new ByteArray());"
        "} catch (limitError:RangeError) {"
        "rejectedLargeFrame = true;"
        "}"
        "if (!rejectedLargeFrame || compressed.position != 0) throw new Error('frame output limit failed');"

        "var emptyEncoder:uint = com.lz4._native.createFrameEncoder();"
        "var emptyFrame:ByteArray = new ByteArray();"
        "com.lz4._native.beginFrame(emptyEncoder, emptyFrame);"
        "com.lz4._native.endFrame(emptyEncoder, emptyFrame);"
        "com.lz4._native.disposeFrameEncoder(emptyEncoder);"
        "emptyFrame.position = 0;"
        "if (!com.lz4._native.decompressFrame(limitedDecoder, emptyFrame, new ByteArray())) {"
        "throw new Error('frame decoder did not recover after output limit');"
        "}"
        "com.lz4._native.disposeFrameDecoder(limitedDecoder);"

        "var rejectedZeroLimit:Boolean = false;"
        "try {"
        "com.lz4._native.createFrameDecoder(0);"
        "} catch (zeroLimitError:ArgumentError) {"
        "rejectedZeroLimit = true;"
        "}"
        "if (!rejectedZeroLimit) throw new Error('zero frame output limit was accepted');"

        /* Decode the complete frame. */
        "var decoder:uint = com.lz4._native.createFrameDecoder();"
        "compressed.position = 0;"
        "var complete:Boolean;"
        "try {"
        "complete = com.lz4._native.decompressFrame(decoder, compressed, restored);"
        "} catch (error:Error) {"
        "throw new Error('full frame decode: ' + error.message);"
        "}"
        "if (!complete || restored.toString() != input) throw new Error('frame round trip failed');"

        /* Stop after each frame when the input contains concatenated frames. */
        "var concatenated:ByteArray = new ByteArray();"
        "var firstRestored:ByteArray = new ByteArray();"
        "var secondRestored:ByteArray = new ByteArray();"
        "concatenated.writeBytes(compressed, 0, compressed.length);"
        "concatenated.writeBytes(compressed, 0, compressed.length);"
        "concatenated.position = 0;"
        "complete = com.lz4._native.decompressFrame(decoder, concatenated, firstRestored);"
        "if (!complete || concatenated.position != compressed.length) throw new Error('first concatenated frame failed');"
        "complete = com.lz4._native.decompressFrame(decoder, concatenated, secondRestored);"
        "if (!complete || secondRestored.toString() != input) throw new Error('second concatenated frame failed');"

        /* Reject corrupt data, restore src.position, and reuse the decoder. */
        "var invalid:ByteArray = new ByteArray();"
        "invalid.writeUTFBytes('not an lz4 frame');"
        "invalid.position = 0;"
        "var rejectedInvalid:Boolean = false;"
        "try {"
        "com.lz4._native.decompressFrame(decoder, invalid, new ByteArray());"
        "} catch (invalidError:Error) {"
        "rejectedInvalid = true;"
        "}"
        "if (!rejectedInvalid || invalid.position != 0) throw new Error('invalid frame handling failed');"
        "compressed.position = 0;"
        "var afterError:ByteArray = new ByteArray();"
        "complete = com.lz4._native.decompressFrame(decoder, compressed, afterError);"
        "if (!complete || afterError.toString() != input) throw new Error('frame decoder did not recover after an error');"
        "com.lz4._native.disposeFrameDecoder(decoder);"

        /* Resume an incomplete frame after more input becomes available. */
        "decoder = com.lz4._native.createFrameDecoder();"
        "var chunked:ByteArray = new ByteArray();"
        "var chunkedRestored:ByteArray = new ByteArray();"
        "chunked.writeBytes(compressed, 0, 5);"
        "chunked.position = 0;"
        "complete = com.lz4._native.decompressFrame(decoder, chunked, chunkedRestored);"
        "if (complete) throw new Error('partial frame was reported complete');"
        "var readPosition:uint = chunked.position;"
        "chunked.position = chunked.length;"
        "chunked.writeBytes(compressed, 5, compressed.length - 5);"
        "chunked.position = readPosition;"
        "complete = com.lz4._native.decompressFrame(decoder, chunked, chunkedRestored);"
        "if (!complete || chunkedRestored.toString() != input) throw new Error('chunked frame round trip failed');"
        "com.lz4._native.disposeFrameDecoder(decoder);");
}

// Smoke test runner
int main(void)
{
    test_block();
    printf("block test passed\n");
    test_stream();
    printf("stream test passed\n");
    test_frame();
    printf("frame test passed\n");
    return 0;
}
