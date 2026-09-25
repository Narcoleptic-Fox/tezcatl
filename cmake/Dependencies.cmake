# Third-party dependencies, pinned by archive hash so a build is reproducible
# without trusting that a tag was never moved.

include(FetchContent)

FetchContent_Declare(
    CLI11
    URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.7.2.tar.gz
    URL_HASH SHA256=46eef3101da70852ec7af026e09d485ccee81813331c8c6052d39344443b83da
    SYSTEM)
FetchContent_MakeAvailable(CLI11)

if(TEZCATL_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
        URL_HASH SHA256=0957cae5821b17ce07f0833aaa52b5137643a8382203221f363a8303c109af34
        SYSTEM)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
