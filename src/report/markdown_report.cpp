#include "report/markdown_report.hpp"

#include "report/module_summary.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::report {

namespace fs = std::filesystem;

namespace {

// How much of each list the report shows before pointing to its CSV file.
constexpr std::size_t most_complex_shown = 20;
constexpr std::size_t highest_effort_shown = 10;
constexpr std::size_t undocumented_shown = 50;
constexpr std::size_t top_files_shown = 10;
// A coupling matrix wider than this does not fit a page; the report lists
// the heaviest cells instead.
constexpr std::size_t coupling_matrix_limit = 12;
constexpr std::size_t coupling_cells_shown = 20;

// Text for a Markdown table cell: pipes escaped, so operator| and a|b stay
// in their cell.
std::string cell(std::string_view text) {
    std::string result;
    for (const char c : text) {
        if (c == '|') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

// A name in a code span, so that template arguments such as clamp<int> are
// not read as HTML.
std::string code(std::string_view text) {
    return "`" + cell(text) + "`";
}

std::string percent(std::size_t part, std::size_t whole) {
    constexpr double hundred = 100.0;
    return whole == 0 ? std::string{"n/a"}
                      : std::format("{:.1f}%", hundred * static_cast<double>(part) /
                                                   static_cast<double>(whole));
}

std::string location(const FileNaming& naming, const fs::path& file, unsigned line) {
    return code(naming.relative(file) + ":" + std::to_string(line));
}

void header(std::ostream& out, const ReportData& data, const FileNaming& naming) {
    const Provenance& p = data.provenance;
    out << "# Code metrics baseline: " << naming.root().filename().generic_string() << "\n\n"
        << "Measured by Tezcatl " << p.tool_version << " (" << p.libclang_version << ") from the "
        << "compilation database in " << code(p.compilation_database.generic_string()) << ": "
        << p.translation_units << " translation units parsed, " << p.units_with_errors
        << " with errors.";
    if (p.units_with_errors > 0) {
        out << " **The figures below are incomplete:** units that fail to parse contribute "
               "only what libclang could recover.";
    }
    out << " Every figure is defined in Tezcatl's `docs/metrics.md`; the same data, complete, "
           "is in `report.json` and the CSV files next to this report.\n\n"
        << "Complexity thresholds: flagged over " << data.thresholds.flagged_over << ", high over "
        << data.thresholds.high_over << ". Test code is recognised by ";
    const std::vector<std::string>& globs = naming.roles().test_globs();
    for (std::size_t i = 0; i < globs.size(); ++i) {
        out << (i == 0 ? "" : ", ") << code(globs.at(i));
    }
    out << "; it counts toward test lines only.\n\n";
}

void summary(std::ostream& out, const std::vector<ModuleRow>& rows, const ModuleRow& total,
             bool with_coverage) {
    out << "## Summary\n\n"
        << "| Module | Code lines | Test code lines | Functions | Mean complexity | Max | "
           "Over threshold | Documented |"
        << (with_coverage ? " Line coverage |" : "") << " In a cycle |\n"
        << "|---|---:|---:|---:|---:|---:|---:|---:|" << (with_coverage ? "---:|" : "") << "---|\n";
    const auto row = [&](const ModuleRow& r, bool is_total) {
        out << "| " << (is_total ? "**Total**" : code(r.module)) << " | " << r.production.code
            << " | " << r.test.code << " | " << r.complexity.count << " | "
            << std::format("{:.2f}", r.complexity.mean) << " | " << r.complexity.max << " | "
            << r.complexity.flagged << " | "
            << percent(r.documentation.documented, r.documentation.entities) << " |";
        if (with_coverage) {
            out << ' '
                << (r.coverage.has_value() ? percent(r.coverage->lines_covered, r.coverage->lines)
                                           : std::string{"n/a"})
                << " |";
        }
        // The project as a whole is not part of the module graph.
        std::string_view cycle;
        if (!is_total) {
            cycle = r.cycle.has_value() ? "yes" : "no";
        }
        out << ' ' << cycle << " |\n";
    };
    for (const ModuleRow& r : rows) {
        row(r, false);
    }
    row(total, true);
    out << '\n';
}

void lines_section(std::ostream& out, const std::vector<ModuleRow>& rows, const ModuleRow& total) {
    out << "## Lines of code\n\n"
        << "Production: " << total.production.code << " code lines of " << total.production.physical
        << " physical (" << total.production.comment << " comment, " << total.production.blank
        << " blank). Test: " << total.test.code << " code lines of " << total.test.physical
        << ".\n\n"
        << "| Module | Files | Physical | Code | Comment | Blank | Test physical | Test code |\n"
        << "|---|---:|---:|---:|---:|---:|---:|---:|\n";
    for (const ModuleRow& r : rows) {
        out << "| " << code(r.module) << " | " << r.files << " | " << r.production.physical << " | "
            << r.production.code << " | " << r.production.comment << " | " << r.production.blank
            << " | " << r.test.physical << " | " << r.test.code << " |\n";
    }
    out << '\n';
}

std::vector<const parse::FunctionInfo*> production_functions(const ReportData& data,
                                                             const FileNaming& naming) {
    std::vector<const parse::FunctionInfo*> result;
    for (const parse::FunctionInfo& function : data.functions) {
        if (!naming.is_test(function.file)) {
            result.push_back(&function);
        }
    }
    return result;
}

void complexity_section(std::ostream& out, const ReportData& data, const FileNaming& naming,
                        const std::vector<ModuleRow>& rows) {
    out << "## Cyclomatic complexity\n\n"
        << "| Module | Functions | Mean | Median | p90 | Max | Over "
        << data.thresholds.flagged_over << " | Over " << data.thresholds.high_over << " |\n"
        << "|---|---:|---:|---:|---:|---:|---:|---:|\n";
    for (const ModuleRow& r : rows) {
        const metrics::Distribution& c = r.complexity;
        out << "| " << code(r.module) << " | " << c.count << " | "
            << std::format("{:.2f} | {:.1f}", c.mean, c.median) << " | " << c.p90 << " | " << c.max
            << " | " << c.flagged << " | " << c.high << " |\n";
    }
    std::vector<const parse::FunctionInfo*> functions = production_functions(data, naming);
    std::ranges::stable_sort(
        functions, [](const auto* a, const auto* b) { return a->complexity > b->complexity; });
    const std::size_t shown = std::min(functions.size(), most_complex_shown);
    out << "\n### Most complex functions\n\n"
        << "The " << shown << " most complex of " << functions.size()
        << " production functions; all of them are in `functions.csv`.\n\n"
        << "| Complexity | Function | Where | Module |\n|---:|---|---|---|\n";
    for (std::size_t i = 0; i < shown; ++i) {
        const parse::FunctionInfo& f = *functions.at(i);
        out << "| " << f.complexity << " | " << code(f.name) << " | "
            << location(naming, f.file, f.line) << " | " << code(naming.module_of(f.file))
            << " |\n";
    }
    out << '\n';
}

void halstead_section(std::ostream& out, const ReportData& data, const FileNaming& naming,
                      const std::vector<ModuleRow>& rows) {
    out << "## Halstead\n\n"
        << "Volume and effort add up across functions; difficulty does not, and is per "
           "function in `functions.csv`.\n\n"
        << "| Module | Volume | Effort |\n|---|---:|---:|\n";
    for (const ModuleRow& r : rows) {
        out << "| " << code(r.module) << " | "
            << std::format("{:.0f} | {:.0f}", r.halstead_volume, r.halstead_effort) << " |\n";
    }
    std::vector<const parse::FunctionInfo*> functions = production_functions(data, naming);
    std::ranges::stable_sort(functions, [](const auto* a, const auto* b) {
        return a->halstead.effort() > b->halstead.effort();
    });
    const std::size_t shown = std::min(functions.size(), highest_effort_shown);
    out << "\n### Highest effort\n\n| Effort | Volume | Difficulty | Function | Where |\n"
        << "|---:|---:|---:|---|---|\n";
    for (std::size_t i = 0; i < shown; ++i) {
        const parse::FunctionInfo& f = *functions.at(i);
        out << "| "
            << std::format("{:.0f} | {:.0f} | {:.1f}", f.halstead.effort(), f.halstead.volume(),
                           f.halstead.difficulty())
            << " | " << code(f.name) << " | " << location(naming, f.file, f.line) << " |\n";
    }
    out << '\n';
}

void documentation_section(std::ostream& out, const ReportData& data, const FileNaming& naming,
                           const std::vector<ModuleRow>& rows) {
    out << "## Documentation\n\n"
        << "Public API declared in production headers with a comment attached. Plain comments "
           "count; the doxygen column says how many use ///, //!, /** or /*!.\n\n"
        << "| Module | Declarations | Documented | Share | Doxygen |\n|---|---:|---:|---:|---:|\n";
    for (const ModuleRow& r : rows) {
        const DocCoverage& d = r.documentation;
        out << "| " << code(r.module) << " | " << d.entities << " | " << d.documented << " | "
            << percent(d.documented, d.entities) << " | " << d.doxygen << " |\n";
    }
    std::vector<const parse::ApiEntity*> undocumented;
    for (const parse::ApiEntity& entity : data.api) {
        if (entity.documentation == parse::DocStyle::none && !naming.is_test(entity.file)) {
            undocumented.push_back(&entity);
        }
    }
    const std::size_t shown = std::min(undocumented.size(), undocumented_shown);
    out << "\n### Undocumented declarations\n\n";
    if (undocumented.empty()) {
        out << "None.\n\n";
        return;
    }
    out << "The first " << shown << " of " << undocumented.size()
        << ", in file order; all of them are in `api.csv`.\n\n"
        << "| Declaration | Kind | Where |\n|---|---|---|\n";
    for (std::size_t i = 0; i < shown; ++i) {
        const parse::ApiEntity& e = *undocumented.at(i);
        out << "| " << code(e.name) << " | " << parse::to_string(e.kind) << " | "
            << location(naming, e.file, e.line) << " |\n";
    }
    out << '\n';
}

void coverage_section(std::ostream& out, const ReportData& data, const std::vector<ModuleRow>& rows,
                      const ModuleRow& total) {
    out << "## Test coverage\n\n";
    if (!data.coverage.has_value()) {
        out << "No coverage data was imported, so there are no coverage figures: not 0%.\n\n";
        return;
    }
    out << "Imported from " << data.provenance.coverage_inputs.size()
        << " file(s); Tezcatl does not run tests. " << data.coverage->outside
        << " recorded file(s) outside the project were left out.\n\n"
        << "| Module | Lines | Branches | Functions |\n|---|---:|---:|---:|\n";
    const auto row = [&out](std::string_view name, const ModuleRow& r) {
        const coverage::Counts c = r.coverage.value_or(coverage::Counts{});
        out << "| " << name << " | " << percent(c.lines_covered, c.lines) << " (" << c.lines_covered
            << "/" << c.lines << ") | " << percent(c.branches_covered, c.branches) << " ("
            << c.branches_covered << "/" << c.branches << ") | "
            << percent(c.functions_covered, c.functions) << " (" << c.functions_covered << "/"
            << c.functions << ") |\n";
    };
    for (const ModuleRow& r : rows) {
        row(code(r.module), r);
    }
    row("**Total**", total);
    out << '\n';
}

void cycles_list(std::ostream& out, std::string_view what, const graph::Digraph& graph,
                 const std::vector<std::vector<std::size_t>>& cycles) {
    if (cycles.empty()) {
        out << "No " << what << " cycles.\n\n";
        return;
    }
    out << cycles.size() << ' ' << what << " cycle" << (cycles.size() == 1 ? "" : "s") << ":\n\n";
    for (std::size_t i = 0; i < cycles.size(); ++i) {
        out << i + 1 << ". ";
        for (std::size_t j = 0; j < cycles.at(i).size(); ++j) {
            out << (j == 0 ? "" : ", ") << code(graph.name(cycles.at(i).at(j)));
        }
        out << '\n';
    }
    out << '\n';
}

void top_files(std::ostream& out, const IncludeGraph& includes, bool by_fan_in) {
    const graph::Digraph& files = includes.files;
    std::vector<std::size_t> nodes(files.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes.at(i) = i;
    }
    const auto measure = [&](std::size_t node) {
        return by_fan_in ? files.fan_in(node) : files.fan_out(node);
    };
    std::ranges::stable_sort(nodes,
                             [&](std::size_t a, std::size_t b) { return measure(a) > measure(b); });
    out << "| " << (by_fan_in ? "Included by" : "Includes") << " | File |\n|---:|---|\n";
    for (std::size_t i = 0; i < std::min(nodes.size(), top_files_shown); ++i) {
        out << "| " << measure(nodes.at(i)) << " | " << code(files.name(nodes.at(i))) << " |\n";
    }
    out << '\n';
}

void coupling(std::ostream& out, const IncludeGraph& includes) {
    const graph::Digraph& modules = includes.modules;
    if (modules.size() <= coupling_matrix_limit) {
        out << "Include edges from each module (rows) to each module (columns):\n\n| |";
        for (std::size_t to = 0; to < modules.size(); ++to) {
            out << ' ' << code(modules.name(to)) << " |";
        }
        out << "\n|---|";
        for (std::size_t to = 0; to < modules.size(); ++to) {
            out << "---:|";
        }
        out << '\n';
        for (std::size_t from = 0; from < modules.size(); ++from) {
            out << "| " << code(modules.name(from)) << " |";
            for (std::size_t to = 0; to < modules.size(); ++to) {
                const auto found = std::ranges::find_if(includes.coupling, [&](const auto& c) {
                    return c.from == modules.name(from) && c.to == modules.name(to);
                });
                out << ' ' << (found == includes.coupling.end() ? 0 : found->edges) << " |";
            }
            out << '\n';
        }
        out << '\n';
        return;
    }
    std::vector<ModuleCoupling> cells = includes.coupling;
    std::ranges::stable_sort(cells, [](const auto& a, const auto& b) { return a.edges > b.edges; });
    out << "The " << std::min(cells.size(), coupling_cells_shown)
        << " heaviest module-to-module include counts; all of them are in "
           "`include-coupling.csv`.\n\n| Edges | From | To |\n|---:|---|---|\n";
    for (std::size_t i = 0; i < std::min(cells.size(), coupling_cells_shown); ++i) {
        out << "| " << cells.at(i).edges << " | " << code(cells.at(i).from) << " | "
            << code(cells.at(i).to) << " |\n";
    }
    out << '\n';
}

void dependencies_section(std::ostream& out, const IncludeGraph& includes) {
    out << "## Include dependencies\n\n"
        << includes.files.size() << " files, " << includes.files.edge_count()
        << " include edges between them. A cycle is a set of files (or modules) that each "
           "reach every other through includes.\n\n";
    cycles_list(out, "file", includes.files, includes.file_cycles);
    cycles_list(out, "module", includes.modules, includes.module_cycles);
    out << "### Most included files\n\n";
    top_files(out, includes, true);
    out << "### Files that include the most\n\n";
    top_files(out, includes, false);
    out << "### Module coupling\n\n";
    coupling(out, includes);
}

} // namespace

void write_report_markdown(std::ostream& out, const ReportData& data, const FileNaming& naming) {
    const std::vector<ModuleRow> rows = summarize_modules(data, naming);
    const ModuleRow total = summarize_project(data, naming);
    header(out, data, naming);
    summary(out, rows, total, data.coverage.has_value());
    lines_section(out, rows, total);
    complexity_section(out, data, naming, rows);
    halstead_section(out, data, naming, rows);
    documentation_section(out, data, naming, rows);
    coverage_section(out, data, rows, total);
    dependencies_section(out, data.includes);
}

} // namespace tezcatl::report
