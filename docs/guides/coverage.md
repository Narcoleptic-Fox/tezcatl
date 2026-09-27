---
title: Coverage
description: Produce coverage data with gcc or clang and import it into the report.
nav_order: 4
---

# Coverage

Tezcatl does not run tests or instrument code. It imports coverage that gcc, clang or lcov already
measured, merges it, and attributes it to the project's files and modules.

## Formats it reads

| Format | Written by |
|---|---|
| gcov JSON | `gcov -b --json-format` (gcc 9 and later), one document or one per line with `--stdout` |
| lcov tracefile | `lcov`, `gcovr --lcov`, `llvm-cov export -format=lcov` |
| llvm-cov JSON | `llvm-cov export -format=text` |

Each file is recognised by its content, so any mix can be passed at once. Without `-b`, gcov writes
no branch data.

## With gcc

Build with coverage, run the tests, then run gcov on **every object the build made**, not only
those the tests ran:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_FLAGS="--coverage -O0" -DCMAKE_CXX_FLAGS="--coverage -O0" \
    -DCMAKE_EXE_LINKER_FLAGS=--coverage
cmake --build build && (cd build && ctest)
(cd build && for d in $(find . -name '*.gcno' -printf '%h\n' | sort -u); do
    (cd "$d" && gcov -b --json-format --stdout ./*.gcno); done) > coverage.gcov.json
tezcatl report -p build --root . --coverage coverage.gcov.json --out metrics
```

An object the tests never ran has a `.gcno` file and no `.gcda`. Reading only the `.gcda` files
leaves it out, so its code is missing rather than counted as 0% covered.

**Use gcc 14 or later when branch figures must match gcovr's.** gcc 14's JSON names each branch by
the blocks it connects, and Tezcatl then agrees with gcovr on lines, branches and functions. gcc 13's
JSON does not, so a line compiled differently in two places can give different branch counts.
Line coverage agrees with either.

## With clang

```sh
clang++ -fprofile-instr-generate -fcoverage-mapping ...
LLVM_PROFILE_FILE=tests.profraw ./tests
llvm-profdata merge -sparse tests.profraw -o tests.profdata
llvm-cov export -format=text -instr-profile=tests.profdata ./tests > coverage.llvm.json
```

Export once from merged profiles. Tezcatl refuses the same file in two llvm-cov exports, and
llvm-cov totals mixed with line data for the same file, because either would count it twice.

## When the data was recorded somewhere else

Coverage recorded in a CI container lists that machine's paths. Map them onto the checkout:

```sh
tezcatl report -p build --root . --coverage ci.info --path-map /ci/work=.
```

If no recorded file lands under the root, Tezcatl stops with an error rather than writing a report
with no coverage. The paths almost always need `--path-map`.

## What is counted

Merging follows gcovr's `--merge-lines`: a line is covered if any unit ran it. Coverage describes
the product only; test files' coverage of themselves is left out of every module and total, and
kept in `coverage.csv`. The exact rules are in [Metric definitions](../metrics.md).
