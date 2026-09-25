#pragma once

#include <filesystem>

namespace tezcatl::scan {

/// True if `path` is `directory` or lies beneath it, judged lexically after
/// normalisation. On Windows the comparison ignores case, as the file system
/// does; compilers do not always preserve the case a path was written in.
[[nodiscard]] bool is_within(const std::filesystem::path& path,
                             const std::filesystem::path& directory);

} // namespace tezcatl::scan
