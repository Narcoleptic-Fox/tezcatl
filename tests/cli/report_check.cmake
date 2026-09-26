# End-to-end check of `tezcatl report`, run as `cmake -P` by CTest:
#   cmake -DTEZCATL=... -DVALIDATE=... -DSCHEMA=... -DFIXTURE=... -DDB=...
#         -DBROKEN_FIXTURE=... -DBROKEN_DB=... -DWORK=... -P report_check.cmake
#
# FIXTURE is tests/fixtures/coverage. Its expected figures are by hand (the
# line counts and complexities) or from the coverage tools (see the
# fixture's README): calc is calc.cpp + calc.hpp, 26 physical lines (22
# code, 4 blank), complexities 3, 1, 3, three undocumented declarations,
# gcovr coverage 12 (9) lines, 8 (6) branches, 4 (3) functions, included by
# main and by the test; main is 10 lines (7 code, 2 comment, 1 blank),
# complexity 2, 5 (5) lines, 1 (1) function; tests/calc_test.cpp is test
# code, 6 lines (4 code); src/platform_win.c is compiled by no unit, so it is
# not parsed: 2 lines (1 comment, 1 code), unassigned, and nothing else.
#
# A run exiting 0 proves nothing about what it wrote, so every run's exit
# code AND its files are checked.

cmake_minimum_required(VERSION 3.25)

function(fail)
    string(JOIN "" message ${ARGN})
    message(FATAL_ERROR "${message}")
endfunction()

# Runs tezcatl report with ARGN into WORK; sets <prefix>_code and _err.
function(run_report prefix)
    execute_process(COMMAND "${TEZCATL}" report ${ARGN} --out "${WORK}"
                    RESULT_VARIABLE code OUTPUT_VARIABLE out ERROR_VARIABLE err)
    set(${prefix}_code "${code}" PARENT_SCOPE)
    set(${prefix}_err "${err}" PARENT_SCOPE)
endfunction()

function(expect_file name expected)
    file(READ "${WORK}/${name}" actual)
    if(NOT actual STREQUAL expected)
        fail("${name} is not as expected.\n--- expected\n${expected}--- actual\n${actual}")
    endif()
endfunction()

function(expect_contains name fragment)
    file(READ "${WORK}/${name}" actual)
    string(FIND "${actual}" "${fragment}" at)
    if(at EQUAL -1)
        fail("${name} does not contain:\n${fragment}\n--- it is\n${actual}")
    endif()
endfunction()

function(expect_valid_json)
    execute_process(COMMAND "${VALIDATE}" "${SCHEMA}" "${WORK}/report.json"
                    RESULT_VARIABLE code OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT code EQUAL 0 OR NOT out STREQUAL "valid\n")
        fail("report.json does not validate against the schema: ${err}")
    endif()
endfunction()

set(common -p "${DB}" --root "${FIXTURE}" --modules "${FIXTURE}/modules.txt")

# 1. With coverage: every output, and the figures.
file(REMOVE_RECURSE "${WORK}")
run_report(full ${common} --coverage "${FIXTURE}/data/calc.info"
           --path-map "/coverage-fixture=${FIXTURE}")
if(NOT full_code EQUAL 0)
    fail("report with coverage exited ${full_code}:\n${full_err}")
endif()
string(FIND "${full_err}" "5 source files, 5 functions, 3 API declarations; wrote 13 files" at)
if(at EQUAL -1)
    fail("unexpected status:\n${full_err}")
endif()
foreach(name report.md report.json modules.csv files.csv functions.csv api.csv coverage.csv
             include-edges.csv include-files.csv include-modules.csv include-cycles.csv
             include-coupling.csv includes.dot)
    if(NOT EXISTS "${WORK}/${name}")
        fail("${name} was not written")
    endif()
endforeach()

expect_file(files.csv "file,module,role,parsed,physical,blank,comment,code
src/calc.cpp,calc,production,yes,15,2,0,13
src/calc.hpp,calc,production,yes,11,2,0,9
src/main.cpp,main,production,yes,10,1,2,7
src/platform_win.c,(unassigned),production,no,2,0,1,1
tests/calc_test.cpp,(unassigned),test,yes,6,1,1,4
")
# TOTAL: complexities 1, 2, 3, 3 (median 2.5); coverage is gcovr's totals.
expect_file(modules.csv "module,files,files_parsed,production_physical,production_code,production_comment,\
production_blank,test_physical,test_code,functions,complexity_mean,complexity_median,\
complexity_p90,complexity_max,flagged,high,halstead_volume,halstead_effort,api,documented,\
documented_percent,coverage_lines,coverage_lines_covered,coverage_branches,\
coverage_branches_covered,coverage_functions,coverage_functions_covered,fan_in,fan_out,cycle
(unassigned),2,1,2,1,1,0,6,4,0,0.00,0.0,0,0,0,0,0.00,0.00,0,0,0.0,0,0,0,0,0,0,0,1,
calc,2,2,26,22,0,4,0,0,3,2.33,3.0,3,3,0,0,281.54,3935.33,3,0,0.0,12,9,8,6,4,3,2,0,
main,1,1,10,7,2,1,0,0,1,2.00,2.0,2,2,0,0,242.90,2989.53,0,0,0.0,5,5,0,0,1,1,0,1,
TOTAL,5,4,38,30,3,5,6,4,4,2.25,2.5,3,3,0,0,524.44,6924.85,3,0,0.0,17,14,8,6,5,4,,,
")
expect_contains(report.md "| `calc` | 2 (2) | 22 | 0 | 3 | 2.33 | 3 | 0 | 0.0% | 75.0% | no |\n")
expect_contains(report.md "| **Total** | 82.4% (14/17) | 75.0% (6/8) | 80.0% (4/5) |\n")
expect_contains(report.md "3 translation units parsed, 0 with errors. 4 of the 5 source files")
expect_valid_json()

# 2. The same directory without coverage: no stale coverage.csv, and the
#    report says there are no coverage figures.
run_report(plain ${common})
if(NOT plain_code EQUAL 0)
    fail("report without coverage exited ${plain_code}:\n${plain_err}")
endif()
if(EXISTS "${WORK}/coverage.csv")
    fail("coverage.csv from the earlier run was left behind")
endif()
string(FIND "${plain_err}" "removed" at)
if(at EQUAL -1)
    fail("removing coverage.csv was not reported:\n${plain_err}")
endif()
expect_contains(report.md "No coverage data was imported")
expect_valid_json()

# 3. --test-files replaces the default globs: main.cpp becomes test code and
#    tests/calc_test.cpp, matched by no glob now, production.
run_report(globs ${common} --test-files "src/main.*")
if(NOT globs_code EQUAL 0)
    fail("report with --test-files exited ${globs_code}:\n${globs_err}")
endif()
expect_contains(files.csv "src/main.cpp,main,test,yes,")
expect_contains(files.csv "tests/calc_test.cpp,(unassigned),production,yes,")

# 4. A unit that fails to parse: the report is still written and says it is
#    incomplete, and the exit code is 1 unless parse errors are allowed.
file(REMOVE_RECURSE "${WORK}")
run_report(broken -p "${BROKEN_DB}" --root "${BROKEN_FIXTURE}")
if(NOT broken_code EQUAL 1)
    fail("report with a parse error exited ${broken_code}, not 1:\n${broken_err}")
endif()
expect_contains(report.md "**The figures below are incomplete:**")
expect_valid_json()
run_report(allowed -p "${BROKEN_DB}" --root "${BROKEN_FIXTURE}" --allow-parse-errors)
if(NOT allowed_code EQUAL 0)
    fail("--allow-parse-errors still exited ${allowed_code}:\n${allowed_err}")
endif()

message(STATUS "tezcatl report: all checks passed")
