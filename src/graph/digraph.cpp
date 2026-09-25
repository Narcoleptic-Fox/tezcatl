#include "graph/digraph.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

namespace tezcatl::graph {

Digraph::Digraph(const std::vector<Edge>& edges, const std::vector<std::string>& isolated)
    : names_(isolated) {
    for (const auto& [from, to] : edges) {
        names_.push_back(from);
        names_.push_back(to);
    }
    std::ranges::sort(names_);
    const auto repeats = std::ranges::unique(names_);
    names_.erase(repeats.begin(), repeats.end());

    const auto number = [this](const std::string& name) {
        return static_cast<std::size_t>(
            std::distance(names_.begin(), std::ranges::lower_bound(names_, name)));
    };
    successors_.resize(names_.size());
    for (const auto& [from, to] : edges) {
        successors_.at(number(from)).push_back(number(to));
    }
    fan_in_.resize(names_.size());
    for (std::vector<std::size_t>& targets : successors_) {
        std::ranges::sort(targets);
        const auto repeated = std::ranges::unique(targets);
        targets.erase(repeated.begin(), repeated.end());
        edge_count_ += targets.size();
        for (const std::size_t target : targets) {
            ++fan_in_.at(target);
        }
    }
}

namespace {

// Tarjan's strongly connected components algorithm. An explicit stack of
// (node, next successor to try) frames replaces recursion, whose depth would
// be the length of the longest path in the graph.
class Tarjan {
public:
    explicit Tarjan(const Digraph& graph)
        : graph_(&graph), order_(graph.size(), unvisited), low_(graph.size(), 0),
          on_stack_(graph.size(), false) {}

    std::vector<std::vector<std::size_t>> cycles() {
        for (std::size_t root = 0; root < graph_->size(); ++root) {
            if (order_.at(root) == unvisited) {
                search_from(root);
            }
        }
        std::ranges::sort(cycles_);
        return std::move(cycles_);
    }

private:
    static constexpr std::size_t unvisited = std::numeric_limits<std::size_t>::max();

    struct Frame {
        std::size_t node = 0;
        std::size_t next = 0;
    };

    void search_from(std::size_t root) {
        discover(root);
        while (!frames_.empty()) {
            Frame& frame = frames_.back();
            const std::vector<std::size_t>& successors = graph_->successors(frame.node);
            if (frame.next == successors.size()) {
                finish(frame.node);
                continue;
            }
            const std::size_t successor = successors.at(frame.next++);
            if (order_.at(successor) == unvisited) {
                discover(successor); // invalidates `frame`
            } else if (on_stack_.at(successor)) {
                low_.at(frame.node) = std::min(low_.at(frame.node), order_.at(successor));
            }
        }
    }

    void discover(std::size_t node) {
        order_.at(node) = counter_;
        low_.at(node) = counter_;
        ++counter_;
        stack_.push_back(node);
        on_stack_.at(node) = true;
        frames_.push_back({.node = node, .next = 0});
    }

    // Every successor of `node` is done: close its component if it is the
    // component's root, and pass its low link up to its parent.
    void finish(std::size_t node) {
        frames_.pop_back();
        if (low_.at(node) == order_.at(node)) {
            close_component(node);
        }
        if (!frames_.empty()) {
            const std::size_t parent = frames_.back().node;
            low_.at(parent) = std::min(low_.at(parent), low_.at(node));
        }
    }

    void close_component(std::size_t root) {
        std::vector<std::size_t> component;
        for (bool closed = false; !closed;) {
            const std::size_t member = stack_.back();
            stack_.pop_back();
            on_stack_.at(member) = false;
            component.push_back(member);
            closed = member == root;
        }
        if (component.size() > 1) {
            std::ranges::sort(component);
            cycles_.push_back(std::move(component));
        }
    }

    const Digraph* graph_;
    std::vector<std::size_t> order_; ///< Tarjan's index: the order of discovery
    std::vector<std::size_t> low_;   ///< Tarjan's low link
    std::vector<bool> on_stack_;
    std::vector<std::size_t> stack_;
    std::vector<Frame> frames_;
    std::size_t counter_ = 0;
    std::vector<std::vector<std::size_t>> cycles_;
};

} // namespace

std::vector<std::vector<std::size_t>> cycles(const Digraph& graph) {
    return Tarjan{graph}.cycles();
}

} // namespace tezcatl::graph
