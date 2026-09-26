#include "analysis/functions.hpp"
#include "parse/merge.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

using tezcatl::analysis::FunctionInfo;
using tezcatl::parse::merge_duplicates;

TEST_CASE("of repeats at one location, the first seen is kept", "[merge]") {
    // A file compiled several ways gives one function several complexities
    // at one location; the first compile command's must win, on every
    // standard library. Small inputs cannot show this: below 16 elements
    // both libstdc++ and MSVC sort by insertion, which happens to be stable.
    constexpr unsigned locations = 10;
    constexpr unsigned repeats = 100;
    std::vector<FunctionInfo> functions;
    functions.reserve(std::size_t{locations} * repeats);
    for (unsigned i = 0; i < locations * repeats; ++i) {
        // Interleaved, so every location's first entry comes early and its
        // later entries (other complexities) are spread through the input.
        functions.push_back({.file = "a.c",
                             .line = locations - (i % locations),
                             .column = 1,
                             .kind = tezcatl::parse::FunctionKind::function,
                             .name = "f",
                             .complexity = i + 1,
                             .halstead = {}});
    }
    merge_duplicates(functions);

    REQUIRE(functions.size() == locations);
    for (const FunctionInfo& function : functions) {
        // Line L was first given at i = locations - L.
        CHECK(function.complexity == locations - function.line + 1);
    }
}
