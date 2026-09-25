#include "cli/functions_command.hpp"

#include "cli/collect.hpp"
#include "parse/functions.hpp"
#include "report/tables.hpp"

#include <format>
#include <ostream>
#include <vector>

namespace tezcatl::cli {

int run_functions(const FunctionsOptions& options, const Streams& streams) {
    std::ostream& out = streams.out;
    std::ostream& err = streams.err;
    metrics::validate(options.thresholds);
    const Project project{options.project};
    const Collected found = collect(project, {.functions = true}, err);
    const std::vector<parse::FunctionInfo>& functions = found.functions;

    if (options.summary) {
        report::write_function_summary(out, functions, project.naming(), options.thresholds);
    } else {
        report::write_function_table(out, functions, project.naming(), options.thresholds);
    }
    return project.finish(found.totals,
                          std::format("{} functions; complexity flagged over {}, high over {}",
                                      functions.size(), options.thresholds.flagged_over,
                                      options.thresholds.high_over),
                          err);
}

} // namespace tezcatl::cli
