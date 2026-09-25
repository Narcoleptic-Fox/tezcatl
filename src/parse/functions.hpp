#pragma once

#include "metrics/halstead.hpp"
#include "parse/translation_unit.hpp"

#include <compare>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::parse {

enum class FunctionKind : std::uint8_t {
    function,
    method,
    constructor,
    destructor,
    conversion,
    function_template,
    lambda,
};

[[nodiscard]] std::string_view to_string(FunctionKind kind) noexcept;

/// The kind of function a cursor of this kind declares, or nothing if it
/// does not declare one.
[[nodiscard]] std::optional<FunctionKind> function_kind(CXCursorKind kind) noexcept;

/// A function definition with a body, as written in the source, and its
/// per-function metrics.
struct FunctionInfo {
    std::filesystem::path file; ///< where the definition is written
    unsigned line = 0;          ///< 1-based, of the function's name (for a lambda, its '[')
    unsigned column = 0;        ///< 1-based, in bytes
    FunctionKind kind = FunctionKind::function;
    /// Enclosing namespaces and classes, then the name with its parameter
    /// types, e.g. "geo::Point::sum()". A lambda is named after the function
    /// that contains it: "use_lambda()::(lambda)".
    std::string name;
    /// McCabe cyclomatic complexity, as defined in docs/metrics.md.
    unsigned complexity = 1;
    /// Halstead's counts and measures, as defined in docs/metrics.md.
    metrics::Halstead halstead;

    /// Ordered by location, so a sorted list reads top to bottom per file.
    friend std::strong_ordering operator<=>(const FunctionInfo& a, const FunctionInfo& b) {
        if (const auto order = a.file <=> b.file; order != 0) {
            return order;
        }
        if (const auto order = a.line <=> b.line; order != 0) {
            return order;
        }
        return a.column <=> b.column;
    }
    friend bool operator==(const FunctionInfo& a, const FunctionInfo& b) { return (a <=> b) == 0; }
};

/// Decides whether functions written in a file belong in the report.
using FileFilter = std::function<bool(const std::filesystem::path&)>;

/// Every function definition with a body in `parsed` whose file passes
/// `include_file`, in source order. Declarations, defaulted and deleted
/// functions have no body and are left out, and so is anything in a system
/// header. Functions defined in headers appear once per translation unit
/// that includes them; the caller merges units.
/// File names are made absolute against the unit's working directory before
/// filtering. `parsed.unit` must not be null.
[[nodiscard]] std::vector<FunctionInfo> find_functions(const ParsedUnit& parsed,
                                                       const FileFilter& include_file);

/// Sorts `functions` by location and removes the repeats that arise when a
/// header's functions are found once per translation unit including it.
void merge_duplicates(std::vector<FunctionInfo>& functions);

} // namespace tezcatl::parse
