include(Catch)

set(TEZCATL_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

# Compilation databases need absolute directories, so each fixture's is
# generated into the build tree. The build tree is also where a real project's
# database lives, outside the sources it describes.
set(TEZCATL_FIXTURE_DBS "${CMAKE_CURRENT_BINARY_DIR}/fixture-dbs")
foreach(fixture complexity coverage docs functions functions-broken functions-werror halstead includes
                mixed)
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
# The complexity fixture as a clang-cl C++17 command, where template bodies
# are not parsed until instantiated unless Tezcatl says otherwise.
set(FIXTURE_DIR "${TEZCATL_FIXTURES_DIR}/complexity")
configure_file("${FIXTURE_DIR}/compile_commands.cl.json.in"
               "${TEZCATL_FIXTURE_DBS}/complexity-cl/compile_commands.json" @ONLY)

add_executable(
    tezcatl_tests
    unit/api_test.cpp
    unit/command_line_test.cpp
    unit/complexity_test.cpp
    unit/coverage_test.cpp
    unit/csv_test.cpp
    unit/function_halstead_test.cpp
    unit/file_roles_test.cpp
    unit/functions_test.cpp
    unit/glob_test.cpp
    unit/graph_test.cpp
    unit/halstead_test.cpp
    unit/includes_test.cpp
    unit/lcov_test.cpp
    unit/json_report_test.cpp
    unit/libclang_info_test.cpp
    unit/loc_test.cpp
    unit/markdown_report_test.cpp
    unit/merge_test.cpp
    unit/module_summary_test.cpp
    unit/modules_test.cpp
    unit/path_map_test.cpp
    unit/project_test.cpp
    unit/paths_test.cpp
    unit/source_files_test.cpp
    unit/summary_test.cpp
    unit/tables_test.cpp)
target_link_libraries(tezcatl_tests PRIVATE tezcatl::core Catch2::Catch2WithMain
                                            nlohmann_json_schema_validator::validator)
target_compile_definitions(
    tezcatl_tests PRIVATE TEZCATL_FIXTURES_DIR="${TEZCATL_FIXTURES_DIR}"
                          TEZCATL_FIXTURE_DBS="${TEZCATL_FIXTURE_DBS}"
                          TEZCATL_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
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
               "\nshared\\.hpp,12,15,function,\"geo::area\\(size_t, size_t\\)\",\\(unassigned\\),1,ok,")

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

# Hand computed from the 26 values in tests/fixtures/complexity: sum 71 (mean
# 2.73); sorted, the 13th and 14th are 2 (median) and the 24th is 5 (p90,
# nearest rank ceil(0.9 * 26) = 24); max 12. Over 5: 6 and 12; over 11: 12. The custom
# thresholds prove the options reach the summary.
add_test(NAME cli.functions.summary
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/complexity" --root
                 "${TEZCATL_FIXTURES_DIR}/complexity" --modules
                 "${TEZCATL_FIXTURES_DIR}/complexity/modules.txt" --summary --flag-over 5
                 --high-over 11)
set_tests_properties(
    cli.functions.summary
    PROPERTIES PASS_REGULAR_EXPRESSION
               "module,functions,mean,median,p90,max,flagged,high,volume,effort\nfixture,26,2\\.73,2\\.0,5,12,2,1,[0-9]+\\.[0-9][0-9],[0-9]+\\.[0-9][0-9]\nTOTAL,26,2\\.73,2\\.0,5,12,2,1,"
)

# Hand computed in tests/fixtures/halstead/halstead.cpp: add has n1 7, n2 3,
# N1 9, N2 5, so V = 14 log2 10 = 46.51, D = 7/2 * 5/3 = 5.83, E = 271.29.
add_test(NAME cli.functions.halstead
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/halstead" --root
                 "${TEZCATL_FIXTURES_DIR}/halstead")
set_tests_properties(
    cli.functions.halstead
    PROPERTIES PASS_REGULAR_EXPRESSION
               "\nhalstead\\.cpp,6,5,function,\"add\\(int, int\\)\",\\(unassigned\\),1,ok,7,3,9,5,46\\.51,5\\.83,271\\.29\n")

# The module sums of the fixture's five functions, by hand: complexities
# 1 2 1 1 1 (mean 1.20, median 1, p90 2); volume 46.507 + 76 + 33 + 49.829 +
# 30 = 235.34; effort 271.291 + 585.2 + 110 + 290.669 + 315 = 1572.16.
add_test(NAME cli.functions.halstead_summary
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/halstead" --root
                 "${TEZCATL_FIXTURES_DIR}/halstead" --summary)
set_tests_properties(
    cli.functions.halstead_summary
    PROPERTIES PASS_REGULAR_EXPRESSION
               "\n\\(unassigned\\),5,1\\.20,1\\.0,2,2,0,0,235\\.34,1572\\.16\nTOTAL,")

add_test(NAME cli.functions.complexity_rating
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/complexity" --root
                 "${TEZCATL_FIXTURES_DIR}/complexity")
set_tests_properties(
    cli.functions.complexity_rating
    PROPERTIES PASS_REGULAR_EXPRESSION
               "\ncomplexity\\.cpp,[0-9]+,5,function,eleven\\(int\\),\\(unassigned\\),12,flagged,")

add_test(NAME cli.functions.rejects_inverted_thresholds
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/complexity" --flag-over 20
                 --high-over 10)
set_tests_properties(cli.functions.rejects_inverted_thresholds PROPERTIES WILL_FAIL TRUE)

# Documentation coverage of tests/fixtures/docs, from the verdicts listed in
# unit/api_test.cpp: api.hpp (module cpp) has 17 declarations, 11 documented,
# 10 of them doxygen; c_api.h (module c) 9, 4 and 1; elsewhere.hpp is
# unassigned with 1 undocumented. 11/17 = 64.7%, 4/9 = 44.4%, 15/27 = 55.6%.
set(DOCS_ARGS -p "${TEZCATL_FIXTURE_DBS}/docs" --root "${TEZCATL_FIXTURES_DIR}/docs" --modules
              "${TEZCATL_FIXTURES_DIR}/docs/modules.txt")
add_test(NAME cli.docs.summary COMMAND tezcatl docs ${DOCS_ARGS} --summary)
set_tests_properties(
    cli.docs.summary
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "^module,entities,documented,percent,doxygen\n\\(unassigned\\),1,0,0\\.0,0\nc,9,4,44\\.4,1\ncpp,17,11,64\\.7,10\nTOTAL,27,15,55\\.6,11\n"
)

# One row per declaration: a plain comment, and the declaration whose
# libclang-attached comment belongs to the macro-made declaration above it.
add_test(NAME cli.docs.declarations COMMAND tezcatl docs ${DOCS_ARGS})
set_tests_properties(
    cli.docs.declarations
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "\ninclude/api\\.hpp,13,5,function,\"subtract\\(int, int\\)\",cpp,yes,plain\n.*\ninclude/api\\.hpp,61,25,function,identity\\(T\\),cpp,no,none\n"
)

# tests/fixtures/coverage/src/calc.hpp is parsed by all three units of its
# database; its template clamp and its three declarations count once. By
# hand: calc is classify 3 (two ifs), never_called 1, clamp 3 (if and ?:).
set(CALC_ARGS -p "${TEZCATL_FIXTURE_DBS}/coverage" --root "${TEZCATL_FIXTURES_DIR}/coverage"
              --modules "${TEZCATL_FIXTURES_DIR}/coverage/modules.txt")
add_test(NAME cli.functions.header_counted_once COMMAND tezcatl functions ${CALC_ARGS} --summary)
set_tests_properties(
    cli.functions.header_counted_once
    PROPERTIES PASS_REGULAR_EXPRESSION "\ncalc,3,2\\.33,3\\.0,3,3,0,0,[^\n]*\nmain,1,2\\.00,")
add_test(NAME cli.docs.header_counted_once COMMAND tezcatl docs ${CALC_ARGS} --summary)
set_tests_properties(cli.docs.header_counted_once
                     PROPERTIES PASS_REGULAR_EXPRESSION "\ncalc,3,0,0\\.0,0\nTOTAL,3,0,0\\.0,0\n")

# A database made by intercepting a build (bear) records every compiler it
# saw: the Fortran entry is counted and skipped, not a C parse error.
add_test(NAME cli.functions.skips_other_languages
         COMMAND tezcatl functions -p "${TEZCATL_FIXTURE_DBS}/mixed" --root
                 "${TEZCATL_FIXTURES_DIR}/mixed")
set_tests_properties(
    cli.functions.skips_other_languages
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "\nanswer\\.c,1,5,function,answer\\(\\),[^\n]*\ntezcatl: parsed 1 translation units, 0 with errors[;] skipped 1 entries that are not C or C\\+\\+[;] 1 functions"
)

# Coverage import from tests/fixtures/coverage/data, recorded under
# /coverage-fixture. Expected values are the tools' own, not Tezcatl's (see the
# fixture's README): gcovr --merge-lines per file, and llvm-cov report.
set(COVERAGE_DIR "${TEZCATL_FIXTURES_DIR}/coverage")
set(COVERAGE_ARGS --root "${COVERAGE_DIR}" --path-map "/coverage-fixture=${COVERAGE_DIR}")
add_test(NAME cli.coverage.lcov_per_file COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.info"
                                                  ${COVERAGE_ARGS})
# Run from the project root, as a user would: the command has no build
# directory, and must not treat the current directory as one.
set_tests_properties(
    cli.coverage.lcov_per_file
    PROPERTIES
        WORKING_DIRECTORY "${COVERAGE_DIR}"
        PASS_REGULAR_EXPRESSION
        "\nsrc/calc\\.cpp,\\(unassigned\\),8,5,4,3,2,1\nsrc/calc\\.hpp,\\(unassigned\\),4,4,4,3,2,2\nsrc/main\\.cpp,\\(unassigned\\),5,5,0,0,1,1\n"
)

# By module: calc is calc.cpp + calc.hpp, 12 lines (9), 8 branches (6), 4
# functions (3), all 75.0%; main has no branches, so no branch percentage.
add_test(NAME cli.coverage.gcov_summary
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.gcov.json" ${COVERAGE_ARGS} --modules
                 "${COVERAGE_DIR}/modules.txt" --summary)
set_tests_properties(
    cli.coverage.gcov_summary
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "\ncalc,2,12,9,75\\.0,8,6,75\\.0,4,3,75\\.0\nmain,1,5,5,100\\.0,0,0,,1,1,100\\.0\nTOTAL,3,17,14,82\\.4,8,6,75\\.0,5,4,80\\.0\n"
)

add_test(NAME cli.coverage.llvm_summary
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.llvm.json" ${COVERAGE_ARGS} --summary)
set_tests_properties(
    cli.coverage.llvm_summary PROPERTIES PASS_REGULAR_EXPRESSION
                                         "\nTOTAL,3,24,18,75\\.0,10,6,60\\.0,4,3,75\\.0\n")

# Without the mapping every recorded file lies outside the root: an error, not
# an empty report.
add_test(NAME cli.coverage.nothing_under_root_fails
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.info" --root "${COVERAGE_DIR}")
set_tests_properties(cli.coverage.nothing_under_root_fails PROPERTIES WILL_FAIL TRUE)
# WILL_FAIL passes on any failure, a crash included; this checks it is this one.
add_test(NAME cli.coverage.nothing_under_root_says_why
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.info" --root "${COVERAGE_DIR}")
set_tests_properties(
    cli.coverage.nothing_under_root_says_why
    PROPERTIES PASS_REGULAR_EXPRESSION
               "0 source files under the root, 3 outside it\n.*map its paths with --path-map")

# llvm-cov's totals and lcov's lines for the same files would count them twice.
add_test(NAME cli.coverage.mixed_llvm_and_lcov_fails
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.llvm.json"
                 "${COVERAGE_DIR}/data/calc.info" ${COVERAGE_ARGS})
set_tests_properties(cli.coverage.mixed_llvm_and_lcov_fails PROPERTIES WILL_FAIL TRUE)
add_test(NAME cli.coverage.mixed_llvm_and_lcov_says_why
         COMMAND tezcatl coverage "${COVERAGE_DIR}/data/calc.llvm.json"
                 "${COVERAGE_DIR}/data/calc.info" ${COVERAGE_ARGS})
set_tests_properties(
    cli.coverage.mixed_llvm_and_lcov_says_why
    PROPERTIES PASS_REGULAR_EXPRESSION "calc\\.info:[0-9]+: line data for .*also has llvm-cov totals")

# The include graph of tests/fixtures/includes, every table hand computed from
# its #include lines (listed in unit/includes_test.cpp). The module map puts
# ring/ and shared/ in modules of those names; the rest is unassigned.
set(INCLUDES_ARGS -p "${TEZCATL_FIXTURE_DBS}/includes" --root "${TEZCATL_FIXTURES_DIR}/includes"
                  --modules "${TEZCATL_FIXTURES_DIR}/includes/modules.txt")

# Exactly one file cycle, the 3-file ring, and one module cycle that no file
# cycle crosses: ring/c.h -> shared/common.h and shared/x.h -> ring/a.h.
add_test(NAME cli.includes.cycles COMMAND tezcatl includes ${INCLUDES_ARGS} --output cycles)
set_tests_properties(
    cli.includes.cycles
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "^level,cycle,size,members\nfile,1,3,ring/a\\.h[;]ring/b\\.h[;]ring/c\\.h\nmodule,1,2,ring[;]shared\ntezcatl: parsed 3 translation units, 0 with errors[;] 10 files, 12 include edges, 1 file cycles, 1 module cycles\n$"
)

add_test(NAME cli.includes.files COMMAND tezcatl includes ${INCLUDES_ARGS} --output files)
set_tests_properties(
    cli.includes.files
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "^file,module,fan_in,fan_out,cycle\ninclude/lib/util\\.h,\\(unassigned\\),1,0,\nlone\\.cpp,\\(unassigned\\),0,0,\nmain\\.cpp,\\(unassigned\\),0,4,\nother\\.cpp,\\(unassigned\\),0,1,\nring/a\\.h,ring,3,1,1\nring/b\\.h,ring,1,1,1\nring/c\\.h,ring,1,2,1\nshared/common\\.h,shared,3,0,\nshared/x\\.h,shared,1,2,\nshared/y\\.h,shared,2,1,\n"
)

add_test(NAME cli.includes.modules COMMAND tezcatl includes ${INCLUDES_ARGS} --output modules)
set_tests_properties(
    cli.includes.modules
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "^module,files,fan_in,fan_out,cycle\n\\(unassigned\\),4,0,2,\nring,3,2,1,1\nshared,3,2,1,1\n"
)

add_test(NAME cli.includes.coupling COMMAND tezcatl includes ${INCLUDES_ARGS} --output coupling)
set_tests_properties(
    cli.includes.coupling
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "^from_module,to_module,edges\n\\(unassigned\\),\\(unassigned\\),1\n\\(unassigned\\),ring,1\n\\(unassigned\\),shared,3\nring,ring,3\nring,shared,1\nshared,ring,1\nshared,shared,2\n"
)

# Edges inside a cycle are drawn red; the others are not.
add_test(NAME cli.includes.dot COMMAND tezcatl includes ${INCLUDES_ARGS} --output dot)
set_tests_properties(
    cli.includes.dot
    PROPERTIES
        PASS_REGULAR_EXPRESSION
        "subgraph cluster_1 {\n    label=\"ring\"[;]\n    \"ring/a\\.h\"[;]\n    \"ring/b\\.h\"[;]\n    \"ring/c\\.h\"[;]\n  }\n.*\"ring/c\\.h\" -> \"ring/a\\.h\" \\[color=\"#7a1f1f\"\\][;]\n  \"ring/c\\.h\" -> \"shared/common\\.h\"[;]\n"
)

add_test(NAME cli.includes.rejects_unknown_output COMMAND tezcatl includes ${INCLUDES_ARGS} --output
                                                        pie)
set_tests_properties(cli.includes.rejects_unknown_output PROPERTIES WILL_FAIL TRUE)

# Validates a report Tezcatl wrote against docs/report.schema.json.
add_executable(validate_json tools/validate_json.cpp)
target_link_libraries(validate_json PRIVATE nlohmann_json_schema_validator::validator)
target_compile_features(validate_json PRIVATE cxx_std_20)
tezcatl_apply_warnings(validate_json)

# The report command end to end: its files, its figures, its exit codes.
add_test(NAME cli.report
         COMMAND "${CMAKE_COMMAND}" -DTEZCATL=$<TARGET_FILE:tezcatl>
                 -DVALIDATE=$<TARGET_FILE:validate_json>
                 -DSCHEMA=${PROJECT_SOURCE_DIR}/docs/report.schema.json
                 -DFIXTURE=${TEZCATL_FIXTURES_DIR}/coverage
                 -DDB=${TEZCATL_FIXTURE_DBS}/coverage
                 -DBROKEN_FIXTURE=${TEZCATL_FIXTURES_DIR}/functions-broken
                 -DBROKEN_DB=${TEZCATL_FIXTURE_DBS}/functions-broken
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/report-check
                 -P "${CMAKE_CURRENT_SOURCE_DIR}/cli/report_check.cmake")
set_tests_properties(cli.report PROPERTIES PASS_REGULAR_EXPRESSION
                                           "tezcatl report: all checks passed")

# A property value is a list, so a ";" in PASS_REGULAR_EXPRESSION splits it into
# several regexes, and CTest passes a test when any one of them matches: a
# fragment such as "ring/b\.h" passes nearly any output. That made two of the
# tests above unable to fail until it was found by sabotage. Write ";" as "[;]"
# (CMake does not split a list inside square brackets); this refuses the rest.
get_property(tezcatl_all_tests DIRECTORY PROPERTY TESTS)
foreach(test_name IN LISTS tezcatl_all_tests)
    get_test_property("${test_name}" PASS_REGULAR_EXPRESSION pass_regex)
    list(LENGTH pass_regex regex_count)
    if(pass_regex AND regex_count GREATER 1)
        message(FATAL_ERROR "test ${test_name}: PASS_REGULAR_EXPRESSION is a list of "
                            "${regex_count} regexes (a ';' split it); write ';' as '[;]'")
    endif()
endforeach()
