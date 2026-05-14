# LowIR Evolution Plan

This document now defines the **completed near-term LowIR work** we intended to
do first, while also recording the later backlog that should follow after that
near-term work landed.

So this file should be read as both:

- the execution plan for the first real LowIR cleanup tranche
- the bridge from today's IR contract to the later optimizer- and
  multi-language-friendly form

The important change from the older version of this document is:

- the first part of the file is now a real patch queue
- the later sections remain longer-term backlog

## Main Direction

Update LowIR so it carries semantic and ABI information that currently leaks
through hardcoded symbol names, naming conventions, and frontend-specific
lowering rules.

The long-term goals are:

- make LowIR easy to read and reason about directly
- make LowIR self-describing enough that validators and later backends do not
  need to recover meaning from frontend heuristics
- preserve enough call/storage/object information to support future
  optimization passes
- keep the core IR generic enough that non-C++ frontends can target it

The main target is to stop relying on symbol spelling alone for:

- `extern "C"` boundary names
- the program entrypoint
- runtime init/fini hooks
- exception/runtime support hooks

The broad strategy should still be:

1. First make the IR self-describing and easy to validate.
2. Then make call/storage semantics explicit enough for optimization.
3. Only then do larger storage/layout refactors where the payoff is clear.

This keeps the early changes broadly useful for readability, validation, and
multi-language targeting instead of immediately overfitting the IR to one
frontend's lowering details.

## LowIR Growth Checklist

For any future LowIR syntax or semantic expansion, we should take an explicit
pass over a small set of recurring “is this IR-level now?” questions before we
freeze the change into one frontend's lowering conventions.

The point of this checklist is not to force all of these into the IR
immediately. The point is to keep asking whether a newly exposed behavior is:

- genuinely language-neutral and backend-useful
- better represented directly in LowIR than reconstructed later from ad hoc
  frontend conventions
- still generic enough that non-C++ frontends could target it too

The current recurring questions are:

- should control-flow lowering now grow a first-class multi-way branch form
  such as `switch`, rather than forcing that structure to remain encoded only
  as hand-built branch diamonds?
- should call-boundary metadata now grow explicit variadic / `va_*` support,
  rather than leaving prototype-relaxed or variadic behavior implicit in
  frontend-specific lowering?
- should scalar-width changes now grow first-class integer conversion forms
  such as `sext`, `zext`, and `trunc`, rather than flattening that behavior
  into copies, loads, or backend-specific widening rules?
- should call/return metadata now grow explicit small-aggregate register ABI
  classification, or should LowIR stay semantic at the call boundary and leave
  host-specific register chunking to a later ABI elaboration step?
- should structured global data now grow explicit readonly-vs-writable storage
  intent, rather than forcing ABI tables such as vtables, RTTI objects, and
  typeinfo names through the same generic writable-data path?

These are all potentially valuable, but they should only land when the answer
is “yes, this makes LowIR more self-describing and still broadly useful,” not
just “C++ lowering needed it once.”

## Near-Term Execution Scope

This plan should now be executed in four concrete phases.

These phases are the active part of the plan:

1. explicit declarations/imports
2. explicit symbol/ABI role metadata
3. explicit structural well-formedness contract
4. explicit call-boundary metadata

These are the right first tranche because they:

- improve readability immediately
- improve validator quality immediately
- unblock the remaining host-ABI follow-up work
- prepare LowIR for PA35 without forcing the most invasive storage rewrite yet

## Current Execution Status

- The concrete near-term LowIR execution lane is now complete and validated
  through `pa34`.
- Phase 1 is implemented in the current execution lane:
  - explicit `declare function` / `declare global` syntax
  - parser / printer / validator support
  - backend acceptance of declaration-only imports
  - frontend declaration completion for referenced external/runtime symbols
- Phase 2 is now implemented as a first concrete role-metadata slice:
  - optional top-level `role=...` metadata on declarations and definitions
  - parser / printer / validator support for singleton entry / init / fini /
    EH helper roles
  - PA13 grammar / README / LowIR spec updates for the new syntax
  - role-driven startup selection in `lowir2cy86` and `lowir2native`
  - role-aware host-EH detection in the MIR builder
  - legacy startup aliases emitted at the object boundary so role-driven
    entry/init/fini still survive separate object linking without baking those
    names back into LowIR
- Phase 2 now also includes a first explicit linkage slice:
  - optional top-level `linkage=c|cpp` metadata on declarations and definitions
  - parser / printer / validator support for that metadata
  - emitted LowIR now uses `linkage=c` where the semantic frontend already
    knows a symbol has C linkage
- Phase 2 now also includes a first explicit binding slice:
  - optional top-level `binding=internal|strong|weak` metadata on declarations
    and definitions
  - parser / printer / validator support for that metadata
  - printed and emitted LowIR now carry weak-vs-strong intent directly instead
    of relying on frontend-only export bookkeeping
- Phase 2 now also includes a first explicit storage slice:
  - optional top-level `storage=readonly|writable` metadata on globals
  - parser / printer / validator support for that metadata
  - printed and emitted LowIR now normalize readonly global storage through
    explicit metadata instead of the older bare `readonly` spelling
  - the shared compare-results validator now accepts that metadata when
    checking emitted LowIR references
- The remaining Phase 2 backlog is now longer-term:
  - broader import / export intent and richer storage-ownership metadata remain
    useful, but they are no longer part of the active execution blocker for
    optimization
  - after the current tranche, the remaining unresolved pieces in that bucket
    are mostly object-file policy details such as exported object spelling,
    internal-alias retention, and local-binding preference; those do not yet
    look like the right generic LowIR surface, so they should stay deferred
    unless a later multi-language or optimizer use case makes them IR-level
    facts rather than backend policy
- Phase 3 is now implemented:
  - the PA13 LowIR spec explicitly lists the structural well-formedness rules
  - `lowir_internal` now rejects duplicate top-level names, duplicate
    parameters / slots / blocks, missing terminators, instructions after
    terminators, and undefined block targets
  - PA13 now has negative owner tests for those malformed-program cases
- Phase 4 is now implemented for the first concrete call-boundary slice:
  - PA13 accepts explicit parameter `pass=` metadata
  - the parser / printer / validator understand
    `indirect_result`, `by_address`, `reference`, and `decay`
  - `lowirgensemantic` emits those semantic call-boundary annotations directly
  - the PA13 docs/tests and affected emit-LowIR refs across `pa14`-`pa29`
    have been refreshed for that syntax
  - PA13 now accepts explicit function
    `arity=` metadata for `fixed` vs `variadic`, the LowIR parser / printer /
    validator understand it, and the harness now uses that metadata for direct
    call arity validation
  - the type model and LowIR surface now also have an explicit
    `prototype_relaxed` arity mode, although the current C++ frontend still
    does not originate that boundary from source declarations
  - PA13 now accepts explicit indirect-call signature metadata via
    `as (...) -> ...`, `lowir_internal` prints and validates it for both value
    and void indirect calls, `lowirgensemantic` emits it on both the
    `--emit-lowir` and backend-owned LowIR paths, and the affected PA13 owners
    plus downstream emit-LowIR refs are refreshed for that syntax
  - printed LowIR no longer spells direct small-object parameters or results as
    host ABI register chunks; instead it uses a semantic direct object
    boundary type `obj<bytesxalign>` in direct parameter positions, direct
    result positions, and explicit indirect-call signatures, while
    LowIR-to-CY86 / MIR / native lowering still performs the backend-facing
    chunk elaboration later
- The first optimization-relevant storage follow-up is also implemented:
  - LowIR now has explicit integer width conversions `sext`, `zext`, and
    `trunc`
  - the emitted LowIR uses those conversions instead of flattening everything
    into widened storage guesses
  - the recent hosted-output / throw / vtable fallout has been fixed and the
    broader `pa13` through `pa34` validation lane is green again
- The next value-vs-storage cleanup slice is also implemented:
  - ordinary scalar explicit casts now lower through a first-class
    semantic cast node instead of relying entirely on flattened
    `materialization_source_type` / `conversion_source_type` side-channel
    bookkeeping
  - emitted LowIR now makes null-to-pointer and similar ordinary scalar cast
    chains visible as real conversion steps instead of silently absorbing them
    into later load/call lowering
  - the affected emitted-LowIR owner refs across `pa16`, `pa21`, and `pa22`
    have been refreshed, and the `pa14` through `pa29` sanity lane is clean
- The first explicit aggregate/object storage follow-up is now implemented:
  - `copyobj` and `zeroinit` now carry explicit storage spans
    `<bytes>x<align>` instead of only a byte count
  - the parser / printer / validator and PA13 docs now treat positive
    power-of-two alignment as part of the public LowIR contract for bulk object
    storage operations
  - emitted LowIR now preserves object-copy alignment directly instead of
    silently degrading to an implied alignment of `1`
  - the affected PA13 owners plus emitted-LowIR refs across `pa15`, `pa16`,
    `pa18`, `pa22`, `pa26`, and `pa27` have been refreshed
  - the `pa14` through `pa29` validation lane is clean again, and the late
    `pa34` timeout pair (`651`, `652`) was checked directly and shown to
    compile successfully in isolation, so that suite blip was ambient timeout
    pressure rather than a new semantic/codegen regression
- The MachineIR preservation follow-up is now implemented:
  - LowIR-to-MachineIR lowering now preserves the explicit `<bytes>x<align>`
    storage span on `copyobj` / `zeroinit` instead of dropping alignment at
    the MIR boundary
  - printed MIR now carries that preserved span as
    `copy_bytes <bytes>x<align>` and `zero_bytes <bytes>x<align>`
  - the owned `PA23` MIR refs now match the preserved alignment-bearing form
  - this closes the last narrow follow-up slice that was still active in the
    near-term LowIR lane
- The first-class multi-way control-flow follow-up is now implemented:
  - LowIR now has a first-class `switch` terminator instead of forcing every
    multi-way dispatch path to remain encoded only as compare/branch ladders
  - the PA13 parser / printer / validator / grammar / docs now treat `switch`
    as part of the public LowIR surface
  - emitted LowIR and simple backends now preserve that dispatch structure
    until later lowering instead of flattening it immediately
  - the owned `PA13` / `PA23` switch owners and the broader `pa14` through
    `pa34` validation lane were refreshed and revalidated for that shape
- The remaining Phase 4 backlog is now future work:
  - `prototype_relaxed` remains in LowIR as a generic capability, but truthful
    source-origin emission is deferred to a future C / FFI frontend rather than
    treated as a current C++-frontend task
  - broader review of whether any additional semantic direct-object surface is
    needed beyond the current parameter/signature slice

## Immediate Completion Sequence

The concrete sequence that was active in this lane is now complete:

1. fixed the regression fallout, including the `pa34` hosted-output / vtable
   ownership cluster, and re-established a clean broader sanity lane
2. implemented the next optimization-relevant storage-model slice, centered on
   explicit integer width conversions and less ad hoc storage/value guessing
3. implemented the remaining strong / weak binding metadata that still belonged
   to Phase 2 of this first concrete tranche
4. reran the full affected validation set and restored a clean `pa13` through
   `pa34` lane
5. preserved explicit bulk-storage alignment through the MachineIR boundary so
   the new storage metadata no longer disappears before `PA23` MIR output
6. landed first-class `switch` control flow so multi-way dispatch no longer has
   to flatten immediately into compare/branch ladders before reaching LowIR

The rest of this file is now later backlog rather than active execution work
for this completed tranche.

## Phase 1. Explicit Declarations And Imports

LowIR currently distinguishes “defined here” from “referenced somehow” mostly
through the presence or absence of a matching `function` or `global`
definition. That forces validators to guess whether an undefined `@symbol`
represents:

- a legitimate declaration-only external
- imported runtime support
- a static data member defined elsewhere
- or a real lowering bug

Add explicit declaration forms such as:

- `declare function @name(...) -> type`
- `declare global @name : type`

This is the best immediate readability win because a reader can tell directly
which symbols are imported and which are owned by the current IR module.

It is also the most language-neutral improvement: every future frontend and
backend benefits from a clean declaration/definition split.

Concrete goal for this phase:

- add explicit declaration forms
- teach the parser/printer/validator about them
- migrate existing imported/runtime/external cases onto declarations instead of
  implicit "undefined symbol means maybe external" guessing

Implementation note:

- the current lane now treats this phase as complete, including the emit-LowIR
  fallout updates needed for the affected `pa14` through `pa29` owner sets

## Phase 2. Explicit Symbol/ABI Role Metadata

LowIR currently has to infer some meaning from symbol spelling. It would be
cleaner if declarations/definitions carried explicit role metadata such as:

- language linkage: C vs C++
- entrypoint kind: normal function vs program entry
- runtime hook kind: init, fini, exception helper, EH personality, etc.
- symbol import/export intent
- weak vs strong linkage intent
- ordinary storage ownership intent: declaration-only, definition, imported
  storage, `thread_local`, etc.

This would let backends choose the right spelling and object-file decoration
without frontend code depending on name conventions.

This is important not just for C++ ABI cleanup, but for keeping the IR useful
to other languages. Linkage and ownership should be generic IR metadata, not a
C++-specific naming convention.

Concrete goal for this phase:

- make runtime/import/export/storage/linkage roles explicit at the IR boundary
- stop relying on symbol spelling alone for entry/runtime/EH meaning
- provide the role layer needed by
  [host-abi-runtime-followup-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-followup-plan.md)

Implementation note:

- the current lane treats the startup/runtime/EH role slice as the first
  concrete implementation of this phase
- the current lane now also includes a first descriptive `linkage=` slice for
  C-linked symbols
- the current lane now also includes explicit `binding=internal|strong|weak`
  metadata
- broader import/export/storage-intent role metadata still belongs to Phase 2,
  but it is no longer the immediate blocker for LowIR correctness, validation,
  or ABI-neutral printed LowIR

## Phase 3. Explicit Structural Well-Formedness Contract

The IR spec should explicitly say what makes a LowIR unit well formed, rather
than leaving this implicit in the test refs.

Examples:

- every block ends in exactly one terminator
- no instructions appear after a terminator
- every referenced block target exists
- every temporary or slot is defined before use
- duplicate function/global/block definitions are invalid

This is not a new opcode family, but it is still a real IR evolution step.
A self-describing IR also needs a clear verifier contract.

Concrete goal for this phase:

- make the well-formedness contract explicit in the IR docs
- keep the validator aligned with that contract
- use that contract as the base for future relaxed validation

Implementation note:

- the current lane treats this phase as complete for the first concrete
  structural slice:
  - the docs now state the structural contract directly
  - the real LowIR parser, not just the Perl oracle, rejects the core malformed
    forms that the plan called out

## Phase 4. Explicit Calling/Passing Mode Metadata

Several lowering paths currently recompute ABI behavior from semantic types.
Likely useful explicit function/parameter metadata:

- hidden sret return vs direct scalar return
- by-value scalar parameter
- by-address aggregate parameter
- array/function decay parameter
- reference parameter kind
- fixed-arity vs variadic/prototype-relaxed call boundary
- explicit indirect-call signature information where the callee value alone is
  not enough

This would reduce duplicated “re-derive ABI from semantic type” logic in
`lowirgensemantic`, object backends, and any future verifier.

This is one of the highest-value additions for future optimization. Without
clear call-boundary metadata, inlining, call-graph construction, alias
reasoning, and argument/result simplification all become frontend-coupled or
too conservative.

Concrete goal for this phase:

- make direct and indirect call boundaries explicit enough for validation and
  later optimization
- stop forcing every later pass to recover call semantics from frontend-only
  conventions

Implementation note:

- the current lane treats the first concrete Phase 4 slice as implemented for
  semantic parameter/result passing modes:
  - PA13 now accepts explicit parameter `pass=` metadata
  - the emitted LowIR now marks indirect-result, by-address, reference, and
    decay boundaries directly
  - the emitted LowIR now also keeps direct small-object parameters and
    results as one semantic object boundary type `obj<bytesxalign>` instead of
    host register chunks, and leaves chunk elaboration to later CY86 / MIR /
    native lowering

### Phase 4 Decision: Keep Core LowIR ABI-Neutral

The current pair / small-aggregate fallout has forced the main design decision
here into the open:

- host ABI register splitting should **not** become part of the stable
  student-authored LowIR syntax
- the core LowIR function signature should describe the semantic call boundary,
  not the final host register assignment
- target-specific coercions such as “this pair travels in two GPRs” belong in a
  later ABI elaboration or backend lowering step

This is the direction used by the major compiler IRs we want to emulate:

- LLVM IR keeps aggregate values as IR values and uses ABI-affecting attributes
  such as `sret`, `byval`, `inalloca`, and `preallocated` only when the source
  calling boundary is itself indirect or requires special storage treatment; the
  target-specific register breakdown happens later in codegen
- MLIR keeps its higher-level function signatures and then uses a separate
  calling-convention conversion layer to pack, unpack, and linearize arguments
  when lowering to the LLVM dialect
- Rust keeps MIR generic and only lowers calls to LLVM IR during codegen
- GCC keeps front-end / optimizer-facing work in GENERIC/GIMPLE and explicitly
  warns that RTL is too target-specific to serve as a front-end interface

That gives us the rule for LowIR:

- if the source/program-semantics say “this argument/result is indirect”, LowIR
  should say so explicitly
- if the source/program-semantics say “this is a direct value”, LowIR should
  keep it as one semantic value, even if the eventual host ABI will coerce it
  into multiple registers
- the later ABI lowering stage may attach or derive register-chunk information,
  but that is backend-facing metadata, not the stable LowIR syntax students are
  expected to write

The old direct-host-chunk parameter surface was transitional debt, not the
desired long-term IR contract, and the current lane now removes that spelling
from printed LowIR for direct parameter positions.

What should become explicit in LowIR is the semantic call mode:

- direct value
- indirect result (`sret`-like)
- indirect/by-address parameter
- reference parameter
- decay / variadic / prototype-relaxed boundary rules

What should remain out of the core syntax:

- SysV pair-in-register splitting
- Mach-O vs ELF calling-sequence details
- exact GPR/XMM chunk layouts for small aggregates
- other target calling-convention coercions that are only meaningful after the
  backend target is known

When we execute Phase 4, the preferred end state is therefore:

1. make the semantic call boundary explicit in LowIR
2. keep `--emit-lowir` stable across host ABI choices for the same semantic
   program
3. move host-specific small-aggregate chunking into a later ABI elaboration
   layer, likely adjacent to LowIR-to-MachineIR lowering rather than in the
   parsed/printed LowIR module itself

This preserves the “student writes true IR” requirement without turning LowIR
into a target-specific calling-convention dump.

## Later Backlog (Not Part Of This First Tranche)

The remaining items in this file are still important, but they are not part of
the first near-term execution slice above.

### 5. Separate Value Type From Storage Type

Recent fixes showed that we still blur together:

- the scalar value type of an expression
- the in-memory storage type used by loads/stores
- the element width used by address/index arithmetic

Examples that were previously wrong:

- `char[]` subscripting lowered through `i64` indexing/load rules
- `int` objects and exception storage widened to 8-byte backing storage
- array assignment/copy treated like scalar store instead of storage copy

Possible LowIR-level improvements:

- make load/store/index operate on explicit storage element types
- make byte-size/alignment explicit for object storage
- avoid assuming that scalar values and storage slots have the same width

Related cast-conversion pain point:

- nested explicit casts currently force the frontend to remember both the
  materialized load type and the later scalar conversion chain through ad hoc
  semantic-node metadata
- a cleaner IR shape would make the sequence explicit:
  load the real storage/materialization type first, then apply a visible chain
  of width/sign/float conversions, including ordinary integer `sext`, `zext`,
  and `trunc` steps where those are semantically real and broadly useful at the
  IR boundary
- examples like `(u64)(u16)x` for `long double x` should lower as:
  `load f80`, `convert fptoui i16 f80`, then zero-extend/copy to `i64`
  instead of flattening into a single semantic node that risks losing the
  original loaded type

That would let us remove some frontend-only bookkeeping such as
`materialization_source_type` and rely more on explicit LowIR conversion steps.

The current lane landed the first concrete slice of this cleanup by making
ordinary integer width changes explicit in LowIR via `sext`, `zext`, and
`trunc`, which removes some of the frontend-only bookkeeping around nested
materialization/conversion chains.

The current lane has now also landed the next narrow slice by giving ordinary
scalar explicit casts a first-class semantic node and lowering path, so
some previously flattened conversion chains now stay explicit without trying to
solve the harder reference / aggregate / storage-shaped cases all at once.

Broader separation of value type from storage type is still one of the most
important longer-term cleanups for optimization and language-neutral lowering,
but it is more invasive than explicit declarations or linkage metadata. It
should continue from a now more self-describing IR base rather than block the
completed first concrete tranche.

### 6. Explicit Aggregate/Object Storage Operations

`copyobj` and `zeroinit` now carry explicit `<bytes>x<align>` storage spans,
but aggregate/object movement is still partly
encoded indirectly through frontend decisions.

Potential additions:

- explicit aggregate copy / move / zero-init operations with size and alignment
- explicit array copy semantics separate from scalar assignment
- explicit trivial-object initialization primitives

This would reduce the need for the frontend to special-case array/object
assignment and trivial aggregate construction.

Implementation note:

- the current lane treats explicit span-bearing bulk storage operations as the
  first concrete slice of this backlog item
- the remaining work in this section is now about whether we still need
  additional object-specific ops beyond the current `copyobj` / `zeroinit`
  surface, not about whether raw bulk storage ops should keep hiding alignment

### 7. First-Class Address Projection Kinds

The recent PA34 hosted-runtime fix required marking base-subobject expressions so we
would not treat them like reference fields needing an extra `load ptr`.

That suggests a more explicit address-projection model would help:

- field projection
- base-subobject projection
- reference-field dereference projection
- array element projection

Instead of carrying this as ad hoc flags on semantic nodes, LowIR could have
clearer projection/address ops or richer metadata around lvalue addresses.

### 8. Explicit Decay Semantics

Array/function decay currently has to be inferred from the original semantic
type and the locally lowered parameter type.

Useful explicit metadata or IR-level distinction:

- original array/function entity
- decayed pointer view of that entity
- whether a local slot holds the object itself or a pointer to it

### 9. Explicit Object Layout Metadata Where Needed

We currently re-derive layout-sensitive behavior from semantic type info and
field offsets.

Possible additions:

- explicit storage size/alignment on locals/globals/temporaries
- explicit field offset metadata at the point of projection
- explicit vtable/rtti/runtime-object import kinds

This would make object backends less dependent on frontend naming and less
likely to silently drift when layout handling changes.

### 10. Runtime/Exception Support As Structured Imports

Exception and runtime support still depend heavily on known symbol naming
patterns.

Potential structured forms:

- EH object storage kind
- RTTI import kind
- exception throw/resume hook kind
- personality/runtime helper kind

This would be cleaner than encoding those contracts only through magic symbol
names.

## Transitional ABI Strategy

Before LowIR grows explicit declarations plus ABI-role metadata, we still need a
practical way to migrate hosted output toward the real host ABI without forcing
wholesale LowIR ref churn.

The intended intermediate rule should be:

- keep the current reserved/private runtime spellings stable at the LowIR-facing
  boundary for now
- treat those spellings as interim role markers, not as permanent final ABI
  names
- migrate host interop by remapping selected roles in object/native emission
  rather than by changing textual LowIR refs first

That gives us a clean staged path:

1. keep current LowIR refs stable
2. move ordinary host-owned families such as `operator new/delete` and clear
   libc/libm-style helpers onto direct host ABI/link resolution
3. keep still-private families such as init/fini and EH support on the private
   runtime path until their ABI migration is ready
4. only later replace the current spellings with explicit declarations plus
   role metadata in the IR itself

This is consistent with the broader goals of this document:

- readability improves because the long-term target is still explicit IR roles
- validation improves because the mapping can be centralized instead of spread
  through symbol-name heuristics
- host ABI progress can happen now without destabilizing the checked-in LowIR
  corpus

## Suggested Order

The intended order for the active near-term slice is:

1. explicit declarations/imports
2. explicit symbol/ABI role metadata
3. explicit structural well-formedness contract
4. explicit call-boundary metadata

That order is important because:

- declarations/imports and roles make the IR readable enough to evolve safely
- the structural contract gives the validator a stable target
- call-boundary metadata then builds on a clearer declaration/linkage surface

## Dependency Notes

This plan is now a prerequisite input to two later efforts:

- [host-abi-runtime-followup-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-followup-plan.md)
  because the hosted EH follow-up wants explicit runtime-role handling below
  LowIR
- [pa35-optimization-buildout-process.md](/Users/vishvananda/cppgm/docs/implemented/pa35-optimization-buildout-process.md)
  because PA35 needs LowIR that is explicit enough to validate and optimize

## Priority Order By Goal

If we optimize for readability, validation, future optimization, and
multi-language reuse together, the best ordering is:

1. explicit declarations/imports
2. explicit linkage/storage/ABI role metadata
3. explicit structural well-formedness rules
4. explicit call-boundary metadata
5. value-type vs storage-type separation

Why this order:

- the first three make the IR far easier to understand and verify without
  forcing a major lowering rewrite
- the fourth unlocks safer future optimization passes
- the fifth is the major semantic cleanup, but it is easiest to do once the
  surrounding symbol/call structure is already explicit

## Student-Facing Design Constraint

LowIR should become more explicit and self-describing without forcing every
student implementation to emit one exact textual shape.

The right goal is:

- require semantic and ABI-relevant facts to be explicit in the IR
- avoid requiring one exact naming, ordering, or helper-decomposition style

In other words, the IR should make the important information visible, but the
validator should still leave room for reasonable implementation choices.

### Things We Should Not Over-Enforce

These should generally remain student choices unless a later assignment
explicitly needs stronger canonicalization:

- internal function/global symbol names
- temporary names
- stack-slot names
- block labels
- top-level definition order when order is semantically irrelevant
- block order when control-flow edges make the structure unambiguous
- exact helper-function factoring
- exact block-splitting strategy
- exact local instruction granularity when multiple forms are equally valid
- exact choice of internal synthesized runtime/helper spellings

This is especially important for teaching. If the test harness requires exact
spelling or presentation for internal artifacts, students end up reverse-
engineering the reference emitter instead of implementing the actual semantic
contract.

### Things We Should Still Enforce

The validator should focus on the information later stages actually need:

- declarations vs definitions are explicit
- imports/exports/linkage/storage class are explicit where relevant
- entrypoint/runtime/EH roles are explicit where relevant
- block/control-flow structure is well formed
- symbol references resolve correctly
- call boundaries are well described enough for later lowering/optimization
- storage/value semantics are explicit enough to avoid frontend-only guesses

## Maintainer Versus Student Validation Modes

The project should explicitly support two different LowIR validation modes.

### 1. Maintainer Strict Mode

This is the mode for the active maintainer repo.

It should keep:

- strict textual LowIR ref matching
- strict internal helper naming/order expectations where our checked-in refs
  already encode them
- exact diff visibility when a LowIR output change is subtle but real

The reason to keep this strict mode is internal quality control. We want ref
churn in the maintainer repo to be intentional and visible, especially when a
LowIR change is only a small structural shift.

So this plan does **not** propose relaxing the maintainer repo’s own LowIR
comparison policy.

### 2. Student Loose Mode

This is the mode intended for the exported student repo.

It should keep strict checking for:

- well-formedness
- declaration/definition closure
- semantic/ABI-relevant facts
- import/export/runtime-role correctness

But it should relax:

- internal helper naming
- definition ordering
- block labels/order where semantically irrelevant
- other implementation-detail presentation choices

That gives students room to make reasonable implementation decisions without
having to reverse-engineer the exact reference printer.

### Recommended Validation Split

To preserve implementation freedom while still catching real bugs, it is useful
to think in three layers:

1. **Well-formedness**
   - structural LowIR validity
   - declaration/definition closure
   - valid CFG and operand references

2. **Semantic/ABI correctness**
   - entrypoint/runtime role correctness
   - import/export/linkage correctness
   - calling/storage conventions are coherent

3. **Canonicalization**
   - naming
   - ordering
   - pretty-print style
   - helper decomposition choices

The first two layers should be the real assignment contract. The third layer
should stay optional wherever possible.

### Practical Consequence For Future IR Additions

Whenever we add a new LowIR feature, we should prefer features that expose
semantic facts directly rather than features that force a single printer style.

Good examples:

- `declare function @f(...) -> ...`
- explicit `linkage=` / `storage=` / runtime-role metadata
- explicit call-boundary metadata

Less desirable if they become mandatory canonical form:

- fixed helper names
- required top-level ordering
- required exact block labels
- required exact function splitting for internal helpers

So the design rule should be:

- make important facts explicit in the IR
- make unimportant presentation details easy to normalize away

## Long-Term Validation Strategy

For the near term, strict textual matching is still a reasonable assignment
contract. It is easy to explain, easy to debug, and keeps the test harness
simple while the IR is still evolving.

Longer term, the validator should move toward:

1. strict enforcement of semantic and ABI-relevant facts
2. structural validation of well-formed LowIR
3. relaxed treatment of implementation-detail helpers and presentation choices

This gives us a path to accept “equivalent but not textually identical” IR in
the future without trying to solve full program equivalence.

### Recommended Validation Layers

#### 1. Strict Text Mode

Keep this as the maintainer/internal mode:

- exact text match
- exact ordering
- exact helper spelling
- exact block structure

This should remain the rule for the active repo even after a student-facing
loose mode exists.

#### 2. Well-Formedness And Closure Mode

This is the long-term base validator:

- declarations vs definitions are explicit
- symbol references resolve correctly
- block/control-flow structure is valid
- temps/slots/blocks are defined before use
- runtime/import/linkage metadata is coherent

This layer should be independent of pretty-print choices and is the real IR
soundness contract.

#### 3. Relaxed Structural Mode

Once names and imports are more explicit, the validator can relax:

- internal symbol names
- temp/slot/block labels
- top-level definition order
- block order
- trivial block splitting/merging

This mode should still avoid proving arbitrary semantic equivalence. It should
only normalize differences that are clearly non-semantic.

This is the mode the export process should carry into the student repo once the
supporting metadata and validators are ready.

### Helper Strategy

Helpers need to be treated differently by category.

#### Helpers That Should Be Explicitly Standardized

These are really ABI/runtime contracts and should not remain arbitrary
spellings:

- entrypoint/runtime init/fini hooks
- EH/runtime support hooks
- imported runtime helpers
- vtable/RTTI/runtime-role objects where the role itself matters

The right solution is not “one magic spelling.” It is explicit IR role
metadata or structured declarations.

#### Helpers That Should Remain Flexible

These are implementation-detail decomposition choices and should not require
exact text equality forever:

- synthesized lambda bodies
- conversion thunks
- defaulted special-member helpers
- wrapper/trampoline helpers
- one-use forwarding helpers
- tiny CFG-shaping helpers

Different student compilers may legitimately choose different helper factoring
while still implementing the same semantics.

#### Recommended IR Support For Flexible Helpers

If we want to relax helper matching safely, LowIR should eventually make helper
classification explicit. For example, declarations/definitions could carry
helper metadata such as:

- `helper=lambda`
- `helper=thunk`
- `helper=defaulted_special_member`
- `helper=runtime_adapter`

The exact spelling is less important than the idea that the helper kind is
visible in the IR instead of inferred from symbol names.

#### Recommended Validation Policy For Helpers

For explicitly marked internal helpers, the validator should not simply ignore
their bodies. Instead it should check:

- the helper exists
- the helper is structurally well formed
- the helper has the required signature/role
- the helper obeys any helper-kind restrictions
- the helper only references valid symbols

But it should avoid enforcing:

- exact helper naming
- exact block structure
- exact decomposition/factoring strategy

That gives flexibility without letting obviously broken helper IR pass.

### What We Should Not Attempt

The validator should not try to prove full equivalence for:

- arbitrary helper extraction vs inlining
- arbitrarily different CFG shapes
- arbitrarily different call decomposition
- arbitrarily different memory operation decomposition

That quickly turns into a compiler equivalence problem rather than an
assignment harness.

So the long-term target should be:

- strict now
- self-describing IR next
- structural and helper-aware validation later
- but not full general semantic equivalence

## Notes From Recent Fixes

The latest PA31-PA34 work suggests these specific LowIR pain points:

- array/object storage size must not default to 8 bytes
- `index TYPE` semantics are important enough that the frontend should not be
  guessing incorrectly between value width and storage width
- array lvalue assignment needs object-storage semantics, not scalar semantics
- decayed array parameters need an explicit pointer/object distinction
- base-subobject addresses should not be modeled the same way as reference
  field access
- nested explicit scalar casts should survive as an explicit conversion chain
  instead of depending on flattened semantic metadata to recover the original
  materialization type

## Possible Future Verifier Checks

If LowIR gets richer metadata, a verifier could catch mistakes earlier:

- loading/storing a value type incompatible with the storage type
- indexing an array/object with the wrong element width
- using scalar assignment on aggregate/object lvalues
- treating a decayed parameter like in-place storage
- missing ABI role metadata for entrypoint/runtime hooks

## Non-Goals For Now

These notes are for later cleanup. They are not a request to rewrite LowIR
immediately, and they should not block current compiler/frontier work.
