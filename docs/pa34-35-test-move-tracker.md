# pa34/35 test-move tracker

Working checklist for relocating the 185 ad-hoc pa34/35 hosted tests per
`docs/pa34-35-test-disposition.md` and the **13-item manual review** decisions of
record in `docs/assignment-restructure-plan.md`. Process: **one test at a time** —
move/rewrite, confirm it compiles/links cleanly in its new home, then check it off.

**Witness refs (`*.ref.*`, `*.ref.witness`) are golden — never regenerate.**

Status legend: `[ ]` todo · `[~]` in progress · `[x]` done · `[D]` dropped (covered) · `[-]` deferred

### Format-compatibility tiers (discovered 2026-06-01)

The pa35/link tests are **multi-TU** cppgm++ programs (`.t.1`/`.t.2`/`.shared.h`,
cross-object link + `.inspect` symbol surface + `.t.N.ref.witness`). Destinations
differ in harness, so "move" ranges from a file-copy to a full recast:

- **pa30 (sep-comp), pa32/33 (EH)** — cppgm++ multi-TU, **same `.ref.program.*` /
  `.ref.impl.*` convention**. ✅ Clean move (renumber; trim `.inspect`/witness if the
  destination harness doesn't run them).
- **pa31 abimangle** — drives the **standalone `abimangle` tool over a DSL**
  (`.t` = `c-function puts ptr:char`), not cppgm++. ❌ The 15 C++ link tests need a
  **DSL recast** *or* a **new cppgm++-driven link subdir in pa31** (harness change).
  Needs a decision before moving.
- **templating (pa18-22), core PAs** — cppgm++ **single-TU** (`.ref`/`.ref.stdout`/
  `.ref.witness`). ⚠️ Collapse cross-TU → single-TU (drops the link oracle) + new
  witness. Needs a decision.
- **LowIR band / backend** — LowIR-text format. ❌ Full recast.

**Recommended order:** pa30 (clean) → decide pa31 strategy → templating/core →
LowIR/backend → compat-features. Verify each with `dev/cppgm++` before checking off.

### `.inspect.expect` is a real oracle (NOT incidental like witness)

Unlike the witness files, `.inspect.expect` asserts the **emitted mangled-symbol
surface** (`defined_/undefined_/absent_defined_symbol_canonical <TU> <mangled>`) and
**is enforced** by pa35's harness (`run_cpphostinterop_tests_worker.pl` +
`compare_results_common.pl link_program_t`). It encodes ctor/dtor variant selection
(C2/D1), abi_tag suffixes, and Itanium substitution — i.e. the mangling oracle.

**Policy (decided 2026-06-01): SPLIT inspect-bearing behavior tests.** A test that
carries an `.inspect.expect` has a symbol-surface assertion that the destination's
toolchain harness can't check. Mangling validation is being **removed** from the
non-pa31 PAs, so such a test splits into two:
1. **behavior half** → its owning PA (pa30/templating/core): the link+run program
   with the inspect dropped (no mangling validation).
2. **mangling half** → **pa31 abimangle DSL**: the symbol-surface assertion recast
   as fact cases. Pipeline: `cppgm++ --emit-abi-facts -o facts <src>` emits the
   fact form (e.g. `terminal constructor-complete`/`-base`, `destructor-complete`/
   `-base`/`-deleting`), then `abimangle facts` produces the mangled name → `.ref`.
Worked example: `special-member-entrypoint` → pa30 `100-constructor-destructor-
defined-in-separate-object` + pa31 `100-constructor-destructor-variants`.

**19** pa35/link tests have inspect; **7** of the 15 pa31-bound do. The other
inspect-bearing, non-mangling tests need this split when reached:
`enum-class-parameter`, `local-lambda-std-function`, `special-member-entrypoint`,
`unordered-set-pointer`, `forward-helper-symbol-surface`,
`functional-include-symbol-surface`, `imported-global-got-load`,
`nested-member-lambda-std-function`, `pcrel-data-reloc`,
`placement-new-symbol-surface`, `nested-template-local-owner-symbol`.

---

## Active queue — RE-HOME the 62 no-header tests (no pa34 split required)

Source for all rows below: `pa35/tests/link/<name>.*`. Homes reflect the 13-item
review (which overrides the heuristic homes in the disposition doc).

### → pa31 abimangle (15)

abi_tag + hand-declared-`std` Itanium-substitution; cross-TU link is the
mangling-consistency oracle.

- [x] 600-hosted-abi-tag-member-link-smoke → `pa31/tests/abi/200-abi-tagged-member-and-constructor` (abi_tag on const members + copy-ctor w/ `RKS_`; distinct from `200-abi-tagged-function`; PASS)
- [ ] 600-hosted-basic-string-char-traits-abi-link-smoke  _(dedup vs `600-inline-namespace-basic-string-param`)_
- [ ] 600-hosted-initializer-list-member-definition-link-smoke
- [ ] 600-hosted-itanium-substitution-mangling-smoke
- [ ] 600-hosted-std-initializer-list-abi-link-smoke
- [ ] 600-hosted-std-vector-pair-abi-link-smoke
- [ ] 600-hosted-template-angle-vector-pair-substitution-link-smoke
- [ ] 600-hosted-vector-string-substitution-link-smoke
- [ ] 600-hosted-pair-vector-arg-ranges-link-smoke  _(review item 7: from core→abimangle)_
- [!] 700-hosted-abi-tag-class-template-member-link-smoke — **NOT a DSL fit**: tests abi_tag *suppression* (declared tag dropped on a class-template member). emit-abi-facts emits no tag → abimangle test would only check untagged template-member mangling (dup). Suppression is a cppgm++ propagation decision, not name construction. **Needs reclassification** (cppgm++ emit-abi-facts golden, or stays a symbol-emission test).
- [!] 700-hosted-abi-tag-class-template-member-template-link-smoke — same suppression issue (`Box<int>::touch<1>()` untagged expected).
- [x] 700-hosted-abi-tag-function-template-link-smoke → `pa31/tests/abi/300-abi-tagged-function-template` (`_Z15tagged_templateB9nqe220100IiET_S0_` — tag before template-args; PASS)
- [x] 700-hosted-abi-tag-operator-template-link-smoke → `pa31/tests/abi/300-abi-tagged-operator-template` (`_ZNK4LessclB9nqe220100ImmEEbRKT_RKT0_`; PASS)
- [ ] 700-hosted-local-class-template-mangling-link-smoke
- [!] 700-hosted-nested-static-abi-tag-link-smoke — same suppression issue (`Box<int>::Cache::detach` untagged expected).

### → templating (pa18-22) (13) — DROP if already covered

- [ ] 600-hosted-template-lambda-helper-link-smoke
- [ ] 600-hosted-using-namespace-vector-definition-link-smoke
- [ ] 600-inline-class-template-member-link-smoke
- [ ] 600-out-of-class-member-template-link-smoke
- [ ] 600-template-aggregate-return-link-smoke
- [ ] 600-template-inline-constructor-return-link-smoke
- [ ] 700-function-template-substitution-index-link-smoke
- [ ] 700-inline-namespace-function-template-param-link-smoke
- [ ] 700-member-template-explicit-local-typedef-link-smoke
- [ ] 700-nested-template-local-owner-symbol-link-smoke
- [ ] 700-template-disambiguator-alias-enable-if-ctor
- [ ] 700-dependent-alias-builtin-transform-link-smoke  _(review item 1: from abimangle→templating; needs compat `__remove_cvref` builtin available)_
- [ ] 700-hosted-function-reference-parameter-link-smoke  _(review item 9: from core→templating)_

### → pa30 separate-compilation (3) — DROP if already covered

- [x] 600-hosted-special-member-entrypoint-link-smoke — **SPLIT** (had inspect oracle = ctor/dtor C2/D1 variant mangling). Behavior → `pa30/tests/general/100-constructor-destructor-defined-in-separate-object` (link+run, no inspect; PASS). Mangling → `pa31/tests/abi/100-constructor-destructor-variants` (C1/C2/D0/D1/D2 via abimangle DSL; PASS). Original removed.
- [x] 700-hosted-namespaced-out-of-class-member-linkage-link-smoke → `pa30/tests/general/100-namespaced-member-function-defined-in-separate-object` (no inspect; stripped `<string>`; verified PASS)
- [x] 700-hosted-special-member-cross-tu-linkage-link-smoke → `pa30/tests/general/100-constructor-defined-in-separate-object` (no inspect; stripped `<string>`; verified PASS)

### → vtable/codegen (LowIR band) (4) — DROP if already covered

- [ ] 600-polymorphic-constructor-vtable-link-smoke
- [ ] 700-hosted-nonvirtual-mi-vtable-layout-hostcall
- [ ] 700-hosted-pure-virtual-base-vtable-link-smoke
- [ ] 700-secondary-base-virtual-dispatch-view

### → backend / lowir2native (3) — DROP if already covered

- [ ] 700-hosted-imported-global-got-load-link-smoke
- [ ] 700-hosted-pcrel-data-reloc-link-smoke
- [ ] 700-thread-local-store-register-pressure-runtime-smoke

### → hosted-compat features PA (14) — home dir TBD (the 248-directed prereq set)

`__builtin_*` ×12 and `gnu-asm` symbol-label ×2; verified end-to-end, no `#include`.

- [ ] 600-builtin-memcpy-strlen-link-smoke
- [ ] 600-builtin-operator-new-delete-link-smoke
- [ ] 600-gnu-asm-label-overload-link-smoke
- [ ] 600-gnu-asm-leading-underscore-label-link-smoke
- [ ] 700-dependent-alias-pack-invoke-result-link-smoke
- [ ] 700-hosted-builtin-bitops-promote-runtime-smoke
- [ ] 700-hosted-builtin-flt-rounds-runtime-smoke
- [ ] 700-hosted-builtin-fp-classification-runtime-smoke
- [ ] 700-hosted-builtin-fpclassify-runtime-smoke
- [ ] 700-hosted-builtin-huge-val-runtime-smoke
- [ ] 700-hosted-builtin-mul-overflow-runtime-smoke
- [ ] 700-hosted-builtin-nans-runtime-smoke
- [ ] 700-hosted-builtin-popcountg-runtime-smoke
- [ ] 700-hosted-builtin-signbit-runtime-smoke

### → core PA (specific owner) / DROP (7) — per 13-item review

- [ ] 700-initlist-const-char-pointer-runtime-smoke → core: initializer_list codegen  _(item 3)_
- [ ] 600-delete-class-pointer-destroys-object-runtime → core: new/delete + ctor/dtor lifetime  _(item 4)_
- [ ] 600-hosted-enum-class-parameter-link-smoke → core: scoped enum + cross-TU  _(item 5)_
- [ ] 600-hosted-member-operator-bang-link-smoke → core: operator overload + out-of-class def  _(item 6)_
- [ ] 700-hosted-alignas-class-layout-link-smoke → core: alignas/layout (recast static_assert)  _(item 8)_
- [ ] 700-pointer-predecrement-reference-argument-link-smoke → core: expression codegen  _(item 12)_
- [ ] 700-wide-string-literal-runtime-smoke → core: wide string-literal (recast static_assert)  _(item 13)_

### → hosted L2 (header-bearing; mis-detected as no-header) (3) — defer with L2 batch

- [-] 700-hosted-inline-header-odr-link-smoke  _(item 2: `.shared.h` pulls `<deque>/<map>/<memory>/<string>`)_
- [-] 700-hosted-header-inline-unemitted-callee-signature  _(item 10: `.h` pulls `<streambuf>`)_
- [-] 700-hosted-user-declared-trivial-dtor-return-link-smoke  _(item 11: back-end ABI / calling convention)_

---

## Deferred — header-bearing hosted tests (need pa34 split) — 123 + the 3 above

Listed in `docs/pa34-35-test-disposition.md`; do **after** the hosted L1/L2 split.

- [-] Hosted L1 — front-end conformance (`static_assert`, perf-gated) — 42
- [-] Hosted L2 — back-end / ABI interop (link + run + assert) — 50 (+3 from above = 53)
- [-] Hosted TRIAGE — weak/cheatable (upgrade or DROP near-dups) — 31

---

## Tally

| Destination | count |
|---|---|
| pa31 abimangle | 15 |
| templating (pa18-22) | 13 |
| pa30 separate-compilation | 3 |
| vtable/codegen (LowIR band) | 4 |
| backend / lowir2native | 3 |
| hosted-compat features PA | 14 |
| core PA / DROP | 7 |
| → hosted L2 (reclassified header-bearing) | 3 |
| **no-header RE-HOME subtotal** | **62** |
| header-bearing (deferred) | 123 |
| **total ad-hoc** | **185** |
