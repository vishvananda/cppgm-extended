#include <algorithm>
#include <vector>

#include "native/select/block_placement.h"
#include "native/select/mir_facts.h"

// Block placement (O1+, before branch layout): the selector emits blocks
// in LowIR layout order, so a likely path often jumps over an unlikely arm
// laid out on its way (an if's then-block before the else, a slow path
// inside a loop). The blocks are ordered by a walk along the likeliest
// edges (Placement::walk). A two-way branch's static taken-probability
// comes from its target only leading to a noreturn end or a call of a cold
// function (cold), from loop back edges and loop exits, from a NaN being
// rare, from a single-bit test being zero, and from an equality test being
// the unlikely outcome (one with a nonzero constant more so, and in a loop,
// one of a register the block steps by a literal as rare as an exit); an
// edge weighs that times its source's frequency (the entry's spread along
// forward edges by those probabilities, 8 times a loop header's inflow; a
// cycle entered twice is weighed so only where blocks are ordered, see
// find_loops). Branch layout (branch_layout.cpp) then drops the jumps to
// the next block. Every block must end in a jump, return or other
// terminator (none falls through yet); a function where one does not keeps
// its order. Blocks keep their instructions (a jump to a short test may
// take a copy of it): only their order and the branch labels change.
// O(blocks + edges) plus the loops' bodies.
namespace native {

namespace {

const uint32_t kNone = 0xffffffffu;
// Taken-probabilities (per mille) of a two-way branch's conditional target.
const uint32_t kColdTarget = 20;
const uint32_t kBackEdge = 880;
const uint32_t kLoopExit = 120;
// A test before a loop (a rotated loop's first test) enters it this often.
const uint32_t kLoopEntered = 700;
const uint32_t kEqualTaken = 450;
// A successor making a call (the slower side) is taken this often.
const uint32_t kCallTaken = 300;
// A successor that returns at once (an early exit) is taken this often.
const uint32_t kReturnTaken = 350;
// A signed value tested against zero is negative this often.
const uint32_t kNegativeTaken = 300;
// A value compared with zero straight from memory is zero this often, a
// byte in a register this often.
const uint32_t kMemoryZeroTaken = 700;
const uint32_t kByteZeroTaken = 600;
// A floating compare is unordered (an operand is a NaN) this often.
const uint32_t kNanTaken = 30;
// A test of one bit (a flag in a word) finds it clear this often.
const uint32_t kBitClearTaken = 850;
// A value compared with a nonzero constant (a kind, an opcode, a character)
// equals it this often.
const uint32_t kConstantEqualTaken = 300;
// In a loop, a counter or pointer stepped by a literal reaches the bound it
// is tested against (a segment's end, a ring's wrap, a count's limit) as
// rarely as a loop exits.
const uint32_t kStepEqualTaken = kLoopExit;
// A range test (a value less a constant, compared unsigned) covers a few
// adjacent constants (a switch's range case) up to this many values.
const int64_t kRangeValues = 8;
const uint64_t kLevelFrequency = 8;

bool terminator(const Inst& inst)
{
	switch (inst.op)
	{
	case Op::Jmp:
	case Op::Ret:
	case Op::FRet:
	case Op::Trap:
	case Op::Throw:
	case Op::Resume:
	case Op::JumpTable:
	case Op::IndirectJump:
	case Op::Exit:
	case Op::WideCmpBranch:
		return true;
	case Op::Call:
		return (inst.flags & MF_NORETURN) != 0;
	default:
		return false;
	}
}

bool cold_inst(const Inst& inst)
{
	return inst.op == Op::Trap || inst.op == Op::Throw || inst.op == Op::Resume ||
		(inst.op == Op::Call && (inst.flags & (MF_NORETURN | MF_COLD)) != 0);
}

class Placement
{
public:
	explicit Placement(Function& f) : f_(f), n_(static_cast<uint32_t>(f.blocks.size())) {}
	uint32_t walk();
	uint32_t cold_blocks();
	bool frequencies(std::vector<uint64_t>& frequency, bool placing);
	void spread(std::vector<uint64_t>& frequency, bool placing) const;
	bool entered_twice() const { return twice_; }
	void odds(std::vector<uint16_t>& taken_odds) const;

private:
	bool successors();
	void find_cold();
	void find_loops();
	uint32_t taken(uint32_t b) const;
	uint32_t test_taken(const Block& block, uint32_t at, bool looped) const;
	void split(uint32_t b, std::vector<uint32_t>& shares) const;
	bool steps(const Block& block, uint32_t at, Reg r) const;
	bool copyable_test(uint32_t b) const;

	Function& f_;
	uint32_t n_;
	std::vector<uint32_t> succ_first_, succs_; // CSR, in instruction order
	std::vector<uint32_t> pred_first_;         // predecessor counts' prefix sums
	std::vector<uint8_t> two_way_;             // ends `jcc T; jmp F`: successors T, F
	std::vector<uint8_t> chain_;               // a compare chain: three or more branches, all `jcc` but a final `jmp`
	std::vector<uint8_t> back_;                // per successor slot: a back edge
	std::vector<uint8_t> cold_;
	std::vector<uint32_t> depth_;              // loop nesting
	std::vector<uint32_t> inner_;              // innermost loop (loops_ index) or kNone
	std::vector<std::vector<uint32_t>> loops_; // members, ascending
	std::vector<uint32_t> finished_;           // reachable blocks in postorder
	std::vector<uint8_t> header_;              // a loop's header: 1, or 2 for a cycle entered twice
	std::vector<uint32_t> headed_;             // a header's loop (loops_ index) or kNone
	bool twice_ = false;                       // some cycle is entered twice
};

bool Placement::successors()
{
	succ_first_.assign(n_ + 1, 0);
	succs_.clear();
	two_way_.assign(n_, 0);
	chain_.assign(n_, 0);
	for (uint32_t b = 0; b < n_; ++b)
	{
		const Block& block = f_.blocks[b];
		if (block.count == 0 || !terminator(f_.insts[block.first + block.count - 1]))
			return false;
		succ_first_[b] = static_cast<uint32_t>(succs_.size());
		uint32_t branches = 0, conditional = 0;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = f_.insts[i];
			if (inst.op == Op::Jcc || inst.op == Op::Jmp)
			{
				++branches;
				conditional += inst.op == Op::Jcc ? 1 : 0;
				if (inst.ops[0].kind == OperandKind::Label)
					succs_.push_back(inst.ops[0].symbol);
			}
			else if (inst.op == Op::WideCmpBranch)
			{
				++branches;
				succs_.push_back(inst.ops[2].symbol);
				succs_.push_back(inst.aux);
			}
			else if (inst.op == Op::JumpTable || inst.op == Op::IndirectJump)
			{
				++branches;
				for (uint32_t k = inst.aux; k < inst.aux + inst.aux2; ++k)
					succs_.push_back(f_.jump_targets[k]);
			}
		}
		const uint32_t count = static_cast<uint32_t>(succs_.size()) - succ_first_[b];
		two_way_[b] = branches == 2 && count == 2 && block.count >= 2 &&
			f_.insts[block.first + block.count - 1].op == Op::Jmp &&
			f_.insts[block.first + block.count - 2].op == Op::Jcc &&
			succs_[succ_first_[b]] != succs_[succ_first_[b] + 1];
		chain_[b] = branches >= 3 && count == branches && conditional + 1 == branches &&
			f_.insts[block.first + block.count - 1].op == Op::Jmp;
	}
	succ_first_[n_] = static_cast<uint32_t>(succs_.size());
	for (uint32_t s : succs_)
		if (s >= n_)
			return false;
	return true;
}

// Cold: a block holding a noreturn call, a call of a cold function, a
// trap, throw or resume, a host landing pad, or one whose every successor
// is cold.
void Placement::find_cold()
{
	cold_.assign(n_, 0);
	for (uint32_t b = 0; b < n_; ++b)
	{
		const Block& block = f_.blocks[b];
		cold_[b] = block.landing;
		for (uint32_t i = block.first; i < block.first + block.count && !cold_[b]; ++i)
			cold_[b] = cold_inst(f_.insts[i]);
	}
	for (bool changed = true; changed;)
	{
		changed = false;
		for (uint32_t b = n_; b-- > 0;)
		{
			if (cold_[b] || succ_first_[b] == succ_first_[b + 1])
				continue;
			bool all = true;
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1] && all; ++k)
				all = cold_[succs_[k]] != 0;
			if (all)
			{
				cold_[b] = 1;
				changed = true;
			}
		}
	}
}

// Back edges (to a block on the depth-first walk's stack from the entry)
// and the natural loop of each header (the blocks reaching a back edge's
// source without passing the header).
void Placement::find_loops()
{
	back_.assign(succs_.size(), 0);
	finished_.clear();
	std::vector<uint8_t> state(n_, 0); // 1 on the stack, 2 done
	std::vector<std::pair<uint32_t, uint32_t>> stack;
	stack.push_back(std::make_pair(0u, succ_first_[0]));
	state[0] = 1;
	while (!stack.empty())
	{
		std::pair<uint32_t, uint32_t>& top = stack.back();
		if (top.second == succ_first_[top.first + 1])
		{
			state[top.first] = 2;
			finished_.push_back(top.first);
			stack.pop_back();
			continue;
		}
		const uint32_t k = top.second++;
		const uint32_t t = succs_[k];
		if (state[t] == 1)
			back_[k] = 1;
		else if (state[t] == 0)
		{
			state[t] = 1;
			stack.push_back(std::make_pair(t, succ_first_[t]));
		}
	}
	std::vector<uint32_t>& pred_first = pred_first_;
	pred_first.assign(n_ + 1, 0);
	std::vector<uint32_t> preds(succs_.size());
	for (uint32_t s : succs_)
		++pred_first[s + 1];
	for (uint32_t b = 0; b < n_; ++b)
		pred_first[b + 1] += pred_first[b];
	std::vector<uint32_t> fill(pred_first.begin(), pred_first.end() - 1);
	for (uint32_t b = 0; b < n_; ++b)
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			preds[fill[succs_[k]]++] = b;
	depth_.assign(n_, 0);
	inner_.assign(n_, kNone);
	header_.assign(n_, 0);
	twice_ = false;
	loops_.clear();
	std::vector<uint32_t>& header_loop = headed_;
	header_loop.assign(n_, kNone);
	std::vector<uint32_t> mark(n_, kNone), work;
	for (uint32_t b = 0; b < n_; ++b)
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			if (!back_[k])
				continue;
			const uint32_t h = succs_[k];
			if (header_loop[h] == kNone)
			{
				header_loop[h] = static_cast<uint32_t>(loops_.size());
				loops_.push_back(std::vector<uint32_t>(1, h));
				header_[h] = 1;
			}
			// The header stops the walk (another loop's walk may have
			// marked it since).
			const uint32_t loop = header_loop[h];
			mark[h] = loop;
			if (mark[b] != loop)
			{
				mark[b] = loop;
				loops_[loop].push_back(b);
				work.push_back(b);
			}
			while (!work.empty())
			{
				const uint32_t x = work.back();
				work.pop_back();
				for (uint32_t p = pred_first[x]; p < pred_first[x + 1]; ++p)
				{
					const uint32_t y = preds[p];
					if (state[y] != 0 && mark[y] != loop)
					{
						mark[y] = loop;
						loops_[loop].push_back(y);
						work.push_back(y);
					}
				}
			}
		}
	for (uint32_t l = 0; l < loops_.size(); ++l)
	{
		std::vector<uint32_t>& members = loops_[l];
		const uint32_t header = members.front();
		std::sort(members.begin(), members.end());
		members.erase(std::unique(members.begin(), members.end()), members.end());
		// A walk reaching the entry found a header not dominating its
		// latch (a loop entered in two places, as jump threading leaves
		// some): its members are no loop, and nothing exits it. Blocks are
		// still ordered as for a loop (its header's inflow is multiplied):
		// the walk's edges and a branch's two targets weigh alike. But with
		// no exit every block after it would weigh as much, so where a
		// block is weighed against the entry (copied tails, the frame's
		// order) the header is none, and padding reads both estimates.
		if (members.front() == 0)
		{
			header_[header] = 2;
			twice_ = true;
			// Its members become the cycle itself: the blocks the header
			// reaches among those that reach its latches (a branch is
			// weighed by whether it stays in the cycle, see taken).
			// (`mark` holds a member's own number, then the number of
			// those reached: both apart from the loop numbers above.)
			const uint32_t member = kNone - 1 - 2 * l, reached = member - 1;
			for (uint32_t b : members)
				mark[b] = member;
			std::vector<uint32_t> cycle(1, header);
			mark[header] = reached;
			for (size_t i = 0; i < cycle.size(); ++i)
				for (uint32_t k = succ_first_[cycle[i]]; k < succ_first_[cycle[i] + 1]; ++k)
				{
					const uint32_t s = succs_[k];
					if (mark[s] == member)
					{
						mark[s] = reached;
						cycle.push_back(s);
					}
				}
			std::sort(cycle.begin(), cycle.end());
			members.swap(cycle);
			continue;
		}
		for (uint32_t b : members)
		{
			++depth_[b];
			if (inner_[b] == kNone || loops_[inner_[b]].size() > members.size())
				inner_[b] = l;
		}
	}
}

// Whether the last write of `r` before instruction `at` in `block` steps it
// by a literal (`add r, k`, `sub r, k`, `lea r, [r + k]`), with only simple
// instructions (no implicit register) after it.
bool Placement::steps(const Block& block, uint32_t at, Reg r) const
{
	for (uint32_t i = at; i-- > block.first;)
	{
		const Inst& inst = f_.insts[i];
		if (!simple_inst(inst))
			return false;
		const bool writes = inst.op != Op::Cmp && inst.op != Op::Test && inst.op != Op::Bt && inst.op != Op::Store &&
			inst.op != Op::Nop;
		if (!writes || !inst.ops[0].is_reg() || inst.ops[0].reg != r)
			continue;
		if (inst.op == Op::Add || inst.op == Op::Sub)
			return inst.ops[1].kind == OperandKind::Imm && inst.ops[1].imm != 0;
		const Operand& address = inst.ops[1];
		return inst.op == Op::Lea && address.kind == OperandKind::Mem && address.reg == r && address.index == kNoReg &&
			address.symbol == lowir::kNoId && address.disp != 0;
	}
	return false;
}

uint32_t Placement::taken(uint32_t b) const
{
	const uint32_t k = succ_first_[b];
	const uint32_t t = succs_[k], f = succs_[k + 1];
	if (cold_[t] != cold_[f])
		return cold_[t] ? kColdTarget : 1000 - kColdTarget;
	const Block& hinted = f_.blocks[b];
	const uint32_t probability = f_.insts[hinted.first + hinted.count - 2].aux;
	if (probability > 0 && probability < 1000)
		return probability;
	// A branch that closes a loop and otherwise leaves it. (One that goes on
	// inside the loop either way, a `continue`, is weighed by its test.)
	if (back_[k] != back_[k + 1])
	{
		const uint32_t header = back_[k] ? t : f, other = back_[k] ? f : t;
		const std::vector<uint32_t>& loop = loops_[headed_[header]];
		if (!std::binary_search(loop.begin(), loop.end(), other))
			return back_[k] ? kBackEdge : 1000 - kBackEdge;
	}
	if (inner_[b] != kNone)
	{
		const std::vector<uint32_t>& loop = loops_[inner_[b]];
		const bool t_exits = !std::binary_search(loop.begin(), loop.end(), t);
		const bool f_exits = !std::binary_search(loop.begin(), loop.end(), f);
		if (t_exits != f_exits)
			return t_exits ? kLoopExit : 1000 - kLoopExit;
	}
	// A loop's guard: one target is the header of a loop that holds neither
	// this block nor the other target.
	auto enters = [&](uint32_t header, uint32_t other) {
		if (header_[header] != 1 || inner_[header] == kNone)
			return false;
		const std::vector<uint32_t>& loop = loops_[inner_[header]];
		return !std::binary_search(loop.begin(), loop.end(), b) && !std::binary_search(loop.begin(), loop.end(), other);
	};
	const bool t_enters = enters(t, f), f_enters = enters(f, t);
	if (t_enters != f_enters)
		return t_enters ? kLoopEntered : 1000 - kLoopEntered;
	const Block& block = f_.blocks[b];
	const uint32_t at = block.first + block.count - 2;
	const Cond cond = static_cast<Cond>(f_.insts[at].sub);
	// A floating compare's parity branch: an operand is a NaN.
	if ((cond == CC_P || cond == CC_NP) && block.count >= 3 && f_.insts[at - 1].op == Op::FCmp)
		return cond == CC_P ? kNanTaken : 1000 - kNanTaken;
	auto calls = [&](uint32_t s) {
		const Block& sb = f_.blocks[s];
		for (uint32_t i = sb.first; i < sb.first + sb.count; ++i)
			if (f_.insts[i].op == Op::Call)
				return true;
		return false;
	};
	const bool t_calls = calls(t), f_calls = calls(f);
	if (t_calls != f_calls)
	{
		// Guards sharing a calling successor split its odds.
		const uint32_t s = t_calls ? t : f;
		const uint32_t p = kCallTaken / std::max<uint32_t>(1, pred_first_[s + 1] - pred_first_[s]);
		return t_calls ? p : 1000 - p;
	}
	auto returns = [&](uint32_t s) {
		const Block& sb = f_.blocks[s];
		return sb.count != 0 && f_.insts[sb.first + sb.count - 1].op == Op::Ret;
	};
	const bool t_returns = returns(t), f_returns = returns(f);
	if (t_returns != f_returns)
		return t_returns ? kReturnTaken : 1000 - kReturnTaken;
	return test_taken(block, at, inner_[b] != kNone);
}

// How often of 1000 times the conditional branch at `at` is taken, by its
// test alone: the instruction before it, or before the branches ahead of
// it that read the same flags. `looped`: the block is in a loop.
uint32_t Placement::test_taken(const Block& block, uint32_t at, bool looped) const
{
	const Cond cond = static_cast<Cond>(f_.insts[at].sub);
	uint32_t set = at;
	while (set > block.first && f_.insts[set - 1].op == Op::Jcc)
		--set;
	const bool tested = set > block.first;
	if (tested && (cond == CC_P || cond == CC_NP) && f_.insts[set - 1].op == Op::FCmp)
		return cond == CC_P ? kNanTaken : 1000 - kNanTaken;
	// A sign test: `test x, x` or `cmp x, 0` just before a signed less
	// (or sign) branch.
	if ((cond == CC_L || cond == CC_S || cond == CC_GE || cond == CC_NS) && tested)
	{
		const Inst& test = f_.insts[set - 1];
		const bool zero_test = test.ops[0].kind == OperandKind::Reg &&
			((test.op == Op::Test && test.ops[1].kind == OperandKind::Reg && test.ops[1].reg == test.ops[0].reg) ||
				(test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm == 0));
		if (zero_test)
			return cond == CC_L || cond == CC_S ? kNegativeTaken : 1000 - kNegativeTaken;
	}
	// An equality test of a register the block steps by a literal, against
	// another value or by the step's own flags.
	if ((cond == CC_E || cond == CC_NE) && tested && looped)
	{
		const Inst& test = f_.insts[set - 1];
		auto stepped = [&](const Operand& op) { return op.is_reg() && steps(block, set - 1, op.reg); };
		bool step = false;
		if (test.op == Op::Cmp)
			step = stepped(test.ops[0]) || stepped(test.ops[1]);
		else if (test.op == Op::Test)
			step = test.ops[0] == test.ops[1] && stepped(test.ops[0]);
		else if (test.op == Op::Add || test.op == Op::Sub)
			step = test.ops[0].is_reg() && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm != 0;
		if (step)
			return cond == CC_E ? kStepEqualTaken : 1000 - kStepEqualTaken;
	}
	// A value compared with zero straight from memory (a flag, an optional
	// pointer field; not an indexed element, which says nothing) is usually
	// zero, and so is a byte (a Boolean) in a register, a little less often.
	if ((cond == CC_E || cond == CC_NE) && tested)
	{
		const Inst& test = f_.insts[set - 1];
		if (test.op == Op::Cmp && test.ops[0].kind == OperandKind::Mem && test.ops[0].index == kNoReg &&
			test.ops[1].kind == OperandKind::Imm && test.ops[1].imm == 0)
			return cond == CC_E ? kMemoryZeroTaken : 1000 - kMemoryZeroTaken;
		const bool byte = test.type == lowir::TypeKind::U8 || test.type == lowir::TypeKind::I8 ||
			test.type == lowir::TypeKind::I1;
		const bool zero_test = test.ops[0].kind == OperandKind::Reg &&
			((test.op == Op::Test && test.ops[1].kind == OperandKind::Reg && test.ops[1].reg == test.ops[0].reg) ||
				(test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm == 0));
		if (byte && zero_test)
			return cond == CC_E ? kByteZeroTaken : 1000 - kByteZeroTaken;
	}
	// A test of one bit (`test x, 2^k` before an equality branch, `bt x, k`
	// before a carry branch), and a compare with a nonzero constant (`cmp x,
	// k` before an equality branch).
	if (tested)
	{
		const Inst& test = f_.insts[set - 1];
		const bool one_bit = test.ops[1].kind == OperandKind::Imm && test.ops[1].imm > 0 &&
			(test.ops[1].imm & (test.ops[1].imm - 1)) == 0;
		if (test.op == Op::Test && one_bit && (cond == CC_E || cond == CC_NE))
			return cond == CC_E ? kBitClearTaken : 1000 - kBitClearTaken;
		if (test.op == Op::Bt && test.ops[1].kind == OperandKind::Imm && (cond == CC_B || cond == CC_AE))
			return cond == CC_AE ? kBitClearTaken : 1000 - kBitClearTaken;
		if (test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm != 0 &&
			(cond == CC_E || cond == CC_NE))
			return cond == CC_E ? kConstantEqualTaken : 1000 - kConstantEqualTaken;
		// One of a few adjacent constants (a range of a switch's cases):
		// the value less the first of them, compared unsigned with their
		// count, is in the range as often as a value equals one constant.
		const bool below = cond == CC_BE || cond == CC_B;
		if (test.op == Op::Cmp && test.ops[0].is_reg() && test.ops[1].kind == OperandKind::Imm &&
			test.ops[1].imm >= 0 && test.ops[1].imm < kRangeValues && (below || cond == CC_A || cond == CC_AE) &&
			set >= block.first + 2)
		{
			const Inst& base = f_.insts[set - 2];
			if (base.op == Op::Lea && base.ops[0].is_reg() && base.ops[0].reg == test.ops[0].reg &&
				base.ops[1].kind == OperandKind::Mem && base.ops[1].reg < 32 && base.ops[1].index == kNoReg &&
				base.ops[1].disp != 0)
				return below ? kConstantEqualTaken : 1000 - kConstantEqualTaken;
		}
	}
	if (cond == CC_E)
		return kEqualTaken;
	if (cond == CC_NE)
		return 1000 - kEqualTaken;
	return 500;
}

// A block's executions divided among its successor slots, per mille: a
// two-way block's by its branch (taken); a compare chain's (conditional
// branches, each after a test, then a jump: a switch's cases and its
// default) test by test, each branch taking its share of what the ones
// before it left; any other block's equally.
void Placement::split(uint32_t b, std::vector<uint32_t>& shares) const
{
	const uint32_t first = succ_first_[b], count = succ_first_[b + 1] - first;
	shares.assign(count, count != 0 ? 1000 / count : 0);
	if (two_way_[b])
	{
		shares[0] = taken(b);
		shares[1] = 1000 - shares[0];
		return;
	}
	if (!chain_[b])
		return;
	const Block& block = f_.blocks[b];
	const std::vector<uint32_t>* loop = inner_[b] != kNone ? &loops_[inner_[b]] : nullptr;
	uint32_t left = 1000, slot = 0;
	for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
	{
		const Inst& inst = f_.insts[i];
		if (inst.op != Op::Jcc)
			continue;
		const uint32_t s = succs_[first + slot];
		uint32_t p = inst.aux > 0 && inst.aux < 1000 ? inst.aux : test_taken(block, i, loop != nullptr);
		if (cold_[s])
			p = kColdTarget;
		else if (loop != nullptr && !std::binary_search(loop->begin(), loop->end(), s))
			p = std::min(p, kLoopExit);
		shares[slot] = left * p / 1000;
		left -= shares[slot++];
	}
	shares[slot] = left;
}

// Each two-way block's taken-probability (0 for any other block).
void Placement::odds(std::vector<uint16_t>& taken_odds) const
{
	taken_odds.assign(n_, 0);
	for (uint32_t b = 0; b < n_; ++b)
		if (two_way_[b])
			taken_odds[b] = static_cast<uint16_t>(taken(b));
}

// A block a jump may take a copy of: at most kCopyCap instructions,
// simple ones, then `jcc; jmp`; two-way, not a landing, and not in a cold
// function (optimized for size).
const uint32_t kCopyCap = 5;

bool Placement::copyable_test(uint32_t b) const
{
	const Block& block = f_.blocks[b];
	if (!two_way_[b] || block.landing || block.count > kCopyCap || f_.cold)
		return false;
	for (uint32_t i = block.first; i + 2 < block.first + block.count; ++i)
		switch (f_.insts[i].op)
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
		case Op::Movzx:
		case Op::Sext:
		case Op::Zext:
		case Op::Nop:
			break;
		default:
			return false;
		}
	return true;
}

} // namespace

void reorder_blocks(Function& f, const std::vector<uint32_t>& order)
{
	const uint32_t n = static_cast<uint32_t>(f.blocks.size());
	std::vector<uint32_t> index(n);
	for (uint32_t k = 0; k < n; ++k)
		index[order[k]] = k;
	std::vector<Block> blocks(n);
	for (uint32_t k = 0; k < n; ++k)
		blocks[k] = f.blocks[order[k]];
	f.blocks.swap(blocks);
	for (const Block& block : f.blocks)
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			Inst& inst = f.insts[i];
			for (unsigned k = 0; k < inst.nops; ++k)
				if (inst.ops[k].kind == OperandKind::Label)
					inst.ops[k].symbol = index[inst.ops[k].symbol];
			if (inst.op == Op::WideCmpBranch)
				inst.aux = index[inst.aux];
		}
	for (uint32_t& target : f.jump_targets)
		target = index[target];
	for (lowir::CodeLabel& label : f.code_labels)
		label.block = index[label.block];
	for (EhLanding& landing : f.eh.landings)
	{
		if (landing.pad != lowir::kNoId)
			landing.pad = index[landing.pad];
		if (landing.select != lowir::kNoId)
			landing.select = index[landing.select];
	}
}

namespace {

// Frequencies (fixed point) in reverse postorder: a block's inflow over
// its forward edges, a loop header's times kLevelFrequency (a cycle entered
// twice's only when `placing`). False when the blocks do not all end in
// branches.
bool Placement::frequencies(std::vector<uint64_t>& frequency, bool placing)
{
	if (n_ == 0 || !successors())
		return false;
	find_cold();
	find_loops();
	spread(frequency, placing);
	return true;
}

void Placement::spread(std::vector<uint64_t>& frequency, bool placing) const
{
	frequency.assign(n_, 0);
	std::vector<uint64_t> inflow(n_, 0);
	std::vector<uint32_t> shares;
	inflow[0] = static_cast<uint64_t>(1) << 20;
	for (uint32_t r = static_cast<uint32_t>(finished_.size()); r-- > 0;)
	{
		const uint32_t b = finished_[r];
		frequency[b] = header_[b] == 1 || (header_[b] == 2 && placing) ? inflow[b] * kLevelFrequency : inflow[b];
		if (frequency[b] > (static_cast<uint64_t>(1) << 50))
			frequency[b] = static_cast<uint64_t>(1) << 50;
		split(b, shares);
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			if (!back_[k])
				inflow[succs_[k]] += frequency[b] * shares[k - succ_first_[b]] / 1000;
	}
}

// A region is cold only when every incoming edge is explicitly unlikely
// or comes from another such region. This avoids frequency extrapolation
// through irreducible indirect-dispatch graphs. Preserve all hot ordering.
uint32_t Placement::cold_blocks()
{
	if (n_ < 3 || !successors())
		return 0;
	std::vector<uint32_t> incoming(n_, 0), rare(n_, 0);
	std::vector<uint8_t> protected_entry(n_, 0), rare_edge(succs_.size(), 0), cold(n_, 0);
	protected_entry[0] = protected_entry[n_ - 1] = 1;
	for (const lowir::CodeLabel& label : f_.code_labels)
		protected_entry[label.block] = 1;
	for (const EhLanding& landing : f_.eh.landings)
	{
		if (landing.pad != lowir::kNoId)
			protected_entry[landing.pad] = 1;
		if (landing.select != lowir::kNoId)
			protected_entry[landing.select] = 1;
	}
	for (uint32_t b = 0; b < n_; ++b)
	{
		protected_entry[b] |= f_.blocks[b].landing;
		const Block& block = f_.blocks[b];
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
			if (f_.insts[i].op == Op::EhPush)
				for (unsigned k = 0; k < f_.insts[i].nops; ++k)
					if (f_.insts[i].ops[k].kind == OperandKind::Label)
						protected_entry[f_.insts[i].ops[k].symbol] = 1;
		if (two_way_[b])
		{
			const uint32_t p = f_.insts[block.first + block.count - 2].aux;
			if (p > 0 && p != 500 && p < 1000)
				rare_edge[succ_first_[b] + (p < 500 ? 0 : 1)] = 1;
		}
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			++incoming[succs_[k]];
			rare[succs_[k]] += rare_edge[k];
		}
	}
	std::vector<uint32_t> work;
	auto visit = [&](uint32_t b) {
		if (!protected_entry[b] && !cold[b] && incoming[b] != 0 && rare[b] == incoming[b])
		{
			cold[b] = 1;
			work.push_back(b);
		}
	};
	for (uint32_t b = 1; b < n_; ++b)
		visit(b);
	for (size_t i = 0; i < work.size(); ++i)
		for (uint32_t k = succ_first_[work[i]]; k < succ_first_[work[i] + 1]; ++k)
			if (!rare_edge[k])
			{
				++rare[succs_[k]];
				visit(succs_[k]);
			}
	if (work.empty())
		return 0;
	std::vector<uint32_t> order;
	order.reserve(n_);
	for (uint32_t b = 0; b + 1 < n_; ++b)
		if (!cold[b])
			order.push_back(b);
	for (uint32_t b = 1; b + 1 < n_; ++b)
		if (cold[b])
			order.push_back(b);
	order.push_back(n_ - 1);
	uint32_t moved = 0;
	for (uint32_t b = 0; b < n_; ++b)
		moved += order[b] != b;
	if (moved != 0)
		reorder_blocks(f_, order);
	return moved;
}

// The order by a walk from the entry (as LLVM's MachineBlockPlacement
// without a profile): a block is followed by its likeliest successor still
// to place, unless another block still to place enters that successor at
// least 1/kConflictRatio as often; then the successor waits for it, so an
// if's arm stays between the test and the join and the branches around it
// are short. A jump to a short test placed already takes a copy of it and
// goes on with one of its successors. With no successor to follow, the
// block that most recently had all its forward predecessors placed comes
// next (the code nearest the path just left), else the first block left.
// Cold blocks come after the others, placed the same way; unreachable ones
// last. O(blocks + edges); a successor's predecessors are read up to
// kConflictScan.
const uint32_t kConflictScan = 64;
const uint64_t kConflictRatio = 2;

uint32_t Placement::walk()
{
	f_.placement_valid = false;
	std::vector<uint64_t> frequency, unplaced;
	if (n_ < 3 || !frequencies(frequency, true))
		return 0;
	// Padding also reads the estimate with no multiple for a cycle entered
	// twice (see the end).
	if (twice_)
		spread(unplaced, false);
	const uint32_t slots = static_cast<uint32_t>(succs_.size());
	std::vector<uint64_t> weight(slots, 0), unplaced_weight;
	if (twice_)
		unplaced_weight.assign(slots, 0);
	std::vector<uint32_t> from(slots, 0), pred_slot(slots, 0), waiting(n_, 0);
	std::vector<uint8_t> reached(n_, 0);
	for (uint32_t b : finished_)
		reached[b] = 1;
	{
		std::vector<uint32_t> fill(pred_first_.begin(), pred_first_.end() - 1), shares;
		for (uint32_t b = 0; b < n_; ++b)
		{
			split(b, shares);
			for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
			{
				const uint64_t share = shares[k - succ_first_[b]];
				weight[k] = frequency[b] * share / 1000;
				if (twice_)
					unplaced_weight[k] = unplaced[b] * share / 1000;
				from[k] = b;
				pred_slot[fill[succs_[k]]++] = k;
				if (reached[b] && !back_[k] && succs_[k] != b)
					++waiting[succs_[k]];
			}
		}
	}
	// 0: reached, 1: reached and cold, 2: not reached.
	auto rank = [&](uint32_t b) { return !reached[b] ? 2u : cold_[b] ? 1u : 0u; };
	std::vector<uint32_t> ready[2]; // per rank: blocks whose forward predecessors are placed, the latest last
	std::vector<uint8_t> placed(n_, 0);
	std::vector<uint32_t> order, starts; // starts: where a block follows no predecessor
	order.reserve(n_);
	uint32_t scan[3] = { 1, 1, 1 };
	auto conflict = [&](uint32_t b, uint32_t s, uint64_t w) {
		const uint32_t end = std::min(pred_first_[s + 1], pred_first_[s] + kConflictScan);
		for (uint32_t i = pred_first_[s]; i < end; ++i)
		{
			const uint32_t k = pred_slot[i], q = from[k];
			if (q != b && q != s && !placed[q] && !back_[k] && rank(q) == rank(b) && weight[k] * kConflictRatio >= w)
				return true;
		}
		return false;
	};
	// The successor to follow `b`: the heaviest edge's target that is free.
	// (After a compare chain only its last test's two targets: the block
	// runs into no other, wherever it lies.)
	auto follow = [&](uint32_t b) {
		uint32_t best = kNone;
		uint64_t best_weight = 0;
		for (uint32_t k = chain_[b] ? succ_first_[b + 1] - 2 : succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const uint32_t s = succs_[k];
			if (s == 0 || placed[s] || rank(s) != rank(b))
				continue;
			// (Of equal edges the branch's own target: the block after `b` is
			// often an edge's own, whatever it leads to.)
			if (best != kNone && weight[k] <= best_weight)
				continue;
			if (conflict(b, s, weight[k]))
				continue;
			best = s;
			best_weight = weight[k];
		}
		return best;
	};
	uint32_t b = 0;
	starts.push_back(0);
	for (;;)
	{
		placed[b] = 1;
		order.push_back(b);
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const uint32_t s = succs_[k];
			if (reached[b] && !back_[k] && s != b && --waiting[s] == 0 && !placed[s])
				ready[rank(s)].push_back(s);
		}
		uint32_t next = follow(b);
		// A block ending in a jump to a short test placed already takes a
		// copy of the test and continues into the test's likelier successor
		// that is free: the path runs the same instructions without the
		// taken jump (bb-reorder's duplication). So does one whose test
		// waits for another block and branches back to placed code (a loop's
		// bottom test that two arms reach): the test follows that block, and
		// this one's path would enter it by a taken jump on every pass. The
		// walk then goes on from the ready blocks.
		if (next == kNone && rank(b) == 0 && succ_first_[b + 1] - succ_first_[b] == 1)
		{
			const Block block = f_.blocks[b];
			const uint32_t t = succs_[succ_first_[b]];
			if (f_.insts[block.first + block.count - 1].op == Op::Jmp && t != b && t != 0 &&
				(placed[t] || rank(t) == 0) && copyable_test(t))
			{
				const uint32_t p = taken(t);
				const uint32_t first = succs_[succ_first_[t]], second = succs_[succ_first_[t] + 1];
				const uint32_t order2[2] = { p >= 500 ? first : second, p >= 500 ? second : first };
				for (uint32_t k = 0; k < 2 && next == kNone && placed[t]; ++k)
					if (order2[k] != 0 && !placed[order2[k]] && rank(order2[k]) == 0)
						next = order2[k];
				const bool waiting_latch = !placed[t] && (placed[first] || placed[second]);
				if (next != kNone || waiting_latch)
				{
					const Block test = f_.blocks[t];
					const uint32_t at = static_cast<uint32_t>(f_.insts.size());
					for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
						f_.insts.push_back(f_.insts[i]);
					for (uint32_t i = test.first; i < test.first + test.count; ++i)
						f_.insts.push_back(f_.insts[i]);
					f_.blocks[b].first = at;
					f_.blocks[b].count = static_cast<uint32_t>(f_.insts.size()) - at;
				}
			}
		}
		if (next == kNone)
		{
			// Ready blocks of the walk's rank, else the first one left of it,
			// then the next rank's.
			for (uint32_t r = 0; r < 3 && next == kNone; ++r)
			{
				while (r < 2 && !ready[r].empty() && placed[ready[r].back()])
					ready[r].pop_back();
				if (r < 2 && !ready[r].empty())
				{
					next = ready[r].back();
					ready[r].pop_back();
					break;
				}
				for (uint32_t& c = scan[r]; c < n_ && next == kNone; ++c)
					if (!placed[c] && rank(c) == r)
						next = c;
			}
			if (next == kNone)
				break;
			starts.push_back(static_cast<uint32_t>(order.size()));
		}
		b = next;
	}
	// A run ending in a return goes last when the last block ended in one:
	// the shared epilogue follows the last block (any other return has its
	// own copy).
	auto returns = [&](uint32_t x) {
		const Block& block = f_.blocks[x];
		return f_.insts[block.first + block.count - 1].op == Op::Ret;
	};
	if (returns(n_ - 1) && !returns(order.back()))
	{
		for (size_t k = starts.size(); k-- > 1;)
		{
			const uint32_t end = k + 1 < starts.size() ? starts[k + 1] : n_;
			if (returns(order[end - 1]))
			{
				std::rotate(order.begin() + starts[k], order.begin() + end, order.end());
				break;
			}
		}
	}
	for (uint32_t k = 1; k < n_; ++k)
	{
		const uint32_t src = order[k - 1], b = order[k];
		uint64_t flow = 0, unplaced_flow = 0;
		for (uint32_t e = succ_first_[src]; e < succ_first_[src + 1]; ++e)
			if (succs_[e] == b)
			{
				flow += weight[e];
				unplaced_flow += twice_ ? unplaced_weight[e] : 0;
			}
		// (A fifth of the block's entries: padding before it would run that
		// often for the entries from elsewhere it helps. Where a cycle is
		// entered twice, by either estimate: with the multiple, the blocks
		// after the cycle weigh too much, and without it, its own blocks no
		// more than the paths that enter them.)
		f_.blocks[b].fallen_into = flow >= frequency[b] / 5 || (twice_ && unplaced_flow >= unplaced[b] / 5);
	}
	f_.placement_valid = true;
	uint32_t moved = 0;
	for (uint32_t k = 0; k < n_; ++k)
		moved += order[k] != k ? 1 : 0;
	if (moved != 0)
		reorder_blocks(f_, order);
	return moved;
}

} // namespace

uint32_t place_cold_blocks(Function& function)
{
	return Placement(function).cold_blocks();
}

uint32_t place_blocks(Function& function)
{
	return Placement(function).walk();
}

void block_frequencies(Function& function, std::vector<uint64_t>& frequency, std::vector<uint64_t>* placing,
	std::vector<uint16_t>* taken_odds)
{
	Placement placement(function);
	if (!placement.frequencies(frequency, false))
	{
		frequency.clear();
		return;
	}
	if (placing != nullptr && placement.entered_twice())
		placement.spread(*placing, true);
	if (taken_odds != nullptr)
		placement.odds(*taken_odds);
}

} // namespace native
