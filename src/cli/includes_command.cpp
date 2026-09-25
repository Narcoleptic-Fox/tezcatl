#include "cli/includes_command.hpp"

#include "graph/digraph.hpp"
#include "parse/includes.hpp"
#include "report/csv.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tezcatl::cli {

namespace {

using graph::Digraph;

/// The file graph and everything derived from it.
struct IncludeGraph {
    Digraph files;
    std::vector<std::string> file_module; ///< per file node
    std::vector<std::vector<std::size_t>> file_cycles;
    std::vector<std::optional<std::size_t>> file_cycle; ///< 1-based, per file node
    Digraph modules;                                    ///< edges between distinct modules
    std::vector<std::vector<std::size_t>> module_cycles;
    std::vector<std::optional<std::size_t>> module_cycle; ///< 1-based, per module node
    std::vector<std::size_t> module_files;                ///< file count, per module node
    std::map<std::pair<std::string, std::string>, std::size_t> coupling;
};

std::vector<std::optional<std::size_t>>
cycle_numbers(std::size_t size, const std::vector<std::vector<std::size_t>>& cycles) {
    std::vector<std::optional<std::size_t>> numbers(size);
    for (std::size_t cycle = 0; cycle < cycles.size(); ++cycle) {
        for (const std::size_t node : cycles.at(cycle)) {
            numbers.at(node) = cycle + 1;
        }
    }
    return numbers;
}

IncludeGraph build_graph(const Project& project, const std::vector<parse::IncludeEdge>& edges,
                         const std::vector<std::filesystem::path>& sources) {
    std::vector<Digraph::Edge> file_edges(edges.size());
    std::ranges::transform(edges, file_edges.begin(), [&project](const parse::IncludeEdge& edge) {
        return Digraph::Edge{project.naming().relative(edge.from),
                             project.naming().relative(edge.to)};
    });
    std::vector<std::string> source_names(sources.size());
    std::ranges::transform(sources, source_names.begin(),
                           [&project](const std::filesystem::path& source) {
                               return project.naming().relative(source);
                           });
    Digraph files{file_edges, source_names};

    std::vector<std::string> file_module(files.size());
    for (std::size_t node = 0; node < files.size(); ++node) {
        file_module.at(node) = project.naming().modules().module_of(files.name(node));
    }
    std::map<std::pair<std::string, std::string>, std::size_t> coupling;
    std::vector<Digraph::Edge> module_edges;
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            ++coupling[{file_module.at(from), file_module.at(to)}];
            if (file_module.at(from) != file_module.at(to)) {
                module_edges.emplace_back(file_module.at(from), file_module.at(to));
            }
        }
    }
    Digraph modules{module_edges, file_module};
    std::map<std::string, std::size_t> files_per_module;
    for (const std::string& module : file_module) {
        ++files_per_module[module];
    }
    std::vector<std::size_t> module_files(modules.size());
    for (std::size_t node = 0; node < modules.size(); ++node) {
        module_files.at(node) = files_per_module.at(modules.name(node));
    }

    auto file_cycles = graph::cycles(files);
    auto file_cycle = cycle_numbers(files.size(), file_cycles);
    auto module_cycles = graph::cycles(modules);
    auto module_cycle = cycle_numbers(modules.size(), module_cycles);
    return {.files = std::move(files),
            .file_module = std::move(file_module),
            .file_cycles = std::move(file_cycles),
            .file_cycle = std::move(file_cycle),
            .modules = std::move(modules),
            .module_cycles = std::move(module_cycles),
            .module_cycle = std::move(module_cycle),
            .module_files = std::move(module_files),
            .coupling = std::move(coupling)};
}

std::string cycle_field(const std::optional<std::size_t>& cycle) {
    return cycle.has_value() ? std::to_string(*cycle) : std::string{};
}

void write_edges(const IncludeGraph& graph, std::ostream& out) {
    out << "from,to,from_module,to_module\n";
    const Digraph& files = graph.files;
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            out << report::csv_field(files.name(from)) << ',' << report::csv_field(files.name(to))
                << ',' << report::csv_field(graph.file_module.at(from)) << ','
                << report::csv_field(graph.file_module.at(to)) << '\n';
        }
    }
}

void write_files(const IncludeGraph& graph, std::ostream& out) {
    out << "file,module,fan_in,fan_out,cycle\n";
    const Digraph& files = graph.files;
    for (std::size_t node = 0; node < files.size(); ++node) {
        out << report::csv_field(files.name(node)) << ','
            << report::csv_field(graph.file_module.at(node)) << ',' << files.fan_in(node) << ','
            << files.fan_out(node) << ',' << cycle_field(graph.file_cycle.at(node)) << '\n';
    }
}

void write_modules(const IncludeGraph& graph, std::ostream& out) {
    out << "module,files,fan_in,fan_out,cycle\n";
    const Digraph& modules = graph.modules;
    for (std::size_t node = 0; node < modules.size(); ++node) {
        out << report::csv_field(modules.name(node)) << ',' << graph.module_files.at(node) << ','
            << modules.fan_in(node) << ',' << modules.fan_out(node) << ','
            << cycle_field(graph.module_cycle.at(node)) << '\n';
    }
}

void write_cycle_rows(std::string_view level, const Digraph& graph,
                      const std::vector<std::vector<std::size_t>>& cycles, std::ostream& out) {
    for (std::size_t cycle = 0; cycle < cycles.size(); ++cycle) {
        std::string members;
        for (const std::size_t node : cycles.at(cycle)) {
            if (!members.empty()) {
                members += ';';
            }
            members += graph.name(node);
        }
        out << level << ',' << cycle + 1 << ',' << cycles.at(cycle).size() << ','
            << report::csv_field(members) << '\n';
    }
}

void write_cycles(const IncludeGraph& graph, std::ostream& out) {
    out << "level,cycle,size,members\n";
    write_cycle_rows("file", graph.files, graph.file_cycles, out);
    write_cycle_rows("module", graph.modules, graph.module_cycles, out);
}

void write_coupling(const IncludeGraph& graph, std::ostream& out) {
    out << "from_module,to_module,edges\n";
    for (const auto& [modules, count] : graph.coupling) {
        out << report::csv_field(modules.first) << ',' << report::csv_field(modules.second) << ','
            << count << '\n';
    }
}

// A DOT double-quoted ID: backslashes and quotes escaped.
std::string dot_id(std::string_view text) {
    std::string quoted = "\"";
    for (const char c : text) {
        if (c == '"' || c == '\\') {
            quoted += '\\';
        }
        quoted += c;
    }
    quoted += '"';
    return quoted;
}

void write_dot(const IncludeGraph& graph, std::ostream& out) {
    const Digraph& files = graph.files;
    out << "digraph includes {\n  node [shape=box];\n";
    // One cluster per named module; unassigned files stay outside any.
    std::size_t cluster = 0;
    for (std::size_t module = 0; module < graph.modules.size(); ++module) {
        const std::string& name = graph.modules.name(module);
        if (name == config::unassigned_module) {
            continue;
        }
        out << "  subgraph cluster_" << ++cluster << " {\n    label=" << dot_id(name) << ";\n";
        for (std::size_t node = 0; node < files.size(); ++node) {
            if (graph.file_module.at(node) == name) {
                out << "    " << dot_id(files.name(node)) << ";\n";
            }
        }
        out << "  }\n";
    }
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            const bool in_cycle = graph.file_cycle.at(from).has_value() &&
                                  graph.file_cycle.at(from) == graph.file_cycle.at(to);
            out << "  " << dot_id(files.name(from)) << " -> " << dot_id(files.name(to))
                << (in_cycle ? " [color=\"#7a1f1f\"]" : "") << ";\n";
        }
    }
    out << "}\n";
}

} // namespace

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
    const IncludeGraph graph = build_graph(project, edges, sources);

    switch (options.output) {
    case IncludesOutput::edges:
        write_edges(graph, out);
        break;
    case IncludesOutput::files:
        write_files(graph, out);
        break;
    case IncludesOutput::modules:
        write_modules(graph, out);
        break;
    case IncludesOutput::cycles:
        write_cycles(graph, out);
        break;
    case IncludesOutput::coupling:
        write_coupling(graph, out);
        break;
    case IncludesOutput::dot:
        write_dot(graph, out);
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
