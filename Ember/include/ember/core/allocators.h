#pragma once

#include "ember/core.h"

/**
 * @brief Creates a freelist allocator.
 *
 * The freelist allocator manages memory by maintaining a list of free blocks
 * within a larger allocation. It is useful for dynamic allocations with
 * frequent allocations and deallocations of varying sizes.
 *
 * @param parent Parent allocator used for backing memory allocation.
 * @return Initialized freelist allocator instance.
 */
ember_allocator freelist_allocator(ember_allocator* parent);

/**
 * @brief Iterates over allocated blocks in the freelist.
 *
 * This function allows traversal of all active (allocated) blocks within the
 * freelist allocator. The iteration state is stored in the provided cursor.
 *
 * @param allocator Pointer to the freelist allocator.
 * @param cursor Pointer to iteration state. Must be initialized to NULL before first call.
 * @return True if a block was found, false if iteration is complete.
 */
b8 freelist_next_block(const ember_allocator* allocator, void** cursor);
