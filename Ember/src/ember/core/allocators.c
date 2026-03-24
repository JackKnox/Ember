#include "defines.h"
#include "allocators.h"

#include "ember/platform/global.h"

#pragma pack(push, 1)
typedef struct freelist_header {
    u64 payload_size;
} freelist_header;
#pragma pack(pop)

void* get_user_memory(freelist* list, void* internal_block) {
    EM_ASSERT(list != NULL && internal_block != NULL && "Invalid arguments passed to get_user_memory");
    return (u8*)internal_block + sizeof(freelist_header);
}

void freelist_create(u64 start_size, memory_tag tag, freelist *out_list) {
    EM_ASSERT(out_list != NULL && "Invalid arguments passed to freelist_create");

    out_list->memory = NULL;
    out_list->size = 0;
    out_list->capacity = 0;
    out_list->tag = tag;

    if (start_size > 0) {
        freelist_resize(out_list, start_size);
    }
}

b8 freelist_empty(freelist* list) {
    return list == NULL || list->capacity == 0;
}

void freelist_destroy(freelist* list) {
    EM_ASSERT(list != NULL && "Invalid arguments passed to freelist_destroy");

    if (list->memory) {
        EM_ASSERT(list->capacity > 0 && "Allocated memory in freelist but not recorded capacity");
        bfree(list->memory, list->capacity, list->tag);
        list->memory = NULL;
    }

    list->capacity = 0;
    list->size = 0;
}

u64 freelist_size(freelist* list) {
    EM_ASSERT(list != NULL && list->memory != NULL && "Invalid arguments passed to freelist_size");
    return list->size;
}

u64 freelist_capacity(freelist* list) {
    EM_ASSERT(list != NULL && list->memory != NULL && "Invalid arguments passed to freelist_capacity");
    return list->capacity;
}

void freelist_resize(freelist* list, u64 new_size) {
    EM_ASSERT(list != NULL && "Invalid arguments passed to freelist_resize");

    if (list->memory == NULL || new_size > list->capacity) {
        u64 new_capacity = (list->capacity == 0) ? 8 : list->capacity;
        while (new_capacity < new_size) new_capacity *= 2;

        void* new_buffer = ballocate(new_capacity, list->tag);

        if (list->memory) {
            // copy only the used bytes
            bcopy_memory(new_buffer, list->memory, list->size);
            bfree(list->memory, list->capacity, list->tag);
        }

        list->memory = new_buffer;
        list->capacity = new_capacity;
    }
}

void freelist_reset(freelist* list, b8 zero_memory, b8 free_memory) {
    EM_ASSERT(list != NULL && list->memory != NULL && "Invalid arguments passed to freelist_reset");
    list->size = 0;

    if (free_memory) {
        bfree(list->memory, list->capacity, list->tag);
        list->memory = NULL;
        list->capacity = 0;
        return;
    }

    if (zero_memory) {
        bzero_memory(list->memory, list->capacity);
    }
}

void* freelist_push(freelist* list, u64 block_size, void* memory) {
    EM_ASSERT(list != NULL && list->memory != NULL && block_size > 0 && "Invalid arguments passed to freelist_push");

    u64 aligned_pos = alignment(list->size, 8ULL);
    u64 padding = aligned_pos - list->size;

    u64 required = padding + sizeof(freelist_header) + block_size;
    freelist_resize(list, list->size + required);

    if (!list->memory) return NULL;

    u8* internal_block = (u8*)list->memory + aligned_pos;
    ((freelist_header*)internal_block)->payload_size = block_size;

    u8* user_ptr = internal_block + sizeof(freelist_header);

    if (memory != NULL) {
        bcopy_memory(user_ptr, memory, block_size);
    }

    list->size = aligned_pos + sizeof(freelist_header) + block_size;
    return (void*)user_ptr;
}

void* freelist_get(freelist* list, u64 index) {
    EM_ASSERT(list != NULL && list->memory != NULL && "Invalid arguments passed to freelist_get");

    u64 pos = 0;
    for (u64 i = 0; i < index; ++i) {
        if (pos + sizeof(freelist_header) > list->size) return NULL; // out of range / corrupted
        freelist_header* hdr = (freelist_header*)((u8*)list->memory + pos);
        if (hdr->payload_size > list->capacity) return NULL;
        pos += sizeof(freelist_header) + hdr->payload_size;
        pos = alignment(pos, 8ULL);
    }

    if (pos + sizeof(freelist_header) > list->size) return NULL;
    return get_user_memory(list, (u8*)list->memory + pos);
}

b8 freelist_next_block(freelist* list, u8** cursor) {
    EM_ASSERT(list != NULL && cursor != NULL && list->memory != NULL && "Invalid arguments passed to freelist_next_block");
    u8* next_block_header = 0;

    if (*cursor == 0) {
        next_block_header = list->memory;
    }
    else {
        freelist_header* current_hdr = (freelist_header*)(*cursor - sizeof(freelist_header));
        next_block_header = *cursor + current_hdr->payload_size;
    }

    u64 memory_limit = (u64)list->memory + list->size;
    if ((u64)next_block_header + sizeof(freelist_header) > memory_limit) {
        return FALSE;
    }

    *cursor = (u8*)alignment((u64)next_block_header + sizeof(freelist_header), 8ULL);
    return TRUE;
}

void* _darray_create(u64 stride, u32 capacity, memory_tag memtag) {
    u64 size = stride * capacity;
    darray_header* new_array = ballocate(sizeof(darray_header) + size, memtag);
    new_array->capacity = capacity;
    new_array->length = 0;
    new_array->memtag = memtag;
    new_array->stride = stride;
    return bzero_memory(new_array + 1, size);
}

void* _darry_from_data(u64 stride, u32 length, void* from_data, memory_tag memtag) {
    void* new_array = _darray_create(stride, length, memtag);
    _darray_header(new_array)->length = length;

    if (from_data)
        bcopy_memory(new_array, from_data, stride * length);
    return new_array;
}

void darray_destroy(void* array) {
    darray_header* hdr = _darray_header(array);
    bfree(hdr, sizeof(darray_header) + hdr->stride * hdr->capacity, hdr->memtag);
}

void* _darray_push(void** out_array, void* value_ptr) {
    darray_header* hdr = _darray_header(*out_array);

    // Resize if needed
    if (hdr->length + 1 > hdr->capacity) {
        u64 new_capacity = (hdr->capacity ? hdr->capacity * DARRAY_RESIZE_FACTOR : DARRAY_DEFAULT_CAPACITY);

        void* new_array = _darray_create(hdr->stride, new_capacity, hdr->memtag);
        bcopy_memory(new_array, *out_array, hdr->length * hdr->stride);

        _darray_header(new_array)->length = hdr->length;
        darray_destroy(*out_array);
        
        *out_array = new_array;
        hdr = _darray_header(*out_array);
    }

    // Write new element
    void* addr = (u8*)(*out_array) + (hdr->length * hdr->stride);
    if (value_ptr)
        bcopy_memory(addr, value_ptr, hdr->stride);

    hdr->length += 1;
    return addr;
}

void darray_pop_at(void* array, u32 index, void* dest) {
    darray_header* hdr = _darray_header(array);

    u8* addr = (u8*)array + index * hdr->stride;
    if (dest) bcopy_memory(dest, (void*)addr, hdr->stride);

    if (index != hdr->length - 1) {
        bcopy_memory(
            addr,
            addr + hdr->stride,
            hdr->stride * (hdr->length - index));
    }

    hdr->length -= 1;
}

darray_header* _darray_header(void* array) {
    return ((darray_header*)array) - 1;
}