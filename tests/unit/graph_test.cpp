#include "graph/digraph.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

using tezcatl::graph::cycles;
using tezcatl::graph::Digraph;

namespace {

// Cycles by node name, for readable failures.
std::vector<std::vector<std::string>> named_cycles(const Digraph& graph) {
    std::vector<std::vector<std::string>> named;
    for (const auto& cycle : cycles(graph)) {
        std::vector<std::string> names(cycle.size());
        std::ranges::transform(cycle, names.begin(),
                               [&graph](std::size_t node) { return graph.name(node); });
        named.push_back(names);
    }
    return named;
}

} // namespace

TEST_CASE("nodes are numbered in name order and repeats merge", "[graph]") {
    const Digraph graph{{{"b", "a"}, {"b", "a"}, {"b", "c"}, {"c", "a"}}, {"d", "a"}};
    REQUIRE(graph.size() == 4);
    CHECK(graph.name(0) == "a");
    CHECK(graph.name(3) == "d");
    CHECK(graph.edge_count() == 3);
    CHECK(graph.fan_in(0) == 2);  // a <- b, c (the repeated b -> a counts once)
    CHECK(graph.fan_out(1) == 2); // b -> a, c
    CHECK(graph.fan_in(3) == 0);
    CHECK(graph.fan_out(3) == 0);
}

TEST_CASE("a three-node cycle with a tail is exactly one cycle", "[graph]") {
    const Digraph graph{{{"a", "b"}, {"b", "c"}, {"c", "a"}, {"c", "d"}, {"e", "a"}}, {}};
    const std::vector<std::vector<std::string>> expected{{"a", "b", "c"}};
    CHECK(named_cycles(graph) == expected);
}

TEST_CASE("separate cycles are reported separately, in order", "[graph]") {
    const Digraph graph{{{"x", "y"}, {"y", "x"}, {"b", "a"}, {"a", "b"}, {"y", "a"}}, {}};
    const std::vector<std::vector<std::string>> expected{{"a", "b"}, {"x", "y"}};
    CHECK(named_cycles(graph) == expected);
}

TEST_CASE("a diamond, a self-edge and an empty graph have no cycles", "[graph]") {
    CHECK(cycles(Digraph{{{"a", "b"}, {"a", "c"}, {"b", "d"}, {"c", "d"}}, {}}).empty());
    CHECK(cycles(Digraph{{{"a", "a"}}, {}}).empty());
    CHECK(cycles(Digraph{{}, {}}).empty());
}

TEST_CASE("nested cycles sharing nodes form one component", "[graph]") {
    // a -> b -> a and b -> c -> d -> b share b.
    const Digraph graph{{{"a", "b"}, {"b", "a"}, {"b", "c"}, {"c", "d"}, {"d", "b"}}, {}};
    const std::vector<std::vector<std::string>> expected{{"a", "b", "c", "d"}};
    CHECK(named_cycles(graph) == expected);
}

TEST_CASE("a very long cycle does not overflow the stack", "[graph]") {
    // 200,000 nodes in one ring: recursive Tarjan would recurse that deep.
    constexpr std::size_t length = 200'000;
    std::vector<Digraph::Edge> edges;
    edges.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        edges.emplace_back(std::to_string(i), std::to_string((i + 1) % length));
    }
    const auto found = cycles(Digraph{edges, {}});
    REQUIRE(found.size() == 1);
    CHECK(found.at(0).size() == length);
}
