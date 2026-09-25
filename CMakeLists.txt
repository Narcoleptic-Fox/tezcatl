cmake_minimum_required(VERSION 3.25...4.4)

project(
    tezcatl
    VERSION 0.1.0
    DESCRIPTION "Measures a C/C++ codebase and writes a code metrics baseline report"
    HOMEPAGE_URL "https://github.com/Narcoleptic-Fox/tezcatl"
    LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(TEZCATL_WERROR "Treat compiler warnings as errors" OFF)
option(TEZCATL_BUILD_TESTS "Build the test suite" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_EXTENSIONS OFF)

include(TezcatlHelpers)
include(Dependencies)
find_package(Libclang REQUIRED)

add_subdirectory(src)

if(TEZCATL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
