#include "native/x86/assembler.h"

namespace native {

namespace {

unsigned scale_bits(uint8_t scale)
{
	switch (scale)
	{
	case 2:
		return 1;
	case 4:
		return 2;
	case 8:
		return 3;
	default:
		return 0;
	}
}

} // namespace

uint32_t Assembler::new_label()
{
	label_pos_.push_back(-1);
	return static_cast<uint32_t>(label_pos_.size() - 1);
}

void Assembler::bind(uint32_t label)
{
	label_pos_[label] = static_cast<int32_t>(code_.size());
}

void Assembler::reset_labels()
{
	label_pos_.clear();
	label_refs_.clear();
}

bool Assembler::resolve_labels()
{
	for (size_t i = 0; i < label_refs_.size(); ++i)
	{
		const LabelRef& ref = label_refs_[i];
		if (label_pos_[ref.label] < 0)
			return false;
		if (ref.base != kNoLabel && label_pos_[ref.base] < 0)
			return false;
		const int64_t value = static_cast<int64_t>(label_pos_[ref.label]) -
			(ref.base != kNoLabel ? label_pos_[ref.base] : static_cast<int64_t>(ref.offset) + ref.end);
		if (ref.size == 1)
		{
			if (!fits_i8(value))
				return false;
			code_[ref.offset] = static_cast<uint8_t>(value);
		}
		else
		{
			for (unsigned k = 0; k < 4; ++k)
				code_[ref.offset + k] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> (8 * k));
		}
	}
	label_refs_.clear();
	return true;
}

void Assembler::imm(int64_t value, unsigned bytes)
{
	for (unsigned k = 0; k < bytes; ++k)
		code_.push_back(static_cast<uint8_t>(static_cast<uint64_t>(value) >> (8 * k)));
}

void Assembler::op(uint8_t prefix, bool rex_w, uint32_t opcode, unsigned opcode_bytes, unsigned reg, const RM& rm,
	bool byte_regs, unsigned imm_bytes)
{
	if (prefix != 0)
		byte(prefix);
	uint8_t rex = static_cast<uint8_t>(0x40 | (rex_w ? 8 : 0) | ((reg & 8) ? 4 : 0));
	bool force = false;
	if (rm.is_reg)
	{
		if (rm.reg & 8)
			rex |= 1;
		if (byte_regs && (rm.reg & 15) >= 4 && (rm.reg & 15) <= 7 && rm.reg < 16)
			force = true;
	}
	else
	{
		if (rm.base != kNoReg && (rm.base & 8))
			rex |= 1;
		if (rm.index != kNoReg && (rm.index & 8))
			rex |= 2;
	}
	// The reg field names a byte register only for the r8 forms (mov,
	// test, xchg and the ALU operations); elsewhere it is an opcode
	// extension or a full-width destination (movzx, setcc, shifts).
	const bool byte_reg_field = byte_regs &&
		((opcode_bytes == 1 &&
			 (opcode == 0x88 || opcode == 0x8a || opcode == 0x84 || opcode == 0x86 ||
				 (opcode < 0x40 && ((opcode & 7) == 0 || (opcode & 7) == 2)))) ||
			(opcode_bytes == 2 && (opcode == 0x0fb0 || opcode == 0x0fc0))); // cmpxchg, xadd r/m8, r8
	if (byte_reg_field && (reg & 15) >= 4 && (reg & 15) <= 7 && reg < 16)
		force = true;
	if (rex != 0x40 || force)
		byte(rex);
	for (unsigned k = opcode_bytes; k-- > 0;)
		byte(static_cast<uint8_t>(opcode >> (8 * k)));
	const unsigned r = (reg & 7) << 3;
	if (rm.is_reg)
	{
		byte(static_cast<uint8_t>(0xc0 | r | (rm.reg & 7)));
		return;
	}
	if (rm.symbol != 0xffffffffu || rm.label != 0xffffffffu)
	{
		byte(static_cast<uint8_t>(0x05 | r));
		if (rm.symbol != 0xffffffffu)
		{
			CodeFixup fixup;
			fixup.offset = size();
			fixup.target = rm.symbol;
			fixup.addend = rm.disp;
			fixup.end = static_cast<uint8_t>(4 + imm_bytes);
			fixup.absolute = false;
			fixup.branch = false;
			fixups_.push_back(fixup);
			imm(0, 4);
		}
		else
		{
			LabelRef ref;
			ref.offset = size();
			ref.label = rm.label;
			ref.size = 4;
			ref.end = static_cast<uint8_t>(4 + imm_bytes);
			label_refs_.push_back(ref);
			imm(rm.disp, 4);
		}
		return;
	}
	if (rm.base == kNoReg)
	{
		byte(static_cast<uint8_t>(0x04 | r));
		const unsigned index = rm.index == kNoReg ? 4 : (rm.index & 7);
		byte(static_cast<uint8_t>((scale_bits(rm.scale) << 6) | (index << 3) | 5));
		imm(rm.disp, 4);
		return;
	}
	const unsigned base = rm.base & 7;
	unsigned mod = 2;
	if (rm.disp == 0 && base != 5)
		mod = 0;
	else if (fits_i8(rm.disp))
		mod = 1;
	if (rm.index != kNoReg || base == 4)
	{
		byte(static_cast<uint8_t>((mod << 6) | r | 4));
		const unsigned index = rm.index == kNoReg ? 4 : (rm.index & 7);
		byte(static_cast<uint8_t>((scale_bits(rm.scale) << 6) | (index << 3) | base));
	}
	else
		byte(static_cast<uint8_t>((mod << 6) | r | base));
	if (mod == 1)
		imm(rm.disp, 1);
	else if (mod == 2)
		imm(rm.disp, 4);
}

void Assembler::op_plus(uint8_t prefix, bool rex_w, uint8_t opcode, Reg reg)
{
	if (prefix != 0)
		byte(prefix);
	if (rex_w || (reg & 8))
		byte(static_cast<uint8_t>(0x40 | (rex_w ? 8 : 0) | ((reg & 8) ? 1 : 0)));
	byte(static_cast<uint8_t>(opcode + (reg & 7)));
}

void Assembler::mov_rr(Reg dst, Reg src)
{
	if (dst != src)
		op(0, true, 0x8b, 1, dst, RM::r(src));
}

void Assembler::mov_ri(Reg dst, int64_t value)
{
	if (value == 0)
		op(0, false, 0x33, 1, dst, RM::r(dst));
	else if (value > 0 && value <= 0xffffffffll)
	{
		op_plus(0, false, 0xb8, dst);
		imm(value, 4);
	}
	else if (fits_i32(value))
	{
		op(0, true, 0xc7, 1, 0, RM::r(dst), false, 4);
		imm(value, 4);
	}
	else
	{
		op_plus(0, true, 0xb8, dst);
		imm(value, 8);
	}
}

void Assembler::mov_load(Reg dst, const RM& src, unsigned bytes, bool sign)
{
	switch (bytes)
	{
	case 1:
		op(0, sign, sign ? 0x0fbe : 0x0fb6, 2, dst, src, true);
		break;
	case 2:
		op(0, sign, sign ? 0x0fbf : 0x0fb7, 2, dst, src);
		break;
	case 4:
		if (sign)
			op(0, true, 0x63, 1, dst, src);
		else
			op(0, false, 0x8b, 1, dst, src);
		break;
	default:
		op(0, true, 0x8b, 1, dst, src);
		break;
	}
}

void Assembler::mov_store(const RM& dst, Reg src, unsigned bytes)
{
	switch (bytes)
	{
	case 1:
		op(0, false, 0x88, 1, src, dst, true);
		break;
	case 2:
		op(0x66, false, 0x89, 1, src, dst);
		break;
	case 4:
		op(0, false, 0x89, 1, src, dst);
		break;
	default:
		op(0, true, 0x89, 1, src, dst);
		break;
	}
}

void Assembler::mov_store_imm(const RM& dst, int64_t value, unsigned bytes)
{
	switch (bytes)
	{
	case 1:
		op(0, false, 0xc6, 1, 0, dst, true, 1);
		imm(value, 1);
		break;
	case 2:
		op(0x66, false, 0xc7, 1, 0, dst, false, 2);
		imm(value, 2);
		break;
	case 4:
		op(0, false, 0xc7, 1, 0, dst, false, 4);
		imm(value, 4);
		break;
	default:
		op(0, true, 0xc7, 1, 0, dst, false, 4);
		imm(value, 4);
		break;
	}
}

void Assembler::lea(Reg dst, const RM& src)
{
	op(0, true, 0x8d, 1, dst, src);
}

void Assembler::mov_got(Reg dst, uint32_t symbol)
{
	op(0, true, 0x8b, 1, dst, RM::rip(symbol, 0));
	fixups_.back().kind = FixupKind::Got;
}

void Assembler::tls_local_exec(Reg dst, uint32_t symbol, int32_t addend)
{
	op(0x64, true, 0x8b, 1, dst, RM::m(kNoReg, 0)); // mov dst, fs:[0]
	op(0, true, 0x81, 1, 0, RM::r(dst), false, 4);  // add dst, imm32
	CodeFixup fixup;
	fixup.offset = size();
	fixup.target = symbol;
	fixup.addend = addend;
	fixup.end = 4;
	fixup.absolute = false;
	fixup.branch = false;
	fixup.kind = FixupKind::TpOff;
	fixups_.push_back(fixup);
	imm(0, 4);
}

void Assembler::tls_initial_exec(Reg dst, uint32_t symbol)
{
	op(0, true, 0x8b, 1, dst, RM::rip(symbol, 0)); // mov dst, [rip + symbol@gottpoff]
	fixups_.back().kind = FixupKind::GotTpOff;
	op(0x64, true, 0x03, 1, dst, RM::m(kNoReg, 0)); // add dst, fs:[0]
}

void Assembler::alu_rr(unsigned digit, Reg dst, const RM& src, unsigned bytes)
{
	// Register sources use the r/m, reg direction (00+/01+), the canonical
	// form for register-register ALU operations.
	if (src.is_reg)
	{
		alu_mr(digit, RM::r(dst), src.reg, bytes);
		return;
	}
	if (bytes == 1)
		op(0, false, digit * 8 + 2, 1, dst, src, true);
	else
		op(bytes == 2 ? 0x66 : 0, bytes == 8, digit * 8 + 3, 1, dst, src);
}

void Assembler::alu_mr(unsigned digit, const RM& dst, Reg src, unsigned bytes)
{
	if (bytes == 1)
		op(0, false, digit * 8, 1, src, dst, true);
	else
		op(bytes == 2 ? 0x66 : 0, bytes == 8, digit * 8 + 1, 1, src, dst);
}

void Assembler::alu_ri(unsigned digit, const RM& dst, int64_t value, unsigned bytes)
{
	if (bytes == 1)
	{
		op(0, false, 0x80, 1, digit, dst, true, 1);
		imm(value, 1);
	}
	else if (fits_i8(value))
	{
		op(bytes == 2 ? 0x66 : 0, bytes == 8, 0x83, 1, digit, dst, false, 1);
		imm(value, 1);
	}
	else
	{
		const unsigned n = bytes == 2 ? 2 : 4;
		op(bytes == 2 ? 0x66 : 0, bytes == 8, 0x81, 1, digit, dst, false, n);
		imm(value, n);
	}
}

void Assembler::shift_ri(unsigned digit, const RM& dst, unsigned count, unsigned bytes)
{
	const uint8_t prefix = bytes == 2 ? 0x66 : 0;
	if (count == 1)
		op(prefix, bytes == 8, bytes == 1 ? 0xd0 : 0xd1, 1, digit, dst, bytes == 1);
	else
	{
		op(prefix, bytes == 8, bytes == 1 ? 0xc0 : 0xc1, 1, digit, dst, bytes == 1, 1);
		imm(count, 1);
	}
}

void Assembler::shift_cl(unsigned digit, const RM& dst, unsigned bytes)
{
	op(bytes == 2 ? 0x66 : 0, bytes == 8, bytes == 1 ? 0xd2 : 0xd3, 1, digit, dst, bytes == 1);
}

void Assembler::unary(unsigned digit, const RM& dst, unsigned bytes)
{
	op(bytes == 2 ? 0x66 : 0, bytes == 8, bytes == 1 ? 0xf6 : 0xf7, 1, digit, dst, bytes == 1);
}

void Assembler::imul_rr(Reg dst, const RM& src, unsigned bytes)
{
	op(bytes == 2 ? 0x66 : 0, bytes == 8, 0x0faf, 2, dst, src);
}

void Assembler::imul_rri(Reg dst, const RM& src, int32_t value, unsigned bytes)
{
	const uint8_t prefix = bytes == 2 ? 0x66 : 0;
	if (fits_i8(value))
	{
		op(prefix, bytes == 8, 0x6b, 1, dst, src, false, 1);
		imm(value, 1);
	}
	else
	{
		const unsigned n = bytes == 2 ? 2 : 4;
		op(prefix, bytes == 8, 0x69, 1, dst, src, false, n);
		imm(value, n);
	}
}

void Assembler::setcc(Cond cond, Reg dst)
{
	op(0, false, 0x0f90 + cond, 2, 0, RM::r(dst), true);
}

void Assembler::movzx_rr8(Reg dst, Reg src)
{
	op(0, false, 0x0fb6, 2, dst, RM::r(src), true);
}

void Assembler::jcc(Cond cond, uint32_t label)
{
	byte(0x0f);
	byte(static_cast<uint8_t>(0x80 + cond));
	LabelRef ref;
	ref.offset = size();
	ref.label = label;
	ref.size = 4;
	ref.end = 4;
	label_refs_.push_back(ref);
	imm(0, 4);
}

void Assembler::jmp(uint32_t label)
{
	byte(0xe9);
	LabelRef ref;
	ref.offset = size();
	ref.label = label;
	ref.size = 4;
	ref.end = 4;
	label_refs_.push_back(ref);
	imm(0, 4);
}

void Assembler::jmp_short(uint32_t label)
{
	byte(0xeb);
	LabelRef ref;
	ref.offset = size();
	ref.label = label;
	ref.size = 1;
	ref.end = 1;
	label_refs_.push_back(ref);
	byte(0);
}

void Assembler::jcc_short(Cond cond, uint32_t label)
{
	byte(static_cast<uint8_t>(0x70 + cond));
	LabelRef ref;
	ref.offset = size();
	ref.label = label;
	ref.size = 1;
	ref.end = 1;
	label_refs_.push_back(ref);
	byte(0);
}

void Assembler::call_symbol(uint32_t symbol)
{
	byte(0xe8);
	CodeFixup fixup;
	fixup.offset = size();
	fixup.target = symbol;
	fixup.addend = 0;
	fixup.end = 4;
	fixup.absolute = false;
	fixup.branch = true;
	fixups_.push_back(fixup);
	imm(0, 4);
}

void Assembler::jmp_symbol(uint32_t symbol)
{
	byte(0xe9);
	CodeFixup fixup;
	fixup.offset = size();
	fixup.target = symbol;
	fixup.addend = 0;
	fixup.end = 4;
	fixup.absolute = false;
	fixup.branch = true;
	fixups_.push_back(fixup);
	imm(0, 4);
}

void Assembler::call_rm(const RM& target)
{
	op(0, false, 0xff, 1, 2, target);
}

void Assembler::jmp_rm(const RM& target)
{
	op(0, false, 0xff, 1, 4, target);
}

void Assembler::label_offset32(uint32_t label, uint32_t base)
{
	LabelRef ref;
	ref.offset = size();
	ref.label = label;
	ref.size = 4;
	ref.end = 0;
	ref.base = base;
	label_refs_.push_back(ref);
	imm(0, 4);
}

void Assembler::push(Reg reg)
{
	op_plus(0, false, 0x50, reg);
}

void Assembler::pop(Reg reg)
{
	op_plus(0, false, 0x58, reg);
}

void Assembler::sse(uint8_t prefix, uint8_t opcode, Reg dst, const RM& src, bool rex_w)
{
	op(prefix, rex_w, 0x0f00 | opcode, 2, dst & 15, src);
}

void Assembler::sse_store(uint8_t prefix, uint8_t opcode, const RM& dst, Reg src)
{
	op(prefix, false, 0x0f00 | opcode, 2, src & 15, dst);
}

void Assembler::x87(uint8_t opcode, unsigned digit, const RM& mem)
{
	op(0, false, opcode, 1, digit, mem);
}

} // namespace native
