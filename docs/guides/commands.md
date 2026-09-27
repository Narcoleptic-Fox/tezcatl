---
title: One measure at a time
description: Print one table to stdout with loc, functions, docs, coverage or includes.
nav_order: 5
---

# One measure at a time

`tezcatl report` writes everything. Each measure also has its own command, which prints one CSV
table to stdout and a status line to stderr, for piping into other tools.

| Command | Prints |
|---|---|
| `tezcatl loc PATH...` | lines per file (physical, blank, comment, code) and a `TOTAL` row; `--lines` classifies every line instead |
| `tezcatl functions -p BUILD` | every function with its complexity, rating and Halstead measures; `--summary` gives one row per module |
| `tezcatl docs -p BUILD` | every public declaration in a header and whether it is documented; `--summary` per module |
| `tezcatl coverage DATA...` | imported coverage per file; `--summary` per module, with percentages |
| `tezcatl includes -p BUILD --output TABLE` | the include graph: `edges`, `files`, `modules`, `cycles`, `coupling` or `dot` |

`loc` counts text and needs no compilation database. Every other command except `coverage` parses
the project and takes the same `--root`, `--modules`, `--exclude`, `-j` and `--allow-parse-errors`
options as [the report](report.md).

```sh
tezcatl functions -p build --root . --modules modules.txt --summary
tezcatl includes -p build --root . --output dot | dot -Tsvg -o includes.svg
tezcatl coverage build/coverage.info --root . --summary
```

## Exit codes

A command that parses exits 1 when a unit fails to parse, after printing its table and every error,
unless `--allow-parse-errors` is given. `coverage` exits 1 when no recorded file is under the root.
