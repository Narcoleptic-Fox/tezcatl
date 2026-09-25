#include "cli/docs_command.hpp"

#include "cli/collect.hpp"
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
    const Collected found = collect(project, {.api = true}, err);
    const std::vector<parse::ApiEntity>& entities = found.api;

    if (options.summary) {
        report::write_api_summary(out, entities, project.naming());
    } else {
        report::write_api_table(out, entities, project.naming());
    }
    report::DocCoverage all;
    for (const parse::ApiEntity& entity : entities) {
        all.add(entity);
    }
    return project.finish(found.totals,
                          std::format("{} API declarations, {} documented ({:.1f}%)", all.entities,
                                      all.documented, all.percent()),
                          err);
}

} // namespace tezcatl::cli
