# Student Assignment Export Process

## Purpose

This document defines the **final export process** that will turn the
maintainer repo into a student-facing assignment repository.

The target export format should follow the existing outer shape of
`/Users/vishvananda/old/cppgm`, especially the PA1-PA9 starter-kit layout,
while carrying forward the better infrastructure that now exists in the active
repo:

- modern GNU make detection
- default parallel build/test behavior where appropriate
- the shared batch-aware test helpers
- the newer wrapper/helper scripts, including
  [scripts/cppgm-cmake-wrapper.sh](../scripts/cppgm-cmake-wrapper.sh)
  where that wrapper is part of the assignment story

This process is intentionally separate from the repo-internal assignment cleanup
pass described in
[pa10-34-assignment-cleanup-process.md](pa10-34-assignment-cleanup-process.md).

## Goal

The end result should be a student-ready repo that:

- uses the established course assignment layout
- contains the tests, fixtures, READMEs, and starter support files needed for
  each assignment
- preserves the existing exported `pa1` through `pa9` assignments unless a
  change is a critical fix or an approved infrastructure improvement
- includes reference binaries and harness helpers as needed
- includes updated root-level student-facing docs such as `README.md` and
  `AGENTS.md`
- does **not** ship the full maintainer implementation source tree
- is internally self-contained and does not depend on the maintainer repo at
  runtime

## Current Preparatory Work

The README/scaffolding preparation pass is tracked separately so that workers
can update student-facing assignment docs without starting the final repository
generation step too early:

- [student-export-readme-scaffold-subagent-plan.md](student-export-readme-scaffold-subagent-plan.md)
- [student-scaffold-audit-subagent-plan.md](student-scaffold-audit-subagent-plan.md)
- [student-export-inventory.md](student-export-inventory.md)

## Reference Inputs

Primary inputs:

- the cleaned assignment directories in this repo
- [ROADMAP.md](../ROADMAP.md)
- [ASSIGNMENT_BUILDOUT_PROCESS.md](../legacy/ASSIGNMENT_BUILDOUT_PROCESS.md)
- the target `paN/README.md`
- the target harness scripts and test fixtures

Packaging and style references:

- `/Users/vishvananda/old/cppgm`
- `/Users/vishvananda/old/assignment-author-guide.md`
- `/Users/vishvananda/old/assignment-authoring-report.md`

## Out Of Scope

This export process does **not** decide assignment scope or move tests between
PAs. That must already be done by the cleanup/buildout pass.

Also out of scope here:

- redesigning the assignment roadmap
- fixing feature bugs in the maintainer compiler
- changing the normative assignment contract after cleanup is complete

## Target Export Shape

The exported repository should preserve the old student-facing outer shape:

```text
cppgm/
├── AGENTS.md
├── Makefile
├── README.md
├── dev/
├── pa1/
├── pa2/
├── ...
├── pa37/
├── tests/                  # shared public test assets only
└── doc/                    # public course docs only
```

Each exported assignment should preserve the familiar starter-kit pattern:

```text
paN/
├── README.md
├── Makefile
├── <binary>-ref
├── scripts/
├── tests/
├── course/                 # if that assignment uses shared course tests
├── grammar/                # only when the grammar is part of the public kit
└── extras/                 # only when explicitly needed
```

PA37 is the main exception to the single-binary pattern. It is a staged
self-host validation assignment, so its export inventory should describe
checkpoint targets, source sets, host compiler requirements, and generated
checkpoint tools instead of forcing it into a `<binary>-ref` / one-entrypoint
shape.

Important rule:

- match the **outer student format** of the old repo
- do **not** blindly copy the maintainer repo internals into that format

The export should be a curated student package, not a mirror of the maintainer
tree.

## Core Principles

### 1. Export From Cleaned Assignments Only

Do not export a PA until its cleanup pass is complete:

- boundary is correct
- oracle is correct
- tests are grouped and numbered correctly
- README is aligned with the assignment authoring guidance
- support fixtures are understood

### 2. Preserve The Student-Facing Contract

The exported repo should preserve:

- the established shipped PA1-PA9 assignment shape by default
- the assignment README contract
- the committed tests and refs
- the expected CLI
- the public harness behavior

It should not accidentally expose extra maintainer-only behavior just because it
exists in the active repo.

For `pa1` through `pa9`, the default export policy is:

- preserve the existing exported assignment packages
- do not rewrite them just because the maintainer repo has evolved
- only change them when the export is carrying:
  - a critical correctness fix
  - a clearly better shared harness or Makefile improvement
  - a root-level packaging improvement that benefits the whole student repo

### 3. Ship Better Infrastructure Where It Improves The Student Experience

The export should carry forward improvements that make the student repo more
usable without revealing internal implementation details.

Examples:

- modern GNU make detection
- processor-count-based parallel build defaults
- the improved `test-report` orchestration where appropriate
- the shared batch test helpers when they are part of the public harness
- newer wrapper scripts, especially `cppgm-cmake-wrapper.sh` for the late
  hosted-toolchain assignments if that wrapper is part of the student-facing
  workflow

### 3a. Export The Student Validation Mode, Not The Maintainer Validation Mode

For LowIR-facing assignments, the export should deliberately distinguish
between:

- the strict maintainer validation used in this repo
- the looser student-facing validation used in the exported repo

The maintainer repo should keep strict textual LowIR matching so that subtle IR
changes remain visible and checked in intentionally.

The exported student repo should instead ship the loose validator mode that:

- still enforces well-formedness and semantic/ABI correctness
- still checks the required public IR facts
- does **not** require the student to match our exact internal helper naming,
  ordering, or other non-semantic LowIR presentation details

So the export process should not blindly copy the maintainer LowIR harness. It
should export the student-oriented validation configuration instead.

### 4. Do Not Ship Maintainer Implementation Source By Default

The student repo should not include the full maintainer implementation tree.

Instead, export only:

- starter source files intended for students to edit
- public helper/support files that the assignment expects
- reference binaries and harness-side helper binaries/scripts

The exported repo should preserve the old-repo source layout:

- the real starter source files live in `dev/`
- the `paN/<binary>.cpp` files are symlinks back into `../dev/`

For binaries where the maintainer repo keeps a sidecar scaffold file, the export should
copy that scaffold over the live `dev/<binary>.cpp` entrypoint in the exported tree while
preserving the `paN` symlink layout. The current scaffold sidecars are:

- [pptoken-scaffold.cpp](../dev/pptoken-scaffold.cpp)
- [posttoken-scaffold.cpp](../dev/posttoken-scaffold.cpp)
- [ctrlexpr-scaffold.cpp](../dev/ctrlexpr-scaffold.cpp)
- [macro-scaffold.cpp](../dev/macro-scaffold.cpp)
- [preproc-scaffold.cpp](../dev/preproc-scaffold.cpp)
- [recog-scaffold.cpp](../dev/recog-scaffold.cpp)
- [nsdecl-scaffold.cpp](../dev/nsdecl-scaffold.cpp)
- [nsinit-scaffold.cpp](../dev/nsinit-scaffold.cpp)
- [cy86-scaffold.cpp](../dev/cy86-scaffold.cpp)
- [cppgm++-scaffold.cpp](../dev/cppgm++-scaffold.cpp)
- [lowir2cy86-scaffold.cpp](../dev/lowir2cy86-scaffold.cpp)
- [lowir2native-scaffold.cpp](../dev/lowir2native-scaffold.cpp)
- [cpplink-scaffold.cpp](../dev/cpplink-scaffold.cpp)
- [cppeh-scaffold.cpp](../dev/cppeh-scaffold.cpp)
- [lowiropt-scaffold.cpp](../dev/lowiropt-scaffold.cpp)

Those scaffold files are intended to copy directly into the exported tree without
post-processing. In particular, their basic `--help` / `-h` usage text should stay
accurate and aligned with the corresponding live maintainer entrypoints.

The scaffold sidecars also depend on a few compatibility support headers that
should export alongside the starter sources when those assignments are packaged:

- [IPPTokenStream.h](../dev/src/IPPTokenStream.h)
- [DebugPPTokenStream.h](../dev/src/DebugPPTokenStream.h)
- [exceptions.h](../dev/src/exceptions.h)
- [tool_help_text.h](../dev/src/tool_help_text.h)

### 5. Prefer Manifest-Driven Export Over Blind `rsync`

The old repo used very simple `export_pa.sh` scripts that effectively did a
blind `rsync`. That is not precise enough for the new post-PA9 assignment set.

The new export process should be manifest-driven:

- define what each PA exports
- define which shared files export once at the repo root
- define which tests/fixtures are public
- define which implementation files are omitted

## Export Phases

## Phase 1: Freeze The Export Contract

Before exporting, record for each PA:

- exported binary name, or staged checkpoint targets for PA37-style assignments
- exported README
- exported tests and refs
- exported support fixtures
- exported grammar/IR spec files
- exported reference binary or binaries
- exported starter source files
- exported shared helper scripts needed by that PA
- explicit omissions

This should produce one export inventory row per PA.

For `pa1` through `pa9`, the inventory must also mark whether the export is:

- `preserve as-is`
- `preserve with critical fix`
- `preserve with shared infrastructure update`

That prevents accidental churn in the already-stable early assignments.

## Phase 2: Define Shared Repo-Level Infrastructure

Decide which repo-level files become part of the public exported repo.

Likely exported:

- root `README.md`, rewritten for the student exported repo
- root `AGENTS.md`, rewritten for the student exported repo
- root [Makefile](../Makefile), adapted to the student
  tree
- `dev/Makefile`, adapted to the student tree
- public shared scripts used by assignment harnesses
- public shared test assets

Likely not exported directly:

- maintainer-only bootstrap scripts
- maintainer-only diagnostics and audit docs
- maintainer-only profiling and performance tooling
- maintainer-only validation helpers not used by students

### Shared Infrastructure That Should Probably Be Carried Forward

At minimum, review these for export into the student repo:

- the exported root `README.md`
- the exported root `AGENTS.md`
- root [Makefile](../Makefile)
- [dev/Makefile](../dev/Makefile)
- [scripts/run_all_tests_common.pl](../scripts/run_all_tests_common.pl)
- [scripts/CppgmBatchWorker.pm](../scripts/CppgmBatchWorker.pm)
- [scripts/cppgm-cmake-wrapper.sh](../scripts/cppgm-cmake-wrapper.sh)

The export version of these files should keep the improvements that matter for
students:

- updated root workflow documentation

For LowIR-related harness pieces, "carry forward" means:

- preserve the improved harness structure
- preserve the real semantic/ABI validation rules
- switch from maintainer strict-text comparison to the exported loose student
  comparison mode where the assignment contract intends that flexibility

## LowIR Export Rule

The export process should explicitly treat LowIR validation as a forked policy:

- maintainer repo:
  - strict textual comparison remains in place
- exported student repo:
  - loose structural/semantic comparison is used instead

This policy should be documented in the exported README/assignment docs where
relevant so students are not led to believe that exact internal helper spelling
and ordering are part of the public contract.
- GNU make detection
- default parallel `make`
- shared batch-aware test execution
- wrapper support for late hosted assignments

## Phase 3: Per-PA Export Curation

Handle each PA in order, `pa1` through `pa37`.

For each PA:

1. Export the existing `paN/README.md` after it has been edited in place to its
   final student form.
2. Export the PA Makefile in student-repo style.
3. Export the public tests, refs, and support fixtures.
4. Export any public grammar/spec files.
5. Export the starter editable files for that assignment.
6. Export the reference binary/binaries.
7. Export only the helper scripts that the PA needs.

Special rule for `pa1` through `pa9`:

- treat the existing exported assignment packages in `/Users/vishvananda/old/cppgm`
  as the baseline
- do not re-author them from scratch unless necessary
- apply only the approved fixes and infrastructure upgrades that should flow
  into the student repo
- keep their student-facing assignment content stable unless a real correction
  is required

Questions to answer for every PA:

- What files is the student expected to edit?
- What public support files does the PA require?
- What helper/runtime/wrapper scripts must be included?
- What files from the maintainer repo must **not** ship?
- Does this PA need a shared wrapper or helper that earlier PAs do not?

## Phase 4: Repo-Wide Validation Of The Exported Tree

After export, validate the exported repo as an independent product.

Required checks:

- root `make` works in the exported repo
- per-PA `make test-paN` works as intended
- the exported test harnesses do not reach back into the maintainer repo
- no `/Users/.../cppgm` absolute paths remain
- no maintainer-only scripts are referenced
- late hosted assignments can use the exported wrapper path if they require it
- parallel build/test behavior still works in the exported tree

## What The Export Must Modernize

The old repo is useful as a layout reference, but it should not be copied
verbatim.

The export process should intentionally modernize these areas.

### 1. Root And `dev/` Makefiles

Replace the older simpler Makefiles with student-facing versions that preserve
today's useful improvements:

- GNU make autodetection
- processor-count-based `-j` defaulting
- propagated `CXX` and host compiler defaults
- consistent root-to-PA test entry points

The exported Makefiles should keep the good ergonomics without dragging in
maintainer-only targets that students do not need.

### 1a. Root Student Docs

The exported root docs should also be updated, not merely copied from the old
repo.

At minimum:

- exported `README.md`
- exported `AGENTS.md`

These should reflect:

- the final exported assignment range
- the exported repo layout
- the expected student workflow
- the modernized shared build/test behavior
- any wrapper-based late-assignment workflow that students are expected to use

### 2. Shared Test Harness

The exported repo should use the shared harness structure rather than many
assignment-specific one-off scripts where the common path is now better.

That means export should consider standardizing on:

- shared `run_all_tests_common.pl`
- shared compare helpers where appropriate
- shared batch-worker support where that improves iteration time

### 3. Wrapper Support

For late hosted/toolchain assignments, the exported repo should carry forward
the public wrapper path instead of forcing students to rediscover it.

Primary candidate:

- [scripts/cppgm-cmake-wrapper.sh](../scripts/cppgm-cmake-wrapper.sh)

This should be exported when the relevant PA README or tests expect that
workflow.

## Export Inventory Template

Each PA should get an export inventory entry with at least these fields:

- `pa`
- `binary`
- `student-editable files`
- `public support files`
- `public tests`
- `public refs`
- `public grammar/spec files`
- `reference binary/binaries`
- `shared scripts required`
- `wrapper/runtime helpers required`
- `internal-only files omitted`
- `special packaging notes`
- `export mode` for stable early PAs:
  - preserve as-is
  - preserve with critical fix
  - preserve with shared infrastructure update

## Export Transformation Rules

### Rule 1: Student-Editable Sources Must Be Explicit

Do not force students to infer what they are expected to change from the repo
layout.

The export manifest should explicitly identify:

- editable source files
- starter headers
- any provided helper stubs

### Rule 2: Public Support Files Must Reduce Accidental Complexity Only

Include helper/support files only when they:

- are necessary for the assignment contract
- make the assignment implementable
- do not solve the core learning objective

### Rule 3: Reference Binaries Must Match The Exported Harness

The exported `*-ref` binaries and helper wrappers must be tested against the
exported tests, not assumed to still work because they worked in the maintainer
repo.

### Rule 4: Exported Scripts Must Be Self-Contained

No exported script should depend on:

- maintainer-only paths
- maintainer-only environment variables
- maintainer-only helper modules not included in the student repo

### Rule 5: Keep Outer Format Stable

Students should see a consistent assignment package shape across the whole
series, even as the internal implementation complexity grows.

That is why the export should continue to resemble the old repo's assignment
layout.

## Validation Checklist For The Final Export

The export is not done until all of the following are true.

### Repo-Level

- exported repo builds from a clean checkout
- exported root `make` works
- exported root `test-report` works, if included
- exported root `README.md` matches the shipped tree
- exported root `AGENTS.md` matches the shipped tree
- exported repo does not reference the maintainer checkout

### PA-Level

- each PA README matches the exported files
- each PA includes the correct starter files
- each PA includes the correct tests and refs
- each PA includes the correct support fixtures
- each PA includes the correct reference binary
- each PA Makefile works in the exported tree

### Infrastructure-Level

- modern Makefile improvements survive export
- shared harness helpers survive export
- wrapper scripts survive export where needed
- no maintainer-only helper silently leaked into the student kit

## Suggested Implementation Strategy

The export process should be implemented as a generated transformation, not a
manual file-copy exercise.

Recommended shape:

1. maintain one export manifest per PA
2. maintain one shared-root export manifest
3. generate the student repo into a separate destination tree
4. validate the exported tree as its own product

That is safer than:

- ad hoc copying
- manual pruning
- one-off `rsync --exclude` lists that are hard to reason about

## Relationship To The Old Repo

`/Users/vishvananda/old/cppgm` is the **format reference**, not the exact file
set to preserve forever.

The export should therefore:

- preserve the recognizable student-facing assignment structure
- preserve the exported-course feel of `pa1` through `pa9`
- preserve `pa1` through `pa9` substantially as they already ship, unless a
  critical fix or clearly approved shared improvement is being applied
- improve the Makefiles, shared helpers, and wrappers where the current repo is
  clearly better

The old repo's simple `export_pa.sh` approach is a useful historical baseline,
but not the desired long-term export mechanism for the full PA1-PA37 series.

## Completion Criteria

This export process is complete when:

- every cleaned PA from `pa1` through `pa37` has an export inventory entry
- the student repo can be generated reproducibly
- the exported repo validates independently
- the exported repo matches the old course format at the outer layer
- the exported repo carries forward the approved modern infrastructure
- the exported repo does not ship the full maintainer implementation source
