package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.compressStream;
    import com.lz4._native.createStreamEncoder;
    import com.lz4._native.disposeStreamEncoder;

    public final class StreamEncoder
    {
        private var _handle:uint;

        public function StreamEncoder()
        {
            _handle = createStreamEncoder();
        }

        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        public function compress(
                src:ByteArray,
                dest:ByteArray,
                acceleration:int = 1
            ):void
        {
            compressStream(_handle, src, dest, acceleration);
        }

        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeStreamEncoder(_handle);
            _handle = 0;
        }
    }
}
