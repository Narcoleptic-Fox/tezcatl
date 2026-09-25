#include "cli/coverage_command.hpp"

#include "coverage/model.hpp"
#include "coverage/readers.hpp"
#include "report/tables.hpp"

#include <ostream>

namespace tezcatl::cli {

namespace fs = std::filesystem;

std::optional<report::AttributedCoverage>
import_coverage(const std::vector<fs::path>& inputs,
                const std::vector<coverage::PathMapping>& path_maps, const Project& project,
                std::ostream& err) {
    coverage::CoverageData data;
    for (const fs::path& input : inputs) {
        coverage::read_coverage_file(input, data);
    }
    report::AttributedCoverage attributed =
        report::attribute(data, path_maps, project.in_project());
    err << "tezcatl: " << inputs.size() << " coverage files, " << attributed.files.size()
        << " source files under the root, " << attributed.outside << " outside it\n";
    if (attributed.files.empty()) {
        err << "tezcatl: no covered file is under the root; if the data was recorded elsewhere, "
               "map its paths with --path-map FROM=TO\n";
        return std::nullopt;
    }
    return attributed;
}

int run_coverage(const CoverageOptions& options, const Streams& streams) {
    const Project project{{.build_directory = {},
                           .root = options.root,
                           .resource_directory = {},
                           .module_map = options.module_map,
                           .test_globs = {},
                           .allow_parse_errors = false}};
    const std::optional<report::AttributedCoverage> attributed =
        import_coverage(options.inputs, options.path_maps, project, streams.err);
    if (!attributed.has_value()) {
        return 1;
    }
    if (options.summary) {
        report::write_coverage_summary(streams.out, attributed->files, project.naming());
    } else {
        report::write_coverage_table(streams.out, attributed->files, project.naming());
    }
    return 0;
}

} // namespace tezcatl::cli
