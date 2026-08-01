package
{
    import com.lz4.Block;
    import com.lz4.FrameDecoder;
    import com.lz4.FrameEncoder;
    import com.lz4.StreamDecoder;
    import com.lz4.StreamEncoder;

    import flash.display.Sprite;
    import flash.utils.ByteArray;

    public final class Lz4Test extends Sprite
    {
        public function Lz4Test()
        {
            testBlock();
            testStream();
            testFrame();
            trace("AS3 tests passed");
        }

        private static function testBlock():void
        {
            const input:String = repeat("block data ", 1000);
            const source:ByteArray = bytes(input);
            const compressed:ByteArray = new ByteArray();
            const restored:ByteArray = new ByteArray();

            Block.compress(source, compressed);
            compressed.position = 0;
            Block.decompress(compressed, restored, source.length);

            assertEquals(input, restored.toString(), "block round trip");
        }

        private static function testStream():void
        {
            const firstInput:String = repeat("first stream block ", 1000);
            const secondInput:String = repeat("second stream block ", 1000);
            const first:ByteArray = bytes(firstInput);
            const second:ByteArray = bytes(secondInput);
            const firstCompressed:ByteArray = new ByteArray();
            const secondCompressed:ByteArray = new ByteArray();
            const firstRestored:ByteArray = new ByteArray();
            const secondRestored:ByteArray = new ByteArray();
            const encoder:StreamEncoder = new StreamEncoder();

            try
            {
                encoder.compress(first, firstCompressed);
                encoder.compress(second, secondCompressed);
            }
            finally
            {
                encoder.dispose();
                encoder.dispose();
            }

            const decoder:StreamDecoder = new StreamDecoder();
            try
            {
                firstCompressed.position = 0;
                secondCompressed.position = 0;
                decoder.decompress(firstCompressed, firstRestored, first.length);
                decoder.decompress(secondCompressed, secondRestored, second.length);

                assertEquals(firstInput, firstRestored.toString(), "first stream block");
                assertEquals(secondInput, secondRestored.toString(), "second stream block");
            }
            finally
            {
                decoder.dispose();
                decoder.dispose();
            }
        }

        private static function testFrame():void
        {
            const input:String = repeat("frame data ", 1000);
            const source:ByteArray = bytes(input);
            const compressed:ByteArray = new ByteArray();
            const restored:ByteArray = new ByteArray();
            const encoder:FrameEncoder = new FrameEncoder();

            try
            {
                encoder.begin(compressed);
                encoder.update(source, compressed);
                encoder.end(compressed);
            }
            finally
            {
                encoder.dispose();
                encoder.dispose();
            }

            const decoder:FrameDecoder = new FrameDecoder();
            try
            {
                compressed.position = 0;
                assertTrue(decoder.decompress(compressed, restored), "complete frame");
                assertEquals(input, restored.toString(), "frame round trip");
            }
            finally
            {
                decoder.dispose();
                decoder.dispose();
            }
        }

        private static function bytes(value:String):ByteArray
        {
            const result:ByteArray = new ByteArray();
            result.writeUTFBytes(value);
            result.position = 0;
            return result;
        }

        private static function repeat(value:String, count:int):String
        {
            return new Array(count + 1).join(value);
        }

        private static function assertEquals(expected:String, actual:String, label:String):void
        {
            if (actual != expected)
                throw new Error(label + " failed");
        }

        private static function assertTrue(value:Boolean, label:String):void
        {
            if (!value)
                throw new Error(label + " failed");
        }
    }
}
