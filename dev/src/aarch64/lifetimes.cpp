#include "aarch64/lifetimes.h"
#include "machine/block_frequency.h"
#include "opt/loops.h"

#include <algorithm>
#include "machine/phi_inputs.h"
#include "machine/ssa_segments.h"
#include "machine/exception_predecessors.h"
#include "machine/exception_layout.h"
#include "opt/function_ir.h"

namespace aarch64 {
using namespace lowir;
namespace {

struct BlockSpan
{
	uint32_t first, last;
};

struct Reads
{
	BlockId definition = kNoId, block = kNoId;
	uint32_t other = kNoId, position = 0, written = 0;
	bool phi = false, incoming = false;
	bool local() const { return !phi && !incoming && (block == kNoId || (block == definition && other == kNoId)); }
};

void include(LiveRange& value, uint32_t at)
{
	value.first = std::min(value.first, at);
	value.last = std::max(value.last, at);
}

struct BlockRead
{
	BlockId block;
	uint32_t position, previous;
};

struct FunctionLiveness
{
	explicit FunctionLiveness(const FunctionBody& body)
		: spans(body.blocks.size()), ranges(body.temps.size()), reads(body.temps.size())
	{
		for (TempId id = 0; id < ranges.size(); ++id)
			ranges[id].id = id;
	}

	void read(TempId id, BlockId block, uint32_t at)
	{
		include(ranges[id], at);
		auto& use = reads[id];
		if (use.block == block)
		{
			use.position = std::max(use.position, at);
			return;
		}
		if (use.block != kNoId)
		{
			other_reads.push_back(BlockRead{ use.block, use.position, use.other });
			use.other = other_reads.size() - 1;
		}
		use.block = block;
		use.position = at;
	}

	void extend(const Program& program, const FunctionBody& body, const machine::ExceptionLayout* exceptions,
		uint32_t end, const std::vector<uint32_t>& uses, std::vector<std::pair<uint32_t, uint32_t>>& segments,
		opt::Cfg& cfg)
	{
		if (std::all_of(reads.begin(), reads.end(), [](const Reads& use) { return use.local(); }))
			return;
		std::vector<uint32_t> extra_first, extra_preds;
		bool full_ranges = !body.code_labels.empty();
		if (exceptions && !full_ranges)
		{
			extra_first.assign(body.blocks.size() + 1, 0);
			auto number = [&](uint32_t b) { return body.layout[b]; };
			auto has_phi = [&](uint32_t b) {
				auto i = body.blocks[number(b)].first;
				return i != kNoId && body.instructions[i].op == Opcode::Phi;
			};
			full_ranges = !machine::exception_predecessors(
				body, exceptions->analysis, number, [](InstId) { return true; }, has_phi, extra_first, extra_preds);
		}
		if (full_ranges)
		{
			// An exceptional or nonlocal transfer can bypass the ordinary predecessors.
			for (auto& range : ranges)
				if (!reads[range.id].local())
				{
					range.first = 0;
					range.last = end;
				}
			return;
		}
		opt::build_cfg_light(program, body, cfg, true);
		machine::SsaLiveness walk(cfg.pred_first, cfg.preds, end / 2 + body.layout.size());
		if (exceptions)
			walk.exception_edges(extra_first, extra_preds);
		machine::SsaSegments parts(body.blocks.size());
		bool within_budget = true;
		for (auto& range : ranges)
		{
			const auto& use = reads[range.id];
			if (use.local())
				continue;
			if (within_budget && use.definition != kNoId)
			{
				walk.begin(uses[range.id]);
				parts.begin();
				const auto definition = use.definition;
				parts.define(definition);
				auto read_block = [&](BlockId b, uint32_t at) {
					parts.read(b, at);
					if (b != definition)
						walk.enter(b);
				};
				if (use.block != kNoId)
					read_block(use.block, use.position);
				for (uint32_t i = use.other; i != kNoId; i = other_reads[i].previous)
					read_block(other_reads[i].block, other_reads[i].position);
				if (use.phi)
				{
					walk.mark_entry(definition);
					for (uint32_t k = cfg.pred_first[definition]; k < cfg.pred_first[definition + 1]; ++k)
						parts.transfer(cfg.preds[k]);
				}
				within_budget = walk.propagate(definition, [&](BlockId b) { parts.live_out(b); });
				if (within_budget)
				{
					range.segments = segments.size();
					auto bounds = [&](BlockId b) {
						return machine::BlockSlots{ static_cast<int32_t>(spans[b].first),
							static_cast<int32_t>(spans[b].last), cfg.succ_first[b + 1] - cfg.succ_first[b] > 1 };
					};
					parts.emit(
						walk, definition, use.written, use.phi, bounds, [&](BlockId, int32_t first, int32_t last) {
							segments.emplace_back(first, last);
							include(range, first);
							include(range, last);
						});
					range.count = segments.size() - range.segments;
					continue;
				}
			}
			range.first = 0;
			range.last = end;
		}
	}

	std::vector<BlockSpan> spans;
	std::vector<LiveRange> ranges;
	std::vector<Reads> reads;
	std::vector<BlockRead> other_reads;
};
} // namespace

void block_frequencies(const Program& program, const FunctionBody& body, opt::Cfg& cfg, std::vector<uint64_t>& weights)
{
	opt::complete_cfg(cfg);
	opt::LoopForest forest;
	opt::find_loops(body, cfg, forest);
	std::vector<uint32_t> first(1, 0), members;
	for (const auto& loop : forest.loops)
	{
		members.insert(members.end(), loop.blocks.begin(), loop.blocks.end());
		first.push_back(members.size());
	}
	std::vector<uint8_t> cold(body.blocks.size(), 0);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId && !cold[b]; i = body.instructions[i].next)
		{
			const auto& inst = body.instructions[i];
			cold[b] = inst.op == Opcode::Throw || inst.op == Opcode::Resume || inst.op == Opcode::Unreachable ||
				program.rare_call(inst);
		}
	machine::block_frequencies(
		body, body.layout.front(), cfg.succ_first, cfg.succs, cfg.pred_first, cfg.preds, cold, first, members,
		[](BlockId b) { return b; }, [&](BlockId b) { return cfg.reachable(b); },
		[](uint32_t, uint32_t, const std::vector<uint32_t>&, const std::vector<uint32_t>&, const Instruction&, bool) {
			return false;
		},
		weights);
}

std::vector<uint64_t> spill_weights(const Program& program, const FunctionBody& body, opt::Cfg& cfg,
	const std::vector<uint32_t>& aliases, std::vector<uint64_t>& weights)
{
	std::vector<uint64_t> cost(body.temps.size(), 0);
	block_frequencies(program, body, cfg, weights);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const auto& inst = body.instructions[i];
			if (inst.result != kNoId && representative(aliases, inst.result) != inst.result)
				continue;
			if (inst.result != kNoId)
				cost[inst.result] += weights[b];
			if (inst.op == Opcode::Phi)
			{
				for (uint32_t k = 0; k < inst.extra_count; k += 2)
				{
					const auto v = body.operands[inst.extra_first + k + 1];
					if (v.is(ValueKind::Temp))
						cost[representative(aliases, v.index)] += weights[body.operands[inst.extra_first + k].index];
				}
			}
			else
				opt::for_each_operand(body, inst, [&](Value v) {
					if (v.is(ValueKind::Temp))
						cost[representative(aliases, v.index)] += weights[b];
				});
		}
	return cost;
}

namespace {
// Prove that truncating a canonical integer home leaves every bit unchanged.
// The shared budget bounds work across both arms of a select and cyclic phis.
bool fits_unsigned(const FunctionBody& body, Value value, unsigned bits, unsigned& budget)
{
	if (value.is(ValueKind::Integer))
		return (body.literals[value.index].low >> bits) == 0;
	if (!value.is(ValueKind::Temp) || !budget)
		return false;
	--budget;
	const auto& temp = body.temps[value.index];
	if (!temp.type.is_integer() || integer_bits(temp.type.kind()) > 64)
		return false;
	if (integer_bits(temp.type.kind()) <= bits)
		return true;
	if (temp.def == kNoId)
		return false;
	const auto& def = body.instructions[temp.def];
	const auto fits = [&](Value v) { return fits_unsigned(body, v, bits, budget); };
	if (def.op == Opcode::Compare)
		return true;
	if (def.op == Opcode::Binary)
	{
		const auto op = static_cast<BinaryOp>(def.sub);
		if (op == BinaryOp::And)
			return fits(def.ops[1]) || fits(def.ops[0]);
		if (op == BinaryOp::Or || op == BinaryOp::Xor)
			return fits(def.ops[0]) && fits(def.ops[1]);
		if (op != BinaryOp::UShr || !def.ops[1].is(ValueKind::Integer))
			return false;
		const auto shift = body.literals[def.ops[1].index].low;
		const auto width = integer_bits(def.type.kind());
		return shift < width && width - shift <= bits;
	}
	if (def.op == Opcode::Select)
		return fits(def.ops[1]) && fits(def.ops[2]);
	if (def.op == Opcode::Copy ||
		(def.op == Opcode::Convert &&
			(def.sub == static_cast<unsigned>(ConvertOp::ZExt) || def.sub == static_cast<unsigned>(ConvertOp::Trunc))))
		return fits(def.ops[0]);
	if (def.op != Opcode::Phi || !def.extra_count || def.extra_count > 16)
		return false;
	for (uint32_t k = 1; k < def.extra_count; k += 2)
		if (!fits(body.operands[def.extra_first + k]))
			return false;
	return true;
}
// Whether `merge` is a minimum or maximum that lane_bound (vector.cpp)
// selects as a scalar bound: read only by the next extraction of its low lane.
bool lane_bound_shape(const FunctionBody& body, const Instruction& merge, const std::vector<uint32_t>& reads)
{
	const auto op = static_cast<VectorOp>(merge.sub);
	const bool wide = op == VectorOp::Minpd || op == VectorOp::Maxpd;
	if (merge.op != Opcode::Vector || (!wide && op != VectorOp::Minps && op != VectorOp::Maxps) ||
		merge.next == kNoId || merge.result == kNoId || reads[merge.result] != 1)
		return false;
	const auto& extract = body.instructions[merge.next];
	return extract.op == Opcode::Vector &&
		static_cast<VectorOp>(extract.sub) == (wide ? VectorOp::Tof64 : VectorOp::Tof32) &&
		extract.ops[0] == Value::temp(merge.result);
}

// The reads of each temp, and those by lane bounds (lane_bound_shape).
void lane_reads(const FunctionBody& body, std::vector<uint32_t>& reads, std::vector<uint32_t>& bounds)
{
	reads.assign(body.temps.size(), 0);
	bounds.assign(body.temps.size(), 0);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			opt::for_each_operand(body, body.instructions[i], [&](Value v) {
				if (v.is(ValueKind::Temp))
					++reads[v.index];
			});
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			if (lane_bound_shape(body, body.instructions[i], reads))
				for (unsigned k = 0; k < 2; ++k)
					if (body.instructions[i].ops[k].is(ValueKind::Temp))
						++bounds[body.instructions[i].ops[k].index];
}
} // namespace

// A copy or conversion that preserves all bits, or a truncation whose
// readers take only its low bits, can share its source's canonical
// register value or eight-byte spill. Account for every later use
// before allocating registers, including outgoing arguments and phi edges.
// A floating value made a vector that only lane bounds read is that value in
// a SIMD register's low lane: it shares the value's register and spill, and
// lane_bound reads the value.
std::vector<uint32_t> scalar_aliases(const FunctionBody& body)
{
	std::vector<uint32_t> parent, reads, bounds;
	std::vector<uint8_t> loose;
	for (TempId id = body.parameter_count; id < body.temps.size(); ++id)
	{
		const InstId def = body.temps[id].def;
		if (def == kNoId)
			continue;
		const auto& inst = body.instructions[def];
		const bool view = inst.op == Opcode::Vector &&
			(inst.sub == static_cast<unsigned>(VectorOp::Fromf64) ||
				inst.sub == static_cast<unsigned>(VectorOp::Fromf32));
		if (!inst.ops[0].is(ValueKind::Temp) || inst.result != id ||
			(inst.op != Opcode::Copy && inst.op != Opcode::Convert && !view))
			continue;
		const auto src = inst.ops[0].index;
		const Type from = body.temps[src].type;
		if (view)
		{
			const auto kind = inst.sub == static_cast<unsigned>(VectorOp::Fromf64) ? TypeKind::F64 : TypeKind::F32;
			if (src >= id || from != Type::scalar(kind))
				continue;
			if (reads.empty())
				lane_reads(body, reads, bounds);
			// The value then has more than one reader, so no selection that
			// takes a value's one reader along changes.
			if (!reads[id] || bounds[id] != reads[id] || reads[id] + reads[src] < 3)
				continue;
		}
		else if (inst.op == Opcode::Copy)
		{
			// Copy and conversion chains terminate at an SSA producer. In
			// particular, the phi on a loop backedge retains its own home.
			if (src >= id || from != inst.type || !(from.is_integer() || from.is_pointer()) ||
				integer_bits(from.kind()) > 64)
				continue;
		}
		else if (inst.sub == static_cast<unsigned>(ConvertOp::Trunc))
		{
			const auto bits = integer_bits(inst.type.kind());
			unsigned budget = 16;
			if (!bits || bits >= 64 || !from.is_integer() || integer_bits(from.kind()) > 64 ||
				from != convert_source_type(inst))
				continue;
			// Otherwise a truncation whose readers take only its low bits
			// (a loose value) holds its source's bits as they are.
			if (!fits_unsigned(body, inst.ops[0], bits, budget))
			{
				if (src >= id)
					continue;
				if (loose.empty())
					loose = loose_values(body, {}, true);
				if (!loose[id])
					continue;
			}
		}
		else if (inst.sub != static_cast<unsigned>(ConvertOp::ZExt) || from != convert_source_type(inst) ||
			!from.is_integer() || !inst.type.is_integer() || integer_bits(inst.type.kind()) > 64 ||
			integer_bits(from.kind()) >= integer_bits(inst.type.kind()))
			continue;
		if (parent.empty())
		{
			parent.resize(body.temps.size());
			for (TempId t = 0; t < parent.size(); ++t)
				parent[t] = t;
		}
		parent[id] = src;
	}
	for (TempId id = 0; id < parent.size(); ++id)
		while (parent[id] != parent[parent[id]])
			parent[id] = parent[parent[id]];
	return parent;
}

std::vector<LiveRange> live_ranges(const Program& program, const FunctionBody& body,
	const machine::ExceptionLayout* exceptions, std::vector<uint32_t>& uses, std::vector<uint32_t>& calls,
	std::vector<std::pair<uint32_t, uint32_t>>& segments, opt::Cfg& cfg, machine::PhiWebs& webs,
	std::vector<uint32_t>& entries, const std::vector<uint32_t>& aliases, bool separate_shared_inputs,
	RangeFacts& facts)
{
	FunctionLiveness plan(body);
	uses.assign(body.temps.size(), 0);
	const BlockId entry = body.layout.front();
	for (TempId id = 0; id < body.parameter_count; ++id)
	{
		plan.reads[id].definition = entry;
		include(plan.ranges[id], 0);
	}
	// Reads precede writes at an instruction. The block end also belongs to its phi transfers.
	uint32_t at = 2;
	for (BlockId b : body.layout)
	{
		plan.spans[b].first = b == entry ? 0 : at;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next, at += 2)
		{
			const auto& inst = body.instructions[i];
			if (inst.result != kNoId && representative(aliases, inst.result) != inst.result)
				continue;
			if (inst.op == Opcode::Call)
				calls.push_back(at);
			if (inst.result != kNoId)
			{
				auto& def = plan.reads[inst.result];
				def.definition = b;
				def.phi = inst.op == Opcode::Phi;
				def.written = def.phi ? plan.spans[b].first : at + 1;
				include(plan.ranges[inst.result], def.written);
			}
			if (inst.op == Opcode::Phi)
				continue;
			opt::for_each_operand(body, inst, [&](Value v) {
				if (v.is(ValueKind::Temp))
				{
					const auto id = representative(aliases, v.index);
					++uses[v.index];
					if (id != v.index)
						++uses[id];
					plan.read(id, b, at);
					plan.ranges[id].vector_address |= inst.op != Opcode::Vector && inst.op != Opcode::Call &&
						inst.op != Opcode::Return && inst.op != Opcode::Store && inst.op != Opcode::Copy;
				}
			});
		}
		plan.spans[b].last = at + 1;
		at += 2;
	}
	// Layout can put a phi before its backedge, so resolve edge positions after scanning.
	machine::PhiFanout fanout;
	for (BlockId b : body.layout)
	{
		bool indexed = false;
		const InstId first = body.blocks[b].first;
		for (InstId i = body.blocks[b].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const auto& phi = body.instructions[i];
			const bool check_fanout = separate_shared_inputs &&
				(phi.type.is_pointer() || phi.type.kind() == TypeKind::F64) &&
				(i != first || (phi.next != kNoId && body.instructions[phi.next].op == Opcode::Phi));
			if (check_fanout && !indexed)
			{
				fanout.read(body, b,
					[&](Value v) { return v.is(ValueKind::Temp) ? Value::temp(representative(aliases, v.index)) : v; });
				indexed = true;
			}
			for (uint32_t k = 0; k + 1 < phi.extra_count; k += 2)
			{
				const auto pred = body.operands[phi.extra_first + k].index;
				const auto v = body.operands[phi.extra_first + k + 1];
				if (v.is(ValueKind::Temp))
				{
					const auto id = representative(aliases, v.index);
					++uses[v.index];
					if (id != v.index)
						++uses[id];
					plan.read(id, pred, plan.spans[pred].last - 1);
					if (!check_fanout || !fanout.contains(pred, id))
					{
						if (webs.empty())
							webs.reset(body.temps.size());
						webs.join(id, phi.result);
						plan.ranges[id].affinity = phi.result;
						plan.ranges[phi.result].affinity = id;
						facts.copies.push_back(PhiCopy{ id, phi.result, pred, b });
					}
					plan.reads[id].incoming = true;
				}
			}
		}
	}
	if (body.layout.size() > 1 ||
		(body.blocks[entry].first != kNoId && body.instructions[body.blocks[entry].first].op == Opcode::Phi))
		plan.extend(program, body, exceptions, at, uses, segments, cfg);
	for (TempId id = 0; id < body.parameter_count; ++id)
		if (!uses[id])
			plan.ranges[id].first = kNoId;
	if (!webs.empty() && !segments.empty() && !exceptions && body.code_labels.empty())
	{
		entries.resize(body.blocks.size());
		for (BlockId block : body.layout)
			entries[block] = plan.spans[block].first;
	}
	if (exceptions)
	{
		for (BlockId block : exceptions->landing_blocks)
			if (block != kNoId)
				facts.landings.push_back(plan.spans[block].first);
		std::sort(facts.landings.begin(), facts.landings.end());
	}
	return std::move(plan.ranges);
}

} // namespace aarch64
