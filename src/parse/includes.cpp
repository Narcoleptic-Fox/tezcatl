#include "parse/includes.hpp"

#include "parse/clang_string.hpp"

#include <algorithm>
#include <stdexcept>

namespace tezcatl::parse {

namespace fs = std::filesystem;

namespace {

struct IncludeContext {
    const fs::path* directory = nullptr; ///< for resolving relative file names
    const FileFilter* include_file = nullptr;
    std::vector<IncludeEdge>* found = nullptr;
};

fs::path absolute_name(CXFile file, const fs::path& directory) {
    return (directory / fs::path{ClangString{clang_getFileName(file)}.view()}).lexically_normal();
}

// Inclusion directives are preprocessing entities, which libclang lists as
// children of the translation unit itself, so this visit never descends.
CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData data) {
    if (clang_getCursorKind(cursor) != CXCursor_InclusionDirective) {
        return CXChildVisit_Continue;
    }
    const auto& context = *static_cast<IncludeContext*>(data);
    const CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile includer = nullptr;
    clang_getExpansionLocation(location, &includer, nullptr, nullptr, nullptr);
    const CXFile included = clang_getIncludedFile(cursor);
    if (includer == nullptr || included == nullptr ||
        clang_Location_isInSystemHeader(location) != 0) {
        return CXChildVisit_Continue;
    }
    IncludeEdge edge{.from = absolute_name(includer, *context.directory),
                     .to = absolute_name(included, *context.directory)};
    if ((*context.include_file)(edge.from) && (*context.include_file)(edge.to)) {
        context.found->push_back(std::move(edge));
    }
    return CXChildVisit_Continue;
}

} // namespace

std::vector<IncludeEdge> find_includes(const ParsedUnit& parsed, const FileFilter& include_file) {
    if (!parsed.unit) {
        throw std::invalid_argument("find_includes: " + parsed.file.string() +
                                    " has no translation unit");
    }
    std::vector<IncludeEdge> found;
    IncludeContext context{
        .directory = &parsed.directory, .include_file = &include_file, .found = &found};
    clang_visitChildren(clang_getTranslationUnitCursor(parsed.unit.get()), visit, &context);
    std::ranges::sort(found);
    const auto repeats = std::ranges::unique(found);
    found.erase(repeats.begin(), repeats.end());
    return found;
}

} // namespace tezcatl::parse
