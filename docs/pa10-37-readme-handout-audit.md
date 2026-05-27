# PA10-PA38 README Handout Audit

## Goal

Audit `pa10/README.md` through `pa38/README.md` so each file reads as a clear
assignment handout for the person implementing that PA.

Each README should describe:

- what the student must implement to complete the assignment
- the executable, flags, inputs, outputs, exit-status rules, and comparison
  surface
- the source-language, LowIR, object, backend, optimizer, hosted, or self-host
  boundary owned by the PA
- useful roadmap context that explains why the PA is shaped this way and what
  the implementation should be reusable for

This audit should not erase assignment goals. When a README has a goals section,
preserve it or rewrite it so the goals are explicitly framed as implementation
goals for the student to complete the assignment.

## Calibrated Forward-Reference Rule

PA1-PA9 intentionally use forward references when they help the student
understand the arc of the compiler. Follow that style.

Keep or rewrite forward references when they:

- explain why the current PA exists
- clarify what should be designed for reuse
- motivate a temporary mock, limited subset, or compatibility boundary
- identify what the current output will become a contract for later PAs
- state what is intentionally not required in the current PA

Examples of good forward-looking wording:

- PA13/PA14 LowIR text may say that LowIR becomes the contract consumed by
  future lowering and backend stages.
- PA23 may say the direct `LowIR -> machine IR -> native` boundary is intended
  to support later object, exception, optimizer, and self-host work.
- Template and class PAs may mention later milestones when that helps explain
  why the current model should be modular and reusable.

Rewrite forward references when they read like commands to use unavailable or
out-of-scope future tools. For example, prefer:

- "This LowIR shape is intended to be consumed by later native lowering."

over:

- "Validate this PA by feeding the output through PA23 `lowir2native`."

Remove forward references only when they are stale, confusing, or purely
maintainer-facing.

## What To Remove Or Rewrite

Use judgment; do not mechanically remove every hit. The target is student
clarity, not eliminating future-looking context.

- **Maintainer-only test taxonomy**: wording such as "tracked suite",
  "implementation regression", "owner", "future tests should", "older
  buildout artifact", or instructions for maintainers deciding where to place
  new tests. Replace with what `make test` runs and what behavior the student
  must satisfy.
- **Exact core-oracle filename anchors**: long lists of specific test names that
  serve as maintainer anchors. Keep the behavior goals, but do not require the
  README to enumerate internal test families unless the exact file naming is
  part of the student contract.
- **Internal validation/debugging paths**: optional maintainer workflows through
  future binaries, cross-check scripts, or private harnesses. Rewrite them as
  future contract/context when useful; otherwise remove them.
- **Temporary implementation history**: phrases such as "currently", "today",
  "for now", "still", "until this is made cheaper", or "no active suite" when
  they describe repo history rather than the assignment contract. Convert to
  stable required behavior or stable out-of-scope behavior.
- **Reference/ref-generation process drift**: keep the role of checked-in
  references, but avoid duplicating broader repository policy already covered
  by `TESTING_AND_REFERENCES.md`.
- **Student/maintainer perspective mismatch**: replace "student-facing",
  "maintainer-owned", or similar repository-process language with direct
  assignment wording aimed at the reader.

## What To Preserve

Preserve or strengthen text that states student implementation work:

- "The most important goals are..."
- "Within this milestone, PA N should..."
- "This PA must support..."
- "The intended direction is..."
- Goals sections that clearly describe what the student must implement to
  complete the assignment
- "Design notes" that recommend typed internal representations, clean reuse, or
  an architecture that later PAs can extend
- stage-handoff or roadmap sections when they explain how the current PA should
  be designed, not just where maintainers will add future tests

If a goals section is mixed with maintainer-only details, keep the goals and
remove only the maintainer details. Prefer behavior bullets over test-name
bullets.

## Per-PA Workflow

For each README, in order:

1. Read the full file before editing.
2. Identify forward references as: keep, rewrite for student context, or remove.
3. Preserve normative contract details and test behavior; do not change the
   assignment boundary unless the README plainly contradicts the harness or
   surrounding assignment docs.
4. Preserve student implementation goals. If the goals are hidden behind
   maintainer wording, rewrite them as direct PA completion goals.
5. Remove maintainer-only paragraphs instead of softening them into vague
   advice.
6. After editing, run `git diff --check` and a focused `rg` pass for suspicious
   wording in that README.
7. Update the tracker row with what changed, what remains, and whether the
   README needs a follow-up decision.

Docs-only README edits do not require compiler tests by default. Run assignment
tests only if the handout edit changes a command, reference expectation, or
assignment boundary in a way that might reveal a mismatch with the harness.

## Focused Search Pattern

Use this as the first pass for each README, then read manually:

```sh
rg -n "maintain|maintainer|regression|regressions|future|later|currently|today|for now|still|PA23|PA30|lowir2native|derived|bucket|tracked|older buildout|validation path|scaffold|cross-check|secondary|core oracle|Stage Handoff|Design Notes" paN/README.md
```

Hits are prompts for review, not automatic removals. In particular, "future",
"later", "Stage Handoff", and "Design Notes" may be correct and useful.

## Tracker

Status values:

- `pending`: not yet audited under the calibrated rules
- `edited`: README cleaned in this pass
- `no-change`: audited and left unchanged
- `blocked`: needs a contract or harness decision before editing

| PA | Status | Review Focus | Changes Made | Follow-Up / Validation |
| --- | --- | --- | --- | --- |
| PA10 | edited | Parser/AST handout; check for inactive test directories and future semantic references that should be framed as reusable AST contract. | Kept the forward-looking AST-as-later-semantic-contract language; simplified `tests/general/` wording; removed inactive `tests/derived/` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references or ordinary starter scaffold wording. |
| PA11 | edited | Type/scope handout; keep future-use architecture guidance, remove inactive test/process notes if present. | Kept the PA8/later-reuse design note; simplified `tests/general/` wording; removed inactive `tests/derived/` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful assignment or design wording. |
| PA12 | edited | Call/conversion handout; keep class/template future context when it motivates reusable call semantics. | Kept class/template future-context and reusable-call-layer guidance; simplified `tests/general/` wording; removed inactive `tests/derived/` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful boundary/design references. |
| PA13 | edited | LowIR contract handout; preserve "contract for future lowering" language, remove instructor-only/debug-process wording. | Preserved LowIR-family/future-stage contract language; removed repository-snapshot debug-helper paragraph; renamed "Later And Instructor-Only Material" to "LowIR Family Context." | Docs-only; `git diff --check` passed. Remaining scan hits are current driver exclusions, LowIR-family context, or debug-info out-of-scope contract. |
| PA14 | edited | First source-to-LowIR lowering; PA23 references may be rewritten as future-consumer context rather than current validation commands. | Preserved PA15-PA17 stage handoff and PA23 future-consumer context; removed PA23 from prerequisites; rewrote future native validation as contract context; simplified test-suite wording; fixed duplicated "The"; changed design note from CY86 values to LowIR values. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, current driver exclusions, or optional PA13 execution scaffold context. |
| PA15 | edited | Object-model lowering goals; preserve PA16/PA17 roadmap if it explains the boundary. | Preserved PA16-PA18 stage handoff and object-model goals; removed PA23 as prerequisite; rewrote PA23 validation text as future-backend contract context; stabilized `noexcept` wording; simplified test-suite wording; removed inactive `tests/derived` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, stage handoff, or optional PA13 scaffold context. |
| PA16 | edited | Value-semantics goals; preserve monotonic-extension guidance and useful PA17 context. | Preserved PA17 stage handoff and monotonic-extension goals; removed PA23 as prerequisite; rewrote PA23 validation text as future-backend contract context; simplified test-suite wording; removed inactive `tests/derived` and earlier-regression placement notes. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, stage handoff, or optional PA13 scaffold context. |
| PA17 | edited | Polymorphism goals; preserve template handoff if it frames reusable object-model work. | Preserved PA18 stage handoff and polymorphic-model goals; removed PA23 as prerequisite; rewrote PA23 validation text as future-backend contract context; simplified test-suite wording; removed inactive `tests/derived` and earlier-regression placement notes. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, legitimate derived-class wording, or optional PA13 scaffold context. |
| PA18 | edited | First template tier; keep PA19 roadmap if it clarifies what metaprogramming is out of scope. | Preserved PA19 stage handoff and first-tier template goals; rewrote PA23 validation path as future-backend contract context; simplified test-suite wording; removed non-oracle witness sidecar note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, stage handoff, or optional PA13 scaffold context. |
| PA19 | edited | Metaprogramming tier; keep PA20-PA22 roadmap if it explains constant-evaluation/template split. | Preserved PA20-PA23 stage handoff and metaprogramming goals; rewrote PA23 validation path as future-backend contract context; simplified test-suite wording; removed non-oracle witness sidecar note; softened optional test wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references, stage handoff, or optional PA13 scaffold context. |
| PA20 | edited | Constant evaluation; keep template/library motivation, clean up test taxonomy if needed. | Preserved PA20 goals and reframed them as assignment-completion goals; kept later template/library motivation and PA21/PA22 stage handoff; simplified test-suite wording; removed stray non-oracle witness note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful forward references or stage handoff context. |
| PA21 | edited | Template entity/specialization model; keep PA22 handoff, remove non-oracle/witness-process notes if confusing. | Preserved student-proof language and PA22 stage handoff; simplified test-suite wording; removed non-oracle witness sidecar note; softened optional test wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful stage-handoff text or starter scaffold wording. |
| PA22 | edited | Deduction/substitution/SFINAE completion; keep backend/toolchain handoff as roadmap context. | Preserved student-proof language and PA23 backend handoff; simplified test-suite wording; removed non-oracle witness sidecar note; softened optional test wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful PA23 handoff or starter scaffold wording. |
| PA23 | edited | Native backend; preserve detailed backend goals, remove exact core-oracle filename anchors if they read as maintainer notes. | Preserved native-backend goals and reframed them as assignment-completion goals; kept PA24/PA25 handoff, MIR comparison contract, and direct-backend quality requirements; removed exact core-oracle filename anchors; rewrote bucket/current-owner/new-test language as student-visible comparison modes and exercised behavior. | Docs-only; `git diff --check` passed. Remaining scan hits are useful roadmap, stage handoff, starter scaffold, or CY86 validation context. |
| PA24 | edited | Object/link pipeline; preserve separate-compilation goals, remove test-name anchors and future-tool commands. | Preserved separate-compilation/linker goals and reframed them as assignment-completion goals; kept PA25 stage handoff; removed exact core-oracle filename anchors; rewrote test bucket/regression/owner wording as source-driven contract language. | Docs-only; `git diff --check` passed. Remaining scan hits are starter scaffold wording and useful stage handoff. |
| PA25 | edited | Private EH/runtime pipeline; preserve private ABI goals, clean up later host-ABI references only if they obscure PA25 scope. | Preserved private EH/runtime goals and reframed them as assignment-completion goals; kept PA26-PA32 roadmap context and host-ABI out-of-scope framing; removed exact core-oracle filename anchors; rewrote test bucket/regression wording as source-driven contract language. | Docs-only; `git diff --check` passed. Remaining scan hits are useful host-ABI/roadmap context or starter scaffold wording. |
| PA26 | edited | Language-closure features; keep optimizer/backend future context if it explains the PA26 boundary. | Preserved PA26 feature goals and reframed them as assignment-completion goals; kept PA23 validation context and PA27 stage handoff; simplified test-suite wording; removed future test-placement instructions and exact core-oracle filename anchors. | Docs-only; `git diff --check` passed. Remaining scan hits are useful backend/optimizer/stage-handoff context or starter scaffold wording. |
| PA27 | edited | Advanced language closure; preserve goals, remove maintainer test-placement wording. | Preserved advanced-language goals and reframed them as assignment-completion goals; kept PA23 validation context and PA28 stage handoff; simplified test-suite wording; removed future test-placement instructions and exact core-oracle filename anchors. | Docs-only; `git diff --check` passed. Remaining scan hits are useful backend/optimizer/stage-handoff context or starter scaffold wording. |
| PA28 | edited | Object-model/ABI-heavy closure; preserve implementation goals and out-of-scope host ABI context. | Preserved multi-base object-model goals and reframed them as assignment-completion goals; kept PA23 validation context and PA29 stage handoff; simplified test-suite wording; removed future test-placement instructions and exact core-oracle filename anchors. | Docs-only; `git diff --check` passed. Remaining scan hits are useful backend/optimizer/stage-handoff context or starter scaffold wording. |
| PA29 | edited | Final language-to-LowIR closure before driver work; keep PA30 handoff if student-facing. | Preserved virtual-base/multi-vtable goals and reframed them as assignment-completion goals; kept PA30 handoff and backend context; simplified test-suite wording; removed future test-placement instructions and exact core-oracle filename anchors. | Docs-only; `git diff --check` passed. Remaining scan hits are useful PA23/PA13 validation context, stage handoff, or starter scaffold wording. |
| PA30 | edited | Driver contract; audit future hosted/runtime notes for clarity. | Preserved compile/link driver goals and PA32 handoff; reframed the core goals as assignment-completion goals; simplified test-suite wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful prerequisites, stage handoff, or starter scaffold wording. |
| PA32 | edited | Host ABI surface; preserve interoperability goals, remove internal compatibility-process notes. | Preserved host-object interoperability goals and PA33 handoff; reframed the supported-subset list as assignment-completion behavior; simplified test-suite wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful ABI roadmap, prerequisites, or starter scaffold wording. |
| PA33 | edited | Host object/codegen surface; preserve ABI/object goals, remove object-inspection maintainer notes if present. | Preserved host ABI/runtime goals and PA34 handoff; reframed the supported-subset list as assignment-completion behavior; simplified test-suite wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful ABI/object ownership language or stage handoff context. |
| PA34 | edited | Hosted header compatibility; preserve bootstrap/self-host motivation, clean up test taxonomy. | Preserved hosted compatibility and bootstrap motivation; reframed goals as assignment-completion goals; simplified test-directory wording and removed inactive `tests/spec` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful bootstrap context, earlier-owner guidance, or starter scaffold wording. |
| PA35 | edited | Hosted compile/link completion; preserve optimizer preparation if framed as design context. | Preserved hosted link/runtime contract and optimizer preparation context; reframed assignment boundary as assignment-completion behavior; simplified test-directory wording and removed inactive `tests/spec` note. | Docs-only; `git diff --check` passed. Remaining scan hits are useful symbol-ownership or stage-handoff context. |
| PA36 | edited | LowIR optimizer; preserve optimizer goals and oracle descriptions needed by students. | Preserved LowIR optimizer contract and validation-mode details; added explicit assignment-completion framing for optimization levels; rewrote bucket terminology as test-directory/validation-mode wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful prerequisite/scaffold wording or ordinary "current value" optimization language. |
| PA37 | edited | Machine/backend optimizer; preserve backend-quality goals and clarify PA23 baseline wording. | Preserved machine-backend optimizer contract, PA23 baseline context, and PA38 handoff; added explicit assignment-completion framing for backend optimization levels; rewrote bucket terminology as test-directory/validation-mode wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful `lowir2native` command/prerequisite references. |
| PA38 | edited | Self-host ladder; keep staged/inception contract, remove reducer/process text only if it is maintainer-only. | Preserved self-host ladder, checkpoint ownership, and reducer workflow; reframed inception as the assignment-completion goal; rewrote bucket/regression wording as focused-test directory wording. | Docs-only; `git diff --check` passed. Remaining scan hits are useful self-host failure-reduction guidance. |
