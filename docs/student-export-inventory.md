# Student Export Inventory

This is the working manifest for the student-facing export. It starts as an
inventory and decision log; later export tooling should consume a stricter
machine-readable form.

The export policy is defined in `docs/student-assignment-export-process.md`.
The README/scaffold worker processes are defined in
`docs/implemented/v3/student-export-readme-scaffold-subagent-plan.md` and
`docs/implemented/v3/student-scaffold-audit-subagent-plan.md`.

The canonical PA handouts are the existing `paN/README.md` files. Export prep
edits those files in place; it does not create duplicate export-only README
copies.

## Shared Export Candidates

| Area | Candidate files | Notes |
| --- | --- | --- |
| Root docs | `docs/student-export-root/*.md` -> root `*.md`; `docs/student-export-root/LICENSE` -> root `LICENSE`; `docs/student-export-root/AUTHORS` -> root `AUTHORS`; `docs/student-export-root/NOTICE` -> root `NOTICE` | Copy-ready student-facing root docs, including README, license, authors, notice, agent instructions, layout, and testing/reference guides. |
| Root build | `Makefile` | Export a student-facing subset with modern GNU make detection, propagated `CXX`, host compiler defaults, and useful test targets. |
| Starter build | `dev/Makefile` | Export only targets needed to build student tools and reference/harness helpers. |
| Shared test helpers | `scripts/run_all_tests_common.pl`, `scripts/CppgmBatchWorker.pm` | Public if used by exported PA harnesses. Must be self-contained. |
| Hosted wrapper | `scripts/cppgm-cmake-wrapper.sh` | Public only for late hosted/toolchain assignments that document the wrapper workflow. |
| Scaffold support headers | `dev/src/IPPTokenStream.h`, `dev/src/DebugPPTokenStream.h`, `dev/src/abi_mangle.h`, `dev/src/ir_symbol_model.h`, `dev/src/lowir_model.h`, `dev/src/mir_model.h`, `dev/src/x86_register_model.h`, `dev/src/exceptions.h`, `dev/src/tool_help_text.h` | Required by the exported starter scaffold sidecars and optional typed IR/fact scaffolds. |

## Initial Starter Scaffold Map

This table records the current scaffold sidecars. Worker slices should refine it
into per-PA inventory entries and mark unresolved decisions explicitly.

| Assignment(s) | Exported binary / staged target | Student-editable entrypoint or source set | Scaffold candidate | Current status |
| --- | --- | --- | --- | --- |
| PA1 | `pptoken` | `dev/pptoken.cpp` | `dev/pptoken-scaffold.cpp` | Scaffold exists. |
| PA2 | `posttoken` | `dev/posttoken.cpp` | `dev/posttoken-scaffold.cpp` | Scaffold exists. |
| PA3 | `ctrlexpr` | `dev/ctrlexpr.cpp` | `dev/ctrlexpr-scaffold.cpp` | Scaffold exists. |
| PA4 | `macro` | `dev/macro.cpp` | `dev/macro-scaffold.cpp` | Scaffold exists. |
| PA5 | `preproc` | `dev/preproc.cpp` | `dev/preproc-scaffold.cpp` | Scaffold exists. |
| PA6 | `recog` | `dev/recog.cpp` | `dev/recog-scaffold.cpp` | Scaffold exists. |
| PA7 | `nsdecl` | `dev/nsdecl.cpp` | `dev/nsdecl-scaffold.cpp` | Scaffold exists. |
| PA8 | `nsinit` | `dev/nsinit.cpp` | `dev/nsinit-scaffold.cpp` | Scaffold exists. |
| PA9 | `cy86` | `dev/cy86.cpp` | `dev/cy86-scaffold.cpp` | Scaffold exists. |
| PA10-PA12 | `cppgm++` | `dev/cppgm++.cpp` | `dev/cppgm++-scaffold.cpp` | Scaffold exists; README must explain staged `--emit-ast`, `--emit-types`, and `--emit-semantics` work. |
| PA13 | `lowir2cy86` | `dev/lowir2cy86.cpp` | `dev/lowir2cy86-scaffold.cpp` | Scaffold exists; optional typed LowIR model header is exported under `dev/src`. |
| PA14 | `abimangle` | `dev/abimangle.cpp` | `dev/abimangle-scaffold.cpp` | Scaffold exists; typed ABI fact header is exported under `dev/src`. |
| PA15-PA28, PA30-PA36 | `cppgm++` | `dev/cppgm++.cpp` | `dev/cppgm++-scaffold.cpp` | Same scaffold candidate; workers must decide how exported cumulative PA docs describe extending it. |
| PA29 | `lowir2native` | `dev/lowir2native.cpp` | `dev/lowir2native-scaffold.cpp` | Scaffold exists; optional typed MIR model header is exported under `dev/src`. |
| PA37 | `lowiropt` | `dev/lowiropt.cpp` | `dev/lowiropt-scaffold.cpp` | Scaffold exists. |
| PA38 | `lowir2native` | `dev/lowir2native.cpp` | `dev/lowir2native-scaffold.cpp` | Reuses the PA29 backend entrypoint; export docs must explain the machine/backend optimization extension. |
| PA39 | self-host ladder | multiple `dev/` tools and source sets | not a binary scaffold | Special export case. Needs an initial inventory audit, then a focused README rewrite around staged self-host validation rather than a single editable binary. |

## Per-PA Inventory Schema

Each PA section should eventually include:

- exported binary or binaries, or staged targets for PA39-style assignments
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

## Open Export Decisions

- Decide whether the student repo is exported as one cumulative working tree,
  one starter tree per assignment, or a cumulative tree with per-PA checkpoints.
  This affects how the shared `cppgm++` scaffold is described for PA10-PA27,
  PA29, and PA31-PA36.
- Decide whether PA10-PA36 should ship checked-in refs only or also reference
  binaries. Current README wording treats checked-in refs as the oracle for
  PA10+.
- Decide the exact `dev/src` support-file set exported with the cumulative
  `cppgm++`, `lowir2cy86`, and native-tool scaffolds.
- Decide whether witness output is a public student oracle for
  PA19/PA20/PA22/PA23/PA24 or a maintainer-only strict validation surface.
- Decide how to expose loose LowIR validation in the student repo while keeping
  maintainer strict text comparison internal.
- Decide how to expose structural MIR validation for PA28 and PA38 while
  keeping maintainer direct `.ref.mir` comparison internal.
- Decide whether PA13 `tests/debuginfo/` and scripts that invoke later tools are
  omitted or packaged as instructor-only extras.
- Decide whether PA34/PA35 hosted-header sweep tooling is omitted entirely or
  exported as optional instructor tooling.
- Decide which PA39 checkpoint source sets are student-facing package inputs and
  which generated/intermediate artifacts are omitted from export.

## Resolved Cleanup Notes

- PA14-PA19 and PA23-PA29 Makefile/script grammar references now
  point at the matching local `paN.gram` file.
- Stale nested PA17 copies under `pa17/grammar/grammar/` and
  `pa17/scripts/scripts/` were removed.
- The stale PA1-PA9 and old backend `scripts/export_pa.sh` files were removed.
- The `cppgm++` scaffold now assigns compile/link driver mode to PA29.
- The PA34 hosted test no longer pins a local Homebrew GCC path, and the PA35
  ostringstream smoke no longer contains an absolute local source path.
- PA39 README has been rewritten as a student-facing staged self-host handout.

## PA Sections

Workers should fill these sections in assigned slices.

### PA1-PA9

Export mode: preserve existing student packages unless applying a critical fix
or approved shared infrastructure update.

### PA10-PA13

- PA10 exports `cppgm++ --emit-ast`; students edit `dev/cppgm++.cpp` from
  `dev/cppgm++-scaffold.cpp`; support files are `pa10.gram`, `grammar/`,
  `tests/spec`, `tests/general`, checked-in refs, and PA/shared harness scripts.
- PA11 exports `cppgm++ --emit-types`; same `cppgm++` scaffold and support shape
  with `pa11.gram`, `grammar/`, `tests/spec`, and `tests/general`.
- PA12 exports `cppgm++ --emit-semantics`; same scaffold and support shape with
  `pa12.gram`, `grammar/`, `tests/spec`, and `tests/general`.
- PA13 exports `lowir2cy86`; students edit `dev/lowir2cy86.cpp` from
  `dev/lowir2cy86-scaffold.cpp`; support files are `pa13.gram`, `lowir.md`,
  `grammar/`, `tests/spec`, checked-in refs, and PA/shared harness scripts.
- Remaining decisions: PA10-PA13 reference binary policy, exact `dev/src`
  support set, and PA13 debuginfo/instructor-extra packaging.

### PA14-PA18

- PA14 exports standalone ABI name construction for `abimangle`; students edit
  `dev/abimangle.cpp` from `dev/abimangle-scaffold.cpp`; support files include
  the typed `dev/src/abi_mangle.h` fact scaffold, `pa14/Makefile`,
  `pa14/scripts`, and `pa14/tests/abi`.
- PA15-PA18 export cumulative `cppgm++ --emit-lowir -O0`; students edit
  `dev/cppgm++.cpp` from `dev/cppgm++-scaffold.cpp`.
- Support files are each PA's `paN.gram`, `grammar/`, checked-in refs,
  `tests/general`, `tests/spec` where present, local support headers, and
  PA/shared harness scripts.
- README wording now treats checked-in grammars and HTML grammar explorers as
  reference artifacts in the same style as PA6-PA9, without exposing grammar
  regeneration helpers.
- Remaining decisions: loose LowIR validator packaging and cumulative
  scaffold/checkpoint policy for the shared `cppgm++` binary.

### PA19-PA24

- PA19-PA24 export cumulative `cppgm++ --emit-lowir -O0`; students edit
  `dev/cppgm++.cpp` from `dev/cppgm++-scaffold.cpp`.
- PA19 and PA20 ship `pa19.gram`/`pa20.gram`, `grammar/`, tests, refs, and local
  support headers. PA21-PA23 inherit the PA20 syntax boundary and ship tests,
  refs, and support headers without new grammar files. PA24 uses the shared
  source grammar and ships template-integration tests and refs.
- README wording describes loose LowIR validation for students and leaves witness
  sidecars out of the required oracle unless export policy changes.
- Remaining decisions: witness/test-strict packaging, course symlink behavior,
  and loose LowIR validator configuration.

### PA25-PA31

- PA25-PA28 export cumulative `cppgm++ --emit-lowir -O0`; students edit
  `dev/cppgm++.cpp` from `dev/cppgm++-scaffold.cpp`; support files include each
  PA's grammar/explorer where present, tests, refs, and harness scripts.
- PA29 exports `lowir2native`; students edit `dev/lowir2native.cpp` from
  `dev/lowir2native-scaffold.cpp`; support files include `pa29.gram`,
  `grammar/`, `../pa13/lowir.md`, the optional `dev/src/ir_symbol_model.h`,
  `dev/src/lowir_model.h`, `dev/src/mir_model.h`, and
  `dev/src/x86_register_model.h` typed IR scaffolds, `tests/strict`,
  `tests/structural`,
  refs, and harness scripts.
- PA30 exports cumulative `cppgm++` compile/link driver mode; students edit
  `dev/cppgm++.cpp` from `dev/cppgm++-scaffold.cpp`; support files include
  `pa30.gram`, `grammar/`, `tests/general`, refs, wrapper/runtime helpers, and
  harness scripts.
- PA31 exports `cppgm++ -c` host-EH facts; students edit `dev/cppgm++.cpp` from
  `dev/cppgm++-scaffold.cpp`; support files include `tests/general`, refs,
  host object/symbol inspection helpers, and normalized host-EH fact dumping.
- README wording assumes x86_64 Linux for the student-facing native target.
- Remaining decisions: loose LowIR validator packaging.

### PA32-PA36

- PA32 exports host-linkable object interoperability for `cppgm++ -c`; support
  files include `tests/general`, refs, harness scripts, host compiler helpers,
  and Linux tools such as `ar`, `nm`, and `readelf`.
- PA33 exports host C++ ABI/runtime behavior after host link; support files
  include `tests/general`, refs, host object/symbol inspection helpers, and
  richer EH/runtime interaction tests beyond the basic PA31 host-EH facts.
- PA34 exports hosted preprocess/compile compatibility; support files include
  `tests/preproc`, `tests/compile`, hosted compile scripts, and portability
  reference checks.
- PA35 exports heavy hosted-header compile compatibility; support files include
  `tests/compile`, hosted compile scripts, checked output sidecars, and
  hosted-header anchors that require real header compilation.
- PA36 exports hosted header-emission link/runtime compatibility; support files
  include `tests/link`, symbol comparison helpers, object inspection helpers, and
  `scripts/cppgm-cmake-wrapper.sh`.
- README wording assumes x86_64 Linux for the student-facing host/toolchain
  environment.
- Remaining decisions: PA34/PA35 hosted sweep omission and reference binary
  policy.

### PA37-PA38

- PA37 exports `lowiropt`; students edit `dev/lowiropt.cpp` from
  `dev/lowiropt-scaffold.cpp` plus shared optimizer/driver support under
  `dev/src`; support files include `pa37/Makefile`, `pa37/scripts`, and
  `tests/{o0,o1,o2,driver,debuginfo}` with checked-in refs. It also exports
  `tests/object-roundtrip`, which validates that `cppgm++ -c` object emission
  can be reconstructed from serialized LowIR using standalone `.cpp` probes and
  symlinked harness `.t` cases.
- PA38 exports `lowir2native -O1/-O2`; students edit `dev/lowir2native.cpp`
  from `dev/lowir2native-scaffold.cpp` plus shared machine/backend optimizer
  support under `dev/src`; support files include `pa38/Makefile`, `pa38/scripts`,
  `scripts/run_lowir_native_tests_worker.pl`, and
  `tests/{o1,o2,debuginfo}` sidecars. The optional MIR scaffolding uses
  `dev/src/mir_model.h` and `dev/src/x86_register_model.h`. The exported README
  should describe this as shared backend optimization work that remains reusable
  by later `cppgm++` object and link-driver modes.
- Remaining decisions: loose LowIR validator configuration for PA37 and
  structural MIR validator configuration for PA38.

### PA39

Special self-host export case. PA39 has no single binary scaffold. It exports a
staged self-host ladder over checkpoint tools:

- `pptoken`, `posttoken`, `ctrlexpr`, `macro`, `preproc`, `recog`, `nsdecl`,
  `nsinit`, `cy86`
- `cppgm++`, `lowiropt`, `lowir2cy86`, `lowir2native`

Source sets derive from `dev/frontend_source_sets.mk`, and generated checkpoint
binaries live under the assignment object root selected by the PA39 Makefile.

Remaining decisions: source-set packaging details and which generated,
intermediate, or extra comparison artifacts are omitted from export.
