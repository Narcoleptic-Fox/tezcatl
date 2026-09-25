#include "cli/report_command.hpp"

#include "cli/collect.hpp"
#include "cli/coverage_command.hpp"
#include "metrics/loc.hpp"
#include "parse/libclang_info.hpp"
#include "report/data.hpp"
#include "report/include_graph.hpp"
#include "report/json_report.hpp"
#include "report/markdown_report.hpp"
#include "report/module_summary.hpp"
#include "report/tables.hpp"
#include "scan/source_files.hpp"
#include "tezcatl/version.hpp"

#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::cli {

namespace fs = std::filesystem;

namespace {

struct Output {
    std::string_view name;
    std::function<void(std::ostream&)> write;
};

// Binary mode: '\n' stays '\n', so a report is the same bytes on every
// platform.
void write_output(const fs::path& directory, const Output& output) {
    const fs::path path = directory / output.name;
    std::ofstream out{path, std::ios::binary};
    if (!out) {
        throw std::runtime_error("cannot write " + path.string());
    }
    output.write(out);
    out.close();
    if (!out) {
        throw std::runtime_error("error while writing " + path.string());
    }
}

// Every source file under the root, parsed or not: a file no unit reached
// still has lines, and leaving it out would hide it.
std::vector<report::FileLines> count_project_lines(const Project& project,
                                                   const report::IncludeGraph& includes) {
    std::vector<report::FileLines> files;
    for (const fs::path& file :
         scan::find_source_files_under(project.naming().root(), project.in_project())) {
        if (project.in_project()(file)) {
            files.push_back({.file = file,
                             .counts = metrics::count_lines(scan::read_file(file)),
                             .parsed = includes.files.contains(project.naming().relative(file))});
        }
    }
    return files;
}

} // namespace

int run_report(const ReportOptions& options, std::ostream& err) {
    metrics::validate(options.thresholds);
    const Project project{options.project};
    const report::FileNaming& naming = project.naming();

    // Coverage first: data that maps to nothing should fail before the
    // slow part, not after it.
    std::optional<report::AttributedCoverage> coverage;
    if (!options.coverage_inputs.empty()) {
        coverage = import_coverage(options.coverage_inputs, options.path_maps, project, err);
        if (!coverage.has_value()) {
            return 1;
        }
    }

    Collected found = collect(project, {.functions = true, .api = true, .includes = true}, err);
    // The include graph's nodes are exactly the files the units reached.
    report::IncludeGraph includes =
        report::build_include_graph(found.includes, found.sources, naming);
    std::vector<report::FileLines> files = count_project_lines(project, includes);
    report::ReportData data{.provenance = {.tool_version = std::string{version},
                                           .libclang_version = parse::libclang_version(),
                                           .compilation_database = options.project.build_directory,
                                           .coverage_inputs = options.coverage_inputs,
                                           .translation_units = found.totals.units,
                                           .units_with_errors = found.totals.units_with_errors},
                            .thresholds = options.thresholds,
                            .files = std::move(files),
                            .functions = std::move(found.functions),
                            .api = std::move(found.api),
                            .includes = std::move(includes),
                            .coverage = std::move(coverage)};

    std::vector<Output> outputs{
        {.name = "report.md",
         .write = [&](std::ostream& out) { report::write_report_markdown(out, data, naming); }},
        {.name = "report.json",
         .write = [&](std::ostream& out) { report::write_report_json(out, data, naming); }},
        {.name = "modules.csv",
         .write =
             [&](std::ostream& out) {
                 report::write_module_table(out, report::summarize_modules(data, naming),
                                            report::summarize_project(data, naming));
             }},
        {.name = "files.csv",
         .write = [&](std::ostream& out) { report::write_file_table(out, data.files, naming); }},
        {.name = "functions.csv",
         .write =
             [&](std::ostream& out) {
                 report::write_function_table(out, data.functions, naming, data.thresholds);
             }},
        {.name = "api.csv",
         .write = [&](std::ostream& out) { report::write_api_table(out, data.api, naming); }},
        {.name = "include-edges.csv",
         .write = [&](std::ostream& out) { report::write_include_edges(out, data.includes); }},
        {.name = "include-files.csv",
         .write = [&](std::ostream& out) { report::write_include_files(out, data.includes); }},
        {.name = "include-modules.csv",
         .write = [&](std::ostream& out) { report::write_include_modules(out, data.includes); }},
        {.name = "include-cycles.csv",
         .write = [&](std::ostream& out) { report::write_include_cycles(out, data.includes); }},
        {.name = "include-coupling.csv",
         .write = [&](std::ostream& out) { report::write_include_coupling(out, data.includes); }},
        {.name = "includes.dot",
         .write = [&](std::ostream& out) { report::write_include_dot(out, data.includes); }},
    };
    const fs::path stale_coverage = options.output_directory / "coverage.csv";
    if (data.coverage.has_value()) {
        outputs.push_back({.name = "coverage.csv", .write = [&](std::ostream& out) {
                               report::write_coverage_table(out, data.coverage->files, naming);
                           }});
    }

    fs::create_directories(options.output_directory);
    for (const Output& output : outputs) {
        write_output(options.output_directory, output);
    }
    if (!data.coverage.has_value() && fs::remove(stale_coverage)) {
        err << "tezcatl: removed " << stale_coverage.generic_string()
            << ", left from a run with coverage data\n";
    }
    return project.finish(
        found.totals,
        std::format("{} source files, {} functions, {} API declarations; wrote {} files to {}",
                    data.files.size(), data.functions.size(), data.api.size(), outputs.size(),
                    options.output_directory.generic_string()),
        err);
}

} // namespace tezcatl::cli
