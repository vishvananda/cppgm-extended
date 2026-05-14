# PA10-PA34 Assignment Cleanup Process

## Purpose

This document defines the repo-internal cleanup process for the post-PA9
assignment series, especially `pa10` through `pa34`.

The goal is to make each assignment:

- correctly scoped against the roadmap and adjacent milestones
- tested at the right boundary with the right oracle
- internally coherent in test families, numbering bands, and coverage
- documented in a consistent, implementable README shape
- easy for a student to understand and implement without accidental hidden
  dependencies

This is a cleanup process with a deferred buildout tail, not the later
**student export** process.

## Inputs

Primary repo-local inputs:

- [ROADMAP.md](/Users/vishvananda/cppgm/ROADMAP.md)
- [ASSIGNMENT_BUILDOUT_PROCESS.md](/Users/vishvananda/cppgm/legacy/ASSIGNMENT_BUILDOUT_PROCESS.md)
- the target `paN/README.md`
- any authoritative grammar/IR spec files such as:
  - `paN/paN.gram`
  - [pa13/lowir.md](/Users/vishvananda/cppgm/pa13/lowir.md)
- the assignment harness scripts and compare scripts

Secondary maintainer references:

- `/Users/vishvananda/old/assignment-author-guide.md`
- `/Users/vishvananda/old/assignment-authoring-report.md`

Rule of thumb:

- `ROADMAP.md`, the assignment README, the grammar/IR spec, and the harness are
  authoritative for assignment ownership and behavior
- the local maintainer documents are style/process aids, not the public contract

## Out Of Scope

This cleanup pass does **not** do the later export work.

Explicitly out of scope here:

- packaging the student starter kit
- deciding the final exported tree layout
- pruning internal-only repo files from the shipped bundle
- exporting the reference binary
- repo-to-starter-kit symlink or file-rewrite cleanup
- final exported fixture curation for the student bundle

Those belong to a separate export process to be defined later.

The only export-related work allowed here is recording notes such as:

- likely student-visible support fixtures
- likely internal-only files
- export concerns to remember later

## Core Principles

This cleanup process should follow the repo rules already captured in
[ASSIGNMENT_BUILDOUT_PROCESS.md](/Users/vishvananda/cppgm/legacy/ASSIGNMENT_BUILDOUT_PROCESS.md):

- freeze the contract first
- keep grammar authoritative and README explanatory
- keep later assignments monotonic over earlier ones
- do not freeze undefined future-feature rejection unless the assignment
  explicitly requires it
- use the primary oracle at the first new boundary the assignment owns
- use link/run only when the milestone actually owns link/runtime behavior

## What This Cleanup Must Cover

Every PA in scope should be reviewed for all of the following.

### 1. Boundary And Ownership

Check that:

- the tests belong in the PA where they currently live
- the README boundary matches the roadmap and the tests
- adjacent handoff between `paN-1`, `paN`, and `paN+1` is clear

Move tests when they are really:

- an earlier core-language owner
- a later runtime/link owner
- a hosted/vendor compatibility owner rather than a core-language owner

### 2. Oracle And Test Surface

Check that the primary oracle matches the first new boundary the PA owns:

- syntax -> grammar / text output
- semantics -> semantic dump
- lowering -> LowIR
- native backend -> MIR / native output
- link/runtime -> object/link/run

Questions to answer:

- Is the current primary oracle too late?
- Is a later smoke suite being used as the main contract when it should be
  secondary?
- Is a structural oracle missing where a scaffold backend is supposed to be
  replaced?

### 3. Test Grouping, Numbering, And Order

For each PA:

- divide tests into coherent local families
- ensure family order matches the teaching and implementation progression
- ensure numeric bands are locally coherent
- move misordered tests
- rename/renumber when the current numbering obscures the structure

Use the `pa1` through `pa9` style by default unless the assignment has a better
established public split. Numeric prefixes are shared feature-family anchors,
not globally unique sequence numbers:

- `100`: first/core family, smokes, and smallest happy paths
- `200`: second/core-extension family
- `300`: third family, often invalid forms, boundaries, or ambiguity traps
- `400+`: later advanced, integration, hosted, or optimization-level families

Use in-between anchors such as `110`, `150`, or `250` only for deliberate
subfamilies or inserted groups that need to remain visually separate. Do not
renumber tests merely to make every numeric prefix unique.

Final cleanup should converge numbered suites toward the shared local-family /
number-band style used by `pa1` through `pa9`, unless a later assignment has a
real public multi-family split that needs its own visible numbering scheme.

### 4. Coverage And Gaps

Within each local family:

- identify the representative tests already present
- identify missing positive cases
- identify missing negative/boundary cases
- identify over-duplicated tests that do not add signal

Prefer:

- one smallest possible positive test per feature
- one boundary/negative test when the contract explicitly requires it
- one regression per fixed real bug

### 5. Negative-Test Discipline

Audit all negative tests and classify them:

- required deterministic rejection by the assignment contract
- acceptable implementation-defined/undefined area that should not be frozen
- later-feature rejection that should be removed or rewritten

Do not keep a negative test just to preserve:

- "not yet supported"
- random parser refusal of later syntax
- accidental current implementation limitations outside the PA contract

unless the README explicitly says rejection is required.

### 6. Monotonicity

Check that later shared implementation has not perturbed earlier in-scope
programs.

If a later shared implementation causes an earlier PA to start accepting
out-of-scope inputs, the cleanup question is:

- should the earlier test be trimmed or rewritten?

not:

- should we add extra guards just to force an older failure mode?

### 7. Harness Contract

Audit the harness as part of the assignment contract:

- test discovery suffix
- sorting/order behavior
- compare mode
- generated sidecars versus committed fixtures
- `course/paN` extension test handling
- batch versus sequential path consistency

The harness should match the README, and the README should not omit material
harness behavior when that behavior matters to the student.

Default final shape:

- one `tests/` directory per assignment

Only keep test subdirectories when they represent a real difference in test role
or oracle, for example:

- strict vs structural backend tests
- compile vs link vs runtime families
- support-fixture-heavy integration families kept distinct from the default local suite

The old `tests/spec/` vs `tests/derived/` split should be treated as a buildout
artifact, not the default final assignment layout.

### 8. Fixture And Support File Audit

For each PA, classify fixtures into:

- canonical shipped test inputs
- canonical shipped reference outputs
- support fixtures that are truly required
- generated sidecars that should stay generated

Support fixtures are acceptable only when they reduce accidental complexity
without solving the core assignment.

Examples:

- companion translation units for multi-file tests
- `.stdin` files for executable tests
- small headers / `.inl` files when a lesson truly needs them
- tiny runtime/support artifacts only when the README owns that runtime surface

### 9. README / Grammar / Harness Consistency

Verify that:

- README structure matches the expected assignment authoring pattern
- grammar/IR spec is authoritative where appropriate
- README explains the boundary rather than acting as a second grammar
- harness-visible format changes are documented
- starter-kit claims match the actual shipped files

Required README shape should generally include, in order:

1. tool overview
2. prerequisites
3. starter kit
4. exact invocation/input format
5. exact output format
6. exact error behavior
7. restrictions, if any
8. required feature list
9. testing / reference implementation notes
10. optional design notes

### 10. Implementability

For each PA, ask:

- Can a student implement this from the README, shipped tree, and earlier PAs?
- Are there hidden dependencies on internal repo knowledge?
- Are there unstated ABI/runtime assumptions?
- Are there unstated support helpers the student would have to reverse-engineer?

This is a student dry-run audit:

- read only the PA spec and shipped materials
- note every place where implementation expectations are implicit
- convert those into README clarifications, fixture additions, or test cleanup

### 11. Cross-PA Flow

After per-PA cleanup, review neighboring assignments together:

- does `paN` introduce the right thing relative to `paN-1`?
- does `paN+1` assume things `paN` never actually promised?
- are tests and handoffs aligned across the transition?

This is especially important around:

- `pa13` / `pa14+`
- `pa21` / `pa22` / `pa23`
- `pa25` / `pa26`
- `pa30` / `pa31` / `pa32`
- `pa32` / `pa33` / `pa34`

### 12. Performance / Practicality

For each PA default suite, ask:

- Is the suite reasonably fast to iterate on?
- Which tests are critical signal?
- Which tests are too heavy for default use and should be secondary?

This does not mean weakening the assignment. It means:

- keeping the default suite useful and practical
- identifying stress/derived suites separately when needed

## Cleanup Pass Structure

The work should happen in four explicit passes.

### Pass A: Audit

This pass combines the old preparation, boundary/oracle review, and test-shape
review into one audit-only tranche.

Before editing a PA:

1. Read `ROADMAP.md`
2. Read the target `paN/README.md`
3. Read the relevant grammar/IR spec if present
4. Read the harness scripts and compare mode
5. Inventory the tests and support fixtures

Then, for each PA:

1. validate test ownership against roadmap + README
2. validate primary oracle against the PA boundary
3. identify tests that need to move backward or forward
4. identify hidden cross-PA inconsistencies
5. define coherent local test families
6. reorder/renumber existing tests so the numbering bands are visible
7. audit negative tests
8. identify missing representative tests
9. reserve number bands and record deferred planned tests

Primary deliverables:

- one tracker entry for the PA
- ownership corrections
- oracle corrections
- clean family/band structure for existing tests
- coherent numbering for existing tests
- documented family/band list
- deferred planned-test list with reserved numbers

Pass A does **not** add the missing tests yet. It only identifies them,
classifies them, and reserves where they should land later.

Pass A should also identify assignments where the current `tests/spec/` and
`tests/derived/` split should be collapsed into a single `tests/` directory
during the cleanup/buildout passes.

### Pass B: README / Implementability

Still one PA at a time:

1. reshape README to match the authoring template
2. make README/harness/grammar consistent
3. clarify boundary, non-goals, and error behavior
4. document support fixtures that are truly part of the student-facing work
5. note any export concerns without trying to solve them here

Primary deliverables:

- implementer-grade README
- clear feature list
- explicit testing notes
- explicit fixture/support expectations

### Pass C: Cross-PA Flow

After the individual PAs are cleaned:

1. review the full `pa10`-`pa34` flow
2. update adjacent README handoffs where needed
3. recheck moved tests for final ownership
4. make sure the series is teachable and monotonic

Primary deliverables:

- consistent handoffs
- no orphan features
- no tests left under the wrong milestone by accident

This pass is best executed by transition group rather than by isolated PA:

- `pa13` / `pa14+`
- `pa21` / `pa22` / `pa23`
- `pa25` / `pa26`
- `pa30` / `pa31` / `pa32`
- `pa32` / `pa33` / `pa34`

### Pass D: Deferred Buildout

Only after the relevant boundary decisions are stable, especially where LowIR
or backend contracts may still move:

1. add the planned missing tests from Pass A
2. trim or replace redundant low-signal tests
3. fill reserved number bands with the now-settled tests

Primary deliverables:

- implemented deferred planned tests
- trimmed redundant tests
- final settled numbering for newly added tests

Pass D is intentionally deferred so we do not freeze the wrong new tests before
plans such as [lowir-second-tranche-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-second-tranche-plan.md)
settle the underlying boundary.

## Working Style Per PA

When actively cleaning one PA during Pass A, use this order:

1. inventory tests and support files
2. form local families and numbering bands
3. validate ownership and oracle
4. move tests if needed
5. renumber and reorder
6. reserve and classify planned new tests without adding them
7. audit README shape and content
8. audit implementability and support fixtures
9. record export concerns only as notes

## Tracker Fields

Each PA should be tracked with the following fields.

- `PA`
- `status`
- `owner/boundary summary`
- `primary oracle`
- `secondary smokes`
- `current harness mode`
- `test families / numbering bands`
- `misplaced tests`
- `oracle issues`
- `numbering/order issues`
- `coverage gaps`
- `planned new tests`
- `reserved number bands`
- `defer reason`
- `negative-test issues`
- `fixture/support issues`
- `README issues`
- `implementability issues`
- `adjacent PA notes`
- `export concerns`
- `actions`

## Initial Tracker

Use the dedicated tracker file
[`pa10-34-assignment-cleanup-tracker.md`](/Users/vishvananda/cppgm/docs/pa10-34-assignment-cleanup-tracker.md)
as the working checklist.

This high-level table records pass status only.

| PA | Pass A Audit | Pass B README/Implementability | Pass C Cross-PA Flow | Pass D Deferred Buildout | Notes |
| --- | --- | --- | --- | --- | --- |
| `pa10` | pending | pending | pending | pending |  |
| `pa11` | pending | pending | pending | pending |  |
| `pa12` | pending | pending | pending | pending |  |
| `pa13` | pending | pending | pending | pending |  |
| `pa14` | pending | pending | pending | pending |  |
| `pa15` | pending | pending | pending | pending |  |
| `pa16` | pending | pending | pending | pending |  |
| `pa17` | pending | pending | pending | pending |  |
| `pa18` | pending | pending | pending | pending |  |
| `pa19` | pending | pending | pending | pending |  |
| `pa20` | pending | pending | pending | pending |  |
| `pa21` | pending | pending | pending | pending |  |
| `pa22` | pending | pending | pending | pending |  |
| `pa23` | pending | pending | pending | pending |  |
| `pa24` | pending | pending | pending | pending |  |
| `pa25` | pending | pending | pending | pending |  |
| `pa26` | pending | pending | pending | pending |  |
| `pa27` | pending | pending | pending | pending |  |
| `pa28` | pending | pending | pending | pending |  |
| `pa29` | pending | pending | pending | pending |  |
| `pa30` | pending | pending | pending | pending |  |
| `pa31` | pending | pending | pending | pending |  |
| `pa32` | pending | pending | pending | pending |  |
| `pa33` | pending | pending | pending | pending |  |
| `pa34` | pending | pending | pending | pending |  |

## Completion Criteria

This cleanup process is complete when, for every PA in scope:

- the test placement matches assignment ownership
- the primary oracle matches the owned boundary
- the tests are organized into coherent local families and numbered coherently
- important gaps are filled and redundant tests are trimmed
- the README matches the expected structure and the actual harness
- support fixtures are justified and documented
- the assignment is implementable from its shipped spec/materials
- adjacent handoffs are clear

The later export process can then start from a cleaned assignment series
instead of having to fix assignment-boundary problems during packaging.
