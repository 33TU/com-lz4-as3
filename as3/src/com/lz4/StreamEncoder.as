package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.compressStream;
    import com.lz4._native.createStreamEncoder;
    import com.lz4._native.disposeStreamEncoder;

    /**
     * Dictionary-dependent LZ4 block compression.
     *
     * <p>Each block is compressed against the previous 64 KiB of input, so a
     * sequence of related blocks compresses better than the same blocks
     * compressed independently. That gain costs coupling: the output carries no
     * framing, so you must transmit each block's uncompressed size yourself and
     * present the blocks to a <code>StreamDecoder</code> in exactly the order
     * and boundaries seen here. Getting that wrong yields a generic failure or
     * silently wrong output, because there is nothing in the payload to detect
     * it with.</p>
     *
     * <p>Prefer <code>FrameEncoder</code> unless you already have your own
     * framing.</p>
     *
     * <p>Instances own native memory. Always call <code>dispose()</code>,
     * preferably from <code>finally</code>.</p>
     *
     * @see com.lz4.StreamDecoder
     * @see com.lz4.FrameEncoder
     */
    public final class StreamEncoder
    {
        private var _handle:uint;

        /**
         * Creates an encoder with an empty dictionary.
         *
         * @throws Error If the native encoder cannot be allocated.
         */
        public function StreamEncoder()
        {
            _handle = createStreamEncoder();
        }

        /**
         * Whether <code>dispose()</code> has released the native encoder.
         *
         * <p>Every other method throws once this is <code>true</code>.</p>
         */
        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        /**
         * Compresses every readable byte of <code>src</code> as the next block
         * in the sequence.
         *
         * <p>Reads <code>src.bytesAvailable</code> bytes from
         * <code>src.position</code> and writes the block at
         * <code>dest.position</code>, advancing both positions. The block is
         * compressed against the preceding blocks and is not independently
         * decodable.</p>
         *
         * @param src          Source bytes, read from its current position.
         * @param dest         Destination, written at its current position.
         * @param acceleration Speed/ratio tradeoff; higher is faster and
         *                     compresses less. Must be at least 1.
         *
         * @throws ArgumentError If this encoder is disposed, either buffer is
         *                       null, both arguments are the same instance,
         *                       <code>src</code> has no readable bytes, or
         *                       <code>acceleration</code> is below 1.
         * @throws RangeError    If <code>src</code> exceeds the LZ4 input limit
         *                       or <code>dest</code> lacks addressable space.
         * @throws Error         If the scratch buffer cannot be allocated or
         *                       LZ4 reports a failure. <code>src.position</code>
         *                       is restored when compression fails.
         */
        public function compress(
                src:ByteArray,
                dest:ByteArray,
                acceleration:int = 1
            ):void
        {
            compressStream(_handle, src, dest, acceleration);
        }

        /**
         * Releases the native encoder and its scratch buffer. Idempotent.
         */
        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeStreamEncoder(_handle);
            _handle = 0;
        }
    }
}
