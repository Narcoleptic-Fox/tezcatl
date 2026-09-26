#pragma once

#include <algorithm>
#include <vector>

namespace tezcatl::parse {

/// Sorts `items` by location and removes the repeats that arise when a
/// header is seen by several translation units, keeping the first of each:
/// the one from the earliest unit in the compilation database. A file
/// compiled several ways (one source built into several programs with
/// different -D flags) is therefore measured as its first compile command.
///
/// stable_sort, not sort: items at one location compare equal but may
/// differ (another configuration, another complexity), and which one an
/// unstable sort keeps depends on the standard library, so the same inputs
/// would give different reports on different platforms.
template <typename Located> void merge_duplicates(std::vector<Located>& items) {
    std::ranges::stable_sort(items);
    const auto repeats = std::ranges::unique(items);
    items.erase(repeats.begin(), repeats.end());
}

} // namespace tezcatl::parse
