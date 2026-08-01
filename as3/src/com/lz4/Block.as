package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.compressBlock;
    import com.lz4._native.decompressBlock;

    public final class Block
    {
        public static function compress(
                src:ByteArray,
                dest:ByteArray,
                acceleration:int = 1
            ):void
        {
            compressBlock(src, dest, acceleration);
        }

        public static function decompress(
                src:ByteArray,
                dest:ByteArray,
                originalSize:uint
            ):void
        {
            decompressBlock(src, dest, originalSize);
        }
    }
}
