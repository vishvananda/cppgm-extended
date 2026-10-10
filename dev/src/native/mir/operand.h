#pragma once

#include <cstdint>

#include "lowir/program.h"
#include "mir_model.h"

// The registers and operands of the machine IR (native/mir/mir.h).
namespace native {

// Registers (Reg, RAX ... R15, XMM0 ... XMM15, kNoReg, kSt0, reg_bit, the
// System V register sets), condition codes and the MIR operations and operand
// kinds are the shipped vocabulary headers native/mir/registers.h and
// mir_model.h, used as they are.
using namespace mir_model;

// A frame slot's base: RBP, or RSP past the prologue without a frame
// pointer (Function::omit_frame_pointer), where RBP may hold a value.
const Reg kFrame = 0xfd;

// Selector scratch (r10), emitter scratch (r11) and the emitter's vector
// scratch (xmm14, xmm15) are never allocated to values.
const Reg kSelectScratch = R10;
const Reg kEmitScratch = R11;
const uint32_t kAllocatableGprs = (kCallerSavedGprs | kCalleeSavedGprs) & ~((1u << R10) | (1u << R11));
const uint32_t kAllocatableXmms = 0x00ff0000u; // xmm0-xmm7

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
