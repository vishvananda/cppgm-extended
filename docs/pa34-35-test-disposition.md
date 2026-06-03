# pa34 / pa35 hosted test disposition tracker

_Generated 2026-05-30 from the verification-axis analysis in
`assignment-restructure-plan.md`._

> Renumbering note: this tracker was written before the PA34 split inserted the
> new header-compile PA35. References to the old `pa35/tests/link` hosted-link
> surface now correspond to `pa36/tests/link`; the new PA35 owns heavy
> hosted-header compile-only tests.

Covers the **185** ad-hoc tests added during self-hosting: pa34's **57**
header-including tests + pa35's **128** `tests/link`. The **248** no-`#include`
prerequisite tests are **not** listed (already directed; stay as the
hosted-builtins prerequisite lesson).

## L1/L2 sort — status (2026-06-02)

The 123 header-bearing tests are sorted under the **compile-anchor L1 strategy** (L1 = compile the
hard header + a cheat-proof trait/`decltype`/`sizeof` anchor; L2 = keep the runtime test, which is
cheat-proof by running). Outcome:

- **~70 L1 compile-anchors** in `pa34/tests/compile`: the 42-candidate table (≈35 converted, plus
  already-`static_assert`/`#error`-guarded ones), **15 distinct-API anchors** for the hard I/O/STL
  headers (ostream/sstream/istream/fstream/locale/iterator/stdexcept/set/cstdio/unordered_set/new/
  iomanip/cstring), and **18 triage upgrades**. Each verified clean in cppgm++ **and** clang.
- **L2 runtime tests kept** in `pa35/tests/link` (the pathological-codegen safety net); I/O tests are
  **split** (L1 anchor added, L2 run kept); cross-TU stays L2.
- **8 triage near-dups dropped** (each covered by a kept cluster exemplar).
- **Resolved (2026-06-02) — L1/L2 sort complete:** `standard-exception-ctor-mangling` → recast to
  pa31 (`300-std-exception-string-constructors`: logic_error/runtime_error `(const std::string&)`
  ctors, clang-verified). The object-file **elision/symbol-surface** tests were **dropped**
  (functional-include-symbol-surface, forward-helper-symbol-surface, placement-new-symbol-surface,
  abi-tag-class-template-member ×2 + nested-static-abi-tag) — asserting which internal symbols an
  object omits is an implementation detail, not conformance. Absence assertions were stripped from
  `unordered-set-pointer` + `local-lambda-std-function` (positive-ownership checks kept), and the
  pa35 README's omission/negative-ownership note was removed. `300-include-next` stays a preproc
  test; `vector-char-assign-initlist` stays L2 (`<vector>` already L1-covered).
- Surfaced cppgm++ gaps (anchored around): `is_copy_constructible<tuple<string&&>>`, constexpr
  null-pointer equality, 3-arg `decltype(strtoull(...))` template-arg resolution.

Commits `43357c176`…`e4d024094`; see git log. Per-row statuses below are the original first cut.

> **Note.** The homes below are the automated first cut. For the 13 tests sent to
> manual review (the 3 "abimangle review" flags + the 10 "core (review)"), the
> **decisions of record** are in `assignment-restructure-plan.md` →
> "13-item manual review" and supersede the heuristic homes here. Two of them
> (`inline-header-odr`, `header-inline-unemitted-callee-signature`) are in fact
> header-bearing (STL pulled via `.shared.h`/`.h`) and move to hosted L2.

## Two-level disposition

**Scope first.** A test either exercises a **hosted library header** (true
hosted-area material) or it is a **core-feature reproducer with no library
header** (template / builtin / mangling / vtable / codegen) that was dumped into
pa35 during self-host debugging.

- **123 tests have a library header** → split into the hosted lessons:
  - **L1 — front-end conformance** (`static_assert`, perf-gated): compile-time
    property (trait / SFINAE / identity / ratio). Cheat-proof via `static_assert`.
  - **L2 — back-end / ABI interop** (link + run + assert): needs the program to
    run (mutable state, heap, I/O, side-effects, cross-TU symbols).
  - **TRIAGE** — weak/cheatable: upgrade into L1/L2 or **drop** as a near-dup.
- **62 tests have NO library header** → **RELOCATE** to the core PA that already
  owns the feature (or **DROP** if covered there). These are not hosted-area
  material.

> **Revised L1 strategy (2026-06-02):** L1's purpose is **compile-time conformance for
> hard headers** — does cppgm++ parse/instantiate the header correctly? The `static_assert`
> is only an **anti-cheat anchor**: it must require the header's types to be present and
> correct so the test can't pass on an empty TU. The anchor need **not** assert a runtime
> value — a **type-trait / `decltype` / `sizeof` / `is_constructible` / type-identity** check
> is compile-time-evaluable even when the container's *operations* are not `constexpr`
> (libc++ at gnu++11). So **most** header tests are L1, anchored on the API surface they
> exercised; no `main` needed, empty `.ref`, exit SUCCESS. L2 is reserved for tests whose
> conformance target is a **runtime observable**: stdout/IO, cross-TU linkage, or ABI/symbol
> surface.

| Bucket | Target | Modification |
|---|---|---|
| `static_assert` | L1 | Keep — already compile-time-checked. |
| `constexpr-pure` / `runtime-state` / `calls-no-result` / `declare-only` | L1 | Recast the exercised surface as a cheat-proof **compile-time anchor** (`static_assert` over traits / `decltype` / `sizeof` / `is_constructible` / type identity). No `main`. → `pa34/tests/compile`. |
| `I/O` | L1 + L2 | **Split:** add an L1 compile-anchor for the hard header (→ `pa34/tests/compile`) AND keep an L2 run+verify-stdout (→ `pa35/tests/link`) — the runtime half catches pathological codegen (e.g. a chained `operator<<` that goes wrong). |
| `cross-TU` | L2 | Keep; cross-TU link + run + assert. → `pa35/tests/link`. |
| ABI / symbol-surface | L2 / pa31 | Mangling → `pa31` abimangle DSL; link symbol-inspect stays `pa35/tests/link`. |
| _no header_ | RELOCATE | Move to owning core PA; DROP if already covered. |

---

## Hosted L1 — front-end conformance (`static_assert`, perf-gated) — 42 tests

| PA | Test | Header(s) | Bucket | Disposition / how to modify |
|---|---|---|---|---|
| pa34 | `600-hosted-max-mixed-arithmetic-deduction` | <algorithm> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-local-class-distinct-member-symbols-compile` | <algorithm> <string> <vector> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-enum-operator-using-namespace-link-smoke` | <algorithm> <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-map-iterator-operator-lookup-link-smoke` | <algorithm> <map> <memory> <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `600-hosted-constructor-assert-preserves-this` | <cassert> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-system-header-nan-redefinition` | <cfloat> <cmath> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-host-climits-llong-min` | <climits> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `600-hosted-cmath-ceilf-link-smoke` | <cmath> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-hosted-csignal-raise-call` | <csignal> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-host-null-macro-pointer-compat` | <cstddef> <cstdlib> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-hosted-deque-member-template-include` | <deque> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-std-exception-construction-compile` | <exception> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `600-hosted-basic-ios-disable-eh-fallback-link-smoke` | <fstream> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-ofstream-file-runtime-smoke` | <fstream> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-hosted-recursive-std-function-string-substr` | <functional> <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-function-typeid-compare-compile` | <functional> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-map-find-iterator-link-smoke` | <map> <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-map-subscript-piecewise-construct-link-smoke` | <map> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-range-for-member-map-link-smoke` | <map> <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-allocator-deallocate-included-class-layout` | <memory> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-shared-ptr-rvalue-assignment-link-smoke` | <memory> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-random-to-address-qualified-call` | <random> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-const-unordered-map-find` | <string> <unordered_map> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-forward-array-string-pair` | <string> <utility> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-static-cast-base-ref-to-derived-ref` | <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-string-compare-prefix-substr` | <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-piecewise-pair-index-sequence-alias` | <string> <tuple> <utility> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-std-invocable-r-variable-template-base-compile` | <string> <type_traits> | static_assert | Keep — already `static_assert`. |
| pa34 | `700-hosted-tuple-rref-disabled-copy-ctor` | <string> <tuple> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `600-hosted-string-rvalue-plus-char-runtime` | <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-local-wide-string-return-link-smoke` | <string> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `600-hosted-forward-as-tuple-rvalue-ref-runtime` | <tuple> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-adl-std-get-hidden-friend-link-smoke` | <tuple> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-member-pointer-traits` | <type_traits> | static_assert | Keep — already `static_assert`. |
| pa34 | `700-hosted-std-invocable-r-function-reference-compile` | <type_traits> | static_assert | Keep — already `static_assert`. |
| pa34 | `700-hosted-template-lambda-rtti-options-lifetime` | <typeinfo> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-typeinfo-hash-runtime-smoke` | <typeinfo> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-forward-helper-symbol-surface-link-smoke` | <utility> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-hosted-vector-bool-storage-allocator-static-cast` | <vector> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `600-nested-class-constructor-reentry` | <vector> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa34 | `700-hosted-vector-size-constructor-compile` | <vector> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |
| pa35 | `700-hosted-namespace-alias-pointer-template-arg` | <vector> | constexpr-pure | Recast `return E?0:1;` -> `static_assert(E)`; VERIFY E folds to a constant expression (else -> L2 run+assert). |

---

## Hosted L2 — back-end / ABI interop (link + run + assert) — 50 tests

| PA | Test | Header(s) | Bucket | Disposition / how to modify |
|---|---|---|---|---|
| pa34 | `600-hosted-algorithm-copy-n-using-directive` | <algorithm> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `600-hosted-dynamic-exception-spec-runtime` | <cstdio> <cstdlib> <exception> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-local-lambda-std-function-link-smoke` | <cstdio> <functional> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-local-vtable-symbol-link-smoke` | <cstdio> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-std-function-call-link-smoke` | <cstdio> <functional> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-string-assign-scope-guard-link-smoke` | <cstdio> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-anon-namespace-vtable-link-smoke` | <cstdio> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-extern-const-cross-tu-link-smoke` | <cstdio> | cross-TU | Keep — cross-TU link + run + assert exit/stdout. |
| pa35 | `700-hosted-indirect-logic-error-catch-runtime-smoke` | <cstdio> <stdexcept> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-member-vs-free-shift-overload-link-smoke` | <cstdio> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-nested-member-lambda-std-function-link-smoke` | <cstdio> <functional> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-std-function-recursive-lambda-abi-link-smoke` | <cstdio> <functional> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-vector-class-brace-init-ref-capture-runtime-smoke` | <cstdio> <vector> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-inline-thread-local-deque-destructor-once` | <cstdlib> | cross-TU | Keep — cross-TU link + run + assert exit/stdout. |
| pa35 | `700-hosted-internal-string-literal-link-smoke` | <cstring> | cross-TU | Keep — cross-TU link + run + assert exit/stdout. |
| pa35 | `700-hosted-deque-move-assign-link-smoke` | <deque> <utility> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-istream-ref-getline-eof-runtime-smoke` | <fstream> <iostream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-ostream-ref-member-char-sequence-runtime-smoke` | <fstream> <ostream> | I/O | Keep — link + run + assert stdout. |
| pa34 | `700-hosted-function-nullary-base-reentry-compile` | <functional> <string> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-ostream-reference-getloc-vbase-link-smoke` | <iomanip> <ostream> <sstream> <string> | cross-TU | Keep — cross-TU link + run + assert exit/stdout. |
| pa35 | `600-hosted-basic-iostream-move-assign-link-smoke` | <iostream> <utility> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-iostream-nounitbuf-string-runtime-smoke` | <iostream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-iostream-runtime-symbol-link-smoke` | <iostream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-num-put-ostreambuf-iterator-runtime-smoke` | <iostream> <iterator> <locale> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-ostream-char-sequence-runtime-smoke` | <iostream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-ostream-integer-chain-runtime-smoke` | <iostream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-shared-ptr-inline-odr-link-smoke` | <iostream> | cross-TU | Keep — cross-TU link + run + assert exit/stdout. |
| pa35 | `700-hosted-use-facet-locale-copy-link-smoke` | <locale> <sstream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-constructor-unwind-member-dtor-link-smoke` | <map> <string> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-map-copy-assign-empty-comparator-link-smoke` | <map> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-shared-ptr-member-template-mangle-link-smoke` | <memory> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-placement-new-symbol-surface-link-smoke` | <new> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-ostream-char-sequence-parameter-runtime-smoke` | <ostream> <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa34 | `700-hosted-function-template-default-allocator-local-lambda-compile` | <set> <vector> | runtime-state | Keep computation; link + run + assert exit status. |
| pa34 | `700-hosted-unreachable-inline-callee-export-closure` | <set> <sstream> <string> <vector> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-set-insert-count-link-smoke` | <set> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-unreachable-inline-callee-link-smoke` | <set> <sstream> <string> <vector> | I/O | Keep — link + run + assert stdout. |
| pa34 | `600-getline-friend-lambda-access` | <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa34 | `600-hosted-ostringstream-unsigned-int` | <sstream> | I/O | Keep — link + run + assert stdout. |
| pa34 | `600-istream-static-member-mask-access` | <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-ostringstream-vtable-link-smoke` | <sstream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-stringbuf-disable-eh-fallback-link-smoke` | <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-getline-indirect-result-vbptr-link-smoke` | <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-global-istringstream-init-link-smoke` | <sstream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-ostringstream-tellp-runtime-smoke` | <sstream> | I/O | Keep — link + run + assert stdout. |
| pa35 | `700-hosted-stringstream-insertion-runtime-smoke` | <sstream> <string> | I/O | Keep — link + run + assert stdout. |
| pa35 | `600-hosted-unordered-map-string-int-link-smoke` | <string> <unordered_map> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-vector-string-pushback-link-smoke` | <string> <vector> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `600-hosted-unordered-set-pointer-link-smoke` | <unordered_set> | runtime-state | Keep computation; link + run + assert exit status. |
| pa35 | `700-hosted-vector-bool-move-runtime-smoke` | <utility> <vector> | runtime-state | Keep computation; link + run + assert exit status. |

---

## Hosted TRIAGE — weak / cheatable (upgrade or drop) — 31 tests

Sorted by header cluster so near-duplicates sit together — keep the strongest
exemplar per cluster and **drop** the rest, or upgrade a survivor.

| PA | Test | Header(s) | Bucket | Disposition / how to modify |
|---|---|---|---|---|
| pa34 | `500-compressed-pair-padding-instantiation` | <__memory/compressed_pair.h> <bits/shared_ptr_base.h> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-chrono-duration-convert-owner` | <chrono> <string_view> | declare-only | Cheatable (declare-only): exercise the feature + assert, or DROP if redundant in cluster. |
| pa34 | `700-hosted-codecvt-wstring-convert-char16-compile` | <codecvt> <locale> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-split-buffer-move-ctor-compile` | <deque> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `700-hosted-deque-assign-param-alias-runtime-smoke` | <deque> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `600-hosted-ofstream-default-constructor-link-smoke` | <fstream> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `500-local-functor-std-function-assignment` | <functional> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `500-reference-wrapper-smoke` | <functional> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-hosted-std-function-lambda-init` | <functional> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-function-string-nullary-compile` | <functional> <string> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `700-hosted-functional-include-symbol-surface-link-smoke` | <functional> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `300-include-next` | <header.h> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-anonymous-allocator-traits-pointer` | <memory> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-hosted-pointer-traits-pair-pointer-to` | <memory> <string> <utility> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-shared-ptr-allocator-shadowing` | <memory> <mutex> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-unnamed-nested-enum-allocator-pointer` | <memory> <string> <unordered_map> | declare-only | Cheatable (declare-only): exercise the feature + assert, or DROP if redundant in cluster. |
| pa34 | `700-hosted-allocator-char-deallocate-compile` | <memory> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-regex-iterator-difference-alias` | <regex> | declare-only | Cheatable (declare-only): exercise the feature + assert, or DROP if redundant in cluster. |
| pa34 | `600-catch-type-id-identifier` | <stdexcept> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `600-hosted-standard-exception-ctor-mangling-link-smoke` | <stdexcept> <string> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-hosted-forward-as-tuple-rvalue-ref` | <string> <tuple> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-anonymous-namespace-const-iterator-compile` | <string> <vector> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-shared-call-unwind-cleanup-compile` | <string> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-unordered-set-const-range-insert-compile` | <string> <unordered_set> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `600-hosted-string-by-value-parameter-lifecycle-runtime` | <string> <utility> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `500-host-apple-target-conditionals-macro` | <TargetConditionals.h> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-imported-base-of-constructor-sfinae` | <type_traits> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-incomplete-vector-reference-parameter` | <vector> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `600-incomplete-vector-signature` | <vector> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa34 | `700-hosted-vector-range-insert-compile` | <vector> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |
| pa35 | `700-hosted-vector-char-assign-initlist-runtime-smoke` | <vector> | calls-no-result | Cheatable: add a checkable result — `static_assert` if constexpr else run+assert; or DROP if dup of a kept test in cluster. |

---

## RE-HOME — no-header tests (pull no STL header) — 62 tests

These pull no hosted header — they are core/compat reproducers dumped into pa35
during self-host debugging. Three destinations:

- **14 → hosted-compat features PA** (with the preproc tests): genuine
  compat-layer features — `__builtin_*` and `gnu-asm` symbol labels — verified
  end-to-end without an `#include`.
- **17 → pa31 abimangle**: their purpose is to **assert symbol names** —
  they hand-spell `std` templates to stress Itanium substitution/compression, or
  use `abi_tag`, and the cross-TU link is just the mangling-consistency oracle.
- **31 → owning core PA / DROP**: generic core C++ (templates, vtables,
  relocations, TLS, linkage) with no compat or mangling purpose.

### → hosted-compat features PA (14)

| PA | Test | Compat feature | Bucket (current) |
|---|---|---|---|
| pa35 | `600-builtin-memcpy-strlen-link-smoke` | builtin | runtime-state |
| pa35 | `600-builtin-operator-new-delete-link-smoke` | builtin | constexpr-pure |
| pa35 | `600-gnu-asm-label-overload-link-smoke` | asm-label | I/O |
| pa35 | `600-gnu-asm-leading-underscore-label-link-smoke` | asm-label | constexpr-pure |
| pa35 | `700-dependent-alias-pack-invoke-result-link-smoke` | builtin | constexpr-pure |
| pa35 | `700-hosted-builtin-bitops-promote-runtime-smoke` | builtin | constexpr-pure |
| pa35 | `700-hosted-builtin-flt-rounds-runtime-smoke` | builtin | constexpr-pure |
| pa35 | `700-hosted-builtin-fp-classification-runtime-smoke` | builtin | calls-no-result |
| pa35 | `700-hosted-builtin-fpclassify-runtime-smoke` | builtin | calls-no-result |
| pa35 | `700-hosted-builtin-huge-val-runtime-smoke` | builtin | calls-no-result |
| pa35 | `700-hosted-builtin-mul-overflow-runtime-smoke` | builtin | constexpr-pure |
| pa35 | `700-hosted-builtin-nans-runtime-smoke` | builtin | runtime-state |
| pa35 | `700-hosted-builtin-popcountg-runtime-smoke` | builtin | constexpr-pure |
| pa35 | `700-hosted-builtin-signbit-runtime-smoke` | builtin | calls-no-result |

### → pa31 abimangle (17)

Purpose is symbol-name / mangling. _Review flags: `dependent-alias-builtin-transform`,
`inline-header-odr`, `initlist-const-char-pointer` reference `std` but read more like
templating / linkage / runtime — confirm before moving._

| PA | Test | Why | Bucket (current) |
|---|---|---|---|
| pa35 | `600-hosted-abi-tag-member-link-smoke` | abi_tag mangling | cross-TU |
| pa35 | `600-hosted-basic-string-char-traits-abi-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-initializer-list-member-definition-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-itanium-substitution-mangling-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-std-initializer-list-abi-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-std-vector-pair-abi-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-template-angle-vector-pair-substitution-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `600-hosted-vector-string-substitution-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `700-dependent-alias-builtin-transform-link-smoke` | Itanium substitution of hand-declared std | constexpr-pure |
| pa35 | `700-hosted-abi-tag-class-template-member-link-smoke` | abi_tag mangling | calls-no-result |
| pa35 | `700-hosted-abi-tag-class-template-member-template-link-smoke` | abi_tag mangling | calls-no-result |
| pa35 | `700-hosted-abi-tag-function-template-link-smoke` | abi_tag mangling | constexpr-pure |
| pa35 | `700-hosted-abi-tag-operator-template-link-smoke` | abi_tag mangling | constexpr-pure |
| pa35 | `700-hosted-inline-header-odr-link-smoke` | Itanium substitution of hand-declared std | cross-TU |
| pa35 | `700-hosted-local-class-template-mangling-link-smoke` | Itanium substitution of hand-declared std | constexpr-pure |
| pa35 | `700-hosted-nested-static-abi-tag-link-smoke` | abi_tag mangling | runtime-state |
| pa35 | `700-initlist-const-char-pointer-runtime-smoke` | Itanium substitution of hand-declared std | runtime-state |

### → core PA / drop (31)

| PA | Test | Relocate to | Bucket (current) | Note |
|---|---|---|---|---|
| pa35 | `700-hosted-imported-global-got-load-link-smoke` | backend / lowir2native | cross-TU | DROP if already covered |
| pa35 | `700-hosted-pcrel-data-reloc-link-smoke` | backend / lowir2native | constexpr-pure | DROP if already covered |
| pa35 | `700-thread-local-store-register-pressure-runtime-smoke` | backend / lowir2native | constexpr-pure | DROP if already covered |
| pa35 | `600-delete-class-pointer-destroys-object-runtime` | core PA (review) / drop | runtime-state | DROP if already covered |
| pa35 | `600-hosted-enum-class-parameter-link-smoke` | core PA (review) / drop | cross-TU | DROP if already covered |
| pa35 | `600-hosted-member-operator-bang-link-smoke` | core PA (review) / drop | cross-TU | DROP if already covered |
| pa35 | `600-hosted-pair-vector-arg-ranges-link-smoke` | core PA (review) / drop | cross-TU | DROP if already covered |
| pa35 | `700-hosted-alignas-class-layout-link-smoke` | core PA (review) / drop | calls-no-result | DROP if already covered |
| pa35 | `700-hosted-function-reference-parameter-link-smoke` | core PA (review) / drop | constexpr-pure | DROP if already covered |
| pa35 | `700-hosted-header-inline-unemitted-callee-signature` | core PA (review) / drop | runtime-state | DROP if already covered |
| pa35 | `700-hosted-user-declared-trivial-dtor-return-link-smoke` | core PA (review) / drop | constexpr-pure | DROP if already covered |
| pa35 | `700-pointer-predecrement-reference-argument-link-smoke` | core PA (review) / drop | runtime-state | DROP if already covered |
| pa35 | `700-wide-string-literal-runtime-smoke` | core PA (review) / drop | runtime-state | DROP if already covered |
| pa35 | `600-hosted-special-member-entrypoint-link-smoke` | pa30 separate-compilation | cross-TU | DROP if already covered |
| pa35 | `700-hosted-namespaced-out-of-class-member-linkage-link-smoke` | pa30 separate-compilation | cross-TU | DROP if already covered |
| pa35 | `700-hosted-special-member-cross-tu-linkage-link-smoke` | pa30 separate-compilation | cross-TU | DROP if already covered |
| pa35 | `600-hosted-template-lambda-helper-link-smoke` | templating (pa18-22) | runtime-state | DROP if already covered |
| pa35 | `600-hosted-using-namespace-vector-definition-link-smoke` | templating (pa18-22) | cross-TU | DROP if already covered |
| pa35 | `600-inline-class-template-member-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `600-out-of-class-member-template-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `600-template-aggregate-return-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `600-template-inline-constructor-return-link-smoke` | templating (pa18-22) | runtime-state | DROP if already covered |
| pa35 | `700-function-template-substitution-index-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `700-inline-namespace-function-template-param-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `700-member-template-explicit-local-typedef-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `700-nested-template-local-owner-symbol-link-smoke` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `700-template-disambiguator-alias-enable-if-ctor` | templating (pa18-22) | constexpr-pure | DROP if already covered |
| pa35 | `600-polymorphic-constructor-vtable-link-smoke` | vtable/codegen (LowIR band) | constexpr-pure | DROP if already covered |
| pa35 | `700-hosted-nonvirtual-mi-vtable-layout-hostcall` | vtable/codegen (LowIR band) | runtime-state | DROP if already covered |
| pa35 | `700-hosted-pure-virtual-base-vtable-link-smoke` | vtable/codegen (LowIR band) | constexpr-pure | DROP if already covered |
| pa35 | `700-secondary-base-virtual-dispatch-view` | vtable/codegen (LowIR band) | constexpr-pure | DROP if already covered |

### RE-HOME destinations

- hosted-compat features PA × 14
- pa31 abimangle × 17
- templating (pa18-22) × 11
- core PA (review) / drop × 10
- vtable/codegen (LowIR band) × 4
- pa30 separate-compilation × 3
- backend / lowir2native × 3

### Hosted TRIAGE clusters (dedup targets)

- 5 × <string>
- 5 × <memory>
- 5 × <functional>
- 4 × <vector>
- 2 × <stdexcept>
- 2 × <deque>
- 1 × <type_traits>
- 1 × <TargetConditionals.h>
- 1 × <regex>
- 1 × <header.h>
- 1 × <fstream>
- 1 × <codecvt>
- 1 × <chrono>
- 1 × <__memory/compressed_pair.h>
