#pragma once

#include "cli/project.hpp"

#include <cstdint>

namespace tezcatl::cli {

/// The tables `tezcatl includes` can write.
enum class IncludesOutput : std::uint8_t {
    edges,    ///< from,to,from_module,to_module: one row per include edge
    files,    ///< file,module,fan_in,fan_out,cycle: one row per file
    modules,  ///< module,files,fan_in,fan_out,cycle: one row per module
    cycles,   ///< level,cycle,size,members: one row per file or module cycle
    coupling, ///< from_module,to_module,edges: the coupling matrix, nonzero cells
    dot,      ///< the file graph in Graphviz DOT, clustered by module
};

struct IncludesOptions {
    ProjectOptions project;
    IncludesOutput output = IncludesOutput::edges;
};

/// `tezcatl includes -p BUILD_DIR`: the project's #include graph at file
/// level, resolved through each unit's own include paths, and aggregated to
/// modules. Only files of the project are nodes: system headers and files
/// under the build directory are left out. Fan-in and fan-out count distinct
/// files (or modules); a cycle is a strongly connected component of more
/// than one node, numbered from 1 in the order of its first member. At
/// module level an edge within a module is not a dependency, so it counts
/// in the coupling matrix but not in module fan-in, fan-out or cycles.
///
/// Parse errors are written to `err`, and make the exit code 1 unless
/// allowed, since a unit that failed to parse may be missing edges.
[[nodiscard]] int run_includes(const IncludesOptions& options, const Streams& streams);

} // namespace tezcatl::cli
