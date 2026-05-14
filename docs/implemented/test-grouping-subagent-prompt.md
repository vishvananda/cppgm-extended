# Test Grouping Subagent Prompt

Use this prompt template when delegating PA test grouping, test renames, README
testing-surface updates, and missing-test audits.

## Coordination Rules

- Assign disjoint PA slices to separate workers.
- Give each worker a unique notes file such as
  `docs/implemented/test-grouping-notes-pa10-pa13.md`.
- Do not let parallel workers edit
  `docs/pa10-34-assignment-cleanup-tracker.md` directly. Merge their notes into
  the tracker in one coordinator pass.
- Do not let workers commit.

## Worker Prompt Template

```text
You are Worker <NAME> for the PA test-grouping cleanup. Work in
/private/tmp/cppgm-template-main-integration-repair-20260430.

You are not alone in the codebase. Other workers may be editing different PA
directories. Do not revert edits outside your assigned write set, and do not
touch unrelated files unless needed to update references to tests you moved.

Assigned PA slice: <PA_LIST>.
Allowed write set: <PA_DIRS>, and <UNIQUE_NOTES_FILE>. Do not edit
docs/pa10-34-assignment-cleanup-tracker.md; the coordinator will merge your
notes later. Do not commit.

Read first:
- docs/plan-implementation-tracker.md
- docs/pa10-34-assignment-cleanup-process.md
- docs/pa10-34-assignment-cleanup-tracker.md
- ROADMAP.md
- doc/n3485.txt
- each assigned paN/README.md
- each assigned paN/Makefile and scripts used by its tests

Task:
Reorganize tests in your assigned PAs so the test buckets have clear meaning,
and update each PA README so it accurately documents the testing surface.

Grouping policy:
- Use tests/spec/ only for tests that directly exercise a specific standard/spec
  contract.
- For C++ language behavior in tests/spec/, add or preserve a leading comment
  naming the exact N3485 focus, using this style:
  `// N3485 focus: 14.8.2 [temp.deduct] ...`
- The N3485 citation must be specific enough that a reviewer can find the
  relevant text in doc/n3485.txt.
- If a test cannot honestly cite a specific N3485 clause, it usually does not
  belong in tests/spec/.
- Move broader regression tests, cross-feature combinations, implementation bug
  reducers, stress/sentinel cases, hosted/library sentinels, ABI/link/runtime
  smokes, and realistic program tests to tests/general/ or a PA-specific role
  bucket.
- Keep PA-specific role buckets when they represent a real oracle or harness
  difference, such as debug-info, strict/structural backend, preproc/compile,
  link/runtime, direct optimizer, driver, or optimization-level surfaces.
- Do not collapse meaningful oracle/role buckets just to force one layout.
- Treat old tests/derived as a buildout artifact unless the README/harness gives
  it a real public role.

Rename policy:
- Use git mv for test moves/renames.
- Move every sidecar with the test: .ref, .ref.stdout, .ref.exit_status,
  .ref.witness, .inspect.expect, .inspect.cmd, .compile.flags, helper .t.1/.t.2,
  .link.flags, support .h/.cpp/.inc files, etc.
- Follow the pa1-pa9 numbering style: the three-digit prefix is a shared
  feature-family anchor, not a globally unique sequence number.
- Prefer broad shared anchors such as `100`, `200`, `300`, etc. when a group of
  tests extends the same conceptual owner. Use in-between values such as `110`,
  `150`, or `250` only for deliberate subfamilies or inserted groups that need
  to remain visually separate.
- If renumbering is needed, keep local family anchors coherent:
  - `100`: first/core family, smokes, and smallest happy paths
  - `200`: second/core-extension family
  - `300`: third family, often invalid forms, boundaries, or ambiguity traps
  - `400+`: later advanced, integration, hosted, or optimization-level families
- Avoid gratuitous renumbering when moving spec to general is enough.
- Do not renumber merely to eliminate duplicate numeric prefixes.

README work:
For each assigned PA README:
- Document the actual test buckets and what each bucket means.
- Explain which bucket is spec-anchored and what citation convention is expected,
  where applicable.
- Mention any PA-specific oracle split.
- Remove stale claims about tests/derived if that bucket is removed or empty.
- Note important missing tests that should be added later, but do not add new
  tests in this pass.

Missing-test audit notes:
Create <UNIQUE_NOTES_FILE> and record a dated subsection for each assigned PA
titled `2026-05-11 Test Grouping / Rename Pass`. Record:
- final test bucket layout
- tests moved from spec to general, derived to general/spec, or between
  PA-specific buckets
- tests kept in spec and their N3485 clause anchors
- tests that still need N3485 comments
- missing spec-anchored tests to add later
- missing general/regression/integration tests to add later
- README changes made
- validation run
- open questions/risky moves deferred

Keep summaries concise but concrete: include paths or test basenames, not vague
category-only notes.

Validation:
- Run git diff --check.
- For each changed PA, run:
  `make -C paN test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  when feasible.
- If a full PA test is too expensive, run focused checks for moved tests and
  clearly say what was not run.

Final response:
List PAs changed, test directories renamed/collapsed, README sections updated,
N3485 citation or role-bucket policy applied, missing tests to add later,
validation commands/results, and files changed.
```
