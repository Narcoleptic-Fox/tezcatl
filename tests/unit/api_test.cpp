#include "parse/api.hpp"
#include "parse/compilation_database.hpp"
#include "parse/translation_unit.hpp"
#include "scan/paths.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::parse;

TEST_CASE("headers are recognised by their extension", "[api]") {
    CHECK(is_header("a/b.h"));
    CHECK(is_header("a/b.HPP"));
    CHECK(is_header("a/b.inl"));
    CHECK_FALSE(is_header("a/b.cpp"));
    CHECK_FALSE(is_header("a/b.c"));
    CHECK_FALSE(is_header("a/vector"));
}

TEST_CASE("the public API of the fixture headers and how each is documented", "[api]") {
    const fs::path fixture = fs::path{TEZCATL_FIXTURES_DIR} / "docs";
    const Parser parser{default_resource_directory()};
    std::vector<ApiEntity> entities;
    for (const CompileCommand& command :
         load_compilation_database(fs::path{TEZCATL_FIXTURE_DBS} / "docs")) {
        const ParsedUnit parsed = parser.parse(command);
        INFO(parsed.file.string());
        REQUIRE(parsed.errors.empty());
        const auto found = find_api(parsed, [&fixture](const fs::path& file) {
            return tezcatl::scan::is_within(file, fixture);
        });
        entities.insert(entities.end(), found.begin(), found.end());
    }
    merge_duplicates(entities);
    std::vector<std::string> rows(entities.size());
    std::ranges::transform(entities, rows.begin(), [&fixture](const ApiEntity& e) {
        return e.file.lexically_relative(fixture).generic_string() + ':' + std::to_string(e.line) +
               ' ' + std::string{to_string(e.kind)} + ' ' + e.name + ' ' +
               std::string{to_string(e.documentation)};
    });

    // Hand-listed from the fixture before the code was run; afterwards two
    // things were corrected: the line numbers from 50 on (the list had
    // miscounted Account's body), and the qualifier of code, which libclang
    // spells status_t::. Absent on purpose: defaulted and deleted members,
    // private members and the private Ledger type with its field, the
    // anonymous namespace, static functions, enumerators, the struct defined
    // inside the typedef status_t (the typedef is the type's name; counting
    // both counted one type twice, which the first run showed), and
    // declarations in .cpp and .c files. identity has no documentation: the
    // comment above it belongs to from_macro, although libclang attaches it
    // to both. The comment on undocumented's definition in src/api.cpp does
    // not document the header.
    // clang-format off: one entity per line, to be read against the fixture
    const std::vector<std::string> expected{
        "include/api.hpp:8 function add(int, int) doxygen",
        "include/api.hpp:10 function undocumented(int) none",
        "include/api.hpp:13 function subtract(int, int) plain",
        "include/api.hpp:16 function checked() doxygen",
        "include/api.hpp:19 type Point doxygen",
        "include/api.hpp:20 field Point::x doxygen",
        "include/api.hpp:21 field Point::y none",
        "include/api.hpp:23 method Point::norm() doxygen",
        "include/api.hpp:28 type Account none",
        "include/api.hpp:31 method Account::balance() doxygen",
        "include/api.hpp:32 method Account::deposit(long) none",
        "include/api.hpp:50 function twice(int) none",
        "include/api.hpp:55 type Shape doxygen",
        "include/api.hpp:59 function from_macro() doxygen",
        "include/api.hpp:61 function identity(T) none",
        "include/api.hpp:64 type_alias Meters doxygen",
        "include/api.hpp:68 function geo::distance(const Point &, const Point &) doxygen",
        "include/c_api.h:7 function max_of(int, int) plain",
        "include/c_api.h:9 function min_of(int, int) none",
        "include/c_api.h:14 type buffer plain",
        "include/c_api.h:15 field buffer::data plain",
        "include/c_api.h:16 field buffer::size none",
        "include/c_api.h:20 function buffer_free(struct buffer *) doxygen",
        "include/c_api.h:23 field status_t::code none",
        "include/c_api.h:24 type_alias status_t none",
        "include/c_api.h:26 variable error_count none",
        "include/elsewhere.hpp:2 function defined_elsewhere(int) none",
    };
    // clang-format on
    CHECK(rows == expected);
}
