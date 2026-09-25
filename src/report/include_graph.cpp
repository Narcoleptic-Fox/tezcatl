#include "report/include_graph.hpp"

#include "config/modules.hpp"
#include "report/csv.hpp"

#include <algorithm>
#include <map>
#include <ostream>
#include <string_view>
#include <utility>

namespace tezcatl::report {

namespace {

using graph::Digraph;

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

} // namespace

IncludeGraph build_include_graph(const std::vector<parse::IncludeEdge>& edges,
                                 const std::vector<std::filesystem::path>& sources,
                                 const FileNaming& naming) {
    std::vector<Digraph::Edge> file_edges(edges.size());
    std::ranges::transform(edges, file_edges.begin(), [&naming](const parse::IncludeEdge& edge) {
        return Digraph::Edge{naming.relative(edge.from), naming.relative(edge.to)};
    });
    std::vector<std::string> source_names(sources.size());
    std::ranges::transform(
        sources, source_names.begin(),
        [&naming](const std::filesystem::path& source) { return naming.relative(source); });
    Digraph files{file_edges, source_names};

    std::vector<std::string> file_module(files.size());
    for (std::size_t node = 0; node < files.size(); ++node) {
        file_module.at(node) = naming.modules().module_of(files.name(node));
    }
    std::map<std::pair<std::string, std::string>, std::size_t> coupling_counts;
    std::vector<Digraph::Edge> module_edges;
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            ++coupling_counts[{file_module.at(from), file_module.at(to)}];
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

    std::vector<ModuleCoupling> coupling;
    coupling.reserve(coupling_counts.size());
    for (const auto& [pair, count] : coupling_counts) {
        coupling.push_back({.from = pair.first, .to = pair.second, .edges = count});
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

namespace {

std::string cycle_field(const std::optional<std::size_t>& cycle) {
    return cycle.has_value() ? std::to_string(*cycle) : std::string{};
}

} // namespace

void write_include_edges(std::ostream& out, const IncludeGraph& graph) {
    out << "from,to,from_module,to_module\n";
    const Digraph& files = graph.files;
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            out << csv_field(files.name(from)) << ',' << csv_field(files.name(to)) << ','
                << csv_field(graph.file_module.at(from)) << ','
                << csv_field(graph.file_module.at(to)) << '\n';
        }
    }
}

void write_include_files(std::ostream& out, const IncludeGraph& graph) {
    out << "file,module,fan_in,fan_out,cycle\n";
    const Digraph& files = graph.files;
    for (std::size_t node = 0; node < files.size(); ++node) {
        out << csv_field(files.name(node)) << ',' << csv_field(graph.file_module.at(node)) << ','
            << files.fan_in(node) << ',' << files.fan_out(node) << ','
            << cycle_field(graph.file_cycle.at(node)) << '\n';
    }
}

void write_include_modules(std::ostream& out, const IncludeGraph& graph) {
    out << "module,files,fan_in,fan_out,cycle\n";
    const Digraph& modules = graph.modules;
    for (std::size_t node = 0; node < modules.size(); ++node) {
        out << csv_field(modules.name(node)) << ',' << graph.module_files.at(node) << ','
            << modules.fan_in(node) << ',' << modules.fan_out(node) << ','
            << cycle_field(graph.module_cycle.at(node)) << '\n';
    }
}

namespace {

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
            << csv_field(members) << '\n';
    }
}

} // namespace

void write_include_cycles(std::ostream& out, const IncludeGraph& graph) {
    out << "level,cycle,size,members\n";
    write_cycle_rows("file", graph.files, graph.file_cycles, out);
    write_cycle_rows("module", graph.modules, graph.module_cycles, out);
}

void write_include_coupling(std::ostream& out, const IncludeGraph& graph) {
    out << "from_module,to_module,edges\n";
    for (const ModuleCoupling& cell : graph.coupling) {
        out << csv_field(cell.from) << ',' << csv_field(cell.to) << ',' << cell.edges << '\n';
    }
}

namespace {

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

} // namespace

void write_include_dot(std::ostream& out, const IncludeGraph& graph) {
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

} // namespace tezcatl::report
