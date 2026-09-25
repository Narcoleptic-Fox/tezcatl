include(Catch)

set(TEZCATL_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

# Compilation databases need absolute directories, so each fixture's is
# generated into the build tree. The build tree is also where a real project's
# database lives, outside the sources it describes.
set(TEZCATL_FIXTURE_DBS "${CMAKE_CURRENT_BINARY_DIR}/fixture-dbs")
foreach(fixture complexity functions functions-broken functions-werror)
    set(FIXTURE_DIR "${TEZCATL_FIXTURES_DIR}/${fixture}")
    configure_file("${FIXTURE_DIR}/compile_commands.json.in"
                   "${TEZCATL_FIXTURE_DBS}/${fixture}/compile_commands.json" @ONLY)
endforeach()
# The same project as MSVC-style (clang-cl) commands, which libclang parses in
# a different driver mode with different flag spellings.
set(FIXTURE_DIR "${TEZCATL_FIXTURES_DIR}/functions")
configure_file("${FIXTURE_DIR}/compile_commands.cl.json.in"
               "${TEZCATL_FIXTURE_DBS}/functions-cl/compile_commands.json" @ONLY)
# A relative include path behind flags Tezcatl does not rewrite (-iprefix and
# -iwithprefixbefore), which only the working-directory fallback can resolve.
configure_file("${FIXTURE_DIR}/compile_commands.iprefix.json.in"
               "${TEZCATL_FIXTURE_DBS}/functions-iprefix/compile_commands.json" @ONLY)

add_executable(
    tezcatl_tests
    unit/command_line_test.cpp
    unit/complexity_test.cpp
    unit/csv_test.cpp
    unit/functions_test.cpp
    unit/libclang_info_test.cpp
    unit/loc_test.cpp
    unit/paths_test.cpp
    unit/source_files_test.cpp)
target_link_libraries(tezcatl_tests PRIVATE tezcatl::core Catch2::Catch2WithMain)
target_compile_definitions(
    tezcatl_tests PRIVATE TEZCATL_FIXTURES_DIR="${TEZCATL_FIXTURES_DIR}"
                          TEZCATL_FIXTURE_DBS="${TEZCATL_FIXTURE_DBS}")
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

# Hand count of tests/fixtures/loc (physical,blank,comment,code):
#   sample.cpp 12,2,5,5   sample.hpp 4,1,1,2   notes.txt is not source.
add_test(NAME cli.loc.fixture_totals COMMAND tezcatl loc "${TEZCATL_FIXTURES_DIR}/loc")
set_tests_properties(
    cli.loc.fixture_totals
    PROPERTIES PASS_REGULAR_EXPRESSION
               "^file,physical,blank,comment,code\n[^\n]*/sample\\.cpp,12,2,5,5\n[^\n]*/sample\\.hpp,4,1,1,2\nTOTAL,16,3,6,7\n$"
)

add_test(NAME cli.loc.missing_path_fails COMMAND tezcatl loc "${TEZCATL_FIXTURES_DIR}/no-such-dir")
set_tests_properties(cli.loc.missing_path_fails PROPERTIES WILL_FAIL TRUE)

# The full function list is asserted in unit/functions_test.cpp; this checks
# the shipped binary end to end, including CSV quoting of a name with a comma.
add_test(NAME cli.functions.fixture
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/functions" --root
                 "${TEZCATL_FIXTURES_DIR}/functions")
set_tests_properties(
    cli.functions.fixture
    PROPERTIES PASS_REGULAR_EXPRESSION
               "\nshared\\.hpp,12,15,function,\"geo::area\\(size_t, size_t\\)\"\n")

# A unit that fails to parse must fail the run, not shrink the report.
add_test(NAME cli.functions.parse_error_fails
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/functions-broken" --root
                 "${TEZCATL_FIXTURES_DIR}/functions-broken")
set_tests_properties(cli.functions.parse_error_fails PROPERTIES WILL_FAIL TRUE)

# The project's -Werror must not turn a warning (here an unused variable) into
# a parse failure: warnings are build policy, not unparsed code.
add_test(NAME cli.functions.werror_is_not_a_parse_error
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/functions-werror" --root
                 "${TEZCATL_FIXTURES_DIR}/functions-werror")
set_tests_properties(
    cli.functions.werror_is_not_a_parse_error
    PROPERTIES PASS_REGULAR_EXPRESSION
               "werror\\.cpp,1,5,function,warns_but_parses\\(\\).*1 translation units, 0 with errors")

add_test(NAME cli.functions.working_directory_fallback
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/functions-iprefix" --root
                 "${TEZCATL_FIXTURES_DIR}/functions")
set_tests_properties(
    cli.functions.working_directory_fallback
    PROPERTIES PASS_REGULAR_EXPRESSION
               "include/util/detail\\.hpp,4,12,function,util::one\\(\\).*1 translation units, 0 with errors")

add_test(NAME cli.functions.parse_error_reported
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/functions-broken" --root
                 "${TEZCATL_FIXTURES_DIR}/functions-broken" --allow-parse-errors)
set_tests_properties(
    cli.functions.parse_error_reported
    PROPERTIES PASS_REGULAR_EXPRESSION
               "missing\\.hpp.*file not found.*broken\\.cpp,3,5,function,still_found\\(\\)")
