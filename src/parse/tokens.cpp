#include "parse/tokens.hpp"

#include "parse/clang_string.hpp"

#include <span>

namespace tezcatl::parse {

namespace {

// `range` as a range of one file's text, mapped the way file_offset maps a
// location. Tokenizing a range that starts in a macro body would lex from
// the #define onwards, through everything written between it and the
// macro's use (measured: 23 unrelated decision points in one function).
// Null if the ends are in different files.
CXSourceRange written_range(CXTranslationUnit unit, CXSourceRange range) {
    CXFile begin_file = nullptr;
    CXFile end_file = nullptr;
    unsigned begin = 0;
    unsigned end = 0;
    clang_getFileLocation(clang_getRangeStart(range), &begin_file, nullptr, nullptr, &begin);
    clang_getFileLocation(clang_getRangeEnd(range), &end_file, nullptr, nullptr, &end);
    if (begin_file == nullptr || end_file == nullptr ||
        clang_File_isEqual(begin_file, end_file) == 0 || end < begin) {
        return clang_getNullRange();
    }
    return clang_getRange(clang_getLocationForOffset(unit, begin_file, begin),
                          clang_getLocationForOffset(unit, end_file, end));
}

} // namespace

TokenList::TokenList(CXTranslationUnit unit, CXSourceRange range) : unit_(unit) {
    const CXSourceRange written = written_range(unit_, range);
    if (clang_Range_isNull(written) != 0) {
        return;
    }
    clang_tokenize(unit_, written, &owned_, &owned_count_);
    const std::span<const CXToken> lexed{owned_, owned_count_};
    tokens_.assign(lexed.begin(), lexed.end());
    cursors_.resize(tokens_.size());
    if (!tokens_.empty()) {
        clang_annotateTokens(unit_, tokens_.data(), owned_count_, cursors_.data());
    }
}

TokenList::~TokenList() {
    if (owned_ != nullptr) {
        clang_disposeTokens(unit_, owned_, owned_count_);
    }
}

std::string TokenList::spelling(std::size_t index) const {
    return std::string{ClangString{clang_getTokenSpelling(unit_, tokens_.at(index))}.view()};
}

unsigned TokenList::offset(std::size_t index) const {
    return file_offset(clang_getTokenLocation(unit_, tokens_.at(index)));
}

unsigned file_offset(CXSourceLocation location) {
    unsigned offset = 0;
    clang_getFileLocation(location, nullptr, nullptr, nullptr, &offset);
    return offset;
}

} // namespace tezcatl::parse
