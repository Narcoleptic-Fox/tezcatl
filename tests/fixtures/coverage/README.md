# Coverage fixture

`src/` is a small program: `classify` has a branch that is never taken (it is never called with
0), `never_called` is never called, and the template `clamp` is instantiated for `int` and
`double`. `data/` holds its coverage, as three tools wrote it, generated 2026-09-25 on Windows.

| File | Written by | Commands |
|---|---|---|
| `calc.gcov.json` | gcc 15.2.0 (mingw) | `g++ --coverage -O0 -std=c++20 -I../src ../src/calc.cpp ../src/main.cpp`, run, then `gcov -b --json-format --stdout *.gcda` |
| `calc.info` | gcovr 8.6 | `gcovr --root ../src --merge-lines --lcov calc.info .` on the same run |
| `calc.llvm.json` | LLVM 22.1.3 | `clang++ -fprofile-instr-generate -fcoverage-mapping -O0 -std=c++20`, run, `llvm-profdata merge -sparse`, `llvm-cov export -format=text` |

The directory the data was generated in has been replaced by `/coverage-fixture`, so paths read
`/coverage-fixture/src/calc.cpp` (the gcov JSON keeps its paths relative to
`/coverage-fixture/gcc`, as gcov wrote them). Nothing else was edited.

`tests/calc_test.cpp` and `compile_commands.json.in` were added afterwards so the same project can
be parsed as well as imported: `calc.hpp` is then seen by three units, and the test file is test
code by the default globs. The test file was never part of the measured program, so the coverage
data has no record of it.

The expected totals come from the tools, not from Tezcatl:

| Data | Lines (covered) | Branches (covered) | Functions (covered) | Oracle |
|---|---|---|---|---|
| gcov JSON and lcov | 17 (14) | 8 (6) | 5 (4) | `gcovr --merge-lines --json-summary` on the same run; the lcov file's own LF/LH, BRF/BRH, FNF/FNH records agree |
| llvm-cov JSON | 24 (18) | 10 (6) | 4 (3) | `llvm-cov report -show-branch-summary` |

gcov and llvm-cov measure differently (llvm counts a template once as a function and counts
lines by region), so their numbers differ for the same program, as expected. Per file, gcovr
reports calc.cpp 8 (5), 4 (3), 2 (1); calc.hpp 4 (4), 4 (3), 2 (2); main.cpp 5 (5), 0 (0), 1 (1).
