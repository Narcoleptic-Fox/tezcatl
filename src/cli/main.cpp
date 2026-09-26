#include "cli/coverage_command.hpp"
#include "cli/docs_command.hpp"
#include "cli/functions_command.hpp"
#include "cli/includes_command.hpp"
#include "cli/loc_command.hpp"
#include "cli/report_command.hpp"
#include "parse/libclang_info.hpp"
#include "parse/translation_unit.hpp"
#include "tezcatl/version.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace {

// If writing to stderr fails there is nowhere left to report that, and the
// exit code still signals failure, so the results are deliberately discarded
// (CERT ERR33-C). fputs rather than iostreams: this must not throw.
void report_fatal(const char* message) noexcept {
    (void)std::fputs("tezcatl: ", stderr);
    (void)std::fputs(message, stderr);
    (void)std::fputs("\n", stderr);
}

tezcatl::cli::ProjectOptions default_project_options() {
    return {.build_directory = {},
            .root = std::filesystem::current_path(),
            .resource_directory = tezcatl::parse::default_resource_directory(),
            .module_map = {},
            .test_globs = {},
            .allow_parse_errors = false,
            .jobs = 0};
}

// The options of every command that parses a project with libclang.
void add_project_options(CLI::App& command, tezcatl::cli::ProjectOptions& options) {
    command
        .add_option("-p,--build-dir", options.build_directory,
                    "Directory containing compile_commands.json")
        ->required();
    command.add_option("--root", options.root, "Only files under this directory are measured")
        ->capture_default_str();
    command
        .add_option("--modules", options.module_map,
                    "Module map file, one 'MODULE = GLOB' rule per line (globs relative to root)")
        ->check(CLI::ExistingFile);
    command
        .add_option("--resource-dir", options.resource_directory,
                    "clang resource directory (built-in headers such as stddef.h)")
        ->capture_default_str();
    command.add_flag("--allow-parse-errors", options.allow_parse_errors,
                     "Exit 0 even if some translation units failed to parse");
    command
        .add_option("-j,--jobs", options.jobs,
                    "Parsing threads (default 0: one per hardware thread)")
        ->capture_default_str();
}

void add_threshold_options(CLI::App& command, tezcatl::metrics::Thresholds& thresholds) {
    command.add_option("--flag-over", thresholds.flagged_over, "Complexity above this is flagged")
        ->capture_default_str();
    command.add_option("--high-over", thresholds.high_over, "Complexity above this is high")
        ->capture_default_str();
}

void add_path_map_option(CLI::App& command, std::vector<std::string>& texts) {
    command.add_option("--path-map", texts,
                       "FROM=TO: source paths recorded under FROM are found under TO");
}

std::vector<tezcatl::coverage::PathMapping> parse_path_maps(const std::vector<std::string>& texts) {
    std::vector<tezcatl::coverage::PathMapping> mappings;
    mappings.reserve(texts.size());
    std::ranges::transform(texts, std::back_inserter(mappings), [](const std::string& text) {
        return tezcatl::coverage::parse_path_mapping(text);
    });
    return mappings;
}

int run(int argc, char** argv) {
    CLI::App app{"Measures a C/C++ codebase and writes a code metrics baseline report.", "tezcatl"};
    app.set_version_flag("--version", [] {
        return "tezcatl " + std::string{tezcatl::version} +
               " (libclang: " + tezcatl::parse::libclang_version() + ")";
    });

    app.require_subcommand(0, 1);

    std::vector<std::filesystem::path> loc_inputs;
    CLI::App* loc = app.add_subcommand(
        "loc", "Count physical, blank, comment and code lines; writes CSV to stdout.");
    loc->add_option("paths", loc_inputs, "Source files or directories (searched recursively)")
        ->required();
    bool loc_by_line = false;
    loc->add_flag("--lines", loc_by_line, "Classify every physical line instead of totalling");

    tezcatl::cli::FunctionsOptions functions_options{
        .project = default_project_options(), .thresholds = {}, .summary = false};
    CLI::App* functions = app.add_subcommand(
        "functions", "List every function definition with its cyclomatic complexity.");
    add_project_options(*functions, functions_options.project);
    add_threshold_options(*functions, functions_options.thresholds);
    functions->add_flag("--summary", functions_options.summary,
                        "One row per module (count, mean, median, p90, max, over thresholds)");

    tezcatl::cli::CoverageOptions coverage_options{.inputs = {},
                                                   .root = std::filesystem::current_path(),
                                                   .module_map = {},
                                                   .path_maps = {},
                                                   .summary = false};
    std::vector<std::string> path_map_texts;
    CLI::App* coverage = app.add_subcommand(
        "coverage", "Import test coverage (lcov, gcov JSON, llvm-cov JSON) per file and module.");
    coverage->add_option("data", coverage_options.inputs, "Coverage files, in any mix of formats")
        ->required()
        ->check(CLI::ExistingFile);
    coverage
        ->add_option("--root", coverage_options.root,
                     "Only files under this directory are measured")
        ->capture_default_str();
    coverage
        ->add_option("--modules", coverage_options.module_map,
                     "Module map file, one 'MODULE = GLOB' rule per line (globs relative to root)")
        ->check(CLI::ExistingFile);
    add_path_map_option(*coverage, path_map_texts);
    coverage->add_flag("--summary", coverage_options.summary,
                       "One row per module, with line, branch and function percentages");

    tezcatl::cli::DocsOptions docs_options{.project = default_project_options(), .summary = false};
    CLI::App* docs = app.add_subcommand(
        "docs", "Documentation coverage of the public API declared in the project's headers.");
    add_project_options(*docs, docs_options.project);
    docs->add_flag("--summary", docs_options.summary,
                   "One row per module (declarations, documented, percent, doxygen)");

    tezcatl::cli::IncludesOptions includes_options{.project = default_project_options(),
                                                   .output = tezcatl::cli::IncludesOutput::edges};
    CLI::App* includes = app.add_subcommand(
        "includes", "The #include graph: edges, fan-in/fan-out, cycles and module coupling.");
    add_project_options(*includes, includes_options.project);
    const std::map<std::string, tezcatl::cli::IncludesOutput> outputs{
        {"edges", tezcatl::cli::IncludesOutput::edges},
        {"files", tezcatl::cli::IncludesOutput::files},
        {"modules", tezcatl::cli::IncludesOutput::modules},
        {"cycles", tezcatl::cli::IncludesOutput::cycles},
        {"coupling", tezcatl::cli::IncludesOutput::coupling},
        {"dot", tezcatl::cli::IncludesOutput::dot}};
    includes->add_option("--output", includes_options.output, "Table to write")
        ->transform(CLI::CheckedTransformer(outputs, CLI::ignore_case))
        ->default_str("edges");

    tezcatl::cli::ReportOptions report_options{.project = default_project_options(),
                                               .thresholds = {},
                                               .coverage_inputs = {},
                                               .path_maps = {},
                                               .output_directory = {}};
    CLI::App* report = app.add_subcommand(
        "report", "Measure everything and write the baseline report (Markdown, JSON, CSV, DOT).");
    add_project_options(*report, report_options.project);
    add_threshold_options(*report, report_options.thresholds);
    report
        ->add_option("--test-files", report_options.project.test_globs,
                     "Glob (relative to root) of test code, repeatable; replaces the defaults "
                     "**/test/**, **/tests/**, **/*_test.*, **/test_*.*")
        ->take_all();
    report
        ->add_option("--coverage", report_options.coverage_inputs,
                     "Coverage files (lcov, gcov JSON, llvm-cov JSON) to import")
        ->check(CLI::ExistingFile);
    add_path_map_option(*report, path_map_texts);
    report->add_option("-o,--out", report_options.output_directory, "Directory to write into")
        ->required();

    CLI11_PARSE(app, argc, argv);

    if (loc->parsed()) {
        if (loc_by_line) {
            tezcatl::cli::run_loc_lines(loc_inputs, std::cout);
        } else {
            tezcatl::cli::run_loc(loc_inputs, std::cout);
        }
    }
    if (functions->parsed()) {
        return tezcatl::cli::run_functions(functions_options, {.out = std::cout, .err = std::cerr});
    }
    if (coverage->parsed()) {
        coverage_options.path_maps = parse_path_maps(path_map_texts);
        return tezcatl::cli::run_coverage(coverage_options, {.out = std::cout, .err = std::cerr});
    }
    if (docs->parsed()) {
        return tezcatl::cli::run_docs(docs_options, {.out = std::cout, .err = std::cerr});
    }
    if (includes->parsed()) {
        return tezcatl::cli::run_includes(includes_options, {.out = std::cout, .err = std::cerr});
    }
    if (report->parsed()) {
        report_options.path_maps = parse_path_maps(path_map_texts);
        return tezcatl::cli::run_report(report_options, std::cerr);
    }
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv) {
    // CLI11_PARSE handles only command-line errors. Anything else escaping
    // main would call std::terminate with no message, so report it and fail.
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        report_fatal(error.what());
    } catch (...) {
        report_fatal("unknown error");
    }
    return EXIT_FAILURE;
}
