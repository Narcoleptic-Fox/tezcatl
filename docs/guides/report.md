---
title: The report
description: Every option, output and exit code of tezcatl report.
nav_order: 2
---

# The report

`tezcatl report` parses the project once and writes the whole baseline to a directory.

```sh
tezcatl report -p build --root . --modules modules.txt --out metrics \
    --coverage build/coverage.gcov.json --test-files "tests/**" --flag-over 15
```

## Options

| Option | Default | Meaning |
|---|---|---|
| `-p, --build-dir DIR` | required | the directory holding `compile_commands.json` |
| `-o, --out DIR` | required | where to write the report; created if missing |
| `--root DIR` | the current directory | only files under it are the project's |
| `--modules FILE` | none: every file is `(unassigned)` | a module map; see [Modules and test code](modules.md) |
| `--test-files GLOB` | `**/test/**`, `**/tests/**`, `**/*_test.*`, `**/test_*.*` | what counts as test code; repeatable, and replaces the defaults |
| `--exclude GLOB` | none | files under the root that are not the project's, such as other build trees or vendored code; repeatable |
| `--coverage FILE` | none | coverage data to import (lcov, gcov JSON, llvm-cov JSON); repeatable |
| `--path-map FROM=TO` | none | coverage recorded under `FROM` is found under `TO`; repeatable |
| `--flag-over N` | 10 | complexity above `N` is flagged |
| `--high-over N` | 20 | complexity above `N` is high; must not be below `--flag-over` |
| `-j, --jobs N` | 0: one per hardware thread | how many units are parsed at once |
| `--allow-parse-errors` | off | exit 0 even when a unit fails to parse |
| `--resource-dir DIR` | found when Tezcatl was built | clang's built-in headers, such as `stddef.h` |

Globs are relative to the root and use `/`: `*` matches within one path component, `?` one
character, and `**` any run of characters including `/`.

The number of threads never changes the result. Every output is identical at `-j 1` and `-j 16`.

## What it writes

| File | Contents |
|---|---|
| `report.md` | the report for people: a summary per module, then each measure, with the functions, declarations and files to look at first |
| `report.json` | everything, as one document; it validates against [`docs/report.schema.json`](https://github.com/Narcoleptic-Fox/tezcatl/blob/main/docs/report.schema.json) |
| `modules.csv` | one row per module across every measure, then a `TOTAL` row |
| `files.csv` | one row per source file: module, role, whether it was parsed, and its lines |
| `functions.csv` | one row per function: complexity, rating and Halstead measures |
| `api.csv` | one row per public declaration in a header, and whether it is documented |
| `coverage.csv` | imported coverage per file; only written with `--coverage` |
| `include-edges.csv`, `include-files.csv`, `include-modules.csv`, `include-cycles.csv`, `include-coupling.csv` | the include graph: its edges, fan-in and fan-out per file and module, its cycles, and the module coupling |
| `includes.dot` | the file include graph for Graphviz, clustered by module, with cycle edges in red |

A directory written with `--coverage` and then again without it loses its `coverage.csv`, so it
never mixes two runs.

## What the report says before any figure

`report.md` opens with what was measured, because every figure depends on it:

- how many units parsed, and how many had errors;
- how many database entries were for other languages, such as Fortran, and were skipped;
- how many units were under the build directory (fetched dependencies, generated code) and skipped;
- how many source files under the root no unit reached. Those count toward lines of code and
  nothing else; `files.csv` marks them `parsed` = `no`.

## Exit codes

| Code | When |
|---|---|
| 0 | the report was written and every unit parsed |
| 1 | a unit failed to parse (the report is still written, and says it is incomplete), unless `--allow-parse-errors`; or coverage data of which nothing is under the root; or any other error, which is printed |
| other non-zero | the command line itself was wrong, such as a missing option or file |

## Next steps

- What each figure means, exactly: [Metric definitions](../metrics.md).
- A real report: [Earthworm](../sample-report/earthworm/index.md).
