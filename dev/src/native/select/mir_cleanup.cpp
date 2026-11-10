#include "native/select/mir_cleanup.h"

#include <algorithm>
#include <array>
#include <climits>

#include "native/mir/type_kinds.h"
#include "native/select/frame_extraction.h"

// MIR cleanups after selection (O1+): see mir_cleanup.h.
namespace native {

namespace {

// A complete run of zero bytes can use the direct vector/scalar bulk form.
// Its scratch XMM is reserved by selection; no ordinary register or flags
// are clobbered. Gaps remain gaps, including padding between fields.
bool merge_zero_run(std::vector<Inst>& insts, uint32_t first, uint32_t past, int32_t lo, uint32_t span,
	const uint8_t* value, const uint8_t* written, bool wide)
{
	if (!wide || span < 16)
		return false;
	for (uint32_t p = 0; p < span; ++p)
		if (!written[p] || value[p] != 0)
			return false;
	Inst& zero = insts[first];
	zero.op = Op::ZeroBytes;
	zero.type = lowir::TypeKind::Void;
	zero.nops = 1;
	zero.ops[0].disp = lo;
	zero.aux = span;
	zero.aux2 = 1;
	for (uint32_t k = first + 1; k < past; ++k)
	{
		insts[k].op = Op::Nop;
		insts[k].nops = 0;
	}
	return true;
}

// The stores insts[first, past) (immediates, one base): rewritten in place
// when fewer chunks cover their bytes. Gaps stay untouched; zero subranges
// can use vector stores. Overlapping writes must agree byte for byte, and
// the whole run spans at most 64 bytes.
void merge_store_run(std::vector<Inst>& insts, uint32_t first, uint32_t past, bool wide)
{
	const uint32_t kSpan = 64;
	int32_t lo = INT32_MAX, hi = INT32_MIN;
	for (uint32_t k = first; k < past; ++k)
	{
		const int32_t disp = insts[k].ops[0].disp;
		if (disp > INT32_MAX - static_cast<int32_t>(register_bytes(insts[k].type)))
			return;
		lo = std::min(lo, disp);
		hi = std::max(hi, disp + static_cast<int32_t>(register_bytes(insts[k].type)));
	}
	if (static_cast<int64_t>(hi) - lo > kSpan)
		return;
	uint8_t value[kSpan] = {}, written[kSpan] = {};
	for (uint32_t k = first; k < past; ++k)
	{
		const uint32_t at = static_cast<uint32_t>(insts[k].ops[0].disp - lo);
		for (uint32_t b = 0; b < register_bytes(insts[k].type); ++b)
		{
			const uint8_t byte = static_cast<uint8_t>(static_cast<uint64_t>(insts[k].ops[1].imm) >> (8 * b));
			if (written[at + b] && value[at + b] != byte)
				return;
			written[at + b] = 1;
			value[at + b] = byte;
		}
	}
	const uint32_t span = static_cast<uint32_t>(hi - lo);
	if (merge_zero_run(insts, first, past, lo, span, value, written, wide))
		return;
	static const lowir::TypeKind kTypes[9] = { lowir::TypeKind::Void, lowir::TypeKind::I8, lowir::TypeKind::I16,
		lowir::TypeKind::Void, lowir::TypeKind::I32, lowir::TypeKind::Void, lowir::TypeKind::Void,
		lowir::TypeKind::Void, lowir::TypeKind::I64 };
	int32_t disps[kSpan];
	uint8_t widths[kSpan];
	bool zeroes[kSpan] = {};
	int64_t imms[kSpan];
	uint32_t count = 0;
	for (uint32_t p = 0; p < span;)
	{
		if (!written[p])
		{
			++p;
			continue;
		}
		uint32_t end = p;
		if (wide)
			while (end < span && written[end] && value[end] == 0)
				++end;
		if (end - p >= 16)
		{
			disps[count] = lo + static_cast<int32_t>(p);
			widths[count] = static_cast<uint8_t>(end - p);
			imms[count] = 0;
			zeroes[count++] = true;
			p = end;
			continue;
		}
		uint32_t w = 8;
		uint64_t v = 0;
		for (; w > 1; w /= 2)
		{
			bool ok = ((lo + static_cast<int32_t>(p)) & static_cast<int32_t>(w - 1)) == 0 && p + w <= span;
			for (uint32_t b = 0; ok && b < w; ++b)
				ok = written[p + b] != 0;
			v = 0;
			for (uint32_t b = 0; ok && b < w; ++b)
				v |= static_cast<uint64_t>(value[p + b]) << (8 * b);
			if (ok && (w < 8 || (static_cast<int64_t>(v) >= INT32_MIN && static_cast<int64_t>(v) <= INT32_MAX)))
				break;
		}
		if (w == 1)
			v = value[p];
		disps[count] = lo + static_cast<int32_t>(p);
		widths[count] = static_cast<uint8_t>(w);
		imms[count++] = static_cast<int64_t>(v);
		p += w;
	}
	if (count >= past - first)
		return;
	for (uint32_t c = 0; c < count; ++c)
	{
		Inst& inst = insts[first + c];
		if (zeroes[c])
		{
			inst.op = Op::ZeroBytes;
			inst.type = lowir::TypeKind::Void;
			inst.nops = 1;
			inst.ops[0].disp = disps[c];
			inst.aux = widths[c];
			inst.aux2 = 1;
			continue;
		}
		inst.type = kTypes[widths[c]];
		inst.ops[0].disp = disps[c];
		inst.ops[1] = Operand::make_imm(imms[c], imms[c] < 0 ? -1 : 0);
	}
	for (uint32_t k = first + count; k < past; ++k)
	{
		insts[k].op = Op::Nop;
		insts[k].nops = 0;
	}
}

// Loop rotation: the largest loop test copied into a back edge.
const uint32_t kRotateCap = 12;

// A block a latch may end with a copy of: a short test (moves, loads, ALU
// operations, compares) ending in `jcc; jmp`, not the entry or a landing.
bool rotatable(const Function& f, uint32_t b)
{
	const Block& block = f.blocks[b];
	if (b == 0 || block.landing || block.count < 2 || block.count > kRotateCap ||
		f.insts[block.first + block.count - 1].op != Op::Jmp || f.insts[block.first + block.count - 2].op != Op::Jcc)
		return false;
	for (uint32_t i = block.first; i + 2 < block.first + block.count; ++i)
	{
		switch (f.insts[i].op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Lea:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
		case Op::Rol:
		case Op::Ror:
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Setcc:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Nop:
			break;
		default:
			return false;
		}
	}
	return true;
}

// Whether the flags insts[i] sets are dead: a later instruction of the
// block sets them, or the block jumps away, before any reads them (the
// selector never keeps flags across blocks).
bool flags_dead_after(const std::vector<Inst>& insts, uint32_t i, uint32_t end)
{
	for (uint32_t k = i + 1; k < end; ++k)
	{
		const Inst& inst = insts[k];
		switch (inst.op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Store:
		case Op::Lea:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Nop:
			break;
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Neg:
		case Op::Imul:
		case Op::Bsf:
		case Op::Bsr:
		case Op::Popcnt:
		case Op::Idiv:
		case Op::Div:
		case Op::MulWide:
		case Op::ImulWide:
		case Op::FCmp:
		case Op::Call:
		case Op::Ret:
		case Op::Jmp:
		case Op::JumpTable:
			return true;
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
			// A shift by a nonzero immediate sets them; one by cl may leave
			// them (a zero count), reading none: the scan goes on.
			if (inst.ops[1].kind == OperandKind::Imm &&
				(inst.ops[1].imm & (inst.type == lowir::TypeKind::U32 ? 31 : 63)) != 0)
				return true;
			if (inst.ops[1].kind == OperandKind::Imm)
				return false;
			break;
		case Op::Rol:
		case Op::Ror:
			// A rotation writes CF and OF only, reading none: the scan
			// goes on.
			break;
		default:
			return false;
		}
	}
	return false;
}

bool plain_gpr(const Operand& op)
{
	return op.kind == OperandKind::Reg && op.reg < 16;
}

// The rotatable tail has at most twelve instructions and no internal
// control flow. Track local load dependencies through its integer registers
// and flags; an unrelated load does not make a counter test data-dependent.
bool loaded_condition(const Function& f, const Block& block)
{
	uint32_t loaded = 0;
	bool loaded_flags = false;
	for (uint32_t i = block.first; i < block.first + block.count; ++i)
	{
		const Inst& inst = f.insts[i];
		if (inst.op == Op::Nop)
			continue;
		if (inst.op == Op::Jcc)
			return loaded_flags;
		const bool assignment = inst.op == Op::Mov || inst.op == Op::Load || inst.op == Op::Lea ||
			inst.op == Op::Movzx || inst.op == Op::Sext || inst.op == Op::Zext;
		bool depends = inst.op == Op::Load || (inst.op == Op::Setcc && loaded_flags);
		for (unsigned k = assignment || inst.op == Op::Setcc ? 1 : 0; k < inst.nops; ++k)
		{
			const Operand& op = inst.ops[k];
			if (plain_gpr(op))
				depends = depends || (loaded & reg_bit(op.reg)) != 0;
			else if (op.kind == OperandKind::Mem)
				depends = depends || inst.op != Op::Lea ||
					(loaded & ((op.reg < 16 ? reg_bit(op.reg) : 0) | (op.index < 16 ? reg_bit(op.index) : 0))) != 0;
		}
		const bool compare = inst.op == Op::Cmp || inst.op == Op::Test || inst.op == Op::Bt;
		if (!compare && inst.nops && plain_gpr(inst.ops[0]))
		{
			loaded &= ~reg_bit(inst.ops[0].reg);
			if (depends)
				loaded |= reg_bit(inst.ops[0].reg);
		}
		if (!assignment && inst.op != Op::Nop && inst.op != Op::Setcc)
			loaded_flags = depends;
	}
	return false;
}

} // namespace

// O1+: a run of consecutive stores of immediates to adjacent bytes of one
// base (a struct zeroed or set field by field) becomes the fewest aligned
// 8/4/2/1-byte stores of the combined bytes (an 8-byte one when they fit a
// sign-extended imm32); the rest become nops. A wider load of those bytes
// then forwards from one store instead of stalling on several. Linear in
// the function's MIR.
void merge_constant_stores(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	auto mergeable = [&](const Inst& inst) {
		return inst.op == Op::Store && inst.ops[1].kind == OperandKind::Imm && inst.ops[0].kind == OperandKind::Mem &&
			inst.ops[0].index == kNoReg && inst.ops[0].symbol == lowir::kNoId && inst.ops[0].reg != kNoReg &&
			!(inst.flags & MF_VOLATILE);
	};
	for (const Block& block : function.blocks)
	{
		for (uint32_t i = block.first; i < block.first + block.count;)
		{
			uint32_t j = i;
			int64_t lo = INT32_MAX, hi = INT32_MIN;
			while (j < block.first + block.count && mergeable(insts[j]) && insts[j].ops[0].reg == insts[i].ops[0].reg)
			{
				const int64_t start = insts[j].ops[0].disp;
				const int64_t end = start + register_bytes(insts[j].type);
				const int64_t next_lo = std::min(lo, start), next_hi = std::max(hi, end);
				if (next_hi - next_lo > 64)
					break;
				lo = next_lo;
				hi = next_hi;
				++j;
			}
			if (j - i < 2)
			{
				i = j == i ? i + 1 : j;
				continue;
			}
			merge_store_run(insts, i, j, function.wide_bulk);
			i = j;
		}
	}
}

// Copy a short straight-line dispatch into an incoming jump. Its guards stay
// at their original sites; calls, stores, side exits and landing pads stay
// shared. Charge the table data too. Each emitted copy gets its own table label.
bool dispatch_tail(const Function& f, uint32_t b)
{
	const Block& block = f.blocks[b];
	if (b == 0 || block.landing || block.count == 0 || block.count > kRotateCap)
		return false;
	const Inst& table = f.insts[block.first + block.count - 1];
	if (table.op != Op::JumpTable || table.aux2 == 0 || table.aux2 > kRotateCap - block.count)
		return false;
	for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
	{
		switch (f.insts[i].op)
		{
		case Op::Mov:
		case Op::Load:
		case Op::Lea:
		case Op::Add:
		case Op::Sub:
		case Op::And:
		case Op::Or:
		case Op::Xor:
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
		case Op::Rol:
		case Op::Ror:
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Setcc:
		case Op::Sext:
		case Op::Zext:
		case Op::Movzx:
		case Op::Nop:
			break;
		default:
			return false;
		}
	}
	return true;
}

TailCopies duplicate_short_tails(Function& f)
{
	TailCopies copies;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const Block block = f.blocks[b];
		if (block.count == 0 || block.landing)
			continue;
		const Inst& last = f.insts[block.first + block.count - 1];
		if (last.op != Op::Jmp || last.ops[0].kind != OperandKind::Label)
			continue;
		const uint32_t target = last.ops[0].symbol;
		const bool dispatch = target != b && dispatch_tail(f, target);
		if (!dispatch && (target >= b || !rotatable(f, target)))
			continue;
		const Block tail = f.blocks[target];
		const uint32_t first = static_cast<uint32_t>(f.insts.size());
		const size_t required = f.insts.size() + block.count - 1 + tail.count;
		// Preserve amortized growth when many block tails are copied.
		if (required > f.insts.capacity())
			f.insts.reserve(std::max(required, f.insts.capacity() * 2));
		for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
			f.insts.push_back(f.insts[i]);
		const bool loaded_test = !dispatch && !tail.counted && loaded_condition(f, tail);
		for (uint32_t i = tail.first; i < tail.first + tail.count; ++i)
		{
			Inst copy = f.insts[i];
			// A rotated memory test owns its prediction site. Later tail
			// sharing must not mix its entry and repeated input histories.
			// Counted loops retain sharing even when their bounds are loaded.
			if (loaded_test && copy.op == Op::Jcc)
				copy.aux2 = static_cast<uint32_t>(f.insts.size()) + 1;
			if (copy.op == Op::JumpTable)
			{
				// Each dispatch owns the targets that later frame splitting
				// and block renumbering rewrite. Sharing a slice would apply
				// a block-index permutation once for every copied instruction.
				const uint32_t first_target = static_cast<uint32_t>(f.jump_targets.size());
				for (uint32_t k = 0; k < copy.aux2; ++k)
					f.jump_targets.push_back(f.jump_targets[copy.aux + k]);
				copy.aux = first_target;
			}
			f.insts.push_back(copy);
		}
		f.blocks[b].first = first;
		f.blocks[b].count = static_cast<uint32_t>(f.insts.size()) - first;
		if (dispatch)
			++copies.dispatches;
		else
			++copies.rotations;
	}
	return copies;
}

// O1+: `mov b, a` then, at 32 or 64 bits, `add b, imm|c`, `sub b, imm`,
// `shl b, 1..3` (when nothing reads their flags) or `lea b, [..b..]`
// becomes one lea from a (a 32-bit one for a 32-bit operation: both write
// the low half zero-extended); the copy becomes a nop, as for `imul b, 2|4|8`.
// `imul b, k` of a k the emitter reduces to a lea and a shift computes
// that lea from a.
// Linear.
uint32_t fuse_copies_into_lea(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	uint32_t fused = 0;
	for (const Block& block : function.blocks)
	{
		const uint32_t end = block.first + block.count;
		for (uint32_t i = block.first; i + 1 < end; ++i)
		{
			const Inst& copy = insts[i];
			if (copy.op != Op::Mov || !plain_gpr(copy.ops[0]) || !plain_gpr(copy.ops[1]) ||
				copy.ops[0].reg == copy.ops[1].reg)
				continue;
			const Reg a = copy.ops[1].reg, b = copy.ops[0].reg;
			Inst& next = insts[i + 1];
			if (next.nops < 2 || !(next.ops[0].kind == OperandKind::Reg && next.ops[0].reg == b))
				continue;
			// Widths: a shift is 32-bit only at u32 (emitter shift), other
			// ALU operations at their type's (emitter alu).
			const bool shift = next.op == Op::Shl;
			const bool narrow = next.type == lowir::TypeKind::U32 || (!shift && next.type == lowir::TypeKind::I32);
			const bool wide = shift ? !narrow
									: next.type == lowir::TypeKind::Void || next.type == lowir::TypeKind::Ptr ||
					next.type == lowir::TypeKind::I64;
			Operand mem;
			const Operand& src = next.ops[1];
			if (next.op == Op::Lea)
			{
				if (src.kind != OperandKind::Mem || src.symbol != lowir::kNoId || !(src.reg == b || src.index == b) ||
					src.reg >= 16 || (src.index != kNoReg && src.index >= 16))
					continue;
				if (src.index == b && a == RSP)
					continue;
				mem = src;
				if (mem.reg == b)
					mem.reg = a;
				if (mem.index == b)
					mem.index = a;
			}
			else
			{
				if ((!wide && !narrow) || !flags_dead_after(insts, i + 1, end))
					continue;
				// A 32-bit operation's immediate is its low half.
				if (next.op == Op::Add && src.kind == OperandKind::Imm &&
					(narrow || (src.imm >= INT32_MIN && src.imm <= INT32_MAX)))
					mem = Operand::make_mem(a, static_cast<int32_t>(static_cast<uint32_t>(src.imm)));
				else if (next.op == Op::Sub && src.kind == OperandKind::Imm &&
					(narrow || (src.imm > INT32_MIN && src.imm <= INT32_MAX)))
					mem = Operand::make_mem(a, static_cast<int32_t>(0u - static_cast<uint32_t>(src.imm)));
				else if (next.op == Op::Add && plain_gpr(src) && src.reg != b && (a != RSP || src.reg != RSP))
					mem = src.reg == RSP ? Operand::make_mem(RSP, 0, a, 1) : Operand::make_mem(a, 0, src.reg, 1);
				else if (a == RSP)
					continue;
				else if (next.op == Op::Shl && src.kind == OperandKind::Imm && src.imm == 1)
					mem = Operand::make_mem(a, 0, a, 1);
				else if (next.op == Op::Shl && src.kind == OperandKind::Imm && (src.imm == 2 || src.imm == 3))
					mem = Operand::make_mem(kNoReg, 0, a, static_cast<uint8_t>(1u << src.imm));
				else if (next.op == Op::Imul && src.kind == OperandKind::Imm && src.imm == 2)
					mem = Operand::make_mem(a, 0, a, 1);
				else if (next.op == Op::Imul && src.kind == OperandKind::Imm && (src.imm == 4 || src.imm == 8))
					mem = Operand::make_mem(kNoReg, 0, a, static_cast<uint8_t>(src.imm));
				else if (next.op == Op::Imul && wide && src.kind == OperandKind::Imm && src.imm > 0)
				{
					// The emitter's strength reduction (lea for an odd 3, 5
					// or 9, then a shift) from a: the copy becomes the lea.
					int64_t odd = src.imm;
					int64_t shift = 0;
					while ((odd & 1) == 0)
					{
						odd >>= 1;
						++shift;
					}
					if (odd != 3 && odd != 5 && odd != 9)
						continue;
					insts[i].op = Op::Lea;
					insts[i].type = lowir::TypeKind::Void;
					insts[i].ops[1] = Operand::make_mem(a, 0, a, static_cast<uint8_t>(odd - 1));
					if (shift != 0)
					{
						next.op = Op::Shl;
						next.type = lowir::TypeKind::Void;
						next.ops[1] = Operand::make_imm(shift);
					}
					else
					{
						next.op = Op::Nop;
						next.nops = 0;
						next.ops[0] = next.ops[1] = Operand();
					}
					++fused;
					continue;
				}
				else
					continue;
			}
			next.op = Op::Lea;
			next.nops = 2;
			next.type = narrow ? lowir::TypeKind::I32 : lowir::TypeKind::Void;
			next.ops[1] = mem;
			insts[i].op = Op::Nop;
			insts[i].nops = 0;
			insts[i].ops[0] = insts[i].ops[1] = Operand();
			++fused;
		}
	}
	return fused;
}

namespace {

bool narrow_kind(lowir::TypeKind type)
{
	return type == lowir::TypeKind::I32 || type == lowir::TypeKind::U32;
}

// Whether `inst` names register r (an operand or an address part); calls
// and other operations with implicit registers count as touching it.
bool reads_or_writes(const Inst& inst, Reg r)
{
	if (inst.op == Op::Asm)
		return ((inst.aux | inst.aux2) & reg_bit(r)) != 0;
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Store:
	case Op::Lea:
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
	case Op::Movzx:
	case Op::Sext:
	case Op::Zext:
	case Op::Nop:
		break;
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
		if (inst.ops[1].kind != OperandKind::Imm && r == RCX)
			return true;
		break;
	default:
		return true;
	}
	for (unsigned k = 0; k < inst.nops; ++k)
		if (inst.ops[k].uses_reg(r))
			return true;
	return false;
}

bool wide_kind(lowir::TypeKind type)
{
	return type == lowir::TypeKind::Void || type == lowir::TypeKind::Ptr || type == lowir::TypeKind::I64;
}

// Whether `inst` leaves the upper half of its destination register zero:
// a 32-bit ALU operation, lea or shift, a zero-extending load, an
// extension from at most 32 bits, a shift right by 32 or more, an
// immediate below 2^32, a copy of such a register (`known`).
bool zeroes_upper(const Inst& inst, uint32_t known)
{
	const Operand& src = inst.ops[1];
	switch (inst.op)
	{
	case Op::Load:
		return inst.type == lowir::TypeKind::U8 || inst.type == lowir::TypeKind::U16 ||
			inst.type == lowir::TypeKind::U32;
	case Op::Mov:
		if (src.kind == OperandKind::Imm)
			return src.imm >= 0 && src.imm <= 0xffffffffll;
		return src.kind == OperandKind::Reg && src.reg < 16 && ((known >> src.reg) & 1u);
	case Op::Add:
	case Op::Sub:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
		return narrow_kind(inst.type);
	case Op::And:
		return narrow_kind(inst.type) || (src.kind == OperandKind::Imm && src.imm >= 0 && src.imm <= 0xffffffffll);
	case Op::Lea:
		return inst.type == lowir::TypeKind::I32;
	case Op::Shl:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
		return inst.type == lowir::TypeKind::U32;
	case Op::Shr:
		return inst.type == lowir::TypeKind::U32 || (src.kind == OperandKind::Imm && (src.imm & 63) >= 32);
	case Op::Movzx:
		return true;
	case Op::Zext:
		return register_bytes(inst.type) <= 4;
	default:
		return false;
	}
}

// A plain instruction: the emitter encodes it without implicit registers
// (no scratch, call or fixed register), so it writes only its destination.
bool simple_inst(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Lea:
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
	case Op::Setcc:
	case Op::Cmov:
	case Op::Movzx:
	case Op::Sext:
	case Op::Zext:
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
	case Op::Store:
	case Op::Nop:
		break;
	default:
		return false;
	}
	for (unsigned k = 0; k < inst.nops; ++k)
	{
		const Operand& op = inst.ops[k];
		if ((op.kind == OperandKind::Mem && op.symbol != lowir::kNoId) || op.kind == OperandKind::Symbol ||
			(op.kind == OperandKind::Imm && static_cast<int64_t>(static_cast<int32_t>(op.imm)) != op.imm) ||
			(op.kind == OperandKind::Reg && op.reg >= 16) ||
			(op.kind == OperandKind::Mem && op.index != kNoReg && op.index >= 16))
			return false;
	}
	return true;
}

// The GPR a simple instruction writes, or kNoReg.
Reg written_reg(const Inst& inst)
{
	if (inst.op == Op::Store || inst.op == Op::Cmp || inst.op == Op::Test || inst.op == Op::Bt || inst.op == Op::Nop ||
		inst.ops[0].kind != OperandKind::Reg)
		return kNoReg;
	return inst.ops[0].reg;
}

// Whether a simple instruction reads its destination register too (two
// address forms, partial writes, in-place extensions).
bool reads_destination(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Mov:
	case Op::Load:
	case Op::Lea:
		return false;
	case Op::Sext:
	case Op::Zext:
		return inst.nops == 1;
	default:
		return true;
	}
}

// Whether a simple instruction reads register r (an address part, a source
// operand, or a destination it also reads).
bool reads_reg(const Inst& inst, Reg r)
{
	if (inst.op == Op::Asm)
		return (inst.aux & reg_bit(r)) != 0;
	for (unsigned k = 0; k < inst.nops; ++k)
	{
		const Operand& op = inst.ops[k];
		if (op.kind == OperandKind::Mem && (op.reg == r || op.index == r))
			return true;
		if (op.kind == OperandKind::Reg && op.reg == r && (k != 0 || written_reg(inst) != r || reads_destination(inst)))
			return true;
	}
	return false;
}

} // namespace

namespace {

const int32_t kHoldsNothing = INT32_MIN;
const int32_t kHoldsAny = INT32_MAX; // an unvisited predecessor's: no constraint

// A holder is a home's offset (8-aligned) with the bytes of it the
// register holds in its low bits: 0 for all eight, else 1, 2 or 4.
int32_t held(int32_t offset, unsigned bytes)
{
	return offset | static_cast<int32_t>(bytes >= 8 ? 0 : bytes);
}
int32_t held_home(int32_t h)
{
	return h & ~7;
}
unsigned held_bytes(int32_t h)
{
	return (h & 7) == 0 ? 8u : static_cast<unsigned>(h & 7);
}

// forward_home_reloads' dataflow: per block, the frame home each GPR holds
// at its start (the meet of its predecessors' ends) and the transfer
// through its instructions.
class HomeForwarding
{
public:
	explicit HomeForwarding(Function& f) : f_(f), n_(static_cast<uint32_t>(f.blocks.size())) {}
	uint32_t run();

private:
	void edges();
	void transfer(uint32_t b, int32_t* holds, bool rewrite);
	uint32_t forward_scratch(uint32_t i, uint32_t end, Reg holder);
	bool forward_narrow(uint32_t i, uint32_t end, const int32_t* holds);

	Function& f_;
	uint32_t n_;
	std::vector<int32_t> homes_;
	// Distinct successors and predecessors (CSR, by block).
	std::vector<uint32_t> succ_first_, succs_, pred_first_, preds_;
	// No inflow: the entry, landings, course handlers, and the joins of a
	// block only a host landing reaches (see edges).
	std::vector<uint8_t> cut_;
	std::vector<uint32_t> order_; // reverse postorder from the entry
	std::vector<std::pair<uint32_t, Inst>> extractions_, field_reads_;
	uint32_t removed_ = 0;
};

void HomeForwarding::edges()
{
	succ_first_.assign(n_ + 1, 0);
	succs_.clear();
	cut_.assign(n_, 0);
	for (uint32_t b = 0; b < n_; ++b)
	{
		const Block& block = f_.blocks[b];
		const size_t start = succs_.size();
		cut_[b] = cut_[b] || b == 0 || block.landing;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = f_.insts[i];
			if (inst.op == Op::JumpTable || inst.op == Op::IndirectJump)
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					succs_.push_back(f_.jump_targets[k]);
			else if (inst.op == Op::WideCmpBranch)
			{
				succs_.push_back(inst.ops[2].symbol);
				succs_.push_back(inst.aux);
			}
			else if ((inst.op == Op::Jcc || inst.op == Op::Jmp) && inst.ops[0].kind == OperandKind::Label)
				succs_.push_back(inst.ops[0].symbol);
			else if (inst.op == Op::EhPush)
				for (unsigned k = 0; k < inst.nops; ++k)
					if (inst.ops[k].kind == OperandKind::Label && inst.ops[k].symbol < n_)
					{
						succs_.push_back(inst.ops[k].symbol);
						cut_[inst.ops[k].symbol] = 1;
					}
		}
		const std::vector<uint32_t>::iterator from = succs_.begin() + static_cast<std::ptrdiff_t>(start);
		std::sort(from, succs_.end());
		succs_.erase(std::unique(from, succs_.end()), succs_.end());
		while (succs_.size() > start && succs_.back() >= n_)
			succs_.pop_back();
		succ_first_[b + 1] = static_cast<uint32_t>(succs_.size());
	}
	// Predecessors in block order.
	pred_first_.assign(n_ + 1, 0);
	for (uint32_t t : succs_)
		++pred_first_[t + 1];
	for (uint32_t b = 0; b < n_; ++b)
		pred_first_[b + 1] += pred_first_[b];
	preds_.assign(succs_.size(), 0);
	{
		std::vector<uint32_t> fill(pred_first_.begin(), pred_first_.end() - 1);
		for (uint32_t b = 0; b < n_; ++b)
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
				preds_[fill[succs_[k]]++] = b;
	}
	std::vector<uint32_t> next(n_, 0), stack(1, 0);
	std::vector<uint8_t> seen(n_, 0);
	seen[0] = 1;
	while (!stack.empty())
	{
		const uint32_t b = stack.back();
		if (succ_first_[b] + next[b] < succ_first_[b + 1])
		{
			const uint32_t t = succs_[succ_first_[b] + next[b]++];
			if (!seen[t])
			{
				seen[t] = 1;
				stack.push_back(t);
			}
			continue;
		}
		order_.push_back(b);
		stack.pop_back();
	}
	std::reverse(order_.begin(), order_.end());
	// A block no branch from the entry reaches is reached only through a
	// host landing pad (calls name their pads, no branch does), or not at
	// all: it is never evaluated, so a block it jumps to (a catch rejoining
	// the code after its try) holds nothing at its start.
	if (f_.eh.landings.empty())
		return;
	for (uint32_t b = 0; b < n_; ++b)
		if (!seen[b])
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
				cut_[succs_[k]] = cut_[succs_[k]] | seen[succs_[k]];
}

// A copy of a holder into a scratch register (r10, r11) at insts[i]: its
// readers up to the scratch's next write (or the block's end: neither is
// live across blocks) take the holder when none reads the scratch as a
// destination it also writes and the holder stays unwritten until the
// last; the copy then goes.
uint32_t HomeForwarding::forward_scratch(uint32_t i, uint32_t end, Reg holder)
{
	std::vector<Inst>& insts = f_.insts;
	const Reg scratch = insts[i].ops[0].reg;
	bool dirty = false;
	uint32_t last = i;
	for (uint32_t j = i + 1; j < end; ++j)
	{
		const Inst& next = insts[j];
		// A conditional branch may sit mid-block (a switch's compare chain):
		// the scratch stays live past it.
		if (next.op == Op::Jcc)
			continue;
		if (!simple_inst(next))
		{
			if (next.op != Op::Jmp)
				return 0;
			break;
		}
		const Reg w = written_reg(next);
		if (reads_reg(next, scratch))
		{
			if (dirty || (w == scratch && reads_destination(next)))
				return 0;
			last = j;
		}
		if (w == scratch)
			break;
		if (w == holder)
			dirty = true;
	}
	for (uint32_t k = i + 1; k <= last; ++k)
	{
		Inst& next = insts[k];
		const bool writes = written_reg(next) == scratch;
		for (unsigned n = 0; n < next.nops; ++n)
		{
			Operand& op = next.ops[n];
			if (op.kind == OperandKind::Mem)
			{
				if (op.reg == scratch)
					op.reg = holder;
				if (op.index == scratch)
					op.index = holder;
			}
			else if (op.kind == OperandKind::Reg && op.reg == scratch && !(n == 0 && writes))
				op.reg = holder;
		}
	}
	Inst& copy = insts[i];
	copy.op = Op::Nop;
	copy.nops = 0;
	copy.ops[0] = copy.ops[1] = Operand();
	return 1;
}

// Extract only bytes covered by a known carrier. A shift changes flags,
// unlike a load, so an interior read needs dead flags. Copy the carrier
// before shifting unless the original load already overwrote it.
bool HomeForwarding::forward_narrow(uint32_t i, uint32_t end, const int32_t* holds)
{
	Inst& inst = f_.insts[i];
	const unsigned bytes = register_bytes(inst.type);
	const int32_t at = inst.ops[1].disp;
	Reg holder = kNoReg;
	unsigned shift = 0;
	for (Reg r = 0; r < 16; ++r)
	{
		if (holds[r] == kHoldsNothing || r == RSP)
			continue;
		const int64_t delta = static_cast<int64_t>(at) - held_home(holds[r]);
		if (delta >= 0 && delta + bytes <= held_bytes(holds[r]))
		{
			holder = r;
			shift = static_cast<unsigned>(delta) * 8;
			break;
		}
	}
	if (holder == kNoReg)
		return false;
	if (shift == 0)
		return extract_integer_carrier(inst, i, holder, shift, extractions_);
	if (!frame_storage_elision_allowed(f_) || !flags_dead_after(f_.insts, i, i + std::min(end - i, 64u)))
		return false;
	const Inst original = inst;
	if (!extract_integer_carrier(inst, i, holder, shift, extractions_))
		return false;
	field_reads_.push_back(std::make_pair(i, original));
	return true;
}

void HomeForwarding::transfer(uint32_t b, int32_t* holds, bool rewrite)
{
	std::vector<Inst>& insts = f_.insts;
	const Block& block = f_.blocks[b];
	const uint32_t end = block.first + block.count;
	auto forget_home = [&](int32_t lo, int32_t hi) {
		for (Reg r = 0; r < 16; ++r)
			if (holds[r] != kHoldsNothing && held_home(holds[r]) < hi && lo < held_home(holds[r]) + 8)
				holds[r] = kHoldsNothing;
	};
	auto drop_caller_saved = [&]() {
		for (Reg r = 0; r < 16; ++r)
			if (kCallerSavedGprs & (1u << r))
				holds[r] = kHoldsNothing;
	};
	// Slots (frame locations other than homes) may have their address
	// taken: a write through a pointer or a call ends what registers hold
	// of them.
	auto drop_slots = [&]() {
		for (Reg r = 0; r < 16; ++r)
			if (holds[r] != kHoldsNothing && !std::binary_search(homes_.begin(), homes_.end(), held_home(holds[r])))
				holds[r] = kHoldsNothing;
	};
	auto frame_slot = [&](const Operand& op, int32_t& offset) {
		if (op.kind != OperandKind::Mem || op.reg != kFrame || op.index != kNoReg || op.symbol != lowir::kNoId)
			return false;
		offset = op.disp;
		return true;
	};
	// What every exit agrees on: a conditional branch mid-block (a switch's
	// compare chain) leaves with the state at that point.
	int32_t exits[16];
	for (Reg r = 0; r < 16; ++r)
		exits[r] = kHoldsAny;
	auto leave = [&]() {
		for (Reg r = 0; r < 16; ++r)
			exits[r] = exits[r] == kHoldsAny || exits[r] == holds[r] ? holds[r] : kHoldsNothing;
	};
	for (uint32_t i = block.first; i < end; ++i)
	{
		Inst& inst = insts[i];
		if (inst.op == Op::Jcc)
		{
			leave();
			continue;
		}
		if (inst.op == Op::Jmp || inst.op == Op::IndirectJump)
			continue;
		// A jump table writes its index and a scratch register.
		if (inst.op == Op::JumpTable)
		{
			if (plain_gpr(inst.ops[0]))
				holds[inst.ops[0].reg] = kHoldsNothing;
			holds[R10] = holds[R11] = kHoldsNothing;
			leave();
			continue;
		}
		if (inst.op == Op::Call)
		{
			drop_caller_saved();
			drop_slots();
			continue;
		}
		// Bulk zeroing through xmm15 and immediates writes no register; a
		// copy may use rdi, rsi, rcx and the scratch registers. Either
		// writes its destination's bytes.
		const bool xmm_zero = inst.op == Op::ZeroBytes &&
			(inst.aux == 16 || (f_.wide_bulk && inst.aux > 16 && inst.aux <= 128));
		if (xmm_zero || (inst.op == Op::CopyBytes && !(inst.flags & MF_COPY_DYNAMIC)))
		{
			if (inst.op == Op::CopyBytes)
				drop_caller_saved();
			if (inst.ops[0].kind == OperandKind::Mem && inst.ops[0].reg == kFrame)
				forget_home(inst.ops[0].disp, inst.ops[0].disp + static_cast<int32_t>(inst.aux));
			else
				drop_slots();
			continue;
		}
		if (!simple_inst(inst))
		{
			for (Reg r = 0; r < 16; ++r)
				holds[r] = kHoldsNothing;
			continue;
		}
		int32_t offset = 0;
		const bool wide = inst.type == lowir::TypeKind::I64 || inst.type == lowir::TypeKind::Ptr;
		// A volatile access reads or writes memory each time: nothing
		// forwards to or from its location.
		if (inst.flags & MF_VOLATILE)
		{
			for (unsigned k = 0; k < inst.nops; ++k)
				if (frame_slot(inst.ops[k], offset))
					forget_home(offset, offset + 8);
			const Reg w = written_reg(inst);
			if (w != kNoReg && w < 16)
				holds[w] = kHoldsNothing;
			continue;
		}
		// The register holding at least `need` bytes of the home at `at`.
		auto holder_of = [&](int32_t at, unsigned need) {
			for (Reg r = 0; r < 16; ++r)
				if (holds[r] != kHoldsNothing && held_home(holds[r]) == at && held_bytes(holds[r]) >= need)
					return r;
			return kNoReg;
		};
		// A narrow reload extends the holder's covered bytes as the load would.
		if (inst.op == Op::Load && !wide && plain_gpr(inst.ops[0]) && frame_slot(inst.ops[1], offset) &&
			register_bytes(inst.type) < 8)
		{
			const unsigned bytes = register_bytes(inst.type);
			const Reg dst = inst.ops[0].reg;
			if (rewrite && forward_narrow(i, end, holds))
				++removed_;
			holds[dst] = dst == R11 || (offset & 7) != 0 ? kHoldsNothing : held(offset, bytes);
			continue;
		}
		// A compare or test reading a home the holder covers reads it.
		if ((inst.op == Op::Cmp || inst.op == Op::Test) && rewrite)
			for (unsigned k = 0; k < 2; ++k)
				if (frame_slot(inst.ops[k], offset) && (offset & 7) == 0 && inst.type != lowir::TypeKind::Void)
				{
					const Reg holder = holder_of(offset, register_bytes(inst.type));
					if (holder != kNoReg && holder != RSP && !inst.ops[1 - k].is_mem())
					{
						inst.ops[k] = Operand::make_reg(holder);
						++removed_;
					}
				}
		if (inst.op == Op::Load && wide && plain_gpr(inst.ops[0]) && frame_slot(inst.ops[1], offset))
		{
			const Reg dst = inst.ops[0].reg;
			const Reg holder = rewrite ? holder_of(offset, 8) : kNoReg;
			holds[dst] = dst == R11 || (offset & 7) != 0 ? kHoldsNothing : offset;
			if (!rewrite || holder == kNoReg)
				continue;
			if (holder == dst)
			{
				inst.op = Op::Nop;
				inst.nops = 0;
				inst.ops[0] = inst.ops[1] = Operand();
				++removed_;
				continue;
			}
			inst.op = Op::Mov;
			inst.type = lowir::TypeKind::Void;
			inst.ops[1] = Operand::make_reg(holder);
			if ((dst == R10 || dst == R11) && holder != RSP && forward_scratch(i, end, holder) != 0)
			{
				holds[dst] = kHoldsNothing;
				++removed_;
			}
			continue;
		}
		if (inst.op == Op::Store && frame_slot(inst.ops[0], offset))
		{
			const unsigned bytes = wide ? 8 : register_bytes(inst.type);
			forget_home(offset, offset + static_cast<int32_t>(bytes));
			if (plain_gpr(inst.ops[1]) && inst.ops[1].reg != R11 && (offset & 7) == 0 &&
				(bytes == 8 || bytes == 4 || bytes == 2 || bytes == 1))
				holds[inst.ops[1].reg] = held(offset, bytes);
			continue;
		}
		// An operation on memory (find_rmw) writes it as a store does.
		const bool rmw = inst.op != Op::Store && inst.op != Op::Cmp && inst.op != Op::Test && inst.op != Op::Bt &&
			inst.nops > 0 && inst.ops[0].kind == OperandKind::Mem;
		if (rmw && frame_slot(inst.ops[0], offset))
		{
			const unsigned bytes = wide ? 8 : register_bytes(inst.type);
			forget_home(offset, offset + static_cast<int32_t>(bytes));
			continue;
		}
		if (inst.op == Op::Store || rmw)
			drop_slots(); // through a pointer, or an indexed frame address
		const Reg w = written_reg(inst);
		if (w != kNoReg && w < 16)
			holds[w] = kHoldsNothing;
	}
	leave();
	for (Reg r = 0; r < 16; ++r)
		holds[r] = exits[r];
	// The scratch registers are dead past the block (a removed copy into
	// one leaves nothing to hold).
	holds[R10] = kHoldsNothing;
	holds[R11] = kHoldsNothing;
}

uint32_t HomeForwarding::run()
{
	if (n_ == 0 || f_.stack_size == 0 || !has_direct_frame_read(f_))
		return 0;
	for (const FrameBinding& binding : f_.bindings)
		if (binding.kind != BindingKind::Slot && !binding.type.is_object() &&
			binding.type.kind() != lowir::TypeKind::F80 && binding.type.kind() != lowir::TypeKind::I128)
			homes_.push_back(binding.offset);
	std::sort(homes_.begin(), homes_.end());
	edges();
	// The available holders to a fixed point (each state only falls:
	// any, then one home, then none), then the rewrite from them.
	std::vector<int32_t> out(static_cast<size_t>(n_) * 16, kHoldsAny);
	std::vector<int32_t> in(static_cast<size_t>(n_) * 16, kHoldsNothing);
	// Visit contiguous register rows instead of rescanning the predecessor list
	// separately for every register. The meet and predecessor order are unchanged.
	auto meet = [&](uint32_t b, int32_t* holds) {
		const uint32_t first = pred_first_[b], past = pred_first_[b + 1];
		if (cut_[b] || first == past)
		{
			std::fill(holds, holds + 16, kHoldsNothing);
			return;
		}
		const int32_t* source = &out[static_cast<size_t>(preds_[first]) * 16];
		std::copy(source, source + 16, holds);
		for (uint32_t k = first + 1; k < past; ++k)
		{
			source = &out[static_cast<size_t>(preds_[k]) * 16];
			for (Reg r = 0; r < 16; ++r)
			{
				const int32_t h = holds[r], o = source[r];
				holds[r] = h == kHoldsAny ? o : (o == kHoldsAny || o == h ? h : kHoldsNothing);
			}
		}
		for (Reg r = 0; r < 16; ++r)
			if (holds[r] == kHoldsAny)
				holds[r] = kHoldsNothing;
	};
	// A block met with the input of its last evaluation leaves the same
	// output (transfer reads nothing else without rewriting): skipped.
	// 0: not visited, 1: clean, 2: a predecessor output changed. Keep the same
	// reverse-postorder rounds; a clean block does not even need another meet.
	std::vector<uint8_t> evaluated(n_, 0);
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b : order_)
		{
			const uint8_t state = evaluated[b];
			if (state == 1)
				continue;
			evaluated[b] = 1;
			int32_t holds[16];
			meet(b, holds);
			int32_t* const block_in = &in[static_cast<size_t>(b) * 16];
			if (state != 0 && std::equal(holds, holds + 16, block_in))
				continue;
			std::copy(holds, holds + 16, block_in);
			transfer(b, holds, false);
			int32_t* const block_out = &out[static_cast<size_t>(b) * 16];
			if (!std::equal(holds, holds + 16, block_out))
			{
				std::copy(holds, holds + 16, block_out);
				changed = true;
				for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
					if (evaluated[succs_[k]] != 0)
						evaluated[succs_[k]] = 2;
			}
		}
	}
	// The fixed point is complete. Rewriting reachable blocks in instruction
	// order leaves their independent input facts unchanged and orders insertions.
	for (uint32_t b = 0; b < n_; ++b)
		if (evaluated[b] != 0)
			transfer(b, &in[static_cast<size_t>(b) * 16], true);
	// An interior extraction can cost more instructions than a load. Keep
	// it when all materialization can disappear, including its reservation.
	if (!field_reads_.empty() && !removable_frame_storage(f_, true))
	{
		for (const auto& read : field_reads_)
			f_.insts[read.first] = read.second;
		removed_ -= static_cast<uint32_t>(field_reads_.size());
		extractions_.clear();
	}
	insert_frame_extractions(f_, extractions_);
	return removed_;
}

} // namespace

// O1+: a reload of a temp's frame home (8 bytes) whose value a register
// holds on every path to it -- since a store of that register to the home
// or a reload of it, neither written since; also of a slot, unwritten
// through a pointer and no call between (available-value dataflow over
// the blocks' branches to a fixed point; nothing flows into the entry, a
// landing, a course handler or a join with a block only a host landing
// reaches) -- goes when that register is its
// destination, else becomes a register copy; a copy into a scratch
// register (r10, r11: dead at block ends) then feeds its readers directly and
// goes. Calls drop the caller-saved holders, anything not simple drops
// all; r11 holds nothing here (the emitter carries stored values in it,
// x86/emitter.cpp). Linear per round; the rounds stop when no state falls.
uint32_t forward_frame_copies(Function& f, const std::vector<FrameObject>& objects)
{
	if (objects.empty() || f.stack_size == 0 || f.variadic || f.eh_records_pushed || !f.eh.landings.empty())
		return 0;
	const FrameObjectIndex index(objects);
	const std::vector<uint8_t> tracked = tracked_frame_objects(f, index);
	// A tracked object's bytes [lo, hi) hold a copy of the bytes at `src`,
	// whose registers are `regs`.
	struct Copied
	{
		uint32_t object;
		int32_t lo, hi;
		Operand src;
		uint32_t regs;
	};
	// Scratch and string registers a bulk copy may use.
	const uint32_t kCopyClobbers = (1u << R10) | (1u << R11) | (1u << RDI) | (1u << RSI) | (1u << RCX);
	// The object a direct frame operand names (lowir::kNoId: none).
	auto direct_object = [&](const Operand& op) {
		if (op.kind != OperandKind::Mem || op.reg != kFrame || op.index != kNoReg || op.symbol != lowir::kNoId)
			return lowir::kNoId;
		return index.find(op.disp);
	};
	auto registers_of = [](const Operand& op) {
		uint32_t m = 0;
		if (op.reg < 16)
			m |= 1u << op.reg;
		if (op.index != kNoReg && op.index < 16)
			m |= 1u << op.index;
		return m;
	};
	std::vector<Copied> copied;
	uint32_t rewritten = 0;
	for (const Block& block : f.blocks)
	{
		copied.clear();
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			if (inst.op == Op::Nop || inst.op == Op::Jcc || inst.op == Op::Jmp)
				continue;
			if (inst.op == Op::Load && inst.nops == 2 && inst.ops[0].kind == OperandKind::Reg)
			{
				const uint32_t o = direct_object(inst.ops[1]);
				const int32_t at = inst.ops[1].disp, past = at + static_cast<int32_t>(register_bytes(inst.type));
				for (size_t k = 0; k < copied.size() && o != lowir::kNoId; ++k)
					if (copied[k].object == o && copied[k].lo <= at && past <= copied[k].hi)
					{
						inst.ops[1] = copied[k].src.displaced(at - copied[k].lo);
						++rewritten;
						break;
					}
			}
			// What the instruction writes: memory a copy's source may be
			// (any but a tracked object's bytes), a tracked object's bytes,
			// registers.
			const bool bulk = (inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes) && !(inst.flags & MF_COPY_DYNAMIC);
			const bool writes_memory = inst.nops > 0 && inst.ops[0].kind == OperandKind::Mem && inst.op != Op::Cmp &&
				inst.op != Op::Test && inst.op != Op::Bt;
			if (!simple_inst(inst) && !bulk)
			{
				copied.clear();
				continue;
			}
			if (writes_memory)
			{
				// (A direct write names its object's bytes only; one whose
				// address exists may also be what a pointer source reads.)
				const uint32_t o = direct_object(inst.ops[0]);
				if (o == lowir::kNoId)
					copied.clear();
				else
				{
					const int32_t lo = inst.ops[0].disp;
					const int32_t hi = lo + static_cast<int32_t>(bulk ? inst.aux : register_bytes(inst.type));
					size_t keep = 0;
					for (size_t k = 0; k < copied.size(); ++k)
					{
						const Copied& c = copied[k];
						const uint32_t src_object = direct_object(c.src);
						const int32_t src_lo = c.src.disp, src_hi = c.src.disp + (c.hi - c.lo);
						const bool hit = (c.object == o && c.lo < hi && lo < c.hi) ||
							(src_object == o && src_lo < hi && lo < src_hi) ||
							(src_object == lowir::kNoId && !tracked[o]);
						if (!hit)
							copied[keep++] = c;
					}
					copied.resize(keep);
				}
			}
			uint32_t clobbered = bulk ? kCopyClobbers : 0;
			const Reg w = bulk ? kNoReg : written_reg(inst);
			if (w != kNoReg && w < 16)
				clobbered |= 1u << w;
			if (clobbered != 0)
			{
				size_t keep = 0;
				for (size_t k = 0; k < copied.size(); ++k)
					if (!(copied[k].regs & clobbered))
						copied[keep++] = copied[k];
				copied.resize(keep);
			}
			// A copy of at most 16 known bytes into a frame object from a
			// location its registers keep naming: a bulk copy, or the
			// selector's scratch copy (`load r11, src; store slot, r11`).
			if (inst.op == Op::CopyBytes && bulk && inst.aux <= 16 && inst.nops == 2 &&
				inst.ops[1].kind == OperandKind::Mem && inst.ops[1].symbol == lowir::kNoId &&
				!(registers_of(inst.ops[1]) & kCopyClobbers))
			{
				const uint32_t o = direct_object(inst.ops[0]);
				if (o != lowir::kNoId)
					copied.push_back(Copied{ o, inst.ops[0].disp, inst.ops[0].disp + static_cast<int32_t>(inst.aux),
						inst.ops[1], registers_of(inst.ops[1]) });
			}
			if (inst.op == Op::Store && inst.nops == 2 && plain_gpr(inst.ops[1]) && inst.ops[1].reg == R11 &&
				i > block.first)
			{
				const Inst& load = f.insts[i - 1];
				const uint32_t o = direct_object(inst.ops[0]);
				if (o != lowir::kNoId && load.op == Op::Load && load.nops == 2 && plain_gpr(load.ops[0]) &&
					load.ops[0].reg == R11 && load.type == inst.type && load.ops[1].kind == OperandKind::Mem &&
					load.ops[1].symbol == lowir::kNoId && !(registers_of(load.ops[1]) & (1u << R11)))
					copied.push_back(
						Copied{ o, inst.ops[0].disp, inst.ops[0].disp + static_cast<int32_t>(register_bytes(inst.type)),
							load.ops[1], registers_of(load.ops[1]) });
			}
		}
	}
	return rewritten;
}

uint32_t forward_home_reloads(Function& function)
{
	HomeForwarding forwarding(function);
	return forwarding.run();
}

// O1+: a floating read of a frame home whose value an XMM register holds
// since a store of it in this block reads the register: the load would wait
// for that store. Integer work keeps what the registers hold; a call or any
// other floating or vector instruction ends it. Linear.
uint32_t forward_float_homes(Function& function)
{
	std::vector<int32_t> homes;
	for (const FrameBinding& binding : function.bindings)
		if (binding.kind != BindingKind::Slot && !binding.type.is_object() &&
			(binding.type.kind() == lowir::TypeKind::F32 || binding.type.kind() == lowir::TypeKind::F64))
			homes.push_back(binding.offset);
	if (homes.empty())
		return 0;
	std::sort(homes.begin(), homes.end());
	const unsigned kXmms = 16;
	int32_t held[kXmms];
	lowir::TypeKind type[kXmms];
	const auto forget_all = [&]() { std::fill(held, held + kXmms, kHoldsNothing); };
	const auto forget_home = [&](int32_t at) {
		for (unsigned r = 0; r < kXmms; ++r)
			if (held[r] != kHoldsNothing && held[r] < at + 8 && at < held[r] + 8)
				held[r] = kHoldsNothing;
	};
	const auto home_of = [&](const Operand& op) {
		return op.kind == OperandKind::Mem && op.reg == kFrame && op.index == kNoReg && op.symbol == lowir::kNoId &&
			std::binary_search(homes.begin(), homes.end(), op.disp);
	};
	const auto holder_of = [&](int32_t at, lowir::TypeKind kind) {
		for (unsigned r = 0; r < kXmms; ++r)
			if (held[r] == at && type[r] == kind)
				return static_cast<Reg>(XMM0 + r);
		return kNoReg;
	};
	uint32_t forwarded = 0;
	for (const Block& block : function.blocks)
	{
		forget_all();
		const uint32_t end = block.first + block.count;
		for (uint32_t i = block.first; i < end; ++i)
		{
			Inst& inst = function.insts[i];
			const bool scalar = inst.type == lowir::TypeKind::F32 || inst.type == lowir::TypeKind::F64;
			const bool volatile_access = (inst.flags & MF_VOLATILE) != 0;
			if (scalar && !volatile_access &&
				(inst.op == Op::FAdd || inst.op == Op::FSub || inst.op == Op::FMul || inst.op == Op::FDiv))
			{
				const Reg d = inst.ops[0].kind == OperandKind::Reg ? inst.ops[0].reg : kNoReg;
				for (unsigned k = 1; k < 3 && d != kNoReg; ++k)
				{
					if (!home_of(inst.ops[k]))
						continue;
					const Reg r = holder_of(inst.ops[k].disp, inst.type);
					// (A second operand must survive the copy of the first.)
					if (r != kNoReg && (k == 1 || r != d))
					{
						inst.ops[k] = Operand::make_reg(r);
						++forwarded;
					}
				}
				if (d != kNoReg && is_xmm(d))
					held[d - XMM0] = kHoldsNothing;
				held[kXmms - 1] = kHoldsNothing;
				continue;
			}
			if (scalar && !volatile_access && inst.op == Op::FMov && inst.ops[0].kind == OperandKind::Reg &&
				is_xmm(inst.ops[0].reg) && home_of(inst.ops[1]))
			{
				const Reg d = inst.ops[0].reg;
				const Reg r = holder_of(inst.ops[1].disp, inst.type);
				const int32_t at = inst.ops[1].disp;
				if (r != kNoReg)
				{
					inst.ops[1] = Operand::make_reg(r);
					++forwarded;
				}
				held[d - XMM0] = d < XMM0 + 14 ? at : kHoldsNothing;
				type[d - XMM0] = inst.type;
				continue;
			}
			if ((inst.op == Op::Store || (scalar && inst.op == Op::FMov)) && inst.ops[0].kind == OperandKind::Mem)
			{
				const bool frame = inst.ops[0].reg == kFrame && inst.ops[0].index == kNoReg;
				if (frame)
					forget_home(inst.ops[0].disp);
				if (scalar && !volatile_access && home_of(inst.ops[0]) && inst.ops[1].kind == OperandKind::Reg &&
					is_xmm(inst.ops[1].reg))
				{
					held[inst.ops[1].reg - XMM0] = inst.ops[0].disp;
					type[inst.ops[1].reg - XMM0] = inst.type;
				}
				continue;
			}
			if (scalar && inst.op == Op::FMov && inst.ops[0].kind == OperandKind::Reg && is_xmm(inst.ops[0].reg))
			{
				held[inst.ops[0].reg - XMM0] = kHoldsNothing;
				continue;
			}
			// (Integer work touches no XMM register.)
			if (simple_inst(inst))
			{
				if (inst.nops > 0 && inst.ops[0].kind == OperandKind::Mem && inst.ops[0].reg == kFrame)
					forget_home(inst.ops[0].disp);
				continue;
			}
			forget_all();
		}
	}
	return forwarded;
}

// O1+: `test r, r` (or `cmp r, 0`, which sets the flags alike) after an
// instruction that set the flags from r at the same width goes when every
// reader of the flags before they are set again (or the block ends: the
// selector keeps no flags across blocks) reads what both set alike: any
// condition after and/or/xor (CF and OF clear in both), only the zero and
// sign conditions after add, sub and a shift by a nonzero immediate. Copies,
// loads, stores and lea may lie between when they write neither r nor the
// flags (no `mov reg, 0`, which may be encoded as an xor). A 64-bit and
// whose result's upper half is zero (an operand's is, from the block's
// instructions) sets the zero flag a 32-bit test of r would. Linear.
uint32_t drop_redundant_tests(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	auto alu_width = [](const Inst& inst) {
		return inst.type == lowir::TypeKind::Void || inst.type == lowir::TypeKind::Ptr ? 8u : register_bytes(inst.type);
	};
	uint32_t removed = 0;
	for (const Block& block : function.blocks)
	{
		const uint32_t end = block.first + block.count;
		// The registers whose upper halves the block's instructions so far
		// left zero (none known at its start).
		uint32_t upper_zero = 0;
		uint32_t known_at = block.first;
		for (uint32_t i = block.first + 1; i < end; ++i)
		{
			Inst& test = insts[i];
			const bool self_test = test.op == Op::Test && plain_gpr(test.ops[1]) && test.ops[1].reg == test.ops[0].reg;
			const bool zero_compare = test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm &&
				test.ops[1].imm == 0;
			if (!(self_test || zero_compare) || !plain_gpr(test.ops[0]))
				continue;
			const Reg r = test.ops[0].reg;
			uint32_t at = i - 1;
			while (at > block.first && simple_inst(insts[at]) &&
				(insts[at].op == Op::Nop || insts[at].op == Op::Store || insts[at].op == Op::Lea ||
					insts[at].op == Op::Load ||
					(insts[at].op == Op::Mov &&
						!(insts[at].ops[1].kind == OperandKind::Imm && insts[at].ops[1].imm == 0))) &&
				written_reg(insts[at]) != r)
				--at;
			const Inst& prior = insts[at];
			if (prior.nops < 2 || !plain_gpr(prior.ops[0]) || prior.ops[0].reg != r)
				continue;
			unsigned width = 0;
			bool full = false;
			bool zero_only = false;
			switch (prior.op)
			{
			case Op::And:
			case Op::Or:
			case Op::Xor:
				width = alu_width(prior);
				full = true;
				// An i64 AND with a mask clearing the upper half is emitted
				// at 32 bits (x86/emitter.cpp): its sign is bit 31.
				if (prior.op == Op::And && width == 8 && prior.ops[1].kind == OperandKind::Imm &&
					prior.ops[1].imm >= 0 && prior.ops[1].imm <= 0xffffffffll)
					width = 4;
				break;
			case Op::Add:
			case Op::Sub:
				width = alu_width(prior);
				break;
			case Op::Shl:
			case Op::Shr:
			case Op::Sar:
				width = prior.type == lowir::TypeKind::U32 ? 4u : 8u;
				if (prior.ops[1].kind != OperandKind::Imm || (prior.ops[1].imm & (width * 8 - 1)) == 0)
					width = 0;
				break;
			default:
				break;
			}
			// A nonnegative mask fitting both signed widths leaves every
			// tested bit unchanged and both sign flags clear. Logical AND
			// and TEST also agree on parity, carry and overflow.
			const unsigned tested = register_bytes(test.type);
			const unsigned smaller = std::min(width, tested);
			if (prior.op == Op::And && width != 0 && width != tested && prior.ops[1].kind == OperandKind::Imm &&
				prior.ops[1].imm >= 0 && static_cast<uint64_t>(prior.ops[1].imm) < (uint64_t(1) << (smaller * 8 - 1)))
				width = tested;
			if (width == 8 && register_bytes(test.type) == 4 && prior.op == Op::And)
			{
				for (; known_at < at; ++known_at)
				{
					const Inst& inst = insts[known_at];
					if (!simple_inst(inst))
					{
						upper_zero = 0;
						continue;
					}
					const Reg w = written_reg(inst);
					if (w < 16)
						upper_zero = zeroes_upper(inst, upper_zero) ? upper_zero | (1u << w) : upper_zero & ~(1u << w);
				}
				const Operand& other = prior.ops[1];
				if (((upper_zero >> r) & 1u) || (plain_gpr(other) && ((upper_zero >> other.reg) & 1u)))
				{
					width = 4;
					zero_only = true;
				}
			}
			if (width == 0 || width != register_bytes(test.type))
				continue;
			bool ok = true;
			for (uint32_t k = i + 1; k < end && ok; ++k)
			{
				const Inst& reader = insts[k];
				if (reader.op == Op::Jcc || reader.op == Op::Setcc || reader.op == Op::Cmov)
				{
					const Cond cond = static_cast<Cond>(reader.sub);
					ok = zero_only ? cond == CC_E || cond == CC_NE
								   : full || cond == CC_E || cond == CC_NE || cond == CC_S || cond == CC_NS;
					continue;
				}
				if (reader.op == Op::Mov || reader.op == Op::Load || reader.op == Op::Store || reader.op == Op::Lea ||
					reader.op == Op::Movzx || reader.op == Op::Sext || reader.op == Op::Zext || reader.op == Op::Nop)
					continue;
				if (flags_dead_after(insts, k - 1, k + 1))
					break;
				ok = false;
			}
			if (!ok)
				continue;
			test.op = Op::Nop;
			test.nops = 0;
			test.ops[0] = test.ops[1] = Operand();
			++removed;
		}
	}
	return removed;
}

// O1+: a 64-bit register copy between registers already holding the same
// value -- an earlier 64-bit copy between them in the block, neither named
// since -- goes (an argument set up from the callee-saved register its
// value was just copied into from that argument register). Linear in each
// block's instructions times the pairs it tracks.
// A call through a register a load just filled reads the target from memory
// itself. The register is free after the call (caller-saved, no argument);
// only moves into other registers may separate the two.
uint32_t fold_call_targets(Function& function)
{
	const unsigned kReach = 6;
	std::vector<Inst>& insts = function.insts;
	uint32_t removed = 0;
	for (const Block& block : function.blocks)
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& call = insts[i];
			if (call.op != Op::Call || !plain_gpr(call.ops[0]))
				continue;
			const Reg target = call.ops[0].reg;
			if (!(kCallerSavedGprs & (1u << target)) || (call.aux & (1u << target)))
				continue;
			uint32_t written = 0;
			for (uint32_t j = i, reach = 0; j > block.first && reach < kReach; ++reach)
			{
				Inst& prior = insts[--j];
				if (prior.op == Op::Nop)
					continue;
				if (prior.op == Op::Load && wide_kind(prior.type) && plain_gpr(prior.ops[0]) &&
					prior.ops[0].reg == target)
				{
					const Operand from = prior.ops[1];
					bool clear = from.kind == OperandKind::Mem && from.reg < 16 && from.reg != RSP;
					for (Reg r = 0; r < 16; ++r)
						clear = clear && !((written & (1u << r)) && from.uses_reg(r));
					if (clear)
					{
						call.ops[0] = from;
						prior.op = Op::Nop;
						prior.nops = 0;
						prior.ops[0] = prior.ops[1] = Operand();
						++removed;
					}
					break;
				}
				const bool move = (prior.op == Op::Mov || prior.op == Op::Lea) && prior.nops == 2 &&
					plain_gpr(prior.ops[0]) && (prior.op == Op::Lea || prior.ops[1].kind != OperandKind::Mem);
				if (!move || reads_or_writes(prior, target))
					break;
				written |= 1u << prior.ops[0].reg;
			}
		}
	return removed;
}

uint32_t drop_redundant_copies(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	uint32_t removed = 0;
	std::vector<std::pair<Reg, Reg>> same;
	for (const Block& block : function.blocks)
	{
		same.clear();
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = insts[i];
			if (inst.op == Op::Nop)
				continue;
			if (inst.op == Op::Mov && inst.nops == 2 && plain_gpr(inst.ops[0]) && plain_gpr(inst.ops[1]) &&
				wide_kind(inst.type))
			{
				const Reg d = inst.ops[0].reg, s = inst.ops[1].reg;
				bool known = d == s;
				for (const std::pair<Reg, Reg>& p : same)
					known = known || (p.first == d && p.second == s) || (p.first == s && p.second == d);
				if (known)
				{
					inst.op = Op::Nop;
					inst.nops = 0;
					inst.ops[0] = inst.ops[1] = Operand();
					++removed;
					continue;
				}
				size_t kept = 0;
				for (const std::pair<Reg, Reg>& p : same)
					if (p.first != d && p.second != d)
						same[kept++] = p;
				same.resize(kept);
				same.push_back(std::make_pair(d, s));
				continue;
			}
			size_t kept = 0;
			for (const std::pair<Reg, Reg>& p : same)
				if (!reads_or_writes(inst, p.first) && !reads_or_writes(inst, p.second))
					same[kept++] = p;
			same.resize(kept);
		}
	}
	return removed;
}

uint32_t drop_redundant_constants(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	uint32_t removed = 0;
	struct Held
	{
		bool known = false;
		lowir::TypeKind type = lowir::TypeKind::Void;
		int64_t imm = 0;
	};
	Held held[16];
	for (const Block& block : function.blocks)
	{
		for (Held& h : held)
			h.known = false;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = insts[i];
			if (inst.op == Op::Nop || inst.op == Op::Jcc)
				continue;
			if (inst.op == Op::Mov && inst.nops == 2 && plain_gpr(inst.ops[0]) && inst.ops[1].kind == OperandKind::Imm)
			{
				Held& h = held[inst.ops[0].reg];
				if (h.known && h.type == inst.type && h.imm == inst.ops[1].imm)
				{
					inst.op = Op::Nop;
					inst.nops = 0;
					inst.ops[0] = inst.ops[1] = Operand();
					++removed;
					continue;
				}
				h.known = true;
				h.type = inst.type;
				h.imm = inst.ops[1].imm;
				continue;
			}
			// Anything the emitter may expand with implicit registers (calls,
			// bulk copies, wide immediates through the scratch) forgets all.
			if (!simple_inst(inst))
			{
				for (Held& h : held)
					h.known = false;
				continue;
			}
			const Reg w = written_reg(inst);
			if (w != kNoReg && w < 16)
				held[w].known = false;
		}
	}
	return removed;
}

namespace {

// Fold an address sum into its immediately following load only when the
// original sum is killed locally before any later reader. Bound the search;
// partial writes and conditional writes do not kill a register's value.
bool fold_load_address(std::vector<Inst>& insts, uint32_t i, uint32_t end)
{
	Inst& add = insts[i];
	if (add.op != Op::Add || !wide_kind(add.type) || !plain_gpr(add.ops[0]) || !plain_gpr(add.ops[1]))
		return false;
	const Reg address = add.ops[0].reg, base = add.ops[1].reg;
	if (address == base || address == RSP || address == RBP)
		return false;
	uint32_t j = i + 1;
	while (j < end && insts[j].op == Op::Nop)
		++j;
	if (j == end)
		return false;
	Inst& load = insts[j];
	const Operand& mem = load.ops[1];
	if (load.op != Op::Load || !plain_gpr(load.ops[0]) || !simple_inst(load) || (load.flags & MF_VOLATILE) ||
		mem.kind != OperandKind::Mem || mem.reg != address || mem.index != kNoReg || mem.symbol != lowir::kNoId)
		return false;
	bool killed = load.ops[0].reg == address;
	for (uint32_t k = j + 1; !killed && k < end && k - j <= 16; ++k)
	{
		const Inst& next = insts[k];
		if (!simple_inst(next) || reads_reg(next, address))
			return false;
		killed = written_reg(next) == address;
	}
	if (!killed || !flags_dead_after(insts, i, end))
		return false;
	load.ops[1] = Operand::make_mem(base, mem.disp, address, 1);
	add.op = Op::Nop;
	add.nops = 0;
	add.ops[0] = add.ops[1] = Operand();
	return true;
}

// A following 32-bit operation uses only the low half before writing
// its result. No memory operand may consume the original full-width address.
bool consumes_low32(const Inst& inst, Reg r)
{
	if (!plain_gpr(inst.ops[0]) || inst.ops[0].reg != r || !narrow_kind(inst.type))
		return false;
	for (unsigned k = 1; k < inst.nops; ++k)
		if (inst.ops[k].kind != OperandKind::Imm && !plain_gpr(inst.ops[k]))
			return false;
	switch (inst.op)
	{
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
		return true;
	case Op::Shl:
	case Op::Shr:
	case Op::Sar:
	case Op::Rol:
	case Op::Ror:
		return inst.type == lowir::TypeKind::U32 && inst.ops[1].kind == OperandKind::Imm && (inst.ops[1].imm & 31) != 0;
	default:
		return false;
	}
}

// insts[k], the last writer of r before a 32-bit zero-extension of it
// that nothing between reads, at 32 bits when the low half of its result
// depends only on the low halves of its operands (and nothing reads its
// flags): it then zeroes the upper half itself. False when it cannot.
bool narrow_to_u32(std::vector<Inst>& insts, uint32_t k, Reg r, uint32_t end)
{
	Inst& w = insts[k];
	if (!simple_inst(w) || written_reg(w) != r || (w.flags & MF_VOLATILE))
		return false;
	const Operand& src = w.ops[1];
	switch (w.op)
	{
	case Op::Mov:
		if (src.kind == OperandKind::Reg && src.reg < 16)
		{
			w.op = Op::Zext;
			w.type = lowir::TypeKind::U32;
			return true;
		}
		if (src.kind != OperandKind::Imm)
			return false;
		w.ops[1].imm = static_cast<int64_t>(static_cast<uint32_t>(src.imm));
		return true;
	case Op::Load:
		if (register_bytes(w.type) < 4 || w.type == lowir::TypeKind::F32 || w.type == lowir::TypeKind::F64)
			return false;
		w.type = lowir::TypeKind::U32;
		return true;
	case Op::Lea:
		if (!wide_kind(w.type))
			return false;
		w.type = lowir::TypeKind::I32;
		return true;
	case Op::Shl:
		if (src.kind != OperandKind::Imm || src.imm < 0 || src.imm >= 32)
			return false;
		[[fallthrough]];
	case Op::Add:
	case Op::Sub:
	case Op::And:
	case Op::Or:
	case Op::Xor:
	case Op::Imul:
	case Op::Neg:
	case Op::Not:
		if (!wide_kind(w.type) || !flags_dead_after(insts, k, end))
			return false;
		w.type = lowir::TypeKind::U32;
		return true;
	default:
		return false;
	}
}

// A scale followed by an add can share an indexed address calculation.
// The intermediate value and flags must be unobserved. Bound both searches
// so this local selection costs constant work per instruction.
bool fuse_scaled_add(std::vector<Inst>& insts, uint32_t i, uint32_t end)
{
	Inst& inst = insts[i];
	if ((inst.op != Op::Shl && inst.op != Op::Imul) || inst.nops != 2 || !plain_gpr(inst.ops[0]) ||
		inst.ops[0].reg == RSP || inst.ops[1].kind != OperandKind::Imm || inst.ops[1].imm <= 0 ||
		(inst.type != lowir::TypeKind::U32 && !wide_kind(inst.type)))
		return false;
	int64_t odd = inst.op == Op::Shl ? 1 : inst.ops[1].imm;
	unsigned shift = inst.op == Op::Shl ? static_cast<unsigned>(inst.ops[1].imm) : 0;
	while ((odd & 1) == 0 && shift < 4)
	{
		odd >>= 1;
		++shift;
	}
	if (shift < 1 || shift > 3 || (odd != 1 && odd != 3 && odd != 5 && odd != 9))
		return false;
	const Reg b = inst.ops[0].reg;
	const uint32_t limit = std::min(end, i + 16);
	uint32_t k = i + 1;
	while (k < limit && !reads_or_writes(insts[k], b))
		++k;
	const bool narrow = inst.type == lowir::TypeKind::U32;
	if (k == limit || insts[k].op != Op::Add || insts[k].nops != 2 || !plain_gpr(insts[k].ops[0]) ||
		!plain_gpr(insts[k].ops[1]) || insts[k].ops[0].reg == insts[k].ops[1].reg ||
		!(narrow ? narrow_kind(insts[k].type) : wide_kind(insts[k].type)) || !flags_dead_after(insts, k, limit))
		return false;
	Inst& add = insts[k];
	const bool reversed = add.ops[0].reg != b;
	if (reversed)
	{
		if (add.ops[1].reg != b)
			return false;
		// The add may put its result in the other operand's register.
		// Removing the scale then changes b until its next full overwrite;
		// prove no instruction can observe that intermediate value.
		bool killed = false;
		for (uint32_t j = k + 1; j < std::min(end, k + 16); ++j)
		{
			const Inst& next = insts[j];
			if (!simple_inst(next) || reads_reg(next, b))
				break;
			if (written_reg(next) == b)
			{
				killed = true;
				break;
			}
		}
		if (!killed)
			return false;
	}
	add.op = Op::Lea;
	add.type = narrow ? lowir::TypeKind::I32 : lowir::TypeKind::Void;
	add.ops[1] = Operand::make_mem(reversed ? add.ops[0].reg : add.ops[1].reg, 0, b, static_cast<uint8_t>(1u << shift));
	inst.op = odd == 1 ? Op::Nop : Op::Lea;
	inst.nops = odd == 1 ? 0 : 2;
	inst.type = add.type;
	inst.ops[0] = odd == 1 ? Operand() : Operand::make_reg(b);
	inst.ops[1] = odd == 1 ? Operand() : Operand::make_mem(b, 0, b, static_cast<uint8_t>(odd - 1));
	return true;
}

} // namespace

// O1+: `add b, imm` next to `add b, c` (c another register, the same
// width, nothing reading the flags after them) becomes `lea b, [b + c +
// imm]`; a 32-bit in-place zero-extension of a register whose upper half
// an earlier instruction of the block zeroed (zeroes_upper, through
// copies; tracking lapses at anything else writing registers) becomes a
// nop, as does one of a value a 64-bit operation just computed that can
// run at 32 bits instead (narrow_to_u32), or whose next low-bit calculation
// itself zeroes the upper half. r10 and r11 (emitter scratch)
// are never tracked. Linear.
uint32_t fuse_adds_and_extensions(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	uint32_t removed = 0;
	for (const Block& block : function.blocks)
	{
		const uint32_t end = block.first + block.count;
		uint32_t known = 0; // registers whose upper half is zero
		for (uint32_t i = block.first; i < end; ++i)
		{
			Inst& inst = insts[i];
			if (fold_load_address(insts, i, end))
			{
				++removed;
				continue;
			}
			const Reg scaled = plain_gpr(inst.ops[0]) ? inst.ops[0].reg : kNoReg;
			if (fuse_scaled_add(insts, i, end))
			{
				known &= ~(1u << scaled);
				++removed;
				continue;
			}
			// The add pair: the next instruction touching b (past ones that
			// neither touch it nor read the flags).
			uint32_t j = i + 1;
			if (inst.op == Op::Add && plain_gpr(inst.ops[0]))
				while (j < end && !reads_or_writes(insts[j], inst.ops[0].reg) && insts[j].op != Op::Jcc &&
					insts[j].op != Op::Setcc && insts[j].op != Op::Jmp && insts[j].op != Op::Call)
					++j;
			if (inst.op == Op::Add && j < end && plain_gpr(inst.ops[0]) && insts[j].op == Op::Add &&
				plain_gpr(insts[j].ops[0]) && insts[j].ops[0].reg == inst.ops[0].reg &&
				((narrow_kind(inst.type) && narrow_kind(insts[j].type)) ||
					(wide_kind(inst.type) && wide_kind(insts[j].type))) &&
				flags_dead_after(insts, j, end))
			{
				Inst& other = insts[j];
				const Reg b = inst.ops[0].reg;
				const Operand* imm = inst.ops[1].kind == OperandKind::Imm ? &inst.ops[1] : &other.ops[1];
				const Operand* reg = inst.ops[1].kind == OperandKind::Imm ? &other.ops[1] : &inst.ops[1];
				// A 32-bit add's immediate is its low half (the lea wraps alike).
				const bool narrow = narrow_kind(inst.type);
				if (imm->kind == OperandKind::Imm && (narrow || (imm->imm >= INT32_MIN && imm->imm <= INT32_MAX)) &&
					plain_gpr(*reg) && reg->reg != b && reg->reg != RSP &&
					(j == i + 1 || inst.ops[1].kind == OperandKind::Imm))
				{
					const int32_t disp = static_cast<int32_t>(static_cast<uint32_t>(imm->imm));
					other.op = Op::Lea;
					other.type = narrow ? lowir::TypeKind::I32 : lowir::TypeKind::Void;
					other.ops[1] = Operand::make_mem(b, narrow ? disp : static_cast<int32_t>(imm->imm), reg->reg, 1);
					inst.op = Op::Nop;
					inst.nops = 0;
					inst.ops[0] = inst.ops[1] = Operand();
					++removed;
					continue;
				}
			}
			// A lea of disp(a) into b, then an add of r to b (b untouched and a
			// unwritten between): one lea of disp(a, r) at the add.
			if (inst.op == Op::Lea && plain_gpr(inst.ops[0]) && inst.ops[1].kind == OperandKind::Mem &&
				inst.ops[1].index == kNoReg && inst.ops[1].symbol == lowir::kNoId && inst.ops[1].reg < 16 &&
				inst.ops[1].reg != inst.ops[0].reg)
			{
				const Reg b = inst.ops[0].reg, a = inst.ops[1].reg;
				uint32_t k = i + 1;
				while (k < end && !reads_or_writes(insts[k], b) && written_reg(insts[k]) != a &&
					insts[k].op != Op::Jcc && insts[k].op != Op::Setcc && insts[k].op != Op::Jmp &&
					insts[k].op != Op::Call && simple_inst(insts[k]))
					++k;
				const bool narrow = inst.type == lowir::TypeKind::I32;
				if (k < end && insts[k].op == Op::Add && plain_gpr(insts[k].ops[0]) && insts[k].ops[0].reg == b &&
					plain_gpr(insts[k].ops[1]) && insts[k].ops[1].reg != b && insts[k].ops[1].reg != RSP &&
					(narrow ? narrow_kind(insts[k].type)
							: (inst.type == lowir::TypeKind::Void && wide_kind(insts[k].type))) &&
					flags_dead_after(insts, k, end))
				{
					Inst& other = insts[k];
					const Reg r = other.ops[1].reg;
					other.op = Op::Lea;
					other.type = inst.type;
					other.ops[1] = Operand::make_mem(a, inst.ops[1].disp, r, 1);
					inst.op = Op::Nop;
					inst.nops = 0;
					inst.ops[0] = inst.ops[1] = Operand();
					++removed;
					continue;
				}
			}
			// The redundant extension.
			const bool in_place_zext = inst.op == Op::Zext && narrow_kind(inst.type) && plain_gpr(inst.ops[0]) &&
				(inst.nops == 1 || (inst.ops[1].kind == OperandKind::Reg && inst.ops[1].reg == inst.ops[0].reg));
			const bool following_narrow = in_place_zext && j < end && consumes_low32(insts[j], inst.ops[0].reg);
			uint32_t w = i;
			if (in_place_zext && !((known >> inst.ops[0].reg) & 1u))
				while (w > block.first && i - w < 16 && !reads_or_writes(insts[w - 1], inst.ops[0].reg))
					--w;
			if (in_place_zext &&
				(((known >> inst.ops[0].reg) & 1u) ||
					(w > block.first && narrow_to_u32(insts, w - 1, inst.ops[0].reg, end)) || following_narrow))
			{
				if (!following_narrow && inst.ops[0].reg != R10 && inst.ops[0].reg != R11)
					known |= 1u << inst.ops[0].reg;
				inst.op = Op::Nop;
				inst.nops = 0;
				inst.ops[0] = inst.ops[1] = Operand();
				++removed;
				continue;
			}
			switch (inst.op)
			{
			case Op::Store:
			case Op::Cmp:
			case Op::Test:
			case Op::Bt:
			case Op::Nop:
			case Op::Jcc:
			case Op::Jmp:
				break;
			case Op::Mov:
			case Op::Load:
			case Op::Lea:
			case Op::Add:
			case Op::Sub:
			case Op::And:
			case Op::Or:
			case Op::Xor:
			case Op::Imul:
			case Op::Neg:
			case Op::Not:
			case Op::Shl:
			case Op::Shr:
			case Op::Sar:
			case Op::Rol:
			case Op::Ror:
			case Op::Cmov:
			case Op::Movzx:
			case Op::Sext:
			case Op::Zext:
			case Op::Setcc:
				if (plain_gpr(inst.ops[0]))
				{
					const Reg w = inst.ops[0].reg;
					// A setcc writes the low byte only.
					const bool zero = inst.op == Op::Setcc ? ((known >> w) & 1u) != 0 : zeroes_upper(inst, known);
					known = zero && w != R10 && w != R11 ? known | (1u << w) : known & ~(1u << w);
				}
				else if (inst.ops[0].kind == OperandKind::Reg)
					known = 0;
				break;
			default:
				known = 0;
				break;
			}
		}
	}
	return removed;
}

namespace {

// A frame operand at a fixed offset (no index, no symbol).
bool fixed_frame(const Operand& op)
{
	return op.kind == OperandKind::Mem && op.reg == kFrame && op.index == kNoReg && op.symbol == lowir::kNoId;
}

// The frame, or the stack below it (rsp-based: outgoing arguments).
bool frame_based(const Operand& op)
{
	return op.kind == OperandKind::Mem && (op.reg == kFrame || op.index == kFrame || op.reg == RSP);
}

// Where a byte a return load reads comes from (forward_return_slots).
struct ByteSource
{
	enum Kind : uint8_t
	{
		Open,
		Bad,
		Imm,
		FromReg,
		FromCopy
	} kind;
	uint8_t value;   // Imm
	uint32_t writer; // FromReg, FromCopy: the store or copy
	int32_t offset;  // the byte's offset in what the writer writes
};

const uint32_t kCopyScratch = (1u << R10) | (1u << R11);

// The frame bytes some instruction may write, by ranges [first, second):
// the frame-pointer-free leaf's never-written bytes are its return
// object's padding. All when a frame address escapes (a lea) or an
// indexed frame operand writes.
class FrameWrites
{
public:
	explicit FrameWrites(const Function& f)
	{
		for (const Inst& inst : f.insts)
		{
			if (inst.op == Op::Nop)
				continue; // (a merged store's keeps its operands)
			if (inst.op == Op::Load || inst.op == Op::Cmp || inst.op == Op::Test || inst.op == Op::Bt)
			{
				for (unsigned k = 0; k < inst.nops; ++k)
					if (frame_based(inst.ops[k]) && !fixed_frame(inst.ops[k]))
						all_ = true;
				continue;
			}
			if (inst.op == Op::Lea && frame_based(inst.ops[1]))
				all_ = true;
			for (unsigned k = 0; k < inst.nops; ++k)
			{
				const Operand& op = inst.ops[k];
				if (!frame_based(op) || (inst.op == Op::CopyBytes && k == 1) || (inst.op == Op::Lea && k == 1))
					continue;
				if (!fixed_frame(op))
				{
					all_ = true;
					continue;
				}
				int32_t width = 16;
				if (inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes)
					width = static_cast<int32_t>(inst.aux);
				else if (simple_inst(inst))
					width = static_cast<int32_t>(register_bytes(inst.type));
				ranges_.push_back(std::make_pair(op.disp, op.disp + width));
			}
		}
	}
	bool written(int32_t byte) const
	{
		if (all_)
			return true;
		for (const std::pair<int32_t, int32_t>& r : ranges_)
			if (r.first <= byte && byte < r.second)
				return true;
		return false;
	}

private:
	bool all_ = false;
	std::vector<std::pair<int32_t, int32_t>> ranges_;
};

// The value a return load reads from the frame, from the writes before it
// in its block (insts[first, end)): its replacement, or false.
// Eight bytes from two four-byte halves (each a register's low half or a
// constant): the instructions that form them in the load's register, with
// R11 or R10 for the high half.
bool compose_halves(const std::vector<Inst>& insts, const ByteSource* src, const Inst& load, std::vector<Inst>& out)
{
	Operand half[2];
	for (uint32_t h = 0; h < 2; ++h)
	{
		const ByteSource& s = src[4 * h];
		uint64_t value = 0;
		for (uint32_t p = 0; p < 4; ++p)
		{
			const ByteSource& b = src[4 * h + p];
			if (b.kind != s.kind ||
				(s.kind == ByteSource::Imm
						? false
						: s.kind != ByteSource::FromReg || b.writer != s.writer || b.offset != static_cast<int32_t>(p)))
				return false;
			if (s.kind == ByteSource::Imm)
				value |= static_cast<uint64_t>(b.value) << (8 * p);
		}
		if (s.kind == ByteSource::Imm)
			half[h] = Operand::make_imm(static_cast<int64_t>(value), 0);
		else if (register_bytes(insts[s.writer].type) == 4)
			half[h] = insts[s.writer].ops[1];
		else
			return false;
	}
	const Reg to = load.ops[0].reg;
	Reg high = R11;
	if (half[0].uses_reg(R11) || to == R11)
		high = R10;
	if (half[0].uses_reg(high) || to == high)
		return false;
	auto inst = [&](Op op, lowir::TypeKind type, const Operand& a, const Operand& b) {
		Inst i = Inst();
		i.op = op;
		i.type = type;
		i.nops = 2;
		i.ops[0] = a;
		i.ops[1] = b;
		out.push_back(i);
	};
	const bool high_zero = half[1].kind == OperandKind::Imm && half[1].imm == 0;
	if (!high_zero)
	{
		if (half[1].kind == OperandKind::Imm)
		{
			const int64_t value = static_cast<int64_t>(static_cast<uint64_t>(half[1].imm) << 32);
			inst(Op::Mov, lowir::TypeKind::Void, Operand::make_reg(high), Operand::make_imm(value, value < 0 ? -1 : 0));
		}
		else
		{
			inst(Op::Mov, lowir::TypeKind::Void, Operand::make_reg(high), half[1]);
			inst(Op::Shl, lowir::TypeKind::I64, Operand::make_reg(high), Operand::make_imm(32, 0));
		}
	}
	if (half[0].kind == OperandKind::Imm)
		inst(Op::Mov, lowir::TypeKind::Void, load.ops[0], half[0]);
	else
		inst(Op::Zext, lowir::TypeKind::U32, load.ops[0], half[0]);
	if (!high_zero)
		inst(Op::Or, lowir::TypeKind::I64, load.ops[0], Operand::make_reg(high));
	return true;
}

bool resolve_return_load(const Function& f, uint32_t first, uint32_t end, const Inst& load, const FrameWrites*& writes,
	Inst& out, std::vector<Inst>* composed)
{
	const std::vector<Inst>& insts = f.insts;
	const int32_t a = load.ops[1].disp;
	const uint32_t w = register_bytes(load.type);
	ByteSource src[8];
	for (uint32_t p = 0; p < w; ++p)
		src[p].kind = ByteSource::Open;
	uint32_t open = w;
	uint32_t clobbered = 0; // GPRs written after the writer at hand
	bool memory_written = false;
	auto overlaps = [&](int32_t lo, int32_t hi) { return lo < a + static_cast<int32_t>(w) && a < hi; };
	for (uint32_t i = end; i-- > first && open != 0;)
	{
		const Inst& inst = insts[i];
		if (inst.op == Op::Nop)
			continue;
		if (inst.flags & MF_VOLATILE)
			break;
		const bool store = inst.op == Op::Store && fixed_frame(inst.ops[0]);
		const bool zero = inst.op == Op::ZeroBytes && fixed_frame(inst.ops[0]);
		const bool copy = inst.op == Op::CopyBytes && !(inst.flags & MF_COPY_DYNAMIC) && inst.aux <= 16 &&
			fixed_frame(inst.ops[0]);
		if (store || zero || copy)
		{
			const int32_t x = inst.ops[0].disp;
			const int32_t bytes = store ? static_cast<int32_t>(register_bytes(inst.type))
										: static_cast<int32_t>(inst.aux);
			// A copy's source must hold still: its registers unwritten, no
			// memory written since, not the frame.
			const Operand& from = inst.ops[1];
			const bool copy_ok = copy && !frame_based(from) && !memory_written &&
				!(from.kind == OperandKind::Reg && (from.reg >= 16 || (clobbered & (1u << from.reg)))) &&
				!(from.kind == OperandKind::Mem &&
					((from.reg != kNoReg && (from.reg >= 16 || (clobbered & (1u << from.reg)))) ||
						(from.index != kNoReg && (from.index >= 16 || (clobbered & (1u << from.index))))));
			for (uint32_t p = 0; p < w; ++p)
			{
				const int32_t byte = a + static_cast<int32_t>(p);
				if (src[p].kind != ByteSource::Open || byte < x || byte >= x + bytes)
					continue;
				--open;
				ByteSource& s = src[p];
				s.offset = byte - x;
				s.writer = i;
				if (zero)
				{
					s.kind = ByteSource::Imm;
					s.value = 0;
				}
				else if (copy)
					s.kind = copy_ok ? ByteSource::FromCopy : ByteSource::Bad;
				else if (inst.ops[1].kind == OperandKind::Imm)
				{
					s.kind = ByteSource::Imm;
					s.value = static_cast<uint8_t>(static_cast<uint64_t>(inst.ops[1].imm) >> (8 * s.offset));
				}
				else if (inst.ops[1].kind == OperandKind::Reg && inst.ops[1].reg < 16 &&
					!(clobbered & (1u << inst.ops[1].reg)))
					s.kind = ByteSource::FromReg;
				else
					s.kind = ByteSource::Bad;
			}
			if (copy)
				clobbered |= kCopyScratch;
			continue;
		}
		if (!simple_inst(inst))
			break;
		if (inst.ops[0].kind == OperandKind::Mem && inst.op != Op::Cmp && inst.op != Op::Test && inst.op != Op::Bt)
		{
			const Operand& to = inst.ops[0];
			if (fixed_frame(to))
			{
				if (overlaps(to.disp, to.disp + static_cast<int32_t>(register_bytes(inst.type))))
					break;
			}
			else if (frame_based(to))
				break;
			else
				memory_written = true;
		}
		const Reg wr = written_reg(inst);
		if (wr != kNoReg && wr < 16)
			clobbered |= 1u << wr;
	}
	// Bytes nothing writes (padding) read as zero.
	for (uint32_t p = 0; p < w; ++p)
	{
		if (src[p].kind != ByteSource::Open)
			continue;
		if (writes == nullptr)
			writes = new FrameWrites(f);
		if (writes->written(a + static_cast<int32_t>(p)))
			return false;
		src[p].kind = ByteSource::Imm;
		src[p].value = 0;
	}
	const ByteSource::Kind kind = src[0].kind;
	for (uint32_t p = 0; p < w; ++p)
		if (src[p].kind != kind || src[p].kind == ByteSource::Bad ||
			(kind != ByteSource::Imm &&
				(src[p].writer != src[0].writer || src[p].offset != src[0].offset + static_cast<int32_t>(p))))
			return w == 8 && composed != nullptr && compose_halves(insts, src, load, *composed);
	out = Inst();
	out.nops = 2;
	out.ops[0] = load.ops[0];
	if (kind == ByteSource::Imm)
	{
		uint64_t v = 0;
		for (uint32_t p = 0; p < w; ++p)
			v |= static_cast<uint64_t>(src[p].value) << (8 * p);
		out.op = Op::Mov;
		out.type = lowir::TypeKind::Void;
		out.ops[1] = Operand::make_imm(static_cast<int64_t>(v), static_cast<int64_t>(v) < 0 ? -1 : 0);
		return true;
	}
	const Inst& writer = insts[src[0].writer];
	if (src[0].offset != 0 && kind == ByteSource::FromReg)
		return false;
	if (kind == ByteSource::FromReg)
	{
		out.ops[1] = writer.ops[1];
		if (w == 8)
		{
			out.op = Op::Mov;
			out.type = lowir::TypeKind::Void;
		}
		else
		{
			out.op = Op::Zext;
			out.type = lowir::TypeKind::U32;
		}
		return true;
	}
	const Operand& from = writer.ops[1];
	out.op = Op::Load;
	out.type = load.type;
	out.ops[1] = (from.kind == OperandKind::Reg ? Operand::make_mem(from.reg, 0) : from).displaced(src[0].offset);
	return true;
}

} // namespace

// O1+: a return's loads of rax and rdx from the frame take the values
// their block stored there (a register's, a constant -- bytes nothing in
// the function writes, padding, as zero -- or a copy's source), and the
// block's frame writes nothing reads before the return go. Linear per
// block (the never-written test scans the function once, on demand).
namespace {

uint32_t forward_return_slots_once(Function& function)
{
	std::vector<Inst>& insts = function.insts;
	const FrameWrites* writes = nullptr;
	uint32_t removed = 0;
	// Whether other memory may be the frame (an address of it escapes).
	bool escapes = function.variadic || function.eh_records_pushed;
	for (const Inst& inst : insts)
		escapes = escapes || (inst.op == Op::Lea && frame_based(inst.ops[1])) || inst.op == Op::EhPush;
	for (Block& block : function.blocks)
	{
		if (block.count == 0 || insts[block.first + block.count - 1].op != Op::Ret ||
			(insts[block.first + block.count - 1].flags & MF_FRAMELESS))
			continue;
		uint32_t ret = block.first + block.count - 1;
		// The moves into rax and rdx just before the return (one each at
		// most): frame loads, or what an earlier round made of them. They
		// form one parallel assignment, so an earlier move whose register
		// the later one reads (a value computed in rdx, then copied to rax)
		// is not one of them.
		uint32_t at[2], count = 0, start = ret;
		for (uint32_t i = ret; i-- > block.first;)
		{
			const Inst& inst = insts[i];
			if (inst.op == Op::Nop)
				continue;
			if (count == 2 || (inst.op != Op::Load && inst.op != Op::Mov && inst.op != Op::Zext) || inst.nops != 2 ||
				inst.ops[0].kind != OperandKind::Reg || (inst.ops[0].reg != RAX && inst.ops[0].reg != RDX) ||
				(count == 1 && insts[at[0]].ops[0].reg == inst.ops[0].reg) || (inst.flags & MF_VOLATILE) ||
				(count == 1 && insts[at[0]].ops[1].uses_reg(inst.ops[0].reg)))
				break;
			at[count++] = i;
			start = i;
		}
		// at[count - 1] runs first.
		Inst now[2];
		bool changed = false;
		std::vector<Inst> composed;
		for (uint32_t k = 0; k < count; ++k)
		{
			const Inst& inst = insts[at[k]];
			now[k] = inst;
			if (inst.op == Op::Load && fixed_frame(inst.ops[1]) &&
				(register_bytes(inst.type) == 8 || inst.type == lowir::TypeKind::U32) &&
				resolve_return_load(
					function, block.first, start, inst, writes, now[k], count == 1 ? &composed : nullptr))
				changed = true;
		}
		if (!changed)
			continue;
		if (!composed.empty())
		{
			// The block moves to the end, the halves' instructions in the
			// load's place.
			const uint32_t moved = static_cast<uint32_t>(insts.size());
			insts.reserve(insts.size() + block.count + composed.size());
			for (uint32_t i = block.first; i <= ret; ++i)
			{
				if (i != at[0])
					insts.push_back(insts[i]);
				else
					insts.insert(insts.end(), composed.begin(), composed.end());
			}
			for (uint32_t i = block.first; i <= ret; ++i)
			{
				insts[i] = Inst();
				insts[i].op = Op::Nop;
			}
			block.first = moved;
			block.count = static_cast<uint32_t>(insts.size()) - moved;
			ret = block.first + block.count - 1;
			count = 0;
		}
		// One parallel assignment: neither reads what the other writes, or
		// the one that does runs first; else a frame load stays (it reads
		// no register).
		auto reads = [](const Inst& inst, Reg r) {
			return inst.ops[1].kind == OperandKind::Reg ? inst.ops[1].reg == r : inst.ops[1].uses_reg(r);
		};
		if (count == 2)
		{
			Inst& first = now[1];
			Inst& second = now[0];
			if (reads(second, first.ops[0].reg))
			{
				if (!reads(first, second.ops[0].reg))
					std::swap(now[0], now[1]);
				else if (insts[at[0]].op == Op::Load && fixed_frame(insts[at[0]].ops[1]))
					now[0] = insts[at[0]];
				else if (insts[at[1]].op == Op::Load && fixed_frame(insts[at[1]].ops[1]))
				{
					now[1] = insts[at[1]];
					std::swap(now[0], now[1]);
				}
				else
					continue;
			}
		}
		for (uint32_t k = 0; k < count; ++k)
			insts[at[k]] = now[k];
		// Frame writes nothing reads before the return: the frame dies there.
		std::vector<std::pair<int32_t, int32_t>> read;
		auto read_overlaps = [&](int32_t lo, int32_t hi) {
			for (const std::pair<int32_t, int32_t>& r : read)
				if (lo < r.second && r.first < hi)
					return true;
			return false;
		};
		for (uint32_t i = ret; i-- > block.first;)
		{
			Inst& inst = insts[i];
			if (inst.op == Op::Nop)
				continue;
			if (inst.flags & MF_VOLATILE)
				break;
			const bool store = inst.op == Op::Store && fixed_frame(inst.ops[0]);
			const bool zero = inst.op == Op::ZeroBytes && fixed_frame(inst.ops[0]);
			const bool copy = inst.op == Op::CopyBytes && !(inst.flags & MF_COPY_DYNAMIC) && fixed_frame(inst.ops[0]);
			if (store || zero || copy)
			{
				const int32_t x = inst.ops[0].disp;
				const int32_t bytes = store ? static_cast<int32_t>(register_bytes(inst.type))
											: static_cast<int32_t>(inst.aux);
				if (!read_overlaps(x, x + bytes))
				{
					inst = Inst();
					inst.op = Op::Nop;
					++removed;
					continue;
				}
				if (!copy)
					continue;
				if (fixed_frame(inst.ops[1]))
					read.push_back(std::make_pair(inst.ops[1].disp, inst.ops[1].disp + bytes));
				else if (escapes || frame_based(inst.ops[1]))
					break;
				continue;
			}
			if (!simple_inst(inst))
				break;
			bool stop = false;
			for (unsigned k = 0; k < inst.nops && !stop; ++k)
			{
				const Operand& op = inst.ops[k];
				if (op.kind != OperandKind::Mem || (k == 0 && inst.op == Op::Store) || (k == 1 && inst.op == Op::Lea))
					continue;
				if (fixed_frame(op))
					read.push_back(std::make_pair(op.disp, op.disp + static_cast<int32_t>(register_bytes(inst.type))));
				else
					stop = escapes || frame_based(op);
			}
			if (stop)
				break;
		}
	}
	delete writes;
	return removed;
}

} // namespace

// (A second round sees the bytes the first's removed writes wrote as
// never written.)
uint32_t forward_return_slots(Function& function)
{
	const uint32_t removed = forward_return_slots_once(function);
	return removed == 0 ? 0 : removed + forward_return_slots_once(function);
}

// O1+, leaves: reserve no frame storage when every use has disappeared.
bool drop_unused_frame(Function& function)
{
	if (!removable_frame_storage(function, false))
		return false;
	function.stack_size = 0;
	return true;
}

namespace {

// Whether `cond` holds after `cmp.T a, b` (with `test`, after `test.T a,
// b`) at T's width: 1, 0, or -1 for a parity test.
int decided_condition(Cond cond, bool test, uint64_t a, uint64_t b, unsigned bytes)
{
	const uint64_t mask = bytes >= 8 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << (8 * bytes)) - 1;
	const uint64_t sign = static_cast<uint64_t>(1) << (8 * bytes - 1);
	a &= mask;
	b &= mask;
	const uint64_t r = test ? a & b : (a - b) & mask;
	const bool zf = r == 0, sf = (r & sign) != 0;
	const bool cf = !test && a < b;
	const bool of = !test && ((a ^ b) & (a ^ r) & sign) != 0;
	switch (cond)
	{
	case CC_O:
		return of;
	case CC_NO:
		return !of;
	case CC_B:
		return cf;
	case CC_AE:
		return !cf;
	case CC_E:
		return zf;
	case CC_NE:
		return !zf;
	case CC_BE:
		return cf || zf;
	case CC_A:
		return !cf && !zf;
	case CC_S:
		return sf;
	case CC_NS:
		return !sf;
	case CC_L:
		return sf != of;
	case CC_GE:
		return sf == of;
	case CC_LE:
		return zf || sf != of;
	case CC_G:
		return !zf && sf == of;
	default:
		return -1;
	}
}

// What a block leaves in GPR r at its final jump: an immediate (`value`),
// or a defined symbol's or frame address (`nonzero`). False when unknown.
bool left_value(
	const Function& f, const lowir::Program& program, const Block& block, Reg r, uint64_t& value, bool& nonzero)
{
	auto defined = [&](uint32_t symbol, SymbolAccess access) {
		if (access != SymbolAccess::Direct || symbol >= program.symbols.size())
			return false;
		const lowir::SymbolKind kind = program.symbols[symbol].kind;
		return kind == lowir::SymbolKind::GlobalDefinition || kind == lowir::SymbolKind::FunctionDefinition;
	};
	for (uint32_t i = block.first + block.count - 1; i-- > block.first;)
	{
		const Inst& inst = f.insts[i];
		const bool writes = inst.nops != 0 && inst.ops[0].kind == OperandKind::Reg && inst.ops[0].reg == r;
		if (writes && inst.op == Op::Mov)
		{
			nonzero = inst.ops[1].kind == OperandKind::Symbol && defined(inst.ops[1].symbol, inst.ops[1].access);
			value = inst.ops[1].imm;
			return nonzero || inst.ops[1].kind == OperandKind::Imm;
		}
		if (writes && inst.op == Op::Lea)
		{
			const Operand& m = inst.ops[1];
			nonzero = (inst.type == lowir::TypeKind::Void || inst.type == lowir::TypeKind::Ptr ||
						  inst.type == lowir::TypeKind::I64) &&
				m.index == kNoReg &&
				(m.symbol != lowir::kNoId ? defined(m.symbol, m.access) : m.reg == RBP || m.reg == RSP);
			return nonzero;
		}
		if (simple_inst(inst) ? written_reg(inst) == r : reads_or_writes(inst, r))
			return false;
	}
	return false;
}

const uint32_t kNoTarget = 0xffffffffu;
// A block copied into the block jumping to it holds at most this many
// instructions, its final `jcc; jmp` included.
const uint32_t kCopyThrough = 5;

// Block b, ending in a jump to t (simple instructions, then `jcc X; jmp
// Y`), ends in a copy of t instead when the value b leaves decides the test
// X or Y is: the copy's branch goes to the decided side. `decided(b, view,
// test)` names that side, or kNoTarget.
template<typename Decided>
bool copy_through(Function& f, uint32_t b, uint32_t t, const Decided& decided)
{
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	if (t == b || t >= blocks || f.blocks[t].landing || f.blocks[t].count < 3 || f.blocks[t].count > kCopyThrough)
		return false;
	const Block from = f.blocks[b], test = f.blocks[t];
	const Inst jcc = f.insts[test.first + test.count - 2], jmp = f.insts[test.first + test.count - 1];
	if (jcc.op != Op::Jcc || jcc.ops[0].kind != OperandKind::Label || jmp.op != Op::Jmp ||
		jmp.ops[0].kind != OperandKind::Label)
		return false;
	for (uint32_t i = test.first; i + 2 < test.first + test.count; ++i)
		if (!simple_inst(f.insts[i]))
			return false;
	const uint32_t at = static_cast<uint32_t>(f.insts.size());
	for (uint32_t i = from.first; i + 1 < from.first + from.count; ++i)
		f.insts.push_back(f.insts[i]);
	for (uint32_t i = test.first; i < test.first + test.count; ++i)
		f.insts.push_back(f.insts[i]);
	Block view = from;
	view.first = at;
	view.count = static_cast<uint32_t>(f.insts.size()) - at - 1; // up to the jcc
	const uint32_t on_jump = decided(b, view, jmp.ops[0].symbol);
	const uint32_t on_jcc = decided(b, view, jcc.ops[0].symbol);
	if (on_jump == kNoTarget && on_jcc == kNoTarget)
	{
		f.insts.resize(at);
		return false;
	}
	if (on_jump != kNoTarget)
		f.insts.back().ops[0].symbol = on_jump;
	if (on_jcc != kNoTarget)
		f.insts[f.insts.size() - 2].ops[0].symbol = on_jcc;
	f.blocks[b].first = at;
	f.blocks[b].count = static_cast<uint32_t>(f.insts.size()) - at;
	return true;
}

} // namespace

// O1+, before block placement: a block ending in a jump to a test block
// (`cmp r, imm` or `test r, r|imm`, then `jcc X; jmp Y`, nothing else)
// whose register it left a known value in jumps to the side the test
// takes: the flags die with the test block, so the test only picks the
// successor. An address (a defined symbol's, a frame location) decides
// only whether it is zero. A rotated loop's entry test of its constant
// start, a switch's cases meeting at a null test of the table they chose.
// A jump to a short block (simple instructions, then `jcc X; jmp Y`) whose
// value decides the test at X or Y takes a copy of that block, branching to
// the decided side: a latch entered with a constant from its header's test
// then skips the test on its way around. Up to four steps per jump; linear.
// A block that starts with the compare of registers each of its predecessors
// ends with reads the flags they left: `if (a > b) ... else if (a < b)`, and
// the copy of a loop's test on its back edge. The compare goes, as does the
// second of two alike in a block with nothing that sets flags between. Linear.
uint32_t drop_repeated_compares(Function& f)
{
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	const uint32_t kUnseen = lowir::kNoId, kMixed = lowir::kNoId - 1;
	auto compare_of = [](const Inst& inst) {
		return (inst.op == Op::Cmp || inst.op == Op::Test) && inst.ops[0].is_reg() &&
			(inst.ops[1].is_reg() || inst.ops[1].is_imm());
	};
	auto same = [](const Inst& a, const Inst& b) {
		return a.op == b.op && a.type == b.type && a.ops[0].reg == b.ops[0].reg && a.ops[1].kind == b.ops[1].kind &&
			a.ops[1].reg == b.ops[1].reg && a.ops[1].imm == b.ops[1].imm;
	};
	// Per block: the compare its predecessors end with.
	std::vector<uint32_t> left(blocks, kUnseen);
	auto edge = [&](uint32_t t, uint32_t compare) {
		if (t >= blocks)
			return;
		if (left[t] == kUnseen)
			left[t] = compare;
		else if (compare == kMixed || left[t] == kMixed || !same(f.insts[left[t]], f.insts[compare]))
			left[t] = kMixed;
	};
	if (blocks != 0)
		edge(0, kMixed);
	for (const EhLanding& landing : f.eh.landings)
	{
		if (landing.pad != lowir::kNoId)
			edge(landing.pad, kMixed);
		if (landing.select != lowir::kNoId)
			edge(landing.select, kMixed);
	}
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const Block& block = f.blocks[b];
		if (block.landing)
			edge(b, kMixed);
		const uint32_t end = block.first + block.count;
		const bool tail = block.count >= 3 && f.insts[end - 1].op == Op::Jmp && f.insts[end - 2].op == Op::Jcc &&
			compare_of(f.insts[end - 3]);
		Op last = Op::Nop;
		for (uint32_t i = block.first; i < end; ++i)
		{
			const Inst& inst = f.insts[i];
			if (inst.op == Op::Nop)
				continue;
			last = inst.op;
			if (inst.op == Op::JumpTable)
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					edge(f.jump_targets[k], kMixed);
			if (inst.op == Op::WideCmpBranch)
				edge(inst.aux, kMixed);
			for (unsigned k = 0; k < inst.nops; ++k)
				if (inst.ops[k].kind == OperandKind::Label)
					edge(inst.ops[k].symbol, tail && i + 2 >= end ? end - 3 : kMixed);
		}
		if (!(last == Op::Jmp || last == Op::Ret || last == Op::TailJump || last == Op::JumpTable || last == Op::Trap ||
				last == Op::Resume || last == Op::Throw))
			edge(b + 1, kMixed);
	}
	uint32_t removed = 0;
	// Two selects of one test in a block compare once: a conditional move
	// keeps the flags, as do copies and memory accesses.
	for (const Block& block : f.blocks)
	{
		uint32_t earlier = kUnseen;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			if (compare_of(inst))
			{
				if (earlier == kUnseen || !same(f.insts[earlier], inst))
				{
					earlier = i;
					continue;
				}
				inst.op = Op::Nop;
				inst.nops = 0;
				++removed;
				continue;
			}
			// (`mov r, 0` may be encoded as an xor.)
			const bool quiet = inst.op == Op::Nop || inst.op == Op::Cmov || inst.op == Op::Lea || inst.op == Op::Load ||
				inst.op == Op::Store || (inst.op == Op::Mov && !(inst.ops[1].is_imm() && inst.ops[1].imm == 0));
			const Reg w = inst.op == Op::Nop ? kNoReg : written_reg(inst);
			if (earlier != kUnseen &&
				(!quiet || w == f.insts[earlier].ops[0].reg ||
					(f.insts[earlier].ops[1].is_reg() && w == f.insts[earlier].ops[1].reg)))
				earlier = kUnseen;
		}
	}
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const Block& block = f.blocks[b];
		if (block.count == 0 || left[b] >= kMixed)
			continue;
		Inst& first = f.insts[block.first];
		if (compare_of(first) && same(first, f.insts[left[b]]) && static_cast<uint32_t>(left[b]) != block.first)
		{
			first.op = Op::Nop;
			first.nops = 0;
			++removed;
		}
	}
	return removed;
}

uint32_t thread_decided_tests(Function& f, const lowir::Program& program)
{
	uint32_t threaded = 0;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	// The side test block t takes for the value block b (its instructions
	// `view`, up to its last) leaves in the tested register, or kNoTarget.
	auto decided = [&](uint32_t b, const Block& view, uint32_t t) {
		if (t == b || t >= blocks || f.blocks[t].count != 3 || f.blocks[t].landing)
			return kNoTarget;
		const Inst* test = &f.insts[f.blocks[t].first];
		const Inst& jcc = test[1];
		const Inst& jmp = test[2];
		const bool is_test = test->op == Op::Test;
		if ((test->op != Op::Cmp && !is_test) || test->ops[0].kind != OperandKind::Reg || test->ops[0].reg >= 16 ||
			jcc.op != Op::Jcc || jcc.ops[0].kind != OperandKind::Label || jmp.op != Op::Jmp ||
			jmp.ops[0].kind != OperandKind::Label)
			return kNoTarget;
		const Reg r = test->ops[0].reg;
		const Operand& other = test->ops[1];
		if (!(other.kind == OperandKind::Imm || (is_test && other.kind == OperandKind::Reg && other.reg == r)))
			return kNoTarget;
		uint64_t value = 0;
		bool nonzero = false;
		if (!left_value(f, program, view, r, value, nonzero))
			return kNoTarget;
		const unsigned bytes = register_bytes(test->type);
		const Cond cond = static_cast<Cond>(jcc.sub);
		int holds;
		if (!nonzero)
			holds = decided_condition(cond, is_test, value, other.kind == OperandKind::Imm ? other.imm : value, bytes);
		else if (bytes != 8 || (other.kind == OperandKind::Imm && other.imm != 0))
			holds = -1;
		else
			holds = cond == CC_NE || cond == CC_A || cond == CC_AE ? 1
				: cond == CC_E || cond == CC_BE || cond == CC_B    ? 0
																   : -1;
		if (holds < 0)
			return kNoTarget;
		return holds ? jcc.ops[0].symbol : jmp.ops[0].symbol;
	};
	for (uint32_t b = 0; b < blocks; ++b)
	{
		if (f.blocks[b].count == 0)
			continue;
		for (unsigned step = 0; step < 4; ++step)
		{
			Inst& jump = f.insts[f.blocks[b].first + f.blocks[b].count - 1];
			if (jump.op != Op::Jmp || jump.ops[0].kind != OperandKind::Label)
				break;
			const uint32_t t = jump.ops[0].symbol;
			const uint32_t to = decided(b, f.blocks[b], t);
			if (to != kNoTarget)
			{
				jump.ops[0].symbol = to;
				++threaded;
				continue;
			}
			if (!copy_through(f, b, t, decided))
				break;
			++threaded;
		}
	}
	return threaded;
}

namespace {
struct IntegerInterval
{
	int64_t lo = 0, hi = 0;
	bool known = false;
	static IntegerInterval make(int64_t lo, int64_t hi)
	{
		IntegerInterval r;
		r.lo = lo;
		r.hi = hi;
		r.known = true;
		return r;
	}
};
using IntegerState = std::array<IntegerInterval, 16>;
IntegerInterval interval_operand(const Operand& op, const IntegerState& state)
{
	if (op.kind == OperandKind::Reg && op.reg < 16)
		return state[op.reg];
	if (op.kind == OperandKind::Imm)
		return IntegerInterval::make(op.imm, op.imm);
	return IntegerInterval();
}
IntegerInterval interval_sum(IntegerInterval a, IntegerInterval b, int64_t scale = 1, int64_t disp = 0)
{
	if (!a.known || !b.known)
		return IntegerInterval();
	const __int128 lo = static_cast<__int128>(a.lo) + static_cast<__int128>(b.lo) * scale + disp;
	const __int128 hi = static_cast<__int128>(a.hi) + static_cast<__int128>(b.hi) * scale + disp;
	if (lo < INT64_MIN || hi > INT64_MAX)
		return IntegerInterval();
	return IntegerInterval::make(static_cast<int64_t>(lo), static_cast<int64_t>(hi));
}
IntegerInterval interval_extension(const Inst& inst)
{
	const unsigned bits = 8 * register_bytes(inst.type);
	if (bits >= 64)
		return IntegerInterval();
	if (inst.op == Op::Sext)
		return IntegerInterval::make(-(int64_t(1) << (bits - 1)), (int64_t(1) << (bits - 1)) - 1);
	return IntegerInterval::make(0, (int64_t(1) << bits) - 1);
}
void transfer_integer_interval(const Inst& inst, IntegerState& state)
{
	if (inst.op == Op::Nop || inst.op == Op::Jcc || inst.op == Op::Jmp || inst.op == Op::Ret)
		return;
	const bool variable_shift = (inst.op == Op::Shl || inst.op == Op::Shr || inst.op == Op::Sar || inst.op == Op::Rol ||
									inst.op == Op::Ror) &&
		inst.ops[1].kind != OperandKind::Imm;
	if (!simple_inst(inst) || variable_shift)
	{
		state = IntegerState();
		return;
	}
	const Reg d = written_reg(inst);
	if (d >= 16)
		return;
	IntegerInterval value;
	const IntegerInterval old = state[d];
	switch (inst.op)
	{
	case Op::Mov:
		if (wide_kind(inst.type))
			value = interval_operand(inst.ops[1], state);
		break;
	case Op::Load:
		switch (inst.type)
		{
		case lowir::TypeKind::U8:
		case lowir::TypeKind::U16:
		case lowir::TypeKind::U32:
			value = IntegerInterval::make(0, (int64_t(1) << (8 * register_bytes(inst.type))) - 1);
			break;
		case lowir::TypeKind::I8:
		case lowir::TypeKind::I16:
		case lowir::TypeKind::I32:
			value = IntegerInterval::make(-(int64_t(1) << (8 * register_bytes(inst.type) - 1)),
				(int64_t(1) << (8 * register_bytes(inst.type) - 1)) - 1);
			break;
		default:
			break;
		}
		break;
	case Op::Sext:
	case Op::Zext:
		value = interval_extension(inst);
		break;
	case Op::Movzx:
		value = IntegerInterval::make(0, 255);
		break;
	case Op::Lea:
		if (inst.ops[1].kind == OperandKind::Mem && inst.ops[1].symbol == lowir::kNoId)
		{
			const Operand& m = inst.ops[1];
			const IntegerInterval a = m.reg == kNoReg ? IntegerInterval::make(0, 0)
				: m.reg < 16                          ? state[m.reg]
													  : IntegerInterval();
			const IntegerInterval b = m.index == kNoReg ? IntegerInterval::make(0, 0)
				: m.index < 16                          ? state[m.index]
														: IntegerInterval();
			value = interval_sum(a, b, m.scale, m.disp);
			if (!wide_kind(inst.type) &&
				!(inst.type == lowir::TypeKind::I32 && value.known && value.lo >= 0 && value.hi <= UINT32_MAX))
				value = IntegerInterval();
		}
		break;
	case Op::And:
		if (inst.ops[1].kind == OperandKind::Imm && inst.ops[1].imm >= 0 &&
			(wide_kind(inst.type) || inst.type == lowir::TypeKind::I32 || inst.type == lowir::TypeKind::U32))
			value = IntegerInterval::make(0, inst.ops[1].imm);
		break;
	case Op::Add:
	case Op::Sub:
		if (inst.ops[1].kind == OperandKind::Imm && (wide_kind(inst.type) || inst.type == lowir::TypeKind::U32))
		{
			IntegerInterval rhs = interval_operand(inst.ops[1], state);
			if (inst.op == Op::Sub && rhs.lo != INT64_MIN)
			{
				rhs.lo = -rhs.lo;
				rhs.hi = -rhs.hi;
			}
			else if (inst.op == Op::Sub)
				rhs = IntegerInterval();
			value = interval_sum(old, rhs);
			if (inst.type == lowir::TypeKind::U32 && value.known && (value.lo < 0 || value.hi > UINT32_MAX))
				value = IntegerInterval();
		}
		break;
	default:
		break;
	}
	state[d] = value;
}
IntegerInterval filter_integer_interval(IntegerInterval value, unsigned bits, int64_t lo, int64_t hi)
{
	if (!value.known || bits >= 64 || lo > hi)
		return IntegerInterval();
	const int64_t modulus = int64_t(1) << bits;
	auto floor = [&](int64_t v) {
		const int64_t q = v / modulus;
		return q - (v < 0 && v % modulus != 0 ? 1 : 0);
	};
	const int64_t first = floor(value.lo), last = floor(value.hi);
	if (last - first > 1)
		return value;
	bool found = false;
	int64_t lower = 0, upper = 0;
	for (int64_t q = first;; ++q)
	{
		const __int128 base = static_cast<__int128>(q) * modulus;
		const __int128 a = std::max<__int128>(value.lo, base + lo), b = std::min<__int128>(value.hi, base + hi);
		if (a <= b)
		{
			if (!found)
			{
				lower = static_cast<int64_t>(a);
				upper = static_cast<int64_t>(b);
				found = true;
			}
			else
				upper = static_cast<int64_t>(b);
		}
		if (q == last)
			break;
	}
	return found ? IntegerInterval::make(lower, upper) : IntegerInterval();
}
class GuardedIntegerRanges
{
	Function& f_;
	std::vector<IntegerState> inputs_;
	std::vector<uint8_t> reached_;
	void push(uint32_t b, const IntegerState& state)
	{
		if (b >= inputs_.size())
			return;
		if (!reached_[b])
		{
			inputs_[b] = state;
			reached_[b] = 1;
			return;
		}
		for (unsigned r = 0; r < 16; ++r)
		{
			IntegerInterval& old = inputs_[b][r];
			const IntegerInterval& in = state[r];
			if (!old.known)
				continue;
			if (!in.known)
				old = IntegerInterval();
			else
			{
				old.lo = std::min(old.lo, in.lo);
				old.hi = std::max(old.hi, in.hi);
			}
		}
	}
	void edge(uint32_t target, const IntegerState& state, const Inst* cmp, Cond cond)
	{
		IntegerState refined = state;
		if (cmp && cmp->op == Op::Cmp && plain_gpr(cmp->ops[0]) && cmp->ops[1].kind == OperandKind::Imm)
		{
			const unsigned bits = 8 * register_bytes(cmp->type);
			if (bits <= 32)
			{
				const int64_t mask = (int64_t(1) << bits) - 1, c = cmp->ops[1].imm & mask;
				int64_t lo = 0, hi = mask;
				bool supported = true;
				switch (cond)
				{
				case CC_BE:
					hi = c;
					break;
				case CC_B:
					hi = c - 1;
					break;
				case CC_A:
					lo = c + 1;
					break;
				case CC_AE:
					lo = c;
					break;
				case CC_E:
					lo = hi = c;
					break;
				default:
					supported = false;
					break;
				}
				if (supported)
					refined[cmp->ops[0].reg] = filter_integer_interval(state[cmp->ops[0].reg], bits, lo, hi);
			}
		}
		push(target, refined);
	}
	void successors(uint32_t b, const IntegerState& state)
	{
		const Block& block = f_.blocks[b];
		const Inst* tail[3] = { nullptr, nullptr, nullptr };
		unsigned n = 0, branches = 0;
		for (uint32_t i = block.first + block.count; i > block.first;)
		{
			const Inst& inst = f_.insts[--i];
			if (inst.op == Op::Jcc || inst.op == Op::Jmp || inst.op == Op::WideCmpBranch || inst.op == Op::JumpTable)
				++branches;
			if (inst.op != Op::Nop && n < 3)
				tail[2 - n++] = &inst;
		}
		if (branches == 2 && tail[1] && tail[2])
		{
			const Inst& j = *tail[1];
			const Inst& end = *tail[2];
			if (j.op == Op::Jcc && end.op == Op::Jmp && j.ops[0].kind == OperandKind::Label &&
				end.ops[0].kind == OperandKind::Label)
			{
				edge(j.ops[0].symbol, state, tail[0], static_cast<Cond>(j.sub));
				edge(end.ops[0].symbol, state, tail[0], invert(static_cast<Cond>(j.sub)));
				return;
			}
		}
		const bool one_terminal = branches == 1 && tail[2] &&
			(tail[2]->op == Op::Jmp || tail[2]->op == Op::WideCmpBranch || tail[2]->op == Op::JumpTable);
		const IntegerState outgoing = one_terminal ? state : IntegerState();
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = f_.insts[i];
			if ((inst.op == Op::Jcc || inst.op == Op::Jmp) && inst.ops[0].kind == OperandKind::Label)
				push(inst.ops[0].symbol, outgoing);
			else if (inst.op == Op::WideCmpBranch)
			{
				push(inst.ops[2].symbol, outgoing);
				push(inst.aux, outgoing);
			}
			else if (inst.op == Op::JumpTable)
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					push(f_.jump_targets[k], outgoing);
		}
		// Unknown/implicit fallthrough contributes no register proof.
		if (!tail[2] ||
			(tail[2]->op != Op::Ret && tail[2]->op != Op::Jmp && tail[2]->op != Op::WideCmpBranch &&
				tail[2]->op != Op::JumpTable))
			push(b + 1, IntegerState());
	}

public:
	explicit GuardedIntegerRanges(Function& f) : f_(f), inputs_(f.blocks.size()), reached_(f.blocks.size(), 0) {}
	uint32_t run()
	{
		if (f_.blocks.empty())
			return 0;
		push(0, IntegerState());
		for (uint32_t b = 0; b < f_.blocks.size(); ++b)
			if (f_.blocks[b].landing)
				push(b, IntegerState());
		// Each predecessor proves its own outgoing facts without assuming any
		// incoming bounds. Their meet therefore needs no iterative loop analysis.
		for (uint32_t b = 0; b < f_.blocks.size(); ++b)
		{
			IntegerState state;
			const Block& block = f_.blocks[b];
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
				transfer_integer_interval(f_.insts[i], state);
			successors(b, state);
		}
		uint32_t removed = 0;
		for (uint32_t b = 0; b < f_.blocks.size(); ++b)
		{
			if (!reached_[b])
				continue;
			IntegerState state = inputs_[b];
			const Block& block = f_.blocks[b];
			for (uint32_t i = block.first; i < block.first + block.count; ++i)
			{
				Inst& inst = f_.insts[i];
				if ((inst.op == Op::Sext || inst.op == Op::Zext) && inst.type >= lowir::TypeKind::I8 &&
					inst.type <= lowir::TypeKind::U32 && plain_gpr(inst.ops[0]))
				{
					const Operand source = inst.nops == 1 ? inst.ops[0] : inst.ops[1];
					const IntegerInterval value = interval_operand(source, state), range = interval_extension(inst);
					if (plain_gpr(source) && value.known && range.known && value.lo >= range.lo && value.hi <= range.hi)
					{
						if (source == inst.ops[0])
						{
							inst.op = Op::Nop;
							inst.nops = 0;
							inst.ops[0] = inst.ops[1] = Operand();
						}
						else
						{
							inst.op = Op::Mov;
							inst.type = lowir::TypeKind::Void;
							inst.nops = 2;
							inst.ops[1] = source;
						}
						++removed;
					}
				}
				transfer_integer_interval(inst, state);
			}
		}
		return removed;
	}
};
} // namespace
uint32_t drop_guarded_integer_extensions(Function& f)
{
	bool any = false;
	for (const Inst& inst : f.insts)
	{
		if (inst.op == Op::Jmp && inst.ops[0].kind != OperandKind::Label)
			return 0;
		any |= inst.op == Op::Sext || inst.op == Op::Zext;
	}
	if (!any || f.blocks.size() > 65536)
		return 0;
	GuardedIntegerRanges ranges(f);
	return ranges.run();
}

} // namespace native
