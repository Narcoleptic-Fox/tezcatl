#include "config/file_roles.hpp"

#include "scan/glob.hpp"

#include <algorithm>
#include <utility>

namespace tezcatl::config {

FileRoles::FileRoles() : globs_{"**/test/**", "**/tests/**", "**/*_test.*", "**/test_*.*"} {}

FileRoles::FileRoles(std::vector<std::string> test_globs) : globs_(std::move(test_globs)) {}

bool FileRoles::is_test(std::string_view relative_path) const {
    return std::ranges::any_of(globs_, [relative_path](const std::string& glob) {
        return scan::glob_match(glob, relative_path);
    });
}

} // namespace tezcatl::config
