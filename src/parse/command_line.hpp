#pragma once

#include "parse/compilation_database.hpp"

#include <string>
#include <vector>

namespace tezcatl::parse {

/// True if the command runs the MSVC-compatible driver (cl.exe or clang-cl),
/// whose options may begin with '/' and whose flag set differs from GCC's.
[[nodiscard]] bool is_cl_driver(const std::vector<std::string>& arguments);

/// The command's arguments, ready for libclang to run from any directory.
///
/// libclang has no working-directory parameter, and the fallback flag is not
/// dependable: in clang-cl mode "-working-directory" is silently ignored, and
/// even "/clang:-working-directory=" leaves relative /I paths unresolved
/// (measured with LLVM 22.1.3). So the paths that decide what gets parsed are
/// made absolute here, against the command's directory: the source file, and
/// the operand of each include-path flag of the command's driver, in both the
/// joined ("-Iinc") and separate ("-I inc") forms. For GCC-style commands,
/// "-working-directory=" is added as well and resolves any other relative path;
/// clang-cl has no dependable equivalent, so there a relative path behind an
/// unlisted flag fails to resolve and is reported as a parse error.
///
/// `extra_options` are placed after the command's own options, so they take
/// precedence, and the source file comes last, after "--" (GCC mode) or
/// "/Tp" or "/Tc" (clang-cl mode), so that no driver can mistake an absolute
/// path for an option.
[[nodiscard]] std::vector<std::string>
portable_arguments(const CompileCommand& command, const std::vector<std::string>& extra_options);

} // namespace tezcatl::parse
