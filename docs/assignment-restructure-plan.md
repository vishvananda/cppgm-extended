# Assignment Restructure Plan (pa18–22 / pa24–25 / pa34–35)

## Purpose

Three structural problems in the post-PA9 arc, raised from teaching experience:

1. **pa24 (cpplink) + pa25 (cppeh) feel redundant** — students build a native
   LowIR linker and native exception handling, then the later compiler links and
   handles exceptions through the **host** toolchain instead.
2. **Templating (pa18/19/21/22) is too hard** — pa22 alone has ~455 tests,
   accumulates everything, and tends to break earlier work.
3. **pa34 is confusing** — it mixes implementing hosted features (builtins,
   parsing, extra macros) with a pile of undirected compile tests that (a) let
   agents cheat (no required output) and (b) expose severe template-parsing
   performance problems.

This plan respects the existing tool-by-tool arc and the feature-ownership rules
in `pa10-34-assignment-cleanup-process.md`, `pa14-pa22-feature-allocation-audit.md`,
and `pa34-pa35-convergence-plan.md`. Renumbering is a known, supported operation
(see the ABI-naming insertion that became pa31).

---

## Issue 1 — pa24 (cpplink) / pa25 (cppeh): validated reuse analysis

**Method.** Compared the per-tool object sets in `dev/frontend_source_sets.mk`
and traced the runtime link/EH paths in `cpp_driver_frontend.cpp`,
`machine_linker.cpp`, and `cpp_toolchain.cpp`.

**Object-level (static):** `cpplink` and `cppeh` each comprise 42 objects;
**39 of 42 are also in `cppgm++`**. The only tool-unique objects are the
standalone CLI driver (`lowir_driver_frontend`, `lowir_tool_cli`, `machine_ir`).
So the linker/EH library code is *shared*, not deleted.

**Runtime (what cppgm++ actually does):** `cpp_driver_frontend.cpp` chooses:

```cpp
if (can_use_host_toolchain_for_output_target(invocation.output_target)) {
    link_host_objects_to_native(...);          // host: g++/clang++
} else {
    link_exception_objects_to_native(...);     // native: machine_linker + EH
}
```

`can_use_host_toolchain_for_output_target` is true when the output target equals
the host platform (`cpp_toolchain.cpp:367`). Every pa30–35 test (and inception)
compiles for the host, so cppgm++ **always takes the host path**. The native
`machine_linker` path is only reachable for non-host (cross) targets, which the
course never exercises through cppgm++.

**Conclusion.**

- **pa24 (cpplink, native linker):** functionally **bypassed** by the end-to-end
  compiler. The student builds `machine_linker`; cppgm++ links host-target output
  with g++/clang++. Strong case for removal/merge. (Caveat: the standalone
  `lowir2native` tool at pa23/pa37 still uses `machine_linker`, and the code is
  shared, so this is "off the critical path," not "dead-deleted.")
- **pa25 (cppeh):** **partially reused.** The EH *metadata* generation
  (`host_eh_object_sections`, the Itanium call-site/LSDA table) **is** used by
  cppgm++'s host path. The native EH *link* is bypassed. So removing pa25 would
  lose a lesson whose core (the EH table) is on the host critical path.

**Recommended change (Issue 1).**

- **pa24 → fold/cut.** Drop the standalone native-linker assignment; keep the
  `lowir2native` native-codegen lesson (pa23) which still legitimately produces
  native objects, and move the small amount of genuinely-reused object/section
  emission into pa23 or the first host-`-c` lesson. Do **not** teach a native
  multi-object linker the compiler never calls for host output.
- **pa25 → refocus, don't delete.** Rebuild it around the part that *is* reused:
  the **host EH call-site/LSDA table** the compiler emits so the host personality
  can unwind (the `host_eh_object_sections` work). Cut the native-EH-link half.
  i.e. pa25 becomes "emit correct host-compatible EH metadata," not "implement a
  native exception linker."
- **Validation gate:** before cutting, confirm no pa30+ test path reaches
  `link_exception_objects_to_native`/`link_machine_objects_to_native` via cppgm++
  (only via the `lowir2native`/`cpplink` standalone tools). Already true by the
  predicate above; re-confirm if the target-selection logic changes.

**Open question for you:** is native (non-host) linking a *learning objective*
you want to keep at all (it's the only thing that makes pure self-hosting
possible without a host linker dependency)? If yes, pa24 stays but should be
explicitly framed as "the self-host linker," not duplicated by the host path. If
no, fold it.

---

## Issue 2 — Templating (pa18/19/21/22): rebalance + add an integration lesson

**Current:** four templating PAs (18, 19, 21, 22). pa22 ≈ **455 tests**, is the
catch-all, and regresses earlier work when implemented.

**Plan.**

- Keep **pa18/19/21** focused on *basic, isolated* template features (one
  constrained surface each), per the existing feature-allocation audit. Audit
  pa22's 455 tests and push any test that exercises a *single* basic feature back
  to its owning basic PA.
- Re-scope **pa22** to a smaller "advanced single-feature" set (the genuinely
  harder individual features that don't belong in 18/19/21).
- **Add a new integration templating lesson** (call it pa22b / renumber to a new
  pa23, cascading later PAs +1) whose tests are the *combination* / "everything
  together" cases — the ones that currently make pa22 huge and brittle. This
  lesson's contract is "make all the template features work together," with no
  new isolated feature ownership.
- Net effect: the difficulty of any single templating PA drops; the brittle
  integration surface is isolated into one clearly-labeled lesson instead of
  hiding inside pa22.

**Needs a data pass:** bucket pa22's tests by (single feature → owning basic PA)
vs (multi-feature integration → new lesson) to size the move. (Test inventory +
the feature-ownership table are the inputs.)

---

## Issue 3 — pa34 (hosted): split out a perf-gated header-compilation lesson

**Current pa34** mixes: implementing hosted features (builtins, extra macros,
parsing extensions) **and** a large set of compile-only tests with little
direction. Two failure modes observed:

1. **Cheating:** compile tests require *no output*, so agents skip actually
   parsing the headers and still "pass."
2. **Performance wall:** by pa34, template parsing is implemented only for simple
   cases; compiling a real hosted header takes **minutes** instead of seconds.

**Plan.**

- **pa34 keeps the hosted *feature* implementation** (builtins, macros, parsing
  extensions) with directed, output-bearing tests — no undirected compile dump.
- **New lesson: hosted-header compilation (perf-gated).** Move the
  header-compilation work here, restructured so it cannot be cheated and forces
  performance:
  - **Per-header**, not a bulk dump: each test names a specific hosted header.
  - **Required, checkable output + execution:** compile the header **and run the
    minimal piece that exercises it**, asserting a concrete result (exit
    status/stdout). "Do something useful with it" — so skipping parse fails.
  - **Hard time budget** (e.g. **≤ 30–60 s** per header) as a first-class pass
    criterion, with the lesson's stated objective being **template-parsing
    performance**: profile and optimize the template-resolution hot path so a
    header compiles in seconds.
- This directly attacks both failure modes: required execution kills the
  no-output cheat; the time budget makes the slow template implementation a
  *failing* condition rather than a tolerated one.

**The pa34/pa35 linking tension (flagged):** you can't *execute* the "minimal
piece" without **linking**, and linking is exactly where the no-output cheating
lived. So the new header-compilation lesson almost certainly needs the host link
step available (it depends on the host toolchain, established by the pa30 `-c`
lesson). Options to resolve:
- **A — header lesson links via host toolchain** (depends on pa30 host `-c`):
  cleanest for "execute and check," keeps pure-link concerns in pa35.
- **B — keep pa35 as the link/runtime gate** and make the header lesson
  compile-+-verify-symbols only (no execution): preserves separation but
  re-opens the "no execution → easier to fake" risk.
- Recommendation: **A** — require execution (host-linked) in the header lesson;
  let pa35 remain the broader hosted link/symbol/ABI/runtime discovery gate. This
  accepts that "compile a header and prove it works" inherently needs a linker,
  rather than pretending header-compile can be validated without running.

> **Superseded (2026-05-30).** The compile-vs-link framing above is retained as
> rationale only. Deeper analysis showed compile-only is ~95% cheatable (only
> `static_assert` is checkable without running) and the Itanium lessons made
> linking cheap, so the axis is now **compile-time-verifiable (`static_assert`,
> L1) vs runtime-verifiable (run+assert, L2)** — see Decision 6 and "Hosted area
> — verification-axis analysis" in the locked plan.

---

## Sequencing & ripple effects

1. **Data passes first (no moves yet):**
   - Issue 1: re-confirm the host-vs-native link predicate; inventory which
     `host_eh_object_sections` behavior pa25 uniquely teaches.
   - Issue 2: bucket pa22's ~455 tests by feature vs integration.
   - Issue 3: inventory pa34's tests into (feature-impl, output-bearing) vs
     (header-compile, undirected).
2. **Decide the new numbering.** Likely +1 insertion for the templating
   integration lesson and +1 for the header-compilation lesson, cascading later
   PAs. Use the same renumber tooling as the pa31 ABI insertion. Update
   `ROADMAP.md`, the feature-ownership table, and per-PA READMEs.
3. **Relocate tests** to their new owners; keep witness refs intact (golden).
4. **Rewrite the affected READMEs** to the standard handout shape, with the new
   lessons' contracts (esp. the header lesson's required-execution + time-budget
   pass criteria).

## Open decisions (need your call before buildout)

- pa24: fold entirely, or keep as an explicit "self-host native linker" lesson?
- pa25: refocus to host-EH-table emission (recommended) or cut?
- Templating: one new integration lesson, or also split a basic feature out of
  pa22 into a fifth templating PA?
- Header lesson: require execution via host link (A, recommended) or
  compile-+-symbol-check only (B)?
---

# LOCKED PLAN (decision-of-record, 2026-05-30)

The sections above are the analysis/rationale. This section is the decided plan.
Net count: **38 → 39 lessons** (decided: one templating integration lesson, no
extra basics split).

## Decisions

1. **Fold pa24 (cpplink).** Native `machine_linker` is bypassed by cppgm++'s
   host-target link path; not a lesson. Salvage two tests into pa30 (below).
2. **Refocus pa25 → host `.gcc_except_table` emission; move it next to the
   host-EH core (before pa33).** Keep the reused EH-table half; drop the native
   EH-link half.
3. **Keep pa9 (stepping stone for pa13) and pa30 (first-executable /
   separate-compilation / driver).**
4. **Consolidate the LowIR-generation band.** pa26–29 are LowIR-text tests with
   **no dependency** on pa23–25 (confirmed) — their separation was a style
   choice. Make pa14–22 + pa26–29 contiguous, then place `lowir2native` (and the
   refocused EH lesson, then the host backend) **after** the full LowIR band.
5. **Templating → 5 lessons.** Keep pa18/19/21 as isolated basics; re-scope pa22
   to advanced **single-feature**; add one **integration** lesson (multi-feature
   interaction + end-to-end). Decided: just the integration lesson, no extra
   basics split.
6. **Re-axis the hosted area: compile-time-verifiable vs runtime-verifiable**
   (supersedes the earlier "COMPILE (34) vs LINK+RUNTIME (35)" split — see
   "Hosted area — verification-axis analysis" below for the data). The pa34/35
   tests were all added ad-hoc while self-compiling and hitting bugs; they must be
   recast into a coherent, cheat-proof curriculum. The compile-vs-link distinction
   no longer holds: the Itanium ABI lessons made linking cheap/host-based, and
   compile-only is ~95% cheatable (only `static_assert` is checkable without
   running). The principled axis is **what can be verified, and how**:
   **Scope first.** Of the 185 ad-hoc pa34/35 tests, **62 (all pa35) pull no
   library header** — core/compat reproducers dumped into pa35 during self-host
   debugging. Three destinations:
   - **14 → hosted-compat features PA** — genuine compat-layer features
     (`__builtin_*` ×12, `gnu-asm` symbol labels ×2), verified end-to-end without
     an `#include`.
   - **17 → pa31 abimangle** — their **only purpose is to assert symbol names**:
     they hand-spell `std` templates to stress Itanium substitution/compression,
     or use `abi_tag` (a mangling annotation); the cross-TU link is just the
     mangling-consistency oracle. These are mangling tests, not hosted-library
     tests. (3 reference `std` but read like templating/linkage/runtime —
     `dependent-alias-builtin-transform`, `inline-header-odr`,
     `initlist-const-char-pointer` — flagged for review before moving.)
   - **31 → owning core PA / DROP** — generic core C++ (templates ×11, vtables/MI
     ×4, separate-compilation ×3, backend reloc/TLS ×3, ~10 review).
   The remaining **123 header-bearing** tests are the true hosted area:
   - **hosted-compat features PA** (the FIRST hosted lesson, formerly "builtins
     prereq") ← the **248** no-include directed tests **+ the preproc tests + the
     14 no-header compat reproducers** above. Implements the host-compatibility
     layer (builtins, macros, parse extensions, asm-labels), verified directly
     (directed output for the 248; run+assert for the reproducers). Kept
     cheat-proof; required before any header compiles. (Itanium mangling — incl.
     `abi_tag` and std-template substitution — belongs to pa31 abimangle, which
     this lesson then relies on.)
   - **Lesson 1 — hosted front-end conformance (`static_assert`, perf-gated):**
     does cppgm++ *parse / template-resolve / type-check* the header's
     **compile-time** semantics correctly and fast? Cheat-proofed by
     `static_assert`. The **perf budget lives here**. **42** tests (3 already
     `static_assert` + 39 constexpr-pure computations recast as `static_assert`).
   - **Lesson 2 — hosted back-end / ABI interop (link + run + assert):** does the
     **emitted code** interoperate with prebuilt libstdc++ at **runtime**?
     Cheat-proofed by running and asserting a runtime value. **50** tests
     (runtime-state + I/O + header-bearing cross-TU link).
   - **31 weak/cheatable** tests (calls-no-result, declare-only) are triaged
     per-test: upgraded into L1/L2 or dropped as near-dupes (the
     `<string>`/`<memory>`/`<functional>`/`<vector>` clusters, ~5 each).
   This keeps the hosted **×3** shape (compat-features → L1 → L2) and the 39-lesson
   count; it replaces the broken compile-vs-link axis with a cheat-proof one and
   evicts the 62 misfiled reproducers. Per-test disposition (all 185) is in
   `docs/pa34-35-test-disposition.md`.

## Resulting arc (39 lessons, by region)

- **1–9 front end** — unchanged (pa9 kept).
- **10–13 semantics + LowIR-bridge** — unchanged (pa9→pa13 intact).
- **LowIR generation (consolidated, contiguous)** — old pa14–22 **+** old
  pa26–29, basics → advanced, **with 5 templating lessons** in-band: 3 basics
  (18/19/21-style), 1 advanced-single (re-scoped 22), 1 **integration (new)**.
- **`lowir2native`** — moved to **after** the LowIR band (no longer mid-stream).
- *(cpplink folded out.)*
- **`-c`/link driver** (old pa30, kept) → **abimangle** (old pa31).
- **host EH ×3** — **EH-metadata (refocused pa25, new home)** → EH-interactions
  (old pa32) → EH-core (old pa33).
- **hosted ×3** — **hosted-compat features** (248 directed + preproc + 30
  no-header compat reproducers) → **front-end conformance (`static_assert`,
  perf-gated)** → **back-end ABI interop (link+run+assert)** — re-axised from the
  ad-hoc pa34/35 tests by scope then verification kind, not compile-vs-link.
- **lowiropt → lowir2native -O → inception** — unchanged.

(Exact renumbering done by the same tooling as the pa31 ABI insertion; update
`ROADMAP.md`, the feature-ownership table, and per-PA READMEs. Witness refs are
golden — do not regenerate.)

Implementation note: before final renumbering, the freed PA24 slot is reused in
place as the template integration lesson. The old `cpplink` contract remains
folded out; its salvaged tests stay in PA30.

## Salvage from pa24 → pa30

7 of cpplink's 9 tests are already covered by pa30 (`duplicate-global-bad`,
`missing-main-bad`, `unresolved-symbol-bad`, `two-object-call`≈`two-source-call`,
`single-/cross-object-global`≈`extern-global`, `single-object`) → drop. Salvage
the two pa30 lacks, **recast as cppgm++ `-c` → host-link** tests (they now verify
cppgm++'s *object emission*, not a native linker):

- **`startup-hooks-across-objects`** — static-init / startup hooks across TUs.
- **`data-reloc-indirect-call`** — cross-object data relocation feeding an
  indirect call.

## Test strategies

- **Refocused pa25 (`.gcc_except_table`):** emit the Itanium LSDA (call-site
  table covering the **whole** function range incl. the head, action table,
  type-info table) from LowIR EH regions. Oracle = **host-linked execution**:
  throw→catch/cleanup, uncaught→`terminate`, nested-frame unwind, no spurious
  terminate; plus a structural check the table is emitted + call-site-complete.
  Boundary: pa25 = correct metadata (basic throw/catch/cleanup + structural);
  pa33 = full semantics (cross-TU, nested, cleanup chains, RTTI matching).
- **hosted-compat features PA (the 248 + preproc + 30 no-header reproducers):**
  directed, output-bearing per-builtin / macro / parse-extension tests — one each,
  assert a value vs g++/clang (the 248, already in this form). The 30 no-header
  compat reproducers (builtins / abi-tags / libstdc++ ABI) join here as
  link+run+assert end-to-end checks of the compat layer. Implements the
  host-compatibility layer required before any header compiles.
- **Lesson 1 — front-end conformance (`static_assert`, perf-gated):** per-header,
  one compile-time property each — a type-trait result, SFINAE outcome,
  `integral_constant`/ratio value, or alias/`decltype` identity — asserted with
  `static_assert`. Compile-only, but cheat-proof: a passing `static_assert`
  *requires* having resolved the template. **Hard wall-clock budget
  (≤30–60 s) that is itself a pass criterion** — slow template resolution is a
  *failing* condition. Headers fixed (not student-chosen) for a comparable bar.
- **Lesson 2 — back-end / ABI interop (link + run + assert):** per-capability:
  build a minimal program that exercises the runtime/ABI behavior (container
  storage, `iostream` I/O, exception propagation through the host personality,
  allocator behavior, vtable dispatch, cross-TU symbol resolution), **run it, and
  assert the runtime result** (exit status / stdout) vs g++/clang. Required
  execution defeats the no-output cheat; cross-TU tests genuinely need the host
  link. Perf still first-class.

## Hosted area — verification-axis analysis (data, 2026-05-30)

Categorized all **185** ad-hoc tests that need re-sorting (pa34's 57
header-including tests + pa35's 128 `tests/link`). The 248 no-include directed
tests are excluded — they are already output-bearing (they form the compat-features
PA's core).

**Step 1 — scope.** Split by whether the test pulls a hosted library header:

| Scope | count | disposition |
|---|---|---|
| no header — compat-layer feature (`__builtin_*` 12, `gnu-asm` label 2) | **14** | → **hosted-compat features PA** (with preproc) |
| no header — symbol-name/mangling (hand-declared `std` substitution, `abi_tag`) | **17** | → **pa31 abimangle** |
| no header — generic core C++ (templates 11, vtable/MI 4, separate-comp 3, backend 3, ~10 review) | **31** | → **owning core PA / DROP** |
| pulls a hosted library header | **123** | → split L1/L2/triage below |

**Step 2 — verification kind** for the **123 header-bearing** tests.
Compile-time-verifiable = property is a constant expression (trait/SFINAE/
identity/ratio), assertable with `static_assert` without running; runtime-
verifiable = depends on mutable state, heap, I/O, side-effects, or cross-TU
symbols. "constexpr-pure" detected by absence of runtime-state markers
(`new`/`delete`, buffer writes, container/string mutation, in-place
`__builtin_mem*`, `std::move`, `throw`, I/O).

| Bucket | pa34 | pa35 | total | target |
|---|---|---|---|---|
| `static_assert` (already) | 3 | 0 | 3 | **L1** front-end |
| computes, constexpr-pure | 22 | 17 | 39 | **L1** (recast `return`→`static_assert`) |
| **Lesson 1 subtotal** | **25** | **17** | **42** | |
| computes, runtime-state | 3 | 10 | 13 | **L2** (run+assert) |
| I/O / streams | 4 | 28 | 32 | **L2** (run+assert) |
| cross-TU link | 0 | 5 | 5 | **L2** (link+run) |
| **Lesson 2 subtotal** | **7** | **43** | **50** | |
| calls-no-result | 22 | 6 | 28 | triage |
| declare-only | 3 | 0 | 3 | triage |
| **weak/cheatable subtotal** | **25** | **6** | **31** | |

Key findings: (1) **62 of pa35's 128 "link" tests pull no STL header** — half the
"link" area is misfiled core/compat reproducers, not hosted-library tests: 14 are
compat-layer features, **17 are symbol-name/mangling tests → pa31 abimangle**, and
31 belong back in earlier core PAs (or drop). (2) Among the 123 real header-bearing tests, both hosted lessons are
well-fed (L1 42, L2 50) before reclaiming any of the 31 weak ones. (3)
"constexpr-pure" is an **upper bound** — some won't fold to a constant expression
(value may route through a non-`constexpr` library call); the per-test pass
confirms which fold (else → L2). (4) The 31 weak/cheatable header tests carry the
remaining redundancy (`<string>`/`<memory>`/`<functional>`/`<vector>` clusters,
~5 each).

Per-test disposition (all 185): `docs/pa34-35-test-disposition.md`.

### 13-item manual review (decisions of record, 2026-05-31)

The heuristic left 13 no-header tests needing a human call — the 3 "abimangle
review" flags + the 10 "core (review)". Read each; calls below are final.

| # | test | call | rationale |
|---|---|---|---|
| 1 | `dependent-alias-builtin-transform` | → **templating** | dependent alias `__remove_cvref_t<_Iter>` drives member-fn overload resolution; the `__remove_cvref` builtin is incidental (needs the compat builtin available — note ordering). Runtime. Not mangling. |
| 2 | `inline-header-odr` | → **hosted L2** (header-bearing) | `.shared.h` pulls `<deque>/<map>/<memory>/<string>`; an `inline` fn in the header is emitted in both TUs → COMDAT/weak ODR folding + cross-TU link. Mis-detected as no-header. Not mangling. |
| 3 | `initlist-const-char-pointer` | → **core: initializer_list codegen** | braced `{"__a",…}` → backing-array synthesis + ranged-for. Runtime. Not mangling/hosted. |
| 4 | `delete-class-pointer-destroys-object` | → **core: new/delete + ctor/dtor lifetime** (DROP if covered) | heap `new`/`delete` invoking ctor/dtor; runtime counter==0. |
| 5 | `enum-class-parameter` | → **core: scoped enum** + cross-TU link (DROP if covered) | scoped `enum class` as a cross-TU fn parameter. |
| 6 | `member-operator-bang` | → **core: operator overload + out-of-class member def** (pa30 link; DROP if covered) | out-of-line `Box::operator!` across TUs. |
| 7 | `pair-vector-arg-ranges` | → **pa31 abimangle** (reclassified from "core review") | inline-namespace `helper_inline_ns::v1` + nested class-template arg type must mangle identically across TUs — same symbol-name purpose as the hand-declared-`std` set. |
| 8 | `alignas-class-layout` | → **core: alignas / class layout** (recast `static_assert`; DROP if covered) | `alignof`/`sizeof` are constant exprs; core layout feature, not hosted. |
| 9 | `function-reference-parameter` | → **templating** | template deduction over a function-reference param + call. Runtime. |
| 10 | `header-inline-unemitted-callee-signature` | → **hosted L2** (header-bearing) | `.h` pulls `<streambuf>`; inline virtual override references the unemitted `streambuf::sbumpc` — vtable + inline emission. Mis-detected as no-header. |
| 11 | `user-declared-trivial-dtor-return` | → **hosted L2: back-end ABI / calling convention** | return-value ABI for a struct with user-declared `~Result()=default` (trivial → returned in registers); `host_choose` is undefined, so it is really an object-emission/ABI-signature check. |
| 12 | `pointer-predecrement-reference-argument` | → **core: expression codegen** (pre-decrement, references, `operator=`; DROP if covered) | `*--result = current(--last_iter)` over a local array. |
| 13 | `wide-string-literal` | → **core: wide string-literal encoding** (recast `static_assert`; DROP if covered) | `L"ab"` wchar_t values; constant expr; core lexer/literal. |

Net effect on the no-header tallies:
- **pa31 abimangle:** −3 (items 1–3 leave) **+1** (item 7 joins) → **15**.
- **hosted L2 (header-bearing):** **+2** — items 2 & 10 pull real STL via
  `.shared.h`/`.h`, so they join the 123 header-bearing set (→ **125**; no-header → **60**).
- **templating:** + items 1, 9. **hosted L2 (ABI):** + item 11.
- **core (specific owners, mostly DROP candidates):** items 3, 4, 5, 6, 8, 12, 13.

> **Re-audit note.** The no-header detector scanned only `.t`/`.t.N` and missed
> headers pulled via `.shared.h`/`.h` (items 2 & 10). Re-run the scope pass
> including `.shared.h`/`.h` includes before finalizing the 123/62 split — a few
> more "no-header" tests may actually be header-bearing.

## Templating split — classification via `scripts/audit_pa_feature_placement.py`

That script already detects per-test feature usage (regex `FeatureRule`s over
source/ref/path) and maps to owning PA clusters; it already covers pa14–22 +
pa26–29. Extend it to size the integration lesson:

1. Ensure `FeatureRule`s exist for the template sub-features
   (function-template, member-template, alias-template, partial-spec,
   variable-template, dependent-name, explicit-spec, non-type-param,
   pack-expansion, SFINAE/void_t, forwarding-ref, nondeduced-context, …).
2. Run it over **pa22's 455 tests** and record, per test, the **set** of template
   features detected.
3. **Arity classify:**
   - **single-concept** (one template feature) → re-scoped advanced-single pa22,
     or push to the owning basics PA;
   - **multi-concept** (≥2 interacting features) → **integration lesson**,
     **clustered by the feature combination** (e.g. {member-template,
     pack-expansion}, {dependent-alias, partial-order}, {forwarding-ref,
     nondeduced-context}).
4. The `dependent` bucket (63 tests) is the swing factor — dependent-name
   resolution interacts with everything, so many "dependent-*" tests are really
   integration tests; weight them toward the integration lesson when they combine
   with another feature.

Output of that pass = the concrete move list (which pa22 tests → integration, in
which cluster) and the integration lesson's size. **This is the next data step.**

Tracker seeds:

- `docs/template-strict-placement-tracker.md` is the all-strict context scan over
  `pa18 pa19 pa21 pa22 pa24`, used to see whether earlier strict PAs already
  contain PA22/PA24-shaped tests before moving PA22 coverage backward or
  forward.
- `docs/pa22-template-placement-tracker.md` is the concrete PA22 move/review
  queue for the new PA24 integration split.

Both are generated from the audit script's `--template-placement` mode and
should be curated in place as each test is reviewed.

## Open decisions (remaining)

- Lesson 1 (front-end conformance): which headers, and how many? The constexpr
  pool clusters on `<type_traits>`, `<chrono>`, `<functional>`, `<ratio>`,
  `<utility>` — pick the fixed set once the per-test pass confirms which
  computations actually fold to a constant expression.
- Confirm the L1/L2 boundary against the per-test disposition once the 64
  "constexpr-pure" candidates are checked for genuine constant-foldability (some
  may fall back to L2 run+assert).
