package
{
    import flash.display.Sprite;
    import flash.utils.ByteArray;
    import com.lz4.*;

    public class Main extends Sprite
    {
        public function Main()
        {
            testLibBlockOps();
        }

        private function testLibBlockOps():void
        {
            function repeatString(str:String, times:int):String
            {
                return new Array(times + 1).join(str);
            }

            var a:ByteArray = new ByteArray();
            var b:ByteArray = new ByteArray();
            var c:ByteArray = new ByteArray();

            var input:String = repeatString("hello world! ", 10000);
            a.writeUTFBytes(input);
            a.position = 0;

            // Call native functions from com.lz4 SWC
            var compressedSize:int = com.lz4.compress(a, b, 5);
            var decompressedSize:int = com.lz4.decompress(b, c, a.length);

            trace("raw size: " + a.length);
            trace("compressed size: " + compressedSize);
            trace("decompressed length: " + decompressedSize);
            trace("data equals: " + (c.toString() == input));
        }
    }
}
