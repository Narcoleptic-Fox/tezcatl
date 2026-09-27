---
title: Testing
description: How Tezcatl is checked, and why a passing test is not trusted until it has been made to fail.
nav_order: 1
---

# Testing

A metrics tool that is subtly wrong looks exactly like one that is right. Tezcatl is therefore
checked in layers, and no check is trusted until it has been seen to fail.

## The layers

| Layer | What it checks |
|---|---|
| Unit tests (Catch2) | each rule in isolation, with expected values worked out by hand before the code ran |
| End-to-end tests (CTest) | the built binary on fixture projects: its output, its files and its exit code |
| Oracles | real data checked against independent tools: gcovr and llvm-cov for coverage, lizard for complexity |
| Sanitizers | the whole suite under AddressSanitizer with UBSan, and under ThreadSanitizer for the parallel scan |
| Static gates | clang-format, clang-tidy and cppcheck, each fatal in CI |
| Self-review | Tezcatl's report on itself, with its own coverage, on every push |

CI runs all of them on MSVC, GCC and Clang.

## Sabotage

Every test was made to fail once: the code it guards was broken on purpose and the test had to turn
red, while a change that alters nothing had to leave everything green. A test that stayed green was
itself the bug, and was fixed until it could fail.

The log is [`tests/SABOTAGE.md`](https://github.com/Narcoleptic-Fox/tezcatl/blob/main/tests/SABOTAGE.md):
142 deliberate breaks, the test that caught each one, and the tests found this way that could not
fail at first.

## Performance changes

A faster version that changes one figure is a bug. Each speed-up was run on Earthworm's 929 units at
1 and at 16 threads, and all twelve report files were compared byte for byte with the run before it.

## Fixtures and their oracles

Fixture expectations come from outside Tezcatl: counted by hand, or given by another tool. The
coverage fixtures carry the output of gcc, llvm-cov and gcovr from one run, and their READMEs record
the commands that generated the data and the figures the other tools gave.
