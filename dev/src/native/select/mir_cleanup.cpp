#include "native/select/mir_cleanup.h"

#include <algorithm>
#include <climits>

#include "native/mir/type_kinds.h"

// MIR cleanups after selection (O1+): see mir_cleanup.h.
namespace native {

namespace {

// The stores insts[first, past) (immediates, one base): rewritten in place
// when fewer chunks cover their bytes; stores that overlap, or span more
// than 64 bytes, stay.
void merge_store_run(std::vector<Inst>& insts, uint32_t first, uint32_t past)
{
	const uint32_t kSpan = 64;
	int32_t lo = INT32_MAX, hi = INT32_MIN;
	for (uint32_t k = first; k < past; ++k)
	{
		const int32_t disp = insts[k].ops[0].disp;
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
			if (written[at + b])
				return;
			written[at + b] = 1;
			value[at + b] = static_cast<uint8_t>(static_cast<uint64_t>(insts[k].ops[1].imm) >> (8 * b));
		}
	}
	static const lowir::TypeKind kTypes[9] = { lowir::TypeKind::Void, lowir::TypeKind::I8, lowir::TypeKind::I16,
		lowir::TypeKind::Void, lowir::TypeKind::I32, lowir::TypeKind::Void, lowir::TypeKind::Void,
		lowir::TypeKind::Void, lowir::TypeKind::I64 };
	int32_t disps[kSpan];
	uint8_t widths[kSpan];
	int64_t imms[kSpan];
	uint32_t count = 0;
	const uint32_t span = static_cast<uint32_t>(hi - lo);
	for (uint32_t p = 0; p < span;)
	{
		if (!written[p])
		{
			++p;
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
			while (j < block.first + block.count && mergeable(insts[j]) && insts[j].ops[0].reg == insts[i].ops[0].reg)
				++j;
			if (j - i < 2)
			{
				i = j == i ? i + 1 : j;
				continue;
			}
			merge_store_run(insts, i, j);
			i = j;
		}
	}
}

// Loop rotation by tail duplication: a block ending in a jump back to an
// earlier rotatable test block ends instead with a copy of that test (its
// range moves to the end of the instructions: ranges need not ascend), so
// each iteration branches back once, conditionally, instead of jumping to
// a test that branches forward (the layout below drops or inverts the
// copied branches). The test still guards the loop's entry. Linear;
// growth at most kRotateCap instructions per back edge.
uint32_t rotate_loops(Function& f)
{
	uint32_t rotated = 0;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const Block block = f.blocks[b];
		if (block.count == 0 || block.landing)
			continue;
		const Inst& last = f.insts[block.first + block.count - 1];
		if (last.op != Op::Jmp || last.ops[0].kind != OperandKind::Label || last.ops[0].symbol >= b ||
			!rotatable(f, last.ops[0].symbol))
			continue;
		const Block test = f.blocks[last.ops[0].symbol];
		const uint32_t first = static_cast<uint32_t>(f.insts.size());
		f.insts.reserve(f.insts.size() + block.count + test.count);
		for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
			f.insts.push_back(f.insts[i]);
		for (uint32_t i = test.first; i < test.first + test.count; ++i)
			f.insts.push_back(f.insts[i]);
		f.blocks[b].first = first;
		f.blocks[b].count = static_cast<uint32_t>(f.insts.size()) - first;
		++rotated;
	}
	return rotated;
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

	Function& f_;
	uint32_t n_;
	std::vector<int32_t> homes_;
	// Distinct successors and predecessors (CSR, by block).
	std::vector<uint32_t> succ_first_, succs_, pred_first_, preds_;
	std::vector<uint8_t> cut_;    // no inflow: the entry, landings, course handlers
	std::vector<uint32_t> order_; // reverse postorder from the entry
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
			if (inst.op == Op::JumpTable)
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
		if (inst.op == Op::Jmp)
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
		const unsigned bytes = wide ? 8 : register_bytes(inst.type);
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
		// A narrow reload extends the holder's low bytes as the load would.
		if (inst.op == Op::Load && !wide && bytes < 8 && plain_gpr(inst.ops[0]) && frame_slot(inst.ops[1], offset) &&
			(offset & 7) == 0)
		{
			const Reg dst = inst.ops[0].reg;
			const Reg holder = holder_of(offset, bytes);
			holds[dst] = dst == R11 ? kHoldsNothing : held(offset, bytes);
			if (!rewrite || holder == kNoReg)
				continue;
			const lowir::TypeKind type = inst.type;
			const bool sign = type == lowir::TypeKind::I8 || type == lowir::TypeKind::I16 ||
				type == lowir::TypeKind::I32;
			inst.op = sign ? Op::Sext : Op::Zext;
			inst.type = type == lowir::TypeKind::I1 ? lowir::TypeKind::U8 : type;
			inst.ops[1] = Operand::make_reg(holder);
			++removed_;
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
			const Reg holder = holder_of(offset, 8);
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
	for (const FrameBinding& binding : f_.bindings)
		if (binding.kind != BindingKind::Slot && !binding.type.is_object() &&
			binding.type.kind() != lowir::TypeKind::F80 && binding.type.kind() != lowir::TypeKind::I128)
			homes_.push_back(binding.offset);
	std::sort(homes_.begin(), homes_.end());
	if (n_ == 0 || f_.stack_size == 0)
		return 0;
	edges();
	// The available holders to a fixed point (each state only falls:
	// any, then one home, then none), then the rewrite from them.
	std::vector<int32_t> out(static_cast<size_t>(n_) * 16, kHoldsAny);
	std::vector<int32_t> in(static_cast<size_t>(n_) * 16, kHoldsNothing);
	auto meet = [&](uint32_t b, int32_t* holds) {
		for (Reg r = 0; r < 16; ++r)
		{
			int32_t h = cut_[b] || pred_first_[b] == pred_first_[b + 1] ? kHoldsNothing : kHoldsAny;
			for (uint32_t k = pred_first_[b]; k < pred_first_[b + 1]; ++k)
			{
				const uint32_t p = preds_[k];
				const int32_t o = out[static_cast<size_t>(p) * 16 + r];
				h = h == kHoldsAny ? o : (o == kHoldsAny || o == h ? h : kHoldsNothing);
			}
			holds[r] = h == kHoldsAny ? kHoldsNothing : h;
		}
	};
	// A block met with the input of its last evaluation leaves the same
	// output (transfer reads nothing else without rewriting): skipped.
	std::vector<uint8_t> evaluated(n_, 0);
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b : order_)
		{
			int32_t holds[16];
			meet(b, holds);
			int32_t* const block_in = &in[static_cast<size_t>(b) * 16];
			if (evaluated[b] && std::equal(holds, holds + 16, block_in))
				continue;
			evaluated[b] = 1;
			std::copy(holds, holds + 16, block_in);
			transfer(b, holds, false);
			for (Reg r = 0; r < 16; ++r)
				if (out[static_cast<size_t>(b) * 16 + r] != holds[r])
				{
					out[static_cast<size_t>(b) * 16 + r] = holds[r];
					changed = true;
				}
		}
	}
	for (uint32_t b : order_)
		transfer(b, &in[static_cast<size_t>(b) * 16], true);
	return removed_;
}

} // namespace

// O1+: a reload of a temp's frame home (8 bytes) whose value a register
// holds on every path to it -- since a store of that register to the home
// or a reload of it, neither written since; also of a slot, unwritten
// through a pointer and no call between (available-value dataflow over
// the blocks' branches to a fixed point; nothing flows into the entry, a
// landing or a course handler) -- goes when that register is its
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

// O1+: `test r, r` (or `cmp r, 0`, which sets the flags alike) right after
// an instruction that set the flags from r at the same width goes when
// every reader of the flags before they are set again (or the block ends:
// the selector keeps no flags across blocks) reads what both set alike:
// any condition after and/or/xor (CF and OF clear in both), only the zero
// and sign conditions after add, sub and a shift by a nonzero immediate.
// Linear.
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
		for (uint32_t i = block.first + 1; i < end; ++i)
		{
			Inst& test = insts[i];
			const Inst& prior = insts[i - 1];
			const bool self_test = test.op == Op::Test && plain_gpr(test.ops[1]) && test.ops[1].reg == test.ops[0].reg;
			const bool zero_compare = test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm &&
				test.ops[1].imm == 0;
			if (!(self_test || zero_compare) || !plain_gpr(test.ops[0]) || prior.nops < 2 || !plain_gpr(prior.ops[0]) ||
				prior.ops[0].reg != test.ops[0].reg)
				continue;
			unsigned width = 0;
			bool full = false;
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
			if (width == 0 || width != register_bytes(test.type))
				continue;
			bool ok = true;
			for (uint32_t k = i + 1; k < end && ok; ++k)
			{
				const Inst& reader = insts[k];
				if (reader.op == Op::Jcc || reader.op == Op::Setcc || reader.op == Op::Cmov)
				{
					const Cond cond = static_cast<Cond>(reader.sub);
					ok = full || cond == CC_E || cond == CC_NE || cond == CC_S || cond == CC_NS;
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

namespace {

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

} // namespace

// O1+: `add b, imm` next to `add b, c` (c another register, the same
// width, nothing reading the flags after them) becomes `lea b, [b + c +
// imm]`; a 32-bit in-place zero-extension of a register whose upper half
// an earlier instruction of the block zeroed (zeroes_upper, through
// copies; tracking lapses at anything else writing registers) becomes a
// nop, as does one of a value a 64-bit operation just computed that can
// run at 32 bits instead (narrow_to_u32). r10 and r11 (emitter scratch)
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
			uint32_t w = i;
			if (in_place_zext && !((known >> inst.ops[0].reg) & 1u))
				while (w > block.first && i - w < 16 && !reads_or_writes(insts[w - 1], inst.ops[0].reg))
					--w;
			if (in_place_zext &&
				(((known >> inst.ops[0].reg) & 1u) ||
					(w > block.first && narrow_to_u32(insts, w - 1, inst.ops[0].reg, end))))
			{
				if (inst.ops[0].reg != R10 && inst.ops[0].reg != R11)
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
bool resolve_return_load(
	const Function& f, uint32_t first, uint32_t end, const Inst& load, const FrameWrites*& writes, Inst& out)
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
			return false;
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
	for (const Block& block : function.blocks)
	{
		if (block.count == 0 || insts[block.first + block.count - 1].op != Op::Ret ||
			(insts[block.first + block.count - 1].flags & MF_FRAMELESS))
			continue;
		const uint32_t ret = block.first + block.count - 1;
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
		for (uint32_t k = 0; k < count; ++k)
		{
			const Inst& inst = insts[at[k]];
			now[k] = inst;
			if (inst.op == Op::Load && fixed_frame(inst.ops[1]) &&
				(register_bytes(inst.type) == 8 || inst.type == lowir::TypeKind::U32) &&
				resolve_return_load(function, block.first, start, inst, writes, now[k]))
				changed = true;
		}
		if (!changed)
			continue;
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

// O1+, frame-pointer-free leaves: a frame no instruction uses (the
// cleanups above took its last reads and writes) is not reserved. Whether
// it did.
bool drop_unused_frame(Function& function)
{
	if (!function.omit_frame_pointer || function.preserve_mask != 0 || function.stack_size == 0 ||
		function.scratch_bytes != 0 || function.variadic || function.eh_records_pushed || !function.eh.landings.empty())
		return false;
	for (const Inst& inst : function.insts)
	{
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
		case Op::Bswap:
		case Op::Shl:
		case Op::Shr:
		case Op::Sar:
		case Op::Cmp:
		case Op::Test:
		case Op::Bt:
		case Op::Setcc:
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Cqo:
		case Op::Idiv:
		case Op::Div:
		case Op::Jcc:
		case Op::Jmp:
		case Op::CopyBytes:
		case Op::ZeroBytes:
		case Op::Ret:
		case Op::FMov:
		case Op::FAdd:
		case Op::FSub:
		case Op::FMul:
		case Op::FDiv:
		case Op::FNeg:
		case Op::FCmp:
		case Op::FSetcc:
		case Op::Convert:
		case Op::JumpTable:
		case Op::MulWide:
		case Op::ImulWide:
		case Op::Trap:
			break;
		case Op::Nop:
			continue;
		default:
			return false;
		}
		for (unsigned k = 0; k < inst.nops; ++k)
			if (frame_based(inst.ops[k]) || (inst.ops[k].kind == OperandKind::Reg && inst.ops[k].reg == RSP))
				return false;
	}
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

} // namespace

// O1+, before block placement: a block ending in a jump to a test block
// (`cmp r, imm` or `test r, r|imm`, then `jcc X; jmp Y`, nothing else)
// whose register it left a known value in jumps to the side the test
// takes: the flags die with the test block, so the test only picks the
// successor. An address (a defined symbol's, a frame location) decides
// only whether it is zero. A rotated loop's entry test of its constant
// start, a switch's cases meeting at a null test of the table they chose.
// Up to four tests per jump; linear.
uint32_t thread_decided_tests(Function& f, const lowir::Program& program)
{
	uint32_t threaded = 0;
	const uint32_t blocks = static_cast<uint32_t>(f.blocks.size());
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const Block block = f.blocks[b];
		if (block.count == 0)
			continue;
		Inst& jump = f.insts[block.first + block.count - 1];
		for (unsigned step = 0; step < 4; ++step)
		{
			if (jump.op != Op::Jmp || jump.ops[0].kind != OperandKind::Label)
				break;
			const uint32_t t = jump.ops[0].symbol;
			if (t == b || t >= blocks || f.blocks[t].count != 3 || f.blocks[t].landing)
				break;
			const Inst* test = &f.insts[f.blocks[t].first];
			const Inst& jcc = test[1];
			const Inst& jmp = test[2];
			const bool is_test = test->op == Op::Test;
			if ((test->op != Op::Cmp && !is_test) || test->ops[0].kind != OperandKind::Reg || test->ops[0].reg >= 16 ||
				jcc.op != Op::Jcc || jcc.ops[0].kind != OperandKind::Label || jmp.op != Op::Jmp ||
				jmp.ops[0].kind != OperandKind::Label)
				break;
			const Reg r = test->ops[0].reg;
			const Operand& other = test->ops[1];
			if (!(other.kind == OperandKind::Imm || (is_test && other.kind == OperandKind::Reg && other.reg == r)))
				break;
			uint64_t value = 0;
			bool nonzero = false;
			if (!left_value(f, program, block, r, value, nonzero))
				break;
			const unsigned bytes = register_bytes(test->type);
			const Cond cond = static_cast<Cond>(jcc.sub);
			int holds;
			if (!nonzero)
				holds = decided_condition(
					cond, is_test, value, other.kind == OperandKind::Imm ? other.imm : value, bytes);
			else if (bytes != 8 || (other.kind == OperandKind::Imm && other.imm != 0))
				holds = -1;
			else
				holds = cond == CC_NE || cond == CC_A || cond == CC_AE ? 1
					: cond == CC_E || cond == CC_BE || cond == CC_B    ? 0
																	   : -1;
			if (holds < 0)
				break;
			jump.ops[0].symbol = holds ? jcc.ops[0].symbol : jmp.ops[0].symbol;
			++threaded;
		}
	}
	return threaded;
}

} // namespace native
