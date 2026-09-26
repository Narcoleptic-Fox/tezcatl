#include "analysis/functions.hpp"
#include "parse/compilation_database.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::parse;
using tezcatl::analysis::FunctionInfo;
using tezcatl::analysis::measure_functions;

TEST_CASE("cyclomatic complexity matches the hand counts in the fixture", "[complexity]") {
    // The same source as a GCC-style C++20 command and as a clang-cl C++17
    // one; clang-cl before C++20 delays parsing template bodies by default.
    const fs::path database = GENERATE(fs::path{TEZCATL_FIXTURE_DBS} / "complexity",
                                       fs::path{TEZCATL_FIXTURE_DBS} / "complexity-cl");
    INFO(database.string());
    const Parser parser{default_resource_directory()};
    const auto commands = load_compilation_database(database);
    REQUIRE(commands.size() == 1);
    const ParsedUnit parsed = parser.parse(commands.at(0));
    REQUIRE(parsed.errors.empty());
    const auto functions = measure_functions(
        parsed, [](const fs::path& file) { return file.filename() == "complexity.cpp"; });
    std::vector<std::string> rows(functions.size());
    std::ranges::transform(functions, rows.begin(), [](const FunctionInfo& f) {
        return f.name + " = " + std::to_string(f.complexity);
    });

    // The values are the ones written above each function in
    // tests/fixtures/complexity/complexity.cpp, counted by hand. Only the
    // spelling of the pointer parameters, "int *", was taken from libclang.
    const std::vector<std::string> expected{
        "straight(int) = 1",
        "ladder(int) = 3",
        "logic(int, int, int) = 6",
        "loops(int) = 5",
        "choose(int) = 4",
        "guarded(int) = 4",
        "pick(int *, int *) = 3",
        "outer(int) = 2",
        "outer(int)::(lambda) = 3",
        "with_local(int) = 1",
        "with_local(int)::Local::sign(int) = 2",
        "macros(int, int) = 3",
        "macro_lambda(int, int) = 2",
        "macro_lambda(int, int)::(lambda) = 2",
        "from_macro(int) = 1",
        "not_decisions(int) = 1",
        "operator&&(Flag, Flag) = 1",
        "combine(Flag, Flag) = 1",
        "dependent_and(T, T) = 2",
        "call_by_name(T, T) = 1",
        "all_of(T...) = 2",
        "any_of(T...) = 2",
        "Box::get_or(T) = 2",
        "clamp_small(T) = 3",
        "Holder::Holder(int) = 2",
        "eleven(int) = 12",
    };
    CHECK(rows == expected);
}
