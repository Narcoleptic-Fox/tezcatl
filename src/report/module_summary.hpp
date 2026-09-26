#pragma once

#include "coverage/model.hpp"
#include "metrics/loc.hpp"
#include "metrics/summary.hpp"
#include "report/data.hpp"
#include "report/naming.hpp"
#include "report/tables.hpp"

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace tezcatl::report {

/// One module's figures across every metric: the table the SOW asks for.
///
/// Lines are split into production and test code. Complexity, Halstead and
/// documentation describe production code only: a test's complexity is not
/// the product's. The include figures are the module graph's.
struct ModuleRow {
    std::string module;
    std::size_t files = 0;        ///< source files, production and test
    std::size_t files_parsed = 0; ///< of which some unit parsed
    metrics::LocCounts production;
    metrics::LocCounts test;
    metrics::Distribution complexity; ///< of production functions
    double halstead_volume = 0.0;
    double halstead_effort = 0.0;
    DocCoverage documentation; ///< of production headers
    std::optional<coverage::Counts> coverage;
    std::size_t fan_in = 0;           ///< modules including this one's files
    std::size_t fan_out = 0;          ///< modules this one's files include
    std::optional<std::size_t> cycle; ///< the module cycle it belongs to, 1-based
};

/// One row per module, in name order, over every module any file of the
/// report belongs to.
[[nodiscard]] std::vector<ModuleRow> summarize_modules(const ReportData& data,
                                                       const FileNaming& naming);

/// The same figures over the whole project. It has no include figures: the
/// project is not part of a module graph.
[[nodiscard]] ModuleRow summarize_project(const ReportData& data, const FileNaming& naming);

/// module,files,files_parsed,production_physical,production_code,production_comment,
/// production_blank,test_physical,test_code,functions,complexity_mean,
/// complexity_median,complexity_p90,complexity_max,flagged,high,
/// halstead_volume,halstead_effort,api,documented,documented_percent,
/// coverage_lines,coverage_lines_covered,coverage_branches,
/// coverage_branches_covered,coverage_functions,coverage_functions_covered,
/// fan_in,fan_out,cycle; then a TOTAL row. Coverage columns are empty
/// without coverage data, and include columns in the TOTAL row.
void write_module_table(std::ostream& out, const std::vector<ModuleRow>& modules,
                        const ModuleRow& total);

} // namespace tezcatl::report
