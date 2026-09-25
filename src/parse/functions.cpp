#include "parse/functions.hpp"

#include "metrics/complexity.hpp"
#include "parse/clang_string.hpp"
#include "parse/function_tokens.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <stdexcept>

namespace tezcatl::parse {

namespace fs = std::filesystem;

std::string_view to_string(FunctionKind kind) noexcept {
    switch (kind) {
    case FunctionKind::function:
        return "function";
    case FunctionKind::method:
        return "method";
    case FunctionKind::constructor:
        return "constructor";
    case FunctionKind::destructor:
        return "destructor";
    case FunctionKind::conversion:
        return "conversion";
    case FunctionKind::function_template:
        return "function_template";
    case FunctionKind::lambda:
        return "lambda";
    }
    return "unknown";
}

std::optional<FunctionKind> function_kind(CXCursorKind kind) noexcept {
    switch (kind) {
    case CXCursor_FunctionDecl:
        return FunctionKind::function;
    case CXCursor_CXXMethod:
        return FunctionKind::method;
    case CXCursor_Constructor:
        return FunctionKind::constructor;
    case CXCursor_Destructor:
        return FunctionKind::destructor;
    case CXCursor_ConversionFunction:
        return FunctionKind::conversion;
    case CXCursor_FunctionTemplate:
        return FunctionKind::function_template;
    case CXCursor_LambdaExpr:
        return FunctionKind::lambda;
    default:
        return std::nullopt;
    }
}

namespace {

// A body is a compound statement, or a try block for a function-try-block,
// directly under the function. Declarations and deleted functions have none.
// A defaulted function has one only when clang writes it itself (defined
// out of line, or used), which is not code written in the project, so it is
// excluded by name rather than by body (measured: 63 in Catch2 v3.16.0).
bool has_body(CXCursor cursor) {
    if (clang_CXXMethod_isDefaulted(cursor) != 0) {
        return false;
    }
    bool found = false;
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor /*parent*/, CXClientData data) {
            const CXCursorKind kind = clang_getCursorKind(child);
            if (kind == CXCursor_CompoundStmt || kind == CXCursor_CXXTryStmt) {
                *static_cast<bool*>(data) = true;
                return CXChildVisit_Break;
            }
            return CXChildVisit_Continue;
        },
        &found);
    return found;
}

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

// Enclosing namespaces and classes outermost first, joined with "::" and
// ending in "::", or empty at global scope. extern "C" blocks are not scopes.
std::string qualifier(CXCursor declaration) {
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

struct Location {
    fs::path file;
    unsigned line = 0;
    unsigned column = 0;
    bool in_system_header = false;
};

// Where the cursor's name is written; for code produced by a macro, where
// the macro is used.
Location location_of(CXCursor cursor, const fs::path& directory) {
    const CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    Location result;
    clang_getExpansionLocation(location, &file, &result.line, &result.column, nullptr);
    if (file != nullptr) {
        result.file =
            (directory / fs::path{ClangString{clang_getFileName(file)}.view()}).lexically_normal();
    }
    result.in_system_header = clang_Location_isInSystemHeader(location) != 0;
    return result;
}

struct VisitContext {
    CXTranslationUnit unit = nullptr;
    const fs::path* directory = nullptr; ///< for resolving relative file names
    const FileFilter* include_file = nullptr;
    std::vector<FunctionInfo>* found = nullptr;
    std::string enclosing_function; ///< qualified name, empty outside any function
};

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData data) {
    auto& context = *static_cast<VisitContext*>(data);
    const std::optional<FunctionKind> kind = function_kind(clang_getCursorKind(cursor));
    if (!kind.has_value()) {
        return CXChildVisit_Recurse;
    }
    const Location location = location_of(cursor, *context.directory);
    if (location.in_system_header) {
        return CXChildVisit_Continue;
    }

    std::string name;
    if (*kind == FunctionKind::lambda) {
        name = context.enclosing_function.empty() ? "(lambda)"
                                                  : context.enclosing_function + "::(lambda)";
    } else {
        name =
            qualifier(cursor) + std::string{ClangString{clang_getCursorDisplayName(cursor)}.view()};
    }

    if (has_body(cursor) && !location.file.empty() && (*context.include_file)(location.file)) {
        const FunctionTokens tokens{context.unit, cursor};
        context.found->push_back({.file = location.file,
                                  .line = location.line,
                                  .column = location.column,
                                  .kind = *kind,
                                  .name = name,
                                  .complexity = metrics::cyclomatic_complexity(tokens),
                                  .halstead = metrics::measure_halstead(tokens)});
    }

    // Visit the body with this function as the enclosing one, so lambdas
    // inside it are named after it.
    VisitContext inner{.unit = context.unit,
                       .directory = context.directory,
                       .include_file = context.include_file,
                       .found = context.found,
                       .enclosing_function = std::move(name)};
    clang_visitChildren(cursor, visit, &inner);
    return CXChildVisit_Continue;
}

} // namespace

std::vector<FunctionInfo> find_functions(const ParsedUnit& parsed, const FileFilter& include_file) {
    if (!parsed.unit) {
        throw std::invalid_argument("find_functions: " + parsed.file.string() +
                                    " has no translation unit");
    }
    std::vector<FunctionInfo> found;
    VisitContext context{.unit = parsed.unit.get(),
                         .directory = &parsed.directory,
                         .include_file = &include_file,
                         .found = &found,
                         .enclosing_function = {}};
    clang_visitChildren(clang_getTranslationUnitCursor(parsed.unit.get()), visit, &context);
    std::ranges::sort(found);
    return found;
}

void merge_duplicates(std::vector<FunctionInfo>& functions) {
    std::ranges::sort(functions);
    const auto repeats = std::ranges::unique(functions);
    functions.erase(repeats.begin(), repeats.end());
}

} // namespace tezcatl::parse
