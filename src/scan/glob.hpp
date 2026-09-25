#pragma once

#include <string_view>

namespace tezcatl::scan {

/// Matches a '/'-separated relative path against a glob pattern:
///   `*`   any run of characters within one path component (no '/')
///   `?`   exactly one character other than '/'
///   `**`  any run of characters including '/'; as a whole component,
///         `**/` also matches no directories at all, so `src/**/x.c`
///         matches `src/x.c`
/// Every other character matches itself, case-sensitively. The whole path
/// must match, not a prefix of it.
[[nodiscard]] bool glob_match(std::string_view pattern, std::string_view path);

} // namespace tezcatl::scan
