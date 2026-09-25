# Third-party dependencies, pinned by archive hash so a build is reproducible
# without trusting that a tag was never moved.

include(FetchContent)

FetchContent_Declare(
    CLI11
    URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.7.2.tar.gz
    URL_HASH SHA256=46eef3101da70852ec7af026e09d485ccee81813331c8c6052d39344443b83da
    SYSTEM)
FetchContent_MakeAvailable(CLI11)

# The release's own archive of the headers and CMake files; its SHA-256 is the
# one published in the v3.12.0 release notes.
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    SYSTEM)
FetchContent_MakeAvailable(nlohmann_json)

if(TEZCATL_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
        URL_HASH SHA256=0957cae5821b17ce07f0833aaa52b5137643a8382203221f363a8303c109af34
        SYSTEM)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

    # Validates the report against docs/report.schema.json in the tests. The
    # project publishes no hash; this is the tag archive's SHA-256 as
    # downloaded on 2026-09-25. It uses the nlohmann_json target above.
    FetchContent_Declare(
        nlohmann_json_schema_validator
        URL https://github.com/pboettch/json-schema-validator/archive/refs/tags/2.4.0.tar.gz
        URL_HASH SHA256=24cbb114609cc9b43d4018b8d03e082ff5d2f26f5dce8bd36538097267b63af9
        SYSTEM)
    FetchContent_MakeAvailable(nlohmann_json_schema_validator)
endif()
