#include "report/tables.hpp"

#include "report/csv.hpp"

#include <cstddef>
#include <format>
#include <map>
#include <ostream>
#include <string>

namespace tezcatl::report {

namespace {

// What the function summary adds up for a module: the complexity of each
// function, and the Halstead volume and effort, which are additive.
struct FunctionTotals {
    std::vector<unsigned> complexities;
    double volume = 0.0;
    double effort = 0.0;

    void add(const parse::FunctionInfo& function) {
        complexities.push_back(function.complexity);
        volume += function.halstead.volume();
        effort += function.halstead.effort();
    }
};

void write_function_summary_row(std::ostream& out, const std::string& module,
                                const FunctionTotals& totals,
                                const metrics::Thresholds& thresholds) {
    const metrics::Distribution distribution = metrics::describe(totals.complexities, thresholds);
    out << csv_field(module) << ',' << distribution.count << ','
        << std::format("{:.2f}", distribution.mean) << ','
        << std::format("{:.1f}", distribution.median) << ',' << distribution.p90 << ','
        << distribution.max << ',' << distribution.flagged << ',' << distribution.high << ','
        << std::format("{:.2f},{:.2f}", totals.volume, totals.effort) << '\n';
}

void write_api_summary_row(std::ostream& out, const std::string& module,
                           const DocCoverage& coverage) {
    out << csv_field(module) << ',' << coverage.entities << ',' << coverage.documented << ','
        << std::format("{:.1f}", coverage.percent()) << ',' << coverage.doxygen << '\n';
}

std::string percent(std::size_t covered, std::size_t total) {
    constexpr double hundred = 100.0;
    return total == 0 ? std::string{}
                      : std::format("{:.1f}", hundred * static_cast<double>(covered) /
                                                  static_cast<double>(total));
}

struct ModuleCoverage {
    std::size_t files = 0;
    coverage::Counts counts;
};

void write_coverage_summary_row(std::ostream& out, const std::string& module,
                                const ModuleCoverage& m) {
    const coverage::Counts& c = m.counts;
    out << csv_field(module) << ',' << m.files << ',' << c.lines << ',' << c.lines_covered << ','
        << percent(c.lines_covered, c.lines) << ',' << c.branches << ',' << c.branches_covered
        << ',' << percent(c.branches_covered, c.branches) << ',' << c.functions << ','
        << c.functions_covered << ',' << percent(c.functions_covered, c.functions) << '\n';
}

} // namespace

void write_file_table(std::ostream& out, const std::vector<FileLines>& files,
                      const FileNaming& naming) {
    out << "file,module,role,parsed,physical,blank,comment,code\n";
    for (const FileLines& file : files) {
        const metrics::LocCounts& c = file.counts;
        out << csv_field(naming.relative(file.file)) << ','
            << csv_field(naming.module_of(file.file)) << ',' << naming.role_of(file.file) << ','
            << (file.parsed ? "yes" : "no") << ',' << c.physical << ',' << c.blank << ','
            << c.comment << ',' << c.code << '\n';
    }
}

void write_function_table(std::ostream& out, const std::vector<parse::FunctionInfo>& functions,
                          const FileNaming& naming, const metrics::Thresholds& thresholds) {
    out << "file,line,column,kind,name,module,complexity,rating,"
           "distinct_operators,distinct_operands,total_operators,total_operands,"
           "volume,difficulty,effort\n";
    for (const parse::FunctionInfo& function : functions) {
        const metrics::Halstead& h = function.halstead;
        out << csv_field(naming.relative(function.file)) << ',' << function.line << ','
            << function.column << ',' << parse::to_string(function.kind) << ','
            << csv_field(function.name) << ',' << csv_field(naming.module_of(function.file)) << ','
            << function.complexity << ','
            << metrics::to_string(metrics::rate(function.complexity, thresholds)) << ','
            << h.distinct_operators << ',' << h.distinct_operands << ',' << h.total_operators << ','
            << h.total_operands << ','
            << std::format("{:.2f},{:.2f},{:.2f}", h.volume(), h.difficulty(), h.effort()) << '\n';
    }
}

void write_function_summary(std::ostream& out, const std::vector<parse::FunctionInfo>& functions,
                            const FileNaming& naming, const metrics::Thresholds& thresholds) {
    std::map<std::string, FunctionTotals> by_module;
    FunctionTotals all;
    for (const parse::FunctionInfo& function : functions) {
        by_module[naming.module_of(function.file)].add(function);
        all.add(function);
    }
    out << "module,functions,mean,median,p90,max,flagged,high,volume,effort\n";
    for (const auto& [module, totals] : by_module) {
        write_function_summary_row(out, module, totals, thresholds);
    }
    write_function_summary_row(out, "TOTAL", all, thresholds);
}

void write_api_table(std::ostream& out, const std::vector<parse::ApiEntity>& api,
                     const FileNaming& naming) {
    out << "file,line,column,kind,name,module,documented,style\n";
    for (const parse::ApiEntity& entity : api) {
        const bool documented = entity.documentation != parse::DocStyle::none;
        out << csv_field(naming.relative(entity.file)) << ',' << entity.line << ',' << entity.column
            << ',' << parse::to_string(entity.kind) << ',' << csv_field(entity.name) << ','
            << csv_field(naming.module_of(entity.file)) << ',' << (documented ? "yes" : "no") << ','
            << parse::to_string(entity.documentation) << '\n';
    }
}

void DocCoverage::add(const parse::ApiEntity& entity) {
    ++entities;
    if (entity.documentation != parse::DocStyle::none) {
        ++documented;
    }
    if (entity.documentation == parse::DocStyle::doxygen) {
        ++doxygen;
    }
}

double DocCoverage::percent() const {
    constexpr double hundred = 100.0;
    return entities == 0
               ? 0.0
               : hundred * static_cast<double>(documented) / static_cast<double>(entities);
}

void write_api_summary(std::ostream& out, const std::vector<parse::ApiEntity>& api,
                       const FileNaming& naming) {
    std::map<std::string, DocCoverage> by_module;
    DocCoverage all;
    for (const parse::ApiEntity& entity : api) {
        by_module[naming.module_of(entity.file)].add(entity);
        all.add(entity);
    }
    out << "module,entities,documented,percent,doxygen\n";
    for (const auto& [module, coverage] : by_module) {
        write_api_summary_row(out, module, coverage);
    }
    write_api_summary_row(out, "TOTAL", all);
}

AttributedCoverage attribute(const coverage::CoverageData& data,
                             const std::vector<coverage::PathMapping>& mappings,
                             const parse::FileFilter& in_project) {
    std::map<std::filesystem::path, coverage::Counts> files;
    AttributedCoverage result;
    for (const auto& [recorded, record] : data) {
        const std::filesystem::path file = coverage::map_path(recorded, mappings);
        if (in_project(file)) {
            files[file] += record.counts();
        } else {
            ++result.outside;
        }
    }
    result.files.assign(files.begin(), files.end());
    return result;
}

void write_coverage_table(std::ostream& out, const FileCoverage& files, const FileNaming& naming) {
    out << "file,module,lines,lines_covered,branches,branches_covered,functions,"
           "functions_covered\n";
    for (const auto& [file, c] : files) {
        out << csv_field(naming.relative(file)) << ',' << csv_field(naming.module_of(file)) << ','
            << c.lines << ',' << c.lines_covered << ',' << c.branches << ',' << c.branches_covered
            << ',' << c.functions << ',' << c.functions_covered << '\n';
    }
}

void write_coverage_summary(std::ostream& out, const FileCoverage& files,
                            const FileNaming& naming) {
    std::map<std::string, ModuleCoverage> modules;
    ModuleCoverage all;
    for (const auto& [file, counts] : files) {
        ModuleCoverage& module = modules[naming.module_of(file)];
        ++module.files;
        module.counts += counts;
        ++all.files;
        all.counts += counts;
    }
    out << "module,files,lines,lines_covered,line_percent,branches,branches_covered,"
           "branch_percent,functions,functions_covered,function_percent\n";
    for (const auto& [module, coverage] : modules) {
        write_coverage_summary_row(out, module, coverage);
    }
    write_coverage_summary_row(out, "TOTAL", all);
}

} // namespace tezcatl::report
