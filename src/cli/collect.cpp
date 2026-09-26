#include "cli/collect.hpp"

#include "parse/merge.hpp"

#include <filesystem>
#include <iterator>
#include <vector>

namespace tezcatl::cli {

namespace {

template <typename T> void append(std::vector<T>& to, std::vector<T> from) {
    to.insert(to.end(), std::make_move_iterator(from.begin()), std::make_move_iterator(from.end()));
}

} // namespace

Collected collect(const Project& project, Collect what, std::ostream& err) {
    Collected result;
    const parse::FileFilter& in_project = project.in_project();
    result.totals = project.scan(
        [&](const parse::ParsedUnit& parsed) {
            if (what.functions) {
                append(result.functions, parse::find_functions(parsed, in_project));
            }
            if (what.api) {
                append(result.api, parse::find_api(parsed, in_project));
            }
            if (what.includes) {
                const std::filesystem::path source = parsed.file.lexically_normal();
                if (in_project(source)) {
                    result.sources.push_back(source);
                }
                append(result.includes, parse::find_includes(parsed, in_project));
            }
        },
        err);
    parse::merge_duplicates(result.functions);
    parse::merge_duplicates(result.api);
    return result;
}

} // namespace tezcatl::cli
