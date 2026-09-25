#include "parse/compilation_database.hpp"
#include "parse/includes.hpp"
#include "parse/translation_unit.hpp"
#include "scan/paths.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::parse;

TEST_CASE("every include edge in the fixture project is found", "[includes]") {
    const fs::path fixture = fs::path{TEZCATL_FIXTURES_DIR} / "includes";
    const Parser parser{default_resource_directory()};
    std::vector<IncludeEdge> edges;
    for (const CompileCommand& command :
         load_compilation_database(fs::path{TEZCATL_FIXTURE_DBS} / "includes")) {
        const ParsedUnit parsed = parser.parse(command);
        INFO(parsed.file.string());
        REQUIRE(parsed.errors.empty());
        const auto found = find_includes(parsed, [&fixture](const fs::path& file) {
            return tezcatl::scan::is_within(file, fixture);
        });
        edges.insert(edges.end(), found.begin(), found.end());
    }
    std::ranges::sort(edges);
    const auto repeats = std::ranges::unique(edges);
    edges.erase(repeats.begin(), repeats.end());

    std::vector<std::string> rows(edges.size());
    std::ranges::transform(edges, rows.begin(), [&fixture](const IncludeEdge& edge) {
        return edge.from.lexically_relative(fixture).generic_string() + " -> " +
               edge.to.lexically_relative(fixture).generic_string();
    });
    std::ranges::sort(rows);

    // Hand-listed from the fixture's #include lines before the code was run.
    // x.h's two includes and c.h's include of a.h are skipped by include
    // guards when main.cpp is preprocessed, and are edges all the same.
    // <stddef.h> is a system header, and main.cpp's second include of y.h
    // is the same edge again.
    // clang-format off: one edge per line, to be read against the fixture
    const std::vector<std::string> expected{
        "main.cpp -> include/lib/util.h",
        "main.cpp -> ring/a.h",
        "main.cpp -> shared/x.h",
        "main.cpp -> shared/y.h",
        "other.cpp -> shared/y.h",
        "ring/a.h -> ring/b.h",
        "ring/b.h -> ring/c.h",
        "ring/c.h -> ring/a.h",
        "ring/c.h -> shared/common.h",
        "shared/x.h -> ring/a.h",
        "shared/x.h -> shared/common.h",
        "shared/y.h -> shared/common.h",
    };
    // clang-format on
    CHECK(rows == expected);
}

TEST_CASE("directives inside system headers are never edges, whatever the filter", "[includes]") {
    // main.cpp reaches <stddef.h> through lib/util.h, and clang's stddef.h
    // includes further headers of its own. The filter accepts everything, so
    // only the system-header rule can keep those directives out.
    const fs::path fixture = fs::path{TEZCATL_FIXTURES_DIR} / "includes";
    const Parser parser{default_resource_directory()};
    const auto commands = load_compilation_database(fs::path{TEZCATL_FIXTURE_DBS} / "includes");
    const ParsedUnit parsed = parser.parse(commands.at(0));
    REQUIRE(parsed.errors.empty());
    const auto edges = find_includes(parsed, [](const fs::path&) { return true; });
    REQUIRE_FALSE(edges.empty());
    for (const IncludeEdge& edge : edges) {
        INFO(edge.from.string() + " -> " + edge.to.string());
        CHECK(tezcatl::scan::is_within(edge.from, fixture));
    }
}
