#include "parse/position_index.hpp"

#include <catch2/catch_test_macros.hpp>

using tezcatl::parse::FileKey;
using tezcatl::parse::PositionIndex;

namespace {

constexpr FileKey file_a{1, 1, 1};
constexpr FileKey file_b{2, 2, 2};

// Declarations of two files in scrambled order, as a scope's children arrive
// when one header includes another part way through.
PositionIndex scrambled() {
    return PositionIndex{
        {{.file = file_a, .offset = 500},
         {.file = file_b, .offset = 10},
         {.file = file_a, .offset = 100},
         {.file = file_b, .offset = 300},
         {.file = file_a, .offset = 50},
         {.file = file_b, .offset = 700},
         {.file = file_a, .offset = 900}},
        {{.file = file_b, .begin = 200, .end = 400}, {.file = file_a, .begin = 20, .end = 80}}};
}

} // namespace

TEST_CASE("a declaration strictly between two offsets of its file is found", "[index]") {
    const PositionIndex index = scrambled();
    CHECK(index.declaration_between(file_a, {.after = 60, .before = 200}));  // a:100
    CHECK(index.declaration_between(file_a, {.after = 100, .before = 600})); // a:500
    CHECK(index.declaration_between(file_b, {.after = 10, .before = 301}));  // b:300
    CHECK_FALSE(index.declaration_between(file_a, {.after = 100, .before = 500}));
    CHECK_FALSE(index.declaration_between(file_a, {.after = 500, .before = 900}));
    CHECK_FALSE(index.declaration_between(file_b, {.after = 300, .before = 700}));
}

TEST_CASE("another file's declarations are never between", "[index]") {
    // Past file a's last declaration the next position is file b's, at a
    // small offset: it must not count as a:10.
    const PositionIndex index = scrambled();
    CHECK_FALSE(index.declaration_between(file_a, {.after = 900, .before = 5000}));
    CHECK_FALSE(index.declaration_between(FileKey{3, 3, 3}, {.after = 0, .before = 5000}));
}

TEST_CASE("a position strictly inside a typedef of its file", "[index]") {
    const PositionIndex index = scrambled();
    CHECK(index.inside_typedef(file_a, 50));
    CHECK(index.inside_typedef(file_b, 250));
    CHECK_FALSE(index.inside_typedef(file_a, 20)); // the typedef's own first byte
    CHECK_FALSE(index.inside_typedef(file_a, 250));
    CHECK_FALSE(index.inside_typedef(file_b, 50));
}
