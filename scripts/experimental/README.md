# Optional WASI and browser inception

This is an experimental maintainer workflow. It does not add a student assignment,
change the PA37 report, or ship in the student export. Run from the solution
checkout on the `experiment/wasi-browser-inception` branch.

The native compiler builds W1. W1 then compiles all 408 frozen compiler sources
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

The 2026-11-12 experiment used a shared Linux machine with two Xeon E5-2696 v4
processors. All 408 corresponding objects and the final module matched through
W1/W2/W3, both locally and in Chromium 153.0.8010.12. The final module was
15,616,764 bytes, SHA-256
`b56ece94db402d4f5a15f38ef07100121b97b839f33496cb5fad24fd23dd47bd`.
Native/WASI x86 output matched for all 1,227 comparisons at O0/O2/O3, including
the floating-point corpus. Browser cancellation, restart, final-module download,
x86 floating-point checks and accepted/rejected program checks passed.

| Build | Observed elapsed time |
| --- | ---: |
| Native x86 compiler produces W1, six jobs | 44.3 s |
| Wasmtime W1 produces W2, six jobs | 91.5 s |
| Wasmtime W2 produces W3, six jobs | 56.5 s |
| Chromium W1 produces W2, serial worker | 367.1 s |
| Chromium W2 produces W3, serial worker | 358.0 s |

Local stage times exclude Wasmtime AOT preparation. The full browser run took
728.6 s, including setup and final checks; peak guest linear memory was
144,244,736 bytes. The host was shared, so these are approximate timings, not
isolated throughput guarantees. Full Firefox inception remains unqualified;
the earlier optional run was canceled. Safari was not tested.

A separate comparison compiled the same frozen compiler sources with SDK Clang
and cppgm, both at O3. **Both resulting WASI programs run cppgm's internal linker**
on the same frozen inputs; this does not compare cppgm's linker against LLD.
Locally prepared Wasmtime AOT modules, one pinned CPU, A/A calibration and four
ABBA blocks gave median link times of 3.59 s for the cppgm-built compiler and
1.70 s for the Clang-built compiler. Every output matched actual bytes. Three
source-compilation workloads had paired Clang/cppgm time ratios of about
0.72–0.78. Observations, commands, hashes and calibration noise remain in the
external experiment package.

Bulk input reads, typed opcode validation and flat relocation-field storage
reduced cppgm-built WASI linking from 6.79 s to 3.66 s on identical frozen
inputs, a median paired time ratio of 0.537. Native linking also improved,
from 1.35 s to 0.94 s. These comparisons retain all warmups and paired blocks.

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

Further candidates remain: ordinary Wasm switches still use linear comparisons,
and a failed region-structuring attempt falls back for the whole function.
Sharing switch-case planning and more control-region analysis with the native
backend could improve these paths while preserving target-specific emission.
Their individual contribution to the remaining Clang/cppgm gap has not yet been
measured.
