#include "parse/functions.hpp"

#include "parse/clang_string.hpp"
#include "parse/cursors.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <tuple>

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

struct VisitContext {
    CXTranslationUnit unit = nullptr;
    const fs::path* directory = nullptr; ///< for resolving relative file names
    const FileFilter* include_file = nullptr;
    std::vector<FunctionDefinition>* found = nullptr;
    std::string enclosing_function; ///< qualified name, empty outside any function
};

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData data) {
    auto& context = *static_cast<VisitContext*>(data);
    const std::optional<FunctionKind> kind = function_kind(clang_getCursorKind(cursor));
    if (!kind.has_value()) {
        return CXChildVisit_Recurse;
    }
    const SourcePosition location = position_of(cursor, *context.directory);
    if (location.in_system_header) {
        return CXChildVisit_Continue;
    }

    std::string name;
    if (*kind == FunctionKind::lambda) {
        name = context.enclosing_function.empty() ? "(lambda)"
                                                  : context.enclosing_function + "::(lambda)";
    } else {
        name = scope_qualifier(cursor) +
               std::string{ClangString{clang_getCursorDisplayName(cursor)}.view()};
    }

    if (has_body(cursor) && !location.file.empty() && (*context.include_file)(location.file)) {
        context.found->push_back({.file = location.file,
                                  .line = location.line,
                                  .column = location.column,
                                  .kind = *kind,
                                  .name = name,
                                  .cursor = cursor});
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

std::vector<FunctionDefinition> find_definitions(const ParsedUnit& parsed,
                                                 const FileFilter& include_file) {
    if (!parsed.unit) {
        throw std::invalid_argument("find_definitions: " + parsed.file.string() +
                                    " has no translation unit");
    }
    std::vector<FunctionDefinition> found;
    VisitContext context{.unit = parsed.unit.get(),
                         .directory = &parsed.directory,
                         .include_file = &include_file,
                         .found = &found,
                         .enclosing_function = {}};
    clang_visitChildren(clang_getTranslationUnitCursor(parsed.unit.get()), visit, &context);
    std::ranges::sort(
        found, {}, [](const FunctionDefinition& d) { return std::tie(d.file, d.line, d.column); });
    return found;
}

} // namespace tezcatl::parse
