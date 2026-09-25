#pragma once

#include "cli/project.hpp"
#include "coverage/path_map.hpp"

#include <filesystem>
#include <vector>

namespace tezcatl::cli {

struct CoverageOptions {
    std::vector<std::filesystem::path> inputs; ///< lcov, gcov JSON or llvm-cov JSON files
    std::filesystem::path root;                ///< only files under here are the project's
    std::filesystem::path module_map;          ///< empty: every file is unassigned
    std::vector<coverage::PathMapping> path_maps;
    bool summary = false; ///< one row per module instead of per file
};

/// `tezcatl coverage DATA...`: imports test coverage that other tools
/// measured, merges every input, and attributes it to the project's files
/// and modules. Writes one CSV row per file, columns
/// file,module,lines,lines_covered,branches,branches_covered,functions,
/// functions_covered; with `summary`, one row per module and a TOTAL row,
/// adding the three percentages (empty where there is nothing to cover).
///
/// Recorded paths are moved by `path_maps` first. Files outside the root
/// are left out and counted on `err`; if no file is under the root, that is
/// an error (exit code 1), since the paths most likely need mapping.
[[nodiscard]] int run_coverage(const CoverageOptions& options, const Streams& streams);

} // namespace tezcatl::cli
