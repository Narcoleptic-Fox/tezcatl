#pragma once

#include "parse/clang_handles.hpp"
#include "parse/compilation_database.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace tezcatl::parse {

/// A compiler diagnostic of severity error or fatal.
struct ParseError {
    std::string message; ///< formatted with file, line and column
};

/// A parsed translation unit and the errors met while parsing it.
///
/// libclang returns a usable AST even when parsing fails, with the broken
/// parts missing. Metrics from such an AST look plausible and are wrong, so
/// every caller must check `errors` and must not present the results of a
/// unit with errors as if they were complete.
struct ParsedUnit {
    std::filesystem::path file;
    /// The working directory of the compile. libclang reports file names as
    /// the command spelled them, so relative names are relative to this.
    std::filesystem::path directory;
    TranslationUnitHandle unit; ///< null only if libclang could not parse at all
    std::vector<ParseError> errors;
};

/// Owns the libclang index that translation units are parsed into.
class Parser {
public:
    /// `resource_directory` holds clang's own headers (stddef.h and the
    /// like); libclang cannot always find it by itself, for example when the
    /// DLL has been copied next to the executable.
    explicit Parser(std::filesystem::path resource_directory);

    /// Parses one compilation database entry with its own flags, as if run
    /// from its own working directory, keeping the detailed preprocessing
    /// record (macro expansions and every #include directive).
    [[nodiscard]] ParsedUnit parse(const CompileCommand& command) const;

private:
    IndexHandle index_;
    std::filesystem::path resource_directory_;
};

/// The resource directory found when Tezcatl was configured, or empty.
[[nodiscard]] std::filesystem::path default_resource_directory();

} // namespace tezcatl::parse
