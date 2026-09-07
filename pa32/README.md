## CPPGM Programming Assignment 32 (`lowiropt`)

### Overview

PA32 adds the first explicit optimization stage to the compiler. The new tool,
`lowiropt`, reads PA8 LowIR text, applies a deterministic optimization
pipeline selected by `-O0`, `-O1`, `-O2`, or `-O3`, and writes LowIR text.

The same LowIR optimizer is also reached from `cppgm++` when source programs
are compiled with `--emit-lowir -O1`, `--emit-lowir -O2`,
`--emit-lowir -O3`, or through the ordinary compile/link driver at an
optimization level.

### How PA32 Is Specified

This README separates three kinds of statement, and the course suite is
built the same way.

- The **contract** is what every implementation must do: the command line,
  the LowIR it reads and writes, the behaviour it must preserve, and the
  object path that must accept its output.  Command Line, Output Format,
  Error Handling and Validation Modes are the contract.
- The **quality bar** is what the output must reach on the course fixtures:
  the outcome and the size envelope each fixture states in its
  `x.ref.expect` sidecar, and the lowering floor.  The Quality Bar section
  is normative.
- **One design** is how the course solution meets that bar, pass by pass,
  with the budgets and caps it chose.  The linked design notes are a worked example.  A different optimizer that meets the bar is a correct PA32; a
  course fixture never compares the shape of your output with the course
  solution's.

Run `make test` from this assignment directory to check the course contract.
See [Testing and references](../TESTING_AND_REFERENCES.md) for selecting tests.

### Prerequisites

You should complete PA31 before starting this assignment.

You will reuse:

- the PA8 LowIR syntax and semantics
- the PA10 through PA31 source-to-LowIR lowering pipeline
- the PA24 native backend, PA25 driver, PA9 ABI naming, and PA26 host-runtime path
- the PA31 hosted compiler driver surface

### Starter Kit

The starter kit supplies:

- `pa32/Makefile`
- `pa32/lowiropt.cpp`, linked to the editable `dev/lowiropt.cpp`
- a `dev/lowiropt.cpp` scaffold based on `dev/lowiropt-scaffold.cpp`
- shared compiler support under `dev/src/`
- test directories under `pa32/tests/`
- harness scripts under `pa32/scripts/`
- checked-in `.ref` and `.ref.exit_status` files for the tests

The expected implementation work is in `dev/lowiropt.cpp` and shared optimizer
or driver support under `dev/src/`, especially the LowIR optimizer and
optimization-level plumbing. Reuse your PA8 LowIR reader/writer and later
driver implementation; the supplied harness does not implement these passes.

Use `lowiropt-ref` to inspect example output.
The harness checks the contract sidecars and quality envelopes.

### Command Line

`lowiropt` accepts exactly one optimization level, one output path, and one or
more LowIR input files:

```sh
lowiropt -O0 -o <outfile> <lowirfile>...
lowiropt -O1 -o <outfile> <lowirfile>...
lowiropt -O2 -o <outfile> <lowirfile>...
lowiropt -O3 -o <outfile> <lowirfile>...
```

`--help` and `-h` print usage information and exit successfully.

PA32 also requires the source driver to route these options through the same
optimizer:

```sh
cppgm++ --emit-lowir -g0 -O1 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -g0 -O2 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -g0 -O3 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -gline-tables-only -O1 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -gline-tables-only -O2 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -gline-tables-only -O3 -o <outfile> <srcfile>...
```

The ordinary `cppgm++ -c` and link-driver paths must also accept `-O0`, `-O1`,
`-O2`, and `-O3` and use the same LowIR optimization level before object
generation. As with GCC and Clang, omitting `-O` selects `-O0`; optimization
must be requested explicitly.
Compile mode must also accept serialized LowIR text as an input:

```sh
cppgm++ -c -O0 -o <objfile> <lowirfile>
cppgm++ -c -O1 -o <objfile> <lowirfile>
cppgm++ -c -O2 -o <objfile> <lowirfile>
cppgm++ -c -O3 -o <objfile> <lowirfile>
```

This LowIR object input mode parses LowIR text, runs the same object-prep and
optimization path used by source object compilation, and writes the same
host-compatible relocatable object format.

### Output Format

`lowiropt` writes LowIR text to `<outfile>`. The output must remain valid LowIR
and must preserve the behavior of every defined input program.

The optimizer works on the same LowIR program representation that the object
path consumes. It may use typed internal data structures, but optimized output
must serialize back to valid LowIR, and object generation at a chosen
optimization level must not require extra semantic facts unavailable from that
optimized LowIR text.

This LowIR/object boundary is required in PA32. A correct compile path
may keep LowIR in memory for speed, but it must not pass private frontend or
semantic side data around the serialized LowIR representation. If object
emission needs a fact after optimization, that fact must either be represented
in LowIR or derived again by the object-lowering layer from LowIR. The direct
`cppgm++ -c` source object and the object produced by `--emit-lowir -O0`
followed by `cppgm++ -c -O<level>` on that LowIR file should therefore match
for the same source, flags, and optimization level.

This durability rule includes PA27 global section placement. A token-safe GNU
section attribute is serialized as global `section=<name>` metadata and must
survive `-O0` through `-O3`. The replayed object must keep the global in the
same named ELF section and keep relocations originating in that section aimed
at the same symbols; direct/replayed byte equality alone is not the feature
definition.

Your output must be deterministic and follow the [LowIR format](../pa8/lowir.md).
Optimization may change instructions and control flow while preserving their
meaning, including initialization, destruction, volatile accesses and cleanup.
`-O0` preserves semantic content without optimization. At higher levels, the
fixture expectations define the required simplifications and size bounds; the
choice and order of passes are yours.

### Error Handling

The tool must fail with a nonzero exit status when:

- no optimization level is provided
- `-o` is missing or has no following path
- there are no input files
- an input file cannot be read
- the input is not valid LowIR
- the output file cannot be written

For failure cases, diagnostics only need to be useful to a developer; exact
diagnostic text is not part of the grading contract. The contents of the
output file after a failed run are undefined.

### Quality Bar

The levels have this contract.  `-O0` parses the input program, preserves all
semantic content, and writes the canonical LowIR dump without running
optimizing transforms.  `-O1`, `-O2` and `-O3` each write valid LowIR that
preserves the behaviour of every defined input program; each higher level
may do everything the lower one does and more.  What the levels do is
your design.

What the course fixtures then check, for each `x.t`:

- the tool's exit status agrees with `x.ref.exit_status`;
- a successful output passes the LowIR structural validator;
- the output meets every predicate in its `x.ref.expect`
  (`../scripts/expect_ir.pl`);
- the output lowers through the object path at `-O0`
  (`cppgm++ -c -O0 x.lowir`) whenever the input does.  `make test` sets
  `CPPGM_LOWER_CHECK_APP` so the harness runs that floor; an optimizer's
  output is not correct until the backend accepts it.

An `x.ref.expect` sidecar has two kinds of line.  Outcome lines state what
the fixture is about in terms of the LowIR it reads and writes:
`none(call @callee) in @caller` for a call that must be inlined,
`count(load) <= 1 in ^body` for a load that must be hoisted,
`none(phi) in ^merge` for a choice that must collapse.  Budget lines bound
the output's size: `instructions <= N`, `blocks <= N`, `count(call)`,
`count(load)`, `count(store)`, `count(phi)`, and `instructions <= N in
@function`.  The budgets were generated from the course solution's output
at 10% tolerance plus one. Each sidecar states the actual required bounds
and outcomes. The adjacent `x.ref` is an informational example; matching its
instruction sequence is not required.

A source fixture in a driver lane is also run: the emitted LowIR is built
into a program through the object path and compared with the source built
at the lane's level (`CPPGM_BEHAVIOR_CHECK_APP`, set by `make test`); both
must print the same output and exit the same way.  A source that does not
build at that level is skipped.

To evaluate one sidecar by hand:

```sh
$ perl ../scripts/expect_ir.pl tests/o1/x.my tests/o1/x.ref.expect
```

### Design example

[Design notes](design-notes.md) describe the course solution’s passes and
implementation choices. They are a worked example, not additional requirements.

### Validation Modes

Successful outputs must pass the LowIR structural validator, the fixture’s
`.ref.expect` predicates, and the object-lowering check. Source-driver cases
also compare execution before and after LowIR serialization. Failed cases
are checked by exit status. See Quality Bar for the sidecar syntax.

### Testing

From this assignment directory, run:

```sh
make test
```

Before moving on, run `make test-report-through-pa32` from the repository root.

`make test` runs:

- `tests/o0`
- `tests/o1`
- `tests/o2`
- `tests/o3`
- `tests/driver/o1`
- `tests/driver/o2`
- `tests/driver/o3`
- `tests/object-roundtrip`


These directories are organized by tool mode and validation mode, not by N3485
source-language clauses.

- `tests/o0` runs `lowiropt -O0` on handwritten LowIR.
- `tests/o1` runs `lowiropt -O1` on handwritten LowIR.
- `tests/o2` runs `lowiropt -O2` on handwritten LowIR.
- `tests/o3` runs `lowiropt -O3` on handwritten LowIR.
- `tests/driver/o1` runs `cppgm++ --emit-lowir -g0 -O1` on source programs.
- `tests/driver/o2` runs `cppgm++ --emit-lowir -g0 -O2` on source programs.
- `tests/driver/o3` runs `cppgm++ --emit-lowir -g0 -O3` on source programs.
- `tests/object-roundtrip` compares direct `cppgm++ -c` output against an
  object produced by `cppgm++ --emit-lowir -O0` followed by `cppgm++ -c` on
  the generated LowIR file. This checks that object emission can be
  reconstructed from serialized LowIR instead of from hidden frontend side
  data. These tests may be standalone `.cpp` files or symlinks to existing
  `.t` harness cases; a selected `.t` test expands to its numbered `.t.1`,
  `.t.2`, ... source files when those sidecars exist. The harness checks
  no-debug objects at `-O0`, `-O1`, `-O2`, and `-O3`. A
  GNU-section reducer additionally checks the global-section and relocation
  relationships in both objects rather than relying only on byte equality. A
  `default-no-optimization` case also checks that omitting `-O` matches
  explicit `-O0`. A parameter-object-extent case additionally makes O3 object
  shape depend on a source-produced member-object boundary, so byte equality
  checks compiler-object and textual LowIR transport of that fact.

Run the debug metadata preservation lanes with:

```sh
make test-debuginfo
```

This also runs `tests/object-roundtrip` in debuginfo mode, comparing direct
`cppgm++ -c` output against LowIR-input `cppgm++ -c` output with
`-gline-tables-only` at `-O0`, `-O1`, `-O2`, and `-O3`.

`make test-debuginfo` runs:

- `tests/debuginfo/o1`
- `tests/debuginfo/o2`
- `tests/debuginfo/o3`
- `tests/debuginfo/driver/o1`
- `tests/debuginfo/driver/o2`
- `tests/debuginfo/driver/o3`

The direct debug-info tests run `lowiropt -O*` over LowIR containing
`!dbg(...)` metadata. The driver debug-info tests run
`cppgm++ --emit-lowir -gline-tables-only -O*` and check that source locations
survive the source-to-LowIR optimizer path.

For each `.t` test, the harness records the tool exit status and compares the
generated output against the oracle for that test directory. Failed reference
cases are judged by exit status; successful reference cases are judged by the
directory's LowIR validation mode.

### Out Of Scope

PA32 does not require:

- input programs to arrive in SSA form
- PRE that requires critical-edge splitting, speculative trapping operations,
  or an unbounded insertion/fixed-point schedule
- alias-driven aggressive dead-store elimination
- partial unrolling, peeling, vectorization, or loop transformations beyond
  the bounded transformations needed by the fixture expectations
- machine-IR scheduling or register-allocation optimization
- unbounded interprocedural cloning, externally observable ABI changes,
  semantic body merging, or indirect-call specialization
- size-specific `-Os` or `-Oz` behavior

### After PA32

Later optimization tests build on this assignment by optimizing after LowIR has
already been lowered to machine IR. PA32 focuses on the LowIR optimization
pipeline and the `lowiropt` contract and quality checks.
