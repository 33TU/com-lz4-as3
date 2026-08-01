# COM.LZ4

LZ4 block, streaming, and frame compression for ActionScript 3. The library
combines a CrossBridge native implementation with a small AS3 API and targets
Flash/AIR AVM2 applications.

## Features

- LZ4 block compression and decompression
- Dependent streaming block compression and decompression
- LZ4 frame compression and incremental decompression
- Frame content checksums, verified automatically on decode
- Direct `ByteArray` input and output
- Reusable scratch buffers to reduce native allocations
- Strict, Flash-compatible AS3 output without ABC 47 `float` support
- Vendored LZ4 and xxHash sources under `native/vendor/`

## Public API

| Class | Methods | Purpose |
| --- | --- | --- |
| `Block` | `compress()`, `decompress()` | Stateless, independent LZ4 blocks |
| `StreamEncoder` | `compress()`, `dispose()`, `disposed` | Ordered, dictionary-dependent block compression |
| `StreamDecoder` | `decompress()`, `dispose()`, `disposed` | Ordered, dictionary-dependent block decompression |
| `FrameEncoder` | `begin()`, `update()`, `end()`, `dispose()`, `disposed` | Standard LZ4 frame compression |
| `FrameDecoder` | `decompress()`, `dispose()`, `disposed` | Incremental and size-limited LZ4 frame decompression |

Only classes in `com.lz4` are supported public API. Symbols under
`com.lz4._native` are implementation details used by those classes.

### Choosing a mode

Prefer `FrameEncoder`/`FrameDecoder` unless you have a reason not to. The LZ4
frame format carries its own block boundaries and content checksum, so the
decoder needs nothing but the bytes, and corrupt input is reported rather than
decoded into garbage.

`Block` suits single self-contained payloads whose uncompressed size you already
know.

`StreamEncoder`/`StreamDecoder` give the best ratio on a sequence of related
blocks, but they carry no framing of their own: you must transmit each block's
uncompressed size yourself and present the blocks to the decoder in exactly the
order and boundaries the encoder saw. Getting that wrong yields a generic
failure or silently wrong output, because there is nothing in the payload to
detect it with. Reach for stream mode only when you already have your own
framing.

### ByteArray behavior

Operations read from `src.position` and write at `dest.position`. Successful
calls advance both positions by the number of bytes consumed or produced.
Output is appended when `dest.position == dest.length`. Source and destination
must be different `ByteArray` instances.

`Block.compress()` and `Block.decompress()` return `void`. Read the compressed
size from the destination position or length. Block and stream decompression
require the exact original uncompressed size.

Validate `originalSize` before block or stream decompression when it comes from
an untrusted source, since it controls the native output allocation.

Stream blocks must be decompressed in the same order in which they were
compressed. `FrameDecoder.decompress()` returns `true` when one complete frame
ends. It can consume partial input across calls and leaves bytes belonging to a
following concatenated frame unread.

`FrameDecoder` limits each complete frame to
`FrameDecoder.DEFAULT_MAX_OUTPUT_SIZE` (32 MiB) by default, including output
produced across incremental calls. The decoder buffers a whole frame natively
before writing to `dest`, so this limit also caps its native memory. Supply a
different nonzero limit to the constructor when needed:

```as3
const decoder:FrameDecoder = new FrameDecoder(64 * 1024 * 1024);
```

Frames carry a content checksum that `FrameDecoder` verifies automatically; a
tampered or corrupt frame throws instead of yielding silent garbage. When a
frame is decoded through several incremental calls, output from earlier calls
has already been written to `dest` by the time the trailing checksum is
verified.

`FrameEncoder` cannot be reused after a native error. Every subsequent call
throws `IllegalOperationError`; discard the instance and construct a new one.

Stateful encoders and decoders own native memory. Always call their idempotent
`dispose()` method, preferably from `finally`. The `disposed` getter reports
whether the native handle has been released:

```as3
const decoder:FrameDecoder = new FrameDecoder();

try
{
    const complete:Boolean = decoder.decompress(source, destination);
}
finally
{
    decoder.dispose();
}
```

## Block example

```as3
import com.lz4.Block;

import flash.utils.ByteArray;

const source:ByteArray = new ByteArray();
const compressed:ByteArray = new ByteArray();
const restored:ByteArray = new ByteArray();

source.writeUTFBytes("hello LZ4");
source.position = 0;

Block.compress(source, compressed);

compressed.position = 0;
Block.decompress(compressed, restored, source.length);

trace(restored.toString());
```

Complete standalone examples are available for each mode:

- [`BlockExample.as`](example/BlockExample.as)
- [`StreamExample.as`](example/StreamExample.as)
- [`FrameExample.as`](example/FrameExample.as)

## Build

Requirements:

- [CrossBridge](https://github.com/crossbridge-community/crossbridge), available
  through the `crossbridge` command
- An AIR SDK providing `acompc` and `amxmlc`
- [just](https://github.com/casey/just)

Run `just` without arguments to list the available recipes.

```sh
just build-native       # Build native/bin/lz4.swc with CrossBridge
just build-as3          # Build the combined build/com-lz4-as3.swc
just build-native-test  # Build the executable native smoke test
just run-native-test    # Build and run the native smoke test
just build-as3-test     # Compile the public AS3 API smoke test SWF
just build-examples     # Compile all standalone example SWFs
just clean              # Remove generated build artifacts
```

The distributable library is `build/com-lz4-as3.swc`. It contains both the
native implementation and the public AS3 classes; consumers only need this one
SWC.

Release AS3 builds use strict type checking, disable debug information, and set
`compiler.float=false` for compatibility with Flash targets that do not support
AIR's ABC 47 `float` type.

## Tests

The native smoke test exercises block round trips, dependent stream blocks,
stream dictionary retention, aliased buffer rejection, frame output limits,
frame state ordering, complete and incremental frame decoding, concatenated
frames, corrupt input, content checksum enforcement, and decoder recovery:

```sh
just run-native-test
```

The public API smoke test is [`Lz4Test.as`](as3/test/Lz4Test.as). Compile it with
`just build-as3-test`; the resulting SWF is
`build/com-lz4-as3-test.swf`.

## TODO

- Frame dictionaries
- Frame metadata APIs where useful
- Randomized and allocation-failure tests

## License

This project integrates the official [LZ4](https://github.com/lz4/lz4) and
[xxHash](https://github.com/Cyan4973/xxHash) libraries, which are BSD-licensed.
