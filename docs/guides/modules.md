---
title: Modules and test code
description: Group a project's files into modules, and tell its test code from the product.
nav_order: 3
---

# Modules and test code

A module map groups files into the parts a reviewer thinks in. Test globs keep the test suite out of
the product's figures.

## Write a module map

One rule per line, `MODULE = GLOB`, with globs relative to the root. **The first matching rule
wins**, so put specific rules before general ones:

```text "modules.txt"
# Vendored code first, so its figures are not read as the project's.
third-party/zlib = src/vendor/zlib/**

# The project's own layers.
core = src/core/**
io   = src/io/**
cli  = src/cli/**
```

Pass it with `--modules modules.txt`. A file no rule matches belongs to `(unassigned)`, which is
reported like any other module and never dropped.

Give each bundled library its own module rather than one shared `third-party` module. One module for
unrelated libraries joins them into one node of the module graph and can show cycles that do not
exist. The [Earthworm sample](../sample-report/earthworm/index.md) has a map that does this.

## Tell test code from the product

A file is test code when its root-relative path matches a test glob. The defaults are
`**/test/**`, `**/tests/**`, `**/*_test.*` and `**/test_*.*`. Give your own with `--test-files`,
once per glob; they replace the defaults:

```sh
tezcatl report -p build --out metrics --test-files "unittest/**" --test-files "**/*Tests.cpp"
```

Test code counts toward a module's test lines only. Complexity, Halstead figures, documentation
coverage and imported coverage describe the product. A test covers its own lines by running, so
counting them would lift the coverage figure by the size of the test suite.

Test functions and declarations are still listed in `functions.csv` and `api.csv`, and `files.csv`
and `report.json` carry every file's role.

## Leave out what is not the project

`--exclude GLOB` takes files under the root out of the project entirely. Use it for build trees
other than the one given with `-p`, and for vendored code you do not want measured at all:

```sh
tezcatl report -p build/release --root . --out metrics --exclude "build/**"
```

The build directory given with `-p` is excluded without asking when it lies inside the root. A
build directory that is the root, as `bear` makes for Make projects, excludes nothing.
