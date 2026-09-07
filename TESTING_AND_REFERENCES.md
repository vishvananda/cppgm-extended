# Testing and references

This guide is for the compiler solution repository. The student export has
[its own guide](docs/student-export-root/TESTING_AND_REFERENCES.md) and pinned
downloadable reference tools. Here, every `*-ref` wrapper invokes the matching
tool built from `dev/`.

## Run tests

Run from the repository root:

```sh
make
make test-pa8
make test-report-through-pa10
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
make test-debuginfo
make test-variants
make test-harness
make inception
```

The report covers PA1–PA33; PA34's final check is inception. Narrow the report
with `ACTIVE_TEST_REPORT_PAS='pa8 pa10'`. Control parallelism with
`TEST_REPORT_ASSIGNMENT_JOBS` and `TEST_REPORT_SUBTEST_JOBS`; `ORDERED=false`
streams completed jobs. Use `CPPGM_TEST_RUNNER=0` to diagnose ordinary versus
batch-runner behavior. Architecture audits and self-host checks are listed in
[AGENTS.md](AGENTS.md).

For quick iteration:

```sh
make -C pa8 check TEST='tests/spec/100-*.t'
```

`TEST=` accepts a path, quoted glob or quoted list. Mixed-input assignments
route each selected fixture to its owning runner.

## Fixture ownership

Every required fixture lives under its owning `paN/tests/`. From PA5 onward,
the three-digit prefix identifies a feature cluster (`100-`, `200-`, ...).
PA1–PA4 use finer subgroups. The placement auditor checks the latest feature
used; put a reducer in the earliest assignment that owns all its requirements.
`controls/` contains focused property checks. Personal inputs and scratch
harnesses belong in `student.tests/` and are run explicitly.

PA24, PA32 and PA33 also have `tests/regression/`, which pins this solution's
particular output shapes. Maintainer `make test` runs both `test-course` and
`test-regression`; student-export `make test` runs the course contract only.
Keep solution-specific expectations out of graded course fixtures.

## Reference policy

Choose the oracle required by the handout: output, exit status, a focused
property, or a code-quality envelope. Diagnostic text is not graded. PA8's
three construction exercises grade program outcomes through `lowir2native-ref`,
allowing alternative valid LowIR. Informational IR/MIR sidecars are not oracles.

Regenerate references from the built solution:

```sh
make ref-test-pa8
make ref-test
make ref-test-debuginfo
```

Before changing a LowIR reference, distinguish behavior from presentation;
[pa8/lowir.md](pa8/lowir.md) defines what normal comparison absorbs. Maintainer
CI enables byte-exact LowIR comparison. Regeneration on an unchanged compiler
must leave tracked references unchanged. Never change a fixture or reference
merely to hide an implementation failure.

Failed-case stdout is an informational diagnostic example, generated on Linux
by the export and untracked here. Successful stdout and required exit-status
sidecars remain tracked. Export validation regenerates and verifies every
portable reference before packaging the student repository.
