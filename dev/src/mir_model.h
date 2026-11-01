#pragma once

// The machine-IR vocabulary: every enumerable concept of the x86-64 MIR and
// its dump (`lowir2native --dump-machine-ir`, pa24/README.md) as an enum,
// with its spellings.  Registers and condition codes are in
// native/mir/registers.h; the LowIR words the dump reuses (types, compare
// predicates, conversions) come from lowir_model.h.  The course solution
// builds on this header exactly as it is shipped.
//
// As in lowir_model.h there are no containers: how a MIR function, block,
// instruction and operand are stored is your design.  PA33 adds a
// liveness-based allocator and MIR passes between selection and encoding,
// so keep selection separate from register assignment and record each
// instruction's uses, definitions and clobbers (see the PA24 handout).
//
// The dump writes some of these with the LowIR type of their operation
// (`load.i32`, `fadd.f64`, `sitofp.f64.i32`) and the compound forms from
// their parts (`set` + condition, `f` + predicate, wide operation +
// `.i128`); the fixed words of the dump's sections (`machine_ir`, `abi`,
// `frame`, `stack_size`, ...) are format text, not vocabulary.

#include <cstddef>
#include <cstdint>

#include "lowir_model.h"
#include "native/mir/registers.h"

namespace mir_model {

// ---------------------------------------------------------------------------
// Operations

enum class Op : std::uint8_t
{
	// Integer moves and memory.
	Mov,   // mov dst, reg|imm|@sym
	Load,  // load.T dst, mem (extending per T)
	Store, // store.T mem, reg|imm
	Lea,   // lea dst, mem
	// Two-address integer ALU: dst op= src.
	Add,
	Sub,
	And,
	Or,
	Xor,
	Imul,
	Neg,
	Not,
	Bswap,
	Bsf, // bsf/bsr.T dst, src: the index of the lowest/highest set bit
	Bsr,
	Shl,
	Shr,
	Sar, // dst, imm or rcx
	Rol, // rol/ror dst, imm or rcx: rotate left/right
	Ror,
	Cmp,   // cmp.T a, b
	Test,  // test.T a, b: the flags of a & b
	Bt,    // bt.T a, b: CF = bit b of a
	Setcc, // set<cond> dst
	Cmov,  // cmov<cond> dst, src
	Movzx, // movzx dst, dst (low byte)
	Sext,  // sext.T dst [, src]: sign-extend the low T bits
	Zext,  // zext.T dst [, src]: zero-extend the low T bits
	Cqo,
	Idiv,
	Div,
	Jcc,       // j<cond> ^label
	Jmp,       // jmp ^label
	Call,      // call @sym | call *reg | call *mem [call facts]
	TailJump,  // tail_jump @sym | *reg | *mem: the epilogue, then a jump
	CopyBytes, // copy_bytes NxA, dst, src; copy_bytes_dynamic (MF_COPY_DYNAMIC)
	ZeroBytes, // zero_bytes NxA, dst
	Ret,       // ret [value]
	// Scalar floating point (f32/f64 in XMM registers; f80 forms in memory).
	FMov,
	FAdd,
	FSub,
	FMul,
	FDiv,
	FNeg,
	FAbs,
	Vector,  // vector.op dst, a [, b] [, imm] (pa8/lowir.md "Vector Operations")
	FCmp,    // fcmp.T a, b (an unordered result sets PF)
	FSetcc,  // f<predicate>.T dst, a, b: the comparison as a Boolean
	Convert, // <conversion>.S.D dst, src (source and destination types)
	FRet,    // fret.f80 src
	FStp,    // fstp.f80 [mem]: store or discard an x87 result
	// Wide integers in memory.
	Wide,          // <wide op>.i128 dst, a, b
	WideSetcc,     // set.<predicate>.i128 dst, a, b
	WideCmpBranch, // cmp_branch.<predicate>.i128 a, b, ^true, ^false
	// Atomics.
	LockXadd,
	Xchg,
	LockCmpxchg,
	LockCmpxchg16b,
	Mfence,
	// Variadic support, stack allocation and runtime operations.
	VaStart,
	VaArg,
	StackAlloc,
	EhPush,
	EhPop,
	Throw,
	Resume,
	LoadException,
	EhMark,       // a landing's cleanup clause keeps its own record
	EhCatch,      // eh_catch [@type-info], selector: one catch clause
	LoadSelector, // load_selector.T dst: the landing's matched clause
	Landing,      // landing exc-slot, sel-slot: a host landing pad
	EhSelect,     // eh_select sel-slot, raw-slot: a landing's selector translation
	CatchAll,     // catch_all sel-slot, n: a mid-landing catch-all clause
	TlsAddr,
	Exit,
	Trap,
	JumpTable, // jump_table idx, [^a, ^b, ...]
	MulWide,   // mul src: rdx:rax = rax * src, unsigned
	ImulWide,  // imul src: the signed form
	Prologue,  // a shrink-wrapped function's frame setup
	Nop        // a removed instruction: no code, not dumped
};

const unsigned kOpCount = static_cast<unsigned>(Op::Nop) + 1;

// The mnemonic, or the stem of a compound one: "set" and "j" and "cmov"
// take a condition, "f" a predicate, "cmp_branch" and "set" (WideSetcc)
// a predicate and ".i128"; Convert and Wide are spelled by their sub-
// operation, and Nop is never written ("").
inline const char* spelling(Op op)
{
	static const char* const kOps[kOpCount] = { "mov", "load", "store", "lea", "add", "sub", "and", "or", "xor", "imul",
		"neg", "not", "bswap", "bsf", "bsr", "shl", "shr", "sar", "rol", "ror", "cmp", "test", "bt", "set", "cmov",
		"movzx", "sext", "zext", "cqo", "idiv", "div", "j", "jmp", "call", "tail_jump", "copy_bytes", "zero_bytes",
		"ret", "fmov", "fadd", "fsub", "fmul", "fdiv", "fneg", "fabs", "vector", "fcmp", "f", "", "fret", "fstp", "",
		"set", "cmp_branch", "lock_xadd", "xchg", "lock_cmpxchg", "lock_cmpxchg16b", "mfence", "va_start", "va_arg",
		"stack_alloc", "eh_push", "eh_pop", "throw", "resume", "load_exception", "eh_mark", "eh_catch", "load_selector",
		"landing", "eh_select", "catch_all", "tls_addr", "exit", "trap", "jump_table", "mul", "imul", "prologue", "" };
	return kOps[static_cast<unsigned>(op)];
}

// The floating conversions of Op::Convert: the last six LowIR ConvertOps,
// spelled as they are.
enum class ConvKind : std::uint8_t
{
	SIToFP,
	UIToFP,
	FPToSI,
	FPToUI,
	FPExt,
	FPTrunc
};

const unsigned kConvKindCount = static_cast<unsigned>(ConvKind::FPTrunc) + 1;

inline lowir_model::ConvertOp convert_op(ConvKind kind)
{
	return static_cast<lowir_model::ConvertOp>(
		static_cast<unsigned>(kind) + static_cast<unsigned>(lowir_model::ConvertOp::SIToFP));
}

// The floating conversion of a LowIR ConvertOp; false for sext/zext/trunc.
inline bool conv_kind(lowir_model::ConvertOp op, ConvKind* kind)
{
	if (op < lowir_model::ConvertOp::SIToFP)
	{
		return false;
	}
	*kind = static_cast<ConvKind>(static_cast<unsigned>(op) - static_cast<unsigned>(lowir_model::ConvertOp::SIToFP));
	return true;
}

inline const char* spelling(ConvKind kind)
{
	return lowir_model::spelling(convert_op(kind));
}

// The operations of Op::Wide.  Div, UDiv, Mod and UMod call the runtime's
// division routines.
enum class WideOp : std::uint8_t
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

const unsigned kWideOpCount = static_cast<unsigned>(WideOp::UMod) + 1;

inline const char* spelling(WideOp op)
{
	static const char* const kWideOps[kWideOpCount] = { "mov", "add", "sub", "mul", "and", "or", "xor", "shl", "shr",
		"sar", "neg", "not", "sext", "zext", "div", "udiv", "mod", "umod" };
	return kWideOps[static_cast<unsigned>(op)];
}

// ---------------------------------------------------------------------------
// Instruction flags

enum InstFlag : std::uint16_t
{
	MF_VARIADIC = 1,       // call: `variadic`
	MF_UNWIND_NO = 2,      // call: `unwind=no`
	MF_NORETURN = 4,       // call: `returns=noreturn`
	MF_VOLATILE = 8,       // a volatile access (Wide: an i128 atomic load)
	MF_COPY_DYNAMIC = 16,  // copy_bytes_dynamic: rdx bytes from rsi to rdi
	MF_STRLEN_PREFIX = 32, // call: `strlen_prefix=16`, a strlen whose first 16 bytes are probed inline
	MF_FRAMELESS = 64      // ret in a shrink-wrapped function's frameless region: no epilogue
};

// The call fact a flag writes in a call's `[args=(...), ...]` list; null
// for flags that are not call facts.
inline const char* call_fact_spelling(InstFlag flag)
{
	switch (flag)
	{
	case MF_VARIADIC:
		return "variadic";
	case MF_UNWIND_NO:
		return "unwind=no";
	case MF_NORETURN:
		return "returns=noreturn";
	case MF_STRLEN_PREFIX:
		return "strlen_prefix=16";
	default:
		return 0;
	}
}

// ---------------------------------------------------------------------------
// Operands

enum class OperandKind : std::uint8_t
{
	None,
	Reg,    // rax, xmm0, st0
	Imm,    // an integer immediate (two 64-bit halves for i128)
	FImm,   // a floating immediate: its bits and its f32/f64/f80 kind
	Mem,    // [base + index*scale + disp], or symbol-relative @sym+disp
	Symbol, // the address of a symbol: @sym, @sym+disp
	Label   // a block of the current function: ^label
};

// How a symbol's address forms: pc-relative; from its GOT entry (a host
// object's imported data); as a thread-local object's address for the
// executing thread (local exec for one the object defines, initial exec
// otherwise).
enum class SymbolAccess : std::uint8_t
{
	Direct,
	Got,
	TlsLocal,
	TlsInitial
};

const unsigned kSymbolAccessCount = static_cast<unsigned>(SymbolAccess::TlsInitial) + 1;

// The operand suffix after `@`: "got", "tpoff", "gottpoff"; "" for Direct.
inline const char* spelling(SymbolAccess access)
{
	static const char* const kAccesses[kSymbolAccessCount] = { "", "got", "tpoff", "gottpoff" };
	return kAccesses[static_cast<unsigned>(access)];
}

// ---------------------------------------------------------------------------
// Frame bindings: the named frame locations a function's `frame` section
// lists (`param-slot %x -> [rbp-8] : i64`, `slot $x ...`, `temp %x ...`).

enum class BindingKind : std::uint8_t
{
	ParamSlot,
	Slot,
	Temp
};

const unsigned kBindingKindCount = static_cast<unsigned>(BindingKind::Temp) + 1;

inline const char* spelling(BindingKind kind)
{
	static const char* const kBindings[kBindingKindCount] = { "param-slot", "slot", "temp" };
	return kBindings[static_cast<unsigned>(kind)];
}

// The sigil of the bound name: a parameter's or temporary's `%`, a slot's `$`.
inline char sigil(BindingKind kind)
{
	return lowir_model::sigil(kind == BindingKind::Slot ? lowir_model::ValueKind::Slot : lowir_model::ValueKind::Temp);
}

} // namespace mir_model
