#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace tezcatl::parse {

/// One entry of a compilation database: how one source file is compiled.
struct CompileCommand {
    std::filesystem::path file;         ///< absolute and normalised
    std::filesystem::path directory;    ///< the working directory of the compile
    std::vector<std::string> arguments; ///< the full command line, compiler first
};

/// Loads compile_commands.json from `build_directory`, in file order. Throws
/// std::runtime_error if the database cannot be loaded or has no entries:
/// an empty database would otherwise produce an empty, plausible-looking
/// report.
[[nodiscard]] std::vector<CompileCommand>
load_compilation_database(const std::filesystem::path& build_directory);

} // namespace tezcatl::parse
