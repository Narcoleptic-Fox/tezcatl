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
