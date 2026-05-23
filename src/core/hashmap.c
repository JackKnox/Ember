#include "ember/core.h"
#include "ember/core/hashmap.h"

#include "ember/core/darray.h"

typedef struct hashmap_entry {
    u64 hash;
    b8 occupied;
    // followed by value bytes (stride)
} hashmap_entry;

typedef struct hashmap_header {
    u64 capacity;
    u64 length;
    u64 stride;
    em_allocator* allocator;
    memory_tag memtag;
} hashmap_header;

#define HASHMAP_INITIAL_CAPACITY 16
#define HASHMAP_LOAD_FACTOR 0.7f

static hashmap_header* hashmap_get_header(void* map) {
    return (hashmap_header*)((u8*)map - sizeof(hashmap_header));
}

static hashmap_entry* hashmap_get_entry(hashmap_header* hdr, u64 index) {
    u8* base = (u8*)(hdr + 1);
    return (hashmap_entry*)(base + index * (sizeof(hashmap_entry) + hdr->stride));
}

static void* hashmap_get_value(hashmap_entry* entry) {
    return (void*)(entry + 1);
}

static void hashmap_resize(void** map_ptr, u64 new_capacity) {
    void* old_map = *map_ptr;
    hashmap_header* old_hdr = hashmap_get_header(old_map);

    void* new_map = _hashmap_create(old_hdr->stride, old_hdr->allocator, old_hdr->memtag);
    hashmap_header* new_hdr = hashmap_get_header(new_map);

    new_hdr->capacity = new_capacity;

    for (u64 i = 0; i < old_hdr->capacity; i++) {
        hashmap_entry* entry = hashmap_get_entry(old_hdr, i);
        if (entry->occupied) {
            hashmap_set(new_map, entry->hash, hashmap_get_value(entry));
        }
    }

    hashmap_destroy(old_map);
    *map_ptr = new_map;
}

u64 hash_bytes(void* ptr, u64 size) {
    u8* data = (u8*)ptr;
    u64 hash = FNV_OFFSET_BASIS;

    for (u64 i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= FNV_PRIME_NUMBER;
    }

    return hash;
}

void* _hashmap_create(u64 stride, em_allocator* allocator, memory_tag memtag) {
    u64 capacity = HASHMAP_INITIAL_CAPACITY;

    u64 total_size = sizeof(hashmap_header) +
        capacity * (sizeof(hashmap_entry) + stride);

    hashmap_header* hdr = mem_allocate(allocator, total_size, memtag);

    hdr->capacity = capacity;
    hdr->length = 0;
    hdr->stride = stride;
    hdr->allocator = allocator;
    hdr->memtag = memtag;

    void* map = (void*)(hdr + 1);

    // zero entries
    em_memset(map, 0, capacity * (sizeof(hashmap_entry) + stride));

    return map;
}

void hashmap_destroy(void* hashmap) {
    if (!hashmap) return;

    hashmap_header* hdr = hashmap_get_header(hashmap);
    mem_free(hdr->allocator, hdr, sizeof(hashmap_header) + hdr->capacity * (sizeof(hashmap_entry) + hdr->stride), hdr->memtag);
}

b8 hashmap_get_index(void* hashmap, u64 idx, void* out_item) {
    hashmap_header* hdr = hashmap_get_header(hashmap);

    hashmap_entry* entry = hashmap_get_entry(hdr, idx);
    if (!entry->occupied) return FALSE;

    em_memcpy(out_item, hashmap_get_value(entry), hdr->stride);
    return TRUE;
}

b8 hashmap_get(void* hashmap, u64 hash, void* out_item) {
    hashmap_header* hdr = hashmap_get_header(hashmap);

    u64 index = hash % hdr->capacity;

    for (u64 i = 0; i < hdr->capacity; i++) {
        hashmap_entry* entry = hashmap_get_entry(hdr, index);

        if (!entry->occupied) {
            return FALSE;
        }

        if (entry->hash == hash) {
            em_memcpy(out_item, hashmap_get_value(entry), hdr->stride);
            return TRUE;
        }

        index = (index + 1) % hdr->capacity;
    }

    return FALSE;
}

void hashmap_set(void* hashmap, u64 hash, void* in_item) {
    hashmap_header* hdr = hashmap_get_header(hashmap);

    // Resize if needed
    if ((f32)(hdr->length + 1) / (f32)hdr->capacity > HASHMAP_LOAD_FACTOR) {
        hashmap_resize(&hashmap, hdr->capacity * 2);
        hdr = hashmap_get_header(hashmap);
    }

    u64 index = hash % hdr->capacity;

    for (u64 i = 0; i < hdr->capacity; i++) {
        hashmap_entry* entry = hashmap_get_entry(hdr, index);

        if (!entry->occupied || entry->hash == hash) {
            if (!entry->occupied) {
                hdr->length++;
            }

            entry->occupied = TRUE;
            entry->hash = hash;
            em_memcpy(hashmap_get_value(entry), in_item, hdr->stride);
            return;
        }

        index = (index + 1) % hdr->capacity;
    }
}

u64 hashmap_length(void* hashmap) {
    hashmap_header* hdr = hashmap_get_header(hashmap);
    return hdr->length;
}