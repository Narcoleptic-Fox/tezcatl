#include "parse/cursors.hpp"

#include "parse/clang_string.hpp"
#include "parse/functions.hpp"

#include <ranges>
#include <vector>

namespace tezcatl::parse {

namespace fs = std::filesystem;

namespace {

std::string scope_name(CXCursor scope) {
    const CXCursorKind kind = clang_getCursorKind(scope);
    if (kind == CXCursor_Namespace && clang_Cursor_isAnonymous(scope) != 0) {
        return "(anonymous namespace)";
    }
    if (clang_isDeclaration(kind) != 0 && function_kind(kind).has_value()) {
        return std::string{ClangString{clang_getCursorDisplayName(scope)}.view()};
    }
    const ClangString spelling{clang_getCursorSpelling(scope)};
    return spelling.view().empty() ? "(anonymous)" : std::string{spelling.view()};
}

} // namespace

std::string scope_qualifier(CXCursor declaration) {
    std::vector<std::string> scopes;
    for (CXCursor parent = clang_getCursorSemanticParent(declaration);
         clang_Cursor_isNull(parent) == 0 && clang_isInvalid(clang_getCursorKind(parent)) == 0 &&
         clang_getCursorKind(parent) != CXCursor_TranslationUnit;
         parent = clang_getCursorSemanticParent(parent)) {
        if (clang_getCursorKind(parent) != CXCursor_LinkageSpec) {
            scopes.push_back(scope_name(parent));
        }
    }
    std::string result;
    for (const std::string& scope : std::views::reverse(scopes)) {
        result += scope;
        result += "::";
    }
    return result;
}

SourcePosition position_of(CXCursor cursor, const fs::path& directory) {
    const CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    SourcePosition result;
    clang_getExpansionLocation(location, &file, &result.line, &result.column, nullptr);
    if (file != nullptr) {
        result.file =
            (directory / fs::path{ClangString{clang_getFileName(file)}.view()}).lexically_normal();
    }
    result.in_system_header = clang_Location_isInSystemHeader(location) != 0;
    return result;
}

} // namespace tezcatl::parse
