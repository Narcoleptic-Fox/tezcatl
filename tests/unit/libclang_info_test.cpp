#include "parse/libclang_info.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("libclang reports its release version at run time", "[parse]") {
    const std::string version = tezcatl::parse::libclang_version();

    // Shape, not just non-empty: vendor builds prefix the string (for
    // example "Ubuntu clang version 18.1.3"), so search rather than compare.
    const auto marker = version.find("clang version ");
    REQUIRE(marker != std::string::npos);

    const auto first_digit = marker + std::string{"clang version "}.size();
    REQUIRE(first_digit < version.size());
    CHECK(version.at(first_digit) >= '1');
    CHECK(version.at(first_digit) <= '9');
}
