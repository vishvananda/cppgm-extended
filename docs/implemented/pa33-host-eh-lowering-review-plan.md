# PA33 Host EH Lowering Review Plan

## Goal

Apply the same explicit case-by-case lowering review method used for PA23 to
the host-linked EH subset owned by PA33.

This is the right place for direct host-compiler comparison. Unlike PA25,
PA33 is trying to participate in the ordinary host C++ ABI/runtime surface, so
Clang `-O0` disassembly and host object metadata are meaningful oracles here.

The review should inspect:

- runtime behavior
- disassembly around throw/catch/rethrow/cleanup paths
- normalized host EH object metadata and relocations

The purpose is to catch simple backend/object-lowering issues before they only
appear through STL-heavy failures or bootstrap-scale self-host crashes.

## Assignment Boundary

This plan stays inside PA33's host-linked EH subset:

- ordinary host-linked throw/catch behavior in the exercised subset
- rethrow
- cleanup/unwind behavior visible through host-linked objects
- foreign catch-all behavior in the exercised subset
- the emitted host EH object metadata and relocations needed to make those
  behaviors work

It does not own:

- the private `cppeh` EH ABI, which belongs in PA25
- RTTI/vtable/thunk review beyond the EH interaction points that overlap with
  throw/catch behavior, which belongs in the companion PA33 RTTI/vtable/thunk
  review plan
- hosted-header/source compatibility, which belongs later in PA34/PA35

## Review Method

For each PA33 host-EH owner family:

1. identify or add the smallest useful source-driven owner test
2. build a host-compiler analogue of the same semantic case
3. inspect:
   - runtime behavior
   - disassembly for throw, landing-pad, cleanup, and rethrow paths
   - normalized object metadata and relocation facts
4. record the comparison explicitly in a tracker with:
   - owner case name
   - analogue source
   - embedded disassembly/object snippets
   - status: `reviewed / fine`, `simplified here`, `tracked gap`, or
     `not meaningfully comparable`
5. land the simplest worthwhile fixes and place each new owner at the earliest
   PA33 surface

## Oracle Shape

PA33 host-EH review should combine:

- runtime result
- disassembly for:
  - throw path
  - catch transfer / landing-pad path
  - cleanup path
  - resume / rethrow path
- normalized symbol/relocation/unwind inspection for:
  - host EH helper references such as `__cxa_*`
  - personality references
  - LSDA / call-site table presence and ranges
  - `.eh_frame` / compact-unwind / corresponding host unwind rows
  - RTTI/typeinfo references where catch typing depends on them
  - relocation classes for EH metadata edges

The metadata comparison should normalize Mach-O and ELF spelling into semantic
facts, the same way later object-inspect work normalized symbol and relocation
surfaces.

## Initial Owner Families

Start with the current PA33 EH owners and extend where coverage is thin:

1. same-TU throw/catch
2. cross-TU throw/catch
3. cleanup during unwind
4. unhandled throw
5. typed class-exception catch
6. rethrow
7. nested cleanup + rethrow
8. foreign catch-all behavior
9. ctor/dtor unwind ownership cases

## Expected Simple Gaps

The likely simple review wins are:

- call-site ranges broader or narrower than needed
- cleanup or landing-pad ranges computed from shortcuts instead of final block
  layout
- personality, LSDA, or RTTI edges emitted through the wrong relocation class
- overly indirect throw/catch helper sequences
- avoidable extra temporaries or metadata duplication around rethrow/unwind

## Deliverables

A complete PA33 host-EH lowering-review tranche should leave behind:

- a checked-in tracker with every reviewed owner family listed explicitly
- added or tightened PA33 EH owners where current coverage is thin
- normalized inspect helpers for Mach-O and ELF host EH metadata
- backend/object fixes for the simple worthwhile gaps
- README language making clear that PA33 ownership includes the observable
  host-linked EH object/runtime surface, not just final runtime behavior
