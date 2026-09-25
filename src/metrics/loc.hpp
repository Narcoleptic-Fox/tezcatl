#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace tezcatl::metrics {

/// What a physical line of C or C++ source contains.
enum class LineKind : std::uint8_t {
    blank,   ///< only whitespace (including whitespace inside a comment or string)
    comment, ///< comment text and no code
    code,    ///< at least one code character, with or without a comment
};

struct LocCounts {
    std::size_t physical = 0;
    std::size_t blank = 0;
    std::size_t comment = 0;
    std::size_t code = 0;

    LocCounts& operator+=(const LocCounts& other) noexcept;
    friend bool operator==(const LocCounts&, const LocCounts&) = default;
};

/// Classifies every physical line of `source`. The rules, published in
/// docs/metrics.md, follow the language's own translation phases:
/// - backslash-newline splices lines before comments are recognised, so a
///   `//` comment ending in a backslash continues onto the next line, except
///   inside raw string literals, where splicing is reverted;
/// - comment markers inside string, character and raw string literals are
///   not comments, and `'` after a digit is a digit separator (1'000);
/// - a line containing any code character is code, even if it also holds a
///   comment; a whitespace-only line is blank wherever it appears;
/// - a splice's backslash is not code, and `#if 0` regions count as code;
/// - "\r\n" and "\n" both end a line, a UTF-8 byte order mark is ignored,
///   and a final line without a newline still counts.
[[nodiscard]] std::vector<LineKind> classify_lines(std::string_view source);

/// Totals of classify_lines(source).
[[nodiscard]] LocCounts count_lines(std::string_view source);

} // namespace tezcatl::metrics
