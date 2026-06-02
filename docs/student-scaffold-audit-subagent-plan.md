# Student Scaffold Audit Subagent Plan

## Goal

Audit and improve the student starter scaffolds that are copied into the
exported `dev/` tree.

The export process copies `dev/*-scaffold.cpp` over the corresponding
`dev/<tool>.cpp` implementation files. These scaffolds should give students a
clear, compilable starting point without solving the assignment. In particular,
they should:

- preserve the public command-line shape students are expected to implement
- handle ordinary query/help flags where that makes the starter easier to use
- return the course-defined not-implemented status for incomplete behavior
- leave feature implementation behind explicit TODOs or small stub functions
- avoid depending on maintainer implementation internals

This pass is about starter scaffolding, not assignment scope changes, test
renumbering, reference updates, or production compiler fixes.

## References

Workers should read the references needed for their slice:

- `docs/student-assignment-export-process.md`
- `docs/student-export-inventory.md`
- `scripts/export_student_repo.sh`
- the relevant `paN/README.md`, `paN/Makefile`, `paN/scripts/`, and tests
- the relevant live implementation entrypoint in `dev/<tool>.cpp`
- the relevant scaffold sidecar in `dev/*-scaffold.cpp`
- `dev/src/tool_help_text.h`
- `/Users/vishvananda/old/cppgm`

For PA1-PA9, `/Users/vishvananda/old/cppgm/dev/*.cpp` is the historical
starter source. Preserve it unless a small compatibility fix is clearly needed
for the current export harness.

## Scope Rules

- Keep production implementation changes out of this pass.
- Keep scaffolds compile-only and starter-oriented.
- Do not copy logic from the maintainer implementation into a scaffold when it
  would solve the assignment.
- Prefer small helpers for argument collection, help output, batch
  not-implemented handling, and required option validation.
- Use `NotImplementedException` and `CPPGM_EXIT_NOT_IMPLEMENTED` for unfinished
  required behavior where the scaffold can parse enough of the invocation to
  know the request is in scope.
- If a real tool has complex mode-specific behavior, scaffold the mode dispatch
  and leave each mode body unimplemented.
- Do not edit tests, refs, or generated artifacts.
- Record unresolved packaging or README questions in the final report rather
  than guessing.

## Worker Slices

### Slice A: PA1-PA9 Historical Starters

Owns:

- `dev/pptoken-scaffold.cpp`
- `dev/posttoken-scaffold.cpp`
- `dev/ctrlexpr-scaffold.cpp`
- `dev/macro-scaffold.cpp`
- `dev/preproc-scaffold.cpp`
- `dev/recog-scaffold.cpp`
- `dev/nsdecl-scaffold.cpp`
- `dev/nsinit-scaffold.cpp`
- `dev/cy86-scaffold.cpp`

Focus:

- Compare each current scaffold to `/Users/vishvananda/old/cppgm/dev/<tool>.cpp`.
- Preserve the old starter shape unless the current export harness requires a
  compatibility improvement.
- Confirm required support headers are exported.
- Report any PA1-PA9 scaffold gaps without broad rewrites.

### Slice B: `cppgm++` Cumulative Scaffold

Owns:

- `dev/cppgm++-scaffold.cpp`

Focus:

- Cover PA10-PA12 `--emit-ast`, `--emit-types`, and `--emit-semantics`.
- Cover PA14-PA22 and PA26-PA29 `--emit-lowir`.
- Cover PA30-PA35 driver/query/preprocess/compile/link-facing modes at the
  command-line skeleton level.
- Check the real `cppgm++` CLI and the PA READMEs for options that should be
  parsed or dispatched before throwing not implemented.
- Do not implement semantic compilation behavior.

### Slice C: LowIR And Backend Tool Scaffolds

Owns:

- `dev/lowir2cy86-scaffold.cpp`
- `dev/lowir2native-scaffold.cpp`
- `dev/lowiropt-scaffold.cpp`

Focus:

- Check PA13, PA23, PA36, and PA37 command-line surfaces.
- Scaffold required option parsing for output paths, optimization levels, dump
  modes, target flags, and batch-not-implemented behavior where useful.
- Do not implement LowIR parsing, lowering, optimization, or machine code
  generation.

### Slice D: Retired Linker And EH Tool Scaffolds

Owns:

- `dev/cpplink-scaffold.cpp`
- `dev/cppeh-scaffold.cpp`

Focus:

- These old linker/EH sidecars are no longer exported after the standalone
  cpplink assignment was folded into PA30 and PA25 was refocused on
  `cppgm++ -c` host-EH facts.
- Keep them only if an internal maintainer tool still needs a scaffold smoke.

### Slice E: Export Packaging Audit

Owns no source files by default.

Focus:

- Check that `scripts/export_student_repo.sh` exports every scaffold/support
  file needed by the slices above.
- Check that generated student docs mention the exported scaffold shape without
  duplicating maintainer-only workflow.
- Report concrete packaging edits, but do not edit files unless the coordinator
  assigns them after reviewing the source-slice results.

## Worker Prompt Template

```text
You are working in /private/tmp/cppgm-template-main-integration-repair-20260430.

Task: audit and improve the student starter scaffold for <SLICE NAME>.

You are not alone in the codebase. Other workers may be editing different
scaffold files. Do not revert or rewrite edits outside your assigned files.
Your write scope is:

- <OWNED FILES>

Read first:

- docs/student-scaffold-audit-subagent-plan.md
- docs/student-assignment-export-process.md
- docs/student-export-inventory.md
- scripts/export_student_repo.sh
- the relevant paN/README.md and paN/Makefile files
- the relevant live dev/<tool>.cpp entrypoint
- the relevant dev/*-scaffold.cpp file
- dev/src/tool_help_text.h
- /Users/vishvananda/old/cppgm when your slice overlaps PA1-PA9

Decision rules:

1. Keep the scaffold a starter. Do not copy maintainer implementation logic that
   solves the assignment.
2. Add or refine command-line parsing only when it helps students start from
   the correct public tool shape.
3. Stub required behavior with NotImplementedException and
   CPPGM_EXIT_NOT_IMPLEMENTED.
4. Keep help/query handling and batch-not-implemented behavior consistent with
   the export harness.
5. Report README, inventory, help-text, or export-script gaps that you are not
   assigned to edit.

Validation:

- run `git diff --check -- <OWNED FILES>`
- if your scaffold files changed, run a focused compile command that builds the
  affected scaffold if practical; otherwise explain why it was not run

Final response:

- list files changed
- summarize useful scaffolding added or confirm no change was needed
- list any unresolved packaging/help/README questions
- list validation commands and results
```

## Coordinator Checklist

After workers finish:

1. Review every scaffold diff for assignment-scope leakage.
2. Integrate any shared `tool_help_text.h`, README, inventory, or export-script
   changes centrally.
3. Run `git diff --check`.
4. Build an exported student tree enough to confirm scaffold compilation.
5. Run focused `make test-report` or export smoke tests if build files changed.
