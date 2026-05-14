# `cppgm++` Driver CLI Compatibility Plan

This is a follow-up plan, not a fresh greenfield design.

We already have two real pieces in place:

- the core parser in
  [cpp_tool_cli.cpp](/Users/vishvananda/cppgm/dev/src/cpp_tool_cli.cpp)
- the compatibility wrapper in
  [cppgm-cmake-wrapper.sh](/Users/vishvananda/cppgm/scripts/cppgm-cmake-wrapper.sh)

The wrapper exists mainly as a compatibility aid for testing and external build
systems. It is useful, but it is not the main goal of this plan. The main goal
is to make the real `cppgm++` CLI easier to use and closer to normal compiler
practice over time.

## Narrow Goal

Improve the real `cppgm++` command-line interface so that:

- it accepts the important standard driver flag families directly
- it remains staged and simple enough for assignment buildout
- it is easier to use with normal toolchains and build systems
- it avoids inventing a fake bespoke CLI culture for students

This plan is **not** trying to make `cppgm++` instantly match full `clang++`.
It is also **not** making wrapper elimination the primary success metric.

## What Is Already In Place

### Core Driver Today

The core parser already handles:

- `-c`
- `-E`
- `-o`
- `-I`
- `-L`
- `-l`
- `--target`
- positional inputs

### Wrapper Today

The wrapper currently adds or normalizes:

- query flags:
  - `--version`
  - `-v`
  - `-dumpmachine`
  - `-dumpversion`
  - `-print-search-dirs`
- preprocessor-oriented controls:
  - `-D`
  - `-U`
  - `-include`
  - `-isystem`
- benign build-system flags that are currently ignored:
  - `-std=...`
  - `-stdlib=...`
  - `-pthread`
  - `-g`
  - `-O*`
  - `-W*`
  - `-pipe`
  - `-f*`
  - `-m*`

That means this plan should be about deciding which standard driver families
`cppgm++` itself should support directly, not about rediscovering what a
compiler driver looks like.

### Completed Groundwork

The first independent CLI slice is now in place:

- the core parser is split by option family rather than one flat chain
- leading query forms are recognized directly by the driver entrypoint
- common benign compatibility flags are explicitly accepted/ignored or given a
  clear not-yet-supported diagnostic
- compile/link default-output and multi-input behavior now follows ordinary
  compiler-driver conventions more closely:
  - link mode with no `-o` defaults to `a.out`
  - `-c a.cpp b.cpp` emits `a.o` and `b.o`
  - a single explicit `-o` is rejected for multi-input `-c` / `-E`

That means the remaining work is narrower than the original plan text implied.

## Final Resolution

This follow-up is now complete.

The final direct-support surface is:

- query-only forms recognized directly by the real driver:
  - `--help`
  - `-h`
  - `--version`
  - `-v`
  - `-dumpmachine`
  - `-dumpversion`
  - `-print-search-dirs`
- direct hosted preprocessor-control forms in `cppgm++` itself:
  - `-D`
  - `-U`
  - `-include`
  - `-isystem`
- compiler-style default output behavior for multi-input `-c` / `-E`
- explicit accepted-but-ignored handling for the common benign build-system
  flag families already listed in this plan

The query-output ownership decision is also settled:

- query-only forms remain host-delegated for their answer text through the
  normal `run_host_cpp_query(...)` path

That is intentional. These answers are environment-specific and primarily
build-system-facing. The important part for this plan was that the real driver
classifies and accepts the forms directly, not that it reimplements a fake
portable version string or search-dir report.

## Recommended Compatibility Levels

To keep this staged and student-friendly, every option family should fall into
one of three categories.

### 1. Fully Implemented

These flags affect real pipeline behavior and must work directly.

Examples:

- `-E`
- `-c`
- `-o`
- `-I`
- `-L`
- `-l`
- `--target`

Later hosted/toolchain stages should also move:

- `-D`
- `-U`
- `-include`
- `-isystem`

into this category.

### 2. Accepted But Ignored

These are common compatibility flags that should stop causing spurious failure
even if they are not assignment-defining yet.

Examples:

- `-g`
- `-O0`
- `-O2`
- many `-W...`
- `-pipe`

This should be explicit in the driver rather than an accidental side effect of
the wrapper.

### 3. Recognized But Not Yet Supported

These should get a clear family-specific diagnostic when the owning assignment
has not arrived yet.

Examples:

- `-pthread`
- `-stdlib=...`
- some `-std=...` or `-x` forms if the active assignment does not own them yet

## Remaining Follow-Up Targets

### Target 1. Split The Core Parser By Option Family

This internal cleanup is now done. The parser no longer grows as one flat
chain.

The completed split covers at least:

- phase/output flags
- macro/include-control flags
- include/link search flags
- query flags
- toolchain/language flags
- benign compatibility flags

The relevant work lives in
[cpp_tool_cli.cpp](/Users/vishvananda/cppgm/dev/src/cpp_tool_cli.cpp) as one
family-oriented parser rather than one flat switchboard.

### Target 2. Add Hosted-Critical Preprocessor Forms To The Real Driver

This is now done.

`cppgm++` directly supports the full hosted preprocessor-control family needed
here:

- `-D`
- `-U`
- `-include`
- `-isystem`

Those forms now run through the real driver/preprocessor path in both `-E` and
`-c` mode, and the compatibility wrapper no longer rewrites source files to
fake them.

### Target 3. Add Direct Query-Flag Support

This is also done.

The real driver classifies and accepts the leading query forms directly, and
the policy choice is now explicit:

- the forms are direct `cppgm++` surface
- the answer text remains host-delegated

### Target 4. Make Benign Compatibility Acceptance Explicit

This explicit classification is largely in place now for the current
high-value families. The remaining work here is mainly to keep that list honest
as later direct-support forms land.

This matters less for semantics than for build-system interoperability, but it
is still important.

## Role Of The Wrapper

The wrapper can remain useful for:

- external build-system integration
- testing compatibility with driver shapes we do not yet support directly
- environment-specific translation that does not belong in the core parser

So the wrapper is not itself a problem to be eliminated. The real question is:

- is `cppgm++` directly supporting the important CLI surface we want students
  and users to rely on?

## What Should Stay Blocked For Now

The runtime-sensitive parts of this plan should stay behind
[host-abi-runtime-followup-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-followup-plan.md)
for the runtime-sensitive parts of hosted behavior.

The driver should not standardize around temporary hosted/runtime escape hatches
that we already intend to remove.

## Suggested Migration Order

1. Reshape the core parser internally by option family.
2. Finish direct support for the remaining macro/include-control flags.
3. Decide whether query flags remain host-delegated or become fully
   self-reported.
4. Keep explicit accepted-but-ignored handling aligned with the real supported
   surface.
5. Re-evaluate which remaining wrapper behaviors are actually core CLI features
   versus build-system/testing conveniences.

## Completion Criteria

This follow-up is now complete because:

- ordinary hosted driver invocations work directly through `cppgm++` for the
  important core preprocessor/include families, including `-U`, `-include`,
  and `-isystem`
- the intended query-form ownership is settled and documented as
  intentionally host-delegated answer text
- common benign build-system flags no longer cause avoidable failures across
  the main supported families
- the supported direct CLI surface is clear and matches the intended
  assignment/user-facing contract

## Assignment Guidance

The assignment-facing rule should remain:

- "support at least these forms"

not:

- "only this tiny bespoke command line is valid"

That keeps the staged student contract simple without teaching a fake permanent
CLI.
