default:
    @just --list

# Build the native com.lz4 SWC.
build-native:
    mkdir -p native/bin
    cd native && crossbridge gcc \
        -emit-swc="com.lz4" \
        -flto-api=exports.txt \
        -fllvm-opt-opt=-strip \
        -disable-telemetry \
        -O4 \
        -DNDEBUG \
        src/block.c \
        src/frame.c \
        src/scratch.c \
        src/stream.c \
        src/module.c \
        vendor/lz4.c \
        vendor/lz4hc.c \
        vendor/lz4frame.c \
        vendor/xxhash.c \
        -Ivendor \
        -o bin/lz4.swc

# Build the native executable containing the embedded smoke tests.
build-native-test:
    mkdir -p native/bin
    cd native && crossbridge gcc \
        -DDOTEST \
        -flto-api=exports.txt \
        -fllvm-opt-opt=-strip \
        -disable-telemetry \
        -O4 \
        -DNDEBUG \
        src/block.c \
        src/frame.c \
        src/scratch.c \
        src/stream.c \
        test/smoke.c \
        vendor/lz4.c \
        vendor/lz4hc.c \
        vendor/lz4frame.c \
        vendor/xxhash.c \
        -Ivendor \
        -o bin/lz4

# Build and run the embedded native smoke tests.
run-native-test: build-native-test
    ./native/bin/lz4

# Build one distributable SWC containing the native code and AS3 API.
build-as3: build-native
    mkdir -p build
    acompc \
        -include-sources as3/src \
        -include-libraries native/bin/lz4.swc \
        -output build/com-lz4-as3.swc \
        -compiler.strict=true \
        -compiler.float=false \
        -debug=false

# Compile the public AS3 API smoke test.
build-as3-test: build-as3
    amxmlc \
        -library-path+=build/com-lz4-as3.swc \
        -output build/com-lz4-as3-test.swf \
        -compiler.strict=true \
        -compiler.float=false \
        -debug=true \
        as3/test/Lz4Test.as

# Compile the standalone block, stream, and frame examples.
build-examples: build-as3
    mkdir -p build/examples
    amxmlc \
        -library-path+=build/com-lz4-as3.swc \
        -output build/examples/BlockExample.swf \
        -compiler.strict=true \
        -compiler.float=false \
        -debug=true \
        example/BlockExample.as
    amxmlc \
        -library-path+=build/com-lz4-as3.swc \
        -output build/examples/StreamExample.swf \
        -compiler.strict=true \
        -compiler.float=false \
        -debug=true \
        example/StreamExample.as
    amxmlc \
        -library-path+=build/com-lz4-as3.swc \
        -output build/examples/FrameExample.swf \
        -compiler.strict=true \
        -compiler.float=false \
        -debug=true \
        example/FrameExample.as

# Remove generated build artifacts.
clean:
    rm -f native/bin/lz4.swc native/bin/lz4 native/bin/lz4.exe build/com-lz4-as3.swc build/com-lz4-as3-test.swf build/examples/BlockExample.swf build/examples/StreamExample.swf build/examples/FrameExample.swf
