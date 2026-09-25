#pragma once

#include "config/file_roles.hpp"
#include "config/modules.hpp"

#include <filesystem>
#include <string>

namespace tezcatl::report {

/// How every table names and groups a file: by its path relative to the
/// project root, its module, and whether it is test or production code.
class FileNaming {
public:
    FileNaming(const std::filesystem::path& root, config::ModuleMap modules,
               config::FileRoles roles);

    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }
    /// `file` relative to the root, with '/' separators.
    [[nodiscard]] std::string relative(const std::filesystem::path& file) const;
    [[nodiscard]] std::string module_of(const std::filesystem::path& file) const;
    [[nodiscard]] bool is_test(const std::filesystem::path& file) const;
    [[nodiscard]] const config::ModuleMap& modules() const noexcept { return modules_; }
    [[nodiscard]] const config::FileRoles& roles() const noexcept { return roles_; }

private:
    std::filesystem::path root_;
    config::ModuleMap modules_;
    config::FileRoles roles_;
};

} // namespace tezcatl::report
