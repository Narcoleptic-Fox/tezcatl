#pragma once

#include "coverage/model.hpp"
#include "coverage/path_map.hpp"
#include "metrics/summary.hpp"
#include "parse/api.hpp"
#include "parse/functions.hpp"
#include "report/naming.hpp"

#include <filesystem>
#include <iosfwd>
#include <utility>
#include <vector>

namespace tezcatl::report {

// The CSV tables Tezcatl writes, one writer each, shared by the commands
// that print one table and the report that writes them all. Every table
// starts with a header row; files are named relative to the root.

/// One row per function: file,line,column,kind,name,module,complexity,
/// rating, then Halstead's distinct_operators,distinct_operands,
/// total_operators,total_operands,volume,difficulty,effort.
void write_function_table(std::ostream& out, const std::vector<parse::FunctionInfo>& functions,
                          const FileNaming& naming, const metrics::Thresholds& thresholds);

/// One row per module and a TOTAL row: module,functions,mean,median,p90,max,
/// flagged,high (the complexity distribution),volume,effort (Halstead
/// totals).
void write_function_summary(std::ostream& out, const std::vector<parse::FunctionInfo>& functions,
                            const FileNaming& naming, const metrics::Thresholds& thresholds);

/// One row per public API declaration: file,line,column,kind,name,module,
/// documented (yes or no),style (doxygen, plain or none).
void write_api_table(std::ostream& out, const std::vector<parse::ApiEntity>& api,
                     const FileNaming& naming);

/// Documentation coverage of a set of declarations.
struct DocCoverage {
    std::size_t entities = 0;
    std::size_t documented = 0;
    std::size_t doxygen = 0;

    void add(const parse::ApiEntity& entity);
    /// 0 when there are no declarations.
    [[nodiscard]] double percent() const;
};

/// One row per module and a TOTAL row: module,entities,documented,percent,
/// doxygen.
void write_api_summary(std::ostream& out, const std::vector<parse::ApiEntity>& api,
                       const FileNaming& naming);

/// Imported coverage per source file, after path mapping, sorted by path.
/// A vector rather than a map: MSVC's std::map allocates when moved, which
/// structs holding one would inherit in their move constructors.
using FileCoverage = std::vector<std::pair<std::filesystem::path, coverage::Counts>>;

/// Coverage attributed to the project's files.
struct AttributedCoverage {
    FileCoverage files;      ///< by file, after path mapping, under the root only
    std::size_t outside = 0; ///< recorded files left out, not under the root
};

/// Moves each recorded path by `mappings`, keeps the files that pass
/// `in_project`, and adds up files that map to the same path.
[[nodiscard]] AttributedCoverage attribute(const coverage::CoverageData& data,
                                           const std::vector<coverage::PathMapping>& mappings,
                                           const parse::FileFilter& in_project);

/// One row per file: file,module,lines,lines_covered,branches,
/// branches_covered,functions,functions_covered.
void write_coverage_table(std::ostream& out, const FileCoverage& files, const FileNaming& naming);

/// One row per module and a TOTAL row: module,files,lines,lines_covered,
/// line_percent,branches,branches_covered,branch_percent,functions,
/// functions_covered,function_percent. A percentage with nothing to cover
/// is empty: 0% and 100% would both be claims.
void write_coverage_summary(std::ostream& out, const FileCoverage& files, const FileNaming& naming);

} // namespace tezcatl::report
