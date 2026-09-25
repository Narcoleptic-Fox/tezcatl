#include "scan/glob.hpp"

#include <catch2/catch_test_macros.hpp>

using tezcatl::scan::glob_match;

TEST_CASE("literal characters and ? match one character each", "[glob]") {
    CHECK(glob_match("src/a.c", "src/a.c"));
    CHECK_FALSE(glob_match("src/a.c", "src/a.cpp")); // the whole path, not a prefix
    CHECK_FALSE(glob_match("src/a.c", "SRC/a.c"));
    CHECK(glob_match("src/?.c", "src/b.c"));
    CHECK_FALSE(glob_match("src?a.c", "src/a.c"));
}

TEST_CASE("* stays within one path component", "[glob]") {
    CHECK(glob_match("src/*.c", "src/a.c"));
    CHECK(glob_match("src/*.c", "src/.c"));
    CHECK_FALSE(glob_match("src/*.c", "src/sub/a.c"));
    CHECK(glob_match("*_test.*", "loc_test.cpp"));
    CHECK_FALSE(glob_match("*_test.*", "tests/loc_test.cpp"));
}

TEST_CASE("** crosses path components", "[glob]") {
    CHECK(glob_match("src/**", "src/a.c"));
    CHECK(glob_match("src/**", "src/x/y/a.c"));
    CHECK_FALSE(glob_match("src/**", "lib/src/a.c"));
    CHECK(glob_match("**/test/**", "a/b/test/c.cpp"));
    CHECK(glob_match("**/test/**", "test/c.cpp")); // "**/" matches no directories
    CHECK(glob_match("src/**/a.c", "src/a.c"));
    CHECK(glob_match("src/**/a.c", "src/x/y/a.c"));
    CHECK_FALSE(glob_match("src/**/a.c", "src/x/ya.c"));
    CHECK(glob_match("**.h", "include/x/y.h"));
}
