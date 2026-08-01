# COM.LZ4

`COM.LZ4` provides LZ4 compression and decompression bindings for **ActionScript
3 (AVM2)**, built using a native SWC. It currently implements **block** and
**stream** operations from `lz4.h`, with partial support for `lz4frame.h`
compression.

---

⚡ **Performance:**\
`COM.LZ4` is **multiple times faster** than any built-in Flash/AS3 compression
methods\
(such as `ByteArray.compress()` using zlib or deflate).\
This makes it ideal for real-time data streaming, multiplayer games, and large
asset handling.

---

## Features

- ✅ Block-level compression and decompression
- ✅ Stream compression and decompression support
- ✅ Frame compression support
- ✅ Integration with `ByteArray` for seamless AS3 use
- ✅ Built from vendored LZ4 sources (`native/vendor/`)

---

## Example

```as3
package
{
    import flash.display.Sprite;
    import flash.utils.ByteArray;
    import com.lz4.*;

    public class Main extends Sprite
    {
        public function Main()
        {
            testLibBlockOps();
        }

        private function testLibBlockOps():void
        {
            function repeatString(str:String, times:int):String
            {
                return new Array(times + 1).join(str);
            }

            var a:ByteArray = new ByteArray();
            var b:ByteArray = new ByteArray();
            var c:ByteArray = new ByteArray();

            var input:String = repeatString("hello world! ", 10000);
            a.writeUTFBytes(input);
            a.position = 0;

            // Call native functions from com.lz4 SWC
            var compressedSize:int = com.lz4.compress(a, b, 5);
            var decompressedSize:int = com.lz4.decompress(b, c, a.length);

            trace("raw size: " + a.length);
            trace("compressed size: " + compressedSize);
            trace("decompressed length: " + decompressedSize);
            trace("data equals: " + (c.toString() == input));
        }
    }
}
```

---

## Build

Requirements:

- [CrossBridge](https://github.com/crossbridge-community/crossbridge), available
  through the `crossbridge` command
- [just](https://github.com/casey/just)

```sh
just build-native       # Build the native com.lz4 SWC
just build-native-test  # Build native/bin/lz4 with embedded smoke tests
just run-native-test    # Build and run the embedded smoke tests
just clean              # Remove generated native artifacts
```

Output files are placed in `native/bin/`.

---

## TODO

Implement **frame decompression** support from `lz4frame.h`, as many third-party
LZ4 libraries use framed streams for cross-platform compatibility.

Frame decompression requires additional logic - specifically, polling or
checking when a `ByteArray` is ready to be processed.

Planned API additions:

```as3
// Check if the frame data in src is ready to decompress
function frameIsReady(frameHandlePtr:uint, src:ByteArray):int
// Returns:
//   >0  -> number of bytes to be consumed before ready
//   <0  -> invalid data
//   0   -> not ready yet

// Get frame length (if available)
function frameLength(frameHandlePtr:uint, src:ByteArray):int

// Decompress a complete frame into dest
function decompressFrame(frameHandlePtr:uint, src:ByteArray, dest:ByteArray):int
// Ensures src.position matches frameLength when done

// Compress a frame from src into dest
function compressFrame(frameHandlePtr:uint, src:ByteArray, dest:ByteArray):int
```

---

## License

This project integrates the official [LZ4](https://github.com/lz4/lz4) and
[XXHASH](https://github.com/Cyan4973/xxHash) libary which are BSD-licensed.




