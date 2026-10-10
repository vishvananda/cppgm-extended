#pragma once

#include <cstdint>
#include <vector>

#include "lowir/program.h"
#include "native/mir/eh_tables.h"
#include "native/mir/operand.h"

// The machine IR of the native backend: one x86-64 function at a time,
// with physical registers, final frame offsets and the call/ABI facts that
// native emission consumes. The MIR dump (native/mir/mir_writer.h) is a view
// of exactly this model. Pseudo instructions (`ret r`, `copy_bytes`,
// `eh_push`, the f80 and i128 forms) are expanded only by the emitter.
//
// Two exception models (native/mir/eh_tables.h): the course's
// handler-stack runtime ABI and the host's Itanium tables.
namespace native {

// The condition codes (Cond), operations (Op, ConvKind, WideOp), instruction
// flags (InstFlag) and frame binding kinds (BindingKind) are the shipped
// vocabulary headers native/mir/registers.h and mir_model.h, which operand.h
// brings into this namespace.

struct Inst
{
	Op op;
	lowir::TypeKind type = lowir::TypeKind::Void;  // T of typed forms
	lowir::TypeKind type2 = lowir::TypeKind::Void; // conversion destination
	uint8_t sub = 0;                               // Cond, ConvKind, WideOp, compare predicate
	uint16_t flags = 0;
	uint8_t nops = 0;
	uint32_t aux = 0;      // Call: argument register mask; Copy/Zero: bytes; EhPush: kind; EhCatch: selector
	uint32_t aux2 = 0;     // Call: stack bytes; Copy/Zero: alignment; EhPush: frame-slot record offset
	uint32_t location = 0; // 1 + index of the LowIR debug location
	uint32_t eh = 0;       // host EH Call: 1 + its landing (EhTables::landings), 0: none
	Operand ops[3];
};

// One ABI location of a parameter or result: a register, or an incoming
// stack offset from RBP. Two-eightbyte values have two entries.
struct AbiPart
{
	Reg reg = kNoReg; // kNoReg: `[rbp+stack]`; 0xfe: st0
	int32_t stack = 0;
	lowir::TypeKind type = lowir::TypeKind::I64;
};

struct AbiParam
{
	lowir::NameId name = 0;
	lowir::Type type;
	uint32_t first_part = 0;
	uint32_t part_count = 0;
};

// A named frame location shown in the dump: `param-slot %x`, `slot $x`,
// `temp %x`. Synthetic names (the register save area) use `label`.
struct FrameBinding
{
	BindingKind kind;
	lowir::NameId name;
	const char* label;
	int32_t offset;
	lowir::Type type;
};

struct Block
{
	lowir::NameId name; // 0 for an edge block
	uint32_t edge_from; // edge blocks: the LowIR predecessor and
	uint32_t edge_to;   // successor names
	uint32_t first;
	uint32_t count;
	bool landing; // a host landing pad (named by its landing block)
};

struct Function
{
	lowir::SymbolId symbol = lowir::kNoId;
	std::vector<AbiParam> params;
	std::vector<AbiPart> parts;
	lowir::Type return_type;
	AbiPart return_parts[2];
	uint8_t return_part_count = 0;
	uint32_t stack_size = 0;
	uint32_t scratch_bytes = 0;
	int32_t scratch_offset = 0;
	uint32_t preserve_mask = 0;  // callee-saved GPRs used
	int32_t preserve_offset = 0; // save area start (negative; -stack_size: the frame's bottom)
	std::vector<FrameBinding> bindings;
	std::vector<Block> blocks;
	// The blocks' instruction ranges, ascending (laid-out MIR leaves the
	// branches it dropped between them).
	std::vector<Inst> insts;
	std::vector<lowir::DebugLocation> locations;
	// The blocks the jump_table instructions name, each table a range.
	std::vector<uint32_t> jump_targets;
	bool variadic = false;
	// Course EH: eh_push places its handler record below the static frame
	// rather than in a frame slot (native/runtime/eh_record.h).
	bool eh_records_pushed = false;
	// O1+: the selector laid out the branches (select/branch_layout.cpp):
	// a block may fall through to the next one, and the encoding follows
	// the blocks and branches as they are.
	bool branches_laid_out = false;
	// O1+: a return before the function's end has its own epilogue (no
	// jump to the shared one).
	bool duplicate_epilogues = false;
	// O1+: copy_bytes and zero_bytes through 128 bytes are direct
	// (select/bulk_policy.h).
	bool wide_bulk = false;
	// O1+, host model: no RBP frame. The frame keeps its layout below a
	// virtual RBP (the saved-RBP slot left unused): the prologue reserves
	// stack_size + 8 less the saves and pushes them, so a frame operand
	// [rbp + d] is encoded [rsp + stack_size + d]. Only for bodies where
	// RSP does not move (no rsp or rbp register operand, stack_alloc,
	// va_start/va_arg, i128 conversion call through the stack).
	bool omit_frame_pointer = false;
	// O1+, frameless functions only: the blocks before the first prologue
	// block (Op::Prologue) run without the frame: no frame operand, no call,
	// no callee-saved register; each edge into the rest sets it up
	// (select/shrink_wrap.cpp).
	bool shrink_wrapped = false;
	// Local objects aligned beyond 16 (7.6.2, 3.11): they lie in the frame
	// region [aligned_disp - 16, aligned_disp + aligned_bytes + aligned_base
	// - 16), laid out from an aligned base (an object's frame operand is
	// [aligned_disp + its offset from that base]), which the prologue puts
	// in R12 (aligned_disp + rbp rounded up to aligned_base), reserved for
	// the function: the region's frame operands are encoded from R12.
	// aligned_base: that alignment (0: none).
	uint32_t aligned_base = 0;
	int32_t aligned_disp = 0;
	uint32_t aligned_bytes = 0;
	EhTables eh;

	void clear()
	{
		symbol = lowir::kNoId;
		params.clear();
		parts.clear();
		return_part_count = 0;
		stack_size = scratch_bytes = 0;
		scratch_offset = 0;
		preserve_mask = 0;
		preserve_offset = 0;
		bindings.clear();
		blocks.clear();
		insts.clear();
		locations.clear();
		jump_targets.clear();
		variadic = false;
		eh_records_pushed = false;
		branches_laid_out = false;
		duplicate_epilogues = false;
		wide_bulk = false;
		omit_frame_pointer = false;
		shrink_wrapped = false;
		aligned_base = aligned_bytes = 0;
		aligned_disp = 0;
		eh.clear();
	}
};

// The registers a function reserves from allocation: R12, the base of its
// over-aligned local objects (Function::aligned_base).
inline uint32_t reserved_gprs(const Function& function)
{
	return function.aligned_base != 0 ? 1u << R12 : 0u;
}

// Whether a frame displacement lies in the over-aligned objects' region.
inline bool in_aligned_region(const Function& function, int32_t disp)
{
	return function.aligned_base != 0 && disp >= function.aligned_disp - 16 &&
		disp < function.aligned_disp + static_cast<int32_t>(function.aligned_bytes + function.aligned_base) - 16;
}

// The program-level MIR facts: startup hooks, and the declared external C
// `atexit` the standalone runtime defines (its callbacks run after the
// entry function returns, most recent first, before the fini hook).
struct Startup
{
	lowir::SymbolId init = lowir::kNoId;
	lowir::SymbolId entry = lowir::kNoId;
	lowir::SymbolId fini = lowir::kNoId;
	lowir::SymbolId atexit = lowir::kNoId;
	uint32_t entry_params = 0;
};

} // namespace native
