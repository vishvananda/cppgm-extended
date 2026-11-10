# Required assembly and later-language course integration

Maintainer plan, 2026-11-10; not a student handout or export document.
Implemented on this branch. The required assignment order, language profile
and ordinary discovery follow this plan. The migration ledger records the
fixture dispositions; qualification is recorded below.

## Base and coordination

Start from `opt/llvm-baseline-call-frames` at
`2a038636fa14a23060f0e589b0f315eea92d430c`, which includes main
`8287f480c7b5bdb2074c8941689aa36f9c233bd7`.
The isolated branch is `docs/course-structure-rewrite`. Do not change or rebase
the LLVM worker's branch or worktree. Rebase this planning commit onto its
qualified successor before starting the migration.

The LLVM work changes lowering, native selection and several early handouts,
especially PA10/12/13/14/16/24/29. Keep its accepted object-lifetime, baseline
code-generation and measurement changes. Finish that integration before the
directory renumbering or broad reference regeneration. Compiler cleanup for
assembly should be a separate commit from mechanical course moves.

The scope is the assembly profile and C++14/17 behavior implemented today,
made required and taught at the appropriate dependency boundary. It does not
require completing either language standard, expanding the assembler ISA, or
finishing RustGM's separate frontend/backend integration.

The implemented branch is now based on LLVM successor `633c162d3`; the
original base above records where planning started. Its native expectations
were refreshed through the owning reference targets before export parity was
qualified. The migration ledger records the completed fixture dispositions.

## Target order

Insert assembly after host ABI interoperation and before hosted compatibility.
At that point native encoding, objects, linking, exception metadata and host
observers exist. Assembly no longer depends on completing optimization first.

| Existing assignment | New assignment | Subject |
|---|---|---|
| PA1–PA28 | PA1–PA28 | Existing foundations, with the language additions below |
| Optional PA36 | PA29 | Required frontend-neutral assembly and integrated assembler |
| PA29 | PA30 | Hosted source/header compatibility |
| PA30 | PA31 | Broader hosted-library compile compatibility |
| PA31 | PA32 | Hosted link/runtime integration |
| PA32 | PA33 | LowIR optimization |
| PA33 | PA34 | Native/backend optimization |
| PA34 | PA35 | Self-host ladder and inception |
| PA35 | PA36 | Compiler performance and generated-code quality |

This gives **36 required assignments**, without adding a separate modern-C++
assignment. Keep the existing distinction between normal reports and the
expensive self-host/performance release checks: normal reports include new
PA29 and extend through new PA34; new PA35/36 keep their explicit checks.
Root `EXPERIMENTAL_PAS` therefore becomes `pa35 pa36`, not the old set.

Use a collision-free `git mv` map, moving old PA36 aside before shifting
PA35 down through PA29 upward. Rewrite active links, symlinks, Makefiles,
reference-wrapper routing, grammar paths, scaffold tables and manifests from
that explicit map. Do not globally substitute numbers in source text or
historical records under `docs/v4/` and `docs/v5/`.

## Required language profile

Describe the course as a C++ compiler course with a defined required
C++11/14/17 profile. Publish one concise student-facing feature/mode table;
the corresponding handout sections remain the detailed contracts. A required
C++17 feature is required in its documented mode, not necessarily in C++11.

Keep the current driver default and self-host source standard during this
structural migration. Give mode-sensitive fixtures explicit standard metadata
and test the existing C++11/14/17 distinctions. Changing the default to C++17
would be a separate behavioral change. Preserve already documented extensions
in earlier modes, including variable templates and visible-body return
deduction, rather than silently tightening their acceptance.

PA5 introduces the relevant syntax/AST forms. Later assignments own semantic
analysis and lowering once their prerequisites exist. The following table
uses **new numbers**; multiple milestones distinguish different observable
interfaces rather than requiring a later feature in an earlier lesson.

| Existing supported feature | Required teaching/validation location |
|---|---|
| Digit separators and existing binary literal support | PA1/2 token/literal contracts; PA4 preprocessing boundaries; later driver mode checks |
| Nested namespace definitions | PA5 syntax, PA6 namespace lookup; ordinary lowering follows in PA10 |
| Attribute using-prefix grammar | PA5 syntax and mode rules; GNU attribute effects remain in PA30 |
| C++14 contextual conversion selection | PA11 with user-defined conversions; composed control-flow cases after their dependencies |
| C++14 aggregates with member initializers; C++17 aggregate bases | PA11 aggregate eligibility/layout, PA12 initialization/cleanup, PA16 constant objects |
| C++17 direct result-object construction and guaranteed prvalue elision | PA12 with object lifetimes; PA16 constant evaluation; keep overlapping-subobject and earlier-mode negatives |
| Fold expressions | PA15 with packs/metaprogramming; later substitution/integration cases in PA18/19 |
| Existing relaxed constant evaluation | PA16, with an explicit supported-body contract and retained restrictions |
| Variable templates | PA17, where they are already taught; correct the whole-course language description |
| Template-template `typename`, placeholder non-type parameters | PA17 declaration/entity model, PA18 deduction and permitted argument kinds |
| Constructor deduction and declared deduction guides | PA18 deduction/overload selection; PA19 composed template cases |
| `if`/`switch` initializer statements | PA10 procedural scope/lowering; dependent combinations in PA19 |
| `if constexpr` and discarded dependent branches | PA19 after constant evaluation and substitution; distinguish parsing, definition-time checking and instantiation |
| `decltype(auto)` and visible-body placeholder return deduction | PA20 with existing `auto` deduction; preserve references, return consistency and deleted/bodyless declarations |
| Generic lambdas, initialized captures, `[*this]`, constexpr closures | PA20 after templates, constant evaluation and object lifetimes; constexpr-lambda rules refer back to PA16 |
| Structured bindings: arrays, members and existing tuple protocol | PA20; minimal local tuple-trait declarations avoid a premature STL dependency |
| Pack-expanded using declarations | PA18 after class/template lookup and packs |
| C++17 `noexcept` function types | PA6/7 typed representation/conversions, PA9 ABI vocabulary, PA16/18 dependent specifications; real host ABI/RTTI checks in PA28/32 |
| Inline variables and implicitly inline static constexpr members | PA12 object initialization, PA16 constexpr members, PA25 multi-TU identity; host/library cases in PA27/28/32 |
| Alignment-aware new/delete already implemented | PA12 allocation selection/cleanup with minimal declarations; PA25/28 target linkage; hosted `<new>` integration in PA32 |
| `variant`, `optional`, `string_view` and standard-library deduction guides | Existing hosted-library profile in PA31/32; these are consumers of earlier language features, not a new STL implementation assignment |

Do not interpret this table as a promise of full C++14/17 coverage. Freeze a
feature inventory from this base, citing each existing test and producer; add
reducers or boundary controls as needed, not unrelated new feature families.

In particular, the current required hosted fixture
`pa29/tests/compile/500-hosted-special-members-and-control-flow.t` mixes C++17
control flow with `explicit(bool)` and an alias-declaration initializer.
Those later constructs are separate compatibility extensions, supported by
[P0892R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0892r2.html)
and [P2360R0](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2360r0.html).
Split out the C++17 reducers; retain the later extension consumer in PA30.
Likewise, `[[no_unique_address]]` remains a documented later/header extension.
Do not turn these consumers into a claim of C++20/23 language coverage.

## One assembly path

Promote the existing general assembly path to the ordinary C++ path. Both
function-scope and namespace assembly must produce the shared owned
template/operand/effect records. GNU basic, extended and goto assembly, raw
functions and module assembly all reach the integrated assembler. External
frontends continue to supply the same LowIR contract without C++ syntax/sema.

Retire the instruction-string lifter rather than keeping two implementations:

- Remove `LiftAsmTemplate`, the legacy `AsmOperation`/`AsmInstruction` model
  and the corresponding operand/flag-capture interpreter. Review consumers
  before removing shared data structures.
- Replace the legacy branch in `sema/analyzer_asm.cpp` and the namespace-data
  branch in `sema/analyzer_asm_data.cpp` with the shared packet path. Remove
  `full_asm_` gates in statement, declaration, body and label-address analysis.
- Remove old instruction-specific lowering in `lower/asm_lowering.cpp`,
  `lower/asm_cpu_query.cpp` and legacy namespace-data lowering once no
  consumers remain. Reuse/rename the current `full_asm_lowering.cpp` path.
- Delete retired source-list entries and declarations. Remove `--full-asm`
  from the required interface and update the assembly runner, Godot wrappers,
  benchmark provenance and every other caller. No flag may select the lifter.
- Audit `cpu_query` and `xstate_query` LowIR roles separately. Remove their
  assembly-only producers/helpers; retire a role only after checking every
  remaining producer, native consumer, reader/validator and external contract.
  Ordinary typed intrinsics, atomics and fences retain their legitimate paths.

Explicit assembly preserves its instructions and declared effects; it is not
reinterpreted as a convenient arithmetic, atomic or fence operation. A memory
clobber remains a compiler ordering effect and does not invent a CPU fence.
Preserve constraint alternatives/ties, early writes, operand evaluation and
output stores, live registers, flags, aliases, volatile effects, goto edges,
module ordering, symbols, relocations and unwind descriptions.

PA8 documents and validates the shared transport/vocabulary. Move pure
reader/writer/schema tests there when they need no native assembly behavior;
new PA29 owns constraint binding, encoding and object/host execution. Update
`pa8/lowir.md`, the LowIR contract ledger/audits, exported model headers and
scaffold usage/spelling checks together. Retain a direct LowIR test independent
of C++ for each boundary; Rust integration remains separately qualified.

New PA29's baseline must work at O0, without an optimizer prerequisite. Move
the existing O3 preservation checks into new PA33/34's normal optimizer lanes,
reusing the fixture selection and common runner. Later optimizer tests verify
declared effects and liveness; they do not inspect/rewrite assembly strings.
Keep the existing ISA/target limits and explicit platform/CPU skips. Encoding
an AVX-512 instruction is different from requiring AVX-512 execution on every
course host. Debug-info and optional target-feature lanes remain distinct.

## Fixture migration and normal test discovery

Inventory at the base commit:

| Current selection | Test drivers |
|---|---:|
| PA29 `tests/modern-cxx/compile` | 84 |
| PA29 `tests/modern-cxx/run` | 41 |
| PA31 `tests/modern-cxx/link` | 8 |
| PA36 `tests/spec`, `tests/source`, `tests/reject` | 33 + 57 + 92 = 182 |
| PA29 test drivers with `asm` in the basename | 35 |

The last row is a triage list, not an exhaustive semantic assembly search.
Search fixture contents and companion inputs as well. Refresh the inventory
when rebasing onto the LLVM worker's final tip.

Create a maintainer migration ledger with old path, every companion/sidecar,
new owner/cluster, governing feature/mode, disposition and replacement tests.
Account for all 133 optional language drivers and 182 assembly drivers. Do not
discard required C++11 correctness checks or LLVM reducers to simplify moves.

For each language fixture:

1. Move a self-contained language reducer to its earliest owning assignment's
   existing required lane. Convert hosted compile/run harnesses to that
   assignment's semantic/LowIR interface where necessary; an early lesson
   cannot require `-c`, host headers or native execution before PA24/25.
2. Split mixed-feature cases. A generic lambda is a PA20 case even if a small
   part tests an earlier type rule. A hosted header consumer stays in new
   PA30–32, with an earlier library-free reducer for the underlying language.
3. Keep multi-TU/object/host ABI observations where those interfaces exist.
   Move companions, flags, platform markers and references as one fixture.
4. Give each owner normal numbered clusters and spec/general/behavior/control
   coverage. Use governing clauses and observable correctness/quality bounds;
   do not prescribe an instruction sequence or the reference implementation's
   internal algorithms.

Concrete starting candidates include PA29's `600-structured-binding-*`,
`600-fold-expression-*`, the control-flow fixture above, and PA31's
`600-hosted-inline-static-members`. The tuple-reference modern fixture uses
local trait declarations already and is suitable for a PA20 reducer. Keep
standard-header variants as later integration tests.

Review old assembly negatives individually: invalid constraints/types or
invalid assembler forms remain negatives, but rejection solely because a
template fell outside the lifter's finite set must disappear. For example,
`500-gnu-asm-carry-without-producer-bad.t` reflects the lifter's flag-producer
restriction, not a general requirement to infer instruction semantics from
user assembly. Replace such expectations with valid contract controls. Regenerate
changed references with the normal tools; never hand-edit their payloads.

Use the existing Perl controls and standard parallel runner. Extend the
existing per-fixture standard metadata to the early semantic/LowIR routes,
applying it equally to implementation and reference commands. Make required
fixtures run under `test`, `test-batch`, `ref-test`, cumulative reports and
single-file `check`. Remove the special `test-modern-cxx` targets and the
separate assembly invocation once discovery proves ordinary coverage.

Update `scripts/tests/test_fixture_lanes.py`: it currently explicitly asserts
that modern fixtures and their mode sidecars remain optional. Replace that
assertion with required discovery and exactly-once execution checks. Update
the placement auditor's optional-lane exclusions, feature ownership/version
rows, assembly markers, native/language phase classification, hosted-header
thresholds and no-feature owners (new PA35/36). Its default scan must actually
cover the moved cases. Update audit/harness tests and export reference parity.

## References, handouts and student export

Keep N3485 for existing C++11 clauses and adopted-defect history. Add pinned,
offline C++14 and C++17 draft references under `doc/`, with text extraction,
source URL/date/checksum and retained attribution. Use
[N4140](https://github.com/cplusplus/draft/tree/n4140)
and [N4659](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
as the versioned starting points; their full library chapters are references,
not additional course requirements. Link the already retained WG21 papers
where they explain a supported change. Add the new files to the explicit
student document manifest and reference inventory, and update `NOTICE`.

Cite draft number plus stable clause tag, not an unqualified paragraph number
from another edition. Record original-version rules, adopted DRs and supported
extensions separately. Audit any oracle disagreement against its pinned
reference and the existing exclusion/provenance records; the new draft does
not automatically impose later CWG decisions on older modes.

Rewrite affected handouts using their usual overview, prerequisites, starter
kit, input/output, required surface, tests and out-of-scope structure. Each
added section states semantics, prerequisites, limits and how to validate it.
Keep implementation names, run journals and this plan out of student prose.
Keep the assembly model/header scaffolds in the export.

Update `ROADMAP.md`, main `README.md`, `spec.md`, root `AGENTS.md`,
`TESTING_AND_REFERENCES.md`, the actual root templates under
`docs/student-export-root/`, assignment grammars/design notes and
`scripts/student_export_documents.txt`. Replace whole-course “C++11 only” or
“optional C++14/17/assembly” claims; retain C++11 wording for genuine
mode-specific tests, upstream correctness campaigns and host-build flags.
Audit links and active assignment numbers instead of blanket text replacement.
The new PA36 is performance, so stale links to `pa36/assembly.md` must fail
validation rather than silently point to an unrelated assignment.

## Delivery order and acceptance

1. **Freeze the base and inventory.** Rebase onto the qualified LLVM result,
   preserve its test/handout updates, and complete the move ledger and bounded
   feature/mode table. Make these independent of live compiler edits.
2. **Unify assembly before renumbering.** Default to the shared path, port
   legacy consumers/negatives, remove dead lifter code, and qualify source,
   direct LowIR, object and host behavior. No new ISA-completeness requirement.
3. **Integrate language lessons in the existing order.** Move core reducers
   and required guidance in small feature groups, teach the actual supported
   modes, and retain hosted consumers. Add the pinned references and grammar
   updates. Do not change the global default standard in this step.
4. **Renumber and switch discovery atomically.** Apply the explicit PA map;
   update all root and per-assignment targets, audits, wrappers and export
   rules. Fold modern/assembly checks into normal Linux/macOS reports, remove
   their duplicate CI jobs/steps, and route self-hosting to PA35 and quality
   checks to PA36. Keep benchmark settings and CPU lanes intact.
5. **Qualify the complete course and export.** Verify the fixture ledger has
   no losses, duplicates or unexplained exclusions. Run harness/placement and
   all architecture/format/file audits; affected suites; full strict report;
   debug-info; the new PA35 self-host through PA5 and inception; and new PA36
   smoke/release checks. Run supported Linux GCC/Clang and Intel macOS lanes.
   Regenerate references only through their owning tools and validate a fresh
   student export, document inventory and installed reference wrappers.
6. **Confirm performance and publish.** Compare immutable pre/post compilers
   on frozen compile inputs, public workloads and LLVM's qualified workload.
   Check generated output equality, compiler overhead and self-build codegen;
   use counters plus paired native timing with A/A controls where indicated.
   The opaque assembly path may alter optimization opportunities: explain
   measured differences and fix general issues rather than restoring a hidden
   instruction lifter. Publish only after required checks pass.

Completion means the normal course teaches and tests the existing supported
language profile and assembly path; later compatibility lessons consume those
features; default reports cover their required cases on both supported hosts;
and the exported course reproduces the same structure. The implementation includes compiler, test harness, handout and export changes.

## Qualification

- Strict ordinary report: 10,556/10,556; debug-info checks passed.
- Self-host through PA5 passed; inception matched every object and the compiler.
- Harness, architecture, placement, scaffold and student-document audits passed.
- Fresh export using immutable tools: 24,077 portable reference outputs matched,
  including 9,683 exit-status files; reference expectations and scaffolds passed.
- PA36 smoke passed, and course/student release calibration passed all 17 gates.
- Against LLVM base `633c162d3`, frozen workload objects are identical at O0/O3;
  compiler instruction counts differ by -0.002%/+0.005% respectively.
- PR #123 runs supported Linux toolchain checks; the Intel macOS lane was
  dispatched separately. CI results remain separate from local qualification.
