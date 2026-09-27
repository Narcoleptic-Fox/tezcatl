---
title: Tezcatl
description: Measures a C or C++ codebase from its own compilation database and writes a code metrics baseline for people and for programs.
---

# Tezcatl

*Nahuatl for "mirror".* Tezcatl measures a C or C++ codebase and writes a code metrics baseline: lines
of code, cyclomatic complexity, Halstead measures, documentation coverage, imported test coverage,
and the include graph with its cycles and coupling. It reads the project's own
`compile_commands.json` and parses every unit with libclang and the project's real flags, so it
measures the code as compiled rather than as text.

One command writes the whole baseline:

```sh
tezcatl report -p build --root . --modules modules.txt --out metrics
```

It writes `report.md` for people, `report.json` for programs, every table as CSV, and the include
graph for Graphviz.

## Where to go next

| You want to | Read |
|---|---|
| build Tezcatl and write a first report | [Getting started](guides/getting-started.md) |
| know every option and output of `report` | [The report](guides/report.md) |
| import test coverage | [Coverage](guides/coverage.md) |
| know exactly what each figure means | [Metric definitions](metrics.md) |
| see what a report looks like on a real system | [Sample reports](sample-report/index.md) |
| understand how Tezcatl is built and tested | [Architecture](architecture/index.md) |

## How far it has been checked

Every figure is checked against an independent tool or a hand count before it is trusted.

| Check | Result |
|---|---|
| [Earthworm](sample-report/earthworm/index.md), 929 C and C++ units | 0 parse errors; 8.6 s at 16 threads |
| Complexity against lizard on Earthworm, in the same configuration | 94.1% equal; 99.0% outside SQLite's amalgamation, with the differences explained |
| Coverage against gcovr: [libmseed](sample-report/libmseed/index.md) built with gcc 14 | lines, branches and functions equal in all 42 files |
| Coverage against gcovr: Tezcatl itself | equal in all 85 files |
| The sabotage log | 142 deliberate breaks of the code, each caught by a test |

Tezcatl is MIT licensed. The source is on
[GitHub](https://github.com/Narcoleptic-Fox/tezcatl).
