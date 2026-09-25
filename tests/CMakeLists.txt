include(Catch)

add_executable(tezcatl_tests unit/libclang_info_test.cpp)
target_link_libraries(tezcatl_tests PRIVATE tezcatl::core Catch2::Catch2WithMain)
tezcatl_apply_warnings(tezcatl_tests)
tezcatl_copy_runtime_dlls(tezcatl_tests)

# PRE_TEST: discovery runs the test binary, which needs libclang.dll copied
# first; discovering at build time would race the copy.
catch_discover_tests(tezcatl_tests DISCOVERY_MODE PRE_TEST)

# End-to-end: the shipped binary starts, finds libclang, and reports both
# versions. PASS_REGULAR_EXPRESSION ignores the exit code, so the regex is the
# assertion and must name the whole shape.
add_test(NAME cli.version COMMAND tezcatl --version)
set_tests_properties(
    cli.version PROPERTIES PASS_REGULAR_EXPRESSION
                           "^tezcatl ${PROJECT_VERSION} \\(libclang: [^)]*clang version [0-9]+\\.")

add_test(NAME cli.rejects_unknown_flag COMMAND tezcatl --no-such-flag)
set_tests_properties(cli.rejects_unknown_flag PROPERTIES WILL_FAIL TRUE)
