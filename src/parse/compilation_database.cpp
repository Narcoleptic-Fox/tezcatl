#include "parse/compilation_database.hpp"

#include "parse/clang_handles.hpp"
#include "parse/clang_string.hpp"

#include <stdexcept>

namespace tezcatl::parse {

namespace fs = std::filesystem;

std::vector<CompileCommand> load_compilation_database(const fs::path& build_directory) {
    CXCompilationDatabase_Error error = CXCompilationDatabase_NoError;
    const CompilationDatabaseHandle database{
        clang_CompilationDatabase_fromDirectory(build_directory.string().c_str(), &error)};
    if (error != CXCompilationDatabase_NoError || !database) {
        throw std::runtime_error("cannot load compile_commands.json from " +
                                 build_directory.string());
    }

    const CompileCommandsHandle commands{
        clang_CompilationDatabase_getAllCompileCommands(database.get())};
    const unsigned count = commands ? clang_CompileCommands_getSize(commands.get()) : 0U;
    if (count == 0) {
        throw std::runtime_error("compile_commands.json in " + build_directory.string() +
                                 " has no entries");
    }

    std::vector<CompileCommand> result;
    result.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        CXCompileCommand command = clang_CompileCommands_getCommand(commands.get(), i);
        CompileCommand entry;
        entry.directory = fs::path{ClangString{clang_CompileCommand_getDirectory(command)}.view()};
        const fs::path file{ClangString{clang_CompileCommand_getFilename(command)}.view()};
        entry.file = (entry.directory / file).lexically_normal();
        const unsigned argument_count = clang_CompileCommand_getNumArgs(command);
        entry.arguments.reserve(argument_count);
        for (unsigned a = 0; a < argument_count; ++a) {
            entry.arguments.emplace_back(
                ClangString{clang_CompileCommand_getArg(command, a)}.view());
        }
        result.push_back(std::move(entry));
    }
    return result;
}

} // namespace tezcatl::parse
