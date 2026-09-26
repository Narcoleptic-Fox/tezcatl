#pragma once

#include "metrics/halstead.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <compare>
#include <filesystem>
#include <string>
#include <vector>

namespace tezcatl::analysis {

/// A function definition with a body, as written in the source, and its
/// per-function metrics.
struct FunctionInfo {
    std::filesystem::path file; ///< where the definition is written
    unsigned line = 0;          ///< 1-based, of the function's name (for a lambda, its '[')
    unsigned column = 0;        ///< 1-based, in bytes
    parse::FunctionKind kind = parse::FunctionKind::function;
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

/// Every function definition parse::find_definitions finds in `parsed`,
/// measured: its cyclomatic complexity and Halstead figures, in source
/// order. This is where parsing and metrics meet, so that neither depends
/// on the other's caller: parse finds definitions, metrics measures tokens.
[[nodiscard]] std::vector<FunctionInfo> measure_functions(const parse::ParsedUnit& parsed,
                                                          const parse::FileFilter& include_file);

} // namespace tezcatl::analysis
