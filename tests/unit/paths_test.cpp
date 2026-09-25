#include "scan/paths.hpp"

#include <catch2/catch_test_macros.hpp>

using tezcatl::scan::is_within;

TEST_CASE("is_within judges containment by whole path components", "[paths]") {
    CHECK(is_within("/a/b/c.cpp", "/a/b"));
    CHECK(is_within("/a/b/c.cpp", "/a/b/"));
    CHECK(is_within("/a/b", "/a/b"));
    CHECK(is_within("/a/b/x/../c.cpp", "/a/b"));
    CHECK_FALSE(is_within("/a/bc/d.cpp", "/a/b")); // a prefix, not a parent
    CHECK_FALSE(is_within("/a/b/../c.cpp", "/a/b"));
    CHECK_FALSE(is_within("/a", "/a/b"));
}

#ifdef _WIN32
TEST_CASE("on Windows, is_within ignores case and separator style", "[paths]") {
    CHECK(is_within("d:/Dev/Proj/src/a.cpp", "D:\\dev\\proj"));
    CHECK_FALSE(is_within("D:/Dev/Projects/a.cpp", "D:/Dev/Proj"));
}
#endif
