#include "report/module_summary.hpp"
#include "report_sample.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl;

TEST_CASE("module rows add up every metric, production and test apart", "[report]") {
    const report::FileNaming naming = test::sample_naming();
    const report::ReportData data = test::sample_report();

    std::ostringstream out;
    report::write_module_table(out, report::summarize_modules(data, naming),
                               report::summarize_project(data, naming));

    // By hand. a: two files, tests/t.c is test code; production functions
    // 3 and 12 (mean 7.5, p90 by rank ceil(1.8) = 2 is 12, 12 is over 10);
    // the test function and the test header are left out; V and E 8 + 8;
    // one of two declarations documented; the coverage given; included by
    // (unassigned). (unassigned): b/y.c, one function of complexity 1, no
    // coverage records but coverage was given, so zeros; includes a. TOTAL:
    // complexities 3, 12, 1 (mean 5.33, median 3, p90 rank 3 is 12).
    // clang-format off: one row per line
    const std::string expected =
        "module,files,files_parsed,production_physical,production_code,production_comment,production_blank,"
        "test_physical,test_code,functions,complexity_mean,complexity_median,complexity_p90,"
        "complexity_max,flagged,high,halstead_volume,halstead_effort,api,documented,"
        "documented_percent,coverage_lines,coverage_lines_covered,coverage_branches,"
        "coverage_branches_covered,coverage_functions,coverage_functions_covered,fan_in,fan_out,"
        "cycle\n"
        "(unassigned),1,1,4,4,0,0,0,0,1,1.00,1.0,1,1,0,0,0.00,0.00,0,0,0.0,0,0,0,0,0,0,0,1,\n"
        "a,2,1,10,7,2,1,5,4,2,7.50,7.5,12,12,1,0,16.00,16.00,2,1,50.0,10,8,4,2,2,1,1,0,\n"
        "TOTAL,3,2,14,11,2,1,5,4,3,5.33,3.0,12,12,1,0,16.00,16.00,2,1,50.0,10,8,4,2,2,1,,,\n";
    // clang-format on
    CHECK(out.str() == expected);
}

TEST_CASE("a test file's own coverage is not the module's", "[report]") {
    // The sample's coverage covers only production code, so this adds a
    // record for its test file: a test runs its own lines, and counting them
    // would lift module a from 8 of 10 lines to 12 of 14.
    const report::FileNaming naming = test::sample_naming();
    report::ReportData data = test::sample_report();
    report::AttributedCoverage covered = data.coverage.value_or(report::AttributedCoverage{});
    covered.files.emplace_back(test::sample_root() / "a/tests/t.c",
                               coverage::Counts{.lines = 4,
                                                .lines_covered = 4,
                                                .branches = 0,
                                                .branches_covered = 0,
                                                .functions = 1,
                                                .functions_covered = 1});
    data.coverage = covered;
    const std::vector<report::ModuleRow> rows = report::summarize_modules(data, naming);
    REQUIRE(rows.size() == 2);
    CHECK(rows.at(1).coverage.value_or(coverage::Counts{}).lines == 10);
    CHECK(rows.at(1).coverage.value_or(coverage::Counts{}).lines_covered == 8);
    CHECK(report::summarize_project(data, naming).coverage.value_or(coverage::Counts{}).lines ==
          10);
}

TEST_CASE("without coverage data the coverage columns are empty", "[report]") {
    const fs::path root = fs::absolute("/p");
    const report::FileNaming naming{root, config::ModuleMap{}, config::FileRoles{}};
    report::ReportData data;
    data.files = {
        {.file = root / "x.c", .counts = {.physical = 1, .blank = 0, .comment = 0, .code = 1}}};
    data.includes = report::build_include_graph({}, {root / "x.c"}, naming);
    std::ostringstream out;
    report::write_module_table(out, report::summarize_modules(data, naming),
                               report::summarize_project(data, naming));
    CHECK(out.str().ends_with(
        "\n(unassigned),1,0,1,1,0,0,0,0,0,0.00,0.0,0,0,0,0,0.00,0.00,0,0,0.0,"
        ",,,,,,0,0,\nTOTAL,1,0,1,1,0,0,0,0,0,0.00,0.0,0,0,0,0,0.00,0.00,0,0,0.0,"
        ",,,,,,,,\n"));
}
