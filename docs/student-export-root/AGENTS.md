# Agent Instructions

This repository is a staged C++11 compiler project for Linux x86_64. Work
assignment by assignment, PA1 through PA37.

Before changing code, read:

- [README.md](README.md) for the student workflow
- [PROJECT_LAYOUT.md](PROJECT_LAYOUT.md) if you need the repository map
- [TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for test and ref rules
- the target `paN/README.md` for the assignment contract

## Core Rules

- Put implementation changes in `dev/` and `dev/src/`.
- Treat `paN/` directories as handouts, harnesses, tests, refs, scripts, and
  wrappers unless the assignment explicitly says otherwise.
- Reuse and extend earlier assignment code. Do not restart from scratch for a
  later PA.
- Prefer real semantic fixes over test-specific workarounds.
- Add small regression tests for bugs or new behavior.
- Do not hardcode answers for specific tests.
- Do not shell out to reference binaries, previous solutions, or host compilers
  to produce required compiler output unless the PA handout explicitly makes
  host-toolchain interaction part of the assignment.

## Tests

For PA N, use this loop:

```sh
make test-paN
make test-report-through-paN
```

For PA1 through PA36, the exit criterion for each assignment is a clean root
`make test-report-through-paN`. Do not move on after only running
`make test-paN`.

PA37 uses inception instead: run root `make inception`, which is wired to the
PA37 `compare-cppgm++-inception` path.

## References

Reference outputs and exit-status sidecars are the oracle. Reference binaries
such as `pptoken-ref` or `cppgm++-ref` are for observation and fixture
regeneration only.

Do not edit tests or `.ref` files to hide incomplete behavior. Ref regeneration
must use the provided `*-ref` binaries through the documented `ref-test`
targets.
