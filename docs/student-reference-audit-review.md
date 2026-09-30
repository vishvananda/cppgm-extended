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
| DETECT | Stop treating scalar-array copyobj as class transfer / ABI evidence | v4codex | Done: 550f44dc2; twenty false positives removed; genuine class controls pass. |
| INIT-ADDR | Static namespace/local-reference and pointer initialization ordering | v4codex group 1 | Done: bc55d6227; five ordering reducers pass; full checks and ABBA pass. |
| INIT-OBJ | Emit evaluated class/base/array/template constant object values and relocations | v4codex group 1 | Done: a33d1456d. Seven copied ordering reducers and two new runtime fixtures pass; strict 5825/5825, full checks and ABBA pass. |
| ARRAY-IMAGE | Reconcile PA10 literal arrays and PA16 general constexpr readonly-image/copy rule | v4codex placement / group 1 | Done: 3e2a9a9d1; 48 references regenerated, two new runtime controls, strict 5827/5827 and full checks pass. Performance follow-up finds no persistent regression. |
| FIELD | Bit-field signed promotion / typed reads and volatile aggregate stores | v4codex group 2 | Done: 31598a5b3; two new runtime controls pass at -O0/-O2, seven references regenerated; strict 5829/5829, full checks and equivalent-output ABBA pass. |
| DTOR | Unqualified explicit virtual destructor dispatch and defined fixture lifetime | v4codex group 3 | Done: 049b5fa74; direct/virtual/further-derived runtime controls pass; strict 5829/5829, full checks and equivalent-output ABBA pass. |
| AGG-DEST | Construct aggregate arrays and braced-result members at their final destination | v4codex group 4 | Done: edd6b2121; preserve typed member actions at final addresses; five new controls and seven regenerated references, strict 5834/5834 and full checks pass. |
| RESULT-ABI | Canonical class result ABI for aliases, indirect calls and nontrivial empty results | v4codex group 5 | Done: 70e7a2920; one completed class fact; strict 5838/5838, full checks, Clang/GCC mixed-object controls and equivalent-output ABBA pass. |
| RESULT-CONV | Explicit conversion-function-template calls use canonical result deduction | v4codex group 5 | Done: 4858ddbc0; typed full target deduction and receiver selection; strict 5839/5839, full checks and equal-output performance pass. |
| CONV-IMPLICIT | Valid class copy initialization with a conversion-function template rejects as ambiguous | Additional reducer during RESULT-CONV | Open with conversion legality work: immutable 70e7a2920 and current reject A a=x when A has A(int) and X::operator T(); GCC/Clang accept. Keep the reducer; the explicit-call fixture uses an aggregate result to isolate its contract. |
| TMPL-VALID | Definition-time expression/bound validation, plus valid dependent bounds | v4codex group 6 | Open; distinguish student-entry observations from supplied-oracle comparisons. |
| DEMAND | Dormant static-member definition and storage demand | v4codex group 7 | Open; discarded reference calls completed separately below. |
| DISCARD-CALL | Discarded reference calls preserve effects without loading the referent | v4codex group 7 | Done in the discarded-call checkpoint: PA10 control plus defined PA18/19 inputs; strict 5840/5840 and full checks pass; equal-output repeat performance shows no persistent regression. |
| REJECT | Four invalid programs currently accepted: noexcept receiver, result-type ambiguity, empty array pack, two user conversions | v4codex group 8 | Open; source inputs unchanged, corrected rejection statuses supported by evidence. |
| DEDUCE | Complete defaulted template arguments and preserve closure type in constructor deduction | v4codex group 9 | Open; defaulted-pack runtime expectation also needs 2 → 9 correction. |
| EH-OVERRIDE | Dynamic exception specifications on virtual overrides require an allowed subset | v4codex PA28 audit154 plus independent current reproduction | Open: ours accepts throw(double) or unrestricted overrides of throw(int); GCC/Clang reject. The allowed throw(int) control passes all three. Student-entry defect, no course oracle changes. |
| EH | Construction prefixes, active-handler lifetime/forwarding and failed-new deallocation | v4codex group 10 | Open; prior fb15cd49e class-value temporary cleanup fixes one case only. |
| MEMBER | Signed member-pointer adjustment, target-word truth, inverse conversion, width checks and repeated empty bases | v4codex group 11 | Open. |
| VBASE | Virtual-base layout/lifecycle, construction RTTI, null placement and diamond flags | v4codex group 12 | Open; correct uninitialized fixture before using it as a runtime oracle. |
| MANGLE-CONV | Conversion-function template names retain the declared dependent target | Additional Clang object check during RESULT-CONV | Open: Clang emits _ZN1XcvT_IKiEEv / _ZN1XcvT_IRiEEv; ours emits _ZN1XcvKiIS0_EEv / _ZN1XcvRiIS0_EEv. No encoder change yet; concrete target has replaced declared T in the name facts. |
| MANGLE | ABI substitution state for address expressions and RTTI template-template arguments | v4codex group 13 | Open, checked against Clang 21.1.8: source compiler already matches the member-address reducer; PA9 fact tool ends ER1C instead of ERS1_; RTTI template prefix uses S4_ instead of Clang's S3_. |
| ABI-GLOBAL | Use the raw ABI name for an ordinary external global-namespace variable | v4codex PA27 overlay145 | Open, checked against Clang 21.1.8: exact fixtures emit `g`, ours `_Z1g`; mixed links fail in both directions. Two inspection expectations and the variable encoder need correction. |
| INPUTS | Define PA13/23 object lifetime/value inputs and PA18/19 reference backing objects | v4codex fixture review | In progress: PA13 lifetime and PA18/19 backing objects corrected; two remain: PA23 initialized virtual bases and PA19 pack count. |
| ARG-REF | Allocate object backing separately from a lifetime-extended local reference slot | Argon 1 | Done: af1b1204c; separate storage and scope lifetime; strict 5835/5835, full checks and equivalent-output ABBA pass. |
| ARG-BRANCH | Remove invalid branch destructor suppression and prevent cross-arm initialized-state leakage | Argon 2 | Open: same-type case exits 233; distinct-type case rejects at -O0/-O2. Additional mechanism within EH. |
| ARG-ARGS | Preserve side effects in empty aggregate-member constructor arguments | Argon 3 | Done: edd6b2121; retain constructor calls and argument/parameter lifetimes; counter and by-value lifetime controls pass. |
| ARG-COND | Apply bidirectional class conversion rules to mixed-class conditional operands | Argon 4 | Open: valid case rejects at -O0/-O2; host GCC passes. |
| ARG-ARRAY | Construct aggregate member arrays of nontrivial class elements | Argon 5 | Done: edd6b2121 with AGG-DEST; final-address class-array construction, local/static/nested lifetime and identity controls pass. |
| ARG-SLOTS | Share stack space for mutually exclusive large temporary lifetimes | Argon 6 | Open optimization issue: independent defined reducer spans 1,639,824 bytes across 64 frames at -O1/-O2/-O3; GCC -O1 spans 103,824. Correct values/destructor counts; use a backend frame-size bound, not an arbitrary language stack budget. |
| BACKEND | Standalone duplicate RTTI/native-label and freestanding dynamic_cast limitations | v4codex backend observations | Open review: shared RTTI host-object route passes; standalone route fails. Private-derived/base reducer already passes both. |
| ROUND | Excess-precision differences | v4codex PA25 | Review only: no proven oracle bug; preserve references unless course policy requires a change. |
| DIALECT | Multi-block-inline note using cmp slt instead of contracted cmp lt | Argon post-run note | No compiler fix established: corrected spelling reportedly passes. |
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
regenerated oracles. PA13 explicit destruction now uses manually managed lifetime
in DTOR. Four remain: PA23
constructor-prvalue virtual-base forwarding needs initialized most-derived
virtual bases; PA19 defaulted-pack cardinality must compare with 9; PA18
300-explicit-template-call-transitive-base-deduction and PA19
300-deleted-return-sfinae-same-parameter-list must return references to real
objects. The latter PA19 main must also pass a real tuple instead of *nullptr.
Keep deduction, deleted-overload and discarded-reference coverage, using a
real typed backing object and structural checks for the absent referent load.
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
