#pragma once

#include "ember/core.h"

#define FNV_OFFSET_BASIS 1469598103934665603ULL

#define FNV_PRIME_NUMBER 1099511628211ULL

u64 hash_bytes(void* ptr, u64 size);

void* _hashmap_create(u64 stride, em_allocator* allocator, memory_tag memtag);
void hashmap_destroy(void* hashmap);

b8 hashmap_get_index(void* hashmap, u64 idx, void* out_item);
b8 hashmap_get(void* hashmap, u64 hash, void* out_item);
void hashmap_set(void* hashmap, u64 hash, void* in_item);

u64 hashmap_length(void* hashmap);

#define hashmap_create(type, allocator, memtag) _hashmap_create(sizeof(type), allocator, memtag)