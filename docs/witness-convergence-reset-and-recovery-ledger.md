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
| 0. Preserve and restore | `afcdb6ae29c4` | 1,339/1,530 expanded; 1,305/1,305 tracked; broad 4,862/4,862 | 1,529 traces; exact ordinary parity | 175,251,868,297 instructions | complete |
| 1. Expanded ownership evidence | `0f69cf8011d5` | 1,339/1,530 expanded; broad 4,862/4,862 | 1,529 traces; 63,229 records; zero unknown routes | 175,152,378,823 instructions | complete |
| 2a. Concrete typedef deferral probe | uncommitted | 1,322/1,530 expanded; broad 4,722/4,862 | 1,511 traces; 17 new witness failures | benchmark does not compile | rejected |
| 2. Class materialization | `6196b6a2020d` | 1,339/1,530 expanded; broad 4,862/4,862 | 1,529 traces; 63,235 records; exact ordinary parity | 175,624,602,849 instructions | lookup modes complete; declaration indexing pending |
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
- [x] Clean Phase 0 commit (`afcdb6ae29c402895cac2712bc763f2e964c83b3`)

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

## Phase 1: expanded ownership evidence

The durable evidence summary is
`docs/evidence/witness-convergence-phase1-20260808/README.md`. Phase 1 adds
stable diagnostic route IDs at the true class, alias, function, variable, and
lifecycle semantic operations, detailed source binding and lifecycle context,
and an occurrence-level strict mismatch analyzer.

All five final producers and all twelve typed upstream routes are exercised.
There are no unknown producer attempts or unknown routes, and no reduced test
was needed merely to exercise instrumentation. The 1,529 traces contain
63,229 records; the same PA23 compiler-exit case remains the only test without
a flushed trace.

Ordinary and diagnostic strict output are byte-for-byte identical. Both have
the Phase 0 strict-log hash and 3,060-output manifest hash. The ordinary binary
contains no provenance symbol and has exactly the Phase 0 file and Mach-O
section sizes. Static materialization and text-reparse audits are clean, 23
analyzer/audit unit tests pass, and the direct-LowIR broad report passes
4,862/4,862.

The 191 remaining mismatching outputs classify as follows. Tests may appear
in more than one family.

| Family | Failing tests | Changed | Missing expected | Unexpected actual | Ordering only |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias | 24 | 18 | 16 | 2 | 0 |
| Class | 62 | 57 | 36 | 10 | 0 |
| Function | 86 | 40 | 62 | 35 | 1 |
| Variable | 3 | 1 | 0 | 2 | 0 |
| Lifecycle | 79 | 0 | 158 | 127 | 0 |

The key semantic-work findings are:

- alias completion has 821 first completions, 1,050 ignored repeats, 95
  class-context upgrades, and four current-specialization enrichments;
- class analysis has 3,270 early repeats and 336 prepublication merges; 15
  missing expected class rows correlate with 49 repeated rejected typed
  materialization decisions, while 21 have no class/materialization attempt;
- the sole final class duplicate is one nested source-owned occurrence in the
  PA22 sibling-namespace fixture;
- function publication has 177 overload-resolution duplicates, 91 `declval`
  duplicates, and four conversion duplicates;
- variable publication is 31 direct and three initializer-replay attempts;
- lifecycle mismatches are missing/extra entity transitions and remain
  independent of source-row admission.

Phase 1 performance passes against the fixed checkpoint: -0.06%
instructions, +0.49% maximum RSS, and -0.03% peak footprint. No RSS rerun is
triggered. Phase 2 begins with repeated rejected class source IDs, then repairs
the class occurrences with no semantic attempt.

## Phase 2: class materialization ownership investigation

Phase 2 first separates three facts that the original probe conflated:

1. the containing semantic owner (`ClassInfo`, `FunctionBinding`, static
   `ValueBinding`, or `VariableTemplateDecl`);
2. the nested parser operation currently resolving a type node; and
3. exact containment of the stable `TemplateIdSyntax::source_location_id` in
   both the operation node and its semantic owner.

The diagnostic compiler uses isolated object root
`../obj/witness-recovery-phase2-owner-state-20260809` and Homebrew Clang. The
ordinary output remains exactly at the Phase 1 checkpoint:

- strict: 1,339/1,530 passing with the same 191 gaps;
- per PA failures: 11, 10, 49, 83, and 38;
- 3,060-output manifest SHA-256:
  `3ff5c80c9a64f0b4bec8273ce678e0fd83c252e4c77aee66eafb7190bffb2d2b`;
- diagnostic strict-log SHA-256:
  `cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`;
- analyzer tests: 11/11 passing.

The final owner-state corpus has 1,529 traces and 63,235 records. Its report is
`/tmp/cppgm-phase2-owner-state-full-r2-report.json`, SHA-256
`b178e7faa7988efa435ef72cfd1270e5eb0dcb08539da33881d8ecd71ebef15b`.
The exact-owner convergence report is
`/tmp/cppgm-phase2-owner-state-full-r3-convergence.json`, SHA-256
`da7229768c7c448f6c1005f272df09d406abfce164b5cfb6ce4a1e5f3498101b`.

### What the owner probe disproves

An active committed owner is necessary but not a materialization fact. Exact
source containment plus matching owner kind still finds 377 occurrences:
only 18 exist in patched-Clang output and 359 do not. Therefore no combination
of owner kind, collection state, output state, or source mode can be the final
admission rule.

The declaration results are especially decisive. All 771 rejected decisions
and all five accepted decisions occur while a tracked concrete class-template
specialization is incomplete and performing full member collection. The
state is identical. The distinction is demand:

- `impl<int>` eagerly resolves the unused member typedef target `slot<int>`;
  patched Clang has no concrete source `TypeLoc` there;
- the member function declaration must instantiate `lock<mutex_type>`, and
  patched Clang does have that source `TypeLoc`;
- the `graph<...>` typedef target becomes materialized only when a later
  qualified lookup demands the alias.

This is unnecessary semantic work, not a witness visibility policy. Phase 2
must split class declaration indexing from member-alias target resolution and
defer a concrete typedef/alias target until semantic lookup demands it.

Function ownership has a second structural split. No candidate owned by a
`FunctionBinding` with `source_template != nullptr` appears in patched-Clang
output. Valid dependent body materializations are ordinary source bodies or
non-template members of instantiated class templates. The current whole-body
scope is therefore too broad; the typed result must originate at the source
type operation in a body that the reference AST actually materializes, not at
CPPGM code-generation or SFINAE analysis of a function-template body.

The static-member positive is not yet owned correctly. Its later constant
lookup carries `StaticMemberInitializer` as an operation but inherits
`main`'s `FunctionBinding` as semantic owner. The real initializer pass owns a
`ValueBinding`; it must retain the stable occurrence result for the later
lookup. The audit now rejects the inherited-main case instead of treating
source mode as ownership. The variable-template initializer already has an
exact `VariableTemplateDecl` owner.

### Missing class rows are not one problem

Of the 36 missing expected class rows, only four currently reach an exact,
matching materialization operation: `lock`, `defaults`, `graph`, and `find`.
The other 32 belong to separate semantic boundaries:

- out-of-class/current-specialization owner source;
- direct qualified member-template source;
- alias-expanded result source;
- CTAD/deduced variable source;
- nested-name-specifier source in a materialized variable initializer; or
- a direct source occurrence for which CPPGM never creates a class-use
  attempt.

They must be repaired at those producers after the materialization branch is
narrowed. Treating all 36 as replay admission would recreate the late policy
the consolidation is meant to remove.

### Next implementation slice

1. Index concrete class member typedefs and aliases once without resolving
   their targets. Extend the existing `DeferredMemberAlias` side map with a
   structured declaration handle; do not grow `Type`, `TemplateArgument`, or
   `ClassInfo`.
2. Resolve and bind that handle once from `lookup_member_type`, carrying the
   exact source occurrence and class owner through the demand operation.
3. Prove the unused `slot<T>` work disappears while demanded `graph<...>` and
   member signatures such as `lock<...>` remain.
4. Move function-body materialization from a whole-body scope to the concrete
   source-type operation and exclude function-template-instantiation analysis
   structurally, not with names, locations, source modes, or renderer rules.
5. Make static initializer analysis retain its exact `ValueBinding`-owned
   result; remove the later constant-lookup admission.
6. Only after these paths are correct, repair the 32 non-materialization rows
   at their actual producer boundaries and delete the old late class policies.

No performance gate is recorded for this diagnostic-only investigation. The
next correctness-clean semantic slice receives the required three-run gate.

## Rejected Phase 2 concrete-typedef deferral probe

The uncommitted probe adds structured typedef declaration pointers to
`ClassInfo::DeferredMemberAlias`, indexes typedef names during concrete class
collection, and resolves the target when `lookup_member_type` requests it.
The diff adds 152 lines and removes eight across two files. Its patch SHA-256
is `f40f0f9420534a5740d9e288b30acad4481252450c5dba51d2065cc8d5c92879`.
Both ordinary and provenance builds complete with Homebrew Clang.

### Correctness result

The fresh strict run compares all 1,530 references:

- 1,322 pass and 208 witness comparisons fail;
- none of the 191 restart gaps close;
- 17 new witness failures appear;
- 14 other tests fail direct LowIR while their witness comparison does not;
- 222 distinct strict tests fail across witness, LowIR, or compiler status;
- 1,511 provenance traces flush, down from 1,529.

The 17 new witness failures have manifest SHA-256
`209a47ed3a64d965e456ed10379564adb5c9c47aebd25e1b437e1d2fbc470729`.
The 14 LowIR-only failures have manifest SHA-256
`a491032c2382b04b31200eb1cd62fbe0a2b5e38ea2e6a698b6c525ccfa94426c`.
The strict log SHA-256 is
`eac2a4c0d5a926e4f158a77b67f03f18721ba68edcd9ce9b878f2c66315bb2a2`.

The broad run passes 4,722 of 4,862 tests. Its 140 failures include 41 in
PA35, 51 in PA36, and four PA37 object-roundtrip cases. The remaining unique
failures occur in PA19, PA22 through PA25, PA27, PA32, and PA34. The broad log
SHA-256 is
`00d98b030329bf22c232755b54c19ce9e7f0e9295fa867e8a173c1ad819ee766`.

### Performance result

No candidate report exists. The frozen performance workload exits during run
one while resolving a libc++ `unordered_set` dependency. The partial process
retires 35,689,074,472 instructions before failing, which cannot serve as a
candidate sample. The last valid result remains the Phase 1 gate: -0.06%
instructions, +0.49% RSS, and -0.03% footprint against the fixed baseline.

The performance script uses defaults that differ from the governing policy.
`check` uses one run, 1% instructions, 3% RSS, and 3% footprint. Phase 2 must
change those defaults to three runs, 0.5%, 3%, and 1% before it records another
candidate.

### Semantic diagnosis and disposition

The probe returns from `collect_class_simple_declaration` after it indexes the
typedef name. That return skips embedded declarations. The reduced
`typedef enum { white, black } color_type;` case then loses its enumerators.

The probe also assumes every consumer reaches `lookup_member_type` before it
uses the placeholder. Many class, template, overload, and hosted-library paths
fetch entries from `Scope::named_types`. Those consumers receive the unresolved
placeholder and fail in SFINAE, default argument evaluation, member lookup,
LowIR generation, or hosted compilation.

The class and alias work still benefits from demand-driven target resolution,
but the compiler needs one member-type lookup boundary first. Preserve this
probe as evidence, remove it from the working tree, and restart from
`b03f2530dad6513aabfa1064a8919bb61fea7d3f`.

Static audits remain clean. The materialization audit reports no findings, the
text-reparse audit exits cleanly, and 25 provenance, convergence, and
performance-gate unit tests pass.

## Phase 2 restart after the rejected probe

The rejected typedef-deferral patch has been removed. The production semantic
sources match `b03f2530dad6513aabfa1064a8919bb61fea7d3f`, while the recovery
plan, owner-state diagnostics, and performance-policy correction remain on the
active branch. Commit `8596cb9567b04fff5d883345c281a23050e08d18` changes the
performance gate defaults to the governing policy: three runs, a 0.5%
instruction limit, a 1% footprint limit, and a 3% RSS warning followed by a
second complete three-run confirmation. Twenty-six analyzer and gate tests
pass.

### Fresh ordinary checkpoint

The ordinary compiler was rebuilt with Homebrew Clang 22.1.0 and isolated
object root `../obj/witness-recovery-restart-ordinary-20260809`. The build is
warning-free. Its binary SHA-256 is
`3d21ee5f596c130674c6615b3c6b7a797b24ea8b48e54be00eb45a116d2a2715`;
the file is 17,005,288 bytes. Mach-O `__TEXT` is 12,951,552 bytes, `__text`
is 11,791,785 bytes, `__DATA_CONST` is 57,344 bytes, and `__DATA` is 442,368
bytes. The binary contains no `witness_provenance` symbol.

The expanded strict result is unchanged:

- PA19: 268/279, with 11 witness mismatches;
- PA20: 148/158, with ten witness mismatches;
- PA22: 244/293, with 49 witness mismatches;
- PA23: 302/385, with 83 witness mismatches;
- PA24: 377/415, with 38 witness mismatches;
- total: 1,339/1,530, with the exact prior 191-gap manifest and no direct
  LowIR mismatch.

The strict log SHA-256 is
`cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`.
The failure-manifest SHA-256 remains
`aba47657850ef74f99513089ee89dec00d8c794427a995a7669dcc050ae447d4`.
The fresh relative-path manifest of all 3,060 witness and LowIR outputs has
SHA-256
`e5add38dada683f43e3b06cde738ded2d56874e9a28e99bd514bef761894a429`.

The PA1-PA38 direct-LowIR report passes 4,862/4,862. Its log SHA-256 is
`17690eaaac57ef327c06a8a927d8a61727d1ed26063af215007650646e338ccc`.
The materialization audit has no findings and retains its two documented
decision boundaries. All 23 forbidden text-reparse categories are zero.

### Fresh diagnostic parity

The diagnostic compiler uses object root
`../obj/witness-recovery-restart-provenance-20260809`. Its binary SHA-256 is
`64757349be1083b2926ba59395c5dd32d242a155d3ea58c22b87f7468b2e7a07`;
the file is 17,171,920 bytes. Mach-O `__TEXT` is 13,086,720 bytes, `__text`
is 11,913,049 bytes, `__DATA_CONST` is 57,344 bytes, and `__DATA` is 446,464
bytes.

The strict run writes 1,529 traces and reproduces the ordinary per-PA counts,
strict-log hash, and 191-gap set. Its 3,060-output manifest is byte-for-byte
identical to the ordinary manifest. The only missing trace remains
`pa23/tests/spec/300-nondeduced-partial-pattern-recursive-completion.t`, whose
compiler exit prevents session finalization.

- trace directory: `/tmp/cppgm-recovery-restart-provenance.WQ9LVg`;
- relative trace-manifest SHA-256:
  `9c3a308c7a73039670ef16aa8597ca8f3643aa91486a24e8b220c977e40021dd`;
- 63,235-record report SHA-256:
  `c2a4f965b29197af2b490cbac32f1ec51517feaee6ea2e77ce14dfcd3cc447d7`;
- convergence report SHA-256:
  `93ba1341d5ad862a35c7fd685012e1e4146ed6658cd49da94e0327149387055a`;
- unknown producer attempts: zero;
- unexercised diagnostic sites: zero.

The fresh convergence report still contains 191 mismatching outputs. Its
current analyzer classification is 24 alias tests, 61 class tests, 86 function
tests, three variable tests, and 79 lifecycle tests. These categories overlap.
The class family contains 57 changed rows, 36 missing rows, and eight extra
rows; alias contains 18 changed, 16 missing, and two extra rows. The ordinary
compiler was restored after the diagnostic run and its SHA-256 was reverified.

### Rolling performance checkpoint

The post-diagnostic three-run artifact is
`/tmp/cppgm-witness-recovery-restart-rolling.json`, SHA-256
`a3d44051f474267dea57d399657b4f25e8f9428540bbf34887044eeae32cdd9c`.
It records commit `8596cb9567b04fff5d883345c281a23050e08d18` against workload epoch
`9764b3835e3c6996b6b80803054f80e1cf50f98e`.

| Metric | Minimum | Median | Maximum |
| --- | ---: | ---: | ---: |
| Instructions | 175,434,108,616 | 175,579,014,888 | 175,818,099,215 |
| Maximum RSS | 750,571,520 | 757,129,216 | 759,046,144 |
| Peak footprint | 575,901,696 | 576,348,160 | 576,438,272 |

Against `/tmp/cppgm-class-materialization-ownership-fixed.json`, the median is
+0.19% instructions, +1.25% maximum RSS, and +0.09% peak footprint. Every
check passes and the RSS result does not require confirmation. The comparison
report SHA-256 is
`f8d966aeeb8edb62602631b706630e4b5086126204bbebd6c7d01f4cb2da159f`.
This artifact is the rolling baseline for the member-type lookup migration;
the Phase 0 fixed artifact remains the final instruction-reduction reference.

### Restart disposition

The next semantic change is not another deferral attempt. First, classify all
direct reads of class-member `Scope::named_types`. Remove duplicate direct-map
fast paths, inherited traversals, and post-lookup fallbacks from consumers
that resolve a member type. Registration, declaration indexing, template
parameter binding, state reset, and diagnostic iteration remain raw storage
operations. Only after strict, broad, provenance, and performance parity is
re-established may concrete typedef and alias targets become lazy.

## Phase 2 member-type lookup consolidation checkpoint

Commit `ee6a5dde60cca46ffed906a5f5e9175dbdbe5dc9` removes seven direct
class-member type-map fast paths from consumers that are asking to resolve a
member. The accepted slice covers known-owner resolution, dependent-member
owner discovery, concrete member-type traits, instantiation-owner recovery,
out-of-class nested-owner traversal, and nested-class refresh after owner
completion. Class targets now enter the canonical completing lookup; namespace
targets retain direct inline-namespace lookup.

This is deliberately narrower than replacing every `Scope::named_types` read.
The rejected broad variants established four distinct non-completing modes:

- generic lexical search must inspect the scopes that are already visible
  without turning a search probe into class completion;
- in-progress class and alias construction must see the bindings collected so
  far without recursively completing the class being built;
- type-equivalence and binding-cache probes must not mutate semantic state;
- inherited-using discovery must inspect declared base members without
  changing the active owner or completion order.

The first broad rewrite introduced 13 strict regressions and fixed none. A
later reduction localized three batch-only regressions to in-progress alias
canonicalization and inherited-using/equivalence/cache probes. Those variants
were discarded; no visibility filter, renderer recovery, text reparse, or
reference change was used to hide the failures. The next code slice must name
the non-completing query modes explicitly before declaration indexing or lazy
alias targets are attempted again.

### Remaining direct class-member map operations

The remaining `member_scope->named_types` operations divide into three groups:

- seven operations are the implementation and access-control guard of the
  canonical lookup itself;
- eight reads are the non-completing modes above, including inherited-using
  discovery, partially collected alias canonicalization, friend/access cached
  equivalence, name hiding, and call-side binding/equivalence probes;
- 16 operations register, index, preserve, reset, bind, or diagnostically
  iterate storage and are not semantic lookup consumers.

This classification is the boundary for the next phase. Raw storage operations
are not migration targets. Non-completing reads become explicit APIs first;
only then can their duplicated traversal or ownership logic be compared and
collapsed.

### Correctness and static evidence

The warning-free ordinary build uses Homebrew Clang and isolated object root
`../obj/witness-recovery-lookup-migration-ordinary-20260809`. Its binary
SHA-256 is
`c1ec07c11edef6cd42376bd8fd36360e27eb512bb9f796cc56a62ca97c5caa10`;
the file is 17,009,648 bytes. Mach-O `__TEXT` is 12,955,648 bytes, `__text`
is 11,792,249 bytes, `__DATA_CONST` is 57,344 bytes, and `__DATA` is 442,368
bytes. It contains no provenance symbol. Relative to the restart ordinary
binary, this intermediate slice adds 4,360 file bytes, 4,096 `__TEXT` bytes,
and 464 `__text` bytes. The cleanup owner is the next explicit non-completing
query API and the eventual deletion of the corresponding duplicate raw
branches.

The full expanded strict run is exactly unchanged:

- PA19: 268/279, with 11 witness mismatches;
- PA20: 148/158, with ten witness mismatches;
- PA22: 244/293, with 49 witness mismatches;
- PA23: 302/385, with 83 witness mismatches;
- PA24: 377/415, with 38 witness mismatches;
- total: 1,339/1,530, with the same 191 gaps and no LowIR drift.

The strict-log SHA-256 is
`cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`;
the failure-manifest SHA-256 is
`aba47657850ef74f99513089ee89dec00d8c794427a995a7669dcc050ae447d4`;
and the 3,060-output manifest SHA-256 is
`e5add38dada683f43e3b06cde738ded2d56874e9a28e99bd514bef761894a429`.
All three are identical to the restart checkpoint.

The PA1-PA38 direct-LowIR report passes 4,862/4,862; its log SHA-256 is
`c4eac0aeff1dfbd6af90ce48e755e56d031c8f1eb8980f380dfe878af6f55758`.
All 23 forbidden text-reparse categories remain zero, and the materialization
audit has no findings. Twenty-nine analyzer and performance-gate unit tests
pass.

### Diagnostic parity

The diagnostic build uses object root
`../obj/witness-recovery-lookup-migration-provenance-20260809`. Its binary
SHA-256 is
`f7b4a25c06c2ad673fea45ae1e575d260f2d047fcc4a334634decbf6680a7661`
and it is 17,172,184 bytes. The strict run writes 1,529 traces, reproduces the
ordinary strict log byte for byte, and produces the same 3,060-output
manifest. The only untraced input remains the one compiler-exit case from the
restart checkpoint.

- trace directory:
  `/tmp/cppgm-recovery-lookup-migration-provenance.aIZX1c`;
- raw relative trace-manifest SHA-256:
  `710297a8ad710c586d709c1cc63af5895bf3ea6b9048529f35766527cfbb9655`;
- 63,235-record report SHA-256:
  `791d32ffe7ff14dbb833cdbef099f241b307a0d82ce86cbd235e076c1a601fac`;
- convergence report SHA-256:
  `d24f908959d20db6355a7b855f07e32802b833fb82e7231c33922d328cc62bc0`;
- unknown producer attempts: zero;
- unexercised diagnostic sites: zero.

Raw diagnostic hashes differ because each report records the fresh trace path
and process-specific filename. After removing that volatile `source` field,
both the provenance report and convergence report are structurally identical
to the restart reports. The semantic counters remain 821 published alias
occurrences from 1,970 completion decisions and 2,573 published class
occurrences from 2,919 completed candidates. The convergence report remains
1,339 matching and 191 mismatching outputs, classified as 24 alias tests, 61
class tests, 86 function tests, three variable tests, and 79 lifecycle tests.

### Performance checkpoint

The three-run comparison report is
`/tmp/cppgm-witness-recovery-lookup-migration-perf.json`, SHA-256
`3c50b2ecde1d4b317b18476b1519840d78255003dcb2236139635d3a499d1a06`.
It uses commit `ee6a5dde60cc` and the unchanged workload epoch.

| Metric | Minimum | Median | Maximum | Rolling delta |
| --- | ---: | ---: | ---: | ---: |
| Instructions | 175,532,287,224 | 175,688,178,308 | 175,753,439,482 | +0.06% |
| Maximum RSS | 745,381,888 | 747,089,920 | 758,132,736 | -1.33% |
| Peak footprint | 575,729,664 | 576,249,856 | 576,479,232 | -0.02% |

The slice passes the 0.5% instruction and 1% footprint limits. RSS is below
the rolling median and does not trigger the 3% confirmation rule. Against the
fixed Phase 0 final reference, the current medians are +0.2490% instructions,
-0.0926% RSS, and +0.0697% footprint. This is acceptable for an intermediate
phase, but it does not satisfy the final instruction-reduction requirement.

## Phase 2 bound-member query checkpoint

Commit `6196b6a2020d9ed42da464c5f6ec9f8c327810ec` introduces one read-only
`find_bound_member_type` boundary and routes all eight non-completing
class-member consumers through it. The query returns a borrowed pointer to the
already-bound `TypePtr`. It does not collect or complete members, traverse
bases, resolve deferred aliases, enforce access, or copy the shared pointer.

The migrated consumers are inherited-using discovery, friend alias
equivalence, partially collected class-alias canonicalization, call-side alias
binding/equivalence probes, and direct name-hiding detection. The only direct
`member_scope->named_types` reads that remain are:

- the read-only query implementation;
- the canonical completing lookup implementation and its access guard;
- base registration, template-parameter binding, nested-type registration,
  storage reset/preservation, and diagnostic iteration.

The first result-struct version was not retained. It copied `TypePtr` on every
query and grew the ordinary binary by 440 bytes over the accepted resolution
slice. Returning a borrowed pointer removes the retain/release work. The final
ordinary binary is only 48 bytes larger than the preceding checkpoint and its
`__text` section is 48 bytes smaller.

### Correctness and size evidence

The warning-free ordinary build uses Homebrew Clang and object root
`../obj/witness-recovery-bound-member-query-ordinary-20260809`. Its SHA-256 is
`2ea5ba6740b9e696a0529730ea11ad21e51fed2361401010951b2273f3a6729d`;
the file is 17,009,696 bytes. Mach-O `__TEXT` is 12,955,648 bytes, `__text`
is 11,792,201 bytes, `__DATA_CONST` is 57,344 bytes, and `__DATA` is 442,368
bytes. It contains no provenance symbol.

All 13 tests that exposed the rejected broad lookup rewrite pass. The complete
strict run is byte-for-byte identical to the restart and resolution-only
checkpoints: 1,339/1,530, the same 191-gap manifest, and no LowIR drift. The
strict-log SHA-256 is
`cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`;
the 3,060-output manifest SHA-256 is
`e5add38dada683f43e3b06cde738ded2d56874e9a28e99bd514bef761894a429`.

The PA1-PA38 report passes 4,862/4,862. Its log SHA-256 is
`17690eaaac57ef327c06a8a927d8a61727d1ed26063af215007650646e338ccc`,
identical to the restart broad report. The materialization audit has no
findings and retains its two decision boundaries; every forbidden text-reparse
category is zero; 32 provenance, convergence, path-normalization, audit, and
performance-gate tests pass.

### Provenance evidence

The diagnostic build uses object root
`../obj/witness-recovery-bound-member-query-provenance-20260809`. Its binary
SHA-256 is
`1a88f2be602070c09a115e84ce6c4d121b3ad7753d211b6f5b5c72a42c1b8d8b`
and it is 17,172,232 bytes. The diagnostic strict log and all 3,060 outputs are
byte-for-byte identical to the ordinary results.

- trace directory:
  `/tmp/cppgm-recovery-bound-member-query-provenance.tAdWKW`;
- trace files: 1,529;
- raw relative trace-manifest SHA-256:
  `7526639bb0ffc4b1a7ba6081587e13aa696aea6771181eb12279b59c6d9f098e`;
- 63,235-record report SHA-256:
  `0c18c5b3dd39891701224d74305131bbeae9e5964fda62b1f3efab24fd3d50ad`;
- convergence report SHA-256:
  `ea1cb160d088e2304d38a517f4916833c0577bb125d91189f1b17f6cf8a9d938`;
- unknown producer attempts: zero;
- unexercised diagnostic sites: zero.

After the volatile trace `source` field is removed, the provenance and
convergence reports are structurally identical to the resolution-only
checkpoint. The 821 alias publications, 2,573 class publications, and all 191
gap classifications are unchanged.

### Performance evidence

The three-run report is
`/tmp/cppgm-witness-recovery-bound-member-query-perf.json`, SHA-256
`b8baa932fbb95cb221140f8f05f4f3b78070a013190722a3e4a342d499feaaaa`.

| Metric | Minimum | Median | Maximum | Rolling delta |
| --- | ---: | ---: | ---: | ---: |
| Instructions | 175,621,294,180 | 175,624,602,849 | 175,714,363,994 | -0.04% |
| Maximum RSS | 742,047,744 | 746,057,728 | 753,078,272 | -0.14% |
| Peak footprint | 575,635,456 | 575,827,968 | 576,163,840 | -0.07% |

Every metric improves against the preceding rolling median, so no RSS
confirmation batch is required. Against the fixed Phase 0 reference, the
medians are +0.2127% instructions, -0.2306% RSS, and -0.0036% footprint. The
new rolling baseline is
`/tmp/cppgm-witness-recovery-bound-member-query-rolling.json`, SHA-256
`e0dd42a347e80a1586eec97915dde949ddcc88342db5fd81db7a3b79da0edd27`.
The final instruction-reduction requirement remains open.

### Disposition

Member-type consumers now have explicit completing and non-completing
boundaries. The next semantic slice can separate declaration indexing from
alias-target evaluation without allowing a dependent placeholder to escape
through an unclassified raw lookup. Nested enum/class declarations, access,
multiple declarators, template-parameter bindings, and storage preservation
remain mandatory declaration-side work during that split.
