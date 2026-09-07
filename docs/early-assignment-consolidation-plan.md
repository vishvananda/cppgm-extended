# Early assignment consolidation plan

Status: implemented; final validation accepted on 2026-09-07. Local functional,
self-host, inception and export checks pass. The user accepted the native
wall-time ABBA results following the O2/O3 measurement protocol; see
[the performance validation record](early-assignment-consolidation-abba-validation.md)
for results, uncertainty and the acceptance decision.
See [the implementation tracker](early-assignment-consolidation-tracker.md).
Rewritten: 2026-09-06; execution strategy revised 2026-09-07. Analysis baseline: `70a634a73`.

Assignment numbers in the analysis and content stages below are the
**original baseline numbers**. Section 9 gives the applied final numbering;
original PA13 is now PA8 and remains the LowIR introduction. See the
[migration record](assignment-numbering-migration-2026-09.md). This document
supersedes the earlier draft. It does not reopen the decisions in
[assignment-restructure-plan.md](assignment-restructure-plan.md).

## 1. Objective and decisions

Help a student reach a working, self-hosting compiler with less repeated work.
Each required assignment should introduce useful compiler machinery, extend
machinery already built, or establish an essential testable boundary. Keep
assignments coherent and manageable without preserving a separate assignment
for every intermediate artifact.

The consolidated required course has **34 assignments**, down from 39:

| Original assignment | Decision | Result |
| --- | --- | --- |
| PA1, PA2, PA3 | Keep their content | Tokenization, post-token conversion, preprocessing-expression evaluation |
| PA4 and PA5 | Combine | One complete preprocessor assignment, delivering `preproc` |
| PA6 and PA10 | Combine their teaching and useful coverage | One parser/AST assignment, delivering `cppgm++ --emit-ast` |
| PA7 | Absorb into PA11 | Build scopes, lookup, declarations, and canonical types once |
| PA8 | Retire its mock-image assignment; distribute useful assertions | Semantics, initialized storage, and linkage tested at the stages that own them |
| PA9 | Omit assembler construction | Teach instruction encoding and executable layout at the native-backend assignment |
| PA13 | Keep the LowIR introduction; remove its CY86 dependency | Establish reusable LowIR machinery and a small required set of behavioral exercises using the supplied native backend |
| PA14 onward | Preserve the existing arc, with the intake described here | Later C++ lowering builds on PA13; no new template, ABI, native, or optimization assignment split |

PA13 remains the assignment that introduces LowIR, before students implement
C++-to-LowIR lowering. Removing its CY86 translation step does not remove that
lesson. The supplied `lowir2native-ref` executes student-produced LowIR for a
small required set of behavioral exercises. Later lowering assignments reuse
that execution boundary with LowIR emitted by the student's C++ compiler.

Omit PA9 and its assembler exercise. Teach instruction encoding, address fixups
and executable layout in the required native-backend assignment. Section 5
separates that change from preserving PA13's LowIR teaching and reusable work.

The consolidated preprocessor and parser are each one assignment. Their test
groups provide an implementation order and quick feedback, not additional
graded assignments, separate binaries, or extra preservation contracts.

## 2. What the codebase establishes

### 2.1 Implementation reuse is evidence, not a workload estimate

At the analysis baseline, `dev/frontend_source_sets.mk` showed that the final
compiler shared the preprocessing implementation with the early tools. It
also identified three independent early implementations, now retired:

- `recognition/recognizer`, used by `recog`;
- `namespace_semantics/`, used by `nsdecl`;
- `namespace_initialization/`, used by `nsinit`.

The compiler instead consumes `syntax/` and `semantic/`. Retiring those three
early tools can remove about 7,209 lines of `.cpp` implementation at this
baseline, plus their associated headers and adapters. That is a maintainer
code-removal measurement, not a promise that every student saves 7,209 lines.
The existing handouts encourage students to reuse earlier parsers and type
helpers even though the reference implementation does not do so.

The old draft's claim that PA5 requires only 47 lines was incorrect. Those
are the lines of `preprocess/tool_support.cpp`, the only additional source
file in its source set. PA5's substantial directive machinery already lives
in `preprocess/macros/macro_processor.cpp`, which PA4 also links. Linking a
shared file first does not make all of its later functionality PA4 work.
Line counts also omit drivers, headers, and inline implementation.

The reason to combine PA4/PA5 is their shared pipeline and cumulative lesson.
The reason to combine PA6/PA10 and PA7/PA11 is to teach one evolving parser and
one evolving semantic representation. PA8's disposable image format is the
principal work removed there; real initialization and linkage remain taught.

### 2.2 The proposed assignment sizes are reasonable

These are filesystem inventories of test roots, not execution counts or
estimates of difficulty. A numbered multi-file case counts once. Combined
figures are upper bounds before screening and deduplication.

| Surface | Existing case roots | Consolidated intake ceiling |
| --- | ---: | ---: |
| PA4 + PA5 | 75 + 72 | 147 |
| PA6 + PA10 | 48 + 167 | 215 |
| PA7 + PA11 | 43 + 78 | 121 before the relevant PA8 intake |
| PA8 | 67 multi-file case roots | Distributed; not added wholesale to PA11 |
| PA12 | 185 | Receives only missing assertions within its semantic slice |
| PA19 | 314 | Comparison with an existing template assignment |
| PA22 | 343 | Comparison with an existing template assignment |

The early combinations leave considerable room relative to the template
assignments, especially in conceptual difficulty. Do not create new numbered
assignments merely because a combined lesson has several test groups. Review
size using new concepts, dependencies on unfinished stages, scaffolding, and
retained implementation work as well as case count.

### 2.3 Migration probes reveal contract differences

The review ran each PA6 input through the existing `--emit-ast` mode and each
PA7/PA8 case through `--emit-types`, preserving PA8 translation-unit groups and
running from each original assignment directory. Outputs went to temporary
files. PA6's reference `OK`/`BAD` verdict was compared with compiler success;
PA7/PA8 used their named reference exit statuses.

| Old suite | Cases | Acceptance disagreements |
| --- | ---: | ---: |
| PA6 | 48 | 6 |
| PA7 | 43 | 1 |
| PA8 | 67 | 7 |

These are intake findings, not 14 mandatory fixes. Matching acceptance also
does not establish equivalent coverage. For example, `int x = 3` and
`int x = 4` produce different PA8 images but identical PA11 type dumps. An
initialization test loses its assertion if it is migrated only as a type dump.

A separate PA4-to-`preproc` probe found six success/failure differences caused
by the full preprocessor rejecting invalid phase-7 tokens that `macro` prints.
Section 3 gives their disposition.

Host screening with `g++ -x c++ -std=c++11 -pedantic-errors -fsyntax-only`
accepted all positive PA7 inputs and all positive PA8 translation units except
the deeply nested namespace stress case, where the host reported its nesting
limit. This is a useful screen, not a complete language or cross-TU oracle.
PA8's signed/unsigned character-array string initialization is supported by
N3485 8.5.2; its rejection by the current compiler should not be explained away
as invalid input.

### 2.4 PA13 introduces LowIR independently of its CY86 adapter

At the analysis baseline, PA13's default test target runs 102 fixed LowIR
inputs through `lowir2cy86`. Its 12 behavior cases translate LowIR to CY86,
assemble, and execute. The current handout explicitly says PA13 does not lower
C++ source. It introduces the LowIR representation, parsing, validation and
format contract that subsequent lowering and backend work use.

Keep that introduction. Replacing CY86 execution with `lowir2native-ref`
removes a dependency on PA9; it does not justify deleting PA13 or postponing
LowIR teaching until PA15. In the revised behavioral exercises, students
produce LowIR that implements a specified behavior. Merely running supplied,
fixed LowIR through the reference backend would not assess their work.

PA13 has 162 tracked `.t` roots: 102 spec, 12 behavior, 30 top-level roots
outside default spec/behavior selection, and 18 debug-info roots. Review these
when changing the adapter contract. Preserve useful early LowIR assertions at
PA13, drop CY86 text-shape expectations, and check later native/optimizer
coverage before moving or adding cases. Keep its specification, grammar and
audit anchors with the surviving lesson. Debug-info observations retain their
current coverage unless a deliberate ownership correction is needed.

## 3. One complete preprocessor assignment

Combine PA4's macro replacement and PA5's directive processing at the current
PA4 location. The deliverable is:

```sh
preproc -o <outfile> <source1> [<source2> ...]
```

It performs phases 1 through 6 and the tokenization part of phase 7, with the
current PA5 course-defined preprocessing contract. Keep the PA5 file output:
`preproc N`, one `sof <path>` section per primary source, PA2-style tokens, and
an `eof` per translation unit. Macro state is isolated between primary files;
included files participate in their including translation unit.

Teach and test the work in this order:

1. Object-like and function-like replacement, arguments, stringizing, token
   pasting, rescanning, and token-local recursion suppression.
2. Conditional inclusion using the PA3 evaluator, including macro-expanded
   expressions and skipped groups.
3. Inclusion, file search, source locations, line control, predefined macros,
   pragmas, `_Pragma`, and active errors.
4. Integration: included macro definitions, nested conditionals, source
   locations through expansion, and reset between primary source files.

Keep focused groups under the owning assignment's `tests/`; document commands
to run them using the existing `make check TEST=...` surface. A student can
pass the macro group before implementing includes. The final exit criterion
remains the cumulative through-assignment report.

### Interface and fixture migration

Remove the separate required `macro` executable. Its stdin interface does not
become a required compatibility flag. Both groups exercise the same `preproc`
file interface; regenerate the former PA4 references with the new wrappers.

The six PA4 cases requiring explicit screening are:

- `150-hash-outside`, `200-invalid-token-macro-replacement`;
- `300-directive-tokens-from-macro-argument`, `300-double-hash`;
- `300-token-paste-multiple-parameter-tokens`, `600-hash-from-macro`.

Several mix useful expansion examples with output that is not a valid phase-7
token stream. Extract valid token-paste/argument examples into positive tests.
Retire invalid-stream dump expectations. Where rejection itself usefully
tests the combined contract, use a small failure case; do not preserve every
old failure or invent an intermediate-output mode solely to keep reference
bytes. If an expansion property becomes unobservable after reduction, record
that loss and choose a valid observable example before claiming it retained.

Preserve the existing macro recursion convention deliberately. This project
does not silently replace the course's macro semantics with a host compiler's
behavior. Likewise, keep the current early include-search contract; hosted
include paths and later compatibility flags stay in their later assignments.

Describe the retained token-stream interface consumed by the parser. The textual `preproc` dump is an external observation,
not a required internal transport to parse again.

## 4. One parser, followed by one semantic foundation

### 4.1 Fold PA6 into the current PA10

The deliverable remains `cppgm++ --emit-ast`. Start directly with the
structured AST the semantic passes will consume. Rehome PA6's useful parsing
explanations: recursive descent and prediction, precedence, backtracking
discipline, declaration/expression ambiguity, contextual identifiers, and
angle-bracket versus shift-operator handling.

Use `shared/source.gram` as the single grammar source and its assignment
symlink/explorer as the student reference. PA6's smaller grammar and generated
recognizer table are retired. Do not require the compiler parser to reproduce
the old recognizer's internal token splitting, generic AST, trace formatting,
or per-file `OK`/`BAD` report.

The lesson proceeds from declarations/expressions through declarators and
statements to class/template syntax and ambiguities. These are test groups
inside one assignment. Parsing a template declaration does not require
template instantiation, deduction, or the full later semantic machinery.

Describe the name-category boundary explicitly. Retain the minimum mock-name
scaffolding already used by PA10 where it helps construct syntax before real
lookup exists. Explain how declarations and later lookup supersede it. A name
containing `T` must not impose a permanent ban on otherwise valid relational
expressions. Tests should prefer actual declarations or clear parameter
contexts when those express the intended ambiguity without artificial names.

PA6's `600-ambig-68` and `700-ambig-82` are useful teaching inputs: retain their
declaration-versus-expression and function-versus-object distinctions as AST
assertions, reducing or making contexts explicit where necessary. An `OK`
verdict alone never proved that those trees were correct.

The six observed PA6 disagreements require these actions:

| Fixture | Intake decision |
| --- | --- |
| `130-postfix-suffix` | Separate valid member/template-id syntax from mock-name-dependent cases; preserve useful postfix AST coverage |
| `250-decl-specifiers-without-type` | Retire the old permissive acceptance requirement; no need to accept invalid declarations to match `recog` |
| `250-decl` | Split its mixed constructs; retain supported declaration/namespace/template syntax and route or retire the assembly example by the shared grammar boundary |
| `400-dots` | Replace the artificial typed constructor/destructor forms with valid syntax exercising useful pack-expansion structures |
| `500-operator-template-angle-boundary` | Reduce the distinct operator-id and angle-boundary examples; establish the intended AST from the contract before changing the parser |
| `500-template-name-angle-commit-bad` | Retire the negative based solely on the mock `T1` convention; a declared value in `int x = T1 < 2;` supplies a valid relational positive |

Apply the same screening to the other 42 cases. Agreement between the old
recognizer and current parser does not make a fixture suitable. Invalid,
redundant, or obsolete syntax negatives may be dropped without replacement.
Keep a small useful rejection surface for the syntax contract; there is no
requirement to maintain the number of old negative tests.

### 4.2 Absorb PA7 into the current PA11

PA11 becomes the first semantic assignment. Its scope remains declarations,
namespace and nested scopes, lookup, bindings, canonical types, and the small
constant-expression subset needed for types. It already has most of the
relevant contract. Add missing explanations and valid assertions from PA7.

Teach type construction and declaration identity directly over the PA10 AST.
Introduce namespace reopening, aliases, using declarations/directives,
qualified/unqualified lookup, declarator-derived types, parameter adjustment,
and reference collapsing in the order the tests exercise them. Define the
canonical type spelling locally instead of referring students back to a
deleted handout. The existing type dump deliberately preserves declared
function-parameter types through a source view, while canonical signatures
apply parameter adjustment. Explain both views and test declaration/call
identity when retaining parameter normalization; a source-style type line
alone is not an assertion of canonical signature equality.

Screen the 43 PA7 cases for unique observable coverage and consolidate cases
already covered by PA11. The observed inline-namespace reopening disagreement
is a declaration-rule intake decision, not an automatic requirement to change
the parser. If retained as a diagnostic, specify it in the semantic contract.
Deep nesting fixtures belong to an explicit, reasonable resource expectation
or the maintainer regression lane; do not infer a required student algorithm
from the reference implementation's limits or cache design.

The semantic representation must survive into PA12 and lowering. Give design
guidance for stable entity/type identities and scope operations without
dictating the reference compiler's physical record layout. Ordinary lookup
stays translation-unit-local. Later program linkage associates entities
across units without merging their lexical lookup scopes.

### 4.3 PA8 contributes concepts and assertions, not an image format

PA8's `PA8` magic, fixed `fun` stubs, mock function size/alignment, offsets
measured from its image origin, and three-block serialization order disappear.
None is a required intermediate product of the new course.

Real constant values, zero initialization, object alignment, reference
identity, symbol-plus-addend addresses, storage duration, and cross-TU
linkage remain relevant. Teach each at its production boundary. PA8's image
ordering is not C++ dynamic-initialization order; do not migrate it under that
label or invent a runtime ordering requirement from its serialization.

| Assertion family | First appropriate current surface | Required observation |
| --- | --- | --- |
| Namespace/type/declaration lookup and type formation | PA11 | Resolved binding/type or an explicitly required declaration failure |
| Integral constants used in array bounds or `static_assert` | PA11/PA12, according to the expression subset | A bound/value-dependent result, not just the declared type |
| Ordinary scalar/pointer conversions, reference binding, overload/redeclaration rules | PA12 where supported | Resolved conversion/binding or targeted semantic failure |
| Procedural zero/constant initialization, ordinary addresses and function pointers | PA15 | LowIR initializer, referenced entity/addend, or a behavioral control using the supplied native backend |
| Strings, aggregate storage, materialized temporaries, and richer object initialization | PA16 or the earliest existing owner | LowIR data and identity, with value/aliasing checks where needed |
| Full pointer/reference/floating constant evaluation | PA21 unless an earlier slice already owns it | Required compile-time value or rejection |
| Cross-TU definitions, internal/external identity, ODR association, relocation | PA30 | Source-link and separate compile/link behavior |
| Per-thread identity and host TLS interaction | PA32 for host runtime interaction; earlier owners for supported storage facts | Actual per-thread behavior when that is the assertion |

This table routes observations, not whole files. Split a fixture that mixes
lookup, conversion, and linkage. A declaration test may move early while a
separate execution test preserves its initialized-value assertion later.
Use existing adequate tests rather than cloning them. Section 10 provides a
case-by-case intake route for all 67 PA8 roots.

When a source-level assertion needs execution before the native assignment,
the harness composes student lowering with the supplied backend:

```text
source -> student cppgm++ --emit-lowir -> supplied lowir2native-ref -> execution
```

Keep the source fixture at the assignment owning its latest language feature.
The helper pipeline must use the student's compiler to generate LowIR and
pass that exact output to the supplied native backend. Use runnable inputs in
the owning language subset; do not make early checks require student-written
native code generation, host interoperability or unavailable runtime support.

Do not expand PA11/PA12 to full constant evaluation or program linking to
accommodate old tests. Their `--emit-*` modes currently run mature compiler
machinery; acceptance there does not prove a student at that milestone has
the necessary implementation.

## 5. Keep PA13's LowIR introduction; remove PA9's assembler

PA13 remains a required assignment. It introduces the LowIR model and text
format, including values and types, globals, functions, blocks, control flow,
memory and calls. Keep its reusable representation, parsing/validation and
serialization work here so later assignments extend it. Scope that work to
the introductory contract; the full later metadata vocabulary does not all
become an early requirement merely because it appears in `lowir.md`.

PA15 subsequently implements C++-to-LowIR lowering using the representation
introduced in PA13. PA29 implements native instruction selection, allocation,
encoding, fixups and executable layout, reusing the LowIR input machinery.
Neither lesson assumes a student-written CY86 assembler or translator.

### PA13's small required behavioral portion

The deliverable is `lowir`, with two required modes:

```text
lowir -o output.lowir input1.lowir [input2.lowir ...]
lowir --exercise sum|swap|call -o output.lowir
```

The first mode reads a complete LowIR program, validates the introductory
contract, and writes its model. Multiple files contribute to that one program;
this is not a linker that resolves duplicate declarations and definitions.
The second mode constructs a small callable implementation using the same
representation and writer. No C++ frontend lowering is required:

```text
behavioral exercise
  -> student-produced LowIR
  -> supplied lowir2native-ref -O0
  -> executable stdout and exit status
```

Students may produce any valid LowIR within PA13's documented subset that
implements the required behavior. Different blocks, temporary names and
instruction sequences are acceptable. The behavioral grader checks backend
success and program outcomes; it does not require reference LowIR or MIR.

The required construction portion has three exercises:

| Exercise | Student-defined function | Required observation |
| --- | --- | --- |
| `sum` | `@sum_to(%n:i64) -> i64` | Sum 1 through n, for n in 0..1,000,000; zero and nonzero cases |
| `swap` | `@swap_values(%left:ptr, %right:ptr) -> void` | Swap two i64 values, including when both pointers name the same storage |
| `call` | `@call_twice(%fn:ptr, %x:i64) -> i64` | Call the supplied callback twice, passing the first result into the second call; retain both calls' effects |

Each fixture supplies a LowIR caller, and a small `.exercise` sidecar selects
the student's construction routine. The native helper consumes both the
student-generated unit and the caller. The caller does not redeclare a
function the student's unit defines. Callback parameters make the call
exercise independent of cross-file symbol resolution. The handout specifies
the callable contracts and permissible inputs; different algorithms are valid.

The test intake retains 106 reader/writer cases: the existing 102 default
spec roots plus four useful phi cases formerly outside default discovery.
Two dormant positive metadata observations extend an existing metadata case.
The 24 remaining dormant diagnostics and 12 fixed-input CY86 behavioral
cases are retired or covered at their actual later owners, as recorded in
the inventory. All 18 debug-info roots remain. The default suite is therefore
109 cases, including the three new construction exercises. No existing
native calculator or backend suite is copied into this assignment.

The behavioral portion complements PA13's reusable LowIR implementation.
Keep independent model, format and validation checks: runtime success does
not prove that a student implemented a reader or preserved required data.
The roundtrip comparator uses supplied `lowir-ref` to normalize both outputs
and then compares the full preserved model, including unused declarations.
It accepts equivalent numeric spellings and formatting, without applying
later C++-lowering relaxations that can discard model data. The maintainer
byte-exact switch bypasses this normalization. None of those reference-dump
requirements applies to the behavioral portion.

Use the outcome-only `program_t` comparator for construction exercises.
`mir_behavior_t` still requires MIR files and can evaluate MIR predicates;
it remains a later-suite comparator. Keep generated artifacts for debugging
without making their shape or existence as a reference sidecar part of the
construction oracle.

Prove the revised harness accepts two different valid LowIR implementations
of one behavior and rejects malformed LowIR, an incorrect program result and
stale executable output after a failed build. Ordinary and batch paths must
apply the same requirements. Normal grading consumes the student's LowIR;
reference regeneration uses the course solution's exercise implementations
and the supplied backend to generate expected outcomes.

### Later source-lowering feedback

Once C++ lowering is taught, a source-level behavioral check can use:

```text
C++ fixture
  -> student cppgm++ --emit-lowir -O0
  -> student-generated LowIR
  -> supplied lowir2native-ref -O0
  -> executable stdout and exit status
```

This is a later use of the same execution helper, not a prerequisite for PA13
or a reason to move PA13's behavioral portion to PA15. Keep source fixtures
at their owning language milestone. The earlier proposal to move three PA15
fixtures into a new behavioral lane is withdrawn from this plan; those cases
remain at their current owners. Add or migrate a source behavior check only
when an adopted assertion requires it, after reviewing existing coverage.
This change does not redesign all PA15–PA28 grading.

### Reuse the native reference bundle

The execution helper defaults to `../dev/lowir2native-ref`, using the existing
downloaded reference bundle. `run_reference_binary.sh` requires a `-ref` name.
Do not build the unfinished student `dev/lowir2native.cpp` as an early test
dependency. Maintainer checks may explicitly select the built native backend.

Export the helper and its dependencies, document its permitted use of the
supplied backend, and exercise the actual downloaded wrapper. Students may
inspect reference LowIR while learning. Copying reference output into a
submission bypasses the exercise, just as delegating required compiler work
to a reference tool does elsewhere; no new anti-cheating mechanism is needed.

PA29 later grades the student's `lowir2native`; its tests must never silently
substitute the supplied backend for that implementation. The existing native
regression gate verifies the supplied tool. No CY86 oracle gate is needed.

### Preserve the lesson and remove its obsolete dependency

- Keep PA13, `pa13/lowir.md`, its grammar/explorer and useful introductory
  tests. Update their eventual paths through the ordinary numbering move.
  Do not relocate the LowIR introduction into PA15 or defer its reusable
  input machinery to PA29.
- Replace the `lowir2cy86` deliverable with the LowIR-focused interface and
  behavioral submissions described above. Retire CY86 text-shape expectations
  while preserving useful LowIR model/format/validation assertions.
- Review PA13's tests against native and optimizer suites by observation.
  Keep early requirements here; move only observations owned by a later
  feature. Preserve debug-info coverage, scripts and root/CI routes.
- Retire PA9, CY86 clients/scaffolds/payloads and private libraries after
  checking shared dependencies. Keep PA13's assignment wrapper and replace
  its tool wiring. Preserve the shared LowIR model, reader, writer, validator,
  native backend and optimizer.
- Move necessary encoding, label-patching and ELF explanations/scaffolding
  from PA9 into the native lesson with attribution and manageable test groups.
  Remove obsolete reading prerequisites and second-backend instructions.

## 6. Test migration policy

The unit of preservation is a useful, course-supported assertion. There is no
requirement to preserve every input, every rejection, or the old test count.

- Prefer valid, small examples that isolate the intended language rule.
- Drop negatives that exist only because of an obsolete grammar restriction,
  a mock name convention, or a reference implementation limitation. Invalid
  syntax negatives may also be dropped when they add no useful coverage to
  the new assignment. Do not undertake diagnostic completeness as a side
  project of consolidation.
- Reduce or retire positive cases whose success depended on accepting invalid
  syntax. Semantically invalid but syntactically meaningful examples can
  still test the parser when that boundary is explicit.
- Keep useful semantic negatives at their actual owner, when rejection is
  part of that assignment's contract. A rejection by a full host compiler
  does not by itself establish a required early parser rejection.
- Retire mock-image formatting and redundant assertions. For useful behavior
  already covered later, record the existing covering test.
- Before adding a case, inspect both `tests/general/` and `tests/spec/` at
  the destination and any relevant later owner. Compare assertions and
  reference artifacts, not just filenames or identical source text. Prefer
  an existing case; extend it with a missing observation when that keeps it
  focused. Do not reproduce every combination of already-covered forms.
- Give each remaining addition a specific coverage gap in the inventory.
  A parser tree, a resolved type, initialized data, and runtime identity are
  different observations; retain more than one only when each is needed.
- Regenerate references only after selecting the intended behavior. An
  existing compiler result is evidence to investigate, not the specification.

During Stage A, maintain a compact checked-in migration inventory containing:

```text
old case/group | assertion | disposition | new or covering path | observation
```

Allowed dispositions are retain, reduce/split, covered elsewhere, and retire
with a reason. Account for the 75+72 preprocessor roots, 48 recognition roots,
43 namespace roots, 67 initialization groups, 20 CY86 groups and all 162
tracked PA13 roots (including the 30 outside default discovery and debug-info).
Multi-assertion cases may have several rows. This is a migration record, not a
permanent new student-facing configuration system.

Move each input with its included files, numbered translation units, stdin,
flags/environment sidecars, and meaningful reference files. Rename early
fine-grained fixture numbers into the destination's hundreds-based clusters
where required. Preserve relative include behavior and update embedded source
paths through reference regeneration. Do not collapse separate translation
units into one file to make the harness easier.

### Harness invariants

The current runner distinguishes `text_t` and `text_t1`, chooses early stdin
handling from the assignment directory name, and reads `CPPGM_APP_ARGS` for a
worker invocation. The environment variable alone does not provide arbitrary
per-case interface routing.

Give the affected suites explicit invocation profiles or small wrappers for
stdin tokens, file tokens, AST/types/semantics, grouped sources, and execution.
Reuse existing routed-test facilities where appropriate. Avoid a general new
harness framework. Ensure profile selection survives renumbering.

For each migrated lane verify:

1. Discovery lists the expected test roots exactly once, with companions in
   the intended order; PA8 `.t.1` groups cannot disappear in a `.t`-only
   destination.
2. `test`, `check`, and `ref-test` select the same tool, flags, grouping, and
   comparator. Batch and ordinary execution discover and check the same cases.
3. Comparators inspect the asserted artifact. Failure-only tests compare
   status, not unstable diagnostics; runtime tests check actual execution.
4. Existing stdin/stdout, date/time/author normalization, file paths, and
   translation-unit state-reset rules remain deliberate.
5. A known-wrong result is detected: for an initialization oracle, alter an
   expected value or substitute a bad output in a harness test and demonstrate
   failure. A green run against regenerated references is insufficient.

## 7. Implementation sequence at current numbers

Content and behavior changes land before renumbering. Every deletion includes
its live consumers; old directories may remain briefly as explicit transition
records, but must not remain discoverable as empty passing suites.

### A0. Freeze inventory and establish the execution boundary

- Record baseline case inventory, references, required tools, and actual
  default test discovery. Extend the original 325-root inventory with all
  162 tracked PA13 roots; the revised inventory has 487 roots.
- The initial PA13 execution wiring is transitional work, superseded by
  section 5. Replace its CY86 execution step with the supplied native backend
  and grade student-produced LowIR in PA13. Preserve the assignment itself.
- Record a performance baseline before parser/semantic changes that could
  affect compile time or memory. Documentation and file moves alone do not
  require a compiler-performance experiment.

### A1. Consolidate preprocessing at PA4

- Rewrite PA4's contract and scaffold around `preproc` and the teaching order
  in section 3. Establish its explicit file-based harness profile.
- Migrate/screen PA4 and PA5 tests, support files, and references. Preserve
  the library pipeline used by the compiler.
- Retire the PA5 numbered contract and separate `macro` tool/scaffold.
  Remove their active build/report/export/checkpoint consumers in the same
  change. Keep the macro-processing library.
- Prove the merged suite and remaining preprocessing consumers still work.

### A2. Consolidate parsing at PA10

- Rewrite the handout to teach recognition and structured AST construction
  together, without assuming completed PA6 or PA9 work.
- Screen PA6 cases and rehome useful assertions and explanatory material.
  Resolve retained parser gaps in `dev/src/syntax/`, using the shared grammar.
- Once intake is accounted for, retire `recog`, its scaffold/source set,
  `recognition/`, PA6 grammar/explorer, and the old numbered contract. Update
  shared grammar documentation/explorer instead of leaving explanations
  that depend on a deleted grammar.

### A3. Establish semantic intake at PA11 and later owners

- Rewrite PA11 as the first semantic lesson, including type spelling and
  scope/lookup teaching currently supplied by PA7. Update PA12's prerequisites.
- Screen PA7 and PA8 using sections 4, 6, and 10. Activate retained assertions
  in destination suites while the old implementations are available for
  comparison. Useful adopted assertions must be covered before deletion;
  explicitly retired negatives and mock-format assertions need no new test.
- Fix only adopted behavior gaps, in their production modules. Do not change
  the compiler just to make all old acceptance verdicts match.
- Retire `nsdecl`, `nsinit`, their scaffolds/source sets, namespace-specific
  early implementation directories, grammars, and numbered contracts.
- Mark the cross-client objective of
  [PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md](PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md)
  as superseded. Deleting its early clients does not resolve every mainline
  semantic-layout or dependency issue discussed there; preserve independently
  useful observations as deferred maintainer work outside this consolidation.

### A4. Preserve PA13 and omit PA9

- Implement section 5 in PA13: keep the LowIR introduction and its reusable
  machinery, with a small required behavioral portion accepting different
  valid LowIR implementations. No C++ lowering is required at this milestone.
- Keep the LowIR specification/grammar with PA13. Review all 162 roots against
  the revised contract and existing coverage before changing their disposition.
- Retire PA9's wrapper and CY86-specific clients/scaffolds/private source sets
  and reference payloads. Replace PA13's obsolete translator wiring while
  preserving its assignment, required tests and place in the progression.
- Update export and self-host tool mappings for PA13's surviving deliverable.
  An assignment stage need not correspond to a separate new binary.
- Preserve debug-info discovery, reference routes and audit-ledger anchors.
  Update them with any actual ownership changes, without duplicate fixtures.

### A5. Close all live dependencies before renumbering

- Update `dev/Makefile`, `dev/frontend_source_sets.mk`, root discovery/report
  lists, reference wrappers, scaffolds, and PA39 mappings as each tool retires.
  Root discovery scans `pa*/Makefile`; remove or exclude retired wrappers
  deliberately instead of leaving empty successful targets.
- Update export arrays, its generated student Makefile/source sets, shared
  contract copying, reference manifests, and
  `scripts/tests/test_exported_dev_makefile.py`. A tool may remain supplied
  even when it is no longer student-built.
- Update owner ledgers, exception-count expectations, and architecture audits
  for removed sources. The compiler rename manifest currently requires every
  historical source to have an existing owner. Add a narrowly checked retired
  disposition with a reason for deleted subsystems; do not erase baseline
  rows or assign an unrelated surviving file as their owner.
- Remove live references to deleted grammars, old output contracts, defunct
  starter-kit clone instructions in rewritten handouts, and missing Reading
  Assignment A/B dependencies. Supply needed explanations in the new
  handouts or linked, shipped reference material.
- Complete the old-path assertion inventory and validate an export at the
  still-current numbers. Teach final content before moving its paths.

## 8. Self-hosting and validation

### The ladder follows useful programs and preserved behavior

PA39 has both assignment stages and binary checkpoints. Several stages use
`cppgm++`; `INCEPTION_PRIMARY_STAGE_*` picks a primary test for a binary, while
`INCEPTION_CHECKPOINT_FOR_*` and `INCEPTION_PREV_STAGE_*` drive the assignment
ladder. Update all of them, along with `CHECKPOINTS`, `INCEPTION_PREV_*`, batch
stage lists, and explicit dependencies. There is no one-assignment/one-binary
constraint.

After Stage A, the required early binary chain is:

```text
pptoken -> posttoken -> ctrlexpr -> preproc -> cppgm++
```

`ctrlexpr` becomes `ppexpr` in Stage B. Retire `macro`, `recog`, `nsdecl`, and
`nsinit`, `cy86` and the obsolete `lowir2cy86` checkpoints. Keep PA13 as an
assignment stage and map its surviving LowIR implementation checks to their
actual student tool. Its behavioral submissions execute through the supplied
native backend; they make no self-host claim about that backend. The later
`lowir2native` checkpoint follows the compiler when its assignment is reached.

The first full compiler self-build remains the important CI boundary: today's
`pa39 test-through-pa10`, eventually `pa34 test-through-pa5`. It builds the
whole current `cppgm++` source set and runs preservation through the AST
assignment. It does not prove all later modes work under that self-built
compiler, and it does not prove reproducible inception.

Keep the actual inception comparison separately: root `make inception`
invokes `compare-cppgm++-inception`, comparing the first self-built compiler
with the compiler it rebuilds. Preserve later stage checks as well. At final
consolidation, run through today's PA13 (eventually PA8), including its
LowIR implementation checks and required behavioral exercises. Also preserve
the through-PA15 gate (eventually PA10) for the first C++ lowering stage.
Source behavior checks there execute the self-built frontend's LowIR through
the supplied native backend.

The ladder loses diagnostic compile workloads when programs retire. Do not
retain student assignments solely to supply them. If a concrete self-host
regression becomes hard to isolate, use focused source/object probes or a
maintainer-only test with its own stated purpose.

### Gates

Run focused checks after each content step. After compiler changes, run the
repository-required compiler gates. Before Stage B and at final completion,
run these at the applicable current or renumbered paths:

```sh
make
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report-nobuild
make test-debuginfo-nobuild
make test-variants
make test-harness
make audit-lowir-contract audit-compiler-layout audit-compiler-rename-manifest \
  audit-compiler-exceptions audit-frontend-source-sets audit-semantic-owners \
  audit-builtin-registry-tables audit-lowering-owners audit-native-owners
perl scripts/cppgm_file_audit.pl --paths dev
python3 scripts/audit_pa_feature_placement.py --fail-on-early
make -C pa39 test-through-pa10 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa39 test-through-pa13 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa39 test-through-pa15 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make inception
make -C pa38 test-perf
```

Use the matching real host compiler and standard-library flags in each CI
flavor; the examples show Linux g++. Preserve the four existing self-host CI
flavors. For this consolidation, use native wall-time ABBA performance
validation as explicitly requested on 2026-09-07. Follow the immutable binaries,
fixed workload, A/A calibration, balanced ordering, output verification and
load-screen rules of the
[O2/O3 measurement plan](implemented/v3/PLAN-O2-O3-OPTIMIZATION.md#measurement-model)
and its [measurement protocol](implemented/v3/PLAN-PERF.md#4-measurement-protocol-for-an-intermittently-loaded-host).
Record CPU time and RSS alongside wall time. This replaces the unavailable
macOS hardware-counter gate for this task; it does not change the course's
native-code performance envelopes or claim a hardware-counter result.
For placement review, explicitly
include destination assignments: the auditor's default feature scan is
PA15–PA28, not every early/late destination.

Regenerate affected references through their owning targets. Final unchanged
regeneration must be clean. Preserve the distinction between maintainer exact
LowIR comparisons and the existing student comparison policy.

Run `scripts/export_student_repo.sh <fresh-destination>` and validate the
export independently: starter build, supplied native backend selection, actual
test discovery, grouped inputs, reference regeneration, script dependencies,
and representative test/check paths with batch execution enabled and disabled.
Required early tests must work without a student-built backend. Neither CY86
client nor an optional assembler exercise is shipped.

Finally review the student handoff at each changed boundary: prior knowledge,
provided interfaces, new implementation, independently runnable test groups,
and the artifact consumed next. A full reference compiler and an export that
merely builds its stubs do not establish that progression.

## 9. Stage B: final numbering and mechanical migration

Only start Stage B after Stage A's content, fixture dispositions, tools,
harnesses, and export are settled. The mapping is:

| Original number | Final number or destination |
| --- | --- |
| PA1 | PA1 |
| PA2 | PA2 |
| PA3 | PA3; rename `ctrlexpr` to `ppexpr` |
| PA4 + PA5 | PA4, `preproc` |
| PA6 + PA10 | PA5, `cppgm++ --emit-ast` |
| PA7 + PA11 | PA6, `cppgm++ --emit-types` |
| PA8 | Distributed assertion intake; no numbered image assignment |
| PA9 | Omitted; necessary encoding/fixup teaching moves to the native backend |
| PA12 | PA7, expression/call semantics |
| PA13 | PA8, LowIR introduction and required behavioral exercises using the supplied native backend |
| PA14 | PA9, typed ABI naming |
| PA15–PA18 | PA10–PA13, procedural lowering and object model |
| PA19–PA24 | PA14–PA19, templates and constant evaluation |
| PA25–PA28 | PA20–PA23, language/object-model closure |
| PA29 | PA24, native backend |
| PA30–PA33 | PA25–PA28, compile/link and host interoperability |
| PA34–PA36 | PA29–PA31, hosted compatibility |
| PA37–PA38 | PA32–PA33, optimization |
| PA39 | PA34, inception |

Every surviving current assignment from PA10 through PA39 subtracts five.
PA13 therefore becomes PA8. Move via temporary paths to avoid directory
collisions. Preserve
symlinks and executable modes. Do not combine this move with further test
screening or compiler fixes; those return to a separate content change.

Rewrite live handouts, `ROADMAP.md`, root/export documentation, Makefiles,
grammar symlinks/explorers, runtime/helper paths, reference wrappers, and
student inventories. Update both `.github/workflows/tests.yml` and
`selfhost.yml`, inception artifact paths, debug-info and performance targets,
and workflow dependencies. Update integer ranges, classification cutoffs,
CLI choices, and fixtures in the placement auditor and its tests; changing
literal `paN` strings alone is insufficient.

Add an explicit numbering migration record and mark historical plans as using
their original numbering. Preserve historical records and attribution rather
than mechanically rewriting their past-tense claims. Check live links and
audit-required owner paths separately from archival references.

`ppexpr` is the sole cosmetic tool rename: it describes preprocessing
expression evaluation and pairs naturally with `pptoken`. Keep `preproc`
and the other surviving names. If a concrete compatibility cost makes this
rename undesirable during implementation, omit it as a separately documented
decision; it has no bearing on consolidation or the assignment count.

After renumbering, the focused examples above become:

```sh
make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa34 test-through-pa8 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa34 test-through-pa10 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make -C pa33 test-perf
make inception
```

## 10. Resolved PA8 intake

The [migration inventory](early-assignment-consolidation-inventory.tsv)
records all 67 original PA8 groups, their frozen inputs, the assertion
actually adopted, and the exact current covering fixtures. Its resolved
counts are 9 retained, 10 reduced/split, 26 covered elsewhere, and 22 retired.
These are implementation decisions; there is no optional diagnostic intake.

The main preserved observations are:

- Declaration/type identity, alias qualification, namespace lookup and
  reference-derived bounds in PA11, with canonical call/conversion facts in
  PA12. The documented source-parameter type view remains distinct from a
  canonical function signature.
- Initialized scalar/pointer bytes, zero definitions, and global reference
  lifetime in PA15; character-array initialization and thread-local storage
  redeclarations in PA16.
- Constant reference/pointer aliases in PA21, and cross-translation-unit
  identity and linking at their actual toolchain owners.

Mock image addresses, fixed mock function alignments, and obsolete grammar
negatives are retired. Existing cases already cover many valid forms: extend
an existing observation when it has a small gap instead of adding another
root for a different spelling. Preserve a separate case only when the outcome
is distinct, such as two translation units retaining separate internal
objects or a global reference keeping its temporary alive.

The inventory also resolves all PA4/PA5, PA6, PA7, PA9 and PA13 inputs—487
baseline groups in total. Baseline paths and hashes remain historical; current
destination fields follow actual moves and the final numbering migration.

## 11. Completion criteria

The consolidation is complete when:

1. The required sequence is PA1–PA34, with one complete preprocessor, one
   parser/AST lesson, one initial semantic model, the retained LowIR
   introduction (current PA13, final PA8), and one native backend.
   There is no required or optional CY86 construction/translation assignment.
2. No student must implement `macro`, `recog`, `nsdecl`, `nsinit`, `cy86`,
   `lowir2cy86`, or the PA8 image format as an extra preserved product. The preprocessing, parsing,
   semantic, and backend handoffs explain how useful work is extended.
3. The migration inventory accounts for every old case group. Useful adopted
   assertions have observable coverage; retired negatives and mock-format
   artifacts have reasons. An input merely continuing to compile is not
   enough to count its test as retained.
4. PA13 retains its reusable LowIR work and a small required behavioral
   portion executing student-produced LowIR through the supplied native
   backend, without requiring C++ lowering or matching IR dumps. Later source
   lowering can reuse that execution path. Native-assignment tests use the
   student backend.
5. Root, per-assignment, self-host, reference, and export paths agree on the
   active stages and test discovery. No deleted milestone remains as an empty
   passing suite or hidden build prerequisite.
6. All applicable compiler, harness, architecture, placement, performance,
   self-host, inception, and export gates pass with the final numbering.
7. Student documents describe the final course and its manageable test groups
   directly, without requiring students to reconstruct this migration.
