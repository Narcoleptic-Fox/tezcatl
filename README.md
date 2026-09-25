# Tezcatl

*Nahuatl for "mirror".* Tezcatl measures a C/C++ codebase and writes a code metrics baseline
report: lines of code, cyclomatic complexity, Halstead metrics, test coverage (imported from
gcov, llvm-cov or lcov), documentation coverage, and the include dependency graph with its
cycles and fan-in/fan-out. It emits machine-readable data alongside a human-readable report.

It is built to produce the baseline a code review of a C or C++ codebase starts from.

> **Status: under construction.** Implemented so far: lines of code, cyclomatic complexity,
> Halstead measures, documentation coverage, and the include dependency graph. Every definition
> is in [docs/metrics.md](docs/metrics.md).

## Usage

```sh
# Lines of code per file (blank, comment, code), and line by line for auditing
tezcatl loc src include
tezcatl loc --lines src/main.cpp

# Every function definition with its cyclomatic complexity, parsed with the
# project's own flags from compile_commands.json
tezcatl functions -p build --root . --modules modules.txt
tezcatl functions -p build --root . --modules modules.txt --summary   # per module

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
# Linux (Ubuntu: apt install libclang-18-dev)
LLVM_ROOT=/usr/lib/llvm-18 cmake --preset linux-gcc
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
