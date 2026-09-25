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

// What the summary adds up for a module: the complexity of each function,
// and the Halstead volume and effort, which are additive.
struct ModuleTotals {
    std::vector<unsigned> complexities;
    double volume = 0.0;
    double effort = 0.0;

    void add(const parse::FunctionInfo& function) {
        complexities.push_back(function.complexity);
        volume += function.halstead.volume();
        effort += function.halstead.effort();
    }
};

void write_summary_row(std::ostream& out, const std::string& module, const ModuleTotals& totals,
                       const metrics::Thresholds& thresholds) {
    const metrics::Distribution distribution = metrics::describe(totals.complexities, thresholds);
    out << report::csv_field(module) << ',' << distribution.count << ','
        << std::format("{:.2f}", distribution.mean) << ','
        << std::format("{:.1f}", distribution.median) << ',' << distribution.p90 << ','
        << distribution.max << ',' << distribution.flagged << ',' << distribution.high << ','
        << std::format("{:.2f},{:.2f}", totals.volume, totals.effort) << '\n';
}

void write_summary(const Project& project, const std::vector<parse::FunctionInfo>& functions,
                   const metrics::Thresholds& thresholds, std::ostream& out) {
    std::map<std::string, ModuleTotals> by_module;
    ModuleTotals all;
    for (const parse::FunctionInfo& function : functions) {
        by_module[project.module_of(function.file)].add(function);
        all.add(function);
    }
    out << "module,functions,mean,median,p90,max,flagged,high,volume,effort\n";
    for (const auto& [module, totals] : by_module) {
        write_summary_row(out, module, totals, thresholds);
    }
    write_summary_row(out, "TOTAL", all, thresholds);
}

void write_functions(const Project& project, const std::vector<parse::FunctionInfo>& functions,
                     const metrics::Thresholds& thresholds, std::ostream& out) {
    out << "file,line,column,kind,name,module,complexity,rating,"
           "distinct_operators,distinct_operands,total_operators,total_operands,"
           "volume,difficulty,effort\n";
    for (const parse::FunctionInfo& function : functions) {
        const metrics::Halstead& h = function.halstead;
        out << report::csv_field(project.relative(function.file)) << ',' << function.line << ','
            << function.column << ',' << parse::to_string(function.kind) << ','
            << report::csv_field(function.name) << ','
            << report::csv_field(project.module_of(function.file)) << ',' << function.complexity
            << ',' << metrics::to_string(metrics::rate(function.complexity, thresholds)) << ','
            << h.distinct_operators << ',' << h.distinct_operands << ',' << h.total_operators << ','
            << h.total_operands << ','
            << std::format("{:.2f},{:.2f},{:.2f}", h.volume(), h.difficulty(), h.effort()) << '\n';
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
