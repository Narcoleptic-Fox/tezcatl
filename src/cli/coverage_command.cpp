#include "cli/coverage_command.hpp"

#include "coverage/model.hpp"
#include "coverage/readers.hpp"
#include "report/tables.hpp"

#include <ostream>

namespace tezcatl::cli {

namespace fs = std::filesystem;

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
    const report::AttributedCoverage attributed =
        report::attribute(data, options.path_maps, project.in_project());
    if (options.summary) {
        report::write_coverage_summary(out, attributed.files, project.naming());
    } else {
        report::write_coverage_table(out, attributed.files, project.naming());
    }

    err << "tezcatl: " << options.inputs.size() << " coverage files, " << attributed.files.size()
        << " source files under the root, " << attributed.outside << " outside it\n";
    if (attributed.files.empty()) {
        err << "tezcatl: no covered file is under the root; if the data was recorded elsewhere, "
               "map its paths with --path-map FROM=TO\n";
        return 1;
    }
    return 0;
}

} // namespace tezcatl::cli
