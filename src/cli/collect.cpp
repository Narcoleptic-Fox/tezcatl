#include "cli/collect.hpp"

#include "parse/merge.hpp"

#include <filesystem>
#include <iterator>
#include <utility>
#include <vector>

namespace tezcatl::cli {

namespace {

template <typename T> void append(std::vector<T>& to, std::vector<T> from) {
    to.insert(to.end(), std::make_move_iterator(from.begin()), std::make_move_iterator(from.end()));
}

} // namespace

Collected collect(const Project& project, Collect what, std::ostream& err) {
    const ScanPlan plan = project.plan();
    // One slot per unit, filled by whichever thread parses it, then joined in
    // database order: merge_duplicates keeps the first of each repeat, so
    // the order must not depend on which thread finished first.
    std::vector<Collected> per_unit(plan.units.size());
    const parse::FileFilter& in_project = project.in_project();
    Collected result;
    result.totals = project.scan(
        plan,
        [&](std::size_t unit, const parse::ParsedUnit& parsed) {
            Collected& found = per_unit.at(unit);
            if (what.functions) {
                found.functions = parse::find_functions(parsed, in_project);
            }
            if (what.api) {
                found.api = parse::find_api(parsed, in_project);
            }
            if (what.includes) {
                const std::filesystem::path source = parsed.file.lexically_normal();
                if (in_project(source)) {
                    found.sources.push_back(source);
                }
                found.includes = parse::find_includes(parsed, in_project);
            }
        },
        err);
    for (Collected& found : per_unit) {
        append(result.functions, std::move(found.functions));
        append(result.api, std::move(found.api));
        append(result.includes, std::move(found.includes));
        append(result.sources, std::move(found.sources));
    }
    parse::merge_duplicates(result.functions);
    parse::merge_duplicates(result.api);
    return result;
}

} // namespace tezcatl::cli
