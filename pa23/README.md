## CPPGM Programming Assignment 23 (`cppgm++ --emit-lowir`)

### Overview

PA23 extends PA22 with the first supported object layouts that require more than the
earlier single-vptr, non-virtual-base ABI:

- virtual inheritance for shared base-subobject layout and access
- polymorphic multiple inheritance with more than one active vtable view
- pointer-form `dynamic_cast` across sibling polymorphic bases
- RTTI / `typeid` through non-primary polymorphic base views

PA23 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 22 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA4
- the PA5 AST as the syntax boundary
- the PA6-PA7 semantic foundation
- the PA10-PA22 LowIR lowering path
- the PA8 LowIR contract

### Starter Kit

The starter kit contains:

- `pa23/README.md`, `pa23/Makefile`, and the test scripts in `pa23/scripts/`
- your cumulative `dev/cppgm++.cpp` compiler entry point
- the `pa23/cppgm++.cpp` symlink back to `../dev/cppgm++.cpp`
- shared support sources and headers under `dev/src/`
- the grammar for this assignment called `pa23.gram`
- an HTML grammar explorer of `pa23.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

Students should implement the assignment in `dev/cppgm++.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. Reuse your earlier compiler infrastructure when implementing this milestone.

Use the supplied reference tools to inspect example output. Tests compare
your compiler with the checked-in contract references.

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

`-O0` is the PA23 test mode. Other optimization levels are later optimizer work and
are not required for this milestone.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa8/lowir.md`. PA23 extends the PA22 lowering
surface only by making more of the C++ source language lower into the already-defined LowIR
family.

LowIR top-level declaration/definition order is a presentation convention, not
a dependency order. Reference outputs and canonical dumps use the order defined
in `../pa8/lowir.md`: `declare global`, `declare function`, `global`, then
`function`, but the relaxed LowIR comparison canonicalizes top-level entries
before comparison. Your output must still be repeatable for the same
inputs; `../pa8/lowir.md` defines the canonical reference presentation and
notes where internal LowIR symbol names are only a presentation tie-breaker.
Your output must also preserve order-sensitive LowIR regions when they are present: instruction order inside
blocks, item order inside structured globals, vtable slot order, and action
order inside generated initialization, finalization, constructor, destructor,
and cleanup bodies.

The generated LowIR must be well-formed and must match the checked-in `.ref` files under
the relaxed LowIR comparison used by the harness. That comparison still checks the
semantic LowIR shape and required IR facts, but it does not make helper metadata
presentation or other non-semantic text details part of the student contract.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic analysis, or LowIR
generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

Tests compare your output with the checked-in references using the LowIR
comparison described in [the format guide](../pa8/lowir.md). The `Makefile` invokes
`cppgm++` with `--emit-lowir -O0`.

The local checked-in tests live in `tests/general/`. They exercise PA23
source-to-LowIR behavior over virtual inheritance, non-primary polymorphic
views, sibling `dynamic_cast`, and RTTI through adjusted base views.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

PA23 is tested against generated LowIR text using the relaxed LowIR comparator described
above. For execution feedback, use the supplied `lowir2native-ref -O0`
backend introduced in PA8. Your own native backend is implemented in PA24.

The shipped PA23 tests are the contract for this milestone.

### PA23 Syntax Spec

The authoritative source syntax is the shared `cppgm++` source grammar, exposed
for this assignment as `pa23.gram`. The grammar defines accepted syntax only;
the PA23 semantic and lowering requirements are defined by the Assignment
Boundary and Out Of Scope sections below.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA8 `lowir.md`, and the checked-in
`.ref` files.

PA23 does not add a new source-language grammar format. It instead enables more
of the already-accepted C++11 syntax to participate in semantic analysis and
lowering.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa23.gram` as the source of truth.

`pa23.gram` uses the same token vocabulary and the same extended BNF operators as
`../shared/source.gram`.

If this README and `pa23.gram` appear to disagree about source syntax, treat `pa23.gram`
as authoritative. If this README and PA8 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA23 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA23 supports the following in addition to the PA22 subset:

- virtual inheritance for shared base-subobject layout in complete objects
- field access through shared virtual bases
- supported constructor and hidden-argument forwarding cases: a by-value parameter of a
  class with virtual bases carries each virtual base's subobject address as a hidden
  pointer argument after the visible parameters, since the complete type is visible to
  both caller and callee; a reference or pointer parameter carries no such hidden argument
  and instead reaches its virtual bases through the object's own vtable at each use
- polymorphic multiple inheritance with separate vtable views for non-primary polymorphic
  bases
- virtual dispatch through primary views whose virtual-base ABI carries
  function/adjustment rows, and through non-primary polymorphic base pointers
  and references
- pointer-form `dynamic_cast<T*>` across sibling polymorphic bases in the supported object
  model
- `typeid(expr)` through supported non-primary polymorphic base lvalue views

Within this milestone, PA23 should produce valid LowIR for ordinary source programs over
that subset. That LowIR should be accepted by PA24 `lowir2native` for the supported cases.

To complete PA23, implement these goals:

1. Shared virtual-base layout.
   Complete objects with a virtual diamond should expose one shared base-subobject at a
   deterministic offset.

2. Polymorphic dispatch over adjusted vtable views.
   Calling a virtual through a class with virtual-base adjustment rows must select the
   requested logical slot. Calling through a later polymorphic base must lower through the
   correct vtable view and apply the required `this` adjustment. A final overrider inherited
   from a non-primary or virtual base must also occupy its required slot in the derived
   class's primary vtable group, in addition to any adjusted secondary-view entry. Each
   vtable segment must contain only the vcall-offset and virtual-base-offset rows owned by
   that segment; in particular, vcall rows belonging to a secondary virtual-base view must
   not enlarge the primary segment or its address point.

3. Sibling cross-cast support.
   Pointer-form `dynamic_cast` across sibling polymorphic bases should lower into the
   supported RTTI / vtable-view scan.

4. RTTI through non-primary views.
   `typeid(expr)` should observe the dynamic type through a supported non-primary
   polymorphic base reference.

### Out Of Scope

The following are explicitly out of scope for PA23:

- virtual-base constructor, copy, assignment, and destructor sequencing beyond the already
  supported simple generated cases
- polymorphic multiple inheritance with virtual destructors
- reference-form `dynamic_cast`
- `dynamic_cast` and RTTI cases that require `bad_cast` / `bad_typeid`
- virtual inheritance combined with the unsupported special-member or exception cases
- toolchain-driver and host-linker integration

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA24, which lowers the completed LowIR family to
native code before PA25 turns the source pipeline into a practical `cppgm++`
toolchain driver and standard object-output flow.

So PA23 should leave behind:

- a stable multi-vtable / virtual-base LowIR lowering path
- deterministic lowering for the supported sibling-cast and RTTI-view cases
- explicit remaining deferrals only where the practical toolchain and remaining ABI/runtime
  work need to take over
- enough stable source behavior that PA25 can start carrying complete
  programs as C++ end-to-end tests through the practical driver/link path

### Design Notes (Non-Normative)

PA23 should extend the existing object-model and RTTI lowering path, not replace it.

The same monotonic-extension rule applies here:

- PA23 should add its new behavior only when the source actually uses the supported PA23
  feature set
- it should not perturb PA22 outputs for programs that remain entirely within the PA22
  subset
- in practice, the richer vtable / RTTI layout should stay source-driven rather than
  changing earlier single-vptr cases unnecessarily

For Itanium-layout vtable segments, emit any vcall-offset rows before the
virtual-base-offset rows, followed by offset-to-top, RTTI, and the function
slots. Track each segment's address point from the rows actually emitted for
that segment instead of using one class-wide negative-row count.

Keep a synthesized constructor or destructor base entry's ABI identity
separate from inlining policy. The entry may need its own object symbol or
retained definition, but it gains `no_inline=yes` only when the source-level
function has the corresponding prohibition.

Choose where a virtual base's address comes from by how the parameter is
passed, not by whether the function happens to see the complete type. A
by-value parameter forces its complete type on every caller, so the caller
can compute each virtual base's address and pass it as a hidden pointer
argument. A reference or pointer parameter does not: a caller may hold only a
forward declaration, in which case it cannot compute a hidden virtual-base
argument, while the definition, compiled where the type is complete, would
expect one -- the two disagree across a translation unit. Give a reference or
pointer parameter no hidden virtual-base argument and recover each virtual
base's address from the object's vtable at the point of use, the way the same
access on any other reference or pointer already does. Restricting the hidden
argument to by-value parameters keeps the calling convention identical whether
or not a translation unit has the complete type in view.
