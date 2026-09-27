---
title: Getting started
description: Build Tezcatl from source and write a first report on a project.
nav_order: 1
---

# Getting started

Build Tezcatl, give it a project's compilation database, and read the report it writes.

## Before you start

You need CMake 3.25 or newer, Ninja, a C++20 compiler, and libclang (the LLVM C API) from LLVM 22.
On Ubuntu 24.04, `scripts/linux-deps.sh` installs and verifies every one of them; it is the same
script CI uses, so it is the authoritative list.

## Build

On Linux, point `LLVM_ROOT` at the LLVM install:

```sh
LLVM_ROOT=/usr/lib/llvm-22 cmake --preset linux-gcc
cmake --build --preset linux-gcc
ctest --preset linux-gcc
```

On Windows, with LLVM in `C:\Program Files\LLVM`, run from a Visual Studio developer prompt, or
through `scripts\msvc.cmd`, which enters one for you:

```bat
scripts\msvc.cmd cmake --preset windows-msvc
scripts\msvc.cmd cmake --build --preset windows-msvc
scripts\msvc.cmd ctest --preset windows-msvc
```

The binary is `build/<preset>/src/tezcatl`. Check it runs:

```sh
tezcatl --version
```

It prints its own version and the libclang release it loaded.

## Get a compilation database

Tezcatl parses each unit the way the project compiles it, so it needs `compile_commands.json`.

- **CMake:** configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. The database is in the build
  directory.
- **Make and other build systems:** run the build under [bear](https://github.com/rizsotto/Bear):
  `bear -- make`. The database is written where you ran it.

Build the project fully before measuring it. A database made from a partial build lists only the
units that were compiled, and a failed step can stop a Make loop without a loud error. Check the
number of entries against the number of sources you expect.

## Write a report

```sh
tezcatl report -p build --root . --out metrics
```

- `-p` is the directory holding `compile_commands.json`.
- `--root` is the project: only files under it are measured.
- `--out` is where the report is written.

Open `metrics/report.md`. It opens with what was measured: how many units parsed, any that failed,
and how many source files under the root no unit reached. Read that paragraph before any figure.

## Next steps

- Group the project into modules: [Modules and test code](modules.md).
- Import test coverage: [Coverage](coverage.md).
- Every option and output: [The report](report.md).
