# Testing and references

Run commands from the repository root, replacing N with the assignment number:

```sh
make build
make test-paN                 # Iterate on PA N
make test-report-through-paN  # Final cumulative report
make test-report              # Full assignment report
make inception                # PA35 completion
```

The report covers PA1–PA34 and PA37. The through report includes PA N;
no separate PA N check is needed.
PA36 ends with `make -C pa36 test`, which is not part of the report: its gates compare your compiler with the host and
its own seed.
Assembly is required before hosted compatibility and runs in normal reports.
PA37 needs the pinned Wasm runtime and validator; follow its installation steps.
Suites select platform-specific checks automatically and list skips separately
from passes. A fixture's `.platforms` sidecar limits the whole test;
`.inspect.platforms` limits only inspection, keeping compilation and execution.

## Local checks and toolchains

```sh
make -C pa8 check TEST='tests/spec/100-*.t'
make -C pa8 check TEST='tests/behavior/100-sum.t tests/behavior/200-swap.t'
make CXX=g++ CPPGM_HOST_CXX=g++
```

`TEST=` accepts a path, quoted glob or quoted list; mixed-input assignments
route each fixture to its runner. Finish with the root through target.
Keep a real host compiler in `CPPGM_HOST_CXX` when PA35 uses
`CXX=../dev/cppgm++`. Use a separate object root for different toolchains.

Each `make` test invocation gives fixture results a unique run ID, inherited by
its recursive makes, runners and comparers. A full report and a focused check
can use the same fixtures concurrently: their files are named
`x.my.<run-id>`, `x.check.<run-id>` or `x.pp.<run-id>`, with sidecars sharing
the prefix. Report totals and failure markers are isolated too. For predictable filenames, pass
`CPPGM_TEST_RUN_ID=debug`; an explicit empty value keeps the plain `.my` and
`.check` names for manual workflows. Reference filenames stay fixed. Report workers inherit the caller's environment
without running shell login scripts.

## Test locations and requirements

- Required fixtures live in `paN/tests/`, in the buckets named by the handout.
  `controls/` holds focused property checks with no complete-output oracle.
- Personal inputs and harnesses live in `student.tests/`; run them explicitly.
  Default assignment targets do not discover that directory.
- Default `make test` and the through reports run the course contract;
  additional compatibility material does not add completion requirements.

PA34 checks executable behavior, declared MIR bounds, debug locations and
kernel instruction counts. It requires no allocator statistics or exact
optimized MIR match.

From PA5 onward, filenames use a three-digit feature-cluster prefix
(`100-`, `200-`, ...); PA1–PA4 use finer subgroups. Required controls follow
the same prefixes and belong in the earliest assignment owning every feature
their source and checker require. Required rejection cases are in the
owning suite; unspecified inputs do not add requirements.

A graded input is an `x.t` file in the tool's input language (companions
`x.t2`, `x.t.1`, ... as the handout describes); its reference outputs are
`x.ref` and `x.ref.*`; a control is a `.cpp` program. A leading
`// VALIDATION: ...` or `// N3485 focus: ...` comment in a fixture is
informational. `CPPGM_BATCH_TESTS=1` runs an assignment's cases through a
worker linked into each tool (`dev/src/support/testing/test_runner.cpp`),
which requires each tool's `main` to live in `dev/<tool>.cpp`; the ordinary
path runs the tool once per case.

## References

Use the oracle named by the handout: output, exit status, a focused property,
or a code-quality envelope. Diagnostic text is not compared. Informational
LowIR/MIR sidecars do not become grading requirements merely because they ship.
An expected `EXIT_FAILURE` is a rejection: a tool that crashes, or exits with
status 70 (an internal compiler error), records `EXIT_SIGNAL` or
`EXIT_INTERNAL_ERROR` and fails the case.

PA8 requires a LowIR model, reader/validator, writer and three construction
exercises. The harness uses `lowir-ref` for roundtrips and `lowir2native-ref`
to execute the student's constructed LowIR. Behavior checks accept alternative
valid LowIR and grade program outcomes. The student compiler must implement
LowIR construction and later C++ lowering itself.

From PA10 through PA23, a control whose leading comment says `expect exit N`
or `expect reject` (or with an `x.expect` sidecar) is a self-checking program
that executes your source-generated LowIR through the supplied native backend.
`make test` compiles it with your `cppgm++ --emit-lowir -O0`, builds the result
with `lowir2native-ref -O0` and requires that exit status (and the output in
`x.stdout`, when present), or requires your frontend to reject it. A program
killed by a signal never has the stated status, and a frontend that crashes
or exits with status 70 does not reject. Any LowIR with the right behavior
passes. Run one assignment's controls alone with `make -C paN test-controls`.
Your own native backend is introduced in PA24.

A positive control's `.limits` sidecar can bound frontend CPU time, output-file
size and, on Linux, virtual address space. macOS reports the small address-space
gate as a platform skip; correctness and CPU/file limits still run.

Reference wrappers download and verify a pinned bundle built from the current
cppgm-extended solution. They provide examples of the required behavior and
are not the original CPPGM binaries. Fetch them ahead of time with:

```sh
make reference-binaries
```

Reference wrappers fail if a tool cannot be downloaded or verified; they
never fall back to your implementation.

Failed-case stdout is an informational example. Successful stdout and
required exit-status sidecars remain oracles.
[The LowIR specification](pa8/lowir.md#what-the-comparison-absorbs-and-what-it-enforces)
lists what normal LowIR comparison absorbs and what it enforces, including
the facts your lowering states (`unwind`, `return`, `effects`,
`storage=readonly`, `object_bytes` and the others), which PA10–PA23 compare
exactly.

Reference implementations and checked outputs are not guaranteed bug-free.
Preserve them by default. You may correct reference outputs when a reduced
reproducer and cited rules for the selected edition—or the LowIR contract for IR-only cases—prove
them incorrect. Document the proof and bundle revision. Compiler agreement
alone is insufficient. Never weaken required behavior, coverage, or comparison
rules.

## Required language modes

The [language profile](LANGUAGE_PROFILE.md) is required in the owning lessons.
Mode-tagged checks run in normal tests and cumulative reports. A fixture's
`NAME.cxx-standard` selects the same C++11/14/17 mode for implementation and
reference requests; driver cases can use their `.flags` file. No separate
later-language validation command is needed.

An early LowIR control can state `accept` (validate LowIR without linking),
`reject`, or `exit N` (run with the supplied backend) in `NAME.expect`.
`NAME.stdout` checks exact output when supplied. The
[offline drafts](doc/REFERENCES.md) retain edition and defect-report provenance.

PA30 also has an optional `make test-target-features` suite for x86 POPCNT.
It is excluded from required reports; see the PA30 handout for the CPU option.

## Debug and inspection checks

Run the debug, object and link checks required by the current handout.
`make test-debuginfo` collects the later debug suites; it is not an extra
prerequisite for early assignments.

`make test-lowir-library` is another optional lane: PA27 checks an in-process
LowIR producer and native host objects at O0; PA33 adds O1–O3. It uses the shared
PA8 scaffold records and a supplied C frontend, with no external language-tool
installation. See [the library contract](pa27/lowir-library.md). The lane stays
outside normal assignment reports.
