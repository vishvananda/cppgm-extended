# Early assignment consolidation implementation

Objective: implement [the consolidation plan](early-assignment-consolidation-plan.md)
in full. Baseline: `70a634a73`. The corrected execution strategy keeps PA13
as the required LowIR introduction, omits PA9, and uses the supplied native
backend for PA13's small behavioral portion. The end state is 34 assignments.
PA13 now delivers `lowir`: a reader/writer and three required construction
exercises. The obsolete translator and PA9's assembler have been retired;
the LowIR introduction, specification, grammar and debug coverage remain.

The content-stage descriptions below use original baseline assignment numbers.
Stage B has been applied: original PA13 is current PA8, and original PA39 is
current PA34. The final-number validation register below uses current paths.
See [the migration map](assignment-numbering-migration-2026-09.md). The
[case inventory](early-assignment-consolidation-inventory.tsv) freezes all 487
old case roots (the original 325 plus all 162 tracked PA13 inputs) and records
their assertions, disposition, destination, and
verification as each intake is completed. A pending row is unfinished work.

## Implementation

| Stage | State | Evidence / remaining work |
| --- | --- | --- |
| A0 inventory and execution boundary | Complete | 487 baseline roots frozen and assigned. The revised LowIR construction path has ordinary/batch and harness evidence below. |
| A1 preprocessing | Complete | One preproc interface and handout; 105 roots after 38 identical duplicates and 4 obsolete dumps retired; 2 mixed inputs reduced; macro driver/scaffold/checkpoint retired; source-set/exception audits and export harness pass |
| A2 parser | Complete | 48 old roots assigned: 26 retain/reduce, 13 covered, 9 retired. Duplicate review leaves 20 new AST roots plus focused extensions to existing cases; 187 PA10 roots total. Parser guide and shared template-id grammar updated; recog removed. Parameter-name, direct-call and unnamed-varargs fixes have focused checks. All 14 affected shared-grammar explorers were refreshed in A5. |
| A3 semantic intake | Complete | All 43 PA7 and 67 PA8 groups assigned. nsdecl/nsinit and their private subsystems retired. PA11 guide uses the source/canonical type distinction. Duplicate review leaves 27 new PA11 roots, 105 total. Useful PA8 observations live in semantic, LowIR and linking suites; no mock image remains. Compiler fixes and current validation are recorded below. |
| A4 keep PA13; omit PA9 | Complete | Required lowir tool, model construction, reader/writer, 106 spec + 3 construction cases; 18 debug roots preserved. Student-generated behavior executes through lowir2native-ref. All 162 old PA13 roots and 20 PA9 roots have dispositions. CY86 translator/assembler clients, private libraries, wrappers, payload mappings and runner modes retired. Native encoding/ELF/fixup lesson integrated into PA29. The proposed PA15 fixture moves remain withdrawn. |
| A5 live dependencies | Complete | Live tool/handout/grammar paths corrected; 14 shared grammar explorers refreshed. Fifteen existing noinline cases moved with companions to PA32 cluster 200; all pass ordinary/batch checks. Placement now distinguishes token inputs, AST syntax, introductory LowIR, and debug preservation from later C++ behavior. All-owner audit passed across 5,432 placed fixtures, with no early-owner or hygiene findings. Six additional existing cases moved to their proper clusters; a memcpy prototype now uses standard decltype(sizeof(0)) instead of a later hosted predefined macro. All seven changed cases pass ordinary/batch checks. The documented PA33 pure/const effect owner is now explicit in the ledger. |
| B numbering | Complete | PA1–PA34 applied; all surviving original PA10–39 subtract five. Original PA13 is PA8 and PA39 is PA34. ppexpr is the renamed PA3 tool. Live paths, numeric audit rules, workflows and inventory destinations are migrated. Historical implementation notes retain original numbering under docs/implemented; the live placement ledger is doc/pa-feature-placement.md. |
| Final completion audit | Complete | All seven criteria have final-state evidence below. The user accepted the completed native wall-time ABBA results on 2026-09-07 and stopped further precision measurements. See [the measurement record](early-assignment-consolidation-abba-validation.md). |

## Final-number validation (2026-09-07)

All commands below run against PA1–PA34, with `g++` as host compiler. Log
paths identify local evidence, not files shipped to students.

| Gate | Result |
| --- | --- |
| Host build | Pass: all nine tools; `/tmp/cppgm-final-numbering-build.log` |
| Full strict report | Pass: 5,825/5,825 with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1`; `/tmp/cppgm-final-numbering-report.log` |
| Debug-info preservation | Pass: `make test-debuginfo-nobuild`; `/tmp/cppgm-final-numbering-debuginfo.log` |
| Backend variants | Pass: PA24, PA32 and PA33 under all configured variants; `/tmp/cppgm-final-numbering-variants.log` |
| Native performance envelopes | Pass: 16 within 10%, one existing policy skip; `/tmp/cppgm-final-numbering-backend-perf.log` |
| Harnesses | Pass: 208 tests, 206 passed and two platform/environment skips; `/tmp/cppgm-final-numbering-harnesses.log` |
| Architecture and file audits | All nine architecture audits pass; file audit passes with 35 warnings; `/tmp/cppgm-final-numbering-audits.log`, `/tmp/cppgm-final-numbering-file-audit.log` |
| Placement across every active assignment | Pass: 5,432 fixtures, zero early-owner or local-hygiene findings; `/tmp/cppgm-final-numbering-placement.log` |
| Reference regeneration | Pass: full ordinary and debug regeneration changes none of 18,244 tracked reference files; `/tmp/cppgm-final-numbering-ref-stability.json`. No exit status changes after numbering. |
| Structure and inventory | Exactly 34 assignment wrappers, nine tools, no missing tracked paths or broken symlinks. All 487 frozen baseline hashes verify and every destination exists. Three obsolete regression aliases were removed; original inspection companions follow their moved tests. `/tmp/cppgm-final-numbering-structure.log` |
| Self-host through PA5, PA8 and PA10 | Pass: cumulative `make -C pa34 test-through-pa10 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++`; `/tmp/cppgm-final-numbering-selfhost.log` |
| Inception | Pass: `make inception` reports `MATCH cppgm++` and matching rebuilt objects; `/tmp/cppgm-final-numbering-inception.log` |
| Independent student export | Pass: 18,244 portable references verified, 6,872 exit statuses, 393 Linux diagnostic examples. Nine-tool bundle and 34 assignment wrappers at `/tmp/cppgm-final-numbering-export`. All 109 PA8 cases pass through actual downloaded wrappers in ordinary/batch modes, including grouped input and mixed check/ref routes, before any student native binary exists. All nine scaffolds build and assignment Markdown links resolve. `/tmp/cppgm-final-numbering-export.log`, `/tmp/cppgm-final-numbering-export-smoke.log` |
| Compiler performance | Accepted by the user on 2026-09-07: native wall-time ABBA replaces the unavailable hardware-counter gate for this task. Paired wall changes are −0.060% for the frozen workload (six blocks) and +0.194% for the full 234-object compiler (twelve blocks). All outputs match exactly. The full-build 95% interval is −0.647% to +1.202%; acceptance does not establish a 1% upper bound. Protocol and evidence are in [the measurement record](early-assignment-consolidation-abba-validation.md). |

The final export initially reached reference verification and packaging but
could not create its Git tree because `/tmp` had exhausted its inodes. Removing
the two superseded scratch exports freed space; a fresh full export then passed.
Their validation logs remain available. Neither the source tree nor the
preserved retired-assignment outputs were removed.

## Completion audit

| Plan criterion | Final evidence |
| --- | --- |
| 1. Coherent PA1–PA34 sequence | Roadmap, student layout map, 34 handouts/wrappers, and self-host stage map agree. Original PA13 remains current PA8. |
| 2. No disposable required products | Nine source sets, exported scaffolds and reference payloads exclude macro, recog, nsdecl, nsinit, cy86 and lowir2cy86. The preprocessor, parser, semantic and LowIR handoffs describe reuse; PA24 includes the native encoding lesson. |
| 3. Assertion-level disposition and deduplication | All 487 baseline groups have verified hashes and resolved dispositions: 157 retain, 148 reduce/split, 100 covered elsewhere, 82 retire. Every current destination exists; duplicate and observable-outcome review is recorded in the inventory. |
| 4. Required LowIR introduction and construction | PA8 teaches the reusable model, reader/validator and writer, with 106 roundtrip cases and three required construction exercises. Behavioral grading accepts alternative valid LowIR and checks program outcomes through the supplied backend. No behavioral LowIR/MIR oracle ships; PA24 tests the student native backend. |
| 5. Live paths and discovery | Root reports, per-PA check/ref routes, self-host, export and placement rules use the final numbers. No missing tracked path or broken symlink remains. All live Markdown targets resolve. PA34 runs the cumulative self-host ladder rather than owning duplicate fixture roots. |
| 6. Final validation | Functional, debug, variants, harness, architecture, placement, backend-envelope, self-host, inception and independent export checks pass. All 18,244 references are byte-stable. The user accepted the completed native wall-time ABBA evidence and instructed proceeding without further measurement. |
| 7. Student handoff | Shipped handouts and root documents explain the current sequence, prerequisites, supplied tools, implementation order and reused artifacts directly. Historical implementation notes are archived outside the student assignments. |

## Validation before final numbering

These entries retain the commands and assignment numbers under which the
content migration was tested. Final-number results above supersede their
former pending checks.

| Gate | State |
| --- | --- |
| Historical PA13 execution wiring | Passed: 102 spec + 12 behavior cases in each mode; mixed check 23 + 3; ordinary reference regeneration clean. Superseded by the revised pipeline. |
| PA13 student-produced LowIR behavior, check/ref routes and ordinary/batch parity | 106/106 spec + 3/3 construction pass in ordinary and batch byte-exact modes. All 62 successful canonical spec outputs reparse/write identically. Mixed check/reference selection is exercised by the harness; exported-wrapper and self-host results are recorded above. |
| Later source-to-native feasibility probe | Three existing PA15 programs passed using built cppgm++ and lowir2native with -O0 (exit 0, 1 and 0; empty stdout). This establishes later pipeline feasibility only; those cases remain in place and this is not validation of the revised PA13 exercises. |
| Harness accepts different valid LowIR and rejects wrong/malformed LowIR, failed translation and stale artifacts | Six tests in test_lowir_program_harness.py pass using a real native backend and CLI-only producers. They also cover missing-helper failure, independent reference routing, numeric/format normalization, and preservation of unused declarations. |
| Changed preprocessing/parser/semantic suites and cumulative reports | Early through-PA4 report passed (205/205). Duplicate review: 144 covering roots pass in ordinary and batch modes; after four additional removals, all three changed declaration cases pass both modes. |
| Baseline/candidate performance when compiler behavior changes | Historical environment limitation: baseline command failed because the validator uses macOS `time -lp`; Linux `perf stat -e instructions -- true` reported no supported event. Superseded for this task by the user-requested and accepted native wall-time ABBA validation above; no hardware-counter pass is claimed. |
| Full host build | Passed after LowIR introduction and CY86 retirement: nine surviving tools build. |
| Byte-exact LowIR test report | Complete PA8 intake/nsinit retirement state passed 5862/5862 (`/tmp/cppgm-pa8-retirement-report.log`), including the storage and linker fixes. Subsequent duplicate reductions have focused ordinary/batch passes; old-number final report now passes 5825/5825 (/tmp/cppgm-consolidation-stage-a-report.log). See the final-number register above. |
| Debug-info suites | Old-number make test-debuginfo-nobuild passed, including retained PA13 coverage. |
| Backend variants | Old-number make test-variants passed for PA29, PA37 and PA38. |
| Harness tests | All 204 discovered tests completed: 202 passed, 2 platform/environment skips (macOS Mach-O, unavailable Docker image). All 45 placement harness tests pass after the classification and owner updates. |
| Nine architecture audits, file audit, placement audit at all affected owners | All nine architecture audits pass after CY86 retirement, with 9 tools and 236 production sources. File audit passes with 35 warnings. All-active-owner placement audit passes after correcting fixture placement and respecting the tested input/output contract. See the final-number register above. |
| Supplied native-wrapper selection; early tests independent of a student-built backend | Independent export downloaded the actual nine-tool bundle through its wrappers and passed all 109 PA13 cases in ordinary/batch modes before a student native binary existed. Mixed check/ref routes and all nine scaffold builds also passed. |
| Self-host through AST, retained PA13 LowIR work and first C++ lowering | Isolated make -C pa39 test-through-pa15 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++ passed all predecessor stages, including PA10, PA13 and PA15. |
| Inception comparison | Old-number make inception passed: MATCH cppgm++, with matching rebuilt objects. |
| Backend performance suite | Old-number PA38 test-perf passed: 16 envelopes within 10%, 1 fixture skipped by the existing envelope policy. |
| Four existing self-host CI configurations | Four workflow flavors preserved; local GCC self-host/inception passed. Ubuntu 24.04/26.04 GCC/libc++ runner executions are external CI validation, not local results. |
| Clean affected/final reference regeneration | PA13 unchanged regeneration leaves all 331 tracked outputs byte-identical. Independent full export verifies every portable reference; see the final-number register above. |
| Independent student export, supplied native tool, discovery, checks | Old-number export passed at /tmp/cppgm-consolidation-stage-a-export: 18,232 portable references verified, 393 Linux failure-diagnostic examples regenerated. No retired tools shipped; wrapper/download and scaffold smoke passed. See the final-number register above. |
| Final numbering, live links, no retired dependencies or empty passing suites | Applied and checked in Stage B; see the final-number register above. |

Do not interpret an unchanged baseline binary passing a case as proof of a
later source change. Do not mark the overall goal complete until the final
completion audit verifies every requirement in the plan.

## Intake observations to carry into A3

PA11's function dump deliberately uses source parameter types
(`RecordSourceTypeOverride` in semantic/declarations/analysis.cpp), while its
function signature uses adjusted parameter types. A dump such as
`function of (function of (int) returning int) returning int` does not by
itself indicate missing parameter decay. Document this distinction and use
canonical declaration/call identity when retaining PA7's adjustment assertion.
The semantic analyzer already had a fallback for the old parenthesized-name
AST; the A2 fix makes the syntax observation correct without removing that
fallback prematurely.

A3 namespace follow-up: the source-parameter view was intentional, but ignoring
`abstract-declarator` children was not. `BuildParameters` now handles them,
and `int (...)` in a parameter is parsed as an unnamed variadic function type.
The new call test checks the canonical signatures for those forms. Incomplete
arrays retain the established PA11 display `array of 0 T`; the guide explains
that spelling rather than requiring the old namespace tool's text format.

PA8 intake is complete: 9 retained, 10 reduced/split, 26 covered elsewhere,
22 retired. The cross-TU constant-visibility negative keeps both primary
sources under PA11 (`.t` plus `.t2`); an independent positive group proves
type scopes stay TU-local. PA10/11/12 wrappers explicitly select grouped input.
The signed/unsigned character-array string case exposed ordinary-string
compatibility missing from `AnalyzeStringArrayInitializer`.

Other retained observations exposed real mainline gaps: a later extern
declaration erased a prior zero definition; constexpr references to mutable
static objects lost their constant address; a namespace scalar reference bound
to a temporary used stack storage; internal native-object symbols collided
across translation units. Focused cases observe the corrected storage,
addresses, or runtime identity. The full 5862-case report passes these fixes.
The early semantic-core cross-client plan is marked superseded, while its
independent mainline observations remain historical maintainer material.

## Duplicate review

The initial intake checked exact duplicates and some equivalent coverage but
missed assertions in existing `spec/` cases. The subsequent review reads both
`general/` and `spec/`, including reference artifacts and later owning suites.
It removes **30 added roots**: 9 PA10, 14 PA11, 4 PA15, 1 PA21 and 2 PA30.
Twelve sources were reduced, and small missing observations were folded into
focused existing cases. The inventory records the covering paths rather than
claiming every old spelling or combination needs another fixture.

Examples: the existing const-int static assertion covers PA8's assertion;
`300-declaration-forms-valid` already covers enclosing-namespace definitions
and reference-chain array bounds; `100-extern-global` already reads a variable
defined in another TU. The existing inline linkage case now also compares
function addresses. A claimed transitive using test never used the imported
value; it adds no new lookup observation. Arbitrary nesting stress does not
require a duplicate reduced-depth fixture when recursive forms already exist.

Preserve distinct observations where needed: type formation alone does not
prove global reference lifetime, initialized bytes, or cross-TU identity.
The plan now explicitly requires this duplicate review before adding a case.


## A4/A5 verification details

The native encoding lesson's C++ example builds a 132-byte ELF image whose
entry is 0x400078 and whose executable exits 42. The native Linux replacement
smoke also emits and executes ELF successfully; all tool-help checks pass.
The legacy full Linux smoke still names an already absent mobjroundtrip
utility in its independent object probe; no full-script pass is claimed.

All 487 baseline input hashes were checked against 70a634a73, and every
recorded destination exists. Nine architecture audits pass after retirement
(9 tools, 236 production sources); the file audit passes with 35 warnings.
The old flat-layout audit baseline is absent from this checkout's history,
so its documented current-owner/provenance checks run without history checks.
