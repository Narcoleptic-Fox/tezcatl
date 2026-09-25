#include "cli/functions_command.hpp"

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
    std::vector<parse::FunctionInfo> functions;
    const ScanTotals totals = project.scan(
        [&](const parse::ParsedUnit& parsed) {
            auto found = parse::find_functions(parsed, project.in_project());
            functions.insert(functions.end(), found.begin(), found.end());
        },
        err);
    parse::merge_duplicates(functions);

    if (options.summary) {
        report::write_function_summary(out, functions, project.naming(), options.thresholds);
    } else {
        report::write_function_table(out, functions, project.naming(), options.thresholds);
    }
    return project.finish(totals,
                          std::format("{} functions; complexity flagged over {}, high over {}",
                                      functions.size(), options.thresholds.flagged_over,
                                      options.thresholds.high_over),
                          err);
}

} // namespace tezcatl::cli
