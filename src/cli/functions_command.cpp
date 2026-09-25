#include "cli/functions_command.hpp"

#include "parse/functions.hpp"
#include "report/csv.hpp"

#include <format>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace tezcatl::cli {

namespace {

void write_summary_row(std::ostream& out, const std::string& module,
                       const metrics::Distribution& distribution) {
    out << report::csv_field(module) << ',' << distribution.count << ','
        << std::format("{:.2f}", distribution.mean) << ','
        << std::format("{:.1f}", distribution.median) << ',' << distribution.p90 << ','
        << distribution.max << ',' << distribution.flagged << ',' << distribution.high << '\n';
}

void write_summary(const Project& project, const std::vector<parse::FunctionInfo>& functions,
                   const metrics::Thresholds& thresholds, std::ostream& out) {
    std::map<std::string, std::vector<unsigned>> by_module;
    std::vector<unsigned> all;
    for (const parse::FunctionInfo& function : functions) {
        by_module[project.module_of(function.file)].push_back(function.complexity);
        all.push_back(function.complexity);
    }
    out << "module,functions,mean,median,p90,max,flagged,high\n";
    for (const auto& [module, values] : by_module) {
        write_summary_row(out, module, metrics::describe(values, thresholds));
    }
    write_summary_row(out, "TOTAL", metrics::describe(all, thresholds));
}

void write_functions(const Project& project, const std::vector<parse::FunctionInfo>& functions,
                     const metrics::Thresholds& thresholds, std::ostream& out) {
    out << "file,line,column,kind,name,module,complexity,rating\n";
    for (const parse::FunctionInfo& function : functions) {
        out << report::csv_field(project.relative(function.file)) << ',' << function.line << ','
            << function.column << ',' << parse::to_string(function.kind) << ','
            << report::csv_field(function.name) << ','
            << report::csv_field(project.module_of(function.file)) << ',' << function.complexity
            << ',' << metrics::to_string(metrics::rate(function.complexity, thresholds)) << '\n';
    }
}

} // namespace

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
        write_summary(project, functions, options.thresholds, out);
    } else {
        write_functions(project, functions, options.thresholds, out);
    }
    return project.finish(totals,
                          std::format("{} functions; complexity flagged over {}, high over {}",
                                      functions.size(), options.thresholds.flagged_over,
                                      options.thresholds.high_over),
                          err);
}

} // namespace tezcatl::cli
