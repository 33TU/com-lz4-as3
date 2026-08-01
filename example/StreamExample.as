package
{
    import com.lz4.StreamDecoder;
    import com.lz4.StreamEncoder;

    import flash.display.Sprite;
    import flash.utils.ByteArray;

    public final class StreamExample extends Sprite
    {
        public function StreamExample()
        {
            const firstInput:String = new Array(5001).join("first stream block ");
            const secondInput:String = new Array(5001).join("second stream block ");
            const first:ByteArray = bytes(firstInput);
            const second:ByteArray = bytes(secondInput);
            const compressed:Vector.<ByteArray> = compress(first, second);
            const restored:Vector.<ByteArray> = decompress(
                    compressed[0],
                    compressed[1],
                    first.length,
                second.length
            );

            trace("first raw size: " + first.length);
            trace("first compressed size: " + compressed[0].length);
            trace("first block equals: " + (restored[0].toString() == firstInput));
            trace("second raw size: " + second.length);
            trace("second compressed size: " + compressed[1].length);
            trace("second block equals: " + (restored[1].toString() == secondInput));
            trace("total raw size: " + (first.length + second.length));
            trace("total compressed size: " + (compressed[0].length + compressed[1].length));
        }

        private static function compress(
                first:ByteArray,
                second:ByteArray
            ):Vector.<ByteArray>
        {
            const firstCompressed:ByteArray = new ByteArray();
            const secondCompressed:ByteArray = new ByteArray();
            const encoder:StreamEncoder = new StreamEncoder();

            try
            {
                encoder.compress(first, firstCompressed, 5);
                encoder.compress(second, secondCompressed, 5);
            }
            finally
            {
                encoder.dispose();
            }

            const result:Vector.<ByteArray> = new Vector.<ByteArray>(2, true);
            result[0] = firstCompressed;
            result[1] = secondCompressed;
            return result;
        }

        private static function decompress(
                first:ByteArray,
                second:ByteArray,
                firstSize:uint,
                secondSize:uint
            ):Vector.<ByteArray>
        {
            const firstRestored:ByteArray = new ByteArray();
            const secondRestored:ByteArray = new ByteArray();
            const decoder:StreamDecoder = new StreamDecoder();

            try
            {
                first.position = 0;
                second.position = 0;
                decoder.decompress(first, firstRestored, firstSize);
                decoder.decompress(second, secondRestored, secondSize);
            }
            finally
            {
                decoder.dispose();
            }

            const result:Vector.<ByteArray> = new Vector.<ByteArray>(2, true);
            result[0] = firstRestored;
            result[1] = secondRestored;
            return result;
        }

        private static function bytes(value:String):ByteArray
        {
            const result:ByteArray = new ByteArray();
            result.writeUTFBytes(value);
            result.position = 0;
            return result;
        }
    }
}
