#include "ember/core.h"
#include "ember/core/datastream.h"

#include "ember/core/darray.h"

datastream datastream_create(ember_allocator* allocator, memory_tag memtag) {
    datastream stream = {};
    stream.allocator = allocator;
    stream.memtag = memtag;

    stream.block_infos = darray_create(datastream_block_info, stream.allocator, stream.memtag);
    return stream;
}

void* datastream_push(datastream* stream, u64 item_size) {
    if (stream->total_allocated < stream->size + item_size) {
        u64 new_capacity = stream->total_allocated + 1;
        while (new_capacity < stream->size + item_size) new_capacity *= 2;

        void* new_buffer = mem_allocate(stream->allocator, new_capacity, stream->memtag);
        if (stream->memory) {
            em_memcpy(
                new_buffer, 
                stream->memory, stream->size);
            mem_free(stream->allocator, stream->memory, stream->total_allocated, stream->memtag);
        }

        stream->total_allocated = new_capacity;
        stream->memory = new_buffer;
    }

    datastream_block_info* block_info = darray_push_empty(stream->block_infos);
    block_info->offset = stream->size;

    stream->size += item_size;
    return (void*)((u8*)stream->memory + stream->size - item_size);
}

void* datastream_get(const datastream* stream, u32 index) {
    return (u8*)stream->memory + stream->block_infos[index].offset;
}