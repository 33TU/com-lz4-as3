@echo off
REM Build LZ4 sample with static linking and optimization

gcc -DDOTEST ^
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
    -o bin\lz4.exe

echo.
echo Build complete: bin\lz4.exe
