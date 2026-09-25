#include "coverage/path_map.hpp"

#include "scan/paths.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>

namespace tezcatl::coverage {

namespace fs = std::filesystem;

PathMapping parse_path_mapping(std::string_view text) {
    const std::size_t equals = text.find('=');
    if (equals == std::string_view::npos || equals == 0 || equals + 1 == text.size()) {
        throw std::invalid_argument("a path mapping is FROM=TO, got '" + std::string{text} + "'");
    }
    return {.from = fs::path{text.substr(0, equals)}, .to = fs::path{text.substr(equals + 1)}};
}

fs::path map_path(const fs::path& recorded, const std::vector<PathMapping>& mappings) {
    for (const PathMapping& mapping : mappings) {
        if (!scan::is_within(recorded, mapping.from)) {
            continue;
        }
        // is_within compares normalised generic spellings (ignoring case on
        // Windows), so the remainder is taken from the same spelling.
        const std::string whole = recorded.lexically_normal().generic_string();
        std::string from = mapping.from.lexically_normal().generic_string();
        while (from.size() > 1 && from.ends_with('/')) {
            from.pop_back();
        }
        std::string rest = whole.substr(from.size());
        while (rest.starts_with('/')) {
            rest.erase(0, 1);
        }
        return (mapping.to / rest).lexically_normal();
    }
    return recorded;
}

} // namespace tezcatl::coverage
