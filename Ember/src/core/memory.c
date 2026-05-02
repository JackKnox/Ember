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
	"TEMP    ",
	"DEVICE  ",
	"PLATFORM",
	"RENDERER",};

static b8 is_initialized = FALSE;
static memory_stats stats = { 0 };
#endif

#define SYSTEM_ALLOC_MAGIC 0xFF

void* system_malloc(ember_allocator* allocator, u64 size, u64 alignment) {
	if (!alignment) {
		return malloc(size);
	}

    if (alignment < sizeof(void*)) alignment = sizeof(void*);

#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);

#elif defined(_POSIX_VERSION)
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) return NULL;
    return ptr;

#else
    /* Fallback: manual alignment */
    uintptr_t raw = (uintptr_t)malloc(size + alignment - 1 + sizeof(void*));
    if (!raw) return NULL;

    uintptr_t aligned = (raw + sizeof(void*) + alignment - 1) & ~(alignment - 1);

    ((void**)aligned)[-1] = (void*)raw; /* store original pointer */
    return (void*)aligned;
#endif
}

void system_free(ember_allocator* allocator, void* block, u64 size, u64 alignment) {
	if (!alignment) {
		free(block);
		return;
	}

#if defined(_MSC_VER)
    _aligned_free(block);

#elif defined(_POSIX_VERSION)
    free(block);

#else
    free(((void**)block)[-1]);
#endif
}

ember_allocator em_allocator_default() {
	ember_allocator allocator = {};
	allocator.alloc = system_malloc;
	allocator.free = system_free;
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
	mem_report_free(size, tag);
	ember_allocator sys = em_allocator_default();
	if (!allocator) allocator = &sys;

#ifdef EMBER_BUILD_DEBUG
	em_memset(block, MEMORY_POISON_VALUE, size);
#endif
	allocator->free(allocator, block, size, 0);
}

void* mem_realloc(ember_allocator* allocator, void* block, u64 old_size, u64 new_size, memory_tag tag) {
	if (!allocator->realloc) {
		mem_free(allocator, block, old_size, tag);
		return mem_allocate(allocator, new_size, tag);
	}

	mem_report_free(old_size, tag);
	mem_report(new_size, tag);
#ifdef EMBER_BUILD_DEBUG
	em_memset(block, MEMORY_POISON_VALUE, old_size);
#endif
	return em_memset(allocator->realloc(allocator, block, new_size, 0), 0, new_size);
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

void show_memory_stats() {
#if EMBER_DEV
	u64 total = 0;

	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	EM_DEV("Core", "System memory use (tagged):");
	EM_DEV("Core", " TAG      BYTES     COUNT");
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		u64 raw_amount = stats.tagged_allocations[i];
		total += raw_amount;

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

		EM_DEV("Core", " %-8s | %7.2f %-3s | %03u",
			tag_strings[i], amount, unit, stats.allocation_count[i]);
	}
#endif
}