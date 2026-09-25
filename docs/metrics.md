# Metric definitions

Every number Tezcatl reports is defined here precisely enough to be reproduced by hand. Where
a definition involves a choice that other tools make differently, the choice and its reason are
stated, together with a measured comparison against an independent tool.

## Functions

Per-function metrics (complexity, Halstead) are reported for every **function definition with a
body** that is written in the project, found by parsing each translation unit with libclang using
the project's own compile flags from `compile_commands.json`.

| Kind | Examples |
|---|---|
| `function` | free functions, including `static` and those in anonymous namespaces |
| `method` | member functions, defined in or out of the class |
| `constructor`, `destructor`, `conversion` | `S(int)`, `~S()`, `operator int()` |
| `function_template` | a function or member function template (reported once, as written, not per instantiation) |
| `lambda` | each lambda expression is a function of its own, named after the function containing it: `use_lambda()::(lambda)` |

- Declarations, `= default` and `= delete` have no body and are not functions for this purpose,
  including a defaulted function that the compiler defines itself (out of line, or because it is
  used): that body is not written in the project.
- Templates are measured as written, whether or not anything instantiates them. clang-cl before
  C++20 normally skips the bodies of uninstantiated templates, as MSVC does; Tezcatl turns that
  off, so an MSVC C++14 project loses no template code.
- Names are qualified with their namespaces and classes and carry their parameter types
  (`geo::area(size_t, size_t)`), which keeps overloads apart. Template parameters are not
  spelled: a function template reads `scaled(T)`.
- Location is where the name is written (for a lambda, its `[`). A function produced by a macro
  is located where the macro is used.
- A function defined in a header is reported once, however many translation units include it.
- Functions in system headers, and in files under the build directory (fetched dependencies,
  generated code), are not part of the project and are not reported.
- **A translation unit that fails to parse is an error**, not a smaller result: the run exits
  non-zero unless `--allow-parse-errors` is given, and every error is printed. Warnings are not
  failures: the project's `-Werror` or `/WX` is overridden while parsing, since a warning is a
  build policy, not unparsed code.

Both GCC-style and MSVC-style (`cl.exe`, clang-cl) compilation databases are supported.

## Lines of code

Each physical line of a file is classified as exactly one of:

| Kind | Meaning |
|---|---|
| **code** | the line contains at least one code character, with or without a comment |
| **comment** | the line contains comment text and no code |
| **blank** | the line contains only whitespace, wherever it appears (inside a comment or a raw string included) |

The classification follows the language's own translation phases rather than pattern matching
on lines:

- **Line splicing comes first.** A backslash immediately followed by a newline joins two
  physical lines before comments are recognised, so a `//` comment ending in a backslash
  continues onto the next line, and `/\` at the end of one line followed by `/` at the start of
  the next opens a comment. Inside a raw string literal splicing is reverted, as the standard
  requires.
- **The splice backslash itself is not code.** A line holding only whitespace and a trailing
  splice backslash is blank; a comment followed by a splice backslash is a comment line.
- **Literals hide comment markers.** `//` and `/*` inside string literals, character literals and
  raw string literals (`R"delim(...)delim"`, with any encoding prefix) are not comments.
  Escapes are honoured, so `"\"/*"` does not open a comment.
- **Digit separators are not character literals.** The `'` in `1'000` or `0x1'FF` continues the
  number.
- A block comment opener does not close itself: `/*/` opens a comment.
- Preprocessor directives are code, and so are regions disabled by `#if 0`: Tezcatl counts what
  is written, not what a particular configuration compiles.
- `\r\n` and `\n` end a line; a UTF-8 byte order mark is ignored; a final line without a
  trailing newline still counts. An unterminated block comment runs to the end of the file.
  Trigraphs are not processed (they were removed in C++17).

`physical = blank + comment + code` for every file.

To audit any count, `tezcatl loc --lines PATH` prints the classification of every line.

### Comparison with cloc

Measured 2026-09-24 with cloc 2.10 on the Catch2 v3.16.0 source tree (416 C/C++ files, both tools
selecting the same files):

| | blank | comment | code |
|---|---:|---:|---:|
| Tezcatl | 13,677 | 7,810 | 54,019 |
| cloc | 13,670 | 7,792 | 54,044 |

405 of the 416 files are counted identically. All differences in the remaining 11 files (25
lines) come from a single rule: cloc counts a trailing splice backslash as code, so a line such
as `    \` inside a multi-line macro, or `/* note */ \`, is code to cloc and blank or comment to
Tezcatl. Each of the 11 files was checked line by line against this explanation.

## Modules

A **module** is a named set of files, given by a module map (`--modules FILE`), one rule per line:

```text
# comments and blank lines are ignored
core tests = core/test/**
core       = core/**
io         = io/**
```

Globs are matched against paths relative to `--root`, with `/` separators and case-sensitively:
`*` matches within one path component, `?` one character other than `/`, and `**` any run of
characters including `/` (`src/**/x.c` also matches `src/x.c`). **The first matching rule
wins**, so specific rules go first. A file no rule matches belongs to `(unassigned)`: it is
reported under that name, never dropped.

## Cyclomatic complexity

McCabe's cyclomatic complexity, per function (every entry in *Functions*, including each
lambda):

**complexity = 1 + the number of decision points written in the function's definition.**

| Counts 1 each | Does not count |
|---|---|
| `if` (so `else if` counts once, as its `if`) | `else`, `default`, `try`, `goto`, `return` |
| `for`, range-based `for`, `while`, `do` | the `while` that ends a `do` loop |
| `case` (each label, including stacked labels) | an overloaded `operator&&` or `operator\|\|` (a function call, which does not short-circuit) |
| `catch` (each handler) | `&&` in a declaration such as `int&& r` |
| `&&`, `\|\|` and their spellings `and`, `or`, including a fold expression `(pack && ...)` (one operator as written) | preprocessor conditions (`#if a && b`), and code the preprocessor disabled |
| `?:`, and the GNU `a ?: b` | decisions inside a lambda or a local class's member function: those are functions of their own |

- **Definition** means everything from the start of the declaration to the closing brace, so a
  constructor's member initializers (`: v(a > 0 ? a : 0)`) count, and so do default arguments.
- `if constexpr` counts: it is written as a decision, whichever branch a given instantiation
  keeps. Function templates are measured once, as written, not per instantiation.
- **Macros: what is written counts, not what expands.** A decision written in a macro's
  *arguments* counts (`CHECK(a && b)` adds 1 for the `&&`); a decision inside a macro's
  *body* does not (the `if` that `CHECK` expands to adds nothing), because it is not written in
  the function and would otherwise make one line of code differ by platform (`assert` expands
  to a branch in some C libraries, to nothing under `NDEBUG`). A function whose whole
  definition comes from a macro has complexity 1.
- In a template, `a && b` whose operands depend on a template parameter counts, even where an
  overloaded `operator&&` is visible and the choice waits for instantiation: as written, it is a
  logical operator. A call written as `operator&&(a, b)` is a call.
- Each decision point is found as a token (`if`, `&&`, `?`, ...) and **confirmed by the AST**:
  the token must belong to the construct it spells (an `if` statement, a built-in logical
  operator, a conditional expression). Neither alone is enough: the tokens include `&&` in
  `int&& r` and the `while` of a `do` loop, and the AST includes what macro bodies expand to.

**Thresholds.** A function whose complexity is **over 10 is flagged**, and **over 20 is high**
(both configurable with `--flag-over` and `--high-over`, and printed with every run). The
`rating` column is `ok`, `flagged` or `high`.

**Per module** (`tezcatl functions --summary`): the number of functions, the mean, the median
(the mean of the two middle values when the count is even), the **90th percentile by nearest
rank** (the value at position ⌈0.9 × n⌉ in ascending order, so always a value that occurs),
the maximum, and how many functions are flagged (including high) and high. A final `TOTAL` row
covers every module.

## Include dependencies

`tezcatl includes` builds the `#include` graph at file level. Each directive is resolved by the
compiler's own include search, with the translation unit's flags, so `"x.h"` resolves to the
file the build actually uses.

- **Nodes** are the project's files: every source file in the compilation database and every
  file an edge reaches. System headers, files outside `--root` and files under the build
  directory are not nodes, and edges to them are dropped.
- **An edge** is an `#include` directive from one project file to another. Repeats (the same
  directive seen from several translation units, or a file included twice) are one edge.
- **Include guards and `#pragma once` do not hide edges.** When a header is skipped because it
  was already included, the directive that tried to include it is still an edge. Tezcatl reads
  the directives from the preprocessing record, not from the files that were entered.
- The graph is the one the preprocessor saw with the project's flags: an `#include` inside an
  `#ifdef` that the configuration disables is not an edge.
- **Fan-in** of a file is the number of distinct project files that include it; **fan-out**, the
  number it includes.
- **A cycle** is a strongly connected component of more than one node (Tarjan's algorithm): a
  set of files that each reach every other by following includes. Cycles are numbered from 1 in
  the order of their first member, and each lists its members in path order.
- **At module level**, every file edge is an edge between the modules of its two files. The
  **coupling matrix** counts file edges for each pair of modules, including a module with
  itself. Module fan-in, fan-out and cycles ignore edges within a module, since a module that
  includes its own headers does not depend on itself. A module cycle can exist without any file
  cycle: `a/x.h → b/y.h` and `b/z.h → a/w.h` make modules `a` and `b` depend on each other.

`--output` selects the table: `edges`, `files` (fan-in, fan-out, cycle per file), `modules`,
`cycles`, `coupling`, or `dot` (the file graph in Graphviz DOT, clustered by module, with the
edges inside a cycle drawn in red).
