#include "metrics/halstead.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string_view>
#include <utility>
#include <vector>

using namespace tezcatl::metrics;

namespace {

using Token = std::pair<TokenCategory, std::string_view>;
constexpr auto P = TokenCategory::punctuation;
constexpr auto K = TokenCategory::keyword;
constexpr auto I = TokenCategory::identifier;
constexpr auto L = TokenCategory::literal;

Halstead count(const std::vector<Token>& tokens) {
    HalsteadCounter counter;
    for (const auto& [category, spelling] : tokens) {
        counter.add(category, spelling);
    }
    return counter.result();
}

// Measures to three decimals, as integers, so a hand-computed value can be
// compared exactly.
constexpr double thousand = 1000;

long thousandths(double value) {
    return std::lround(value * thousand);
}

} // namespace

TEST_CASE("operators, operands and the tokens that do not count", "[halstead]") {
    CHECK(halstead_role(P, "(") == HalsteadRole::operator_token);
    CHECK(halstead_role(P, ")") == HalsteadRole::not_counted);
    CHECK(halstead_role(P, "}") == HalsteadRole::not_counted);
    CHECK(halstead_role(P, "::") == HalsteadRole::operator_token);
    CHECK(halstead_role(K, "return") == HalsteadRole::operator_token);
    CHECK(halstead_role(K, "int") == HalsteadRole::operator_token);
    CHECK(halstead_role(K, "nullptr") == HalsteadRole::operand);
    CHECK(halstead_role(K, "this") == HalsteadRole::operand);
    CHECK(halstead_role(I, "value") == HalsteadRole::operand);
    CHECK(halstead_role(L, "42") == HalsteadRole::operand);
    CHECK(halstead_role(TokenCategory::comment, "// x") == HalsteadRole::not_counted);
}

TEST_CASE("Halstead measures of int add(int a, int b) { return a + b; }", "[halstead]") {
    // By hand. Operators: int ( int , int { return + ; is N1 = 9, of which
    // int ( , { return + ; are distinct, n1 = 7. Operands: add a b a b is
    // N2 = 5, n2 = 3. So n = 10, N = 14: V = 14 log2 10 = 46.507,
    // D = 7/2 * 5/3 = 5.833, E = D V = 271.291.
    const Halstead h = count({{K, "int"},
                              {I, "add"},
                              {P, "("},
                              {K, "int"},
                              {I, "a"},
                              {P, ","},
                              {K, "int"},
                              {I, "b"},
                              {P, ")"},
                              {P, "{"},
                              {K, "return"},
                              {I, "a"},
                              {P, "+"},
                              {I, "b"},
                              {P, ";"},
                              {P, "}"}});
    CHECK(h == Halstead{.distinct_operators = 7,
                        .distinct_operands = 3,
                        .total_operators = 9,
                        .total_operands = 5});
    CHECK(thousandths(h.volume()) == 46507);
    CHECK(thousandths(h.difficulty()) == 5833);
    CHECK(thousandths(h.effort()) == 271291);
}

TEST_CASE("Halstead measures with literals and a value keyword", "[halstead]") {
    // bool is_empty(const char* text) { return text == nullptr || text[0] == '\0'; }
    // By hand. Operators: bool ( const char * { return == || [ == ; is
    // N1 = 12, n1 = 11 (== twice). Operands: is_empty text text nullptr
    // text 0 '\0' is N2 = 7, n2 = 5. n = 16, N = 19: V = 19 * 4 = 76,
    // D = 11/2 * 7/5 = 7.7, E = 585.2.
    const Halstead h =
        count({{K, "bool"},    {I, "is_empty"}, {P, "("},    {K, "const"},  {K, "char"}, {P, "*"},
               {I, "text"},    {P, ")"},        {P, "{"},    {K, "return"}, {I, "text"}, {P, "=="},
               {K, "nullptr"}, {P, "||"},       {I, "text"}, {P, "["},      {L, "0"},    {P, "]"},
               {P, "=="},      {L, "'\\0'"},    {P, ";"},    {P, "}"}});
    CHECK(h == Halstead{.distinct_operators = 11,
                        .distinct_operands = 5,
                        .total_operators = 12,
                        .total_operands = 7});
    CHECK(thousandths(h.volume()) == 76000);
    CHECK(thousandths(h.difficulty()) == 7700);
    CHECK(thousandths(h.effort()) == 585200);
}

TEST_CASE("Halstead measures with too few tokens are zero, not undefined", "[halstead]") {
    CHECK(thousandths(count({}).volume()) == 0);
    CHECK(thousandths(count({}).difficulty()) == 0);
    // One distinct token: log2 1 = 0.
    CHECK(thousandths(count({{P, ";"}, {P, ";"}}).volume()) == 0);
    // No operands: difficulty is 0 rather than a division by zero.
    CHECK(thousandths(count({{P, ";"}, {P, "{"}}).difficulty()) == 0);
}
