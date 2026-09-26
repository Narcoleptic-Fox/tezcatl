#pragma once

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

/// A function definition with a body, as written in the source: where it
/// is, what kind, and its name. Measuring it is analysis's job.
struct FunctionDefinition {
    std::filesystem::path file; ///< where the definition is written
    unsigned line = 0;          ///< 1-based, of the function's name (for a lambda, its '[')
    unsigned column = 0;        ///< 1-based, in bytes
    FunctionKind kind = FunctionKind::function;
    /// Enclosing namespaces and classes, then the name with its parameter
    /// types, e.g. "geo::Point::sum()". A lambda is named after the function
    /// that contains it: "use_lambda()::(lambda)".
    std::string name;
    CXCursor cursor = clang_getNullCursor(); ///< valid while its translation unit is
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
[[nodiscard]] std::vector<FunctionDefinition> find_definitions(const ParsedUnit& parsed,
                                                               const FileFilter& include_file);

} // namespace tezcatl::parse
