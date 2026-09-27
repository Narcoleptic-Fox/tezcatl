# Finds the libclang C API (clang-c/Index.h) and defines the imported target
# Libclang::Libclang.
#
# Point it at an LLVM install with LLVM_ROOT (CMake or environment variable),
# e.g. "C:/Program Files/LLVM" on Windows or "/usr/lib/llvm-22" on Ubuntu.
#
# Sets Libclang_FOUND, Libclang_INCLUDE_DIR, Libclang_LIBRARY, Libclang_VERSION
# (the C API version from Index.h, not the LLVM release), Libclang_RESOURCE_DIR
# (clang's built-in headers), and on Windows Libclang_DLL.

set(_libclang_hints ${LLVM_ROOT} $ENV{LLVM_ROOT})
if(WIN32)
    list(APPEND _libclang_hints "C:/Program Files/LLVM")
endif()

find_path(
    Libclang_INCLUDE_DIR clang-c/Index.h
    HINTS ${_libclang_hints}
    PATH_SUFFIXES include)

find_library(
    Libclang_LIBRARY
    NAMES libclang clang
    HINTS ${_libclang_hints}
    PATH_SUFFIXES lib)

set(_libclang_required Libclang_INCLUDE_DIR Libclang_LIBRARY)

if(WIN32)
    # On Windows the .lib is only an import library; the DLL must be found too
    # or the binaries build fine and then fail to start.
    find_file(
        Libclang_DLL libclang.dll
        HINTS ${_libclang_hints}
        PATH_SUFFIXES bin)
    list(APPEND _libclang_required Libclang_DLL)
endif()

if(Libclang_INCLUDE_DIR AND EXISTS "${Libclang_INCLUDE_DIR}/clang-c/Index.h")
    file(STRINGS "${Libclang_INCLUDE_DIR}/clang-c/Index.h" _libclang_version_lines
         REGEX "^#define CINDEX_VERSION_(MAJOR|MINOR) [0-9]+")
    string(REGEX REPLACE ".*MAJOR ([0-9]+).*" "\\1" _libclang_major "${_libclang_version_lines}")
    string(REGEX REPLACE ".*MINOR ([0-9]+).*" "\\1" _libclang_minor "${_libclang_version_lines}")
    set(Libclang_VERSION "${_libclang_major}.${_libclang_minor}")
endif()

# clang's resource directory (stddef.h and the other built-in headers) sits at
# <libdir>/clang/<major> on both Windows and Linux installs. libclang finds it
# relative to its own shared library, which fails once the DLL is copied next
# to an executable, so Tezcatl passes it explicitly.
if(Libclang_LIBRARY)
    get_filename_component(_libclang_libdir "${Libclang_LIBRARY}" DIRECTORY)
    file(GLOB _libclang_resource_candidates LIST_DIRECTORIES true "${_libclang_libdir}/clang/*")
    foreach(_candidate IN LISTS _libclang_resource_candidates)
        if(EXISTS "${_candidate}/include/stddef.h")
            set(Libclang_RESOURCE_DIR "${_candidate}")
        endif()
    endforeach()
    list(APPEND _libclang_required Libclang_RESOURCE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Libclang
    REQUIRED_VARS ${_libclang_required}
    VERSION_VAR Libclang_VERSION)

if(Libclang_FOUND AND NOT TARGET Libclang::Libclang)
    add_library(Libclang::Libclang SHARED IMPORTED)
    set_target_properties(Libclang::Libclang PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                        "${Libclang_INCLUDE_DIR}")
    if(WIN32)
        set_target_properties(Libclang::Libclang PROPERTIES IMPORTED_IMPLIB "${Libclang_LIBRARY}"
                                                            IMPORTED_LOCATION "${Libclang_DLL}")
    else()
        set_target_properties(Libclang::Libclang PROPERTIES IMPORTED_LOCATION "${Libclang_LIBRARY}")
    endif()
endif()

mark_as_advanced(Libclang_INCLUDE_DIR Libclang_LIBRARY Libclang_DLL)
