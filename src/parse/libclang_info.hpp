#pragma once

#include <string>

namespace tezcatl::parse {

/// The version string of the libclang loaded at run time, such as
/// "clang version 22.1.3". Read at run time rather than from the headers
/// because the shared library can differ from the one Tezcatl was built
/// against, and the report must state the parser that actually ran.
[[nodiscard]] std::string libclang_version();

} // namespace tezcatl::parse
