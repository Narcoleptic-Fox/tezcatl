#include "config/modules.hpp"

#include "scan/glob.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace tezcatl::config {

namespace {

std::string_view trim(std::string_view text) {
    constexpr std::string_view whitespace = " \t\r";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(whitespace) - first + 1);
}

[[noreturn]] void fail(const std::filesystem::path& source, std::size_t line_number,
                       std::string_view problem) {
    throw std::runtime_error(source.generic_string() + ':' + std::to_string(line_number) + ": " +
                             std::string{problem} + " (expected MODULE = GLOB)");
}

} // namespace

ModuleMap ModuleMap::parse(std::string_view text, const std::filesystem::path& source) {
    ModuleMap map;
    std::size_t line_number = 0;
    while (!text.empty()) {
        const std::size_t end = text.find('\n');
        const std::string_view line = trim(text.substr(0, end));
        text.remove_prefix(end == std::string_view::npos ? text.size() : end + 1);
        ++line_number;
        if (line.empty() || line.starts_with('#')) {
            continue;
        }
        const std::size_t equals = line.find('=');
        if (equals == std::string_view::npos) {
            fail(source, line_number, "no '='");
        }
        const std::string_view module = trim(line.substr(0, equals));
        const std::string_view pattern = trim(line.substr(equals + 1));
        if (module.empty()) {
            fail(source, line_number, "no module name");
        }
        if (pattern.empty()) {
            fail(source, line_number, "no glob");
        }
        if (module == unassigned_module) {
            fail(source, line_number,
                 "the module name " + std::string{unassigned_module} + " is reserved");
        }
        map.rules_.push_back({.module = std::string{module}, .pattern = std::string{pattern}});
    }
    return map;
}

ModuleMap ModuleMap::load(const std::filesystem::path& file) {
    std::ifstream stream{file, std::ios::binary};
    if (!stream) {
        throw std::runtime_error("cannot read module map " + file.string());
    }
    std::ostringstream text;
    text << stream.rdbuf();
    return parse(text.str(), file);
}

std::string ModuleMap::module_of(std::string_view relative_path) const {
    const auto rule = std::ranges::find_if(rules_, [relative_path](const ModuleRule& candidate) {
        return scan::glob_match(candidate.pattern, relative_path);
    });
    return rule == rules_.end() ? std::string{unassigned_module} : rule->module;
}

} // namespace tezcatl::config
