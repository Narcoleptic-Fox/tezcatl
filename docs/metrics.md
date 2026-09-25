# Metric definitions

Every number Tezcatl reports is defined here precisely enough to be reproduced by hand. Where
a definition involves a choice that other tools make differently, the choice and its reason are
stated, together with a measured comparison against an independent tool.

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
