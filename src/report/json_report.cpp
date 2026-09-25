#include "report/json_report.hpp"

#include "report/module_summary.hpp"

#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

namespace tezcatl::report {

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

std::string role(const FileNaming& naming, const fs::path& file) {
    return naming.is_test(file) ? "test" : "production";
}

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

json cycles(const graph::Digraph& graph, const std::vector<std::vector<std::size_t>>& found) {
    json result = json::array();
    for (const std::vector<std::size_t>& cycle : found) {
        json members = json::array();
        for (const std::size_t node : cycle) {
            members.push_back(graph.name(node));
        }
        result.push_back(members);
    }
    return result;
}

json settings(const ReportData& data, const FileNaming& naming) {
    json modules = json::array();
    for (const config::ModuleRule& rule : naming.modules().rules()) {
        modules.push_back({{"module", rule.module}, {"glob", rule.pattern}});
    }
    return {{"complexity",
             {{"flagged_over", data.thresholds.flagged_over},
              {"high_over", data.thresholds.high_over}}},
            {"test_globs", naming.roles().test_globs()},
            {"modules", modules}};
}

json input(const ReportData& data, const FileNaming& naming) {
    json coverage_inputs = json::array();
    for (const fs::path& file : data.provenance.coverage_inputs) {
        coverage_inputs.push_back(file.generic_string());
    }
    return {{"root", naming.root().generic_string()},
            {"compilation_database", data.provenance.compilation_database.generic_string()},
            {"coverage", coverage_inputs},
            {"translation_units", data.provenance.translation_units},
            {"units_with_errors", data.provenance.units_with_errors}};
}

json files(const ReportData& data, const FileNaming& naming) {
    json result = json::array();
    for (const FileLines& file : data.files) {
        result.push_back({{"path", naming.relative(file.file)},
                          {"module", naming.module_of(file.file)},
                          {"role", role(naming, file.file)},
                          {"lines", lines(file.counts)}});
    }
    return result;
}

json functions(const ReportData& data, const FileNaming& naming) {
    json result = json::array();
    for (const parse::FunctionInfo& f : data.functions) {
        const metrics::Halstead& h = f.halstead;
        result.push_back(
            {{"file", naming.relative(f.file)},
             {"line", f.line},
             {"column", f.column},
             {"kind", parse::to_string(f.kind)},
             {"name", f.name},
             {"module", naming.module_of(f.file)},
             {"role", role(naming, f.file)},
             {"complexity", f.complexity},
             {"rating", metrics::to_string(metrics::rate(f.complexity, data.thresholds))},
             {"halstead",
              {{"distinct_operators", h.distinct_operators},
               {"distinct_operands", h.distinct_operands},
               {"total_operators", h.total_operators},
               {"total_operands", h.total_operands},
               {"volume", h.volume()},
               {"difficulty", h.difficulty()},
               {"effort", h.effort()}}}});
    }
    return result;
}

json api(const ReportData& data, const FileNaming& naming) {
    json result = json::array();
    for (const parse::ApiEntity& e : data.api) {
        result.push_back({{"file", naming.relative(e.file)},
                          {"line", e.line},
                          {"column", e.column},
                          {"kind", parse::to_string(e.kind)},
                          {"name", e.name},
                          {"module", naming.module_of(e.file)},
                          {"role", role(naming, e.file)},
                          {"documentation", parse::to_string(e.documentation)}});
    }
    return result;
}

json coverage(const ReportData& data, const FileNaming& naming) {
    if (!data.coverage.has_value()) {
        return nullptr;
    }
    json result = json::array();
    for (const auto& [file, counts] : data.coverage->files) {
        result.push_back({{"file", naming.relative(file)},
                          {"module", naming.module_of(file)},
                          {"counts", coverage_counts(counts)}});
    }
    return result;
}

json includes(const IncludeGraph& graph) {
    json edges = json::array();
    const graph::Digraph& files = graph.files;
    for (std::size_t from = 0; from < files.size(); ++from) {
        for (const std::size_t to : files.successors(from)) {
            edges.push_back({{"from", files.name(from)}, {"to", files.name(to)}});
        }
    }
    json coupling = json::array();
    for (const ModuleCoupling& cell : graph.coupling) {
        coupling.push_back({{"from", cell.from}, {"to", cell.to}, {"edges", cell.edges}});
    }
    return {{"edges", edges},
            {"file_cycles", cycles(graph.files, graph.file_cycles)},
            {"module_cycles", cycles(graph.modules, graph.module_cycles)},
            {"coupling", coupling}};
}

} // namespace

void write_report_json(std::ostream& out, const ReportData& data, const FileNaming& naming) {
    json modules = json::array();
    for (const ModuleRow& row : summarize_modules(data, naming)) {
        modules.push_back(module_row(row, true));
    }
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
