#include "parse/compilation_database.hpp"
#include "parse/functions.hpp"
#include "parse/merge.hpp"
#include "parse/translation_unit.hpp"
#include "scan/paths.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::parse;

namespace {

fs::path fixture_dir() {
    return fs::path{TEZCATL_FIXTURES_DIR} / "functions";
}
fs::path fixture_db() {
    return fs::path{TEZCATL_FIXTURE_DBS} / "functions";
}

// "file:line:column kind name", file relative to the fixture: one readable
// string per function, so a failure prints exactly which row differs.
std::vector<std::string> as_rows(const std::vector<FunctionInfo>& functions) {
    std::vector<std::string> rows(functions.size());
    std::ranges::transform(functions, rows.begin(), [](const FunctionInfo& f) {
        return f.file.lexically_relative(fixture_dir()).generic_string() + ':' +
               std::to_string(f.line) + ':' + std::to_string(f.column) + ' ' +
               std::string{to_string(f.kind)} + ' ' + f.name;
    });
    return rows;
}

} // namespace

TEST_CASE("the compilation database loads with absolute file paths", "[parse]") {
    const auto commands = load_compilation_database(fixture_db());
    REQUIRE(commands.size() == 2);
    CHECK(commands.at(0).file == (fixture_dir() / "a.cpp").lexically_normal());
    CHECK(commands.at(1).file == (fixture_dir() / "b.cpp").lexically_normal());
    CHECK(commands.at(0).arguments.at(0) == "clang++");
}

TEST_CASE("a directory without compile_commands.json is an error", "[parse]") {
    CHECK_THROWS_AS(load_compilation_database(fixture_dir() / "include"), std::runtime_error);
}

TEST_CASE("every function definition in the fixture project is found", "[parse]") {
    // The same sources compiled GCC-style and MSVC-style (clang-cl, with /WX)
    // must give the same list.
    const fs::path database =
        GENERATE(fixture_db(), fs::path{TEZCATL_FIXTURE_DBS} / "functions-cl");
    INFO(database.string());
    const Parser parser{default_resource_directory()};
    std::vector<FunctionInfo> functions;
    for (const CompileCommand& command : load_compilation_database(database)) {
        const ParsedUnit parsed = parser.parse(command);
        // Zero errors proves the resource directory and the working directory
        // worked: b.cpp includes <cpuid.h>, which only clang's resource
        // directory provides (stddef.h is also in the OS headers, so it proves
        // nothing on Windows), and a.cpp needs the relative -Iinclude.
        INFO(parsed.file.string());
        REQUIRE(parsed.errors.empty());
        REQUIRE(parsed.unit);
        const auto found = find_functions(parsed, [](const fs::path& file) {
            return tezcatl::scan::is_within(file, fixture_dir());
        });
        functions.insert(functions.end(), found.begin(), found.end());
    }
    merge_duplicates(functions);

    // Hand-listed from the fixture sources before the code was run.
    // Declarations, "= default" and "= delete" have no body and are absent;
    // shared.hpp is included by both units and each function appears once.
    // Names are libclang's display names, which do not spell a function
    // template's parameters: "scaled(T)", not "scaled<T>(T)" (the list first
    // guessed the latter; the kind column already marks it as a template).
    const std::vector<std::string> expected{
        "a.cpp:5:5 function (anonymous namespace)::helper(int)",
        "a.cpp:9:5 function geo::declared_only(int)",
        "a.cpp:15:14 constructor Shape::Shape(int)",
        "a.cpp:16:5 destructor Shape::~Shape()",
        "a.cpp:18:5 conversion Shape::operator int()",
        "a.cpp:19:29 function_template Shape::scaled(T)",
        "a.cpp:25:5 function use_lambda()",
        "a.cpp:26:18 lambda use_lambda()::(lambda)",
        // Produced by a macro: located where the macro is used.
        "a.cpp:31:1 function forty_two()",
        // Counted's three "= default" members are absent, although clang
        // synthesizes bodies for them (out of line, or used by copy_of).
        "a.cpp:42:9 function copy_of(const Counted &)",
        "b.cpp:3:5 function from_b()",
        "include/util/detail.hpp:4:12 function util::one()",
        "shared.hpp:9:9 method geo::Point::sum()",
        "shared.hpp:12:15 function geo::area(size_t, size_t)",
    };
    CHECK(as_rows(functions) == expected);
}

TEST_CASE("functions in system headers are never reported, whatever the filter", "[parse]") {
    // b.cpp includes <cpuid.h>, which defines inline functions such as
    // __get_cpuid. The filter accepts everything, so only the system-header
    // rule can keep them out.
    const Parser parser{default_resource_directory()};
    const auto commands = load_compilation_database(fixture_db());
    const ParsedUnit parsed = parser.parse(commands.at(1));
    REQUIRE(parsed.errors.empty());
    const auto found = find_functions(parsed, [](const fs::path&) { return true; });
    std::vector<std::string> names(found.size());
    std::ranges::transform(found, names.begin(), [](const FunctionInfo& f) { return f.name; });
    const std::vector<std::string> expected{"from_b()", "geo::Point::sum()",
                                            "geo::area(size_t, size_t)"};
    std::ranges::sort(names);
    CHECK(names == expected);
}

TEST_CASE("a unit that fails to parse reports errors and keeps what it found", "[parse]") {
    const Parser parser{default_resource_directory()};
    const auto commands =
        load_compilation_database(fs::path{TEZCATL_FIXTURE_DBS} / "functions-broken");
    REQUIRE(commands.size() == 1);
    const ParsedUnit parsed = parser.parse(commands.at(0));
    REQUIRE_FALSE(parsed.errors.empty());
    CHECK(parsed.errors.at(0).message.find("missing.hpp") != std::string::npos);
    REQUIRE(parsed.unit);
    const auto found = find_functions(parsed, [](const fs::path&) { return true; });
    REQUIRE(found.size() == 1);
    CHECK(found.at(0).name == "still_found()");
}
