# X86 vs Object-Format Boundary Plan

## Purpose

This plan defines the backend refactor needed to keep:

- x86-specific lowering

separate from:

- Mach-O / ELF object-format encoding

The immediate motivation is recent machine-IR and vtable/runtime work. Those
changes are valuable, but they increase the risk that Mach-O assumptions leak
upward into code that should only be expressing x86 codegen or relocation
intent.

If that continues, Linux/ELF support becomes harder every time we touch:

- vtable address points
- RTTI/data references
- GOT / PC-relative data access
- weak/coalesced object ownership
- host-EH metadata sections

This plan exists to make the backend seam explicit before more platform-specific
behavior gets embedded into the wrong layer.

## Main Goal

The backend should describe three distinct layers:

1. **LowIR / semantic intent**
   - what symbol or object is being referenced
   - what role it has (code/data/vtable/RTTI/runtime/helper)
   - what call/data/reference semantics are required

2. **x86 machine lowering**
   - instruction selection
   - calling convention and register conventions
   - abstract relocation/reference intent
   - address-point vs ordinary symbol reference intent

3. **object-format encoding**
   - Mach-O section choice and relocation records
   - ELF section choice and relocation records
   - weak/coalesced/COMDAT encoding
   - exact symbol-table binding/visibility encoding
   - unwind/auxiliary metadata section encoding

The central rule is:

- x86 lowering may choose *what kind of reference* is needed
- object-format code should choose *how that reference is encoded*

## Why This Matters Now

Recent and active work is stressing exactly the wrong seam:

- vtable emission / imported address-point use
- runtime symbol policy and hosted ABI references
- Linux/ELF platform recovery
- host-EH metadata, still split across Mach-O vs ELF needs

If we do not separate the layers now, each new fix risks baking one platform's
encoding assumptions into:

- `lowir_machine_ir.cpp`
- `machine_ir.cpp`
- `lowir_object_backend.cpp`

That would make:

- Linux/ELF host-ABI completion
- future non-Mach-O validation
- backend-quality work in PA23

all much harder.

## Intended Ownership Boundary

### X86-Owned

The x86 layer should own:

- register use and calling convention
- instruction opcode and operand width selection
- stack-frame/address calculation shape
- direct vs indirect call shape
- abstract relocation/reference intent, such as:
  - direct code symbol call
  - PC-relative code reference
  - GOT-backed data reference
  - address-point data reference
  - ordinary data symbol reference
  - runtime helper / imported symbol reference

It should **not** need to know:

- Mach-O section names
- ELF section names
- exact relocation record kinds by format
- weak-def/coalesced encoding details
- COMDAT/group-section rules
- unwind section layout details

### Object-Format-Owned

The format layer should own:

- Mach-O vs ELF section layout
- relocation record encoding
- symbol binding/visibility encoding
- weak/coalesced/COMDAT representation
- platform-specific auxiliary section preservation
- object writer/parser round-tripping

It should not need to rediscover semantic meaning from symbol spelling if the
machine/object boundary already carries the right roles and relocation intents.

## Current Suspect Files

This plan is mainly about clarifying responsibilities across:

- [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
- [machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/machine_ir.cpp)
- [machine_ir.h](/Users/vishvananda/cppgm/dev/src/machine_ir.h)
- [lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp)
- [machine_object.cpp](/Users/vishvananda/cppgm/dev/src/machine_object.cpp)
- [macho_writer.cpp](/Users/vishvananda/cppgm/dev/src/macho_writer.cpp)
- [elf_writer.cpp](/Users/vishvananda/cppgm/dev/src/elf_writer.cpp)

The likely smell to look for is:

- target-independent/x86 code branching directly on `"macos"` or `"linux"`
  when it is really deciding a relocation or object-format encoding choice

That does not mean every `target == ...` check is wrong.
Some checks belong naturally in the format layer.
The problem is when those checks leak into x86 lowering or generic machine-IR
construction.

## Refactor Direction

## Progress

### Completed

This plan is complete.

The two concrete boundary leaks it targeted are now separated:

- imported symbol/data references use a format-neutral machine/object intent
  (`indirect rel32`) instead of x86-side Mach-O/ELF terminology
- host-EH auxiliary section construction no longer lives in
  [lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp);
  the Mach-O/ELF-specific compact-unwind, LSDA, `.eh_frame`, and
  personality-reference packaging now lives in
  [host_eh_object_sections.cpp](/Users/vishvananda/cppgm/dev/src/host_eh_object_sections.cpp)

The practical result is:

- x86 lowering continues to choose code/data/reference intent
- object-format code now owns the remaining Mach-O/ELF host-EH auxiliary
  section encoding details

After this refactor, the only target checks left in
[lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp)
are ordinary host OS/runtime choices, not Mach-O vs ELF object-format
encoding decisions.

### Initial Imported-Reference Slice Landed

The first useful seam cleanup has now landed in the backend:

- x86 lowering no longer names imported data references as a Mach-O/ELF
  `GOT_LOAD` concept at the machine/object boundary
- the shared relocation/fixup vocabulary now uses a format-neutral
  `indirect rel32` intent for imported symbol references
- Mach-O and ELF translation still map that intent to their concrete relocation
  encodings (`GOT_LOAD` / `GOTPCREL`) in the object-format layer

That change is intentionally narrow, but it is important because it moves one
active imported-reference family out of:

- x86-lowering terminology that directly described one platform encoding

and into:

- an abstract machine/object intent that both object formats can encode

Validation status for that slice:

- macOS host subset: `pa23`, `pa24`, `pa25`, `pa33`, `pa35` clean
- Linux Clang 22 subset: `pa23`, `pa24`, `pa25`, `pa33` clean
- Linux `pa35` remains baseline-broken in the same hosted-runtime cases on a
  clean checkout, so the imported-reference slice did not introduce a new ELF
  regression there

### Host-EH Auxiliary-Section Slice Landed

The remaining high-value leak from this plan also landed:

- Mach-O-specific compact-unwind / LSDA construction
- ELF-specific LSDA / `.eh_frame` / personality-reference packaging

were moved out of
[lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp)
into
[host_eh_object_sections.cpp](/Users/vishvananda/cppgm/dev/src/host_eh_object_sections.cpp),
which is explicitly the object-format side of the backend seam.

Validation for that completion slice used a lighter-weight but sufficient
runtime subset on both host and Linux:

- full `pa24`, `pa25`, and `pa33`
- targeted hosted-EH `pa35` owners:
  - `668`
  - `675`
  - `683`
  - `684`
  - `690`
  - `709`

That subset passed on:

- macOS host
- Linux Clang 22 in Docker with an isolated object root

### Phase 1: Document And Classify Existing Reference Kinds

Before moving code, identify the concrete reference families we already have:

- direct function call reference
- indirect function value call
- ordinary data symbol address/reference
- imported/global GOT-style reference
- vtable address-point reference
- RTTI/typeinfo reference
- runtime helper reference
- startup/runtime hook reference

The first implementation step should be to make those families explicit in the
machine/object boundary rather than leaving them as:

- symbol names plus ad hoc conditionals
- backend-local comments
- scattered `target == ...` branches

### Phase 2: Introduce Format-Neutral Reference/Relocation Intents

Add a small format-neutral abstraction in the machine/object boundary that can
say things like:

- code symbol call
- data symbol absolute address
- data symbol pcrel reference
- GOT-indirected data reference
- address-point reference

This should be rich enough for:

- vtable / RTTI references
- current hosted-runtime/global import patterns
- future ELF host-EH metadata references

without forcing `lowir_machine_ir.cpp` to know Mach-O vs ELF encoding details.

### Phase 3: Push Mach-O / ELF Encoding Decisions Down

After the intent layer exists:

- move Mach-O-specific relocation/section decisions out of x86 lowering paths
- move ELF-specific relocation/section decisions out of x86 lowering paths
- keep format writers authoritative for final encoding

The likely code movement is:

- less platform branching in `lowir_machine_ir.cpp`
- more centralized format translation in:
  - `machine_object.cpp`
  - `macho_writer.cpp`
  - `elf_writer.cpp`

### Phase 4: Revisit Vtable/RTTI/Data Address-Point Handling

Once the format-neutral intent layer exists, re-audit the active vtable/runtime
paths:

- address-point references
- vtable-owner data emission
- RTTI pointer/reference payloads
- GOT/PC-relative imported data loads

The success condition is:

- x86 lowering says *what kind of reference it needs*
- object-format code decides *how that is spelled and relocated*

### Phase 5: Align Linux Recovery And Host-EH Work With The New Boundary

This plan should then feed directly into:

- [linux-platform-recovery-plan.md](/Users/vishvananda/cppgm/docs/implemented/linux-platform-recovery-plan.md)
- the remaining Linux/ELF host-EH path

That is one of the main reasons to do this split early.
The ELF work should not be forced to copy Mach-O-shaped assumptions upward into
the x86 lowering layer.

## Testing Strategy

The refactor should be validated at three levels:

1. machine-IR / object-surface owners
   - reduced MIR and object inspections
   - especially vtable / RTTI / imported-data cases

2. format-specific object tests
   - Mach-O object/relocation checks
   - ELF object/relocation checks

3. runtime/link canaries
   - existing hosted ABI tests
   - Linux platform-recovery canaries

This plan should avoid a giant “move everything, then hope bootstrap works”
approach. Instead, each new reference-intent family should get:

- a reduced owner test
- format-specific encoding proof
- then broader hosted validation

## Relation To Other Plans

- [pa23-machine-ir-quality-plan.md](/Users/vishvananda/cppgm/docs/implemented/pa23-machine-ir-quality-plan.md)
  should continue to own backend-quality rules such as compare lowering and
  register/path quality.
  It should not also have to own Mach-O vs ELF boundary cleanup.

- [lowir-evolution-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-evolution-plan.md)
  should continue to own IR semantics and role metadata.
  This plan begins below that layer, at the machine/object boundary.

- [linux-platform-recovery-plan.md](/Users/vishvananda/cppgm/docs/implemented/linux-platform-recovery-plan.md)
  depends on this seam being clear, but should not absorb the whole backend
  refactor. Linux recovery needs the outcome, not ownership of the architecture
  cleanup itself.

## First Concrete Slice

The first implementation slice from this plan should be:

1. inventory the current machine/object reference families used by:
   - vtable address-point references
   - RTTI references
   - imported runtime/global data references
2. identify which of those still branch on Mach-O vs ELF in x86-side code
3. introduce the smallest format-neutral reference-intent representation needed
   for one of those families
4. move just that family down to the object-format layer with reduced owner
   tests

The best first family is probably:

- imported data / address-point reference intent

because it sits directly in the blast radius of both:

- current vtable work
- Linux/ELF recovery
