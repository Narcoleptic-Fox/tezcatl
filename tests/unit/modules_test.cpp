#include "config/modules.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <stdexcept>

using tezcatl::config::ModuleMap;

TEST_CASE("the first matching rule names the module", "[modules]") {
    const ModuleMap map = ModuleMap::parse("# a layered project\n"
                                           "\n"
                                           "  core tests = core/test/**  \n"
                                           "core = core/**\r\n"
                                           "SA/DM = dm/**\n"
                                           "SA/DM = sa/**",
                                           "modules.txt");
    CHECK(map.rules().size() == 4);
    CHECK(map.module_of("core/test/t.cpp") == "core tests");
    CHECK(map.module_of("core/src/e.cpp") == "core");
    CHECK(map.module_of("sa/x.c") == "SA/DM");
    CHECK(map.module_of("io/f.cpp") == "(unassigned)");
    CHECK(ModuleMap{}.module_of("anything.c") == "(unassigned)");
}

TEST_CASE("malformed rules are rejected with their line number", "[modules]") {
    using Catch::Matchers::ContainsSubstring;
    CHECK_THROWS_WITH(ModuleMap::parse("A = a/**\nB b/**\n", "m.txt"),
                      ContainsSubstring("m.txt:2: no '='"));
    CHECK_THROWS_WITH(ModuleMap::parse(" = a/**", "m.txt"),
                      ContainsSubstring("m.txt:1: no module name"));
    CHECK_THROWS_WITH(ModuleMap::parse("A =  ", "m.txt"), ContainsSubstring("m.txt:1: no glob"));
    CHECK_THROWS_WITH(ModuleMap::parse("(unassigned) = x/**", "m.txt"),
                      ContainsSubstring("reserved"));
}

TEST_CASE("a missing module map file is an error", "[modules]") {
    CHECK_THROWS_AS(ModuleMap::load("no/such/modules.txt"), std::runtime_error);
}
