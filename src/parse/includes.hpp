#pragma once

#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <compare>
#include <filesystem>
#include <vector>

namespace tezcatl::parse {

/// One `#include` directive, resolved: the file it is written in and the
/// file it names, both absolute.
struct IncludeEdge {
    std::filesystem::path from;
    std::filesystem::path to;

    friend std::strong_ordering operator<=>(const IncludeEdge& a, const IncludeEdge& b) {
        if (const auto order = a.from <=> b.from; order != 0) {
            return order;
        }
        return a.to <=> b.to;
    }
    friend bool operator==(const IncludeEdge& a, const IncludeEdge& b) { return (a <=> b) == 0; }
};

/// Every `#include` directive the preprocessor met in `parsed` whose
/// includer and included file both pass `include_file`, resolved through
/// the unit's own include paths. Directives inside a file that an include
/// guard or `#pragma once` kept from being entered again are recorded when
/// it was first entered, so no edge is lost to either. A directive that
/// names a file that was not found has no edge (the unit has an error for
/// it). Directives in system headers are left out. Edges repeat across
/// units; the caller merges them.
[[nodiscard]] std::vector<IncludeEdge> find_includes(const ParsedUnit& parsed,
                                                     const FileFilter& include_file);

} // namespace tezcatl::parse
