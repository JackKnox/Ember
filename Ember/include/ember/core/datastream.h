#pragma once

#include "ember/core.h"

typedef struct datastream_block_info {
    u64 offset;
} datastream_block_info;

typedef struct datastream {
    memory_tag memtag;
    ember_allocator* allocator;
    void* memory;
    
    u64 total_allocated, size;
    datastream_block_info* block_infos;
} datastream;

datastream datastream_create(ember_allocator* allocator, memory_tag memtag);
void* datastream_push(datastream* stream, u64 item_size);
void* datastream_get(const datastream* stream, u32 index);