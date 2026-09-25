#pragma once

#include <clang-c/CXString.h>

#include <string_view>

namespace tezcatl::parse {

/// Owns a CXString returned by libclang and disposes of it exactly once.
class ClangString {
public:
    explicit ClangString(CXString raw) noexcept : raw_(raw) {}
    ~ClangString() { clang_disposeString(raw_); }

    ClangString(const ClangString&) = delete;
    ClangString& operator=(const ClangString&) = delete;
    ClangString(ClangString&&) = delete;
    ClangString& operator=(ClangString&&) = delete;

    /// The string's contents, valid for the lifetime of this object. libclang
    /// may hand back a null C string; that is returned as empty, not as UB.
    [[nodiscard]] std::string_view view() const noexcept {
        const char* text = clang_getCString(raw_);
        return text == nullptr ? std::string_view{} : std::string_view{text};
    }

private:
    CXString raw_;
};

} // namespace tezcatl::parse
