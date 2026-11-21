#include "aarch64/selector.h"
#include "aarch64/lifetimes.h"

#include <algorithm>

namespace aarch64 {
using namespace lowir;

namespace {
bool narrow(Type type)
{
	const unsigned bits = integer_bits(type.kind());
	return bits == 8 || bits == 16 || bits == 32;
}

// Whether `sub` begins a narrow range test, which narrow_range_branch
// selects on its operand as normalized.
bool range_test(const FunctionBody& body, const lowir::Instruction& sub)
{
	const unsigned width = integer_bits(sub.type.kind());
	if ((width != 8 && width != 16) || !sub.ops[1].is(ValueKind::Integer) || sub.next == kNoId)
		return false;
	const auto& cmp = body.instructions[sub.next];
	return cmp.op == Opcode::Compare && cmp.type == sub.type && cmp.ops[0] == Value::temp(sub.result) &&
		cmp.ops[1].is(ValueKind::Integer) && cmp.next != kNoId && body.instructions[cmp.next].op == Opcode::Branch;
}

// Whether the selected code for `inst` reads only the low bits of the
// operand in slot `k` (ops[0..2], then the extra operands), which holds `t`.
// Each case names a selection that uses W registers or a field of the
// operand's width: the simple arithmetic and logical forms, shifts by a
// literal, 32-bit division, comparison, selection and branch conditions,
// the stores of the same width, truncation, sign extension and 32-bit
// subscripts. The paths that combine these instructions with a neighbour
// keep that property, except a narrow range test (range_test).
bool low_bits(const FunctionBody& body, const lowir::Instruction& inst, unsigned k, Type type)
{
	const unsigned bits = integer_bits(type.kind());
	switch (inst.op)
	{
	case Opcode::Binary:
		if (inst.type != type || k > 1)
			return false;
		switch (static_cast<BinaryOp>(inst.sub))
		{
		case BinaryOp::Sub:
			return !range_test(body, inst);
		case BinaryOp::Add:
		case BinaryOp::Or:
		case BinaryOp::Xor:
			return true;
		case BinaryOp::And:
			return bits == 32 || inst.ops[1 - k].is(ValueKind::Integer);
		case BinaryOp::Mul:
			return bits == 32;
		case BinaryOp::Shl:
		case BinaryOp::UShr:
			return bits == 32 || (k == 0 && inst.ops[1].is(ValueKind::Integer));
		case BinaryOp::Shr:
		case BinaryOp::Div:
		case BinaryOp::UDiv:
		case BinaryOp::Mod:
		case BinaryOp::UMod:
			return bits == 32;
		default:
			return false;
		}
	case Opcode::Compare:
		// Every comparison of 32-bit operands reads W registers; narrower
		// comparisons read the normalized register.
		return inst.type == type && k <= 1 && bits == 32;
	case Opcode::Select:
		// The condition and CSEL or CSINC of 32-bit alternatives read W
		// registers; the path for objects and symbol alternatives does not.
		if (bits != 32 || memory_value(inst.type) || inst.ops[1].is(ValueKind::Symbol) ||
			inst.ops[2].is(ValueKind::Symbol))
			return false;
		return k == 0 || (inst.type == type && k <= 2);
	case Opcode::Branch:
		return k == 0 && bits == 32;
	case Opcode::Index:
		// A 32-bit subscript is an SXTW or UXTW extended register, or is
		// extended first; only an unsigned subscript of an element larger
		// than 16 bytes is shifted as a doubleword (address_sequence).
		return k == 1 && bits == 32 && (type.kind() == TypeKind::I32 || storage_bytes(inst.type) <= 16);
	case Opcode::Unary:
		return inst.type == type && k == 0 &&
			(inst.sub == static_cast<unsigned>(UnaryOp::Neg) || inst.sub == static_cast<unsigned>(UnaryOp::BitNot));
	case Opcode::Store:
		return inst.type == type && k == 0;
	case Opcode::Convert:
	{
		if (k != 0)
			return false;
		if (inst.sub == static_cast<unsigned>(ConvertOp::Trunc))
			return true;
		// A sign extension is SBFM of the low bits, except of a value
		// zero-extended from fewer bits, which it moves as it is.
		const InstId def = body.temps[inst.ops[0].index].def;
		return inst.sub == static_cast<unsigned>(ConvertOp::SExt) &&
			(def == kNoId || body.instructions[def].op != Opcode::Convert ||
				body.instructions[def].sub != static_cast<unsigned>(ConvertOp::ZExt));
	}
	default:
		return false;
	}
}
}

// Integer values of 8, 16 and 32 bits are normalized: held zero-extended
// to 64 bits in their register or home. Extensions, switches, calls,
// returns and most readers of 8- and 16-bit values rely on it, and a value
// computed by an X-register instruction or arriving as a parameter must be
// extended before it is saved. A value is loose instead, its bits above its
// width undefined, when every reader takes only its low bits (low_bits), and
// save() then leaves its high bits alone. A phi or copy to a type of the same
// width reads its input's low bits only if its own result is loose, so loose
// values form webs. A zero extension's source stays normalized, since the
// extension may share its register (scalar_aliases). A store of a value into
// an object word held in a register (`word_accesses`) moves the whole
// register; before allocation, when those words are not yet known, every
// store reads all of its value's bits, so a value loose then stays loose.
std::vector<uint8_t> loose_values(
	const FunctionBody& body, const std::vector<uint8_t>& word_accesses, bool before_allocation)
{
	std::vector<uint8_t> loose(body.temps.size(), 0);
	bool any = false;
	for (TempId t = 0; t < body.temps.size(); ++t)
		if (narrow(body.temps[t].type))
		{
			loose[t] = 1;
			any = true;
		}
	if (!any)
		return loose;
	// Transfers from an input to a phi or copy result, kept by result.
	std::vector<std::pair<TempId, TempId>> transfers;
	std::vector<TempId> strict;
	const auto mark = [&](TempId t) {
		if (loose[t])
		{
			loose[t] = 0;
			strict.push_back(t);
		}
	};
	for (const auto& block : body.blocks)
		for (InstId i = block.first; i != kNoId; i = body.instructions[i].next)
		{
			const auto& inst = body.instructions[i];
			const bool object_word = before_allocation ? inst.op == Opcode::Store
													   : !word_accesses.empty() && word_accesses[i];
			const bool transfer = inst.op == Opcode::Phi || inst.op == Opcode::Copy;
			const auto visit = [&](Value v, unsigned k) {
				if (!v.is(ValueKind::Temp) || !loose[v.index])
					return;
				if (transfer && inst.result != kNoId && narrow(body.temps[inst.result].type) &&
					integer_bits(body.temps[inst.result].type.kind()) == integer_bits(body.temps[v.index].type.kind()))
					transfers.emplace_back(inst.result, v.index);
				else if (object_word || !low_bits(body, inst, k, body.temps[v.index].type))
					mark(v.index);
			};
			for (unsigned k = 0; k < 3; ++k)
				visit(inst.ops[k], k);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				visit(body.operands[inst.extra_first + k], 3 + k);
		}
	if (transfers.empty())
		return loose;
	std::sort(transfers.begin(), transfers.end());
	while (!strict.empty())
	{
		const TempId t = strict.back();
		strict.pop_back();
		for (auto it = std::lower_bound(transfers.begin(), transfers.end(), std::make_pair(t, TempId(0)));
			it != transfers.end() && it->first == t; ++it)
			mark(it->second);
	}
	return loose;
}

void Selector::find_loose_values()
{
	loose_.clear();
	if (level_)
		loose_ = loose_values(body_, word_accesses_, false);
}

} // namespace aarch64
