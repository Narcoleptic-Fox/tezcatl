#pragma once

#include <filesystem>
#include <iosfwd>

namespace tezcatl::cli {

struct FunctionsOptions {
    std::filesystem::path build_directory;    ///< holds compile_commands.json
    std::filesystem::path root;               ///< only functions written under here count
    std::filesystem::path resource_directory; ///< clang's built-in headers
    bool allow_parse_errors = false;
};

/// `tezcatl functions -p BUILD_DIR`: parses every compilation database entry
/// and writes one CSV row per function definition, columns
/// file,line,column,kind,name, with files relative to the root. Functions in
/// system headers or under the build directory (fetched dependencies,
/// generated code) are left out. Parse errors are written to `err`, and make
/// the exit code 1 unless allowed, because a unit that failed to parse yields
/// an incomplete function list that would otherwise pass for a complete one.
[[nodiscard]] int run_functions(const FunctionsOptions& options, std::ostream& out,
                                std::ostream& err);

} // namespace tezcatl::cli
