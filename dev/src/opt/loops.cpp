#include "opt/loops.h"

#include "lowir/builder.h"

#include <algorithm>
#include <utility>

namespace opt {

using namespace lowir;

void find_loops(const FunctionBody& body, const Cfg& cfg, LoopForest& forest)
{
	(void)body;
	forest.loops.clear();
	forest.innermost.assign(cfg.block_count, kNoId);
	forest.dropped.assign(cfg.block_count, 0);
	forest.partial = false;
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
		{
			for (BlockId b : loop.blocks)
				forest.dropped[b] = 1;
			continue;
		}
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
		{
			forest.partial = true;
			break;
		}
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

static uint64_t common_factor(uint64_t a, uint64_t b)
{
	while (b != 0)
	{
		const uint64_t r = a % b;
		a = b;
		b = r;
	}
	return a;
}

// Values one address's multiples read.
const unsigned kMultipleSteps = 32;

// Whether a product of v and a literal below 2^31 cannot wrap at 64 bits.
static bool bounded_factor(const FunctionBody& body, Value v)
{
	if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId)
		return false;
	const Instruction& def = body.instructions[body.temps[v.index].def];
	return def.op == Opcode::Convert &&
		(static_cast<ConvertOp>(def.sub) == ConvertOp::SExt || static_cast<ConvertOp>(def.sub) == ConvertOp::ZExt) &&
		storage_bytes(convert_source_type(def)) <= 4;
}

// A number v is a multiple of, 0 for any (a phi's own step). A wrapped product
// keeps only the power of two in its factor, so a larger factor needs a product
// that cannot wrap. A sum of such products or literal steps would need more
// iterations than can run to wrap.
static uint64_t multiple_of(const FunctionBody& body, Value v, std::vector<TempId>& open, unsigned& steps)
{
	if (v.is(ValueKind::Integer))
	{
		const uint64_t low = body.literals[v.index].low;
		return low >> 63 != 0 ? ~low + 1 : low;
	}
	if (!v.is(ValueKind::Temp) || body.temps[v.index].def == kNoId || ++steps > kMultipleSteps)
		return 1;
	const Type type = body.temps[v.index].type;
	if (type.kind() != TypeKind::I64)
		return 1;
	const Instruction& def = body.instructions[body.temps[v.index].def];
	if (def.op == Opcode::Copy)
		return multiple_of(body, def.ops[0], open, steps);
	if (def.op == Opcode::Phi)
	{
		for (TempId seen : open)
			if (seen == v.index)
				return 0;
		open.push_back(v.index);
		uint64_t factor = 0;
		for (uint32_t k = 0; k < def.extra_count && factor != 1; k += 2)
			factor = common_factor(factor, multiple_of(body, body.operands[def.extra_first + k + 1], open, steps));
		open.pop_back();
		return factor;
	}
	if (def.op != Opcode::Binary)
		return 1;
	const BinaryOp op = static_cast<BinaryOp>(def.sub);
	if (op == BinaryOp::Add || op == BinaryOp::Sub)
	{
		open.push_back(v.index);
		const uint64_t left = multiple_of(body, def.ops[0], open, steps);
		const uint64_t factor = left == 1 ? 1 : common_factor(left, multiple_of(body, def.ops[1], open, steps));
		open.pop_back();
		return factor;
	}
	if (op != BinaryOp::Mul)
		return 1;
	const bool literal_first = def.ops[0].is(ValueKind::Integer);
	const Value literal = def.ops[literal_first ? 0 : 1], factor = def.ops[literal_first ? 1 : 0];
	if (!literal.is(ValueKind::Integer))
		return 1;
	const uint64_t scale = body.literals[literal.index].low;
	if (scale == 0 || scale >= 1ull << 31)
		return 1;
	return (def.flags & IF_NO_WRAP) != 0 || bounded_factor(body, factor) ? scale : scale & (~scale + 1);
}

Strided strided_address(const FunctionBody& body, Value v)
{
	Strided out;
	std::vector<TempId> open;
	unsigned steps = 0;
	for (unsigned step = 0; step < 8; ++step)
	{
		if (!v.is(ValueKind::Temp) || v.index < body.parameter_count || body.temps[v.index].def == kNoId)
			break;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op == Opcode::Addr || (def.op == Opcode::Copy && def.type.is_pointer()))
			v = def.ops[0];
		else if (def.op == Opcode::Index)
		{
			const uint64_t size = storage_bytes(def.type);
			if (def.ops[1].is(ValueKind::Integer))
				out.offset += static_cast<int64_t>(body.literals[def.ops[1].index].low * size);
			else
				out.stride = common_factor(out.stride, multiple_of(body, def.ops[1], open, steps) * size);
			v = def.ops[0];
		}
		else if (def.op == Opcode::Phi && def.extra_count == 4)
		{
			// (A pointer stepping by a literal, from its entry value.)
			const Value first = body.operands[def.extra_first + 1], second = body.operands[def.extra_first + 3];
			const auto steps = [&](Value x) {
				if (!x.is(ValueKind::Temp) || body.temps[x.index].def == kNoId)
					return false;
				const Instruction& step_def = body.instructions[body.temps[x.index].def];
				return step_def.op == Opcode::Index && step_def.ops[0] == v && step_def.ops[1].is(ValueKind::Integer);
			};
			const bool first_steps = steps(first);
			if (!first_steps && !steps(second))
				break;
			const Instruction& step_def = body.instructions[body.temps[(first_steps ? first : second).index].def];
			const uint64_t size = storage_bytes(step_def.type) * body.literals[step_def.ops[1].index].low;
			out.stride = common_factor(out.stride, size >> 63 != 0 ? ~size + 1 : size);
			v = first_steps ? second : first;
		}
		else
			break;
	}
	out.base = v;
	return out;
}

bool strided_apart(const Strided& a, uint64_t a_bytes, const Strided& b, uint64_t b_bytes)
{
	if (!(a.base == b.base) || a.base.is(ValueKind::None))
		return false;
	const uint64_t stride = common_factor(a.stride, b.stride);
	if (stride <= 1 || a_bytes + b_bytes > stride)
		return false;
	// Where b starts after a's start, modulo the stride.
	const int64_t modulus = static_cast<int64_t>(stride);
	const uint64_t from = static_cast<uint64_t>(((b.offset - a.offset) % modulus + modulus) % modulus);
	return from >= a_bytes && from + b_bytes <= stride;
}

Located locate_steps(const FunctionBody& body, Value v)
{
	Located out;
	for (unsigned step = 0; step < 8; ++step)
	{
		if (!v.is(ValueKind::Temp) || v.index < body.parameter_count || body.temps[v.index].def == kNoId)
			break;
		const Instruction& def = body.instructions[body.temps[v.index].def];
		if (def.op == Opcode::Addr || (def.op == Opcode::Copy && def.type.is_pointer()))
			v = def.ops[0];
		else if (def.op == Opcode::Index && def.ops[1].is(ValueKind::Integer))
		{
			out.offset += static_cast<int64_t>(body.literals[def.ops[1].index].low) *
				static_cast<int64_t>(storage_bytes(def.type));
			v = def.ops[0];
		}
		else
			break;
	}
	out.root = v;
	return out;
}

// The header's phis take from the new preheader what they took from the blocks
// entering the loop, merged there by a phi where those differ.
static void merge_entry_values(FunctionBody& body, const LoopForest& forest, uint32_t loop, BlockId pre)
{
	const BlockId header = forest.loops[loop].header;
	InstId at = kNoId;
	for (InstId i = body.blocks[header].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
		i = body.instructions[i].next)
	{
		const Instruction phi = body.instructions[i];
		std::vector<Value> kept, entering;
		bool same = true;
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
		{
			const Value from = body.operands[phi.extra_first + k];
			const Value value = body.operands[phi.extra_first + k + 1];
			std::vector<Value>& side = in_loop(forest, loop, from.index) ? kept : entering;
			side.push_back(from);
			side.push_back(value);
			same = same && (&side == &kept || value == entering[1]);
		}
		Value merged = entering[1];
		if (!same)
		{
			Instruction values = make_instruction(Opcode::Phi, phi.type);
			values.extra_first = append_operands(body, entering.data(), static_cast<uint32_t>(entering.size()));
			values.extra_count = static_cast<uint32_t>(entering.size());
			const InstId id = append_instruction(body, values);
			const TempId result = append_temp(body, phi.type, id);
			body.instructions[id].result = result;
			insert_after(body, pre, at, id);
			at = id;
			merged = Value::temp(result);
		}
		kept.push_back(Value::block(pre));
		kept.push_back(merged);
		const uint32_t first = append_operands(body, kept.data(), static_cast<uint32_t>(kept.size()));
		body.instructions[i].extra_first = first;
		body.instructions[i].extra_count = static_cast<uint32_t>(kept.size());
	}
}

bool make_loop_preheader(FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop)
{
	const Loop& l = forest.loops[loop];
	std::vector<BlockId> entries;
	for (uint32_t k = cfg.pred_first[l.header]; k < cfg.pred_first[l.header + 1]; ++k)
	{
		const BlockId p = cfg.preds[k];
		if (in_loop(forest, loop, p) || std::find(entries.begin(), entries.end(), p) != entries.end())
			continue;
		if ((cfg.flags[p] & BF_HANDLER) != 0)
			return false;
		entries.push_back(p);
	}
	if (entries.empty())
		return false;
	const BlockId pre = append_block(body, BlockKind::LoopPreheader);
	Instruction jump = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
	jump.ops[0] = Value::block(l.header);
	insert_after(body, pre, kNoId, append_instruction(body, jump));
	std::vector<BlockId> layout;
	for (BlockId b : body.layout)
	{
		if (b == l.header)
			layout.push_back(pre);
		layout.push_back(b);
	}
	body.layout.swap(layout);
	for (BlockId entry : entries)
		for_each_successor(body, body.instructions[body.blocks[entry].last], [&](BlockId& t) {
			if (t == l.header)
				t = pre;
		});
	if (entries.size() == 1)
	{
		rename_phi_incoming(body, l.header, entries[0], pre);
		return true;
	}
	merge_entry_values(body, forest, loop, pre);
	return true;
}

bool claim_loop(const Cfg& cfg, const Loop& l, std::vector<uint8_t>& touched)
{
	std::vector<BlockId> near;
	for (BlockId b : l.blocks)
	{
		near.push_back(b);
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			near.push_back(cfg.succs[k]);
	}
	for (uint32_t k = cfg.pred_first[l.header]; k < cfg.pred_first[l.header + 1]; ++k)
		near.push_back(cfg.preds[k]);
	for (BlockId b : near)
		if (touched[b] != 0)
			return false;
	for (BlockId b : near)
		touched[b] = 1;
	return true;
}

} // namespace opt
