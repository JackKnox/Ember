#include "ember/core.h"
#include "memory.h"

#ifdef EMBER_DEV
typedef struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
	u64 allocation_count[MEMORY_TAG_MAX_TAGS];
} memory_stats;

static const char* tag_strings[] = {
	"CORE    ",
	"FRAME   ",
	"DEVICE  ",
	"PLATFORM",
	"RENDERER",};

static memory_stats stats = { 0 };
#endif

#define SYSTEM_ALLOC_MAGIC 0xFF

void* system_malloc(ember_allocator* allocator, u64 size, u64 alignment) {
	if (!alignment) {
		return malloc(size);
	}

    if (alignment < sizeof(void*)) alignment = sizeof(void*);

#if defined(EM_PLATFORM_WINDOWS)
    return _aligned_malloc(size, alignment);
#elif defined(EM_PLATFORM_POSIX)
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) return NULL;
    return ptr;
#else
	#error "Unsupported platform in memory subsystem!"
#endif
}

void system_free(ember_allocator* allocator, void* block, u64 size, u64 alignment) {
	(void)allocator;
	(void)size;
	if (!alignment) {
		free(block);
		return;
	}

#if defined(EM_PLATFORM_WINDOWS)
    _aligned_free(block);
#elif defined(EM_PLATFORM_POSIX)
    free(block);
#else
	#error "Unsupported platform in memory subsystem!"
#endif
}

void* system_realloc(ember_allocator* allocator, void* block, u64 old_size, u64 new_size, u64 alignment) {
	(void)allocator;
    if (!alignment) {
        return realloc(block, new_size);
    }

    if (alignment < sizeof(void*)) alignment = sizeof(void*);

#if defined(EM_PLATFORM_WINDOWS)
    return _aligned_realloc(block, new_size, alignment);
#elif defined(EM_PLATFORM_POSIX)
    void* new_ptr = NULL;
    if (posix_memalign(&new_ptr, alignment, new_size) != 0) {
        return NULL;
    }

    // Copy old data (up to the smaller size)
    u64 copy_size = old_size < new_size ? old_size : new_size;
    memcpy(new_ptr, block, copy_size);

    free(block);
    return new_ptr;
#else
    #error "Unsupported platform in memory subsystem!"
#endif
}

ember_allocator em_allocator_default() {
	ember_allocator allocator = {};
	allocator.alloc = system_malloc;
	allocator.free = system_free;
	allocator.realloc = system_realloc;
	allocator.magic = SYSTEM_ALLOC_MAGIC;
	return allocator;
}

u64 alignment_ptr(u64 v, u64 alignment) {
    return (v + (alignment - 1)) & ~(alignment - 1);
}

void* mem_allocate(ember_allocator* allocator, u64 size, memory_tag tag) {
	mem_report(size, tag);
	ember_allocator sys = em_allocator_default();
	if (!allocator) allocator = &sys;

	return em_memset(allocator->alloc(allocator, size, 0), 0, size);
}

void mem_free(ember_allocator* allocator, void* block, u64 size, memory_tag tag) {
	if (allocator && !allocator->free) {
		EM_TRACE("Core", "Called free on a allocator that doesn't support that");
		return;
	}
	mem_report_free(size, tag);
	ember_allocator sys = em_allocator_default();
	if (!allocator) allocator = &sys;

#ifdef EMBER_BUILD_DEBUG
	em_memset(block, MEMORY_POISON_VALUE, size);
#endif
	allocator->free(allocator, block, size, 0);
}

void* mem_reallocate(ember_allocator* allocator, void* block, u64 old_size, u64 new_size, memory_tag tag) {
	if (!allocator->realloc) {
		mem_free(allocator, block, old_size, tag);
		return mem_allocate(allocator, new_size, tag);
	}

    if (!block) {
        return mem_allocate(allocator, new_size, tag);
    }

    if (new_size == 0) {
        mem_free(allocator, block, old_size, tag);
        return NULL;
    }

	mem_report_free(old_size, tag);
	mem_report(new_size, tag);
#ifdef EMBER_BUILD_DEBUG
	em_memset(block, MEMORY_POISON_VALUE, old_size);
#endif
	return em_memset(allocator->realloc(allocator, block, old_size, new_size, 0), 0, new_size);
}

void mem_report(u64 size, memory_tag tag) {
#if EMBER_DEV
	stats.total_allocated += size;
	stats.tagged_allocations[tag] += size;
	stats.allocation_count[tag]++;
#endif
}

void mem_report_free(u64 size, memory_tag tag) {
#if EMBER_DEV
	stats.total_allocated -= size;
	stats.tagged_allocations[tag] -= size;
	stats.allocation_count[tag]--;
#endif
}

void memory_leaks() {
#if EMBER_DEV
	if (stats.total_allocated == 0) return;
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		if (!stats.allocation_count[i]) continue;
		EM_ERROR("Core", "Unfreed %llu bytes on MEMORY_TAG_%s", stats.tagged_allocations[i], tag_strings[i]);
		EM_ERROR("Core", "%i unfreed allocations on tag...", stats.allocation_count[i]);
	}
	EM_ERROR("Core", "In total, forget to free %i bytes.", stats.total_allocated);
#endif
}

#if EMBER_DEV
void show_memory_row(const char* name, u32 tag_bytes, u64 tag_count) {
	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	u64 raw_amount = tag_bytes;

	const char* unit = "B";
	double amount = (double)raw_amount;

	if (raw_amount >= gib) {
		unit = "GiB";
		amount = raw_amount / (double)gib;
	} else if (raw_amount >= mib) {
		unit = "MiB";
		amount = raw_amount / (double)mib;
	} else if (raw_amount >= kib) {
		unit = "KiB";
		amount = raw_amount / (double)kib;
	}

	EM_DEV("Core", " %-8s | %7.2f %-3s | %03u", name, amount, unit, tag_count);
}
#endif

void show_memory_stats() {
#if EMBER_DEV
	u32 total_bytes = 0, total_count = 0;

	EM_DEV("Core", "System memory use (tagged):");
	EM_DEV("Core", " TAG      BYTES     COUNT");
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		total_bytes += stats.tagged_allocations[i];
		total_count += stats.allocation_count[i];
		show_memory_row(tag_strings[i], stats.tagged_allocations[i], stats.allocation_count[i]);
	}

	EM_DEV("Core", "----------+-------------+----");
	show_memory_row("TOTAL", total_bytes, total_count);
#endif
}