#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::config {

/// Whether a file is test code or production code, by glob. The SOW's
/// "production vs test" split, and the defaults it suggests.
class FileRoles {
public:
    /// The default test globs: anything in a directory named test or tests,
    /// and files named *_test.* or test_*.*, at any depth.
    FileRoles();
    /// Test globs given explicitly (scan::glob_match patterns, relative to
    /// the project root), replacing the defaults.
    explicit FileRoles(std::vector<std::string> test_globs);

    /// Whether the file at `relative_path` (relative to the root, with '/'
    /// separators) is test code.
    [[nodiscard]] bool is_test(std::string_view relative_path) const;
    [[nodiscard]] const std::vector<std::string>& test_globs() const noexcept { return globs_; }

private:
    std::vector<std::string> globs_;
};

} // namespace tezcatl::config
