#include "ember/core.h"
#include "ember/core/datastream.h"

#include "ember/core/darray.h"

static inline u64 align_up(u64 value, u64 alignment) {
    // alignment must be a power of two
    return (value + alignment - 1) & ~(alignment - 1);
}

datastream datastream_create(em_allocator* allocator, memory_tag memtag) {
    datastream stream = {};
    stream.allocator = allocator;
    stream.memtag = memtag;

    stream.block_infos = darray_create(datastream_block_info, stream.allocator, stream.memtag);
    return stream;
}

void* datastream_push(datastream* stream, u64 item_size, u64 alignment) {
    u64 offset = align_up(stream->size, alignment);

    if (stream->total_allocated < offset + item_size) {
        u64 new_capacity = stream->total_allocated == 0 ? 64 : stream->total_allocated;
        while (new_capacity < offset + item_size) new_capacity *= 2;

        void* new_buffer = mem_allocate(stream->allocator, new_capacity, stream->memtag);
        if (stream->memory) {
            em_memcpy(
                new_buffer, 
                stream->memory, stream->size);
            mem_free(stream->allocator, stream->memory, stream->total_allocated, stream->memtag);
        }
        
        stream->total_allocated = new_capacity;
        stream->memory           = new_buffer;
    }

    // Advance the cursor to the aligned position (padding bytes are wasted,
    // but the allocator owns the whole buffer so that's fine)
    stream->size = offset + item_size;

    datastream_block_info* block_info = darray_push_empty(stream->block_infos);
    block_info->offset = offset;

    return (u8*)stream->memory + offset;
}

void* datastream_get(const datastream* stream, u32 index) {
    return (u8*)stream->memory + stream->block_infos[index].offset;
}