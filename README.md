# Tezcatl

*Nahuatl for "mirror".* Tezcatl measures a C/C++ codebase and writes a code metrics baseline
report: lines of code, cyclomatic complexity, Halstead metrics, test coverage (imported from
gcov, llvm-cov or lcov), documentation coverage, and the include dependency graph with its
cycles and fan-in/fan-out. It emits machine-readable data alongside a human-readable report.

It is built to produce the baseline a code review of a C or C++ codebase starts from.

> **Status: under construction.** The project skeleton builds and tests on Windows and Linux.
> No metric is implemented yet.

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
