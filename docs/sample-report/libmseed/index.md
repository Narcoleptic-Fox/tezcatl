# Sample report: libmseed

A Tezcatl baseline of [libmseed](https://github.com/EarthScope/libmseed), EarthScope's C library
for miniSEED seismic data (Apache 2.0), **with its test coverage imported**: the sample that checks
coverage import on a real project. Generated 2026-09-25.

| | |
|---|---|
| libmseed | commit `d6e6ad306de5b7bdde072e5a5e3d58ddcc11abd8` (2026-08-05) |
| Tezcatl | commit `966eabf`, libclang 22.1.8 |
| Build | Ubuntu 24.04.5, gcc 13.3.0, CMake 3.28.3, `--coverage -O0`, its 11 tests passing |
| Units | 38 parsed, 0 with errors |
| Run | 8 s wall, 125 MB peak |

## Reproduce

```sh
git clone https://github.com/EarthScope/libmseed.git && cd libmseed
git checkout d6e6ad306de5b7bdde072e5a5e3d58ddcc11abd8
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_C_FLAGS="--coverage -O0" -DCMAKE_EXE_LINKER_FLAGS=--coverage \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build && (cd build && ctest)
# gcov on every .gcno, not only the .gcda: objects the tests never ran are coverage too
(cd build && for d in $(find . -name '*.gcno' -printf '%h\n' | sort -u); do
    (cd "$d" && gcov -b --json-format --stdout *.gcno); done) > coverage.gcov.json
tezcatl report -p build --root . --coverage coverage.gcov.json --out report \
    --modules /path/to/tezcatl/docs/sample-report/libmseed/modules.txt
```

## The coverage against gcovr

gcovr 8.6 (`--merge-lines`) read the same build. Of the 43 files it reports, one is
`build/CMakeFiles/.../CMakeCCompilerId.c` (9 lines, none covered), which Tezcatl leaves out as
build-directory code. For the other 42:

| Measure | Files where Tezcatl equals gcovr | Totals |
|---|---:|---|
| Lines and lines covered | **42 of 42** | 15,976 (8,655) both, before test code is set aside |
| Functions | 31 of 42 | gcovr excludes two functions that start on one line |
| Branches | 30 of 42 | see below |

Where they differ, the cause was traced to the gcov data:

- **Two functions on one line.** The tests' `TEST(suite, name)` macro defines a test function
  and its registration function on the same line. gcov records both, and both ran; gcovr cannot
  tell them apart, names them `<unknown function 100>` and excludes them, and their branches
  with them (in `test-crc.c`, all 20 of its branches). Tezcatl counts what gcov recorded. The 11
  test files are exactly the files whose function counts differ, and they account for 11 of the
  12 whose branch counts differ.
- **A header line expanded in several functions** (`yyjson.h`, 72 of about 4,700 branches).
  gcov reports such a line once per unit, with the branches of whatever that unit compiled there,
  and different units compile different code on it (2 branches in one, 40 in another). Tezcatl
  pairs the branches by their position on the line; gcovr keeps the instances apart. gcovr's is
  the more precise count.

Line coverage, the figure a baseline leads with, agrees exactly.

**With gcc 14, everything agrees.** Both differences come from gcc 13's JSON, which has no block
ids; gcovr reads gcc 13 through gcov's text output instead, which does. gcc 14's JSON (format 2)
carries them, and Tezcatl keys branches by them since `b053c62`. Built again with gcc 14.2 and
compared with gcovr 8.6 reading the same run, lines (15,976, 8,653 covered), branches (33,427,
6,006) and functions (539, 409) are equal in all 42 files. The report above is still the gcc 13
run; use gcc 14 or later where branch figures must match gcovr's.

## Reading it

- **Coverage is production code only.** The report above: 46.5% of lines (5,639 of 12,128).
  Counting the 3,848 lines of the test files, which the tests cover by running, would make it
  54.2%; Tezcatl no longer does (docs/metrics.md).
- **yyjson**, the JSON parser libmseed bundles, is its own module: 12,827 code lines, 31.1% of its
  lines covered, against 57.1% for libmseed's own code.
- `example/` is not built by this configuration, so its 11 files are counted and not parsed.
