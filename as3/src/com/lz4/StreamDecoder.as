package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.createStreamDecoder;
    import com.lz4._native.decompressStream;
    import com.lz4._native.disposeStreamDecoder;

    public final class StreamDecoder
    {
        private var _handle:uint;

        public function StreamDecoder()
        {
            _handle = createStreamDecoder();
        }

        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        public function decompress(
                src:ByteArray,
                dest:ByteArray,
                originalSize:uint
            ):void
        {
            decompressStream(_handle, src, dest, originalSize);
        }

        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeStreamDecoder(_handle);
            _handle = 0;
        }
    }
}
