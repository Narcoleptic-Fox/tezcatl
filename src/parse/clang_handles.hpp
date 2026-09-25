#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <memory>
#include <type_traits>

namespace tezcatl::parse {

// Owning handles for libclang's opaque pointer types, each released by the
// matching dispose function exactly once.

struct IndexDeleter {
    void operator()(CXIndex index) const noexcept { clang_disposeIndex(index); }
};
using IndexHandle = std::unique_ptr<std::remove_pointer_t<CXIndex>, IndexDeleter>;

struct TranslationUnitDeleter {
    void operator()(CXTranslationUnit unit) const noexcept { clang_disposeTranslationUnit(unit); }
};
using TranslationUnitHandle =
    std::unique_ptr<std::remove_pointer_t<CXTranslationUnit>, TranslationUnitDeleter>;

struct CompilationDatabaseDeleter {
    void operator()(CXCompilationDatabase database) const noexcept {
        clang_CompilationDatabase_dispose(database);
    }
};
using CompilationDatabaseHandle =
    std::unique_ptr<std::remove_pointer_t<CXCompilationDatabase>, CompilationDatabaseDeleter>;

struct CompileCommandsDeleter {
    void operator()(CXCompileCommands commands) const noexcept {
        clang_CompileCommands_dispose(commands);
    }
};
using CompileCommandsHandle =
    std::unique_ptr<std::remove_pointer_t<CXCompileCommands>, CompileCommandsDeleter>;

struct DiagnosticDeleter {
    void operator()(CXDiagnostic diagnostic) const noexcept { clang_disposeDiagnostic(diagnostic); }
};
using DiagnosticHandle = std::unique_ptr<std::remove_pointer_t<CXDiagnostic>, DiagnosticDeleter>;

} // namespace tezcatl::parse
