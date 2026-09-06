# Plan: the backend from the student's seat

Status: implemented, 2026-09-05 (`a1775864` audit, `8b5f15fe` harness,
`afb1206b` migration, `8775c4c8` READMEs, suite split and variants,
`f67e32b3` a second allocation design, and the newcomer trial that
followed).  Phase 6 was done at the allocator's seam: a fresh agent,
forbidden the course solution's allocator and lowering, wrote a
linear-scan allocator from the PA38 README and `planning_seam.h` that
passes the course suites and the performance envelope
(`CPPGM_BACKEND_VARIANT=linear-scan`); its notes
(`doc/backend-review/newcomer-notes.md`) changed the README, the seam
header, two generators and the harness.  The trial did not rewrite the
lowering, encoding and object paths.  Opened 2026-09-05 at `5cdc5786`, when
`PLAN-LIBCXX-PERFORMANCE.md` closed.  The audit and its rerun after the
migration are `doc/backend-student-review.md`.

Where it stands against the exit criteria:

- READMEs: PA29, PA37 and PA38 have the three-part structure and their
  quality bars name no mechanism; the One Design sections still carry the
  old bullets' wording under a non-normative preamble.
- Course suite: no strict or structural oracle outside the contract-shape
  set (seven PA29 ABI fixtures); every PA37 optimization fixture runs the
  lowering floor, and the driver-lane sources are built and run against
  the emitted LowIR's program; 86 course fixtures carry outcome lines.
- Mutation audit: zero course fixtures fail under the three perturbations,
  which are now the `make test-variants` designs.
- Regression lane: holds every reference and control the course dropped
  and runs with `make test`.
- Second implementation: a newcomer's linear-scan allocator, written from
  the README and the seam header without the course solution, passes PA38's
  course suite and envelope and the PA37 and PA29 course suites; the
  course solution's lowering, encoding and object paths were not
  reimplemented.

## The question

A student following PA13 through PA39 has to build a backend: LowIR into
machine IR, machine IR into objects and executables, an optimizer over LowIR,
an optimizer over machine IR, and finally a compiler that builds itself.
Have we set that student up to build *a* backend, or do our instructions and
tests only admit *our* backend?  This plan is the review that answers it,
the evidence gathered so far, and the proposal for instructing and testing
lowering and optimization so that a different correct design passes.

## What the first pass found

The review has not been done yet; this is the survey that scopes it.  The
counts below are from the tree at `5cdc5786` and are the material the review
will work from.

### The assignments and how each one judges its output

| assignment | what the student builds | how success is judged |
|---|---|---|
| PA13 `lowir2cy86` | adapter from LowIR text to CY86 | behaviour through the CY86 machine; structural validation of the LowIR subset |
| PA29 `lowir2native` | LowIR to machine IR to native executable | 184 tests: 36 strict (exact machine-IR dump), 60 structural (dump after canonicalization), 88 behaviour; course roots strict 4, structural 25, behaviour 78 |
| PA30 to PA36 `cppgm++ -c` | objects, linking, host EH facts, hosted ABI, sections | program stdout and exit status, plus `.inspect` sidecars that ask the object questions (sections, symbols, relocations) |
| PA37 `lowiropt` | the LowIR optimizer at -O1 to -O3 | 253 sources, 201 references compared after presentation canonicalization (names, metadata, order); 27 control fixtures judged by 29 pass-specific checker scripts |
| PA38 `lowir2native -O*` | the machine-IR optimizer | -O1 11, -O2 13, debuginfo 8 fixtures with machine-IR and canonical machine-IR oracles; 6 behaviour fixtures; 20 controls judged by 5 native checker scripts |
| PA39 inception | the compiler compiling itself | reproducibility: the staged build must MATCH |

Three comparison modes exist in the harness (`scripts/compare_results_common.pl`):

- `compare_lowir_text`: what every PA37 `.t` reference uses.  It validates
  both outputs and compares them after canonicalizing local value and block
  names, internal symbol names by position, metadata groups and top-level
  order.  Instruction sequence, block structure and every optimization
  decision must match.
- `compare_strict_machine_ir`: byte equality of the machine-IR dump after
  whitespace normalization.
- `compare_structural_machine_ir`: the dump after renaming the free general
  registers (`rbx`, `r10` to `r15`), the free vector registers (`xmm2` to
  `xmm7`) and frame displacements in first-use order, and dropping code
  alignment.  Instruction selection, instruction order, block order, the
  scratch-register discipline (`r11`), argument and result registers, and
  the phi-transfer form must still match ours.

### Where the instructions prescribe a mechanism

- PA37's "Optimization Levels" section is 719 lines and 71 bullets, naming
  63 numbers: the 768-instruction caller budget, the 512-instruction
  single-call body, the 40 and 48 instruction size caps, the four-instruction
  trivial leaf, the 128 and 192 instruction specialization caps, the eight
  calls a constant group needs, and so on.  Each bullet describes one of our
  passes and its policy.  The canonical-text references make every one of
  those numbers normative: an optimizer that inlines one call more or fewer
  produces a different text and fails.
- PA38's "Optimization Levels" section is 320 lines and 38 bullets, each a
  behaviour of our reactive planner: "reuse a final-use pointer register as
  the destination of an eligible scalar", "recolor a callee-saved physical
  register after complete MIR liveness", "keep a frequently reused,
  iteration-local scalar call result available across a call".  The
  controls pin those decisions by name: "compare was not deferred until
  after the call", "call-crossing value received no call-safe capacity",
  "call-free floating loop consumed integer preserved capacity".  A
  linear-scan allocator that produces a faster program fails those
  predicates.
- PA29's "Output Format" says the exact instruction inventory is target- and
  lowering-dependent, and then 96 of its 184 tests compare that inventory.
  A student who lowers `index` through `lea` where we use `add`, or who
  chooses `r10` as scratch, fails the structural lane.
- Both READMEs promise otherwise.  PA37 "Validation Modes": "Exact textual
  LowIR matching is not a PA37 grading requirement unless a test explicitly
  says so."  PA38 "Validation Modes": "Exact textual machine-IR matching is
  not a PA38 grading requirement unless a test explicitly makes that shape
  part of the oracle."  The letter holds: names, metadata and order are
  canonicalized before the comparison.  The shape is not: every
  instruction, block and decision must match, which is what the sentence
  leads a reader to think it does not require.
- The "Design Notes" sections are labelled non-normative and are the most
  useful text a student gets: PA29's five-stage structure, PA38's dense
  interval tables.  They are short next to the normative bullets.

### Where the instructions get it right

PA30 to PA36 judge objects by behaviour and by inspection: does the program
print what it should, does the object have the section, the symbol, the
relocation.  Nothing there asks how the bytes were produced.  PA39 asks only
that the compiler reproduce itself.  PA29's behaviour lane and PA38's
behaviour lane exist for "tests where several valid machine-IR layouts are
possible", which is the right idea applied to a minority of the fixtures.

### Why it happened

The course suites are also our regression suites.  Every optimizer landing
in the ledgers moved references and added a control so that the next landing
could not silently undo it; the references are our memory of our own
decisions.  Those two roles pull in opposite directions: a regression suite
wants the exact shape, a course suite wants any shape that meets the
contract.  Serving both with one set of files made the course suite exact.

## The review

Phase 1 is the review itself; the rest is what the review is expected to
lead to, stated now so the review knows what evidence to gather.

### Phase 1: audit (reading and measuring, no changes)

1. **Requirement classification.**  Read each backend README (PA13, PA29,
   PA30 to PA36, PA37, PA38, PA39, and `pa13/lowir.md`, 1,283 lines) as the
   student would, and classify every normative sentence as one of:
   *contract* (an observable fact: format, ABI, semantics preserved, a
   section present), *quality bar* (a measurable envelope: size, count,
   speed), or *mechanism* (a pass, a policy number, a data structure).  The
   deliverable is a table per assignment with the three counts and the
   list of mechanism sentences.
2. **Oracle classification.**  For every fixture, record its oracle kind:
   behaviour, inspection, structural-canonical, strict-text, or predicate
   script.  The table above is the coarse version; the audit produces the
   per-lane version and names the predicate each control checks.
3. **Design-freedom probes.**  For each component, write down two or three
   legitimate designs a student might choose and estimate which fixtures
   each fails.  Candidates: a linear-scan allocator with spilling instead of
   the reactive planner; a graph-colouring allocator; frame pointer always
   present; `cmov` for empty diamonds; a different scratch register;
   `lea`-based address formation; a classic SSA GVN and SCCP instead of our
   rule set; an inliner with a per-call cost model and no caller budget; a
   different phi-transfer strategy (sequential with a temporary versus
   parallel copies).
4. **Mutation audit.**  Cheaper and more honest than estimating: perturb
   our own implementation in ways that preserve correctness and count the
   failing fixtures.  Swap the order of two independent passes; change a
   budget by one; choose `r10` as the encoder scratch; emit `lea` for one
   addition form; run the diamond collapses at -O2.  Each perturbation is a
   correct compiler; every fixture it fails is a fixture that tests our
   shape rather than the contract.  Record the count per perturbation per
   lane.  This number is the baseline the rest of the plan improves.
5. **Contradiction audit.**  List every README promise the harness does not
   keep (the two "Validation Modes" statements are the first entries) and
   every harness behaviour the README does not state.
6. **Effort audit.**  Estimate the size of a student implementation that
   meets the contract for each assignment against the size needed to match
   our shape, using our own source as the upper bound (`dev/src/native`,
   `dev/src/lowir/optimize`).  The gap is the cost of over-specification.

Deliverable: `doc/backend-student-review.md` with the six tables, and a
decision on the proposal below made on those numbers rather than on this
survey.

### Phase 2: the contract-and-envelope test model

The proposal, to be confirmed or amended by Phase 1:

- **Behaviour is the floor.**  Every optimization fixture at every level
  compiles to an executable through the unoptimized native path and runs;
  the optimized output must behave the same.  PA37's `.t` fixtures do not do
  this today; the object-roundtrip and PA38 behaviour lanes show the
  mechanism exists.
- **Well-formedness is required.**  The LowIR structural validator and a
  machine-IR validator (contract facts only: every phi leads its block,
  definitions precede uses, calls carry the ABI registers, callee-saved
  registers are preserved, the stack is aligned at calls) run on every
  output.
- **Envelopes replace shapes.**  Each fixture may carry a budget sidecar
  (`x.ref.budget`) naming metrics with limits: instruction count, block
  count, loads, stores, calls, frame operands, spills, callee-saved saves,
  and per-region counts ("no load in `^loop`").  Our reference output sets
  the limit, with a stated tolerance; any design at or under the limit
  passes.  When we improve the reference, the limit tightens only at a
  course edition boundary.
- **Properties replace pass predicates.**  A small declarative sidecar
  (`x.ref.expect`) states the observable outcome of the optimization the
  fixture is about: `count(call @callee) == 0` for an inlining fixture,
  `count(load) <= 1 in ^body` for hoisting, `count(phi) == 0 in ^merge` for
  a diamond collapse, `program.stdout == ...` for behaviour.  One evaluator
  (`scripts/expect_lowir.pl`, `scripts/expect_mir.pl`) replaces the 34
  per-pass checker scripts for course purposes.  The expectation is written
  in terms the student's README defines, never in terms of our pass names.
- **Shape oracles stay only where the shape is the contract.**  The ABI
  (argument and result registers, callee-saved preservation, alignment, the
  red zone), unwinding and debug facts, object sections and relocations:
  these are what a debugger, an unwinder and a linker depend on, and a
  strict or structural oracle is right for them.  Instruction selection,
  register choice among the free registers, block order, and the internal
  form of a phi transfer are not the contract.
- **A performance envelope at the assignment level.**  A small benchmark
  set per optimizing assignment (micro-kernels and, at PA39, the self-build
  workload) with the reference's instruction count under Cachegrind, which
  is deterministic and load-immune, and the student's -O1/-O2/-O3 must land
  within a stated percentage.  This is the one place a student's design is
  compared to ours on outcome rather than on shape, and it is the right
  place: a linear-scan allocator that is within ten percent is a pass.
- **Two suites, not one.**  Our exact-shape references and pass-specific
  controls move to an implementation regression lane that the course does
  not see (`cppgm.tests/regression/`).  The course suite keeps behaviour,
  inspection, envelopes, properties and the contract-shape oracles.  Our
  ledgers keep their memory; the student keeps their freedom.

### Phase 3: the instructions

Each backend README is restructured into three parts with the same headings
in every assignment:

1. **Contract** (normative): formats, ABI, what must be preserved, what the
   validators check, what the inspection sidecars may ask.  This is most of
   PA29's "Assignment Boundary", PA30 to PA36's implementation surfaces, and
   the LowIR and machine-IR grammars.
2. **Quality bar** (normative): the envelopes and the benchmark
   percentages, and the properties the fixtures state, described by outcome
   ("a call to a small non-recursive internal function with no exception
   region is inlined; the fixture's expectation names the call") rather
   than by policy number.
3. **One design** (non-normative): what our solution does, including the
   pass list, the budgets and the data structures, moved from the
   "Optimization Levels" bullets and labelled as an approach that meets the
   bar, with its measured results.  The current "Design Notes" grow into
   this section.

The 71 PA37 bullets and 38 PA38 bullets are the material for part 3, not
part 2.  The numbers in them become the record of what our design chose,
not what the student must choose.

### Phase 4: migrating the fixtures

For each fixture in PA29, PA37 and PA38: keep it as a contract-shape oracle
if Phase 1 classified its subject as contract; otherwise convert it to
behaviour plus an expectation and a budget generated from the current
reference output.  The mutation audit from Phase 1 is rerun after each lane
migrates; the count of fixtures failed by a benign perturbation is the
progress metric and should reach zero on the course suite.

### Phase 5: keeping it that way

The reason the suite became exact is that nothing pushed back.  Two
mechanisms give it a counterweight:

- **Design variants in CI.**  Keep two or three deliberately different but
  correct backends alive behind flags in our own tree (`--allocator=linear`,
  `--frame-pointer=always`, `--select=cmov`, a pass-order shuffle) and run
  the course suite against every variant.  A course fixture that fails a
  variant is over-specified and is fixed before it merges.  The variants
  are cheap to keep because they are our own code with a policy switch, and
  they double as the ablation baselines the performance ledgers keep
  reaching for.
- **A rule for new tests.**  A change that adds a course fixture states the
  contract or the outcome it tests; a fixture whose only justification is
  "the shape our pass produces" goes to the regression lane.

### Phase 6: a second implementation

The only complete proof is a backend that is not ours passing the course
suite.  Have someone, or an agent, implement PA29 and PA38 with a linear-scan
allocator against the migrated suite and README, without reading
`dev/src/native`, and record what they needed that the instructions did not
give them.  Their notes become README changes.

## Exit criteria

- Every backend README has the three-part structure and no normative
  sentence names a mechanism or a policy number.
- The course suite has no strict-text or structural-shape oracle outside
  the contract-shape set, and every optimization fixture runs its program.
- The mutation audit fails zero course fixtures for every benign
  perturbation in its list.
- The regression lane holds every reference and control the course suite
  dropped, and our ledgers point at it.
- The second implementation passes PA29 and PA38.

## Risks

- Losing regression sensitivity.  The regression lane is the answer; it
  must exist before any course reference is relaxed, and `make test-cells`
  must run both.
- Envelopes that are too loose let a poor design pass, too tight readmit
  the shape.  The tolerance is set from the design variants: every variant
  we consider acceptable must pass, and the loosest limit that admits them
  is the limit.
- The second implementation is the expensive step and the only one that
  cannot be faked.  Budget it as an assignment-sized effort, not a review.
