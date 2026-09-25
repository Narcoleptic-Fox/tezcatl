#include "report/naming.hpp"

#include <utility>

namespace tezcatl::report {

namespace fs = std::filesystem;

FileNaming::FileNaming(const fs::path& root, config::ModuleMap modules, config::FileRoles roles)
    : root_(fs::absolute(root).lexically_normal()), modules_(std::move(modules)),
      roles_(std::move(roles)) {}

std::string FileNaming::relative(const fs::path& file) const {
    return file.lexically_relative(root_).generic_string();
}

std::string FileNaming::module_of(const fs::path& file) const {
    return modules_.module_of(relative(file));
}

bool FileNaming::is_test(const fs::path& file) const {
    return roles_.is_test(relative(file));
}

} // namespace tezcatl::report
