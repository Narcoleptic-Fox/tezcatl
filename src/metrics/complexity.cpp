#include "metrics/complexity.hpp"

#include "parse/tokens.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace tezcatl::metrics {

namespace {

bool is_pack_expansion(const parse::TokenList& tokens, std::size_t index) {
    return tokens.spelling(index) == "...";
}

// The "&&" or "||" token at `index` is a logical operator, not an rvalue
// reference, a ref-qualifier, or a call to an overloaded operator.
bool is_logical_operator(const parse::TokenList& tokens, std::size_t index,
                         CXBinaryOperatorKind expected) {
    const CXCursor cursor = tokens.cursor(index);
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_BinaryOperator:
        return clang_getCursorBinaryOperatorKind(cursor) == expected;
    // In a template, when the operands depend on a template parameter and an
    // overloaded operator of that name is visible, which operator applies is
    // unknown until instantiation. As written it is a logical operator, so it
    // counts. A call to an overloaded operator, resolved or written by name
    // as operator&&(a, b), is annotated as a DeclRefExpr instead, and does not
    // count (measured with LLVM 22.1.3).
    case CXCursor_OverloadedDeclRef:
        return true;
    // A fold expression, "(pack && ...)" or "(... && pack)", has no exposed
    // cursor kind in libclang 22. It is one operator as written.
    case CXCursor_UnexposedExpr:
        return (index > 0 && is_pack_expansion(tokens, index - 1)) ||
               (index + 1 < tokens.size() && is_pack_expansion(tokens, index + 1));
    default:
        return false;
    }
}

// The token is a decision keyword or operator, and the AST node it belongs
// to is the construct it spells. The token alone is not enough ("&&" also
// spells an rvalue reference, "while" ends a do-while, and tokens in code
// the preprocessor disabled belong to no statement at all), and the AST
// alone is not enough (it also holds what macro bodies expanded to).
bool is_decision_point(const parse::TokenList& tokens, std::size_t index) {
    const std::string spelling = tokens.spelling(index);
    const CXCursor cursor = tokens.cursor(index);
    const CXCursorKind kind = clang_getCursorKind(cursor);
    if (spelling == "if") {
        return kind == CXCursor_IfStmt;
    }
    if (spelling == "for") {
        return kind == CXCursor_ForStmt || kind == CXCursor_CXXForRangeStmt;
    }
    if (spelling == "while") {
        return kind == CXCursor_WhileStmt;
    }
    if (spelling == "do") {
        return kind == CXCursor_DoStmt;
    }
    if (spelling == "case") {
        return kind == CXCursor_CaseStmt;
    }
    if (spelling == "catch") {
        return kind == CXCursor_CXXCatchStmt;
    }
    if (spelling == "&&" || spelling == "and") {
        return is_logical_operator(tokens, index, CXBinaryOperator_LAnd);
    }
    if (spelling == "||" || spelling == "or") {
        return is_logical_operator(tokens, index, CXBinaryOperator_LOr);
    }
    if (spelling == "?") {
        if (kind == CXCursor_ConditionalOperator) {
            return true;
        }
        // The GNU "a ?: b" has no cursor kind of its own in libclang 22; it
        // is the only expression whose own tokens are "?" then ":".
        return kind == CXCursor_UnexposedExpr && index + 1 < tokens.size() &&
               tokens.spelling(index + 1) == ":";
    }
    return false;
}

} // namespace

unsigned cyclomatic_complexity(const parse::FunctionTokens& function) {
    const auto decisions = std::ranges::count_if(function.own(), [&function](std::size_t index) {
        return is_decision_point(function.all(), index);
    });
    return 1 + static_cast<unsigned>(decisions);
}

} // namespace tezcatl::metrics
