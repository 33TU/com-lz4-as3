package com.lz4
{
    import flash.utils.ByteArray;

    import com.lz4._native.beginFrame;
    import com.lz4._native.createFrameEncoder;
    import com.lz4._native.disposeFrameEncoder;
    import com.lz4._native.endFrame;
    import com.lz4._native.updateFrame;

    public final class FrameEncoder
    {
        private var _handle:uint;

        public function FrameEncoder(compressionLevel:int = 0)
        {
            _handle = createFrameEncoder(compressionLevel);
        }

        public function begin(dest:ByteArray):void
        {
            beginFrame(_handle, dest);
        }

        public function update(src:ByteArray, dest:ByteArray):void
        {
            updateFrame(_handle, src, dest);
        }

        public function end(dest:ByteArray):void
        {
            endFrame(_handle, dest);
        }

        public function dispose():void
        {
            if (_handle == 0)
                return;

            disposeFrameEncoder(_handle);
            _handle = 0;
        }
    }
}
