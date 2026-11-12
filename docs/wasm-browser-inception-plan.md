# Optional Wasm self-hosting and browser inception

The experimental implementation and reproducible commands now live in
[`scripts/experimental/README.md`](../scripts/experimental/README.md). The notes
below preserve the original investigation; their incomplete-work statements
describe those earlier checkpoints. Student expectations remain unchanged.

Investigation at `41c232dd6`, 2026-11-12. This is maintainer planning material,
excluded from the student export. The user selected optional qualification:
running cppgm inside Wasm or a browser is not a required PA37 correctness gate.
The full compiler and its self-build have not run in either environment yet.

Follow-up at `1087b2daf`: target builtin templates and unresolved weak runtime
calls are fixed. Ordinary WASI file streams now pass without the extra runtime
object described in the original probe below. The remaining compiler-host
portability and complete browser round-trip work is still optional.

The subsequent binding work carries `import_module`, `import_name` and
`export_name` from source through typed LowIR and standard objects. Separate
host-module calls, imported globals, persistent state and imported-function
re-exports execute correctly. This qualifies the explicit host interface;
it does not establish the full compiler's WASI import closure or portability.

## Finding

The practical route is one `wasm32-wasip1` compiler executable, used unchanged
under Wasmtime and in a dedicated browser worker. The browser supplies a WASI
Preview 1 filesystem, arguments, environment and standard streams. Our own
preprocessor, optimizer, encoder and static linker remain in the executable.
This should not require an Emscripten target or another code-generation backend;
that is an engineering conclusion from the working probes below.

The substantial shared work is making cppgm itself portable to a 32-bit WASI
host, compiling its libc++ dependencies and establishing deterministic output.
The additional browser work is packaging, synchronous filesystem glue, worker
orchestration and memory/stack qualification.

## Executed probes

The evidence directories are `wasm-inception-probe/` and
`browser-inception-probe/` under `/mnt/vizier/cppgm-wasm32-evidence`.
They retain commands, source, module hashes, dependency integrity, browser
versions and results. Generated binaries and the downloaded shim stay out of Git.

Both Chromium 153.0.8010.12 and Firefox 150.0.2 passed all five probes in a
module worker, on localhost, with `crossOriginIsolated == false`:

| Probe | Executed behavior |
| --- | --- |
| SIMD128 | Four independent scalar-oracle kernel results from the existing cppgm benchmark module |
| Modern EH | Seven opaque scalar/pointer payload results, including i64 values crossing the JavaScript boundary |
| C++ EH | Nested cleanup, throwing destructor and outer catch; two calls in one instance |
| WASI command | Arguments, global constructor, C++ throw/catch, stdout and reading/writing virtual files |
| Persistent worker files | OPFS synchronous access handle, write/read equality and removal |

These are cppgm-produced modules. Safari and the complete compiler executable
were not tested. Detect the actual modern EH/SIMD profile with a small module
at startup; support for older Wasm exception instructions is insufficient.

The WASI command probe needed one explicit runtime object, `chdir.c.obj`,
extracted from the pinned SDK's libc archive. Without that workaround the
ordinary link fails with an unresolved weak function relocation for
`__wasilibc_find_relpath_alloc`. This is a core linker gap, not a browser shim
failure. The original probe therefore establishes the browser host services.
The later target-library checkpoint separately qualifies the ordinary
file-I/O path.

The shim was `@bjorn3/browser_wasi_shim` 0.4.2, with registry SHA-512 integrity
checked and both upstream licences retained. Its archive is 31,373 bytes;
that is not a complete application download estimate. Upstream explicitly
describes incomplete Preview 1 coverage, so audit the full compiler's actual
imports and exercised operations before adopting it.
[Upstream implementation and limitations](https://github.com/bjorn3/browser_wasi_shim).

## Confirmed compiler prerequisites

A complete syntax-only Clang scan of the 406 translation units passed 274 and
failed 132. All reported errors fell into the three host-portability categories
below: record padding, process resource APIs and x87 host floating representation.
This scan disables telemetry and the POSIX test runner; it does not test code
generation, final linkage or behavior. Its results and every diagnostic are in
`wasm-inception-probe/clang-portability/`.

| Area | Observed problem | Required change |
| --- | --- | --- |
| Host object layout | Clang reports `sizeof(CompoundItems) == 12`, versus an 8-byte slice, in `syntax/ast_statements.h` | Preserve the closing location on 32-bit hosts; retain the compact native layout without asserting that every host has its padding |
| Stack/resource APIs | `support/stack_budget.h` uses `getrlimit`; driver files include `sys/resource.h` even with telemetry disabled | Explicit WASI stack budget and guarded host telemetry/resource code; measure the engine call stack separately from the linear-memory stack |
| Host floating representation | `lex/number_literal.cpp` and `lowir/literals.cpp` require x87's 64-bit significand; WASI `long double` has 113 bits | Separate host floating conversion from serialized target bits; preserve rounding, NaNs, signed zero and native output identity |
| Target builtin declarations | Clang compiles `opt/optimizer.cpp` for WASI; cppgm rejects libc++'s `__type_pack_element` declaration | Share target-independent builtin templates with the Wasm environment; retain its own `va_list` and target layout |
| Undefined weak functions | The stdio probe cannot relocate a guarded direct call to an absent weak function | Match the linking contract: null function addresses, correctly typed trapping direct-call stubs, and no extraction of an archive solely for a weak reference |

The floating code also copies ten x87 bytes from host `long double`; deleting
its assertions would silently corrupt values on WASI. Reuse explicit floating
representations/conversions and test round trips and rounding boundaries.

The scan is not an exhaustive portability qualification. Compile every source
to an object with pinned Clang first, then with cppgm, to separate compiler-host
portability from target frontend/backend failures.
The later cppgm probe reaches another distinction: the SDK's `errno.h` uses
the C11 spelling `_Thread_local`, which Clang accepts in C++ but cppgm does not
currently parse. Qualify the single-thread libc declaration and its storage ABI
before choosing a compatibility fix; removing the qualifier without checking
the linked runtime would not establish correctness.
Keep required PA37 fixes, such as weak linking and target builtin declarations,
covered independently of the optional self-host qualification.

Build with `CPPGM_TEST_RUNNER=0` and `CPPGM_TELEMETRY_ENABLED=0`. Browser/WASI
execution cannot use the POSIX fork/exec test runner. Keep C++ exceptions enabled:
the compiler uses them internally. Use the pinned WASI libc++, libc++abi,
compiler-rt and modern EH runtime, with an isolated object directory. Do not
reuse native objects or native standard-library headers.

## Optional round trip

Freeze the compiler sources, generated configuration, target headers, runtime
archives, flags and logical paths in one hashed input manifest.

1. Build a native cppgm and use it to produce the first Wasm compiler, W1.
   A Clang-built Wasm seed is useful earlier to qualify host portability.
2. Run W1 under Wasmtime to compile the frozen inputs and link W2.
3. Run W2 under the same configuration to compile and link W3.
4. Require byte-identical corresponding object files and final modules for
   W2 and W3. If W1 was produced by the same cppgm implementation, also compare
   W1/W2 to test native-host versus Wasm-host behavior. A Clang seed has no such
   expected byte identity.
5. Repeat steps 2–4 entirely in a browser worker, without a native compilation
   service. Compare its objects/modules against the equivalent Wasmtime stages.

Compare actual bytes, with hashes for reporting and the first differing object
and byte offset on failure. Do not strip sections or normalize artifacts to
force equality. Diagnose path/time macros, generated headers, host library
iteration order and pointer-width-dependent behavior instead. Existing
behavioral and ABI checks remain necessary alongside the fixed-point check.

Use a fresh compiler instance for each translation unit and link invocation.
Compile each stage's WebAssembly module once and reuse it across instances;
reset globals, heap and runtime state while retaining immutable input files.
Browser modules can be shared with workers without sharing guest memories.
[WebAssembly.Module](https://developer.mozilla.org/en-US/docs/WebAssembly/Reference/JavaScript_interface/Module).

## Browser host and packaging

Start with one dedicated worker and an in-memory read-only source/sysroot tree,
plus writable stage directories. Fetch and unpack inputs before invoking the
synchronous compiler entry point. Provide stable absolute logical paths and
explicit argv/environment rather than depending on the deployment URL.
Regenerate the embedded host configuration for this package; do not retain
native machine macros, search directories or build-machine SDK paths. Preserve
stable file device/inode identities: source caching and include-once behavior
use `stat`, so placeholder identities shared by unrelated files are incorrect.
The shim's existing `SyncOPFSFile` can back large outputs: prepare file handles
asynchronously before a compile, then service synchronous guest file operations.
Do not attempt an asynchronous fetch or filesystem operation from an ordinary
synchronous WASI import.

OPFS synchronous handles are available in dedicated workers in a secure context.
Serve over HTTPS (localhost suffices for development). The probes needed neither
shared memory nor cross-origin isolation. Precreate known output handles, close
them after invocation and publish a stage manifest only after every file closes;
incomplete stages must not be reused after cancellation.
[Synchronous file API](https://developer.mozilla.org/en-US/docs/Web/API/FileSystemSyncAccessHandle).

The page should show per-file progress, diagnostics, elapsed time and artifact
comparison results. Cancel by terminating the worker and discard its incomplete
stage. A browser does not provide Wasmtime's fuel mechanism, so use an external
deadline rather than claiming an instruction bound.
[Worker termination](https://developer.mozilla.org/en-US/docs/Web/API/Worker/terminate).

Serve modules as `application/wasm` for streaming compilation. If the deployment
uses CSP, allow Wasm compilation (`script-src 'self' 'wasm-unsafe-eval'`) and
same-origin workers. Cache input packages by hash; verify their manifest before
using them, including downloaded runtime archives.
[Streaming requirements](https://developer.mozilla.org/en-US/docs/WebAssembly/Reference/JavaScript_interface/compileStreaming_static),
[Wasm CSP policy](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Content-Security-Policy/script-src#unsafe_webassembly_execution).

The current source set contains 406 cppgm translation units, 8,756,352 bytes of
implementation source and 1,447,551 bytes across shared headers. These are raw
source totals, excluding SDK headers, libraries, generated files and other
tools. Measure the reachable package before choosing download/UI limits.
Wasm32 has a 4 GiB address space ceiling; actual browser memory availability and
peak compiler memory are separate limits. Measure peak memory on the largest
source file and link, the compiler's linear stack and native engine recursion
before choosing a worker count. Start serially; scale only after measurement.
[Wasm32 memory ceiling](https://v8.dev/blog/4gb-wasm-memory).

## Delivery sequence

| Step | Exit evidence |
| --- | --- |
| Fix mandatory target/library gaps | Reduced required PA37 fixtures for builtin templates, weak direct calls and ordinary stdio linking |
| Port compiler host | All compiler sources compile/link with pinned Clang for WASI; executable handles one successful and one rejected compile under Wasmtime |
| Build cppgm-produced seed | Native cppgm builds W1; W1 produces validated objects and a working command |
| Qualify Wasmtime inception | Frozen W2/W3 objects and modules are byte-identical; all commands, hashes and resource observations retained |
| Add browser host | Same seed compiles, links and runs a small program in Chromium/Firefox; filesystem/import closure audited |
| Qualify browser inception | Full browser W2/W3 identity plus Wasmtime/browser agreement; measured memory/time, cancellation and restart behavior |

Only then expose an optional inception target and concise student instructions.
Keep the ordinary PA37 report independent. Required fixtures continue to use
Perl and the standard parallel runner; JavaScript is browser host glue. A future
maintainer browser lane may drive it through Playwright. Do not claim Safari
qualification from a Linux WebKit run; test the supported Safari version on macOS.
