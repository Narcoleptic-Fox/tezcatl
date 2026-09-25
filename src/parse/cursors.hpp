#pragma once

#include <clang-c/Index.h>

#include <filesystem>
#include <string>

namespace tezcatl::parse {

/// Where a declaration is written.
struct SourcePosition {
    std::filesystem::path file; ///< absolute; empty if the cursor has no file
    unsigned line = 0;          ///< 1-based
    unsigned column = 0;        ///< 1-based, in bytes
    bool in_system_header = false;
};

/// Where the cursor's name is written, with the file made absolute against
/// `directory`; for code produced by a macro, where the macro is used.
[[nodiscard]] SourcePosition position_of(CXCursor cursor, const std::filesystem::path& directory);

/// The declaration's enclosing namespaces and classes, outermost first, each
/// followed by "::", or empty at global scope, e.g. "geo::Point::". Functions
/// that enclose it appear with their parameter types, anonymous namespaces as
/// "(anonymous namespace)", and extern "C" blocks not at all.
[[nodiscard]] std::string scope_qualifier(CXCursor declaration);

} // namespace tezcatl::parse
