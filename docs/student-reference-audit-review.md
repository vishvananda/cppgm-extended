# Student reference audit review

Maintainer evidence from the read-only review of `~/work/v4codex` on 2026-09-30. This document is excluded from the student export.

The existing fixture/harness work is committed as `fb15cd49e` on `fix/student-audit-regressions`. Placement detection is corrected in `550f44dc2`: all twenty scalar-array false positives disappear, with genuine class-transfer detection retained. Static pointer/reference initialization is fixed by the accompanying compiler checkpoint. Constant class-object initialization is completed by the next compiler checkpoint; the other open work is recorded in the unified table.

## Unified progress tracker

Branch: `fix/student-audit-regressions`. Update this table at each checkpoint;
retain the numbered evidence below instead of treating a reference difference
as a bug by itself. **Open** means independently reproduced unless explicitly
marked **Needs verification**. ABI spellings must be checked against Clang before changing code or references,
as explicitly requested. Student-export validation is deferred until the
fix sequence is complete, as requested.

| ID | Work item | Discovery | Status / checkpoint |
| --- | --- | --- | --- |
| HARNESS | Quiet successful test-report output, expose failures, propagate export recipes | User | Done: fb15cd49e; source and isolated export each print one success total. Final combined export pending. |
| HARNESS-FAIL | Suppress successful focused-control summaries when another check in the assignment fails | Conversion-selection strict-report trial | Done in 2481b326d: the report exports its quiet setting to all 39 focused-control producers. Source and sanitized student Makefile tests expose real failures and suppress neighboring successes in both output orders. Strict 5969/5969 remains one line; harness and producer syntax checks pass. Full combined export remains deferred. |
| PLACE | Remove numbered-fixture host exemption; rewrite PA26/27 hosted-header fixtures; keep unique PA31 hosted coverage | User / v4codex | Done: fb15cd49e; default numbered fixtures are student-compiled. |
| DETECT | Stop treating scalar-array copyobj as class transfer / ABI evidence | v4codex | Done: 550f44dc2 removes twenty scalar-array false positives. The static-declaration checkpoint uses parsed template headers for pointer/reference NTTPs, removing four more false positives while preserving actual pointer/reference/member/function NTTP controls. |
| INIT-ADDR | Static namespace/local-reference and pointer initialization ordering | v4codex group 1 | Done: bc55d6227; five ordering reducers pass; full checks and ABBA pass. |
| INIT-OBJ | Emit evaluated class/base/array/template constant object values and relocations | v4codex group 1 | Done: a33d1456d. Seven copied ordering reducers and two new runtime fixtures pass; strict 5825/5825, full checks and ABBA pass. |
| ARRAY-IMAGE | Reconcile PA10 literal arrays and PA16 general constexpr readonly-image/copy rule | v4codex placement / group 1 | Done: 3e2a9a9d1; 48 references regenerated, two new runtime controls, strict 5827/5827 and full checks pass. Performance follow-up finds no persistent regression. |
| FIELD | Bit-field signed promotion / typed reads and volatile aggregate stores | v4codex group 2 | Done: 31598a5b3; two new runtime controls pass at -O0/-O2, seven references regenerated; strict 5829/5829, full checks and equivalent-output ABBA pass. |
| DTOR | Unqualified explicit virtual destructor dispatch and defined fixture lifetime | v4codex group 3 | Done: 049b5fa74; direct/virtual/further-derived runtime controls pass; strict 5829/5829, full checks and equivalent-output ABBA pass. |
| AGG-DEST | Construct aggregate arrays and braced-result members at their final destination | v4codex group 4 | Done: edd6b2121; preserve typed member actions at final addresses; five new controls and seven regenerated references, strict 5834/5834 and full checks pass. |
| RESULT-ABI | Canonical class result ABI for aliases, indirect calls and nontrivial empty results | v4codex group 5 | Done: 70e7a2920; one completed class fact; strict 5838/5838, full checks, Clang/GCC mixed-object controls and equivalent-output ABBA pass. |
| RESULT-CONV | Explicit conversion-function-template calls use canonical result deduction | v4codex group 5 | Done: 4858ddbc0; typed full target deduction and receiver selection; strict 5839/5839, full checks and equal-output performance pass. |
| CONV-IMPLICIT | Valid class copy initialization with a conversion-function template rejects as ambiguous | Additional reducer during RESULT-CONV | Done in 425bc2a90: remove the competing two-conversion constructor path; PA18 runtime control and Clang/GCC agree at O0/O2. Full validation recorded below. |
| CONV-SELECTION | Rank converting constructors against conversion functions during class copy initialization | Additional conversion-sequence controls | Done in 505a1cd3d: unified typed candidate selection, inherited receiver rules and required final-copy legality. Fifty host-agreed boundary programs and 21 PA12/18 fixtures pass at O0/O2; strict 5969/5969, full compiler checks and placement pass. Seven Alpha instruction/RSS gates pass with equal objects. One Clang/GCC-disputed derived-result control is excluded from required fixtures; the implementation follows Clang's selection rule as documented below. |
| LAMBDA-CONV-SPEC | Captureless lambda pointer conversion has a nonthrowing exception specification | Additional hosted-trait controls / CWG 1722 | Done in 425bc2a90: preserve the call operator specification independently; PA20 noexcept and hosted trait controls agree with Clang/GCC. |
| TMPL-VALID | Definition-time expression/bound validation, plus valid dependent bounds | v4codex group 6 | Done: ad5d5dbb8; known type/category facts validate unused operators/calls/bounds without evaluating dependent values. Eight PA14 fixtures, 22 copied rejection controls, strict 5887/5887 and full checks pass. Alpha instruction/RSS gates pass with equal outputs; fixed-call GCC disagreement documented below. PARAM-ADJUST completes the valid bound failure. |
| PARAM-ADJUST | Parameter declarator scope uses adjusted array/function object types | v4codex group 6 reducer | Done: 27eef472d; parameter lookup reuses ParameterBindingType; original PA6 source types remain. Two PA6/14 fixtures, strict 5879/5879, full compiler checks and placement pass. Alpha instruction/RSS gates pass with equal outputs. |
| DEMAND | Dormant static-member definition and storage demand | v4codex group 7 | Done in the accompanying checkpoint: indexed binding requests preserve unused/sibling/nested definitions, explicit instantiation and specialization ownership. Eight PA14/17 fixtures, fourteen reviewed references, strict 5895/5895 and all required checks pass. Six Alpha inputs pass instruction/RSS gates with equal outputs. Static declaration legality remains STATIC-DECL. |
| STATIC-DECL | Diagnose static-definition redeclarations/type/member mismatches; preserve explicit specialization declarations | Extended student storage controls during DEMAND | Done in the accompanying checkpoint: all eight original failures fixed; 37 agreed boundary controls and 19 new PA11/14/15/17 fixtures match Clang/GCC at O0/O2. Strict 5943/5943, full compiler/harness checks and placement pass. All six Alpha instruction/RSS gates pass with equal objects; cached name inventories remove the initial measured regression. |
| STATIC-BASE-ADDR | Recheck nonzero static base-reference offset reducer | Extended student storage controls during DEMAND | Done in the accompanying checkpoint: independent typed address projection, known complete-object virtual layouts, cleared unproved facts and guarded runtime null conversions. Thirty-eight boundary cases, 42 original controls and five PA22/23 fixtures agree with Clang/GCC at O0/O2. Strict 5948/5948 and all required checks pass. Six Alpha instruction/RSS gates pass with equal objects; the root-binding layout proof removes an initial targeted regression. |
| DISCARD-CALL | Discarded reference calls preserve effects without loading the referent | v4codex group 7 | Done in ddcd20c8c: PA10 control plus defined PA18/19 inputs; strict 5840/5840 and full checks pass; equal-output repeat performance shows no persistent regression. |
| REJECT | Four invalid programs currently accepted: noexcept receiver, result-type ambiguity, empty array pack, two user conversions | v4codex group 8 | Done: receiver/array in 1cb054e23, result identity in 88f5d368b, and implicit conversion chaining in 425bc2a90. All four original inputs remain unchanged; rejection references regenerated through ref-test. Full validation recorded below. |
| DEDUCE | Complete defaulted template arguments and preserve closure type in constructor deduction | v4codex group 9 | Done: closure type in 425bc2a90; canonical defaulted-pack deduction and PA19 runtime expectation in d65f8b02e. Five PA19 controls, strict 5924/5924, all required checks and four Alpha instruction/RSS gates pass. Declared ABI pattern remains MANGLE-PACK. |
| EH-OVERRIDE | Dynamic exception specifications on virtual overrides require an allowed subset | v4codex PA28 audit154 plus independent current reproduction | Done: typed restrictions compare incoming final overriders after completion, retain finite destructor unions and catch-reference rules. Fifteen new PA13/14/23 fixtures; strict 5869/5869, full checks and equal-output performance pass. Existing references unchanged; later runtime EH/backend issues remain separate. |
| EH-SPEC-COMPLETE | Complete-class lookup in ordinary member exception specifications | Additional timing controls / CWG 1330 | Done: 64f1a59d4; eight PA6/12/13/17 fixtures; strict 5877/5877, full compiler checks and placement pass. Alpha instruction/RSS gates pass with equal outputs; GCC late-typedef disagreement documented below. |
| EH-SPEC-TIMING | Timing of a virtual template exception specification using sizeof its current class | Additional override controls | Needs contract review: both hosts reject a noexcept(sizeof(D<T>)>0) virtual override while ours accepts. The entry behavior predates EH-OVERRIDE; keep its evidence separate from valid sizeof(T) deferred controls. |
| EH | Construction prefixes, active-handler lifetime/forwarding and failed-new deallocation | v4codex group 10 | In progress: EH-HANDLER-TEMP, EH-FORWARD, EH-CLEANUP and the aggregate-prefix candidate fix six of seven refreshed reducers at O0/O2. Failed-new deallocation remains; the expanded independent EH rows below retain additional gaps and contract reviews. |
| EH-HANDLER-TEMP | Destroy full-expression temporaries before ending their active catch | v4codex reference110 plus expanded EH controls | Done in ac2aaf704: existing typed handler boundaries cover return, statement, initializer and condition cleanup. Nine agreed controls and two PA21 fixtures pass at O0/O2; one dormant reference edge is corrected. Strict 5971/5971, full compiler checks and placement pass. Nine Alpha instruction/RSS gates pass with equal objects. Nested forwarding remains EH. |
| EH-FORWARD | Advertise enclosing catch clauses and unwind prefixes/active handlers in lifetime order | v4codex references106/112 plus independent boundary controls | Done in the accompanying checkpoint: 40 agreed boundary programs and six new PA21 fixtures pass at O0/O2; three independently reviewed references change. Strict 5977/5977, full compiler checks and zero placement findings pass. Nine Alpha instruction/RSS gates pass with equal objects. |
| EH-CLEANUP | Preserve handler lifetime and remaining-object unwind tails during lexical destruction | Original EH reducer plus expanded local cleanup controls | Done in the accompanying checkpoint: 34 host-agreed runtime boundary programs and seven new PA21/28 fixtures pass at O0/O2; three reviewed references change. Strict 5984/5984, all compiler checks and zero placement findings pass. Nine Alpha instruction/RSS gates pass with equal objects. Array element progress and non-NRVO returned-object cleanup remain separate rows. |
| EH-AGG-PREFIX | Preserve completed aggregate members/elements and interleaved temporaries when a later initializer throws | Original aggregate-prefix reducer plus expanded construction controls | Done: 37 agreed construction controls pass at O0/O2; nine independently checked PA21 fixtures and one reviewed reference correction. Strict 5993/5993, debug-info, variants, self-host PA5, all architecture/file/placement audits and twelve Alpha instruction/RSS gates pass. ARCH-FUNCTION closes the previously exposed file-audit findings. Synthesized-copy failures remain EH-SPECIAL-PREFIX; source-handler escape remains EH-CTOR-HANDLER. |
| EH-AGG-NESTED | Invoke a completed nested aggregate's custom destructor when a later outer member fails | Additional EH-AGG-PREFIX boundary control | Needs contract review: Clang invokes the nested destructor, GCC skips its body; both destroy its member objects. N3485's principal-constructor wording predates P0490R0's explicit completed-aggregate rule. Keep this difference separate from the 23 agreed cleanup failures; no oracle changed. |
| EH-AGG-TEMP-DTOR | Destroy a completed aggregate when an initializer temporary's normal destructor throws | Additional EH-AGG-PREFIX boundary control | Needs contract review: GCC and the candidate destroy both aggregate members; Clang 21.1.8 leaks them at O0/O2. Retain this host disagreement separately from ordinary construction failure; no required oracle added. |
| EH-SPECIAL-PREFIX | Destroy completed members when a later member in a synthesized copy/move constructor throws | Extended conditional controls and direct memberwise construction reducers | Done: 54 copy/move/base/array/assignment controls agree with Clang/GCC at O0/O2, as do both original memberwise reducers and both extended conditional all-step reducers. Fourteen PA21 fixtures pass their O0 contract; the two independently retained full-TU O2 crashes remain BACKEND-ARRAY-OPT. Strict 6013/6013, debug-info, variants, self-host PA5, all architecture/file/placement checks and fourteen Alpha instruction/RSS gates pass. |
| EH-CTOR-HANDLER | Retain constructor member cleanup when an inner source handler rethrows or misses | Additional aggregate-in-constructor control | Done: 46 boundary programs agree with Clang/GCC at O0/O2, including exact destruction traces. Twelve PA21 fixtures and five independently reviewed references; strict 6025/6025, debug-info, variants, self-host PA5 and all audits pass. Fifteen Alpha instruction/RSS gates and focused calibrated cycle gates pass. Standalone runtime gaps remain BACKEND. |
| EH-DTOR-HANDLER | Retain destructor member cleanup when an inner source handler rethrows, replaces or misses | Expanded EH-CTOR-HANDLER controls | Done with EH-CTOR-HANDLER: destructor escape, miss, replacement, swallowing, nine-member and twelve-element controls agree with both hosts. Five second-fault programs terminate correctly at O0/O2. Unique termination selectors prevent collision with typed source catches; full compiler and performance checks pass. |
| EH-COND-THROW | Normalize a class conditional with a raw throw operand before destination lowering | Additional conditional aggregate boundary control | Done: materialized class prvalues use destination-ready arms; nonreturning arms retire their cleanup segments and staged throw sites restore enclosing cleanup after a split. Original reducer and thirteen agreed boundary programs pass at O0/O2; six PA21 fixtures, strict 5999/5999 and all required compiler audits/checks pass. Alpha instruction/RSS gates and an interleaved two-image cycle check pass. Extended synthesized-copy and reference-initializer failures remain separate rows. |
| EH-CTOR-ARRAY-PREFIX | Retain earlier constructor subobjects after partial construction of a later loop-lowered member array | Expanded EH-SPECIAL-PREFIX controls | Done with EH-SPECIAL-PREFIX: partial-array cleanup explicitly enters the remaining constructor cleanup. Seven independently checked second-fault controls also cover scalar, inline-array, loop-array and completed-array unwind destruction; a second exception terminates immediately. The containing-constructor continuation has its own retired entry, shared by host-object and standalone paths. EH-CTOR-HANDLER also checks inline partial-array continuation through earlier subobjects. |
| EH-REF-INIT | Retain enclosing object cleanup while initializing an automatic reference | Expanded EH-COND-THROW boundary controls | Open, independently reproduced: nine direct lvalue/xvalue/reference-call/const-reference/aggregate-reference controls leak prior local objects here at O0/O2; Clang/GCC pass. StageAutomaticInitializerException excludes reference declarations, so no enclosing cleanup is attached. Preserve lifetime-extended backing storage when repairing this staging boundary. |
| EH-RESULT-CLEANUP | Destroy a non-NRVO returned object when later return-time destruction throws | Additional EH-CLEANUP result-ownership controls / CWG 2176 | Needs contract review: three prvalue/call/conditional controls fail here and in Clang 21.1.8 at O0/O2, but pass GCC. CWG 2176 adds returned-object destruction beyond the frozen N3485 wording. Keep this host disagreement separate; no required fixture or reference changes. |
| EH-ARRAY-DTOR | Preserve remaining elements when an unrolled class-array destructor throws | Additional EH-CLEANUP array boundary controls | Open, independently verified: a three-element array skips its first element after the second destructor throws; entry and cleanup candidates fail at O0/O2, Clang/GCC pass. A twelve-element control passes all compilers because the loop path already owns an unwind-progress suffix. |
| TMPL-FTRY | Retain the complete definition of a function template using a function-try block | Additional EH-CLEANUP source control | Open: the entry and cleanup candidates emit an empty instantiated body and return zero; Clang/GCC run the specified body and handler at O0/O2. Pattern registration uses a direct compound-statement lookup and does not retain handler syntax. |
| EH-RETHROW-DYNAMIC | Accept operandless throw in a function called with a dynamically active handler | Additional defined destructor/helper controls | Open: three programs reject here and pass Clang/GCC at O0/O2. N3485 15.1/8-9 requires the runtime active exception, rather than a lexical catch in the callee definition. No compiler/reference change yet. |
| MEMBER | Signed member-pointer adjustment, target-word truth, inverse conversion, width checks and repeated empty bases | v4codex group 11 | Open. |
| VBASE | Virtual-base layout/lifecycle, construction RTTI, null placement and diamond flags | v4codex group 12 | Open; correct uninitialized fixture before using it as a runtime oracle. |
| MANGLE-CONV | Conversion-function template names retain the declared dependent target | Additional Clang object check during RESULT-CONV | Open: Clang emits _ZN1XcvT_IKiEEv / _ZN1XcvT_IRiEEv; ours emits _ZN1XcvKiIS0_EEv / _ZN1XcvRiIS0_EEv. No encoder change yet; concrete target has replaced declared T in the name facts. |
| MANGLE-RESULT | Dependent decltype result forms retain expression identity and unparenthesized id category | Template result identity host-symbol controls | Open, confirmed against Clang/GCC and unchanged entry 1cb054e23: bare decltype(value) uses DT instead of Dt; named dependent selected(value) loses the expression and emits a concrete result type. Parenthesized decltype((value)) already agrees. No encoder change in the result identity checkpoint. |
| MANGLE-PACK | Preserve the declared expansion in function-template parameter name facts | Defaulted-pack Clang object controls / v4codex correction91 | Open: entry 425bc2a90 and candidate flatten a fixed-primary parameter expansion and append defaults; Clang/GCC retain `tuple<T_,DpT0_>`. The deduction checkpoint corrects the concrete template argument pack cardinality; the parameter pattern remains wrong. No encoder change made. |
| MANGLE-BOUND | ABI spelling for a template bound using sizeof an adjusted parameter | Additional PARAM-ADJUST Clang comparison | Needs contract review: Clang spells RAszfL0p__i; GCC and ours spell RA8_i. The parameter type is fixed after adjustment. No encoder or old oracle change made; retain host and typed-name evidence. |
| MANGLE | ABI substitution state for address expressions and RTTI template-template arguments | v4codex group 13 | Open, checked against Clang 21.1.8: source compiler already matches the member-address reducer; PA9 fact tool ends ER1C instead of ERS1_; RTTI template prefix uses S4_ instead of Clang's S3_. |
| ABI-GLOBAL | Use the raw ABI name for an ordinary external global-namespace variable | v4codex PA27 overlay145 | Open, checked against Clang 21.1.8: exact fixtures emit `g`, ours `_Z1g`; mixed links fail in both directions. Two inspection expectations and the variable encoder need correction. |
| INPUTS | Define PA13/23 object lifetime/value inputs and PA18/19 reference backing objects | v4codex fixture review | In progress: PA13 lifetime and PA18/19 backing objects corrected; PA19 pack count is corrected in the deduction checkpoint; PA23 initialized virtual bases remain. |
| ARG-REF | Allocate object backing separately from a lifetime-extended local reference slot | Argon 1 | Done: af1b1204c; separate storage and scope lifetime; strict 5835/5835, full checks and equivalent-output ABBA pass. |
| ARG-BRANCH | Remove invalid branch destructor suppression and prevent cross-arm initialized-state leakage | Argon 2 | Done in 5d4ff5a34: both original reducers and normal/nested throwing-arm controls pass; strict 5843/5843, full checks and equal-output ABBA pass. Other EH mechanisms remain open. |
| ARG-ARGS | Preserve side effects in empty aggregate-member constructor arguments | Argon 3 | Done: edd6b2121; retain constructor calls and argument/parameter lifetimes; counter and by-value lifetime controls pass. |
| ARG-COND | Apply bidirectional class conversion rules to mixed-class conditional operands | Argon 4 | Done in d240819cf: direct binding then value fallback, base/cv constraints and implicit-candidate controls; strict 5846/5846, full checks and equal-output ABBA pass. COND-RESULT remains separate. |
| COND-RESULT | Preserve const class conditional result types and copy glvalue class conditional results | Additional controls while fixing ARG-COND | Done in af5f041ed: const result facts, selected glvalue copying and scoped reference backing; strict 5849/5849, full checks and equal-output ABBA pass. |
| ARG-ARRAY | Construct aggregate member arrays of nontrivial class elements | Argon 5 | Done: edd6b2121 with AGG-DEST; final-address class-array construction, local/static/nested lifetime and identity controls pass. |
| ARG-SLOTS | Share stack space for mutually exclusive large temporary lifetimes | Argon 6 | Open optimization issue: independent defined reducer spans 1,639,824 bytes across 64 frames at -O1/-O2/-O3; GCC -O1 spans 103,824. Correct values/destructor counts; use a backend frame-size bound, not an arbitrary language stack budget. |
| BACKEND-ARRAY-OPT | Keep optimized array cleanup frames valid when helper bodies are defined in the same translation unit | Self-contained EH-SPECIAL-PREFIX fixture controls | Open, independently reproduced: the entry and copy-cleanup candidates crash at O2 for the twelve-element move and trivial-copy-prefix array fixtures, in standalone and host-linked object routes. Clang/GCC pass; our O0 routes and corresponding external-companion forms pass. Retain frozen sources, binary hashes, host-link controls and debugger observations under synthesized-construction-prefix/. |
| BACKEND | Standalone duplicate RTTI/native-label and freestanding dynamic_cast limitations | v4codex backend observations | Open review: shared RTTI host-object route passes; standalone route fails. Private-derived/base reducer already passes both. Two defined source-handler controls also retain identical entry/candidate standalone failures at O0/O2: nested-outer-swallow returns 10 and function-try-body-local aborts (134); their host-object routes pass Clang/GCC and the candidate. Keep these runtime routes separate from PA21 LowIR cleanup correctness. |
| ROUND | Excess-precision differences | v4codex PA25 | Review only: no proven oracle bug; preserve references unless course policy requires a change. |
| DIALECT | Multi-block-inline note using cmp slt instead of contracted cmp lt | Argon post-run note | No compiler fix established: corrected spelling reportedly passes. |
| HOST-TRIVIAL | Verify the deleted-copy triviality oracle and declaration-property semantics | v4codex PA29 handoff156 question | Done in 89a33c0a8: source assertions corrected, deleted/member/overload facts queried and cached; strict 5851/5851, full checks and equal-output ABBA pass. Viability and ABI classification stay separate. |
| HOST-SHORTHAND | Give the hosted nothrow trait fixture complete, typed definitions | v4codex PA29 handoff156 question | Done: complete typed definitions replace compiler template-name synthesis (spec.md section 10). Generic character and noexcept reducers move to PA14/PA16; incomplete/body controls enforce ordinary template rules. Strict 5854/5854, placement/harness/audits and performance pass; student has not edited its oracle. |
| PA29-ALIGN | Preserve GNU alias alignment through declarations and expression indirection | v4codex audit158 entry regressions | Open, independently confirmed: using-alias aligned(1) and typedef-alias *&i controls fail here at O0/O2 and pass Clang/GCC. No required fixture or oracle change was made for these controls. |
| EXPORT | Validate final combined shipped recipes, fixture discovery and quiet report | User | Pending until the fix sequence is complete; initial and INIT-ADDR exports already passed. |
| HARNESS-AUDIT | Suppress an expected missing-history Git diagnostic in the rename-manifest audit | Aggregate compiler validation | Done: quiet baseline verification accepts only the expected missing-object status; genuine Git failures remain visible and fail the audit. Five focused controls and the full harness pass; successful audit stderr is empty. Strict 5993/5993 remains one final line. |
| HARNESS-FILE | Stop counting an entire class as a function after an unrecognized constructor signature | Aggregate compiler validation | Done: both scans retire declaration-scope state before stripping an earlier inline body. A large-class control passes, a real oversized member still fails, and the full harness passes. ARCH-FUNCTION subsequently closes all eight genuine baseline size findings. |
| ARCH-FUNCTION | Refactor eight existing oversized functions missed by stale file-audit signature state | HARNESS-FILE detection correction | Done: responsibilities extracted without changing references or relaxing the 240-line limit; slot visits use one typed worklist. Exact optimizer stats and 210 unchanged control observations; strict 5993/5993, debug-info, variants, self-host PA5 and every required audit pass. Twelve Alpha instruction/RSS gates pass with equal objects; checkpoint evidence below. |

## Static address initialization checkpoint

The semantic graph now carries an evaluated static address with its binding or
string identity and byte offset. Namespace and local-static lowering emit that
relocation directly. Reference aliases retain address facts independently of
whether their referent has a constant readable value. Local references obtained
from runtime calls retain first-use guards. The eager startup queue for constant
local references is removed. Runtime pointer reads no longer claim the address
of their pointer variable, and the existing pointer/integer/pointer relocation
contract is preserved.

Twelve affected references were regenerated through `ref-test`. Added PA11
coverage observes pointer/reference/alias bindings from an earlier namespace
constructor and checks runtime pointer-copy/reference initialization. Added PA16
coverage observes a constant local reference during dynamic startup and checks
one-time initialization of a reference obtained through a runtime call. All five
ordering reducers return 0; the old compiler returned 1 or faulted.

Validation passed: strict report 5823/5823 (one total line), debug-info, all
backend variants, self-host through PA5, all nine architecture targets, file
limits and placement. An isolated student export verified 17898 portable
reference sidecars (6892 statuses), includes both new fixtures, excludes this
maintainer document, and passes its strict report 5458/5458 with exactly one
output line. Reference binaries act as the validation implementation in this
export; the running student's compiler was never edited or built.

Performance uses immutable before/after binaries, pinned `semantic_overload.cpp`
and its frozen headers, four A/A calibration blocks and six A/B ABBA blocks at
`-O1`. Every output is identical. Median paired CPU ratio is 0.99879 and wall
ratio is 0.97494; timing variation is substantial on the shared host, so these
measurements establish no detected compile-time regression, not a speedup.
Median paired peak-RSS ratio is 1.00508 (about 0.5% higher). All observations and
input/compiler hashes remain in
`/tmp/cppgm-v4-audit-review/perf-static-address/{aa,ab}.json`.

## Constant object initialization checkpoint

Evaluated constant address/object/element tables now belong to semantic graph
storage and survive analysis without copying. Static lowering consumes those
typed values, emits nested class/base/array members at their layout offsets,
and preserves function/string relocations, bit-field storage and floating bits.
Unsupported representations fall back to the previous initializer path; failed
constant probes still use dynamic initialization. The owning graph storage is
now declared in `semantic/model/storage.h`, keeping the analyzer header inside
its file limit. The named-address path reuses its already looked-up address fact.

Seven copied initialization-order reducers and both new PA16/17 runtime controls
pass. The PA16 control covers calls, base copies, unary results and class arrays,
plus a runtime initializer whose call must happen exactly once during startup.
The PA17 control observes a template static object's string relocation before
its reader initializes a namespace integer. Eleven references were regenerated
through `ref-test`; no acceptance statuses changed. The existing PA22 member-
function-pointer array continues to serialize its target and adjustment words.

Validation passed: strict report 5825/5825 with one success line, debug-info,
backend variants, self-host through PA5, all nine architecture targets, file
limits and placement. Suites ran sequentially where their generated paths
might overlap. Student export is deferred until the final combined checkpoint.

Performance retained four A/A blocks and six A/B ABBA blocks with immutable
binaries and the frozen semantic-overload input/headers at `-O1`. Every object
hash is identical. Median paired CPU ratio is 0.997628, wall ratio 0.992314 and
peak-RSS ratio 1.001937. Shared-host timing remains noisy (A/A CPU ratio 1.01308);
these measurements show no detected regression, not a speedup. All runs and
hashes are in `/tmp/cppgm-v4-audit-review/perf-static-objects/{aa,ab}.json`.

## Automatic scalar-array image checkpoint

Mutable automatic scalar arrays now use the existing constant-data pool when
the entire initializer is known. Eligibility is checked by array/element type,
cv and the typed initializer, rather than by the variable's constant binding.
Eligible array initializers enter the existing constant-evaluation context
while their syntax is analyzed once; nonconstant calls retain runtime demand
and execution. PA10 now states the literal-image case and PA16 explains the
extension to general constexpr evaluation. Class lifetime paths remain separate.

Regenerated exactly 48 existing references through the harness, including the
PA32 solution regression via its driver-root override. The changes replace
known element stores with a readonly image and one copy, with expected local
SSA renumbering. Source fixtures and acceptance statuses are unchanged. New
PA10/16 controls each copy two independent mutable arrays from one shared image,
then modify one. The PA10 control also preserves two dynamic calls and volatile
stores; the PA16 control covers a named constexpr value and a constexpr call.
Both runtime programs return 0. Strict report 5827/5827 prints one success line;
debug-info, backend variants, self-host through PA5, all nine architecture
checks, file limits and post-regeneration placement pass (zero findings).

Performance retained immutable A/B binaries and the same frozen semantic-
overload input/headers. Four initial A/A blocks measured paired CPU +1.17%;
six A/B blocks measured +2.64%, prompting investigation instead of declaring a
pass. Optimizer/native work counters are identical. Eight additional A/B blocks
pinned to CPU 0 measured -3.89% paired CPU and -3.08% wall time; matching four
pinned A/A blocks measured +0.054% CPU and -0.342% wall. The initial timing signal
does not persist. The shared host changes load substantially, so no speedup or
exact zero-cost claim is made. Every output object across all 88 observations
is byte-identical; paired peak-RSS differences stay within 0.14% in the pinned
runs. Every run, load snapshot and compiler/input hash is retained under
`/tmp/cppgm-v4-audit-review/perf-array-image/` in `aa.json`, `ab.json`,
`ab-pinned.json` and `aa-pinned.json`. Final student-export validation remains
pending until the fix sequence is complete.

## Field typing and volatile initialization checkpoint

Integral arithmetic, comparisons, shifts and compound assignments now consume
bit-field width facts when selecting promotions. A narrow unsigned or long
field whose values fit `int` uses `int`; a full-width unsigned field retains
unsigned arithmetic. Enum bit-fields follow their enumeration type's promotion
rules. Ordinary overload ranking remains unchanged. Lowered bit-field values
use explicit typed conversions instead of changing an operand's type silently.
Generated aggregate helpers preserve volatile stores to volatile members.

Seven existing references were regenerated through `ref-test`; all acceptance
statuses remain unchanged. Two new PA11 fixtures check narrow/full-width fields,
long and enum fields, unary and compound arithmetic, and volatile aggregate-array
initialization. Both compilers return zero at `-O0` and `-O2`; LowIR explicitly
contains the required typed copies and volatile helper store. The shared semantic
ownership ledger includes the three new helper definitions.

Validation passes: strict report 5829/5829 with exactly one output line,
debug-info, backend variants, self-host through PA5, all nine architecture
checks, file limits and placement (zero findings). Export remains deferred.

Performance observations are retained in `perf-fields/` beneath the review
scratch directory. The original semantic-overload input itself reads bit-fields;
its corrected object differs. Symbol-size inspection narrows executable growth
to one function, 1083 → 1089 bytes, caused by two signed typed conversions before
truth tests. These A/B observations are not an identical-output performance gate.
The initial measurement watcher also began tests before the intermediate report
was final; its overlapping observations remain retained rather than discarded.
The `rtti_names` equivalence trial rejects with both compilers and supplies no
performance result.

After all suites finished, immutable A/B binaries compiled the frozen
`recog_token_buffer.cpp` and headers at `-O1` on CPU 0. Four A/A calibration blocks
and six A/B ABBA blocks all produce the identical SHA-256
`08c380bf4060d88ae18fc59b0cfa858fd6c7c02a4c2d0ca1413b59559f55f906`.
Paired A/A wall ratio is 1.000; A/B user-CPU ratio is 0.996063, wall ratio
0.996552 and peak-RSS ratio 0.998712. No compile-time or memory regression is
observed on this input; shared-host measurements do not establish a speedup or
an exact zero-cost claim. All initial and follow-up observations are preserved.

## Virtual exception override checkpoint

Independent controls reproduce the audit154 entry defect: wider/unrestricted
specifications over throw(int), lax noexcept overrides and a specification wider
than an intermediate override are accepted by the entry compiler, contrary to
Clang/GCC and N3485 15.4 paragraphs 5/8. The same issue affects implicitly declared
destructors and restrictions from both branches of a shared virtual base.
Sources, commands and host diagnostics are retained under
/tmp/cppgm-v4-audit-review/exception-overrides.

Polymorphic completion records only actual restricted or deferred override
edges before replacing each incoming final overrider. Shared-view merges retain
both restrictions by consulting the incoming direct-base facts only when the
merged slot has distinct overriders. After layout and special-member completion,
the existing exception-specification demand resolves these edges once; detached
pending edges prevent references into a fact table from surviving recursive
semantic growth. Validation consumes canonical types, public/unambiguous base
paths and qualification-conversion facts; no global or source-text scan is added.

Implicit/defaulted/user-provided destructors with omitted specifications retain
the union of the directly invoked subobject destructor lists. An unrestricted
subobject makes the result unrestricted; duplicate canonical list entries are
removed. The existing subobject walk produces these facts, avoiding a second
walk. Early compatibility probes caught and corrected loss of finite lists in
the first candidate. Explicit lists also retain reference forms for catch
matching, reject forbidden rvalue-reference types and restrict pointer
conversions through nonconst pointer references. Existing RTTI canonicalization
continues to use the referred object type; no mangling encoder/spelling changes.
An executed finite-destructor reducer exposes an existing backend limitation:
entry and candidate both fail standalone emission with an unbound native label,
LowIR rendering with an invalid pooled string, and host-object emission with a
missing landing-pad block. Clang/GCC execute the defined reducer and exit 41.
Its runtime behavior is not claimed fixed by declaration compatibility; sources
and all three route diagnostics remain in finite-destructor-{runtime,lowir,
mixed-runtime}.json and are covered by BACKEND/EH follow-up.

Fifteen new fixtures exercise ordinary PA13 restrictions, PA14 dependent lists,
and PA23 shared virtual-base restrictions. References are generated only for
these new fixtures, with no existing reference changes. The 192 main matrix and
126 additional/shipped O0/O2 checks agree with ours and Clang throughout. GCC
rejects several pointer/qualification/nullptr/reference-list cases allowed by
N3485 catch matching, and accepts the forbidden rvalue-reference list as an
extension; these disagreements are retained rather than silently used as an
oracle. The different sizeof(current-class) virtual-template timing control is
recorded separately as EH-SPEC-TIMING; valid sizeof(T) controls pass.

The placement detector masks function-suffix dynamic exception metadata while
preserving throw expressions, nested parentheses, control statements and
operator/function-pointer declarations. It assigns the metadata its PA6 owner;
polymorphic/template/hierarchy assertions retain their later owners. Metadata
also no longer conceals hidden generated EH during review. All 52 detector tests
pass. The final strict report passes 5869/5869 in one line. Separate PA13/PA14/PA23
checks pass 48/48, 319/319 and 47/47 respectively; debug-info, backend variants,
self-host through PA5, all nine architecture audits, the file audit and the
harness pass. Placement scans 2999 fixtures with zero placement/hygiene findings;
the existing 57 EH review notes are unchanged. Final logs and statuses are in
exception-overrides/validation.json and final-architecture.log. Performance
passes on frozen recog and 1500 virtual override pairs: four A/A calibration
blocks and six A/B ABBA blocks each give CPU median ratios 1.0 for both inputs,
wall ratios 1.0/0.98 and RSS ratios 1.001260/1.002006. A/A CPU is 0.996454/1.0;
shared-host noise does not establish a speedup. All 80 objects match their
respective baselines (recog SHA-256 08c380bf4060d88ae18fc59b0cfa858fd6c7c02a4c2d0ca1413b59559f55f906;
virtual SHA-256 f479c1d3ba01bdcdd4ced85b0e4211d032413bc0dba7f13f82e76ffad4b850f0).
Commands, immutable binaries and every observation are retained under
/tmp/cppgm-v4-audit-review/perf-exception-overrides. Early affected
checks were invoked as concurrent root test targets, which share summary-count
files; those results are superseded by separate sequential assignment checks.

## Hosted trait definitions checkpoint

Reviewing HOST-SHORTHAND exposes three template-name shortcuts forbidden by
spec.md section 10. The original forward-only nothrow and char_traits fixtures
compile here but Clang/GCC reject their undefined primary instantiations. The
original vendor __is_nothrow_invocable fixture provides a false primary body,
but the compiler overrides it with a calculated value; both hosts reject its
resulting assertion. Entry proof, source copies and host diagnostics are in
/tmp/cppgm-v4-audit-review/trait-deleted-probes/named-template-observations.json
and /tmp/cppgm-v4-audit-review/hosted-template-definitions/entry-controls.json.
These additional cases are independently confirmed here, rather than new
student reference edits or an explicit personal-reference disagreement.

The three reducers now provide complete definitions: nothrow construction
traits use __is_nothrow_constructible, the custom character primary supplies its
alias and conversion bodies, and the vendor invocation trait computes noexcept
from a declared declval call expression. Clang/GCC compile all three at -O0/-O2.
The compiler no longer classifies or fabricates a template specialization by
these names. Template declarations, bodies and explicit specializations take
the ordinary typed instantiation path; unused forward declarations stay valid,
but missing members/definitions are rejected when demanded. The construction-builtin reducer and supplied primary/specialization control
stay in PA29. The complete character-conversion reducer moves to PA14 cluster
300, and the ordinary template/noexcept cache reducer to PA16 cluster 200.
Incomplete character/type-member and static trait-value controls live at PA14
100 and PA15 100 respectively. These checks need neither host headers nor a
duplicate PA31 fixture; their earlier native/LowIR harnesses make the generic
goals observable. No mangling was changed.
Strict report passes 5854/5854 with one success line; debug-info, variants,
self-host PA5, architecture and file audits pass. Placement first identifies a
real cluster correction for the dependent character reducer plus a false
dependent-name claim on the concrete incomplete-character control. The former
now lives at PA14 cluster 300. The placement detector requires an in-scope
template parameter in a template-id qualifier, while preserving explicit
typename evidence and later dependent matches after concrete qualifications.
Focused negative/positive/scope controls are added. All 49 detector tests pass;
placement reports zero findings for 2984 default fixtures and 404 PA29 fixtures.
The harness passes after staging the moved fixture paths: its first run used
the old Git-index inventory and failed on removed sidecars. The final strict
report remains one line, 5854/5854. Check logs and both initial/corrected audit
results remain in hosted-template-definitions-final-validation.json.

Immutable A/A calibration (four blocks) and A/B ABBA (six blocks), on the frozen
recog input at -O1, give candidate/baseline medians of 1.0 CPU,
0.993103 wall time and 1.003746 RSS; A/A CPU is 1.003521. All 40 objects have the
same SHA-256, 08c380bf4060d88ae18fc59b0cfa858fd6c7c02a4c2d0ca1413b59559f55f906.
Every observation is retained under
/tmp/cppgm-v4-audit-review/perf-hosted-template-definitions. No persistent
regression or speedup is established on this shared host.

## PA29 declaration-property trait checkpoint

The deleted-copy fixture now asserts the positive trivially-copyable, trivial
and POD declaration properties supported by N3485 and independently by Clang
and GCC. Copy construction and assignment remain unavailable. Its compile-only
success oracle does not change. A new PA29 cluster-500 fixture checks implicit
nested deletion, template members, nontrivial subobjects under explicit deletion,
all copy/assignment overloads, two defaulted overloads, deleted destructors,
defaulted-later members and virtual classes. Both fixtures compile with ours,
Clang and GCC at -O0/-O2. A nine-type bitmask originally gives ours 56 versus
both hosts 165; the corrected query gives 165.

Declaration queries consume retained member indexes, special-member kinds and
selected subobject facts, independent of deleted-call eligibility. Existing
trivial transfer facts provide a fast path; request-local flat binding sets
visit shared subobject declarations once, avoiding repeated recursive work.
The completed class declaration property is then cached on its special-member
facts, so repeated trait queries use a constant-time fact lookup; incomplete
classes are not cached and specialization reset clears the fact. Initial frozen
5000-query probes showed roughly 19% extra CPU before this completed-fact cache;
all initial observations are retained in perf-trivial-declarations/.
A second PA29 fixture covers 18-level shared ordinary/deleted class graphs. Destructor
queries likewise distinguish declaration triviality from destruction viability.
Out-of-class defaulted constructor/destructor facts retain their user-provided
status. No ABI classification, transfer lowering or mangling was changed. Clang
LLVM confirms that consume(Deleted) takes a pointer despite the positive
trivially-copyable trait, as specified by the all-deleted ABI rule:
https://itanium-cxx-abi.github.io/cxx-abi/abi.html#definitions .
Evidence and commands are retained under
/tmp/cppgm-v4-audit-review/trait-deleted-probes/.
Strict report passes 5851/5851 with one success line; debug-info, variants,
self-host through PA5, all nine architecture checks and file audit pass.
Default placement checks 2980 fixtures with zero findings; a separate PA29
placement audit scans all 405 fixtures with zero findings. Final statuses/logs
are retained in /tmp/cppgm-v4-audit-review/trait-deleted-validation.json.

Immutable A/A and ABBA evidence is in perf-trivial-declarations/. The first
post-cache normal workload has B/A user-CPU 1.011400 and wall 1.020699, with
wide sample spread; the 5000-query workload has user-CPU 1.023256 versus A/A
1.011628 near the time counter resolution. All emitted objects compare equal.
Repeat four-block A/A and eight-block ABBA results: normal input user-CPU
0.982283, wall 0.979258, RSS 0.999185; 20000-query input user-CPU 0.996737,
wall 0.989869, RSS 1.000222. Corresponding A/A user-CPU medians are
0.997882 and 0.986394. All 176 gate objects compare equal within each frozen
input (88 normal, 40 initial trait, 48 amplified trait). The initial 19%
repeated-work cost is removed; no persistent compile-time/memory regression is
observed. Broad shared-host spreads do not support a speedup claim. Earlier
observations remain alongside the repeat data. No existing reference changed.

## Qualified conditional results and glvalue copies checkpoint

Class conditional expressions retain the combined top-level class qualifiers
in their semantic result and dump fact; their arms still construct unqualified
object storage. A selected class value conversion retains its qualified target.
Direct trivial class initialization bypasses a copy constructor only for an
actual prvalue conditional recipe; a glvalue source now takes the ordinary
selected copy-construction path. Deleted copies are rejected there.

Class conditional prvalues bound to local references are materialized before
binding. A directly bound complete temporary receives a scope lifetime even
when its constructor recipe contains conditional children; those children keep
full-expression cleanup. No scope lifetime is extended through a reference-
returning call. The new PA12 cluster-200 controls check const/nonconst overload
selection, mixed conversion and const-returning call operands, selected base
references followed by value copying, copy observations and deleted-copy
rejection, plus backing-object/argument destruction timing. Ours/GCC/Clang
agree at -O0/-O2 on the final fixtures.

Evidence lives in /tmp/cppgm-v4-audit-review/conditional-result-fixtures/.
The initial single-expression test exposed a separate preexisting cleanup
failure under a five-call compound || condition: both the immutable entry
compiler and current compiler fail its liveness check; Clang/GCC pass. The
compound-condition-observations.json reducer is retained with the remaining
EH work. The required qualification fixture checks each overload in a separate
full expression and independently checks scope extension. The initial generated
reference from the failing development control is superseded via ref-test only
after the source/native assertions pass; it is not accepted as a runtime oracle.
Strict report passes 5849/5849 with only its final line; debug-info, backend
variants, self-host through PA5, all nine architecture checks and file audit
pass. Placement checks 2980 fixtures with zero findings. Logs/statuses are in
/tmp/cppgm-v4-audit-review/conditional-result-final-validation.json.

Immutable binaries, CPU-0 frozen input, four A/A and six ABBA blocks retain
all observations under /tmp/cppgm-v4-audit-review/perf-conditional-result/.
Paired B/A medians: CPU 0.992395, wall 0.952381, RSS 1.000381; A/A CPU
0.996094, wall 1.003521. All 40 objects have identical SHA-256 08c380bf…55f906.
No regression is observed; shared-host wall spread does not support a speedup
claim. No previous fixture reference was changed.

## Mixed-class conditional conversion checkpoint

The class-to-class matching step retains its selected conversion facts and
tries direct reference binding before the value alternative. A constructor-
created temporary does not count as a direct lvalue binding. The value fallback
retains the underlying base-direction and cv restrictions, so a converting
Derived(Base const&) constructor cannot compete with a direct derived-to-base
reference match. Both viable conversion directions reject as ambiguous;
explicit constructors remain excluded from implicit matching.

The original Argon Tracker/Box reducer now passes at -O0/-O2. A header-free
PA12 cluster-400 fixture checks both operand orders, selected conversions,
normal destruction/liveness, const operands, direct conversion-function
reference identity and a base/derived control. Two negative controls cover
bidirectional and explicit-only conversion. Ours/GCC/Clang agree on all three
fixtures at -O0/-O2. Evidence lives in
/tmp/cppgm-v4-audit-review/conditional-conversion-probes/ and
conditional-conversion-fixtures/. No existing reference is changed. Strict
report passes 5846/5846 with one success line; debug-info, backend variants,
self-host through PA5, nine architecture checks, file audit and placement pass
(2977 fixtures, zero findings). Logs and command statuses are retained in
conditional-conversion-final-*.log and conditional-conversion-final-validation.json.

After all validation finishes, immutable compilers process the frozen
recog_token_buffer source and headers at -O1 on CPU 0. Four A/A and six A/B ABBA
blocks all produce identical objects (40 observations, SHA-256
08c380bf4060d88ae18fc59b0cfa858fd6c7c02a4c2d0ca1413b59559f55f906).
Paired A/B CPU ratio is 1.000000, wall ratio 0.996552 and peak RSS ratio
0.999606; A/A CPU ratio is 0.984733 and wall ratio 0.993080. No regression is
observed on this input. All observations and binaries are retained in
perf-conditional-conversion/.

Additional boundary controls reproduce two separate entry defects and remain
tracked as COND-RESULT: const class conditional results lose their qualifier
when overloads distinguish const lvalues from nonconst rvalues; initializing a
base-class value from a glvalue conditional reaches a prvalue-only lowering
route and rejects. The immutable entry compiler reproduces both, so they are
not introduced by this fallback. The direct base-reference control passes once
the base-direction constraint is enforced. These are not promoted as successful
required oracles before their implementations are corrected.

## Conditional temporary cleanup checkpoint

Removed the local-literal reachability guess and its graph/lowering flag.
Semantic analysis demands destructors for every conditional arm that lowering
emits. Branch-local cleanup retires the arm's initialized identities before a
sibling is lowered. A terminated arm materializes its pending unwind targets and
retires its cleanup-region identity without emitting a continuation or eh_end
in its sibling. All three conditional lowering forms share the same finish path.

The original same-type Argon reducer changes from exit 233 to 0; the distinct-
type reducer changes from a missing-destructor binding error to 0, at -O0/-O2.
An additional nested terminated-arm reducer previously crashes when the sibling
is selected; both sides now pass. Ours, GCC and Clang agree at -O0/-O2 on these
reducers and the three new defined fixtures. PA12 owns the normal branch control;
PA21 owns throw/unwind observations. Existing PA12 direct-class-call and PA21
hidden-EH conditional references are regenerated through ref-test: emitted arms
now retain their normal/exceptional destructor calls. No runtime oracle changes.

Evidence: /tmp/cppgm-v4-audit-review/branch-cleanup-probes/,
branch-cleanup-fixtures/, branch-cleanup-final-*.log and
branch-cleanup-final-validation.json. Strict report passes 5843/5843 with one success line; debug-info, backend
variants, self-host through PA5, nine architecture checks, file audit and
placement pass (2974 fixtures, zero findings).

After validation finishes, immutable A/A and A/B compilers process the frozen
recog_token_buffer source and headers at -O1 on CPU 0. Four A/A and six A/B
ABBA blocks produce identical objects (40 observations, SHA-256
08c380bf4060d88ae18fc59b0cfa858fd6c7c02a4c2d0ca1413b59559f55f906).
Paired A/B user CPU ratio is 1.000000, wall ratio 0.987590 and peak RSS ratio
0.987371; A/A CPU ratio is 0.996335 and wall ratio 1.017681. No performance
regression is observed on this input. Raw observations and immutable binaries
are retained in perf-branch-cleanup/. Other EH items remain open.

## Discarded reference calls and defined deduction inputs checkpoint

A void cast of a reference-returning call now evaluates the call through its
address/storage path, without loading its referent. Existing volatile id accesses
and used reference results retain their reads. This is a focused correction of
the reported call form, not a claim that every discarded-expression form has
been audited. PA10's new cluster-200 control emits all four source calls and only
the two required volatile reads. Its explicit void casts, ordinary discarded
call, volatile-id control and consumed result distinguish effect preservation
from removal of the forbidden read.

PA18 transitive-base helper results now name an initialized member in the
existing typed tuple implementation, retaining base deduction and deleted
overload coverage. PA19's helper uses typed static backing, and its main passes a
real tuple through the existing pointer instead of dereferencing null. Both
source corrections preserve their template/return-SFINAE goals and make native
execution defined. All three fixtures pass with ours/GCC/Clang at -O0/-O2.
The initial strict report flags exactly the two changed-input references; both
are regenerated through ref-test. The isolated scalar-call control owns PA10;
the later PA18/19 fixtures retain their additional template coverage.

Validation and performance evidence are retained under
/tmp/cppgm-v4-audit-review/discard-reference-* and perf-discard-reference/.
Strict report passes 5840/5840 with one success line. Debug-info, backend
variants, self-host through PA5, nine architecture checks, file audit and placement
all pass (2971 fixtures, zero findings). The frozen recog_token_buffer -O1 gate
uses immutable binaries, CPU 0, initial four A/A + six ABBA blocks and a repeat
four A/A + eight ABBA blocks after validation stops. Every object is identical.
Initial B/A CPU median 1.011509, wall 1.006826, RSS 1.000645 prompted repetition;
repeat CPU 1.001581, wall 1.002110, RSS 0.999852 versus repeat A/A CPU 1.015564.
The initial timing increase does not persist beyond calibration variation. All
88 observations are retained; no speedup is claimed. The unused static-member
demand half of DEMAND and two remaining INPUTS corrections are open. Export
remains deferred to the combined checkpoint.

## Explicit conversion-template call checkpoint

Retain the conversion-type-id already parsed at a member call, then build its
canonical type and reuse conversion-template deduction. Explicit names preserve
that complete type (including cv and references); implicit conversion deduction
keeps its existing adjustments. Match all candidates to the target and retain
receiver overload selection. Qualified calls collect from the named base;
unqualified calls retain the receiver's naming class even when an inherited
specialization was collected first. No source/output reparsing is introduced.

The new PA18 cluster-300 fixture checks class/alias, fundamental, cv-qualified,
pointer and reference targets, dependent calls, mutable/const receiver overloads,
implicit calls before explicit calls, and qualified/unqualified inheritance.
It passes with ours/GCC/Clang at -O0/-O2. The focused controls preserve the initial
qualified-call regression and its subsequent correction. Private access rejects
with all three compilers. Final strict report is 5839/5839 with one success line and no existing
reference changes, including the cv/reference extensions. Debug-info, backend
variants, self-host through PA5, nine architecture checks, file audit and final
placement pass (2970 fixtures, zero findings).

A separate valid class copy-initialization reducer still rejects as ambiguous
in immutable 70e7a2920 and the edited compiler; CONV-IMPLICIT retains it for the
conversion-legality work. Its ambiguity is not caused by explicit calls. The
fixture uses an aggregate result to keep that separate defect out of this test.
The scalar/reference full-target controls additionally expose a naming defect:
Clang raw object symbols use cvT_ with Ki/Ri template arguments, whereas ours
places the substituted concrete target in the conversion name. MANGLE-CONV
records both exact raw spellings and /tmp/cppgm-v4-audit-review/conversion-call-probes/
clang-object-spellings.json. No mangling implementation was changed. A stripped
standalone executable's absent symbols are retained as an invalid inspection
method, not evidence of spelling agreement; relocatable objects provide the
actual comparison.

Frozen recog_token_buffer -O1 performance uses immutable binaries, CPU 0, four
A/A and six ABBA blocks; every output object is identical. Paired B/A medians:
CPU 1.003846, wall 0.996552, RSS 0.999960. The small positive CPU median is within
observed variation (A/A CPU ratios 0.893–1.022); no regression is detected. All
observations and checks are retained in perf-conversion-call/ and
conversion-call-*.json. Student export remains deferred.

## Canonical class result ABI checkpoint

Class result classification now depends on the completed class entity: large or
nontrivial-for-calls results use caller storage, including empty and exactly
16-byte classes. Remove function-specific dependent-spelling overrides, their
binding flag and publisher; definitions, direct calls, indirect calls and named
return destinations consume the same fact. Parameter classification and symbol
spellings are unchanged. The semantic owner ledger removes the retired publisher.

Clang 21.1.8 checks precede this edit. LLVM signatures confirm small alias direct
results and nontrivial empty/template-empty/16-byte indirect results. The original
dependent alias pointer faults here but passes with Clang/GCC. A 16-byte mixed
Clang/ours result links both ways before the edit, returning 1 in one direction
and faulting in the other; both directions now return 0. Full evidence lives in
/tmp/cppgm-v4-audit-review/clang-result-abi/.

Two new frontend controls live in PA12 cluster 200 and PA14 cluster 300, the
earliest result and dependent-name owners. A new PA27 fixture adds host object
interoperability in both directions and runs with GCC and Clang providers. Only
.lib.provider.cpp is host compiled; .t.1 is student compiled. Existing PA16/18
fixtures retain their constexpr, deduction, access and conversion coverage.
Five existing references are regenerated through ref-test: PA16 nontrivial-empty,
three PA18 result cases and PA20 dependent-owner lifecycle. No source/status
corrections accompany these shape changes. All new frontend controls and both
copied reducers pass with ours/GCC/Clang at -O0/-O2. Explicit conversion-template
calls still need RESULT-CONV; correcting result ABI alone does not fix lookup.

Architecture, owner and file audits pass. Initial placement passes 2969 fixtures;
the final default audit passes 2969 fixtures. A separate PA27 audit covers
159 host-object fixtures with zero findings. Frozen recog_token_buffer -O1
measurements use immutable binaries, CPU 0, four A/A and six ABBA blocks, all
output objects equal. Paired B/A medians: CPU 0.892444, wall 0.885022, RSS 1.001966.
A/A CPU 1.128061 and wall 1.116413 expose substantial machine noise, so claim no
speedup; no candidate regression is detected. Retain all observations under
perf-result-abi/. Strict report passes 5838/5838 with one success line;
debug-info, backend variants and self-host through PA5 also pass.

## Reference-bound temporary backing checkpoint

Constructor-based class prvalues are now materialized before conversion to a
reference, matching the already-materialized aggregate paths. The temporary owns
its object storage; the reference binding holds only a pointer. Existing local
reference lifetime extension then retains and destroys the correct object.

The new PA12 control uses two 64-bit fields, const and rvalue references, an
adjacent sentinel, nested scopes and observable destruction. It and the original
Argon reducer pass with ours, GCC and Clang at -O0/-O2. The generated LowIR has
separate obj<16x8> slots and pointer stores. No existing reference changed.
Strict report passes 5835/5835 with one success line; debug-info, variants,
self-host through PA5, nine architecture checks and file audit pass. Placement
passes with zero findings across 2967 fixtures. It caught an initial cluster
100 placement; the fixture now uses the owning temporary cluster 200. Focused
PA12 discovery passes 260/260 and 13/13 survivor properties after the move, and
regenerated references are byte-identical to the verified original-path output.

The frozen recog_token_buffer -O1 gate retains four A/A and six ABBA blocks with
immutable binaries, pinned CPU 0 and equal output objects. Paired B/A medians:
user CPU 0.961760, wall 0.960008, RSS 0.999597; A/A user 1.007576, wall 1.005989.
The shared machine produces noisy paired observations, so these support no
regression claim, not a speedup claim. Evidence lives in
/tmp/cppgm-v4-audit-review/reference-backing-*.json and perf-reference-backing/.

## Aggregate final-destination checkpoint

Preserve typed aggregate initialization actions whenever a member contains a
class, and initialize class-array leaves at their retained destination address.
This avoids relocating constructed objects or adding observable transfers of
braced-result members. Scalar-only helpers retain their existing path. Remove
the empty-body constructor shortcut: arguments and by-value parameter lifetimes
remain observable even when the constructor body is empty. Partial-construction
exception cleanup remains tracked under EH.

Five new controls live at the earliest feature owner: PA11 aggregate class-array
lifetime and empty-constructor argument effects, PA12 member-copy observations
and parameter lifetime, and PA15 templated array element identity. The placement
audit caught an initial PA11 cluster mistake; these fixtures now use its aggregate
cluster, rather than weakening detection. All five controls and four discovery
reducers pass with ours, GCC and Clang at -O0 and -O2. Seven existing references
were regenerated through ref-test; no acceptance expectations changed.

Strict report: 5834/5834 with one success line. Debug-info, backend variants,
self-host through PA5, all nine architecture checks, file audit and placement
pass (2966 fixtures, zero findings). The immutable A/B performance gate uses the
frozen recog_token_buffer input at -O1, CPU 0, four A/A and six ABBA blocks.
Every output agrees byte-for-byte. Paired B/A medians: user CPU 0.996154, wall
1.003285, RSS 1.002870; no detected regression. A/A wall variation was larger
(0.935460); retain all observations and make no speedup claim. Evidence is in
/tmp/cppgm-v4-audit-review/aggregate-*.json and perf-aggregate/. Student export
remains deferred to the final combined checkpoint.

## Explicit virtual destructor checkpoint

Explicit destructor calls now suppress virtual dispatch only when the destructor
name is qualified. Unqualified `p->~Base()` uses the complete-destructor virtual
slot; an inherited `this->~Derived()` body can dispatch to a further-derived
object. Qualified `p->Base::~Base()` retains a direct call.

Renamed the PA13 fixture to
`400-explicit-virtual-destructor-call-dispatch.t`. It allocates each object,
explicitly destroys it once, and releases storage directly, avoiding the old
automatic object's second destruction. Traces verify 21 for virtual derived/base
destruction, 1 for qualified base-only destruction, and 321 for an inherited
body acting on a further-derived object. The copied student reducer and new
fixture return zero at `-O0`/`-O2` with our compiler and GCC; the new fixture also
passes both levels with Clang. New references are harness-generated; the other
existing course outputs and acceptance statuses need no changes.

Strict report passes 5829/5829 with one total line; debug-info, backend variants,
self-host through PA5, all nine architecture checks, file limits and placement
also pass. Final export validation is still deferred. Four A/A blocks and six
ABBA blocks use immutable binaries and frozen `recog_token_buffer.cpp`/headers
at `-O1` on CPU 0; all forty objects have the same SHA-256 as the FIELD gate.
A/A wall ratio is 1.00704; A/B user-CPU ratio is 0.984675, wall ratio 0.993041
and RSS ratio 0.999745. No regression is detected on this input; timing remains
noisy and does not prove a speedup. Every observation is retained in
`/tmp/cppgm-v4-audit-review/perf-dtor/{aa,ab}.json`.

## Scope and evidence

The initial review read the milestone plans/audits through PA26, all 30 reference-correction
notes, PA17 storage-references.md, and the associated reducers/manifests.
The running checkout advanced from 78410bfc to PA27 during this read-only
review; the correction evidence is from completed PA9-23 work, while the
PA26 audit was initially a draft. Nothing in that checkout was edited or
built. Reproducers were copied into this scratch directory and run against
/home/vishvananda/cppgm-extended/dev/cppgm++ at solution HEAD 3299e7ff3,
including the prior locally authorized cleanup fix.

134 shared reference sidecars differ: 130 LowIR/ABI outputs and four rejection
statuses. The corresponding course source fixtures are byte-identical.
Reference changes are not themselves violations of the student's policy:
the notes cite language/ABI/course rules and provide reduced proofs.
review-evidence.json records the inventory, binary identity and 64 source
reducer observations; final-probes/ contains additional handler/member-pointer
observations. Different references alone do not establish an outstanding bug:
compare the cited obligation and current implementation.

## Placement conclusions

No separate list of literally "misplaced" tests occurs in the plans/audits.
Before `550f44dc2`, our placement checker, using our tracker against the student tree,
reported ten additional tests with twenty early-feature findings. All ten are
array-reference revisions introducing copyobj. The old detector incorrectly inferred
class copy/move and class by-value ABI from every copyobj instruction. A scalar
array copy does not imply either feature. These tests should remain in PA10/11.

The ten tests are:
pa10/tests/general/100-array-cv-rvalue-reference-overload.t
pa10/tests/general/100-unary-plus-array-decay.t
pa10/tests/general/200-conditional-array-decay-subscript.t
pa10/tests/general/200-const-cast-reference-array-subscript.t
pa10/tests/general/200-const-cast-reference-similar-pointer.t
pa10/tests/general/200-inferred-local-array-bound.t
pa10/tests/general/200-local-direct-init-array-subscript.t
pa10/tests/general/200-partial-local-array-zero-initialization.t
pa11/tests/general/200-out-of-class-member-default-argument.t
pa11/tests/general/300-discarded-comma-reference-result-no-copy.t

The student's tree still contains the three hosted-header fixtures already
rewritten in our working tree; those are the six additional hygiene findings.
The checker also records 51 early LowIR EH review leads, not placement failures.
Generated cleanup operations are not automatically source throw/catch coverage.

PA16's array rule creates a real cumulative contract/reference conflict. It
requires readonly constant data plus one copy for every completely known,
nonvolatile automatic scalar array. Forty-eight revised references implement
that rule across earlier and later milestones. Scalar stores are legal C++;
this is a course representation issue, not a C++ semantic defect. A compiler
with no PA mode cannot emit both shapes for the same unchanged input. Reconcile
our handouts, shared lowering and regenerated references together. If earlier
literal-array references adopt this shape, make the basic literal-image rule
explicit at PA10; PA16 can add general constexpr eligibility. Keep array
behavior fixtures in their original PAs and retain focused policy controls in
PA16. Do not move them merely because copyobj appears.

The revised PA12 scalar-new oracle adds failed-construction EH deallocation.
The leak is real (our copied reducer returns 1), but PA21 owns exception-aware
construction; PA12 explicitly excludes exception-aware value transfers. Preserve
a PA12 allocation/constructor smoke test with an explicit noexcept constructor,
and add the external throwing-constructor/deallocation test in PA21. Avoid
silently importing an EH requirement into PA12 solely through a reference edit.

## Confirmed findings and fixes in this checkout

1. Static initialization and storage identity (PA10/11/16/17/18/19).
   At the initial review, copied early-observer address/reference/constexpr-object
   reducers returned 1 or faulted because initialization was delayed to the dynamic
   hook. INIT-ADDR and INIT-OBJ now resolve the reproduced ordering cases. Constant reference binding must precede dynamic initialization, even
   when the referent's own constructor is genuinely dynamic. Constant local
   references likewise need static binding rather than queued startup work.
   Fix classification/publication in shared initialization and constant-value
   owners, then regenerate references. Promote simple ordering reducers at
   PA10/11 and retain constexpr/template extensions at their feature owners.
   Evidence: pa10/reference-corrections.md, pa11/reference-corrections.md,
   pa16/reference-corrections.md, pa17/storage-references.md,
   pa18/reference-correction65.md and reference-correction83.md,
   pa19/reference-correction92.md.

2. Bit-field typing/promotion and volatile initialization (PA11/12).
   At entry, the small unsigned bit-field arithmetic reducers returned incorrect values;
   a field whose entire range fits int must promote to int. The aggregate
   Device helper still emits an ordinary store into its volatile member.
   The reference's missing typed copy operations also violate the LowIR typing
   contract. Fix the shared field-read/promotion and scalar-initialization
   paths. FIELD now resolves these cases and regenerates the seven affected outputs.
   Evidence: pa11/reference-corrections.md; pa12/reference-corrections.md.

3. Unqualified explicit virtual destruction (PA13).
   At entry the defined reducer returned 1: unqualified p->~B() and an inherited destroy
   body must use virtual D1 dispatch, whereas p->B::~B() is qualified/direct.
   The existing fixture destroys an automatic object twice. Rewrite it with
   manually managed lifetime/storage and retain a further-derived control.
   Rename the fixture to describe unqualified virtual dispatch. Do not use
   the existing main as a defined runtime oracle. DTOR now implements the dispatch
   fix and the defined fixture; the copied reducer passes.
   Evidence: pa13/reference-corrections.md and
   student.tests/pa13/audit-explicit-destruction.cpp.

4. Aggregate construction at the final member address (PA15/20).
   The self-pointer aggregate-array reducer and braced-return member-copy
   reducer both return 1. Raw relocation of nontrivial elements breaks identity;
   return {1, first, second} must initialize those aggregate members from the
   lvalues, without introducing additional observable member moves.
   Share typed destination construction instead of fixing fixture-specific
   helpers. Promote the identity/copy-count reducers in PA15 and PA20.
   Evidence: pa15/reference-corrections.md; pa20/reference-corrections99.md.

5. Canonical class-result ABI (PA12/16/18).
   An alias-spelled class result called through its exact function pointer
   faults. A nontrivial empty class factory still returns obj<1x1> instead of
   the required indirect result. Explicit conversion-template calls also
   reject. Class properties and completed types must determine one coherent
   definition/direct/indirect-call boundary; dependent spelling must not.
   Put the simple ABI regression at PA12, retaining PA16/18 dependent cases
   for their additional substitution/conversion coverage.
   Evidence: pa16/result-reference-correction.md;
   pa18/reference-correction87.md and its alias-pointer reducer.

6. Template definition/value checks (PA14).
   The added dependent-bounds.cpp valid input rejects here. Six invalid unused
   sizeof(T) body expressions and one invalid bound declaration are accepted.
   These are additional student controls, not edits to course reference files.
   Their historical first binary is a student entry compiler, so do not label
   all those historical observations as supplied-reference comparisons.
   Our current reproductions independently establish the needed course checks.
   Evidence: student.tests/pa14/check_body_values.py, check_value_queries.py,
   dependent-bounds.cpp, value_query_evidence.py, and PA14's definition-time
   validation contract.

7. Static-member definition demand and discarded reference values (PA17+).
   Merely instantiating a class runs a dormant static initializer (returns 1)
   and rejects an unused T::missing initializer. A discarded volatile-reference
   function call still emits a forbidden volatile load. Separate declaration,
   constant-value and storage demand; preserve the call without reading its
   discarded reference result. Add positive storage-demand controls too.
   Evidence: pa17/storage-references.md; pa18/reference-correction85.md;
   pa19/reference-correction92.md; pa22/reference-corrections119.md.

8. Four justified rejection corrections (PA18/20).
   All four invalid source forms still compile successfully here:
   - noexcept(value()->~I()) where value() may throw;
   - distinct nondependent template result types yielding an ambiguous call;
   - an empty pack initializing an unknown-bound array;
   - implicit closure -> function pointer -> Wrapper conversion.
   Fix receiver effects, definition-time result lookup/signature identity,
   array bound completion and conversion sequence limits. Regenerate the four
   rejection statuses after the implementation is fixed. Keep nearby positive
   controls, such as use(+lambda), explicit Wrapper(lambda), and nonempty packs.
   Evidence: pa18/reference-correction69.md, reference-correction79.md,
   reference-correction83.md; pa20/reference-corrections98.md.

9. Full defaulted template arguments and closure deduction (PA19/20).
   defaulted_pack91.cpp and constructor_closure98.cpp fail their static asserts.
   A ten-argument specialization with seven defaults still has ten arguments;
   its trailing pack has nine, not two. Constructor deduction must preserve a
   lambda's closure type instead of implicitly converting it to a pointer.
   Fix canonical argument completion/deduction. Also correct the PA19
   fixture main's expected pack size from 2 to 9, allowing it to serve as a
   valid runtime check instead of preserving its known-wrong expectation.
   Evidence: pa19/reference-correction91.md; pa20/reference-corrections98.md.

10. Exceptional construction and active-handler lifetime (PA21).
    Nested catch mismatch IR is rejected by our object backend for an active
    region at exit. Aggregate-copy failure leaves completed members live.
    Active-handler temporary, nested mismatch and throwing-cleanup reducers
    return 1. New-expression initialization failure leaks its allocation.
    Fix complete cleanup state/parent forwarding, construction prefixes,
    catch-object lifetime and saved allocation ownership together. Retain
    throwing external companions so dormant exceptional edges are exercised.
    Evidence: pa21/reference-corrections106.md, 108.md, 110.md, 112.md, 113.md;
    final-probes/handler-observations.json and more/observations.json.

11. Member pointers and repeated empty-base identity (PA22).
    The host-built inverse-conversion caller using our call0/call1 object
    returns 1; our source frontend rejects that valid caller. Unknown member
    receivers require the incoming signed adjustment even for a base class
    with no bases. Member-pointer truth must use its target word. Our LowIR
    validator accepts the invalid i128 operand of an i64 comparison. Repeated
    same-type empty bases are both placed at zero (reducer returns 1).
    Fix conversion legality, receiver facts, target-word tests, width checking
    and layout identity. Keep separate-TU unknown-input controls.
    Evidence: pa22/reference-corrections119.md and 120.md;
    final-probes/observations.json; more/observations.json.

12. Virtual-base layout/lifecycle and construction RTTI (PA23).
    A nonpolymorphic virtual-base reference reads a sibling field (returns 1);
    construction RTTI/offset-to-top reducer returns 2; an indirect virtual-base
    mem-initializer rejects; standard null placement wrongly calls a ctor
    (returns 1). Repeated/shared diamond descriptor flags are also zero where
    ABI requires 1/2, even though one repeated-base runtime control passes.
    Fix object-table layout independently of C++ polymorphism, dynamic base
    projections, complete/base lifecycle entries and construction views.
    Do not require a hidden virtual-base argument for a reference/pointer.
    Rewrite 100-constructor-prvalue-virtual-base-forwarding.t so the most-
    derived object's scalar virtual bases are explicitly initialized. Its
    current main reads an indeterminate field; copying an ordinary base must
    not accidentally initialize the most-derived virtual base.
    Evidence: pa23/reference-observation123.md; reference-correction122.md;
    reference-correction125.md.

13. ABI substitution state (PA9/21).
    address_abi_reducer.abi still emits ER1C instead of ERS1_ because entity
    names inside template-address expressions fail to share substitution state.
    The source compiler already matches Clang on the member-address reducer;
    the remaining mismatch is in the PA9 normalized fact tool. PA21's RTTI
    template-template-argument encoding names the wrong substituted template,
    also confirmed against Clang. These need typed ABI encoder fixes and focused
    PA9 coverage, retaining PA21 RTTI coverage for the consumed result.
    Evidence: pa18/reference-correction67.md;
    pa21/reference-corrections102.md; address-abi-main.txt.

## Fixture corrections authorized by the bug evidence

Five existing inputs were identified for source corrections in addition to
regenerated oracles. DTOR gives PA13 explicit destruction manually managed
lifetime. DISCARD-CALL gives PA18
300-explicit-template-call-transitive-base-deduction and PA19
300-deleted-return-sfinae-same-parameter-list real reference backing objects;
the latter main now also passes a real tuple. Their deduction, deleted-overload
and discarded-reference goals remain covered, including the absent referent load.
Two source corrections remain: PA23 constructor-prvalue virtual-base forwarding
needs initialized most-derived virtual bases, alongside the VBASE compiler work;
PA19 defaulted-pack cardinality must compare with 9, alongside DEDUCE's completed
argument facts. Keep each defined source correction with its owning semantic fix.
Compile-only grading explains how these defective runtime bodies went unnoticed;
it is not a reason to preserve them when a defined reducer can retain the goal.

## Other added student tests explicitly outside reference agreement

The static-order, destructor, aggregate, class-result, demand, member-pointer
and virtual-base reducers above were added expressly to distinguish an
unchanged simple fixture's output from its missing semantic obligations.
They are useful regression candidates, not evidence of cheating.

There are also standalone supplied-backend limitations without oracle edits.
A shared-RTTI program passes as a host-linked object here, but our standalone
lowir2native rejects a duplicate native table symbol. Student evidence additionally
records duplicate fundamental RTTI/private labels and freestanding dynamic_cast
limitations. Promote standalone/backend failures at their backend/runtime
owners, while keeping source RTTI controls at PA21/23 through the hosted lane.
The public_base_inside_private_derived reducer now passes both routes here;
that specific historical limitation is already resolved in the current tree.
Evidence: student.tests/pa21/exceptions106.py and validate102.py;
student.tests/pa23/backend-limit126.json; more/observations.json.

PA25 rounding136.md records five direct-double versus extended-intermediate
differences, expressly saying they do not prove a reference bug under C++11's
excess-precision rules. No oracle was changed. Clarify the course's evaluation
policy if needed; do not alter expectations just to follow another compiler.

## Argon report cross-check

Also reviewed `/home/vishvananda/v4argon-reference-bugs-and-issues.md`.
Its historical run says no course fixtures were edited; instead, the student
introduced reference-matching cleanup/elision workarounds. That claim is evidence
from the report, not a fresh audit of an unavailable Argon checkout. These
workarounds do not justify preserving incorrect source behavior here.

Copied the six complete source examples into
`/tmp/cppgm-v4-audit-review/argon/` and ran immutable checkpoint `bc55d6227`
and host GCC at both `-O0` and `-O2`. GCC passes every example. Our observations
match the report; `observations.json` and `host-observations.json` retain them.
The two fenced implementation fragments are not standalone source reproducers.

| Argon issue | Relationship to the v4codex list | Current evidence and action |
| --- | --- | --- |
| 1. Class prvalue bound to a local reference uses the pointer slot as object storage | Additional case; related to lifetime/ABI owners in groups 4/5/10 | Both optimization levels fault with SIGSEGV. Allocate final lifetime-extended object storage separately from its reference binding; add PA12 coverage with two 64-bit fields and a destructor. |
| 2. Single-use literal condition suppresses branch destructor demand and leaks initialized state into the other arm | Additional mechanism within group 10 | Same-type throwing arm returns 233; distinct-type arms reject with a missing destructor binding. Correct reachability/demand and restore branch-local lifetime state independently; retain PA12 ordinary cleanup and PA21 throwing/distinct-type controls. The prior PA21 class-value cleanup fix does not resolve these examples. |
| 3. Empty aggregate-member constructor elision drops argument side effects | Additional case; adjacent to group 4 | Returns 10 instead of 0. Preserve evaluation of every argument (or keep the call), including throwing/temporary-producing arguments. Add the plain counter case at PA11 and later EH coverage only for its additional value. |
| 4. Mixed-class conditional prvalue/lvalue conversion rejects | Additional semantic fix | Rejects compatible TempTracker/Box arms. Implement the conditional operator's bidirectional class conversion rules, with ambiguity, explicit-constructor and category controls at PA12. |
| 5. Aggregate member array of nontrivial class elements uses scalar lowering | Additional case in group 4 | Rejects a local Ext containing Box[3]. Use typed element construction at the final array address; cover local and static objects plus destruction. Earliest ownership follows class-array/aggregate construction, rather than the hosted PA28 location where Argon noticed it. |
| 6. Mutually exclusive large temporary slots never share frame space | Additional optimizer/performance issue | Independent slot-reuse.cpp reproduces 1,639,824 bytes across 64 frames at all three optimization levels, with correct values and 64 destructors; GCC -O1 uses 103,824 bytes. Full original Argon fixture is unavailable. PA33 bounds frame size through explicit expectation sidecars; retain a deterministic backend bound instead of making the report's arbitrary stack budget a language requirement. |

The independent stack reducer is retained in `argon/slot-reuse.cpp`, with all
host-object-linked observations in `slot-hosted-observations.json`. A printf
diagnostic needs host linking; the freestanding driver correctly lacks printf.

The separate multi-block-inline note supplies `cmp slt`, while this repository's
LowIR contract spells it `cmp lt`. The report says the corrected spelling passes
our backend. Record this as a test-dialect discrepancy, not a confirmed inliner
bug or a reason to add an undocumented comparison spelling.

## Later running-checkout refresh

A read-only refresh reached student commit `100de24b` (PA28 loop152); its
uncommitted EH work was left untouched. PA27 `reference-corrections.md`,
`audit148.md`, final `audit.md` (audit150), and PA28 `plan.md` add one reference
correction to the initial review: ordinary global-namespace variable `g` must
use `g`, not `_Z1g`, under Itanium ABI 5.1.2. Two inspection expectations and
two generated inspection outputs changed; source, relocation class and runtime
oracles did not. Our shared variable encoder still prefixes `_Z`, and our two
inspection expectations retain `_Z1g`; track this separately as ABI-GLOBAL.
The student's two-source definition/consumer reducer supplies a host cross-link
control. This belongs to ABI naming, not a reason to move the PA27 object tests.

The later audit also explicitly records a personal `#line __has_include(...)`
control for which GCC accepts an extension beyond its documented conditional
probe contract. That is a host-policy difference, not a changed course oracle.
Our probe rewriter is already called from controlling-expression processing;
our copied controls confirm ordinary-source and `#line` rejection plus conditional
acceptance. GCC accepts the `#line` case. No additional compiler fix is needed
for these controls. PA28's current
plan reports no new reference corrections and still has six implementation
failures; those unfinished student behaviors are not evidence of reference bugs.

## Later read-only student refresh through PA28 audit154

Read-only checkout at 66bb1c6b (PA29 baseline), with clean tracked status. Compared
with the earlier 100de24b review, the only relevant tracked changes are PA28 audit/
plan and the new PA29 plan; no further required tests, reference outputs or
correction documents changed. PA29 implementation is unfinished and its entry
failures are not oracle evidence.

PA28 audit154 reports a student-entry dynamic-exception override defect and 33
personal controls, rather than a reference correction. Independently compile
three fresh declaration-only controls here: an override with throw(double) or
no restriction over a throw(int) base is accepted by ours and rejected by both
Clang/GCC; an equal throw(int) override passes all three. EH-OVERRIDE adds this
confirmed semantic issue to the same tracker. It is distinct from a personal
test explicitly described as differing from the supplied reference compiler:
the student's proof identifies its entry compiler, not that oracle. Commands,
sources and diagnostics live in /tmp/cppgm-v4-audit-review/student-refresh-pa28/.

## Later read-only PA29 handoff156 refresh

Read-only refresh at 5716fcfd; ongoing uncommitted student builtin work is left
untouched. Since 66bb1c6b, the only tracked plan/audit/test/reference change is
PA29 plan.md. Its handoff records 272/403 current PA29 passes, 4538/4538 earlier
checks and 21,987 unchanged fixture files. Neither this diff nor the current
tracked status shows course test/reference edits. Unfinished student builtin
implementation remains separate from supplied-oracle evidence.

The plan explicitly identifies two independent review questions without a proof
bundle or reference correction: 500-builtin-trivial-deleted-copy expects false
for three declaration properties, and
600-hosted-nothrow-default-constructible-shorthand accesses value on std trait
templates that have only forward declarations. HOST-TRIVIAL/HOST-SHORTHAND initially record
these as Needs verification. Independent current-checkout/Clang/GCC probes now
confirm both issues; they are not personal tests explicitly reported by the
student to disagree with the supplied reference implementation. The plan keeps
both required fixtures as failures; no new personal-reference disagreement is
established by this handoff.

Independent evidence is retained in
/tmp/cppgm-v4-audit-review/student-refresh-pa29/observations.json. Exact copied
fixtures pass ours but fail Clang/GCC at -std=c++11 -O0. A separate declaration-
property reducer returns a three-bit mask: ours 0, Clang/GCC 7. N3485 9/6 defines
trivially copyable/trivial classes through their declaration properties; 8.4.2/4
states that an explicitly deleted first declaration is not user-provided, and
12.8/12,25 give the trivial copy/assignment conditions. The shorthand fixture
has no class-template definitions from which value can be obtained; host
errors identify undefined/incomplete instantiations. Compiler/reference fixes
for these newly verified issues remain open.

## Later read-only PA29 handoff157 refresh

The student checkout advances to 76430506 after validated attribute/layout work.
Since 5716fcfd, its only tracked plan/audit/required-test/reference change is
PA29 plan.md; no required source or oracle changed. Handoff157 reports PA1–28
4538/4538, PA29 316/403 and 53/53 new explicit controls, alongside inherited
77/77 and 40/40 controls plus 10/10 inspections. Its performance evidence
retains A/A and ABBA observations and makes no speedup claim. Unfinished atomic,
syntax/type, lifetime, demand/ABI and caller-context work remains implementation
work, not established supplied-oracle defects.

The same two oracle/source questions remain explicit, with no claimed reference
correction or proof bundle: deleted-copy triviality and forward-only nothrow
traits. They are covered by HOST-TRIVIAL and HOST-SHORTHAND here. Searching this
handoff, performance157.md and evidence157/controls157 notes establishes no new
personal test explicitly described as disagreeing with the supplied reference.
Ongoing student implementation is left untouched.

## Clang verification before ABI edits

Per the user's explicit instruction, ABI corrections require an independent
Clang check before changing the encoder or expectations. The checks below use
Ubuntu Clang 21.1.8, target `x86_64-pc-linux-gnu`, `-std=c++11 -O0` and raw ELF
symbol inspection. No mangling implementation or expected symbol was changed
while gathering this evidence. Commands, diagnostics and full symbol listings
are retained in `/tmp/cppgm-v4-audit-review/clang-abi/`.

| Case | Clang | Current checkout | Consequence |
| --- | --- | --- | --- |
| Exact PA27 definition, import and provider for external global `g` | Defines/imports `g` | Defines/imports `_Z1g` | Confirmed mismatch; mixed Clang/ours links fail in both directions. |
| `ns::Holder<&C::m>::f(C&)` source reducer | Ends `ERS1_` | Source compiler also ends `ERS1_` | Source compiler already agrees; do not change this working source boundary. |
| Equivalent PA9 normalized ABI facts | C++ meaning ends `ERS1_` | `abimangle` and its existing reference end `ER1C` | Remaining mismatch belongs to the fact-tool encoding path/reference. |
| Reduced RTTI `O<n::W<n::M>>` | Reuses `n::V` with `S3_` | Uses `S4_` | Confirmed mismatch. |
| Full PA21 `json_encoder<ordered_json>` type with exported address accessor | Reuses vector with `NS4_IhJEE` | Uses `NS5_IhJEE` | Confirms the same RTTI mismatch on the full fixture type. |

Variable boundary controls additionally show that C linkage, namespace data and
static class members already match Clang. Plain external data and external const
data have the global-name mismatch. Internal const/static data remain local ELF
symbols but omit Clang's `L` component; record this spelling difference separately
and consult the internal-name contract before treating it as an interoperability
defect. A global-name fix must preserve these distinct linkage categories.

The full fixture's unchanged `&typeid(...) ? 0 : 1` main does not require Clang
to emit RTTI even at `-O0`. The supplemental accessor forces emission without
changing the type whose spelling is compared. Its original no-symbol result is
retained separately, rather than treated as agreement or disagreement. GCC
corroborates the global and reduced substitution observations, but Clang is the
requested host comparison for subsequent changes.

## Implementation order

First fix initialization/demand and class/aggregate ABI ownership; next EH
cleanup and member/virtual-base layout; then template legality/deduction and
ABI naming. Reconcile representation-only array/empty-value requirements with
handouts and placement detection separately. Regenerate references through the
repository harness after compiler changes, rather than importing the student's
edited oracle bytes. Add defined, runtime-observable reducers at the earliest
feature owner, with hosted companions only where they provide needed ABI/EH
observations. The PA26/27 string rewrites already in this working tree remain
valid and need no extra duplicate move to PA31.

## Later read-only PA29 audit158 and implementation159 refresh

Snapshot HEAD 5d2b1657 changes exactly one checked oracle since 76430506:
500-builtin-trivial-deleted-copy.ref.exit_status becomes EXIT_FAILURE, with all
403 required sources unchanged. reference-correction158.md supplies the pinned
bundle revision, C++11 clause proof and positive/negative controls. This is a
valid student correction to the original false assertions, already covered by
HOST-TRIVIAL here. Our 89a33c0a8 instead corrects the assertions to preserve all
three positive declaration-property goals and keeps the success oracle. Neither
approach requires a compiler to recognize that fixture. The student still leaves
the forward-only std trait oracle unresolved; HOST-SHORTHAND now fixes its goals
here using complete definitions and the ordinary template path.

Audit158 reports 317/403 with 86 remaining PA29 failures, 4538/4538 earlier
checks, zero new failures and 34/34 new controls. Implementation159 later records
321/403, 44/44 code controls and the earlier report still passing; it remains
unfinished, with final performance/handoff evidence pending. No other required
source/reference change or new placement claim appears in the reviewed diff.
The control/proof documents identify student-entry/host comparisons, not a new
personal test explicitly compared against the supplied reference executable.
Reference-lifetime trait controls use “reference” in the language sense.

Two audit158 entry reducers do establish a new current-checkout issue:
using I __attribute__((aligned(1))) = int loses the alias storage alignment;
typedef int I __attribute__((aligned(1))) also loses it through *&i. Both exact
sources reject here at O0/O2 and compile with Clang/GCC. PA29-ALIGN records this
separately; compiler observations and copied sources are retained in
/tmp/cppgm-v4-audit-review/student-refresh-pa29-158/alignment-controls.json.
The running student checkout was read only and was not built or modified.

## Ordinary complete-class exception specification checkpoint

[CWG 1330](https://cplusplus.github.io/CWG/issues/1330.html) adds exception
specifications to complete-class contexts. Ordinary nondependent declarations
now retain their specification syntax and scope until the enclosing class is
complete. This includes unused declarations, nested classes and nondependent
member-template expressions; dependent template expressions remain demand-driven.
A sparse pending table retains only declarations that require deferral. Nested
override restrictions travel with their enclosing completion work instead of
forcing an incomplete outer-class size query. The existing cached specification
query publishes the final binding facts. sizeof(type) also distinguishes an
injected class name from its constructor binding within class scope.

Eight new PA6/12/13/17 fixtures cover ordinary and nested size/type/function
lookup, constructors/destructors/conversions, member-template queries, and
unused-invalid/nested-override rejection. Of 48 final fixture controls at O0/O2,
46 match; GCC rejects the two late-typedef dynamic-specification controls that
Clang and ours accept. CWG 1330 explicitly extends class-name scope to exception
specifications, so the valid fixture remains. The preceding fixture version
without this additional late-typedef declaration matched all 48 controls.
The 87-command matrix has no additional disagreement beyond
the three previously recorded virtual-template timing controls. Those remain
EH-SPEC-TIMING review work; this checkpoint does not infer their correctness
from host acceptance alone or change any associated oracle. Existing references
and ABI spellings are unchanged; only the eight new fixtures use ref-test.

Final validation after the parser/qualifier refinement passes strict 5877/5877
with exactly one total line, PA6/12/13/17 (108/269/50/346), debug-info, variants,
self-host through PA5, all nine architecture targets, file limits, and default
plus focused placement. File audit retains 36 inherited warnings and no errors.
The fact type lives in the model header to preserve the analyzer header limit. Bare fundamental exception lists keep immediate validation using
a fact captured while parsing their type-ids; named and declarator-bearing lists
retain completion work. The deferral helper consumes the already-found qualifier.

Local wall-time ABBA measurements were noisy, including identical-binary
calibration. Alpha provides supported hardware counters. Its isolated immutable
A/A (four blocks) and A/B (eight blocks) comparisons cover frozen recognition
and 1500/6000-pair virtual-declaration inputs. Median instruction ratios are
0.999965, 1.001963 and 1.002021; all fall within the repository's 0.5% instruction
tolerance. Median RSS ratios are 0.999740, 0.999206 and 1.002923. Cycles are
recorded (ratios 0.997959, 1.008360 and 1.011586), with calibration variation;
they are not the repository gate. Every one of the 144 objects is identical for
its input. Local Cachegrind confirms about 0.2% added instruction work on the
largest declaration input. All intermediate observations are retained.

Evidence is retained in /tmp/cppgm-v4-audit-review/exception-spec-timing/ and
/tmp/cppgm-v4-audit-review/perf-complete-class-exceptions/alpha/, including raw
counter/time logs, command and hash manifests, paired summaries and controls.
The isolated Alpha directory is
/tmp/cppgm-v4-audit-review-20261001-complete-class/. Existing checkouts there and
the running student checkout were not modified. Final student export remains
deferred.

## Parameter declarator scope checkpoint

The valid dependent-bounds input reduces to sizeof an earlier array parameter
in a later parameter's bound. BuildParameters had published the unadjusted
array/function type in its declarator scope, while function bodies already used
ParameterBindingType. Reusing that existing query preserves original declared
types for the source dump and applies adjustment plus parameter cv rules to
lookup. PA6 observes declaration types; PA14 observes template instantiation,
function-pointer parameters and preservation of a const parameter object's type.

Before/after and host controls are retained in
/tmp/cppgm-v4-audit-review/parameter-adjustment/ and template-values-final/.
The original broad reducer and seven simpler bound controls isolate the problem.
Only the two new fixtures have generated references; existing references are
unchanged. No encoder change was made. Clang and GCC disagree
on mangling the adjusted-query template itself: Clang uses RAszfL0p__i and GCC
uses RA8_i; name facts require separate review before any ABI change.

## Read-only PA29 audit166 / handoff168 refresh

Student HEAD is 217dc69f; current plan records implementation168 and audit166.
There are no further tracked course fixture/reference edits since 5d2b1657 and
the student tree is clean. Audits162/166 explicitly preserve fixture/reference
discovery and describe corrections to student implementation entry behavior.
Their atomic, assembly, allocation/evaluation and storage findings are not
additional supplied-oracle corrections. The forward-only std trait and false
nothrow-invocable primary questions repeat HOST-SHORTHAND, already resolved here.

The current handoff reports 357/403 PA29 passes with 46 remaining; its aggregate
and mutation controls are implementation evidence. No new personal test was
explicitly described as disagreeing with the supplied reference implementation.
A recorded scan of all 329 changed PA29 personal-test paths finds no such claim;
plan/audit text likewise preserves the oracle questions rather than changing
their expected status. This does not label every student-entry failure as a
reference failure. Existing PA29-ALIGN remains open. Evidence and exact read-only
plan/audit copies live in
/tmp/cppgm-v4-audit-review/student-refresh-pa29-168/.

PARAM-ADJUST validation: all 24 final source/optimization/compiler combinations
compile and return 0 (four inputs, three compilers, O0/O2); the entry compiler
rejects all eight corresponding source/optimization inputs. This includes the
original broad dependent-bounds input. PA6/7/14 pass 109/186/320; strict report
passes 5879/5879 with exactly one line. Debug-info, variants, self-host through
PA5, nine architecture targets, file audit and placement pass. The unused invalid
body/bound controls remain TMPL-VALID work.

Alpha immutable A/A four-block and A/B eight-block comparisons for each of three
frozen inputs pass the instruction/RSS gates. Median instruction ratios are
1.000116, 1.000013 and 1.000006; RSS ratios are 1.000314, 1.000910 and 1.000163.
All 144 objects are identical. Raw counter/time logs, input/binary manifests and
paired summaries are retained under parameter-adjustment/alpha/; the isolated
remote directory is /tmp/cppgm-v4-audit-review-20261001-parameter-adjustment/.
Final student export remains deferred.

## Definition-time fixed expression types checkpoint

N3485 [temp.dep.expr]/4 makes sizeof result types independent of template
arguments; [temp.dep.constexpr]/2 separately permits dependent values. The
retained-template visitor now publishes sparse type/category facts, consumes
source literal facts, and validates known unary/binary/member/assignment/call
operands without computing a dummy size or emitting runtime expression nodes.
Unknown operand types remain deferred. Typed expression tags use an early visitor
dispatch rather than traversing unrelated declaration cases. Simple template declarations also enter
the same declaration-scope visitor, so unused bound operands retain their type
obligations. Existing parameter-shape scopes and frozen ordinary call sets remain
the owners. Baseline compiler and intermediate checks live under
/tmp/cppgm-v4-audit-review/template-definition-types/. Eight PA14 fixtures and
all 22 copied rejection controls pass. Final PA14 passes 328/328; strict report
passes 5887/5887 with exactly one success line. Debug-info, variants, self-host
through PA5, all nine architecture targets, file audit and placement pass.
Existing references are unchanged. A qualified static-member declaration now
inherits its retained owner scope and reuses its predeclared member name,
preserving dependent aliases and initializer lookup without demanding storage.
An initial function-size finding was resolved by extracting the call visitor.

Clang rejects all seven required invalid forms. GCC accepts the unused fixed-call
form take(sizeof(T)) with an int* parameter; the course's definition-time
validation contract requires this check. This unused form has no valid
specialization, so compiler acceptance alone does not establish a GCC defect.
Additional numeric-zero/nullptr controls agree with Clang; GCC also accepts a
character-zero pointer argument which Clang rejects. Known result types carry
no fabricated size values: the valid 5/sizeof(T), dependent operand, shorted
bound and reference-result subscript controls compile and run at O0/O2.

Alpha's final immutable four-block A/A and eight-block A/B comparisons for each
of three frozen inputs pass the instruction/RSS gates. Median instruction ratios
are 1.004012, 1.000034 and 0.999992; RSS ratios are 0.990625, 0.999307 and
1.000198. All 144 objects are identical and the measured candidate hash matches
the validated compiler. Initial recognition measurements were just above the
instruction gate (1.005173 / 1.005145); early expression visitor dispatch reduces
that overhead. All intermediate observations, including the partial failed
scope experiment, are retained separately in alpha-initial, alpha-refined,
alpha-completed and alpha-dispatch. The final remote directory is
/tmp/cppgm-v4-audit-review-20261001-template-definition-types-dispatch/.
Student export remains deferred until the combined final checkpoint.

## Selective static-member definition demand checkpoint

The original PA17 student reducers are independently confirmed: merely using a
class specialization executes an unused effectful initializer and instantiates
an unused T::missing initializer in the entry compiler. Calling a member or a
conversion function has the same effect. Demanding a single static member also
pulls in its unused siblings. Neither follows N3485 [temp.inst]/1,2,8,10.

Retained static definitions now carry their member name. Storage demand selects
the canonical binding, follows its enclosing template owners and queues matching
definition indices through the existing (pattern, name) table. Definition states
prevent duplicate replay; nested routing retains the same name/index facts, and
class reset restores pending states. Late definitions join an existing request.
Owner/function demand alone carries no static-member request. Constant scalar
value queries reuse their existing binding identity instead of repeating name
lookup; known inline constexpr values need no out-of-class storage replay.
Reference conversion explicitly requests storage for its referent. Explicit
class definitions request their static members whose definitions are already
available, while a member specialization retains its own definition.

Eight required PA14/17 fixtures cover dormant invalid/effectful initializers,
selected siblings and dependencies, nested owners with equal member names,
explicit class/member instantiation, extern declarations, and specialized
address use. Template declarations still have one declarator ([temp]/3); two
host-rejected comma declarations remain negative controls. The copied 37 student
storage controls and 20 additional controls are retained separately. Broader
controls expose preexisting static declaration/redefinition/type failures and a
nonzero base-reference failure; STATIC-DECL and STATIC-BASE-ADDR keep those open.
They are implementation-entry observations, not new student oracle edits.

Reference review removes genuinely undemanded static objects and retains needed
storage. It also records constant-load folding and declaration-order changes
caused by selective demand. Only exact changed fixtures are regenerated through
ref-test. No source input or old status is edited, and no ABI encoder changes.
Fourteen existing references were regenerated: ten lose undemanded objects,
two replace a constant load with its known value, and two reorder otherwise
unchanged declarations/functions. All evidence lives under
/tmp/cppgm-v4-audit-review/static-member-demand/. Strict report passes 5895/5895
with exactly one success line. Debug-info, backend variants, self-host through
PA5, all nine architecture targets, file limits and placement pass. Final fixture
and specialized-address controls cover all three compilers at O0/O2; the indexed
path preserves all 114 earlier compiler-control outcomes. Existing negative
static-declaration outcomes remain tracked rather than claimed as passing.

Alpha's original three frozen inputs show no instruction regression. Added
16/64/256-member inputs with identical A/B outputs expose a whole-class rescan
regression in the first implementation: 1.016447 and 1.075249 instruction ratios
for 16/64 members (the 256-member observations are retained too). Indexed pending
work removes those rescans. Final ratios are 1.002607, 1.004926 and 1.004184;
RSS ratios are 1.000646, 1.000449 and 1.000430. Original workload instruction
ratios are 0.999937, 1.000062 and 0.999961, with RSS 1.000150, 0.998348 and
0.999931. All six inputs pass the unchanged 0.5% instruction / 3% RSS gates.
Each input has four A/A calibration blocks and eight A/B ABBA blocks; all 288
objects are identical within their input. Both measured candidate hashes match
the compiler being validated. Raw logs/manifests and earlier experiments remain
in separate alpha-selected, alpha-final, alpha-reviewed, alpha-heavy,
alpha-indexed and alpha-indexed-heavy directories. Student export is deferred.

## Rejection corrections: receiver effects and unknown-bound arrays

Two of the four REJECT mechanisms are corrected in this checkpoint. The original
PA18 source inputs are preserved. Their expected exit statuses were regenerated
through `ref-test`; the obsolete successful empty-array LowIR and success-only
stdout sidecars were removed by the runner. Result-type ambiguity and the
closure-to-function-pointer-to-Wrapper conversion remain open.

The exception-effect walk previously skipped an entire pseudo-destructor node,
including its receiver. It now visits those child actions. N3485 [expr.pseudo]/1
states that evaluating the postfix-expression before the dot or arrow is the
only effect of a scalar pseudo-destructor call; [expr.unary.noexcept]/3 makes a
potentially evaluated call without a nonthrowing specification yield false.
GCC agrees with the correction. Clang 21.1.8 accepts the original scalar assertion
and rejects the new potentially throwing scalar positive assertions, including
template, arrow, dot, indirect, address, comma and conditional receivers. This
is a recorded host disagreement; Clang acceptance alone does not justify the
old course expectation. Runtime receiver exceptions reach handlers on all three
compilers. No ABI spelling or encoder is changed.

A related host-object optimization erased explicit calls to trivial class
destructors even when declared `noexcept(false)`. Elision now also requires a
nonthrowing destructor specification. This preserves the exception facts on the
ordinary typed call. Class receiver/destructor query controls agree with both
hosts at O0/O2, and the LowIR route remains covered independently.

Both ordinary aggregate initialization and the expanded-pack initializer reject
an unknown-bound array completed from zero elements. N3485 [dcl.init.aggr]/4
explicitly forbids this form. The old pack path invented one byte of storage
instead; that exception is removed. Known-bound empty initialization, a nonempty
pack, an empty pack followed by a fixed element, and legal zero-length `new[]`
remain accepted. The allocation control checks that no element is constructed
or destroyed; it makes no assumption about allocation-call counts, which the
language permits compilers to elide.

Seven new required controls are placed at their earliest owners: ordinary empty
array rejection in PA10, zero-length class-array allocation in PA12, pack array
completion/rejection in PA15, scalar and trivial-class noexcept queries in PA16,
and throwing receivers under handlers in PA21. Placement has zero findings.
Fifteen noexcept cases run at O0/O2 with both hosts; all candidate outcomes agree
with GCC, and the scalar Clang disagreement is retained. Eleven array/runtime
cases run at O0/O2 against entry, candidate and both hosts; all candidate compile
and runtime outcomes agree with both hosts. All fourteen new fixture LowIR
compile/reject controls pass; native runtime controls additionally cover the
allocation fixture after the elision-safe correction.

Performance evidence is in
`/tmp/cppgm-v4-audit-review/rejection-queries/alpha/`, with immutable f57a391e1
A/A and final candidate binaries, frozen recognition/virtual inputs, pinned
Alpha user counters, four calibration blocks and eight ABBA blocks per input.
All 144 object outputs agree within each input. Paired candidate/entry instruction
ratios are recognition 0.999983, virtual 0.999999 and large virtual 1.000048;
RSS ratios are 1.000613, 0.998912 and 1.000163. The existing 0.5% instruction
and 3% RSS gates pass. Cycle observations are retained separately rather than
used as evidence of a precise wall-time speedup. Semantic controls, original
fixture host diagnostics, reference commands and validation logs are retained
in `/tmp/cppgm-v4-audit-review/rejection-queries/`.

Full strict report passes 5902/5902 and prints exactly one success line. Debug
info, backend variants, self-host through PA5, every architecture audit, the
compiler file audit and placement all pass. Final combined student export
remains deferred until the unified tracker sequence is complete.

## Rejection correction: definition-time function-template result identity

Trailing `decltype` results were always classified as deferred, even when their
operands used no dependent names or dependent parameters. Their signatures then
compared rendered syntax and reused first-declaration lookup. The declaration
builder now forms fixed trailing result types at definition time, through the
existing typed type-id path. In the unchanged PA18 input, `selected(0)` first
resolves to `long`, then to `int`. The templates have distinct return types and
the final call is ambiguous under N3485 [defns.signature.templ] and
[temp.over.link]. First-declaration lookup applies to dependent names, so it
cannot merge these fixed signatures. The negative PA18 reference was regenerated
through `ref-test`; its original program text is unchanged.

Conversely, different fixed expressions that produce the same return type now
redeclare one template, including a trailing `decltype` paired with an ordinary
result spelling. Duplicate definitions are rejected. The old compiler wrongly
accepted the distinct-result call and a duplicate-definition reducer, and
rejected several valid equal-result declarations.

The dependent comparison also preserves renamed function parameter positions.
It compares retained expression structure under decltype and trailing-return
wrappers, rather than their rendered spelling. Namespace qualification,
parameter positions, parentheses and surrounding operators remain significant.
Both trailing results are compared once; a shared lookup root alone cannot
establish equality. The existing retained lookup remapping preserves the first
call set for an equivalent dependent redeclaration, including renamed parameters.
Template-parameter normalization is bypassed when the two clauses already have
the same names at the same positions. Equal payloads avoid redundant wrapper and
owner checks. A small local syntax-pair worklist avoids allocation on shallow
comparisons and spills to a vector for larger trees; a 64-level dependent result
control preserves acceptance and runtime behavior with both hosts.

The compiler's name facts now retain the concrete type for a fixed result
instead of manufacturing a dependent decltype expression. This was checked
against Clang before changing the publication guard: fixed `decltype(value)`
with an `int` function parameter should emit `_Z6resultIiEiT_i`; the previous
facts emitted `_Z6resultIiEDTfp0_ET_i`. The ABI encoder is unchanged. Fixed-result
object symbols now agree with Clang and GCC.

The symbol controls separately exposed two pre-existing dependent-result issues,
recorded as MANGLE-RESULT. Unparenthesized `decltype(value)` emits `DT` where
Clang/GCC emit `Dt`; `decltype(selected(value))` falls back to a concrete result
instead of retaining the dependent named call. Immutable 1cb054e23 and the
candidate emit the same wrong names. The parenthesized parameter expression
already agrees with both hosts. These observations do not justify changing
unrelated ABI spellings or silently normalizing a bad reference.

Eight new required fixtures cover fixed-result ambiguity, same-type lookup,
spelling equivalence, duplicate definitions, renamed dependent parameters,
parameter positions, distinct wrappers around a shared root and the original
PA18 fixture's qualified positive obligations. Simple signatures live in PA14;
the partial-specialization alias lookup control remains in PA18. All 64 fixture
compile/reject/runtime controls pass at O0/O2, using both LowIR and native routes
and both hosts. Seventeen independent semantic controls run at O0/O2 against
Clang/GCC; all 102 final outcomes agree. The unchanged original PA18 input is also checked at O0/O2: entry accepts, while candidate and both hosts reject as ambiguous. Evidence, exact commands, entry
reductions and object-symbol observations are retained in
`/tmp/cppgm-v4-audit-review/template-result-identity/`.

The first correct comparison passed the ordinary Alpha inputs but regressed the
600-namespace, 3000-equivalent-declaration counter workload by 2.4176%. Removing
comparison allocations and duplicate root walks reduced that to 1.3726%, still
above the gate. Both full observations are retained in `alpha-redeclarations/`
and `alpha-redeclarations-inline/`; neither was accepted as the final result.
The final stable-name and equal-payload paths pass the same gates: recognition,
virtual and large-virtual instruction ratios are 0.999964, 0.999962 and 0.999950;
the targeted declaration ratio is 1.004858. Corresponding RSS ratios are
0.995368, 0.999045, 0.999592 and 1.000995. Four A/A blocks and eight ABBA blocks
per input retain all 192 outputs; hashes agree within each input. Both final
counter manifests match the candidate binary. Final evidence lives in
`alpha-final/` and `alpha-redeclarations-final/`; the initial/intermediate
experiments remain separately retained. Cycles are retained independently from
the instruction/RSS gate and are not reported as an exact wall-time gain.
The final strict report passes 5910/5910 with exactly one success line. Debug
info, variants, self-host through PA5, all architecture checks, the compiler
file audit and placement pass on that final binary. REJECT now has three of
four corrections complete; implicit chaining of two user conversions remains
open. Final combined export remains deferred until all tracker work is complete.

## Conversion sequences and closure constructor deduction checkpoint

An implicit converting constructor now requires a standard conversion for its
first argument. The special captureless-lambda exception, its fake deduction
arguments and the nested conversion-function recipe fields are removed. Class
copy initialization rejects scalar-to-constructor and closure-to-pointer-to-
constructor chains. A copy/move construction action may still follow a conversion
function that produces the destination class; this separate construction step
is valid and fixes CONV-IMPLICIT's conversion-function-template reducer.
Direct construction and ordinary constructor arguments under list initialization
retain their valid user conversions. A broad initial list restriction was caught
by positive controls and removed before validation.

Constructor-template deduction consumes the actual closure type. Both
captureless and capturing lambdas preserve that type, while a function-pointer
parameter cannot deduce it through conversion. Unary plus and explicit
construction remain valid. The original PA20 implicit Wrapper source is
unchanged and now rejects, as Clang/GCC do. The original preferred-constructor
source is also unchanged: its reference now passes the closure and invokes its
call operator, retaining the value 7. Exact references were regenerated through
ref-test. Before accepting the resulting ABI metadata, object controls checked
the closure constructor against Clang: both emit
`_ZN4SinkC2IZ4mainE3$_0EET_`. The ABI encoder is unchanged. GCC uses its own
lambda discriminator spelling.

The pointer conversion function is now nonthrowing independently of the lambda
call operator. This follows the defect clarification in
[CWG 1722's adopted wording](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0164r0.html),
rather than a guarantee stated in the original N3485 text. Hosted trait controls
check constructibility, nonthrowing construction and the invalid implicit
conversion separately. Both hosts agree at O0/O2.

Nine required fixtures live in PA12, PA18 and PA20 at their earliest owners.
Twenty-six boundary programs produce 156 agreeing compile/runtime controls
across candidate, Clang and GCC at O0/O2. The nine fixtures supply 72 passing
LowIR/native/host controls. Sixteen original-fixture controls preserve the
rejection and closure-deduction evidence against immutable entry 88f5d368b.
Twenty-four additional controls identify the independently pre-existing mixed
constructor/conversion-function ambiguity recorded as CONV-SELECTION; neither
entry nor candidate diagnoses it, so it is not claimed as fixed.

Alpha uses immutable entry/candidate binaries, CPU 0, three frozen inputs, four
A/A calibration blocks and eight A/B ABBA blocks per input. All 144 output
objects agree within their input. Instruction ratios are recognition 1.000026,
virtual 1.000023 and large virtual 1.000127; RSS ratios are 0.995552, 0.998974
and 1.000034. The unchanged 0.5% instruction / 3% RSS gates pass, and the
manifest matches the validated candidate. Cycles are retained separately.
Controls, exact reference commands, full validation and raw performance
observations are retained under
`/tmp/cppgm-v4-audit-review/conversion-sequences/`.

Full strict report passes 5919/5919 with exactly one success line. Debug-info,
backend variants, self-host through PA5, all nine architecture checks, file limits
and placement pass. The layout audit initially identified two stale PA20 comment
allowlist entries; those obsolete entries are removed and the audit passes. The defaulted-pack portion of DEDUCE and CONV-SELECTION remain open.
Final combined student export remains deferred until all tracker work is complete.

## Defaulted class arguments and trailing-pack deduction checkpoint

The actual class specialization already stores every canonical argument,
including omitted defaults. The function pattern also acquired a synthetic
default suffix after its symbolic expansion. Deduction incorrectly anchored
that suffix and excluded matching defaults from the pack; explicitly writing
the same defaults did not help. A trailing expansion now consumes every
remaining actual argument, as N3485 [temp.deduct.type]/9 requires. The old
suffix trial, repeated dependent queries and deduction copies are removed.

Typed arguments retain whether an expansion has a written suffix. This fact is
published during the existing argument construction and forwarded with symbolic
packs. It participates in interned shape identity, so `box<T,Ts...>` and
`box<T,Ts...,marker>` cannot collapse after defaults are appended. A written
suffix makes the entire argument list non-deduced, preserving valid explicit
arguments or deduction from other call arguments. The added flag fits existing
padding: entry and candidate TemplateArgument are both 40 bytes. Concrete
canonical argument identities remain unchanged.

Five PA19 fixtures cover completed type and dependent integral defaults, aliases,
base deduction, repeated consistent/inconsistent packs, written suffix rejection,
explicit arguments and separate deduction. Sixteen boundary programs produce
96 candidate/Clang/GCC controls at O0/O2; all compile/reject/runtime outcomes
agree. Forty fixture LowIR/native/host controls pass. The original ten-parameter
PA19 fixture now compares its nine-element trailing pack with 9 instead of 2.
Sixteen controls preserve the old and corrected sources: entry returns 0 on the
old wrong expectation and 1 on the correction; candidate and both hosts do the
reverse. References are generated through exact ref-test selection.

Object names were checked against Clang before accepting the reference changes.
The concrete function template argument list now contains all nine pack elements,
matching Clang/GCC. A separately pre-existing declared-parameter fact defect
remains MANGLE-PACK: the exact Clang/GCC name is
`_Z4takeIiJiiN6tuples9null_typeES1_S1_S1_S1_S1_S1_EEiRKNS0_5tupleIT_DpT0_EE`;
our parameter suffix instead flattens the expansion and appends defaults. The
entry already lacks `Dp`, so this is not a reason to change the ABI encoder
without reviewing publication of the source pattern. Full original object
symbols and host commands are retained in original-fixture-controls.json.

Evidence lives in `/tmp/cppgm-v4-audit-review/defaulted-pack-deduction/`. Strict
report passes 5924/5924 with exactly one success line. Debug-info, backend
variants, self-host through PA5, all nine architecture checks, file limits and
placement pass. A targeted 600-namespace workload exercises variadic and fixed
class deduction with identical entry/candidate output, complementing the three
standard frozen inputs. All four unchanged gates pass. Instruction ratios are
recognition 1.000045, virtual 0.999955, large virtual 1.000027 and targeted
deduction 1.000429; RSS ratios are 0.998575, 0.998117, 1.000000 and 0.997954.
Each input retains four A/A calibration blocks and eight A/B ABBA blocks; all
192 objects agree within their input, and both manifests match the validated
candidate. Cycles are retained separately from the instruction/RSS gate. All raw
observations remain in alpha/ and alpha-deduction/. Combined student export
remains deferred until the full tracker sequence is complete.

## Static-member declaration checkpoint

Immutable d65f8b02e reproduces the eight STATIC-DECL failures at O0/O2.
Canonical binding facts now distinguish static member declarations from storage
definitions. An explicit specialization without an initializer declares the
member; a later definition clears storage suppression. Duplicate ordinary or
specialized definitions, nonstatic members and incompatible types reject.
Retained class declarations index direct data-member and nested-owner facts.
Definition checks form a canonical type only when its identity is known,
preserving renamed parameter shapes and equivalent aliases. Unknown dependent
member types remain checked during concrete replay; unused initializers remain
undemanded. Duplicate checks inspect only the existing indexed group for that
member and compare the complete owner shape.

All 37 agreed boundary inputs and 19 new required fixtures agree with Clang/GCC
at O0/O2, including runtime positives. The extra declaration-after-definition
control agrees with Clang but GCC rejects it as a redefinition; no required
oracle was added for this disputed case. The fixture controls preserve their
original sources and use exact ref-test generation. Ordinary static members
belong to PA11, basic template declarations to PA14, specialization/non-type
array bounds to PA15 and partial-owner selection to PA17.

The first strict run exposed five eager alias-proof regressions in PA22/27.
These now pass individually: dependent member-pointer owners and inaccessible
member types defer to normal replay. A first six-input Alpha run retained all
288 observations and equal output objects, but failed the instruction gate
on the static-heavy inputs (ratios 1.00748, 1.01624, 1.01238). Retained class-name
inventories now avoid walking the complete class source for every out-of-class
member declaration while preserving the original validation scope and name
kinds. Final validation passes: strict report 5943/5943 with exactly one success line,
debug-info, backend variants, self-host through PA5, all nine architecture
checks, file limits, test-harness and placement (zero findings). The placement
rule now uses the existing parsed template-header facts for pointer/reference
NTTPs; it no longer lets a regexp cross into a following pointer declarator.
Four false positives disappear while actual pointer/reference/member/function
NTTP controls still pass. Two PA11 controls use ordinary classes; the original
unused nonstatic template case has separate PA14 coverage.

The final immutable run uses four A/A calibration blocks and eight A/B ABBA
blocks for each of six frozen inputs. All 288 output objects agree within their
input and the manifest matches the final compiler. Instruction/RSS ratios are:
recog 0.997452/1.000749, virtual 0.999961/0.998876, virtual-large
1.000089/0.986546, static-16 0.999607/0.997709, static-64
0.950077/1.000745 and static-256 0.770482/0.999373. All A/A and A/B gates pass;
cycles and every preliminary observation are retained separately. No general
speedup is inferred. The canonical definition bit fits the existing 136-byte
BindingRecord. Final measurements are in alpha-final/.
The initial failures and subsequent controls remain in
`/tmp/cppgm-v4-audit-review/static-member-declarations/`; student export remains
deferred until the full tracker sequence is complete.

## Static base-reference verification

The unsigned-free A/B/D reducer is now isolated by seven variants, each compiled
with ours, Clang and GCC at O0/O2 (42 commands). Both the write and address checks
fail for the static reference here; a runtime local reference passes. Returning
the byte offset directly gives 0 for our static reference and 4 for both hosts;
our ordinary derived-to-base pointer conversion correctly returns 4. LowIR
confirms `global @ref = addr @value` while the pointer conversion adds 4.
ApplyTarget already computes the base projection offset, but updates the static
address only when ProjectConstexprObject also produces a constant object value.
These independent facts must be kept separate. That verification accompanied
STATIC-DECL; the following checkpoint records the fix. Controls and LowIR
remain in /tmp/cppgm-v4-audit-review/static-base-address/.

## Static base-address projection checkpoint

ApplyTarget, ApplyMemberObjectTarget and explicit pointer casts now project
address facts independently of constant object values. Ordinary conversions use
the existing typed base offset; inverse conversions subtract it. Known complete
object identities provide the layout for virtual conversions, including arrays
and direct class members. A conversion whose complete layout cannot be proved
clears the source address fact and retains its runtime operation. Null explicit
pointer casts publish a null address without changing reinterpret_cast behavior.

Expanded controls also exposed a separate null runtime pointer bug: virtual-base
adjustment loaded the vptr before the ordinary pointer guard. The existing
nullable adjustment helper now performs the virtual offset load only in its
nonnull branch, including projection through a virtual anchor to a nested base.
Reference, this and address-of conversions preserve their nonnull paths.

Thirty-eight boundary inputs agree with Clang/GCC at O0/O2 (228 commands),
including ambiguous/private/inverse-virtual rejections and defined runtime
positives. All 42 original controls now agree, including the direct byte-offset
probe returning 4. Five required fixtures live at their earliest owning features:
nonvirtual multiple inheritance in PA22:100 and virtual-base projection in
PA23:100. Their 30 compiler/runtime controls agree with both hosts; ten repeated
candidate controls also pass on the final layout-proof binary. Exact ref-test
commands generate every new reference. Existing references remain unchanged;
strict report passes 5948/5948 with exactly one success line. Debug-info,
backend variants, self-host through PA5, all nine architecture checks, file
limits and placement (zero findings) pass on the final binary.

The initial five-input Alpha measurement passed, but an additional unchanged
primary-base/virtual-owner input exposed a 1.005734 instruction ratio, exceeding
the unchanged 1.005 gate. The source binding's root identity and declared type
now prove when the conversion's layout already matches the complete object,
avoiding the extra virtual-path walk and its temporary storage. All initial
observations and the failed gate are retained. The final six-input run uses
immutable entry/candidate binaries, CPU 0, four A/A calibration blocks and eight
A/B ABBA blocks per input. All 288 objects agree within their input; every A/A
and A/B instruction/RSS gate passes. Instruction/RSS ratios are recognition
0.999979/0.995687, virtual 1.000005/0.998847, large virtual 0.999966/0.999373,
base addresses 1.001907/0.999207, runtime base conversions 1.000408/0.999934
and primary-base/virtual-owner addresses 1.001685/1.000513. The manifest matches
the final compiler hash ebe7a62408ab3b9ceab8d23f1127075e69679dcb6f6fe80f6d5ef528a19a9e1a.
Cycles are retained separately; no general speedup is claimed.

Evidence, earlier validation trials and every raw counter observation remain
under /tmp/cppgm-v4-audit-review/static-base-address/. Combined student export
remains deferred until the full tracker sequence is complete. CONV-SELECTION
is the next semantic work item; the remaining virtual-layout/lifecycle issues
remain VBASE.

## Read-only PA29 audit174 / active handoff176 refresh

A new snapshot at student HEAD 1b19e9ac18eba368b76e1b6389e5126fe5f60a5a
captures audits170/174 and active implementation176. There are no tracked
course fixture/reference edits. Both audits explicitly preserve every required
input, sidecar, discovery and comparison rule. The forward-only std trait and
false-primary invocable questions repeat HOST-SHORTHAND, already addressed here;
they are retained student failures, not new oracle corrections. An audit control
had a hand-calculated sum corrected; this is a personal-control correction.

Audit174 retains a preliminary pack-grouping ABI regression and reports its
final correction. Its NTTP pack comparison uses GCC and Clang's
-fclang-abi-compat=17; current default Clang additionally encodes template
parameter declarations. This evidence belongs with MANGLE-PACK and must be
checked against the requested Clang policy before any ABI change here. It does
not establish another supplied-reference bug. Exact audit/plan copies, source
HEAD/status and their hashes remain in
/tmp/cppgm-v4-audit-review/student-refresh-pa29-176/. Final student-export
validation remains deferred until the complete tracker fix sequence is finished.

## Class copy-conversion selection verification

Immutable 3bcfffacc supplies 38 boundary inputs and 228 O0/O2 commands.
Clang and GCC agree on 37 inputs. Twenty fail here: sixteen required
rejections are accepted, while four valid inputs call the constructor instead
of the conversion function selected by both hosts. Direct, direct-list and
copy-list construction still select their expected constructor; same-class
and derived-class copying, explicit controls and the two-user-conversion
rejection supply positive and negative boundaries. The derived-result
conversion-versus-constructor case rejects in Clang and selects the constructor
in GCC; no required reference has been added for that disputed case.

The competing source conversion is treated as a user conversion to a copy
constructor's parameter in SelectConstructor, so the ordinary converting
constructor incorrectly wins as a standard argument conversion. CallConversion
also picks a converting constructor over a separately selected conversion
function. An ambiguous constructor or conversion-function group can disappear
when its helper returns an invalid fact, allowing a losing group to win.
These initial candidates need to compete using the source argument conversion
to each constructor parameter or conversion function's implicit object
parameter, as specified by N3485 8.5 and 13.3.1.4. The later direct construction
from the selected conversion result also requires a valid copy/move action: a
deleted copy/move constructor must reject even when copying could be elided in
C++11. Existing direct construction and list-initialization paths must retain
their own candidate rules.

At this verification checkpoint no compiler change or reference rewrite had
been made for this item.
Evidence and the initial classification live in
/tmp/cppgm-v4-audit-review/class-copy-selection/. Ordinary conversion functions
and class value semantics belong to PA12; template candidates additionally
require PA18. The tracker remains active for this item and all later open work.

## Class copy-conversion selection implementation checkpoint

Class copy initialization now ranks converting constructors and source
conversion functions in one candidate set. Each constructor uses the standard
conversion to its first parameter; each conversion function uses its implicit
object parameter. Ambiguous groups remain present during selection. Source
cv/ref qualifications, constructor defaults, ordinary/template preference and
template ordering participate before the selected action is built. Direct and
list initialization retain their constructor rules. Inherited conversion
functions use the source class for selection and their actual declaring class
for the receiver projection; a hiding declaration still hides the base
conversion when the hiding declaration is explicit.

The selected constructor's result must permit the final direct copy/move
construction required by C++11, even when elision is possible. Deleted final
copies also cause substitution failure during template deduction. Completed
implicit trivial move facts prove the common valid final construction without
forming additional constructor-template candidates. This avoids extra
instantiations and their presentation names. Ordinary initialization clears the
call-argument staging flag instead of allocating an unused argument object.

The expanded boundary set has 51 programs. Clang and GCC agree on 50, and the
candidate matches those 50 at O0/O2 (300 host/candidate commands). The one
derived-result constructor/conversion disagreement remains excluded from
required fixtures. Selection follows Clang here: final-result conversions break
ties between two conversion functions, rather than between a constructor and a
conversion function. Clang 21.1.8's
[SemaOverload implementation](https://github.com/llvm/llvm-project/blob/llvmorg-21.1.8/clang/lib/Sema/SemaOverload.cpp#L10046)
confirms that distinction; its inherited-conversion treatment at lines
7688-7691 also confirms the implicit object class used for selection.

Twenty-one new required fixtures cover PA12 copy legality and conversion
selection, plus PA18 conversion templates and substitution. All 126 fixture
compiler/runtime commands agree with Clang/GCC at O0/O2. Exact ref-test commands
generate all new sidecars and three reviewed existing references. Those three
changes only rename internal LowIR functions and their uses: the PA18
empty-middle-pack constructor, PA18 implicit conversion-template copy and PA19
current-specialization constructor. All 24 entry/candidate/Clang/GCC runtime
controls pass. Their ABI names are unchanged. Clang's constructor aliases agree;
the conversion-template name still differs from Clang as recorded in
MANGLE-CONV, and this checkpoint does not alter that encoder or expectation.

The seven-input Alpha run uses immutable entry/candidate binaries, CPU 0, four
A/A calibration blocks and eight A/B ABBA blocks per input. Every one of the
336 observed objects agrees within its input; every A/A and A/B instruction
and RSS gate passes. Instruction/RSS ratios are recognition
1.000008/0.999741, virtual 1.000008/1.001538, large virtual
1.000000/0.999827, constructor copies 0.984093/1.000033, conversion-function
copies 0.973391/1.001389, constructor-template copies 0.981192/0.998039
and competing conversion candidates 0.982157/0.999439. The counter manifest
matches compiler hash
7fc7aee9d338781d07c24a098135a16ae8dadd039162c84669a05ae6a3b1667e.
Cycles are retained separately. All controls, exact reference commands,
validation logs and raw observations remain under
/tmp/cppgm-v4-audit-review/class-copy-selection/. Final combined export remains
deferred until the tracker sequence is complete.

Final validation passes: strict report 5969/5969 with exactly one success line,
debug-info, all backend variants, self-host through PA5, all nine architecture
checks, file/function limits and placement with no early-placement findings.
The failed PA12 log from an earlier compiler trial exposed the remaining
successful-control summary leak recorded in HARNESS-FAIL; no harness code was
changed in this compiler checkpoint.

## Failed-report control output checkpoint

The report now exports CPPGM_REPORT_QUIET to its assignment subprocesses. All
39 focused Perl controls suppress their successful summary in that mode while
retaining their failure diagnostics. Ordinary explicit controls still print
their summary. This prevents a successful PA12 survivor check from appearing
beside unrelated fixture failures when the report prints a failed assignment's
log. The report does not filter diagnostic text by guessing which words denote
errors.

The seven report-output tests pass, including an actual focused control beside
a failed comparison in both output orders and under the sanitized student
Makefile. An actual failing focused control retains its diagnostic. PA12's
13 survivor controls also pass with the real compiler/backend in ordinary and
quiet modes, with the expected summary in the former and empty output in the
latter. All 39 producers pass Perl syntax checks; make test-harness passes;
strict report remains 5969/5969 with exactly one output line. The compiler hash
still matches the Alpha conversion-selection performance manifest, so this
output-only change does not require another compiler performance run.

Evidence remains under /tmp/cppgm-v4-audit-review/report-failure-output/.
The root Makefile sanitizer retains the quiet export statement and introduces
no missing script dependency; the focused scripts retain their shipped paths.
Full fixture discovery, reference bundle and combined student-export validation
remain deferred until all tracker fixes are complete.

## Active-handler full-expression lifetime checkpoint

Refreshed immutable 2481b326d evidence reproduces six failures among the seven
original EH reducers at O0/O2; Clang and GCC pass all seven. The aggregate
prefix companion is made definition-compatible with its source: its extra move
constructor declaration/definition is removed from the copied host control.
The original student evidence remains unchanged. This corrected two-TU control
still proves the aggregate prefix failure, so the discovery does not depend on
the original definition mismatch.

Full-expression cleanup staging previously returned early if there were no
local objects to unwind, even when a temporary had a live catch context. Its
landing prefix then ended the catch before destroying the temporary. Return,
condition and control-expression paths also appended local unwind actions
directly, bypassing the existing typed handler boundaries. They now reuse
StageExceptionalFullExpression, preserving temporary destruction before each
required handler exit. Automatic initializer staging also reaches that helper
inside a handler without requiring an additional local object. The exception
and cleanup context identities remain the existing typed facts.

An initial broad handler-only staging trial regressed ordinary rethrow and
nested forwarding. The final guard adds a handler boundary when the expression
is already staged, or when local unwind obligations require staging. A simple
rethrow without these obligations keeps its handler's own cleanup region.
Failed trials are retained. Nine agreed boundary inputs now pass at O0/O2
(54 compiler/runtime observations); the two additional nested-handler/inner-try
inputs still reject during object generation and remain part of EH forwarding.
The five other original failures remain EH: nested mismatch (two reducers), throwing local
cleanup, aggregate prefixes and failed-new deallocation.

Two new PA21:200 fixtures cover return, statement, initializer and condition
boundaries, normal completion, and both conditional temporary arms. Explicit
exception copy constructors count live objects regardless of copy elision;
the conditional control checks balanced construction/destruction rather than
prescribing an optional copy count. All twelve candidate/Clang/GCC fixture
observations pass at O0/O2, and all four Clang/GCC controls with copy elision
disabled also pass.

The existing handler-context fixture changes only its dormant exceptional
cleanup continuation and the resulting local block names. Exact ref-test
commands regenerate that reference and the two new fixtures. A controlled
copy of its generated LowIR retains choose and its cleanup blocks, externalizes
read and moves the fixture entry aside. The host companion throws a class
exception for value 7 and a long for value 11; the class exception destructor
appends 3. Required order gives 813. Entry fails at O0/O2; the candidate passes
at both levels. Clang confirms the injected read symbol; all ABI identities
remain unchanged. The known global-variable spelling issue remains ABI-GLOBAL.

The nine-input Alpha run uses immutable binaries, frozen sources, CPU 0, four
A/A calibration blocks and eight A/B ABBA blocks per input. All 432 objects
agree within their input; every A/A and A/B instruction/RSS gate passes.
Instruction/RSS ratios are recognition 0.999993/0.999932, virtual
0.999985/0.996955, large virtual 1.000053/1.000035, constructor copies
0.999999/0.999372, conversion-function copies 0.999984/0.999288,
constructor-template copies 1.000017/1.000812, competing copies
1.000013/0.997711, ordinary EH returns 1.000476/0.999737 and handlers without
temporaries 0.999986/1.000198. The counter manifest matches compiler hash
e5dfb62ab187c76cbce6faea1ecb7561c6be9924441c8d064fa3597241c28dc9.
Cycles are retained separately; no general speedup is claimed.

Evidence, retained trial logs, controls and every raw observation remain under
/tmp/cppgm-v4-audit-review/exception-lifetimes/. Combined student export stays
deferred. The next EH work is complete nested catch forwarding and lifetime
boundaries before the construction-prefix and allocation ownership work.

Final validation passes: strict report 5971/5971 with exactly one output line,
debug-info, all backend variants, self-host through PA5, all nine architecture
checks, file/function limits and placement with zero findings. No other
existing fixture or reference changed.

## Nested catch forwarding boundary verification

Immutable ac2aaf704 supplies eighteen defined nested-handler inputs and 108
O0/O2 compiler/runtime observations. Clang and GCC pass all eighteen. Fifteen
fail here: six abort instead of reaching the dynamically enclosing handler,
five violate lifetime/order observations, and four fail object generation with
unbalanced protected regions. Three positives pass: a matching inner handler
keeps its enclosing guard alive, the matching path through an active outer
handler preserves its caught object, and an ordinary typed rethrow keeps that
exception alive for the caller's matching handler. Two external throwing
companions prove the failures without relying on a locally visible throw body.

The abort set includes a plain nested int/long mismatch without any class
objects, and a three-level match that skips two inner handler types. The current
catch landing records its own clauses but does not generally advertise the
enclosing matching clauses. Cleanup-bearing and active-handler dispatches also
need to retire their retained region before ordinary source matching begins.
After a real miss or handler cleanup, the forwarded path must destroy its
complete object prefix, finish intervening handlers in lifetime order, and
retire the enclosing try before jumping to its catch entry. Catch-all miss
edges need separate treatment because matching is exhaustive while the O0
branch skeleton still participates in region validation.

The native analysis distinguishes cleanup-bearing landings from catch-only
landings and currently tolerates some catch-entry joins by adopting the
landing edge's region state. That tolerance does not establish the explicit
source-lowering exits required by the LowIR contract. The lowering already
has typed exception-region and handler identities; the fix should derive its
dispatch and forwarding facts from those identities and the existing child
inventory, preserving ordinary matched paths and avoiding rescanning rendered
LowIR or speculative source analysis.

This entry verification preceded the implementation recorded below.
Source/compiler hashes, immutable entry binary, companions and all observations
remain under /tmp/cppgm-v4-audit-review/nested-catch-forwarding/. EH and all later
open rows remain active; the final combined export is still deferred.


## Nested catch forwarding and complete handler prefixes

The lowering advertises the current try's handler clauses followed by enclosing
tries, stopping at the first catch-all. This lets phase-one host unwinding find
a dynamically enclosing match even when an inner typed catch misses. The single
try-child inventory records whether forwarding has cleanup obligations in the
existing region state; entry and candidate states both occupy 16 bytes. A
cleanup-bearing dispatch explicitly retires its retained try before ordinary
source matching. Real misses and escaping catch bodies destroy their typed
prefix, finish intervening handlers, and retire the enclosing try before its
catch entry. Full-expression cleanup inside a handler now forwards to its
nearest enclosing try after its boundary actions finish.

Semantic prefix actions stop at the nearest enclosing try, rather than replaying
all function objects at every nested miss. They share the existing segmented
full-expression cleanup builder, so each handler boundary follows its own local
objects. The indexed lifetime walk also normalizes its exclusive stop through
the same nearest-obligation index: an empty lexical stop must not allow the
walk to destroy a live object outside the try. Four additional controls expose
this case with matching, missing, local-prefix and handler-throw paths; entry
fails all four and Clang/GCC keep the outside guard alive.

A catch-all's impossible miss successor exits enclosing regions before resume.
Normal try, handler and conditional continuations use existing incoming-edge
bookkeeping, preventing an unreachable try end from creating a spurious
non-void fallthrough or unbalanced protected return. The combined temporary
fixture retains both conditional paths rather than avoiding this case.

By-value controls also expose unnamed catch-parameter ownership. Named catch
parameters already have ordinary destructor actions; an unnamed copied
parameter must be destroyed before end_catch when its handler is crossed.
Boundary actions retain the active handler's ordinal in the existing action
value field, already included in cleanup action identity. Lowering uses that
typed position to destroy the right unnamed parameter, including two active
by-value handlers. This adds no handler lookup table or allocation. Native
region validation and its acceptance rules are unchanged.

Forty boundary programs pass with our compiler, Clang and GCC at O0/O2
(240 observations). They include ancestor matching, matched-path preservation,
external throws, active typed/catch-all handlers, rethrow lifetime, throwing
handler bodies, empty lexical stops, named and unnamed catch parameters, and
two active catches. Five final policy controls also verify noexcept nested
matching, allowed dynamic-specification escape and constructor function-try
forwarding; all agree with Clang/GCC. Six new PA21 fixtures have 36 additional
passing controls:

- 100-source-nested-catch-ancestor-dispatch.t
- 200-source-nested-catch-prefix-order.t
- 200-source-nested-handler-lifetime-forwarding.t
- 200-source-nested-handler-temporary-forwarding.t
- 200-source-exception-empty-scope-boundary.t
- 200-source-unnamed-catch-value-forwarding.t

All eleven prior expanded handler controls also pass (66 observations), including
the two nested temporary controls left open by EH-HANDLER-TEMP. Three existing
fixture runtime controls pass (18 observations). Their references are regenerated
through exact ref-test commands: nested-catch-miss-cleans-active-handler adds
balanced dispatch/handler exits; source-catch-miss-cleans-outer-scope advertises
the outer match and destroys its guard on handler escape; source-handler-branch-
call-cleans-outer-scope retains cleanup on the raw handler escape as well as the
staged branch-call route. No other existing reference changes. New fixture
function object names are checked against Clang; no ABI encoder or existing
object spelling changes. The known ABI-GLOBAL issue remains independent.

The final compiler hash is
4bb13f38a42e51028e11e4c1bd351a38b644cb570392b7f5dde648e8b8e53444.
Alpha uses immutable A/AA copies of ac2aaf704 and final B, identical frozen
inputs, CPU 0, four A/A and eight ABBA blocks per input. All 432 observations
succeed with byte-identical objects. Every instruction ratio is <=1.005 and
RSS ratio <=1.03; median B/A instruction/RSS ratios are:

| Input | Instructions | RSS |
| --- | ---: | ---: |
| recognition | 1.000005943 | 0.999740891 |
| virtual overrides | 0.999936829 | 0.997461525 |
| large virtual overrides | 0.999949345 | 1.000312219 |
| copy constructors | 1.000038776 | 1.000727445 |
| conversion functions | 1.000034767 | 1.000050683 |
| conversion templates | 0.999983075 | 1.001155249 |
| competing conversions | 1.000058344 | 0.998566572 |
| ordinary EH returns | 1.000065191 | 0.998037572 |
| handlers without temporaries | 1.000044892 | 1.001554236 |

Final measurements are perf-final/ locally and
alpha:/tmp/cppgm-v4-audit-review-20261001-nested-forwarding-final/ remotely;
earlier trial binaries and measurements are retained separately. The measured
B hash matches the current compiler. All required checks pass: strict
5977/5977 with exactly one output line, debug-info, backend variants, self-host
through PA5, all nine architecture checks, file/function limits (36 inherited
warnings), and placement with zero findings.
The refreshed seven original EH reducers pass four cases; throwing local
cleanup, aggregate prefixes and failed-new deallocation still fail here and
pass Clang/GCC at O0/O2. Those remain EH; later rows and final combined export
remain pending.


## Lexical cleanup boundary verification after EH-FORWARD

Immutable d59b47c5b supplies nineteen defined controls and 114 O0/O2
compiler/runtime observations. Clang and GCC pass all nineteen. Seventeen
runtime controls fail here, one rethrow control is rejected semantically, and
one positive passes. The runtime failures cover scalar/void/shared return,
normal handler and nested-block exit, break/continue/goto both out of and
within a handler, two active handlers, an unnamed by-value handler, a match in
an enclosing try, and throwing destruction of an outside guard after the catch
has ended. A nonthrowing return also observes premature destruction of the
caught exception, so this is not confined to destructor escape.

The passing outside-guard control requires its guard's destructor to run after
end_catch. Handler-owned local objects require the opposite order. Moving all
end_catch calls after a flat destructor list therefore cannot satisfy both.
Likewise, keeping a handler active during a throwing local destructor is
insufficient: every remaining local destructor must run before the active
exception is released. A return from a try body must preserve that try's own
handlers while destroying its locals, and publish the remaining local prefix
if one destructor throws before source matching.

BeginExceptionControlExit currently closes the top handler before the flat
return/goto list. Structured jumps and ordinary scope exits leave it active,
but their direct destructor calls lack the remaining-object cleanup tail. The
semantic lifetime and source control-region facts should publish precise
normal boundaries and unwind obligations; lowering should share those tails
through the existing cleanup continuation interner. Keep catch parameters,
return storage/NRVO ownership, the current exception context and each exact
remaining tail in that identity. Avoid source reanalysis or speculative
per-destructor suffix rebuilding. The current region validator must continue
to require balanced exits. EH-CLEANUP owns this next implementation.

The separate rethrow control defines throw; in a destructor called while an
exception is being handled. Two ordinary helper-function controls confirm the
same rejection independently of lexical cleanup. All three reject here at
O0/O2 with "rethrow outside an exception handler"; Clang/GCC compile and run
all twelve host configurations successfully. The source
analyzer tests exception_handler_depth_, which counts lexical handlers in the
function definition. N3485 15.1/8 describes reactivation of the currently
handled exception, and 15.1/9 makes executing an operandless throw without an
active exception call terminate at runtime. The current compile-time lexical
restriction cannot establish that dynamic condition. EH-RETHROW-DYNAMIC
records this additional defect; it is separate from cleanup-tail ordering.

No compiler, required fixture or generated reference changes were made for
these two new rows. Sources, immutable compiler/hash manifest and observations
remain under /tmp/cppgm-v4-audit-review/local-cleanup/. Initial malformed scratch
authoring trials were corrected before the final host-agreed boundary set;
those trials remain separately named and are excluded from the evidence above.
The successful EH-FORWARD compiler checkpoint, its full checks and performance
measurements remain unchanged. Other open rows and the final combined student
export stay active.


## EH-CLEANUP implementation progress

The uncommitted candidate publishes shared semantic lifetime prefixes with
object, try-exit and handler-exit facts. Normal destruction retires each
source region at its lexical boundary; throwing destruction follows the exact
remaining prefix before source matching. Named and unnamed by-value catch
parameters participate in the same lifetime model, including ordinary handler
fallthrough. The cleanup interner retains the prefix identity, current source
context and constructor/destructor body continuation. A second exception during
remaining-object destruction routes to terminate.

Thirty-two independently host-agreed runtime programs pass the fifth candidate
at O0/O2: eighteen original boundary controls, seven additional scope/handler
controls, and seven catch-parameter/NRVO/base-lifetime controls. The latter
include named and unnamed catch parameters whose destructor throws, an
abandoned NRVO return, and constructor/destructor local cleanup before base
cleanup, both with and without function-try handlers. The separate dynamic
rethrow reducer remains rejected; EH-RETHROW-DYNAMIC is unchanged.

The full fifth strict report passes 5972/5977. The five remaining differences
are existing PA21 LowIR references, with no remaining runtime failures in that
report. Ordinary constructor/destructor bodies keep their previous entry
shape; only bodies with throwing lexical cleanup publish a detached body
cleanup continuation. No references have been regenerated. The broad trial
that added these continuations to every body was rejected and is retained only
as scratch evidence. Two further frozen controls independently fail the entry
compiler and pass Clang/GCC: an ordinary function-template function-try body,
and a source unit using imported potentially throwing destructors. The former
needs the same semantic source-region facts during demanded-function emission;
the latter verifies runtime preparation without source throw/catch syntax.

The second trial's nine Alpha inputs pass all instruction/RSS gates with equal
objects (432 observations, AA calibration plus paired ABBA). Those measurements
are preliminary and do not certify the final compiler. The final immutable
candidate will be measured again. DumpNode remains 152 bytes; each shared plan
is 16 bytes and each lifetime obligation grows from 16 to 20 bytes. Owner
preflights pass; a large return dispatcher is being split into its existing
scalar/reference result operations to keep the file/function audit clean.
Final fixture/reference review, architecture/debug/variant/self-host checks,
performance verification and the commit remain pending. Final student export
is still deferred until all tracker fixes are complete.


## EH-CLEANUP final checkpoint

The final compiler preserves lexical ownership during return, shared return,
fallthrough, nested block exit, break, continue and goto. It closes handlers
after their owned locals and catch parameters, and before outside objects.
Potentially throwing destruction publishes an exact shared remaining tail;
constructor/destructor body continuations retire their landing frame before
joining that tail. Existing try boundaries handle bare destructor calls
without redundant wrappers. The action interner includes the lexical plan
identity, so a full-expression representative cannot silently erase a normal
cleanup tail. Runtime preparation covers imported destructors in a unit with
no source throw/catch. Ordinary demanded member function-try bodies retain
the same semantic source-region facts as ordinary function bodies.

Thirty-four defined runtime programs agree with Clang/GCC at O0/O2 (204
observations). Seven new required fixtures consolidate those boundaries into
six PA21 LowIR inputs and one PA28 hosted terminate-handler program. Their
42 compiler/link/runtime observations all pass. The five potentially affected
existing fixtures pass all thirty host/candidate runtime observations; only
three require reference regeneration:

- 200-destructor-body-unwind-runs-base-destruction shares a retired entry
  continuation between local destruction and base cleanup.
- 200-source-exception-empty-scope-boundary retires the source try before
  destroying an object owned by an outside scope.
- 200-source-unnamed-catch-value-forwarding gives unnamed copied catch
  parameters ordinary object storage and a lifetime obligation; failure of a
  later construction cleans up that completed catch copy before end_catch.

The three references and seven new fixture bundles were generated through
exact ref-test selections. The other two destructor references retain their
previous bytes after removing redundant wrappers. No fixture expectations
were weakened. Forty earlier forwarding programs and eleven handler-temporary
programs pass all 306 repeated observations. Five of the seven original EH
reducers now pass; aggregate construction prefixes and failed-new deallocation
remain unchanged failures and remain EH.

All required checks pass: strict 5984/5984 with exactly one output line,
debug-info, backend variants, self-host through PA5, all nine architecture
checks, file/function limits (36 inherited warnings), and placement with zero
findings or review cases. DumpNode is still 152 bytes, a shared plan is 16
bytes, and a lifetime obligation is 20 bytes. All defined mangled function
names in the seven new fixture objects agree with Clang; the explicit
terminate/set_terminate imports agree as well. No ABI encoder changes were made.

Final Alpha measurements use immutable d59b47c5b and the final candidate, CPU 0,
identical frozen inputs, four AA calibration blocks and eight paired ABBA
blocks per input: 432 retained observations, all outputs equal. Every paired
instruction ratio stays below 1.005 and every RSS ratio below 1.03:

| Input | Instructions B/A | RSS B/A |
| --- | ---: | ---: |
| copy-competing | 1.000068552 | 0.999768073 |
| copy-constructors | 1.000318332 | 0.997682794 |
| copy-functions | 1.000311699 | 1.000680150 |
| copy-templates | 1.000238348 | 1.000263773 |
| eh-handlers | 1.000398131 | 0.999684891 |
| eh-returns | 1.000264239 | 0.999160075 |
| recog | 1.000024439 | 0.999849672 |
| virtual | 0.999962906 | 0.999339482 |
| virtual-large | 1.000002260 | 0.999764749 |

The measured B SHA-256 is
b1ec05004d614ec51747b82d977f64f4fbf93ebcd53c842371cc14b23e7754db,
which matches the compiler after all validation. Authoritative measurements
are local-cleanup/perf-empty-tail-final/ locally and
alpha:/tmp/cppgm-v4-audit-review-20261001-lexical-cleanup-empty-tail-final/
remotely. Earlier trial snapshots and their measurements remain separately
named. Frozen input hashes, Clang symbol observations and the checkpoint
verification are retained under /tmp/cppgm-v4-audit-review/local-cleanup/.

Three further boundaries remain explicitly separate. EH-RETHROW-DYNAMIC still
rejects its three valid dynamic rethrow programs. TMPL-FTRY loses an ordinary
function-template function-try definition before demanded-body emission.
EH-ARRAY-DTOR skips remaining elements in the three-element unrolled path,
while its twelve-element loop positive passes all compilers. No required
oracles were changed for these open failures. EH-RESULT-CLEANUP records three
additional non-NRVO returned-object controls: current Clang agrees with this
compiler's failure, while GCC destroys the already initialized result.
[CWG 2176](https://cplusplus.github.io/CWG/issues/2176.html) specifies that
returned-object cleanup after a return-time destructor exception; its wording
was adopted after N3485. Keep the course-policy decision and host disagreement
visible before changing that contract. The passing NRVO control remains a
separate requirement for a named local that already has a lifetime obligation.
Other tracker rows and the final combined student export remain pending.

## Aggregate construction-prefix baseline after EH-CLEANUP

EH-AGG-PREFIX is the next sequential compiler fix. The immutable baseline is
72a55cd47, with compiler SHA-256
b1ec05004d614ec51747b82d977f64f4fbf93ebcd53c842371cc14b23e7754db.
Sources, companions, runner versions and hashes are retained under
/tmp/cppgm-v4-audit-review/aggregate-prefix-cleanup/. The student's checkout
and original audit reducers remain unchanged.

Twenty-four construction controls compile and link with the baseline,
Clang 21.1.8 and GCC at O0/O2. The complete runner observes every configured
throw point, instead of stopping after the first leaked lifetime. Its 144
observations are retained in entry-complete-controls.json. All 48 baseline
executions fail the live-object check; all 96 host executions pass it.
Twenty-three controls also have identical host traces. The remaining
nested-custom-destructor control differs only in whether the completed nested
aggregate's own destructor body runs. Earlier 90 and 54 observations remain
retained as entry-controls.json and entry-boundary-controls.json; their
runners stop at the first failure. entry-verification.json verifies frozen
hashes and distinguishes agreement from the disputed trace.

The agreed failures cover flat and nested member initialization, member arrays,
arrays of aggregates, omitted/defaulted members and elements, direct braces,
scalar initializers after and between members, a twelve-element array, a
two-dimensional array, active source handlers, aggregate results, a completed
array element with a custom destructor, no preceding local object, successive
aggregate declarations, constructor member initialization, conditional
destinations and distinct branch prefixes. Conditional destinations use
permitted C++11 copy elision differently: the baseline performs two additional
member copies on its successful path. That normal trace difference is not
recorded as another compiler bug. The failing live-object check remains valid
independently of whether those copies are elided.

The original reducer currently invokes member copy constructors directly at
their final addresses, as required by AGG-DEST. Each call receives the same
full-expression unwind suffix for the surrounding locals. Successful members
never enter that suffix. The array and recursive aggregate walkers likewise
lack a completed-member prefix. Moving the fixture or changing its expected
result cannot repair this failure.

The mixed temporary controls constrain the implementation more tightly than
a separate member-cleanup list. In temporary-and-final-scalar, member 3
finishes, temporary 4 is constructed, and member 6 finishes before scalar
initializer 7 throws. Both hosts destroy 6, 4 and 3, then the surrounding
locals. If initializer 5 or member constructor 6 throws instead, temporary 4
precedes member 3 during cleanup. A blanket rule placing all temporaries
before all completed subobjects is therefore incorrect. Normal completion
still destroys the temporary at the end of the full expression and leaves
the completed aggregate alive until its owning scope ends.

The implementation must record successful construction transitions in one
ordered prefix, retaining typed destination, type, destructor and ownership
facts. Nested completion must transfer member obligations to the completed
subobject without duplicating destruction or losing pending temporaries.
Untaken branches, retired temporaries, later scalar calls and empty lexical
suffixes need that same prefix. It must join the existing full-expression
dispatcher before outer lexical/handler cleanup; a separate enclosing handler
that inner cleanup bypasses would still lose the members. Equal tails must
retain their full source exception context and terminal continuation, with
incremental sharing rather than copying every preceding member at every call.
Destructor demand must respect completed class ownership. Lowering must not
fabricate semantic nodes to reuse an existing destructor path.

EH-AGG-NESTED records the separate host disagreement. When the third outer
member's copy constructor throws, Clang calls the completed inner aggregate's
custom destructor and then its member destructors. GCC calls only the member
destructors. The equivalent completed array-element control invokes the custom
destructor with both hosts. N3485 15.2/2 describes completed subobjects through
their principal constructors; [P0490R0, US 28](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0490r0.html)
explicitly extends the deemed-constructed rule to completed aggregate
initialization. [CWG 2227](https://cplusplus.github.io/CWG/issues/2227.html)
also discusses aggregate element destructors and exceptional cleanup. Keep
these later wording changes visible when deciding the course oracle; no
required fixture or reference was changed at this baseline checkpoint.

Three nonthrowing aggregate inputs with 32, 128 and 512 nontrivially
destructible members are frozen for an additional performance gate.
performance-entry.json records successful baseline object and LowIR emission
and output hashes. They complement the previous nine equivalent-output
workloads and will expose unnecessary cleanup machinery on the nonthrowing
path. Alpha's instructions:u and cycles:u counters were verified accessible;
the actual A/A and A/B measurements await a compiler candidate. No compiler
code changed in this evidence checkpoint, so the prior validated compiler
and 5984-test strict result still identify the entry binary. Final combined
student-export validation remains deferred until all fixes are complete.

## Aggregate construction-prefix candidate and expanded boundaries

The candidate retains successful subobject and temporary completion in one
persistent lowering prefix. Each step carries a typed destination, object type
and retained destructor binding, or an existing temporary cleanup identity.
Nested completion transfers member obligations to the completed object while
retaining initializer temporaries. Normal temporary retirement removes that
obligation before calling its destructor. Conditional joins combine the arms'
existing guarded temporary obligations instead of retaining only the last
lowered arm. The prefix precedes the existing lexical/handler suffix, whose
cache key also retains source exception context and constructor-body terminal.
No synthetic semantic nodes, source reparsing or ABI encoder changes are added.

Semantic construction recipes cache their nonthrowing and cleanup requirements
at construction analysis. Destructor demand remains in the completed-owner
scheduler. The nonthrowing path keeps the previous lowering, and the new flags
fit existing DumpNode padding: its size remains 152 bytes. A retained arena fact
requests the terminate helper only when a cleanup destructor can throw. Early
trials used unresolved destructor specifications and produced four unnecessary
helper/reference changes; those changes were removed before accepting any
oracle. The sole existing reference correction is PA21
200-indirect-param-prologue-copy: a failure in the second or third member copy
now destroys the completed aggregate members before the surrounding locals.
The original three member constructors still receive their final addresses.

Evidence remains under /tmp/cppgm-v4-audit-review/aggregate-prefix-cleanup/.
The immutable candidate is compiler-eighth, SHA-256
1aed930df76736a69356974296f97ceb9c961f58b3009975f45c5cb5c6c2e446.
eighth-verification.json checks 37 host-agreed construction programs at O0/O2,
three separately disputed programs and two separately failing boundaries.
The original 24-program corpus, ten extended boundaries and eight further
boundaries are frozen with source/companion manifests. Conditional destinations
permit different C++11 copy elision, so their live-object and exception checks
remain authoritative without requiring identical copy-event traces. All other
agreed controls have identical Clang/GCC/candidate traces. Twelve further
second-fault observations reach the installed terminate handler with the
expected still-live outer objects. Their first trial lacked an outer handler;
the resulting uncaught-exception termination did not guarantee unwinding and
was not used as cleanup evidence. Both trials are retained.

Nine freestanding source fixtures in PA21 cover flat members, nested member
arrays, a matrix, interleaved temporary/scalar initializers, a long conditional
temporary prefix, constructor member arrays, conditional destinations with
temporaries, template ownership and a short-circuit initializer. No hosted
headers are needed. Thirty-six independent Clang/GCC executions validate their
live-object and agreed trace checks. All eighteen baseline fixture executions
fail; all eighteen candidate executions pass. References were generated only
for these new fixtures and the independently reviewed existing fixture.
The strict report is one line, 5993/5993. Debug-info, backend variants,
self-host through PA5 and all nine architecture audits pass. Placement reports
zero findings. Compiler validation is retained in validation-eighth/; its first
file-audit run failed on a false whole-class function span, detailed below.

perf-eighth/ retains the final Alpha counter run, immutable A/AA/B compilers,
frozen inputs, manifests and all 576 observations. Twelve inputs each have four
A/A calibration blocks and eight paired A/B blocks. Every output object agrees
within and across compilers. All instruction and RSS gates pass: the largest
A/B instruction increase is below 0.067%, and the largest RSS increase is below
0.31%. Cycles and every individual observation are retained. The additional
32/128/512-member nonthrowing inputs also retain byte-identical LowIR and object
outputs against the entry compiler. The earlier fourth-candidate run remains
separate provisional evidence and is not substituted for the final hash.

The expanded tests deliberately retain four distinct follow-ups:

- EH-AGG-NESTED now also records a completed nested aggregate with an interleaved
  initializer temporary. Clang and the candidate destroy the completed inner
  object before the independent temporary; GCC flattens the inner members and
  interleaves that temporary between them. No disputed expectation was added.
- EH-AGG-TEMP-DTOR records normal-temporary-destructor-throws. The corrected
  host companion permits the temporary destructor to throw after successful
  aggregate initialization. GCC and the candidate destroy both aggregate
  members; Clang leaves two live objects. The first companion had a duplicated
  noexcept(false) spelling and failed host compilation; it remains trial
  evidence, not a compiler failure. No disputed required oracle was added.
- EH-CTOR-HANDLER records constructor-inner-try. An initialized constructor
  member remains live when an inner source handler rethrows. The frozen entry
  and candidate fail identically; both hosts destroy it. This escape bypasses
  the constructor suffix and is separate from completed aggregate members.
  EH-COND-THROW separately records a class/throw conditional rejected by the
  entry and candidate. Its retained semantic shape is a raw temporary and
  throw operand, whereas destination lowering expects normalized arms.
- EH-SPECIAL-PREFIX records the later synthesized-copy boundary. The original
  conditional fixtures cover every aggregate initializer throw point; their
  configured bound does not include additional permitted copies. Two extended
  all-steps companions derive their bound from the observed successful path.
  Those expose one leaked member when a later synthesized member copy throws.
  Direct implicit-copy-prefix and implicit-move-prefix controls reproduce that
  failure in the entry and candidate independently of conditional joins; both
  hosts pass at O0/O2. Do not claim the original fixed-bound controls establish
  correctness of these later memberwise constructors.

Six of the seven original EH reducers now pass at O0/O2; failed-new remains
failing, as recorded independently. The missing-history rename-manifest audit
also writes an expected Git diagnostic before reporting success; HARNESS-AUDIT
retains that cleanup separately. Other tracker work and the final combined
student export remain pending.

The whole-class file-audit finding was stale signature state from a short
constructor initializer list. Both audit scans now retire declaration-scope
state before discarding an earlier inline body. A large-class test passes and
an actual 243-line member still fails. This corrected scan also exposes eight
pre-existing oversized functions. file-audit-entry/ contains immutable copies
from 72a55cd47, with hashes; file-audit-entry.log reports exactly the same
eight failures as file-audit-scope-fixed.log in the candidate tree:

| Owner | Function | Lines |
| --- | --- | --- |
| lowering/objects/storage_slots.h | CollectSlots | 317 |
| lowir/driver/stats_report.cpp | ReportOptimizer | 517 |
| lowir/optimize/small_object_promotion.cpp | promote_small_objects_impl | 275 |
| native/lowering/indexes.h | emit_index | 377 |
| semantic/extensions/hosted_builtins.cpp | AnalyzeBuiltinTypeTrait | 297 |
| semantic/extensions/range_for.cpp | AnalyzeRangeFor | 253 |
| semantic/object_model/inheritance_analysis.cpp | AnalyzeCast | 371 |
| semantic/templates/function_deduction.cpp | DeduceFunctionTemplatePackType | 320 |

ARCH-FUNCTION retains these required refactors. The detection correction is
kept, and the 240-line function limit is unchanged. Full file-audit success is
pending those repairs; do not describe this checkpoint as fully validated.
The constructor's formatting cleanup leaves the rebuilt compiler byte
identical to compiler-eighth, so its complete runtime and Alpha evidence
still identifies the current compiler. Final student export remains deferred.

### Function-size audit repair checkpoint

ARCH-FUNCTION is complete. The eight functions listed above now meet the
unchanged 240-line limit. Their existing responsibilities stay in the same
compiler owners: declared storage and array-new slots, optimizer reporting,
proven object rewriting, materialized native indexes, builtin shape traits,
range-for body/index setup, scalar casts, and named template-pack deduction.
Slot planning replaces five parallel vectors with one typed visit worklist,
preserving traversal and slot order. Five existing control-flow fact records
move from Analyzer into semantic/analysis/control_flow_facts.h; analyzer.h is
2365 lines. Semantic and lowering symbol ledgers include the extracted owners.
No fixture or reference changes accompany these refactors.

Evidence is retained in /tmp/cppgm-v4-audit-review/function-audit-repair/.
The immutable entry compiler is 1aed930df76736a69356974296f97ceb9c961f58b3009975f45c5cb5c6c2e446;
compiler-first is 0645718d1c63cd31a7af17bd23ea3acc8a85204af1c081d8f6ba2ab3764367f6.
entry-manifest.json retains source hashes. A frozen stats probe sets 347 scalar
fields and all entries in three matrices; both path variants match the entry
output exactly. control-comparison.json checks 210 compile/link/run observations
against the aggregate checkpoint, including known independent failures and
second-fault termination. All statuses and output are unchanged.

validation-first/validation.json records passing strict report (5993/5993,
one final output line), debug-info, backend variants, self-host through PA5,
all nine architecture audits, complete file audit and feature placement.
The complete file audit passes with 37 advisory warnings and no errors. This
closes the outstanding audit gate for EH-AGG-PREFIX without claiming any of the
separate EH rows are fixed.

perf-first/ retains all 576 Alpha counter observations across twelve frozen
workloads, four A/A and eight A/B paired blocks per workload, compiler/input
hashes and exact object equality. paired-performance-gate.json passes the
0.5% instruction and 3% RSS gates. The largest A/B instruction increase is
0.0041%; the largest RSS increase is 0.148%. Cycle observations are retained alongside the counter calibration.
Final combined student export remains deferred until the other fixes.

### Quiet historical-baseline audit checkpoint

HARNESS-AUDIT uses git rev-parse --verify --quiet to probe the historical tree,
capturing its normal stdout. Only the expected missing-object exit status is
accepted without history; signals and other Git failures terminate the audit
and preserve their diagnostics. The focused harness now requires empty stderr
for a successful missing-history checkout and verifies that a non-repository
fails visibly. Its five controls and the complete make test-harness pass.
The live rename-manifest audit passes with zero stderr bytes; strict 5993/5993
prints only its final total. Evidence is in
/tmp/cppgm-v4-audit-review/rename-history-quiet/. No compiler binary or student
oracle changes occur in this checkpoint. Final combined export remains pending.

### Conditional throw normalization checkpoint

EH-COND-THROW is complete. Evidence is retained in
/tmp/cppgm-v4-audit-review/conditional-throw-normalization/. The immutable entry
compiler is 0645718d1c63cd31a7af17bd23ea3acc8a85204af1c081d8f6ba2ab3764367f6;
compiler-third and compiler-fourth are byte-identical at
4a38b8559df13cb9b6c5829972bc612e3c8404a8dde3e8e137408812bf202fb0.

The semantic owner recognizes materialized class prvalues despite their
internal xvalue storage category. Actual reference casts/calls retain their
categories. The existing destination-ready conditional arms are reused.
Lowering pauses cleanup bookkeeping for a nonreturning class arm, so a sibling
completion cannot inherit its open segment and emit a spurious eh_end at the
join. Throw lowering restores staged cleanup before invoking the exception
runtime after a conditional split; scalar and void controls demonstrate the
previous leak of a live enclosing object.

fourth-controls.json retains 200 observations for 25 programs at O0/O2 using
the entry, candidate, Clang and GCC. Thirteen programs pass all candidate/host
runs. Each branch is exercised through every observed throwing step, plus a
nonthrowing run and a threshold beyond the last step; selected branch,
exception value, payload and live-object counts are checked. Six new PA21
fixtures implement the affected constructor, aggregate-with-nothrow-copy,
scalar and void cases, and are independently compiled and run with all three
compilers at O0/O2 (36 passing runs). The initial fixture runner used an
unsupported -x driver option; its failed trial is retained separately, and the
valid runner freezes byte-identical .cpp inputs with hashes. References were
generated through ref-test; no existing tracked oracle changes.

The wider controls retain three synthesized-copy failures as
EH-SPECIAL-PREFIX. Nine reference-initializer failures are now EH-REF-INIT:
seven actual lvalue/xvalue/reference-call/const-reference controls and two
lifetime-extended aggregate-reference controls leak prior local objects in
both entry and candidate. Both hosts pass. StageAutomaticInitializerException
currently excludes reference declarations; the repair must preserve the
extended backing object's scope lifetime rather than turn it into an ordinary
full-expression temporary.

fourth-regression-verification.json checks the previous 210 observations. The
only status changes are the original conditional-aggregate-throw-arm control:
it now compiles, links and passes its fixed-bound runtime at O0/O2. Its extended
all-copy-step failure remains EH-SPECIAL-PREFIX. All other statuses and output
are unchanged, including the independent constructor-inner-handler failure,
second-fault termination and failed-new failure.

validation-fourth/ and validation-checkpoint.json record strict 5999/5999
(one final output line), debug-info, variants, self-host through PA5, all nine
architecture audits, the full file audit and zero placement findings. All pass.
Thirteen Alpha instruction/RSS gates pass in perf-third/ across 624 retained
observations, with exact object equality and matching compiler hashes.
The initial run and a focused 288-observation repeat showed positive cycle
ratios on EH handlers and conditional references. Those trials are retained.
perf-third-interleaved/ then records 576 observations: twelve paired blocks
for each of three workloads, alternating A/A, B/B and two independent-image
A/B comparisons with a frozen randomized order. Every object is equal and
instruction/RSS gates pass. Combined A/B cycle estimates are -0.200% for EH
handlers, +0.207% for conditional references and +0.226% for recog. All three
paired bootstrap 95% confidence intervals include zero change. The earlier
increase does not reproduce across both images and the interleaved windows;
retain the observed image/calibration variation rather than attributing a
precise cycle change to this source fix. The exploratory subtraction of A/A
and B/B ratios is not the primary estimate: those are different image pairs,
not an unbiased common timing offset. Final combined export remains deferred.

### Synthesized construction prefixes — validated checkpoint

The entry is e11367e6e, with frozen compiler
4a38b8559df13cb9b6c5829972bc612e3c8404a8dde3e8e137408812bf202fb0.
Evidence is under /tmp/cppgm-v4-audit-review/synthesized-construction-prefix/.
The final compiler-eleventh is
d913a284cc1f67a176c4c8fe35f84d4e67e4c8f057d75c75a183e646928c8ec7.

Semantic synthesis publishes typed destructor bindings and cached construction
throw effects. Representation-copy prefixes retain lifetime-only subobject
steps, including trivial-copy classes with nontrivial destructors. Nonthrowing
constructors keep the fast path. The packed binding and flag leave DumpNode at
152 bytes; two Analyzer helpers have explicit semantic-owner ledger entries.
Lowering retains completed scalar/class/inline-array subobjects in the existing
construction prefix. Counted member arrays retain their completed-element index
and enter the remaining cleanup in the same constructor before resuming.
Ordinary constructors with array cleanup use retired entry continuations too.
Unwind destruction does not install a normal array-destructor recovery loop:
a second exception reaches the terminate handler immediately. Distinct guard
contexts get distinct terminal landings. Assignment controls remain unchanged.

entry-expanded-controls.json retains 324 observations for 54 programs. All
216 Clang/GCC observations pass and have identical traces. The final candidate's
108 observations match those traces exactly. These include direct and custom-
destructor bases/members, nested objects, scalar/reference members, inline and
counted arrays, matrices, defaulted constructors, representation-copy prefixes,
empty subobjects, unions and four assignment controls. Both original copy/move
reducers and both original extended conditional all-step reducers pass all 24
candidate/Clang/GCC observations. Seven second-fault controls pass all 42
observations, checking the still-live objects in an installed terminate handler.
eleventh-runtime-verification.json also confirms no status/stdout changes among
102 candidate observations corresponding to the previous 210-observation EH
corpus. Its existing constructor-inner-handler and failed-new gaps remain open.
All three expanded conditional implicit-copy failures are fixed at O0/O2; the
nine reference-initializer failures remain EH-REF-INIT.

Fourteen freestanding PA21 fixtures cover the construction boundaries. Every
candidate O0 standalone execution and every Clang/GCC execution passes.
Two full-TU array fixtures crash in our O2 routes, with host linking as well as
standalone linking. The frozen entry reproduces both crashes; corresponding
external-companion controls pass. This is explicitly BACKEND-ARRAY-OPT, with
entry-host-array-fixture-controls.json and debugger observations retained.
The PA21 handout states the completed-subobject and containing-array cleanup
requirements. Its existing conditional-aggregate reference gains only the
previously missing first-member destruction when the second member copy throws.
No fixture input was weakened to conceal that failure.

The two affected PA11 array-lifecycle fixtures exercise normal counts and
values. PA11's handout permits direct noexcept metadata on constructors and
destructors. Their nonthrowing bodies now state that property explicitly, so
these ordinary lifecycle tests do not acquire incidental EH scaffolding.
All 48 frozen-entry/candidate/Clang/GCC observations for the before/after inputs
pass. Only their two references and the reviewed PA21 conditional reference
were regenerated, together with the fourteen new fixture references.

validation-eleventh/ records seven successful gates: strict report, debug-info,
backend variants, self-host through PA5, all nine architecture audits, file
audit and placement. The strict report contains exactly one line, 6013/6013.
The file audit passes with the same 37 warnings. No early placements remain.
perf-eleventh/ retains 672 Alpha hardware-counter observations across fourteen
frozen workloads, immutable A/AA/B binaries, A/A calibration and paired ABBA
blocks. All objects are byte-identical. Every instruction/RSS gate passes;
the largest A/B instruction median is +0.0077% and the largest RSS median is
+0.1525%. Cycle medians range from -1.082% to +0.298%. The added workload covers
256 nonthrowing synthesized class-array copies. No ABI encoder spelling changed.

Intermediate builds and failures remain available. A temporary environment
restriction interrupted build-sixth and left an empty build lock; process
inspection proved that no build owned it before removal. compiler-sixth was
verified byte-identical to compiler-fifth, so those observations are labelled
as measurements of that older image in sixth-stale-build-verification.json.
The resumed build has an explicit successful exit record and fresh binary hash.
A scratch second-fault output-name collision was corrected; the complete entry
baseline remains in entry-expanded-controls.json. None of these intermediate
results substitute for final validation. Final student-export validation remains
deferred until all tracker items are complete.

### Constructor/destructor source-handler cleanup checkpoint

EH-CTOR-HANDLER and EH-DTOR-HANDLER share a body-unwind continuation.
The N3485 15.2/2 rule covers both failed construction and destruction, including
completed delegating targets; 15.2/3 requires termination if unwind destruction
throws again. This checkpoint adds no ABI spelling change.

Independent evidence is retained under
`/tmp/cppgm-v4-audit-review/constructor-handler-cleanup/`:

- `entry-manifest.json` freezes `2eaf1c4f0` and the entry compiler
  `d913a284cc1f67a176c4c8fe35f84d4e67e4c8f057d75c75a183e646928c8ec7`.
  `entry-expanded-controls.json` retains 276 observations for 46 programs,
  Clang/GCC/entry and O0/O2. The entry fails 39 programs; seven swallowing,
  normal-completion or ordinary body controls succeed.
- All 92 final boundary observations agree with the 184 retained host
  observations, including exact destruction traces. Controls cover handler
  rethrow/miss/replacement, external throwing calls, local objects, nested
  handlers, function-try blocks, bases, custom member destruction, delegating
  targets, nine-member bodies and three/twelve-element arrays.
- `sixth-runtime-verification.json` checks 274 earlier regression observations.
  The two original constructor-inner-try observations change from leaking to
  correct cleanup; every other prior status and trace stays unchanged.
  Existing EH-REF-INIT, failed-new and other independently tracked gaps remain.
- Twelve freestanding `200-inner-handler-*.t` fixtures use destruction-trace
  checksums derived from matching independent Clang/GCC traces. All 72 final
  host-object observations pass at O0/O2. `entry-fixture-final-controls.json`
  retains the same fixture inputs on the frozen entry.
- Five second-fault programs agree on the expected termination exit 77 across
  candidate/Clang/GCC and both optimization levels. The miss case exposed a
  catch-all selector collision: termination guards previously reused selector 1
  and could inherit the type of a source catch. Unique handler identities fix
  that collision; constructor and destructor unwind actions now share guarded
  lowering, with already-unwinding array destruction stopping immediately.

The semantic owner records body source-try presence from the existing typed
exception contexts, and body cleanup when exceptional full expressions are
staged. The function-control-flow snapshot saves and restores that body fact
across nested function demand. Nonthrowing local destructors can still require
an enclosing member-cleanup suffix; the old throwing-only body flag missed
those paths. `DumpNode` remains 152 bytes and the saved control-flow state
remains 144 bytes. No additional AST scan or per-node allocation is introduced.

Lowering advertises cleanup for source-handler misses, dispatches inner source
tries before the member suffix, and invokes the member suffix before a
function-try handler. Full-expression continuation keys include the precise
body terminal. Inline member-array failure also continues through the earlier
constructor subobjects. Body-cleanup emission is a separate non-inlined helper
so this uncommon case does not expand ordinary dispatch code.

Five existing PA21 references were regenerated through the built reference
wrapper, without editing their input programs or reference text by hand:

- `200-source-lexical-body-unwind-tail`: four dormant full-expression tails now
  enter member/base cleanup before leaving a constructor/destructor or reaching
  its function-try handler; direct fault controls establish the missing cleanup.
- `200-source-lexical-handler-return-order` and
  `200-source-lexical-handler-scope-order`: termination guards and source catches
  receive distinct selectors, as independently required by the second-fault
  miss control.
- `200-destructor-subobject-unwind-runs-later-base-destruction` and
  `200-destructor-unwind-shares-generated-suffix`: the unwind bodies include
  guarded subobject destruction and its demanded terminate helper. Ordinary
  destructor-exception sequencing stays the same.

`entry-existing-controls.json` and `fifth-existing-controls.json` retain 60
successful O0/O2 checks of these five inputs, using both frozen compilers and
both hosts. All 92 boundary objects and traces are byte-identical between the
inline candidate and the final compiler with the separate body helper.

Two standalone runtime gaps remain in BACKEND: nested-outer-swallow returns 10
and function-try-body-local exits 134 at both optimization levels. The frozen
entry and candidate produce identical results for all 24 standalone fixture/
optimization pairs. The host-object controls pass; these are retained native runtime
limitations, rather than a reason to move the valid LowIR tests out of PA21.

Final validation (`validation-sixth/validation.json`) passes the affected PA21
suite, strict **6025/6025**, debug-info, backend variants, self-host through PA5,
all nine architecture audits, the full file audit (37 existing warnings) and
placement audit (zero placement/hygiene findings). The strict report log is
exactly its single final-total line. The actual rebuilt compiler matches the
measured immutable candidate SHA-256
`0959dd558c4fd55fd75a7004630715780ced93a703da9f664d1bf17894316a02`.

Alpha hardware-counter evidence is retained locally under `perf/` and with
all generated objects at
`alpha:/tmp/cppgm-v4-audit-review-20261001-constructor-handler-fifth/`.
`sixth-performance-verification.json` checks the final 720 observations across
15 frozen workloads, identical outputs, A/A calibration and paired A/B blocks.
All instruction (1.005) and RSS (1.03) gates pass: maximum final A/B deltas are
+0.0327% instructions and +0.4672% RSS. Two balanced, interleaved three-image
runs retain another 1,152 observations and pass calibrated cycle gates (1.005):
source-try +0.2432%, noexcept synthesis +0.3606%, virtual +0.2010%, and large
virtual +0.2593%. Recognition controls also pass.

The earlier inline candidate showed a source-try cycle signal; separating the
body-emission helper preserves all 92 boundary objects/traces and reduces that
signal. Raw cycle results, including the noisy broad large-virtual result,
remain available; focused repeats compare them with their A/A calibration.
Every observation from all candidate/calibration/repeat runs is retained
(3,552 total). No unsuccessful build or mismatched binary is counted as final
validation. The final combined student export remains deferred until the full
tracker is resolved.
