#include "ember/core.h"
#include "memory.h"

typedef struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
	u64 allocation_count[MEMORY_TAG_MAX_TAGS];
} memory_stats;

static const char* tag_strings[] = {
	"CORE    ",
	"FRAME  "
	"PLATFORM",
	"RENDERER",
	"AUDIO   ",
	"NETWORK "};

static memory_stats stats = { 0 };

u64 alignment_ptr(u64 v, u64 alignment) {
    return (v + (alignment - 1)) & ~(alignment - 1);
}

void* mem_allocate(em_allocator* allocator, u64 size, memory_tag tag) {
	return allocator->alloc(allocator, size, 0, tag);
}

void mem_free(em_allocator* allocator, void* block, u64 size, memory_tag tag) {
	allocator->free(allocator, block, size, 0, tag);
}

void* mem_reallocate(em_allocator* allocator, void* block, u64 old_size, u64 new_size, memory_tag tag) {
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

	return allocator->realloc(allocator, block, old_size, new_size, 0, tag);
}

void mem_report(u64 size, memory_tag tag) {
	stats.total_allocated += size;
	stats.tagged_allocations[tag] += size;
	stats.allocation_count[tag]++;
}

void mem_report_free(u64 size, memory_tag tag) {
	stats.total_allocated -= size;
	stats.tagged_allocations[tag] -= size;
	stats.allocation_count[tag]--;
}

void memory_leaks() {
	if (stats.total_allocated == 0) return;
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		if (!stats.allocation_count[i]) continue;
		EM_ERROR("Core", "Unfreed %llu bytes on MEMORY_TAG_%s", stats.tagged_allocations[i], tag_strings[i]);
		EM_ERROR("Core", "%i unfreed allocations on tag...", stats.allocation_count[i]);
	}
	EM_ERROR("Core", "In total, forget to free %i bytes.", stats.total_allocated);
}

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

	EM_DEV("Core", "  %-8s | %7.2f %-3s | %03u", name, amount, unit, tag_count);
}

void show_memory_stats() {
	u32 total_bytes = 0, total_count = 0;

	EM_DEV("Core", "System memory use (tagged):");
	EM_DEV("Core", "  TAG      BYTES     COUNT");
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		total_bytes += stats.tagged_allocations[i];
		total_count += stats.allocation_count[i];
		show_memory_row(tag_strings[i], stats.tagged_allocations[i], stats.allocation_count[i]);
	}

	EM_DEV("Core", "-----------+-------------+----");
	show_memory_row("TOTAL", total_bytes, total_count);
}