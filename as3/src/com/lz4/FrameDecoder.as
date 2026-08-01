package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.createFrameDecoder;
    import com.lz4._native.decompressFrame;
    import com.lz4._native.disposeFrameDecoder;

    public final class FrameDecoder
    {
        private var _handle:uint;

        public function FrameDecoder()
        {
            _handle = createFrameDecoder();
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
