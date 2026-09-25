#include "parse/libclang_info.hpp"
#include "tezcatl/version.hpp"

#include <CLI/CLI.hpp>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

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

    CLI11_PARSE(app, argc, argv);
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
