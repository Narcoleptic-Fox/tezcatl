#include "report/module_summary.hpp"

#include "report/csv.hpp"

#include <format>
#include <functional>
#include <map>
#include <ostream>
#include <utility>

namespace tezcatl::report {

namespace fs = std::filesystem;

namespace {

struct Accumulator {
    ModuleRow row;
    std::vector<unsigned> complexities;
};

using KeyOf = std::function<std::string(const fs::path&)>;

// Adds every file, function, declaration and coverage record of the report
// to the accumulator its file's key names. One function serves modules and
// the whole project, so the two cannot disagree on what they count.
std::map<std::string, Accumulator> accumulate(const ReportData& data, const FileNaming& naming,
                                              const KeyOf& key_of) {
    std::map<std::string, Accumulator> result;
    for (const FileLines& file : data.files) {
        ModuleRow& row = result[key_of(file.file)].row;
        ++row.files;
        row.files_parsed += file.parsed ? 1 : 0;
        (naming.is_test(file.file) ? row.test : row.production) += file.counts;
    }
    for (const parse::FunctionInfo& function : data.functions) {
        if (naming.is_test(function.file)) {
            continue;
        }
        Accumulator& accumulator = result[key_of(function.file)];
        accumulator.complexities.push_back(function.complexity);
        accumulator.row.halstead_volume += function.halstead.volume();
        accumulator.row.halstead_effort += function.halstead.effort();
    }
    for (const parse::ApiEntity& entity : data.api) {
        if (!naming.is_test(entity.file)) {
            result[key_of(entity.file)].row.documentation.add(entity);
        }
    }
    if (data.coverage.has_value()) {
        for (const auto& [file, counts] : data.coverage->files) {
            ModuleRow& row = result[key_of(file)].row;
            row.coverage = row.coverage.value_or(coverage::Counts{});
            *row.coverage += counts;
        }
        for (auto& [key, accumulator] : result) {
            accumulator.row.coverage = accumulator.row.coverage.value_or(coverage::Counts{});
        }
    }
    for (auto& [key, accumulator] : result) {
        accumulator.row.module = key;
        accumulator.row.complexity = metrics::describe(accumulator.complexities, data.thresholds);
    }
    return result;
}

std::string optional_number(const std::optional<std::size_t>& value) {
    return value.has_value() ? std::to_string(*value) : std::string{};
}

void write_row(std::ostream& out, const ModuleRow& row, bool with_includes) {
    const metrics::Distribution& c = row.complexity;
    out << csv_field(row.module) << ',' << row.files << ',' << row.files_parsed << ','
        << row.production.physical << ',' << row.production.code << ',' << row.production.comment
        << ',' << row.production.blank << ',' << row.test.physical << ',' << row.test.code << ','
        << c.count << ',' << std::format("{:.2f},{:.1f}", c.mean, c.median) << ',' << c.p90 << ','
        << c.max << ',' << c.flagged << ',' << c.high << ','
        << std::format("{:.2f},{:.2f}", row.halstead_volume, row.halstead_effort) << ','
        << row.documentation.entities << ',' << row.documentation.documented << ','
        << std::format("{:.1f}", row.documentation.percent()) << ',';
    if (row.coverage.has_value()) {
        const coverage::Counts& v = *row.coverage;
        out << v.lines << ',' << v.lines_covered << ',' << v.branches << ',' << v.branches_covered
            << ',' << v.functions << ',' << v.functions_covered << ',';
    } else {
        out << ",,,,,,";
    }
    if (with_includes) {
        out << row.fan_in << ',' << row.fan_out << ',' << optional_number(row.cycle);
    } else {
        out << ",,";
    }
    out << '\n';
}

} // namespace

std::vector<ModuleRow> summarize_modules(const ReportData& data, const FileNaming& naming) {
    std::map<std::string, Accumulator> modules = accumulate(
        data, naming, [&naming](const fs::path& file) { return naming.module_of(file); });
    const graph::Digraph& graph = data.includes.modules;
    for (std::size_t node = 0; node < graph.size(); ++node) {
        ModuleRow& row = modules[graph.name(node)].row;
        row.module = graph.name(node);
        row.fan_in = graph.fan_in(node);
        row.fan_out = graph.fan_out(node);
        row.cycle = data.includes.module_cycle.at(node);
    }
    std::vector<ModuleRow> rows;
    rows.reserve(modules.size());
    for (auto& [module, accumulator] : modules) {
        rows.push_back(std::move(accumulator.row));
    }
    return rows;
}

ModuleRow summarize_project(const ReportData& data, const FileNaming& naming) {
    std::map<std::string, Accumulator> all =
        accumulate(data, naming, [](const fs::path& /*file*/) { return std::string{"TOTAL"}; });
    if (all.empty()) {
        ModuleRow empty;
        empty.module = "TOTAL";
        empty.coverage =
            data.coverage.has_value() ? std::optional{coverage::Counts{}} : std::nullopt;
        return empty;
    }
    return std::move(all.begin()->second.row);
}

void write_module_table(std::ostream& out, const std::vector<ModuleRow>& modules,
                        const ModuleRow& total) {
    out << "module,files,files_parsed,production_physical,production_code,production_comment,"
           "production_blank,test_physical,test_code,functions,complexity_mean,"
           "complexity_median,complexity_p90,complexity_max,flagged,high,halstead_volume,"
           "halstead_effort,api,documented,documented_percent,coverage_lines,"
           "coverage_lines_covered,coverage_branches,coverage_branches_covered,"
           "coverage_functions,coverage_functions_covered,fan_in,fan_out,cycle\n";
    for (const ModuleRow& row : modules) {
        write_row(out, row, true);
    }
    write_row(out, total, false);
}

} // namespace tezcatl::report
