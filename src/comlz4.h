#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <AS3/AS3.h>
#include <lz4.h>
#include <lz4frame.h>

// Function definitions //
void compress(void) __attribute__((used,
                                   annotate("as3sig:public function compress(src:ByteArray, dest:ByteArray, acceleration:int = 1):int"),
                                   annotate("as3import:flash.utils.ByteArray"),
                                   annotate("as3package:com.lz4")));

void decompress(void) __attribute__((used,
                                     annotate("as3sig:public function decompress(src:ByteArray, dest:ByteArray, originalSize:uint):int"),
                                     annotate("as3import:flash.utils.ByteArray"),
                                     annotate("as3package:com.lz4")));

void init_stream(void) __attribute__((used,
                                      annotate("as3sig:public function initStream():uint"),
                                      annotate("as3package:com.lz4")));

void free_stream(void) __attribute__((used,
                                      annotate("as3sig:public function freeStream(handlePtr:uint):uint"),
                                      annotate("as3package:com.lz4")));

void init_stream_decode(void) __attribute__((used,
                                             annotate("as3sig:public function initStreamDecode():uint"),
                                             annotate("as3package:com.lz4")));

void free_stream_decode(void) __attribute__((used,
                                             annotate("as3sig:public function freeStreamDecode(handlePtr:uint):uint"),
                                             annotate("as3package:com.lz4")));

void compress_stream(void) __attribute__((used,
                                          annotate("as3sig:public function compressStream(handlePtr:uint, src:ByteArray, dest:ByteArray, acceleration:int = 0):int"),
                                          annotate("as3import:flash.utils.ByteArray"),
                                          annotate("as3package:com.lz4")));

void decompress_stream(void) __attribute__((used,
                                            annotate("as3sig:public function decompressStream(handlePtr:uint, src:ByteArray, dest:ByteArray, originalSize:uint):int"),
                                            annotate("as3import:flash.utils.ByteArray"),
                                            annotate("as3package:com.lz4")));

void init_frame_compressor(void) __attribute__((used,
                                                annotate("as3sig:public function initFrameCompressor(compressionLevel:int = 0):uint"),
                                                annotate("as3package:com.lz4")));

void free_frame_compressor(void) __attribute__((used,
                                                annotate("as3sig:public function freeFrameCompressor(handlePtr:uint):uint"),
                                                annotate("as3package:com.lz4")));

void frame_compress_begin(void) __attribute__((used,
                                               annotate("as3sig:public function compressFrameBegin(handlePtr:uint, dest:ByteArray):int"),
                                               annotate("as3import:flash.utils.ByteArray"),
                                               annotate("as3package:com.lz4")));

void frame_compress_update(void) __attribute__((used,
                                                annotate("as3sig:public function compressFrameUpdate(handlePtr:uint, src:ByteArray, dest:ByteArray):int"),
                                                annotate("as3import:flash.utils.ByteArray"),
                                                annotate("as3package:com.lz4")));

void frame_compress_end(void) __attribute__((used,
                                             annotate("as3sig:public function compressFrameEnd(handlePtr:uint, dest:ByteArray):int"),
                                             annotate("as3import:flash.utils.ByteArray"),
                                             annotate("as3package:com.lz4")));

void buffer_release(void) __attribute__((used,
                                         annotate("as3sig:public function bufferRelease():void"),
                                         annotate("as3package:com.lz4")));

// Macros //
#define HANDLE_ERROR(msg)  \
    do                     \
    {                      \
        AS3_Trace(msg);    \
        goto _err_cleanup; \
    } while (0)

#define NULL_CHECK(ptr)                           \
    do                                            \
    {                                             \
        if (!ptr)                                 \
            HANDLE_ERROR("memory not allocated"); \
    } while (0)

// Buffer reusing //

char *buffer_reserve(size_t size);
size_t buffer_capacity(void);
void buffer_release(void);