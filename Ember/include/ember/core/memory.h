#pragma once

#include "ember/core.h"

typedef enum memory_tag {
	MEMORY_TAG_CORE,     /**< Allocated by ember core */
	MEMORY_TAG_PLATFORM, /**< Allocated by ember plat */
	MEMORY_TAG_RENDERER, /**< Allocated by ember gpu */
	MEMORY_TAG_DEVICE,   /**< Internal memory used by internal systems */
	MEMORY_TAG_MAX_TAGS,
} memory_tag;

void show_memory_stats();

void memory_leaks();

void* mem_allocate(u64 size, memory_tag tag);

void mem_free(void* block, u64 size, memory_tag tag);

void mem_report(i64 size, memory_tag tag);