#include "defines.h"
#include "memory.h"

#include "ember/platform/global.h"

#if EM_ENABLE_DIAGNOSTICS
typedef struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
	u64 allocation_count[MEMORY_TAG_MAX_TAGS];
} memory_stats;

static const char* tag_strings[] = {
    "UNKNOWN   ",
	"ENGINE    ",
	"PLATFORM  ",
	"CORE      ",
	"RESOURCES ",
	"RENDERER  ",
	"TOTAL     "};

static b8 is_initialized = FALSE;
static memory_stats stats = { 0 };
#endif

void memory_shutdown() {
#if EM_ENABLE_DIAGNOSTICS
	if (stats.total_allocated == 0) return;
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
		if (!stats.allocation_count[i]) continue;
		EM_ERROR("Unfreed %llu bytes on MEMORY_TAG_%s", stats.tagged_allocations[i], tag_strings[i]);
		EM_ERROR("%i unfreed allocations on tag...", stats.allocation_count[i]);
	}
#endif
}

void* ballocate(u64 size, memory_tag tag) {
	breport(size, tag);
	return bzero_memory(emc_malloc(size, FALSE), size);
}

void bfree(void* block, u64 size, memory_tag tag) {
	breport_free(size, tag);
	emc_free(block, FALSE);
}

void breport(u64 size, memory_tag tag) {
#if EM_ENABLE_DIAGNOSTICS
	stats.total_allocated += size;
	stats.tagged_allocations[tag] += size;
	stats.allocation_count[tag]++;
#endif
}

void breport_free(u64 size, memory_tag tag) {
#if EM_ENABLE_DIAGNOSTICS
	stats.total_allocated -= size;
	stats.tagged_allocations[tag] -= size;		
	stats.allocation_count[tag]--;
#endif
}

void* bzero_memory(void* block, u64 size) {
	return bset_memory(block, 0, size);
}

void* bcopy_memory(void* dest, const void* source, u64 size) {
	return emc_memcpy(dest, source, size);
}

void* bset_memory(void* dest, i32 value, u64 size) {
	return emc_memset(dest, value, size);
}

b8 bcmp_memory(void* buf1, void* buf2, u64 size) {
	return emc_memcmp(buf1, buf2, size);
}

void show_memory_stats() {
#if EM_ENABLE_DIAGNOSTICS
	u64 total = 0;

	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	EM_TRACE("System memory use (tagged):");
	EM_TRACE(" TAG          BYTES     COUNT");
	for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS + 1; ++i) {
		u64 raw_amount = i < MEMORY_TAG_MAX_TAGS ? stats.tagged_allocations[i] : total;
		total += raw_amount;

		const char* unit = "B  ";
		double amount = (double)raw_amount;

		if (raw_amount >= gib) {
			unit = "GiB";
			amount = raw_amount / (double)gib;
		}
		else if (raw_amount >= mib) {
			unit = "MiB";
			amount = raw_amount / (double)mib;
		}
		else if (raw_amount >= kib) {
			unit = "KiB";
			amount = raw_amount / (double)kib;
		}

		EM_TRACE("  %s | %.2f%s | %.3i", tag_strings[i], amount, unit, stats.allocation_count[i]);
	}
#endif
}