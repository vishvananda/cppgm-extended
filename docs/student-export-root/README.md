# cppgm assignments

Build a C++ compiler in 36 cumulative assignments, from preprocessing to
native code, optimization and self-hosting. Later handouts identify the
required language and hosted-library profile; see
[LANGUAGE_PROFILE.md](LANGUAGE_PROFILE.md). Start with
[PA1](pa1/README.md) and extend the same implementation in `dev/` and
`dev/src/` throughout.

This student repository contains scaffolds, handouts, tests, references and
wrappers that download pinned reference tools. The complete course solution
is maintained in [cppgm-extended](https://github.com/vishvananda/cppgm-extended).

## Environment and workflow

Use Linux x86-64 with a C++11 compiler (normally `g++`), GNU make, Bash, Perl,
Python 3, `curl` or `wget`, and standard archive/checksum tools. Later native
and debug checks need binutils and a debugger. Generated programs execute
locally; use an isolated environment for autonomous agents.

For assignment N, read `paN/README.md`, then:

```sh
make build
make test-paN                 # Iterate on PA N
make test-report-through-paN  # Final validation for PA1–PA33
```

Replace N with the assignment number. The through report includes PA N;
no separate PA N check is needed.
[PA34](pa34/README.md) runs the self-host ladder and ends with `make inception`,
comparing the self and inception generations.
[PA35](pa35/README.md) makes the compiler fast and ends with
`make -C pa35 test`, its own gates on generated code and compile speed.
Integrated assembly is required after PA28 and before hosted compatibility,
and runs in normal assignment reports.
Later milestones extend your existing compiler; do not reset to the initial
scaffolds.

Reference tools download when needed. `make reference-binaries` fetches them
ahead of time. Personal test inputs and harnesses can live in `student.tests/`.

See [PROJECT_LAYOUT.md](PROJECT_LAYOUT.md) for the assignment map,
[TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for test and reference
policy, and [AGENTS.md](AGENTS.md) for coding-agent instructions. The
[offline reference guide](doc/REFERENCES.md) covers the C++11/14/17 drafts. The [offline CWG reference index](doc/cwg/INDEX.txt)
collects the cited decisions and related papers, including later corrections.

## Origins and license

This is an independently maintained adaptation of the original C++ Grandmaster
Certification course. The cppgm-extended authors substantially edited,
combined, renumbered and extended the inherited handouts, tests and starter
code. Current PA numbers and reference tools belong to this 36-assignment
version of the course.

Copyright 2026 The cppgm-extended Authors. Project-maintained code and materials
use the Apache License 2.0; inherited material retains its original notices.
See [LICENSE](LICENSE), [NOTICE](NOTICE) and [AUTHORS](AUTHORS).
