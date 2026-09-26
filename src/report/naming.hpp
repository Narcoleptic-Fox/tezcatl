#pragma once

#include "config/file_roles.hpp"
#include "config/modules.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

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
    /// "test" or "production": the role as every table and the JSON name it.
    [[nodiscard]] std::string_view role_of(const std::filesystem::path& file) const;
    [[nodiscard]] const config::ModuleMap& modules() const noexcept { return modules_; }
    [[nodiscard]] const config::FileRoles& roles() const noexcept { return roles_; }

private:
    /// What every table asks about a file, worked out once per file: a
    /// report asks for each function, declaration and row, and matching a
    /// module's globs each time was most of a report's time after parsing.
    struct Names {
        std::string relative;
        std::string module;
        bool test = false;
    };
    /// Shared by copies, which name files identically; guarded, so naming a
    /// file is safe from any thread.
    struct Cache {
        std::mutex mutex;
        std::unordered_map<std::filesystem::path::string_type, Names> by_path;
    };
    [[nodiscard]] const Names& names_of(const std::filesystem::path& file) const;

    std::filesystem::path root_;
    config::ModuleMap modules_;
    config::FileRoles roles_;
    std::shared_ptr<Cache> cache_ = std::make_shared<Cache>();
};

} // namespace tezcatl::report
