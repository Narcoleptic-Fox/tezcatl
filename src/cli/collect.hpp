#pragma once

#include "cli/project.hpp"
#include "parse/api.hpp"
#include "parse/functions.hpp"
#include "parse/includes.hpp"

#include <filesystem>
#include <iosfwd>
#include <vector>

namespace tezcatl::cli {

/// Which metrics a scan collects. Each costs time per unit, so a command
/// asks only for what it writes.
struct Collect {
    bool functions = false;
    bool api = false;
    bool includes = false;
};

/// What one scan of a project found. Functions and declarations seen in
/// more than one unit (inline functions, headers) are listed once.
struct Collected {
    ScanTotals totals;
    std::vector<parse::FunctionInfo> functions;
    std::vector<parse::ApiEntity> api;
    std::vector<parse::IncludeEdge> includes;
    /// The project's main files of the units parsed: include graph nodes
    /// even when they include nothing and nothing includes them.
    std::vector<std::filesystem::path> sources;
};

/// Parses every unit of `project` once and collects `what`. Parse errors are
/// written to `err`.
[[nodiscard]] Collected collect(const Project& project, Collect what, std::ostream& err);

} // namespace tezcatl::cli
