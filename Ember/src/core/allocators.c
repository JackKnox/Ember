#include "ember/core.h"
#include "ember/core/allocators.h"

#include "ember/core/darray.h"

#define FREELIST_ALLOC_MAGIC 0xAE

typedef struct freelist_header {
    u64 capacity, size;
    void* memory;
} freelist_header;

typedef struct freelist_block_header {
    u64 size;
} freelist_block_header;

void* freelist_alloc(ember_allocator* allocator, u64 size, u64 alignment) {
    if (!allocator->user_data) {
        // Create allocator data.
        allocator->user_data = mem_allocate(allocator->parent, sizeof(freelist_header), MEMORY_TAG_CORE);
    }

    freelist_header* hdr = (freelist_header*)allocator->user_data;

    // Capacity < size check... resize
    u64 required_size = sizeof(freelist_block_header) + size;
    if (hdr->capacity < hdr->size + required_size) {
        u64 new_capacity = hdr->capacity + 1;
        while (new_capacity < hdr->size + required_size) new_capacity *= 2;

        void* new_buffer = mem_allocate(allocator->parent, new_capacity, MEMORY_TAG_CORE);
        if (hdr->memory) {
            em_memcpy(
                new_buffer, 
                hdr->memory, hdr->size);
            mem_free(allocator->parent, hdr->memory, hdr->capacity, MEMORY_TAG_CORE);
        }

        hdr->capacity = new_capacity;
        hdr->memory = new_buffer;
    }

    freelist_block_header* new_block = (freelist_block_header*)((u8*)hdr->memory + hdr->size);
    new_block->size = size;
    hdr->size += required_size;
    return (void*)(new_block + 1);
}

void freelist_free(ember_allocator* allocator, void* block, u64 size, u64 alignment) {
    if (!allocator->user_data) return;
}

ember_allocator freelist_allocator(ember_allocator* parent) {
    ember_allocator allocator = {};
    allocator.alloc = freelist_alloc;
    allocator.free = freelist_free;
    allocator.parent = NULL;
    
#if !EMBER_DIST
    allocator.magic = FREELIST_ALLOC_MAGIC;
#endif
    return allocator;
}

b8 freelist_next_block(const ember_allocator* allocator, void** cursor) {
    EM_ASSERT(allocator->magic == FREELIST_ALLOC_MAGIC && "Mismatched allocator functions");

    freelist_header* hdr = (freelist_header*)allocator->user_data;
    freelist_block_header* next_block = hdr->memory;

    if (*cursor != 0) {
        freelist_block_header* current_hdr = ((freelist_block_header*)*cursor) - 1;
        next_block = (freelist_block_header*)((u8*)(*cursor) + current_hdr->size);
    }

    if ((u64)(next_block + 1) > (u64)hdr->memory + hdr->size)
        return FALSE;
    
    *cursor = next_block + 1;
    return TRUE;
}