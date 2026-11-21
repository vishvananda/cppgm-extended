#include "aarch64/selector.h"
#include "aarch64/lifetimes.h"
#include "opt/function_ir.h"
#include "opt/loops.h"
#include "machine/frame_lifetimes.h"
#include "machine/interval_slots.h"
#include "machine/register_pressure.h"
#include "machine/shrink_region.h"

#include <algorithm>
#include <cmath>

namespace aarch64 {
using namespace lowir;

namespace {
using Segments = std::vector<std::pair<uint32_t, uint32_t>>;
constexpr uint32_t kPreservedSimd = 0xffu << 8;

template<typename F>
void each_interval(const LiveRange& range, const Segments& segments, F f)
{
	if (!range.count)
		f(range.first, range.last);
	else
		for (uint32_t i = range.segments; i < range.segments + range.count; ++i)
			f(segments[i].first, segments[i].second);
}

enum class RegisterPressure
{
	Fits,
	AcrossCalls,
	Saturated
};

bool uses_simd(const FunctionBody& body, const LiveRange& range)
{
	const auto kind = body.temps[range.id].type.kind();
	return kind == TypeKind::F32 || kind == TypeKind::F64 || (kind == TypeKind::V128 && !range.vector_address);
}

// Count values that are definitely live at the same time. Half-open
// segments omit values dying at a call and its newly defined result.
RegisterPressure register_pressure(const FunctionBody& body, const std::vector<LiveRange>& ranges,
	const Segments& segments, const std::vector<uint32_t>& calls, uint32_t slots, int preserved, Bank bank)
{
	if (segments.empty() || ranges.size() <= static_cast<unsigned>(preserved))
		return RegisterPressure::Fits;
	const bool simd = bank == Bank::Simd;
	const auto belongs = [&](const LiveRange& range) {
		const auto type = body.temps[range.id].type;
		return range.first != kNoId &&
			(simd ? uses_simd(body, range)
				  : type.is_pointer() || (type.is_integer() && integer_bits(type.kind()) <= 64));
	};
	if (simd && std::count_if(ranges.begin(), ranges.end(), belongs) <= preserved)
		return RegisterPressure::Fits;
	std::vector<int32_t> change(slots + 1);
	for (const auto& range : ranges)
	{
		if (!belongs(range))
			continue;
		each_interval(range, segments, [&](uint32_t first, uint32_t last) {
			++change[first];
			--change[last];
		});
	}
	uint32_t next = 0;
	int live = 0;
	bool across_calls = false;
	const int capacity = preserved + (simd ? 22 : 11);
	for (uint32_t slot = 0; slot < slots; ++slot)
	{
		live += change[slot];
		if (live > capacity)
			return RegisterPressure::Saturated;
		if (next < calls.size() && calls[next] == slot)
		{
			if (live > preserved)
			{
				if (!simd)
					return RegisterPressure::AcrossCalls;
				across_calls = true;
			}
			++next;
		}
	}
	return across_calls ? RegisterPressure::AcrossCalls : RegisterPressure::Fits;
}

// The factor of a range of `slots` read/write slots against the reference range.
double length_factor(uint32_t slots)
{
	constexpr double reference_length = 1024; // Read/write slots in the reference range.
	return std::sqrt(reference_length / std::max(1u, slots));
}

// In a saturated SIMD bank, one long range can force many short arithmetic
// results into integer registers. Discount long ranges while retaining
// the block frequency and use counts in the ordinary spill weights.
double simd_spill_priority(const FunctionBody& body, const LiveRange& range, const Segments& segments, uint64_t weight)
{
	if (range.first == kNoId || !uses_simd(body, range))
		return static_cast<double>(weight);
	uint32_t slots = 0;
	each_interval(range, segments, [&](uint32_t first, uint32_t last) { slots += last - first; });
	return static_cast<double>(weight) * length_factor(slots);
}

// In a saturated general bank, the spill-weight order can leave a short
// range no register where the heavier long ranges already fill the bank, so
// each of many short results pays a store and a reload on its own chain
// although one register would hold them in turn. Such a range takes the
// SIMD bank's discounted priority, which puts it before the long ranges it
// meets: a short integer or address range (at most `short_length` slots)
// that would find no register in a model of the order. In the model a range
// holds a register where fewer than `assignable` ranges before it hold one,
// and, crossing a call, fewer than `preserved` crossing ranges. A phi's or a
// move's partner keeps its place, where it shares its partner's register.
// Returns whether a range was raised.
bool raise_crowded_short_ranges(const FunctionBody& body, const std::vector<uint32_t>& aliases,
	const std::vector<LiveRange>& ranges, const Segments& segments, const std::vector<uint32_t>& calls, uint32_t slots,
	int32_t assignable, int32_t preserved, const std::vector<uint32_t>& order, std::vector<double>& priority)
{
	constexpr uint32_t short_length = 16;
	std::vector<uint8_t> partner(body.temps.size(), 0);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const auto& inst = body.instructions[i];
			if ((inst.op == Opcode::Copy || inst.op == Opcode::Convert) && inst.result != kNoId &&
				inst.ops[0].is(ValueKind::Temp))
			{
				partner[representative(aliases, inst.result)] = 1;
				partner[representative(aliases, inst.ops[0].index)] = 1;
			}
		}
	machine::PressureTree held, crossing;
	bool raised = false;
	held.build(std::vector<int32_t>(slots + 1, 0));
	crossing.build(std::vector<int32_t>(slots + 1, 0));
	for (uint32_t index : order)
	{
		const auto& range = ranges[index];
		const auto type = body.temps[range.id].type;
		if (range.first == kNoId || !(type.is_pointer() || (type.is_integer() && integer_bits(type.kind()) <= 64)))
			continue;
		uint32_t length = 0;
		int32_t most = 0, most_crossing = 0;
		bool across = false;
		each_interval(range, segments, [&](uint32_t first, uint32_t last) {
			length += last - first;
			most = std::max(most, held.max(first, last));
			const auto call = std::lower_bound(calls.begin(), calls.end(), first);
			across |= call != calls.end() && *call < last;
		});
		if (across)
			each_interval(range, segments, [&](uint32_t first, uint32_t last) {
				most_crossing = std::max(most_crossing, crossing.max(first, last));
			});
		if (most < assignable && (!across || most_crossing < preserved))
		{
			each_interval(range, segments, [&](uint32_t first, uint32_t last) {
				held.add(first, last, 1);
				if (across)
					crossing.add(first, last, 1);
			});
		}
		else if (length <= short_length && range.affinity == kNoId && !partner[range.id])
		{
			priority[index] *= length_factor(length);
			raised = true;
		}
	}
	return raised;
}

// Let each scalar phi web's call-crossing members choose before its other
// members. Otherwise a hot join can reserve a caller register the crossing
// members cannot use, leaving transfers on its incoming edges and backedge.
// With spill priorities, a crossing member takes the priority of the web's
// heaviest other member when that is higher, so a cold web still leaves the
// preserved registers to hotter values. Without them, the crossing members
// simply choose first.
void prioritize_call_webs(const FunctionBody& body, const std::vector<LiveRange>& ranges, const Segments& segments,
	const std::vector<uint32_t>& calls, machine::PhiWebs& webs, const std::vector<double>& priority,
	std::vector<uint32_t>& order)
{
	std::vector<uint8_t> constrained(ranges.size());
	for (uint32_t index : order)
	{
		const auto& range = ranges[index];
		const auto type = body.temps[range.id].type;
		if (range.affinity == kNoId ||
			!(type.kind() == TypeKind::F32 || type.kind() == TypeKind::F64 || type.is_pointer() ||
				(type.is_integer() && integer_bits(type.kind()) <= 64)))
			continue;
		each_interval(range, segments, [&](uint32_t first, uint32_t last) {
			const auto call = std::lower_bound(calls.begin(), calls.end(), first);
			constrained[index] |= call != calls.end() && *call < last;
		});
	}
	if (priority.empty())
	{
		std::stable_partition(order.begin(), order.end(), [&](uint32_t index) { return constrained[index]; });
		return;
	}
	std::vector<double> heaviest(body.temps.size(), 0.0), key(priority);
	for (uint32_t index : order)
		if (ranges[index].affinity != kNoId && !constrained[index])
		{
			auto& web = heaviest[webs.root(ranges[index].id)];
			web = std::max(web, priority[index]);
		}
	for (uint32_t index : order)
		if (constrained[index])
			key[index] = std::max(key[index], heaviest[webs.root(ranges[index].id)]);
	std::stable_sort(order.begin(), order.end(),
		[&](uint32_t a, uint32_t b) { return key[a] != key[b] ? key[a] > key[b] : constrained[a] > constrained[b]; });
}

// Give scalar SIMD phis and their backedge values first choice over short
// intermediates. Their shared web preference can then eliminate a transfer
// cycle on each iteration. When one header's phis alone outnumber the SIMD
// registers (v0-v15, v18-v31), some of them spill anyway, and a value read
// more often than any of them (by spill weight) keeps its place.
void prioritize_loop_webs(const FunctionBody& body, const opt::Cfg& cfg, const std::vector<LiveRange>& ranges,
	const std::vector<uint32_t>& aliases, const std::vector<uint64_t>& weight, std::vector<uint32_t>& order)
{
	if (cfg.rpo_index.empty())
		return;
	std::vector<uint8_t> carried;
	uint32_t phis = 0;
	for (BlockId b : body.layout)
	{
		if (cfg.rpo_index[b] == kNoId)
			continue;
		uint32_t here = 0;
		for (InstId i = body.blocks[b].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
			i = body.instructions[i].next)
		{
			const auto& phi = body.instructions[i];
			if (phi.type.kind() != TypeKind::F32 && phi.type.kind() != TypeKind::F64)
				continue;
			for (unsigned k = 0; k < phi.extra_count; k += 2)
			{
				const BlockId pred = body.operands[phi.extra_first + k].index;
				const Value incoming = body.operands[phi.extra_first + k + 1];
				const uint32_t from = cfg.rpo_index[pred];
				if (!incoming.is(ValueKind::Temp) || from == kNoId || from < cfg.rpo_index[b])
					continue;
				if (carried.empty())
					carried.assign(body.temps.size(), 0);
				here += !carried[phi.result];
				carried[phi.result] = 1;
				carried[representative(aliases, incoming.index)] = 1;
			}
		}
		phis = std::max(phis, here);
	}
	if (carried.empty())
		return;
	if (order.empty())
	{
		order.resize(ranges.size());
		for (uint32_t i = 0; i < order.size(); ++i)
			order[i] = i;
	}
	uint64_t top = 0;
	const bool crowded = !weight.empty() && phis > 30;
	if (crowded)
		for (const auto& range : ranges)
			if (range.first != kNoId && carried[range.id])
				top = std::max(top, weight[range.id]);
	std::stable_partition(order.begin(), order.end(), [&](uint32_t i) {
		const auto id = ranges[i].id;
		return carried[id] || (crowded && weight[id] > top);
	});
}

// A guarded call that cannot repeat has a path to a return on which
// preserving its live registers would be unnecessary.
bool guarded_call(const FunctionBody& body, const opt::Cfg& cfg, BlockId site)
{
	if (cfg.succ_first.empty() || body.layout.size() < 3 || body.layout.size() > 96 || site == body.layout.front())
		return false;
	std::vector<uint8_t> seen(body.blocks.size());
	std::vector<BlockId> pending;
	auto successors = [&](BlockId b) {
		pending.insert(pending.end(), cfg.succs.begin() + cfg.succ_first[b], cfg.succs.begin() + cfg.succ_first[b + 1]);
	};
	successors(site);
	while (!pending.empty())
	{
		const auto b = pending.back();
		pending.pop_back();
		if (b == site)
			return false;
		if (!seen[b])
		{
			seen[b] = 1;
			successors(b);
		}
	}
	std::fill(seen.begin(), seen.end(), 0);
	seen[site] = 1;
	pending.push_back(body.layout.front());
	while (!pending.empty())
	{
		const auto b = pending.back();
		pending.pop_back();
		if (seen[b])
			continue;
		seen[b] = 1;
		const auto last = body.blocks[b].last;
		if (last != kNoId && body.instructions[last].op == Opcode::Return)
			return true;
		successors(b);
	}
	return false;
}

// Whether the range is live at one of the sorted positions.
bool live_into(const LiveRange& range, const Segments& segments, const std::vector<uint32_t>& positions)
{
	bool live = false;
	if (!positions.empty())
		each_interval(range, segments, [&](uint32_t a, uint32_t b) {
			const auto it = std::lower_bound(positions.begin(), positions.end(), a);
			live |= it != positions.end() && *it <= b;
		});
	return live;
}

// The first and last positions of each block, as live_ranges numbers them.
void block_positions(const FunctionBody& body, std::vector<uint32_t>& first, std::vector<uint32_t>& last)
{
	first.assign(body.blocks.size(), 0);
	last.assign(body.blocks.size(), 0);
	uint32_t at = 2;
	for (BlockId b : body.layout)
	{
		first[b] = b == body.layout.front() ? 0 : at;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			at += 2;
		last[b] = at + 1;
		at += 2;
	}
}

// The positions [first, last] of the entry and the short returns it
// enters when shrink_wrap.cpp can run them without the frame, found on
// LowIR with the region analysis it uses (its narrowest plan: other blocks
// may need the frame for spills). A block needs the frame when it calls, or
// names a slot, a symbol or a floating value. The positions are those of
// live_ranges.
std::vector<uint32_t> frameless_spans(const FunctionBody& body, const opt::Cfg& cfg)
{
	std::vector<uint32_t> spans;
	const auto n = static_cast<uint32_t>(body.layout.size());
	if (n < 2 || cfg.succ_first.empty())
		return spans;
	std::vector<uint32_t> index(body.blocks.size(), kNoId), first(n), last(n);
	for (uint32_t k = 0; k < n; ++k)
		index[body.layout[k]] = k;
	machine::ShrinkGraph graph;
	graph.succs.resize(n);
	graph.needs.assign(n, 0);
	graph.tabled.assign(n, 0);
	graph.returns.assign(n, 0);
	graph.short_return.assign(n, 0);
	graph.counts.assign(n, 0);
	uint32_t at = 2;
	for (uint32_t k = 0; k < n; ++k)
	{
		const BlockId b = body.layout[k];
		first[k] = k ? at : 0;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next, at += 2)
		{
			const auto& inst = body.instructions[i];
			bool needs = inst.op == Opcode::Call || inst.type.is_float() || inst.type.is_vector();
			opt::for_each_operand(body, inst,
				[&](Value v) { needs |= v.is(ValueKind::Slot) || v.is(ValueKind::Symbol) || v.is(ValueKind::Float); });
			graph.needs[k] |= needs;
			graph.tabled[k] |= inst.op == Opcode::Switch;
			graph.returns[k] = inst.op == Opcode::Return;
			++graph.counts[k];
		}
		last[k] = at + 1;
		at += 2;
		for (uint32_t s = cfg.succ_first[b]; s < cfg.succ_first[b + 1]; ++s)
			graph.succs[k].push_back(index[cfg.succs[s]]);
		graph.short_return[k] = !graph.needs[k] && graph.returns[k] && graph.counts[k] <= 4;
	}
	std::vector<uint8_t> framed;
	machine::framed_blocks(graph, framed);
	machine::ShrinkRegion region;
	if (framed[0] || !machine::shrink_region(graph, framed, machine::ShrinkPlan::Minimal, 0, 0, region))
		return spans;
	for (uint32_t k = 0; k < n; ++k)
		if (region.region[k] || region.copied[k])
		{
			spans.push_back(first[k]);
			spans.push_back(last[k]);
		}
	return spans;
}

// Whether the range is live within one of the spans.
bool live_within(const LiveRange& range, const Segments& segments, const std::vector<uint32_t>& spans)
{
	bool live = false;
	if (!spans.empty())
		each_interval(range, segments, [&](uint32_t a, uint32_t b) {
			for (size_t k = 0; !live && k < spans.size(); k += 2)
				live = a <= spans[k + 1] && spans[k] <= b;
		});
	return live;
}

// The register each parameter arrives in, when it stays there. An indirect
// result parameter takes x8.
std::vector<unsigned> incoming_registers(
	const Program& program, const lowir::Signature& signature, const FunctionBody& body, bool darwin, bool& free_x8)
{
	std::vector<unsigned> incoming(body.parameter_count, kNoId);
	Arguments args(darwin);
	for (TempId id = 0; id < body.parameter_count; ++id)
	{
		const auto& param = program.parameters_of(signature)[id];
		if (param.pass == ir_model::PPM_INDIRECT_RESULT)
		{
			free_x8 = false;
			continue;
		}
		const auto arg = args.next(param.type, param.shape);
		if ((arg.bank == Bank::General && !memory_value(param.type)) ||
			(arg.bank == Bank::Simd && (param.type.kind() == TypeKind::V128 || !memory_value(param.type))))
			incoming[id] = arg.first;
	}
	return incoming;
}

// Reuse the tail of the slot area when all its remaining owners live in
// registers: the frame ends after the last slot still in memory. The
// original slot offsets below it stay valid.
uint32_t reused_slot_tail(const FunctionBody& body, const std::vector<uint32_t>& slots,
	const std::vector<uint8_t>& object_words, uint32_t base)
{
	uint32_t frame = base;
	for (SlotId i = 0; i < slots.size(); ++i)
		if (!object_words[i] && slots[i] != kNoId)
		{
			const Type type = body.slots[i].type;
			const uint64_t bytes = (storage_bytes(type) + 7) & ~uint64_t(7);
			const unsigned align = data_alignment(type);
			frame = std::max(frame, static_cast<uint32_t>(slots[i] + bytes + (align > 16 ? align - 16 : 0)));
		}
	return frame;
}

// The order in which ranges choose homes: by spill priority when the
// registers may not suffice, with the phi web adjustments above; empty
// (program order) otherwise. `general_registers` counts the preserved
// general registers, `assignable` every general register a range can take.
std::vector<uint32_t> allocation_order(const Program& program, const FunctionBody& body, opt::Cfg& cfg,
	const std::vector<uint32_t>& aliases, std::vector<uint64_t>& block_weights, const std::vector<LiveRange>& ranges,
	const Segments& segments, const std::vector<uint32_t>& calls, machine::PhiWebs& webs, uint32_t slots,
	unsigned level, unsigned general_registers, unsigned assignable, std::vector<uint64_t>& weight)
{
	std::vector<uint32_t> order;
	std::vector<double> priority;
	const auto simd_pressure = register_pressure(body, ranges, segments, calls, slots, 8, Bank::Simd);
	const auto general_pressure = register_pressure(
		body, ranges, segments, calls, slots, general_registers, Bank::General);
	if (simd_pressure != RegisterPressure::Fits || general_pressure != RegisterPressure::Fits)
	{
		weight = spill_weights(program, body, cfg, aliases, block_weights);
		order.resize(ranges.size());
		priority.reserve(ranges.size());
		for (uint32_t i = 0; i < order.size(); ++i)
		{
			order[i] = i;
			const auto& range = ranges[i];
			priority.push_back(simd_pressure == RegisterPressure::Saturated
					? simd_spill_priority(body, range, segments, weight[range.id])
					: static_cast<double>(weight[range.id]));
		}
		const auto by_priority = [&](uint32_t a, uint32_t b) { return priority[a] > priority[b]; };
		std::stable_sort(order.begin(), order.end(), by_priority);
		if (general_pressure == RegisterPressure::Saturated &&
			raise_crowded_short_ranges(body, aliases, ranges, segments, calls, slots, static_cast<int32_t>(assignable),
				static_cast<int32_t>(general_registers), order, priority))
			std::stable_sort(order.begin(), order.end(), by_priority);
		if (level >= 2 && !calls.empty() && !webs.empty())
			prioritize_call_webs(body, ranges, segments, calls, webs, priority, order);
	}
	if (level >= 3 && !segments.empty() && !webs.empty())
		prioritize_loop_webs(body, cfg, ranges, aliases, weight, order);
	if (level >= 2 && priority.empty() && !order.empty() && !calls.empty() && !webs.empty())
		prioritize_call_webs(body, ranges, segments, calls, webs, priority, order);
	return order;
}

// Phi transfers whose two ends never overlap can use one register. Join
// them into groups, forward transfers first, while no two members of a group
// overlap. A group crosses a call when one of its members does.
class PhiGroups
{
public:
	PhiGroups(const FunctionBody& body, const std::vector<uint32_t>& aliases, const std::vector<LiveRange>& ranges,
		const Segments& segments, const std::vector<PhiCopy>& copies, const std::vector<uint32_t>& calls)
	{
		if (copies.empty() || segments.empty())
			return;
		const size_t count = body.temps.size();
		std::vector<uint32_t> range_of(count, kNoId), position(body.blocks.size(), 0), order(copies.size());
		for (uint32_t i = 0; i < ranges.size(); ++i)
			if (ranges[i].first != kNoId)
				range_of[ranges[i].id] = i;
		for (uint32_t k = 0; k < body.layout.size(); ++k)
			position[body.layout[k]] = k;
		for (uint32_t c = 0; c < copies.size(); ++c)
			order[c] = c;
		std::stable_partition(
			order.begin(), order.end(), [&](uint32_t c) { return position[copies[c].from] < position[copies[c].to]; });
		root_.assign(count, kNoId);
		Spans spans{ ranges, segments, range_of, std::vector<uint32_t>(count, kNoId), {} };
		std::vector<uint32_t> target(copies.size());
		for (uint32_t c = 0; c < copies.size(); ++c)
			target[c] = copies[c].phi;
		split_inputs(body, aliases, copies, position, spans, target);
		for (uint32_t c : order)
		{
			uint32_t ends[] = { copies[c].source, target[c] };
			if (range_of[ends[0]] == kNoId || range_of[ends[1]] == kNoId)
				continue;
			for (auto& end : ends)
			{
				if (root_[end] == kNoId)
				{
					root_[end] = end;
					spans.get(end);
				}
				end = find(end);
			}
			if (ends[0] == ends[1] || spans.overlap(ends[0], ends[1]))
				continue;
			const auto keep = std::min(ends[0], ends[1]), gone = std::max(ends[0], ends[1]);
			auto& kept = spans.list[spans.index[keep]];
			auto& lost = spans.list[spans.index[gone]];
			Segments joined;
			joined.reserve(kept.size() + lost.size());
			std::merge(kept.begin(), kept.end(), lost.begin(), lost.end(), std::back_inserter(joined));
			kept = std::move(joined);
			lost = Segments();
			root_[gone] = keep;
		}
		first_.assign(count + 1, 0);
		for (TempId t = 0; t < count; ++t)
			if (root_[t] != kNoId)
				++first_[(root_[t] = find(t)) + 1];
		for (size_t t = 0; t < count; ++t)
			first_[t + 1] += first_[t];
		members_.resize(first_[count]);
		crosses_.assign(count, 0);
		std::vector<uint32_t> fill(first_.begin(), first_.end() - 1);
		for (TempId t = 0; t < count; ++t)
			if (root_[t] != kNoId)
			{
				members_[fill[root_[t]]++] = range_of[t];
				each_interval(ranges[range_of[t]], segments, [&](uint32_t a, uint32_t b) {
					const auto call = std::lower_bound(calls.begin(), calls.end(), a);
					crosses_[root_[t]] |= call != calls.end() && *call < b;
				});
			}
	}
	bool crosses_call(TempId root) const { return crosses_[root]; }
	// A phi whose inputs share a register without it.
	bool split(TempId id) const { return !split_.empty() && split_[id]; }
	bool empty() const { return root_.empty(); }
	TempId root(TempId id) const { return root_.empty() ? kNoId : root_[id]; }
	// The range indices of a group's members.
	template<typename F>
	void each_member(TempId root, F f) const
	{
		for (uint32_t k = first_[root]; k < first_[root + 1]; ++k)
			f(members_[k]);
	}

private:
	TempId find(TempId t)
	{
		while (root_[t] != t)
			t = root_[t] = root_[root_[t]];
		return t;
	}
	static bool overlap(const Segments& a, const Segments& b)
	{
		for (size_t i = 0, j = 0; i < a.size() && j < b.size();)
		{
			if (a[i].first <= b[j].second && b[j].first <= a[i].second)
				return true;
			if (a[i].second < b[j].second)
				++i;
			else
				++j;
		}
		return false;
	}

	// A scalar loop phi that overlaps every value it takes on a back edge, or
	// every input of the phi of a merge block that it takes, needs a copy on
	// that edge. When the copy needs its own block there, join the inputs
	// instead: every edge then makes the same copy, which the loop header
	// makes once while the branches go to it directly.
	// Sorted intervals of temps, computed once each. A group's root holds
	// the merged intervals of its members.
	struct Spans
	{
		const std::vector<LiveRange>& ranges;
		const Segments& segments;
		const std::vector<uint32_t>& range_of;
		std::vector<uint32_t> index;
		std::vector<Segments> list;
		uint32_t get(TempId t)
		{
			if (index[t] == kNoId)
			{
				index[t] = list.size();
				list.emplace_back();
				each_interval(
					ranges[range_of[t]], segments, [&](uint32_t a, uint32_t b) { list.back().push_back({ a, b }); });
				std::sort(list.back().begin(), list.back().end());
			}
			return index[t];
		}
		bool overlap(TempId a, TempId b)
		{
			const auto x = get(a), y = get(b);
			return PhiGroups::overlap(list[x], list[y]);
		}
	};

	// A scalar loop phi that overlaps every value it takes on a back edge, or
	// every input of the phi of a merge block that it takes, needs a copy on
	// that edge. When the copy needs its own block there, join the inputs
	// instead: every edge then makes the same copy, which the loop header
	// makes once while the branches go to it directly.
	void split_inputs(const FunctionBody& body, const std::vector<uint32_t>& aliases,
		const std::vector<PhiCopy>& copies, const std::vector<uint32_t>& position, Spans& spans,
		std::vector<uint32_t>& target)
	{
		const auto& range_of = spans.range_of;
		// The transfers of each phi are adjacent.
		std::vector<uint32_t> first_copy;
		const auto inputs_of = [&](TempId phi) {
			if (first_copy.empty())
			{
				first_copy.assign(body.temps.size(), kNoId);
				for (uint32_t c = copies.size(); c-- > 0;)
					first_copy[copies[c].phi] = c;
			}
			uint32_t end = first_copy[phi];
			while (end < copies.size() && copies[end].phi == phi)
				++end;
			return std::make_pair(first_copy[phi], end);
		};
		// A block of phis and its jump needs a jump for the copy, and a
		// conditional branch a block, unless the copy can run before the
		// branch; two such branches need two blocks.
		const auto needs_block = [&](BlockId block) -> uint32_t {
			const auto end = body.blocks[block].last;
			if (end == kNoId || body.instructions[end].op != Opcode::Jump)
				return end != kNoId;
			for (auto i = body.blocks[block].first; i != end; i = body.instructions[i].next)
				if (body.instructions[i].op != Opcode::Phi)
					return 0;
			return 2;
		};
		const auto back = [&](uint32_t c) { return position[copies[c].from] >= position[copies[c].to]; };
		// The header makes the copy only when its other phis need none: each
		// takes temps that can share its register.
		const auto others_join = [&](BlockId header, TempId split) {
			for (auto i = body.blocks[header].first; i != kNoId && body.instructions[i].op == Opcode::Phi;
				i = body.instructions[i].next)
			{
				const auto& phi = body.instructions[i];
				if (phi.result == split)
					continue;
				if (range_of[phi.result] == kNoId)
					return false;
				for (uint32_t k = 0; k < phi.extra_count; k += 2)
				{
					const auto v = body.operands[phi.extra_first + k + 1];
					if (!v.is(ValueKind::Temp))
						return false;
					const auto t = representative(aliases, v.index);
					if (t != phi.result && (range_of[t] == kNoId || spans.overlap(phi.result, t)))
						return false;
				}
			}
			return true;
		};
		std::vector<TempId> seen;
		for (uint32_t first = 0, last = 0; first < copies.size(); first = last)
		{
			const TempId phi = copies[first].phi;
			for (last = first; last < copies.size() && copies[last].phi == phi;)
				++last;
			if (range_of[phi] == kNoId || body.temps[phi].type.is_float() || body.temps[phi].type.is_vector())
				continue;
			// Every source needs a range, and some transfer must come from
			// before the loop and the back edges need two blocks or a jump.
			bool forward = false, valid = true;
			uint32_t block = 0;
			for (uint32_t c = first; valid && c < last; ++c)
			{
				valid = range_of[copies[c].source] != kNoId && copies[c].source != phi;
				forward |= !back(c);
				block += back(c) ? needs_block(copies[c].from) : 0;
			}
			if (!valid || !forward || block < 2)
				continue;
			// Each source counts at its first transfer.
			forward = false;
			TempId joined = kNoId;
			seen.clear();
			for (uint32_t c = first; valid && c < last; ++c)
			{
				const auto source = copies[c].source;
				if (std::find(seen.begin(), seen.end(), source) != seen.end())
					continue;
				seen.push_back(source);
				forward |= !back(c);
				if (!back(c))
					continue;
				joined = joined == kNoId ? source : joined;
				if (spans.overlap(phi, source))
					continue;
				const auto merged = inputs_of(source);
				valid = merged.first < merged.second;
				for (auto m = merged.first; valid && m < merged.second; ++m)
					valid = copies[m].to == copies[c].from && copies[m].source != phi &&
						range_of[copies[m].source] != kNoId && spans.overlap(phi, copies[m].source);
			}
			for (size_t a = 0; valid && a < seen.size(); ++a)
				for (size_t b = a + 1; valid && b < seen.size(); ++b)
					valid = !spans.overlap(seen[a], seen[b]);
			if (!valid || !forward || joined == kNoId || !others_join(copies[first].to, phi))
				continue;
			if (split_.empty())
				split_.assign(body.temps.size(), 0);
			split_[phi] = 1;
			for (uint32_t c = first; c < last; ++c)
				target[c] = joined;
		}
	}

	std::vector<TempId> root_;
	std::vector<uint32_t> first_, members_;
	std::vector<uint8_t> crosses_, split_;
};

// A straight-line function only needs the end of each register's last use.
// When a register has holes, query the same interference tree used on x86.
class RegisterBank
{
public:
	// `caller` is the bank's class of registers a call may clobber.
	RegisterBank(const Segments& segments, uint32_t slots, uint32_t caller)
		: segments_(segments), slots_(slots), caller_(caller)
	{}
	uint32_t live_at(uint32_t slot) const { return initialized_ ? tree_.regs(slot, slot) : 0; }
	// The registers that ranges hold somewhere in [a, b], and taking one there.
	uint32_t busy(uint32_t a, uint32_t b) const { return initialized_ ? tree_.regs(a, b) : ~0u; }
	void take(uint32_t a, uint32_t b, unsigned reg) { tree_.add(a, b, 1, 1u << reg); }
	// Ask later ranges without a usable preference to leave `reg` free over
	// `range`, which would remove one of its phi transfers.
	void hint(const LiveRange& range, unsigned reg)
	{
		if (segments_.empty())
			return;
		if (!hinted_)
		{
			hints_.build_regs(slots_);
			hinted_ = true;
		}
		each_interval(range, segments_, [&](uint32_t a, uint32_t b) {
			hints_.add_regs(a, b, 1u << reg);
			hint_from_[reg] = std::min(hint_from_[reg], a);
			hint_to_[reg] = std::max(hint_to_[reg], b);
		});
	}
	// Without a usable preference, a register in `partners` comes next;
	// those are the registers of the range's phi partners.
	unsigned assign(
		const LiveRange& range, uint32_t allowed, unsigned preferred, unsigned alternate = kNoId, uint32_t partners = 0)
	{
		if (!allowed)
			return kNoId;
		if (segments_.empty())
		{
			for (uint32_t regs = allowed; regs; regs &= regs - 1)
			{
				unsigned reg = __builtin_ctz(regs);
				if (until_[reg] > range.first)
					allowed &= ~(1u << reg);
			}
		}
		else
		{
			// Retries for the same range reuse its interference.
			if (&range != queried_)
			{
				if (!initialized_)
				{
					tree_.build(std::vector<int32_t>(slots_, 0));
					initialized_ = true;
				}
				busy_ = 0;
				each_interval(range, segments_, [&](uint32_t a, uint32_t b) { busy_ |= tree_.regs(a, b); });
				queried_ = &range;
			}
			allowed &= ~busy_;
		}
		if (!allowed)
			return kNoId;
		const unsigned reg = preferred != kNoId && (allowed & (1u << preferred)) ? preferred
			: alternate != kNoId && (allowed & (1u << alternate))                ? alternate
			: (allowed & partners)                                               ? __builtin_ctz(allowed & partners)
																				 : unhinted(range, allowed, partners);
		if (segments_.empty())
			until_[reg] = range.last + 1;
		else
			each_interval(range, segments_, [&](uint32_t a, uint32_t b) { tree_.add(a, b, 1, 1u << reg); });
		queried_ = nullptr;
		return reg;
	}

private:
	// The lowest register of the lowest one's class that no other range's
	// hint holds over `range`, else the lowest register.
	unsigned unhinted(const LiveRange& range, uint32_t allowed, uint32_t partners) const
	{
		const unsigned lowest = __builtin_ctz(allowed);
		if (!hinted_)
			return lowest;
		uint32_t open = allowed & ((1u << lowest) & caller_ ? caller_ : ~caller_);
		// Only registers hinted somewhere within the range's extent can be
		// hinted over it.
		uint32_t from = ~0u, to = 0, near = 0;
		each_interval(range, segments_, [&](uint32_t a, uint32_t b) {
			from = std::min(from, a);
			to = std::max(to, b);
		});
		for (uint32_t regs = open & ~partners; regs; regs &= regs - 1)
		{
			const unsigned reg = __builtin_ctz(regs);
			if (hint_from_[reg] <= to && hint_to_[reg] >= from)
				near |= 1u << reg;
		}
		if (near)
			each_interval(
				range, segments_, [&](uint32_t a, uint32_t b) { open &= ~(hints_.regs(a, b) & ~partners & near); });
		return open ? __builtin_ctz(open) : lowest;
	}

	const Segments& segments_;
	const LiveRange* queried_ = nullptr;
	uint32_t slots_, caller_, until_[32] = {}, busy_ = 0, hint_to_[32] = {};
	uint32_t hint_from_[32] = { ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u,
		~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u };
	machine::PressureTree tree_, hints_;
	bool initialized_ = false, hinted_ = false;
};
// The one block outside a loop that enters its header (the preheader, or a
// block that also branches elsewhere), or kNoId.
BlockId loop_entry(const opt::Cfg& cfg, const opt::LoopForest& forest, uint32_t loop)
{
	const auto& l = forest.loops[loop];
	if (l.preheader != kNoId)
		return l.preheader;
	BlockId entry = kNoId;
	for (uint32_t p = cfg.pred_first[l.header]; p < cfg.pred_first[l.header + 1]; ++p)
	{
		uint32_t in = forest.innermost[cfg.preds[p]];
		while (in != kNoId && in != loop)
			in = forest.loops[in].parent;
		if (in == loop)
			continue;
		if (entry != kNoId && entry != cfg.preds[p])
			return kNoId;
		entry = cfg.preds[p];
	}
	return entry;
}

// A division by a literal that constant_division selects needs its
// multiplier, and a remainder its divisor, in registers. In a loop each is
// built once at the start of the block that enters the loop when one
// register is free there and over every block of the loop: a caller register
// in a loop without calls, or a preserved register the function saves anyway.
// The divisions of one loop share equal constants.
void hold_division_constants(const FunctionBody& body, opt::Cfg& cfg, RegisterBank& bank, uint32_t caller,
	uint32_t saved, std::vector<HeldConstant>& held, std::vector<std::pair<InstId, uint32_t>>& divisions)
{
	opt::LoopForest forest;
	std::vector<uint32_t> first, last, busy, holder, entries;
	// The registers held over a loop's entry and blocks, and taking one there.
	const auto held_over = [&](const opt::Loop& l, BlockId entry) {
		uint32_t regs = bank.busy(first[entry], last[entry]);
		for (BlockId member : l.blocks)
			regs |= bank.busy(first[member], last[member]);
		return regs;
	};
	const auto take_over = [&](const opt::Loop& l, BlockId entry, unsigned reg) {
		bank.take(first[entry], last[entry], reg);
		for (BlockId member : l.blocks)
			bank.take(first[member], last[member], reg);
	};
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			uint64_t values[2];
			if (!division_constants(body.instructions[i], body, values[0], values[1]))
				continue;
			if (first.empty())
			{
				if (cfg.dom_in.size() != cfg.block_count)
					opt::complete_cfg(cfg);
				opt::find_loops(body, cfg, forest);
				block_positions(body, first, last);
				busy.assign(forest.loops.size(), 0);
				entries.assign(forest.loops.size(), kNoId - 1);
			}
			const uint32_t loop = forest.innermost[b];
			if (loop == kNoId)
				continue;
			const auto& l = forest.loops[loop];
			if (entries[loop] == kNoId - 1)
			{
				const BlockId entry = loop_entry(cfg, forest, loop);
				entries[loop] = entry != kNoId && last[entry] ? entry : kNoId;
				if (entries[loop] == kNoId)
					continue;
				// A constant built at the start of the entry block lives through its calls too.
				bool calls = false;
				const auto calling = [&](BlockId member) {
					for (InstId k = body.blocks[member].first; k != kNoId; k = body.instructions[k].next)
						calls |= body.instructions[k].op == Opcode::Call;
				};
				calling(entry);
				for (BlockId member : l.blocks)
					calling(member);
				busy[loop] = ~(saved | (calls ? 0 : caller)) | held_over(l, entry);
			}
			if (entries[loop] == kNoId)
				continue;
			uint32_t regs = 0;
			for (unsigned k = 0; k < 2; ++k)
			{
				unsigned reg = 255;
				for (uint32_t h = 0; h < held.size() && reg == 255 && values[k]; ++h)
					if (holder[h] == loop && held[h].value == values[k])
						reg = held[h].reg;
				if (reg == 255 && values[k] && ~busy[loop])
				{
					reg = __builtin_ctz(~busy[loop]);
					busy[loop] |= 1u << reg;
					take_over(l, entries[loop], reg);
					held.push_back(HeldConstant{ entries[loop], reg, values[k] });
					holder.push_back(loop);
				}
				regs |= reg << (8 * k);
			}
			if (regs != 0xffff)
				divisions.push_back(std::make_pair(i, regs));
		}
	std::sort(divisions.begin(), divisions.end());
}

// The phi groups' registers as ranges choose theirs. A group's first member
// to get a register asks the later ones, and the ranges assigned in between,
// to leave it to them.
class GroupRegisters
{
public:
	GroupRegisters(const FunctionBody& body, const std::vector<uint32_t>& aliases, const std::vector<LiveRange>& ranges,
		const Segments& segments, const std::vector<PhiCopy>& copies, const std::vector<uint32_t>& calls)
		: ranges_(ranges), groups_(body, aliases, ranges, segments, copies, calls),
		  regs_(groups_.empty() ? 0 : body.temps.size(), kNoId), assigned_(groups_.empty() ? 0 : ranges.size())
	{}
	void start(uint32_t index)
	{
		if (!assigned_.empty())
			assigned_[index] = 1;
	}
	// A member takes its group's register first. The first member of a group
	// that crosses a call, unless it is a parameter, tries the registers
	// `first_choice` gives for the group before the others.
	template<typename FirstChoice>
	unsigned choose(RegisterBank& bank, const LiveRange& range, uint32_t allowed, FirstChoice first_choice,
		unsigned preferred, unsigned alternate, bool across_call, bool parameter)
	{
		const auto group = groups_.root(range.id);
		const unsigned group_reg = group == kNoId ? kNoId : regs_[group];
		if (groups_.split(range.id))
			preferred = alternate = kNoId;
		uint32_t partners = 0;
		// A member whose phi partner in the group already holds another
		// register keeps that partner's register first.
		const bool apart = preferred != kNoId && preferred != group_reg && range.affinity != kNoId &&
			groups_.root(range.affinity) == group;
		if (group_reg != kNoId && apart)
			partners = 1u << group_reg;
		else if (group_reg != kNoId)
		{
			partners = preferred < 32 ? 1u << preferred : 0;
			preferred = group_reg;
		}
		unsigned reg = kNoId;
		if (group != kNoId && group_reg == kNoId && !across_call && !parameter && groups_.crosses_call(group))
			reg = bank.assign(range, first_choice(group), preferred, alternate, partners);
		if (reg == kNoId)
			reg = bank.assign(range, allowed, preferred, alternate, partners);
		if (reg != kNoId && group != kNoId && group_reg == kNoId)
		{
			regs_[group] = reg;
			groups_.each_member(group, [&](uint32_t index) {
				if (!assigned_[index])
					bank.hint(ranges_[index], reg);
			});
		}
		return reg;
	}
	// A floating member chooses among SIMD registers by the same rule, with
	// the groups' SIMD registers kept apart from their general ones.
	unsigned choose_simd(RegisterBank& bank, const LiveRange& range, uint32_t allowed, unsigned preferred,
		unsigned alternate, bool across_call)
	{
		if (simd_regs_.size() != regs_.size())
			simd_regs_.assign(regs_.size(), kNoId);
		regs_.swap(simd_regs_);
		const auto reg = choose(
			bank, range, allowed, [](TempId) { return 0u; }, preferred, alternate, across_call, false);
		regs_.swap(simd_regs_);
		return reg;
	}
	// Whether `rare` holds for every member that crosses a call, found once
	// for each group.
	template<typename Rare>
	bool rare_calls_only(TempId root, const Segments& segments, const std::vector<uint32_t>& calls, Rare rare)
	{
		if (rare_.empty())
			rare_.assign(regs_.size(), 0);
		if (!rare_[root])
		{
			bool only = true;
			groups_.each_member(root, [&](uint32_t index) {
				bool crosses = false;
				each_interval(ranges_[index], segments, [&](uint32_t a, uint32_t b) {
					const auto call = std::lower_bound(calls.begin(), calls.end(), a);
					crosses |= call != calls.end() && *call < b;
				});
				only = only && (!crosses || rare(ranges_[index]));
			});
			rare_[root] = only ? 1 : 2;
		}
		return rare_[root] == 1;
	}

	// Whether a member is live within the spans, found once for each group.
	bool member_within(TempId root, const Segments& segments, const std::vector<uint32_t>& spans)
	{
		if (spans.empty())
			return false;
		if (within_.empty())
			within_.assign(regs_.size(), 0);
		if (!within_[root])
		{
			bool live = false;
			groups_.each_member(
				root, [&](uint32_t index) { live = live || live_within(ranges_[index], segments, spans); });
			within_[root] = live ? 1 : 2;
		}
		return within_[root] == 1;
	}

private:
	const std::vector<LiveRange>& ranges_;
	PhiGroups groups_;
	std::vector<uint32_t> regs_, simd_regs_;
	std::vector<uint8_t> assigned_, rare_, within_;
};

} // namespace

// A scalar dying at a call may already occupy its outgoing register. The
// argument layout never reuses a register between parameters; intersect the
// permitted registers over every occurrence, including indirect call targets.
std::vector<Selector::CallRegisters> Selector::call_registers() const
{
	std::vector<CallRegisters> masks;
	if (calls_.empty())
		return masks;
	masks.resize(body_.temps.size());
	for (InstId i = 0; i < call_indices_.size(); ++i)
	{
		if (call_indices_[i] == kNoId)
			continue;
		const auto& inst = body_.instructions[i];
		const auto& layout = calls_[call_indices_[i]];
		if (inst.ops[0].is(ValueKind::Temp))
			masks[representative(aliases_, inst.ops[0].index)].allowed = 0;
		for (uint32_t n = 0; n < layout.args.size(); ++n)
		{
			const auto src = body_.operands[inst.extra_first + n];
			if (!src.is(ValueKind::Temp))
				continue;
			const auto& arg = layout.args[n];
			const auto type = layout.types[n];
			const bool simd = type.is_float() || type.kind() == TypeKind::V128;
			const bool matches = !arg.indirect && arg.pieces == 1 && arg.first < 8 &&
				arg.bank == (simd ? Bank::Simd : Bank::General) &&
				(!memory_value(type) || type.kind() == TypeKind::V128);
			auto& choice = masks[representative(aliases_, src.index)];
			if (!layout.parallel)
				choice.allowed &= matches ? 1u << arg.first : 0;
			if (matches && choice.preferred == 8)
				choice.preferred = arg.first;
		}
	}
	return masks;
}

// Incoming objects can go straight into their named slots when the entry
// starts with whole-object initializations. On a backedge the LowIR copy
// must execute again. Slots that share frame storage may have an earlier owner.
void Selector::parameter_homes(opt::Cfg& cfg)
{
	if (body_.layout.empty() || !body_.code_labels.empty() || slots_.empty())
		return;
	const BlockId entry = body_.layout.front();
	InstId previous = kNoId;
	for (InstId i = body_.blocks[entry].first; i != kNoId; previous = i, i = body_.instructions[i].next)
	{
		const auto& inst = body_.instructions[i];
		if (inst.op == Opcode::Addr)
			continue;
		const uint32_t slot = machine::parameter_copy_slot(body_, inst, uses_);
		if (slot == kNoId)
			break;
		const auto id = inst.ops[0].index;
		const Type type = body_.temps[id].type;
		if (slot == result_slot_ || body_.slots[slot].type != type || data_alignment(type) > 16 ||
			std::count(slots_.begin(), slots_.end(), slots_[slot]) != 1 ||
			std::find(homes_.begin(), homes_.begin() + body_.parameter_count, slots_[slot]) !=
				homes_.begin() + body_.parameter_count)
			break;
		if (entry_elided_.empty() &&
			(body_.layout.size() != 1 || body_.instructions[body_.blocks[entry].last].op != Opcode::Return))
		{
			if (cfg.pred_first.empty())
				opt::build_cfg_light(program_, body_, cfg, true);
			if (cfg.pred_count(entry))
				break;
		}
		homes_[id] = slots_[slot];
		const Value dst = inst.ops[1];
		if (previous != kNoId && body_.instructions[previous].op == Opcode::Addr && dst.is(ValueKind::Temp) &&
			body_.temps[dst.index].def == previous && uses_[dst.index] == 1)
			entry_elided_.push_back(previous);
		entry_elided_.push_back(i);
	}
}

// A private destination can take the result directly. When the whole
// copy immediately follows a register return, the store is at the same
// point even if the callee could read the destination.
void Selector::call_result_homes()
{
	if (calls_.empty() || body_.slots.empty())
		return;
	std::vector<uint8_t> escaped;
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& call = body_.instructions[i];
			const auto slot = machine::call_result_copy_slot(body_, call, uses_);
			if (slot != kNoId && out_.exceptions && site(call) && opt::may_unwind(program_, call))
				continue;
			if (slot == kNoId || body_.slots[slot].type != call.type)
				continue;
			const Argument returned = classify(call.type, program_.call_boundary(call)->boundary.result_shape);
			const bool simd = level_ && returned.bank == Bank::Simd &&
				storage_bytes(call.type) == returned.pieces * returned.piece_bytes;
			if (!returned.indirect && !simd)
				continue;
			if (!simd || body_.instructions[call.next].op != Opcode::CopyObject)
			{
				if (escaped.empty())
					escaped = opt::escaped_slots(body_, true);
				if (escaped[slot])
					continue;
			}
			if (call_result_slots_.empty())
				call_result_slots_.resize(body_.temps.size(), kNoId);
			call_result_slots_[call.result] = slot;
		}
}

void Selector::protect_simd_at_calls(unsigned reg, uint64_t calls)
{
	if (simd_call_spills_.empty())
		simd_call_spills_.resize(32, kNoId);
	if (simd_call_spills_[reg] == kNoId)
		simd_call_spills_[reg] = reserve(8, 8);
	for (; calls; calls &= calls - 1)
		calls_[__builtin_ctzll(calls)].saved_simd |= 1u << reg;
}

uint64_t Selector::cold_crossings(
	const LiveRange& range, const Segments& segments, const std::vector<uint32_t>& calls, uint64_t budget) const
{
	uint64_t crossed = 0, cost = 0;
	each_interval(range, segments, [&](uint32_t a, uint32_t b) {
		for (auto it = std::lower_bound(calls.begin(), calls.end(), a); it != calls.end() && *it < b; ++it)
		{
			const auto k = static_cast<unsigned>(it - calls.begin());
			crossed |= uint64_t(1) << k;
			if (!block_weights_.empty())
				cost += block_weights_[calls_[k].block];
		}
	});
	return cost <= budget ? crossed : 0;
}

// A scalar temp's home, marked for forward_reloads (reloads.cpp).
uint32_t Selector::spill_home(uint32_t offset)
{
	if (spill_homes_.size() <= offset / 8)
		spill_homes_.resize(offset / 8 + 1);
	spill_homes_[offset / 8] = 1;
	return offset;
}

// A fixed frame address is one add at the use. Computing it twice can
// cost less than saving a register across a call.
bool Selector::rematerialize_frame_address(TempId id)
{
	const InstId definition = body_.temps[id].def;
	if (uses_[id] > 2 || definition == kNoId)
		return false;
	const auto& def = body_.instructions[definition];
	if (def.op != Opcode::Addr || !def.ops[0].is(ValueKind::Slot))
		return false;
	const SlotId slot = def.ops[0].index;
	if (slot == result_slot_ || data_alignment(body_.slots[slot].type) > 16)
		return false;
	if (remat_slots_.empty())
		remat_slots_.resize(body_.temps.size(), kNoId);
	remat_slots_[id] = slot;
	return true;
}

void Selector::allocate()
{
	const size_t count = body_.temps.size();
	registers_.assign(count, kNoId);
	vectors_.assign(count, kNoId);
	homes_.assign(count, kNoId);
	auto home = [&](TempId id) {
		const auto type = body_.temps[id].type;
		return reserve((storage_bytes(type) + 7) & ~uint64_t(7), std::max(8u, data_alignment(type)));
	};
	if (!level_)
	{
		plan_slots();
		if (dynamic_stack_)
			preserved_.push_back({ 28, reserve(8, 8) });
		for (TempId id = 0; id < count; ++id)
			homes_[id] = home(id);
		return;
	}
	std::vector<uint32_t> calls;
	Segments segments;
	opt::Cfg cfg;
	machine::PhiWebs webs;
	RangeFacts facts;
	aliases_ = scalar_aliases(body_);
	auto ranges = live_ranges(program_, body_, out_.exceptions.get(), uses_, calls, segments, cfg, webs, entry_regs_,
		aliases_, level_ >= 3, facts);
	std::vector<uint32_t> web_regs(webs.empty() ? 0 : count, kNoId);
	std::stable_sort(
		ranges.begin(), ranges.end(), [](const LiveRange& a, const LiveRange& b) { return a.first < b.first; });
	// x9-x11, x15-x17 and v16-v17 are selector scratch; x18 is the platform
	// register on Darwin.
	bool free_x8 = !named_.darwin && !result_.indirect && !out_.exceptions;
	const auto incoming = incoming_registers(program_, signature_, body_, named_.darwin, free_x8);
	const uint32_t slots = 2 * (body_.instructions.size() + body_.layout.size()) + 3;
	const auto outgoing = call_registers();
	std::vector<uint64_t> weight;
	// x0-x7, x12-x14, x8 when free, x18 off Darwin and the preserved registers.
	const unsigned preserved_count = dynamic_stack_ ? 9 : 10;
	const unsigned assignable = preserved_count + 11 + (free_x8 ? 1 : 0) + (named_.darwin ? 0 : 1);
	const auto order = allocation_order(program_, body_, cfg, aliases_, block_weights_, ranges, segments, calls, webs,
		slots, level_, preserved_count, assignable, weight);
	const bool split_calls = level_ >= 2 && !named_.darwin && !dynamic_stack_ && body_.code_labels.empty() &&
		calls.size() <= 64;
	if (split_calls && calls.size() > 1 && block_weights_.empty() && !cfg.succ_first.empty())
		block_frequencies(program_, body_, cfg, block_weights_);
	const bool single = split_calls && calls.size() == 1 && guarded_call(body_, cfg, calls_.front().block);
	const bool cold_calls = split_calls && (single || !block_weights_.empty());
	// A guarded single call, or calls that together run at most half as often
	// as the function, cost less to save around than a preserved register.
	const uint64_t cold_budget = single ? ~uint64_t(0)
										: (block_weights_.empty() ? 0 : block_weights_[body_.layout.front()] / 2);
	GroupRegisters groups(body_, aliases_, ranges, segments, facts.copies, calls);
	// The entry's frameless region, found when a group first asks.
	bool frameless_known = minimize_size_ || named_.darwin || out_.exceptions || dynamic_stack_ ||
		!body_.code_labels.empty() || calls.empty();
	std::vector<uint32_t> frameless_blocks;
	constexpr uint32_t high_caller_simd = 0xfffc0000; // v18 through v31, clear of argument and scratch registers.
	RegisterBank general(segments, slots, (1u << 19) - 1), simd(segments, slots, ~kPreservedSimd);
	std::vector<uint32_t> simd_web_regs;
	uint32_t saved_fp = 0;
	uint32_t saved = dynamic_stack_ ? 1u << 28 : 0;
	// A group whose members cross only rare calls saves caller registers
	// around them, so it starts in a preserved register only when one is
	// saved already. A group with a member in the entry's frameless region
	// does not start in one.
	const auto rare = [&](const LiveRange& range) {
		return !live_into(range, segments, facts.landings) && cold_crossings(range, segments, calls, cold_budget);
	};
	const auto first_choice = [&](TempId group) {
		const uint32_t all = (dynamic_stack_ ? 0x1ffu : 0x3ffu) << 19;
		if (!frameless_known)
		{
			frameless_blocks = frameless_spans(body_, cfg);
			frameless_known = true;
		}
		if (groups.member_within(group, segments, frameless_blocks))
			return 0u;
		return cold_calls && groups.rare_calls_only(group, segments, calls, rare) ? all & saved : all;
	};
	// A floating value that finds no SIMD register takes a general one only
	// after every integer and address value has chosen: there it trades a
	// reload for a move at each use, which an integer value saves outright.
	std::vector<uint32_t> deferred;
	for (uint32_t i = 0; i < ranges.size() + deferred.size(); ++i)
	{
		const bool second = i >= ranges.size();
		const uint32_t index = second ? deferred[i - ranges.size()] : order.empty() ? i : order[i];
		const auto& range = ranges[index];
		if (range.first == kNoId)
			continue;
		if (!second)
			groups.start(index);
		bool at_call = false, across_call = false;
		uint32_t kept = !out_.exceptions && !leaf_registers_.empty() ? kLeafKept : 0;
		if (!calls.empty())
			each_interval(range, segments, [&](uint32_t a, uint32_t b) {
				const auto call = std::lower_bound(calls.begin(), calls.end(), a);
				at_call |= call != calls.end() && *call <= b;
				across_call |= call != calls.end() && *call < b;
				for (auto it = call; kept && it != calls.end() && *it <= b; ++it)
					kept &= calls_[it - calls.begin()].preserved_callers;
			});
		const auto type = body_.temps[range.id].type;
		if (across_call && type.is_pointer() && rematerialize_frame_address(range.id))
			continue;
		uint32_t arguments = at_call
			? (!across_call && outgoing[range.id].preferred < 8 ? outgoing[range.id].allowed : 0)
			: 0xff;
		if (range.first == 0)
			arguments &= range.id < incoming.size() && incoming[range.id] != kNoId ? 1u << incoming[range.id] : 0;
		if (type.is_float() && !memory_value(type) && !second)
		{
			// Both Arm ABIs preserve the low 64 bits of v8 through v15.
			const auto preferred = range.affinity == kNoId ? kNoId : vectors_[range.affinity];
			uint32_t allowed = across_call ? kPreservedSimd : arguments | high_caller_simd;
			// Try a register from the phi web when the direct preference
			// is unavailable, as in the general register bank.
			const auto root = range.affinity == kNoId ? kNoId : webs.root(range.id);
			if (root != kNoId && simd_web_regs.empty())
				simd_web_regs.assign(count, kNoId);
			const auto alternate = root == kNoId ? kNoId : simd_web_regs[root];
			if (!across_call && range.first != 0)
				allowed |= saved_fp & ((preferred < 32 ? 1u << preferred : 0) | (alternate < 32 ? 1u << alternate : 0));
			auto reg = groups.choose_simd(simd, range, allowed, preferred, alternate, across_call);
			if (reg == kNoId && !across_call)
				reg = groups.choose_simd(simd, range, kPreservedSimd, preferred, alternate, across_call);
			// Keep scalar arithmetic in its register bank. With only rare
			// calls to cross, save the live scalar lanes on those paths.
			if (reg == kNoId && across_call && cold_calls && !live_into(range, segments, facts.landings))
			{
				const uint64_t crossed = cold_crossings(range, segments, calls, cold_budget);
				if (crossed)
					reg = simd.assign(range, high_caller_simd, preferred, alternate);
				if (reg != kNoId)
					protect_simd_at_calls(reg, crossed);
			}
			if (reg != kNoId)
			{
				vectors_[range.id] = reg;
				const uint32_t preserved = (1u << reg) & kPreservedSimd;
				if (root != kNoId && (simd_web_regs[root] == kNoId || preserved))
					simd_web_regs[root] = reg;
				saved_fp |= preserved;
				continue;
			}
			deferred.push_back(index);
			continue;
		}
		// The incoming SIMD argument does not own the corresponding GPR.
		if (second && (range.first == 0 || at_call))
			arguments = 0;
		if (memory_value(type))
		{
			// Argument registers are free between calls. A quadword
			// that crosses a call must be spilled, including its high half.
			if (type.kind() == TypeKind::V128 && !range.vector_address && !across_call)
			{
				const auto preferred = range.affinity == kNoId ? kNoId : vectors_[range.affinity];
				auto reg = simd.assign(range, arguments | high_caller_simd, preferred);
				if (reg == kNoId)
				{
					reg = simd.assign(range, kPreservedSimd, preferred);
					if (reg != kNoId)
						saved_fp |= 1u << reg;
				}
				vectors_[range.id] = reg;
			}
			continue;
		}
		if (range.first == 0)
			kept &= ~0xffu | (range.id < incoming.size() && incoming[range.id] < 8 ? 1u << incoming[range.id] : 0);
		// Linux leaves x18 to the program as a temporary; Darwin reserves it.
		uint32_t caller = across_call
			? kept
			: arguments | (7u << 12) | (!at_call && free_x8 ? 1u << 8 : 0) | (named_.darwin ? 0 : 1u << 18);
		// Keep an incoming scalar in its ABI register when its live range
		// crosses only cold calls. The saved slot belongs to this register.
		// A value a landing pad reads stays in its home at every call.
		uint64_t crossed = across_call && cold_calls && !live_into(range, segments, facts.landings)
			? cold_crossings(range, segments, calls, cold_budget)
			: 0;
		uint32_t crossing = 7u << 12;
		if (across_call && range.id < incoming.size() && incoming[range.id] < 8)
			crossing |= outgoing[range.id].allowed & (1u << incoming[range.id]);
		else if (across_call && range.first != 0)
			crossing |= outgoing[range.id].allowed & 0xffu;
		if (crossed)
			caller |= crossing;
		const uint32_t preserved = (dynamic_stack_ ? 0x1ffu : 0x3ffu) << 19;
		unsigned preferred = range.affinity == kNoId ? kNoId : registers_[range.affinity];
		if (preferred == kNoId && at_call && !across_call && outgoing[range.id].preferred < 8)
			preferred = outgoing[range.id].preferred;
		const auto root = range.affinity == kNoId ? kNoId : webs.root(range.id);
		const unsigned alternate = root == kNoId ? kNoId : web_regs[root];
		const bool parameter = range.id < body_.parameter_count;
		auto reg = groups.choose(
			general, range, caller | preserved, first_choice, preferred, alternate, across_call, parameter);
		// Out of registers, saving a caller register around the calls the
		// range crosses can still cost less than its spills and reloads.
		if (reg == kNoId && across_call && !crossed && cold_calls && !weight.empty() &&
			!live_into(range, segments, facts.landings))
		{
			crossed = cold_crossings(range, segments, calls, weight[range.id] / 2);
			if (crossed)
				reg = groups.choose(
					general, range, crossing, [](TempId) { return 0u; }, preferred, alternate, across_call, parameter);
			if (reg == kNoId)
				crossed = 0;
		}
		registers_[range.id] = reg;
		if (across_call && reg < 15 && !(kept & (1u << reg)))
		{
			if (argument_spills_.empty())
				argument_spills_.resize(15, kNoId);
			if (argument_spills_[reg] == kNoId)
				argument_spills_[reg] = reserve(8, 8);
			for (; crossed; crossed &= crossed - 1)
				calls_[__builtin_ctzll(crossed)].saved_args |= 1u << reg;
		}
		if (reg != kNoId && reg >= 19)
			saved |= 1u << reg;
		if (root != kNoId && reg != kNoId && (web_regs[root] == kNoId || (reg >= 19 && web_regs[root] < 19)))
			web_regs[root] = reg;
	}
	if (level_ >= 3 && !segments.empty() && body_.code_labels.empty())
		hold_division_constants(body_, cfg, general,
			0xffu | (7u << 12) | (free_x8 ? 1u << 8 : 0) | (named_.darwin ? 0 : 1u << 18),
			saved & ((dynamic_stack_ ? 0x1ffu : 0x3ffu) << 19), held_constants_, held_divisions_);
	for (auto& entry : entry_regs_)
		entry = general.live_at(entry);
	// Keeping callee saves together lets STP and LDP share both the transfer
	// and the stack adjustment, independently of the number of local slots.
	for (; saved; saved &= saved - 1)
		preserved_.push_back({ static_cast<unsigned>(__builtin_ctz(saved)), reserve(8, 8) });
	for (; saved_fp; saved_fp &= saved_fp - 1)
		preserved_fp_.push_back({ static_cast<unsigned>(__builtin_ctz(saved_fp)), reserve(8, 8) });
	const uint32_t slot_base = frame_;
	plan_slots();
	parameter_homes(cfg);
	register_objects();
	if (!object_words_.empty())
		frame_ = reused_slot_tail(body_, slots_, object_words_, slot_base);
	call_result_homes();
	spill_homes(ranges);
}

// The frame homes of the ranges left without a register, a call result
// copy slot or a home already; then the aliases take their representatives'
// registers and homes. Ranges that never overlap share a spill slot.
void Selector::spill_homes(const std::vector<LiveRange>& ranges)
{
	const auto home = [&](TempId id) {
		const auto type = body_.temps[id].type;
		return reserve((storage_bytes(type) + 7) & ~uint64_t(7), std::max(8u, data_alignment(type)));
	};
	machine::IntervalSlots scalar_spills, vector_spills;
	for (const auto& range : ranges)
	{
		const auto id = range.id;
		const auto reg = registers_[id];
		if (range.first == kNoId || rematerialized(id) || reg != kNoId || vectors_[id] != kNoId ||
			homes_[id] != kNoId || (!call_result_slots_.empty() && call_result_slots_[id] != kNoId))
			continue;
		const auto type = body_.temps[id].type;
		const auto def = body_.temps[id].def;
		if (type.is_object() && def != kNoId && load_returns_directly(body_.instructions[def]))
			continue;
		if (!memory_value(type))
			homes_[id] = spill_home(scalar_spills.assign(range.first, range.last, [&]() { return reserve(8, 8); }));
		else if (type.kind() == TypeKind::V128 && !range.vector_address)
			homes_[id] = vector_spills.assign(range.first, range.last, [&]() { return home(id); });
		else
			homes_[id] = home(id);
	}
	for (TempId id = 0; id < aliases_.size(); ++id)
		if (aliases_[id] != id)
		{
			if (!remat_slots_.empty())
				remat_slots_[id] = remat_slots_[aliases_[id]];
			registers_[id] = registers_[aliases_[id]];
			homes_[id] = homes_[aliases_[id]];
			if (body_.temps[id].type.is_vector())
				vectors_[id] = vectors_[aliases_[id]];
		}
}
} // namespace aarch64
