# Plan: libc++ cell performance

`PLAN-CLANG-LIBCXX-SUPPORT.md` made clang and libc++ a supported cell: every
header compiles, the self build links, and inception matches at -O3.  What it
did not do is make that cell fast.  On the same eight translation units, the
compiler built by itself against libc++ runs 2.4x slower than the compiler
clang built against libc++, where the libstdc++ cell sits at 1.55x; and the
clang-built libc++ compiler itself takes 2.3x as long as the gcc-built
libstdc++ one to compile the same eight files.  This plan is the measured
decomposition of both gaps and the program that closes them.

The short version: the host compiler is not the cause; libc++ hands our
optimizer three loop shapes that libstdc++ writes as calls, and our optimizer
keeps all three; and our preprocessor re-reads and re-lexes every guarded
header on every re-inclusion, which libc++'s 768 small headers turn into 68 MB
of lexing per translation unit.

## Objective

Two targets, on one workload, with the measurement defined before the work.

**The workload** is the eight largest translation units of the compiler,
compiled serially, one process each, at `-O1` and again at `-O3`, best of
three runs, by four binaries: the host-built and the self-built compiler of
each cell.  The self-built binaries are the `INCEPTION_SELFHOST_OPT_LEVEL=3`
products, which is the level inception runs at.

```
dev/src/native/lowering/function.cpp
dev/src/lowir/optimize/pipeline.cpp
dev/src/semantic/declarations/analysis.cpp
dev/src/semantic/analysis/analyzer.cpp
dev/src/native/object/elf_writer.cpp
dev/src/semantic/initialization/analysis.cpp
dev/src/semantic/templates/classes.cpp
dev/src/semantic/constants/scalar_evaluator.cpp
```

| lane | -O1 | -O3 | self / host |
| --- | ---: | ---: | --- |
| gcc-built, libstdc++ | 18.970 s | 19.373 s | |
| self-built, libstdc++ | 29.288 s | 30.024 s | **1.544x / 1.550x** |
| clang-built, libc++ | 44.360 s | 44.597 s | |
| self-built, libc++ | 105.533 s | 106.539 s | **2.379x / 2.389x** |

**T1 -- codegen ratio.**  Self-built over host-built in the libc++ cell at or
below **1.50x** at both levels, from 2.38x.  The libstdc++ cell's ratio may not
move above its current 1.55x while this is done; it should move down, because
two of the three shapes below occur in libstdc++ code too.

**T2 -- front-end cost on libc++ input.**  The host-built libc++ compiler's
cost per byte of preprocessed output within **20%** of the host-built
libstdc++ compiler's, from 1.33x now.  In absolute terms that is the
clang-built libc++ lane from 44.4 s to about 31 s.

**What "within 20% of the libstdc++ lane" cannot mean.**  The two lanes do not
compile the same input.  For `analyzer.cpp`, the preprocessed libc++
translation unit is 1.72x the bytes and 1.67x the lines of the libstdc++ one
(11.82 MB against 6.86 MB), because libc++ in C++11 mode pulls in more and
instantiates more.  A libc++ lane within 20% of the libstdc++ lane in wall
time would have to compile 1.7x the input in 1.2x the time, which is not a
codegen question.  T2 is stated per byte for that reason, and the absolute
projection is given alongside so nobody has to convert.

## Evidence

Everything below is from one session on one box, `analyzer.cpp` at `-O1`
unless stated, with `perf record -F 1999` flat profiles and the workload above.
The four profiled binaries, the profiles, the annotated hot functions, and the
reproducers are under `$HOME/perf-libcxx/`; the O1 backend reproducers from the
support plan are copied to `$HOME/perf-libcxx/o1-bug/`.

### E1. The host compiler axis is clean

A fifth binary -- the compiler built by clang against **libstdc++** -- compiles
`analyzer.cpp` in the same time as the gcc-built one.

| binary | wall | user | peak RSS |
| --- | ---: | ---: | ---: |
| gcc-built, libstdc++ | 2.52 s | 2.29 s | 218 MB |
| clang-built, libstdc++ | 2.64 s | 2.39 s | 217 MB |
| self-built, libstdc++ | 3.97 s | 3.72 s | 220 MB |
| clang-built, libc++ | 5.86 s | 5.34 s | 316 MB |
| self-built, libc++ | 14.62 s | 14.11 s | 318 MB |

So clang's code generation is not why the libc++ lane is slow; everything
in the 2.52 s to 5.86 s gap is the input and the standard library, and
everything in the 5.86 s to 14.62 s gap is our code generation of that
library.  Both are ours to fix.

### E2. The lanes compile different inputs, and one of them is re-read

Headers entered, as the host compilers report with `-H`:

| | inclusions entered | distinct files | bytes in distinct files | `#include` directives inside them |
| --- | ---: | ---: | ---: | ---: |
| libstdc++ | 309 | 264 | 4.08 MB | 910 |
| libc++ | 1,000 | 768 | 5.02 MB | 5,768 |

The distinct bytes are similar.  What differs is that libc++ is 768 small
files including one another 5,768 times, and every one of those directives
names a file that is already guarded.  Our preprocessor honours `#pragma once`
(`once_files_` in `ProcessResolvedInclude`) but does not remember an
`#ifndef` guard, so each re-inclusion opens the file, reads it, and lexes it
to the `#endif`.  `strace` on the host-built binaries:

| | header opens | distinct | bytes read and lexed |
| --- | ---: | ---: | ---: |
| libstdc++ | 516 | 136 | 15.7 MB |
| libc++ | **4,794** | 605 | **68.3 MB** |

That is the whole of the front-end gap.  By area of the profile, in
gigacycles:

| area | gcc-built libstdc++ | clang-built libc++ | ratio |
| --- | ---: | ---: | ---: |
| lexer and cursors | 0.419 | **2.240** | 5.3x |
| macro processor | 0.071 | 0.098 | 1.4x |
| parser | 0.107 | 0.156 | 1.5x |
| semantic | 0.636 | 0.848 | 1.3x |
| lowering | 0.065 | 0.079 | 1.2x |
| LowIR optimizer | 0.185 | 0.209 | 1.1x |
| library, allocator, libc | 0.735 | 1.549 | 2.1x |
| kernel | 0.244 | 0.489 | 2.0x |
| other | 0.155 | 0.379 | |
| **total** | **2.595** | **5.945** | 2.3x |

Every other area scales with the 1.7x input, or with the allocation and page
faulting that more input brings (the library and kernel rows, at about 2x).
The lexer scales with the 4.4x bytes and then some: 5.3x the cycles for 4.4x
the bytes, because each of the 4,794 opens also pays to read the file and
restart the cursors.  `TranslationCursor::Next` alone is 0.895 G against
0.068 G.  The libstdc++ lane re-reads too (15.7 MB for 4.1 MB of distinct
headers), which is why A1 helps it as well.

### E3. Where the self-built libc++ lane's time goes

Same table, host-built against self-built, both libc++:

| area | clang-built | self-built | ratio |
| --- | ---: | ---: | ---: |
| lexer and cursors | 2.240 | 1.734 | 0.8x |
| macro processor | 0.098 | 0.110 | 1.1x |
| parser | 0.156 | 0.293 | 1.9x |
| semantic | 0.848 | 2.024 | 2.4x |
| lowering | 0.079 | 0.168 | 2.1x |
| LowIR optimizer | 0.209 | 0.422 | 2.0x |
| library, allocator, libc | 1.549 | **8.661** | **5.6x** |
| kernel | 0.489 | 0.423 | |
| other | 0.379 | 0.703 | |
| **total** | **5.945** | **14.897** | 2.5x |

The lexer is ours already: our code for it is faster than clang's.  The
parser, semantic, lowering and optimizer areas sit at the 2x the libstdc++
cell also shows and the residency plans already work on.  The library bucket
is where the cell differs: 8.66 G against 1.55 G, where the libstdc++ cell's
same bucket is 1.37 G against 0.74 G.  The two symbols at the top of it are
not library code at all by name, but their time is:

| symbol | self-built | share |
| --- | ---: | ---: |
| `std::vector<unsigned char>::vector(size_type, const value_type&)` | 2.844 G | 19.1% |
| `TypesContainLocalContext` (`semantic/templates/preemption.cpp`) | 2.290 G | 15.4% |
| `TranslationCursor::Next` | 0.903 G | 6.1% |
| `Lexer::Run` | 0.758 G | 5.1% |
| `AbiFactBuilder::UsesFunctionTemplateParameter` (`lowering/abi/mangling.cpp`) | 0.532 G | 3.6% |
| `operator==(const std::string&, const char*)` | 0.337 G | 2.3% |
| `MacroProcessor::AnnotateParentheses` | 0.153 G | |
| `MacroProcessor::AddSourceToken` | 0.149 G | |
| `std::string::push_back` | 0.106 G | |
| `MacroProcessor::Drain(std::deque<...>)` | 0.095 G | |

In the clang-built binary the first two are below 0.05 G combined, and in the
self-built **libstdc++** binary `TypesContainLocalContext` is 0.002 G.  The
function itself did not change; what changed is what the library asked the
optimizer to do.

### E4. The three shapes

`TypesContainLocalContext` allocates `std::vector<unsigned char>
visited(program.types.Size() + 1, 0)` on every call, walks a handful of types,
and returns; `UsesFunctionTemplateParameter` does the same.  A twelve-line
reproducer (`$HOME/perf-libcxx/vec.cpp`) compiled at `-O3` by the libc++
cell's compiler shows all three shapes, and `perf annotate` on the self-built
binary puts 99.7% of each hot function's samples on them.

**Shape 1: the empty destroy loop.**  libc++'s `~vector` runs
`__base_destruct_at_end`, which is `while (__new_last != __soon_to_be_end)
allocator_traits::destroy(--__soon_to_be_end)`.  For a trivially destructible
element `destroy` is nothing, and clang deletes the loop.  We keep it, with
the pointer in a stack slot:

```
1b3aaf:  ...
1b3ab6:  cmpq  %rdx, -0x1c0(%rbp)      46.7%
1b3abf:  movq  -0x148(%rbp), %rsi      13.7%
1b3aca:  movq  %rsi, -0x148(%rbp)      13.1%
1b3ad1:  jmp   0x1b3aaf                26.3%
```

That is 2.29 G cycles of doing nothing, once per element, on every
destruction of every `std::vector` of a trivially destructible type in the
compiler -- not only the two functions above.  libstdc++ never presents this
loop because its `_Destroy` is specialised away for trivial types before the
optimizer sees it.

**Shape 2: the fill loop.**  `vector(n, value)` is `__construct_at_end(n,
value)`, which is a per-element `construct` loop that clang recognises as
`memset`.  libstdc++'s `__uninitialized_fill_n_a` calls `__builtin_memset`
itself for byte types, so the libstdc++ cell hands us a call and the libc++
cell hands us a loop.  Ours stores one byte per iteration with the value
reloaded through its reference and the pointer round-tripping a slot:

```
11d:  mov   -0x88(%rbp),%rdx
124:  cmp   %rdx,-0x68(%rbp)
128:  je    14d
12a:  mov   -0x90(%rbp),%rcx
131:  movzbl (%rcx),%eax
134:  mov   -0x68(%rbp),%rcx
138:  mov   %al,(%rcx)
13b:  mov   -0x68(%rbp),%r8
13f:  lea   0x1(%r8),%r8
143:  mov   %r8,-0x60(%rbp)
147:  mov   %r8,-0x68(%rbp)
14b:  jmp   11d
```

2.84 G cycles, about 2.8 cycles per byte against memset's 0.03.

**Shape 3: the guard flag that stays a branch.**  libc++ wraps constructors
in `__exception_guard`, a stack object whose destructor tests a `bool` the
happy path set.  At `-O3` the test is still there with a constant operand:
`xor %eax,%eax; test %rax,%rax; je` and `mov $1,%eax; test %rax,%rax; je`.
The store reached the load, but the branch was never folded.  It is cheap in
cycles and expensive in code size, and it says the pass that forwards the
member store runs after the last pass that folds branches.

Underneath all three is the residency defect the residency plans own: every
loop-carried value above lives in a stack slot, so each loop costs three
to four times what its instruction count says.  This plan does not take that
on; it removes the loops libc++ hands us that should not be loops at all, and
leaves the loops that should be loops to `PLAN-HOT-LOOP-RESIDENCY.md`.

### E5. The third tier

After the shapes, what is left of the library bucket is libc++'s `std::string`
-- `operator==` against a literal at 0.34 G (already an inlined SSE compare;
it is the call count, since clang inlines the whole comparison),
`push_back` at 0.11 G, construction and destruction at 0.05 G each -- and the
growth path of `std::vector<Token>` at 0.07 G.  None is above 0.35 G on its
own; together they are about 0.6 G, and they matter only once the shapes
are gone.

## Model

For the profiled unit, in gigacycles, the plan predicts:

| | clang-built libc++ | self-built libc++ | ratio |
| --- | ---: | ---: | ---: |
| now | 5.94 | 14.90 | 2.51x |
| after A1 (guards remembered) | 4.1 | 13.5 | 3.3x |
| after B1 and B2 (the two loop shapes) | 4.1 | 7.5 | 1.8x |
| after B3-B6 (guard folding, strings, residency share) | 4.0 | 6.0 | **1.5x** |

The ratio gets *worse* after A1 alone, which is worth saying now so it does
not read as a regression when it happens: A1 removes the same absolute cost
from both binaries, and the self-built one has more of everything else.
Order of landing does not change the end state, and A1 lands first because it
is the largest single win in seconds and it makes every later measurement
cheaper.

The last row is the projection, not a promise.  It assumes that the
vector-destructor tax outside the two named functions is about 1 G, which is
inferred from the size of the library bucket rather than measured; L0 measures
it before B1 starts.

## Execution program

Each increment: reduce, place the fixture in the assignment that owns the
surface, fix, run that assignment's lane, run the cumulative gate, run the
performance gate, commit.  The performance gate is the workload above, in both
cells.

### L0. Baselines and the fast loop

- Put the workload benchmark in `scripts/` as a checked-in script taking the
  compiler path and level, so the numbers in this file can be reproduced by
  anyone; record the four-lane baseline with it in the ledger below.
- Keep the four profiled binaries.  The libc++ cell's `dev/cppgm++` is one
  shared path with the other cells and is silently replaced whenever another
  cell builds, so a copy per lane is the only stable reference.
- **The fast loop for Track B.**  Inception matches, so the objects the
  self-built compiler is linked from are exactly the objects the host-built
  libc++ compiler produces from `dev/src`.  To measure a codegen change on the
  self lane without a self build, compile the two hot translation units with
  the changed compiler, link them against the untouched
  `obj-clang-libcxx/pa39/selfhost` objects, and time the workload.  A full
  self build is the gate, not the loop.
- **Measure the destructor tax.**  Count, in the self-built binary, the empty
  loops of shape 1 across all functions (an objdump scan for a
  compare-decrement-jump cycle with no store) and sample their share.  This
  decides whether B1 is the 2.3 G the two named functions show or the 3.5 G
  the model assumes.

### Track A. The front end on libc++ input

**A1. Remember include guards.**  In `macro_processor.cpp`,
`ProcessResolvedInclude` already skips a file `once_files_` knows.  Add the
multiple-inclusion optimisation every production preprocessor has: when
`ProcessSource` finishes a file whose first directive was `#ifndef NAME` (or
`#if !defined NAME`), whose next directive was `#define NAME`, and whose
matching `#endif` was the last directive with nothing but whitespace and
comments after it, record `identity -> NAME` in a guarded-files table beside
`once_files_`.  On the next `#include` of that identity, if `NAME` is
currently defined, skip the read.  `ConditionalFrame` and
`SourceFrame::conditional_base` already give the depth needed to know the
`#endif` closed the first conditional.  Count the skips in
`PreprocessingStats` beside `skipped_once_includes`.

  Exit: the host-built libc++ compiler opens each libc++ header once per
  translation unit on `analyzer.cpp` (605, from 4,794), and its lexer area
  drops from 2.24 G to under 0.5 G.  The libstdc++ lane improves by a few
  percent for the same reason.  Preprocessor output is unchanged for every
  test in the tree; PA5 owns the fixture, which is a guarded header included
  twice with a macro defined between the inclusions so that a wrong
  optimisation would change the output.

**A2. Skipped groups are scanned, not lexed.**  If A1 leaves lexer cost in
the profile, the remaining source is conditional groups that are skipped:
libc++ guards whole sections on `_LIBCPP_STD_VER` and on `__has_feature`.  A
skipped group needs only directive lines found, not tokens formed.  Take this
only on evidence from the post-A1 profile.

**A3. Re-profile the macro processor.**  With the lexer quiet, the macro
processor's share on libc++ input becomes visible for the first time.  libc++
expands `_LIBCPP_HIDE_FROM_ABI` on every function and evaluates
`__has_builtin` and `__has_attribute` thousands of times per unit.  Measure
before deciding anything.

Exit for the track: T2.

### Track B. Code generation of libc++ shapes

All three passes live in `dev/src/lowir/optimize/`, use `discover_loops` and
`FunctionAnalysis::loop_forest()` from `lowir/analysis/function.h`, and sit in
`optimize_function_bodies` after `promote_slots` -- the loop must be in SSA
form with its phis before any of them runs -- and before
`simplify_counted_loops`.  PA37 owns the fixtures: a `.t` LowIR input with a
`.ref` of the expected output, in the O1 directory, since all three are O1
shapes.

**B1. Delete loops without effects.**  A natural loop whose blocks contain
only phis, arithmetic on loop-local values, comparisons and branches -- no
stores, calls, atomics, `copyobj`, EH instructions, or loads whose values
escape -- and which has a single exit is deleted.  Live-out values are the
ones the exit condition determines: for an equality exit `while (p != q)
step(p)`, `p` is `q` after the loop, which is the only closed form libc++'s
destroy loops need.  Loops with an inequality exit and no computable closed
form are left alone in the first landing.

  Termination is not proved.  The source language is C++, whose forward
  progress rule (N3485 1.10/24) lets an implementation assume a loop without
  side effects terminates, and that is the licence clang uses to delete the
  same loop.  Write that assumption in the pass header and in PA37's readme,
  because LowIR is also a contract students target.

  Exit: `TypesContainLocalContext` loses its loop; the reproducer's `~vector`
  is a compare and a call to `operator delete`; the destructor tax measured in
  L0 is gone from the self-built profile.

**B2. Recognise fill and copy loops.**  A loop that stores one value of a
loop-invariant operand through a pointer stepping by the store width, with
no other effects, is a fill of `(end - begin)` bytes; a loop that loads from
one such pointer and stores to another is a copy.  Emit the bulk operation
and delete the loop.  `native/lowering/bulk.h` already lowers a dynamic-length
copy through a `memcpy` symbol (`try_emit_preserving_dynamic_copy`), so the
question to settle first is whether the LowIR contract already has a
dynamic-length form for both directions or needs one; if a grammar change is
needed it moves with the pa13 readme and the `pa13.gram` validator, and the
lowering gets a PA38 fixture.  Byte fills first, since that is the measured
cost; wider splats (`std::vector<unsigned>(n, 0)`) second.

  Exit: the reproducer's constructor is an allocation and one bulk fill;
  `vector<unsigned char>::vector(size_type, const value_type&)` leaves the
  profile.

**B3. Fold the guard flag.**  Find why the `__exception_guard` test survives
`-O3` with a constant operand: whether the member store is forwarded to the
load after the last `simplify_values`, or the forwarding pass produces a
value `simplify_values` does not fold.  Fix the ordering or the fold.  Small
in cycles; it removes a branch and a dead cleanup path from every libc++
constructor.

**B4. Loop-carried values in slots.**  Not this plan's work.  Every loop
above pays 3x to 4x for slot-resident induction variables, and
`PLAN-HOT-LOOP-RESIDENCY.md` and `PLAN-INLINE-PARITY.md` own that.  This plan
records the libc++ loops as census entries for those plans and takes their
improvements as they land.  The 1.5x target assumes about 0.5 G from this
source on the profiled unit, which is the same share the libstdc++ cell gets
from the same work.

**B5. String shapes.**  After B1-B3, re-profile.  libc++'s `basic_string`
is a union with a short-string bit and every operation branches on it; the
0.6 G in E5 is mostly call overhead where clang inlines.  Whether the answer
is the inline policy (B6) or a specific fold is decided by the post-B3
profile, not now.

**B6. Inline policy for wrapper chains.**  libc++ reaches a leaf through six
to eight forwarding layers (`vector(n)` -> `__vallocate` ->
`__allocate_at_least` -> `allocator_traits::allocate` -> `allocator::allocate`
-> `__libcpp_allocate` -> `operator new`), where libstdc++ uses three or four.
`--inline-limit` exposes `caller_budget`, `single_call_limit`,
`single_call_caller_budget` and `hint_late_cap`.  Run the workload at two or
three settings of each in the libc++ cell **and** the libstdc++ cell; a
setting that helps one and costs the other is not taken.  This is an
experiment with a ledger entry, not a change to ship on its own.

Exit for the track: T1.

### Track C. What the support plan still owes, and this plan depends on

These are correctness items recorded in `PLAN-CLANG-LIBCXX-SUPPORT.md` under
its third run.  They are listed here because two of them gate this plan's
measurement.

- **C1. -O1 inception in the libc++ cell.**  The self-built `-O1` compiler
  reuses a caller-saved register across a call in
  `Program::EnsureVisibleName` (`%r8` live across `operator delete`).
  Reproducer and checker are in `$HOME/perf-libcxx/o1-bug/`.  Until it is
  fixed there is no `-O1`-built self compiler in the libc++ cell, so T1 at
  `-O1` is measured with the `-O3`-built one, and the ledger says so.
- **C2. The libc++ cell suite** is 5,541 of 5,555 with five causes
  (undefined `basic_string` constructor symbol under `extern template`,
  extern-template vtable reference, `>` inside parentheses in a template
  argument list, a duplicate object-symbol label in `wstring_convert`, a
  `<regex>` overload).  None blocks measurement; all block the support plan's
  completion criterion 2.
- **C3. The support plan's performance gates** were not run for its last
  three commits.  Run them before this plan's first landing so the baseline
  is attributable.

### Order of work

L0, A1, then B1 and B2 (in either order; B2's contract question is settled
before either lands), B3, then re-profile and decide B5/B6, with A2/A3 taken
if the post-A1 profile asks for them.  C1 is taken by whoever is in the
backend for B2, since both are native-lowering work; C2 is taken as its own
increments in the support plan's style.

## Testing requirements

- Every pass has a PA37 fixture that shows the transformation and one that
  shows it declining: a loop with a store is not deleted; a fill through a
  pointer that steps by the wrong width is not recognised; a loop whose
  induction variable escapes keeps its closed form.
- A1 has a PA5 fixture in which the optimisation, if wrong, changes output.
- The full tree is green in all three cells (`make test-cells`) and inception
  matches in all three at `-O3`, and in the default cell at `-O1`, after every
  landing.  No LowIR `.ref` is regenerated wholesale; a pass that changes
  reference output changes only the lines it is responsible for, checked by
  diff.

## Performance protocol

The support plan's protocol stands: `scripts/run_ab_compile_benchmark.py`
with `--repo-root`, aggregate CPU as the gate for full builds, cachegrind Ir
with `--vgdb=no` on the frozen translation unit, outputs under `$HOME`, and
the 1% CPU and 0.5% Ir thresholds for regressions.  This plan adds:

- The workload above, four lanes, both levels, recorded per landing.  T1 and
  T2 are read from it.
- The libstdc++ cell is measured on every landing, not only the libc++ cell.
  A change that helps libc++ and costs libstdc++ is not accepted without a
  written reason.
- The frozen-TU Ir lane is run for every optimizer landing, because B1 and B2
  change the compiler's own code too and that lane resolves what the workload
  cannot.

## Commit sequence

One commit per increment, in the repository's style: a bare imperative
subject and a body that says what was wrong and why the fix is shaped as it
is.  A fixture lands with the change it covers.

## Rollback rule

An increment that cannot be made green in the default cell is reverted, not
carried.  An optimizer change that moves a LowIR reference it did not intend
to move is reverted and re-approached.

## Non-goals

- Reshaping `dev/src` to avoid the shapes.  `TypesContainLocalContext`
  allocating a type-table-sized vector per call is wasteful in any library,
  and changing it would hide the codegen defect without fixing it.  The
  compiler's own sources are the workload, not the lever.
- Absolute wall-time parity between the libc++ and libstdc++ lanes.  See the
  objective.
- Vectorising the loops that remain.  A fill that becomes a bulk operation
  does not need it, and the loops that stay are the residency plans' work.
- Matching clang's inlining decisions.  B6 measures ours; it does not copy
  theirs.

## Completion criteria

1. T1: self-built over host-built in the libc++ cell at or below 1.50x at
   both levels on the workload, with the libstdc++ cell at or below its
   starting 1.55x.
2. T2: the host-built libc++ lane within 20% of the host-built libstdc++
   lane per preprocessed byte.
3. Every pass has its PA37 fixtures, A1 has its PA5 fixture, and every
   surface they test is described in the owning readme.
4. All three cells green and matching at `-O3`; the default cell matching at
   `-O1`; the libc++ cell matching at `-O1` once C1 lands.
5. The ledger below has an entry per landing with both cells' workload
   numbers and the frozen-TU Ir.

## Ledger

- **2026-09-04, baseline.**  Tree at `9c8c85d4`.  Workload: gcc/libstdc++
  18.970 / 19.373 s; self/libstdc++ 29.288 / 30.024 s (1.544x / 1.550x);
  clang/libc++ 44.360 / 44.597 s; self/libc++ 105.533 / 106.539 s
  (2.379x / 2.389x).  `analyzer.cpp` at `-O1`: 2.60 / 4.00 / 5.94 / 14.90 G
  cycles for the same four binaries; clang/libstdc++ host 2.64 s against
  gcc/libstdc++ 2.52 s.  Profiles, annotations and reproducers under
  `$HOME/perf-libcxx/`.  The self-built binaries are `-O3` products; the
  libc++ cell has no `-O1` self build (Track C1).

- **2026-09-04, `14c6b096` Answer a shape-only type trait without completing
  its class.**  Not a performance change: the frozen translation unit would
  not compile in the libc++ cell (`__is_const` asked of an incomplete class
  from inside `std::allocator`), and the frozen lane needs it.  Trait
  completion is now limited to the traits that look inside a class.  PA34
  fixture.
- **2026-09-04, `8884b08f` Skip a guarded header instead of re-lexing it
  (A1).**  `analyzer.cpp` header opens 516 to 157 (default cell), wall
  2.52 s to 2.12 s; frozen compile, gcc-built before against after, 3 ABBA
  blocks: wall -10.5%, user -11.4%, peak RSS -2.1%.  Host-built libc++
  workload: 44.36 s to 22.50 s at -O1, 44.60 s to 23.11 s at -O3.  Gold
  standard selfhost stage (libc++ cell, 32 jobs): 58.5 s to 23.9 s.  PA5
  fixture.  Default suite 5574/5574, inception MATCH.
- **2026-09-04, `a65a6936` Delete a pointer walk that does nothing (B1).**
  L0 measured the destructor tax first: 22.5% of the self-built libc++
  binary's samples sat in 9,633 empty slot-only loops, 17.0% in
  `TypesContainLocalContext` and 4.0% in `UsesFunctionTemplateParameter`,
  about 1.5% spread over every other destructor.  After landing: self-built
  libc++ workload 105.5 s to 44.0 s at -O1; `analyzer.cpp` self-built 14.90 G
  to 6.14 G cycles; **gold standard 1.818x** (inception 43.4 s over selfhost
  23.9 s, MATCH).  The licence is the pointer range walk (N3485 5.7/5), not
  forward progress, so the course fixture that keeps an unproven integer loop
  is untouched.  PA37 fixtures.  Default suite 5579/5579, inception MATCH.
- **2026-09-04, `9362ccaf` Turn a fill loop into one inline fill (B2).**  The
  fill becomes a call to a new builtin the backend lowers to `rep stosb`, on
  the pattern of the dynamic memcpy builtin, so it links freestanding.  Two
  things had to give before it fired on the real libc++ constructor: the loop
  takes its final shape only after the late inlining wave inlines
  `_ConstructTransaction` and promotes its object, so both loop rewrites now
  also run there; and promotion leaves a twin phi beside the walked pointer,
  which the matcher accepts.  After landing: self-built libc++ workload
  38.55 s at -O1 (ratio **1.717x**), 39.33 s at -O3 (**1.702x**);
  `analyzer.cpp` self-built 5.20 G against host-built 3.08 G (1.69x, from
  2.51x at the start); **gold standard 1.651x** (inception 39.5 s over
  selfhost 23.9 s, MATCH).  Frozen compile, B1 binary against B2 binary,
  default cell: wall +0.42%, user +0.35%, RSS +0.02% -- the late loop passes'
  compile-time cost, inside the 1% gate.  PA37 and PA38 fixtures.  Default
  suite 5586/5586, inception MATCH.
- **2026-09-04, position after B2.**  Remaining excess on `analyzer.cpp` is
  2.12 G of 5.20 G.  By area: library and allocator 2.09 G against 0.93 G
  (2.25x); semantic 1.37 against 0.88 (1.56x); LowIR optimizer 2.0x; parser
  2.0x; lexer 0.82x (ours is faster).  No symbol is above 2.4% any more.  The
  libc++ residue that is left: `std::string` comparison against a literal
  (0.09 G, fully inlined by clang), vector relocation loops
  (`__swap_out_circular_buffer`, `__emplace_back_slow_path`, about 0.07 G,
  the copy sibling of B2), and word fills with a runtime value
  (`resize(n, v)`, `vector<unsigned>(n, v)`, `vector<bool>(n, v)`, about
  0.08 G).  Everything else is the generic gap the residency plans own.

- **2026-09-04, `7b97d1e9` Keep a replayed index base live until its last
  replay (Track C1, first cause).**  The -O1 miscompile in
  `Program::EnsureVisibleName` was the backend retiring a replayed index's
  base at its last *direct* use: `extend_shared_storage_liveness` computed
  the last replay position into a side table that nothing read.  The base's
  `last_use` now advances to it, before the clobber and call facts are
  derived.  Two PA38 behaviour fixtures.  Default suite 5588/5588, inception
  MATCH at -O1 and -O3.  The reduced reproducers (`evn.lowir`, `min2.lowir`)
  compile with no caller-saved read after a call.
- **2026-09-04, `64ea98f4` Find the lowering builtins once per program.**
  B2's +0.907% frozen-TU Ir was the fill-builtin lookup: one more
  full-declaration scan per function beside the strlen and memcpy scans.
  The session finds all three once.  Frozen-TU Ir against the pre-B2
  binary: **+0.107%**, inside the 0.5% gate.  Default suite 5588/5588,
  inception MATCH.
- **2026-09-04, `83f44163` Operate in place only on a register (Track C1,
  second cause).**  With the first cause gone, the libc++ -O1 self build
  stopped in `native/mir/optimize.cpp` on "integer encoder expected a
  register operand": `binary_destination`'s `v op v` shortcut reused a
  frame-resident left operand in place, giving `sub [frame], [frame]`.  The
  shape is `bound - bound`, which B2's outside-use rewrite now produces from
  `pointer - bound`; the shortcut requires a register home on both paths.
  Default suite 5588/5588, inception MATCH.
- **2026-09-04, libc++ inception at -O1 MATCHES for the first time.**  Gold
  standard, libc++ cell, 32 jobs: **-O1 1.747x** (inception 41.61 s over
  selfhost 23.82 s), **-O3 1.651x** (39.46 s over 23.90 s).  Default cell
  for comparison, same script: -O3 **1.585x** (23.92 s over 15.09 s).  The
  libc++ cell's ratio is within 4% of the libstdc++ cell's; both are above
  the 1.5x target, and what separates both from it is the generic gap the
  residency plans own.  Absolute wall in the libc++ cell is 1.58x the
  libstdc++ cell's on the host-built stage, which is the 1.7x input volume.

- **2026-09-04, final position of this run (tree `d42fe85e`).**  Gold
  standard, 32 jobs, both cells at the same tree, all MATCH:

  | cell | level | selfhost | inception | ratio |
  | --- | --- | ---: | ---: | ---: |
  | clang / libc++ | -O3 | 24.08 s | 39.14 s | **1.625x** |
  | clang / libc++ | -O1 | 23.82 s | 41.61 s | **1.747x** |
  | g++ / libstdc++ | -O3 | 14.46 s | 23.13 s | **1.600x** |

  The libc++ cell started this plan at 2.4x and is within 1.6% of the
  libstdc++ cell.  Neither cell is at 1.50x: what remains is the generic
  gap the residency plans own, not a libc++ shape.  The support plan's owed
  performance gates for its last three commits (`516fe8c7` to `9c8c85d4`)
  were also run: frozen-TU Ir **+0.115%** (inside the 0.5% gate); paired
  wall +1.14%, user +0.73%, RSS +0.07% on the timing lane, which at this size
  reports noise the Ir lane does not.

- **2026-09-05, position at `ba29256c`, after the support plan's C10 fixes
  (fourteen suite failures, seven commits, no performance intent).**  Gold
  standard, 32 jobs, all MATCH.  CPU is user+sys over the stage; the wall
  ratio moves about 3% between identical runs (the libc++ -O3 stage's CPU
  is within 0.3% of the `d42fe85e` run that reported 1.625x), so the CPU
  ratio is the number to decide by and the wall ratio the one to report:

  | cell | level | selfhost wall | inception wall | wall | CPU |
  | --- | --- | ---: | ---: | ---: | ---: |
  | clang / libc++ | -O3 | 23.67 s | 40.02 s | 1.691x | 1.649x |
  | clang / libc++ | -O1 | 23.53 s | 41.76 s | 1.775x | 1.766x |
  | g++ / libstdc++ | -O3 | 14.49 s | 22.77 s | 1.571x | 1.552x |
  | clang / libstdc++ | -O3 | 14.91 s | 22.95 s | 1.539x | 1.517x |

  The libc++ cell's CPU ratio at `d42fe85e` was 1.647x: the C10 fixes cost
  nothing measurable.  The suites are 5596 / 5596 in all three cells.

- **2026-09-05, B6 screened and closed.**  One gold run per setting in
  the libc++ cell at `ba29256c`, `INCEPTION_EXTRA_CC_FLAGS` carrying the
  `--inline-limit` override into both stages (CPU ratio, baseline
  1.649x): caller-budget=1536 **1.640x**; once-cap=1024 with
  once-caller-budget=2048 **1.664x**; hint-late-cap=96 **1.674x**.  The
  budgets are not the constraint, and more inlining is a loss with this
  backend: the extra bodies cost more in our code than the calls did.
  Not taken.
- **2026-09-05, `71492145` Exempt inline functions from explicit
  instantiation declarations.**  Found from the -O3 profile: libc++'s
  extern template list names `basic_string::compare` (declared inline),
  its destructor and copy constructor (defined in class), and we
  suppressed them along with the non-inline members, so every string
  compare, copy and destroy in the self-built compiler was a PLT call
  into libc++.so (`operator==` 1.5% of cycles on `analyzer.cpp`, the
  destructor stub 0.5%).  N3485 14.7.2/10 exempts inline functions and
  clang inlines them.  Gold standard after landing, libc++ -O3: wall
  **1.664x**, CPU **1.662x** (inception user 1032.4 s against 1023.7 s
  before, selfhost unchanged) -- the bodies inlined into our own code
  cost 0.85% more than the calls did, the same lesson as B6.  Kept for
  conformance and for the interop it fixes (no more calls into the
  library for what the header defines), recorded as a small loss.  Two
  defects it exposed and that are fixed with it: the binding's
  inline_function flag is wider than the standard's inline (a
  constructor or destructor carries it for having a definition
  anywhere), which emitted libc++'s non-inline
  `basic_ostream::sentry::sentry` whose weak copy then interposed the
  library's own and crashed at exit -- a function fact records the
  specifier now; and `ed4013e9`: the primary template's out-of-class
  member definitions were replayed onto explicit specializations, so
  libstdc++'s `ctype<char>::id` was defined by every unit that reached
  the facet and the default cell's self build stopped linking.  Suites
  5598 / 5598 in all three cells.
- **Interposition note.**  The self-built copies of exported library
  members are default-visibility weak symbols, so the shared library's
  own PLT calls bind to them.  That is what gcc does with libstdc++ and
  it is safe as long as the copy is ABI-identical; it made the `sentry`
  defect visible because our constructor of a class taking a reference
  to a class with virtual bases carries the virtual-base pointer
  parameter contract (`virtual_bases.h`), an ABI of our own that the
  library's caller does not know.  Anything exempted must therefore be a
  true inline function, which the specifier fact now guarantees.

- **2026-09-05, `f31be3e4` Fill words with one rep stos (B2c), then
  `3bb58ff8` Keep the loop for a short word fill and walk a twin down.**
  A second builtin lowers 2-, 4- and 8-byte fills to `rep stosw/d/q`; the
  libc++ shapes reload the value through the element reference, so a fill
  wider than a byte is versioned on the source's position (outside the
  range, or on an element boundary inside it, where every iteration
  rewrites the element it read).  The first landing measured **worse**
  (CPU 1.677x): `vector<unsigned>::resize` went 0.037 to 0.083 G, because
  the compiler resizes tables by one element and `rep stos` starts in
  tens of cycles.  Versioning on the unit count too (the loop below
  sixteen units) took it back: gold at `3bb58ff8`, libc++ -O3 wall
  1.660x / **CPU 1.660x**; default -O3 wall 1.553x / **CPU 1.540x** (from
  1.552x -- the twin-phi walk B1 now deletes is in every vector growth).
  PA37 and PA38 fixtures.
- **2026-09-05, `2b6f349f` Forward a load through a phi of addresses.**
  The profile after the inline exemption showed `basic_string::compare`
  as our own copy running 4.6x slower than the library's: `std::min` on
  references is a conditional address (a phi of the operands' addresses
  over an empty diamond) and a reload through it, with the operands
  spilled to slots for their addresses; clang folds it to a select.  The
  new rule turns the reload into a phi of the values already loaded for
  the comparison, names the slots directly and re-promotes them:
  `std::min(a, b)` is a compare, a branch and a phi, and `compare` now
  inlines into `operator==`.  Runs in the body phase and after late
  inlining.  Suites 5603 / 5603 in all three cells.  Gold: see the next
  entry.

- **2026-09-05, gold at `aa316c05` (the address-phi forwarding, restricted
  to scalar loads after the self build stopped on an object-typed phi).**
  libc++ -O3 wall 1.649x / **CPU 1.660x**; default -O3 wall 1.552x / CPU
  1.546x; all MATCH, suites 5603 / 5603 in all three cells.  Neutral on
  the ratio: the LowIR gain is absorbed by the backend, which gives a
  two-arm merge phi a frame home (both arms store, the join reloads) --
  `location_planning.cpp` notes merge phis in registers were measured as a
  net dynamic regression, so `std::min(a, b)` is still a store, a store
  and a load where clang has a `cmov`.  A select-shaped lowering of an
  empty diamond to `cmov` is the open item this leaves.

- **2026-09-05, hoist a load through an invariant pointer out of a
  store-free loop.**  The front end's byte hash read the string's
  long/short flag and data pointer per byte; the hoister lifted only
  global and slot loads.  A loop that writes nothing leaves memory as it
  found it, so a pointer load that dominates the latches hoists.  Gold:
  libc++ -O3 wall 1.661x / CPU 1.661x; default 1.554x / 1.544x -- neutral;
  suites 5604 / 5604, MATCH.

- **2026-09-05, position at `889acc06`, end of this run.**  Gold standard,
  32 jobs, all MATCH, suites 5604 / 5604 in all three cells:

  | cell | level | selfhost | inception | wall | CPU |
  | --- | --- | ---: | ---: | ---: | ---: |
  | clang / libc++ | -O3 | 24.10 s | 40.02 s | 1.661x | 1.661x |
  | g++ / libstdc++ | -O3 | 14.52 s | 22.57 s | 1.554x | 1.544x |

  Against the run's start (`ba29256c`: libc++ CPU 1.649x, default
  1.552x) the libc++ cell is flat and the default cell 0.5% better.  On
  `analyzer.cpp` at -O3 the self-built libc++ compiler takes 5.19 s
  against the host-built 3.08 s (1.68x), unchanged across the six
  landings.  What each did, measured one at a time: the inline exemption
  +0.8% (our copies of `compare` and the string constructors are slower
  than the library's), the word fills +1.0% then -0.5% after the count
  threshold (the compiler fills one element at a time), the twin-phi walk
  and the two forwarding rules neutral.

  The reading that holds up: every remaining excess is the native
  backend's handling of merged values.  A two-arm phi gets a frame home,
  so a `std::min` that the optimizer now leaves as a compare and a phi
  is still two stores and a load; `vector<bool>`'s constructor lost six
  slots and gained phis in LowIR and runs slower for it.  The library
  bucket (1.80 G against 0.89 G on the unit) is that gap applied to
  libc++'s code, not a libc++ shape left unrecognised: the fills, the
  destroy walks, the relocation (already `memcpy` in libc++), the string
  comparison and the selections are all in the form clang gets.  Reaching
  1.5x is the allocator work `PLAN-INLINE-PARITY.md` names P30 (merge
  phis and spills placed by region, `cmov` for an empty diamond), and
  it would move both cells.

  Two libc++ items remain in this plan's own scope, both small:
  `vector<bool>`'s bit-iterator fill (0.04 G) and `operator==` against a
  literal, which clang inlines into the caller where `strlen` of the
  literal folds (0.07 G); the inliner's constant-actual discount is the
  place to take the second.

- **2026-09-05, method change and one mechanical landing.**  Sampling at
  2 kHz over a 5 s compile is noise at the 0.01 G level (the same symbol
  read 0.021, 0.043 and 0.028 G on three runs), so per-function reads now
  come from Cachegrind on `analyzer.cpp` at -O1 (deterministic; `rep`
  strings count per iteration, so fills are over-weighted).  That count
  puts the self-built compiler's exemption-era copies of `compare` at
  2.4x and of the copy constructor at 4.6x the library's instructions,
  and shows the six landings of this run net +1.6% instructions on the
  self-built compiler against +0.55% of extra work on the host-built one.
  Across the cells: the library bucket's excess is 1.02 G under libc++
  and 0.49 G under libstdc++ on the same unit, strings 0.36 vs 0.13 and
  vectors 0.36 vs 0.23 -- libc++ puts more of its library in headers,
  so more of it is our code.  Zero-extension after a byte-sized ALU
  operation (`load u8; and 1; zext`, libc++'s short-string test) is
  now omitted by the encoder: -0.105% instructions on the unit, 6,601
  static sites in the compiler; gold libc++ -O3 wall 1.629x / CPU
  **1.651x**, MATCH; suites 5605 / 5605 in all three cells.  Prologue
  shrink-wrapping was censused and dropped: no function's entry path
  avoids the registers it pushes, because parameters are homed in
  callee-saved registers at entry.

- **2026-09-05, two landings in the optimizer, both measured by the
  Cachegrind count.**  `0b8bf5cd` forwards a block-local store to its
  reload (the address may differ from the store's as long as both name the
  same slot or global; a store to a different slot or global keeps the other
  entries): self-built compiler 18.455 -> 18.324 G on `analyzer.cpp` at
  -O1 (-0.71%), host-built +0.13% of work.  Three PA37 controls counted
  loads to prove that no load is reused across a writing store and had to
  be re-pinned (`ab87fad6`): the forwarded load takes the stored value,
  which is the one shape reuse of the earlier load cannot produce.
  `e3a6650d` bypasses jump-only blocks in functions that hold phis:
  `cleanup_cfg` had stopped after the Boolean folds for every post-SSA
  function, so the diamonds two inlined empty callees leave on both arms
  of a branch (libc++'s annotation hooks: three in a row in the string
  copy constructor) and the jump-only continuation block every inlined
  call leaves survived to the backend.  An edge into a jump-only block is
  retargeted to its successor when the successor's phis can take the new
  predecessor; a diamond whose arms supply the same phi values folds to a
  jump; orphaned blocks are erased with their phi inputs.  The copy
  constructor goes from 40 blocks / 13 branches to 19 / 7; on the unit
  the optimizer emits 2.9% fewer instructions for 1.6% more of its own
  work.  Self-built 18.324 -> 18.158 G (-0.91%), host-built +0.18%.
  Gold at `e3a6650d`: libc++ -O3 wall 1.620x / CPU **1.626x** (from
  1.652x), default 1.514x / **1.529x** (from 1.541x), MATCH in both;
  suites 5607 / 5607 in all three cells.  Left in the copy constructor: a
  second test of the short flag inside the long arm, which
  `fold_edge_known_branches` misses because it reads only the immediate
  predecessor's branch; walking the single-predecessor chain is the next
  lever.  Method note: a stash-and-rebuild experiment leaves the pristine
  binary in `dev/`; rebuild before any measurement that follows.

- **2026-09-05, `f207fff9` Decide a branch from any single-predecessor
  ancestor.**  The edge-known fold read only the immediate predecessor's
  terminator; it now walks the single-predecessor chain (stopping at a
  landing pad and at the entry block), which removes the copy
  constructor's second test of the short flag inside the long arm (15
  blocks / 6 branches, from 19 / 7).  The walk exposed a latent
  miscompile in the one-step fold: an entry block whose only explicit
  predecessor is a loop back edge branching on the same parameter was
  folded as if the back edge had been taken (`@keep_entry_back_edge` in
  the new fixture returns 7 for every input at `0b8bf5cd`).  Neutral on
  the unit by the count (self-built 18.158 -> 18.155 G, host-built
  unchanged); gold libc++ -O3 wall 1.643x / CPU 1.633x, default 1.586x /
  1.534x, inception CPU -0.55% under libc++ but within the selfhost
  stage's 1% spread; MATCH; 5608 / 5608 in all three cells.  The
  post-inline memory GVN control's oversized shared callee, a chain of
  re-tests of one flag, now folds to a straight line; its steps test
  distinct comparisons instead so the control keeps its meaning.

- **2026-09-05, `b51c90c9` Fold constant terminals in functions with
  phis, and `f20a2263` Restore definition order before lowering.**  The
  terminal fold sat behind the phi gate too; libc++'s `max_size` is
  `std::min` of two equal constants and left `branch 0` with two empty
  arms in vector's `_M_check_len` / `_M_realloc_append` and string's
  `_M_create` (twelve on the unit).  The edge-aware fold computes
  reachability as if each constant terminal kept its selected edge,
  refuses when reachable code uses a value of the dead region, erases the
  region and drops the phi inputs of blocks that no longer reach.  Two
  coverage holes closed with it: the late-inline cleanup reruns the CFG
  pass after any of its dead-code passes emptied something, and a closing
  CFG pass at the end of the pipeline folds what the last simplification
  of a phase left.  Unit: 28,197 instructions in 6,417 blocks (from
  28,304 / 6,479 before the phi gate opened); self-built 18.155 -> 18.046
  G (-0.60%), host-built +0.08%.  The default cell's self-host then failed
  on `post_tokenizer.cpp` at -O3: the fold made a versioned fill's fast
  block, appended at the end of the function, dominate the loop before
  it, a later reuse took its `this + 2048`, and the native lowering (which
  visits blocks in list order) met the use before the definition.  The
  reader has the same rule.  The closing pass now checks definition order
  per function and puts a violating function into reverse postorder;
  only violators move, because block order also carries cold-path
  placement and a full reorder after every edit perturbed the inline
  heuristics of three large functions (+65 instructions on the unit).
  Gold at `f20a2263`, run alone: libc++ -O3 wall 1.637x / CPU **1.624x**,
  default 1.521x / **1.523x**, MATCH; inception CPU under libc++ 1107.0
  -> 1098.3 s across the two landings; 5610 / 5610 in all three cells.
  Process notes: `make test-cells` stops at PA38 and never runs the PA39
  self-host, so gold in both cells is part of every landing's validation
  from now on; a killed shell leaves its background chain running, so two
  chains measured concurrently once (discarded).

- **2026-09-05, `44c4f7aa` Merge a block into its sole jumping
  predecessor.**  Post-SSA functions never merged blocks, so every inlined
  call left its callee's entry and its continuation behind a jump: 952 of
  the unit's 6,417 blocks.  The phi-aware merge (single-input phis become
  copies, successor phis rename their input, landing pads and the entry
  stay) takes the unit to 27,155 instructions in 5,471 blocks, 1,070
  merges, optimizer time unchanged; two O3 threading passes that matched
  the unmerged layout now take the decision from the merge block too.
  Self-built 18.048 -> 17.902 G (-0.81%), host-built 9.718 -> 9.699 G.
  Gold at `44c4f7aa`, alone: libc++ -O3 wall 1.602x / CPU **1.609x** (from
  1.624x; inception CPU 1098.3 -> 1087.1 s), default 1.571x / **1.521x**;
  MATCH; 5611 / 5611 in all three cells.  Run total for the post-SSA CFG
  work (from `3e8f1fae`): self-built 18.455 -> 17.902 G (-3.0%), libc++
  CPU ratio 1.652x -> 1.609x, default 1.541x -> 1.521x.

- **2026-09-05, `45528be4` Remove dead slots in the closing pass.**  A
  census of the unit's output after the merge found 242 slots that were
  only ever stored (inlined by-reference arguments and `max_size`
  temporaries whose loads had been forwarded away); the dead-slot pass
  already treats them as dead but never ran after the last forwarding.
  Run in the closing pass: 26,909 instructions from 27,155, 413 slots
  from 659.  Neutral by the count (self-built 17.9023 -> 17.9029 G, host
  +0.01%) and by gold (libc++ -O3 CPU 1.610x, default 1.518x, MATCH):
  the stores were in cold code.  Retained as output hygiene at no cost.
  The -O3 output of the unit (the code the self-built compiler actually
  runs) censused clean after the CFG work: no constant branches, no
  same-value diamonds, no single-input phis, 2 merge candidates; what
  remains there is 79 different-value diamonds (selects), 510 critical-
  edge splits and 47 jumps into return-only blocks.  Two shapes were
  measured off the table by the parity plan and stay there: repeated
  constant-index and global-address computations (the backend
  rematerializes them, L49/L50) and the 1,308 pointer copies that carry
  the temporary-address contract for loads and stores.

- **2026-09-05, `a0238d75` Retry a busy local-phi register after the
  transfers.**  The time-sampled profile (hardware counters are
  unavailable on this VM) ranked the real excess after the CFG work:
  string-to-literal equality with its `compare` callees (~2.5% of the
  self-built compiler's time, kept out of line by the L35-closed EH
  front) and our own `Program::FindEntry` (~2% with `EnsureEntry`), whose
  small-scope loop keeps its `u32` counter in a frame slot.  The planner
  had placed that phi in a caller-saved register, but the walk found the
  register busy at the transfer and left the phi in the frame for the
  whole loop.  The walk now retries after the predecessor's transfers with
  the planned register or any free, unclobbered register of the local
  pool, moving the value from the frame on the entry edge alone.  Four
  units at -O3: 14 of 23 busy failures recovered; not FindEntry's, whose
  loop finds all four caller-saved registers held by parameters (the
  exclusive-region liveness the closed P30 program documented; a
  callee-saved home is refused by the unavoidable-header gate).  Count:
  self-built 17.9029 -> 17.8904 G (-0.07%), host unchanged; gold libc++
  -O3 wall 1.628x / CPU 1.611x, default 1.580x / 1.517x, MATCH; 5611 /
  5611.  Loop census of the unit at -O3: 103 of 107 loops carry frame
  homes, 3,240 of their 3,568 frame operands in call-bearing loops (the
  P30 residual); the largest call-free case is libc++'s `vector<bool>`
  constructor, whose bit-iterator objects survive as 16-byte slots that
  the aggregate split declines -- under investigation.

- **2026-09-05, `6df07c1d` Split object slots again in the closing
  pass.**  The aggregate split ran only after the first inlining wave and
  in the late-inline cleanup before that cleanup's store forwarding; an
  object slot whose address was stored into a by-reference argument slot,
  or passed to a callee inlined later, failed the split while those uses
  stood and was never revisited.  libc++'s bit iterators in
  `vector<bool>`'s constructor reached the backend as 16-byte slots, its
  fill loop shuffling them through the frame.  The closing pass now reruns
  the split, the small-object promotion and slot promotion: the
  constructor goes from 14 slots / 339 instructions to 2 / 214; its fill
  loop (72 instructions, 52 frame operands) becomes two 11-instruction
  loops with 6 frame operands each; the unit's call-free loops carry 298
  frame operands, from 328.
  analyzer.cpp -O1: 26,831 instructions from 26,909, about 2% more
  optimizer time on the unit.  Count: self-built 17.890 -> 17.844 G
  (-0.26%), host-built +0.13%; gold libc++ -O3 wall 1.628x / CPU
  **1.603x** (from 1.611x), default 1.540x / **1.514x**; MATCH; 5612 /
  5612 in all three cells.  Retained.

- **2026-09-05, folded copies of exception-bearing callees (the string
  equality against a literal).**  Clang's path, traced on this host with
  `-Rpass=inline` and `-print-changed`: `compare` inlines into
  `operator==(const string&, const char*)` (cost 110 of threshold 325),
  that inlines into the caller with its terminate landing pad intact (185
  of 325), and only then InstCombine, CorrelatedValuePropagation and
  SimplifyCFG in the caller fold `strlen("abc")`, kill the `npos` test
  and the throw block, and drop the pad: a size select, a compare with 3,
  a data select and `bcmp`, twenty instructions.  Ours refuses any callee
  with an EH instruction before the literal's length can flow in
  (`inline_reject_callee_eh`), and the strlen fold of the pipeline runs
  after every inlining wave, so no wave ever sees the constant.
  The inliner now retries such a call from a per-site copy of the callee
  with the call's constants substituted (an integer literal, or the
  `addr` of an internal readonly byte string): strlen, value
  simplification and CFG cleanup fold it; a call inside the copy that
  still can unwind and takes constants is folded and spliced first, to
  depth two -- in a unit where `compare` is not inlined into `operator==`
  (it has several callers there) the throw is reached only that way;
  dead regions are stripped and the copy is spliced when nothing
  exceptional remains.  The main wave never succeeds on this shape (the
  copies still move their parameters through slots there), the late wave
  does.  With it, the unsigned-zero compare fold the throw predicate
  needs (`0 >u size` survived unfolded in every string function).
  PA37 fixtures 547 (literal and integer cases with non-constant
  controls) and 548; README rules for both.  A fixture lesson recorded
  in memory: LowIR `unreachable` is undefined continuation, so a throw
  block written `call @__cxa_throw; unreachable` loses its throw path.
  Measurements, each a full `make test-cells` (5613 / 5613, then 5614 /
  5614 with the second fixture), gold in both cells, and Cachegrind on
  the self-built and host-built libc++ compilers over analyzer.cpp -O1
  (baseline 17.844 G self / 9.713 G host, `operator==` 461.9 M = 2.6%):
  * copy cap 64 (the ordinary hint cap +16): no hot site folds -- the
    real unit's copy is 66 instructions, the size diamond twice, three
    minimum diamonds and the result merge, where clang's select form is
    20; self 17.864 G (+0.11%), host +0.11% (the attempts), `operator==`
    unchanged; gold libc++ 1.579x / CPU 1.581x, default 1.501x /
    1.512x -- inside the stage's noise, as the count shows.
  * copy cap 80: the hot sites fold; `operator==` 461.9 -> 245.4 M, the
    strlen calls gone; self 17.796 G (**-0.27%**), host +0.32%; text
    +2.85%; gold libc++ 1.626x / CPU **1.602x**, default 1.547x /
    1.511x, MATCH.  Neutral on CPU.  The remaining out-of-line literal
    sites are string switches (36 in `ClassifyOperator`, 30 in
    `ResolveFunctionalCastType`) where the 768-instruction caller budget
    admits about eleven 66-instruction copies.
  * copies charged to their own 2,048-instruction caller budget (landed):
    `analysis.cpp` folds 53 of its sites instead of 29, `calls.cpp` 55
    instead of 24; `operator==` 461.9 -> 215.6 M; self 17.778 G
    (**-0.37%**, -0.7% net of the attempts the host-built compiler also
    pays at +0.33%); text of the self-built compiler +3.9%; gold libc++
    -O3 wall 1.593x / CPU **1.594x** (from 1.603x; inception user 996.9 s
    against 995.2 s, the host stage +0.4% from the attempts), default
    1.516x / **1.515x**, MATCH; 5614 / 5614 in all three cells.
  Retained as a small real gain by the count, with its size cost on
  record.  What it did not reach: the folded body is still three times
  clang's, and that is the whole difference between -0.37% and the 2.6%
  the symbol cost.  Three midend collapses would shrink it to clang's
  shape and cut the text growth with it: value numbering across
  identical diamonds (libc++ recomputes `__is_long()` for `size()` and
  `data()` in every string function -- the loads merge late, the
  branches never), the unsigned-minimum identity on a diamond
  (`x <u -1 ? x : -1` is `x`, which is how `std::min(n, npos)`
  disappears for clang), and the compared value known on a branch's
  edge (`size == 3` on the equal edge makes the last minimum a
  constant).  Those are general, not equality-specific, and are the next
  lever this direction names.

- **2026-09-05, the three midend collapses (closing O3 pass).**  Built as
  the previous entry named them, all in `boolean_cfg.cpp`, run together in
  the closing pass after the last load reuse until nothing folds:
  * value numbering across diamonds (`value_number_diamonds`): a diamond
    whose condition is the same computation as a dominating diamond's --
    the same value, or the same pure shape over agreeing operands through
    `copy`, loads agreeing only when nothing that may write memory lies on
    any path from the earlier load's block to the later parent -- branches
    on the earlier condition, and each phi whose arm inputs are the same
    computations as an earlier phi's becomes that phi.  Positional arm
    matching was the first attempt and failed on libc++'s shape, whose
    two size arms differ by one `copy`; the value-chain form does not.
  * select identities (`fold_diamond_select_identities`): a phi over two
    jump-only arms choosing between its comparison's operands is one of
    them when the comparison fixes the other -- unsigned minimum with the
    type's largest value, unsigned maximum with zero, either operand of an
    equality.  `std::min(n, npos)` is the first.
  * the edge-known equality (`propagate_edge_integer_equalities`, which
    existed for the O3 GVN follow-up) now also runs in the closing loop,
    so the length a size check fixes reaches the minimum and compare it
    feeds.
  A dead store into the folded copy's by-reference argument slot read as a
  change of memory between the size and data diamonds; dead slots are
  removed before the loop.  On the analyzer unit's equality site the
  folded body goes from 66 instructions to 35 including the caller's
  merge, clang's structure: the size diamond once, the length check, the
  data diamond on the first condition, `memcmp` with the constant length.
  Unit outputs at -O3: analyzer.cpp 24,965 -> 24,829 instructions,
  declarations/analysis.cpp 27,191 -> 25,913 (-4.7%), expressions/calls.cpp
  20,871 -> 19,342 (-7.3%).  PA37 course O3 fixtures 570 (a repeated size
  diamond with a pure use between, a data diamond after a call that keeps
  its own reload, a store between that blocks the numbering) and 571 (the
  four identities and the length-check chain); README paragraph.  No
  existing reference moves in any lane; one control moved: 523's
  `retain_non_boolean_forwarded` chose between `%value` and 0 under
  `value != 0`, which is `%value` on both edges and now folds, so its
  short arm yields 2 to keep testing that the O3 forwarding pass leaves a
  non-Boolean forwarded phi alone (its behaviour check, 256 -> 0, is
  unchanged).
  Measured (test-cells 5616 / 5616 in all three cells, gold in both
  cells, Cachegrind over analyzer.cpp -O1): self-built 17.778 -> 17.715 G
  (**-0.35%**; -0.72% from the day's start), host-built unchanged
  (-0.002%: the collapses cost no compile time); text of the self-built
  compiler 9.19 -> 9.05 MB (-1.6%, +2.3% from the start); gold libc++ -O3
  wall 1.628x / CPU 1.599x (from 1.594x; inception user 995.3 s against
  996.9 s), default 1.555x / 1.519x (from 1.515x), MATCH.  Retained: the
  count moves by the oracle and the text comes back, the CPU ratio stays
  inside the stage's spread as it has for every LowIR-only landing since
  the allocator program closed.  The remaining `operator==` cost (215.6 M)
  is the out-of-line string switches beyond the copy budget and callers
  with non-literal arguments; the folded body itself is now at clang's
  structure.

## Closed (2026-09-05)

Closed at `5cdc5786`.  Where it ends: libc++ -O3 gold wall 1.628x / CPU
1.599x (from 1.652x / 1.647x when the plan opened), default 1.555x / 1.519x
(from 1.541x / 1.521x); self-built libc++ compiler on analyzer.cpp -O1
17.715 G instructions (from 18.455 G at the first oracle run); suites
5616 / 5616 in all three cells; MATCH everywhere.  The target below 1.5x is
not met.

What the plan established, for whoever reopens it:

- The Cachegrind count on the self-built compiler is the oracle that moves
  with LowIR work; gold CPU has stayed inside the stage's spread for every
  LowIR-only landing since the allocator program closed, because the
  backend gives merged values and selects frame homes.  The ratio lever is
  therefore in the backend (P30's open select lowering and merge-phi
  register homes), not in more midend.
- The post-SSA CFG cleanup, the folded copies of exception-bearing callees,
  and the three diamond collapses are complete for the shapes libc++
  hands us; what remains out of line is string switches beyond the copy
  budget and callers with non-literal arguments.
- The preprocessor re-lexing of guarded headers (the 68 MB per unit in the
  opening decomposition) is the untouched half of the host-compiler gap and
  is independent of the optimizer.
