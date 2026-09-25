#include "cli/includes_command.hpp"

#include "parse/includes.hpp"
#include "report/include_graph.hpp"

#include <filesystem>
#include <format>
#include <ostream>
#include <vector>

namespace tezcatl::cli {

int run_includes(const IncludesOptions& options, const Streams& streams) {
    std::ostream& out = streams.out;
    std::ostream& err = streams.err;
    const Project project{options.project};
    std::vector<parse::IncludeEdge> edges;
    std::vector<std::filesystem::path> sources;
    const ScanTotals totals = project.scan(
        [&](const parse::ParsedUnit& parsed) {
            const std::filesystem::path source = parsed.file.lexically_normal();
            if (project.in_project()(source)) {
                sources.push_back(source);
            }
            auto found = parse::find_includes(parsed, project.in_project());
            edges.insert(edges.end(), found.begin(), found.end());
        },
        err);
    const report::IncludeGraph graph =
        report::build_include_graph(edges, sources, project.naming());

    switch (options.output) {
    case IncludesOutput::edges:
        report::write_include_edges(out, graph);
        break;
    case IncludesOutput::files:
        report::write_include_files(out, graph);
        break;
    case IncludesOutput::modules:
        report::write_include_modules(out, graph);
        break;
    case IncludesOutput::cycles:
        report::write_include_cycles(out, graph);
        break;
    case IncludesOutput::coupling:
        report::write_include_coupling(out, graph);
        break;
    case IncludesOutput::dot:
        report::write_include_dot(out, graph);
        break;
    }
    return project.finish(
        totals,
        std::format("{} files, {} include edges, {} file cycles, {} module cycles",
                    graph.files.size(), graph.files.edge_count(), graph.file_cycles.size(),
                    graph.module_cycles.size()),
        err);
}

} // namespace tezcatl::cli
