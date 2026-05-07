#pragma once

#include "ember/core.h"

u64 hash_bytes(void* ptr, u64 size);

void* _hashmap_create(u64 stride, em_allocator* allocator, memory_tag memtag);
void hashmap_destroy(void* hashmap);

b8 hashmap_get(void* hashmap, u64 hash, void* out_item);
void hashmap_set(void* hashmap, u64 hash, void* in_item);

u64 hashmap_length(void* hashmap);

#define hashmap_create(type, allocator, memtag) _hashmap_create(sizeof(type), allocator, memtag)