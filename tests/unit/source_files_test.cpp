#include "scan/source_files.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
using tezcatl::scan::find_source_files;
using tezcatl::scan::is_source_file;

TEST_CASE("source file extensions", "[scan]") {
    CHECK(is_source_file("a.c"));
    CHECK(is_source_file("a.cpp"));
    CHECK(is_source_file("dir/a.HPP"));
    CHECK(is_source_file("a.c++"));
    CHECK(is_source_file("a.inl"));
    CHECK_FALSE(is_source_file("a.txt"));
    CHECK_FALSE(is_source_file("a.cs"));
    CHECK_FALSE(is_source_file("Makefile"));
}

TEST_CASE("directories are walked for sources, sorted, without duplicates", "[scan]") {
    const fs::path dir = fs::path{TEZCATL_FIXTURES_DIR} / "loc";
    // The directory twice and one of its files again: each file appears once.
    const auto files = find_source_files({dir, dir, dir / "sample.cpp"});
    REQUIRE(files.size() == 2);
    CHECK(files.at(0).filename() == "sample.cpp");
    CHECK(files.at(1).filename() == "sample.hpp");
}

TEST_CASE("a path that does not exist is an error, not an empty result", "[scan]") {
    CHECK_THROWS_AS(find_source_files({fs::path{TEZCATL_FIXTURES_DIR} / "no-such-dir"}),
                    std::runtime_error);
}
