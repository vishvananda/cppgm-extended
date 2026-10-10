#pragma once

#include <cstdint>

#include "lowir/program.h"

// The registers and operands of the machine IR (native/mir/mir.h).
namespace native {

// Registers: 0-15 are the GPRs in x86 encoding order, 16-31 XMM0-XMM15.
typedef uint8_t Reg;

enum : Reg
{
	RAX = 0,
	RCX,
	RDX,
	RBX,
	RSP,
	RBP,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	XMM0 = 16
};

const Reg kNoReg = 0xff;
// A frame slot's base: RBP, or RSP past the prologue without a frame
// pointer (Function::omit_frame_pointer), where RBP may hold a value.
const Reg kFrame = 0xfd;

inline bool is_xmm(Reg reg)
{
	return reg >= XMM0 && reg < XMM0 + 16;
}
inline uint32_t reg_bit(Reg reg)
{
	return 1u << reg;
}

// Selector scratch (r10), emitter scratch (r11) and the emitter's vector
// scratch (xmm14, xmm15) are never allocated to values.
const Reg kSelectScratch = R10;
const Reg kEmitScratch = R11;
const uint32_t kCallerSavedGprs = (1u << RAX) | (1u << RCX) | (1u << RDX) | (1u << RSI) | (1u << RDI) | (1u << R8) |
	(1u << R9) | (1u << R10) | (1u << R11);
const uint32_t kCalleeSavedGprs = (1u << RBX) | (1u << R12) | (1u << R13) | (1u << R14) | (1u << R15);
const uint32_t kAllXmms = 0xffff0000u;
const uint32_t kAllocatableGprs = (kCallerSavedGprs | kCalleeSavedGprs) & ~((1u << R10) | (1u << R11));
const uint32_t kAllocatableXmms = 0x00ff0000u; // xmm0-xmm7

// How a Symbol operand's address forms: pc-relative, from its GOT entry
// (a host object's imported data), or as a thread-local object's for the
// executing thread (a host object's thread_local: local exec for one the
// object defines, initial exec otherwise).
enum class SymbolAccess : uint8_t
{
	Direct,
	Got,
	TlsLocal,
	TlsInitial
};

enum class OperandKind : uint8_t
{
	None,
	Reg,
	Imm,    // integer immediate (imm, imm_high for i128)
	FImm,   // floating literal: bits in imm/imm_high, kind in `type`
	Mem,    // [base + index*scale + disp] or symbol-relative
	Symbol, // the address of a symbol (`@g`)
	Label   // a block of the current function
};

// A memory operand. With `symbol` set the address is PC-relative to that
// symbol (base and index absent); otherwise base is a GPR (RBP for frame
// locations, RSP for outgoing arguments) or absent with an index.
struct Operand
{
	OperandKind kind = OperandKind::None;
	Reg reg = kNoReg;                             // Reg; Mem base
	Reg index = kNoReg;                           // Mem
	uint8_t scale = 1;                            // Mem
	lowir::TypeKind type = lowir::TypeKind::Void; // FImm kind
	// Symbol: how its address forms (see FunctionSelector::symbol_operand).
	SymbolAccess access = SymbolAccess::Direct;
	int32_t disp = 0;               // Mem; Symbol addend
	uint32_t symbol = lowir::kNoId; // Mem, Symbol; Label: block
	int64_t imm = 0;
	int64_t imm_high = 0;

	bool is_reg() const { return kind == OperandKind::Reg; }
	bool is_mem() const { return kind == OperandKind::Mem; }
	bool is_imm() const { return kind == OperandKind::Imm; }
	bool uses_reg(Reg r) const
	{
		return (kind == OperandKind::Reg && reg == r) || (kind == OperandKind::Mem && (reg == r || index == r));
	}

	static Operand make_reg(Reg r)
	{
		Operand op;
		op.kind = OperandKind::Reg;
		op.reg = r;
		return op;
	}
	static Operand make_imm(int64_t value, int64_t high = 0)
	{
		Operand op;
		op.kind = OperandKind::Imm;
		op.imm = value;
		op.imm_high = high;
		return op;
	}
	static Operand make_mem(Reg base, int32_t disp, Reg index = kNoReg, uint8_t scale = 1)
	{
		Operand op;
		op.kind = OperandKind::Mem;
		op.reg = base;
		op.index = index;
		op.scale = scale;
		op.disp = disp;
		return op;
	}
	static Operand make_frame(int32_t disp) { return make_mem(kFrame, disp); }
	static Operand make_symbol_mem(uint32_t symbol, int32_t addend)
	{
		Operand op;
		op.kind = OperandKind::Mem;
		op.symbol = symbol;
		op.disp = addend;
		return op;
	}
	static Operand make_symbol(uint32_t symbol, int32_t addend = 0)
	{
		Operand op;
		op.kind = OperandKind::Symbol;
		op.symbol = symbol;
		op.disp = addend;
		return op;
	}
	static Operand make_label(uint32_t block)
	{
		Operand op;
		op.kind = OperandKind::Label;
		op.symbol = block;
		return op;
	}
	static Operand make_fimm(lowir::TypeKind kind, const lowir::Literal& bits)
	{
		Operand op;
		op.kind = OperandKind::FImm;
		op.type = kind;
		op.imm = static_cast<int64_t>(bits.low);
		op.imm_high = static_cast<int64_t>(bits.high);
		return op;
	}
	// The same location displaced by `delta` bytes (Mem only).
	Operand displaced(int32_t delta) const
	{
		Operand op = *this;
		op.disp += delta;
		return op;
	}
};

bool operator==(const Operand& a, const Operand& b);
inline bool operator!=(const Operand& a, const Operand& b)
{
	return !(a == b);
}

} // namespace native
