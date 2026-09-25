#include "scan/source_files.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
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

TEST_CASE("a directory the walk may not descend into is never entered", "[scan]") {
    const fs::path root = fs::temp_directory_path() / "tezcatl-prune-test";
    fs::remove_all(root);
    fs::create_directories(root / "keep");
    fs::create_directories(root / "build" / "deeper");
    for (const fs::path& file : {root / "keep" / "a.c", root / "build" / "b.c",
                                 root / "build" / "deeper" / "c.c", root / "d.c"}) {
        std::ofstream{file} << "int x;\n";
    }
    std::vector<fs::path> asked;
    const auto files = tezcatl::scan::find_source_files_under(root, [&](const fs::path& dir) {
        asked.push_back(dir);
        return dir.filename() != "build";
    });
    fs::remove_all(root);

    REQUIRE(files.size() == 2);
    CHECK(files.at(0).filename() == "d.c");
    CHECK(files.at(1).filename() == "a.c");
    // Pruned, not filtered: nothing below build/ was even looked at.
    CHECK(asked.size() == 2);
    CHECK(std::ranges::none_of(asked, [](const fs::path& p) { return p.filename() == "deeper"; }));
}

TEST_CASE("a path that does not exist is an error, not an empty result", "[scan]") {
    CHECK_THROWS_AS(find_source_files({fs::path{TEZCATL_FIXTURES_DIR} / "no-such-dir"}),
                    std::runtime_error);
}
