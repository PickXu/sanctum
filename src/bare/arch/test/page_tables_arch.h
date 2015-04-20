#if !defined(BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED)
#define BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED

#include "../../cpu_context.h"  // For current_core().
#include "../../phys_ptr.h"

namespace sanctum {
namespace testing {

// For testing, the page table root registers are virtualized.
extern uintptr_t core_ptbr[], core_eptbr[];

};  // namespace sanctum::testing

namespace bare {

static_assert(sizeof(uintptr_t) >= 8, "Sanctum tests require 64-bit pointers");

constexpr size_t page_shift() {
  return 13;  // Same as 64-bit RISC V
}
constexpr size_t page_table_levels() {
  return 3;  // Same as 64-bit RISC V
}
constexpr inline size_t page_table_shift(size_t level) {
  // NOTE: we can't assert(level < page_table_levels()) because constexpr

  // NOTE: we intentionally diverge from RISC V here, because we need the
  //       quirkiest page table setup possible to test the page walking code

  // Three levels: L1 has 2048 entries/table, L2 has 1024 entries/table, L3 has
  //               4096 entries/table
  return (level == 0) ? 11 : ((level == 1) ? 10 : 12);
}
constexpr inline size_t page_table_entry_shift(size_t level) {
  // NOTE: we can't assert(level < page_table_levels()) because constexpr

  // NOTE: we intentionally diverge from RISC V here, because we need the
  //       quirkiest page table setup possible to test the page walking code

  // L1 has 16-byte entries (4 pages/table), L2 has
  return (level == 0) ? 4 : ((level == 1) ? 3 : 5);
}

inline void set_eptbr(uintptr_t value) {
  testing::core_eptbr[current_core()] = value;
}
inline void set_ptbr(uintptr_t value) {
  testing::core_ptbr[current_core()] = value;
}

inline bool is_valid_page_table_entry(uintptr_t entry_addr, size_t level) {
  return *(phys_ptr<uintptr_t>{entry_addr}) & 1;
}
inline uintptr_t page_table_entry_target(uintptr_t entry_addr, size_t level) {
  uintptr_t target_mask = ~((1 << page_shift()) - 1);
  return *(phys_ptr<uintptr_t>{entry_addr}) & target_mask;
}
inline void write_page_table_entry(uintptr_t entry_addr, size_t level,
    uintptr_t target, uintptr_t acl) {
  uintptr_t acl_mask = (1 << page_shift()) - 1;
  acl &= acl_mask;  // Mask off non-ACL bits.
  acl |= 1;         // Force valid to true.
  *(phys_ptr<uintptr_t>{entry_addr}) = target | acl;
}

};  // namespace sanctum::bare
};  // namespace sanctum
#endif  // !definded(BARE_ARCH_TEST_PAGE_TABLES_ARCH_H_INCLUDED)
