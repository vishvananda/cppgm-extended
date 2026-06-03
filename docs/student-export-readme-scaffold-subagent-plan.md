# Student Export README And Scaffold Subagent Plan

## Goal

Prepare the student-facing export by making the PA READMEs and starter
scaffolding inventory match the actual package students will receive.

This is a preparatory export pass. It does not change assignment scope, move
tests between PAs, or implement compiler features. The intended output is:

- in-place edits to the existing `paN/README.md` files, which are the
  student-facing assignment handouts
- explicit per-PA scaffold/support-file inventory
- a small list of unresolved export blockers that require coordinator decisions

Do not create separate duplicate exported README files. The export process should
copy the edited `paN/README.md` files into the student repo.

## Inputs

Use these as the required references for every slice:

- `docs/student-assignment-export-process.md`
- `docs/pa10-34-assignment-cleanup-tracker.md`
- `/Users/vishvananda/old/assignment-author-guide.md`
- `/Users/vishvananda/old/assignment-authoring-report.md`
- the assigned `paN/README.md`, `paN/Makefile`, `paN/scripts/`, `paN/tests/`,
  and any `paN/*.gram`, `paN/lowir.md`, `paN/grammar/`, or role-specific docs
- scaffold sidecars in `dev/*-scaffold.cpp`

The authoring guide gives the README shape:

1. overview
2. prerequisites
3. starter kit
4. command-line invocation or input format
5. output format
6. error handling
7. restrictions and course-defined behavior
8. required features
9. testing and reference implementation notes
10. optional design notes

## Scope Rules

- Keep normative assignment requirements first.
- Keep implementation advice under clearly non-normative design notes.
- State the exact binary and invocation for ordinary tool assignments, or the
  exact build/test targets for PA38-style staged assignments, plus inputs,
  outputs, and success/failure behavior.
- Make the `Starter Kit` section match the export package, not the maintainer
  checkout.
- Identify exactly which files students edit.
- Identify which support files are provided and why they do not solve the core
  assignment challenge.
- Describe tests as part of the assignment contract.
- For LowIR-facing assignments, distinguish exported loose student validation
  from maintainer strict textual LowIR comparison.
- Use relative links inside READMEs. Do not add `/Users/...` paths.
- Do not expose maintainer-only targets, frontier/debug scripts, performance
  tools, or internal audit workflows as part of the student assignment.
- Do not edit reference outputs, generated artifacts, or test placement unless
  the coordinator explicitly assigns that work.

## Cleanup Status

The broad README/scaffold pass has been completed for PA10-PA38. The current
state is tracked in `docs/student-export-inventory.md`.

Resolved items from the initial audit:

- PA10+ READMEs describe the editable `dev/` entry points and scaffold sidecars.
- PA14-PA22 READMEs name the required `--emit-lowir -O0` invocation.
- PA13 is scoped to the `lowir2cy86` surface, with later debuginfo/native tests
  recorded as a separate packaging decision.
- PA14-PA18 and PA23-PA30 grammar helper scripts point at the matching
  `paN.gram`.
- Stale nested PA17 `grammar/grammar/` and `scripts/scripts/` trees were removed.
- PA18-PA22 handouts no longer present "Missing Tests To Add Later" as student
  scope, and witness sidecars are described as outside the required oracle unless
  export policy changes.
- PA32-PA35 handouts assume a Linux host/toolchain surface.
- PA36/PA37 handouts now describe optimizer/backend behavior and testing
  contracts.
- PA38 is now a staged self-host handout rather than a wrapper status note.
- The stale PA1-PA9 and PA23 `scripts/export_pa.sh` files were removed.
- Scaffold TODO ownership has been aligned with the current assignment split.

Remaining decisions are about export shape and validator packaging, not README
cleanup.

## Coordinator Work Before Spawning

Before starting edit workers, the coordinator should create or choose the
central export inventory file. The recommended file is
`docs/student-export-inventory.md`.

Each worker should add rows or sections only for its assigned PA range, using
this schema:

- PA
- exported binary or binaries, or staged targets for PA38-style assignments
- student-editable files
- scaffold source used for export
- public support files
- public tests and refs
- public grammar/spec files
- reference binary/binaries
- shared scripts required
- wrapper/runtime helpers required
- internal-only files omitted
- special packaging notes
- unresolved decisions

## Subagent Slices

Use worker agents for edit passes. Each worker owns only its assigned files and
must not revert or rewrite other workers' changes.

### Slice A: PA10-PA13

Owns:

- `pa10/README.md` through `pa13/README.md`
- corresponding PA sections in `docs/student-export-inventory.md`

Focus:

- AST, namespace/type output, and first LowIR/CY86 handoff
- grammar/LowIR public docs
- clear `cppgm++ --emit-*` and `lowir2cy86` contracts
- exported starter scaffold mapping for `cppgm++` and `lowir2cy86`

### Slice B: PA14-PA17

Owns:

- `pa14/README.md` through `pa17/README.md`
- corresponding inventory sections

Focus:

- LowIR generation and source-language growth
- N3485/spec-vs-general test explanation
- public grammar artifacts
- student-visible output and error behavior

### Slice C: PA18-PA23

Owns:

- `pa18/README.md` through `pa22/README.md`
- `pa23/README.md`
- corresponding inventory sections

Focus:

- template and constexpr assignment boundaries
- template integration boundary in PA23
- strict/witness tests as maintainer validation versus student-facing tests
- source-to-LowIR contract without exposing internal witness implementation as
  a required student oracle unless it ships

### Slice D: PA24-PA31

Owns:

- `pa24/README.md` through `pa31/README.md`
- corresponding inventory sections

Focus:

- source-level runtime features, `lowir2native`, driver, ABI, and host-EH facts
- native/object/runtime prerequisites
- scaffold sidecars for `lowir2native` and `cppgm++`
- grammar helper consistency

### Slice E: PA32-PA35

Owns:

- `pa30/README.md` through `pa35/README.md`
- corresponding inventory sections

Focus:

- object generation, host ABI/runtime, hosted compile/link compatibility
- host compiler, symbol-tool, and wrapper prerequisites
- filtering maintainer-only hosted-header sweep/frontier tooling out of the
  student contract

### Slice F: PA36-PA37

Owns:

- `pa36/README.md` and `pa37/README.md`
- corresponding inventory sections

Focus:

- LowIR optimizer and machine-backend optimization
- optimization-level contracts
- direct text compare versus student structural validation
- removal of absolute local links

### Slice F2: PA38 Initial Audit Only

Owns:

- the PA38 section of `docs/student-export-inventory.md`
- optional notes for `pa38/README.md`, but not a broad rewrite

Focus:

- identify the student-facing self-host ladder contract
- identify source sets, generated checkpoint binaries, reference tools, and
  host compiler prerequisites
- separate maintainer implementation details from public workflow
- record what needs a focused PA38 rewrite after the general README pass

PA38 does not have a single assignment binary or one scaffold entrypoint. Treat
it as a staged build/test harness over the cumulative compiler tools.

### Slice G: Shared Export Infrastructure

Owns only docs or draft manifests unless the coordinator explicitly authorizes
Makefile/script edits:

- exported root README plan
- exported root AGENTS plan
- shared-root inventory section
- shared scripts and wrappers inventory
- root/dev Makefile export notes

Focus:

- what the generated student repo should contain
- which current shared scripts are public
- which maintainer-only targets stay out
- how root `make`, `test-report`, and per-PA targets should be documented

### Slice H: Scaffold Audit

Owns:

- scaffold inventory sections only
- optional focused fixes to scaffold usage/help text if assigned

Focus:

- verify each `dev/*-scaffold.cpp` exists
- verify scaffold `--help`/usage text matches the corresponding live tool
  contract described by the PA README
- identify PA ranges that share one scaffold, especially `cppgm++`
- identify support headers required by PA1-PA9 and later shared wrappers

## Worker Prompt Template

```text
You are working in /private/tmp/cppgm-template-main-integration-repair-20260430.

Task: prepare the student-export README/scaffold pass for <PA RANGE>.

Important: edit the existing paN/README.md files in place. Those READMEs are
the student-facing handouts and should be copied by the export. Do not create
parallel exported README files.

You are not alone in the codebase. Other workers may be editing different PA
ranges and shared docs. Do not revert edits outside your assigned files. Keep
your changes scoped to:

- <OWNED README FILES>
- the <PA RANGE> section of docs/student-export-inventory.md

References to read first:

- docs/student-assignment-export-process.md
- docs/student-export-readme-scaffold-subagent-plan.md
- docs/pa10-34-assignment-cleanup-tracker.md, if your range overlaps PA10-PA35
- /Users/vishvananda/old/assignment-author-guide.md
- /Users/vishvananda/old/assignment-authoring-report.md
- each assigned paN/Makefile, paN/scripts/, paN/tests/, and public grammar/spec
  files
- relevant dev/*-scaffold.cpp files

Edit goals:

1. Make each assigned README student-facing:
   - exact binary and invocation, or exact build/test targets for PA38-style
     assignments
   - exact inputs, outputs, and exit behavior
   - prerequisites
   - exported starter kit contents
   - files students edit
   - support files included in the export
   - test buckets and grading contract
   - course-defined behavior
   - optional design notes separated from requirements
2. Remove or reframe maintainer-only language:
   - frontier/debug/perf/audit process
   - internal validation-only flags
   - absolute local paths
   - claims about files that will not ship
3. Add or update the inventory rows for your assigned PAs.
4. Record unresolved export decisions in the inventory instead of guessing.

Do not:

- move tests
- update refs
- edit production compiler code
- expose maintainer-only scripts as public workflow
- add exact diagnostic requirements unless the assignment already requires them

Validation:

- run git diff --check for your assigned files
- run rg for absolute /Users paths in your assigned READMEs
- if you change Makefiles or scripts, run the relevant make test target;
  otherwise README/inventory changes do not require full compiler tests

Final response:

- list files changed
- summarize README contract changes by PA
- list inventory gaps/unresolved decisions
- list validation commands and results
```

## PA38 Follow-Up Rewrite

After the broad workers finish, run a focused PA38 rewrite. That pass should not
try to fit PA38 into the ordinary binary README template. It should instead
document:

- what the self-host ladder proves
- which checkpoint tools are rebuilt
- which source sets are public inputs
- how `CXX` and `CPPGM_HOST_CXX` are used
- which targets students run
- how failures are diagnosed
- which maintainer-only source-set machinery or generated artifacts are omitted
  from export

## Coordinator Merge Checklist

After workers finish:

1. Check every PA has an inventory entry.
2. Check every README has a synchronized `Starter Kit` / student-editable file
   section.
3. Check no student-facing README contains absolute maintainer paths.
4. Check LowIR-facing READMEs consistently explain loose student validation.
5. Check all scaffolds listed in the inventory exist.
6. Check unresolved decisions are real export choices, not worker uncertainty
   that can be answered from the tree.
7. Run `git diff --check`.

Do not run a full `make test-report` for README-only changes. Run focused tests
only if Makefiles, scripts, tests, refs, or scaffold source behavior changed.
