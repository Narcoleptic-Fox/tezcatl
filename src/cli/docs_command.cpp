#include "cli/docs_command.hpp"

#include "parse/api.hpp"
#include "report/csv.hpp"

#include <cstddef>
#include <format>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace tezcatl::cli {

namespace {

struct Coverage {
    std::size_t entities = 0;
    std::size_t documented = 0;
    std::size_t doxygen = 0;

    void add(const parse::ApiEntity& entity) {
        ++entities;
        if (entity.documentation != parse::DocStyle::none) {
            ++documented;
        }
        if (entity.documentation == parse::DocStyle::doxygen) {
            ++doxygen;
        }
    }

    [[nodiscard]] double percent() const {
        constexpr double hundred = 100.0;
        return entities == 0
                   ? 0.0
                   : hundred * static_cast<double>(documented) / static_cast<double>(entities);
    }
};

void write_summary_row(std::ostream& out, const std::string& module, const Coverage& coverage) {
    out << report::csv_field(module) << ',' << coverage.entities << ',' << coverage.documented
        << ',' << std::format("{:.1f}", coverage.percent()) << ',' << coverage.doxygen << '\n';
}

void write_summary(const Project& project, const std::vector<parse::ApiEntity>& entities,
                   std::ostream& out) {
    std::map<std::string, Coverage> by_module;
    for (const parse::ApiEntity& entity : entities) {
        by_module[project.naming().module_of(entity.file)].add(entity);
    }
    out << "module,entities,documented,percent,doxygen\n";
    for (const auto& [module, coverage] : by_module) {
        write_summary_row(out, module, coverage);
    }
}

void write_entities(const Project& project, const std::vector<parse::ApiEntity>& entities,
                    std::ostream& out) {
    out << "file,line,column,kind,name,module,documented,style\n";
    for (const parse::ApiEntity& entity : entities) {
        const bool documented = entity.documentation != parse::DocStyle::none;
        out << report::csv_field(project.naming().relative(entity.file)) << ',' << entity.line
            << ',' << entity.column << ',' << parse::to_string(entity.kind) << ','
            << report::csv_field(entity.name) << ','
            << report::csv_field(project.naming().module_of(entity.file)) << ','
            << (documented ? "yes" : "no") << ',' << parse::to_string(entity.documentation) << '\n';
    }
}

} // namespace

int run_docs(const DocsOptions& options, const Streams& streams) {
    std::ostream& out = streams.out;
    std::ostream& err = streams.err;
    const Project project{options.project};
    std::vector<parse::ApiEntity> entities;
    const ScanTotals totals = project.scan(
        [&](const parse::ParsedUnit& parsed) {
            auto found = parse::find_api(parsed, project.in_project());
            entities.insert(entities.end(), found.begin(), found.end());
        },
        err);
    parse::merge_duplicates(entities);

    Coverage all;
    for (const parse::ApiEntity& entity : entities) {
        all.add(entity);
    }
    if (options.summary) {
        write_summary(project, entities, out);
        write_summary_row(out, "TOTAL", all);
    } else {
        write_entities(project, entities, out);
    }
    return project.finish(totals,
                          std::format("{} API declarations, {} documented ({:.1f}%)", all.entities,
                                      all.documented, all.percent()),
                          err);
}

} // namespace tezcatl::cli
