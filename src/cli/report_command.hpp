#pragma once

#include "cli/project.hpp"
#include "coverage/path_map.hpp"
#include "metrics/summary.hpp"

#include <filesystem>
#include <iosfwd>
#include <vector>

namespace tezcatl::cli {

struct ReportOptions {
    ProjectOptions project;
    metrics::Thresholds thresholds;
    std::vector<std::filesystem::path> coverage_inputs; ///< none: no coverage figures
    std::vector<coverage::PathMapping> path_maps;
    std::filesystem::path output_directory; ///< created if missing
};

/// `tezcatl report -p BUILD_DIR --out DIR`: measures everything in one pass
/// and writes the baseline report to DIR:
///
///   report.md, report.json   the report, for people and for programs
///   modules.csv              one row per module across every metric
///   files.csv                lines per source file, with module and role
///   functions.csv, api.csv   every function and public API declaration
///   coverage.csv             imported coverage per file (only with data)
///   include-*.csv            edges, files, modules, cycles, coupling
///   includes.dot             the file include graph, for Graphviz
///
/// Lines are counted in every source file under the root, outside the
/// build directory. A coverage.csv left from an earlier run with coverage
/// is removed, so a directory never mixes two runs.
///
/// Parse errors are written to `err`, and make the exit code 1 unless
/// allowed; the report is written either way and says it is incomplete.
[[nodiscard]] int run_report(const ReportOptions& options, std::ostream& err);

} // namespace tezcatl::cli
