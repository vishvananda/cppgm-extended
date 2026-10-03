# Student reference audit review

Maintainer evidence from the read-only review of `~/work/v4codex` on 2026-09-30. This document is excluded from the student export.

The existing fixture/harness work is committed as `fb15cd49e` on `fix/student-audit-regressions`. Placement detection is corrected in `550f44dc2`: all twenty scalar-array false positives disappear, with genuine class-transfer detection retained. Static pointer/reference initialization is fixed by the accompanying compiler checkpoint. Constant class-object initialization is completed by the next compiler checkpoint; the other open work is recorded in the unified table.

## Current pause and strategy review — 2026-10-03

Implementation paused at the user's request after a70d1174b and resumed
on their subsequent instruction. The latest
qualified compiler passes strict 6118/6118 and the required audits. There are
28 open compiler families and six unresolved reviews; this is not a count of
independent failing root causes. Since fb15cd49e the branch has 107 commits,
46 documentation-only commits and 331 added test sources (301 outside opt-in
controls, 30 controls). These counts do not establish that each fixture is
needed. The previous reduction in benchmarking overhead helped, but speculative
design and adjacent conformance exploration still delay concrete fixes.

Recommended next workflow: fix only frozen student/Argon reproductions, group
them by existing compiler owner, and make a concrete patch before expanding
boundary analysis. Reuse one regression per demonstrated rule; add independent
negatives only when needed to prevent an incorrect repair. Use targeted checks
during edits and qualify a small coherent batch once, retaining all required
checks and calibrated performance evidence. Keep disputed reviews out of the
critical path. Record only status, regression, result and evidence path for
each fix. Finish with fixture minimization and one combined export validation.
No compiler or fixture change is made during this review.

## Previous updated workflow — resumed 2026-10-03

Freeze the discovery inventory and finish confirmed student/Argon defects by
root cause. Reuse fixtures; add only independent missing regressions. During
editing use targeted checks, then run repository-required qualification on the
final patch. Performance gates follow the repository instruction/memory policy;
use focused A/A-calibrated confirmation for suspicious timing differences.
Keep one concise completion record per fix and detailed observations in scratch
artifacts. Final fixture minimization and combined student-export validation
remain part of this goal. The preserved namespace patch is the first resumed fix.
Its smaller performance protocol is declared before measurement in
`/tmp/cppgm-v4-audit-review/namespace-mixed-lookup/updated-screen/protocol.json`:
eight relevant frozen workloads, six paired blocks, 384 observations, unchanged
instruction/RSS limits, equal outputs and unscaled counters. The old cycle-gate
studies remain historical evidence with their original results.

## Historical pause and strategy review — 2026-10-02

The user requested a pause and review of the approach. The thread goal was
paused. The tenth local validation process group stopped (handle 23278, exit 143)
at that request; five affected suites had passed, and the strict report was
interrupted. No tenth Alpha measurement started. The six-file typed-dispatch
patch and immutable image remain preserved; their hashes still match the
recorded tenth metadata. Previously completed checks and measurements retain
their original status. No compiler change is made by this review.

The workflow expanded too far: the branch has 96 commits since fb15cd49e,
45 documentation-only commits, 291 added numbered fixtures and a 5306-line
tracker before this pause note. These figures describe scope and overhead,
not a claim that every added fixture is redundant. The mixed namespace fix
alone reached ten candidates and five full 4032-observation performance studies
(20,160 compiler measurements), despite the correctness-complete candidates
passing instruction and memory gates. The additional 0.5% raw/calibrated cycle
gates were imposed by this work, not scripts/validate_perf_regression.py,
whose gating metrics are instructions and memory. Those extra gates and
repeated full validation of intermediate variants became a major bottleneck.

Recommended resumption strategy: freeze the existing discovery inventory and
prioritize original student/Argon defects and demonstrated reference errors;
group reducers by root cause; reuse existing required fixtures and admit a
minimal regression for each distinct missing rule. Run targeted checks during
editing, a small frozen performance screen before expensive qualification,
and all repository-required checks once the final patch is ready. Use the
repository instruction/memory policy, with focused A/A-calibrated confirmation
for suspected cycle slowdowns; establish repeatability and uncertainty before
rewriting correct code. Keep disputed language cases in the review queue
without making them block unrelated confirmed fixes. Keep one concise summary
per completed fix and raw detail in artifacts. Final fixture minimization and
combined student-export validation remain required. These recommendations became
the updated workflow on resumption; the original interrupted run remains stopped.

## Unified progress tracker

Branch: `fix/student-audit-regressions`. Update this table at each checkpoint;
retain the numbered evidence below instead of treating a reference difference
as a bug by itself. **Open** means independently reproduced unless explicitly
marked **Needs verification**. ABI spellings must be checked against Clang before changing code or references,
as explicitly requested. Student-export validation is deferred until the
fix sequence is complete, as requested.

New discoveries enter the fix queue only when they are C++11 features or when
a concrete hosted-header dependency requires the newer feature. A personal
extension test or a later assignment requirement alone does not establish that
dependency. Retain excluded observations as review evidence without adding
required fixtures or expanding the compiler scope.
Host acceptance with `-std=c++11` does not by itself establish C++11 ownership:
hosts can accept later features as extensions. Record the language clause for
ordinary fixes; for newer features, record the supported header/version and
the use that remains enabled in its C++11 configuration. The admitted
`explicit(bool)` constructor work relies on that header evidence, rather than
on extension-mode acceptance. Other newer controls remain held individually.
Later defect-report wording also needs a recorded review: distinguish a
correction to existing C++11 rules from a new language feature, and do not
turn every modern host result into an N3485 oracle. In particular, pointer
reference aliasing and arrays of pointers involve CWG 2352 and CWG 330.
Their already supported boundaries are retained as regression observations;
the required qualification fixtures below exercise C++11 const safety and
ordinary initialization without imposing a new aliasing expectation.

Remaining-work count on 2026-10-03: **21 compiler issue families** remain
open or in progress. LOOKUP-NAMESPACE-MIXED is completed under the updated
instruction/memory qualification policy. LOOKUP-BASE-ALIAS is also completed.
**Six further reviews** have no established
required compiler change: EH-AGG-NESTED, EH-AGG-TEMP-DTOR,
EH-RESULT-CLEANUP, MANGLE-BOUND, INHERITED-DEFAULT-EXCEPT and ROUND.
ATTR-NORETURN, ASSERT-MESSAGE, CONST-MEMBER-BOOL, CONST-BITFIELD and
LOOKUP-TAG, ABI-GLOBAL, NOEXCEPT-LIST, REF-BITFIELD and
CONST-REF-STATIC-TEMP, REF-BRACE, INIT-LIST-STATIC, EH-RETHROW-DYNAMIC,
TMPL-FTRY, REF-BASE-COND and MANGLE-CONV are also completed. These are tracker
families, not individual failing fixtures or a proven count of distinct root
causes. INPUTS overlaps VBASE; closed reviews, fixture pruning and final export
validation are excluded from the compiler issue count. The passing default
report does not include all opt-in unresolved controls.

The recent discovery inventory is not all C++11:

| Discovery family | Language status | Admission |
| --- | --- | --- |
| Ordinary lookup, template substitution/matching, constant expressions, assertions and references | C++11 | Review against the owning contract and language clauses. |
| Conditional constructor `explicit(bool)` | C++20 | Admitted only for the independently identified libc++ 21 C++11 `pair` constructor branch. |
| Structured bindings, inline variables and deduction guides | C++17 | Held without a necessary selected-header use; constructor evidence does not admit deduction guides. |
| Static call and subscript operators | C++23 | Held without a necessary selected-header use. |
| Additional bit-integer, extended-float, vector, complex, tag and sequence builtin controls | Vendor extensions | Held individually without their own necessary-header evidence; existing supported extensions are not automatically expanded. |

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
| EH-SPEC-TIMING | Timing of a virtual template exception specification using sizeof its current class | Additional override controls | Reviewed: no required compiler change or fixture. The adopted CWG 1330 complete-class context applies to exception specifications, including overrides; its needed-on-comparison rule does not require comparison before class completion. Keep existing acceptance of the three current-class-size controls, despite both hosts rejecting them. CWG 2510 confirms the analogous declaration-matching delay principle. Fresh 42 observations retain that host difference while ordinary complete-class positives and both outside-context incomplete-size negatives agree across all three compilers. No supplied oracle is changed; proof and sources are recorded below and in exception-spec-timing/contract-review/. |
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
| EH-REF-INIT | Retain enclosing object cleanup while initializing an automatic reference | Expanded EH-COND-THROW boundary controls | Done in accompanying checkpoint: all nine original reducers and 48 of 50 expanded programs pass at O0/O2; the two preexisting initialization-form failures are REF-BRACE and REF-BASE-COND. Thirteen C++11 PA21 fixtures pass both hosts and our compiler. Strict 6038/6038 and all eight required check groups pass; seventeen Alpha instruction/RSS gates and four focused raw/calibrated cycle gates pass with equal objects. Initializer-list backing and generic second-fault cleanup remain separately tracked. |
| EH-RESULT-CLEANUP | Destroy a non-NRVO returned object when later return-time destruction throws | Additional EH-CLEANUP result-ownership controls / CWG 2176 | Needs contract review: three prvalue/call/conditional controls fail here and in Clang 21.1.8 at O0/O2, but pass GCC. CWG 2176 adds returned-object destruction beyond the frozen N3485 wording. Keep this host disagreement separate; no required fixture or reference changes. |
| REF-INIT-LIST | Give a reference-bound initializer-list backing array one lexical lifetime | Additional EH-REF-INIT boundary control | Done in accompanying checkpoint: normal list-object materialization and typed binding ownership prevent duplicate backing destruction and reference-slot corruption; partial-array landings retain earlier automatic cleanup. Sixteen C++11 PA21 fixtures and 36 agreed boundary programs pass ours/Clang/GCC at O0/O2, including twelve-element construction progress, borrowed/copy boundaries and static/global references. Strict 6054/6054 and all eight required check groups pass, with zero placement findings. Alpha instruction/RSS and raw/calibrated cycle gates pass across 2,016 equal-object observations. AUTO-CONST-REF, INIT-LIST-STATIC and REF-VOLATILE remain separate preexisting gaps; disputed value-copy/comma forms remain review evidence. |
| AUTO-CONST-REF | Deduce const auto& from an rvalue without imposing an auto& lvalue constraint | REF-INIT-LIST boundary controls | Done in accompanying checkpoint: ordinary non-volatile const deduction, function cv handling, typed reference recipes and class-element copies retain C++11 behavior. Seventeen PA20/21 fixtures pass ours/Clang/GCC, including strict host C++11 checks; strict 6071/6071 and all required checks pass with zero placement/hygiene findings. Final Alpha instruction/RSS and eight focused raw/calibrated cycle gates pass across 4032 equal-object observations; 5312 earlier timing observations remain recorded. CONST-REF-STATIC-TEMP, REF-ARRAY-CV, REF-POINTER-QUAL and EH-LOCAL-ARRAY-CATCH remain independent owner gaps. |
| CONST-REF-STATIC-TEMP | Give a constexpr reference an address fact for a static-storage temporary | AUTO-CONST-REF explicit-type boundary | Done: private unindexed backing storage supplies the existing constant address and static initialization paths. Extend the existing PA16 static-object reference fixture; remove the redundant control. Original scalar, aliases, distinct identities, local-static, floating and pointer boundaries pass at O0/O2; invalid automatic/runtime initializers still reject. Strict 6118/6118 and all required checks pass; 144 Alpha observations pass instruction/RSS/equality gates. |
| REF-ARRAY-CV | Recognize element cv-qualification when binding a reference to a const array xvalue | AUTO-CONST-REF explicit-type boundary | Done in the accompanying performance-approved checkpoint: consume normalized element cv while matching array dimensions and reference qualification. Explicit and deduced const-array xvalue bindings pass, as do all required qualifier/category boundaries. Thirty qualification fixtures, 574 fresh controls plus 376 retained host observations, strict 6108/6108, all compiler checks and the global 4800-observation performance gate pass. |
| REF-POINTER-QUAL | Require valid qualification and reference compatibility when referent pointer types differ | REF-ARRAY-CV preparation controls | Done in the accompanying performance-approved checkpoint: enforce cv subset, atomic parity and intermediate const while distinguishing direct reference compatibility from pointer-prvalue conversion temporaries. Mutable/volatile/deep-pointer negatives and valid const/deep-pointer boundaries pass. Already supported host-agreed aliases remain preserved; no new CWG2352/330 aliasing oracle is imposed. The full boundary matrix, strict report, required checks and global performance gate pass. Bit-field materialization remains separate. |
| REF-BASE-CATEGORY | Reject a related derived lvalue bound to a base rvalue reference | Reference qualification boundary controls | Done in the accompanying performance-approved checkpoint: reject the lvalue category before accepting the related derived-to-base reference conversion, per N3485 8.5.3/5. Base xvalue and const-volatile base lvalue positives remain. Full boundary/regression controls, strict report, all required compiler checks and global performance gate pass. |
| REF-BITFIELD | Copy a bit-field for const-reference binding and reject mutable or volatile lvalue-reference binding | Reference qualification boundary controls | Done. Use the existing typed binding bit-field fact after overload selection: non-volatile const lvalue references copy through the scalar temporary path; mutable/volatile bindings reject. Extend the existing PA11 bit-field aggregate test for local and argument snapshots and promote two independent negatives, with the mutable case also checking overload selection. Remove three duplicate opt-in controls. Strict 6118/6118 and all required checks pass; final Alpha gates pass. The disputed rvalue-reference cast control remains held without a mandatory oracle. |
| EH-LOCAL-ARRAY-CATCH | Reach a handler in the current function after partial initializer-list backing construction | AUTO-CONST-REF fault boundary | Open, independently reproduced: a throw from the second backing-element constructor or copy constructor terminates instead of reaching the same-function int handler. Both unchanged a3aaf2015 entry and deduction candidate fail with explicit reference types; Clang/GCC execute successfully in C++11 at O0/O2. Frozen inputs and sixteen explicit-type observations are in auto-const-reference/fault-controls/explicit-final-controls.json. The earlier caller-handler prefix controls remain unchanged. Review the partial-array landing and native handler search without attributing this preexisting gap to auto deduction. |
| INIT-LIST-STATIC | Keep a local-static initializer-list value's backing array alive after initialization | REF-INIT-LIST storage controls | Done: persistent backing and cleanup now work through direct native, compiler-object and host-object routes at O0/O2, including shutdown and failed-initialization retry. Native atexit callbacks drain in LIFO order before the shutdown hook and preserve the entry result. Extend existing PA21/PA24 fixtures; remove the opt-in duplicate. Strict 6118/6118, all required checks including variants and Alpha instruction/RSS/equality gates pass. |
| REF-VOLATILE | Reject an rvalue bound to a const-volatile lvalue reference | REF-INIT-LIST negative boundary | Done in the accompanying performance-approved checkpoint: temporary lvalue-reference binding requires const without volatile, including normalized array element cv. The independent scalar/class/array negatives and valid lvalue controls pass; full compiler checks and global performance gate pass. |
| REF-BRACE | Initialize a local reference from a braced class temporary | Additional EH-REF-INIT controls | Done: reference-related single elements bind directly; other class lists create a separate temporary through existing materialization. Reuse prepared typed elements. Extend the existing PA12 constructor-argument lifetime fixture; remove the opt-in duplicate. Original, alias, conversion, aggregate, derived and rejection boundaries pass 48 focused checks at O0/O2. PA12 298/298, strict 6118/6118, all required checks and Alpha instruction/RSS/equality gates pass. |
| REF-BASE-COND | Bind a base reference to a conditional derived-class temporary | Additional EH-REF-INIT controls | Done: materialize the complete derived prvalue before projecting the base reference, preserve cv and binding category, and publish later lifecycle definitions to cached base entries. Extend the existing PA21 lifetime fixture with a nonzero base offset; add one independent PA12 mutable-xvalue cast rejection. Original and fixture pass all three link routes at O0/O2. Eighteen reviewed LowIR references add only the previously missing base-destructor body. Strict 6119/6119, all required checks and Alpha gates pass. |
| EH-UNWIND-DTOR | Terminate when a staged lexical/full-expression unwind destructor throws | Additional EH-REF-INIT boundary controls | Open, independently reproduced: four reference-initializer controls and an ordinary object/throw control return 10 with both the unchanged entry and cleanup candidate; Clang/GCC invoke the installed termination handler (77) at O0/O2. Generic full-expression cleanup continuations lower destructor calls without the terminating guard used by constructor/destructor body cleanup. Frozen inputs and host traces are retained under reference-initializer-cleanup/. |
| EH-ARRAY-DTOR | Preserve remaining elements when an unrolled class-array destructor throws | Additional EH-CLEANUP array boundary controls | Open, independently verified: a three-element array skips its first element after the second destructor throws; entry and cleanup candidates fail at O0/O2, Clang/GCC pass. A twelve-element control passes all compilers because the loop path already owns an unwind-progress suffix. |
| TMPL-FTRY | Retain the complete definition of a function template using a function-try block | Additional EH-CLEANUP source control | Done: retain body, nested constructor initializer and function-try syntax through registration, definition adoption, specialization upgrades and explicit specialization. The original program passes all three link routes at O0/O2; specialization/upgrade boundaries match Clang/GCC. Extend the existing PA21 function-try fixture and retire the opt-in duplicate. The extended fixture's preexisting native failure remains BACKEND. Pattern storage remains 512 bytes. Strict 6118/6118, all required semantic-change checks and Alpha gates pass. |
| EH-RETHROW-DYNAMIC | Accept operandless throw in a function called with a dynamically active handler | Additional defined destructor/helper controls | Done for source acceptance and lowering: remove the lexical-handler restriction and its unused counter. All three original programs pass host linking at O0/O2; the called-function case and rewritten existing fixture also pass standalone/object routes. No-active-exception execution invokes the installed termination handler. Same-function nested-handler and destructor traces still fail only in the native BACKEND family; retain/reclassify the existing opt-in reducer. Strict 6118/6118, all required semantic-change checks and Alpha gates pass. |
| MEMBER | Signed member-pointer adjustment, target-word truth, inverse conversion, width checks and repeated empty bases | v4codex group 11 | Open. |
| VBASE | Virtual-base layout/lifecycle, construction RTTI, null placement and diamond flags | v4codex group 12 | Open; correct uninitialized fixture before using it as a runtime oracle. |
| MANGLE-CONV | Conversion-function template names retain the declared dependent target | Additional Clang object check during RESULT-CONV | Done: build conversion terminals from retained template recipes; resolve explicit conversion specializations and qualified conversion addresses through existing deduction using retained syntax. Fresh Clang/GCC O0 symbols agree. Extend existing PA18 direct-conversion and PA22 pointer-target fixtures; retire the opt-in duplicate. Fourteen other references change only conversion object-name metadata. Strict 6119/6119, all required checks and Alpha gates pass; no ABI encoder change. |
| MANGLE-RESULT | Dependent decltype result forms retain expression identity and unparenthesized id category | Template result identity host-symbol controls | Open, confirmed against Clang/GCC and unchanged entry 1cb054e23: bare decltype(value) uses DT instead of Dt; named dependent selected(value) loses the expression and emits a concrete result type. Parenthesized decltype((value)) already agrees. No encoder change in the result identity checkpoint. |
| MANGLE-PACK | Preserve the declared expansion in function-template parameter name facts | Defaulted-pack Clang object controls / v4codex correction91 | Open: entry 425bc2a90 and candidate flatten a fixed-primary parameter expansion and append defaults; Clang/GCC retain `tuple<T_,DpT0_>`. The deduction checkpoint corrects the concrete template argument pack cardinality; the parameter pattern remains wrong. No encoder change made. |
| MANGLE-BOUND | ABI spelling for a template bound using sizeof an adjusted parameter | Additional PARAM-ADJUST Clang comparison | Needs contract review: Clang spells RAszfL0p__i; GCC and ours spell RA8_i. The parameter type is fixed after adjustment. No encoder or old oracle change made; retain host and typed-name evidence. |
| MANGLE | ABI substitution state for address expressions and RTTI template-template arguments | v4codex group 13 | Open, checked against Clang 21.1.8: source compiler already matches the member-address reducer; PA9 fact tool ends ER1C instead of ERS1_; RTTI template prefix uses S4_ instead of Clang's S3_. |
| ABI-GLOBAL | Use the raw ABI name for an ordinary external global-namespace variable | v4codex PA27 overlay145 | Done. Fresh Clang O0/O2 checks and mixed links in both directions establish `g`; the typed ABI variable target now preserves that raw name. Existing PA9/PA27 tests carry the regression, and related LowIR/hosted inspection references were regenerated and independently checked. Strict 6116/6116, required checks and Alpha instruction/memory gates pass. |
| INPUTS | Define PA13/23 object lifetime/value inputs and PA18/19 reference backing objects | v4codex fixture review | In progress: PA13 lifetime and PA18/19 backing objects corrected; PA19 pack count is corrected in the deduction checkpoint; PA23 initialized virtual bases remain. |
| ARG-REF | Allocate object backing separately from a lifetime-extended local reference slot | Argon 1 | Done: af1b1204c; separate storage and scope lifetime; strict 5835/5835, full checks and equivalent-output ABBA pass. |
| ARG-BRANCH | Remove invalid branch destructor suppression and prevent cross-arm initialized-state leakage | Argon 2 | Done in 5d4ff5a34: both original reducers and normal/nested throwing-arm controls pass; strict 5843/5843, full checks and equal-output ABBA pass. Other EH mechanisms remain open. |
| ARG-ARGS | Preserve side effects in empty aggregate-member constructor arguments | Argon 3 | Done: edd6b2121; retain constructor calls and argument/parameter lifetimes; counter and by-value lifetime controls pass. |
| ARG-COND | Apply bidirectional class conversion rules to mixed-class conditional operands | Argon 4 | Done in d240819cf: direct binding then value fallback, base/cv constraints and implicit-candidate controls; strict 5846/5846, full checks and equal-output ABBA pass. COND-RESULT remains separate. |
| COND-RESULT | Preserve const class conditional result types and copy glvalue class conditional results | Additional controls while fixing ARG-COND | Done in af5f041ed: const result facts, selected glvalue copying and scoped reference backing; strict 5849/5849, full checks and equal-output ABBA pass. |
| ARG-ARRAY | Construct aggregate member arrays of nontrivial class elements | Argon 5 | Done: edd6b2121 with AGG-DEST; final-address class-array construction, local/static/nested lifetime and identity controls pass. |
| ARG-SLOTS | Share stack space for mutually exclusive large temporary lifetimes | Argon 6 | Open optimization issue: independent defined reducer spans 1,639,824 bytes across 64 frames at -O1/-O2/-O3; GCC -O1 spans 103,824. Correct values/destructor counts; use a backend frame-size bound, not an arbitrary language stack budget. |
| BACKEND-ARRAY-OPT | Keep optimized array cleanup frames valid when helper bodies are defined in the same translation unit | Self-contained EH-SPECIAL-PREFIX fixture controls | Open, independently reproduced: the entry and copy-cleanup candidates crash at O2 for the twelve-element move and trivial-copy-prefix array fixtures, in standalone and host-linked object routes. Clang/GCC pass; our O0 routes and corresponding external-companion forms pass. Retain frozen sources, binary hashes, host-link controls and debugger observations under synthesized-construction-prefix/. |
| BACKEND | Standalone duplicate RTTI/native-label and freestanding dynamic_cast limitations | v4codex backend observations | Open review: shared RTTI host-object route passes; standalone route fails. Private-derived/base reducer already passes both. Two defined source-handler controls also retain identical entry/candidate standalone failures at O0/O2: nested-outer-swallow returns 10 and function-try-body-local aborts (134); their host-object routes pass Clang/GCC and the candidate. Keep these runtime routes separate from PA21 LowIR cleanup correctness. The standalone shutdown-registration dependency exposed by INIT-LIST-STATIC is fixed in the native callback checkpoint below. Existing object metadata already maps its helper to C atexit; no ABI spelling change was needed. The two source-handler routes above remain open. Dynamic rethrow acceptance now exposes two more native-only observations within this handler/cleanup family: same-function nested rethrow terminates (134), and the destructor-rethrow trace returns 1, while both host-object routes pass. Retain pa21/tests/controls/200-audit-dynamic-rethrow.cpp under BACKEND; no new fixture. |
| ROUND | Excess-precision differences | v4codex PA25 | Review only: no proven oracle bug; preserve references unless course policy requires a change. |
| DIALECT | Multi-block-inline note using cmp slt instead of contracted cmp lt | Argon post-run note | No compiler fix established: corrected spelling reportedly passes. |
| HOST-TRIVIAL | Verify the deleted-copy triviality oracle and declaration-property semantics | v4codex PA29 handoff156 question | Done in 89a33c0a8: source assertions corrected, deleted/member/overload facts queried and cached; strict 5851/5851, full checks and equal-output ABBA pass. Viability and ABI classification stay separate. |
| HOST-SHORTHAND | Give the hosted nothrow trait fixture complete, typed definitions | v4codex PA29 handoff156 question | Done: complete typed definitions replace compiler template-name synthesis (spec.md section 10). Generic character and noexcept reducers move to PA14/PA16; incomplete/body controls enforce ordinary template rules. Strict 5854/5854, placement/harness/audits and performance pass. Student later corrected the three original success sidecars in handoff189; our sources retain complete typed definitions and the positive test goals. |
| PA29-ALIGN | Preserve GNU alias alignment through declarations and expression indirection | v4codex audit158 entry regressions | Open, independently confirmed: using-alias aligned(1) and typedef-alias *&i controls fail here at O0/O2 and pass Clang/GCC. No required fixture or oracle change was made for these controls. |
| LOOKUP-RESTORE | Restore genuine namespace type/value ambiguity coverage lost during early consolidation | Namespace reference review | Done in the accompanying test-only checkpoint: distinct-type PA6 and original distinct-variable PA7 negatives have exact generated references and pass their owning checks. Strict 6103/6103 prints one line; PA6/PA7 placement scans 297 inputs with zero findings, and the standard full placement audit remains clean. No compiler change. The valid same-int PA6 oracle correction remains LOOKUP-NAMESPACE. |
| LOOKUP-NAMESPACE | Converge namespace typedefs and aliases naming the same type or namespace | v4codex PA30 implementation197 / PA6 reference correction | Done in the accompanying checkpoint: enable existing canonical-type equivalence when merging namespace graph/import results; preserve direct lookup and class-base rules. All 72 core observations pass at O0/O2 and in both dump modes, including distinct-entity negatives. The reviewed composite replaces the wrong PA6 same-int rejection in PA6/200 and its temporary control is removed; ours and both hosts pass. PA6 112/112, strict 6109/6109 in one line, all required local groups and all frozen incremental Alpha gates pass against approved 8a6bcd105. All 2880 observations retain equal objects and unscaled counters. Independent mixed type/value and class-base alias rows remain open. N3485 7.1.3 and 7.3.4 support this correction. |
| LOOKUP-NAMESPACE-MIXED | Diagnose a type and value found through different namespace imports without hiding either | LOOKUP-NAMESPACE boundary controls | Done in this compiler checkpoint: typed ordinary/type-name lookup diagnoses imported type/value conflicts while preserving local hiding, injected constructor names and restricted elaborated lookup. Reuse the existing PA6 typedef/value and PA7 class/value expression controls as required negatives; exact ref-test generates their references. All 184 focused outcomes and affected suites pass, strict 6111/6111 prints one line, debug-info/self-host PA5/all nine architecture targets/file/placement pass. The 384-observation screen plus 384-observation focused confirmation pass instruction/RSS gates, object equality and unscaled-counter verification (maximum instructions 1.000289, median RSS ratios 1.0). Confirmed namespace/auto-alias calibrated cycle costs around 1–1.6% remain explicit diagnostics under the updated policy; no timing-neutrality claim. Full immutable patch, binary and raw observations are retained in namespace-mixed-lookup/. Historical cycle-gate failures remain unchanged. |
| LOOKUP-BASE-ALIAS | Converge same-type typedef lookup through unrelated class bases | v4codex implementation197 / active audit198 correction | Done in this compiler checkpoint: compare canonical designated types in the existing merge owner, preserving the declaration representative for access checks. All 20 focused outcomes pass, including fundamental/class/dependent-alias runtime input and distinct type/value/template/private-access boundaries. Move the unchanged erroneous PA30 rejection fixture to PA22/100; exact ref-test generates successful LowIR/status references, and redundant opt-in metadata is removed. No new source fixture. N3485 10.2/3,6,7 supplies the C++11 proof despite GCC disagreement; mixed-access base-order observations remain scratch evidence. Affected suites 1590/1590, strict 6111/6111 in one line, debug/self-host PA5/all nine architecture checks/file/placement pass. All 192 frozen performance observations pass instruction/RSS/equality/unscaled-counter checks, with no timing signal requiring confirmation. Candidate and full raw results are retained in base-alias-convergence/. |
| LOOKUP-TAG | Keep hidden friend class tags out of ordinary lookup and honor a new nested class forward declaration | v4codex PA30 source195 controls | Done: own-scope nested declarations, a packed hidden-friend visibility fact, declaration-only canonical tag lookup, and namespace-bounded unqualified friend lookup implement N3485 3.4.4/2 and 7.3.1.2/3. Matching declarations publish the same class identity. Promote two unchanged controls to PA11/100 and /200; no new source fixture. All 72 focused outcomes and eight final qualification groups pass; affected 1271/1271, strict 6116/6116 in one line, placement/review/hygiene zero. All 336 screen/confirmation observations pass instruction/RSS/equality gates; the noisy initial virtual timing signal does not repeat. BindingRecord/EntityRecord remain 136/208 bytes. Artifacts in tag-introduction/. |
| TMPL-LATE-TYPE | Retain dependent member-type queries until the selected class definition is available | v4codex PA29 controls189/defined-conversions.cpp | Open, independently reproduced: the valid C++11 composite rejects at the dependent traits<T>::int_type declaration here and runs successfully with Clang/GCC at O0/O2. A forward-declared primary is defined before the member is demanded; the dormant invalid body must stay undemanded. |
| TMPL-MEMBER-MATCH | Review inherited return-type aliases when matching an out-of-class member definition | v4codex PA30 source199 | Reviewed: no additional required fix or fixture. Instantiating the mismatched member rejects with conflicting function return type at O0/O2 and in PA14 LowIR, while the matching definition passes; both strict C++11 hosts agree. The unused form remains a diagnostic-timing difference under N3485 14.6/8, not a mandatory rejection oracle. Fourteen new observations and contract review are recorded below. |
| NEW-ARRAY-DTOR-ACCESS | Check destructor accessibility when constructing a class array with new | v4codex PA30 source200 | Done in the accompanying performance-approved checkpoint: use the indexed destructor binding and naming/object-class access check before trivial destruction is elided. Promote the private-array negative, add one protected-base-object negative and retain the existing scalar/own-array positive in PA12/400. Demanded scratch boundaries pass ours and Clang 22/22; three GCC-disputed cases stay scratch evidence. PA12 298/298, strict 6108/6108 in one line, all required compiler checks and global Alpha gates pass. N3485 5.3.4/17 and 11.4/1 supply the rule. |
| TMPL-ACCESS-SFINAE | Treat inaccessible dependent aliases in an immediate substitution context as candidate failure | v4codex PA29 controls189 and PA30 source197 | Open, independently reproduced: both the C++11 overload fallback and partial-specialization/private-alias fallback reject with hard access errors here; Clang/GCC execute successfully at O0/O2. The corresponding ambiguous partial-specialization rejection and ordinary alias-order positive already pass. |
| ASSERT-MESSAGE | Reject non-string and user-defined-literal static_assert messages | v4codex PA29 controls189 | Done: typed ordinary-string token category and retained message range enforce PA15 semantics while preserving PA5 generic-literal AST acceptance. Promote two unchanged independent controls; no new source fixture or record field. Adjacent literal kinds use one guarded range check. All 49 focused outcomes and eight required final validation groups pass; strict 6114/6114 prints one line. Final 288-observation screen/confirmation passes instruction/RSS/equality gates; confirmed virtual-class cycles increase 3.49%, explicitly retained. A 72-compile profile found no actionable new hotspot. First-candidate evidence and final source/image/results remain in static-assert-message/. |
| CONST-MEMBER-BOOL | Evaluate a nonnull member pointer as a constant boolean | v4codex PA29 assertion-context.cpp | Done: add the missing member-pointer predicate to ApplyContextualBool, reusing typed ApplyMemberPointerTarget folding (N3485 4.12/1). Extend the existing PA22 contextual-bool fixture with data/function nonnull and null assertions; remove the superseded opt-in control. All 48 focused outcomes and eight final qualification groups pass; strict 6114/6114 stays one line. The expanded 240-observation final screen includes 3000 accepted contextual conversions and passes instruction/RSS/equality gates with no timing confirmation indicated. Preserve the first shared-classifier candidate and its confirmed 2.69% cycle cost; the final patch avoids that additional call. No new source fixture. Artifacts in constant-member-pointer-bool/. |
| CONST-BITFIELD | Apply bit-field width conversion during constant aggregate initialization | v4codex PA29 controls190/fixed-lists.cpp | Done: BuildConstexprObjectElement reuses NormalizeWideConstant with the typed BindingLayoutFact width after declared-type conversion. Skip full-width and bool values; retain signed values. Three assertions extend the existing PA16 aggregate fixture and the superseded opt-in control is removed. All 42 focused outcomes and eight final qualification groups pass; affected 876/876, strict 6114/6114 prints one line. The 240-observation screen includes targeted bit-field constants and ordinary aggregates; instruction/RSS/equality gates pass with no timing confirmation indicated (maximum instruction ratio 1.003711, RSS 1.0). No new source fixture. Artifacts in constant-bitfield-width/. |
| EXPLICIT-CONTEXT | Validate access and substitution in a conditional constructor explicit-specifier | v4codex PA29 controls189 / hosted-header dependency | Open, independently reproduced: private conversion is accepted and invalid immediate conditions cause a hard error instead of selecting the fallback; Clang/GCC corroborate in C++11 extension mode. This C++20 feature is necessary for the supported libc++ profile: release/21.x __utility/pair.h lines 140/147/162 use conditional explicit in the C++11 constructor branch. Frozen header SHA and observations are recorded below. Deduction-guide controls are excluded from this row without their own header dependency. |
| EH-SPEC-SET | Compare dynamic exception specifications as sets of adjusted types | v4codex PA30 source201 | Done in the accompanying performance-approved checkpoint: deduplicate adjusted TypeIds and compare declarations as sets while retaining first-declaration order. The one positive is promoted to PA6/300 and its temporary control removed; the distinct-set negative remains. Nine scratch boundaries pass ours and Clang; GCC's adjusted-array/function disagreement is retained. PA6 112/112, strict report, every required compiler check and the final combined 4800-observation global performance gate pass. Earlier two failed cycle gates remain recorded. N3485 15.4/2,3 supplies the rule. |
| LOCAL-ODR | Reject automatic outer-local odr-use across an ordinary local-class member and invalid default/capture contexts | v4codex PA30 source201 | Open: 13 strict-host-agreed rejection controls are accepted here, including parameters, array access, address/reference binding, discarded use, volatile constants and captures/default arguments. Entry and candidate compile statuses agree. Keep valid constant and unevaluated uses separate; Clang/GCC-disputed constant-default and explicit constant-capture cases are held. Review owning class/default/lambda contracts before selecting minimal independent negatives. |
| FLOW-DEFINED | Preserve valid constant-loop, unreachable-handler and label control flow | v4codex PA30 source201 | Open: defined positive composites reject here with no-return or unbound-native-label errors while strict C++11 hosts accept. Reaching a non-void end is undefined behavior under N3485 6.6.3/2; host warnings for separate fallthrough negatives do not establish a missing diagnostic requirement. Reduce the genuine positive failures and review the course diagnostic policy separately. |
| ATTR-NORETURN | Diagnose standard noreturn argument and non-function target constraints | v4codex PA30 source201 | Done: argument rejection in 8a6bcd105; variable-target/provenance fix in this checkpoint. N3485 7.6.1/4 and 7.6.3/1 restrict the standard attribute to functions. Preserve GNU attribute provenance (including both namespace spellings), ignore unknown scoped attributes, and check resolved declarator types plus variable/member/parameter owners. Ours and Clang pass 28/28 focused outcomes; GCC invalid-target acceptance differences remain recorded. Add one PA29/500 variable negative and reuse the existing GNU-attribute fixture for an unknown-scoped-variable positive and existing noreturn runtime coverage. Affected 558/558, strict 6112/6112 in one line, debug/self-host PA5/all nine architecture checks/file/placement pass. The 192-observation screen plus 96-observation namespace timing confirmation pass instruction/RSS/equality/unscaled-counter checks: maximum instruction ratio 1.000729, median RSS ratios 1.0; combined namespace cycles median 1.004159 with interval [1.001728,1.010226], retained as diagnostic cost. Raw evidence is in noreturn-variable-appertainment/. |
| INHERITED-DEPENDENT | Recognize using T::T as a dependent inherited-constructor declaration | v4codex PA31 source205 | Open, independently reduced without zero-argument inheritance: D<T> : T with using T::T and D<B>(7) rejects as template-parameter redeclaration here, in entry/candidate type dumps and at O0/O2; strict C++11 Clang/GCC accept. N3485 7.3.3 and 12.9 support the parameterized constructor. Ordinary using B::T shadowing remains a separate held host disagreement. |
| INHERITED-VALIDITY | Include other-subobject viability in inherited-constructor trait queries | v4codex PA31 source205 | Open, independently reduced with a parameterized inherited constructor: __is_constructible(D,int) wrongly remains true when another member has a deleted default constructor; strict C++11 hosts report false, and the negative static assertion rejects in entry/candidate. Original zero-argument controls are not the sole evidence. Preserve private, reference and throwing-subobject boundaries when reviewing the shared validity owner. |
| INHERITED-ZERO | Review zero-argument inherited construction and default-argument exception traits | v4codex PA31 source205/206 | Zero-candidate review complete: P0136R1/N4429 establish the adopted C++11 defect-resolution basis. Six fresh host-agreed runtime observations already pass here for ordinary zero-argument availability, member initialization and local hiding; eighteen access/deletion/member negatives also agree. No new zero-availability fix or fixture is needed. Dependent using and parameterized viability remain separate open rows. The throwing-default/nothrow disagreement is preserved in INHERITED-DEFAULT-EXCEPT; no oracle is imposed for it. |
| INHERITED-DEFAULT-EXCEPT | Review throwing defaults on a zero-argument inherited constructor and the corresponding nothrow trait | Existing INHERITED-ZERO exception review, isolated source206 | Needs contract review. Six fresh observations show ours catches the second default-argument throw, while Clang/GCC terminate at O0/O2; all three report the zero construction nothrow in the isolated trait control. The student's false nothrow assertion disagrees with all three. Preserve the default-argument runtime/trait discrepancy without changing its oracle or adding a mandatory fixture. Primary adopted C++11 DR sources and frozen controls are in inherited-zero-contract/. |
| NOEXCEPT-LIST | Preserve constructor ownership facts across parameter template instantiation | v4codex PA31 source205 | Done. The shared failure also affects ordinary calls: a stale EntityRecord reference loses the user-provided-constructor fact when parameter template instantiation moves the entity vector. Reacquire by stable ID before publishing constructor facts. Reuse the PA21 private initializer-list argument fixture, including nonthrowing/throwing-element assertions and runtime class-list conversion; remove the redundant control. Strict 6116/6116 and all required checks pass; Alpha instruction/RSS gates pass. |
| NEW-SPEC-REF | Review the PA30 replacement-new reference change against actual declarations | v4codex reference-correction201 | Done in accompanying test-only checkpoint. Clang explicitly implements named-bad_alloc acceptance as legacy compatibility, and selected libstdc++/libc++ C++11 declarations are unrestricted. The existing positive is replaced by unrestricted hosted replacement-new object emission; the independent wrong-spec negative remains. Exact ref-test generation, PA30 153/153, strict 6103/6103 in one line and placement/hygiene checks pass; the unchanged compiler entry also accepts it. No extra fixture or compiler rule is added. Combined export remains deferred. |
| LOOP-PTR-FINITE | Preserve nontermination when an effect-free pointer loop has unproved equal residues | v4codex PA32 reference correction217, completed audit218 | Open, independently reproduced. The student changed only the existing backward-loop reference and quality envelope; source and status remain unchanged. Here O1/O2/O3 return for unequal mod-eight pointer inputs, while O0 preserves the loop, with both native paths. PA32 requires behavior preservation for defined LowIR; plain index carries no stronger optimization or source forward-progress promise. Eighty fresh observations and the frozen two-sidecar delta are retained. Review/fix the existing loop-finiteness proof and regenerate this reference, without adding a duplicate fixture. |
| TEST-ADDITIONS | Complete admitted regression definitions separately from compiler fixes | User latest scope | Done for the admitted test-definition queue: thirty qualification fixtures committed in e4c80f69d, two additional passing default fixtures, and the opt-in definitions recorded in 4d375b564/6d18f7c04. At that definition checkpoint there were 56 controls covering forty-one families; 99 strict C++11 checks agreed per host, with the additional reused base-alias input exposing the documented GCC disagreement. The solution then passed 3/110 observations, exposing known failures. Subsequent approved fixes promote controls and remove their temporary copies: exception sets and private-array access in ed64e4b4b, and the standard noreturn argument rejection in the accompanying checkpoint. One independent protected-base-array access negative is added. The final tests-only checkpoint replaces the PA6 class/value declaration control with a PA7 ordinary-expression rejection control, preserving the independent PA6 typedef/value control. Both hosts pass 4/4 and explicit placement is clean. At the definition checkpoint the namespace candidate was held for cycle gates. The completed LOOKUP-NAMESPACE-MIXED fix now promotes its two existing controls under the updated qualification policy; strict passes 6111/6111 in one line. Remaining compiler fixes, final course-fixture minimization/promotion and combined export stay open. |
| FIXTURE-REVIEW | Minimize added fixtures after the complete addition sequence | User final-review requirement | Pending final review after the outstanding fixes promote their controls; test definitions are complete. Inventory every added/rewritten fixture since fb15cd49e against retained coverage, including the final additions. Keep a fixture only for distinct required language/header behavior or an independently justified regression boundary; exploratory permutations and different implementation paths alone do not justify duplicates. Remove or combine overlapping fixtures while keeping independent rejection checks, earliest milestone ownership and useful failure identification. Regenerate changed references only through exact ref-test selections. Record the retain/combine/remove rationale and rerun affected suites, strict report and placement before final combined export validation. |
| EXPORT | Validate final combined shipped recipes, fixture discovery and quiet report | User | Pending until the fix sequence and final FIXTURE-REVIEW are complete; initial and INIT-ADDR exports already passed. |
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


## Reference initializer cleanup checkpoint

Independent evidence is retained under
`/tmp/cppgm-v4-audit-review/reference-initializer-cleanup/`. The immutable entry
is commit `97f2d4f52`, compiler SHA-256
`0959dd558c4fd55fd75a7004630715780ced93a703da9f664d1bf17894316a02`.
Thirty-eight initial programs and twelve additional programs compile and run
with Clang/GCC at O0/O2. The entry fails 33 of the original programs and all
12 additional ones; the brace form crashes and the conditional-derived-base
form rejects. The staging candidate passes 48 programs at both optimization
levels, including all nine original EH-REF-INIT reducers. The two initialization
forms remain explicitly open in REF-BRACE and REF-BASE-COND.

Automatic reference initialization now has one typed lifetime owner before the
reference is registered. `CollectReferenceLifetimeObjects` follows the binding
chain through members, subscripts, casts, comma expressions and conditionals;
it excludes a reference call's borrowed result and its temporary arguments.
The cast/address-materialization flag alone does not imply borrowed storage:
the initializer's reference type establishes that distinction. Full-expression
actions for extended backing are unwind-only, become active after construction,
and transfer to lexical scope on successful completion. Conditional backing
uses its existing lifetime state through scope exit rather than retiring at
the arm join. Earlier objects retain their unwind actions, and source handlers
still own their dispatch before constructor/destructor body cleanup.

The ordinary alias path returns before temporary or exception scans. The
reference owner reuses both the exception analysis and the validated destructor
action; lexical elision rules remain the same. No graph fields or record sizes
were added. The semantic-owner ledger records the new lifetime method.
Initializer-list references retain their existing, separate owner rather than
being silently included in this refactoring.

Thirteen self-contained PA21 fixtures cover lvalue/xvalue/reference calls,
scalar member references, cast backing, conditional aggregates, aggregate
members, comma temporaries, non-extending calls and constructor/destructor
bodies. Their trace/count expectations come from independent Clang/GCC O0/O2
runs; all 78 host/compiler observations pass. Four existing PA12 references
were regenerated with unchanged inputs. All 32 original/candidate/Clang/GCC
execution observations of those four sources pass; the changed LowIR adds
initialization unwind boundaries rather than changing their lifecycle outcomes.

The earlier 274-observation regression set changes only the nine old reference
reducers, from 10 to 0 at both optimization levels. Expected termination codes
and the still-failing failed-new control are preserved. The temporary lifetime
optimizations retain every tested object and stdout/status result, as checked
by `third-equivalence.json` and `fourth-equivalence.json` (400 observations each).
The expanded successful runs have 94 exact host traces. A nested class
conditional retains one optional C++11 copy; both the ordinary and
`-fno-elide-constructors` host variants pass its value/live-object checks. Its
copy-dependent trace is not a required oracle.

Additional controls are recorded, not hidden: REF-INIT-LIST fails its backing
lifetime check with the entry and candidate while both hosts pass. Four
reference-initializer second-fault controls and an ordinary object/throw control
fail to terminate here at O0/O2, with unchanged entry and candidate; both hosts
invoke the installed termination handler (77). EH-UNWIND-DTOR owns this generic
full-expression cleanup guard gap. These inputs do not become required
fixtures until their owning fixes are independently validated.

The final immutable compiler SHA-256 is
`366793492b521f0d772e98625981796dc48a179f54317bd630cf80ba3fd8c5ce`.
`sixth-manifest.json` binds sources, inputs, fixtures and regenerated references.
`sixth-equivalence.json` checks all 400 prior status/stdout/object outcomes;
the ordinary-alias fast path changes none. All eight sequential final check
groups pass: PA21, strict report, debug-info, variants, self-host through PA5,
all nine architecture audits, exact file audit and placement. The strict log is
exactly one line, **6038/6038**, and placement has zero early violations. The
existing substantial-header warnings remain warnings.

Alpha's final broad run retains 816 observations across seventeen frozen inputs.
All native outputs are identical; the 1.005 instruction and 1.03 RSS gates pass.
Maximum broad A/B deltas are +0.2107% instructions and +0.1732% RSS. Reference
aliases use fewer instructions than entry after the typed fast path.
Earlier focused runs using separate byte-identical baseline files exhibited
A/A cycle drift and failed calibrated cycle gates; all those observations and
candidate attempts are retained, not counted as final passing gates. A fourfold
input run retained that separate-file calibration signal. A final balanced,
interleaved 768-observation run uses the same immutable baseline executable
for both A/A labels and freezes the larger reference inputs before measurement.
It passes both raw and calibrated 1.005 cycle gates for recognition, reference
aliases, nonthrowing reference backing and EH handlers. Raw/calibrated ratios
are respectively 0.997077/0.997763, 1.001074/0.995486,
0.996356/0.996439 and 0.999172/0.999394. Focused instruction/RSS gates also pass.
The alias A/A ratio remains 1.005613, so the raw passing ratio is reported
alongside calibration rather than treating its calibrated decrease as a speedup.

`sixth-performance-verification.json` validates every final observation, output
hash and gate. All completed counter runs from this checkpoint retain **7,920**
observations, plus the separately recorded failed permission launch. The partial
409-observation download is retained as a prefix of its completed run, not counted
again. Raw logs/counters and frozen binaries are local under `perf/`, and Alpha
retains all generated objects at
`/tmp/cppgm-v4-audit-review-20261001-reference-initializer/`.
The student export remains deferred until the complete tracker is resolved.

## Read-only PA29 completion / PA30 implementation197 refresh

The read-only snapshot is student commit
`94faedf12200563a8886acf21ee1c0ada8c998b3`, compared with the previously reviewed
`1b19e9ac18eba368b76e1b6389e5126fe5f60a5a`. Exact history, 46 changed plan/audit/
handoff/proof documents, 477 personal source/control files and their hashes are
retained under `/tmp/cppgm-v4-audit-review/student-refresh-pa30-197/`.
The snapshot had only untracked `student.tests/pa30/evidence197/`; this checkout
was never modified or used to run our controls. The active student may advance
after this pinned observation.

Only seven supplied sidecars changed since that checkpoint; no supplied test
input, handout, harness, discovery or comparison rule changed:

| Student change | Independent disposition here |
| --- | --- |
| Three PA29 trait success-status sidecars in correction189 | Their undefined primaries/false declared values cannot support the asserted successes. HOST-SHORTHAND already fixes these goals here with actual definitions and lower owning milestones; do not revert those positive fixtures or merely copy the student's rejection sidecars. |
| PA29 `400-host-gnu-hex-float-pp-number.ref`, correction192 | Q now has binary128 type/bytes rather than the old x87 long-double bytes. Our scratch preprocessor output still has the old representation. The mathematical encoding and both host typed controls corroborate the correction. GNU extension/header necessity must be established before expanding this fix queue, as requested below. |
| PA6 `300-ambiguous-using-directive-type-bad` output/status/stdout, correction197 | The input's two typedefs both name int and should converge. O0/O2 Clang/GCC compile it, while entry/candidate and both semantic dump modes reject. N3485 7.1.3 and 7.3.4 supply the C++11 proof. LOOKUP-NAMESPACE owns the compiler correction and subsequent generated reference. |

The original PA30 namespace-positive fixture also passes here, but declares a
global typedef that hides the competing imported typedefs; it does not prove
the missing convergence rule. The student's transitive/class/namespace-alias
positive catches that gap. Distinct imported types, variables and namespace
targets remain rejection boundaries. The base-member-typedef negative was not
changed by the student. Clang accepts it; GCC and ours reject. N3485 10.2/3
normalizes type declarations before merging lookup sets, so LOOKUP-BASE-ALIAS
retains this as a separate rule/contract review, without changing an oracle
solely on one host's result.

`targeted-corrected-controls.json` retains 606 O0/O2 observations for 101 selected
inputs with the immutable reference-cleanup candidate and both hosts. The
initial trial supplied an unsupported gnu++23 flag to our compiler for extension
inputs; that launch error is retained in `targeted-controls.json` and is not
counted as a compiler defect. The corrected run uses our supported C++11 driver
mode. Host extension tests explicitly use their required dialect. A further
202 entry observations have identical status/stdout outcomes, proving these
new findings predate the pending cleanup fix. Twenty-four independent reduced
observations isolate namespace convergence, constant member-pointer truth and
constant bit-field truncation.

The new C++11 fix rows cover hidden friend/nested tag lookup, dependent type
demand after a later primary definition, access substitution fallback, assertion
message syntax, member-pointer constant truth and bit-field constant conversion.
The current-instantiation construction/qualified-member positive composites,
partial-alias ordering positive and ambiguous-partial rejection already pass
here and with both hosts; the student's fixes are not presumed necessary here.
These ordinary language reducers belong to their earliest semantic owners,
not automatically PA29/30 because that is where the student discovered them.
No additional explicit misplaced-fixture claim was found in the refreshed
plans/audits. New required fixtures must still pass placement checks.

The newer personal controls are a mixture of language versions. Per the user's
scope clarification, conditional explicit, structured bindings, inline variables,
static call/subscript operators, bit integers, extended floating formats,
representation/vector builtins and complex/tag extensions are retained as
observations, not automatically added to the fix queue. A concrete selected
hosted-header use in the supported dialect must establish necessity first.
One dependency is now independently identified: the PA29 handout's supported
libc++ profile uses conditional explicit in its C++11 pair constructor branch.
[LLVM release/21.x pair.h](https://github.com/llvm/llvm-project/blob/release/21.x/libcxx/include/__utility/pair.h)
lines 140, 147 and 162 are outside a C++20 guard. The frozen copy
`libcxx-21-pair.h` has SHA-256
`28ee2f80a5d61a596a77e4588ab03b7a9780da06ff956a40018703ebfb4b1496`.
Eight further Clang/GCC C++11-mode observations corroborate the constructor
positive/negative boundary; EXPLICIT-CONTEXT is admitted on this header proof.
Its deduction-guide controls are held without a separate necessary-header proof.
Our reproduced quad type/classification and signaling-NaN failures,
including the NaN compile crash, remain in this evidence rather than becoming
new required fixtures without that scope proof.

`abi-tag-controls.json` retains 156 commands on 39 source193 controls, with
Clang C++11 O0/O2 symbols checked before any encoder change. Some declaration
tags disappear here and some explicit-member tags remain when Clang removes
them; raw global-variable differences repeat ABI-GLOBAL. There was no encoder
change. These GNU attribute observations also require the hosted-header scope
proof before adding new tag-policy work. The student's used-before-specialization
negatives are C++11 ill-formed-with-no-diagnostic-required boundaries, so their
acceptance here alone does not justify mandatory rejection fixtures.

The refreshed audits disclose several personal expectations which deliberately
do not use the supplied reference or universal host agreement: zero-sized GNU
class arrays have differing host constructor/destructor counts; dependent complex
component mangling follows GCC where Clang erases the operator; contextual
explicit positives and demanded-definition negatives use different hosts;
polymorphic typeid-in-default rejection disagrees with Clang; and fixed false
assertions in dormant templates are demanded in host controls because modern
Clang defers them. Excess-precision and optional-copy observations remain review
evidence. Audit178 narrowed a personal no-builtin-call inspection to permit
declarations; audit182 corrected a personal source/prepared-LowIR comparison to
validate each phase and compare final objects. Neither weakened supplied tests.
At this pinned snapshot no evidence establishes another supplied-reference
correction beyond the seven sidecars listed above.

### Active audit198 follow-up

A final read found student HEAD `c0b26910289ea30848547c968f3ece8625c3c7f4`,
with active, uncommitted audit198 changes in semantic lookup, LowIR symbol
presentation and the PA30 same-type base-alias status/stdout sidecars. The input
and empty `.ref` remain unchanged. The new proof, source hashes, status and diff
are frozen read-only under `/tmp/cppgm-v4-audit-review/student-refresh-pa30-198/`.
The working-state snapshot does not claim the student has completed validation
or committed those active changes.

The base-type proof follows the N3485 wording independently read above:
normalization makes both declaration sets `{int}`, so the equal-set merge
is valid. Audit198 expressly supersedes its earlier claim that independently
declared member typedefs must remain ambiguous. LOOKUP-BASE-ALIAS is now an
open C++11 fix, with surrounding identity/access boundaries retained.
Forty-two O0/O2 observations on seven new controls retain compiler/host outcomes.
The expanded base-alias positive fails here and passes Clang; GCC's disagreement
is recorded rather than substituted for the rule proof.

The new namespace/member `main` control passes our direct object route and the
explicit LowIR/native adapter, as well as both hosts. The student's reserved
presentation-label fix is therefore not presumed necessary here. These active
discoveries exercise C++11 lookup and functions; no newer-language feature is
admitted by this follow-up. The additional two active sidecars bring the observed
reference delta to nine, with all supplied source inputs and harnesses preserved.

### Initializer-list lifetime work in progress

The current REF-INIT-LIST exploration is frozen under
`/tmp/cppgm-v4-audit-review/initializer-list-reference-lifetime/`. N3485 8.5.4/6
extends a backing array when the initializer-list object is initialized from
it, exactly as a reference lifetime extension; 18.9/2 says copying the wrapper
does not copy its elements. This is C++11 work, independent of hosted-header
extensions.

The one-line duplicate-registration patch fixes the original reducer but is
insufficient. The expanded complete-definition controls also expose a 16-byte
list object written into an 8-byte reference slot, and a broad temporary scan
which extends call arguments. The pending implementation uses normal temporary
materialization for the list object and follows its typed binding recipe for
backing ownership. It is not validated or ready to commit. The second candidate
retains compile failures at direct list-reference storage and a partial-array
unwind which skips the earlier automatic object. These results remain in
`second-controls.json`; no failed observation is claimed as a pass.

The original matrix deliberately retains exploratory expectations which require
correction or review: conditional list-wrapper copies do not extend the backing
array through the copy (both hosts destroy it before the next statement), while
comma and parenthesized-brace controls show host disagreements. These will not
be promoted to required positive fixtures on one host result. The const auto&
control exposed the independent AUTO-CONST-REF row above. Earlier malformed
control generation and incomplete-list/companion setup failures are retained
in separate scratch directories and are not compiler-bug evidence.

The third immutable candidate materializes a separate list object before
reference conversion. All direct noexcept copy-list, direct-list, rvalue-reference,
empty-list and wrapper-alias controls now execute successfully at O0/O2; empty
references no longer crash. `third-controls.json` retains all 72 observations.
Throwing second-element initialization still skips the earlier automatic
object, and explicit-cast binding still fails its lifetime check. Those remain
required work before this checkpoint can be marked done. No full-suite or
performance result is claimed for these intermediate candidates.

The fourth immutable candidate has SHA-256
`fc11a9e94b0d50d3fb77371b4193b82bbfac68924daf1d0e24b1359dadba63df`.
The partial-array landing now enters the existing enclosing construction/full-
expression cleanup continuation, which preserves earlier automatic objects in
both the inline and loop backing-array paths. Functional list construction uses
the direct list recipe rather than an artificial wrapper copy. Reference storage
retains the separately materialized list object.

`v2/fourth-controls.json` retains 228 observations. All 36 agreed programs pass
with ours/Clang/GCC at O0/O2. The two additional value-functional programs expose
a host disagreement: Clang and ours destroy the backing before the next
statement, whereas GCC extends it. They remain review evidence rather than
required positive fixtures. `v2/entry-controls.json` retains 76 entry observations
on exactly the same frozen inputs. `storage/{entry,fourth}-controls.json` each
retain 42 observations; global/rvalue/static/scalar references improve, the
mutable-reference rejection improves, and the two independent preexisting
static-value/volatile-reference issues are tracked above.

Sixteen PA21 required fixtures use a complete local definition of the permitted
standard initializer-list model, with no hosted includes. The final fixture
text is independently compiled and executed by both hosts and ours at O0/O2:
`fixture-controls/fourth-controls.json` has 90 positive executions and six
expected mutable-reference compile rejections. The first host launch omitted
`-x c++` for the `.t` suffix; its setup failures are retained separately and are
not counted as compiler defects. `fourth-regression-equivalence.json` proves
all 274 previous cleanup observations retain their compile/link/runtime statuses
and stdout.

The initial strict report has only one changed existing reference,
`200-initializer-list-backing-array-lifetime.ref`; the source is unchanged. Its
new guarded landing destroys an already completed backing array when a later
initializer expression throws. Only this independently covered reference and
the sixteen new fixtures are regenerated through `ref-test`. Full validation
and the Alpha performance gates are still running; this row is not yet done.

### Initializer-list lifetime checkpoint validation

The final fourth candidate and all sixteen fixture inputs retain their frozen
hashes after validation. `validation-fourth/validation.json` records all eight
check groups passing: affected PA21, full strict report, debug-info, backend
variants, PA34 self-host through PA5, nine architecture audits, file audit and
placement. The strict log contains exactly one final 6054/6054 success line.
Placement scans 3,179 fixtures with zero findings and zero local hygiene findings.
The file audit has no errors; its existing advisory warnings remain.

`fourth-original-controls.json` retains sixteen observations on the original
forward-declaration reducers, including the measured final live-object count.
The final candidate returns zero for the success reducer and three for the
`alive+3` observation, matching both hosts at O0/O2; entry returns two and one,
respectively. `fourth-host-trace-verification.json` verifies all agreed v2 and
required-fixture stdout traces match both hosts exactly. The earlier 100 expanded
reference-cleanup observations also retain their statuses and stdout, as proved
by `fourth-expanded-equivalence.json`.

Alpha measurements are retained at
`alpha:/tmp/cppgm-v4-audit-review-20261002-initializer-list/` and downloaded under
`perf/`. Frozen entry SHA is
`366793492b521f0d772e98625981796dc48a179f54317bd630cf80ba3fd8c5ce`; final
candidate SHA is recorded above. `complete-input-verification.json` on Alpha
verifies all 83 binary/source/header hashes against the local frozen manifest.
The A/A commands execute the same actual entry image as the A/B entry commands.
`verify-final-performance.py` and `fourth-performance-verification.json` retain
864 broad observations on eighteen workloads, 768 balanced interleaved focused
observations on four workloads, and 384 interleaved observations on the two
broad workloads with calibration drift. Every generated object agrees exactly.
All instruction and RSS gates (1.005/1.03) pass; maxima are +0.2265% and +0.9299%.
All raw cycle gates pass, and the six interleaved focused/calibrated gates pass
(1.005), with a maximum calibrated ratio of 1.004845. The initial broad
conditional/virtual-large calibration differences are retained, rather than
discarded; the interleaved follow-up ratios are 1.001477 and 0.983517. The initial
missing-symlink setup launch contains no measurements and is retained separately.

REF-VOLATILE also has an independent ordinary scalar/class reduction:
`volatile-reference/controls.json` retains 24 C++11 compile observations. Entry
and candidate accept both rvalue negatives, while Clang/GCC reject; every compiler
accepts the volatile lvalue positive. N3485 8.5.3/5 requires a non-volatile const
lvalue reference in the temporary-binding alternative. This evidence establishes
the new row without attributing it to the completed lifetime patch.

This checkpoint closes REF-INIT-LIST only. The other open tracker rows and final
combined student export remain required work; no student export is regenerated
between fixes.

## C++11 auto-reference deduction candidate

The pending AUTO-CONST-REF candidate starts from `a3aaf2015`. Its frozen image
has SHA-256 `7902309d70743a55d5fd27f43ebac0949c77db47ea7afa7868fc20dd2c72313c`;
sources and controls are retained under
`/tmp/cppgm-v4-audit-review/auto-const-reference/`. N3485 7.1.6.4/6 explicitly
models `const auto &i = expr` by deduction against `const U&`. The patch admits
non-volatile const rvalue binding, ignores function cv qualification, preserves
the typed reference initializer and constructs class elements of deduced lists
with their ordinary copy recipe. No newer-language syntax or ABI encoder change
is included.

Thirteen ordinary deduction fixtures belong to PA20:100; four initializer-list
companions belong to PA21:200. The initial fixture run retains 102 O0/O2
observations: 90 successful executions and twelve expected compile rejections.
Three PA20 helpers are subsequently marked `noexcept` to keep incidental EH
lowering out of their milestone; `final-controls.json` adds eighteen successful
observations for those changed sources. Both hosts also check all seventeen
final sources with `-std=c++11 -pedantic-errors`: all fifteen positives compile,
and both negatives reject. Host extension acceptance is not used as language
version proof.

The initial twenty programs and eighteen agreed expanded controls retain their
host boundaries. CONST-REF-STATIC-TEMP and REF-ARRAY-CV remain independently
reproduced explicit-type owner gaps. All 274 prior cleanup and 100 expanded
reference observations have identical status/stdout outcomes to the preceding
checkpoint. Of six added fault programs, four agree with both hosts at O0/O2;
the two partial initializer-list programs establish EH-LOCAL-ARRAY-CATCH above.
Their explicit-type entry controls prove that this same-function-handler failure
predates deduction changes. No failed boundary is silently counted as passing.

The initial strict report passes 6071/6071, as do PA20/21, debug information,
variants, self-host through PA5, architecture and file checks. The initial
placement check identifies only the three helper EH hygiene issues described
above. Following targeted `ref-test` regeneration, final PA20 and strict checks
pass again, with one report line for 6071/6071; final placement checks scan 3196
tests with zero placement or hygiene findings. No existing reference source or
oracle is changed.

Alpha retains 960 broad and 1152 focused observations, followed by 1152
interleaved cycle-review observations for all six timing leads. Object hashes
agree throughout, and all 85 frozen input hashes match remotely. Instruction
and RSS gates pass; the deduced-class-prvalue workload executes approximately
13.6 percent fewer compiler instructions after avoiding repeated initializer
analysis. Timing is still under review: the first focus EH-handler raw ratio is
1.005870 (calibrated 1.004637); the follow-up passes that boundary but retains
virtual raw/calibrated ratios 1.005930/1.008190 and initializer-list calibrated
ratio 1.005054. These observations are preserved in `second-gate-review.json`
and `cycle-review/`, and are not represented as a completed performance gate.
This second-image trial did not close AUTO-CONST-REF; its measurements are kept
as failed timing evidence. The final image and closure evidence follow below.
The full goal and deferred combined export remain open.

The final third image has SHA-256
`e4bcdecf0e30356f800ccebd7ba88e6686ed261f479874c50fe566499d78af6c`.
It classifies the deduced list's element once and skips qualification queries
when no cv qualifier is added. Both adjustments preserve the required typed
construction recipe while avoiding repeated work. All original twenty controls,
the eighteen agreed expanded cases and the two separately recorded owner gaps
retain identical outcomes; all seventeen final fixtures pass their 102 O0/O2
compile/link/run observations. Repeating the 274 cleanup, 100 expanded reference
and twelve fault observations with this final image produces no status/stdout
change, including the two independently retained same-function-handler failures.

`validation-third/validation.json` records every required command with status
zero: PA20, PA21, strict report, debug information, variants, self-host through
PA5, all nine architecture audits, file audit and placement. The strict log is
one line for 6071/6071. Placement scans 3196 tests with no placement or hygiene
findings. No ABI encoder, earlier fixture source or existing reference changes.

The final Alpha study retains 960 broad and 3072 focused observations. The
eight focus workloads use 48 balanced interleaved A/A and A/B blocks each;
calibration executes the same entry image. Every object agrees between images,
and all 86 frozen input hashes match. `perf/third-gate-review.json` records the
1.005 instruction and 1.03 RSS gates for all twenty broad and eight focused
workloads, and the 1.005 raw/calibrated cycle gates for all eight focus workloads.
Maximum instruction growth is 0.0035 percent; maximum RSS growth is 0.4839
percent. Focused raw/calibrated cycle maxima are 1.001958/1.003148. The broad
copy-competing calibrated ratio 1.006089 remains a diagnostic: its raw ratio
1.002827 passes, and the separate broad calibration is not treated as an
interleaved cycle gate. The second-image extended timing study has 2048 further
observations. All 9344 observations across both images are retained; none is
discarded to claim a passing final study.

This closes AUTO-CONST-REF only. The explicit static-temporary, array cv,
pointer qualification, volatile-reference and local array-handler owners remain
in the unified queue, along with every other open item and the final export.

## Read-only PA30 implementation200 refresh

The next pinned student commit is
`b0790726de6c67722826833b395b76853e051e2d`. Its snapshot under
`/tmp/cppgm-v4-audit-review/student-refresh-pa30-200/` retains 57 files, including
44 personal sources and the plan/audit/design/performance documents. A full
path-filtered Git delta from the previous `c0b26910` snapshot changes only the
two PA30 same-type-base-alias sidecars already reviewed in LOOKUP-BASE-ALIAS.
No further supplied source, handout, harness, discovery or comparison rule
changes. The active implementation201 work has no supplied-fixture changes in
the observed working state. This tree is still read-only; controls use copies.

Twenty-eight ordinary-language sources yield 168 O0/O2 observations using the
final deduction image and both hosts; host compilation uses
`-std=c++11 -pedantic-errors`. The mismatched inherited return-type definition
and inaccessible array-new destructor are new review leads above. Other
aggregate-member, inactive-union and undemanded enclosing-deleted-constructor
controls disagree between Clang and GCC, so the student's negative labels are
not adopted as mandatory oracles. Matching inherited definitions, access/friend
controls, selected-union cleanup, complete-enclosing-class positives, scalar new
with a private destructor and ordinary dependent new forms retain their
independently observed boundaries. Vector extensions and compiler-generated
integer-sequence controls remain held without their own necessary-header proof.

The personal `source199/new-placement.cpp` checks the original buffer at the
returned array's subscript, assuming no array-allocation overhead. Its divergent
runtime result here is not a C++11 compiler bug proof: N3485 5.3.4/12 explicitly
permits overhead for placement array new, including the standard void-pointer
form. A copied reducer checks the returned `p[argc]` instead; all six O0/O2
ours/Clang/GCC executions pass. Preserve the allocation/construction goal with
that portable observation rather than copying the zero-overhead assumption.
No additional claim that a new personal test deliberately differs from the
supplied reference is present in these implementation199/200 handoffs; the
earlier host-disputed extension expectations remain recorded separately.


## C++11 reference qualification review

The current entry is `1de7a8cbc`, frozen as
`reference-cv-qualification/compiler-after-auto` with SHA-256
`e4bcdecf0e30356f800ccebd7ba88e6686ed261f479874c50fe566499d78af6c`.
The refreshed 26 core, nineteen expanded and five pointer-alias programs
retain 300 O0/O2 observations with ours and strict C++11 Clang/GCC. Six
additional pointer-prvalue/xvalue controls add 36 observations per image.
Inputs, commands and outcomes remain under
`/tmp/cppgm-v4-audit-review/reference-cv-qualification/`.

N3485 3.9.3/5 gives arrays their element cv qualification; 8.5.3/4-5 requires
qualification preservation, a non-volatile const lvalue-reference for indirect
binding, and rejection of a related lvalue for an rvalue-reference. The unsafe
deep-pointer conversion is excluded by 4.4/4's intermediate-const rule. These
are C++11 corrections. No newer syntax, builtin or encoder feature is admitted.

Modern hosts also implement later corrections to reference compatibility.
[CWG 330](https://cplusplus.github.io/CWG/issues/330.html) was moved to DR in
2014, with the array-of-pointers wording in
[N4261](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4261.html).
[CWG 2352](https://cplusplus.github.io/CWG/issues/2352.html) was accepted as a
DR in 2019 and changes similar-pointer reference binding. The five alias
controls preserve observed existing behavior without demanding a new copied
pointer temporary. The mutable-pointer-xvalue conversion to a const-pointee
rvalue reference disagrees between Clang and GCC; it remains review evidence,
not a required acceptance fixture. The bit-field cast mutation control also
has a host disagreement, separate from the three agreed bit-field failures.

The candidate streams qualification levels without allocating cv vectors or
interning virtual pointer types. A reference includes its referent's own cv in
the intermediate-const check; arrays retain their bounds and element cv.
Ordinary scalar/class bindings exclude volatile lvalue-reference temporaries,
and a base rvalue reference rejects a derived lvalue. A pointer prvalue can
still undergo a valid qualification conversion to a temporary. The first image
missed that last boundary and changed the existing PA12 rvalue-overload fixture;
its failing strict report is retained. The second image restores the existing
reference without regenerating it. Its SHA-256 is
`fc299a4572716244041e42c67bfb2579d7e67812ccf253448593a5530af717ff`.

Twenty-two independently reviewed fixtures live in PA10's reference/cast
clusters and PA12's class-value cluster. All 132 ours/Clang/GCC observations
agree: nine positive programs execute successfully and thirteen negatives
reject. Generated sidecars come only from their exact `ref-test` selections.
The candidate's 26 core cases all agree with both hosts; the five pointer-alias
boundaries remain unchanged. The agreed non-bit-field expanded cases also
agree. The 274 cleanup, 100 expanded-reference and twelve fault observations
retain their previous status/stdout outcomes. Bit-field reference materialization
remains a separate C++11 fix in REF-BITFIELD.

The strict report passes 6093/6093 with one output line. All eleven validation
commands in `validation-second/validation.json` succeed: PA10, PA12, PA20, PA21,
strict report, debug information, variants, self-host through PA5, the nine
architecture audits, file audit and placement. Placement scans 3218 inputs with
zero placement/hygiene findings. The second image does not pass the performance gate;
no work item is closed on functional checks alone. The first timing trial retains
960 broad observations and a
65-observation incomplete focus run: the extra array benchmark incorrectly
used an adjusted array parameter. Its corrected declaration compiles with
equal entry/candidate objects. A subsequent upload failed to replace a
read-only remote image; hash verification exposed the wrong-image trial,
which is retained separately and is not used to judge this candidate.
The fresh final study verifies all ninety input hashes before launching any
measurements and uses the actual entry image for A/A calibration. The final
combined export remains deferred until the full fix sequence is complete.


The completed second-image study retains 960 broad and 3456 focused observations
in `perf-final-results/`, with equal objects and verified input identities.
Instruction and RSS gates pass. Five focused cycle gates fail: auto aliases
(raw 1.005207), auto prvalues (raw/calibrated 1.009293/1.008811), EH handlers
(1.006429/1.006348), initializer-list values (1.007915/1.007056), and the added
qualification views (1.006645/1.006751). `perf-final/gate-review.json` preserves
all results. The wrong-image study retains 960 broad and 366 completed focus
rows, plus its raw in-flight measurement files. It is not candidate validation.
The initial failed benchmark and wrong-image studies remain separate from this
completed study; none is discarded to manufacture a passing timing result.

The third image has SHA-256
`e86b966c092ab3aa8186abff53606f9a71691bc4083320133b1f85814dfec926`.
It caches the effective source type and defers array qualification queries until
an unchanged lvalue-reference fast path has been ruled out. All 132 fixture
observations agree with the second image, as do the core reference, pointer
alias and pointer-rvalue boundaries. The expanded invalid bit-field programs
remain accepted and can produce different garbage runtime results; their missing
diagnostic is the separately tracked REF-BITFIELD bug, not a runtime oracle.
The completed third-image study retains 4416 observations in
`perf-third-results/`. Objects match, and instruction/RSS gates pass. The
focused cycle gates fail for auto prvalues (raw/calibrated
1.002996/1.005622) and qualification views (1.010357/1.011424). Its functional
validation reaches the file audit, which rejects analyzer.cpp at 3008 lines.
The qualification helper subsequently moves beside ArrayElementCv in the
existing calls.cpp module, with the semantic ownership ledger updated.

The fourth image, SHA-256
`2e78b44abfba2c8da92883cb03c59c098d2d66597e5afd167909d3dacef2d7ad`,
passes all eleven validation commands, including the file audit. Additional
volatile-array boundary controls uncover six false rejections introduced by
counting an array and its qualified element as separate pointer levels.
No timing study is launched for this known incorrect image. The fifth image
walks array dimensions without adding a qualification level; it retains the
array bounds and checks the actual element qualifiers and intervening pointers.

Eight additional PA10 reference/cast fixtures cover volatile scalar arrays,
multidimensional arrays, array pointers, volatile pointer elements, and the
unsafe pointee/deep-pointer boundaries. All thirty fixtures retain 180 agreed
ours/Clang/GCC observations at O0/O2: fifteen positives run successfully and
fifteen negatives reject. The eight new array controls add 48 agreed
observations. The existing 300 reference controls, 36 pointer-category controls
and 386 previous regression observations retain their expected boundaries;
only the separately documented bit-field and host-disputed cases differ.
Sidecars are generated only through each exact ref-test selection.

The fifth image has SHA-256
`7e11d9c887ccf959a253c14aa66ff0e50699ee2bb84ba6f1833344c268bab88f`.
All eleven checks in validation-fifth/validation.json pass. The strict report
prints exactly one line, 6101/6101, and placement scans 3226 tests with zero
placement/hygiene findings. Its completed Alpha study verifies all 91 input
hashes and retains 960 broad plus 3840 focused observations in
perf-fifth-results/. All objects match, and instruction/RSS gates pass.
The focused cycle gates fail for EH handlers (raw/calibrated
1.009143/1.013147), qualification views (1.012187/1.014563), and volatile views
(1.008290/1.008031). perf-fifth/gate-review.json preserves every result.

The next candidate reuses the already-read source and referent records for
qualification stripping and cv checks, instead of repeating type-table reads.
The qualification walker reads records through pointers rather than copying
and reassigning full TypeRecord values. No type-table mutation occurs while
these records are read. The compiler change and its fixtures remain
uncommitted until performance validation succeeds. The four reference owners,
bit-field owner, other open items and final combined export remain open.

The sixth candidate is frozen with SHA-256
`fe4d5baeec6ae760882d9b58b0e882969d96614d30e7db25e4d8bdc24158ebd3`.
All 950 control/regression observations retain the intended boundaries; the
accepted invalid volatile bit-field reference continues to have arbitrary
runtime results and stays in REF-BITFIELD. sixth-matrix-review.json records
those comparisons. All eleven checks in validation-sixth/validation.json pass, including the
single-line strict 6101/6101 report, self-hosting and placement over 3226
inputs with zero placement/hygiene findings. The fresh immutable Alpha study
has completed with 4800 equal-object observations, retained in perf-sixth-results/.
Instruction/RSS and all raw focused cycle gates pass. Calibrated cycle gates
fail for auto aliases (raw/calibrated 1.003820/1.005141), qualification views
(1.004462/1.009575) and virtual overrides (1.004578/1.005248). No owner is
closed on this partial performance result. A separate qualification-workload
profile is launched only after the balanced measurements have ended.
Its manifest verifies 87 compilation inputs before measurement; the
four unchanged harness scripts are independently checked against their frozen
fifth-study hashes in verified-harness-scripts.json. No input or image is
changed during measurement.

Eighteen reduced namespace controls are prepared independently in
namespace-convergence/, with 144 corrected entry/host observations across
C++11 O0/O2 and our two semantic dump modes. All host results agree with the expected
boundaries. Seven positive typedef/class/array convergence cases reject here;
the direct-typedef masking case, same namespace aliases, repeated variable
entity and repeated function entity already pass. All seven distinct-type,
cv, array-bound, class, variable, function and namespace-target negatives
reject here and with both hosts. This narrows LOOKUP-NAMESPACE to equivalent
type merging; existing same-namespace/entity convergence needs preservation,
not an assumed additional compiler fix. No namespace implementation change
has begun before the reference qualification checkpoint passes its gates.

The separate sixth-image qualification profile retains both perf data files,
reports and all 256 successful equal-object compilations in profile-sixth/.
TypeTable::Get appears among the measured owners; conversion/qualification
functions are each below the report's 0.1% cutoff, so this is not evidence for
a large direct helper cost or a basis to change unrelated compiler modules.
The next candidate stops walking once the remaining canonical type identities
are equal. Equality already establishes identical remaining qualification,
array bounds and type structure, avoiding two redundant type-table reads of
an unchanged terminal suffix. This is an ordinary typed-fact optimization;
calibrated gates remain required and no failed trial is discarded.

The seventh candidate is frozen as compiler-seventh, SHA-256
`b6d938ab29a1308e244989bfb9fee4f9b5ad3ef043cdfbd2dc1ade25f424296c`.
The complete Alpha manifest verifies all 91 inputs, including harness scripts,
before any observation. The fresh study is under
/tmp/cppgm-v4-audit-review-20261002-reference-cv-seventh/; its immutable A/A
labels both invoke compiler-a. validation-seventh/ and the same 950 controls
are running. The prior sixth-image study and separate profiling data are
retained in full. Compiler code, thirty fixture sets and the semantic owner
ledger remain uncommitted pending the required performance gates.

The initial namespace preparation invocations incorrectly included
-std=c++11 in our dump modes, which do not accept that flag. Their 48 original
phase observations (36 core and twelve extra) are retained as invocation errors,
not semantic evidence. corrected-entry-controls.json and
extra-corrected-entry-controls.json rerun all 192 observations with valid
dump-mode commands and retain strict C++11 flags for ordinary compilation.
The seven typedef/type convergence positives still fail in both dump modes,
while the direct masking, same namespace/entity and distinct type/entity
boundaries have the expected results. Four additional type/value namespace
conflicts are incorrectly accepted in every current mode; both hosts reject.
N3485 7.3.4/6 supplies the explicit no-hiding rule. LOOKUP-NAMESPACE-MIXED
tracks this independent diagnostic gap. The elaborated-class and local-type
hiding positives pass every mode, so the eventual diagnostic must respect
lookup kind and scope. These controls expose a preexisting bug, rather than
a regression introduced by the pending reference or namespace changes.

The seventh image passes all eleven functional/audit checks and all 950
control/regression boundaries. Its completed 4800-observation Alpha study
passes object equality and instruction/RSS gates but fails focused cycle gates:
auto aliases (raw/calibrated 1.020689/1.020386), EH handlers
(1.007124/1.005222), qualification views (1.019971/1.021345), reference aliases
(1.012242/1.011554), reference temporaries (1.004812/1.005448), virtual
(1.009283/1.008743), and volatile views (1.016070/1.016167).
perf-seventh-results/ and perf-seventh/gate-review.json retain all observations.
The terminal-identity shortcut is removed; the compiler source and owner ledger
match sixth-source.patch exactly. No unrelated optimization or layout padding
is introduced to manufacture a favorable sample.

A fixed extension of the sixth image is declared before launch in
perf-sixth-extended/predeclared-study.json: 128 additional balanced interleaved
A/A and A/B blocks for each of the same ten focused workloads, using the
original immutable images and identical source paths. All 10240 additional
observations will be pooled with all original 3840 sixth-image focus
observations; none of the original failed calibration data will be excluded.
The original 960 broad observations remain part of its final gate. All 91
original inputs and the new harness identity are checked before measurement.
The instruction/RSS and raw/calibrated 1.005 cycle thresholds are unchanged.
This resolves the sixth study's asymmetric A/A calibration signal with a
larger predeclared sample; it does not reclassify a failed gate as a pass.

## Required-fixture growth and current coverage review

Since the initial fb15cd49e harness checkpoint, the branch adds 255 committed
numbered test input files, modifies eight and removes three. Thirty reference
qualification inputs remain pending. PA21 accounts for 91 of the committed
additions. These counts are source inputs, rather than independent bug counts;
compiler/optimization combinations and repeated timing runs are observations.
The expansion combines student/Argon discoveries, independent language proof,
and boundary controls generated during shared-owner repairs. Some original
references encoded incorrect behavior (the PA6 same-int typedef rejection is
one independently confirmed example), while some interactions had no supplied
fixture. Positive preservation controls are also required to keep a repair
from rejecting legitimate neighboring programs.

fixture-baseline-review.json checks all thirty pending inputs against the
immutable compiler-after-auto entry at both O0 and O2: eighteen expose
preexisting failures, and twelve already passed and guard valid/rejected
boundaries. Six of the latter caught false rejections introduced by the fourth
qualification candidate, before any compiler checkpoint was committed. Thus
these thirty inputs do not imply thirty previously missing implementations.
Required coverage should be justified by distinct contract boundaries; the
larger exploratory and timing matrices remain scratch evidence. This review
establishes the current thirty-input classification and does not claim that
all 255 committed additions have completed a redundancy review.

The namespace lookup preparation also retains sixteen agreed positive
observations for mixed class/value and alias/value names before ::. N3485
3.4.3/1 explicitly restricts that lookup to namespaces/types/type templates;
it must remain different from ordinary declaration lookup's no-hiding rule.
The upcoming LOOKUP-NAMESPACE-MIXED diagnostic must preserve this existing
scope-carrier behavior. Commands and outcomes are in
namespace-convergence/qualifier-corrected-entry-controls.json.

The user explicitly requires fixture minimization as the final review after all
additions are complete. FIXTURE-REVIEW now records that requirement before the
combined export validation. The inventory must compare new fixtures with both
new and existing retained coverage; the current thirty-input classification
is evidence for that later review, not an assertion that all thirty should
ship separately. Keep broad exploratory matrices out of the default suite,
and preserve independent negatives when combining would let an earlier error
mask the behavior being checked. Final export validation must run against the
minimized fixture set and verify discovery and quiet report output together.

The predeclared sixth-image extension has completed successfully. Its pooled
review retains all 14080 focused observations plus the original 960 broad
observations, verifies output equality and passes instruction/RSS and all raw
cycle gates. Two calibrated cycle gates still fail: qualification views
(raw/calibrated 1.003665/1.005452) and virtual overrides
(1.004503/1.005853). The remaining eight focused workloads pass both cycle
checks. perf-sixth-extended/gate-review-pooled.json records the complete result;
perf-sixth-extended-results/ retains every additional raw measurement, manifest,
status and review. The immutable remote output objects remain available.
No original failed calibration observation is excluded and no threshold changes.
The four reference corrections and compiler checkpoint remain pending; further
progress requires a justified conversion-path improvement, not another blind
timing repeat. The user reiterates completing the additions first and keeping
the final fixture redundancy review deferred until that sequence is complete.

The eighth qualification image, compiler-eighth, has SHA-256
`2b48e83c30c9bb978b99c25a957fd558c046ce65a747f1b3619159c25fdda6bd`.
Conversion now reads the target record once and snapshots only its kind and
referent ID before any call that may mutate the type table, avoiding a full
record copy. Its compiled conversion function has 804 instructions versus the
sixth image's 821; this static count is not a runtime gate result. All 950
control/regression observations retain their expected boundaries, recorded in
eighth-matrix-review.json. Invalid accepted bit-field references remain outside
the runtime oracle and in REF-BITFIELD. All eleven required checks in
validation-eighth/ pass: the four affected suites, strict single-line 6101/6101
report, debug information, variants, self-host through PA5, nine architecture
audits, file limits and placement. Placement scans 3226 inputs with zero
placement/hygiene findings. A fresh immutable Alpha study verifies
all 91 inputs before launch and uses the actual entry binary for A/A. All
4800 observations complete with equal objects, and instruction/RSS gates pass.
Nine focused workloads pass both cycle gates. EH handlers still fail
(raw/calibrated 1.007373/1.009440). perf-eighth/gate-review.json records the full
result; perf-eighth-results/ retains every raw measurement, and remote objects
remain available. The compiler code and thirty fixture sets remain uncommitted.
Disassembly identifies a full-record copy in EffectiveType, which is inlined
into conversion/type-decay paths even though it consumes only kind and child.
The next candidate reads that immutable record by reference; the helper makes
no intervening type-table mutation. No failed study or threshold is discarded.

The next namespace addition is prepared as a compact positive covering
qualified scalar/array/shared-class aliases, namespace identity, transitive
cyclic imports and a local using directive. A separate distinct-type negative
preserves genuine ambiguity when the existing same-int PA6 oracle is corrected.
These two planned inputs remain in scratch until the preceding checkpoint is
complete; fixture-plan.json records destinations and coverage. Sixteen valid
entry/host observations in planned-fixture-entry-controls.json confirm strict
C++11 Clang/GCC acceptance of the positive and rejection of the negative.
The immutable entry rejects the positive in both dump modes and at O0/O2.
No exploratory namespace matrix is copied wholesale into required tests.

The ninth qualification candidate is frozen as compiler-ninth, SHA-256
`2b734a93e9196e335b8a97279ccca0c2ab1c22f0b873c6ae34fd4ea592be3d30`.
Its only change from the eighth image reads EffectiveType's immutable record
by reference. ninth-matrix-review.json retains 574 freshly executed candidate
observations and explicitly reuses 376 unchanged strict C++11 host observations
from the eighth matrix. All expected boundaries are preserved; arbitrary
runtime values from accepted invalid bit-field references remain non-oracles.
All eleven required validation groups in validation-ninth/ pass, including
the single-line strict 6101/6101 report, self-hosting and zero placement/hygiene
findings across 3226 inputs. The
new immutable Alpha study under reference-cv-ninth verifies all 91 inputs and
completes 960 broad plus 3840 focused equal-object observations, with unchanged
A/A calibration and thresholds. Instructions/RSS pass, but focused cycles fail
for auto prvalues (raw/calibrated 1.004920/1.009339), EH handlers
(1.009875/1.009353), initializer-list values (1.011085/1.011717), qualification
views (1.009268/1.008553), reference aliases (1.008364/1.009103), and volatile
views (1.007707/1.006734). perf-ninth/gate-review.json and perf-ninth-results/
retain the full result and every raw measurement. No previous trial is
substituted for this image.
The eighth study's 4800 time/counter files and observations are fully collected
locally; the ninth source patch and image are frozen before any measurements.
Reference owners remain open until both correctness and performance gates pass.

The tenth image is compiler-tenth, SHA-256
`233382a7dcf1565f0e837025b6ce553c940c32cb48c47fde1c253a651ad6b7a1`.
It removes twelve additional full-record copies from conversion's shared type
predicates, arithmetic rank/promotion, decay and similarity helpers. Reads in
the predicates and recursive similarity checks do not mutate the type table;
decay/parameter adjustment read kind/child before their terminal interning call
and never dereference the record afterwards. tenth-change.json records the
scope and lifetime review. Its source/image and all 91 inputs are frozen before
the new complete Alpha study. All eleven required correctness/audit groups
and the 574 fresh candidate controls pass, with 376 unchanged host observations
reused explicitly. The 4800-observation performance study verifies output equality
and passes instructions/RSS. Focused cycles fail for EH handlers
(raw/calibrated 1.005620/1.010129) and qualification views
(1.001766/1.008652); the other eight workloads pass both cycle gates.
perf-tenth/gate-review.json and perf-tenth-results/ retain all observations.
No new required fixture is introduced for these representation-only read changes.

Git evidence also corrects a consolidation coverage claim. The original
4b8ac246c parent source pa8/tests/320-using-directive-ambiguity-bad.t.1 declares
two distinct namespace variables and initializes result from their ambiguous
value name. Its individual source SHA-256 is
`3787a02c33b9d94f4d4780d88a2ba53a3e1c3126c3f5ced1f9e35b8be3f918a1`.
early-assignment-consolidation-inventory.tsv incorrectly lists the valid
same-int PA6 typedef program as covering that negative. The planned namespace
checkpoint therefore includes a PA7 distinct-variable negative retaining the
original source, separately from the PA6 type-convergence correction. PA6
excludes expression typing; PA7 owns the global id-expression in the initializer.
The frozen complete implementation also diagnoses this input in its type dump,
but that observation does not make the initializer a PA6 requirement. Twenty-four
corrected observations in planned-fixture-corrected-entry-controls.json confirm
both planned negatives with strict C++11 hosts; the composite positive still
exposes the namespace convergence bug. Initial phase-expectation metadata is
retained separately. Update the inventory's destination when the PA7 fixture is
installed; its historical baseline hash and historical lane inventory stay intact.

The eleventh image is compiler-eleventh, SHA-256
`892ea8b052e214e636a0f1c283257af3f1345c78e518978ebc13f2c1f008ad34`.
Conversion uses canonical identity immediately for identical non-reference
source/target types after the reference branch. Arrays and functions retain
normal decay. This skips repeated immutable type unwrapping for the ordinary
same-type case without bypassing reference category/cv checks. All 574 fresh
candidate observations retain their previous boundaries, with 376 unchanged
host observations reused; eleventh-matrix-review.json records the comparison.
All eleven required groups in validation-eleventh/ pass, including the one-line
strict 6101/6101 report, variants, self-hosting and zero placement/hygiene
findings. The complete immutable Alpha study verifies all 91 inputs and is
complete with 4800 equal-object observations. Instructions/RSS and eight
focused cycle gates pass. EH handlers (raw/calibrated 1.007692/1.011947) and
qualification views (1.003191/1.007344) still fail. perf-eleventh/gate-review.json
and perf-eleventh-results/ retain every observation. Compiler code and thirty
fixture sets remain uncommitted until every performance gate passes.

An independent baseline-only diagnostic investigates the A/A command-line
observer: unique output filenames differ by one character between a and aa,
although both execute the actual same compiler-a image. The predeclared
argv-calibration/ study retains 512 observations across fixed and unique output
paths, with all compiler arguments identical in each fixed group and each
object archived after measurement. EH median A/A ratios are 0.999992 (fixed)
and 1.000368 (unique); qualification views are 0.999265 and 0.999406. These
results do not show a meaningful output-filename effect and do not explain or
excuse the failed gates. All diagnostic raw counters, times and logs are
retained locally and objects remain remote. The original gate thresholds and
measurement harness stay unchanged.

The twelfth image is compiler-twelfth, SHA-256
`b17b0f6ea5528890f606716d70e1463f76136af906298d56178ae15917c6f996`.
The ordinary pointer path snapshots its two pointee IDs before any downstream
call and reads qualifier/void-pointee records by reference, removing five
remaining full-record copies from Conversion. No record is read across a
mutating call. All 574 fresh candidate observations preserve their boundaries;
376 unchanged strict C++11 host observations are reused explicitly in
twelfth-matrix-review.json. All eleven required groups in validation-twelfth/
pass, including the single-line strict 6101/6101 report, variants, self-hosting,
architecture/file limits and zero placement/hygiene findings across 3226 inputs.
The complete immutable Alpha study verifies all 91 inputs and retains 4800
equal-object observations. Instruction/RSS gates and eight focused cycle gates
pass. EH handlers (raw/calibrated 1.011892/1.014228) and qualification views
(1.004239/1.012619) fail the unchanged cycle gate. Every observation is retained
in perf-twelfth-results/, with the gate decision in perf-twelfth/gate-review.json.
A separate profile-twelfth/ study, started after timing completed, retains 512
successful equal-object compilations and all four profiles. Conversion and
QualificationConversion are each below the report's 0.1% cutoff in both images;
TypeTable::Get occupies 0.34%/0.33% of EH samples and 1.15%/1.38% of qualification
samples for entry/candidate respectively. These percentages do not establish a
new dominant owner and do not justify changes to unrelated compiler modules.

The thirteenth image is compiler-thirteenth, SHA-256
`5a9d61c287af1c6f4fc2825bae3c04c67a777a6ab4400d986de044c7a0053ef3`.
The existing reference-binding body moves unchanged into ReferenceConversion
in the existing semantic call module, leaving ordinary Conversion smaller.
The conversion counter remains in the caller, and the helper consumes the
already identified referent and reference category. Its ownership ledger entry
is explicit. All 574 fresh candidate observations retain their boundaries;
376 unchanged host observations are reused in thirteenth-matrix-review.json.
All eleven required groups in validation-thirteenth/ pass, including strict
6101/6101, debug-info, variants, self-host through PA5 and architecture/file/
placement audits. The immutable Alpha study verifies all 91 frozen inputs and
is running the unchanged complete broad/focused measurement sequence. The four reference
owners and all subsequent additions remain pending until all gates pass.

The two independent namespace rejection fixtures are now installed without a
compiler change: PA6 300-distinct-using-directive-types-bad.t rejects genuinely
different designated types, and PA7 300-distinct-using-directive-values-bad.t
restores the exact original distinct-variable source. Both refs were generated
through exact ref-test selections; both owning check selections pass. The early
consolidation inventory now points to the PA7 fixture and explains why the valid
same-type typedef case cannot replace its expression-lookup goal. Historical
input hashes remain untouched. The namespace positive and its existing wrong
negative oracle remain pending the namespace compiler fix.

The test-only restoration passes the strict one-line 6103/6103 report. A focused
PA6/PA7 placement run scans 297 inputs with no placement or hygiene findings;
the standard placement run also passes. These two already supported rejection
boundaries do not depend on the pending reference-conversion implementation or
its performance decision.

## PA32 student refresh and fixed-layout timing review

Read-only snapshot student-refresh-pa32-410c67bf/ pins student HEAD
`410c67bfc9098d726537f98d615d0479a5fe6e27`. Since the last pinned PA30 snapshot,
the student changed one PA30 exit-status reference and four PA31 inspection
sidecars. Course sources, Makefiles and fixture discovery remain unchanged.
The two global relocation expectations use g rather than _Z1g and repeat the
existing Clang-verified ABI-GLOBAL issue; they are not a new mangling family.
The replacement-new status change has its separate proof and held host
disagreement in NEW-SPEC-REF. No Itanium encoder changes were made.

Seventy-three copied personal source controls have 438 fresh strict-C++11
candidate/Clang/GCC compilation observations at O0/O2. Another 146 frozen
entry compilations preserve every candidate status; these are preexisting
boundaries. Six reductions supply 60 current observations, including semantic
mode checks and the header-free PA21 noexcept-list failure. controls.json,
baseline-controls.json and reducer-controls.json retain commands and diagnostics;
manifest.json pins the read-only source snapshot. Each group above separates
verified C++11 failures from disputed results and pending contract/DR reviews.
No exploratory matrix is installed wholesale as required fixtures.

The student's PA32 call-nested-cleanup.cpp is explicitly recorded as a failing
personal frontend reducer there. Here it compiles and runs successfully in ten
checks: both optimization levels with all three host-linked object routes and
both immutable entry/candidate standalone routes. nested-runtime-controls.json
records those outcomes. No additional required fixture is justified by that
student observation. The inherited-defaults personal control is expressly
retained against an exploratory host disagreement; all three compilers here
reject its nothrow assertion. This is a held oracle review, not a passing host
consensus test. The older bundle's replacement-new acceptance is also expressly
recorded in the student's reference-correction201.md.

The complete original thirteenth Alpha study retains all 4800 equal-object
observations in perf-thirteenth-results/. Instruction/RSS gates pass; focused
raw/calibrated cycle ratios fail for EH handlers (1.005620/1.011590),
qualification views (1.004618/1.012707) and virtual dispatch
(1.004340/1.007381). The original decision is not discarded.

A predeclared baseline-only aslr-calibration/ diagnostic retains 512
observations, identical compiler/input/output arguments and equal objects.
Process-only setarch -R reduces paired A/A median absolute deviation from
0.006015 to 0.002351 for EH and from 0.005378 to 0.002015 for qualification.
Fixed-layout A/A medians are 0.999213 and 1.000560. Both predeclared criteria
(lower variation and medians within 0.25% of unity) pass. Every counter/time/log
is retained locally; objects remain remote. No global setting is modified.

One complete perf-thirteenth-fixed-layout/ study measures the same immutable
candidate against the same entry with that process setting. Samples, workloads,
A/A identity, output equality and all thresholds remain unchanged. Its manifest
records only the two planned runner-hash changes; all compiler/source hashes
remain identical. Initial preflight rejected the stale runner hashes before any
measurement; that failed preflight log is retained. Corrected manifest
verification passes all 91 entries before the measured run. Compiler code and
thirty pending fixture sets remain uncommitted until the gate decision.

The complete fixed-layout thirteenth study also retains all 4800 equal-object
observations in perf-thirteenth-fixed-layout-results/. Instruction/RSS gates
pass; three focused cycle gates fail: EH raw/calibrated 1.016530/1.019994,
initializer-list values 1.006478/1.007762 and qualification views
1.006715/1.012903. Disabling ASLR alone does not clear the performance concern.
A separate predeclared 512-observation fixed-layout-argv-calibration/ diagnostic
compares unique output arguments with identical compiler arguments. Neither
workload meets its required 0.25% median separation; its criterion fails. All
observations are retained locally and objects remote. No further output-argument
protocol change is made, and none of the failed candidate gates is excused.

The fourteenth image is compiler-fourteenth, SHA-256
`3164e691811917a83d2e74f9e3166563a7650ee0af06d33c547db6fa258ff7c5`.
It removes thirteen nonessential record-copy changes from the patch, restores
QualificationConversion to its original analysis owner, and appends the existing
reference-binding helper after the existing call functions. Reference-binding
and qualification policy are byte-identical to the thirteenth bodies; the
ownership ledger follows the retained definitions. The file sizes are 2932 and
2567 lines, within the unchanged limits. A fresh build passes. All 574 fresh candidate observations preserve their
previous boundaries; 376 unchanged host observations are explicitly reused in
fourteenth-matrix-review.json. All eleven required validation groups pass,
including the one-line strict 6103/6103 report, debug-info, variants, self-host
through PA5, nine architecture checks, file limits and zero placement/hygiene
findings. One immutable complete Alpha study
uses the established fixed-layout protocol, unchanged thresholds and all 91
verified manifest entries. No additional compiler/source or output-argument
protocol changes are made during its measurements. The four reference rows,
thirty fixture sets and later compiler fixes remain pending.

The fourteenth study is complete: all 91 manifest entries verify and all 4800
objects agree. Broad instruction/RSS and nine focused cycle gates pass. Only
EH handlers fail, at raw/calibrated cycles 1.010908/1.013188.
perf-fourteenth/gate-review.json and perf-fourteenth-results/ retain the decision
and every observation. Inspection of the frozen EH input shows no reference
initialization: it consists of unused class declarations and ordinary int
returns through try/catch. Its timing does not measure the corrected reference
binding rules directly; no unrelated compiler owner is changed on that basis.

The next patch removes the remaining optional ordinary-conversion optimizations:
target-record copying, normal decay/exact matching and pointer-record copies
return to the original implementation. The required array-element cv check for
pointer-to-void conversion is retained, as are the unchanged reference-binding
and qualification bodies. An initial fifteenth image
`84cb306478bb57c48417a2bc169e7258cddef7942f9221daf5539056f0cc2677`
exposed an accidental duplicate pointee unwrapping and unused variable during
build review. Its partial validation and frozen image are retained; it was
never timed. The warning cleanup reuses the already computed pointee.

The measured image is compiler-fifteenth-clean, SHA-256
`9c4a63202b5f742a556b757119d022078f07a245658b9d0e8b73044b40aa7616`.
Its build has no compiler-source warnings. All 574 fresh candidate observations
retain their boundaries, with 376 unchanged host observations reused in
fifteenth-clean-matrix-review.json. All eleven required groups in
validation-fifteenth-clean/ pass, including the one-line strict 6103/6103 report,
debug-info, variants, self-host through PA5, nine architecture audits, file
limits and zero placement/hygiene findings. Live and frozen compiler hashes
match before the next checkpoint; the Alpha performance decision is pending.
The fresh immutable Alpha study inherits the fourteenth fixed-layout runners,
thresholds and sample counts. All 91 manifest entries verify before counting.
Initial preflight logs are retained: the untimed fifteenth root had no source
inputs, so preparation now copies the identical verified inputs from the
complete fourteenth root. No preflight failure produced timing observations.
Compiler code and thirty fixture sets remain uncommitted until all gates pass.


The fifteenth-clean study is complete. All 91 manifest entries verify and all
4800 objects agree. Instruction/RSS gates pass. Four focused raw/calibrated
cycle gates fail: auto aliases 1.006692/1.007228, EH handlers
1.007596/1.008375, initializer-list values 1.004718/1.006834 and
qualification views 1.009737/1.015294. The decision and every observation are
retained in perf-fifteenth-clean/ and perf-fifteenth-clean-results/.

A subsequent 256-compilation EH sampling profile compares the same immutable
entry and fifteenth-clean images, with successful equal objects. Conversion,
ReferenceConversion and QualificationConversion are below the 0.1% reporting
cutoff; TypeTable::Get accounts for 0.32%/0.29% of entry/candidate samples.
This does not justify changing unrelated lexer or native owners. Assembly
inspection does reveal a concrete conversion-path change: GCC splits ordinary
Conversion into a wrapper and .part.0, with an additional prologue on ordinary
value conversions. The entry keeps this path in one function. Both assembly
listings and profiles are retained in reference-cv-qualification/.

The sixteenth candidate removes the reference-helper extraction, restoring the
reference-binding branch inside Conversion. QualificationConversion moves to
the existing calls module to keep both source files within their unchanged
limits (2950/2541 lines); the owner ledger follows its definition. Binding and
qualification policy are unchanged. The fresh warning-free image is
compiler-sixteenth, SHA-256
`77c106abf5a0f559ab9403ee8cb7bee08767d5db64bc81060f4aca7bad036f99`.
Its symbol table confirms that Conversion has no .part.0 clone. Fresh matrix,
full compiler validation and one immutable Alpha study are pending; thresholds,
samples, inputs and process-only fixed-layout protocol remain unchanged.
Compiler changes and the thirty fixture sets remain uncommitted until the
required gates pass. Later additions, final fixture review and export remain
pending.


All eleven sixteenth validation groups pass, including strict 6103/6103 in
exactly one output line, debug-info, backend variants, self-host through PA5,
nine architecture audits, file limits and zero placement/hygiene findings.
sixteenth-matrix-review.json verifies all 574 fresh observations against
fifteenth-clean, reusing 376 unchanged strict C++11 host observations. Only
arbitrary runtime values from already accepted invalid bit-field negatives
vary; those remain explicitly excluded from the oracle. The live compiler
matches the immutable sixteenth image. Alpha remains live in its focused
phase; the compiler/fixture checkpoint awaits that decision.


Namespace preparation is rechecked with the immutable sixteenth image:
after-reference-controls.json retains 116 fresh observations over the eighteen
core controls, six mixed-name controls, two scope-qualifier controls and three
planned fixtures, in both dumps and at O0/O2. The same seven core convergence
positives and planned positive fail; the same four mixed-name negatives are
incorrectly accepted. All other boundaries remain unchanged. The reference
binding patch therefore does not resolve or introduce these lookup defects.


The sixteenth Alpha study is complete: all 91 inputs verify and all 4800
objects agree. Instruction/RSS and nine focused raw/calibrated cycle gates
pass. EH handlers alone fail at 1.008330/1.011434. Every raw measurement is
retained in perf-sixteenth-results/; perf-sixteenth/gate-review.json retains
the complete gate decision. Removing GCC's split ordinary path was useful
but insufficient to clear that remaining threshold; no row is closed.

The next candidate short-circuits an identical fundamental source/target type
before record copying, decay and qualification. It obtains the target fact
once and preserves the existing full-record snapshot for all other paths.
The ordinary EH source returns int to int, so this change removes actual
redundant type-table reads on the measured path rather than changing an
unrelated compiler owner. References, arrays, functions, class values and
qualified types retain their previous conversion paths. The preliminary build
log is retained; no preliminary image was timed. Fresh validation and an
immutable full study remain required before the compiler/fixture checkpoint.


The seventeenth warning-free image is compiler-seventeenth, SHA-256
`158bc4d281f0548dfbab381fc24a1e55ed65bb7d5e6cb200f7be558d6f32d4ca`.
Conversion remains one function, without a .part.0 clone. Source sizes are
2954/2541 lines, within unchanged limits. Its matrix and all eleven required
validation groups are running. The fresh Alpha study inherits the exact
sixteenth runners, sample counts, inputs, fixed-layout setting and thresholds;
only the candidate image changes. No measured image or source will change
during counting, and the prior sixteen-image evidence remains retained.


## Inherited return-type matching review

TMPL-MEMBER-MATCH is reviewed against PA14's supported instantiated member
machinery and definition-time checks, PA17's dependent-return-type scope rule,
and N3485 13.1/1–2, 14.5.1.1/1–2 and 14.6/8. The original student negative has
no use of Service<T>::get. Its out-of-class return spelling precedes the
qualified declarator and denotes a retained qualified template type. Modern
hosts diagnose the mismatch eagerly, while our compiler waits for demand.
The C++11 template rule permits no diagnostic for an invalid uninstantiated
template; PA14's explicit unused-body checks do not impose that eager
out-of-class dependent-return matching rule.

A copied reducer adds a call to Service<int>::get without changing the
mismatched declaration or definition. Our immutable seventeenth image rejects
it at O0/O2 and in PA14 O0 LowIR with conflicting function return type. Both
strict C++11 hosts reject it. The original matching-definition positive passes
all seven corresponding compiler/mode checks. All fourteen commands, sources,
statuses and diagnostics are retained in
/tmp/cppgm-v4-audit-review/template-member-match-review/controls.json.
The original unused observations remain preserved in student-refresh-pa30-200/.
No compiler rule, reference or required fixture changes merely to enforce the
hosts' earlier diagnostic timing. This resolves the review item; the distinct
namespace, template access/substitution and demand bugs remain in the queue.


## Replacement-new reference review

NEW-SPEC-REF is reviewed against N3485 3.7.4/2, 15.4/3–4 and 18.6.1.1,
and the actual selected header declarations. LLVM tag llvmorg-21.1.8
[Clang exception matching](https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-21.1.8/clang/lib/Sema/SemaExceptionSpec.cpp)
lines 565–587 deliberately accepts the std::bad_alloc difference for backward
compatibility. That host acceptance is not evidence that C++11 generally
permits restrictive redeclarations of an unrestricted allocation declaration.
The same release's
[libc++ global allocation declarations](https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-21.1.8/libcxx/include/__new/global_new_delete.h)
lines 22–27 enable the dynamic throw macro only in C++03. The selected
libstdc++ 15 C++11 macro is likewise empty, as recorded by the student.
Pinned primary files and hashes are retained in
replacement-new-reference-review/primary-source-manifest.json.
N3485's Annex C older-code example retains the old specification, but its
normative implicit/library declarations and compatibility clauses remain the
basis for portable required coverage.

The proposed replacement-new positive removes the dynamic specification and
preserves hosted object emission. controls.json records eighteen fresh
observations: the new positive passes ours/Clang/GCC at O0/O2; the original
hosted legacy positive also passes all three here, unlike the student's
header-free reducer with an explicit unrestricted redeclaration. The existing
wrong-spec negative rejects ours/Clang but GCC accepts it as a concession.
The strict language argument, not that permissive host result, justifies
retaining the negative. Initial commands accidentally passed unsupported -x
to our driver; all eighteen initial observations and the script are retained
separately as invocation errors, then rerun with copied .cpp sources and valid
commands. No invocation error is counted as semantic evidence.

fixture-plan.json proposes renaming/rewriting the existing positive as
pa30/tests/compile/700-hosted-replaceable-operator-new.t, retaining its owning
hosted-header milestone and cluster. This keeps one positive and one
independent negative without adding another required input. The wrong-spec
comment should describe the incompatible restriction rather than claim the
C++11 header has a dynamic specification. No default fixture or reference is
changed before the active reference-conversion checkpoint; exact regeneration,
owning/strict/placement checks and the later final export remain pending.


All eleven seventeenth validation groups pass, including the one-line strict
6103/6103 report, debug-info, variants, self-host through PA5, all nine
architecture audits, file limits and zero placement/hygiene findings. The
574 fresh matrix observations retain their boundaries, with 376 unchanged
strict C++11 host observations reused; invalid bit-field runtime variations
remain explicitly outside the oracle. seventeenth-matrix-review.json records
the complete comparison. The live compiler matches its frozen image. The
Alpha focused study remains live, with no gate decision or repeated trial yet.


EH-SPEC-SET preparation now has one compact order/duplicate positive and one
independent equal-cardinality/different-types negative. Sixteen fresh commands
in exception-spec-set/entry-controls.json confirm that both strict C++11 hosts
accept the positive and reject the negative; our immutable seventeenth image
rejects both in type/semantic dumps and at O0/O2. No default fixture is installed
yet. An initial expanded cv/array/function adjustment control has a Clang/GCC
compatibility disagreement despite the N3485 adjustment wording; its source and
all sixteen original observations are retained as exploratory evidence, not
added to the required positive. That neighboring implementation behavior is
not presumed to be an additional compiler bug or fixture obligation.


The seventeenth study is complete and retained in perf-seventeenth-results/.
All 91 inputs verify, all 4800 objects agree, and instruction/RSS gates pass.
Nine focused raw/calibrated cycle gates fail: auto aliases 1.019783/1.020064,
auto prvalues 1.008613/1.008435, EH handlers 1.007588/1.009007,
initializer-list values 1.008996/1.009668, qualification views
1.018251/1.018377, recognition 1.006036/1.007144, reference aliases
1.014106/1.012620, virtual dispatch 1.010230/1.011604 and volatile views
1.011706/1.010774. Reference noexcept temporaries alone pass both cycle gates.
The fundamental identity shortcut is removed; the source restoration is
verified byte-for-byte against sixteenth-source.patch before the next change.
No failed timing observation or threshold is discarded.

The next candidate removes the remaining nonessential cached record reads in
the reference branch itself. It restores the original EffectiveType,
RemoveTopCv and full-record read locations in exact-type and derived-to-base
binding, keeping only the required array/cv, volatile, qualification and value-
category corrections. The ordinary value path remains the entry implementation;
QualificationConversion remains in the existing calls module. This narrows the
source change to required policy rather than retaining an unsuccessful generic
optimization. The eighteen-image build, fresh matrices, complete validation and
immutable performance study are pending. No other compiler fix or required
fixture is installed before this checkpoint passes its gates.


The eighteenth warning-free image is compiler-eighteenth, SHA-256
`26bad327a8becf777647c542070bfce6b63e377755d48c0036f109868e2bebea`.
Conversion remains one function without a split clone. Its stack reservation
matches the entry's 0x168 bytes; the whole function size is 0xea7 versus the
entry's 0xeb6. These assembly facts explain the narrowed implementation and
are not a performance gate. The source sizes are 2947/2541 lines, within the
unchanged limits. Fresh matrix and all eleven checks are underway. One complete
immutable Alpha study inherits the seventeenth runners, inputs, thresholds and
process-only fixed-layout setting. All 91 manifest entries verify before any
counting. The four reference rows and thirty fixture sets remain uncommitted.


The eighteenth matrix review verifies all 574 fresh observations against the
seventeenth image and reuses the 376 unchanged strict C++11 host observations.
All intended boundaries remain; only arbitrary runtime values in the separately
tracked invalid bit-field negatives vary. All thirty frozen fixture sets retain
their exact source/reference hashes. Local complete validation and the Alpha
focused study remain live, so no compiler/fixture checkpoint or row closure is
claimed yet. The template matching review is resolved, and namespace,
replacement-new and exception-set additions have concrete prepared inputs for
the subsequent sequential checkpoints.


## Array-new destructor access review

NEW-ARRAY-DTOR-ACCESS is directly within C++11: doc/n3485.txt 5.3.4/17
explicitly requires access and ambiguity checking for the destructor when a
new-expression creates an array of class objects. PA12's supported array
new/delete surface and its 400 cluster own this case. The clause already
exists in the pinned draft; it does not require importing CWG 1424's later
potential-invocation wording or its broader constructor-subobject rules.
The current array allocation semantic owner checks deletion and demands a
nontrivial destructor, but does not check CanAccessMember as delete does.
The required repair belongs to that existing typed allocation owner.

new-array-destructor-access/entry-controls.json retains 49 fresh observations
for seven reducers, in PA12 O0 LowIR and native O0/O2 for ours and strict C++11
native O0/O2 for each host. The private two-element array negative is accepted
here and rejected by both hosts; private scalar allocation and array allocation
from the owning class pass all three. Public arrays and scalar allocation with
a user-provided constructor/deleted destructor also pass. Deleted-array
rejection already works here. GCC accepts the zero-sized private array and the
explicit-constructor deleted array while Clang rejects them. The initial broad
host-agreement assertion therefore fails; every observation is retained and
those disagreements are not advertised as consensus or new required fixtures.

fixture-plan.json prepares just two PA12/400 inputs: one compact positive
combines private scalar allocation outside the class with private array
allocation inside it, and the independent original private-array negative.
planned-entry-controls.json retains fourteen fresh observations confirming
those boundaries. No extra permutation or already working deleted-array
negative is installed. Aggregate-member and inactive-union disputes remain
separate from the explicit array-new access requirement. Installation and
exact generated references remain after the active conversion checkpoint.


All eleven eighteenth validation groups pass, including strict 6103/6103 in
exactly one line, debug-info, variants, self-host through PA5, all nine
architecture audits, file limits and zero placement/hygiene findings. The
live compiler matches its immutable image; all thirty fixture hash sets remain
unchanged. Alpha is still in the original focused phase. One status-file read
raced the runner's write and returned partial JSON; that observation is not a
measurement failure or terminal state, and the same live process is rechecked.
No measured study is restarted because of a polling error.


The eighteenth performance study completed without restarting. All 91 inputs
verify and all 4800 output/status observations agree. Instruction and RSS gates
pass, but eight focused cycle gates fail: auto aliases, auto prvalues, EH
handlers, initializer-list values, qualification views, recog, reference aliases
and volatile views. Both raw and calibrated ratios are retained in
reference-cv-qualification/perf-eighteenth/gate-review.json; complete raw results
are also retained locally in perf-eighteenth-results. This is not a passing
compiler checkpoint.

The nineteenth candidate returns QualificationConversion to its original
analysis owner now that the narrowed reference implementation fits that file.
Only parallel declarations and the new helper's own comment are compacted;
there is no unrelated padding or audit-limit change. analyzer.cpp is 2999
lines, and calls.cpp and the semantic ownership ledger exactly match the entry.
The immutable compiler-nineteenth image has SHA-256
`fe82f4ca3d471b6fa0ec8f5e6a0d4ab9101a585b7795cfc85b21b105a45a70cf`.
All 574 fresh matrix observations preserve the eighteenth boundaries, with
376 unchanged strict C++11 host observations reused. Full validation and one
unchanged-protocol Alpha study are pending. No performance success is claimed.


## Portable replacement-new fixture checkpoint

The prepared unrestricted positive replaces, rather than supplements, the old
700-hosted-replaceable-operator-new-dynamic-exception-spec input. Its new name
is pa30/tests/compile/700-hosted-replaceable-operator-new.t. The wrong-spec
negative keeps its source behavior and corrects its comment to describe the
unrestricted C++11 declaration. Exact make -C pa30 ref-test generates only the
new positive's sidecars; the superseded fixture and its references are removed.
PA30 passes 153/153; placement and local hygiene findings remain zero. The
unchanged compiler-after-auto entry independently compiles the new positive.
This test-only correction proceeds independently of the immutable reference
conversion performance study. The new combined strict report passes 6103/6103 in exactly one line;
student export remains deferred to the requested final check.


All eleven nineteenth compiler validation groups pass, including the unchanged
strict 6103/6103 report, debug-info, variants, PA34 self-host through PA5, nine
architecture audits, file limits and placement/hygiene. The subsequently
rewritten PA30 fixture passes its own combined strict report with the same
count. The live compiler still matches compiler-nineteenth exactly. The Alpha
study remains live; no reference-binding compiler checkpoint is claimed yet.


## Completed PA32 student review refresh

The read-only checkout now reports HEAD
441d5ec9bc49b631675c3dc5632b20fe93d11d5f, with ongoing PA33 source work.
Since the previous 410c67bf snapshot, the only course fixture/reference changes
are the existing PA32 backward even-stride loop's .ref and .ref.expect. No
course input, exit status or comparison implementation changed. Seven review
files and their SHA-256 hashes are frozen in
student-refresh-pa32-441d5ec9/source-manifest.json, along with the exact delta.
The ongoing student worktree was neither built nor modified.

Its correction is substantive: deleting a pointer loop with stride minus eight
requires proof that the starting and ending addresses have the same residue.
The independent pointer arguments supply no such fact. PA8 lowir.md's Memory
and Addressing clause defines plain index without a stronger optimization
claim; PA32's Output Format requires preserving every defined input program.
A C++ source forward-progress rule cannot add a promise to this direct LowIR
fixture. The reference should retain the potentially nonterminating branch.

current-controls.json retains eighty fresh observations: sixteen optimizer
invocations, thirty-two lowerings and thirty-two executions, using equal,
finite same-residue and unequal-residue pointer inputs at O0 through O3 with
both cppgm++ and lowir2native. Every build succeeds. All finite executions
return zero. O0's unequal-residue executions time out; both native paths at
O1/O2/O3 incorrectly return zero instead. A timeout alone is not a proof;
the invariant end - 8*k modulo eight supplies the missing termination
boundary. LOOP-PTR-FINITE enters the same tracker. Reuse the existing required
fixture rather than adding a duplicate; exact generated references follow the
compiler repair.

The completed audit also discloses five inherited optional PA8 debug-shape
mismatches, present with its frozen entry as well as its final tools, and a
removed personal debug-only must-unroll/fill assertion. Those statements are
recorded as diagnostic/contract review leads, not automatic bug claims. Its
required PA32 debug checks pass. No additional course reference changed in
audit218. Further PA33 claims await completed evidence.


The nineteenth performance study is terminal: all 91 inputs verify and all
4800 object/status observations agree. Instruction/RSS gates and six focused
raw/calibrated cycle gates pass, including EH handlers. Four fail: auto aliases
1.009021/1.010096, initializer-list values 1.007198/1.007359, qualification
views 1.004464/1.005550 and reference aliases 1.006833/1.005922. Full raw
results are retained locally in perf-nineteenth-results, and the exact review
is perf-nineteenth/gate-review.json. No compiler checkpoint is committed.

The next targeted change replaces the existing ArrayElementCv recursion with
an iterative accumulation of the same qualified/array records. The corrected
reference-binding policy invokes this owner on the affected paths. Nineteenth
assembly confirms that the recursive implementation saves five registers,
expands repeated qualified-type cases and still makes a recursive call; the
iterative query can avoid that work without changing type policy or adding a
cache. It stays in its original calls.cpp owner. The original pointer-to-void
cv comment is restored, shortening only the new reference-helper comment;
file limits remain unchanged. Twentieth build, matrices, full validation and
one unchanged-protocol performance study are pending.


The twentieth immutable image is compiler-twentieth, SHA-256
`183c573123ee8160872f9271300821c9838ad042484537a677bd71971b623803`.
The iterative ArrayElementCv machine body is 0x49 bytes versus 0xfd in the
nineteenth image. This source-specific assembly finding is not a timing gate.
analyzer.cpp is exactly 3000 lines within the unchanged limit; the original
pointer-to-void comment is restored verbatim. No ownership ledger change is
needed. Local full validation and nine fresh matrix groups are underway.
The same Alpha protocol and all frozen inputs are prepared for one study;
no prior observation is discarded and no threshold is changed.


The twentieth matrix review preserves all 574 fresh semantic/runtime boundaries
and reuses the unchanged 376 strict C++11 host observations. Two arbitrary
bit-field-negative runtime values change between 1 and 253; those programs
remain invalid and belong to REF-BITFIELD, so their values are not oracles.
Full validation and the original twentieth performance study remain live.

Eighteen supplementary array-new observations check a private defaulted
(trivial) destructor. Clang rejects external array allocation, GCC and ours
accept it, and all three accept scalar allocation and allocation inside the
owning class. The access clause still requires checking before the trivial-
destructor lowering shortcut, but host disagreement is retained explicitly in
new-array-destructor-access/defaulted-destructor-entry-controls.json. No extra
required fixture is added for this permutation; the prepared two-fixture plan
remains unchanged.


## Noreturn argument constraint and remaining fixture inventory

ATTR-NORETURN's argument case is directly C++11 under doc/n3485.txt
7.6.3/1: no attribute-argument-clause is permitted. The existing support.attribute
ledger assigns standard-attribute support to PA29/500. The token-owned parser
ConsumeLeadingStandardObjectAttribute currently records a noreturn identifier
without validating its arguments. The repair belongs to that existing parser,
without reparsing rendered text. noreturn-attribute-contract/argument-controls.json
retains six O0/O2 native-compile observations for ours and strict C++11 hosts.
One negative fixture is prepared. Existing PA29 noreturn-call-before-break
runtime coverage supplies the positive boundary; no new positive permutation
is proposed. Variable appertainment remains a distinct diagnostic review and
is not silently claimed as a host-consensus rejection.

The final-review scratch input-inventory.json records 297 input changes since
fb15cd49e: 254 committed additions, nine modifications, two removals, two
renames and thirty pending reference fixtures. These are changes, not 297
independent bugs. This is an inventory only; required retain/combine/remove
review remains after the complete addition sequence, followed by final export.


All eleven twentieth validation groups pass: PA10/12/20/21, strict 6103/6103
in exactly one line, debug-info, variants, PA34 self-host through PA5, all nine
architecture audits, file limits and zero placement/hygiene findings. All
thirty fixture hash sets are unchanged, and the live compiler matches its
immutable image. The original Alpha study remains live; the four reference
rows remain uncommitted pending its terminal performance decision.


The twentieth Alpha study is terminal, with all 91 verified inputs and 4800
equal-object/status observations. Instruction and memory gates pass; eight
focused cycle gates fail: auto aliases 1.016909/1.015098, auto prvalues
1.009474/1.008950, EH handlers 1.017166/1.015666, initializer-list values
1.015408/1.013814, qualification views 1.017626/1.018837, reference aliases
1.018592/1.020936, reference noexcept temporaries 1.012454/1.012400 and
volatile views 1.011016/1.010250. Recog and virtual pass. All raw results are
retained in perf-twentieth-results, with exact gate-review.json. The smaller
iterative machine body does not establish a performance success.

The next targeted investigation records array element cv as an immutable
derived type fact at canonical insertion, then consumes it with one record
read. This directly supports the required reference-binding check and the
architecture's typed-fact contract. Type identity, hashing, existing cv fields,
trait behavior and published output remain unchanged. The existing model owner
constructs ordinary, dependent and zero-sized arrays through Intern; repeated
intern hits need no extra derivation. Qualified types are already normalized
so they cannot directly wrap arrays or another qualified record. A new derived
byte must fit existing padding without enlarging TypeRecord or changing prior
field offsets. No candidate is frozen or timed before those facts are verified.


The proposed derived array_element_cv byte occupies prior TypeRecord padding.
The host layout probe verifies size 56 and all previous field offsets unchanged:
cv 40, ref_qualifier 41, variadic 42, bitint_unsigned 43, zero_length_array 44
and fundamental 48. Its first invocation omitted the required dev/src include
path and failed to compile; the corrected before/after invocations pass and
their outputs agree exactly. This is a layout check, not a language oracle.
Intern derives the array fact only after an identity miss and before publishing
the new record, covering ordinary/dependent/zero-sized arrays through their
existing common owner. The existing hash, equality and cv field are untouched.
ArrayElementCv consumes one record and returns either normalized qualifier cv
or the derived array fact. Other type kinds keep the default zero fact. The
scratch freezer now includes both model source paths so no required source
change is omitted from the frozen patch. The twenty-first build is pending.


The twenty-first build succeeds without compiler warnings. The frozen image is
compiler-twenty-first, SHA-256
`50eff87d24b5747dd74759eef5cc204928af7bea0ffee23f9260495805f34a02`.
The direct-fact ArrayElementCv machine body is 0x31 bytes. All fresh matrices
and full compiler checks are launched; Alpha inputs are uploaded but counting
waits for the fresh semantic boundary review. Earlier immutable images and
failed timing studies remain retained. The compiler/30 fixtures remain
uncommitted, and the goal remains active.


The twenty-first fresh matrix review preserves all 574 observations, reusing
376 unchanged strict host observations. Only arbitrary bit-field-negative
runtime values differ; those invalid programs remain separately tracked.
All 91 Alpha manifest entries verify before one unchanged-protocol study is
started. The explicit atomic commit manifest now has 111 paths: five compiler
source/header paths, the tracker and 105 required fixture/sidecar paths. No
unrelated untracked artifacts are included. Complete local validation and the
performance study remain live, so no row closure is claimed.


All eleven twenty-first local validation groups pass, including strict
6103/6103 in exactly one line, debug-info, variants, PA34 self-host through
PA5, all nine architecture audits, file limits and zero placement/hygiene
findings. All thirty fixture sets keep their frozen hashes, and the live
compiler exactly matches compiler-twenty-first. The original Alpha study
remains live, with its terminal performance review still pending.


The twenty-first study is terminal. All 91 inputs and 4800 object/status
observations agree; instruction/RSS gates pass. Five focused cycle gates fail:
EH handlers 1.012033/1.012076, initializer-list values 1.007987/1.009039,
recog 1.005282/1.004288, reference noexcept temporaries 1.003515/1.006571
and virtual 1.006300/1.006138. Auto aliases, auto prvalues, qualification views,
reference aliases and volatile views pass both cycle gates. Complete results
are retained in perf-twenty-first-results with the exact gate-review.json.

Constructor inspection rejects one possible explanation: TypeRecord's machine
constructor has the same 0x3c-byte instruction sequence as the nineteenth image;
the padding byte adds no initialization store. No tagged-storage redesign is
justified by that evidence. There is, however, an avoidable array-element Get
in generic Intern, after every array builder already reads that same element.
The next targeted change computes the derived fact in the three existing array
builders using their already-read record, restoring generic Intern completely
to the entry code. This eliminates the second element query and the added kind
branch from every canonical insertion. It preserves the new field, existing
identity and original allocation footprint. The zero-sized array builder's
numeric_types.cpp path must also be frozen with the next candidate.


The twenty-second cache-builder alternative builds warning-free and is frozen
as compiler-twenty-second, SHA-256
`2de6268bf59603da1970008232b285640419afaa18a3db89296a84f74b51b4f5`.
It is retained with its full seven-path scratch patch and is not timed. Further
source review shows that no derived cache is needed: TryQualify distributes cv
into array elements and merges nested qualifiers, and is the sole constructor
of TYPE_QUALIFIED. A qualified record therefore cannot wrap an array or another
qualified record. ArrayElementCv can walk array dimensions and return a
qualified record's own cv immediately, avoiding its extra child query.

The twenty-third preparation restores program.cpp, program.h and
numeric_types.cpp byte-for-byte to 1de7a8cbc, with assertions before writing.
Only the necessary reference policy, qualification walker and the normalized
cv query remain changed. No derived field, layout change, generic-intern branch
or array-construction work remains. This is a source invariant based reduction,
not an assertion that the untimed twenty-second image failed performance.
The next matrices must verify all prior cv, atomic and reference boundaries.


The twenty-third normalized-query image is frozen as compiler-twenty-third,
SHA-256 `c32713ca936d130b3837d4ceb86e96fd9b8a11fb585e6375d69cdedc5d3b57a4`.
Its ArrayElementCv body is 0x36 bytes, versus the original 0xfd, without any
new type record field or constructor work. Five required source/header paths
are reduced back to three; the explicit atomic manifest is 109 paths including
the tracker and 105 fixture/sidecar paths. Fresh matrices and full validation
are underway. Its performance setup inherits the fully populated twenty-first
study rather than the untimed twenty-second scratch alternative. No compiler
or reference-binding row is claimed complete yet.


The twenty-third fresh review preserves all 574 observations, reusing the
376 unchanged strict C++11 host observations. Only the separately tracked
invalid bit-field-negative garbage value changes. All 91 immutable inputs
verify before one unchanged Alpha study starts. Full local validation and
performance review are pending; no further compiler fix is installed. Final
fixture minimization and combined export remain after the complete additions.


## Inherited zero-argument contract review

N4429 explicitly states CWG's intent to apply the inheritance rewording as a
C++11 defect resolution. P0136R1 supplies the adopted wording; CWG1941 records
October 2015 adoption. Primary sources are frozen with exact URLs and SHA-256
hashes in inherited-zero-contract/primary-source-manifest.json. This establishes
a C++11 DR basis rather than inferring language ownership from host acceptance.
N3485's earlier exclusion of parameterless inherited constructors remains
recorded. The course's class.inheriting_constructor owner is PA11/500.

The pure ordinary zero-argument runtime reducer passes ours and both strict
C++11 hosts at O0/O2: move-declared derived construction, members following the
inherited base construction, local constructor hiding and parameterized
construction all retain their expected values. Three independent private,
deleted and deleted-member negatives reject in all eighteen observations.
The original composite's dependent using T::T and parameterized validity failures
therefore do not establish a zero-availability bug here. Existing inheriting-
constructor fixtures and the independent open rows remain; no new zero-arity
fixture is added merely to repeat a working boundary.

The isolated throwing-default runtime is different: ours catches int 17, while
both hosts abort at O0/O2. A separate trait observation is accepted by all three
and reports zero construction nothrow; the student's opposite assertion is not
an agreed reference oracle. All thirty observations, including every diagnostic
and signal, are retained in isolated-zero-controls.json, alongside the six
ordinary-runtime observations. The existing exception review is preserved in
INHERITED-DEFAULT-EXCEPT rather than silently closing it with availability.
No required input/reference or compiler source is changed for this review.


## Test-addition checkpoint: reference qualification

The user requested finishing test additions separately from the remaining fixes.
Thirty PA10/12 qualification fixtures and their 75 generated reference sidecars
are ready as a test-only checkpoint. All 105 files still match the frozen
fixture-sidecar manifest; the references were generated through exact ref-test
selections, not edited by hand. The four unrelated untracked reference artifacts
remain outside this checkpoint. Final overlap review stays deferred until the
remaining additions are complete.

The twenty-third compiler image passes all eleven local validation groups:
PA10/12/20/21, strict 6103/6103 with one success line, debug information, backend
variants, self-host through PA5, architecture, file and placement audits. Its
574 fresh and 376 reused strict-host observations preserve the reviewed
boundaries. This is correctness validation, not performance approval.

Its Alpha study is terminal: all 91 frozen inputs and all 4800 output/status
observations agree, and instruction/memory gates pass. Five focused cycle
gates fail (auto aliases, EH handlers, initializer-list values, qualification
views and reference aliases); exact results remain in
reference-cv-qualification/perf-twenty-third/gate-review.json and the complete
raw study in perf-twenty-third-results/. The three compiler source changes
remain uncommitted and the corresponding compiler-fix rows remain open.
No further performance variant is part of this test-addition checkpoint.


## Completed tests-first additions

The user narrowed the immediate work to completing test additions. No further
compiler variant, ABI encoder edit, performance study or export regeneration
was made in this checkpoint. The live compiler remains the frozen twenty-third
image, SHA-256 c32713ca936d130b3837d4ceb86e96fd9b8a11fb585e6375d69cdedc5d3b57a4;
its three existing source changes remain uncommitted because its performance
gate has not passed. Test completion does not close the corresponding fixes.

Two already supported boundaries are added to the default suite through exact
ref-test selections: PA12/400 permits private-destructor scalar allocation
in an unevaluated decltype and array allocation inside the owning class;
PA6/300 rejects same-cardinality exception specifications with different types.
The positive combines both access contexts without introducing a second
allocation role into one LowIR reference. Both selected checks pass, as do eight
strict C++11 host checks. Thirty earlier qualification fixtures are separately
committed as e4c80f69d.

The remaining definitions use explicit outcome predicates in the existing
controls lanes. scripts/check_pending_audit_regressions.py runs them explicitly;
it does not join make test-report or change student grading discovery. Its
maintainer Python checker is excluded by the current export filter. These
tests are ready for the fix queue, not a claim that the failing implementation
or a student release is ready. Do not generate successful references from the
current incorrect behavior. Promotion into required suites and the final
combined export remain with the corresponding fixes.

The inventory contains 47 C++ controls, one LowIR frame control and seven small
metadata controls, covering forty tracker families. One additional LowIR input
supplies the reader control for comparison operand width. Metadata reuses existing
inputs for PA9 address mangling, both optimized array-construction programs,
both standalone source-handler programs and the PA32 backward pointer loop.
It avoids six duplicate input programs. The registry below records the intended
additional value; it does not install exploratory matrices wholesale.

| Family | Added or reused boundary |
| --- | --- |
| FLOW-DEFINED | Constant-loop return analysis and unreachable handlers; no undefined fallthrough rejection oracle. |
| REF-BITFIELD | One copied-value runtime and independent mutable/volatile rejection checks. |
| LOOKUP-TAG | Hidden friend visibility and an explicitly declared nested tag are distinct lookup obligations. |
| LOCAL-ODR | Ordinary local-class use, member default argument and lambda capture contexts; disputed constant captures remain held. |
| REF-BRACE | A minimal braced class temporary bound to a local reference. |
| NEW-ARRAY-DTOR-ACCESS | One negative for inaccessible array-element destruction; the combined valid allocation contexts are in the default suite. |
| INHERITED-DEPENDENT | Parameterized using T::T construction; no duplicate zero-argument test. |
| ASSERT-MESSAGE | An ordinary non-string message and a suffixed string token require independent rejections; no character/integer permutation matrix. |
| CONST-BITFIELD | Width truncation before constexpr conversion; separate from reference binding. |
| CONST-REF-STATIC-TEMP | A constexpr reference and assertion over its static temporary. |
| TMPL-ACCESS-SFINAE | Function-overload and partial-specialization immediate contexts; positive outcomes retain fallback selection. |
| MANGLE-CONV | Declared conversion target symbols at O0, where the functions are emitted; allowed O2 elimination is not a failure. |
| MANGLE-RESULT | One object combines bare-id decltype and a dependent named-call result; exported addresses retain symbols at O2. |
| TMPL-LATE-TYPE | A later traits definition plus an undemanded invalid member body, with no hosted headers. |
| MANGLE-PACK | One completed default pack with an address that retains the declared parameter expansion symbol. |
| EH-ARRAY-DTOR | Three elements expose the missing normal-destruction unwind suffix; no extra array-size matrix. |
| REF-BASE-COND | A conditional derived temporary bound to its base reference. |
| EH-RETHROW-DYNAMIC | One called helper rethrows the dynamically active exception. |
| EH-LOCAL-ARRAY-CATCH | One direct backing-element failure must reach the current function handler and retire the prefix. |
| NOEXCEPT-LIST | A header-free initializer-list argument inside noexcept. |
| INIT-LIST-STATIC | Static value backing survives initialization and repeated access. |
| TMPL-FTRY | A demanded function template retains its body and handler, with a destruction trace. |
| MEMBER | Signed inverse member-function adjustment through an unknown receiver, repeated empty-base identity and rejection of mismatched comparison operand width using ordinary i64/i32 types. |
| CONST-MEMBER-BOOL | A nonnull member pointer in static_assert, distinct from ordinary runtime invocation. |
| VBASE | Defined most-derived initialization of all virtual scalar bases; no indeterminate value oracle. |
| ABI-GLOBAL | The external global symbol is g; a correct object-symbol oracle supplements current incorrect inspection references. |
| EH | Global replacement new/delete count failed-construction deallocation without hosted includes. |
| BACKEND | Two existing source programs are reused through the standalone native route. |
| EH-UNWIND-DTOR | An ordinary automatic destructor throwing during unwind must invoke the installed termination handler. |
| MANGLE | Clang-checked RTTI template substitution plus reused PA9 entity-address facts. |
| PA29-ALIGN | Combine the already required GNU aligned alias and indirect-expression boundaries; no new vendor feature is admitted. |
| EXPLICIT-CONTEXT | Constructor-template private conversion and substitution fallback; only the documented libc++ 21 conditional-explicit dependency. |
| INHERITED-VALIDITY | The already supported constructibility intrinsic includes deleted other-member construction. |
| ATTR-NORETURN | One forbidden argument-clause rejection; existing valid noreturn tests are reused. |
| LOOP-PTR-FINITE | Reuse the existing backward-loop input with congruent and incongruent LowIR addresses at O1/O2/O3. |
| ARG-SLOTS | Two mutually exclusive 1600-byte LowIR objects, correct values from both paths, and a 2048-byte MIR frame bound. |
| BACKEND-ARRAY-OPT | Reuse both existing full-TU construction programs at O2; no new array fixtures. |
| LOOKUP-NAMESPACE | The prepared same-type convergence positive, pending replacement of the existing wrong rejection fixture. |
| EH-SPEC-SET | One positive combines ordering and duplicates; the independent different-set negative is in the default suite. |
| LOOKUP-NAMESPACE-MIXED | Independent class/value and typedef/value conflicts; positive elaborated lookup remains covered separately. |

All 99 applicable strict-C++11 host observations pass independently with both
Clang and GCC. Conditional-explicit controls keep C++11 mode but permit the
documented required extension; ordinary controls use pedantic-errors. ABI
symbol predicates were first checked against fresh Clang objects. The solution
passes 3 of 108 observations: the three congruent pointer-loop inputs. All
other observations expose tracked compiler, backend, ABI or quality failures.
The opposite loop inputs return incorrectly instead of remaining nonterminating.
O0 calibration passes both loop controls; nontermination follows the modulo-eight
invariant for plain 64-bit LowIR indexing, with a two-second timeout used only
as an observation. No source-language pointer UB or forward-progress rule is
used as an oracle.

The normal strict report passes 6105/6105 with exactly one line. The full
harness passes; four new integration tests check diagnostic rejection, signals
and unsupported exits, runtime results after successful compilation, and a
successful encoder emitting a wrong name. All 47 C++ controls are explicitly
checked through the placement detector because the normal audit excludes the
controls lane; no early feature or cluster remains. The normal placement audit
also passes. No normalized duplicate C++ control exists.

Retain/combine decisions for this batch are recorded in the table: one combined
positive per compatible family, independent negative obligations, six reused
input programs, no zero-arity inheritance permutation, no duplicate default
noreturn positive, and no disputed/post-C++11 feature expansion. Full course
fixture minimization stays open until the pending controls are promoted with
their fixes, so their overlapping temporary check paths can then be removed.
Raw commands, diagnostics, hashes and terminal outcomes are retained in
/tmp/cppgm-v4-audit-review/test-additions/. The four original unrelated untracked
reference artifacts remain outside this test-only checkpoint.

The reader-width control also reproduces the missing validation: lowiropt O0
accepts an i64 parameter used directly by an i32 comparison. It uses ordinary
scalar widths rather than adding i128 support as a new requirement. Existing
PA22 member-pointer null/contextual-bool fixtures remain; a fresh converted-null
runtime observation already passes here and does not justify another fixture.
The final focused report is therefore 3/108, with 105 failing observations of
tracked obligations, rather than 105 separate bug discoveries.

### Final test-definition coverage check

The final inventory review found one omitted admitted family:
LOOKUP-BASE-ALIAS. It is supported by the already reviewed N3485 10.2/3,6,7
rule, even though GCC disagrees with Clang. The additional PA22/100 metadata
control reuses PA30's existing same-type member-alias input with an explicit
compile-success predicate at O0/O2. PA22 owns non-virtual multiple inheritance.
No second source, permutation matrix or generated successful reference is
added. Correct and move the existing required fixture when its compiler fix
lands; its present incorrect rejection reference remains visible in the queue.

The complete test-definition inventory is now 32 new default fixtures and
56 opt-in controls covering 41 remaining families. Fresh complete focused
runs give Clang 101/101, GCC 99/101 and the solution 3/110. GCC's only two
failures are this independently justified base-alias boundary. The solution's
two additional failures reproduce the known lookup bug, rather than creating
new issue families. The four runner integration tests pass again. Explicit
feature detection checks the reused input against PA22/100; the standard
placement audit also passes. Full focused reports and placement evidence are
retained as final-{clang,gcc,solution}.json, base-alias-placement.json and
final-placement.log under the same test-additions scratch directory.

This final addition changes only an opt-in control and this tracker. The live
compiler hash remains c32713ca936d130b3837d4ceb86e96fd9b8a11fb585e6375d69cdedc5d3b57a4;
the previously verified strict default report remains 6105/6105 in one line.
No compiler/performance change or export regeneration belongs to this
test-addition checkpoint. Outstanding fixes, final fixture pruning/promotion
and the combined student export remain separate work in this same tracker.

## Reference-binding performance diagnosis and continued fix

The frontend-counter diagnostic is now downloaded and independently checked:
256 observations have successful status, four unscaled counters and identical
objects for each workload. For EH handlers, A/B instructions are unchanged
(0.999999997), cycles are 1.017431, instruction-cache misses are 1.048450 and
frontend undersupply is 1.035265. A/A medians are respectively 1.000000047,
1.001462, 1.004556 and 1.003591. Qualification-view A/B instruction and cycle
ratios are 0.999905799 and 1.006089, with cache/stall ratios 1.012871 and
1.013261; its A/A cycle ratio is 1.006443. These explain a fetch-related
cost for the prior image; they do not replace or pass the failed cycles gate.
Raw results remain in reference-cv-qualification/frontend-counter-diagnostic-results/.

The next source change removes an eager temporary-binding cv query from
qualification-only returns and skips it for rvalue references. It preserves
the existing const/nonvolatile temporary-binding rule. This is a code-driven
change rather than a layout flag, padding adjustment or a blind timing rerun.
The frozen twenty-fourth image is
3aeebec9c057a22386fcd08b23ee43fee2596cc00f80be97cbbd887b2254b984.
All nine boundary/regression matrices complete successfully: 574 fresh
observations retain their previous status, link and defined runtime boundaries;
376 unchanged strict-C++11 host observations are reused. Only the already
invalid negative bit-field runtime can vary and is excluded as an oracle.

Full compiler validation and the preexisting broad/focused Alpha protocol are
running against that frozen image. The remote setup verified all 91 frozen
inputs before starting. No compiler source is committed and no open reference
row is closed by this diagnostic or by starting the measurements. Retain the
existing instruction/RSS gates and both raw/calibrated focused cycle gates.
The required next decision is based on terminal validation and performance
results, without restarting an observed live process or ignoring a failed gate.

### Terminal reference-only results and the next required fix

The twenty-fourth reference-only image passes all eleven local check groups:
PA10/12/20/21, strict 6105/6105 in one line, debug-info, variants, self-host
through PA5, nine architecture audits, file audit and placement. Its performance
study completes 960 broad and 3,840 focused observations with all objects equal
and successful statuses. Every CSV independently has four unscaled counters.
All instruction/RSS gates pass, but nine focused cycle gates fail. Raw/calibrated
ratios are: auto aliases 1.015117/1.013720, auto prvalues 1.006887/1.006779,
EH handlers 1.010349/1.010619, initializer-list values 1.015923/1.016733,
qualification views 1.009722/1.011109, reference aliases 1.015723/1.014737,
nonthrowing reference temporaries 1.006621/1.008568, virtual dispatch
1.005602/1.006067 and volatile views 1.009457/1.007082. This is a failed
performance verdict, not an accepted compiler checkpoint. Raw results and the
independently checked gate-review.json remain in perf-twenty-fourth-results/.

The next concrete correctness work is EH-SPEC-SET, rather than another
reference-only layout experiment. ConfigureFunctionExceptionSpecification
deduplicates adjusted TypeIds in its existing vector and checks membership in
the canonical declaration's range. It preserves first-occurrence order and
adds no cache/table or extra allocation. Nine scratch programs independently
check duplicate-first declarations, aliases, adjusted types, incompatible sets,
references/pointees and empty/nonempty boundaries. Ours and Clang each pass
18/18; GCC passes 16/18, disagreeing only on the array/function adjustment
explicitly specified by N3485 15.4/2. All observations and full diagnostics are
retained; this disagreement adds no required permutation fixture.

The one existing positive definition moves from the opt-in controls lane to
pa6/tests/general/300-dynamic-exception-specification-set.t; its references
are generated through the exact ref-test selection. The independent different-set
negative stays. PA6 passes 112/112, with full validation still running. The
frozen combined image is ee8fb044cbeed13d8e930a2ee21040060f624b67767df342238d161efbdac199.
Alpha verifies the same 91 inputs and runs the unchanged global protocol against
the last approved baseline e4bcdecf0e30356f800ccebd7ba88e6686ed261f479874c50fe566499d78af6c.
This baseline is not reset to a previously failed candidate. Results and frozen
source patches live under /tmp/cppgm-v4-audit-review/exception-specification-sets/;
Alpha retains /tmp/cppgm-v4-audit-review-20261002-exception-specification-sets/.
Both compiler fixes remain uncommitted until required checks and gates resolve.

## Exception-set terminal checks and array-new access work

The exception-set image completes all eight local validation groups with
successful terminal status: PA6, strict report, debug-info, variants, self-host
through PA5, architecture, file audit and placement. Its strict report is
exactly one line, 6106/6106. Alpha completes all 4,800 observations with equal
objects and four unscaled counters per observation. All instruction/RSS gates
pass. Two focused raw/calibrated cycle gates fail: initializer-list values
1.005242/1.005932 and nonthrowing reference temporaries 1.006389/1.005479.
Retain this failed global verdict in exception-specification-sets/perf-results/;
the baseline remains the last approved compiler, not a failed candidate.

NEW-ARRAY-DTOR-ACCESS now has a concrete source fix. AnalyzeArrayNewExpression
gets its existing indexed destructor binding for every class element, checks
deletion/access, and keeps destructor emission conditional on nontriviality.
The access call supplies the allocated class as both naming and object class;
N3485 11.4/1 and fresh Clang/GCC observations require rejecting a derived
member which allocates base objects with a protected base destructor. No new
lookup table, cache, reparse or speculative instantiation is introduced.
DestructorForType is a const indexed query. Scalar new is unchanged.

Eleven scratch programs preserve owner/friend/public/scalar positives and
private/protected/deleted/zero/trivial boundaries at O0/O2. The initial inline
member examples were undemanded in our compiler; their source snapshots and
observations are retained, but are not evidence that the allocation body ran.
Ordinary callers now demand those definitions. The complete demanded set
passes ours and Clang 22/22; GCC passes 16/22 and accepts the deleted,
zero-extent private and defaulted-private cases. Those three disagreements
remain scratch evidence; deleted-array rejection already existed here, and
none adds a required fixture. The pre-fix image passed 12/22, retaining the
ten observations of missing access checks.

The existing private-array rejection moves into
pa12/tests/general/400-new-array-private-destructor-bad.t and its temporary
control is removed. One independent protected-base-object rejection is added
as 400-new-array-protected-base-destructor-bad.t; it demands the inline body.
The existing combined scalar/own-array positive stays. Both negative reference
sets are generated through the exact ref-test selection. This is one access
boundary per fixture, rather than a permutation matrix or a cloned positive.

PA12 passes 298/298; strict passes 6108/6108 in exactly one line, and debug-info
passes. Remaining local checks and Alpha measurements are running against the
frozen combined image 59e3168a112b07e27745a4b549edab14d19f994d7fc1276500408893249804aa.
The preceding Alpha study is terminal before this one starts. All 91 inputs
are verified, the approved baseline and all gates are unchanged, and every
observation is retained. Scratch sources, image hashes, source patches and
check logs live in /tmp/cppgm-v4-audit-review/array-new-destructor-access/;
Alpha retains /tmp/cppgm-v4-audit-review-20261002-array-new-destructor-access/.
Compiler source and promoted fixtures remain uncommitted pending the full gates.

## Performance-approved reference, exception-set and array-access checkpoint

The combined array-access image 59e3168a112b07e27745a4b549edab14d19f994d7fc1276500408893249804aa
passes every required gate. All 4,800 observations have successful status,
identical objects for each input and four unscaled counters. Every instruction
ratio is at most 1.000086 and every median paired RSS ratio is 1.0. All ten
focused workloads pass both the raw and calibrated 1.005 cycle limits. The
largest raw/calibrated cycle ratios are 1.001437/1.002108. Ratios below one
are retained as measurements, without claiming a general speedup.

The approved baseline remains e4bcdecf0e30356f800ccebd7ba88e6686ed261f479874c50fe566499d78af6c,
the last approved AUTO-CONST-REF image. Every frozen input hash is verified.
Earlier failed variants and studies remain recorded; no gate, baseline,
workload, compiler flag or observation is removed to obtain this result.
Independent review is in array-new-destructor-access/perf-results/gate-review.json.

All eight final local check groups pass: PA12, strict 6108/6108 in one line,
debug-info, backend variants, self-host through PA5, all nine architecture
targets, file audit and placement. The exception-set checkpoint independently
passes PA6 and those same broad checks. The final frozen image also reruns all
nine reference-boundary/regression matrices: 574 fresh observations preserve
all required boundaries, with 376 unchanged strict-C++11 host observations
retained. Only runtime garbage from an independently invalid negative bit-field
control can vary; it is not an oracle. The thirty required qualification inputs
and current semantic source patch match the frozen approved snapshot exactly.

Commit the five semantic source files and the three promoted/added fixture
sets together: four reference-qualification/category tracker rows, EH-SPEC-SET
and NEW-ARRAY-DTOR-ACCESS are complete. The pending noreturn syntax edit is
excluded from this snapshot and receives its own validation/performance study.
No ABI encoder or generated reference is hand-edited, and the unrelated four
untracked artifacts remain outside the checkpoint. Final course-fixture pruning
and the combined student export remain open with the other compiler fixes.

## Standard noreturn argument checkpoint and completed test definitions

The preceding semantic checkpoint is committed as ed64e4b4b. The separate
syntax checkpoint fixes the reviewed C++11 argument constraint in N3485
7.6.3/1: the unqualified standard noreturn attribute cannot have an
attribute-argument-clause, including an empty one. The existing standard
attribute consumer checks that boundary and consumes balanced argument tokens
once. Identifiers inside an opaque argument payload no longer accidentally
become declaration attributes. No new parser helper, source file, cache or
vendor attribute is introduced.

Eight scratch programs run at O0/O2. Ours and GCC pass all sixteen observations;
Clang passes fourteen, accepting the malformed unknown-attribute argument
cases which ours and GCC reject. Those disagreements remain scratch evidence.
Unknown-attribute behavior is implementation-defined under N3485 7.6.1/5;
the opaque-payload controls preserve the course's existing policy, rather than
claiming a universal C++11 acceptance rule. Variable appertainment remains a
separate review and adds no mandatory fixture.

The existing temporary negative moves to
pa29/tests/compile/500-standard-noreturn-arguments-bad.t with its exact
ref-test-generated reference and rejection status. Its temporary copy is
removed. Existing valid noreturn runtime fixtures are reused; no new positive
or malformed-unknown-attribute fixture is added. The independent protected
base-object array-new negative in the preceding checkpoint tests the distinct
object-class access restriction, and its inline allocation body is demanded.

All eight local groups finish successfully: PA29 405/405, strict report
6109/6109 in exactly one line, debug-info, backend variants, self-host through
PA5, all nine architecture targets, file audit and placement. The four pending
regression runner integration tests also pass. The source patch and frozen
compiler hashes are retained in
/tmp/cppgm-v4-audit-review/noreturn-attribute-arguments/.

Alpha's frozen global comparison is terminal with 4,800 successful observations,
equal objects for every workload and four independently verified unscaled
counters per observation. All instruction/RSS gates and all ten focused raw
and calibrated cycle gates pass. The largest instruction ratio is 1.000086;
all median paired RSS ratios are 1.0. The largest focused raw/calibrated cycle
ratios are 1.001939/1.002583. This protocol was frozen against the approved
AUTO-CONST-REF image e4bcdecf0e30356f800ccebd7ba88e6686ed261f479874c50fe566499d78af6c
before ed64e4b4b was approved; it is a global comparison against that image,
not a separate incremental timing claim against ed64e4b4b. The candidate is
bdecba5ad57cdaf6293fe5e53ca98513e6b9a250d00136a3aab8f6ee5b55b283.
All 91 input hashes are verified. Protocol, raw observations and gate review
remain in noreturn-attribute-arguments/perf/ and perf-results/ locally, with
the full objects retained on Alpha under
/tmp/cppgm-v4-audit-review-20261002-noreturn-attribute-arguments/.

The requested admitted test-definition queue is complete. Unfixed compiler
regressions remain explicit opt-in controls with independently reviewed
expectations, rather than accepting today's incorrect solution behavior as
generated reference truth. Promotions remove their temporary controls, and
metadata controls reuse existing inputs where appropriate. Final course-wide
fixture minimization follows the remaining fix/promotion sequence; combined
student-export validation remains deferred to that final stage as requested.

## Namespace type convergence candidate

LOOKUP-NAMESPACE now has a three-line semantic change in the existing model
owner. Namespace graph results use MergeLookup's existing same-canonical-type
option, and pending namespace imports use that option even without a masking
direct typedef. Class-base graph merging retains its current separate rule.
The change uses existing indexed edges and TypeIds, keeps direct-lookup early
returns, and introduces no allocation, cache, reparse or declaration scan.
N3485 7.1.3/1,2 makes typedef/alias declarations synonyms for their types;
7.3.4/6 preserves ambiguity between distinct entities found in namespaces.

The frozen compiler is
fd842d2ce4c05b3d7588291bf2eca6e4cce9c67dae870eacd681c2bc6b9cf7e0.
All 72 core observations pass: eighteen programs at O0/O2 and in both
semantic dump modes. Qualified, transitive, cyclic, block and array/class
alias convergence succeeds. Distinct types, cv qualifications, bounds,
classes, variables, functions and namespace targets still reject. Twenty
planned-fixture/scope-carrier observations and eight elaborated/local-hiding
observations also preserve their required outcomes. The sixteen previously
incorrect mixed type/value observations remain incorrect and are retained
as LOOKUP-NAMESPACE-MIXED, rather than claimed fixed by this change.

Replace the old PA6/300 same-int-typedef rejection with the reviewed composite
pa6/tests/general/200-namespace-type-convergence.t. Its exact reference is
generated through ref-test, and its temporary control is removed. The existing
genuine distinct-type PA6 and distinct-variable PA7 negatives remain. This
replacement adds no duplicate source fixture. The promoted input passes ours,
Clang and GCC at O0/O2, and PA6 passes 112/112.

The remaining required local groups are running under handle 74963 in
namespace-convergence/validation-first/. Alpha handle 91870 measures immutable
images against the now-approved 8a6bcd105 compiler
bdecba5ad57cdaf6293fe5e53ca98513e6b9a250d00136a3aab8f6ee5b55b283.
All 92 frozen input hashes are verified. The protocol retains all twenty broad
workloads (960 observations), and 48 interleaved A/A and A/B blocks on each of
five focused workloads (1920 observations): namespace lookups, token handling,
reference aliases, automatic aliases and virtual class lookup. Instruction,
RSS and focused raw/calibrated cycle limits remain 1.005, 1.03 and 1.005.
Objects must be identical for every workload; all observations are retained.
The namespace performance input is unchanged from the earlier pre-fix review.
Protocol, scripts, hashes and results are retained in
/tmp/cppgm-v4-audit-review/namespace-convergence/perf-first/ and on Alpha in
/tmp/cppgm-v4-audit-review-20261002-namespace-convergence-first/.
Compiler code and promoted references remain uncommitted pending these gates;
the full tracker, final fixture pruning and combined export remain open.

The namespace candidate's gates are now terminal and successful. All eight
local groups pass, including backend variants, self-host through PA5, all nine
architecture targets and the file audit. Strict output is exactly one line,
6109/6109. Placement scans 3229 inputs with zero placement findings, zero tests
needing review and zero local hygiene findings; the existing separate EH
review leads are unchanged. The source patch and live compiler match the
frozen image and patch hashes exactly.

The incremental Alpha study passes all declared gates: 960 broad and 1920
focused observations have successful status and equal objects per workload.
All 2880 CSV files independently contain the four required unscaled counters.
The largest paired instruction ratio is 0.999991 and all median paired RSS
ratios are 1.0. The largest focused raw/calibrated cycle ratios are
0.999910/1.000078. Namespace lookups measure 0.988297/0.988226; ratios below
one remain measurements, without a general speedup claim. No observation,
input, baseline or gate is changed or removed. The complete raw study and
independent review are retained in namespace-convergence/perf-first-results/,
with the full objects retained in the declared Alpha directory.

Commit the existing semantic owner and the PA6 replacement/reference set
together. The genuine ambiguity negatives remain, and the temporary convergence
control is removed. LOOKUP-NAMESPACE is complete; the sixteen mixed type/value
failures remain open in LOOKUP-NAMESPACE-MIXED, and LOOKUP-BASE-ALIAS remains
separate. Neither this checkpoint nor the completed test-definition queue
closes the remaining compiler fixes, contract reviews, final fixture pruning
or combined student-export validation.

## Mixed namespace ambiguity: test promotion and validation

The pending LOOKUP-NAMESPACE-MIXED tests now cover two distinct use contexts:
PA6/300 rejects an imported typedef/value conflict in a declaration, and
PA7/300 rejects an imported class/value conflict in an ordinary expression.
The PA7 class is only forward-declared; the test needs no class construction,
layout or class-aware call. Exact ref-test selections generate empty rejection
outputs and failure sidecars. Both hosts and the candidate agree at O0/O2.
Remove the two overlapping opt-in controls rather than adding their entire
qualified, function and template permutation matrix to the required suite.
The earlier tests-first definitions remain available for the independent fixes.

The compiler distinguishes ordinary type-name visibility from restricted
elaborated lookup, and checks type/namespace compatibility before merging
template markers. Functional casts reuse that ordinary lookup. Constructor
names stored by the implementation retain the injected class name using the
canonical binding's existing constructor fact, including out-of-class
redeclarations. No source file, record field, scan, cache or allocation is added.

Earlier correctness candidates and their diagnostics are retained, including
the first constructor regressions and the fourth candidate's two PA12
out-of-class constructor failures. None was performance-approved. The fifth
image is 343f056c6e0c1134fba9e91cef0f84b18e45076303aa9f15d1159c549cc764f3;
its patch hash is 416c254b1e8dca2e9cbb09dfc8ad3df0a1b9cabb688b9e0321b5dee315fb5943.
All 116 namespace controls and forty additional agreed boundary observations
pass. Explicit template prefixes follow ordinary visibility, while the
elaborated-template host disagreement remains scratch evidence. The additional
using-declaration probes preserve overload merging and reproduce an existing
same-scope constructor limitation in both baseline and candidate; no new
required fixture is inferred from that existing limitation.

PA6/7/11/12/18, strict 6111/6111 with exactly one line and debug information
pass. Backend variants and the remaining local checks are running in
namespace-mixed-lookup/validation-fifth/. Alpha handle 24705 measures the
immutable fifth image against approved c286b1db6 (fd842d2ce4c05b3d7588291bf2eca6e4cce9c67dae870eacd681c2bc6b9cf7e0).
All 92 input hashes are verified. The declared protocol retains twenty broad
workloads (960 observations) and eight focused workloads with 48 interleaved
A/A and A/B blocks (3072 observations). The previous five focused workloads
are supplemented by existing constructor, conversion-function and template
copy workloads because ordinary type visibility changes those paths. Limits
remain 1.005 instructions, 1.03 RSS and 1.005 raw/calibrated focused cycles;
all objects must agree. Raw results remain under perf-fifth/ and the declared
Alpha directory. Code and promoted tests await these gates before committing;
other fixes, final course-wide pruning and combined export remain open.

The fifth candidate's checks are now terminal. All twelve local groups pass,
including self-host through PA5, all nine architecture targets, the file audit
and placement (3229 inputs, zero early/review/hygiene findings). The complete
Alpha study retains 960 broad and 3072 focused observations, successful statuses,
equal objects and four unscaled counters per CSV. All 92 input hashes agree.
Instruction and RSS gates pass, but seven focused raw/calibrated cycle gates
fail: automatic aliases, constructor/function/template copy, namespace lookup,
reference aliases and virtual classes. Raw ratios range from 1.006146 to
1.012321 on those workloads; the gate is 1.005. No input, observation, baseline
or threshold changes. Full results are retained in perf-fifth-results/ and
on Alpha; this candidate is not approved for the compiler checkpoint.

Respect the user's immediate test-addition scope: retain the full candidate
patch and generated default promotions in scratch, restore the approved
compiler source, and keep the rejection definitions opt-in until the fix passes
its gates. The PA6 typedef/value control is unchanged. Replace the overlapping
PA6 class/value declaration control with the reviewed ordinary-expression
control in PA7/300, its earliest expression owner. There is no net addition of
probe inputs and no generated default reference committed from the held fix.
Fresh Clang/GCC checks pass 4/4 per host at O0/O2; explicit control placement
passes 2/2. The four regression-runner integration tests also pass. This completes
the admitted regression definitions without launching another compiler variant;
remaining fixes, final course-wide fixture pruning/promotion and the combined
student export remain open in this tracker.

The approved compiler rebuild exactly matches c286b1db6 image
fd842d2ce4c05b3d7588291bf2eca6e4cce9c67dae870eacd681c2bc6b9cf7e0.
The final strict default report passes 6109/6109 and contains exactly one line.
The two opt-in rejections still fail 0/4 with the approved solution because it
accepts the ill-formed programs; these are retained bug predicates, not blessed
references or default-suite failures. The tests-only checkpoint contains the
control replacement and this tracker, with no compiler diff or export rebuild.
The four original unrelated untracked reference artifacts remain excluded.

## Continued mixed-lookup performance review

The broader tracker goal resumes after the tests-only checkpoint a308555f0.
The approved source and image were verified before this continuation. No Alpha
measurement process remained live. Forty-eight interleaved profiles of the
approved and held fifth images now retain successful status and identical
objects on the frozen namespace and constructor workloads. The reports contain
about eight thousand samples per image/workload; individual lookup functions
have too few samples to attribute the approximately one-percent cycle difference.
Do not treat their noisy function deltas as a proved cause. Profiles, commands
and objects remain in profile-fifth-results/ and the terminal Alpha directory.

Source review identifies a concrete avoidable change in the fifth candidate:
inserting LOOKUP_TYPE_NAME after LOOKUP_TYPE renumbers ordinary, scope-carrier,
function-template and variable-template kinds, changing constants throughout
otherwise existing lookup call sites. The sixth candidate appends the new kind,
preserving all existing values. The full type/value ambiguity semantics,
canonical-constructor handling and restricted elaborated lookup remain intact;
no scan, cache, record field, padding or narrower feature behavior is introduced.
This is a hypothesis to measure, not performance approval. The source is restored
from the retained full fifth patch and the host build is running under handle
50431. All existing correctness controls and the unchanged incremental gates
remain required before promotion or a compiler commit.

The sixth build completed successfully. Its immutable image is
358ff8dc9a28d33a7154a5750b5c5023689d24b59e44d7e0e347c9fdc4a99d01;
its source patch is 5e6536beb9929b48574a42005437d9e67750b46d2c5c2d6a68408b4b729b59d2.
All 184 focused outcomes pass: 116 namespace/dump observations, forty agreed
boundaries, eighteen reviewed explicit-template-prefix outcomes, six member-name
checks and the four exact planned rejection observations. Fresh hosts also pass
the forty agreed boundaries; the known baseline failures and the elaborated
template host disagreement remain separate. PA6/7/11/12/18 pass. The strict
report and remaining required groups are running under handle 61765 in
validation-sixth/. Alpha has verified all 92 frozen inputs for the unchanged
960-broad/3072-focused protocol; measurement awaits strict correctness. No
control is promoted and no compiler source is committed before all gates pass.

The sixth strict report passes 6109/6109 with exactly one success line. Debug
information and backend variants pass as well; self-host and the remaining
audits continue under handle 61765. The live image matches the frozen hash.
Alpha handle 59519 now runs the declared 4032-observation incremental protocol
in /tmp/cppgm-v4-audit-review-20261002-namespace-mixed-lookup-sixth/. Baseline,
inputs, 48 focused blocks, unscaled counters, equality requirements and all
gates are unchanged. These are live checks, not a completed performance claim.

All twelve sixth-candidate local groups are terminal and successful, and its
live image/source patch match the frozen hashes. Placement scans 3229 inputs
with zero early, review or hygiene findings. Alpha handle 59519 is terminal zero
with all 92 input hashes verified, all 4032 successful observations and equal
objects. Independent CSV verification confirms four unscaled counters per run.
All instruction and RSS gates pass (maximum instruction ratio 1.000841, RSS
1.0). Six focused cycle gates pass; namespace lookup measures raw/calibrated
1.005543/1.007785 and virtual classes 1.007608/1.007572, both above 1.005.
This reduces the failed workload count from seven to two but does not approve
the candidate. Complete evidence is retained in perf-sixth-results/ and Alpha;
no observation or gate is discarded or changed.

Producer review finds another concrete avoidable dependency in the injected
class-name guard: ordinary primary constructors already have their local
BindingRecord.constructor fact, set by declaration analysis. Using-access
aliases copy that fact; fresh out-of-class aliases may only have the canonical
fact, so the fallback must remain. BindingRecord places canonical identity and
constructor flags far apart. The next candidate will prefer an existing true
local constructor fact and consult the canonical record only when necessary;
it will preserve variable hiding, the aliases that caused the earlier PA12
regressions, the existing enum values and the complete mixed lookup behavior.
This uses existing typed facts without adding a field, cache, scan or category
shortcut; correctness and the unchanged incremental gates remain required.

## Constructor-fact fast path for mixed lookup

The seventh candidate now implements the reviewed local-fact fast path: use
the ordinary binding's existing constructor flag when true, otherwise retain
the canonical lookup. This changes one predicate, adds no field or scan, and
keeps the complete mixed namespace fix and stable lookup-kind values. The
immutable image is 10fe466dfe53f7f84d78e9ebca1891769e6715cc511e85200fb1fe5d9be5faad,
with patch cef47e837d7f78783668af23614387e21e06ea675a36c536022bb012877dc2db.
All 184 focused checks pass with exact outcomes, and both hosts again pass the
forty agreed boundaries. Earlier host disagreements remain separate evidence.
PA6 passes; the remaining twelve-group validation is running under handle
30170 in validation-seventh/. The same 92-input, 4032-observation Alpha protocol
is prepared under perf-seventh/ and awaits strict correctness. Baseline and
all gates remain unchanged; source and controls await full approval.

The seventh strict report passes 6109/6109 with exactly one final line, and
debug information passes. Remaining local groups continue under handle 30170.
The live compiler matches the immutable seventh image. Alpha handle 97820 now
measures that image against the unchanged approved baseline in
/tmp/cppgm-v4-audit-review-20261002-namespace-mixed-lookup-seventh/. All 92 input
hashes were verified before launch; every declared gate and workload remains
unchanged. This records a running measurement, not a completed gate result.

All twelve seventh-candidate local groups are terminal and successful.
Placement scans 3229 inputs with zero early, review or hygiene findings; live
image and patch hashes match the frozen metadata. Alpha's 960-observation broad
phase is terminal zero with matching objects. The 3072-observation focused
phase remains live under handle 97820; final gate review is still required.

The seventh Alpha run is terminal zero under handle 97820. All 92 input
hashes agree; 960 broad and 3072 focused observations have successful status
and identical objects, with four independently verified unscaled counters per
CSV. Instruction and RSS gates pass (maximum instruction ratio 1.000864, RSS
1.0). Every focused raw cycle gate passes; only calibrated copy constructors
fail, at 1.005646 (raw 1.003302). The other seven calibrated gates pass. The
namespace and virtual workload ratios are now below one; this is measurement,
not a general speedup claim. All observations and thresholds remain unchanged
in perf-seventh-results/ and the declared Alpha directory.

Source review confirms the completed type-result branch can return immediately:
its kinds are TYPE, SCOPE_CARRIER, ORDINARY and TYPE_NAME (1,3,2,6), disjoint
from FUNCTION_TEMPLATE and VARIABLE_TEMPLATE (4,5). Ordinary hiders already
return earlier, and namespace/type identity fields are populated before the
new return. The eighth candidate adds that return, retaining every required
lookup behavior and avoiding subsequent irrelevant template-only filters. No
field, cache, helper or scan is added. Build and all required gates remain
pending; this is a source rationale, not claimed performance attribution.

The eighth host build is terminal zero and produces a distinct immutable
image, 1348fa72086ebd8b692c2374bad19f98f995d940091f5dce4627074a66338166,
with patch a94dfbcf59804f884e677964dd079315b5af1249818809f702bbd2ee987677f0.
The released DirectLookup symbol shrinks from 994 to 836 bytes (cold stub
unchanged); this confirms an executable change, not a performance result.
All 184 focused outcomes pass, including planned rejections and canonical
constructor aliases; both hosts again pass the forty agreed boundaries.
The twelve required local groups are running under handle 35939 in
validation-eighth/. The unchanged 4032-observation, 92-input Alpha protocol is
prepared under perf-eighth/ and awaits strict correctness. No gate is relaxed
and no compiler commit or test promotion is made before full approval.

The eighth strict report passes 6109/6109 with exactly one final line, and
debug information passes. Remaining local groups run under handle 35939.
The live binary matches the frozen eighth image. Alpha handle 21633 now runs
the unchanged 4032-observation protocol against the approved baseline in
/tmp/cppgm-v4-audit-review-20261002-namespace-mixed-lookup-eighth/. All 92 input
hashes were verified before launch; every workload and gate remains unchanged.
This is a live run, not a completed performance claim.

All twelve eighth-candidate local groups are terminal and successful.
Placement scans 3229 inputs with zero early, review or hygiene findings; the
live image and patch match their frozen hashes. Alpha's 960-observation broad
phase is terminal zero with matching objects; the focused phase remains live
under handle 21633. Final counter scaling/equality and all declared gates must
still be independently checked. No performance claim or test promotion is made
from a partial run.

The eighth Alpha run is terminal zero under handle 21633. All 92 frozen
inputs agree; every one of the 4032 observations has successful status and
identical objects, with four independently verified unscaled counters per CSV.
Instruction and RSS gates pass (maximum instruction ratio 1.000873, RSS 1.0),
but six focused workloads fail both raw and calibrated cycle gates: auto
aliases, copy constructors, copy functions, copy templates, namespace lookups
and reference aliases. All observations remain in perf-eighth-results/ and
the declared Alpha directory; no gate or result is discarded.

Revert only the eighth candidate's early return. The restoration build is
terminal zero under handle 32380. The live compiler again has seventh-image
SHA 10fe466dfe53f7f84d78e9ebca1891769e6715cc511e85200fb1fe5d9be5faad,
and its six-file source patch again has seventh-patch SHA
cef47e837d7f78783668af23614387e21e06ea675a36c536022bb012877dc2db.
The prior twelve-group correctness validation therefore applies to these
identical bytes; its performance hold remains in force.

A measurement-script review found unequal output-path lengths in A/A blocks:
label `aa` adds one character relative to `a`, whereas A/B labels have equal
width. Both A/A labels execute the same immutable baseline binary. The audit
is retained in namespace-mixed-lookup/output-path-calibration-audit.json.
This establishes an argument-length difference, not its effect on timing or
the cause of the seventh calibrated failure. Any follow-up methodology study
must retain the original observations and thresholds and declare its protocol
before collecting new data.


## Baseline calibration study and ninth lookup candidate

The predeclared baseline-only calibration-path-study is terminal zero under
handle 56959. All 512 observations (64 paired blocks) have successful status,
equal objects and four unscaled counters. Median cycle ratios are 0.999773
for unequal output-name lengths and 1.000792 for equal lengths. The paired
unequal-minus-equal median is -0.001296, with a deterministic block-bootstrap
95% interval [-0.003919, 0.001095], which includes zero. This study therefore
does not demonstrate an output-path timing bias and supplies no basis to waive
the seventh candidate's failed calibrated gate. Protocol, every raw counter,
commands, hashes and analysis are retained in calibration-path-study/; remote
objects remain in /tmp/cppgm-v4-audit-review-20261002-calibration-path-study/.
The original thresholds, calibration formula and prior results are unchanged.

The ninth candidate moves the ordinary-entry presence guard before the
injected-class-name test. Type-only and namespace-only entries cannot hide a
type and need not inspect constructor identity. Ordinary hiders and canonical
constructor aliases retain exactly the seventh behavior; the full namespace
ambiguity and restricted elaborated lookup rules remain. No record field,
helper, cache, allocation or scan is added. This source rationale requires
measurement and is not a performance claim.

Its immutable image SHA is
5bdef69d74b97f1d4511bf681da6dd8496887e2d68077e0e3bd3f988769b111a;
its six-file patch SHA is
f4e4f339a286cf5cb2a805e00608a26947b155eac4781446b9f920a7f58a575e.
All 184 focused outcomes pass with exact status checks, including forty agreed
boundaries (also passed freshly by both hosts), eighteen explicit-template
prefix observations and ten member-name/planned controls. The twelve required
local groups run under handle 22878 in validation-ninth/. The unchanged
4032-observation Alpha protocol is prepared in perf-ninth/; all 92 remote
input hashes have been verified. An initial preparation failed safely because
the older source directory's compiler-a is a different baseline. Copy the two
already approved immutable baseline images from the seventh directory, verify
their expected fd842d2c hashes and all inputs, then proceed. No measurement
started with the wrong image. Measurement awaits strict correctness; no source
commit or required control promotion precedes full approval.

## Standard noreturn variable appertainment review resolved

N3485 7.6.1/4 explicitly makes an attribute on an unsupported entity ill-formed;
7.6.3/1 permits noreturn on the declarator-id in a function declaration, and
7/3 identifies the entity to which a leading declaration attribute appertains.
Thus GCC's warning is a diagnostic of the invalid program, not evidence that
a variable is a permitted target. Both [[noreturn]] int value; and
int value [[noreturn]]; silently pass the approved entry at O0/O2; Clang
rejects and GCC warns at both optimization levels. Ordinary function and
unknown scoped variable-attribute positives pass all three. Four frozen inputs,
commands, diagnostics, source hashes and all 24 observations are retained in
/tmp/cppgm-v4-audit-review/noreturn-variable-appertainment/observations.json.
ATTR-NORETURN therefore retains the approved argument fix but has a confirmed
remaining appertainment fix. This changes the remaining inventory from 37/8
to 38 compiler families / seven unresolved reviews without introducing a new
feature. The fix belongs to PA29/500; one independent variable negative will
suffice, with existing valid function coverage and scratch spelling boundaries.
No compiler change or new required fixture is made by this contract review.


The ninth strict report is terminal zero, passing 6109/6109 with exactly one
success line; PA6/7/11/12/18 and debug-info are also terminal zero. Variants,
self-host through PA5 and required audits continue under local handle 22878.
The live image matches its immutable SHA. Alpha handle 88708 now measures
that frozen image against the unchanged approved baseline, with all 92 input
hashes verified, in
/tmp/cppgm-v4-audit-review-20261002-namespace-mixed-lookup-ninth/.
All workloads, A/A calibration, output-equality checks, counter-scaling
requirements and thresholds are unchanged. This is a running measurement,
not a performance result; both default promotions remain held.


All twelve ninth-candidate local validation groups are terminal zero under
handle 22878: PA6/7/11/12/18, strict report, debug-info, backend variants,
self-host through PA5, all nine architecture checks, file audit and placement.
The strict report still contains exactly one success line. Placement scans
3229 inputs with zero early placement, required-review or local-hygiene findings;
its informational late candidates and EH review inventory are not reported as
new cleanups or as zero. The live compiler and source patch exactly match the
immutable ninth hashes. All results remain in validation-ninth/.

Alpha's ninth broad phase is terminal zero and the focused phase remains live
under handle 88708. Keep polling this same process; the final independent
scaling, equality and unchanged gates require all 4032 observations. No source
commit, test promotion or performance approval is made from a partial run.


## Virtual current-class exception timing review resolved

The course already adopts the C++11 defect correction
[CWG 1330](https://cplusplus.github.io/CWG/issues/1330.html) for member exception
specifications. Its October 2012 resolution makes such specifications a
complete-class context and retains separate, demand-driven specification
instantiation. Comparison with an override makes a specification needed, but
the resolution does not require comparison before the class has completed.
There is no virtual-function exception to the complete-class rule. Therefore
our existing completion followed by comparison is a supported reading; eager
host rejection alone does not establish a required compiler correction.

[CWG 2510](https://cplusplus.github.io/CWG/issues/2510.html), a later NAD
interpretation concerning friend declarations, explicitly rejects the notion
that declaration matching must defeat complete-class specification parsing.
Its example differs from these virtual template controls; use it as supporting
interpretation, not a new C++11 language feature or an adopted extra oracle.
The conclusion about permitting deferred override comparison is an inference
from the already adopted CWG 1330 rules.

Fresh strict-C++11 O0/O2 observations preserve the original three differences:
ours accepts current-class sizeof specifications on restricted/unrestricted
virtual overrides and the explicit overriding destructor, while Clang/GCC
reject. Ordinary template and non-template sizeof specifications pass all three.
Both an outside-class specification on a forward declaration and an array
member bound using its incomplete current class reject in all three. Seven
frozen inputs, SHA values, commands, diagnostics and all 42 observations are
retained in exception-spec-timing/contract-review/observations.json. This
supports the contextual distinction without silently converting general
incomplete types to complete ones. EH-SPEC-TIMING is reviewed with no required
source change, fixture or reference correction; retain every host observation.
The remaining inventory is 38 compiler families and six unresolved reviews.


The ninth Alpha run is terminal zero under handle 88708. All 92 input hashes
agree and all 4032 observations have successful status and matching objects.
Independent verification confirms four unscaled counters in every raw CSV.
Instruction/RSS limits pass (maximum instruction ratio 1.000339, RSS 1.0).
Seven focused cycle workloads pass both limits; virtual declarations fail at
raw 1.008045 and calibrated 1.008623 against 1.005. All measurements remain
in perf-ninth-results/ and the declared Alpha directory. The focused method
and thresholds remain unchanged. The full fix and its two promotions remain
held; do not retry these same bytes merely to seek a passing sample.

The next source hypothesis dispatches DirectLookup once by its typed LookupKind
instead of repeating the namespace/type/ordinary/template kind filters.
Namespace-only and template-only queries return their own result; ordinary
hiders and canonical constructor aliases retain the exact ninth behavior;
type, carrier and unhiding ordinary queries share type-identity publication.
All seven kind values remain unchanged. This removes repeated kind predicates
without adding a record field, cache, helper, scan, allocation or source reparse.
It requires the complete correctness suite and unchanged performance protocol;
it is not performance approval or a narrower ambiguity fix.


The tenth host build is terminal zero under handle 41540. Its immutable image
SHA is 4e02cd02bc0d92249aada07413bf8e7a551e3b029636c1beedafb29cd493c3bf;
its six-file patch SHA is
f46a74bf4616018a1fd83ab46319496a9a95a957aa251b04e7510a9d135cd5e7.
DirectLookup's released symbol is 675 bytes, with a 19-byte cold stub and no
constprop clone in the symbol table. This is evidence of a code change, not
performance approval. The dispatch keeps every existing kind value and result
field behavior, including the typed empty result for an invalid internal kind.

All 184 focused outcomes pass with exact status checks; both hosts again pass
the forty agreed boundary observations. The three elaborated-template host
differences remain unchanged scratch evidence. All twelve required local
groups now run under handle 23278 in validation-tenth/. The unchanged Alpha
protocol is prepared in perf-tenth/ and the new remote directory
/tmp/cppgm-v4-audit-review-20261002-namespace-mixed-lookup-tenth/.
Preparation handle 73528 is terminal zero; all 92 frozen input hashes agree.
The prior ninth directory supplies the already verified baseline images and
inputs by read-only copy; the tenth candidate is separately uploaded and
verified. Measurement awaits strict correctness. No gate, fixture, required
reference, ABI encoder or student-export content is changed by this attempt.


### Completed mixed namespace lookup — updated workflow, 2026-10-03

The preserved six-file patch is now qualified; no additional exploratory
fixtures were introduced. Its immutable image remains
`4e02cd02bc0d92249aada07413bf8e7a551e3b029636c1beedafb29cd493c3bf`.
Reuse the five earlier affected-suite results for identical source bytes; rerun
PA6/PA7 after their two control promotions, then strict/debug/self-host/audits
once on the final patch. All final groups pass, with zero placement/review/hygiene
findings; the file audit retains its existing 37 warnings. Detailed command/status
records are in `namespace-mixed-lookup/validation-final/validation.json`; both
performance studies and the independent review are in
`namespace-mixed-lookup/updated-performance-results/`. Cycles were confirmed and
reported separately from instruction/memory qualification. Remaining count: 37
compiler families and six reviews. Base-alias convergence is next; final global
fixture minimization and combined student export remain pending.


### Completed same-type base-alias lookup — 2026-10-03

The merge now applies designated-type equality to class lookup as well as
namespace lookup, with ordinary identities, distinct template markers and access
representatives preserved. The existing PA30 fixture is corrected and moved to
PA22 without changing its input; no duplicate source remains. All eight final
validation groups pass; strict remains 6111/6111 in one line and placement has
zero early/review/hygiene findings. The 192-observation screen passes instruction
and RSS gates with equal objects and independently verified unscaled counters;
no timing confirmation is indicated. Image
`612e1d10f669773ab111885729240ef9347454cd6ddad34b1fc8665e64daebb1`,
source patch, commands and observations are retained in `base-alias-convergence/`.
Remaining: 36 compiler families and six reviews; final fixture minimization and
combined export remain pending.


### Completed standard noreturn variable targets — 2026-10-03

The three-file correction retains typed attribute provenance and diagnoses
non-function targets without rejecting unknown scoped attributes. One required
negative is added; an existing positive fixture supplies the unknown-attribute
boundary. Ours and Clang pass 28 focused outcomes each; GCC warning/acceptance
differences remain explicit. All eight final validation groups pass, including
strict 6112/6112 in one line and zero placement/review/hygiene findings. The
192-observation screen and 96-observation focused confirmation pass instruction
and RSS gates, output equality and independent unscaled-counter verification.
The small confirmed namespace cycle cost is recorded without claiming timing
neutrality. Qualified image, source patch and raw results are retained in
`noreturn-variable-appertainment/`; image SHA
`760b15be4963c77c48b44bbcf7bd576ae4487b156528689ef1e39f73182e3b9a`.
Remaining: 35 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.


### Completed static-assert message validation — 2026-10-03

The existing parser grammar and PA5 rendering remain unchanged. PA15 semantics
consume the retained message token range and ordinary-string category, rejecting
integer, character and user-defined messages. Two existing controls become
required negatives; no source fixture is added. All 49 focused outcomes and
eight required final qualification groups pass; strict is 6114/6114 in one line.
The final 288 counter observations independently pass instruction/RSS and equal
object checks with every counter unscaled. Virtual-class calibrated cycles
increase 3.49% (focused 95% bootstrap interval 3.08–4.15%); this is a timing
limitation, not a claim of neutrality. The short alternating 72-compile profile
finds no actionable new hotspot. Namespace/template timing signals from the
first candidate disappear with the guarded single-range classification.
Qualification follows the revised instruction/memory policy; both candidates'
evidence is retained in `static-assert-message/`. Final image SHA:
`b1e20ba434feb50af3b96240228117ea268b86bfa89b3bad02eae7306df808de`.
Remaining: 34 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.


### Completed constant member-pointer boolean conversion — 2026-10-03

One missing predicate routes member pointers through the existing typed constant
conversion. Four assertions extend the existing PA22 runtime fixture; the
superseded opt-in control is removed. All 48 focused outcomes and eight required
final validation groups pass; strict remains 6114/6114 in one line. The final
240-observation screen covers five frozen workloads, including accepted
integral, pointer and class contextual conversions, and independently passes
instruction/RSS, equal-output and unscaled-counter checks. Maximum instruction
ratio is 1.000666, RSS ratio 1.0; no focused timing confirmation is indicated.
The original shared-classifier candidate's 336 observations and confirmed 2.69%
contextual-conversion cycle increase remain archived. The final predicate avoids
the additional call, with no change to constant-folding representation. Image:
`d73a30a167cbe28d1208ce6996bf4d7a166bba334cfb0324e42076ea804ad9aa`.
All raw inputs, source patches, commands and qualification results are retained
in `constant-member-pointer-bool/`. Remaining: 33 compiler families and six
reviews. Final fixture minimization and combined export remain pending.


### Completed constant bit-field width conversion — 2026-10-03

Constant object elements now apply the recorded stored field width after the
existing declared-type conversion. Reuse the wide scalar normalizer for narrow
fields; full-width and bool values retain their previous conversion. N3485 9.6
provides the field-width and bool-preservation boundaries; PA11's existing
width-masked storage and Clang/GCC agree on the targeted unsigned truncation.
Three assertions extend the existing PA16 aggregate fixture; the superseded
opt-in control is removed. No source fixture or record field is added. All
42 focused outcomes and eight required qualification groups pass; affected
876/876, strict 6114/6114 in one line, placement/review/hygiene zero. The
240-observation performance screen independently passes instruction/RSS, equal
objects and unscaled counters. It includes the affected constant-field path and
ordinary aggregates; maximum instruction ratio is 1.003711, RSS 1.0, with no
timing confirmation indicated. Image:
`99b837c84360a172f42d1fd50a55ca5ba2195d2c43b3fcbcec7fb78707eab7d4`.
Source, frozen inputs, commands and raw qualification results remain in
`constant-bitfield-width/`. Remaining: 32 compiler families and six reviews.
Final fixture minimization and combined student-export validation remain pending.


### Completed friend-tag visibility and nested introduction — 2026-10-03

Nested forward declarations introduce into their own scope. New friend class
identities remain hidden from ordinary/qualified lookup until a matching
namespace declaration publishes them; a declaration-only lookup keeps that
identity stable, and unqualified friend lookup stops at the innermost namespace.
N3485 3.4.4/2 and 7.3.1.2/3 supply the rules. One packed visibility bit preserves
BindingRecord/EntityRecord sizes at 136/208 bytes. Two unchanged controls become
required PA11/100 and /200 negatives; no source fixture is added. All 72 focused
outcomes and eight required qualification groups pass: affected 1271/1271,
strict 6116/6116 in one line, placement/review/hygiene zero. The 240-observation
screen plus 96-observation timing confirmation independently pass instruction,
RSS, equal-object and unscaled-counter checks. Maximum instruction ratio is
1.000235, RSS 1.0; the initial virtual-class timing signal does not repeat in
confirmation (calibrated ratio 1.002048). Image:
`7d4aa5ecce4eff12fe4452edd2804cc555889025d1736de5b31dfa4bcb43f848`.
Source, fixed inputs, commands and raw results remain in `tag-introduction/`.
Remaining: 31 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.

### Completed external global variable symbols — 2026-10-03

ABI-GLOBAL is fixed in the existing typed variable-name encoder: an unscoped
ordinary target returns its source name; namespace/member encodings, explicit
internal-linkage ABI facts and expression entity references retain their
existing encodings. Fresh Clang object checks precede the change. Thirty focused
checks pass, including mixed links in both directions at O0/O2. Extend the
existing PA9 namespace-variable case, correct the two existing PA27 and two
PA31 relocation predicates, and remove the redundant opt-in control. No new
fixture files are added. The 420 regenerated LowIR references (one debug)
were independently verified to change only unscoped object-name metadata;
instructions, linkage and other metadata remain identical.

Affected PA9/PA27 passes 270/270; strict passes 6116/6116 with one output line.
Debug information, self-host through PA5, all nine architecture audits, file
audit and placement pass. Final image SHA-256:
`52c67e9f93bdf6a5c3aeda17bd0c2291296b900e2971081df6affe7a05edf522`.
Alpha verifies 240 screen observations and 192 focused confirmation observations,
including all raw unscaled counters, RSS, zero statuses and equal objects.
Maximum screen instruction ratio is 1.000489 and RSS is 1.000. Confirmation
cycle ratios are 1.008993 for copy templates (95% bootstrap interval
1.003011–1.012519) and 1.005528 for virtual overrides (1.000306–1.011863).
These small measured timing costs are recorded; instruction/memory gates pass
and neither confirmation exceeds the agreed 1.01 trigger. Evidence and
regeneration logs are in `/tmp/cppgm-v4-audit-review/global-variable-symbol/`.
Remaining: 30 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.

### Completed initializer-list constructor ownership — 2026-10-03

NOEXCEPT-LIST is a stale entity reference in special-member declaration
analysis, shared by ordinary calls and noexcept operands. Parameter template
instantiation can move the entity vector before user-provided constructor facts
are published. Reacquire the class record by its existing ID after parameter
analysis. This preserves C++11 aggregate classification (N3485 8.5.1/1) without
new records, allocations or parsing. The extended existing PA21 private
initializer-list argument fixture fails on the committed preceding compiler,
passes strict Clang/GCC C++11, and tests class-list conversion in an ordinary
call, a nonthrowing noexcept operand and a throwing list-element operand.
Remove the duplicate opt-in control; no new fixture file is added.

All 42 focused checks and 1410 affected-suite checks pass. Strict remains
6116/6116 in one line. Debug information, self-host through PA5, nine
architecture audits, file audit and placement pass. Final image SHA-256:
`02cb8a5691984e1d446bb66fdad7864380ec91ddf8007c463d909fa606c4f1ce`.
Alpha's 240 observations independently verify every raw unscaled counter, RSS,
zero status and equal object hash. Maximum instruction ratio is 1.000002 and
RSS is 1.000; all calibrated cycle medians remain below 1.01. No timing
confirmation is triggered. Evidence is in
`/tmp/cppgm-v4-audit-review/noexcept-list-argument/`.
Remaining: 29 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.

### Completed bit-field lvalue-reference binding — 2026-10-03

REF-BITFIELD consumes the existing binding bit-field fact after overload
selection. Reject mutable/volatile lvalue references and route non-volatile
const bindings through the existing scalar copy/temporary conversion, without
assigning an address to the original field (N3485 8.5.3/5 and 9.6/3).
Overload ranks remain unchanged: choosing a mutable-reference overload for a
bit-field is diagnosed rather than falling back to a const-reference overload.
Extend the existing aggregate fixture to check copied values in local binding
and function arguments. Two independent negatives replace two opt-in controls;
remove the duplicate positive control. No separate positive fixture is added.

All 42 final focused checks pass against strict C++11 Clang/GCC. The affected
suite passes 1687/1687; final strict passes 6118/6118 with one output line.
Debug information, self-host through PA5, nine architecture audits, file audit
and placement pass. The first image passed correctness/performance but exceeded
analyzer.cpp's 3000-line cap. Move the unchanged cold source-location diagnostic
code into existing diagnostic_names.cpp and update its live method owner;
analyzer.cpp is now 2977 lines. Final image SHA-256:
`e9ae960a938e6aea771f236d7e3b120f37eb082aea0a1b1686307b2d886597e0`.
Alpha independently verifies 240 observations for each image, including all
raw unscaled counters, RSS, zero statuses and equal object hashes. Final maximum
instruction ratio is 1.000091 and RSS is 1.000219. All calibrated cycle medians
remain below 1.01 (largest 1.008705); no timing confirmation is triggered.
Both images, checks and raw observations remain in
`/tmp/cppgm-v4-audit-review/bitfield-reference/`.
Remaining: 28 compiler families and six reviews. Final fixture minimization and
combined student-export validation remain pending.

### Completed static constexpr reference temporary — 2026-10-03

CONST-REF-STATIC-TEMP reuses private semantic bindings and existing static
initialization actions for lifetime-extended constant temporaries (N3485
5.19 and 12.2/5). References and aliases retain the backing object's address
and constant referent facts. Extend the existing PA16 reference-alias fixture;
remove its opt-in duplicate. No fixture file is added. All 48 admitted focused
checks pass across ours/Clang/GCC at O0/O2; PA16 passes 160/160. Strict remains
6118/6118 in one line; debug-info, self-host PA5, nine architecture audits,
file audit and placement pass. Final SHA-256:
`256ecbabc005eb0c9fbda58c6a57203ca380d3595e21dd85359d631ee4150f5e`.
Alpha's 144 raw observations independently pass unscaled counter, RSS, status
and equal-output verification. Maximum instruction ratio 1.000045 and RSS
1.009711; all calibrated cycle medians below 1.01. No repeat is indicated.
Evidence: `/tmp/cppgm-v4-audit-review/static-reference-temporary/`.
Remaining: 27 compiler families and six reviews; fixture pruning and combined
export remain pending.

### Completed braced class reference binding — 2026-10-03

REF-BRACE follows N3485 8.5.4/3: bind a reference-related single element
directly, otherwise materialize a separate class object and bind to it.
Reuse the existing braced-expression cache and conversion/lifetime machinery.
The preceding compiler crashes on the original source and fails the extended
existing PA12 fixture; the patch passes both. No fixture file is added.
All 48 focused observations agree with strict C++11 Clang/GCC at O0/O2.
PA12 passes 298/298; strict stays 6118/6118 in one line. Debug-info,
self-host PA5, nine architecture audits, file audit and placement pass.
Final SHA-256: `9bc4ae3b83d9d1de7ee6b619d71a926a3f9ac3889f49622b9701d060afc71023`.
Alpha's 144 observations independently pass raw unscaled counter, RSS, zero
status and equal-output checks. Maximum instruction ratio 1.00000024, RSS
1.000; all calibrated cycle medians below 1.01. No repeat is indicated.
Evidence: `/tmp/cppgm-v4-audit-review/braced-reference-temporary/`.
Remaining: 26 compiler families and six reviews; final pruning/export pending.

### Qualified static initializer-list storage/lifetime component — 2026-10-03

INIT-LIST-STATIC now retains a private static backing array and registers its
reverse element cleanup at shutdown. Retain the backing temporary while
finishing other initializer temporaries. Reuse existing namespace/static
storage and destructor emission; the added typed field fits padding and
LocalStaticObjectAction remains 88 bytes. Extend the existing PA21 static
reference fixture with static value access; no new fixture file. Host-object
O0/O2 checks pass for the original source, required fixture, shared-owner
workload, shutdown order and failed-initialization retry. PA21 passes 208/208;
strict stays 6118/6118 in one line. Debug-info, variants, self-host PA5, all
nine architecture audits, file audit and placement pass. Image SHA-256:
`29d4b37293c23fa121f3e62b1ca5fac8575cebf6362bafe003b9b91e065336a4`.
Alpha's 192 screen and 96 focused confirmation observations independently
verify every raw unscaled counter, RSS, status and equal output. Maximum
instruction ratio 1.0000671; maximum median RSS ratio 1.000. The initial
local-static calibrated cycle ratio 1.012575 triggers one repeat; confirmation
is 1.005287 (95% interval 1.001988–1.009039). This small timing cost is retained
as evidence; instruction/memory gates pass, and no further repeat is indicated.

The component is qualified, but the family remains open pending the shared
BACKEND standalone route: the original control now fails native linking on
__cppgm_runtime_atexit instead of running with prematurely destroyed elements.
Its original control is retained. Existing C atexit object-symbol mapping is
correct (also checked with Clang); the host-object route executes successfully.
No runtime-symbol mapping or Itanium spelling is changed. Evidence is in
`/tmp/cppgm-v4-audit-review/static-initializer-list-backing/`. Remaining: 26
compiler families and six reviews; fixture pruning and combined export pending.


### Native shutdown callbacks — INIT-LIST-STATIC complete

The standalone backend now supplies the existing external C `atexit` declaration
with callback registration and startup draining. Typed declarations select the
runtime; source definitions retain ownership. Nodes are released before each
callback, so new registrations during shutdown run in the same LIFO sequence.
The entry return value survives shutdown. Host-object linkage is unchanged;
there is no Itanium spelling change.

Reuse PA24's existing startup/shutdown fixture for LIFO order, registration from
a callback and the nonzero entry result; regenerate only its reference and
size envelope. The existing PA21 fixture covers persistent value/reference
backing. Remove its redundant opt-in control. No new required fixture files.
Original, fixture, shutdown and retry programs pass 24 checks across O0/O2 and
all three link routes. A user-defined `atexit` passes six ours/Clang/GCC checks.
A discarded static-registration observer depended on each host's choice of
`atexit` versus `__cxa_atexit`; it is not an admitted language regression.

PA21/24 pass 629/629 in one serialized report. Strict 6118/6118, debug-info,
backend variants, self-host through PA5, nine architecture audits, file audit
and placement all pass. Alpha's 192 immutable ABBA/BAAB and interleaved A/A
observations have equal object hashes and independently verified unscaled
counters/RSS. Maximum median instruction ratio 1.000005481; RSS 1.000054546.
No calibrated cycle median exceeds the 1.01 confirmation threshold. These
measurements qualify compilation of the frozen workloads, not a universal
runtime timing claim. Candidate SHA256:
`65c8c3872832975831fa315c51d1f1ca80ee78f4e7b8ae73b97a29b5aabe6b42`.
Raw checks and performance evidence: `/tmp/cppgm-v4-audit-review/native-shutdown-runtime/`.
Remaining: 25 compiler families and six reviews; final fixture pruning and
combined student-export validation remain pending.


### Dynamic rethrow — source acceptance and lowering complete

N3485 15.1/8–9 bases operandless throw on the dynamically handled exception.
Remove the invalid lexical-handler check and its otherwise-unused depth field.
Existing typed throw lowering already calls the exception runtime. Extend PA21's
existing function-rethrow fixture with a called helper; regenerate only its
LowIR reference. No new required fixture files and no ABI encoder changes.

All three original programs compile and pass host-object execution at O0/O2.
The caller-handler program and rewritten fixture also pass direct/native-object
execution. A no-active-exception control calls the installed termination handler
with ours, Clang and GCC at O0/O2. Two other native routes expose the existing
BACKEND handler/cleanup gaps: same-function nested rethrow exits 134, and the
destructor trace returns 1. Preserve/reclassify the original opt-in reducer;
these observations remain open, rather than claiming complete native EH support.

PA21 208/208, strict 6118/6118, debug-info, self-host through PA5, nine architecture
audits, file audit and placement pass. No lowering/optimizer implementation
changed, so backend variants were not repeated. Alpha's 192 equal-object
observations pass instruction/RSS gates with independently verified raw counters.
A copy-template calibrated cycle median of 1.011133 triggered the single required
96-observation confirmation: 1.001314, bootstrap 95% interval 0.998849–1.014178.
The confirmation passes instruction/RSS/equality checks; no further repeats.
Candidate SHA256: `cf82417206f899edccd4bdafaf74c4e8b6ba1dedd92589ff1857b9936b549f9a`.
Evidence: `/tmp/cppgm-v4-audit-review/dynamic-rethrow/`.
Remaining: 24 compiler families and six reviews; final pruning/export pending.


### Function-template try blocks — TMPL-FTRY complete

Function-template patterns now retain typed function-try syntax alongside the
body and constructor initializer. Reuse FunctionDefinitionPart to find the
nested definition parts, and propagate the retained syntax when adopting a
later definition or upgrading/instantiating a specialization. Explicit member
and namespace specializations use the same existing definition-part helper.
The additional NodeId fills padding: FunctionTemplatePattern stays 512 bytes.
No parsing pass, ABI encoder change or new required fixture file.

Extend the existing PA21 function-try/body-cleanup fixture with a declared-then-
defined function template and a constructor template using a nested initializer.
Retire the now-passing opt-in template reducer. The original passes direct,
compiler-object and host-object routes at O0/O2, matching Clang/GCC. Explicit
namespace/member specialization and use-before-definition upgrade controls pass
all three compilers at O0/O2. The extended fixture passes host linking and both
host compilers; its unchanged standalone 134 failure remains in BACKEND.

PA21 208/208, strict 6118/6118, debug-info, self-host through PA5, nine architecture
audits, file audit and placement pass. Alpha's 192 observations retain equal
object hashes, independently verified unscaled counters and RSS. Maximum median
instruction ratio 1.000236909 (+0.0237%); RSS ratio 1.0. No calibrated cycle median
exceeds the 1.01 confirmation threshold. Compiler candidate SHA256:
`40caf0f7fb30c0ba728ca600c9d625bb440052ae903ae5b3959f85fe016d5df1`.
Evidence: `/tmp/cppgm-v4-audit-review/template-function-try/`.
Remaining: 23 compiler families and six reviews; final fixture pruning and
combined student-export validation remain pending.


### Conditional base-reference temporary — REF-BASE-COND complete

Materialize a derived prvalue before its static base-reference projection while
preserving source cv and the required binding category. This also rejects an
explicit mutable lvalue-base cast of a derived xvalue. Reuse the existing PA21
conditional-reference lifetime fixture with a nonzero base offset and complete
derived destruction. One PA12 negative covers the independent explicit-cast
rejection boundary. Retire the now-passing opt-in reducer.

The strengthened fixture exposed base lifecycle entries cached before their
source definition. Publish the later typed body, initializer, handler and
linkage metadata before demanding these entries. Fresh Clang checks confirm
existing C1/C2 and D1/D2 spellings and weak inline linkage; no ABI encoder change.
Eighteen other regenerated references add only the previously absent base
destructor body, with every existing function byte-identical.

Original and lifetime fixture pass direct, compiler-object and host-object
routes at O0/O2. Cast boundaries and later inline definitions match Clang/GCC.
PA12 299/299, PA21 208/208, strict 6119/6119, debug-info, self-host through PA5,
nine architecture audits, file audit and placement pass. Alpha's 192 raw
observations independently verify equal object hashes, unscaled counters, RSS
and zero statuses. Maximum median instruction ratio 1.000000430; RSS 1.0. No
calibrated cycle median exceeds the 1.01 confirmation threshold. Final compiler
SHA256: `0b5da77d53228caf12a72fc2e95756c693beeca816aa7cc3a839ed278dbd1459`.
Evidence: `/tmp/cppgm-v4-audit-review/conditional-base-reference/`.
Remaining: 22 compiler families and six reviews; final fixture pruning and
combined student-export validation remain pending.


### Conversion-template target patterns — MANGLE-CONV complete

Fresh Clang and GCC objects reproduce the two retained target-pattern names:
`_ZN1XcvT_IKiEEv` and `_ZN1XcvT_IRiEEv`. Build both ordinary conversion
terminals and member-address facts from the existing template ABI recipe
instead of its substituted concrete target. The encoder is unchanged.

Explicit conversion specializations now select their template specialization
through existing conversion deduction and retain inline/constexpr facts.
Qualified conversion addresses retain their target-type syntax during parsing
and consume it through existing target deduction. No source reparse, new
persistent fields or fixture files. Extend the existing PA18 direct-conversion
fixture with an explicit specialization. Extend the PA22 member-function
pointer-target fixture with direct and NTTP conversion addresses and invocation.
Placement correctly assigns member-pointer invocation to PA22; keep that check
out of PA18. Retire the now-passing opt-in conversion reducer and symbol sidecar.

Fourteen other regenerated references change only conversion object symbol
metadata. Clang omits one unused constexpr specialization that we emit. The
existing dependent non-type parameter annotation matches GCC and Clang's
ABI-17 compatibility mode; current Clang's default uses a newer annotation.
This does not establish another C++ language bug or require another fixture.
Every O0 conversion/address symbol in the focused direct, specialization,
SFINAE and owning-fixture checks agrees with Clang/GCC. O2 execution passes,
with unused-symbol elimination allowed. Original and final owning fixtures
pass direct, compiler-object and host-object routes at O0/O2.

PA18 427/427, PA19 428/428 and PA22 102/102 pass. Strict 6119/6119, debug-info,
backend variants, self-host through PA5, nine architecture audits, file audit
and final placement pass. Alpha's final 240 observations cover four existing
frozen workloads plus non-template conversions, with pinned alternating paired
blocks and interleaved A/A calibration. Independently verify every unscaled
counter, RSS, status and equal object hash. Maximum median instruction ratio
1.000455783 (+0.0456%); RSS ratios 1.0. No calibrated cycle median exceeds the
1.01 confirmation threshold. Final compiler SHA256:
`c8ba5e3ffe148208472151fcea20f470793c2596db071a7cbd5a48edb4850feb`.
Evidence: `/tmp/cppgm-v4-audit-review/conversion-template-target/`, including
`owning-validation.json` for the final fixture placement qualification.
Remaining: 21 compiler families and six reviews; final fixture pruning and
combined student-export validation remain pending.
