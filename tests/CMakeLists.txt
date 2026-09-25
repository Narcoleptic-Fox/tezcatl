include(Catch)

set(TEZCATL_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

add_executable(
    tezcatl_tests
    unit/libclang_info_test.cpp
    unit/loc_test.cpp
    unit/source_files_test.cpp)
target_link_libraries(tezcatl_tests PRIVATE tezcatl::core Catch2::Catch2WithMain)
target_compile_definitions(tezcatl_tests PRIVATE TEZCATL_FIXTURES_DIR="${TEZCATL_FIXTURES_DIR}")
tezcatl_apply_warnings(tezcatl_tests)
tezcatl_copy_runtime_dlls(tezcatl_tests)

# PRE_TEST: discovery runs the test binary, which needs libclang.dll copied
# first; discovering at build time would race the copy.
catch_discover_tests(tezcatl_tests DISCOVERY_MODE PRE_TEST)

# End-to-end tests of the shipped binary. PASS_REGULAR_EXPRESSION ignores the
# exit code, so each regex is the assertion and must name the whole shape.
add_test(NAME cli.version COMMAND tezcatl --version)
set_tests_properties(
    cli.version PROPERTIES PASS_REGULAR_EXPRESSION
                           "^tezcatl ${PROJECT_VERSION} \\(libclang: [^)]*clang version [0-9]+\\.")

add_test(NAME cli.rejects_unknown_flag COMMAND tezcatl --no-such-flag)
set_tests_properties(cli.rejects_unknown_flag PROPERTIES WILL_FAIL TRUE)
