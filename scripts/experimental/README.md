# Optional WASI and browser inception

This is an experimental maintainer workflow. It does not add a student assignment,
change the PA37 report, or ship in the student export. Run from the solution
checkout on the `experiment/wasi-browser-inception` branch.

The native compiler builds W1. W1 then compiles all 416 frozen compiler sources
and links W2; W2 repeats that work to produce W3. Every corresponding object and
final module must match **actual bytes**, without stripping or normalization.
The same W1 runs unchanged under Wasmtime and in a dedicated browser worker.
The browser performs both complete rebuilds locally; the HTTP server only serves
files. It also checks x86 floating-point output and accepted/rejected programs.

## Dependencies and preparation

Build the native tools with one consistent host toolchain first:

```sh
make -j8 CXX=g++ CPPGM_HOST_CXX=g++
```

Qualification uses WASI SDK 34.0 (Clang 23.1.0), Wasmtime 50.0.1 and
`@bjorn3/browser_wasi_shim` 0.4.2. Supply an extracted SDK and shim package;
the runner does not download dependencies. Keep them and the output package
outside the repository. The shim's MIT and Apache licenses accompany the package.
The optional automated browser driver additionally needs Playwright and installed
Chromium/Firefox executables; those are not compiler build dependencies.

```sh
python3 scripts/experimental/wasi_inception.py prepare \
  --directory /absolute/path/inception-package \
  --sdk /absolute/path/wasi-sdk \
  --shim /absolute/path/browser_wasi_shim-package \
  --x86-headers

python3 scripts/experimental/wasi_inception.py build \
  --directory /absolute/path/inception-package \
  --wasmtime /absolute/path/wasmtime --jobs 6

python3 scripts/experimental/wasi_inception.py x86 \
  --directory /absolute/path/inception-package \
  --wasmtime /absolute/path/wasmtime --jobs 8
```

`prepare` requires a new directory. It freezes sources, generated target
configuration, SDK headers/runtime archives, host x86 headers, the native
compiler, orchestration files and browser shim with SHA-256 hashes. The default
output target stays x86-64; its predefined macros are preserved, and its include
paths are relocated inside the package. WASI compilation passes an explicit
target and sysroot. Inputs are verified before and after each local operation.

`prepare --optimization 0` selects an unoptimized baseline; the default is `-O3`.
`build --stages 1` prepares only W1; `build --stages 2 3` continues from it. An
existing stage directory is an error, including a partial stage. Use a new package
after source/toolchain changes. Wasmtime's precompiled cache is produced locally
from that stage's module, never downloaded. Successful `wN-complete.json` files
contain every command, diagnostic-log path, output size/hash and elapsed time.
The cumulative child-process RSS includes Wasmtime compilation and execution.

`x86` compares native-host and WASI-host ELF objects for every compiler source
and a deterministic floating-point corpus at `-O0` and `-O2`. The corpus covers
NaN signs/payloads, signed zero, infinities, subnormals, rounding boundaries,
128-bit integer conversion and seeded finite x87 arithmetic. `--levels 0 1 2 3`
selects all optimization levels. All observations are retained; any failure
prevents creation of `x86-complete.json`.

## Browser run

Complete W1 and the x86 comparison before starting the page. Serve the package:

```sh
python3 -m http.server 8000 --bind 127.0.0.1 \
  --directory /absolute/path/inception-package
```

Open `http://127.0.0.1:8000/wasi_browser.html` and select **Start fresh run**.
The worker verifies the manifest and inputs, then creates fresh compiler
instances for each translation unit and link invocation. It reuses each compiled
WebAssembly module and keeps an in-memory filesystem with read-only inputs.
The page shows progress and publishes the final module and JSON observations
only after all comparisons pass. **Cancel** terminates the worker and discards
partial work; restart begins a fresh run. An external 60-minute deadline bounds
the run. This is a time limit, not an instruction/fuel bound.

The automated browser driver checks cancellation/restart, complete W2/W3 builds,
x86 floating output, and the downloaded final module:

```sh
node scripts/experimental/wasi_browser_test.cjs \
  /absolute/path/inception-package /absolute/path/playwright-core \
  /absolute/path/chromium /absolute/path/firefox
```

An optional final argument, `chromium` or `firefox`, selects one engine. Separate
invocations can run concurrently. Browser binaries, package downloads and test
results stay outside Git. No cross-origin isolation or shared guest memory is
required. The engine must support the module's modern exceptions and SIMD128;
compiling the actual seed checks that support. Safari has not been qualified.

## Compiler host profile

The seed uses C++11, `-O3`, disabled telemetry and no POSIX test runner. It links
the SDK's exception-enabled libc++, libc++abi, libunwind, libc and compiler-rt
with the compiler's own Wasm linker. `wasi_host.h` selects the single-thread
libc++ header profile and maps the SDK's `_Thread_local` errno declaration to
its qualified ordinary data symbol. This does not add general TLS or atomics.
The SDK files remain unchanged.

`-Wl,-z,stack-size=8388608` reserves an 8 MiB linear-memory stack; the compiler's
WASI recursion budget is 4 MiB. Wasmtime is also given an 8 MiB engine-stack
limit. Browser engine stacks are independent of the linear stack. The reported
browser peak is guest linear memory, not total browser process memory.

Most host portability changes are guarded by `__wasi__`: resource APIs,
optional textual-IR storage and exact x87 conversion/arithmetic on the WASI
host's binary128 `long double`. Native/WASI code identity also required fixed
identifier alignment and deterministic ordering for equal register live ranges
and spill moves. Target-layout and SDK frontend fixes have independent normal
regression fixtures. The optional browser workflow remains a maintainer choice.

## Qualification and performance

The earlier shared-backend qualification used a shared Linux machine with two
Xeon E5-2696 v4 processors. All 413 corresponding objects and the final module
matched through W1/W2/W3, both locally and in Chromium 153.0.8010.12. The final
module was 15,508,798 bytes, SHA-256
`fd03a82e3d5b401dbc53cc8be349689b59338eed7f6f17b5d34936de40a5142c`.
Native/WASI x86 output matched for all 1,242 comparisons at O0/O2/O3, including
the floating-point corpus. Browser cancellation, restart, final-module download,
x86 floating-point checks and accepted/rejected program checks passed.

| Build | Observed elapsed time |
| --- | ---: |
| Native x86 compiler produces W1, six jobs | 25.3 s |
| Wasmtime W1 produces W2, six jobs | 51.0 s |
| Wasmtime W2 produces W3, six jobs | 49.8 s |
| Chromium W1 produces W2, serial worker | 507.0 s |
| Chromium W2 produces W3, serial worker | 375.3 s |

Local stage times exclude Wasmtime AOT preparation. The full browser run took
894.4 s, including setup and final checks; peak guest linear memory was
144,048,128 bytes. The host was shared, so these are approximate timings, not
isolated throughput guarantees. Full Firefox inception remains unqualified;
the earlier optional run was canceled. Safari was not tested.

A separate comparison compiled the same frozen compiler sources with SDK Clang
and cppgm, both at O3. **Both resulting WASI programs run cppgm's internal linker**
on the same frozen inputs; this does not compare cppgm's linker against LLD.
Locally prepared Wasmtime AOT modules, one pinned CPU, A/A calibration and four
ABBA blocks gave median link times of 2.71 s for the cppgm-built compiler and
1.78 s for the Clang-built compiler, a paired Clang/cppgm time ratio of 0.658.
Every output matched actual bytes. One A/A link block varied by 18%; the four
paired link ratios were 0.612–0.659. Source-compilation timings were too noisy
for a general throughput claim. Observations, commands, hashes and calibration
noise remain in the external experiment package.

Bulk input reads, typed opcode validation and flat relocation-field storage
reduced cppgm-built WASI linking from 6.79 s to 3.66 s on identical frozen
inputs, a median paired time ratio of 0.537. Native linking also improved,
from 1.35 s to 0.94 s. These comparisons retain all warmups and paired blocks.

The shared backend planning below further reduced WASI linking on the previous
frozen inputs from 3.66 s to 2.67 s, a median paired time ratio of 0.732 (about
27% less elapsed time). Native linking was 0.854 s versus 0.865 s, with a paired
ratio of 1.012 inside the A/A variation. Three native compilation probes had
paired ratios of 0.999, 0.970 and 0.984. Their object bytes were unchanged.

## Dispatcher regression

The earlier 126–154 s linker had a code-generation pathology. Its object decoder
expanded to 1,291 LowIR blocks. General Wasm control-flow lowering selected the
next block by scanning block IDs on every transition. A dense `br_table` now
performs that dispatch. The frozen before/after sources differ only in
`wasm/selector.cpp`, with identical build flags and unchanged linker sources.
A later diagnostic replay linked identical objects in 116.5 s versus 6.35 s,
with identical module bytes; this pair is a reproduction, not a repeated
performance estimate.

A reduced cycle with multiple entries executes 1,000 steps. Increasing its
block count from 16 to 64 to 256 raised the old engine fuel requirement from
53,137 to 148,241 to 528,401; the fixed version uses 17,039 in all three cases.
The control fixture `300-irreducible-dispatch-work` checks results and a generous
dynamic fuel bound, without prescribing an instruction sequence. The x86 backend
uses direct block branches and did not have this dispatcher pathology.

## Shared backend planning

The `machine` layer now supplies stable switch-case clustering and a bounded
density predicate to both backends. Wasm emits dense tables or balanced range
tests; native keeps its existing table, bit-test and tree policies. Signedness,
64-bit bounds before index narrowing and PHI edge transfers remain explicit in
the emitters. Target discovery uses a hash index for large switches, avoiding a
quadratic scan over distinct destinations.

Both backends also share the call-plan cache lifecycle, indexed PHI predecessor
lookup, conservative object-copy lifetime facts and semantic builtin boundary
checks. Wasm computes variadic classification/storage once for frame planning
and emission. Target ABI classification, physical PHI move scheduling, costs and
instruction selection stay in their existing backends.

Object storage sharing requires a complete copy and one typed by-value call in
one block with disjoint intervals. Escapes, partial writes, exceptions, dynamic
stack allocation and inline assembly pin the storage. Wasm additionally reuses
outgoing aggregate call storage for functions without exception handling. The
reduced 8/32 sequential 64-byte call probes now use constant 192-byte Wasm frames
and 144-byte native frames, versus 1,088/4,160 and 592/2,128 bytes respectively.

Eligible Wasm i128 temporaries use two i64 locals. Address users, loads, call
results and unsupported operations retain memory storage; the public ABI stays
unchanged. The reduced `(x+y)^(x-y)` probe has no frame and two return stores,
versus a 128-byte frame and 40 static loads/stores previously.

PA37 fixtures check dense-switch fuel, sparse and high-bit selectors, PHI
transfers, simultaneous aggregate arguments, recursion and wide arithmetic.
Validation also fixed worker cleanup overwriting a checker's failing exit status.
WASI values reserved by Wasmtime are checked with a host asserting the exact
`proc_exit` argument, preserving guest code and ordinary host exit checks.

General local live-range allocation is unchanged. Individual contributions of
these shared planning refactors to the measured link improvement have not been
isolated.

## Local dispatch regions

The decoder contained a four-block cycle with two entries. Although its other
loops were reducible, that cycle prevented topological ordering and forced all
797 reachable blocks through a whole-function dispatcher. It accounted for most
of the remaining execution gap against the Clang-built worker.

Wasm control planning now condenses such cycles into local dispatch regions.
An iterative SCC walk runs only when the ordinary topological ordering fails.
Each local dispatcher contains at most 32 children; the function-wide budget is
128 dispatch children. A child can be a basic block or a complete natural loop.
Surrounding acyclic code and natural loops stay structured, and each child is
emitted once. Larger cycles and incomplete loop analysis retain the dense
whole-function fallback. This avoids the code duplication that made an earlier
direct-branch experiment slower.

Local region entry states, PHI transfers and exception selector updates are
explicit. Active natural-loop backedges keep their direct labels. Regression
fixtures cover multi-entry cycles, nested natural loops and dispatch regions,
cyclic i64 PHI copies, exits, and exception cleanup. A runtime fuel bound catches
the previous whole-function fallback without prescribing an instruction sequence.

The LowIR optimizer and decoder source are unchanged. The generic lowering and
CFG-shape issues found during analysis are recorded separately, with repros, for
the LowIR optimizer work.

With immutable baseline and fixed workers linking the same frozen 413-object
input, four paired ABBA blocks measured 18.124 billion versus 11.853 billion
retired user instructions: 34.6% fewer instructions and 34.0% fewer CPU cycles.
The paired elapsed-time ratio was 0.708 (about 29% less time). Wall-time A/A
calibration varied by up to 19.6% on this shared host, while instruction-count
ratios were stable. All 47 warmup, calibration and comparison links produced
the exact original module bytes.

Against the original Clang-built worker's 10.783 billion instructions, the fix
removes 85.4% of cppgm's excess instruction work, leaving a 9.9% instruction
gap on this input. This comparison uses the same cppgm internal linker in all
workers. The decoder's Wasmtime-generated native code shrank from 134,824 to
52,888 bytes and its host frame from 3,536 to 1,136 bytes. Its Wasm local count
changed only from 655 to 648; local count alone did not explain the slowdown.

A fresh comparison builds both workers from the fixed 415-source package.
Both workers link exactly the same objects with cppgm's internal linker. Four
ABBA blocks measured 3.38 s for the cppgm-built worker and 3.15 s for the
Clang-built worker (paired Clang/cppgm ratio 0.932, A/A variation at most 1.1%).
Hardware counters give 11.868 billion versus 10.796 billion instructions,
leaving about 10% excess instruction work. Three native compilation probes have
paired fixed/baseline time ratios of 0.998, 1.014 and 1.009, with unchanged object
bytes. The corresponding Wasm compilation probes show no slowdown, but their
wall-time observations are too noisy for a general speedup claim.

These timings use pinned logical CPUs, not isolated physical cores. CPU 0 and
CPU 44 are SMT siblings; the concurrent browser qualification initially included
CPU 44. External workloads also occupied both siblings. Absolute timings from
separate campaigns must not be compared as compiler regressions: the unchanged
old Clang worker measured 1.78 s in the earlier campaign and about 3.16 s in this
one, with unchanged instruction work. An old/new Clang control on identical old
inputs measured 2.84/2.87 s, with unchanged instruction counts and a paired CPU
cycle ratio of 1.0005. All 26 control outputs matched. Both threads of that
physical core were also occupied by external work. The available observations
do not separate SMT contention, frequency changes and memory contention.

The fixed 415-source compiler passes complete W1/W2/W3 byte comparisons under
Wasmtime and Chromium 153.0.8010.12. All 1,248 native/WASI x86 object comparisons
pass at O0/O2/O3. The final module is 15,517,182 bytes, SHA-256
`191897eb5b7140c54e799106f2612e977a36df6a27dc39bea61fd4fbea344e7d`.
Observed local W1/W2/W3 times were 60.8/108.4/138.7 s with six jobs; Chromium's
serial W2/W3 times were 399.8/385.4 s. Browser setup and final checks bring its
total to 788.6 s, with peak guest linear memory 144,113,664 bytes. Cancellation,
restart, download, x86 floating-point output and accepted/rejected source checks
all pass. These are shared-host observations; Firefox and Safari remain
unqualified.

## Rebase onto PR135

The experiment was rebased onto the published [LLVM optimization series,
PR135](https://github.com/vishvananda/cppgm-extended/pull/135), head
`4fa32a6b478dfd8ee7393f5121be28019bd52039`. Compiler commit `70b53fa0f`
retains the Wasm implementation, sharing changes and bounded local dispatch.
The pre-rebase tip remains at `backup/wasi-before-pr135-20261113`.

The new package contains 416 compiler sources. W1/W2/W3 objects and final modules
match under Wasmtime and Chromium 153.0.8010.12, including the downloaded browser
module. Its 15,650,661 bytes have SHA-256
`b5170b3c924b9f1cdea906e382407d52969afd50f4b1d4bb31e717246131f075`.
All 1,251 native/WASI x86 comparisons at O0/O2/O3 pass. The strict report passes
10,861/10,861, along with debug-info, PA5 self-hosting, native inception, PA36
smoke, harness, architecture/format/placement audits and student export.

The performance comparison keeps the previous frozen 415-source workload and
flags unchanged. Four immutable AOT workers cover cppgm-built and Clang-built
compilers, before and after the rebase. Two A/A builds and two mirrored blocks
retain every observation. Each version's outputs must match that version's
native reference; optimizer changes intentionally alter generated objects
between versions. All 7,488 outputs from 18 full builds pass those comparisons.

| Worker version | cppgm-built instructions | Clang-built instructions | Excess |
| --- | ---: | ---: | ---: |
| Before PR135 | 1.1209 trillion | 0.8021 trillion | 39.7% |
| After PR135 | 1.0837 trillion | 0.8037 trillion | 34.8% |

This removes 3.3% of full-build instruction work and about 12% of the excess
instruction gap. Wall-time results do not resolve a reliable speedup: the two
paired cppgm before/after ratios were 1.009 and 0.938, and some Clang runs varied
from about 39 to 68 seconds. No observations are discarded. Shared-host timings
from separate campaigns must still not be treated as compiler regressions.

Much of the PR improves native MIR and x86 layout, which does not select Wasm
instructions. The generic changes transfer partially. The pointer-view and
loop-reducibility repros now pass, and the new worker has none of the three
earlier large whole-function dispatcher patterns. The aggregate-accessor
inlining and demanded-field/PHI repros remain. The separate local Wasm accessor
fix is not included in the published PR measured here. Recheck commands and
remaining generic optimization leads are retained outside the student export.

## Cycles and matched libc++ comparison

The comparison uses the same WASI SDK 34 libc++ headers and EH archives in both
producers. The cppgm producer additionally passes
`-D__cpp_exceptions=199711L -D__cpp_rtti=199711L`, matching Clang's C++11 feature
macros. cppgm's current Wasm environment omits these macros despite enabling the
language features; libc++ otherwise selects different exception/RTTI branches.
The matched producers both emit the `nqe230100` libc++ ABI tag. Library hashes,
commands and constant-return configuration probes are retained with the evidence.

Four pinned-core cohorts run every version on each frozen source in close
succession, rotating order. All 415 translation units and the internal link are
covered. Four campaigns contain 18 complete worker rebuilds; all 7,488 outputs
and 96 calibration/warmup outputs match current-version native bytes. External
SMT/cache contention remains; no observations or calibration outliers are removed.
These measurements compare summed CPU cycles, not parallel-build elapsed time.

With matched libc++ configuration, cppgm uses 22.4% more cycles than Clang
and 33.2% more instructions. The field-address diagnostic saves 3.1% of
cppgm's cycles. A combined field/copy/return/inlining diagnostic saves 5.7%,
closing 31% of the excess cycles and leaving 15.5% over Clang.
The reverse control, disabling the same feature macros in Clang, changes its
instruction work by only 0.13%; configuration differences do not explain the gap.

The strongest isolated shape is field addressing. A dependent field-gather
kernel is within about 1% of Clang's native x86 cycles but uses about 32% more
through Wasm. Supplying field allocation facts and selecting constant memory
offsets reduces that kernel's gap to about 5%. Plain LowIR index wraps at pointer
width; Wasm memory offsets use a wider effective address, so folding needs a
nonwrapping proof. Most source fields lack `inbounds`, and the Wasm selector
also leaves existing constant offsets outside memory instructions. x86 already
folds this shape into memory operands.

These are causal diagnostic builds. Broad field annotations and early return-slot
writes require allocation, escape and alias proofs before production use. The
combined diagnostic's instruction savings are substantially larger than its
cycle savings. Exact constant division is another backend opportunity, but Clang
emits the same Wasm division in the repro and its measured whole-build effect
is small. Profiles show distributed frontend/optimizer execution, without a new
large dispatcher catastrophe; the remaining gap is not causally classified as
generic LowIR or Wasm selection. Repros and generic leads remain in the external
LowIR notes. Evidence: `wasm-cycle-analysis17/analysis-summary.json`.
