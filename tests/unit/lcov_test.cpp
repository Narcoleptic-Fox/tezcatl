#include "coverage/readers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
using namespace tezcatl::coverage;

namespace {

CoverageData read(const std::string& text) {
    std::istringstream in{text};
    CoverageData data;
    read_lcov(in, fs::path{"/base"}, "test.info", data);
    return data;
}

} // namespace

TEST_CASE("an lcov tracefile is merged per file, line, branch and function", "[coverage]") {
    // /src/a.c appears in two records, as when two test binaries cover it.
    // By hand: lines 3 (1 + 0), 4 (1), 10 (0 + 2), 11 (0): 4 lines, 3
    // covered; line 3's second record has 0 hits, so replacing instead of
    // adding would lose it.
    // Branches 4/0,0 (1), 4/0,1 (0 + 3), 4/1,0 (never evaluated): 3, 2
    // covered. Functions main (1), helper (0 + 2): 2, both covered.
    // rel/b.cpp, relative to the base: FNA (lcov 2.2) gives add 4 hits, sub
    // has no hit record: 2 functions, 1 covered; one line, covered.
    const CoverageData data = read("TN:\n"
                                   "SF:/src/a.c\n"
                                   "FN:3,main\n"
                                   "FN:10,14,helper\n"
                                   "FNDA:1,main\n"
                                   "FNDA:0,helper\n"
                                   "DA:3,1\n"
                                   "DA:4,1,9f86d081884c7d65\n"
                                   "DA:10,0\n"
                                   "BRDA:4,0,0,1\n"
                                   "BRDA:4,0,1,0\n"
                                   "BRDA:4,1,0,-\n"
                                   "LF:3\n"
                                   "LH:2\n"
                                   "end_of_record\n"
                                   "SF:/src/a.c\n"
                                   "FNDA:2,helper\n"
                                   "DA:3,0\n"
                                   "DA:10,2\n"
                                   "DA:11,0\n"
                                   "BRDA:4,0,1,3\n"
                                   "end_of_record\n"
                                   "SF:rel/b.cpp\r\n"
                                   "FNL:0,5,7\r\n"
                                   "FNA:0,4,add(int, int)\r\n"
                                   "FN:20,sub(int, int)\r\n"
                                   "DA:5,4\r\n"
                                   "end_of_record\r\n");
    REQUIRE(data.size() == 2);
    CHECK(data.at(fs::path{"/src/a.c"}).counts() == Counts{.lines = 4,
                                                           .lines_covered = 3,
                                                           .branches = 3,
                                                           .branches_covered = 2,
                                                           .functions = 2,
                                                           .functions_covered = 2});
    CHECK(data.at((fs::path{"/base"} / "rel/b.cpp").lexically_normal()).counts() ==
          Counts{.lines = 1,
                 .lines_covered = 1,
                 .branches = 0,
                 .branches_covered = 0,
                 .functions = 2,
                 .functions_covered = 1});
}

TEST_CASE("a malformed lcov tracefile is an error naming the line", "[coverage]") {
    using Catch::Matchers::ContainsSubstring;
    CHECK_THROWS_WITH(read("DA:1,1\n"), ContainsSubstring("test.info:1: coverage record outside"));
    CHECK_THROWS_WITH(read("SF:/a.c\nDA:1,x\n"),
                      ContainsSubstring("test.info:2: expected a number"));
    CHECK_THROWS_WITH(read("SF:/a.c\nDA:0,1\n"), ContainsSubstring("line number out of range"));
    CHECK_THROWS_WITH(read("SF:/a.c\nBRDA:1,0\n"), ContainsSubstring("too few fields"));
}
