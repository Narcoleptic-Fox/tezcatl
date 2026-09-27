# Tezcatl

*Nahuatl for "mirror".* Tezcatl measures a C/C++ codebase and writes a code metrics baseline
report: lines of code, cyclomatic complexity, Halstead metrics, test coverage (imported from
gcov, llvm-cov or lcov), documentation coverage, and the include dependency graph with its
cycles and fan-in/fan-out. It emits machine-readable data alongside a human-readable report.

It is built to produce the baseline a code review of a C or C++ codebase starts from.

> **Status: 0.1.0, the first release.** Every metric is implemented, and `tezcatl report` writes
> them all as one baseline. Every definition is in [docs/metrics.md](docs/metrics.md); the JSON
> report's shape is [docs/report.schema.json](docs/report.schema.json).

**Documentation:** [narcoleptic-fox.github.io/tezcatl](https://narcoleptic-fox.github.io/tezcatl/):
guides, metric definitions and architecture, built from [docs/](docs/index.md) with docmd.

**Self-review:** CI measures Tezcatl itself on every push, with its own test coverage, and keeps
the report with the run (`scripts/self-report.sh`, artifact `tezcatl-self-report`).

**Sample:** [Earthworm](docs/sample-report/earthworm/index.md), the open-source seismic processing system:
929 C and C++ units, 0 parse errors, 3.5 s at 16 threads, with the complexity figures checked against lizard (99.0%
agreement outside SQLite's amalgamation, measured in the same configuration).

## Usage

```sh
# The whole baseline in one pass: report.md for people, report.json for programs,
# and every table as CSV, plus the include graph for Graphviz
tezcatl report -p build --root . --modules modules.txt --out metrics
tezcatl report -p build --root . --modules modules.txt --out metrics \
    --coverage build/coverage.info --test-files "tests/**" --flag-over 15
```

`report` writes:

| File | Contents |
|---|---|
| `report.md` | summary per module, then each metric with the functions, declarations and files to look at first |
| `report.json` | everything, validating against `docs/report.schema.json` |
| `modules.csv` | one row per module across every metric, and a TOTAL row |
| `files.csv`, `functions.csv`, `api.csv` | one row per source file, function, public declaration |
| `coverage.csv` | imported coverage per file (only with `--coverage`) |
| `include-*.csv`, `includes.dot` | include edges, fan-in/fan-out per file and module, cycles, coupling |

Units are parsed on one thread per core (`-j N` to choose). The result does not depend on the
number of threads: every figure is identical at `-j 1` and `-j 16`. Earthworm's 929 units take
3.5 s at 16 threads and 23.7 s on one (Release build, AMD Ryzen 7 7840HS).

Test code (by default `**/test/**`, `**/tests/**`, `**/*_test.*`, `**/test_*.*`) counts toward
test lines only; complexity, Halstead and documentation figures are the product's. A unit that
fails to parse still produces a report, marked incomplete, and exit code 1 unless
`--allow-parse-errors`.

Each measure also has its own command, which writes one table to stdout:

```sh
# Lines of code per file (blank, comment, code), and line by line for auditing
tezcatl loc src include
tezcatl loc --lines src/main.cpp

# Every function definition with its cyclomatic complexity, parsed with the
# project's own flags from compile_commands.json
tezcatl functions -p build --root . --modules modules.txt
tezcatl functions -p build --root . --modules modules.txt --summary   # per module

# Test coverage measured by gcc/clang, per file and module (lcov, gcov JSON, llvm-cov JSON)
tezcatl coverage build/coverage.info --root . --modules modules.txt --summary
tezcatl coverage ci/coverage.info --root . --path-map /ci/work=.   # recorded elsewhere

# Documentation coverage of the public API declared in headers
tezcatl docs -p build --root . --modules modules.txt --summary

# The #include graph: edges, fan-in/fan-out, cycles, module coupling, Graphviz
tezcatl includes -p build --root . --modules modules.txt --output cycles
tezcatl includes -p build --root . --output dot | dot -Tsvg -o includes.svg
```

A module map assigns files to modules, first matching glob wins:

```text
core  = src/core/**
tests = tests/**
```

## Building

Requirements: CMake 3.25 or newer, Ninja, a C++20 compiler, and libclang (the LLVM C API).

```sh
# Linux: scripts/linux-deps.sh installs and checks everything, as CI does (Ubuntu 24.04, LLVM 22)
LLVM_ROOT=/usr/lib/llvm-22 cmake --preset linux-gcc
cmake --build --preset linux-gcc
ctest --preset linux-gcc
```

```bat
:: Windows, from a Visual Studio developer command prompt, with LLVM in C:\Program Files\LLVM
cmake --preset windows-msvc
cmake --build --preset windows-msvc
ctest --preset windows-msvc
```

## License

MIT. See [LICENSE](LICENSE).
