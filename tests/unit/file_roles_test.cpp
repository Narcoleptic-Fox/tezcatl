#include "config/file_roles.hpp"

#include <catch2/catch_test_macros.hpp>

using tezcatl::config::FileRoles;

TEST_CASE("the default test globs: test directories and *_test / test_* files", "[roles]") {
    const FileRoles roles;
    CHECK(roles.is_test("test/a.c"));
    CHECK(roles.is_test("src/tests/b.cpp"));
    CHECK(roles.is_test("src/parser/tests/unit/c.cpp"));
    CHECK(roles.is_test("parser_test.cpp"));
    CHECK(roles.is_test("src/test_parser.c"));
    CHECK_FALSE(roles.is_test("src/parser.c"));
    CHECK_FALSE(roles.is_test("src/testing/helpers.c")); // "testing" is not "test"
    CHECK_FALSE(roles.is_test("src/contest.c"));
    CHECK_FALSE(roles.is_test("src/attest_x.c")); // the name must start with test_
}

TEST_CASE("explicit test globs replace the defaults", "[roles]") {
    const FileRoles roles{{"qa/**"}};
    CHECK(roles.is_test("qa/check.c"));
    CHECK_FALSE(roles.is_test("test/a.c"));
}
