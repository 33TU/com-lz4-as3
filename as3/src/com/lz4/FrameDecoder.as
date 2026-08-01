package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.createFrameDecoder;
    import com.lz4._native.decompressFrame;
    import com.lz4._native.disposeFrameDecoder;

    /**
     * Incremental, size-limited LZ4 frame decompression.
     *
     * <p>Decodes frames written by <code>FrameEncoder</code> or any standard
     * LZ4 frame tool. The frame carries its own block boundaries, so no
     * uncompressed size has to be supplied, and its content checksum is
     * verified automatically: a corrupt or tampered frame throws instead of
     * yielding garbage.</p>
     *
     * <p>Input may arrive in pieces. Feed whatever bytes you have; the decoder
     * consumes what it can and reports whether a frame ended. Bytes belonging
     * to a following concatenated frame are left unread, so the same instance
     * decodes a stream of frames back to back.</p>
     *
     * <listing version="3.0">
     * const decoder:FrameDecoder = new FrameDecoder();
     *
     * try
     * {
     *     while (!decoder.decompress(source, destination))
     *         source = awaitMoreBytes();
     * }
     * finally
     * {
     *     decoder.dispose();
     * }
     * </listing>
     *
     * <p>Instances own native memory. Always call <code>dispose()</code>,
     * preferably from <code>finally</code>.</p>
     *
     * @see com.lz4.FrameEncoder
     */
    public final class FrameDecoder
    {
        /**
         * Default ceiling on the output of a single frame, in bytes.
         *
         * @see com.lz4.FrameDecoder#FrameDecoder()
         */
        public static const DEFAULT_MAX_OUTPUT_SIZE:uint = 32 * 1024 * 1024;

        private var _handle:uint;

        /**
         * Creates a decoder that rejects any frame decoding to more than
         * <code>maxOutputSize</code> bytes.
         *
         * <p>The limit spans a whole frame, including output produced across
         * several <code>decompress()</code> calls, and it is what bounds the
         * cost of a hostile frame. The decoder buffers a whole frame natively
         * before writing to <code>dest</code>, so this also caps its native
         * memory. Raise it only as far as you are willing to allocate.</p>
         *
         * @param maxOutputSize Maximum uncompressed bytes per frame. Must be
         *                      greater than 0.
         *
         * @throws ArgumentError If <code>maxOutputSize</code> is 0.
         * @throws Error         If the native decoder cannot be allocated or
         *                       created.
         */
        public function FrameDecoder(
                maxOutputSize:uint = DEFAULT_MAX_OUTPUT_SIZE
            )
        {
            _handle = createFrameDecoder(maxOutputSize);
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
         * Decodes as much of <code>src</code> as belongs to the current frame.
         *
         * <p>Reads from <code>src.position</code> and writes any output at
         * <code>dest.position</code>, advancing <code>src.position</code> by
         * the bytes consumed and <code>dest.position</code> by the bytes
         * produced. Returns <code>false</code> when the input ran out mid-frame
         * — call again with more bytes appended, and the decoder resumes where
         * it stopped.</p>
         *
         * <p>A frame decoded in one call is all-or-nothing: nothing is written
         * to <code>dest</code> unless the whole frame succeeded. Across several
         * calls that no longer holds, because output from earlier calls has
         * already been written by the time the trailing checksum is verified.
         * </p>
         *
         * @param src  Frame bytes, read from its current position. An empty
         *             <code>src</code> is a no-op returning <code>false</code>.
         * @param dest Destination, written at its current position.
         *
         * @return <code>true</code> when a complete frame ended in this call.
         *
         * @throws ArgumentError If this decoder is disposed, either buffer is
         *                       null, or both arguments are the same instance.
         * @throws RangeError    If the frame exceeds the configured
         *                       <code>maxOutputSize</code> or <code>dest</code>
         *                       lacks addressable space.
         * @throws Error         If a buffer cannot be allocated, or the frame
         *                       is malformed or fails its content checksum.
         *                       <code>src.position</code> is restored and the
         *                       decoder resets, so the next call may start a
         *                       fresh frame.
         */
        public function decompress(src:ByteArray, dest:ByteArray):Boolean
        {
            return decompressFrame(_handle, src, dest);
        }

        /**
         * Releases the native decoder and its buffers. Idempotent.
         */
        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeFrameDecoder(_handle);
            _handle = 0;
        }
    }
}
