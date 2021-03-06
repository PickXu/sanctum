#if !defined(MONITOR_DRAM_REGIONS_H_INCLUDED)
#define MONITOR_DRAM_REGIONS_H_INCLUDED

#include "bare/base_types.h"
#include "bare/phys_atomics.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::api::os::dram_region_state_t;
using sanctum::bare::atomic;
using sanctum::bare::atomic_flag;
using sanctum::bare::phys_ptr;
using sanctum::bare::size_t;
using sanctum::bare::uintptr_t;

// Per-DRAM region accounting information.
struct dram_region_info_t {
  atomic_flag lock;             // lock for all the DRAM region's state
  enclave_id_t owner;           // nullptr if not owned by enclave
  enclave_id_t previous_owner;  // nullptr if previously owned by OS
  size_t pinned_pages;          // pages that can't be removed from DRAM
  size_t blocked_at;            // only valid for blocked regions
};

// Accounting information for all DRAM regions.
struct dram_regions_info_t {
  // NOTE: the lock generation counter is NOT protected by a lock, because it
  //       must be accessible in dram_region_flush(), which must be lock-free
  atomic<size_t> block_clock;
};

// The regions are allocated at boot time, so the physical pointers never
// change.

extern phys_ptr<dram_region_info_t> g_dram_region;
extern phys_ptr<dram_regions_info_t> g_dram_regions;

// The fields below are set by boot_init_dram_regions() and never change
// afterwards. Therefore, they do not require locking.

// Amount of DRAM installed on the system.
extern size_t g_dram_size;
// The bits in a physical address used to indicate the DRAM region.
//
// DRAM regions may not be contiguous in DRAM.
extern size_t g_dram_region_mask;
// The position of the least significant 1 bit in the DRAM region mask.
extern size_t g_dram_region_shift;
// The number of DRAM regions indexed by the mask.
//
// This is always 1 + (g_dram_region_mask >> g_dram_region_shift).
extern size_t g_dram_region_count;
// The size of a DRAM stripe.
//
// Stripes are continuous ranges of DRAM. A region is made up of multiple
// stripes. If the cache address shift is optimal, each DRAM region is
// contiguous and has exactly one stripe. Otherwise, the stripes alternate in
// DRAM as follows.
//
// R1S1 R1S2 ... R1Sn R2S1 R2S2 ... R2Sn ... RmS1 RmS2 ... RmSn
//
// The stripe size is always (1 << g_dram_region_shift).
extern size_t g_dram_stripe_size;

// The size of a DRAM region bitmap, in units of sizeof(size_t).
//
// This is ceil(g_dram_region_count / (sizeof(size_t) * 8)).
extern size_t g_dram_region_bitmap_words;

// The first byte of the allowed DMA transfers memory range.
//
// Accesses to this must have acquired the lock of DRAM region 0.
extern size_t g_dma_range_start;

// The first byte past the allowed DMA transfers memory range.
//
// Accesses to this must have acquired the lock of DRAM region 0.
extern size_t g_dma_range_end;

// The special enclave ID values below are used to make it possible to infer a
// DRAM region's state by reading its owner field. The values will not be
// validated by is_valid_enclave_id() because it will extract DRAM region 0 from
// the ID, and the owner of DRAM region 0 is always 0 (the OS).

// The enclave ID used as the owner of a blocked DRAM region.
constexpr enclave_id_t blocked_enclave_id = 1;

// The enclave ID used as the owner of a free DRAM region.
constexpr enclave_id_t free_enclave_id = 2;

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_DRAM_REGIONS_H_INCLUDED)
