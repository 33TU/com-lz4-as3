package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.compressBlock;
    import com.lz4._native.decompressBlock;

    /**
     * Stateless LZ4 block compression.
     *
     * <p>Each block is independent, so blocks may be compressed and
     * decompressed in any order. Nothing is stored alongside the output, which
     * means the uncompressed size must be transmitted separately and supplied
     * to <code>decompress()</code>. Use <code>FrameEncoder</code> when you want
     * that size, the block boundaries, and an integrity check carried by the
     * payload itself.</p>
     *
     * @see com.lz4.FrameEncoder
     */
    public final class Block
    {
        /**
         * Compresses every readable byte of <code>src</code> as one LZ4 block.
         *
         * <p>Reads <code>src.bytesAvailable</code> bytes from
         * <code>src.position</code> and writes the block at
         * <code>dest.position</code>, advancing both positions. Read the
         * compressed size from the destination position or length.</p>
         *
         * @param src          Source bytes, read from its current position.
         * @param dest         Destination, written at its current position.
         * @param acceleration Speed/ratio tradeoff; higher is faster and
         *                     compresses less. Must be at least 1.
         *
         * @throws ArgumentError If either buffer is null, both arguments are
         *                       the same instance, <code>src</code> has no
         *                       readable bytes, or <code>acceleration</code>
         *                       is below 1.
         * @throws RangeError    If <code>src</code> exceeds the LZ4 input limit
         *                       or <code>dest</code> lacks addressable space.
         * @throws Error         If the scratch buffer cannot be allocated or
         *                       LZ4 reports a failure. <code>src.position</code>
         *                       is restored when compression fails.
         */
        public static function compress(
                src:ByteArray,
                dest:ByteArray,
                acceleration:int = 1
            ):void
        {
            compressBlock(src, dest, acceleration);
        }

        /**
         * Decompresses one LZ4 block whose uncompressed size is already known.
         *
         * <p>Reads <code>src.bytesAvailable</code> bytes from
         * <code>src.position</code> and writes the result at
         * <code>dest.position</code>, advancing both positions.</p>
         *
         * <p><code>originalSize</code> must be exact; it is not a hint, and it
         * controls the native output allocation. Validate it before calling
         * when it arrives from an untrusted source, since this API applies no
         * ceiling of its own.</p>
         *
         * @param src          Compressed block, read from its current position.
         * @param dest         Destination, written at its current position.
         * @param originalSize Exact uncompressed size of the block.
         *
         * @throws ArgumentError If either buffer is null, both arguments are
         *                       the same instance, <code>src</code> has no
         *                       readable bytes, or <code>originalSize</code>
         *                       is 0.
         * @throws RangeError    If either size exceeds the LZ4 API limit or
         *                       <code>dest</code> lacks addressable space.
         * @throws Error         If the scratch buffer cannot be allocated, the
         *                       block is corrupt, or <code>originalSize</code>
         *                       is wrong. <code>src.position</code> is restored
         *                       when decompression fails.
         */
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
