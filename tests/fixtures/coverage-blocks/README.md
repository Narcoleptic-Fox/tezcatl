# Coverage fixture: one line, compiled two ways

`src/range.h` defines `in_range()` on one line using `IN_RANGE`, a macro that `narrow.c` and
`wide.c` define differently. The line therefore compiles to different branches in each unit: 4 in
`narrow.c`, 6 in `wide.c`, and only two of them are the same branch (the same blocks). This is
what gcov's JSON format 2 (GCC 14 and later) can tell apart and format 1 cannot: each branch
carries the ids of the blocks it leaves and enters.

`data/blocks.gcov.json` was written on 2026-09-26 by gcc 14.2.0 (Ubuntu 24.04):

```sh
gcc-14 --coverage -O0 ../src/narrow.c ../src/wide.c ../src/main.c -o blocks && ./blocks
gcov-14 -b --json-format --stdout *.gcda > ../blocks.gcov.json
```

The directory it was generated in has been replaced by `/blocks-fixture`; nothing else was edited.

The expected figures are gcovr 8.6's (`--gcov-executable gcov-14 --merge-lines`), on the same run:

| File | Lines (covered) | Branches (covered) | Functions (covered) |
|---|---|---|---|
| `src/range.h` | 1 (1) | 8 (5) | 1 (1) |
| `src/main.c`, `src/narrow.c`, `src/wide.c` | 2 (2) each | 0 (0) | 1 (1) each |

By hand, from the data: narrow's branches are 2→3, 2→5, 3→4, 3→5 and wide's are 2→3, 2→6, 3→4,
3→6, 4→5, 4→6. Eight distinct branches, of which 2→5, 3→6 and 4→5 were never taken: 8 (5).
Pairing branches by their position on the line instead gives 6 (5), which is what Tezcatl reported
before it read the block ids.
