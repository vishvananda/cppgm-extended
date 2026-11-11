#include "native/select/mir_cleanup.h"

#include "native/mir/type_kinds.h"
#include "native/select/mir_facts.h"

// The 32-bit forms of instructions whose result nothing reads above bit 31.
//
// A backward dataflow over the laid-out function: per program point, the
// general registers whose bits 32-63 a later instruction may read. A 32-bit
// operation reads its operands' low halves only and clears its result's upper
// half, so the fact dies at such a result and is born at the operands of what
// stays 64 bits wide: addresses, wide compares and stores, wide arguments.
namespace native {

namespace {

using lowir::TypeKind;

const uint32_t kEvery = 0xffffu;

uint32_t bit(Reg r)
{
	return r < 16 ? 1u << r : 0u;
}

// The registers an operand's address is formed from.
uint32_t address_regs(const Operand& op)
{
	return op.kind == OperandKind::Mem ? bit(op.reg) | bit(op.index) : 0u;
}

// Every general register an instruction names.
uint32_t named_regs(const Inst& inst)
{
	uint32_t regs = 0;
	for (unsigned k = 0; k < inst.nops; ++k)
		regs |= inst.ops[k].kind == OperandKind::Reg ? bit(inst.ops[k].reg) : address_regs(inst.ops[k]);
	return regs;
}

// The bytes a two-address integer operation works on (the emitter's rule).
unsigned alu_bytes(const Inst& inst)
{
	return inst.type == TypeKind::Void || inst.type == TypeKind::Ptr ? 8 : register_bytes(inst.type);
}

struct Halves
{
	Function& f;
	std::vector<uint32_t> in; // per block: the registers wanted on entry
	uint32_t narrowed = 0;

	explicit Halves(Function& function) : f(function), in(function.blocks.size(), 0) {}

	uint32_t entry(uint32_t block) const { return block < in.size() ? in[block] : kEvery; }

	// The registers wanted before insts[i], given those wanted after it;
	// with `rewrite`, the instruction takes its 32-bit form where it may.
	uint32_t before(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite);
	uint32_t two_address(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite);
	uint32_t shift(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite);
	uint32_t call(const Inst& inst, uint32_t wanted) const;
	bool sweep(bool rewrite);
};

// add, sub, and, or, xor, adc, sbb, imul, neg, not.
uint32_t Halves::two_address(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite)
{
	Inst& inst = f.insts[i];
	const Operand& dst = inst.ops[0];
	const bool unary = inst.op == Op::Neg || inst.op == Op::Not;
	const unsigned w = unary ? (narrow_kind(inst.type) ? 4 : 8) : alu_bytes(inst);
	// imul dst, src, imm reads no destination.
	const bool reads_dst = !(inst.op == Op::Imul && inst.nops == 3);
	uint32_t sources = 0, addresses = 0;
	for (unsigned k = 1; k < inst.nops; ++k)
	{
		sources |= inst.ops[k].kind == OperandKind::Reg ? bit(inst.ops[k].reg) : 0u;
		addresses |= address_regs(inst.ops[k]);
	}
	if (dst.kind != OperandKind::Reg)
		return wanted | address_regs(dst) | addresses | (w == 8 ? sources : 0u);
	if (dst.reg >= 16)
		return kEvery;
	const uint32_t d = bit(dst.reg);
	if (w < 4)
		return wanted | addresses;
	if (w == 4)
		return (wanted & ~d) | addresses;
	// An `and` with a mask that clears the upper half is encoded in 32 bits.
	if (inst.op == Op::And && inst.ops[1].is_imm() && inst.ops[1].imm >= 0 && inst.ops[1].imm <= 0xffffffffll)
		return wanted & ~d;
	const bool carries = inst.op == Op::Adc || inst.op == Op::Sbb;
	if ((wanted & d) || carries || (inst.op != Op::Not && !flags_dead_after(f.insts, i, end)))
		return wanted | (reads_dst ? d : 0u) | sources | addresses;
	// The low half of the result comes from the operands' low halves. A
	// multiplication by a constant keeps its 64-bit strength reduction.
	if (rewrite && !(inst.op == Op::Imul && (inst.nops == 3 || inst.ops[1].is_imm())))
	{
		inst.type = TypeKind::U32;
		if (inst.nops == 2 && inst.ops[1].is_imm())
			inst.ops[1].imm = static_cast<int32_t>(inst.ops[1].imm);
		++narrowed;
	}
	return (wanted & ~d) | addresses;
}

uint32_t Halves::shift(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite)
{
	Inst& inst = f.insts[i];
	const Operand& dst = inst.ops[0];
	const unsigned w = inst.type == TypeKind::U32 ? 4 : 8;
	if (dst.kind != OperandKind::Reg)
		return wanted | address_regs(dst);
	if (dst.reg >= 16)
		return kEvery;
	const uint32_t d = bit(dst.reg);
	const bool constant = inst.ops[1].is_imm();
	const unsigned count = constant ? static_cast<unsigned>(inst.ops[1].imm & (w * 8 - 1)) : 0;
	// A count of zero encodes nothing.
	if (constant && count == 0)
		return wanted;
	if (w == 4)
		return wanted & ~d;
	if (inst.op == Op::Shl && constant && count < 32 && !(wanted & d) && flags_dead_after(f.insts, i, end))
	{
		if (rewrite)
		{
			inst.type = TypeKind::U32;
			++narrowed;
		}
		return wanted;
	}
	return wanted | d;
}

// A call leaves the registers it preserves as they are and reads its target
// and the arguments wider than 32 bits; an unwinding one continues at its
// landing with the preserved registers.
uint32_t Halves::call(const Inst& inst, uint32_t wanted) const
{
	if (inst.op == Op::TailJump)
		wanted = 0;
	if (inst.eh != 0)
	{
		if (inst.eh > f.eh.landings.size())
			return kEvery;
		const EhLanding& landing = f.eh.landings[inst.eh - 1];
		wanted |= (landing.pad != lowir::kNoId ? entry(landing.pad) : 0u) |
			(landing.select != lowir::kNoId ? entry(landing.select) : 0u);
	}
	return (wanted & ~kCallerSavedGprs) | ((inst.aux & kEvery) & ~narrow_arguments(inst)) | named_regs(inst);
}

uint32_t Halves::before(uint32_t i, uint32_t end, uint32_t wanted, bool rewrite)
{
	Inst& inst = f.insts[i];
	const Operand& a = inst.ops[0];
	const Operand& b = inst.ops[1];
	// A branch continues at its target too.
	for (unsigned k = 0; k < inst.nops; ++k)
		if (inst.ops[k].kind == OperandKind::Label)
			wanted |= entry(inst.ops[k].symbol);
	const bool to_gpr = inst.nops >= 1 && plain_gpr(a);
	const uint32_t d = to_gpr ? bit(a.reg) : 0u;
	switch (inst.op)
	{
	case Op::Nop:
	case Op::Jcc:
	case Op::Jmp:
	case Op::Setcc:
	case Op::Prologue:
		return wanted;
	case Op::Mov:
		if (!to_gpr || inst.nops != 2)
			return kEvery;
		if (b.kind == OperandKind::Reg)
		{
			if (b.reg >= 16)
				return kEvery;
			if (wanted & d)
				return (wanted & ~d) | bit(b.reg);
			if (rewrite && b.reg != a.reg)
			{
				inst.op = Op::Zext;
				inst.type = TypeKind::U32;
				++narrowed;
			}
			return wanted;
		}
		if (rewrite && b.is_imm() && !(wanted & d) && (b.imm < 0 || b.imm > 0xffffffffll))
		{
			inst.ops[1].imm = static_cast<int64_t>(static_cast<uint32_t>(b.imm));
			++narrowed;
		}
		return wanted & ~d;
	case Op::Load:
		if (!to_gpr)
			return kEvery;
		// (A volatile access keeps its width: only a word's extension goes.)
		if (rewrite && !(wanted & d) &&
			(inst.type == TypeKind::I32 ||
				(!(inst.flags & MF_VOLATILE) && (inst.type == TypeKind::I64 || inst.type == TypeKind::Ptr))))
		{
			inst.type = TypeKind::U32;
			++narrowed;
		}
		return (wanted & ~d) | address_regs(b);
	case Op::Store:
		return wanted | address_regs(a) |
			(b.kind == OperandKind::Reg && register_bytes(inst.type) == 8 ? bit(b.reg) : 0u);
	case Op::Lea:
		if (!to_gpr)
			return kEvery;
		if (inst.type == TypeKind::I32)
			return wanted & ~d;
		// (An address of the frame or of a symbol stays whole.)
		if ((wanted & d) || b.kind != OperandKind::Mem || b.symbol != lowir::kNoId || b.reg >= 16 ||
			(b.index != kNoReg && b.index >= 16))
			return (wanted & ~d) | address_regs(b);
		if (rewrite)
		{
			inst.type = TypeKind::I32;
			++narrowed;
		}
		return wanted;
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Adc:
	case Op::Sbb:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
		return two_address(i, end, wanted, rewrite);
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
		return shift(i, end, wanted, rewrite);
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
		// (A wide test of a mask below bit 31 is encoded in 32 bits.)
		if (register_bytes(inst.type) != 8 ||
			(inst.op == Op::Test && b.is_imm() && b.imm >= 0 && b.imm <= 0x7fffffffll))
			return wanted | address_regs(a) | address_regs(b);
		return wanted | named_regs(inst);
	case Op::Cmov:
		if (!to_gpr)
			return kEvery;
		return wanted | address_regs(b) | ((wanted & d) && b.kind == OperandKind::Reg ? bit(b.reg) : 0u);
	case Op::Movzx:
		return to_gpr ? wanted & ~d : kEvery;
	case Op::Sext:
	case Op::Zext:
	{
		if (!to_gpr || register_bytes(inst.type) == 8)
			return kEvery;
		// An extension to 64 bits nothing reads the upper half of is a
		// 32-bit copy, or nothing in place.
		const TypeKind word = inst.op == Op::Sext ? TypeKind::I32 : TypeKind::U32;
		if (rewrite && !(wanted & d) && inst.type == word)
		{
			if (inst.nops == 2 && b.reg != a.reg)
			{
				narrowed += inst.op == Op::Sext ? 1 : 0;
				inst.op = Op::Zext;
				inst.type = TypeKind::U32;
			}
			else
			{
				inst.op = Op::Nop;
				inst.nops = 0;
				++narrowed;
			}
		}
		return wanted & ~d;
	}
	case Op::Cqo:
		return inst.type == TypeKind::I32 ? wanted & ~bit(RDX) : (wanted & ~bit(RDX)) | bit(RAX);
	case Op::Idiv:
	case Op::Div:
		if (inst.type == TypeKind::I32)
			return (wanted & ~(bit(RAX) | bit(RDX))) | address_regs(a);
		return wanted | bit(RAX) | bit(RDX) | named_regs(inst);
	case Op::MulWide:
	case Op::ImulWide:
		return (wanted & ~bit(RDX)) | bit(RAX) | named_regs(inst);
	case Op::Call:
	case Op::TailJump:
		return call(inst, wanted);
	case Op::Ret:
		if (inst.nops == 0)
			return bit(RAX) | bit(RDX);
		return a.kind == OperandKind::Reg && !narrow_integer(inst.type) ? bit(a.reg) : 0u;
	case Op::JumpTable:
	case Op::IndirectJump:
		wanted = named_regs(inst);
		for (uint32_t k = inst.aux; k < inst.aux + inst.aux2 && k < f.jump_targets.size(); ++k)
			wanted |= entry(f.jump_targets[k]);
		return wanted;
	case Op::CopyBytes:
	case Op::ZeroBytes:
		return (inst.flags & MF_COPY_DYNAMIC) ? kEvery : wanted | named_regs(inst);
	case Op::FMov:
	case Op::FAdd:
	case Op::FSub:
	case Op::FMul:
	case Op::FDiv:
	case Op::FNeg:
	case Op::FAbs:
	case Op::FCmp:
	case Op::Vector:
		return wanted | named_regs(inst);
	default:
		return kEvery;
	}
}

// One pass over the blocks, last to first. Returns whether an entry grew.
bool Halves::sweep(bool rewrite)
{
	// The stack and frame registers, and the base of over-aligned objects.
	const uint32_t held = bit(RSP) | (f.omit_frame_pointer ? 0u : bit(RBP)) | reserved_gprs(f);
	bool grew = false;
	for (uint32_t b = static_cast<uint32_t>(f.blocks.size()); b-- != 0;)
	{
		const Block& block = f.blocks[b];
		const uint32_t end = block.first + block.count;
		Op last = Op::Nop;
		for (uint32_t i = end; i-- != block.first && last == Op::Nop;)
			last = f.insts[i].op;
		const bool leaves = last == Op::Jmp || last == Op::Ret || last == Op::TailJump || last == Op::JumpTable ||
			last == Op::IndirectJump || last == Op::Trap || last == Op::Resume || last == Op::Throw;
		uint32_t wanted = leaves ? 0u : entry(b + 1);
		for (uint32_t i = end; i-- != block.first;)
			wanted = before(i, end, wanted | held, rewrite) | held;
		if (wanted & ~in[b])
		{
			in[b] |= wanted;
			grew = true;
		}
	}
	return grew;
}

} // namespace

uint32_t narrow_unread_halves(Function& function)
{
	// Opaque assembly and the course's handler records transfer control in
	// ways the blocks do not show.
	if (function.assembly || function.eh_records_pushed || function.blocks.empty())
		return 0;
	for (const Inst& inst : function.insts)
		if (inst.op == Op::EhPush || inst.op == Op::Asm || inst.op == Op::WideCmpBranch)
			return 0;
	Halves halves(function);
	while (halves.sweep(false))
	{
	}
	halves.sweep(true);
	return halves.narrowed;
}

} // namespace native
