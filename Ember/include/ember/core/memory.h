#pragma once

#include "ember/core.h"

#include <string.h>
#include <stdlib.h>

/**
 * @brief Alias for standard memcpy, used for maximum compatibility.
 */
#define em_memcpy memcpy

/**
 * @brief Alias for standard memset, used for maximum compatibility..
 */
#define em_memset memset

/**
 * @brief Alias for standard memcmp, used for maximum compatibility..
 */
#define em_memcmp memcmp

struct ember_allocator;

/**
 * @brief Function pointer type for custom memory allocation.
 *
 * @param allocator Allocator instance.
 * @param size Number of bytes to allocate.
 * @param alignment Required memory alignment.
 *
 * @return Pointer to allocated memory, or NULL on failure.
 */
typedef void* (*PFN_allocate_mem)(struct ember_allocator* allocator, u64 size, u64 alignment);

/**
 * @brief Function pointer type for custom memory deallocation.
 *
 * @param allocator Allocator instance.
 * @param block Pointer to memory block to free.
 * @param size Original allocation size.
 * @param alignment Alignment used during allocation.
 */
typedef void (*PFN_free_mem)(struct ember_allocator* allocator, void* block, u64 size, u64 alignment);

/**
 * @brief Categorises allocations for debugging, profiling, and leak detection.
 */
typedef enum memory_tag {
	MEMORY_TAG_CORE,     /**< Allocated by ember_core */
	MEMORY_TAG_TEMP,     /**< Tempory data for any subsystem */
	MEMORY_TAG_DEVICE,   /**< Internal memory used by internal systems */
	MEMORY_TAG_PLATFORM, /**< Allocated by ember_plat */
	MEMORY_TAG_RENDERER, /**< Allocated by ember_gpu */
	MEMORY_TAG_MAX_TAGS,
} memory_tag;

/**
 * @brief Generic allocator interface used across the library.
 *
 * Allows pluggable memory systems (default malloc, arena allocators, etc.).
 */
typedef struct ember_allocator {
	/** @brief Allocation function. */
    PFN_allocate_mem alloc;

	/** @brief Deallocation function. */
    PFN_free_mem free;

	/** @brief Opaque context per allocator. */     
    void* user_data;       

#if !EMBER_DIST
	/** @brief Debug-only validation marker for allocator integrity */
    u8 magic; 
#endif
} ember_allocator;

/**
 * @brief Creates the default system allocator (malloc/free backed).
 *
 * @return Initialized allocator instance.
 */
ember_allocator em_allocator_default();

/**
 * @brief Allocates memory from the library allocator system.
 *
 * @param allocator Allocator instance.
 * @param size Number of bytes to allocate.
 * @param tag Memory category for tracking and debugging.
 * @return Pointer to allocated memory, or NULL on failure.
 * 
 * @note If @p allocator is NULL, allocates using the default system allocator.
 */
void* mem_allocate(ember_allocator* allocator, u64 size, memory_tag tag);

/**
 * @brief Frees memory allocated with mem_allocate.
 *
 * @param allocator Allocator instance.
 * @param block Pointer to memory block.
 * @param size Original allocation size.
 * @param tag Memory category used at allocation time.
 * 
 * @note If @p allocator is NULL, frees using the default system allocator.
 */
void mem_free(ember_allocator* allocator, void* block, u64 size, memory_tag tag);

/**
 * @brief Internal bookkeeping: records allocation event.
 *
 * Used for tracking allocations per tag for profiling/debugging.
 */
void mem_report(u64 size, memory_tag tag);

/**
 * @brief Internal bookkeeping: records deallocation event.
 *
 * Used for tracking memory usage and detecting leaks.
 */
void mem_report_free(u64 size, memory_tag tag);

/**
 * @brief Dumps current memory usage statistics.
 *
 * @note Only enabled in dev builds (See EMBER_DEV)
 */
void show_memory_stats();

/**
 * @brief Reports any detected memory leaks.
 *
 * @note Only enabled in dev builds (See EMBER_DEV)
 */
void memory_leaks();