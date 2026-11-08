# Frontend work and self-built compiler performance, 2026-11-08

The frontend has a small opportunity to reuse immutable type facts. The next
self-build codegen investigations are register-copy propagation across joins
and repeated materialization of wide constants in loops. None of the broader
prototype rewrites has established a reliable CPU-time win.

## What is being compared

The codegen score compares **GCC-built cppgm++ with self-built cppgm++ executing
identical compiler inputs**. It is separate from cppgm++ versus GCC compiling
C++ source. The latter measures the compiler algorithms and feature coverage;
the former measures the native code those algorithms run as.

The baseline is peeled v8, `b7c6b8e72595a524e1b3f147212d69c02b257dd8`.
The current compiler is `17b024f692a5b537a4db6c4d3b6dc33bc930c680`;
`6a6a15f446cf4249b207924c70da749b98b10d1a` adds the CI split with identical
compiler sources. GCC 15.2 builds the native controls. Three immutable common
preprocessed inputs keep source and headers fixed across versions. Native and
self-built compilers must produce byte-identical objects for every control.

All ratios labelled speed below are higher-is-better. Instruction ratios
measure executed work, not elapsed time.

## Frontend growth since v8

Hardware instruction counts, current versus v8:

| Frozen input | Whole frontend (`-fsyntax-only`) | Initial semantic phase |
| --- | ---: | ---: |
| compiler-loop | +2.76% | +5.75% |
| frozen-semantic | +3.03% | +4.61% |
| boost-flat-map | +2.25% | +3.37% |

There is more semantic work as well as additional checking. On `frozen-semantic`,
the initial graph creates 143,083 types versus 139,526 at v8 (+2.55%), performs
456,461 lookups versus 449,199 (+1.62%), and instantiates 11,044 classes and
7,959 functions versus 10,850 and 7,696. These counters establish changed work;
they do not assign a precise fraction of the extra time to each correction.

Lookup already uses dense `(scope ID, identifier ID)` keys and indexed entries.
The synthetic scope, member and template-default scaling controls remain
approximately linear. There is no evidence here for replacing the entire
lookup structure. Profiles instead point to declaration processing, template
pack queries, reference compatibility and dependence queries. Scope-sensitive
answers and nominal types whose template facts are still being formed need
particular care when considering caches.

### Candidate: retain facts when they become fixed

The candidate seeds the existing dependence cache at three construction points:

- Fundamental types are non-dependent.
- Template, pattern and partial-specialization roles are dependent, independently
  of their later definitions.
- A class specialization's dependence is known after its fixed argument list
  has been classified, before publishing it in the specialization index.

This adds no type-record field or allocation. Ordinary nominal types continue
to be queried until the relevant facts are fixed. It preserves parsing and
semantic checks and gives subsequent queries a fact already established by
construction. No student language requirement or LowIR contract changes.

Whole-frontend instruction speed, baseline divided by candidate:

| Frozen input | GCC-built compiler | Self-built compiler |
| --- | ---: | ---: |
| compiler-loop | 1.0038× | 1.0038× |
| frozen-semantic | 1.0074× | 1.0074× |
| boost-flat-map | 1.0070× | 1.0068× |

The complete `-O3` compile saves about 0.05–0.18% of instructions on these
inputs because the frontend is only part of that work. All three optimized
objects remain byte-identical. Native and self-built frontend savings are
similar, so this change alone should have little effect on their ratio.

The CPU-time evidence is less precise. EPYC and Intel paired controls show
A/A variation comparable to or larger than these effects. A repeated Intel
campaign was stopped after roughly 4% A/A variation; every completed
observation and the cancellation reason are retained. The separate Intel
hardware-counter campaign confirms instruction and branch savings, with
cycle A/A ratios spanning roughly 0.983–1.027. Do not advertise a resolved
sub-percent time improvement from these runs.

## Where the codegen gap changed

On the same `frozen-semantic` input, GCC-built instructions divided by
self-built instructions:

| Phase | v8 | Current |
| --- | ---: | ---: |
| Preprocess, lex and parse | 1.0375× | 1.0337× |
| Initial semantic analysis | 0.9943× | 0.9838× |
| Lowering and deferred semantics | 0.9784× | 0.9787× |
| Optimizer | 0.9803× | 0.9749× |
| Native object generation | 0.9258× | 0.9290× |

The native object's absolute instruction gap remains substantial, but it
improved slightly here. The relative overhead grew in initial semantics and
the optimizer. A quiet EPYC 7B13 single-input `-O3` CPU control measured
0.9619× at v8 and 0.9568× current, about a 0.5% relative decline. This is a
single-input control, not the full CI self-build score. The local full-source
campaign had large individual timing outliers and cannot establish its size.

A generator control also rules out a recent source of regression: building
the identical v8 implementation with the pre-Godot main compiler and with
`17b024f69` produces the same 259 objects and executable. The identical current
implementation likewise produces the same 321 objects and executable with
those two generators. The recent assembly/Godot branch does not change the
native compiler bytes for either implementation.

The first independent CI self-build run at `6a6a15f44` reports 0.98× versus
GCC-built and 1.02× versus Clang-built cppgm++ on EPYC 9V74, with GCC/Clang
A/A ratios 1.013/1.018. Its compiler sources are unchanged by the CI split.
Those calibrations limit interpretation of a one-percentage-point move.

## Prototype decisions

| Prototype | Evidence | Decision |
| --- | --- | --- |
| Hash and compare only active type-key fields | Fewer instructions; no reliable CPU win and some slower controls | Withhold |
| Transpose the register-home meet and compare whole rows | Saves roughly 0.4–0.9% of complete `-O3` instructions; CPU results mostly flat | Withhold |
| Revisit a dataflow block only after a predecessor output changes | Same fixed point and objects; fewer meet operations; no resolved CPU win | Withhold |
| Select 32-bit unsigned bitwise and short logical shifts | High-bit/wider-reader checks agree with GCC and Clang at O0–O3; 86 of 321 self-build objects change, but measured compiler instructions and CPU are essentially flat | Withhold |
| Track local register equality with value numbers | PA33 driver checks pass 396/396; more copies disappear on two controls, but the compile-work effect is tiny | Retain as a prototype for the next investigation |

The type-key and integer-width experiments remain outside production sources.
The revised register-home dataflow implementation is evaluated separately in
the codegen-pass measurements below.
No benchmark-specific source checks, instruction sequences or magic values are
used to select a program for optimization.

## Codegen work

1. **Reduce the cost of producing the existing machine code.** Include
   allocation, machine cleanup and layout in the compile-time investigation.
   Profile repeated graph construction, unchanged dataflow inputs, register
   effects and unnecessary queries. Preserve typed facts and respect the
   mutations that invalidate them. The first change makes register-home
   forwarding cheaper; [the measurements](codegen-pass-efficiency-20261108.md)
   include a frozen compiler rebuild with byte-identical output.
2. **Carry register equivalence across joins.** In the generated type hash,
   incoming phi transfers leave two registers equal, then the join copies one
   back into the other. Local copy cleanup forgets that equality at the block
   boundary. Model equivalence partitions over all incoming edges, with writes
   splitting classes and copies merging them. Use finite dataflow, reverse
   postorder and changed-predecessor scheduling. Account for every mid-block
   exit, exceptional entry, addressed label and inline-assembly clobber. Reuse
   CFG and instruction-effect facts; avoid repeated unbounded backward scans.
3. **Give loop constants an economical register lifetime.** The same hash's
   wide multiplier is materialized into the emitter scratch on each iteration;
   GCC retains it in a register. Expose materialization and register effects at
   the appropriate machine stage so allocation can weigh reuse against
   pressure. Scratch-register lifetime assumptions and hidden emitter clobbers
   must remain valid. Do not merely retain a scratch value across a label.
4. **Continue frontend attribution before changing containers.** Separate
   extra instantiated entities from work per entity, then target repeated
   structural or already-published facts. Preserve definition-point lookup,
   substitution, access and completion dependencies. Avoid global caches whose
   invalidation introduces speculative hot-path work.

Use reduced behavior tests for new behavior or generated-code improvements,
broad cost bounds where useful, immutable compiler controls and an
existing-workload sanity check. Tests
should permit different registers, instructions and block layouts. Accept a
change to generated code only after it improves execution and its compile cost
is justified. An implementation change that produces the same code qualifies
through lower compilation cost and byte-for-byte output checks instead.

## Validation of the fact-cache candidate

- Strict required report: 9,860/9,860.
- Architecture, layout, exception, source-set, registry, scaffold, IR,
  formatting, file and feature-placement audits pass.
- Debug-info checks, 120 optional target-feature checks and 182 optional
  assembly checks pass.
- Self-host through PA5 passes; inception rebuilds a byte-identical compiler.
- PA35 smoke passes 3/3.
- The qualified native compiler's `.text` matches the measured private probe;
  the three frozen `-O3` outputs also match the baseline byte for byte.

## Receipts and repeatability

Local recipes, binary/source hashes, commands, output hashes, PMU samples and
all paired observations are retained under
`/mnt/vizier/cppgm-frontend-perf/20261108`. The EPYC archives preserve all three
screening campaigns and the separate v8/current score control. The temporary
EPYC VM is stopped. Raw prototype and phase data are maintainer evidence and
are outside the student export.

The complete self-build measurement is independently reproducible through:

```sh
python3 scripts/public_benchmarks.py selfhost \
  --work obj/self-build-benchmarks/work \
  --output obj/self-build-benchmarks/selfhost.json
python3 scripts/public_benchmarks.py report-selfhost \
  obj/self-build-benchmarks/selfhost.json
```

The independent public-suite and self-build workflows retain separate
reports, artifacts and measured revisions on each host.
