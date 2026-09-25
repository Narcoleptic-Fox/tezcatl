#pragma once

#include "graph/digraph.hpp"
#include "parse/includes.hpp"
#include "report/naming.hpp"

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace tezcatl::report {

/// How many include edges run from one module's files to another's.
struct ModuleCoupling {
    std::string from;
    std::string to;
    std::size_t edges = 0;
};

/// The project's include graph at file level, aggregated to modules, with
/// its cycles, as docs/metrics.md defines them.
struct IncludeGraph {
    graph::Digraph files;                 ///< nodes named relative to the root
    std::vector<std::string> file_module; ///< per file node
    std::vector<std::vector<std::size_t>> file_cycles;
    std::vector<std::optional<std::size_t>> file_cycle; ///< 1-based, per file node
    graph::Digraph modules;                             ///< edges between distinct modules
    std::vector<std::vector<std::size_t>> module_cycles;
    std::vector<std::optional<std::size_t>> module_cycle; ///< 1-based, per module node
    std::vector<std::size_t> module_files;                ///< file count, per module node
    std::vector<ModuleCoupling> coupling;                 ///< sorted by module pair
};

/// The graph of `edges`, with every file in `sources` a node even if it has
/// no edges.
[[nodiscard]] IncludeGraph build_include_graph(const std::vector<parse::IncludeEdge>& edges,
                                               const std::vector<std::filesystem::path>& sources,
                                               const FileNaming& naming);

/// from,to,from_module,to_module: one row per include edge.
void write_include_edges(std::ostream& out, const IncludeGraph& graph);
/// file,module,fan_in,fan_out,cycle: one row per file.
void write_include_files(std::ostream& out, const IncludeGraph& graph);
/// module,files,fan_in,fan_out,cycle: one row per module.
void write_include_modules(std::ostream& out, const IncludeGraph& graph);
/// level,cycle,size,members: one row per file or module cycle.
void write_include_cycles(std::ostream& out, const IncludeGraph& graph);
/// from_module,to_module,edges: the nonzero cells of the coupling matrix.
void write_include_coupling(std::ostream& out, const IncludeGraph& graph);
/// The file graph in Graphviz DOT, clustered by module, cycle edges in red.
void write_include_dot(std::ostream& out, const IncludeGraph& graph);

} // namespace tezcatl::report
