#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <string_view>

namespace tezcatl::metrics {

/// A token's lexical category, as the lexer reports it.
enum class TokenCategory : std::uint8_t { punctuation, keyword, identifier, literal, comment };

/// How a token counts in Halstead's measures.
enum class HalsteadRole : std::uint8_t { operator_token, operand, not_counted };

/// The convention, stated in docs/metrics.md: keywords and punctuation are
/// operators, identifiers and literals are operands. A bracket pair is one
/// operator, counted at its opening `(`, `[` or `{`. The keywords that name
/// values rather than operations (`true`, `false`, `nullptr`, `this`) are
/// operands. Comments do not count.
[[nodiscard]] HalsteadRole halstead_role(TokenCategory category,
                                         std::string_view spelling) noexcept;

/// Halstead's counts for one function, and the measures derived from them.
struct Halstead {
    std::size_t distinct_operators = 0; ///< n1
    std::size_t distinct_operands = 0;  ///< n2
    std::size_t total_operators = 0;    ///< N1
    std::size_t total_operands = 0;     ///< N2

    /// V = N log2 n, with N = N1 + N2 and n = n1 + n2; 0 when n is 0 or 1.
    [[nodiscard]] double volume() const noexcept;
    /// D = (n1 / 2) (N2 / n2); 0 when there are no operands.
    [[nodiscard]] double difficulty() const noexcept;
    /// E = D V.
    [[nodiscard]] double effort() const noexcept;

    friend bool operator==(const Halstead&, const Halstead&) = default;
};

/// Counts tokens in the order they are written.
class HalsteadCounter {
public:
    void add(TokenCategory category, std::string_view spelling);
    [[nodiscard]] Halstead result() const;

private:
    std::set<std::string, std::less<>> operators_;
    std::set<std::string, std::less<>> operands_;
    std::size_t total_operators_ = 0;
    std::size_t total_operands_ = 0;
};

} // namespace tezcatl::metrics
