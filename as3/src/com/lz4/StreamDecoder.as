package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.createStreamDecoder;
    import com.lz4._native.decompressStream;
    import com.lz4._native.disposeStreamDecoder;

    /**
     * Dictionary-dependent LZ4 block decompression.
     *
     * <p>Reverses <code>StreamEncoder</code>. Blocks must be supplied in
     * exactly the order and boundaries the encoder saw, each with its exact
     * uncompressed size, because the decoder rebuilds the same rolling 64 KiB
     * dictionary as it goes. A block fed out of order, split differently, or
     * given the wrong size either fails with a generic error or produces
     * silently wrong output.</p>
     *
     * <p>Instances own native memory. Always call <code>dispose()</code>,
     * preferably from <code>finally</code>.</p>
     *
     * @see com.lz4.StreamEncoder
     * @see com.lz4.FrameDecoder
     */
    public final class StreamDecoder
    {
        private var _handle:uint;

        /**
         * Creates a decoder with an empty dictionary.
         *
         * @throws Error If the native decoder cannot be allocated.
         */
        public function StreamDecoder()
        {
            _handle = createStreamDecoder();
        }

        /**
         * Whether <code>dispose()</code> has released the native decoder.
         *
         * <p>Every other method throws once this is <code>true</code>.</p>
         */
        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        /**
         * Decompresses the next block in the sequence.
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
         * @param originalSize Exact uncompressed size of this block.
         *
         * @throws ArgumentError If this decoder is disposed, either buffer is
         *                       null, both arguments are the same instance,
         *                       <code>src</code> has no readable bytes, or
         *                       <code>originalSize</code> is 0.
         * @throws RangeError    If either size exceeds the LZ4 API limit or
         *                       <code>dest</code> lacks addressable space.
         * @throws Error         If the scratch buffer cannot be allocated, the
         *                       dictionary cannot be configured, the block is
         *                       corrupt or out of order, or
         *                       <code>originalSize</code> is wrong.
         *                       <code>src.position</code> is restored when
         *                       decompression fails.
         */
        public function decompress(
                src:ByteArray,
                dest:ByteArray,
                originalSize:uint
            ):void
        {
            decompressStream(_handle, src, dest, originalSize);
        }

        /**
         * Releases the native decoder and its scratch buffer. Idempotent.
         */
        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeStreamDecoder(_handle);
            _handle = 0;
        }
    }
}
