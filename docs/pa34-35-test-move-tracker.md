# pa34/35 test-move tracker

Working checklist for relocating the 185 ad-hoc pa34/35 hosted tests per
`docs/pa34-35-test-disposition.md` and the **13-item manual review** decisions of
record in `docs/assignment-restructure-plan.md`. Process: **one test at a time** —
move/rewrite, confirm it compiles/links cleanly in its new home, then check it off.

**Witness refs (`*.ref.*`, `*.ref.witness`) are golden — never regenerate.**

Status legend: `[ ]` todo · `[~]` in progress · `[x]` done · `[D]` dropped (covered) · `[-]` deferred

### abimangle conversion process (pa31)

1. **Generate facts with `--emit-abi-facts`, don't hand-author them.**
   `dev/cppgm++ --emit-abi-facts -o <facts.t> <source.cpp>` emits the fact DSL for the
   entity directly from cppgm++'s own semantic model (correct inline-namespace spelling,
   substitution structure, ctor/dtor variants, abi-tags). Hand-authoring is error-prone;
   only edit the emitted facts to trim to the single name under test.
2. **One name per test file.** Each `.t`/`.ref` covers exactly one mangled name. Only
   *very closely related* names from the **same class** (e.g. multiple constructors,
   ctor + dtor variants of one type) may share one file as multiple `case` blocks.
   Otherwise split into separate files (free fn vs member, distinct owners → separate).
3. **Oracle = clang on this host (libc++ `__1`).** Verify each `.ref` against
   `clang++ -c … && nm | c++filt`. Files: `.t`, `.ref`, `.ref.exit_status` (=`EXIT_SUCCESS`)
   are the only git-tracked ones; `.my*` are harness-generated.

### templating/core rehoming process (pa14-22+)

- Collapse cross-TU link tests to single-TU; generate golden `.ref` via `cppgm++ --emit-lowir -O0`; verify the owning PA's `make test` (and `clang -fsyntax-only` for semantics).
- **Run `python3 scripts/audit_pa_feature_placement.py` after placing** to catch a test placed before its feature's owning PA/cluster (`cluster-early`/`violation`). Move to the flagged owner PA/cluster. `semantic-owner` rows are informational (place by the enclosing LowIR feature).
- EH/`try`/`catch` is now tracked as `exception.try_catch` (owner `pa25` = `cppeh`). **Caveat:** it over-flags non-LowIR tool-PAs that legitimately use EH for their own purpose — `pa10` (`--emit-ast`) parses the syntax, `pa13` (`lowir2cy86`) consumes EH LowIR. Treat flags only in LowIR-source PAs (pa14-22, pa26-29) as real placement risks; a test whose *headline* feature is later than pa25 (e.g. a lambda test, pa26) stays at that later PA since pa25 EH is then satisfied.

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
- [x] 600-hosted-basic-string-char-traits-abi-link-smoke → `pa31/tests/abi/300-std-string-parameter-substitution` (`accept_basic_string(const std::string&, std::string&)` → `…basic_stringIc…EERS5_`; the `RS5_` back-ref exercises the repeated-substitution fix; clang/libc++ `__1`-verified; PASS). Distinct from `600-inline-namespace-basic-string-param` (`__cxx11`/getline).
- [x] 600-hosted-initializer-list-member-definition-link-smoke → mangling: `pa31/tests/abi/300-std-initializer-list-member-parameter` (`_ZN30InitializerListMemberLinkSmoke5totalESt16initializer_listIiE`, clang-verified; PASS). **NOTE: source `main` asserts `total({1,2,3})==6` (real behavior) — behavior half needs a core/templating home or coverage confirmation before deleting source.**
- [x] 600-hosted-itanium-substitution-mangling-smoke → `pa31/tests/abi/300-namespace-class-and-string-substitution` (`mixed_subst_one(const nsrepro::Program&, const std::string&)` → inner `NS3_`; clang-verified; PASS). **Exposed + FIXED a real cppgm++-vs-clang bug** (abimangle-only): cv/ref param types weren't registered as Itanium subst candidates → inner `std::__1` ref was `NS1_` vs clang `NS3_`. Fixed in `abi_mangle.cpp` (`with_type_substitution_key` on cv/ref/pointer builders); codegen was already correct (no strict-audit churn). See memory `cvref-substitution-candidate-bug`.
- [x] 600-hosted-std-initializer-list-abi-link-smoke → `pa31/tests/abi/300-std-initializer-list-parameter` (`_Z20sum_initializer_listSt16initializer_listIiE`, clang-verified; PASS). Pure link/mangle smoke (body `(void)values;return 0;` — no behavior); source removable.
- [x] 600-hosted-std-vector-pair-abi-link-smoke → `pa31/tests/abi/300-std-vector-pair-substitution` (`accept_std_pair_ranges(const std::vector<std::pair<unsigned long,unsigned long>>&)` → `…6vectorINS_4pairImmEENS_9allocatorIS2_EEEE`; pair + allocator back-ref; clang-verified; PASS).
- [D] 600-hosted-template-angle-vector-pair-substitution-link-smoke — **DUP of `300-std-vector-pair-substitution`**: its substitution-bearing type is `std::vector<std::pair<std::size_t,std::size_t>>` ≡ `vector<pair<unsigned long,unsigned long>>` (size_t=m). Remaining params are cppgm-internal types (IRecogTokenSequence/NameLookup), not a mangling concern. Covered.
- [x] 600-hosted-vector-string-substitution-link-smoke → `pa31/tests/abi/300-std-vector-string-substitution` (`accept_texts(const std::vector<std::string>&)` → `…6vectorINS_12basic_string…EENS4_IS6_EEEE`; nested string+allocator back-refs; clang-verified; PASS).
- [x] 600-hosted-pair-vector-arg-ranges-link-smoke → `pa31/tests/abi/300-user-inline-namespace-substitution` (`accept_arg_ranges(helper_inline_ns::v1::vec<helper_inline_ns::v1::pair<unsigned long,unsigned long>>&)` → `RN16helper_inline_ns2v13vecINS0_4pairImmEEEE`; user inline-ns `v1` mangled + `S0_` back-ref; clang-verified; PASS). _(review item 7)_ Behavior half (aggregate value) → `pa20/tests/general/100-inline-namespace-aggregate-member-value` (static_assert; **placement audit moved pa19→pa20**: full-constexpr fn owns pa20). Source removed.
- [-] 700-hosted-abi-tag-class-template-member-link-smoke — **L2/host-compat (stay in pa35/link)**: validates abi_tag *suppression* on `Box<int>::touch()` (clang + cppgm++ both emit untagged, verified). Not a pa31 fit (abimangle can't model dropping a declared tag → dup); abi_tag is pa34 (`support.attribute`) so no pre-pa34 LowIR home, and pa34 is static_assert-only (can't inspect a symbol). Needs the inspect oracle → deferred L1/L2 recategorization.
- [-] 700-hosted-abi-tag-class-template-member-template-link-smoke — **L2/host-compat (stay in pa35/link)**: validates abi_tag *suppression* on `Box<int>::touch<1>()` (clang + cppgm++ both emit untagged, verified). Not a pa31 fit (abimangle can't model dropping a declared tag → dup); abi_tag is pa34 (`support.attribute`) so no pre-pa34 LowIR home, and pa34 is static_assert-only (can't inspect a symbol). Needs the inspect oracle → deferred L1/L2 recategorization.
- [x] 700-hosted-abi-tag-function-template-link-smoke → `pa31/tests/abi/300-abi-tagged-function-template` (`_Z15tagged_templateB9nqe220100IiET_S0_` — tag before template-args; PASS)
- [x] 700-hosted-abi-tag-operator-template-link-smoke → `pa31/tests/abi/300-abi-tagged-operator-template` (`_ZNK4LessclB9nqe220100ImmEEbRKT_RKT0_`; PASS)
- [ ] 700-hosted-local-class-template-mangling-link-smoke
- [-] 700-hosted-nested-static-abi-tag-link-smoke — **L2/host-compat (stay in pa35/link)**: validates abi_tag *suppression* on `Box<int>::Cache::detach` (clang + cppgm++ both emit untagged, verified). Not a pa31 fit (abimangle can't model dropping a declared tag → dup); abi_tag is pa34 (`support.attribute`) so no pre-pa34 LowIR home, and pa34 is static_assert-only (can't inspect a symbol). Needs the inspect oracle → deferred L1/L2 recategorization.

> **Note (2026-06-02):** of the substitution subset, **6 are actually header-bearing**
> (`<string>`/`<vector>`/`<utility>`), so they were mis-listed as no-header:
> `600-hosted-itanium-substitution-mangling-smoke`, `…basic-string-char-traits…`,
> `…vector-string-substitution…`, `…std-vector-pair…`, `…template-angle-vector-pair…`,
> `…std-initializer-list…`. Their **mangling is fully captured in pa31** (above), so the
> retained source is now a pure hosted **L2 link/run behavior** test — **moved to the
> deferred hosted-L2 batch**, inspect/mangling no longer needed there. Sources kept (cannot
> move pre-pa34). Only the two genuinely no-header behavior tests were fully rehomed +
> source-removed: `…initializer-list-member…` → pa30 + pa31; `…pair-vector-arg-ranges…` →
> pa20 (static_assert) + pa31.

### → templating (pa18-22) (13) — DROP if already covered

- [x] 600-hosted-template-lambda-helper-link-smoke → `pa27/tests/general/200-function-template-lambda-decltype-eh-fallback` (lambda arg + decltype(fn()) return + try/catch fallback; **placement audit moved pa22→pa27**: lambda owns pa26, try/catch owns pa27 → latest wins; pa27 PASS)
- [x] 600-hosted-using-namespace-vector-definition-link-smoke → `pa21/tests/general/100-using-directive-inline-namespace-class-template` (collapsed single-TU; using-directive finds inline-ns `vec`; cppgm+++clang clean; pa21 PASS)
- [D] 600-inline-class-template-member-link-smoke — **exact duplicate** of existing `pa21/tests/general/400-inline-class-template-member-required-output.t`; source removed.
- [x] 600-out-of-class-member-template-link-smoke → `pa18/tests/general/300-out-of-class-member-template-definition` (out-of-class member-template def; **placement audit moved cluster 100→300**: `template.member_template` owns pa18:300; pa18 PASS)
- [x] 600-template-aggregate-return-link-smoke → `pa18/tests/general/100-function-template-returns-aggregate-class-template` (fn template returns aggregate via brace-init; pa18 PASS)
- [x] 600-template-inline-constructor-return-link-smoke → `pa18/tests/general/100-function-template-returns-constructed-class-template` (fn template returns class via ctor + nested aggregate; pa18 PASS)
- [ ] 700-function-template-substitution-index-link-smoke
- [ ] 700-inline-namespace-function-template-param-link-smoke
- [x] 700-member-template-explicit-local-typedef-link-smoke → `pa22/tests/general/100-member-template-explicit-local-typedef-call` (explicit `.template` w/ local typedef; pa22 PASS individually)
- [x] 700-nested-template-local-owner-symbol-link-smoke → `pa22/tests/general/100-nested-class-template-local-class-argument` (nested class template + local class as template arg; pa22 PASS individually)
- [D] 700-template-disambiguator-alias-enable-if-ctor — covered by pa22 enable_if/alias + `__is_constructible` suite (300-constructor-template-*-enable-if-*, 500-qualified-member-function-value-fallback-sfinae); source removed.
- [D] 700-dependent-alias-builtin-transform-link-smoke — covered: `__remove_cvref` alias already exercised by `pa22/500-internal-remove-cvref-alias-sfinae`; source removed. _(review item 1)_
- [x] 700-hosted-function-reference-parameter-link-smoke → `pa21/tests/general/400-function-reference-template-parameter` (fn-ref template deduction; restores live coverage — existing `400-function-reference-deduction` is an orphaned .ref with **no .t**; pa21 PASS) _(review item 9)_

### → pa30 separate-compilation (3) — DROP if already covered

- [x] 600-hosted-special-member-entrypoint-link-smoke — **SPLIT** (had inspect oracle = ctor/dtor C2/D1 variant mangling). Behavior → `pa30/tests/general/100-constructor-destructor-defined-in-separate-object` (link+run, no inspect; PASS). Mangling → `pa31/tests/abi/100-constructor-destructor-variants` (C1/C2/D0/D1/D2 via abimangle DSL; PASS). Original removed.
- [x] 700-hosted-namespaced-out-of-class-member-linkage-link-smoke → `pa30/tests/general/100-namespaced-member-function-defined-in-separate-object` (no inspect; stripped `<string>`; verified PASS)
- [x] 700-hosted-special-member-cross-tu-linkage-link-smoke → `pa30/tests/general/100-constructor-defined-in-separate-object` (no inspect; stripped `<string>`; verified PASS)

### → vtable/codegen (LowIR band) (4) — DROP if already covered

- [D] 600-polymorphic-constructor-vtable-link-smoke — covered by `pa17/400-inline-polymorphic-constructor-vtable` (abstract base + override + vtable ctor) + `pa17/400-header-out-of-class-virtual-vtable`; source removed.
- [x] 700-hosted-nonvirtual-mi-vtable-layout-hostcall → `pa17/tests/general/300-multiple-inheritance-vtable-layout` (MI vtable layout, distinct; pa17 PASS)
- [D] 700-hosted-pure-virtual-base-vtable-link-smoke — covered by pa17 pure-virtual/virtual-dispatch suite (pa17/400 abstract base + 300-virtual-call-*); source removed.
- [x] 700-secondary-base-virtual-dispatch-view → `pa17/tests/general/300-secondary-base-virtual-class-return` (MI secondary-base dispatch + class return; distinct; pa17 PASS)

### → backend / lowir2native (3) — DROP if already covered

- [ ] 700-hosted-imported-global-got-load-link-smoke
- [ ] 700-hosted-pcrel-data-reloc-link-smoke
- [ ] 700-thread-local-store-register-pressure-runtime-smoke

### → hosted-compat features PA (14) — home dir TBD (the 248-directed prereq set)

> **Policy (clarified 2026-06-02): pa34 = static_assert ONLY** (L1 front-end conformance). A compat
> builtin that cppgm++ can constexpr-evaluate → `pa34/tests/compile` static_assert. One that can
> only be validated by **linking/running** stays in **`pa35/tests/link`** (the existing host-interop
> link+run style via `run_cpphostinterop_tests_worker.pl`) for the deferred L1/L2 recategorization —
> pa34 eventually splits into an L1 PA and a host-compatibility PA; the link host-compat tests land in
> the latter. `pa34/tests/link` is **not** wired (only orphaned `.my` from an abandoned experiment).

`__builtin_*` ×12 and `gnu-asm` symbol-label ×2; verified end-to-end, no `#include`.

- [-] 600-builtin-memcpy-strlen-link-smoke
- [-] 600-builtin-operator-new-delete-link-smoke
- [-] 600-gnu-asm-label-overload-link-smoke
- [-] 600-gnu-asm-leading-underscore-label-link-smoke
- [-] 700-dependent-alias-pack-invoke-result-link-smoke (uses `__builtin_invoke` + runtime → host-compat link, not template)
- [x] 700-hosted-builtin-bitops-promote-runtime-smoke → `pa34/tests/compile/600-builtin-bitops-promote` (static_assert recast: `__builtin_clz*`/`popcount`; PASS)
- [-] 700-hosted-builtin-flt-rounds-runtime-smoke (runtime FP env — not constexpr)
- [-] 700-hosted-builtin-fp-classification-runtime-smoke
- [-] 700-hosted-builtin-fpclassify-runtime-smoke
- [-] 700-hosted-builtin-huge-val-runtime-smoke
- [-] 700-hosted-builtin-mul-overflow-runtime-smoke (cppgm++ can't constexpr-eval)
- [-] 700-hosted-builtin-nans-runtime-smoke (`__builtin_memcpy` bit-cast — not constexpr)
- [x] 700-hosted-builtin-popcountg-runtime-smoke → `pa34/tests/compile/600-builtin-popcountg` (static_assert recast; constexpr in cppgm++; PASS)
- [-] 700-hosted-builtin-signbit-runtime-smoke (cppgm++ can't constexpr-eval)

### → core PA (specific owner) / DROP (7) — per 13-item review

- [x] 700-initlist-const-char-pointer-runtime-smoke → `pa27/tests/general/200-initializer-list-const-char-pointer-range-for` (initializer_list + range-for; gated to pa27 by `template.initializer_list`; PASS) _(item 3)_
- [x] 600-delete-class-pointer-destroys-object-runtime → `pa16/tests/general/300-delete-class-pointer-lifetime` (new/delete + ctor/dtor; `expr.new_delete` pa16:300; PASS) _(item 4)_
- [x] 600-hosted-enum-class-parameter-link-smoke → `pa30/tests/general/100-scoped-enum-parameter-defined-in-separate-object` (behavior, cross-TU; PASS). Mangling half DROPPED — `classify(Kind)`→`_Z8classify4Kind` is a named-type param, already covered by pa31 named-type/function-param tests. _(item 5)_
- [x] 600-hosted-member-operator-bang-link-smoke → `pa15/tests/general/300-member-operator-bang-out-of-class` (operator! + out-of-class def; `operator.overload` pa15:300; PASS) _(item 6)_
- [x] 700-hosted-alignas-class-layout-link-smoke → `pa19/tests/general/100-alignas-class-layout` (static_assert recast; **placement audit: static_assert gates to pa19** though alignas is pa15:300; PASS) _(item 8)_
- [x] 700-pointer-predecrement-reference-argument-link-smoke → `pa16/tests/general/200-pointer-predecrement-reference-argument` (pre-decrement + ref args; **audit: gated to pa16 by `value.copy_move`** (cell `operator=`), not pa14; PASS) _(item 12)_
- [x] 700-wide-string-literal-runtime-smoke → `pa19/tests/general/100-wide-string-literal-constexpr` (static_assert recast of `L"ab"` subscript; PASS) _(item 13)_

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
