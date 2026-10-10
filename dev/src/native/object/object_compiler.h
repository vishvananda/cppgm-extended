#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lowir/program.h"
#include "native/object/object_file.h"
#include "native/select/selector.h"
#include "native/x86/emitter.h"

// One translation unit's LowIR program -> a relocatable object (the
// `cppgm++ -c` back end): each function definition is selected, encoded
// and its body released as it is completed, then the code, constants and
// globals are placed in sections and every reference becomes a relocation.
//
// Placement and symbols (pa8/lowir.md symbol metadata):
// - binding=internal definitions are local symbols; binding=weak
//   definitions (inline functions, template instances, vtables, type
//   information) are weak symbols, each in its own section of a COMDAT
//   group named by its object symbol, so the linker keeps one copy (a
//   GNU weak definition, SF_REPLACEABLE, is a weak symbol in the shared
//   section, as GCC places it); other definitions are global.
//   Declarations are undefined symbols,
//   weak for binding=weak (a weak reference: Itanium's thread-local
//   entries; vague-linkage references are declared strong).
// - The object symbol is the `object=` spelling, else the LowIR name;
//   `keep_alias=yes` adds the LowIR name and `alias object` lines add
//   their spellings at the same place.
// - Functions go to .text, read-only globals to .rodata (after the
//   floating constants), zero-initialized writable globals to .bss, the
//   others to .data, each in definition order.
// - The init and fini roles become .init_array and .fini_array entries:
//   link mode runs every object's initialization in link order before
//   `main` and the finalizations in reverse order after it.
// - A declared thread-local wrapper (`tls_for`) is defined locally; the
//   runtime's state and routines (eh_*) and builtins are undefined symbols
//   that link mode's runtime library defines.
// - Internal definitions without an object spelling are local symbols
//   spelled with their LowIR sigil (`@name`), which no C or C++ object
//   name takes.
//
// Exception models (EhModel): the course's handler-stack runtime for
// objects link mode reads (the PA25 course object), or the host's
// Itanium tables for objects the host toolchain links (pa26): every
// function gets an FDE in .eh_frame (one CIE with the personality for
// the functions with an LSDA, one without), each function with a landing
// a call reaches an LSDA in .gcc_except_table (a COMDAT function's in its
// own group's section), and the personality and catch types are reached
// through local DW.ref cells in .data (pc-relative indirect: no text or
// pc-relative data relocation against an imported symbol). Builtin
// routines take their C library names.
namespace native {

struct ObjectStats
{
	// Whether the phase clocks run (--stats); the counters always do.
	bool clocks = false;
	// `cppgm++ --stats-functions`: each selected function's census lines
	// (native/object/function_census.h).
	bool census = false;
	std::vector<std::string> census_lines;
	SelectStats select;
	EmitStats emit;
	telemetry::Milliseconds select_ms = 0;
	telemetry::Milliseconds encode_ms = 0;
	telemetry::Counter sections = 0;
	telemetry::Counter symbols = 0;
	telemetry::Counter relocations = 0;
	telemetry::Counter landings = 0; // host EH: landing pads
	telemetry::Counter lsdas = 0;
	telemetry::Counter unwind_bytes = 0;     // .eh_frame and .gcc_except_table
	telemetry::Counter pruned_functions = 0; // unreferenced unit-local functions left out
};

class ObjectCompiler;

// The object back end fed one definition at a time: as a DefinitionSink,
// it selects and encodes each function the lowering completes (the
// lowering then releases the body); finish encodes the definitions not
// yet seen, places everything and builds the object. Throws
// native::BackendError for LowIR outside the native subset and InternalError
// for a broken invariant of the back end.
class ObjectBuilder : public lowir::DefinitionSink
{
public:
	ObjectBuilder(
		lowir::Program& program, ObjectStats& stats, EhModel eh_model = EhModel::Course, int optimization_level = 0);
	~ObjectBuilder();

	ObjectBuilder(const ObjectBuilder&) = delete;
	ObjectBuilder& operator=(const ObjectBuilder&) = delete;

	void append(const lowir::Program& program, lowir::SymbolId function) override;
	bool needs_boundaries() const override { return true; }
	bool reads_jump_thunks() const override { return true; }
	void finish(object::ObjectFile& out);

private:
	ObjectCompiler* compiler_;
};

// A program read from LowIR text has no Program::jump_thunks: each
// definition whose object spelling is an Itanium this-adjusting thunk's
// (5.1.4.2: `_ZTh`, `_ZTv`) and whose body is the forwarding call a
// lowering writes for it becomes one again, as the object-mode lowering
// stated it: in host objects every such thunk, in course objects a
// variadic function's.
void recover_jump_thunks(lowir::Program& program, EhModel eh_model);

// Compiles a complete `program` (validated or constructed by the lowering)
// into `out`, releasing each function body once encoded. Returns false
// with `error` set for LowIR outside the native subset; an InternalError
// propagates.
bool compile_object(lowir::Program& program, object::ObjectFile& out, std::string& error, ObjectStats* stats,
	EhModel eh_model = EhModel::Course, int optimization_level = 0);

} // namespace native
