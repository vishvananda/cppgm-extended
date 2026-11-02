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

bool rare_float_path(const FunctionBody& body, const Cfg& cfg, BlockId b)
{
	if (cfg.pred_count(b) != 1)
		return false;
	const Instruction& branch = body.instructions[body.blocks[cfg.preds[cfg.pred_first[b]]].last];
	if (branch.op != Opcode::Branch || !branch.ops[0].is(ValueKind::Temp) || !(branch.ops[1].index == b))
		return false;
	const InstId def = body.temps[branch.ops[0].index].def;
	if (def == kNoId)
		return false;
	const Instruction& test = body.instructions[def];
	if (test.op != Opcode::Compare || !test.type.is_float())
		return false;
	const ComparePredicate pred = static_cast<ComparePredicate>(test.sub);
	return (pred == ComparePredicate::Ne && test.ops[0] == test.ops[1]) ||
		(pred == ComparePredicate::Lt && test.ops[1].is(ValueKind::Float) && body.literals[test.ops[1].index].low == 0);
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

// The largest value an index operand may hold, unsigned: its literal, an
// unsigned type's or a zero extension's range, or an `and` with a literal.
// ~0 when unknown.
static uint64_t index_bound(const FunctionBody& body, Value v)
{
	if (v.is(ValueKind::Integer))
		return body.literals[v.index].high == 0 && body.literals[v.index].low >> 63 == 0 ? body.literals[v.index].low
																						 : ~uint64_t(0);
	if (!v.is(ValueKind::Temp))
		return ~uint64_t(0);
	const TypeKind kind = body.temps[v.index].type.kind();
	if (kind == TypeKind::U8 || kind == TypeKind::U16)
		return kind == TypeKind::U8 ? 0xff : 0xffff;
	const InstId d = body.temps[v.index].def;
	if (d == kNoId)
		return ~uint64_t(0);
	const Instruction& def = body.instructions[d];
	if (def.op == Opcode::Convert && static_cast<ConvertOp>(def.sub) == ConvertOp::ZExt)
	{
		const uint64_t from = storage_bytes(convert_source_type(def));
		return from < 8 ? (uint64_t(1) << (8 * from)) - 1 : ~uint64_t(0);
	}
	if (def.op == Opcode::Binary && static_cast<BinaryOp>(def.sub) == BinaryOp::And)
		return std::min(index_bound(body, def.ops[0]), index_bound(body, def.ops[1]));
	return ~uint64_t(0);
}

// Whether `bytes` at `address` lie in a slot for any value of its index: a
// table lookup by a byte reads the table, whatever the byte.
bool table_entry(const FunctionBody& body, Value address, uint64_t bytes)
{
	if (!address.is(ValueKind::Temp) || body.temps[address.index].def == kNoId)
		return false;
	const Instruction& def = body.instructions[body.temps[address.index].def];
	if (def.op != Opcode::Index)
		return false;
	const Located base = locate(body, def.ops[0]);
	const uint64_t bound = index_bound(body, def.ops[1]), element = storage_bytes(def.type);
	if (!base.known || !base.root.is(ValueKind::Slot) || base.offset < 0 || bound >= (uint64_t(1) << 32) ||
		element >= (uint64_t(1) << 16))
		return false;
	return static_cast<uint64_t>(base.offset) + bound * element + bytes <=
		storage_bytes(body.slots[base.root.index].type);
}

// Whether two addresses are one value or the same index steps from one.
static bool same_address(const FunctionBody& body, Value a, Value b)
{
	for (unsigned depth = 0; depth < 4; ++depth)
	{
		if (a == b)
			return true;
		if (!a.is(ValueKind::Temp) || !b.is(ValueKind::Temp) || body.temps[a.index].def == kNoId ||
			body.temps[b.index].def == kNoId)
			return false;
		const Instruction& x = body.instructions[body.temps[a.index].def];
		const Instruction& y = body.instructions[body.temps[b.index].def];
		if (x.op != Opcode::Index || y.op != Opcode::Index || x.type != y.type || !(x.ops[1] == y.ops[1]) ||
			x.immediate != y.immediate)
			return false;
		a = x.ops[0];
		b = y.ops[0];
	}
	return false;
}

// What a load moved to `pre` reads when `pre`, or the one block before it,
// already loaded or stored that address at its type with no write after it:
// that value (None otherwise).
Value read_in_preheader(
	const Program& program, const FunctionBody& body, const Cfg& cfg, BlockId pre, const Instruction& load)
{
	BlockId blocks[2] = { kNoId, pre };
	if (cfg.pred_first[pre + 1] - cfg.pred_first[pre] == 1)
		blocks[0] = cfg.preds[cfg.pred_first[pre]];
	Value known = Value::none();
	for (BlockId b : blocks)
		for (InstId i = b == kNoId ? kNoId : body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			const bool same = inst.type == load.type && (inst.flags & IF_VOLATILE) == 0;
			if (inst.op == Opcode::Load && same && same_address(body, inst.ops[0], load.ops[0]))
				known = Value::temp(inst.result);
			else if (inst.op == Opcode::Store && same && same_address(body, inst.ops[1], load.ops[0]))
				known = inst.ops[0];
			else if (may_write_memory(program, inst))
				known = Value::none();
		}
	return known;
}

// A phi of `header` that `v` now enters from every edge (or that reads only
// itself otherwise) is `v`.
void fold_header_phis(FunctionBody& body, BlockId header, Value v, std::vector<uint32_t>& uses)
{
	for (InstId i = body.blocks[header].first, prev = kNoId; i != kNoId;)
	{
		const Instruction& phi = body.instructions[i];
		const InstId next = phi.next;
		if (phi.op != Opcode::Phi)
			break;
		bool same = true;
		for (uint32_t k = 1; k < phi.extra_count && same; k += 2)
		{
			const Value in = body.operands[phi.extra_first + k];
			same = in == v || in == Value::temp(phi.result);
		}
		if (!same)
		{
			prev = i;
			i = next;
			continue;
		}
		const TempId result = phi.result;
		unlink_after(body, header, prev, i);
		if (v.is(ValueKind::Temp) && v.index < uses.size() && result < uses.size())
			uses[v.index] += uses[result];
		replace_all_uses(body, result, v);
		i = next;
	}
}

// The test number (0-based) at which a counted loop leaves: the induction
// starts at a literal and steps by a literal, the bound is a literal, and
// the tested values move from continuing to leaving without leaving the
// comparison's integer range (no wrap). `next` tests the stepped value.
bool counted_trips(Type type, ComparePredicate pred, bool continue_on_true, __int128 init, __int128 step,
	__int128 bound, bool next, int64_t& trips, __int128& last)
{
	const unsigned bits = integer_bits(type.kind());
	const bool unsigned_order = pred == ComparePredicate::Ult || pred == ComparePredicate::Ule ||
		pred == ComparePredicate::Ugt || pred == ComparePredicate::Uge;
	if (bits == 0 || bits > 64)
		return false;
	const __int128 lo = unsigned_order ? 0 : -(static_cast<__int128>(1) << (bits - 1));
	const __int128 hi = unsigned_order ? (static_cast<__int128>(1) << bits) - 1
									   : (static_cast<__int128>(1) << (bits - 1)) - 1;
	auto in_range = [&](__int128 v) { return v >= lo && v <= hi; };
	// Literals read in the comparison's order.
	auto order = [&](__int128 v) {
		const __int128 mask = (static_cast<__int128>(1) << bits) - 1;
		v &= mask;
		if (!unsigned_order && v > hi)
			v -= mask + 1;
		return v;
	};
	init = order(init);
	bound = order(bound);
	auto holds = [&](__int128 x) {
		bool r;
		switch (pred)
		{
		case ComparePredicate::Eq:
			r = x == bound;
			break;
		case ComparePredicate::Ne:
			r = x != bound;
			break;
		case ComparePredicate::Lt:
		case ComparePredicate::Ult:
			r = x < bound;
			break;
		case ComparePredicate::Le:
		case ComparePredicate::Ule:
			r = x <= bound;
			break;
		case ComparePredicate::Gt:
		case ComparePredicate::Ugt:
			r = x > bound;
			break;
		default:
			r = x >= bound;
			break;
		}
		return r == continue_on_true;
	};
	const __int128 first = init + (next ? step : 0);
	if (!in_range(first))
		return false;
	if (!holds(first))
	{
		trips = 0;
		last = first;
		return true;
	}
	// The first k with !holds(first + k*step), found by the side the bound
	// lies on; any candidate is checked exactly.
	__int128 k;
	const __int128 gap = bound - first;
	if (step > 0 ? gap < 0 : gap > 0)
		k = 1; // already past the bound: the next value decides
	else
		k = gap / step;
	for (__int128 probe = k > 1 ? k - 1 : 1; probe <= k + 2; ++probe)
	{
		const __int128 x = first + probe * step;
		if (!in_range(x))
			return false;
		if (!holds(x))
		{
			// Every earlier value continued (monotone between first and x).
			for (__int128 q = 1; q < probe && q < 3; ++q)
				if (!holds(first + q * step))
					return false;
			if (probe > 0x7fffffff)
				return false;
			trips = static_cast<int64_t>(probe);
			last = x;
			return true;
		}
	}
	return false;
}

// The most dominators and blocks between them and a loop read_before_loop
// searches.
const unsigned kReadBefore = 4;
const size_t kReadBetween = 16;

bool same_place(const Located& a, Type a_type, const Located& b, Type b_type)
{
	return a.root == b.root && a.known && b.known && a.offset == b.offset && a_type == b_type;
}

bool read_before_loop(
	const FunctionBody& body, const Cfg& cfg, const LoopForest& forest, uint32_t loop, const Located& at, Type type)
{
	const BlockId header = forest.loops[loop].header;
	BlockId b = cfg.idom[header];
	for (unsigned step = 0; b != kNoId && step < kReadBefore; ++step, b = cfg.idom[b])
	{
		bool read = false, call = false;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if ((inst.op == Opcode::Load || inst.op == Opcode::Store) && (inst.flags & IF_VOLATILE) == 0 &&
				inst.type == type &&
				same_place(locate(body, inst.ops[inst.op == Opcode::Load ? 0 : 1]), inst.type, at, type))
				read = true;
			else if (inst.op == Opcode::Call)
				read = false, call = true;
		}
		if (!read)
		{
			if (call)
				return false;
			continue;
		}
		// The blocks from b to the loop, back from the header's entries.
		std::vector<BlockId> between;
		for (uint32_t k = cfg.pred_first[header]; k < cfg.pred_first[header + 1]; ++k)
			if (!in_loop(forest, loop, cfg.preds[k]) && cfg.preds[k] != b)
				between.push_back(cfg.preds[k]);
		for (size_t n = 0; n < between.size(); ++n)
		{
			if (between.size() > kReadBetween)
				return false;
			const BlockId x = between[n];
			for (InstId i = body.blocks[x].first; i != kNoId; i = body.instructions[i].next)
				if (body.instructions[i].op == Opcode::Call)
					return false;
			for (uint32_t k = cfg.pred_first[x]; k < cfg.pred_first[x + 1]; ++k)
			{
				const BlockId p = cfg.preds[k];
				if (p != b && std::find(between.begin(), between.end(), p) == between.end())
					between.push_back(p);
			}
		}
		return true;
	}
	return false;
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
