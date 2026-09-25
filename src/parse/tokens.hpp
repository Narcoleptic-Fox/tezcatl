#pragma once

#include <clang-c/Index.h>

#include <cstddef>
#include <string>
#include <vector>

namespace tezcatl::parse {

/// The tokens of a source range as written, each annotated with the most
/// specific cursor that covers it. Macro invocations are not expanded: a
/// macro's name and arguments are tokens here, its body is not. A range
/// produced by a macro body is lexed where the macro is used, so it holds
/// the macro's name at most.
///
/// Arguments are annotated with the AST nodes they expand into only when the
/// unit was parsed with the detailed preprocessing record (Parser does).
class TokenList {
public:
    TokenList(CXTranslationUnit unit, CXSourceRange range);
    ~TokenList();

    TokenList(const TokenList&) = delete;
    TokenList& operator=(const TokenList&) = delete;
    TokenList(TokenList&&) = delete;
    TokenList& operator=(TokenList&&) = delete;

    [[nodiscard]] std::size_t size() const noexcept { return tokens_.size(); }
    [[nodiscard]] std::string spelling(std::size_t index) const;
    /// Punctuation, keyword, identifier, literal or comment.
    [[nodiscard]] CXTokenKind kind(std::size_t index) const {
        return clang_getTokenKind(tokens_.at(index));
    }
    /// The most specific cursor covering the token.
    [[nodiscard]] CXCursor cursor(std::size_t index) const { return cursors_.at(index); }
    /// Byte offset of the token's start in the file it is written in.
    [[nodiscard]] unsigned offset(std::size_t index) const;

private:
    CXTranslationUnit unit_;
    CXToken* owned_ = nullptr; ///< as returned by clang_tokenize, for disposal
    unsigned owned_count_ = 0;
    std::vector<CXToken> tokens_;
    std::vector<CXCursor> cursors_;
};

/// Byte offset of `location` in the file it is written in. For a location in
/// a macro argument that is where the argument is written; for one in a
/// macro body, where the macro is used.
[[nodiscard]] unsigned file_offset(CXSourceLocation location);

} // namespace tezcatl::parse
