#include "cli/docs_command.hpp"

#include "parse/api.hpp"
#include "report/tables.hpp"

#include <format>
#include <ostream>
#include <vector>

namespace tezcatl::cli {

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

    if (options.summary) {
        report::write_api_summary(out, entities, project.naming());
    } else {
        report::write_api_table(out, entities, project.naming());
    }
    report::DocCoverage all;
    for (const parse::ApiEntity& entity : entities) {
        all.add(entity);
    }
    return project.finish(totals,
                          std::format("{} API declarations, {} documented ({:.1f}%)", all.entities,
                                      all.documented, all.percent()),
                          err);
}

} // namespace tezcatl::cli
