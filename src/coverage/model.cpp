#include "coverage/model.hpp"

#include <algorithm>

namespace tezcatl::coverage {

Counts& Counts::operator+=(const Counts& other) noexcept {
    lines += other.lines;
    lines_covered += other.lines_covered;
    branches += other.branches;
    branches_covered += other.branches_covered;
    functions += other.functions;
    functions_covered += other.functions_covered;
    return *this;
}

std::filesystem::path resolve_recorded_path(const std::filesystem::path& recorded,
                                            const std::filesystem::path& base) {
    return (recorded.has_root_directory() ? recorded : base / recorded).lexically_normal();
}

void FileRecord::add_line(unsigned line, std::uint64_t hits) {
    lines_[line] += hits;
}

void FileRecord::add_branch(unsigned line, const std::string& branch, std::uint64_t hits) {
    branches_[{line, branch}] += hits;
}

void FileRecord::add_function(const std::string& name, std::uint64_t hits) {
    functions_[name] += hits;
}

void FileRecord::add_totals(const Counts& totals) noexcept {
    totals_ += totals;
}

Counts FileRecord::counts() const {
    const auto hit = [](const auto& entry) {
        return entry.second > 0;
    };
    Counts result = totals_;
    result.lines += lines_.size();
    result.lines_covered += static_cast<std::size_t>(std::ranges::count_if(lines_, hit));
    result.branches += branches_.size();
    result.branches_covered += static_cast<std::size_t>(std::ranges::count_if(branches_, hit));
    result.functions += functions_.size();
    result.functions_covered += static_cast<std::size_t>(std::ranges::count_if(functions_, hit));
    return result;
}

} // namespace tezcatl::coverage
