package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.createFrameDecoder;
    import com.lz4._native.decompressFrame;
    import com.lz4._native.disposeFrameDecoder;

    public final class FrameDecoder
    {
        public static const DEFAULT_MAX_OUTPUT_SIZE:uint = 32 * 1024 * 1024;

        private var _handle:uint;

        public function FrameDecoder(
                maxOutputSize:uint = DEFAULT_MAX_OUTPUT_SIZE
            )
        {
            _handle = createFrameDecoder(maxOutputSize);
        }

        public function get disposed():Boolean
        {
            return _handle == 0;
        }

        public function decompress(src:ByteArray, dest:ByteArray):Boolean
        {
            return decompressFrame(_handle, src, dest);
        }

        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeFrameDecoder(_handle);
            _handle = 0;
        }
    }
}
