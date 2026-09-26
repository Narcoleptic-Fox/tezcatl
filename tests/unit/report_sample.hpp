#pragma once

#include "report/data.hpp"
#include "report/naming.hpp"

#include <filesystem>

namespace tezcatl::test {

/// A small report built by hand, with every section filled, for testing the
/// writers without parsing anything. Its figures are worked out in
/// unit/module_summary_test.cpp:
///   a/x.c        production, 10 lines (7 code, 2 comment, 1 blank); two
///                functions of complexity 3 and 12, Halstead V = E = 8 each;
///                coverage 10/8 lines, 4/2 branches, 2/1 functions
///   a/tests/t.c  test, 5 lines (4 code, 1 comment); one function of 5;
///                not parsed (no unit compiles it)
///   b/y.c        production, 4 code lines; one function of complexity 1;
///                includes a/x.h
///   a/x.h        two declarations, one doxygen-documented
///   a/tests/t.h  one documented declaration, test code
/// Modules: a = a/**; b/ is unassigned.
inline const std::filesystem::path& sample_root() {
    static const std::filesystem::path root = std::filesystem::absolute("/p");
    return root;
}

inline report::FileNaming sample_naming() {
    return {sample_root(), config::ModuleMap::parse("a = a/**", "m.txt"), config::FileRoles{}};
}

inline report::ReportData sample_report() {
    const std::filesystem::path& root = sample_root();
    const metrics::Halstead eight{
        .distinct_operators = 2, .distinct_operands = 2, .total_operators = 2, .total_operands = 2};
    const auto function = [&](const char* file, unsigned line, unsigned complexity,
                              const metrics::Halstead& h) {
        return analysis::FunctionInfo{.file = root / file,
                                      .line = line,
                                      .column = 1,
                                      .kind = parse::FunctionKind::function,
                                      .name = "f" + std::to_string(line) + "()",
                                      .complexity = complexity,
                                      .halstead = h};
    };
    const auto api = [&](const char* file, unsigned line, parse::DocStyle style) {
        return parse::ApiEntity{.file = root / file,
                                .line = line,
                                .column = 1,
                                .kind = parse::ApiKind::function,
                                .name = "f" + std::to_string(line) + "()",
                                .documentation = style};
    };
    report::ReportData data;
    data.provenance = {.tool_version = "0.0.0-test",
                       .libclang_version = "clang version 0.0.0",
                       .compilation_database = root / "build",
                       .coverage_inputs = {root / "coverage.info"},
                       .translation_units = 3,
                       .units_with_errors = 0,
                       .units_skipped = 0,
                       .units_in_build_directory = 0};
    data.files = {{.file = root / "a/x.c",
                   .counts = {.physical = 10, .blank = 1, .comment = 2, .code = 7},
                   .parsed = true},
                  {.file = root / "a/tests/t.c",
                   .counts = {.physical = 5, .blank = 0, .comment = 1, .code = 4},
                   .parsed = false},
                  {.file = root / "b/y.c",
                   .counts = {.physical = 4, .blank = 0, .comment = 0, .code = 4},
                   .parsed = true}};
    data.functions = {function("a/x.c", 1, 3, eight), function("a/x.c", 5, 12, eight),
                      function("a/tests/t.c", 1, 5, eight), function("b/y.c", 1, 1, {})};
    data.api = {api("a/x.h", 1, parse::DocStyle::doxygen), api("a/x.h", 2, parse::DocStyle::none),
                api("a/tests/t.h", 1, parse::DocStyle::plain)};
    data.includes = report::build_include_graph({{.from = root / "b/y.c", .to = root / "a/x.h"}},
                                                {root / "a/x.c", root / "b/y.c"}, sample_naming());
    data.coverage = report::AttributedCoverage{
        .files = {{root / "a/x.c", coverage::Counts{.lines = 10,
                                                    .lines_covered = 8,
                                                    .branches = 4,
                                                    .branches_covered = 2,
                                                    .functions = 2,
                                                    .functions_covered = 1}}},
        .outside = 0};
    return data;
}

} // namespace tezcatl::test
