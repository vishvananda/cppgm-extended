# Testing and references

Run these commands from the repository root, replacing N with the assignment
number:

```sh
make build
make test-paN
make test-report-through-paN
make test-report
make inception
```

For PA1–PA33, a clean through-milestone report is the exit criterion. PA34 ends
with inception. `make test` builds once and runs all assignment suites;
`make test-report` keeps going and summarizes failures. Use
`ACTIVE_TEST_REPORT_PAS='pa8 pa10'` to narrow a report.

## Local checks and toolchains

```sh
make -C pa8 check TEST='tests/spec/100-*.t'
make -C pa8 check TEST='tests/behavior/100-sum.t tests/behavior/200-swap.t'
make CXX=g++ CPPGM_HOST_CXX=g++
```

`TEST=` accepts a path, quoted glob or quoted list; mixed-input assignments
route each fixture to its runner. Finish with the root through target.
Keep a real host compiler in `CPPGM_HOST_CXX` when PA34 uses
`CXX=../dev/cppgm++`. Use a separate object root for different toolchains.

## Test locations and requirements

- Required fixtures live in `paN/tests/`, in the buckets named by the handout.
  `controls/` holds focused property checks with no complete-output oracle.
- Personal inputs and harnesses live in `student.tests/`; run them explicitly.
  Default assignment targets do not discover that directory.
- PA24, PA32 and PA33 also ship `tests/regression/`, which pins the course
  solution's particular design. Run it explicitly with `make -C paN
  test-regression` if useful for that design. It is outside the student exit
  criterion; default `make test` and the through reports run the course contract.

Permanent fixtures belong in the earliest assignment owning every feature
used. From PA5 onward, filenames use a three-digit feature-cluster prefix
(`100-`, `200-`, ...); PA1–PA4 use finer subgroups. Controls and solution
regressions follow their own local naming. Keep required rejection tests in the
owning suite; omit inputs whose outcome the assignment leaves unspecified.

## References

Use the oracle named by the handout: output, exit status, a focused property,
or a code-quality envelope. Diagnostic text is not compared. Informational
LowIR/MIR sidecars do not become grading requirements merely because they ship.

PA8 requires a LowIR model, reader/validator, writer and three construction
exercises. The harness uses `lowir-ref` for roundtrips and `lowir2native-ref`
to execute the student's constructed LowIR. Behavior checks accept alternative
valid LowIR and grade program outcomes. The student compiler must implement
LowIR construction and later C++ lowering itself.

Reference wrappers download and verify a pinned bundle built from the current
cppgm-extended solution. They provide examples and regenerate fixtures; they
are not the original CPPGM binaries. Fetch them eagerly or regenerate with:

```sh
make reference-binaries
make ref-test-paN
make -C paN ref-test TEST='tests/path/to/case.t'
```

These targets fail if the reference tool cannot be downloaded or verified;
they never fall back to the student's implementation. References can have bugs
on untested inputs: prefer the handout and the C++11 standard over exact parity
there. Do not edit existing fixtures or references to hide incomplete behavior.

Failed-case stdout is an informational example regenerated on the Linux
export host. Successful stdout and required exit-status sidecars remain
oracles. [The LowIR specification](pa8/lowir.md) explains normal comparison's
presentation tolerance; byte-exact LowIR comparison is a maintainer CI check.

## Debug and inspection checks

Run the targets required by the handout when changing debug, object or link
behavior. The broad debug targets are `make test-debuginfo` and
`make ref-test-debuginfo`. Reference regeneration maintains fixtures from the
supplied reference tools; it is not a way to make an incorrect implementation pass.
