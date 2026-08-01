package
{
    import com.lz4.FrameDecoder;
    import com.lz4.FrameEncoder;

    import flash.display.Sprite;
    import flash.utils.ByteArray;

    public final class FrameExample extends Sprite
    {
        public function FrameExample()
        {
            const input:String = new Array(10001).join("hello framed world! ");
            const source:ByteArray = new ByteArray();
            source.writeUTFBytes(input);
            source.position = 0;

            const compressed:ByteArray = compress(source);
            const restored:ByteArray = decompress(compressed);

            trace("raw size: " + source.length);
            trace("compressed size: " + compressed.length);
            trace("data equals: " + (restored.toString() == input));
        }

        private static function compress(source:ByteArray):ByteArray
        {
            const compressed:ByteArray = new ByteArray();
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
            }

            return compressed;
        }

        private static function decompress(compressed:ByteArray):ByteArray
        {
            const restored:ByteArray = new ByteArray();
            const decoder:FrameDecoder = new FrameDecoder();

            try
            {
                compressed.position = 0;

                if (!decoder.decompress(compressed, restored))
                    throw new Error("incomplete frame");
            }
            finally
            {
                decoder.dispose();
            }

            return restored;
        }
    }
}
