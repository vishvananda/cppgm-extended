# cppgm

`cppgm` is a staged C++ compiler project. This repository contains the active
compiler implementation, assignment directories, shared tests, validation
scripts, and documentation used to maintain the self-hosting toolchain.

Implementation work lives in `dev/`. Assignment directories `pa1` through
`pa37` contain handouts, tests, references, and milestone wrappers.

This repository can also produce the student-facing assignment export. The
published assignment repository is
[vishvananda/cppgm-assignments](https://github.com/vishvananda/cppgm-assignments).

## Project Background

This project grew out of the C++ Grandmaster Certification, a staged compiler
course that originally walked through PA1-PA9: preprocessing, parsing, and a
small backend path. The current repository continues that shape through PA37,
adding the missing compiler layers: AST construction, semantic analysis,
templates, typed IR, native object generation, linking, runtime support, and a
self-hosting ladder.

The repository is also an experiment in long-running agentic software
development. The assignment boundaries, reference outputs, regression tests,
inception builds, and performance gates are designed to give an autonomous
coding agent a tight loop: make a change, run the compiler, reduce failures,
and keep moving without losing correctness at the milestone boundaries. The
story behind the project is in the blog series starting with
[I Spent 2 Billion Tokens Writing a C++ Compiler So You Don't Have To](https://medium.com/@vishvananda/i-spent-2-billion-tokens-writing-a-c-compiler-so-you-dont-have-to-d3e4eec4781e).

## License

Copyright 2026 The cppgm-extended Authors.

The project code and repository-maintained materials are licensed under the
Apache License, Version 2.0. See [LICENSE](LICENSE) and [AUTHORS](AUTHORS).
See [NOTICE](NOTICE) for third-party and archival attribution.

## Archival Note

PA1 through PA9 include assignment material from the CPP Grandmasters challenge.
The original site appears to be defunct, so these materials are preserved here
for archival and continuity purposes. See [NOTICE](NOTICE) for the PA1-PA9
copyright attribution and removal contact note.
