# LowIR Boundary And Object-Format Plan

## Goals

1. Preserve PA29 as the first compile/link driver assignment without forcing the
   host object ABI before the ABI and host-object assignments.
2. Make the LowIR text boundary explicit: implementation code may use a typed
   internal model, but every backend-relevant fact needed after source lowering
   must survive LowIR serialization and parsing.
3. Mirror the PA30 `abimangle` scaffold style for LowIR and MIR by providing
   student-facing typed model headers that the maintainer implementation also
   uses as the IR boundary surface.
4. Clarify that hosted compatibility uses the same source-to-LowIR path as
   ordinary compilation and must not carry extra hosted-only facts around the
   textual LowIR contract.
5. Add a student-visible validation hook that catches object-generation facts
   being carried through hidden side channels instead of textual LowIR.

## Step 1: PA29 Object-Format Wording

PA29 remains implementation-defined. It comes before PA30 ABI naming and PA31
host object emission, so requiring host-linker-compatible `.o` output here would
collapse several later assignments into PA29.

Update PA29 to say:

- `cppgm++ -c` writes an implementation-defined compiler object file.
- The PA29 object file only has to be consumed by `cppgm++` itself.
- The checked-in PA29 harness uses `.obj` for this internal object format.
- Host-linker-compatible relocatable objects begin in PA31/PA32 and should not
  be backported into PA29's contract.

## Step 2: LowIR/MIR Student Scaffolding

Add exported support headers:

- `dev/src/ir_symbol_model.h`
- `dev/src/lowir_model.h`
- `dev/src/mir_model.h`
- `dev/src/x86_register_model.h`

These headers should be model/API scaffolds, not display-only sketches. The
maintainer implementation should route LowIR parsing, LowIR optimization, MIR
lowering, and MIR serialization through this surface so the scaffold is
validated the same way as the PA30 ABI fact model. Semantic-only details must be
converted into the small IR exported-symbol payload before crossing the LowIR
boundary.

The export script must include these headers in `dev_support_files`.

## Step 3: LowIR Text Boundary Documentation

Update PA13 and `pa13/lowir.md` to say:

- LowIR is the serialized form of an internal typed program representation.
- Object/native backends may operate on a typed model, but that model must be
  reconstructible from textual LowIR.
- If a later backend needs a symbol, linkage, layout, TLS, runtime-role, debug,
  or object-emission fact, that fact belongs in LowIR text.
- No source-semantic side channel may affect backend lowering after the LowIR
  boundary.

Update PA28, PA37, PA38, and PA39 to carry that contract forward for native
lowering, MIR dumping, LowIR optimization, machine optimization, and self-host
debugging.

PA38 should also make clear that machine-backend optimization is shared backend
work. The first visible tests live on `lowir2native`, but the optimized MIR path
must remain usable by `cppgm++` object and link-driver modes instead of becoming
a standalone display transform.

## Step 4: Hosted Compatibility Clarification

Update PA34-PA36 to say hosted compatibility extends the same frontend,
semantic, and LowIR lowering path. `cppgm++ --emit-lowir` for hosted-compatible
source must be representative of the object path. Hosted mode may change include
search and supported declarations, but it must not create a parallel object-only
compiler route.

## Step 5: Roundtrip Validation

Add a PA37 harness check that compiles a source file both ways:

1. normal `cppgm++ -c`
2. `cppgm++ -c --roundtrip-object-lowir`, which forces object emission through
   textual LowIR serialization and parsing

The two object outputs must match byte-for-byte. This keeps
`--roundtrip-object-lowir` as a harness validation hook rather than a public
driver mode, while making the no-backchannel invariant visible in the exported
assignment.

## Validation

- Run syntax checks for new scripts.
- Run focused PA37 roundtrip validation.
- Run focused PA37/PA38 tests after converting the maintainer implementation to
  the scaffold model headers.
- Run `git diff --check`.
