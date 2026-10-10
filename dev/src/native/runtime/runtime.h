#pragma once

#include <cstdint>
#include <string>

#include "lowir/program.h"
#include "native/image/image.h"
#include "native/mir/mir.h"
#include "native/x86/emitter.h"

// The standalone runtime linked into every executable: process entry
// (init hook, entry function, fini hook, exit), the handler-stack
// exception dispatcher of the course's EH ABI, and the builtin routines a
// program may declare (`object=cppgm_builtin_*`), encoded directly.
//
// The handler stack links the records (eh_record.h) that
// `eh_try`/`eh_cleanup ^b` fill in their frames; `eh_top` is the innermost.
// - eh_try/eh_cleanup ^b: push a record of that kind. A bare `eh_cleanup`
//   (a landing's cleanup clause) pushes nothing: the landing's own record,
//   when it is this frame's active try record on top, becomes a cleanup
//   record, which its `eh_end` pops and an `exception` read leaves.
// - eh_end: pop the top record.
// - throw: store the payload and dispatch; resume: dispatch.
// - dispatch: pop the active records on top, then mark the innermost
//   record active and continue at its landing with the frame and
//   callee-saved registers of its push; with no record left, exit 134.
// - exception: read the payload; an active try record on top that the
//   reading frame pushed is popped.
// A record therefore stays on the stack while its landing runs: a cleanup
// landing's `eh_end` pops its own record, as does `resume` (or a throw)
// leaving a landing, and a catch landing's `exception` read; an eh_end
// after that read pops an enclosing region, as the producers expect.
// The selector: a catch clause (`eh_catch`) calls the clause routine,
// which sets the landing's selector to the clause's when no earlier clause
// matched and the exception matches its type; dispatch clears the selector
// before entering a landing.
namespace native {

// Object symbol names of the runtime state and routines emitted code
// refers to: link mode's runtime library defines them.
extern const char kEhTopSymbol[];
extern const char kEhPayloadSymbol[];
extern const char kEhSelectorSymbol[];
extern const char kEhDispatchSymbol[];
extern const char kEhUncaughtSymbol[];
extern const char kEhCatchSymbol[];
extern const char* const kWideDivisionSymbols[4];
extern const char* const kFloatConversionSymbols[12];

// Creates the runtime's image symbols (data and routines referenced by
// emitted code) before any function is emitted; the state is defined in
// zero-filled storage, the routines by emit_runtime (lowir2native has no
// clause routine: catch clauses belong to link mode's runtime library).
void declare_runtime(Image& image, RuntimeSymbols& runtime);

// Emits the runtime routines and process entry; defines every declared
// but undefined function the runtime provides (builtins and
// thread-local wrappers). Returns the entry symbol.
uint32_t emit_runtime(
	Image& image, const lowir::Program& program, const Startup& startup, const RuntimeSymbols& runtime);

// The pieces, for link mode's runtime library and the object compiler:
// the uncaught-exception exit and the dispatcher (defining
// runtime.eh_uncaught and runtime.eh_dispatch); a builtin routine
// (`object=cppgm_builtin_*` names builtin_known accepts) or a thread-local
// wrapper returning its object's address, defined at `symbol`.
void emit_dispatcher(Image& image, const RuntimeSymbols& runtime);
bool builtin_known(const char* name);
void emit_builtin_routine(Image& image, uint32_t symbol, const char* name);
void emit_tls_wrapper(Image& image, uint32_t symbol, uint32_t object);
// A jump thunk (lowir::JumpThunk) defined at `symbol`: `self` moved by
// `adjustment`, then by the vcall offset at `vcall_offset` from its
// table's address point (none when 0), then a jump to `target`; it
// touches no other register (r10 holds the table) and leaves the stack
// as its caller made it.
void emit_jump_thunk(
	Image& image, uint32_t symbol, uint32_t target, Reg self, int64_t adjustment, int64_t vcall_offset);
// Process exit with the status in `status` (exit_group).
void emit_exit(Assembler& a, Reg status);

} // namespace native
