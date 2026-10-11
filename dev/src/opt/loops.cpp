#include "opt/loops.h"

#include <utility>

namespace opt {

using namespace lowir;

void find_loops(const FunctionBody& body, const Cfg& cfg, LoopForest& forest)
{
	(void)body;
	forest.loops.clear();
	forest.innermost.assign(cfg.block_count, kNoId);
	if (forest.mark.size() < cfg.block_count)
		forest.mark.assign(cfg.block_count, 0);
	static std::vector<BlockId> work; // (kept for its capacity, like size and cursor below)
	// The loops' block lists sum to the body's size times the nesting
	// depth: past a budget (eight memberships per block) the loops not yet
	// found are not reported (no loop transform applies to them).
	uint64_t memberships = 0;
	const uint64_t budget = 8 * static_cast<uint64_t>(cfg.rpo.size()) + 4096;
	for (BlockId h : cfg.rpo)
	{
		Loop loop;
		loop.header = h;
		loop.preheader = kNoId;
		loop.parent = forest.innermost[h];
		for (uint32_t k = cfg.pred_first[h]; k < cfg.pred_first[h + 1]; ++k)
		{
			const BlockId u = cfg.preds[k];
			if (cfg.reachable(u) && cfg.dominates(h, u))
				loop.latches.push_back(u);
		}
		if (loop.latches.empty())
			continue;
		const uint32_t epoch = ++forest.epoch;
		forest.mark[h] = epoch;
		loop.blocks.push_back(h);
		bool reducible = true;
		work.clear();
		for (BlockId u : loop.latches)
			if (forest.mark[u] != epoch)
			{
				forest.mark[u] = epoch;
				loop.blocks.push_back(u);
				work.push_back(u);
			}
		while (!work.empty() && reducible)
		{
			const BlockId x = work.back();
			work.pop_back();
			for (uint32_t k = cfg.pred_first[x]; k < cfg.pred_first[x + 1]; ++k)
			{
				const BlockId y = cfg.preds[k];
				if (!cfg.reachable(y) || forest.mark[y] == epoch)
					continue;
				if (!cfg.dominates(h, y))
				{
					reducible = false;
					break;
				}
				forest.mark[y] = epoch;
				loop.blocks.push_back(y);
				work.push_back(y);
			}
		}
		bool handlers = false;
		for (BlockId b : loop.blocks)
			handlers = handlers || (cfg.flags[b] & BF_HANDLER) != 0;
		if (!reducible || handlers)
			continue;
		BlockId outside = kNoId;
		uint32_t outside_count = 0;
		for (uint32_t k = cfg.pred_first[h]; k < cfg.pred_first[h + 1]; ++k)
			if (forest.mark[cfg.preds[k]] != epoch)
			{
				outside = cfg.preds[k];
				++outside_count;
			}
		if (outside_count == 1 && cfg.succ_count(outside) == 1)
			loop.preheader = outside;
		memberships += loop.blocks.size();
		if (memberships > budget)
			break;
		const uint32_t index = static_cast<uint32_t>(forest.loops.size());
		for (BlockId b : loop.blocks)
			forest.innermost[b] = index;
		forest.loops.push_back(std::move(loop));
	}
	// Preorder intervals of the loop tree: a parent precedes its children
	// in `loops`, so a loop's subtree is the run of loops after it whose
	// parent chains reach it; sizes accumulate children first.
	const uint32_t n = static_cast<uint32_t>(forest.loops.size());
	static std::vector<uint32_t> size, cursor;
	size.assign(n, 1);
	for (uint32_t k = n; k-- > 0;)
		if (forest.loops[k].parent != kNoId)
			size[forest.loops[k].parent] += size[k];
	forest.enter.assign(n, 0);
	forest.leave.assign(n, 0);
	cursor.assign(n, 0);
	uint32_t roots = 0;
	for (uint32_t k = 0; k < n; ++k)
	{
		const uint32_t p = forest.loops[k].parent;
		forest.enter[k] = p == kNoId ? roots : cursor[p];
		if (p == kNoId)
			roots += size[k];
		else
			cursor[p] += size[k];
		cursor[k] = forest.enter[k] + 1;
		forest.leave[k] = forest.enter[k] + size[k];
	}
}

bool in_loop(const LoopForest& forest, uint32_t l, BlockId b)
{
	if (b >= forest.innermost.size())
		return false; // a block made after the loops were found
	const uint32_t x = forest.innermost[b];
	return x != kNoId && forest.enter[l] <= forest.enter[x] && forest.enter[x] < forest.leave[l];
}

Located locate(const FunctionBody& body, Value v)
{
	Located out;
	for (unsigned step = 0; step < 8; ++step)
	{
		if (!v.is(ValueKind::Temp) || v.index < body.parameter_count || body.temps[v.index].def == kNoId)
			break;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op == Opcode::Addr || (def.op == Opcode::Copy && def.type.is_pointer()))
			v = def.ops[0];
		else if (def.op == Opcode::Index)
		{
			if (def.ops[1].is(ValueKind::Integer))
				out.offset += static_cast<int64_t>(body.literals[def.ops[1].index].low) *
					static_cast<int64_t>(storage_bytes(def.type));
			else
				out.known = false;
			v = def.ops[0];
		}
		else
			break;
	}
	out.root = v;
	return out;
}

} // namespace opt
