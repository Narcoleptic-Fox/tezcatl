#include "scan/source_files.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string_view>

namespace tezcatl::scan {

namespace fs = std::filesystem;

bool is_source_file(const fs::path& path) {
    constexpr std::array<std::string_view, 15> extensions{".c",   ".h",   ".cc",  ".cp",  ".cpp",
                                                          ".cxx", ".c++", ".hh",  ".hpp", ".hxx",
                                                          ".h++", ".inl", ".ipp", ".tpp", ".cppm"};
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    });
    return std::ranges::find(extensions, extension) != extensions.end();
}

std::vector<fs::path> find_source_files(const std::vector<fs::path>& inputs) {
    std::vector<fs::path> files;
    for (const fs::path& input : inputs) {
        if (fs::is_regular_file(input)) {
            files.push_back(input.lexically_normal());
        } else if (fs::is_directory(input)) {
            for (const auto& entry : fs::recursive_directory_iterator(input)) {
                if (entry.is_regular_file() && is_source_file(entry.path())) {
                    files.push_back(entry.path().lexically_normal());
                }
            }
        } else {
            throw std::runtime_error("no such file or directory: " + input.string());
        }
    }
    std::ranges::sort(files);
    const auto duplicates = std::ranges::unique(files);
    files.erase(duplicates.begin(), duplicates.end());
    return files;
}

std::string read_file(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        throw std::runtime_error("cannot open " + path.string());
    }
    std::string contents{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    if (stream.bad()) {
        throw std::runtime_error("error while reading " + path.string());
    }
    return contents;
}

} // namespace tezcatl::scan
