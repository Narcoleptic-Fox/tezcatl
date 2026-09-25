#include "parse/translation_unit.hpp"

#include "parse/clang_string.hpp"
#include "parse/command_line.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace tezcatl::parse {

namespace fs = std::filesystem;

namespace {

std::vector<ParseError> collect_errors(CXTranslationUnit unit) {
    std::vector<ParseError> errors;
    const unsigned count = clang_getNumDiagnostics(unit);
    for (unsigned i = 0; i < count; ++i) {
        const DiagnosticHandle diagnostic{clang_getDiagnostic(unit, i)};
        const CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diagnostic.get());
        if (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal) {
            errors.push_back({std::string{ClangString{
                clang_formatDiagnostic(diagnostic.get(), clang_defaultDiagnosticDisplayOptions())}
                                              .view()}});
        }
    }
    return errors;
}

} // namespace

Parser::Parser(fs::path resource_directory)
    : index_(clang_createIndex(/*excludeDeclarationsFromPCH=*/0, /*displayDiagnostics=*/0)),
      resource_directory_(std::move(resource_directory)) {
    if (!index_) {
        throw std::runtime_error("libclang could not create an index");
    }
}

ParsedUnit Parser::parse(const CompileCommand& command) const {
    // The command line as the build ran it, with its paths made independent of
    // the current directory, followed by Tezcatl's own options:
    // - where clang's built-in headers live (clang-cl accepts this spelling
    //   too, measured with LLVM 22.1.3);
    // - "-Wno-error", overriding the project's -Werror or /WX. A warning is
    //   the project's build policy, not a failure to parse, and libclang
    //   itself adds flags that clang-cl warns about, which /WX would turn into
    //   errors in every unit. Real errors (a missing header, bad syntax) still
    //   count;
    // - the detailed preprocessing record, which is not optional. Without it
    //   libclang annotates tokens written in a macro's arguments with the
    //   enclosing statement, so the "&&" in CHECK(a && b) is invisible to
    //   complexity, and it holds the #include directives that an include
    //   guard or #pragma once skipped, which the include graph needs.
    //   CXTranslationUnit_DetailedPreprocessingRecord alone is not enough:
    //   libclang implements it by appending these same two arguments after
    //   the caller's, where the GCC-mode "--" before the source file turns
    //   them into file names and the record silently disappears (measured
    //   with LLVM 22.1.3). Passed here, they come before the "--";
    // - "-fno-delayed-template-parsing". clang-cl before C++20 does not parse
    //   the body of a template until it is instantiated, as MSVC does, so an
    //   uninstantiated function template or member of a class template has no
    //   body in the AST and would silently vanish from the report (measured:
    //   Catch2 built as C++14 lost its template member functions). Accepted
    //   by both drivers.
    std::vector<std::string> extra_options;
    if (!resource_directory_.empty()) {
        extra_options.push_back("-resource-dir=" + resource_directory_.string());
    }
    extra_options.emplace_back("-Wno-error");
    extra_options.emplace_back("-fno-delayed-template-parsing");
    extra_options.emplace_back("-Xclang");
    extra_options.emplace_back("-detailed-preprocessing-record");
    const std::vector<std::string> arguments = portable_arguments(command, extra_options);

    std::vector<const char*> argv(arguments.size());
    std::ranges::transform(arguments, argv.begin(),
                           [](const std::string& argument) { return argument.c_str(); });

    constexpr unsigned options =
        CXTranslationUnit_KeepGoing | CXTranslationUnit_DetailedPreprocessingRecord;
    CXTranslationUnit raw_unit = nullptr;
    const CXErrorCode code = clang_parseTranslationUnit2FullArgv(
        index_.get(), /*source_filename=*/nullptr, argv.data(), static_cast<int>(argv.size()),
        /*unsaved_files=*/nullptr, 0, options, &raw_unit);

    ParsedUnit parsed{.file = command.file,
                      .directory = command.directory,
                      .unit = TranslationUnitHandle{raw_unit},
                      .errors = {}};
    if (code != CXError_Success || !parsed.unit) {
        parsed.unit.reset();
        parsed.errors.push_back({"libclang failed to parse " + command.file.string() +
                                 " (error code " + std::to_string(static_cast<int>(code)) + ")"});
        return parsed;
    }
    parsed.errors = collect_errors(parsed.unit.get());
    return parsed;
}

fs::path default_resource_directory() {
    return fs::path{TEZCATL_CLANG_RESOURCE_DIR};
}

} // namespace tezcatl::parse
