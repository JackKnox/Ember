#pragma once

#include "ember/core.h"

typedef struct darray_header {
    memory_tag memtag;
    u32 capacity, length;
    u64 stride;
} darray_header;

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

void*          _darray_create(u64 stride, u32 capacity, memory_tag memtag);
void*          _darry_from_data(u64 stride, u32 length, void* from_data, memory_tag memtag);
void*          _darray_push(void** out_array, void* value_ptr);
darray_header* _darray_header(void* array);

void           darray_destroy(void* array);
void           darray_pop_at(void* array, u32 index, void* dest);

#define darray_create(type, memtag) \
    _darray_create(sizeof(type), DARRAY_DEFAULT_CAPACITY, memtag)

#define darray_from_data(type, length, from_data, memtag) \
    _darry_from_data(sizeof(type), length, from_data, memtag)

#define darray_reserve(type, capacity, memtag) \
     _darray_create(sizeof(type), capacity, memtag)

#define darray_push(array, value)          \
    do {                                   \
        __typeof__(*array) _tmp = (value); \
        _darray_push((void**) &(array), &_tmp); \
    } while (0)

#define darray_push_empty(array) \
    _darray_push((void**) &(array), NULL)

#define darray_clear(array) \
    _darray_header(array)->length = 0

#define darray_length(array) \
    (_darray_header(array)->length)

#define darray_capacity(array) \
    (_darray_header(array)->capacity)

#define darray_stride(array) \
    (_darray_header(array)->stride)

typedef struct freelist {
    memory_tag tag;
    u64 capacity, size;
    void* memory;
} freelist;

void  freelist_create(u64 start_size, memory_tag tag, freelist* out_list);
void  freelist_destroy(freelist* list);

void  freelist_reset(freelist* list, b8 zero_memory, b8 free_memory);
void  freelist_resize(freelist* list, u64 new_size);

void* freelist_push(freelist* list, u64 block_size, void* memory);
void* freelist_get(freelist* list, u64 index);
b8    freelist_next_block(freelist* list, u8** cursor);

u64   freelist_size(freelist* list);
u64   freelist_capacity(freelist* list);