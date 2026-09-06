# Testing And References

Run build and test targets from the repository root unless a PA handout says to
use a narrower command.

## Common Root Targets

```sh
make build
make test
make test-paN
make test-report-through-paN
make test-report
make inception
```

- `make build` builds the compiler tools in `dev/`.
- `make test` builds once, then runs the assignment tests.
- `make test-paN` runs one assignment.
- `make test-report-through-paN` runs the report suite through PA N.
- `make test-report` runs the broad keep-going report.
- `make inception` runs the final PA39 self-host comparison.

The exit criterion for PA N is a clean root `make test-report-through-paN`.

## Assignment-Local Targets

Inside an assignment directory:

```sh
make
make test
make check TEST=tests/path/to/case.t
make check TEST='tests/path/to/100-*.t'
make check TEST='tests/path/to/a.t tests/path/to/b.t'
```

`TEST=` accepts one checked-in test file, a quoted shell glob, or a quoted
space-separated list of files and globs. Later assignments with multiple test
kinds can route mixed entries to the right local runner, for example a
preprocessor case plus a compile case. Use local targets for quick iteration,
then return to the root through target before considering the assignment
complete.

## Compiler Selection

You may pass compilers explicitly:

```sh
make CXX=g++ CPPGM_HOST_CXX=g++
```

For normal host builds, `CXX` and `CPPGM_HOST_CXX` should usually match. For
PA39 self-host work, `CXX` may be `../dev/cppgm++` while `CPPGM_HOST_CXX`
remains a real host compiler.

## Test Locations

- Every test an assignment runs lives in `paN/tests/`.  Later PA handouts
  describe the buckets under it (strict, structural, behavior, debuginfo,
  object-inspection, link, driver).
- `paN/tests/controls/` holds focused property checks that have no complete
  reference output; the assignment's Makefile routes each to its checker.
- `paN/tests/regression/` (PA29, PA37 and PA38) pins the course solution's
  own outputs and its pass-specific controls.  It runs with `make test` and
  with `make test-regression`, but it is not part of the assignment's
  contract: a fixture whose only justification is the shape the course
  solution produces belongs there, never in a graded bucket, and a
  different design that meets the quality bar is expected to fail it.
- `cppgm.tests/undefined/paN/` keeps inputs whose outcome the course leaves
  unspecified.  No lane runs them.

Add focused regression tests for new bugs under the `paN/tests/` of the
earliest assignment that owns the behaviour, in the numeric group of the
feature the test exercises.

## Fixture Numbering

A fixture's name starts with a three-digit number.  From PA10 on the number
is the cluster of the feature the fixture exercises, a multiple of one
hundred (`100-`, `200-`, ...); the assignment's handout describes what each
cluster covers, and `scripts/audit_pa_feature_placement.py` checks that a
fixture sits in the cluster of the latest feature it uses and in the
assignment that owns it.  PA1 to PA9 number more finely, with the tens
naming a sub-group inside a cluster.  The regression lane and the controls
are outside the audit and keep whatever number says most about them.

Do not create a dormant proposed or candidate test tree. When a new course
requirement intentionally changes a checked-in semantic, LowIR, MIR, or object
contract, update the authoritative reference and put the reducer directly in
the earliest owning course suite. Correct an existing assignment fixture and
regenerate its reference in place when that fixture already owns the behavior.

Deliberately ill-formed inputs belong in the active owning suite when rejection
is required; name them as negative tests and compare failure status. Source
that relies on implementation-reserved identifiers is not a portable course
requirement and should be rewritten or removed.

## References

Reference outputs, stdout refs, exit-status refs, and inspect refs are the test
oracle. Do not edit them to hide an incomplete implementation.

Some assignment handouts retain informational output that is not a grading
oracle. In particular, PA29 behavior tests include the reference MIR so
students can inspect it, but grade only compiler and generated-program
outcomes. The owning README identifies which sidecars are informational.

Reference binaries such as `pptoken-ref` or `cppgm++-ref` are provided for
observing expected behavior and regenerating reference fixtures. They must not
be used by your compiler implementation.

The reference binaries are not perfect. Only checked-in fixtures gate an
assignment. Synthesizing new inputs is fine, but reference behavior outside
the test suites is intended to be correct, not guaranteed; prefer the handout
and the standard over exact reference parity. Error message text is never
compared: failing tests check only the exit status.

Failed-case stdout files are generated on the Linux export host and retained
as informative examples for students. They are not portable reference oracles
and are therefore not tracked in the compiler source repository. Successful
stdout files remain exact checked-in references.

The repository does not store the large binary payloads in Git. The checked-in
`*-ref` wrappers automatically download, verify, and unpack the pinned
reference-binary bundle the first time a reference tool is needed. To fetch the
bundle before running a ref target, use:

```sh
make reference-binaries
```

The ref regeneration targets use the provided reference binaries:

```sh
make ref-test-paN
make -C paN ref-test
```

These targets intentionally fail if the needed reference binary cannot be
downloaded or verified. There is no fallback to the implementation under test.

## Debug Fixtures

Some later assignments include optional debug-info fixtures or object/link
inspection checks. Run the targets named in the PA handout when you touch that
surface. The broad root commands are:

```sh
make test-debuginfo
make ref-test-debuginfo
```

Reference regeneration is for maintaining fixtures from the provided reference
tools, not for making current incorrect output pass.
