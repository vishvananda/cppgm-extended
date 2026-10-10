#include <algorithm>
#include <vector>

#include "native/select/block_placement.h"

// Block placement (O1+, before branch layout): the selector emits blocks
// in LowIR layout order, so a likely path often jumps over an unlikely arm
// laid out on its way (an if's then-block before the else, a slow path
// inside a loop). The blocks are chained along their edges, heaviest
// first: an edge links a block to a successor when the block ends its
// chain and the successor starts its own. A two-way branch's static
// taken-probability comes from its target only leading to a noreturn end
// (cold), from loop back edges and loop exits, and from an equality test
// being the unlikely outcome; an edge weighs that times its source's
// frequency (the entry's spread along forward edges by those
// probabilities, 8 times a loop header's inflow). The entry's chain comes first,
// then the other chains in their first block's order, cold chains last.
// Branch layout (branch_layout.cpp) then drops the jumps to the next
// block. Every block must end in a jump, return or other terminator (none
// falls through yet); a function where one does not keeps its order.
// Blocks keep their instructions: only their order and the branch labels
// change. O(edges log edges) plus the loops' bodies.
namespace native {

namespace {

const uint32_t kNone = 0xffffffffu;
// Taken-probabilities (per mille) of a two-way branch's conditional target.
const uint32_t kColdTarget = 20;
const uint32_t kBackEdge = 880;
const uint32_t kLoopExit = 120;
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
		(inst.op == Op::Call && (inst.flags & MF_NORETURN) != 0);
}

struct Edge
{
	uint64_t weight;
	uint32_t from, to;
	bool next; // to the block after `from` in the current order
};

class Placement
{
public:
	explicit Placement(Function& f) : f_(f), n_(static_cast<uint32_t>(f.blocks.size())) {}
	uint32_t run();
	bool frequencies(std::vector<uint64_t>& frequency);

private:
	bool successors();
	void find_cold();
	void find_loops();
	uint32_t taken(uint32_t b) const;
	uint32_t root(uint32_t b);
	bool copyable_test(uint32_t b) const;

	Function& f_;
	uint32_t n_;
	std::vector<uint32_t> succ_first_, succs_; // CSR, in instruction order
	std::vector<uint32_t> pred_first_;         // predecessor counts' prefix sums
	std::vector<uint8_t> two_way_;             // ends `jcc T; jmp F`: successors T, F
	std::vector<uint8_t> back_;                // per successor slot: a back edge
	std::vector<uint8_t> cold_;
	std::vector<uint32_t> depth_;              // loop nesting
	std::vector<uint32_t> inner_;              // innermost loop (loops_ index) or kNone
	std::vector<std::vector<uint32_t>> loops_; // members, ascending
	std::vector<uint32_t> finished_;           // reachable blocks in postorder
	std::vector<uint8_t> header_;              // a loop's header
	std::vector<uint32_t> parent_;             // chains: union-find over blocks
};

bool Placement::successors()
{
	succ_first_.assign(n_ + 1, 0);
	succs_.clear();
	two_way_.assign(n_, 0);
	for (uint32_t b = 0; b < n_; ++b)
	{
		const Block& block = f_.blocks[b];
		if (block.count == 0 || !terminator(f_.insts[block.first + block.count - 1]))
			return false;
		succ_first_[b] = static_cast<uint32_t>(succs_.size());
		uint32_t branches = 0;
		for (uint32_t i = block.first; i < block.first + block.count; ++i)
		{
			const Inst& inst = f_.insts[i];
			if (inst.op == Op::Jcc || inst.op == Op::Jmp)
			{
				++branches;
				if (inst.ops[0].kind == OperandKind::Label)
					succs_.push_back(inst.ops[0].symbol);
			}
			else if (inst.op == Op::WideCmpBranch)
			{
				++branches;
				succs_.push_back(inst.ops[2].symbol);
				succs_.push_back(inst.aux);
			}
			else if (inst.op == Op::JumpTable)
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
	}
	succ_first_[n_] = static_cast<uint32_t>(succs_.size());
	for (uint32_t s : succs_)
		if (s >= n_)
			return false;
	return true;
}

// Cold: a block holding a noreturn call, trap, throw or resume, a host
// landing pad, or one whose every successor is cold.
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
	loops_.clear();
	std::vector<uint32_t> header_loop(n_, kNone), mark(n_, kNone), work;
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
		std::sort(members.begin(), members.end());
		members.erase(std::unique(members.begin(), members.end()), members.end());
		// A walk reaching the entry found a header not dominating its
		// latch (a loop entered in two places, as jump threading leaves
		// some): its members are no loop, and nothing exits it.
		if (members.front() == 0)
			continue;
		for (uint32_t b : members)
		{
			++depth_[b];
			if (inner_[b] == kNone || loops_[inner_[b]].size() > members.size())
				inner_[b] = l;
		}
	}
}

uint32_t Placement::taken(uint32_t b) const
{
	const uint32_t k = succ_first_[b];
	const uint32_t t = succs_[k], f = succs_[k + 1];
	if (cold_[t] != cold_[f])
		return cold_[t] ? kColdTarget : 1000 - kColdTarget;
	if (back_[k] != back_[k + 1])
		return back_[k] ? kBackEdge : 1000 - kBackEdge;
	if (inner_[b] != kNone)
	{
		const std::vector<uint32_t>& loop = loops_[inner_[b]];
		const bool t_exits = !std::binary_search(loop.begin(), loop.end(), t);
		const bool f_exits = !std::binary_search(loop.begin(), loop.end(), f);
		if (t_exits != f_exits)
			return t_exits ? kLoopExit : 1000 - kLoopExit;
	}
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
	const Block& block = f_.blocks[b];
	const Cond cond = static_cast<Cond>(f_.insts[block.first + block.count - 2].sub);
	// A sign test: `test x, x` or `cmp x, 0` just before a signed less
	// (or sign) branch.
	if ((cond == CC_L || cond == CC_S || cond == CC_GE || cond == CC_NS) && block.count >= 3)
	{
		const Inst& test = f_.insts[block.first + block.count - 3];
		const bool zero_test = test.ops[0].kind == OperandKind::Reg &&
			((test.op == Op::Test && test.ops[1].kind == OperandKind::Reg && test.ops[1].reg == test.ops[0].reg) ||
				(test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm == 0));
		if (zero_test)
			return cond == CC_L || cond == CC_S ? kNegativeTaken : 1000 - kNegativeTaken;
	}
	// A value compared with zero straight from memory (a flag, an optional
	// pointer field) is usually zero, and so is a byte (a Boolean) in a
	// register, a little less often.
	if ((cond == CC_E || cond == CC_NE) && block.count >= 3)
	{
		const Inst& test = f_.insts[block.first + block.count - 3];
		if (test.op == Op::Cmp && test.ops[0].kind == OperandKind::Mem && test.ops[1].kind == OperandKind::Imm &&
			test.ops[1].imm == 0)
			return cond == CC_E ? kMemoryZeroTaken : 1000 - kMemoryZeroTaken;
		const bool byte = test.type == lowir::TypeKind::U8 || test.type == lowir::TypeKind::I8 ||
			test.type == lowir::TypeKind::I1;
		const bool zero_test = test.ops[0].kind == OperandKind::Reg &&
			((test.op == Op::Test && test.ops[1].kind == OperandKind::Reg && test.ops[1].reg == test.ops[0].reg) ||
				(test.op == Op::Cmp && test.ops[1].kind == OperandKind::Imm && test.ops[1].imm == 0));
		if (byte && zero_test)
			return cond == CC_E ? kByteZeroTaken : 1000 - kByteZeroTaken;
	}
	if (cond == CC_E)
		return kEqualTaken;
	if (cond == CC_NE)
		return 1000 - kEqualTaken;
	return 500;
}

// A block a jump may take a copy of: at most kCopyCap instructions,
// simple ones, then `jcc; jmp`; two-way, not a landing.
const uint32_t kCopyCap = 5;

bool Placement::copyable_test(uint32_t b) const
{
	const Block& block = f_.blocks[b];
	if (!two_way_[b] || block.landing || block.count > kCopyCap)
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

uint32_t Placement::root(uint32_t b)
{
	while (parent_[b] != b)
		b = parent_[b] = parent_[parent_[b]];
	return b;
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
// its forward edges, a loop header's times kLevelFrequency. False when the
// blocks do not all end in branches.
bool Placement::frequencies(std::vector<uint64_t>& frequency)
{
	if (n_ == 0 || !successors())
		return false;
	find_cold();
	find_loops();
	frequency.assign(n_, 0);
	std::vector<uint64_t> inflow(n_, 0);
	inflow[0] = static_cast<uint64_t>(1) << 20;
	for (uint32_t r = static_cast<uint32_t>(finished_.size()); r-- > 0;)
	{
		const uint32_t b = finished_[r];
		frequency[b] = header_[b] ? inflow[b] * kLevelFrequency : inflow[b];
		if (frequency[b] > (static_cast<uint64_t>(1) << 50))
			frequency[b] = static_cast<uint64_t>(1) << 50;
		const uint32_t count = succ_first_[b + 1] - succ_first_[b];
		const uint32_t p = two_way_[b] ? taken(b) : 0;
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const uint64_t share = two_way_[b] ? (k == succ_first_[b] ? p : 1000 - p) : 1000 / count;
			if (!back_[k])
				inflow[succs_[k]] += frequency[b] * share / 1000;
		}
	}
	return true;
}

uint32_t Placement::run()
{
	std::vector<uint64_t> frequency;
	if (n_ < 3 || !frequencies(frequency))
		return 0;
	std::vector<Edge> edges;
	for (uint32_t b = 0; b < n_; ++b)
	{
		const uint32_t count = succ_first_[b + 1] - succ_first_[b];
		const uint32_t p = two_way_[b] ? taken(b) : 0;
		for (uint32_t k = succ_first_[b]; k < succ_first_[b + 1]; ++k)
		{
			const uint32_t to = succs_[k];
			if (to == b || to == 0)
				continue;
			const uint64_t share = two_way_[b] ? (k == succ_first_[b] ? p : 1000 - p) : 1000 / count;
			edges.push_back(Edge{ frequency[b] * share / 1000, b, to, to == b + 1 });
		}
	}
	std::sort(edges.begin(), edges.end(), [](const Edge& x, const Edge& y) {
		if (x.weight != y.weight)
			return x.weight > y.weight;
		if (x.next != y.next)
			return x.next;
		return x.from != y.from ? x.from < y.from : x.to < y.to;
	});
	parent_.resize(n_);
	std::vector<uint32_t> head(n_), tail(n_), next(n_, kNone);
	for (uint32_t b = 0; b < n_; ++b)
		parent_[b] = head[b] = tail[b] = b;
	for (const Edge& e : edges)
	{
		const uint32_t a = root(e.from), c = root(e.to);
		if (a == c || tail[a] != e.from || head[c] != e.to)
			continue;
		next[e.from] = e.to;
		tail[a] = tail[c];
		parent_[c] = a;
	}
	// A chain ending in a jump to a short test block placed elsewhere
	// takes a copy of the test and continues into the test's likelier
	// successor that still starts a chain: the path runs the same
	// instructions without the taken jump (bb-reorder's duplication).
	for (uint32_t b = 0; b < n_; ++b)
	{
		if (next[b] != kNone || cold_[b] || succ_first_[b + 1] - succ_first_[b] != 1)
			continue;
		const Block block = f_.blocks[b];
		if (f_.insts[block.first + block.count - 1].op != Op::Jmp)
			continue;
		const uint32_t t = succs_[succ_first_[b]];
		if (t == b || t == 0 || !copyable_test(t) || head[root(t)] == t)
			continue;
		const uint32_t p = taken(t);
		const uint32_t first = succs_[succ_first_[t]], second = succs_[succ_first_[t] + 1];
		const uint32_t order2[2] = { p >= 500 ? first : second, p >= 500 ? second : first };
		uint32_t s = kNone;
		for (uint32_t k = 0; k < 2 && s == kNone; ++k)
		{
			const uint32_t c = order2[k];
			if (c != 0 && c != b && head[root(c)] == c && root(c) != root(b) && !cold_[c])
				s = c;
		}
		if (s == kNone)
			continue;
		const Block test = f_.blocks[t];
		const uint32_t at = static_cast<uint32_t>(f_.insts.size());
		for (uint32_t i = block.first; i + 1 < block.first + block.count; ++i)
			f_.insts.push_back(f_.insts[i]);
		for (uint32_t i = test.first; i < test.first + test.count; ++i)
			f_.insts.push_back(f_.insts[i]);
		f_.blocks[b].first = at;
		f_.blocks[b].count = static_cast<uint32_t>(f_.insts.size()) - at;
		const uint32_t a = root(b), c = root(s);
		next[b] = s;
		tail[a] = tail[c];
		parent_[c] = a;
	}
	// The entry's chain, then the others by their first block, cold last.
	std::vector<uint32_t> chains;
	for (uint32_t b = 0; b < n_; ++b)
		if (root(b) == b && head[b] != 0)
			chains.push_back(b);
	std::stable_sort(chains.begin(), chains.end(), [&](uint32_t x, uint32_t y) {
		if (cold_[head[x]] != cold_[head[y]])
			return cold_[head[y]] != 0;
		return head[x] < head[y];
	});
	// A chain ending in a return goes last when the last block ended in
	// one: the shared epilogue follows the last block (any other return
	// has its own copy).
	const Block& last = f_.blocks[n_ - 1];
	if (f_.insts[last.first + last.count - 1].op == Op::Ret && !chains.empty() &&
		f_.insts[f_.blocks[tail[chains.back()]].first + f_.blocks[tail[chains.back()]].count - 1].op != Op::Ret)
	{
		for (size_t k = chains.size(); k-- > 0;)
		{
			const Block& end = f_.blocks[tail[chains[k]]];
			if (f_.insts[end.first + end.count - 1].op == Op::Ret)
			{
				const uint32_t c = chains[k];
				chains.erase(chains.begin() + static_cast<std::ptrdiff_t>(k));
				chains.push_back(c);
				break;
			}
		}
	}
	std::vector<uint32_t> order;
	order.reserve(n_);
	for (uint32_t b = 0; b != kNone; b = next[b])
		order.push_back(b);
	for (uint32_t c : chains)
		for (uint32_t b = head[c]; b != kNone; b = next[b])
			order.push_back(b);
	uint32_t moved = 0;
	for (uint32_t k = 0; k < n_; ++k)
		moved += order[k] != k ? 1 : 0;
	if (moved != 0)
		reorder_blocks(f_, order);
	return moved;
}

} // namespace

uint32_t place_blocks(Function& function)
{
	Placement placement(function);
	return placement.run();
}

void block_frequencies(Function& function, std::vector<uint64_t>& frequency)
{
	Placement placement(function);
	if (!placement.frequencies(frequency))
		frequency.clear();
}

} // namespace native
