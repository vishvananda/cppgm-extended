# Witness Convergence Reset and Recovery Ledger

## Recovery identity

- Governing plan: `docs/witness-convergence-reset-and-recovery-plan.md`
- Worktree: `/Users/vishvananda/cppgm-alias-consolidation`
- Branch: `experiment-witness-alias-path-consolidation`
- Clean control: `5add5290c69be6b76138dfc1f6696915eb0278ae`
- Homebrew compiler: `/usr/local/opt/llvm/bin/clang++`
- Patched-Clang reference compiler:
  `/Users/vishvananda/llvm-project-template-metrics-20260416/build-clang-template-trace/bin/clang++`
- Patched LLVM checkout: `59c5d9c...`
- Expanded strict corpus: 1,530 references (1,305 previously tracked and 225
  regenerated references)

## Phase status

| Phase | Commit | Correctness | Provenance | Performance | Status |
| --- | --- | --- | --- | --- | --- |
| 0. Preserve and restore | checkpoint pending | 1,339/1,530 expanded; 1,305/1,305 tracked; broad 4,862/4,862 | 1,529 traces; exact ordinary parity | 175,251,868,297 instructions | complete |
| 1. Expanded ownership evidence | pending | pending | pending | pending | pending |
| 2. Class materialization | pending | pending | pending | pending | pending |
| 3. Alias convergence | pending | pending | pending | pending | pending |
| 4. Lifecycle ownership | pending | pending | pending | pending | pending |
| 5. Function and variable results | pending | pending | pending | pending | pending |
| 6. Scaffolding deletion | pending | pending | pending | pending | pending |
| 7. Final gates | pending | pending | pending | pending | pending |
| 8. Joint inception | pending | pending | pending | pending | pending |

## Reset audit

The evidence and architectural diagnosis are recorded in the governing plan.
Exact strict failure-set manifests are in
`docs/evidence/witness-convergence-reset-20260808/`.

Audit summary:

- clean control: 1,339/1,530 strict, with all 1,305 tracked references exact;
- dirty experiment: 1,218/1,530 strict;
- dirty experiment fixes 18 expanded-corpus gaps and introduces 139 failures;
- dirty broad report: 4,857/4,862;
- dirty performance confirmation versus fixed: +9.60% instructions, +4.61%
  RSS, and +4.59% footprint.

## Phase 0: preserve the experiment and restore the clean control

### Archive

The complete dirty experiment is preserved as a Git side reference:

- ref: `archive/witness-convergence-dirty-20260808`;
- commit: `7ed7db30a925e7c8ce6c01ace57a448e8fd064e8`;
- parent: `5add5290c69be6b76138dfc1f6696915eb0278ae`;
- new patched-Clang references: 225;
- evidence files: four;
- archived changed/added paths relative to the parent: 285.

The archive was built with an isolated temporary Git index. The active branch
and ordinary index were not moved. `git cat-file` and the branch reference
verify that the commit is readable.

### Restore

After the archive was verified, all dirty tracked changes under `dev/` and
`scripts/` were restored explicitly from `5add5290c...`. The reset notices in
the two active plans, the recovery plan and ledger, the evidence manifests,
and the 225 regenerated references remain in the active worktree.

`pa12/tests/general/200-switch-case-declaration.t` now scopes the declaration
inside the `case 0` arm. This keeps the fixture positive without weakening the
compiler's correct rejection of a later label that bypasses initialization.
The PA15 and PA19 negative fixtures remain unchanged.

### Pending Phase 0 evidence

- [x] Warning-free Homebrew-Clang ordinary build
- [x] Expanded strict 1,339/1,530 with the exact 191 clean-gap manifest
- [x] Tracked subset 1,305/1,305, derived from the expanded failure manifest
- [x] PA1-PA38 4,862/4,862
- [x] Materialization and text-reparse audits clean
- [x] Provenance analyzer unit tests clean
- [x] Ordinary/provenance output parity
- [x] Post-diagnostic three-run fixed baseline
- [ ] Clean Phase 0 commit

### Correctness and static evidence

The warning-free ordinary build uses isolated object root
`../obj/witness-recovery-phase0-20260808`. Its binary is 17,004,368 bytes;
Mach-O `__TEXT` is 12,951,552 bytes, `__text` is 11,788,265 bytes,
`__DATA_CONST` is 57,344 bytes, and `__DATA` is 442,368 bytes. `dev/src`
contains 413,807 lines. The binary SHA-256 is
`24af5251473d91c4cb4a88061f85837aef0a55613b296997a243522af0f6a74e`
and it contains no `witness_provenance` symbol.

The expanded ordinary strict run reproduces the clean-control result exactly:

- PA19: 268/279 passing, 11 failing;
- PA20: 148/158 passing, 10 failing;
- PA22: 244/293 passing, 49 failing;
- PA23: 302/385 passing, 83 failing;
- PA24: 377/415 passing, 38 failing;
- total: 1,339/1,530 passing;
- direct LowIR mismatches: zero;
- tracked-reference failures: zero of 1,305;
- failure-set difference from the reset clean control: zero.

Strict log SHA-256:
`c80cba6b301171fd0c087f7ffba43eddd89c7b70e9f36111550a9df5eff07e27`.
Failure manifest SHA-256:
`aba47657850ef74f99513089ee89dec00d8c794427a995a7669dcc050ae447d4`.
The 3,060 ordinary witness/LowIR output hashes have manifest SHA-256
`3ff5c80c9a64f0b4bec8273ce678e0fd83c252e4c77aee66eafb7190bffb2d2b`.

The full direct-LowIR report passes 4,862/4,862. PA12 passes 166/166 after
the scoped-case fixture correction. The materialization audit reports no
findings, the text-reparse audit reports no findings, and the clean analyzer
unit suite passes four tests.

The broad report SHA-256 is
`e37ef5a5230c7732859909552601ed54443d24aa50254c1ff644c2a59acc7b44`.

### Provenance parity and starting ownership

The diagnostic compiler uses isolated object root
`../obj/witness-recovery-phase0-provenance2`. The strict harness was invoked
through `test-strict-nobuild` so it could not replace the provenance binary
with an ordinary build. After the run, the preserved ordinary binary was
restored and its SHA-256 reverified exactly.

- trace directory: `/tmp/cppgm-recovery-phase0-provenance.bewjZR`;
- trace files: 1,529;
- records: 61,265;
- trace-manifest SHA-256:
  `5d76b5a6901bd6f8662efac65d83647e722b78ebe06bec9799c925729724ae6d`;
- report: `/tmp/cppgm-recovery-phase0-provenance-report.json`;
- report SHA-256:
  `505df6b079f9b63aca95df4d99388759219c8cdd82d8669abf1735f3409d1c98`;
- diagnostic strict-log SHA-256:
  `cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`.

The diagnostic run has the same per-PA counts and the same 191 failures as the
ordinary run. All 3,060 witness/LowIR outputs have exactly the same hashes;
the diagnostic output-manifest SHA-256 is the same
`3ff5c80c9a64f0b4bec8273ce678e0fd83c252e4c77aee66eafb7190bffb2d2b`.

The one test without a trace is
`pa23/tests/spec/300-nondeduced-partial-pattern-recursive-completion.t`; its
witness compilation exits before the session can flush. It remains part of
the 191-gap correctness manifest and must gain a trace when its semantic
failure is repaired.

Starting source-table evidence:

| Family | Attempts | Inserted | Exact duplicate | Surviving table rows | Final visible rows |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias | 821 | 821 | 0 | 821 | 821 |
| Class | 2,573 | 2,572 | 1 | 2,572 | 2,572 |
| Function | 1,298 | 1,026 | 272 | 1,026 | 763 |
| Variable | 34 | 34 | 0 | 34 | 34 |
| Lifecycle | 6,030 | 6,030 | 0 | 6,030 | not represented by the source-row final-visible counter |

Alias consolidation records 1,970 completed candidates, 1,149
prepublication merges, 821 collected occurrences, and 821 publications.
Class consolidation records 2,919 completed candidates, 3,270 early repeats,
336 prepublication merges, 2,583 collected occurrences, and 2,573
publications. These earlier repeats are Phase 1 ownership evidence even where
the final source table is already nearly idempotent.

The expanded corpus records 21,200 concrete class materialization decisions:
21,192 rejected and eight typed admissions. The eight admissions are two each
for declaration type, function body, static-member initializer, and
variable-template initializer. The original five-row count was a property of
the smaller corpus and is not a compiler invariant.

### Fixed performance epoch

The missing fixed artifact now exists:

- artifact: `/tmp/cppgm-class-materialization-ownership-fixed.json`;
- SHA-256:
  `af098bc8cb320e7b5d5ffb183ea5f57b80e52b5c202042731327c5db5f1fa1cc`;
- commit: `5add5290c69be6b76138dfc1f6696915eb0278ae`;
- workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`;
- median instructions: 175,251,868,297;
- median maximum RSS: 747,782,144 bytes;
- median peak footprint: 575,848,448 bytes.

The three instruction samples are 175,685,187,235, 175,241,699,573, and
175,251,868,297. RSS samples are 748,265,472, 747,782,144, and 745,799,680
bytes. Relative to the recreated alias fixed artifact, this checkpoint is
-0.44% instructions, -1.23% RSS, and -2.90% footprint. All advisory checks
pass without an RSS warning.
