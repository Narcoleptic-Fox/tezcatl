#pragma once

#include "parse/tokens.hpp"

#include <clang-c/Index.h>

#include <cstddef>
#include <vector>

namespace tezcatl::parse {

/// The tokens a function definition is written with, and which of them are
/// its own: not those of the functions nested in it (lambdas, member
/// functions of local classes), which are measured separately, and not those
/// the preprocessor consumed (directive lines, and code an #if removed), which
/// the compiler never sees. Every per-function metric reads the same own
/// tokens, so they agree on what belongs to a function.
class FunctionTokens {
public:
    FunctionTokens(CXTranslationUnit unit, CXCursor function);

    /// Every token of the definition, nested functions included, so that a
    /// token's neighbours can be inspected.
    [[nodiscard]] const TokenList& all() const noexcept { return tokens_; }
    /// Indices into all() of the function's own tokens, in source order.
    [[nodiscard]] const std::vector<std::size_t>& own() const noexcept { return own_; }

private:
    TokenList tokens_;
    std::vector<std::size_t> own_;
};

} // namespace tezcatl::parse
