#pragma once

#include "metrics/loc.hpp"
#include "metrics/summary.hpp"
#include "parse/api.hpp"
#include "parse/functions.hpp"
#include "report/include_graph.hpp"
#include "report/tables.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace tezcatl::report {

/// What was run to produce a report, recorded in it so the numbers can be
/// reproduced.
struct Provenance {
    std::string tool_version;
    std::string libclang_version;
    std::filesystem::path compilation_database; ///< the build directory given
    std::vector<std::filesystem::path> coverage_inputs;
    std::size_t translation_units = 0;
    std::size_t units_with_errors = 0;
    std::size_t units_skipped = 0; ///< database entries not in C or C++
};

/// Everything a report says about a project.
struct ReportData {
    Provenance provenance;
    metrics::Thresholds thresholds;
    std::vector<FileLines> files; ///< every source file under the root, sorted
    std::vector<parse::FunctionInfo> functions;
    std::vector<parse::ApiEntity> api;
    IncludeGraph includes;
    std::optional<AttributedCoverage> coverage; ///< only if coverage data was given
};

} // namespace tezcatl::report
