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

**Control:** a sabotage that only adds a comment must leave every test green, and does. The
harness deletes the sabotaged file's object before building and refuses a result if it was not
rebuilt: one run reported a green that turned out to be a stale binary.

**Tests that could not fail, found by this log** (each fixed, then its sabotage re-run red):

| Found by | Why it could not fail | Fix |
|---|---|---|
| 33 | every count tested (1, 10, 11, 12, 21) has ⌈0.9n⌉ equal to round(0.9n) | a 6-value case (0.9 × 6 = 5.4) |
| 47, and 44 against `cli.includes.cycles` | a `;` in `PASS_REGULAR_EXPRESSION` splits it into a list, and CTest passes if any part matches; the cycles regex contained the bare fragment `ring/b\.h` | `;` written as `[;]`, and tests/CMakeLists.txt refuses any pass regex that splits (sabotaged: a bare `;` stops the configure) |
| 48 | every source file in the fixture also had an include edge | `lone.cpp`, which includes nothing |
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
