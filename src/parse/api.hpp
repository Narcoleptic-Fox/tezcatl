#pragma once

#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <compare>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::parse {

enum class ApiKind : std::uint8_t {
    function,   ///< a free function or function template
    method,     ///< a member function, constructor, destructor or conversion
    field,      ///< a data member, static or not
    variable,   ///< a variable at namespace scope
    type,       ///< a class, struct, union, enum or class template
    type_alias, ///< a typedef or using alias
};

enum class DocStyle : std::uint8_t {
    none,    ///< no comment attached
    plain,   ///< an ordinary comment
    doxygen, ///< a documentation comment: ///, //!, /** or /*!
};

[[nodiscard]] std::string_view to_string(ApiKind kind) noexcept;
[[nodiscard]] std::string_view to_string(DocStyle style) noexcept;

/// A public declaration in one of the project's headers, and the comment
/// attached to it.
struct ApiEntity {
    std::filesystem::path file; ///< where the declaration is written
    unsigned line = 0;          ///< 1-based, of its name
    unsigned column = 0;        ///< 1-based, in bytes
    ApiKind kind = ApiKind::function;
    std::string name; ///< qualified; functions carry their parameter types
    DocStyle documentation = DocStyle::none;

    /// Ordered by location.
    friend std::strong_ordering operator<=>(const ApiEntity& a, const ApiEntity& b) {
        if (const auto order = a.file <=> b.file; order != 0) {
            return order;
        }
        if (const auto order = a.line <=> b.line; order != 0) {
            return order;
        }
        return a.column <=> b.column;
    }
    friend bool operator==(const ApiEntity& a, const ApiEntity& b) { return (a <=> b) == 0; }
};

/// Whether `file` is a header, judged by its extension: .h, .hh, .hpp,
/// .hxx, .h++, .inl, .ipp, .tpp or .tcc, ignoring case.
[[nodiscard]] bool is_header(const std::filesystem::path& file);

/// The public API declared in headers that pass `include_file`, as the unit
/// sees it: functions and variables with external linkage, public members
/// of public classes, named class, struct, union and enum definitions, and
/// type aliases. Defaulted and deleted functions, and redeclarations after
/// the first, are left out. docs/metrics.md has the full definition.
///
/// The unit must be parsed with -fparse-all-comments for plain comments to
/// be seen (Parser does). `parsed.unit` must not be null.
[[nodiscard]] std::vector<ApiEntity> find_api(const ParsedUnit& parsed,
                                              const FileFilter& include_file);

} // namespace tezcatl::parse
