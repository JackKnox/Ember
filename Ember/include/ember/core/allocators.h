#pragma once

#include "ember/core.h"

typedef struct freelist_header {
    memory_tag tag;
    u64 size;
} freelist_header;

ember_allocator  freelist_allocator(ember_allocator* top_allocator);

b8               freelist_next_block(const ember_allocator* allocator, void** cursor);
freelist_header* _freelist_header(ember_allocator* allocator);
