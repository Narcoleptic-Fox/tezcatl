#include "parse/command_line.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <string_view>

namespace tezcatl::parse {

namespace fs = std::filesystem;

namespace {

// Flags whose operand is a path that changes what is parsed. Longer names come
// before names they begin with, so "-FI" is tried before any shorter match.
constexpr std::array<std::string_view, 12> gcc_path_flags{"--include-directory=",
                                                          "--sysroot=",
                                                          "-cxx-isystem",
                                                          "-iframework",
                                                          "-idirafter",
                                                          "-isysroot",
                                                          "-isystem",
                                                          "-imacros",
                                                          "-include",
                                                          "-iquote",
                                                          "-I",
                                                          "-F"};

// In cl mode "/X" and "-X" are the same option. "-F" is deliberately absent:
// there it begins /Fo, /Fd and /Fe, which name outputs, not inputs.
constexpr std::array<std::string_view, 5> cl_path_flags{"-external:I", "-isystem", "-imsvc", "-FI",
                                                        "-I"};

std::string absolute(std::string_view path, const fs::path& directory) {
    const fs::path candidate{path};
    if (path.empty() || candidate.is_absolute()) {
        return std::string{path};
    }
    return (directory / candidate).lexically_normal().string();
}

std::string_view file_stem_of_program(std::string_view program) {
    const auto slash = program.find_last_of("/\\");
    std::string_view name = slash == std::string_view::npos ? program : program.substr(slash + 1);
    if (name.ends_with(".exe") || name.ends_with(".EXE")) {
        name.remove_suffix(4);
    }
    return name;
}

// "/Tp" (C++) or "/Tc" (C) for the source file in a clang-cl command: the
// project's own /TP or /TC wins, as it would in the real build, and otherwise
// a ".c" file is C.
std::string cl_source_marker(const std::vector<std::string>& arguments, const fs::path& file) {
    for (const std::string& argument : arguments) {
        if (argument == "/TP" || argument == "-TP") {
            return "/Tp";
        }
        if (argument == "/TC" || argument == "-TC") {
            return "/Tc";
        }
    }
    const std::string extension = file.extension().string();
    return (extension == ".c" || extension == ".C") ? "/Tc" : "/Tp";
}

} // namespace

bool is_cl_driver(const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        return false;
    }
    if (std::ranges::find(arguments, "--driver-mode=cl") != arguments.end()) {
        return true;
    }
    const std::string_view program = file_stem_of_program(arguments.front());
    return program == "cl" || program == "CL" || program == "clang-cl";
}

std::vector<std::string> portable_arguments(const CompileCommand& command,
                                            const std::vector<std::string>& extra_options) {
    const std::vector<std::string>& original = command.arguments;
    const bool cl = is_cl_driver(original);
    const auto flags =
        cl ? std::vector<std::string_view>(cl_path_flags.begin(), cl_path_flags.end())
           : std::vector<std::string_view>(gcc_path_flags.begin(), gcc_path_flags.end());

    std::vector<std::string> arguments;
    arguments.reserve(original.size() + extra_options.size() + 3);
    if (!original.empty()) {
        arguments.push_back(original.front());
    }
    if (!cl) {
        arguments.push_back("-working-directory=" + command.directory.string());
    }
    for (std::size_t i = 1; i < original.size(); ++i) {
        const std::string& argument = original.at(i);
        // Compare cl options in their "-" spelling; the prefix length is the same.
        std::string spelled = argument;
        if (cl && spelled.starts_with('/')) {
            spelled.front() = '-';
        }
        const auto flag = std::ranges::find_if(
            flags, [&spelled](std::string_view name) { return spelled.starts_with(name); });
        if (flag != flags.end()) {
            if (spelled.size() == flag->size()) {
                arguments.push_back(argument);
                if (i + 1 < original.size()) {
                    arguments.push_back(absolute(original.at(++i), command.directory));
                }
            } else {
                arguments.push_back(
                    argument.substr(0, flag->size()) +
                    absolute(std::string_view{argument}.substr(flag->size()), command.directory));
            }
            continue;
        }
        // The source file is moved to the end, below.
        if (!argument.empty() &&
            (command.directory / fs::path{argument}).lexically_normal() == command.file) {
            continue;
        }
        arguments.push_back(argument);
    }
    arguments.insert(arguments.end(), extra_options.begin(), extra_options.end());
    // The source goes last, marked so that no driver mistakes it for an
    // option: on Linux, clang-cl reads "/w/a.cpp" as the /w option ("no input
    // files") and "/Users/x/a.cpp" as /U (both measured). GCC mode takes "--". libclang rejects
    // "--" in clang-cl mode (CXError_ASTReadError, LLVM 22.1.3), so there the file is named with
    // /Tp or /Tc, which also states its language.
    if (cl) {
        arguments.push_back(cl_source_marker(original, command.file));
    } else {
        arguments.emplace_back("--");
    }
    arguments.push_back(command.file.string());
    return arguments;
}

} // namespace tezcatl::parse
