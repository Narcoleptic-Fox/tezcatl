# Sabotage log

A test that cannot fail is a comment. Each row below is a deliberate break of the
implementation, the check that caught it, and the date it was last confirmed. Re-run a row
whenever the code or the check it names changes; a row whose check stayed green is a bug in
the check.

| # | Sabotage | Caught by | Last confirmed |
|---|---|---|---|
| 1 | `libclang_version()` returns `""` (src/parse/libclang_info.cpp) | unit test *libclang reports its release version at run time*, and `cli.version` | 2026-09-24, MSVC; and in CI on all four test jobs (PR #1, run 36085525279) |
| 2 | `main` swallows a CLI parse error and exits 0 (src/cli/main.cpp) | `cli.rejects_unknown_flag` | 2026-09-24, MSVC |
| 3 | Null pointer dereference added to src/parse/libclang_info.cpp | cppcheck `nullPointer` via `scripts/lint.sh` | 2026-09-24, cppcheck 2.22 |

### Lines of code (src/metrics/loc.cpp), all confirmed 2026-09-24 on MSVC

| # | Sabotage | Caught by (loc_test.cpp test case) |
|---|---|---|
| 4 | `finish_line` records comment lines as code | every test case (17 assertions) |
| 5 | `splice_length` never finds a splice | *line splices* |
| 6 | `is_raw_string_prefix` never matches | *raw string literals* |
| 7 | the digit-separator branch never taken | *digit separators are not character literals* |
| 8 | backslash escapes in literals ignored | *comment markers inside literals are not comments* |
| 9 | the UTF-8 byte order mark not skipped | *a UTF-8 byte order mark is not code* |
| 10 | `/` after an opener's own `*` closes the comment | *block comments span lines...* |

### Parsing and function discovery (src/parse/), all confirmed 2026-09-24 on MSVC

| # | Sabotage | Caught by |
|---|---|---|
| 11 | no `-resource-dir` passed | *every function definition...*, *functions in system headers...* (via `<cpuid.h>`; with only `<stddef.h>` this sabotage passed on Windows, because the OS headers also provide it) |
| 12 | no working directory handling at all | 5 tests, including both fixture databases and the CLI tests |
| 13 | `has_body` always true (declarations, `= default`, `= delete` reported) | *every function definition...* |
| 14 | `merge_duplicates` keeps repeats | *every function definition...* |
| 15 | functions in system headers kept | *functions in system headers are never reported...* |
| 16 | error diagnostics ignored | *a unit that fails to parse...*, `cli.functions.parse_error_fails`, `cli.functions.parse_error_reported` |
| 17 | GCC-mode `-working-directory=` fallback removed | `cli.functions.working_directory_fallback` and the command-line unit tests |
| 18 | include-path flags left relative | the command-line unit tests and the clang-cl fixture |
| 19 | the source file left relative | the command-line unit tests and the clang-cl fixture |
| 19a | the clang-cl source file not marked with /Tp | only on Linux, where the clang-cl fixture's absolute path begins with an option letter (`/w/...` in the container, `/opt/...` on the runner); on Windows the path is `D:\...` and cannot be misread. The unit test *clang-cl names the source file with its language* catches it everywhere |
| 20 | `-Wno-error` not appended | `cli.functions.werror_is_not_a_parse_error` and the clang-cl fixture (`/WX`) |

### Cyclomatic complexity and module summaries, all confirmed 2026-09-25 on MSVC

"The fixture" is *cyclomatic complexity matches the hand counts in the fixture*, which runs on
both the GCC-style and the clang-cl database.

| # | Sabotage | Caught by |
|---|---|---|
| 21 | `&&` never a decision point (src/metrics/complexity.cpp) | the fixture, `cli.functions.summary` |
| 22 | tokens of nested lambdas and local classes not excluded (src/parse/function_tokens.cpp since 2026-09-25, shared with Halstead) | the fixture, `cli.functions.summary`, and the Halstead fixture test |
| 23 | `-Xclang -detailed-preprocessing-record` not passed (the libclang option alone, which a GCC-mode `--` disables) | the fixture, both include tests, `cli.functions.summary` and all five `cli.includes.*` table tests |
| 24 | tokens lexed from the raw extent instead of the written range (a macro-made function lexed from its `#define`) | the fixture, `cli.functions.summary` |
| 25 | the `while` of a do-while counted as well as its `do` | the fixture, `cli.functions.summary` |
| 26 | every `&&` token counted, without the AST check (rvalue references, `operator&&`) | the fixture, `cli.functions.summary` |
| 27 | the GNU `a ?: b` not counted | the fixture, `cli.functions.summary` |
| 28 | `-fno-delayed-template-parsing` not passed | the fixture, clang-cl database only (template bodies vanish below C++20) |
| 29 | a dependent `&&` (unresolved, `OverloadedDeclRef`) not counted | the fixture, `cli.functions.summary` |
| 30 | a fold expression over `&&` not counted | the fixture, `cli.functions.summary` |
| 31 | `= default` functions reported when clang synthesizes their bodies (src/parse/functions.cpp) | *every function definition in the fixture project is found* |
| 32 | ratings inclusive (`>=` instead of `>`) (src/metrics/summary.cpp) | *ratings use strict thresholds*, *the distribution of a set of values*, `cli.functions.summary` |
| 33 | p90 rank rounded to nearest instead of ceiling | *p90 by nearest rank at the boundaries*, `cli.functions.summary` |
| 34 | p90 rank truncated | *p90 by nearest rank at the boundaries*, *the distribution of a set of values*, `cli.functions.summary` |
| 35 | even-count median takes the upper middle value | *the distribution of a set of values* |
| 36 | inverted thresholds accepted (src/cli/functions_command.cpp) | `cli.functions.rejects_inverted_thresholds` |
| 37 | glob `*` crosses `/` (src/scan/glob.cpp) | *\* stays within one path component* |
| 38 | glob `**/` never matches zero directories | *\*\* crosses path components* |
| 39 | the last matching module rule wins (src/config/modules.cpp) | *the first matching rule names the module* |

### Halstead, all confirmed 2026-09-25 on MSVC

| # | Sabotage | Caught by |
|---|---|---|
| 49 | regions a false `#if` skipped are counted (src/parse/function_tokens.cpp) | *Halstead counts of each function match the hand counts in the fixture* |
| 50 | preprocessor directive lines are counted | the same |
| 51 | closing brackets counted as operators (src/metrics/halstead.cpp) | the fixture test, both hand-computed measure tests, *operators, operands and the tokens that do not count* |
| 52 | `true`, `false`, `nullptr`, `this` counted as operators | the fixture test, *Halstead measures with literals and a value keyword*, the role test |
| 53 | difficulty not halved (n1 N2 / n2) | both hand-computed measure tests |
| 54 | volume with the natural logarithm | both hand-computed measure tests |
| 55 | a module's volume summed from difficulty (src/cli/functions_command.cpp) | `cli.functions.halstead_summary` |
| 56 | difficulty and effort columns swapped | `cli.functions.halstead` |

The no-op control stayed green in both runs.

### Documentation coverage, all confirmed 2026-09-25 on MSVC

"The API test" is *the public API of the fixture headers and how each is documented*.

| # | Sabotage | Caught by |
|---|---|---|
| 57 | a comment counts across a sibling declaration (src/parse/api.cpp) | the API test (identity, after the macro-made from_macro) |
| 58 | a comment in another file counts | the API test (defined_elsewhere, documented only at its definition) |
| 59 | `-fparse-all-comments` not passed (src/parse/translation_unit.cpp) | the API test (every plain comment lost) |
| 60 | private members counted | the API test |
| 61 | defaulted and deleted functions counted | the API test |
| 62 | a struct defined inside a typedef counted as well as the typedef | the API test |
| 63 | plain comments reported as doxygen | the API test |
| 64 | functions with internal linkage counted | the API test |
| 65 | redeclarations counted again | the API test (add is declared twice) |
| 66 | declarations outside headers counted | the API test (only_in_source) |
| 67 | the doxygen count includes plain comments (src/cli/docs_command.cpp) | `cli.docs.summary` |
| 68 | the percentage of undocumented declarations reported | `cli.docs.summary` |
| 69 | the documented column inverted | `cli.docs.declarations` |

Two of these could not fail until the fixture was extended: 58 (no header declaration had a
comment only in another file) and 66 (every function in the .cpp and .c files was a
redeclaration). Row 65 needed the second declaration of `add` for the same reason. The no-op
control stayed green in both runs.

### Coverage import, all confirmed 2026-09-25 on MSVC

| # | Sabotage | Caught by |
|---|---|---|
| 70 | a repeated line replaces its hits instead of adding them (src/coverage/model.cpp) | the lcov merge test (line 3's second record has 0 hits) |
| 71 | a line with 0 hits counts as covered | the lcov merge test |
| 72 | an lcov branch never evaluated (`-`) counts as taken (src/coverage/lcov.cpp) | the lcov merge test |
| 73 | lcov 2's `FN:<line>,<end>,<name>` keeps the end line in the name | the lcov merge test |
| 74 | a relative SF path is not joined to the tracefile's directory | the lcov merge test |
| 75 | a CRLF line ending is kept | the lcov merge test |
| 76 | lcov 2.2's FNA records ignored | the lcov merge test |
| 77 | gcov branches all given one id (src/coverage/json_readers.cpp) | the gcov merge test, the real-data oracle test |
| 78 | gcov paths not resolved against the working directory | the real-data oracle test and the gcov merge test |
| 79 | gcov's one-document-per-line output rejected | the real-data oracle test, the gcov merge test |
| 80 | llvm-cov branches taken from its function totals | the llvm tests |
| 81 | one file allowed in two llvm-cov exports | *llvm-cov totals are taken as they are, once per file* |
| 82 | lcov line data allowed on top of llvm-cov totals | the same |
| 83 | a rooted path such as `/src/a.c` joined to the tracefile's drive (src/coverage/model.cpp) | the real-data oracle test. **Windows only**: on Linux a rooted path is absolute, so this cannot fail there |
| 84 | a file that is not coverage data reads as empty (src/coverage/read_file.cpp) | *a file that is not coverage data is an error* |
| 85 | llvm-cov JSON read as gcov JSON | the real-data llvm test |
| 86 | `--path-map` not applied (src/cli/coverage_command.cpp) | the three `cli.coverage` oracle tests |
| 87 | a percentage with nothing to cover written as 0.0 | `cli.coverage.gcov_summary` (main has no branches) |
| 88 | no file under the root exits 0 | `cli.coverage.nothing_under_root_fails` |
| 89 | with no build directory, the current directory excludes everything under it (src/cli/project.cpp) | `cli.coverage.lcov_per_file`, which runs from the project root. It could not fail until then: CTest runs from the build tree |
| 90 | path mappings matched by string prefix, not whole components (src/coverage/path_map.cpp) | *recorded paths move by whole components* |

The no-op control stayed green in all three runs.

### Include graph, all confirmed 2026-09-25 on MSVC

| # | Sabotage | Caught by |
|---|---|---|
| 40 | only directives in the main file (so guard-skipped headers lose their edges) (src/parse/includes.cpp) | *every include edge in the fixture project is found* and all five `cli.includes.*` table tests |
| 41 | directives inside system headers kept | *directives inside system headers are never edges, whatever the filter* |
| 42 | Tarjan: low link not passed up to the parent (src/graph/digraph.cpp) | three graph tests including the 200,000-node ring, `cli.includes.cycles`, `.files`, `.dot` |
| 43 | Tarjan: edges to nodes no longer on the stack lower the low link | *a diamond, a self-edge and an empty graph have no cycles*, *separate cycles are reported separately* |
| 44 | components of one node reported as cycles | two graph tests, `cli.includes.cycles`, `.files`, `.modules` |
| 45 | repeated edges kept | *nodes are numbered in name order and repeats merge*, `cli.includes.cycles`, `.files`, `.modules`, `.coupling` |
| 46 | edges within a module count toward module fan-in, fan-out and cycles (src/cli/includes_command.cpp) | `cli.includes.modules` |
| 47 | cycle edges not drawn red in DOT | `cli.includes.dot` |
| 48 | source files with no includes are not nodes | `cli.includes.cycles`, `.files`, `.modules` |

### The report, all confirmed 2026-09-25 on MSVC

| # | Sabotage | Caught by |
|---|---|---|
| 91 | Markdown: a pipe in a name left unescaped (src/report/markdown_report.cpp) | *a pipe in a name stays inside its table cell* |
| 92 | Markdown: test functions listed among the most complex | *the Markdown lists leave test code out*, *a long list is cut…* |
| 93 | Markdown: test declarations listed as undocumented | *an undocumented test declaration is not listed* |
| 94 | Markdown: least complex listed first | *the Markdown lists leave test code out* |
| 95 | Markdown: the complexity list not cut at 20 | *a long list is cut and names the CSV with all of it* |
| 96 | Markdown: a percentage over nothing written 0.0% | *the Markdown summary has one row per module and a total* |
| 97 | Markdown: a coverage column without coverage data | *without coverage data the report says so rather than 0%* |
| 98 | Markdown: coupling matrix transposed | *the Markdown include section shows the coupling matrix* |
| 99 | Markdown: one unit with errors not stated | *parse errors are stated at the top* |
| 100 | main files of units not collected as graph nodes (src/cli/collect.cpp) | `cli.includes.cycles`, `.files`, `.modules` |
| 101 | functions seen by several units not merged | `cli.functions.header_counted_once` |
| 102 | declarations seen by several units not merged | `cli.docs.header_counted_once` |
| 103 | the source walk filters a rejected directory instead of not entering it (src/scan/source_files.cpp) | *a directory the walk may not descend into is never entered* |
| 104 | report: a stale coverage.csv kept (src/cli/report_command.cpp) | `cli.report` |
| 105 | report: test files' lines not counted | `cli.report` |
| 106 | report: parse errors not passed to the report | `cli.report` |
| 107 | report: coverage table written under another name | `cli.report` |
| 108 | report.json no longer valid (src/report/json_report.cpp) | `cli.report` (the real output), and the sample's schema test |
| 109 | `--path-map` not passed to the report (src/cli/main.cpp) | `cli.report` |
| 110 | `--test-files` bound to another command's options | `cli.report` |
| 111 | the default glob `**/test_*.*` dropped (src/config/file_roles.cpp) | *the default test globs…* |
| 112 | a file is test code only if every glob matches, not any | nine unit tests and `cli.report` |
| 113 | test code counted as production lines (src/report/module_summary.cpp) | *module rows add up every metric…*, the JSON and Markdown sample tests, `cli.report` |
| 114 | test functions in the complexity figures | the same |
| 115 | test headers in documentation coverage | *module rows…* and two Markdown tests |
| 116 | a module without coverage records given no coverage rather than zeros | *module rows…*, `cli.report` |
| 117 | module fan-in and fan-out swapped | *module rows…*, *the JSON report carries the sample's figures*, `cli.report` |
| 118 | a JSON field renamed (src/report/json_report.cpp) | both schema tests, `cli.report` |
| 119 | the schema accepts unknown fields (docs/report.schema.json) | *the schema rejects what the report must not contain* |
| 120 | the JSON totals carry include figures | *the JSON report carries the sample's figures* |

**Control:** a sabotage that only adds a comment must leave every test green, and does. The
harness deletes the sabotaged file's object before building and refuses a result if it was not
rebuilt: one run reported a green that turned out to be a stale binary.

**Tests that could not fail, found by this log** (each fixed, then its sabotage re-run red):

| Found by | Why it could not fail | Fix |
|---|---|---|
| 33 | every count tested (1, 10, 11, 12, 21) has ⌈0.9n⌉ equal to round(0.9n) | a 6-value case (0.9 × 6 = 5.4) |
| 47, and 44 against `cli.includes.cycles` | a `;` in `PASS_REGULAR_EXPRESSION` splits it into a list, and CTest passes if any part matches; the cycles regex contained the bare fragment `ring/b\.h` | `;` written as `[;]`, and tests/CMakeLists.txt refuses any pass regex that splits (sabotaged: a bare `;` stops the configure) |
| 48 | every source file in the fixture also had an include edge | `lone.cpp`, which includes nothing |
| 93 | the sample's one test-code declaration is documented, so it could never reach the undocumented list | a test that makes it undocumented |
| 101, 102 | no CLI fixture had a header parsed by two units; this was as true of the per-command code before collect() | the coverage fixture's compile database, where calc.hpp is parsed by three units |
| 41 | the project filter already excluded system headers | a test whose filter accepts everything |
| a guard against `operator&&` as a name counting | libclang annotates that `&&` as a `DeclRefExpr`, never `OverloadedDeclRef`, so the guard never ran | guard removed; *call_by_name* in the fixture pins the behaviour |

Tried and found to change nothing, so not a check: spelling the resource directory
`/clang:-resource-dir=` versus `-resource-dir=` in clang-cl mode. Both work, so only the plain
spelling is used.

Checks that have caught real defects (so they are known to fire):

| Check | Defect it caught | Date |
|---|---|---|
| clang-tidy `bugprone-exception-escape` | `main` could let a CLI11 `InvalidError` escape to `std::terminate` | 2026-09-24 |
| clang-tidy `cppcoreguidelines-pro-bounds-avoid-unchecked-container-access` | unchecked `operator[]` in a test | 2026-09-24 |
| clang-tidy `cert-err33-c` | ignored `fputs` results in the fatal-error path | 2026-09-24 |
| clang-format `--dry-run --Werror` | unformatted line in src/cli/main.cpp | 2026-09-24 |
| clang-tidy `bugprone-easily-swappable-parameters` | `out` and `err` streams passable in either order; a table on stderr would pass every CLI test, which reads both streams together | 2026-09-25 |
| clang-tidy `readability-function-cognitive-complexity` | Tarjan's loop (27) and the glob matcher (34), both split | 2026-09-25 |
| cppcheck `uninitMemberVarNoCtor` | Tarjan's frame had an uninitialised node member | 2026-09-25 |
| the lizard comparison on Catch2 (docs/metrics.md) | template bodies skipped under clang-cl (180 functions), `= default` reported (63), dependent `&&` not counted (2) | 2026-09-25 |
