#include "report/json_report.hpp"

#include "report/module_summary.hpp"

#include <algorithm>
#include <iterator>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

namespace tezcatl::report {

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

json lines(const metrics::LocCounts& counts) {
    return {{"physical", counts.physical},
            {"blank", counts.blank},
            {"comment", counts.comment},
            {"code", counts.code}};
}

json coverage_counts(const coverage::Counts& c) {
    return {{"lines", c.lines},         {"lines_covered", c.lines_covered},
            {"branches", c.branches},   {"branches_covered", c.branches_covered},
            {"functions", c.functions}, {"functions_covered", c.functions_covered}};
}

json module_row(const ModuleRow& row, bool with_includes) {
    const metrics::Distribution& c = row.complexity;
    json result = {
        {"module", row.module},
        {"files", row.files},
        {"lines", {{"production", lines(row.production)}, {"test", lines(row.test)}}},
        {"complexity",
         {{"functions", c.count},
          {"mean", c.mean},
          {"median", c.median},
          {"p90", c.p90},
          {"max", c.max},
          {"flagged", c.flagged},
          {"high", c.high}}},
        {"halstead", {{"volume", row.halstead_volume}, {"effort", row.halstead_effort}}},
        {"documentation",
         {{"declarations", row.documentation.entities},
          {"documented", row.documentation.documented},
          {"doxygen", row.documentation.doxygen},
          {"percent", row.documentation.percent()}}},
        {"coverage", row.coverage.has_value() ? coverage_counts(*row.coverage) : json(nullptr)},
        {"includes", nullptr}};
    if (with_includes) {
        result.at("includes") = {
            {"fan_in", row.fan_in},
            {"fan_out", row.fan_out},
            {"cycle", row.cycle.has_value() ? json(*row.cycle) : json(nullptr)}};
    }
    return result;
}

// A JSON array with one element per item.
template <typename Items, typename ToJson> json array_of(const Items& items, ToJson to_json) {
    json result = json::array();
    std::ranges::transform(items, std::back_inserter(result), to_json);
    return result;
}

json cycles(const graph::Digraph& graph, const std::vector<std::vector<std::size_t>>& found) {
    return array_of(found, [&](const std::vector<std::size_t>& cycle) {
        return array_of(cycle, [&](std::size_t node) { return json(graph.name(node)); });
    });
}

json settings(const ReportData& data, const FileNaming& naming) {
    const json modules = array_of(naming.modules().rules(), [](const config::ModuleRule& rule) {
        return json{{"module", rule.module}, {"glob", rule.pattern}};
    });
    return {{"complexity",
             {{"flagged_over", data.thresholds.flagged_over},
              {"high_over", data.thresholds.high_over}}},
            {"test_globs", naming.roles().test_globs()},
            {"modules", modules}};
}

json input(const ReportData& data, const FileNaming& naming) {
    const json coverage_inputs =
        array_of(data.provenance.coverage_inputs,
                 [](const fs::path& file) { return json(file.generic_string()); });
    return {{"root", naming.root().generic_string()},
            {"compilation_database", data.provenance.compilation_database.generic_string()},
            {"coverage", coverage_inputs},
            {"translation_units", data.provenance.translation_units},
            {"units_with_errors", data.provenance.units_with_errors},
            {"units_skipped", data.provenance.units_skipped}};
}

json files(const ReportData& data, const FileNaming& naming) {
    return array_of(data.files, [&](const FileLines& file) {
        return json{{"path", naming.relative(file.file)},
                    {"module", naming.module_of(file.file)},
                    {"role", naming.role_of(file.file)},
                    {"parsed", file.parsed},
                    {"lines", lines(file.counts)}};
    });
}

json functions(const ReportData& data, const FileNaming& naming) {
    return array_of(data.functions, [&](const parse::FunctionInfo& f) {
        const metrics::Halstead& h = f.halstead;
        return json{{"file", naming.relative(f.file)},
                    {"line", f.line},
                    {"column", f.column},
                    {"kind", parse::to_string(f.kind)},
                    {"name", f.name},
                    {"module", naming.module_of(f.file)},
                    {"role", naming.role_of(f.file)},
                    {"complexity", f.complexity},
                    {"rating", metrics::to_string(metrics::rate(f.complexity, data.thresholds))},
                    {"halstead",
                     {{"distinct_operators", h.distinct_operators},
                      {"distinct_operands", h.distinct_operands},
                      {"total_operators", h.total_operators},
                      {"total_operands", h.total_operands},
                      {"volume", h.volume()},
                      {"difficulty", h.difficulty()},
                      {"effort", h.effort()}}}};
    });
}

json api(const ReportData& data, const FileNaming& naming) {
    return array_of(data.api, [&](const parse::ApiEntity& e) {
        return json{{"file", naming.relative(e.file)},
                    {"line", e.line},
                    {"column", e.column},
                    {"kind", parse::to_string(e.kind)},
                    {"name", e.name},
                    {"module", naming.module_of(e.file)},
                    {"role", naming.role_of(e.file)},
                    {"documentation", parse::to_string(e.documentation)}};
    });
}

json coverage(const ReportData& data, const FileNaming& naming) {
    if (!data.coverage.has_value()) {
        return nullptr;
    }
    return array_of(data.coverage->files, [&](const auto& file_counts) {
        const auto& [file, counts] = file_counts;
        return json{{"file", naming.relative(file)},
                    {"module", naming.module_of(file)},
                    {"counts", coverage_counts(counts)}};
    });
}

json includes(const IncludeGraph& graph) {
    json edges = json::array();
    const graph::Digraph& file_graph = graph.files;
    for (std::size_t from = 0; from < file_graph.size(); ++from) {
        std::ranges::transform(
            file_graph.successors(from), std::back_inserter(edges), [&](std::size_t to) {
                return json{{"from", file_graph.name(from)}, {"to", file_graph.name(to)}};
            });
    }
    const json coupling = array_of(graph.coupling, [](const ModuleCoupling& cell) {
        return json{{"from", cell.from}, {"to", cell.to}, {"edges", cell.edges}};
    });
    return {{"edges", edges},
            {"file_cycles", cycles(graph.files, graph.file_cycles)},
            {"module_cycles", cycles(graph.modules, graph.module_cycles)},
            {"coupling", coupling}};
}

} // namespace

void write_report_json(std::ostream& out, const ReportData& data, const FileNaming& naming) {
    const json modules = array_of(summarize_modules(data, naming),
                                  [](const ModuleRow& row) { return module_row(row, true); });
    const json report = {{"schema", "tezcatl-report"},
                         {"schema_version", report_schema_version},
                         {"tool",
                          {{"name", "tezcatl"},
                           {"version", data.provenance.tool_version},
                           {"libclang", data.provenance.libclang_version}}},
                         {"input", input(data, naming)},
                         {"settings", settings(data, naming)},
                         {"totals", module_row(summarize_project(data, naming), false)},
                         {"modules", modules},
                         {"files", files(data, naming)},
                         {"functions", functions(data, naming)},
                         {"api", api(data, naming)},
                         {"coverage", coverage(data, naming)},
                         {"includes", includes(data.includes)}};
    out << report.dump(2) << '\n';
}

} // namespace tezcatl::report
