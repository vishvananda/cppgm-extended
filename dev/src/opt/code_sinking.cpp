#include "opt/loops.h"
#include "opt/passes.h"

#include <algorithm>
#include <vector>

namespace opt {

using namespace lowir;

namespace {

// The most dominator-tree steps one question may walk (past it, the value stays).
constexpr unsigned kWalk = 128;

// Pure instructions move down to the block nearest their readers that lies
// in no deeper loop, so a path that reads nothing computes nothing and a
// step meets the test of what it computes. Blocks are visited after every
// block they dominate, so a reader's move is known when its operands are
// placed: one walk moves a whole chain.
class CodeSinking
{
public:
	explicit CodeSinking(FunctionContext& context) : c_(context), body_(context.body), cfg_(context.cfg) {}

	bool run();

private:
	struct Move
	{
		InstId inst;
		BlockId to;
		uint32_t position;
	};

	BlockId common(BlockId a, BlockId b) const;
	void read(Value v, BlockId b);
	uint32_t depth(BlockId b) const;
	BlockId place(BlockId from, BlockId lowest) const;
	void settle(BlockId b);
	void unlink_moved();
	void insert(size_t first, size_t last);

	FunctionContext& c_;
	FunctionBody& body_;
	Cfg& cfg_;
	LoopForest forest_;
	std::vector<uint32_t> loop_depth_;
	std::vector<BlockId> lowest_; // per temp: the nearest block every reader lies under
	std::vector<Move> moves_;
	std::vector<uint8_t> moved_;       // per instruction
	std::vector<uint32_t> first_read_; // per temp, in the target at hand
	std::vector<InstId> order_;
};

// (`kNoId` when the walk is too long, which pins the value.)
BlockId CodeSinking::common(BlockId a, BlockId b) const
{
	for (unsigned step = 0; step < kWalk; ++step)
	{
		if (cfg_.dominates(a, b))
			return a;
		if (cfg_.dominates(b, a))
			return b;
		if (cfg_.rpo_index[a] > cfg_.rpo_index[b])
			a = cfg_.idom[a];
		else
			b = cfg_.idom[b];
	}
	return kNoId;
}

void CodeSinking::read(Value v, BlockId b)
{
	if (v.kind != ValueKind::Temp)
		return;
	BlockId& lowest = lowest_[v.index];
	lowest = lowest == kNoId ? b : common(lowest, b);
	if (lowest == kNoId)
		lowest = cfg_.rpo[0];
}

uint32_t CodeSinking::depth(BlockId b) const
{
	return forest_.innermost[b] == kNoId ? 0 : loop_depth_[forest_.innermost[b]];
}

BlockId CodeSinking::place(BlockId from, BlockId lowest) const
{
	if (!cfg_.dominates(from, lowest))
		return from;
	BlockId best = from;
	uint32_t best_depth = depth(from);
	bool found = false;
	unsigned step = 0;
	for (BlockId x = lowest; x != from; x = cfg_.idom[x])
	{
		if (++step > kWalk)
			return from;
		if ((cfg_.flags[x] & BF_HANDLER) != 0 || forest_.dropped[x] != 0)
			continue;
		if (found ? depth(x) < best_depth : depth(x) <= best_depth)
		{
			best = x;
			best_depth = depth(x);
			found = true;
		}
	}
	return best;
}

void CodeSinking::settle(BlockId b)
{
	order_.clear();
	for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		order_.push_back(i);
	for (size_t k = order_.size(); k-- > 0;)
	{
		const Instruction& inst = body_.instructions[order_[k]];
		if (inst.op == Opcode::Phi)
			continue;
		BlockId at = b;
		if (is_pure(inst) && inst.result != kNoId && lowest_[inst.result] != kNoId && lowest_[inst.result] != b)
			at = place(b, lowest_[inst.result]);
		if (at != b)
		{
			moves_.push_back(Move{ order_[k], at, 0 });
			moved_[order_[k]] = 1;
		}
		for_each_operand(body_, inst, [&](const Value& v) { read(v, at); });
	}
}

void CodeSinking::unlink_moved()
{
	for (BlockId b : cfg_.rpo)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const InstId next = body_.instructions[i].next;
			if (moved_[i] != 0)
				unlink_after(body_, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
}

// moves_[first, last) enter one block, each before its first reader there
// (the end, for a phi's read): last-placed readers come first in the list.
void CodeSinking::insert(size_t first, size_t last)
{
	const BlockId to = moves_[first].to;
	order_.clear();
	for (InstId i = body_.blocks[to].first; i != kNoId; i = body_.instructions[i].next)
		order_.push_back(i);
	const uint32_t end = static_cast<uint32_t>(order_.size() - 1);
	for (size_t m = first; m < last; ++m)
		first_read_[body_.instructions[moves_[m].inst].result] = end;
	for (uint32_t k = end + 1; k-- > 0;)
	{
		const Instruction& inst = body_.instructions[order_[k]];
		if (inst.op == Opcode::Phi)
			continue;
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.kind == ValueKind::Temp && first_read_[v.index] != kNoId)
				first_read_[v.index] = k;
		});
	}
	// (A moved reader's own place bounds the values it reads.)
	for (size_t m = first; m < last; ++m)
	{
		const Instruction& inst = body_.instructions[moves_[m].inst];
		const uint32_t position = first_read_[inst.result];
		moves_[m].position = position;
		for_each_operand(body_, inst, [&](const Value& v) {
			if (v.kind == ValueKind::Temp && first_read_[v.index] != kNoId)
				first_read_[v.index] = std::min(first_read_[v.index], position);
		});
	}
	// Each goes right after what stood before its position: the moves are
	// in reverse program order, so the earliest ends up first of those there.
	for (size_t m = first; m < last; ++m)
	{
		const uint32_t position = moves_[m].position;
		insert_after(body_, to, position == 0 ? kNoId : order_[position - 1], moves_[m].inst);
		first_read_[body_.instructions[moves_[m].inst].result] = kNoId;
	}
}

bool CodeSinking::run()
{
	if (body_.layout.size() == 1)
		return false;
	c_.ensure_cfg();
	if (cfg_.rpo.size() != body_.layout.size())
		return false;
	find_loops(body_, cfg_, forest_);
	if (forest_.partial)
		return false;
	loop_depth_.assign(forest_.loops.size(), 1);
	for (uint32_t l = 0; l < forest_.loops.size(); ++l)
		if (forest_.loops[l].parent != kNoId)
			loop_depth_[l] = loop_depth_[forest_.loops[l].parent] + 1;
	lowest_.assign(body_.temps.size(), kNoId);
	moved_.assign(body_.instructions.size(), 0);
	moves_.clear();
	for (BlockId b : cfg_.rpo)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Phi)
				break;
			for (uint32_t k = 0; k + 1 < inst.extra_count; k += 2)
				read(body_.operands[inst.extra_first + k + 1], body_.operands[inst.extra_first + k].index);
		}
	for (size_t k = cfg_.rpo.size(); k-- > 0;)
		settle(cfg_.rpo[k]);
	if (moves_.empty())
		return false;
	unlink_moved();
	first_read_.assign(body_.temps.size(), kNoId);
	std::stable_sort(moves_.begin(), moves_.end(), [](const Move& a, const Move& b) { return a.to < b.to; });
	for (size_t first = 0; first < moves_.size();)
	{
		size_t last = first + 1;
		while (last < moves_.size() && moves_[last].to == moves_[first].to)
			++last;
		insert(first, last);
		first = last;
	}
	c_.invalidate_block_of();
	restore_definition_order(c_);
	return true;
}

} // namespace

bool sink_values(FunctionContext& context)
{
	CodeSinking pass(context);
	return pass.run();
}

} // namespace opt
