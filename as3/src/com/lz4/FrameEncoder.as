package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.beginFrame;
    import com.lz4._native.createFrameEncoder;
    import com.lz4._native.disposeFrameEncoder;
    import com.lz4._native.endFrame;
    import com.lz4._native.updateFrame;

    /**
     * Standard LZ4 frame compression.
     *
     * <p>The frame format carries its own block boundaries and a content
     * checksum, so a <code>FrameDecoder</code> needs nothing but the bytes and
     * reports corruption rather than decoding it into garbage. This is the mode
     * to reach for unless you have a specific reason not to.</p>
     *
     * <p>Call <code>begin()</code>, then <code>update()</code> for each chunk,
     * then <code>end()</code>. After <code>end()</code> the encoder is ready to
     * write another frame.</p>
     *
     * <listing version="3.0">
     * const encoder:FrameEncoder = new FrameEncoder();
     *
     * try
     * {
     *     encoder.begin(destination);
     *     encoder.update(source, destination);
     *     encoder.end(destination);
     * }
     * finally
     * {
     *     encoder.dispose();
     * }
     * </listing>
     *
     * <p>An encoder cannot be reused after a native error: every subsequent
     * call throws <code>IllegalOperationError</code>. Discard the instance and
     * construct a new one.</p>
     *
     * <p>Instances own native memory. Always call <code>dispose()</code>,
     * preferably from <code>finally</code>.</p>
     *
     * @see com.lz4.FrameDecoder
     */
    public final class FrameEncoder
    {
        private var _handle:uint;

        /**
         * Creates an encoder that emits independent blocks and a trailing
         * content checksum.
         *
         * @param compressionLevel Ratio/speed tradeoff. 0 selects the LZ4
         *                         default; higher values compress harder and
         *                         more slowly. Out-of-range values are clamped
         *                         by LZ4.
         *
         * @throws Error If the native encoder cannot be allocated or created.
         */
        public function FrameEncoder(compressionLevel:int = 0)
        {
            _handle = createFrameEncoder(compressionLevel);
        }

        /**
         * Whether <code>dispose()</code> has released the native encoder.
         *
         * <p>Every other method throws once this is <code>true</code>. This
         * does not report the terminal failure state described on the class;
         * that is only observable by catching
         * <code>IllegalOperationError</code>.</p>
         */
        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        /**
         * Writes the frame header at <code>dest.position</code> and advances
         * it, starting a new frame.
         *
         * @param dest Destination, written at its current position.
         *
         * @throws ArgumentError          If this encoder is disposed or
         *                                <code>dest</code> is null.
         * @throws IllegalOperationError  If a frame is already open, or the
         *                                encoder failed earlier.
         * @throws RangeError             If <code>dest</code> lacks
         *                                addressable space.
         * @throws Error                  If the header buffer cannot be
         *                                allocated or LZ4 reports a failure.
         */
        public function begin(dest:ByteArray):void
        {
            beginFrame(_handle, dest);
        }

        /**
         * Compresses every readable byte of <code>src</code> into the open
         * frame.
         *
         * <p>Reads <code>src.bytesAvailable</code> bytes from
         * <code>src.position</code> and appends any output at
         * <code>dest.position</code>, advancing both positions. A call may
         * produce no output, since LZ4 splits input on its own block
         * boundaries.</p>
         *
         * @param src  Source bytes, read from its current position.
         * @param dest Destination, written at its current position.
         *
         * @throws ArgumentError          If this encoder is disposed, either
         *                                buffer is null, both arguments are
         *                                the same instance, or <code>src</code>
         *                                has no readable bytes.
         * @throws IllegalOperationError  If <code>begin()</code> has not been
         *                                called, or the encoder failed earlier.
         * @throws RangeError             If the required buffer is too large or
         *                                <code>dest</code> lacks addressable
         *                                space.
         * @throws Error                  If the scratch buffer cannot be
         *                                allocated or LZ4 reports a failure.
         *                                <code>src.position</code> is restored
         *                                when compression fails.
         */
        public function update(src:ByteArray, dest:ByteArray):void
        {
            updateFrame(_handle, src, dest);
        }

        /**
         * Flushes any buffered data, writes the frame footer and content
         * checksum at <code>dest.position</code>, and advances it.
         *
         * <p>The encoder is ready for another <code>begin()</code> afterwards.
         * </p>
         *
         * @param dest Destination, written at its current position.
         *
         * @throws ArgumentError          If this encoder is disposed or
         *                                <code>dest</code> is null.
         * @throws IllegalOperationError  If no frame is open, or the encoder
         *                                failed earlier.
         * @throws RangeError             If <code>dest</code> lacks
         *                                addressable space.
         * @throws Error                  If the footer buffer cannot be
         *                                allocated or LZ4 reports a failure.
         */
        public function end(dest:ByteArray):void
        {
            endFrame(_handle, dest);
        }

        /**
         * Releases the native encoder and its scratch buffer. Idempotent.
         *
         * <p>Discards any open frame; the output written so far is left
         * unterminated and will not decode.</p>
         */
        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeFrameEncoder(_handle);
            _handle = 0;
        }
    }
}
