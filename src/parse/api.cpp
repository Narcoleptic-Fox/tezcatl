#include "parse/api.hpp"

#include "parse/clang_string.hpp"
#include "parse/cursors.hpp"
#include "parse/position_index.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tezcatl::parse {

namespace fs = std::filesystem;

std::string_view to_string(ApiKind kind) noexcept {
    switch (kind) {
    case ApiKind::function:
        return "function";
    case ApiKind::method:
        return "method";
    case ApiKind::field:
        return "field";
    case ApiKind::variable:
        return "variable";
    case ApiKind::type:
        return "type";
    case ApiKind::type_alias:
        return "type_alias";
    }
    return "unknown";
}

std::string_view to_string(DocStyle style) noexcept {
    switch (style) {
    case DocStyle::none:
        return "none";
    case DocStyle::plain:
        return "plain";
    case DocStyle::doxygen:
        return "doxygen";
    }
    return "unknown";
}

bool is_header(const fs::path& file) {
    std::string extension = file.extension().string();
    std::ranges::transform(extension, extension.begin(), [](char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    });
    constexpr std::array<std::string_view, 9> headers{".h",   ".hh",  ".hpp", ".hxx", ".h++",
                                                      ".inl", ".ipp", ".tpp", ".tcc"};
    return std::ranges::find(headers, extension) != headers.end();
}

namespace {

/// A byte offset in one file.
struct FilePoint {
    CXFile file = nullptr;
    unsigned offset = 0;
};

FilePoint file_point(CXSourceLocation location) {
    FilePoint point;
    clang_getFileLocation(location, &point.file, nullptr, nullptr, &point.offset);
    return point;
}

bool same_file(CXFile a, CXFile b) {
    return a != nullptr && b != nullptr && clang_File_isEqual(a, b) != 0;
}

std::optional<FileKey> file_key(CXFile file) {
    CXFileUniqueID id{};
    if (file == nullptr || clang_getFileUniqueID(file, &id) != 0) {
        return std::nullopt;
    }
    return FileKey{id.data[0], id.data[1], id.data[2]};
}

// The positions of a scope's declarations and typedefs, gathered from its
// cursors. Built only when a declaration of the project in that scope needs
// it.
PositionIndex index_siblings(const std::vector<CXCursor>& siblings) {
    std::vector<PositionIndex::Position> declarations;
    std::vector<PositionIndex::Extent> typedefs;
    for (const CXCursor& sibling : siblings) {
        const CXCursorKind kind = clang_getCursorKind(sibling);
        if (clang_isDeclaration(kind) != 0) {
            const FilePoint point = file_point(clang_getCursorLocation(sibling));
            if (const std::optional<FileKey> key = file_key(point.file)) {
                declarations.push_back({.file = *key, .offset = point.offset});
            }
        }
        if (kind == CXCursor_TypedefDecl) {
            const CXSourceRange extent = clang_getCursorExtent(sibling);
            const FilePoint begin = file_point(clang_getRangeStart(extent));
            const FilePoint end = file_point(clang_getRangeEnd(extent));
            if (const std::optional<FileKey> key = file_key(begin.file)) {
                typedefs.push_back({.file = *key, .begin = begin.offset, .end = end.offset});
            }
        }
    }
    return PositionIndex{std::move(declarations), std::move(typedefs)};
}

std::vector<CXCursor> children_of(CXCursor scope) {
    std::vector<CXCursor> children;
    clang_visitChildren(
        scope,
        [](CXCursor child, CXCursor /*parent*/, CXClientData data) {
            static_cast<std::vector<CXCursor>*>(data)->push_back(child);
            return CXChildVisit_Continue;
        },
        &children);
    return children;
}

bool is_record(CXCursorKind kind) noexcept {
    return kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl ||
           kind == CXCursor_UnionDecl || kind == CXCursor_ClassTemplate;
}

bool is_first_declaration(CXCursor cursor) {
    return clang_equalCursors(cursor, clang_getCanonicalCursor(cursor)) != 0;
}

// A function whose body the compiler writes, or that cannot be called, needs
// no documentation of its own.
bool is_defaulted_or_deleted(CXCursor cursor) {
    return clang_CXXMethod_isDefaulted(cursor) != 0 || clang_CXXMethod_isDeleted(cursor) != 0;
}

std::optional<ApiKind> function_api_kind(CXCursor cursor, bool in_record) {
    if (!is_first_declaration(cursor) || is_defaulted_or_deleted(cursor)) {
        return std::nullopt;
    }
    if (in_record) {
        return ApiKind::method;
    }
    // At namespace scope only free functions and function templates: a
    // member defined outside its class is never its first declaration.
    const CXCursorKind kind = clang_getCursorKind(cursor);
    const bool free_function = kind == CXCursor_FunctionDecl || kind == CXCursor_FunctionTemplate;
    return (free_function && clang_getCursorLinkage(cursor) == CXLinkage_External)
               ? std::optional{ApiKind::function}
               : std::nullopt;
}

// The kind of API entity `cursor` declares in a class (`in_record`) or a
// namespace, or nothing.
std::optional<ApiKind> api_kind(CXCursor cursor, bool in_record) {
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_FunctionDecl:
    case CXCursor_FunctionTemplate:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction:
        return function_api_kind(cursor, in_record);
    case CXCursor_FieldDecl:
        return ClangString{clang_getCursorSpelling(cursor)}.view().empty()
                   ? std::nullopt
                   : std::optional{ApiKind::field};
    case CXCursor_VarDecl:
        if (!is_first_declaration(cursor)) {
            return std::nullopt;
        }
        if (in_record) {
            return ApiKind::field;
        }
        return clang_getCursorLinkage(cursor) == CXLinkage_External
                   ? std::optional{ApiKind::variable}
                   : std::nullopt;
    case CXCursor_StructDecl:
    case CXCursor_ClassDecl:
    case CXCursor_UnionDecl:
    case CXCursor_ClassTemplate:
    case CXCursor_EnumDecl:
        // An unnamed struct in a typedef is documented through the typedef.
        return (clang_isCursorDefinition(cursor) != 0 && clang_Cursor_isAnonymous(cursor) == 0)
                   ? std::optional{ApiKind::type}
                   : std::nullopt;
    case CXCursor_TypedefDecl:
    case CXCursor_TypeAliasDecl:
    case CXCursor_TypeAliasTemplateDecl:
        return ApiKind::type_alias;
    default:
        return std::nullopt;
    }
}

DocStyle style_of(std::string_view comment) {
    const bool doxygen = (comment.starts_with("///") && !comment.starts_with("////")) ||
                         comment.starts_with("//!") ||
                         (comment.starts_with("/**") && !comment.starts_with("/**/")) ||
                         comment.starts_with("/*!");
    return doxygen ? DocStyle::doxygen : DocStyle::plain;
}

// The style of the comment attached to `cursor`, if it is attached directly:
// written in the same file, and either after the declaration begins (a
// trailing comment such as ///<) or before it with no other declaration of
// the same scope in between. libclang also attaches a comment across
// anything that is not ; { } # or @, including a macro that declares
// something, which would document the wrong declaration.
DocStyle documentation_of(CXCursor cursor, const PositionIndex& siblings) {
    const ClangString raw{clang_Cursor_getRawCommentText(cursor)};
    if (raw.view().empty()) {
        return DocStyle::none;
    }
    const CXSourceRange comment = clang_Cursor_getCommentRange(cursor);
    const FilePoint comment_begin = file_point(clang_getRangeStart(comment));
    const FilePoint comment_end = file_point(clang_getRangeEnd(comment));
    const FilePoint declaration = file_point(clang_getRangeStart(clang_getCursorExtent(cursor)));
    if (!same_file(comment_begin.file, declaration.file)) {
        return DocStyle::none;
    }
    if (comment_begin.offset >= declaration.offset) {
        return style_of(raw.view());
    }
    if (comment_end.offset > declaration.offset) {
        return DocStyle::none;
    }
    // The declaration's own location is at or after its start, so it is
    // never "between" and needs no exclusion.
    const std::optional<FileKey> file = file_key(declaration.file);
    const bool interrupted =
        file.has_value() && siblings.declaration_between(
                                *file, {.after = comment_end.offset, .before = declaration.offset});
    return interrupted ? DocStyle::none : style_of(raw.view());
}

struct ApiContext {
    const fs::path* directory = nullptr; ///< for resolving relative file names
    const FileFilter* include_file = nullptr;
    std::vector<ApiEntity>* found = nullptr;
};

// Whether the type declared at `member` is defined inside a typedef among
// `siblings`, as in `typedef struct { ... } name_t;`. The typedef is then the
// type's name, and the API entity; counting the struct too would count one
// type twice. In C, libclang even gives the unnamed struct the typedef's name.
bool defined_in_typedef(CXCursor member, const PositionIndex& siblings) {
    const FilePoint begin = file_point(clang_getRangeStart(clang_getCursorExtent(member)));
    const std::optional<FileKey> file = file_key(begin.file);
    return file.has_value() && siblings.inside_typedef(*file, begin.offset);
}

// The scope's sibling index, built on first use: most scopes a unit sees
// are in system headers and never need one.
class LazySiblings {
public:
    explicit LazySiblings(const std::vector<CXCursor>& siblings) : siblings_(&siblings) {}
    const PositionIndex& get() {
        if (!index_.has_value()) {
            index_.emplace(index_siblings(*siblings_));
        }
        return *index_;
    }

private:
    const std::vector<CXCursor>* siblings_; ///< the scope's members, which outlive this
    std::optional<PositionIndex> index_;
};

void record(const ApiContext& context, CXCursor member, ApiKind kind, LazySiblings& siblings) {
    const SourcePosition position = position_of(member, *context.directory);
    if (position.file.empty() || position.in_system_header || !is_header(position.file) ||
        !(*context.include_file)(position.file)) {
        return;
    }
    if (kind == ApiKind::type && defined_in_typedef(member, siblings.get())) {
        return;
    }
    const bool is_function = kind == ApiKind::function || kind == ApiKind::method;
    const ClangString name{is_function ? clang_getCursorDisplayName(member)
                                       : clang_getCursorSpelling(member)};
    context.found->push_back({.file = position.file,
                              .line = position.line,
                              .column = position.column,
                              .kind = kind,
                              .name = scope_qualifier(member) + std::string{name.view()},
                              .documentation = documentation_of(member, siblings.get())});
}

// Visits the declarations of a namespace, extern "C" block or class. Only
// public members of a class are part of the API, and a scope in a system
// header is not visited at all.
void visit_scope(const ApiContext& context, CXCursor scope, bool in_record) {
    const std::vector<CXCursor> members = children_of(scope);
    LazySiblings siblings{members};
    for (const CXCursor& member : members) {
        if (in_record && clang_getCXXAccessSpecifier(member) != CX_CXXPublic) {
            continue;
        }
        const CXCursorKind kind = clang_getCursorKind(member);
        const bool opens_scope =
            (kind == CXCursor_Namespace && clang_Cursor_isAnonymous(member) == 0) ||
            kind == CXCursor_LinkageSpec ||
            (is_record(kind) && clang_isCursorDefinition(member) != 0);
        const std::optional<ApiKind> api = api_kind(member, in_record);
        if (api.has_value()) {
            record(context, member, *api, siblings);
        }
        if (opens_scope && clang_Location_isInSystemHeader(clang_getCursorLocation(member)) == 0) {
            visit_scope(context, member, is_record(kind));
        }
    }
}

} // namespace

std::vector<ApiEntity> find_api(const ParsedUnit& parsed, const FileFilter& include_file) {
    if (!parsed.unit) {
        throw std::invalid_argument("find_api: " + parsed.file.string() +
                                    " has no translation unit");
    }
    std::vector<ApiEntity> found;
    const ApiContext context{
        .directory = &parsed.directory, .include_file = &include_file, .found = &found};
    visit_scope(context, clang_getTranslationUnitCursor(parsed.unit.get()), false);
    std::ranges::sort(found);
    return found;
}

} // namespace tezcatl::parse
