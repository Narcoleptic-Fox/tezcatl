#include "metrics/halstead.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace tezcatl::metrics {

namespace {

bool is_closing_bracket(std::string_view spelling) noexcept {
    return spelling == ")" || spelling == "]" || spelling == "}";
}

bool is_value_keyword(std::string_view spelling) noexcept {
    constexpr std::array<std::string_view, 4> values{"true", "false", "nullptr", "this"};
    return std::ranges::find(values, spelling) != values.end();
}

TokenCategory category(CXTokenKind kind) noexcept {
    switch (kind) {
    case CXToken_Punctuation:
        return TokenCategory::punctuation;
    case CXToken_Keyword:
        return TokenCategory::keyword;
    case CXToken_Identifier:
        return TokenCategory::identifier;
    case CXToken_Literal:
        return TokenCategory::literal;
    case CXToken_Comment:
        return TokenCategory::comment;
    }
    return TokenCategory::comment;
}

} // namespace

HalsteadRole halstead_role(TokenCategory category, std::string_view spelling) noexcept {
    switch (category) {
    case TokenCategory::punctuation:
        return is_closing_bracket(spelling) ? HalsteadRole::not_counted
                                            : HalsteadRole::operator_token;
    case TokenCategory::keyword:
        return is_value_keyword(spelling) ? HalsteadRole::operand : HalsteadRole::operator_token;
    case TokenCategory::identifier:
    case TokenCategory::literal:
        return HalsteadRole::operand;
    case TokenCategory::comment:
        return HalsteadRole::not_counted;
    }
    return HalsteadRole::not_counted;
}

double Halstead::volume() const noexcept {
    const std::size_t vocabulary = distinct_operators + distinct_operands;
    if (vocabulary < 2) {
        return 0.0;
    }
    const auto length = static_cast<double>(total_operators + total_operands);
    return length * std::log2(static_cast<double>(vocabulary));
}

double Halstead::difficulty() const noexcept {
    if (distinct_operands == 0) {
        return 0.0;
    }
    // (n1 / 2) (N2 / n2), as one division so it is exact in the integers.
    return static_cast<double>(distinct_operators * total_operands) /
           static_cast<double>(2 * distinct_operands);
}

double Halstead::effort() const noexcept {
    return difficulty() * volume();
}

void HalsteadCounter::add(TokenCategory category, std::string_view spelling) {
    switch (halstead_role(category, spelling)) {
    case HalsteadRole::operator_token:
        operators_.emplace(spelling);
        ++total_operators_;
        break;
    case HalsteadRole::operand:
        operands_.emplace(spelling);
        ++total_operands_;
        break;
    case HalsteadRole::not_counted:
        break;
    }
}

Halstead measure_halstead(const parse::FunctionTokens& function) {
    HalsteadCounter counter;
    for (const std::size_t index : function.own()) {
        counter.add(category(function.all().kind(index)), function.all().spelling(index));
    }
    return counter.result();
}

Halstead HalsteadCounter::result() const {
    return {.distinct_operators = operators_.size(),
            .distinct_operands = operands_.size(),
            .total_operators = total_operators_,
            .total_operands = total_operands_};
}

} // namespace tezcatl::metrics
