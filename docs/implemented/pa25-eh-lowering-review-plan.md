# PA25 EH Lowering Review Plan

## Goal

Review PA25's private EH pipeline case-by-case, but keep the oracle aligned
with what PA25 actually owns.

PA25 is a compiler-private exception/runtime ABI, not a host-compatible EH
surface. That means direct Clang/host-EH comparison is only weakly informative
here. The useful PA25 review questions are:

- does the runtime behavior match the documented private EH semantics?
- is the internal object/runtime support surface deterministic and minimal?
- do cleanup/resume/control-transfer paths have an obviously unnecessary or
  structurally wrong lowering?

So this plan is a private-EH sanity review, not a "match host C++ EH output"
project. The Clang-comparison style review belongs later in the host-linked EH
lane under PA33.

## Assignment Boundary

This plan stays inside PA25's owned surface:

- `eh_try`
- `eh_cleanup`
- `eh_end`
- `throw`
- `exception`
- `resume`
- the private `cppeh -c -> cppeh` compile/link/runtime path
- the private `__cppgm_eh_*` role family and the object/runtime metadata needed
  to make that path work

It does not own:

- host final-link ABI behavior for `cppgm++ -c` objects, which belongs in PA33
- hosted-header/source acceptance, which belongs later in PA34/PA35

## Review Method

For each PA25 owner family:

1. identify or add the smallest useful LowIR owner test
2. inspect our output at three layers:
   - linked runtime behavior
   - disassembly around throw/catch/cleanup/resume paths
   - normalized private object/runtime metadata
3. record the review explicitly in a tracker with:
   - owner case name
   - owner source
   - embedded disassembly/object snippets
   - status: `reviewed / fine`, `simplified here`, `tracked gap`, or
     `not meaningfully comparable`
4. land the simplest worthwhile fixes and add/adjust owner tests at the
   earliest PA25 surface

Clang/LLVM analogues may still be recorded opportunistically where they help
explain a control-flow shape, but they are not the primary oracle for this
plan.

## Oracle Shape

PA25 review should combine:

- runtime result:
  - program exit status
  - program stdout/stderr when relevant
- disassembly:
  - throw path
  - catch transfer path
  - cleanup/resume path
- normalized private object/runtime metadata:
  - EH support section presence
  - private EH support symbol imports/definitions
  - relocation edges into the private EH role family
  - unwind/support tables used by the private `cppeh` pipeline

This metadata should be normalized semantically, but it does not need to mimic
host DWARF/Itanium EH naming or section structure. That host-compatible review
is a later PA33 concern.

## Initial Owner Families

Start with the PA25 owners that already exist or are obvious from the README:

1. same-function throw/catch
2. cross-function catch
3. cross-object catch
4. cleanup + resume
5. unhandled throw
6. exception object materialization width and address stability
7. nested rethrow / rethrow after partial cleanup
8. multiple catch clauses / catch ordering, if/when the LowIR surface grows to
   express them cleanly
9. destructor-on-unwind ownership cases, if/when the private EH pipeline grows
   beyond the current scalar/pointer payload subset

## Expected Simple Gaps

Based on the current private-EH pipeline, the likely simple gaps to look for
are:

- over-materialized throw paths
- call-site ranges that are broader or narrower than needed
- cleanup ranges or landing-pad cutoffs computed indirectly instead of from the
  final MIR block layout
- private EH support edges emitted through the wrong relocation class
- synthetic helper calls or temporaries where a more direct private lowering is
  already possible

## Deliverables

A complete PA25 lowering-review tranche should leave behind:

- a checked-in tracker with every reviewed owner family listed explicitly
- new or tightened PA25 owner tests for any missing cases discovered during the
  review
- backend/object fixes for the simple worthwhile gaps
- README language making clear that PA25 correctness is not only runtime
  behavior but also the owned private EH object/runtime metadata surface

## Explicit Non-Goal

This plan is not responsible for comparing PA25 output against Clang's host EH
metadata/disassembly as though they should match one another. That later review
belongs in a separate PA33 host-EH lowering plan, where the assignment boundary
actually owns ordinary host-linked EH behavior.

## Completion

This plan was completed by:

- adding a normalized private-EH object-facts helper in
  [`dump_private_eh_object_facts.py`](/Users/vishvananda/cppgm/scripts/dump_private_eh_object_facts.py)
  with parser/unit coverage
- checking in the explicit case-by-case review tracker in
  [`pa25-private-eh-lowering-tracker.md`](/Users/vishvananda/cppgm/docs/implemented/pa25-private-eh-lowering-tracker.md)
- reviewing all five current PA25 owner tests (`100` through `140`) with
  runtime, link-map, disassembly, and normalized private object metadata
- confirming that the current owner surface does not expose another simple
  worthwhile backend/object fix at this boundary
- validating the PA25 suite on both the macOS host target and Linux Clang 22
