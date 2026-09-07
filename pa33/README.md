## CPPGM Programming Assignment 33 (`lowir2native -O*`)

### Overview

PA33 adds machine-backend optimization levels to `lowir2native` and to the
shared native backend used by `cppgm++` object and link-driver paths.

PA32 optimizes LowIR before backend lowering. PA33 starts after that boundary:
LowIR has already been translated into machine IR, and the backend must improve
the generated native path while preserving program behavior.

The questions for this assignment are:

- can `lowir2native -O1` perform local machine-IR cleanup?
- can `lowir2native -O2` perform whole-function machine-IR cleanup?
- can `lowir2native -O3` carry the maximum optimization level through the
  same optimized machine path?
- can all levels preserve debug metadata and generated program behavior?
- can the optimized machine-IR path stay reusable by `cppgm++` rather than
  becoming a standalone `lowir2native` shortcut?

### How PA33 Is Specified

This README separates three kinds of statement, and the course suite is
built the same way.

- The **contract** is what every implementation must do: the command line,
  the machine-IR dump it writes, the ABI, unwinding and debug facts it must
  keep, and the behaviour of every generated program.  Command Line, Output
  Format, Error Handling and Validation Modes are the contract.
- The **quality bar** is what the optimized output must reach on the course
  fixtures: the program behaves, the machine IR stays inside each fixture's
  size envelope (`x.ref.expect`).  The Quality Bar section is
  normative.
- **One design** is how the course solution meets that bar: its reactive
  placement, its planner, and the placement decisions in the linked
  design notes. Those notes are a worked example.  A different
  allocator that meets the bar is a correct PA33; a course fixture never
  compares your placement decisions with the course solution's.

Run `make test` from this assignment directory to check the course contract.
See [Testing and references](../TESTING_AND_REFERENCES.md) for selecting tests.

### Prerequisites

You should complete:

- PA24 for the baseline `lowir2native` backend
- PA32 for the explicit LowIR optimization stage

Reuse the PA8 LowIR input language, including explicit role metadata such
as `[role=entry]` where required.

### Starter Kit

The starter kit supplies:

- `pa33/Makefile`
- `pa33/lowir2native.cpp`, linked to the editable `dev/lowir2native.cpp`
- your cumulative `dev/lowir2native.cpp` implementation from PA24
- shared machine-IR and native backend support under `dev/src/`
- optional typed machine-IR model scaffolding in
  `dev/src/mir_model.h`, with shared register support in
  `dev/src/native/mir/registers.h`
- test directories under `pa33/tests/`
- harness scripts under `pa33/scripts/`
- checked-in structural machine-IR and generated-program oracle sidecars

The expected implementation work is in `dev/lowir2native.cpp` and the shared
machine-IR/native backend modules under `dev/src/`, especially machine-IR
optimization and object-generation plumbing. Reuse your earlier LowIR reader,
native backend, object writer and linking support.

Use `lowir2native-ref` to inspect example output. Tests run your backend and check the contract sidecars.

### Command Line

PA33 requires these invocations:

```sh
lowir2native -O1 -o <program> <lowirfile>...
lowir2native -O2 -o <program> <lowirfile>...
lowir2native -O3 -o <program> <lowirfile>...
lowir2native -O1 --dump-machine-ir <mirfile> <lowirfile>...
lowir2native -O2 --dump-machine-ir <mirfile> <lowirfile>...
lowir2native -O3 --dump-machine-ir <mirfile> <lowirfile>...
lowir2native -O1 --dump-machine-ir <mirfile> -o <program> <lowirfile>...
lowir2native -O2 --dump-machine-ir <mirfile> -o <program> <lowirfile>...
lowir2native -O3 --dump-machine-ir <mirfile> -o <program> <lowirfile>...
```

`--help` and `-h` print usage information and exit successfully.

The `--target <target>` option is inherited from the native backend. Tests may
set the target through the harness environment, but the optimization
contract is independent of host-specific elapsed time.

`-O0` remains the PA24 baseline. PA33 must preserve that earlier behavior while
adding the explicit `-O1`, `-O2`, and `-O3` backend optimization levels.

When `cppgm++` emits native objects or executables at an optimization
level, it should use this same backend optimization pipeline after PA32 LowIR
optimization has produced the LowIR program to lower. Do not implement PA33 as
a display-only `lowir2native` transform that the compiler driver cannot reuse.

For backend investigation, the compile-only driver accepts
`--stats-functions` together with an optimization level.  It writes one
`function_census symbol=...` record per lowered function to standard error.
Each record includes MIR size, movement load/store/copy counts, planned
location grants/releases, planned grants blocked by live parameter or ordinary
value holders, the actual location class of planned definitions, spills, and
frame-home counts.  These contention fields let backend work distinguish a
value that was never planned from a plan that could not claim its selected
register.  The records are diagnostic data: tests validate their structure
and function coverage, not timings or exact counter values.

For every final-MIR natural loop, the same option also writes a
`loop_census symbol=...` record.  A natural loop is identified by a CFG
backedge whose header dominates its latch; arbitrary backward branches are
not loops.  The record identifies the stable header block and reports the
loop's member-block and MIR counts, calls, exception operations, nesting
depth, frame operands, function frame bindings, and callee-saved register
count.  A compiler-created block without a presentation label uses `block_`
followed by its numeric identity.  This is a structural view of loop pressure:
consumers may compare shapes or select a reducer, but must not depend on exact
counter values from an unrelated program.

### Output Format

With `--dump-machine-ir <mirfile>`, `lowir2native` writes the optimized machine
IR dump to `<mirfile>`.

With `-o <program>`, `lowir2native` writes a native executable to `<program>`.
When both options are present, both outputs must be produced from the same
optimized machine-IR program.

`--dump-machine-ir` is the serialized view of the machine-IR program that the
native backend consumes. The object/native path may keep the MIR in memory, but
it should not use a different hidden representation with extra backend facts
that the MIR dump cannot express.

The same rule applies when the machine backend is reached through `cppgm++`:
optimization, object writing, and executable writing should consume the same
machine-IR facts that `--dump-machine-ir` can serialize.

The harness judges generated programs by execution and checks MIR against
fixture expectations. Most fixtures allow different instruction sequences;
Validation Modes describes the few required contract shapes.

Call argument-use, stack-argument, variadic, unwind, and no-return annotations
described by PA24 remain part of optimized MIR.  A machine optimization may
remove argument setup only when the surviving call annotation no longer names
the removed register use.  It must preserve stack and exception-boundary facts.
Indexed memory operands likewise retain both register uses and their scale;
copy propagation must rewrite or invalidate the base and index independently.

### Error Handling

The tool must fail with a nonzero exit status when:

- neither `-o` nor `--dump-machine-ir` is provided
- an option requiring a path is missing that path
- there are no input files
- an input file cannot be read
- the input is not valid LowIR
- the target is unsupported
- a requested output file cannot be written
- native code generation or linking fails

For failure cases, exact diagnostic text is not part of the grading
contract. Output files after a failed run are undefined.

### Quality Bar

At every level the generated program must behave as at `-O0`: the same
standard output and exit status on every fixture, and the same debug and
unwind facts where the debuginfo and exception fixtures check them.  What
the levels do to the machine IR is your design.

What the course fixtures then check, for each `x.t`:

- the tool's exit status agrees with `x.ref.impl.exit_status`;
- the generated program's standard output and exit status agree with
  `x.ref.program.stdout` and `x.ref.program.exit_status`;
- when `x.ref.expect` is present, the machine-IR dump meets every predicate
  in it (`../scripts/expect_ir.pl`): a size envelope of instructions,
  blocks, calls, memory operands, pushes and per-function sizes, with a
  32-byte frame allowance, generated from the course solution's dump at
  10% tolerance plus one, and outcome
  lines where a fixture states one;
- when `x.ref.expect` is absent, the fixture's shape is its contract and
  the dump is compared with `x.ref.cmir` after canonicalization
  (Validation Modes).  Those fixtures are the exception, unwind, volatile
  and callee-saved cases, where a debugger, an unwinder or the ABI depends
  on the shape.

The additional `make test-perf` diagnostic builds the behavior programs and
compares their Cachegrind instruction counts with the supplied `.ref.ir`
envelopes (10% tolerance). It requires Valgrind and is skipped without it.
The assignment’s completion check is the course suite described below.

### Design example

[Design notes](design-notes.md) describe the course solution’s passes and
implementation choices. They are a worked example, not additional requirements.

### Validation Modes

Every lane records the tool's exit status and the generated program's
standard output and exit status, and compares them with the reference
sidecars.  For the machine-IR dump:

- a fixture with `x.ref.expect` is judged by that expectation (Quality
  Bar), whichever lane it is in; every course fixture that is not a
  contract-shape case has one;
- a fixture in an `o1`, `o2` or `o3` lane without one is compared with
  `x.ref.cmir` after canonicalization, which renames the free general
  registers (`rbx`, `r10` to `r15`) and free vector registers (`xmm2` to
  `xmm7`) in first-use order, renames frame displacements in first-use
  order, and hides the frame's total size and code alignment; instruction
  selection, instruction order and block order must match;
- a fixture in a `behavior` lane without one is compared with the raw
  `x.ref.mir` when that file exists (the strict mode).

Exact textual machine-IR matching is therefore part of the contract only
for the contract-shape fixtures named in Quality Bar.

### Testing

From this assignment directory, run:

```sh
make test
```

Before moving on, run `make test-report-through-pa33` from the repository root.

`make test` runs:

- `tests/o1`
- `tests/o2`
- `tests/o3`
- `tests/behavior/o1`
- `tests/behavior/o2`
- `tests/behavior/o3`
- `tests/controls/native` for builtin and bulk-copy behavior at O0 and O1
- `tests/driver` for the `cppgm++ --stats-functions -c` diagnostic


Run the debug metadata preservation lanes with:

```sh
make test-debuginfo
```

`make test-debuginfo` runs:

- `tests/debuginfo/o1`
- `tests/debuginfo/o2`
- `tests/debuginfo/o3`

These directories are organized by backend role and validation mode, not by
N3485 source-language clauses.

- `tests/o1` runs `lowir2native -O1` over LowIR inputs and checks local
  backend cleanup.
- `tests/o2` runs `lowir2native -O2` over LowIR inputs, repeats the `-O1`
  surface, and adds O2-only layout and frame cleanup cases.
- `tests/o3` runs `lowir2native -O3` over LowIR already optimized by PA32 and
  checks that the O2 machine path remains in use.
- `tests/behavior/o1`, `tests/behavior/o2`, and `tests/behavior/o3` check
  successful lowering and generated-program behavior where several valid
  machine-IR layouts are possible. These tests do not compare a machine-IR
  oracle.
- `tests/debuginfo/o1`, `tests/debuginfo/o2`, and `tests/debuginfo/o3` run
  equivalent machine-IR rewrite cases carrying `!dbg(...)` metadata.

Each test records compilation status and generated-program output. The
Validation Modes section defines its MIR checks. Failed compilations are
judged by exit status; diagnostic wording is not prescribed.

### Out Of Scope

PA33 does not require:

- changing the LowIR optimizer from PA32
- redefining `lowir2native -O0`
- source-language semantic changes
- wall-clock performance grading
- unbounded graph-coloring register allocation or a private virtual-register
  IR that is absent from the machine-IR dump
- instruction scheduling, vectorization, or target-specific peephole work not
  covered by the tests
- interprocedural backend optimization

### Handoff

PA34 uses the PA32 LowIR optimizer and the PA33 machine-backend optimizer as
part of the self-host ladder. By the end of PA33, optimized and unoptimized
native paths should remain deterministic enough for staged self-host builds and
test reruns.
