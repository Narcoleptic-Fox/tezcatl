#include "cli/coverage_command.hpp"

#include "coverage/model.hpp"
#include "coverage/readers.hpp"
#include "report/csv.hpp"

#include <cstddef>
#include <format>
#include <map>
#include <ostream>
#include <string>

namespace tezcatl::cli {

namespace fs = std::filesystem;

namespace {

// A percentage, or empty when there is nothing to cover: 0% and 100% would
// both be claims.
std::string percent(std::size_t covered, std::size_t total) {
    constexpr double hundred = 100.0;
    return total == 0 ? std::string{}
                      : std::format("{:.1f}", hundred * static_cast<double>(covered) /
                                                  static_cast<double>(total));
}

struct ModuleCoverage {
    std::size_t files = 0;
    coverage::Counts counts;
};

void write_counts(std::ostream& out, const coverage::Counts& c) {
    out << c.lines << ',' << c.lines_covered << ',' << c.branches << ',' << c.branches_covered
        << ',' << c.functions << ',' << c.functions_covered;
}

void write_summary_row(std::ostream& out, const std::string& module, const ModuleCoverage& m) {
    const coverage::Counts& c = m.counts;
    out << report::csv_field(module) << ',' << m.files << ',' << c.lines << ',' << c.lines_covered
        << ',' << percent(c.lines_covered, c.lines) << ',' << c.branches << ','
        << c.branches_covered << ',' << percent(c.branches_covered, c.branches) << ','
        << c.functions << ',' << c.functions_covered << ','
        << percent(c.functions_covered, c.functions) << '\n';
}

} // namespace

int run_coverage(const CoverageOptions& options, const Streams& streams) {
    std::ostream& out = streams.out;
    std::ostream& err = streams.err;
    coverage::CoverageData data;
    for (const fs::path& input : options.inputs) {
        coverage::read_coverage_file(input, data);
    }

    const Project project{{.build_directory = {},
                           .root = options.root,
                           .resource_directory = {},
                           .module_map = options.module_map,
                           .test_globs = {},
                           .allow_parse_errors = false}};
    // Mapped paths, sorted, so output is in path order whatever the input.
    std::map<fs::path, coverage::Counts> files;
    std::size_t outside = 0;
    for (const auto& [recorded, record] : data) {
        const fs::path file = coverage::map_path(recorded, options.path_maps);
        if (project.in_project()(file)) {
            files[file] += record.counts();
        } else {
            ++outside;
        }
    }

    if (options.summary) {
        std::map<std::string, ModuleCoverage> modules;
        ModuleCoverage all;
        for (const auto& [file, counts] : files) {
            ModuleCoverage& module = modules[project.naming().module_of(file)];
            ++module.files;
            module.counts += counts;
            ++all.files;
            all.counts += counts;
        }
        out << "module,files,lines,lines_covered,line_percent,branches,branches_covered,"
               "branch_percent,functions,functions_covered,function_percent\n";
        for (const auto& [module, coverage] : modules) {
            write_summary_row(out, module, coverage);
        }
        write_summary_row(out, "TOTAL", all);
    } else {
        out << "file,module,lines,lines_covered,branches,branches_covered,functions,"
               "functions_covered\n";
        for (const auto& [file, counts] : files) {
            out << report::csv_field(project.naming().relative(file)) << ','
                << report::csv_field(project.naming().module_of(file)) << ',';
            write_counts(out, counts);
            out << '\n';
        }
    }

    err << "tezcatl: " << options.inputs.size() << " coverage files, " << files.size()
        << " source files under the root, " << outside << " outside it\n";
    if (files.empty()) {
        err << "tezcatl: no covered file is under the root; if the data was recorded elsewhere, "
               "map its paths with --path-map FROM=TO\n";
        return 1;
    }
    return 0;
}

} // namespace tezcatl::cli
