#pragma once

#include "ember/core.h"

/**
 * @brief Value used to poison memory in debug builds.
 *
 * Typically written into freed or uninitialized memory to help detect
 * use-after-free and uninitialized access bugs.
 */
#define MEMORY_POISON_VALUE 0xfe

struct em_allocator;

/**
 * @brief Function pointer type for custom memory allocation.
 *
 * @param allocator Allocator instance.
 * @param size Number of bytes to allocate.
 * @param alignment Required memory alignment.
 *
 * @return Pointer to allocated memory, or NULL on failure.
 */
typedef void* (*PFN_allocate_mem)(struct em_allocator* allocator, u64 size, u64 alignment);

/**
 * @brief Function pointer type for custom memory deallocation.
 *
 * @param allocator Allocator instance.
 * @param block Pointer to memory block to free.
 * @param size Original allocation size.
 * @param alignment Alignment used during allocation.
 */
typedef void (*PFN_free_mem)(struct em_allocator* allocator, void* block, u64 size, u64 alignment);

/**
 * @brief Function pointer type for custom memory reallocation.
 * 
 * @param allocator Allocator instance.
 * @param block Pointer to memory block to reallocate.
 * @param new_size New allocation size.
 * @param alignment Alignment used during allocation.
 */
typedef void* (*PFN_reallocate_mem)(struct em_allocator* allocator, void* block, u64 new_size, u64 alignment);

/**
 * @brief Generic allocator interface used across the library.
 *
 * Allows pluggable memory systems (default malloc, arena allocators, etc.).
 */
typedef struct em_allocator {
	/** @brief Allocation function. */
    PFN_allocate_mem alloc;

	/** @brief Reallocation function. */
	PFN_reallocate_mem realloc;

	/** @brief Deallocation function. */
    PFN_free_mem free;

	/** @brief Opaque context per allocator. */     
    void* user_data;       

    /** @brief Optional parent allocator used for backing allocations. */
	struct em_allocator* parent;

	/** @brief Debug-only validation marker for allocator integrity */
    u8 magic;
} em_allocator;

#ifdef EMBER_DEFINE_HELPERS

/**
 * @brief Aligns a value to the specified alignment.
 *
 * @param v Value to align.
 * @param alignment Alignment in bytes (must be a power of two).
 * @return Aligned value.
 */
u64 alignment_ptr(u64 v, u64 alignment);

#endif

/**
 * @brief Allocates memory from the library allocator system.
 *
 * @param allocator Allocator instance.
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 * 
 * @note If @p allocator is NULL, allocates using the default system allocator.
 */
void* mem_allocate(em_allocator* allocator, u64 size);

/**
 * @brief Frees memory allocated with mem_allocate.
 *
 * @param allocator Allocator instance.
 * @param block Pointer to memory block.
 * @param size Original allocation size.
 * 
 * @note If @p allocator is NULL, frees using the default system allocator.
 */
void mem_free(em_allocator* allocator, void* block, u64 size);

/**
 * @brief Reallocates memory in place allocated with mem_allocate.
 * 
 * @param allocator Allocator instance.
 * @param block Pointer to memory block.
 * @param new_size New allocation size.
 * @return Pointer to reallocated memory, or NULL on failure.
 */
void* mem_reallocate(em_allocator* allocator, void* block, u64 new_size);
