#include "report/naming.hpp"

#include <utility>

namespace tezcatl::report {

namespace fs = std::filesystem;

FileNaming::FileNaming(const fs::path& root, config::ModuleMap modules, config::FileRoles roles)
    : root_(fs::absolute(root).lexically_normal()), modules_(std::move(modules)),
      roles_(std::move(roles)) {}

const FileNaming::Names& FileNaming::names_of(const fs::path& file) const {
    const std::scoped_lock lock{cache_->mutex};
    const auto [entry, added] = cache_->by_path.try_emplace(file.native());
    if (added) {
        Names& names = entry->second;
        names.relative = file.lexically_relative(root_).generic_string();
        names.module = modules_.module_of(names.relative);
        names.test = roles_.is_test(names.relative);
    }
    // Elements of an unordered_map stay where they are as it grows.
    return entry->second;
}

std::string FileNaming::relative(const fs::path& file) const {
    return names_of(file).relative;
}

std::string FileNaming::module_of(const fs::path& file) const {
    return names_of(file).module;
}

bool FileNaming::is_test(const fs::path& file) const {
    return names_of(file).test;
}

std::string_view FileNaming::role_of(const fs::path& file) const {
    return is_test(file) ? "test" : "production";
}

} // namespace tezcatl::report
