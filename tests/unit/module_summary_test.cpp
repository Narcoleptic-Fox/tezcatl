#include "report/module_summary.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using namespace tezcatl;

TEST_CASE("module rows add up every metric, production and test apart", "[report]") {
    const fs::path root = fs::absolute("/p");
    const report::FileNaming naming{root, config::ModuleMap::parse("a = a/**", "m.txt"),
                                    config::FileRoles{}};
    // Halstead with n1 = n2 = N1 = N2 = 2: V = 4 log2 4 = 8, D = 1, E = 8.
    const metrics::Halstead eight{
        .distinct_operators = 2, .distinct_operands = 2, .total_operators = 2, .total_operands = 2};
    const auto function = [&](const char* file, unsigned complexity, metrics::Halstead h) {
        return parse::FunctionInfo{.file = root / file,
                                   .line = 1,
                                   .column = 1,
                                   .kind = parse::FunctionKind::function,
                                   .name = "f()",
                                   .complexity = complexity,
                                   .halstead = h};
    };
    const auto api = [&](const char* file, unsigned line, parse::DocStyle style) {
        return parse::ApiEntity{.file = root / file,
                                .line = line,
                                .column = 1,
                                .kind = parse::ApiKind::function,
                                .name = "f()",
                                .documentation = style};
    };
    report::ReportData data;
    data.files = {
        {.file = root / "a/x.c", .counts = {.physical = 10, .blank = 1, .comment = 2, .code = 7}},
        {.file = root / "a/tests/t.c",
         .counts = {.physical = 5, .blank = 0, .comment = 1, .code = 4}},
        {.file = root / "b/y.c", .counts = {.physical = 4, .blank = 0, .comment = 0, .code = 4}}};
    data.functions = {function("a/x.c", 3, eight), function("a/x.c", 12, eight),
                      function("a/tests/t.c", 5, eight), function("b/y.c", 1, {})};
    data.api = {api("a/x.h", 1, parse::DocStyle::doxygen), api("a/x.h", 2, parse::DocStyle::none),
                api("a/tests/t.h", 1, parse::DocStyle::plain)};
    data.includes = report::build_include_graph({{.from = root / "b/y.c", .to = root / "a/x.h"}},
                                                {root / "a/x.c", root / "b/y.c"}, naming);
    data.coverage = report::AttributedCoverage{
        .files = {{root / "a/x.c", coverage::Counts{.lines = 10,
                                                    .lines_covered = 8,
                                                    .branches = 4,
                                                    .branches_covered = 2,
                                                    .functions = 2,
                                                    .functions_covered = 1}}},
        .outside = 0};

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
        "module,files,production_physical,production_code,production_comment,production_blank,"
        "test_physical,test_code,functions,complexity_mean,complexity_median,complexity_p90,"
        "complexity_max,flagged,high,halstead_volume,halstead_effort,api,documented,"
        "documented_percent,coverage_lines,coverage_lines_covered,coverage_branches,"
        "coverage_branches_covered,coverage_functions,coverage_functions_covered,fan_in,fan_out,"
        "cycle\n"
        "(unassigned),1,4,4,0,0,0,0,1,1.00,1.0,1,1,0,0,0.00,0.00,0,0,0.0,0,0,0,0,0,0,0,1,\n"
        "a,2,10,7,2,1,5,4,2,7.50,7.5,12,12,1,0,16.00,16.00,2,1,50.0,10,8,4,2,2,1,1,0,\n"
        "TOTAL,3,14,11,2,1,5,4,3,5.33,3.0,12,12,1,0,16.00,16.00,2,1,50.0,10,8,4,2,2,1,,,\n";
    // clang-format on
    CHECK(out.str() == expected);
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
    CHECK(
        out.str().ends_with("\n(unassigned),1,1,1,0,0,0,0,0,0.00,0.0,0,0,0,0,0.00,0.00,0,0,0.0,"
                            ",,,,,,0,0,\nTOTAL,1,1,1,0,0,0,0,0,0.00,0.0,0,0,0,0,0.00,0.00,0,0,0.0,"
                            ",,,,,,,,\n"));
}
