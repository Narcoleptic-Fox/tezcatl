#pragma once

#include "cli/project.hpp"
#include "metrics/summary.hpp"

namespace tezcatl::cli {

struct FunctionsOptions {
    ProjectOptions project;
    metrics::Thresholds thresholds;
    bool summary = false; ///< one row per module instead of per function
};

/// `tezcatl functions -p BUILD_DIR`: parses every compilation database entry
/// and writes one CSV row per function definition, columns
/// file,line,column,kind,name,module,complexity,rating, then Halstead's
/// distinct_operators,distinct_operands,total_operators,total_operands,
/// volume,difficulty,effort, with files relative to the root. Functions in
/// system headers or under the build directory (fetched dependencies,
/// generated code) are left out.
///
/// With `summary`, writes one row per module instead, columns
/// module,functions,mean,median,p90,max,flagged,high,volume,effort (the
/// complexity distribution and total Halstead volume and effort), then a
/// TOTAL row over all modules. The thresholds are written to `err`, since a
/// count of flagged functions means nothing without them.
///
/// Parse errors are written to `err`, and make the exit code 1 unless
/// allowed.
[[nodiscard]] int run_functions(const FunctionsOptions& options, const Streams& streams);

} // namespace tezcatl::cli
