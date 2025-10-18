@echo off
REM Build SWC version of LZ4 sample

gcc -emit-swc="com.lz4" ^
    -flto-api=exports.txt ^
    -fllvm-opt-opt=-strip ^
    -disable-telemetry ^
    -O4 ^
    src\comlz4_block.c ^
    src\comlz4_stream.c ^
    src\comlz4_frame.c ^
    src\comlz4.c ^
    lib\lz4.c ^
    lib\lz4hc.c ^
    lib\lz4frame.c ^
    lib\xxhash.c ^
    -Ilib ^
    -o bin\lz4.swc

echo.
echo Build complete: bin\lz4.swc
