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
        src/comlz4_block.c \
        src/comlz4_buffer.c \
        src/comlz4_stream.c \
        src/comlz4_frame.c \
        src/comlz4.c \
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
        src/comlz4_block.c \
        src/comlz4_buffer.c \
        src/comlz4_stream.c \
        src/comlz4_frame.c \
        src/comlz4.c \
        vendor/lz4.c \
        vendor/lz4hc.c \
        vendor/lz4frame.c \
        vendor/xxhash.c \
        -Ivendor \
        -o bin/lz4

# Build and run the embedded native smoke tests.
run-native-test: build-native-test
    ./native/bin/lz4

# Remove only generated native artifacts.
clean:
    rm -f native/bin/lz4.swc native/bin/lz4 native/bin/lz4.exe
