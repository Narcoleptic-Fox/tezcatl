#pragma once

#include "cli/project.hpp"

namespace tezcatl::cli {

struct DocsOptions {
    ProjectOptions project;
    bool summary = false; ///< one row per module instead of per declaration
};

/// `tezcatl docs -p BUILD_DIR`: documentation coverage of the public API
/// declared in the project's headers. Writes one CSV row per declaration,
/// columns file,line,column,kind,name,module,documented,style, where
/// documented is yes or no and style is doxygen, plain or none.
///
/// With `summary`, writes one row per module instead, columns
/// module,entities,documented,percent,doxygen, then a TOTAL row.
///
/// Parse errors are written to `err`, and make the exit code 1 unless
/// allowed.
[[nodiscard]] int run_docs(const DocsOptions& options, const Streams& streams);

} // namespace tezcatl::cli
