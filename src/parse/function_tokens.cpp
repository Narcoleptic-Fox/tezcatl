#include "parse/function_tokens.hpp"

#include "parse/functions.hpp"

#include <algorithm>

namespace tezcatl::parse {

namespace {

/// A half-open range of byte offsets in one file.
struct OffsetRange {
    unsigned begin = 0;
    unsigned end = 0;

    [[nodiscard]] bool contains(unsigned offset) const noexcept {
        return offset >= begin && offset < end;
    }
};

// Where the functions directly nested in `function` are written. Deeper
// nesting lies inside these ranges, so the search stops at each one.
std::vector<OffsetRange> nested_functions(CXCursor function) {
    std::vector<OffsetRange> nested;
    clang_visitChildren(
        function,
        [](CXCursor child, CXCursor /*parent*/, CXClientData data) {
            if (!function_kind(clang_getCursorKind(child)).has_value()) {
                return CXChildVisit_Recurse;
            }
            const CXSourceRange extent = clang_getCursorExtent(child);
            static_cast<std::vector<OffsetRange>*>(data)->push_back(
                {.begin = file_offset(clang_getRangeStart(extent)),
                 .end = file_offset(clang_getRangeEnd(extent))});
            return CXChildVisit_Continue;
        },
        &nested);
    return nested;
}

} // namespace

FunctionTokens::FunctionTokens(CXTranslationUnit unit, CXCursor function)
    : tokens_(unit, clang_getCursorExtent(function)) {
    const std::vector<OffsetRange> nested = nested_functions(function);
    for (std::size_t index = 0; index < tokens_.size(); ++index) {
        const unsigned offset = tokens_.offset(index);
        const bool in_nested_function = std::ranges::any_of(
            nested, [offset](const OffsetRange& range) { return range.contains(offset); });
        if (!in_nested_function) {
            own_.push_back(index);
        }
    }
}

} // namespace tezcatl::parse
