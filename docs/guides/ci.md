---
title: In CI
description: Keep a code metrics baseline with every build, as Tezcatl does for itself.
nav_order: 6
---

# In CI

A baseline is worth most when every build has one. Generate the report in CI and keep it with the
run, rather than committing it, so it can never describe code that has since changed.

## Tezcatl does this for itself

Every push to Tezcatl builds it with coverage, runs its tests, measures itself and keeps the report
as a build artifact named `tezcatl-self-report`. The steps are in
[`scripts/self-report.sh`](https://github.com/Narcoleptic-Fox/tezcatl/blob/main/scripts/self-report.sh):

```sh
cmake --preset linux-coverage && cmake --build --preset linux-coverage
ctest --preset linux-coverage
scripts/self-report.sh self-report
```

The script ends with receipts, not an exit code: it fails if gcov wrote no coverage records, and it
validates `report.json` against the schema before the report is kept.

## In your own pipeline

1. Build the project with its compilation database and, if you want coverage, with coverage
   flags. See [Coverage](coverage.md).
2. Run the tests.
3. Run `tezcatl report` and keep the output directory as an artifact.
4. Decide what fails the build. `tezcatl report` exits 1 only when a unit fails to parse; it sets no
   quality gates of its own. To gate on a figure, read it from `report.json`, whose shape is fixed by
   its schema and its `schema_version`.

On a shared runner, other build trees may sit under the root. Leave them out with
`--exclude "build/**"`, or they are counted as the project's code.
