#include "coverage/readers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
using namespace tezcatl::coverage;
using Catch::Matchers::ContainsSubstring;

namespace {

Counts total(const CoverageData& data) {
    Counts sum;
    for (const auto& [file, record] : data) {
        sum += record.counts();
    }
    return sum;
}

fs::path fixture_data(const char* name) {
    return fs::path{TEZCATL_FIXTURES_DIR} / "coverage" / "data" / name;
}

fs::path source(const char* name) {
    return (fs::path{"/coverage-fixture/src"} / name).lexically_normal();
}

} // namespace

TEST_CASE("gcov JSON documents are merged per line, branch position and function", "[coverage]") {
    // Two documents, one per line as gcov --stdout writes them, for the two
    // instances of a template in ../src/t.hpp. By hand: line 3 (1 + 0),
    // line 4 (0): 2 lines, 1 covered. Branches at line 3, positions 0
    // (1 + 0) and 1 (0 + 2): 2, both covered. Functions f<int> (1) and
    // f<double> (0): 2, 1 covered.
    const std::string text =
        R"json({"format_version":"2","current_working_directory":"/build","files":[{"file":"../src/t.hpp",)json"
        R"json("functions":[{"name":"_Z1fIiEvT_","demangled_name":"void f<int>(int)","execution_count":1}],)json"
        R"json("lines":[{"line_number":3,"count":1,"branches":[{"count":1},{"count":0}]},)json"
        R"json({"line_number":4,"count":0,"branches":[]}]}]})json"
        "\n"
        R"json({"format_version":"2","current_working_directory":"/build","files":[{"file":"../src/t.hpp",)json"
        R"json("functions":[{"name":"_Z1fIdEvT_","demangled_name":"void f<double>(double)","execution_count":0}],)json"
        R"json("lines":[{"line_number":3,"count":0,"branches":[{"count":0},{"count":2}]},)json"
        R"json({"line_number":4,"count":0}]}]})json"
        "\n";
    CoverageData data;
    read_gcov_json(text, "t.json", data);
    REQUIRE(data.size() == 1);
    CHECK(data.at(fs::path{"/src/t.hpp"}.lexically_normal()).counts() ==
          Counts{.lines = 2,
                 .lines_covered = 1,
                 .branches = 2,
                 .branches_covered = 2,
                 .functions = 2,
                 .functions_covered = 1});
}

TEST_CASE("llvm-cov totals are taken as they are, once per file", "[coverage]") {
    const std::string text =
        R"json({"type":"llvm.coverage.json.export","version":"2.0.1","data":[{"files":[{"filename":"/src/a.cpp",)json"
        R"json("summary":{"lines":{"count":10,"covered":7},"branches":{"count":4,"covered":3},)json"
        R"json("functions":{"count":2,"covered":1}}}]}]})json";
    CoverageData data;
    read_llvm_json(text, "a.json", data);
    CHECK(data.at(fs::path{"/src/a.cpp"}).counts() == Counts{.lines = 10,
                                                             .lines_covered = 7,
                                                             .branches = 4,
                                                             .branches_covered = 3,
                                                             .functions = 2,
                                                             .functions_covered = 1});
    // A second export of the same file would add its totals again.
    CHECK_THROWS_WITH(read_llvm_json(text, "b.json", data), ContainsSubstring("reported twice"));
    // Line data on top of llvm-cov's totals would count the file twice.
    std::istringstream lcov{"SF:/src/a.cpp\nDA:1,1\nend_of_record\n"};
    CHECK_THROWS_WITH(read_lcov(lcov, "/", "c.info", data), ContainsSubstring("llvm-cov totals"));
}

TEST_CASE("real gcov JSON and lcov data give gcovr's totals", "[coverage]") {
    // The oracle is gcovr 8.6 --merge-lines on the same run: see
    // tests/fixtures/coverage/README.md.
    for (const char* name : {"calc.gcov.json", "calc.info"}) {
        INFO(name);
        CoverageData data;
        read_coverage_file(fixture_data(name), data);
        CHECK(data.at(source("calc.cpp")).counts() == Counts{.lines = 8,
                                                             .lines_covered = 5,
                                                             .branches = 4,
                                                             .branches_covered = 3,
                                                             .functions = 2,
                                                             .functions_covered = 1});
        CHECK(data.at(source("calc.hpp")).counts() == Counts{.lines = 4,
                                                             .lines_covered = 4,
                                                             .branches = 4,
                                                             .branches_covered = 3,
                                                             .functions = 2,
                                                             .functions_covered = 2});
        CHECK(total(data) == Counts{.lines = 17,
                                    .lines_covered = 14,
                                    .branches = 8,
                                    .branches_covered = 6,
                                    .functions = 5,
                                    .functions_covered = 4});
    }
}

TEST_CASE("real llvm-cov data gives llvm-cov's own totals", "[coverage]") {
    CoverageData data;
    read_coverage_file(fixture_data("calc.llvm.json"), data);
    CHECK(data.size() == 3);
    CHECK(total(data) == Counts{.lines = 24,
                                .lines_covered = 18,
                                .branches = 10,
                                .branches_covered = 6,
                                .functions = 4,
                                .functions_covered = 3});
}

TEST_CASE("a file that is not coverage data is an error, not an empty result", "[coverage]") {
    CoverageData data;
    CHECK_THROWS_WITH(
        read_coverage_file(fs::path{TEZCATL_FIXTURES_DIR} / "loc" / "notes.txt", data),
        ContainsSubstring("no coverage records"));
    CHECK_THROWS_AS(read_coverage_file(fixture_data("no-such-file.info"), data),
                    std::runtime_error);
}
