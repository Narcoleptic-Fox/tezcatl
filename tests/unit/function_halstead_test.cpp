#include "parse/compilation_database.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::parse;

TEST_CASE("Halstead counts of each function match the hand counts in the fixture",
          "[halstead][parse]") {
    const Parser parser{default_resource_directory()};
    const auto commands = load_compilation_database(fs::path{TEZCATL_FIXTURE_DBS} / "halstead");
    REQUIRE(commands.size() == 1);
    const ParsedUnit parsed = parser.parse(commands.at(0));
    REQUIRE(parsed.errors.empty());
    const auto functions = find_functions(parsed, [](const fs::path&) { return true; });
    std::vector<std::string> rows(functions.size());
    std::ranges::transform(functions, rows.begin(), [](const FunctionInfo& f) {
        const auto& h = f.halstead;
        return f.name + " n1=" + std::to_string(h.distinct_operators) +
               " N1=" + std::to_string(h.total_operators) +
               " n2=" + std::to_string(h.distinct_operands) +
               " N2=" + std::to_string(h.total_operands);
    });

    // The counts written above each function in
    // tests/fixtures/halstead/halstead.cpp, made by hand.
    const std::vector<std::string> expected{
        "add(int, int) n1=7 N1=9 n2=3 N2=5",
        "is_empty(const char *) n1=11 N1=12 n2=5 N2=7",
        "guarded(int) n1=5 N1=7 n2=3 N2=4",
        "outer_sum(int) n1=7 N1=10 n2=3 N2=5",
        "outer_sum(int)::(lambda) n1=7 N1=7 n2=1 N2=3",
    };
    CHECK(rows == expected);
}
