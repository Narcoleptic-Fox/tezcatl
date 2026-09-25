#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace tezcatl::graph {

/// A directed graph over named nodes, without parallel edges. Nodes are
/// numbered in name order and successor lists are sorted, so everything
/// derived from a graph comes out in the same order on every platform.
class Digraph {
public:
    using Edge = std::pair<std::string, std::string>;

    /// An empty graph.
    Digraph() = default;
    /// Nodes are the endpoints of `edges` plus `isolated` (nodes that may
    /// have no edges, such as a source file nothing includes). Repeated
    /// nodes and edges are merged. A self-edge is kept.
    Digraph(const std::vector<Edge>& edges, const std::vector<std::string>& isolated);

    [[nodiscard]] std::size_t size() const noexcept { return names_.size(); }
    [[nodiscard]] const std::string& name(std::size_t node) const { return names_.at(node); }
    /// True if a node is named `name`.
    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] const std::vector<std::size_t>& successors(std::size_t node) const {
        return successors_.at(node);
    }
    /// Number of distinct nodes with an edge to `node`.
    [[nodiscard]] std::size_t fan_in(std::size_t node) const { return fan_in_.at(node); }
    /// Number of distinct nodes `node` has an edge to.
    [[nodiscard]] std::size_t fan_out(std::size_t node) const {
        return successors_.at(node).size();
    }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edge_count_; }

private:
    std::vector<std::string> names_;
    std::vector<std::vector<std::size_t>> successors_;
    std::vector<std::size_t> fan_in_;
    std::size_t edge_count_ = 0;
};

/// The cycles of `graph`: its strongly connected components of more than one
/// node (Tarjan's algorithm). Each component lists its nodes in ascending
/// order, and components are ordered by their first node. A self-edge alone
/// is not a cycle. Iterative, so a long chain cannot overflow the stack.
[[nodiscard]] std::vector<std::vector<std::size_t>> cycles(const Digraph& graph);

} // namespace tezcatl::graph
