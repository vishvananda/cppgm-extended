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

// x86 condition codes (the low nibble of Jcc/SETcc).
enum Cond : uint8_t
{
	CC_O = 0,
	CC_NO,
	CC_B,
	CC_AE,
	CC_E,
	CC_NE,
	CC_BE,
	CC_A,
	CC_S,
	CC_NS,
	CC_P,
	CC_NP,
	CC_L,
	CC_GE,
	CC_LE,
	CC_G
};

inline Cond invert(Cond cond)
{
	return static_cast<Cond>(cond ^ 1);
}

enum class Op : uint8_t
{
	// Integer moves and memory.
	Mov,   // mov dst, reg|imm|@sym
	Load,  // load.T dst, mem (extending per T)
	Store, // store.T mem, reg|imm
	Lea,   // lea dst, mem
	// Two-address integer ALU at 64 bits (at T's width with a memory
	// operand): dst op= src.
	Add,
	Sub,
	And,
	Or,
	Xor,
	Imul,
	Neg,
	Not,
	Bswap,
	Bsf,
	Bsr, // bsf/bsr.T dst, src: the lowest/highest set bit's index (dst undefined for 0)
	Shl,
	Shr,
	Sar,   // dst, imm or rcx (u32: the 32-bit form, count below 32)
	Cmp,   // cmp.T a, b
	Test,  // test.T a, b (flags of a & b; O1+)
	Bt,    // bt.T a, b (CF: bit b of a; O1+)
	Setcc, // set<cc> dst
	Cmov,  // cmov<cc> dst, src: dst = src when cc holds (64-bit registers; O2+ selects)
	Movzx, // movzx dst, dst (low byte)
	Sext,  // sext.T dst [, src]: sign-extend the low T bits in place (or src's)
	Zext,  // zext.T dst [, src]: zero-extend the low T bits in place (or src's)
	Cqo,
	Idiv,
	Div, // idiv/div src
	Jcc,
	Jmp,
	Call,      // call @sym | call *reg | call *mem
	TailJump,  // tail_jump @sym | *reg | *mem: the epilogue, then a jump (O2+ sibling call; aux as Call's)
	CopyBytes, // copy_bytes NxA, dst, src (address operands); copy_bytes_dynamic
			   // (MF_COPY_DYNAMIC): rdx bytes from rsi to rdi, a call's registers
	ZeroBytes, // zero_bytes NxA, dst
	Ret,       // ret [value]
	// Scalar floating point (f32/f64 in XMM; f80 forms take memory).
	FMov, // fmov.T dst, src
	FAdd,
	FSub,
	FMul,
	FDiv,    // f<op>.T dst, a, b
	FNeg,    // fneg.T dst, a
	FCmp,    // fcmp.T a, b (flags; unordered sets PF)
	FSetcc,  // f<pred>.T dst, a, b (materialized Boolean)
	Convert, // <conv>.S.D dst, src
	FRet,    // fret.f80 src
	FStp,    // fstp.f80 [mem] (store a call's x87 result, or discard it)
	// Wide integers in memory.
	Wide,          // <op>.i128 dst, a, b (sub = WideOp)
	WideSetcc,     // set.<pred>.i128 dst, a, b
	WideCmpBranch, // cmp.i128 a, b, ^true, ^false (sub = predicate)
	// Atomics.
	LockXadd,
	Xchg,
	LockCmpxchg,
	LockCmpxchg16b,
	Mfence,
	// Variadic support, stack allocation and runtime operations.
	VaStart,    // va_start mem (list)
	VaArg,      // va_arg.T dst, list-mem
	StackAlloc, // stack_alloc dst, size
	EhPush,
	EhPop,
	Throw,
	Resume,
	LoadException,
	EhMark,       // eh_mark: a landing's cleanup clause keeps its own record
	EhCatch,      // eh_catch [@type-info], selector: one catch clause
	LoadSelector, // load_selector.T dst: the landing's matched clause
	Landing,      // landing exc-slot, sel-slot[, sel-slot]: a host landing pad stores rax, rdx
	EhSelect,     // eh_select sel-slot, raw-slot: a host landing's selector translation (aux: landing)
	CatchAll,     // catch_all sel-slot, n: a host mid-landing catch-all clause (aux: n)
	TlsAddr,
	Exit,
	Trap,
	// jump_table idx, [^a, ^b, ...] (O1+ switch): a jump to the block the
	// table's entry `idx` names; idx is an unsigned register below the
	// entry count (aux2), which a preceding compare checked. The entries
	// are Function::jump_targets[aux .. aux + aux2).
	JumpTable,
	// Widening multiplies (constant division): rdx:rax = rax * src,
	// unsigned (`mul src`) or signed (`imul src`).
	MulWide,
	ImulWide,
	// A shrink-wrapped function's prologue (Function::shrink_wrapped): the
	// frame is set up here instead of at the entry.
	Prologue,
	// A removed instruction (merged into another): no code, not dumped.
	Nop
};

// Sub-operations of Op::Convert.
enum class ConvKind : uint8_t
{
	SIToFP,
	UIToFP,
	FPToSI,
	FPToUI,
	FPExt,
	FPTrunc
};

// Div, UDiv, Mod and UMod call the runtime's division routines (rdi:rsi
// by rdx:rcx, the result in rax:rdx).
enum class WideOp : uint8_t
{
	Move,
	Add,
	Sub,
	Mul,
	And,
	Or,
	Xor,
	Shl,
	Shr,
	Sar,
	Neg,
	Not,
	SExt,
	ZExt,
	Div,
	UDiv,
	Mod,
	UMod
};

enum InstFlag : uint16_t
{
	MF_VARIADIC = 1,
	MF_UNWIND_NO = 2,
	MF_NORETURN = 4,
	MF_VOLATILE = 8,
	MF_COPY_DYNAMIC = 16, // CopyBytes: the count is dynamic (rdi, rsi, rdx)
	// Call: a strlen whose string's first 16 bytes are probed inline (when
	// they lie in one page); the call runs only if none is NUL.
	MF_STRLEN_PREFIX = 32,
	// Ret in a shrink-wrapped function's frameless entry region: no epilogue.
	MF_FRAMELESS = 64
};

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

enum class BindingKind : uint8_t
{
	ParamSlot,
	Slot,
	Temp
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

const Reg kSt0 = 0xfe;

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
