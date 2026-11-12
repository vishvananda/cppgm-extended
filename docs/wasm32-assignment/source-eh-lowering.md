# Source-to-Wasm EH implementation design

Maintainer working notes; excluded from the student export. This is unfinished
implementation work, not additional assignment scope or completion evidence.

## Established boundary

`machine::ExceptionAnalysis` now produces the native backend's existing handler,
action, selector-translation and resume facts without frame/register assumptions.
Its graph indices are layout positions. Calls' `eh_sites_` entries name
`1 + landing-table index`; resume entries name `1 + layout position`; the
terminal sentinel distinguishes a jump to a resume-only block. Mid-landing
catch-all entries use the same vector as a boolean marker. Keep these distinct.

The native adapter supplies its existing CFG, copies only the small block facts
when EH is present, applies pruning and assigns the native frame slot. Wasm
should adapt its CFG to this same analysis rather than implement another
handler-stack/matching analysis.

The external `source-eh-throw-boundary/` probe passes at O0/O3: cppgm emits an
integer throw, LLVM emits its typed catcher, and cppgm links and runs the result
against the pinned EH archives. There are no cppgm-generated handlers in this
probe. Source handling/cleanup/resume remain missing.

## Runtime and object preparation

Before module signature classification, inject typed declarations for
`_Unwind_CallPersonality : (ptr) -> i32` and the 12-byte
`__wasm_lpad_context` data object when required. Carry their actual SymbolIds;
do not recognize source/mangled names to rediscover semantic facts.

Import the supplied runtime's `__cpp_exception` tag, with `(i32) -> ()`
signature, as an ordinary undefined tag symbol. Do not synthesize a competing
definition. Add a `(i32, exnref)` result type for catch-reference labels.
Synthetic module symbol IDs currently follow program symbols as stack pointer,
stack floor and table; reserve tag and per-function LSDA identities explicitly.
Map them through ordinary object-symbol relocation handling.

Encode per-function LSDA from the shared action/type facts. Wasm uses the
SjLj-style call-site table: each landing index selects one `(landing, action)`
ULEB pair, not a native PC range. Use absolute four-byte type-information
pointers and `MemoryAddressI32` relocations, reverse the type table, and append
the shared exception-specification bytes after its base. A cleanup-only action
is zero. For a linear chain, the next action follows its one-byte displacement
field, so the forward displacement is one. The native emitter's frame/PC-relative
encoding stays separate.

## Selection and control flow

Wrap a protected call before pushing ABI arguments. Capture the current linear
stack pointer in a typed local, then use modern `try_table` with a `catch_ref`
to a surrounding `(i32, exnref)` label. Consume the normal call result inside
the try, branch past handler code, and store the caught payload/reference in
locals on the exceptional path. Restore the saved linear stack pointer before
personality or cleanup calls. Restoring only the fixed frame base could lose a
live dynamic allocation. A zero-frame function still needs the saved pointer.

Write the landing index and LSDA address to context offsets 0/4, call the
personality with the unwind pointer, and read its selector at offset 8. Apply
the shared landing's filter-to-selector map; preserve the raw filter for local
resumes. Let the supplied C++ personality implement matching and conversions.

EH call/resume edges must participate in control-region ordering. Keep natural
regions for reducible loops, including loops containing protected calls; use
the existing dispatcher only for genuinely irreducible control flow.

Select `exception`, `exception_selector` and mid-landing catch-all operations
from these typed locals/facts. Markers have no dynamic handler-record encoding.
A local resume transfers to its recorded landing and translates the raw filter;
a terminal resume restores the function stack and emits `throw_ref`.

One reference local per function can lose an outer exception if an inlined
cleanup catches an inner exception. Retain references per active landing/context
and track the appropriate source through ordinary merges. Shared active-record
facts can supply this identity; merged active tails need a dynamic reference
transfer, not a guessed landing. Add a reducer before claiming nested handling.

LowIR's explicit `throw <type> <value>` is an opaque scalar/pointer payload,
distinct from source C++ throw-helper calls. The shared native-host analysis
currently rejects that instruction. Preserve its required Wasm behavior with
an explicit domain/tag and full payload bits; never reinterpret it as a C++
unwind pointer. A VM trap must remain outside C++ catches. Foreign tags need
cleanup/reference propagation where the selected profile requires it.

## Completion checks

Add normal Perl controls for own/LLVM throwing and catching in both directions,
both linkers and O0–O3; cleanup, converted/base catches, rethrow, function-try
blocks, destructor throws, noexcept, nested/merged handlers, dynamic stack
restoration and explicit LowIR payloads. Integrate automatic EH/no-EH C++
runtime selection with `--sysroot` and preserve explicit library-disable flags.
Then continue SIMD, the remaining object profile, quality and benchmarks.
