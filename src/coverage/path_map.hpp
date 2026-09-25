#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace tezcatl::coverage {

/// Where source files recorded under `from` are found now: coverage is often
/// collected on another machine or in another directory (a CI job, a
/// container) than the one the project is measured in.
struct PathMapping {
    std::filesystem::path from;
    std::filesystem::path to;
};

/// Parses "FROM=TO", splitting at the first '='. Throws
/// std::invalid_argument if either side is empty.
[[nodiscard]] PathMapping parse_path_mapping(std::string_view text);

/// `recorded` moved from the first mapping whose `from` contains it (judged
/// by whole path components) to that mapping's `to`, or unchanged.
[[nodiscard]] std::filesystem::path map_path(const std::filesystem::path& recorded,
                                             const std::vector<PathMapping>& mappings);

} // namespace tezcatl::coverage
