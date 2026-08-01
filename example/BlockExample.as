package
{
    import com.lz4.Block;

    import flash.display.Sprite;
    import flash.utils.ByteArray;

    public final class BlockExample extends Sprite
    {
        public function BlockExample()
        {
            const input:String = new Array(10001).join("hello block world! ");
            const source:ByteArray = new ByteArray();
            const compressed:ByteArray = new ByteArray();
            const restored:ByteArray = new ByteArray();

            source.writeUTFBytes(input);
            source.position = 0;

            Block.compress(source, compressed, 5);
            compressed.position = 0;
            Block.decompress(compressed, restored, source.length);

            trace("raw size: " + source.length);
            trace("compressed size: " + compressed.length);
            trace("restored size: " + restored.length);
            trace("data equals: " + (restored.toString() == input));
        }
    }
}
