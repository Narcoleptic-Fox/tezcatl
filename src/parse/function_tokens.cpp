#include "parse/function_tokens.hpp"

#include "parse/clang_handles.hpp"
#include "parse/functions.hpp"

#include <algorithm>
#include <span>

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

// The regions of the function's file that the preprocessor skipped because
// an #if, #ifdef or #elif condition was false.
std::vector<OffsetRange> skipped_regions(CXTranslationUnit unit, CXCursor function) {
    CXFile file = nullptr;
    clang_getFileLocation(clang_getRangeStart(clang_getCursorExtent(function)), &file, nullptr,
                          nullptr, nullptr);
    if (file == nullptr) {
        return {};
    }
    const SourceRangeListHandle list{clang_getSkippedRanges(unit, file)};
    if (!list) {
        return {};
    }
    std::vector<OffsetRange> skipped;
    for (const CXSourceRange& range : std::span{list->ranges, list->count}) {
        skipped.push_back({.begin = file_offset(clang_getRangeStart(range)),
                           .end = file_offset(clang_getRangeEnd(range))});
    }
    return skipped;
}

bool is_directive(CXCursor cursor) {
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_PreprocessingDirective:
    case CXCursor_MacroDefinition:
    case CXCursor_InclusionDirective:
        return true;
    default:
        return false;
    }
}

bool inside_any(const std::vector<OffsetRange>& ranges, unsigned offset) {
    return std::ranges::any_of(
        ranges, [offset](const OffsetRange& range) { return range.contains(offset); });
}

} // namespace

FunctionTokens::FunctionTokens(CXTranslationUnit unit, CXCursor function)
    : tokens_(unit, clang_getCursorExtent(function)) {
    const std::vector<OffsetRange> nested = nested_functions(function);
    const std::vector<OffsetRange> skipped = skipped_regions(unit, function);
    for (std::size_t index = 0; index < tokens_.size(); ++index) {
        const unsigned offset = tokens_.offset(index);
        if (!inside_any(nested, offset) && !inside_any(skipped, offset) &&
            !is_directive(tokens_.cursor(index))) {
            own_.push_back(index);
        }
    }
}

} // namespace tezcatl::parse
