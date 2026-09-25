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

Checks that have caught real defects (so they are known to fire):

| Check | Defect it caught | Date |
|---|---|---|
| clang-tidy `bugprone-exception-escape` | `main` could let a CLI11 `InvalidError` escape to `std::terminate` | 2026-09-24 |
| clang-tidy `cppcoreguidelines-pro-bounds-avoid-unchecked-container-access` | unchecked `operator[]` in a test | 2026-09-24 |
| clang-tidy `cert-err33-c` | ignored `fputs` results in the fatal-error path | 2026-09-24 |
| clang-format `--dry-run --Werror` | unformatted line in src/cli/main.cpp | 2026-09-24 |
