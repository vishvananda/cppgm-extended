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
| CONV-SELECTION | Rank converting constructors against conversion functions during class copy initialization | Additional conversion-sequence controls | Open: entry 88f5d368b and candidate accept A a=x with A(X&) and X::operator A(); Clang/GCC reject as ambiguous at O0/O2. Separate from the two-user-conversion restriction. |
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
| EH | Construction prefixes, active-handler lifetime/forwarding and failed-new deallocation | v4codex group 10 | Open; prior fb15cd49e class-value temporary cleanup fixes one case only. |
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
| BACKEND | Standalone duplicate RTTI/native-label and freestanding dynamic_cast limitations | v4codex backend observations | Open review: shared RTTI host-object route passes; standalone route fails. Private-derived/base reducer already passes both. |
| ROUND | Excess-precision differences | v4codex PA25 | Review only: no proven oracle bug; preserve references unless course policy requires a change. |
| DIALECT | Multi-block-inline note using cmp slt instead of contracted cmp lt | Argon post-run note | No compiler fix established: corrected spelling reportedly passes. |
| HOST-TRIVIAL | Verify the deleted-copy triviality oracle and declaration-property semantics | v4codex PA29 handoff156 question | Done in 89a33c0a8: source assertions corrected, deleted/member/overload facts queried and cached; strict 5851/5851, full checks and equal-output ABBA pass. Viability and ABI classification stay separate. |
| HOST-SHORTHAND | Give the hosted nothrow trait fixture complete, typed definitions | v4codex PA29 handoff156 question | Done: complete typed definitions replace compiler template-name synthesis (spec.md section 10). Generic character and noexcept reducers move to PA14/PA16; incomplete/body controls enforce ordinary template rules. Strict 5854/5854, placement/harness/audits and performance pass; student has not edited its oracle. |
| PA29-ALIGN | Preserve GNU alias alignment through declarations and expression indirection | v4codex audit158 entry regressions | Open, independently confirmed: using-alias aligned(1) and typedef-alias *&i controls fail here at O0/O2 and pass Clang/GCC. No required fixture or oracle change was made for these controls. |
| EXPORT | Validate final combined shipped recipes, fixture discovery and quiet report | User | Pending until the fix sequence is complete; initial and INIT-ADDR exports already passed. |

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
