## CPPGM Programming Assignment 35 (`lowiropt`)

### Overview

PA35 adds the first explicit optimization stage to the compiler. The new tool,
`lowiropt`, reads PA13 LowIR text, applies a deterministic optimization
pipeline selected by `-O0`, `-O1`, or `-O2`, and writes LowIR text.

The same LowIR optimizer is also reached from `cppgm++` when source programs
are compiled with `--emit-lowir -O1`, `--emit-lowir -O2`, or through the
ordinary compile/link driver at an optimization level.

### Prerequisites

You should complete PA34 before starting this assignment.

You will reuse:

- the PA13 LowIR syntax and semantic contract
- the PA14 through PA34 source-to-LowIR lowering pipeline
- the PA23 through PA25 native backend and runtime path
- the PA34 hosted compiler driver surface

### Starter Kit

The starter kit supplies:

- `pa35/Makefile`
- `pa35/lowiropt.cpp`, linked to the editable `dev/lowiropt.cpp`
- a `dev/lowiropt.cpp` scaffold based on `dev/lowiropt-scaffold.cpp`
- shared compiler support under `dev/src/`
- test directories under `pa35/tests/`
- harness scripts under `pa35/scripts/`
- checked-in `.ref` and `.ref.exit_status` files for the tests

The expected implementation work is in `dev/lowiropt.cpp` and shared optimizer
or driver support under `dev/src/`, especially the LowIR optimizer and
optimization-level plumbing. The supplied LowIR parser, dumper, driver helpers,
and test harness are support code; they do not implement the optimization
passes for you.

The harness uses checked-in references as the oracle. There is no
separate `lowiropt-ref` binary in the starter kit.

### Command Line

`lowiropt` accepts exactly one optimization level, one output path, and one or
more LowIR input files:

```sh
lowiropt -O0 -o <outfile> <lowirfile>...
lowiropt -O1 -o <outfile> <lowirfile>...
lowiropt -O2 -o <outfile> <lowirfile>...
```

`--help` and `-h` print usage information and exit successfully.

PA35 also requires the source driver to route these options through the same
optimizer:

```sh
cppgm++ --emit-lowir -g0 -O1 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -g0 -O2 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -gline-tables-only -O1 -o <outfile> <srcfile>...
cppgm++ --emit-lowir -gline-tables-only -O2 -o <outfile> <srcfile>...
```

The ordinary `cppgm++ -c` and link-driver paths must also accept `-O0`, `-O1`,
and `-O2` and use the same LowIR optimization level before object generation.

### Output Format

`lowiropt` writes LowIR text to `<outfile>`. The output must remain valid LowIR
and must preserve the behavior of every defined input program.

LowIR top-level declaration/definition order is a presentation convention, not
a dependency order. Reference outputs and canonical dumps use the order defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`, but the relaxed LowIR comparison canonicalizes top-level entries
before comparison. Your output must still be repeatable for the same
inputs; `../pa13/lowir.md` defines the canonical reference presentation and
notes where internal LowIR symbol names are only a presentation tie-breaker.
Your output must also preserve order-sensitive LowIR regions when they are present: instruction order inside
blocks, item order inside structured globals, vtable slot order, and action
order inside generated initialization, finalization, constructor, destructor,
and cleanup bodies.

For successful runs:

- `-O0` performs a deterministic parse/dump round trip.
- `-O1` applies local and control-flow-aware LowIR simplifications.
- `-O2` applies all `-O1` work and additional conservative slot-promotion
  optimizations.

The assignment grades the optimized LowIR shape as well as behavior
preservation. The goal is a deterministic optimization stage, not elapsed-time
benchmark wins.

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

### Optimization Levels

To complete PA35, implement these optimization levels:

`-O0` is the baseline canonicalization mode. It should parse the input program,
preserve all semantic content, and write the canonical LowIR dump without
running optimizing transforms.

`-O1` must include these semantic-preserving pass families where safe:

- constant folding for scalar `copy`, `unary`, `binary`, `cmp`, and `convert`
  instructions with known constant operands
- algebraic identities such as `x + 0`, `x - 0`, `x * 1`, `x & -1`, redundant
  `unary decay`, identity `convert`, and compares whose operands are known to
  be identical
- local and executable-edge-aware copy and constant propagation
- local and executable-edge-aware pure-expression reuse for eligible `addr`,
  `index`, `unary`, `binary`, `cmp`, and `convert` instructions
- safe normalization of commutative integer operations and reversible compare
  directions so equivalent expressions reuse the same producer
- boolean compare cleanup for `cmp eq` and `cmp ne` against `0` or `1` when the
  compared value is already known to be an `i64` boolean
- local reassociation of repeated integer `add`, `mul`, `and`, `or`, and `xor`
  chains with constants
- control-flow cleanup, including folding known `branch` and `switch`
  selectors, removing unreachable blocks, bypassing trivial jump-only blocks,
  and merging safe straight-line block pairs
- preservation of exceptional handler targets and exception-structure blocks
  while doing CFG cleanup
- dead-code elimination for unused pure temp-producing instructions
- removal of unused calls only when the callee is explicitly `readnone`, cannot
  unwind, and is not `noreturn`
- removal of slot declarations that become unused after simplification

`-O2` must include all `-O1` work and then conservatively promote eligible
non-escaping scalar slots, including eligible `ptr` slots. Promotion must be
limited to slots accessed through direct `store` and `load` operations whose
current value can be tracked without introducing phi nodes. `-O2` also removes
dead stores to promoted slots when no observable load can see the stored value.

### Validation Modes

LowIR output has presentation details, such as internal helper names and
metadata ordering, that are not semantic requirements. The PA35 harness checks
exit status, LowIR well-formedness, required IR facts, and behavior
preservation without requiring every non-semantic presentation choice to match
the course solution exactly. Exact textual LowIR matching is not a PA35 grading
requirement unless a test explicitly says so.

### Testing

Run the PA35 suite with:

```sh
make test
```

`make test` runs:

- `tests/o0`
- `tests/o1`
- `tests/o2`
- `tests/driver/o1`
- `tests/driver/o2`

These directories are organized by tool mode and validation mode, not by N3485
source-language clauses.

- `tests/o0` runs `lowiropt -O0` on handwritten LowIR.
- `tests/o1` runs `lowiropt -O1` on handwritten LowIR.
- `tests/o2` runs `lowiropt -O2` on handwritten LowIR.
- `tests/driver/o1` runs `cppgm++ --emit-lowir -g0 -O1` on source programs.
- `tests/driver/o2` runs `cppgm++ --emit-lowir -g0 -O2` on source programs.

Run the debug metadata preservation lanes with:

```sh
make test-debuginfo
```

`make test-debuginfo` runs:

- `tests/debuginfo/o1`
- `tests/debuginfo/o2`
- `tests/debuginfo/driver/o1`
- `tests/debuginfo/driver/o2`

The direct debug-info tests run `lowiropt -O*` over LowIR containing
`!dbg(...)` metadata. The driver debug-info tests run
`cppgm++ --emit-lowir -gline-tables-only -O*` and check that source locations
survive the source-to-LowIR optimizer path.

For each `.t` test, the harness records the tool exit status and compares the
generated output against the oracle for that test directory. Failed reference
cases are judged by exit status; successful reference cases are judged by the
directory's LowIR validation mode.

### Out Of Scope

PA35 does not require:

- SSA construction as a IR contract
- global value numbering or partial redundancy elimination
- alias-driven aggressive dead-store elimination
- loop optimizations, vectorization, or inlining beyond the supplied LowIR optimizer contract
- machine-IR scheduling or register-allocation optimization
- interprocedural optimization
- size-specific `-Os` or `-Oz` behavior

### Handoff

PA36 builds on this assignment by optimizing after LowIR has already been
lowered to machine IR. PA35 owns the LowIR optimization pipeline and the
`lowiropt` structural oracle; PA36 owns machine-backend optimization.
