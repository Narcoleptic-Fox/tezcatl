#include "cli/functions_command.hpp"
#include "cli/loc_command.hpp"
#include "parse/libclang_info.hpp"
#include "parse/translation_unit.hpp"
#include "tezcatl/version.hpp"

#include <CLI/CLI.hpp>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
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
        .build_directory = {},
        .root = std::filesystem::current_path(),
        .resource_directory = tezcatl::parse::default_resource_directory(),
        .allow_parse_errors = false};
    CLI::App* functions = app.add_subcommand(
        "functions", "List every function definition, parsed with the project's own flags.");
    functions
        ->add_option("-p,--build-dir", functions_options.build_directory,
                     "Directory containing compile_commands.json")
        ->required();
    functions
        ->add_option("--root", functions_options.root,
                     "Only report functions written under this directory")
        ->capture_default_str();
    functions
        ->add_option("--resource-dir", functions_options.resource_directory,
                     "clang resource directory (built-in headers such as stddef.h)")
        ->capture_default_str();
    functions->add_flag("--allow-parse-errors", functions_options.allow_parse_errors,
                        "Exit 0 even if some translation units failed to parse");

    CLI11_PARSE(app, argc, argv);

    if (loc->parsed()) {
        if (loc_by_line) {
            tezcatl::cli::run_loc_lines(loc_inputs, std::cout);
        } else {
            tezcatl::cli::run_loc(loc_inputs, std::cout);
        }
    }
    if (functions->parsed()) {
        return tezcatl::cli::run_functions(functions_options, std::cout, std::cerr);
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
