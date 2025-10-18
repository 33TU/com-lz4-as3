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
- ✅ Stream compression support
- ✅ Integration with `ByteArray` for seamless AS3 use
- ✅ Built from native LZ4 sources (`lib/`)

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

## Example

```as3
var input:ByteArray = new ByteArray();
input.writeUTFBytes("hello world! hello world! hello world!");
input.position = 0;

var compressed:ByteArray = new ByteArray();
var decompressed:ByteArray = new ByteArray();

var compressedSize:int = com.lz4.compress(input, compressed, 5);
var decompressedSize:int = com.lz4.decompress(compressed, decompressed, input.length);

trace("compressed:", compressedSize, "bytes");
trace("decompressed:", decompressedSize, "bytes");
trace("equal:", decompressed.toString() == input.toString());
```

---

## Build

```bat
build-exe.bat => builds native EXE test
build-swc.bat => builds SWC (com.lz4)
```

Output files are placed in `/bin`.

---

## License

This project integrates the official [LZ4](https://github.com/lz4/lz4) and
[XXHASH](https://github.com/Cyan4973/xxHash) libary which are BSD-licensed.
