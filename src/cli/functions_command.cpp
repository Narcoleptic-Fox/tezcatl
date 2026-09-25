#include "cli/functions_command.hpp"

#include "parse/compilation_database.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"
#include "report/csv.hpp"
#include "scan/paths.hpp"

#include <cstddef>
#include <ostream>
#include <vector>

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

int run_functions(const FunctionsOptions& options, std::ostream& out, std::ostream& err) {
    const fs::path root = fs::absolute(options.root).lexically_normal();
    const fs::path build = fs::absolute(options.build_directory).lexically_normal();
    const parse::FileFilter in_project = [&root, &build](const fs::path& file) {
        return scan::is_within(file, root) && !scan::is_within(file, build);
    };

    const parse::Parser parser{options.resource_directory};
    std::vector<parse::FunctionInfo> functions;
    std::size_t units = 0;
    std::size_t units_with_errors = 0;
    for (const parse::CompileCommand& command : parse::load_compilation_database(build)) {
        const parse::ParsedUnit parsed = parser.parse(command);
        ++units;
        if (!parsed.errors.empty()) {
            ++units_with_errors;
            report_errors(parsed, err);
        }
        if (parsed.unit) {
            auto found = parse::find_functions(parsed, in_project);
            functions.insert(functions.end(), found.begin(), found.end());
        }
    }
    parse::merge_duplicates(functions);

    out << "file,line,column,kind,name\n";
    for (const parse::FunctionInfo& function : functions) {
        out << report::csv_field(function.file.lexically_relative(root).generic_string()) << ','
            << function.line << ',' << function.column << ',' << parse::to_string(function.kind)
            << ',' << report::csv_field(function.name) << '\n';
    }

    err << "tezcatl: parsed " << units << " translation units, " << units_with_errors
        << " with errors; " << functions.size() << " functions\n";
    return (units_with_errors > 0 && !options.allow_parse_errors) ? 1 : 0;
}

} // namespace tezcatl::cli
