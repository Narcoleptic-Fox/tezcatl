#include "report/markdown_report.hpp"
#include "report_sample.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>

using namespace tezcatl;
using Catch::Matchers::ContainsSubstring;

namespace {

std::string markdown(const report::ReportData& data) {
    std::ostringstream out;
    report::write_report_markdown(out, data, test::sample_naming());
    return out.str();
}

} // namespace

TEST_CASE("the Markdown summary has one row per module and a total", "[report]") {
    const std::string text = markdown(test::sample_report());
    // Figures from unit/module_summary_test.cpp. (unassigned) has no API and
    // no coverage records: n/a, not 0%.
    CHECK_THAT(text,
               ContainsSubstring(
                   "| `(unassigned)` | 1 (1) | 4 | 0 | 1 | 1.00 | 1 | 0 | n/a | n/a | no |\n"));
    CHECK_THAT(text, ContainsSubstring(
                         "| `a` | 2 (1) | 7 | 4 | 2 | 7.50 | 12 | 1 | 50.0% | 80.0% | no |\n"));
    CHECK_THAT(text,
               ContainsSubstring(
                   "| **Total** | 3 (2) | 11 | 4 | 3 | 5.33 | 12 | 1 | 50.0% | 80.0% |  |\n"));
}

TEST_CASE("the Markdown lists leave test code out", "[report]") {
    const std::string text = markdown(test::sample_report());
    // The complexity 5 function is in a/tests/t.c and the documented t.h
    // declaration is test code: neither is the product's.
    CHECK_THAT(text, ContainsSubstring("The 3 most complex of 3 production functions"));
    CHECK_THAT(text, ContainsSubstring(
                         "| 12 | `f5()` | `a/x.c:5` | `a` |\n| 3 | `f1()` | `a/x.c:1` | `a` |\n"
                         "| 1 | `f1()` | `b/y.c:1` | `(unassigned)` |\n"));
    CHECK_THAT(text, !ContainsSubstring("a/tests/t.c:1"));
    CHECK_THAT(text, ContainsSubstring("The first 1 of 1, in file order"));
    CHECK_THAT(text, ContainsSubstring("| `f2()` | function | `a/x.h:2` |\n"));
    CHECK_THAT(text, !ContainsSubstring("a/tests/t.h"));
}

TEST_CASE("an undocumented test declaration is not listed", "[report]") {
    // The sample's test declaration is documented, so the check above could
    // not tell whether test code is filtered: here it would be listed.
    report::ReportData data = test::sample_report();
    data.api.back().documentation = parse::DocStyle::none;
    const std::string text = markdown(data);
    CHECK_THAT(text, ContainsSubstring("The first 1 of 1, in file order"));
    CHECK_THAT(text, !ContainsSubstring("a/tests/t.h"));
}

TEST_CASE("the Markdown include section shows the coupling matrix", "[report]") {
    const std::string text = markdown(test::sample_report());
    CHECK_THAT(text, ContainsSubstring("3 files, 1 include edges between them."));
    CHECK_THAT(text, ContainsSubstring("No file cycles.\n\nNo module cycles.\n"));
    CHECK_THAT(text, ContainsSubstring("| | `(unassigned)` | `a` |\n|---|---:|---:|\n"
                                       "| `(unassigned)` | 0 | 1 |\n| `a` | 0 | 0 |\n"));
}

TEST_CASE("a pipe in a name stays inside its table cell", "[report]") {
    report::ReportData data = test::sample_report();
    data.functions.front().name = "operator|()";
    CHECK_THAT(markdown(data), ContainsSubstring("`operator\\|()`"));
}

TEST_CASE("without coverage data the report says so rather than 0%", "[report]") {
    report::ReportData data = test::sample_report();
    data.coverage.reset();
    const std::string text = markdown(data);
    CHECK_THAT(
        text, ContainsSubstring("No coverage data was imported, so there are no coverage figures"));
    CHECK_THAT(text,
               ContainsSubstring("| `a` | 2 (1) | 7 | 4 | 2 | 7.50 | 12 | 1 | 50.0% | no |\n"));
}

TEST_CASE("a long list is cut and names the CSV with all of it", "[report]") {
    report::ReportData data = test::sample_report();
    constexpr unsigned extra = 25;
    for (unsigned i = 0; i < extra; ++i) {
        analysis::FunctionInfo copy = data.functions.front();
        copy.line = 100 + i;
        data.functions.push_back(copy);
    }
    CHECK_THAT(
        markdown(data),
        ContainsSubstring(
            "The 20 most complex of 28 production functions; all of them are in `functions.csv`."));
}

TEST_CASE("files no unit parsed are counted and named at the top", "[report]") {
    report::ReportData data = test::sample_report();
    CHECK_THAT(markdown(data),
               ContainsSubstring("2 of the 3 source files under the root were parsed, as a unit "
                                 "or through an include. The other 1 (`parsed` is `no`"));
    for (report::FileLines& file : data.files) {
        file.parsed = true;
    }
    CHECK_THAT(markdown(data), ContainsSubstring("All 3 source files under the root were parsed."));
}

TEST_CASE("skipped database entries are stated at the top", "[report]") {
    report::ReportData data = test::sample_report();
    CHECK_THAT(markdown(data), !ContainsSubstring("other languages"));
    data.provenance.units_skipped = 120;
    CHECK_THAT(markdown(data),
               ContainsSubstring("0 with errors. 120 database entries for other languages (such "
                                 "as Fortran) were not parsed."));
    data.provenance.units_in_build_directory = 90;
    CHECK_THAT(markdown(data), ContainsSubstring("were not parsed. 90 units under the build "
                                                 "directory (fetched dependencies, generated "
                                                 "code) were not parsed."));
}

TEST_CASE("parse errors are stated at the top", "[report]") {
    report::ReportData data = test::sample_report();
    CHECK_THAT(markdown(data), !ContainsSubstring("incomplete"));
    data.provenance.units_with_errors = 1;
    CHECK_THAT(markdown(data),
               ContainsSubstring("3 translation units parsed, 1 with errors. **The figures below "
                                 "are incomplete:**"));
}
