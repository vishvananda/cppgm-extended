# Allocation-Time Optimization Plan

## Goal

Reduce compile time lost to allocator traffic, especially repeated `malloc` and
`free` in semantic/output compilation. Lower retained memory is useful, but the
primary goal is less allocator CPU time and fewer transient heap objects.

The host is often loaded, so wall time alone is not a reliable keep/reject
signal. Treat wall time as supporting evidence and rely on paired A/B runs with
allocator-sensitive secondary counters.

## Measurement Strategy

Use three layers of evidence:

1. Paired benchmark runs on the stable self-compile input:

   ```sh
   /usr/bin/time -lp ./dev/cppgm++ -I dev/src -c \
     -o /tmp/<label>-semantic_overload.o \
     benchmarks/self_compile/stable/semantic_overload.cpp \
     > /tmp/<label>.stdout 2> /tmp/<label>.stderr
   ```

   Record `real`, `user`, `sys`, `instructions retired`, `cycles elapsed`,
   `maximum resident set size`, and `peak memory footprint`. Prefer repeated
   nearby A/B/A runs over isolated samples.

2. Compiler-internal allocation proxies:

   - `CPPGM_CALLSEM_CONSTRUCTION_CENSUS=1` for retained/generated `CallSemNode`
     construction volume by phase and kind.
   - `CPPGM_MEMORY_CENSUS=1` for retained structure size and hot retained
     containers.
   - `CPPGM_SEMANTIC_PHASE_STATS=1 CPPGM_SEMANTIC_STATS=1` to tie allocation
     candidates back to semantic phases.

3. System allocation profiling when the local machine is quiet enough:

   - Instruments Time Profiler or Allocations, filtered to `malloc`, `free`,
     `operator new`, and `operator delete`.
   - `sample <pid>` during the long stable compile, looking for allocator frames
     immediately below compiler functions.
   - macOS malloc diagnostics only for focused runs; they perturb allocator
     behavior too much for keep/reject timing.

Keep a patch only when strict LowIR compare passes and the paired evidence shows
one of:

- lower `sys` time with flat or lower retired instructions,
- materially lower max RSS or peak footprint with flat wall/instructions,
- fewer measured transient allocation events in an instrumented path and no
  stable benchmark regression.

Reject patches that only look locally cheaper but raise retired instructions.
Recent output-seed work repeatedly showed that extra hot-path branches can cost
more than the allocation they avoid.

## Candidate Order

### 1. Avoid Duplicate Text-Interner Node Allocation

The global text interner backs `CallSemText` and several template lookup keys.
The old `text_intern::intern(std::string)` called `unordered_set::insert`
directly. On some library implementations this can allocate a candidate node
before discovering the string is already interned, then immediately free it.
That is exactly the kind of transient allocator traffic this plan targets.

Experiment:

- Probe the interner with `find` before insertion.
- Move the temporary string into the set for `intern(data, len)` misses.
- Temporarily add interner stats to dump duplicate hit/miss counts at the end of
  semantic analysis.

Result: rejected. A direct libc++ probe showed duplicate
`unordered_set<string>::insert` does not allocate/free an extra node on the
current Homebrew toolchain, so the find-before-insert patch only adds lookup
work on this host. The stats run confirmed many duplicate interner calls, but
that no longer implies duplicate allocator traffic here.

Validation:

- `make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++`
- strict LowIR compare
- stable self-compile A/B with and without temporary interner stats

### 2. Reserve Known Child Counts During CallSem Construction

Many output nodes know their child count before appending parameters, bases,
captures, vtable entries, or generated constructor/destructor actions. Add small
local `reserve()` calls only where the count is already available and the vector
is append-only in that block.

Avoid broad helper abstractions until census points at a repeated pattern. A
reserve that needs extra scans or branch-heavy shape analysis is likely to lose.

Implemented:

- `semantic_output.cpp::emit_function_variant` reserves
  `binding.params.size() + 1` children for function definitions before appending
  parameter nodes and the body node. This is exact for ordinary function output
  and avoids parameter/body vector growth for multi-parameter functions.

Rejected follow-up:

- Reserving compound/body statement vectors from AST child counts passed strict
  comparison, but the stable benchmark sample moved `sys` time and peak memory
  the wrong way (`sys=4.33`, peak `959418368`), so it was backed out.

### 3. Reuse Short-Lived Scratch Vectors in Recursive Walks

Hot recursive helpers repeatedly create temporary `vector<const CallSemNode *>`
or `vector<...>` scratch containers. Convert selected walkers to thread a
caller-owned scratch vector only when the vector is cleared and reused in a
simple depth-first pattern.

Do not share scratch globally; the semantic pipeline is already complex enough
that global scratch state risks reentrancy bugs.

### 4. Remove Temporary String Construction Before Lookup

Look for paths that build formatted strings only to probe a map/set/cache:

- mangling substitutions,
- function output names,
- source-location strings,
- template argument keys,
- qualified-name fallback text.

Prefer structural or atom-key lookup over string materialization. This has been
one of the most reliable historical wins, but it must be validated path by path.

### 5. Compact Rare CallSem Payload Allocation

`CallSemNodeExtra` currently uses multiple `shared_ptr` subobjects for rare
payload, rare strings, symbols, and qualified-name syntax. A future pass can
reduce allocator traffic by:

- replacing shared ownership with value or `unique_ptr` where copy-on-write is
  not buying much,
- grouping rare strings/payload into one allocation for nodes that need both,
- interning common symbol payloads more aggressively.

This is higher risk because `CallSemNode` copies are common and observable
through output/witness generation. Do it after census identifies the retained
and constructed-node distribution.

### 6. Arena Only For True Analyzer-Lifetime Objects

Do not add a broad arena allocator first. It can reduce `free` time but hide
lifetime bugs and increase footprint. Consider monotonic storage only for
objects that already live for the whole analyzer run, such as durable semantic
model records, and only after the retained-memory census shows enough volume to
justify the complexity.

## Current Status

The surviving implementation is the exact function-definition child reserve in
`semantic_output.cpp`. It is intentionally narrow: it removes a small source of
`vector<CallSemNode>` growth allocation without changing semantic ownership,
node lifetime, or output shape.

Rejected experiments:

- Text-interner find-before-insert. Strict comparison passed, but a direct
  libc++ probe showed duplicate insert does not allocate/free a discarded node
  on this host. Stable samples were neutral to worse in instructions.
- Recursive scratch-vector visitor for required-callee scans. Strict comparison
  passed, but the stable benchmark regressed (`338651199951` instructions,
  `sys=4.74`, peak `959971328`).
- Broad compound/body statement reserves. Strict comparison passed, but the
  stable benchmark regressed allocator-adjacent proxies (`sys=4.33`, peak
  `959418368`).

Final validation for the surviving patch:

- Build:
  `make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- Strict LowIR compare:
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-nobuild CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed for pa18, pa19, pa21, and pa22.
- Stable benchmark samples for function-reserve-only:
  - `/tmp/alloc-reserve-semantic_overload.stderr`: `real=93.24`, `user=88.57`,
    `sys=2.83`, RSS `1212469248`, instructions `339311804652`, peak
    `955559936`.
  - `/tmp/alloc-reserve2-semantic_overload.stderr`: `real=167.13`,
    `user=131.17`, `sys=3.40`, RSS `1236418560`, instructions `337136857147`,
    peak `956010496`.

The benchmark host was visibly noisy, so this is recorded as a small allocator
cleanup with acceptable validation, not as a proven large compile-time win.
