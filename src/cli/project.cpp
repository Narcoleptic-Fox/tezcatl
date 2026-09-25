#include "cli/project.hpp"

#include "parse/compilation_database.hpp"
#include "scan/paths.hpp"

#include <ostream>

namespace tezcatl::cli {

namespace fs = std::filesystem;

namespace {

// Enough to diagnose a broken unit without burying the rest of the output.
constexpr std::size_t max_errors_shown_per_unit = 5;

void report_errors(const parse::ParsedUnit& parsed, std::ostream& err) {
    std::size_t shown = 0;
    for (const parse::ParseError& error : parsed.errors) {
        if (shown++ == max_errors_shown_per_unit) {
            err << "  ... and " << parsed.errors.size() - max_errors_shown_per_unit
                << " more errors in " << parsed.file.generic_string() << '\n';
            break;
        }
        err << "error: " << error.message << '\n';
    }
}

} // namespace

Project::Project(const ProjectOptions& options)
    : options_(options), root_(fs::absolute(options.root).lexically_normal()),
      build_(options.build_directory.empty()
                 ? fs::path{}
                 : fs::absolute(options.build_directory).lexically_normal()),
      in_project_([this](const fs::path& file) {
          return scan::is_within(file, root_) &&
                 (excluded_.empty() || !scan::is_within(file, excluded_));
      }),
      naming_(options.root,
              options.module_map.empty() ? config::ModuleMap{}
                                         : config::ModuleMap::load(options.module_map),
              options.test_globs.empty() ? config::FileRoles{}
                                         : config::FileRoles{options.test_globs}) {
    // A build directory inside the root holds generated code and fetched
    // dependencies. One that is the root, or above it, is an in-source build
    // (Make with bear writes compile_commands.json at the top): excluding it
    // would exclude the whole project.
    if (!build_.empty() && scan::is_within(build_, root_) && !scan::is_within(root_, build_)) {
        excluded_ = build_;
    }
}

ScanTotals Project::scan(const std::function<void(const parse::ParsedUnit&)>& visit,
                         std::ostream& err) const {
    const parse::Parser parser{options_.resource_directory};
    ScanTotals totals;
    for (const parse::CompileCommand& command : parse::load_compilation_database(build_)) {
        const parse::ParsedUnit parsed = parser.parse(command);
        ++totals.units;
        if (!parsed.errors.empty()) {
            ++totals.units_with_errors;
            report_errors(parsed, err);
        }
        if (parsed.unit) {
            visit(parsed);
        }
    }
    return totals;
}

int Project::finish(const ScanTotals& totals, std::string_view detail, std::ostream& err) const {
    err << "tezcatl: parsed " << totals.units << " translation units, " << totals.units_with_errors
        << " with errors; " << detail << '\n';
    return (totals.units_with_errors > 0 && !options_.allow_parse_errors) ? 1 : 0;
}

} // namespace tezcatl::cli
