#include "parse/libclang_info.hpp"

#include "parse/clang_string.hpp"

#include <clang-c/Index.h>

namespace tezcatl::parse {

std::string libclang_version() {
    const ClangString version{clang_getClangVersion()};
    return std::string{version.view()};
}

} // namespace tezcatl::parse
