#include "scan/paths.hpp"

#include <algorithm>
#include <string>

namespace tezcatl::scan {

namespace {

std::string comparable(const std::filesystem::path& path) {
    std::string text = path.lexically_normal().generic_string();
#ifdef _WIN32
    std::ranges::transform(text, text.begin(), [](char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    });
#endif
    while (text.size() > 1 && text.back() == '/') {
        text.pop_back();
    }
    return text;
}

} // namespace

bool is_within(const std::filesystem::path& path, const std::filesystem::path& directory) {
    const std::string candidate = comparable(path);
    const std::string base = comparable(directory);
    if (candidate == base) {
        return true;
    }
    // "/a/bc" is not within "/a/b": the match must end at a separator.
    return candidate.starts_with(base) && (base.ends_with('/') || candidate.at(base.size()) == '/');
}

} // namespace tezcatl::scan
