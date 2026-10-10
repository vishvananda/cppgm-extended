#include "opt/function_ir.h"

#include <algorithm>
#include <string>

#include "lowir/literals.h"
#include "support/flat_map.h"

namespace opt {

using namespace lowir;

bool call_may_write_memory(const Program& program, const Instruction& call)
{
	const Signature* boundary = program.call_boundary(call);
	return boundary == nullptr ||
		(boundary->boundary.effects != ir_model::CFXM_READNONE &&
			boundary->boundary.effects != ir_model::CFXM_READONLY);
}

bool may_unwind(const Program& program, const Instruction& inst)
{
	if (inst.op == Opcode::Throw || inst.op == Opcode::Resume)
		return true;
	if (inst.op != Opcode::Call)
		return false;
	const Signature* boundary = program.call_boundary(inst);
	return boundary == nullptr || boundary->boundary.unwind != ir_model::CUM_NO;
}

namespace {

void build_edges(const Program& program, const FunctionBody& body, Cfg& cfg)
{
	const uint32_t n = cfg.block_count;
	std::vector<uint32_t>& mark = cfg.scratch;
	mark.assign(n, kNoId);
	for (BlockId b : body.layout)
		cfg.flags[b] |= BF_PLACED;
	for (BlockId b = 0; b < n; ++b)
	{
		cfg.succ_first[b] = static_cast<uint32_t>(cfg.succs.size());
		cfg.handler_first[b] = static_cast<uint32_t>(cfg.handlers.size());
		if ((cfg.flags[b] & BF_PLACED) == 0 || body.blocks[b].last == kNoId)
			continue;
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (is_eh_instruction(inst.op))
				cfg.flags[b] |= BF_EH;
			if ((inst.op == Opcode::EhTry || inst.op == Opcode::EhCleanup) && inst.ops[0].is(ValueKind::Block))
				cfg.handlers.push_back(inst.ops[0].index);
		}
		const Instruction& term = body.instructions[body.blocks[b].last];
		auto add = [&](BlockId s) {
			if (mark[s] != b)
			{
				mark[s] = b;
				cfg.succs.push_back(s);
			}
		};
		for_each_successor(const_cast<FunctionBody&>(body), const_cast<Instruction&>(term), add);
	}
	cfg.succ_first[n] = static_cast<uint32_t>(cfg.succs.size());
	cfg.handler_first[n] = static_cast<uint32_t>(cfg.handlers.size());
	for (uint32_t k = 0; k < cfg.handlers.size(); ++k)
		cfg.flags[cfg.handlers[k]] |= BF_HANDLER;
	cfg.pred_first.assign(n + 1, 0);
	for (uint32_t k = 0; k < cfg.succs.size(); ++k)
		++cfg.pred_first[cfg.succs[k] + 1];
	for (uint32_t b = 0; b < n; ++b)
		cfg.pred_first[b + 1] += cfg.pred_first[b];
	std::vector<uint32_t>& fill = cfg.scratch;
	fill.assign(cfg.pred_first.begin(), cfg.pred_first.end() - 1);
	cfg.preds.assign(cfg.succs.size(), 0);
	for (BlockId b = 0; b < n; ++b)
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			cfg.preds[fill[cfg.succs[k]]++] = b;
	(void)program;
}

void build_order(const FunctionBody& body, Cfg& cfg)
{
	const uint32_t n = cfg.block_count;
	if (body.layout.empty())
		return;
	// Iterative DFS: ordinary successors, then handler targets (the
	// postorder collects in rpo, reversed at the end).
	std::vector<std::pair<BlockId, uint32_t>>& stack = cfg.scratch_stack;
	stack.clear();
	std::vector<BlockId>& post = cfg.rpo;
	post.clear();
	const BlockId entry = body.layout[0];
	cfg.flags[entry] |= BF_REACHABLE;
	stack.push_back(std::make_pair(entry, 0u));
	while (!stack.empty())
	{
		// The top block's next unvisited successor (its ranges read once
		// per resumption), else its postorder place.
		const BlockId b = stack.back().first;
		const uint32_t s0 = cfg.succ_first[b], succs = cfg.succ_first[b + 1] - s0;
		const uint32_t h0 = cfg.handler_first[b], total = succs + cfg.handler_first[b + 1] - h0;
		uint32_t k = stack.back().second;
		BlockId next = kNoId;
		while (k < total && next == kNoId)
		{
			const BlockId s = k < succs ? cfg.succs[s0 + k] : cfg.handlers[h0 + k - succs];
			++k;
			if ((cfg.flags[s] & BF_PLACED) != 0 && (cfg.flags[s] & BF_REACHABLE) == 0)
			{
				cfg.flags[s] |= BF_REACHABLE;
				next = s;
			}
		}
		if (next == kNoId)
		{
			post.push_back(b);
			stack.pop_back();
			continue;
		}
		stack.back().second = k;
		stack.push_back(std::make_pair(next, 0u));
	}
	std::reverse(cfg.rpo.begin(), cfg.rpo.end());
	cfg.rpo_index.assign(n, kNoId);
	for (uint32_t i = 0; i < cfg.rpo.size(); ++i)
		cfg.rpo_index[cfg.rpo[i]] = i;
}

// Dominator intersection over RPO numbers (Cooper, Harvey and Kennedy).
BlockId intersect(const Cfg& cfg, BlockId a, BlockId b)
{
	while (a != b)
	{
		while (cfg.rpo_index[a] > cfg.rpo_index[b])
			a = cfg.idom[a];
		while (cfg.rpo_index[b] > cfg.rpo_index[a])
			b = cfg.idom[b];
	}
	return a;
}

// Graphs up to this many reachable blocks take the iterative
// intersection (its walks are bounded by the block count squared per
// pass); larger ones Lengauer and Tarjan.
const uint32_t kIterativeDominatorBlocks = 32;

void build_dominators(Cfg& cfg)
{
	const uint32_t n = cfg.block_count;
	cfg.idom.assign(n, kNoId);
	if (cfg.rpo.size() <= 1)
		return;
	// Predecessors over both edge kinds: the ordinary ones the edges
	// already hold, and the handler edges' (rare; unreachable sources are
	// skipped below, and the immediate dominators do not depend on the
	// order predecessors are met in).
	// (The vectors persist for their capacity: each is assigned before use.)
	static std::vector<uint32_t> hfirst, hall;
	hfirst.clear();
	if (!cfg.handlers.empty())
	{
		hfirst.assign(n + 1, 0);
		for (BlockId b : cfg.rpo)
			for (uint32_t k = cfg.handler_first[b]; k < cfg.handler_first[b + 1]; ++k)
				++hfirst[cfg.handlers[k] + 1];
		for (uint32_t b = 0; b < n; ++b)
			hfirst[b + 1] += hfirst[b];
		hall.assign(hfirst[n], 0);
		static std::vector<uint32_t> fill;
		fill.assign(hfirst.begin(), hfirst.end() - 1);
		for (BlockId b : cfg.rpo)
			for (uint32_t k = cfg.handler_first[b]; k < cfg.handler_first[b + 1]; ++k)
				hall[fill[cfg.handlers[k]]++] = b;
	}
	const BlockId entry = cfg.rpo[0];
	if (cfg.rpo.size() <= kIterativeDominatorBlocks)
	{
		cfg.idom[entry] = entry;
		for (bool changed = true; changed;)
		{
			changed = false;
			for (uint32_t i = 1; i < cfg.rpo.size(); ++i)
			{
				const BlockId b = cfg.rpo[i];
				BlockId dom = kNoId;
				for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1]; ++k)
					if (cfg.idom[cfg.preds[k]] != kNoId)
						dom = dom == kNoId ? cfg.preds[k] : intersect(cfg, cfg.preds[k], dom);
				for (uint32_t k = hfirst.empty() ? 0 : hfirst[b]; k < (hfirst.empty() ? 0 : hfirst[b + 1]); ++k)
					if (cfg.idom[hall[k]] != kNoId)
						dom = dom == kNoId ? hall[k] : intersect(cfg, hall[k], dom);
				if (dom != cfg.idom[b])
				{
					cfg.idom[b] = dom;
					changed = true;
				}
			}
		}
		cfg.idom[entry] = kNoId;
		return;
	}
	// Lengauer and Tarjan with path compression, O(edges log blocks) (the
	// iterative intersection walk is quadratic on a long chain feeding one
	// merge): a depth-first preorder over both edge kinds, semidominators
	// by EVAL over the linked forest, then immediate dominators.
	static std::vector<uint32_t> number;
	static std::vector<BlockId> vertex, parent;
	static std::vector<std::pair<BlockId, uint32_t>> stack;
	number.assign(n, kNoId);
	vertex.clear();
	parent.clear();
	stack.clear();
	number[entry] = 0;
	vertex.push_back(entry);
	parent.push_back(kNoId);
	stack.push_back(std::make_pair(entry, 0u));
	while (!stack.empty())
	{
		const BlockId b = stack.back().first;
		const uint32_t k = stack.back().second++;
		const uint32_t succs = cfg.succ_first[b + 1] - cfg.succ_first[b];
		const uint32_t total = succs + cfg.handler_first[b + 1] - cfg.handler_first[b];
		if (k >= total)
		{
			stack.pop_back();
			continue;
		}
		const BlockId s = k < succs ? cfg.succs[cfg.succ_first[b] + k] : cfg.handlers[cfg.handler_first[b] + k - succs];
		if (!cfg.reachable(s) || number[s] != kNoId)
			continue;
		number[s] = static_cast<uint32_t>(vertex.size());
		vertex.push_back(s);
		parent.push_back(number[b]);
		stack.push_back(std::make_pair(s, 0u));
	}
	const uint32_t count = static_cast<uint32_t>(vertex.size());
	static std::vector<uint32_t> semi, label, ancestor, idom, bucket, next, chain;
	semi.assign(count, 0);
	label.assign(count, 0);
	ancestor.assign(count, kNoId);
	idom.assign(count, 0);
	bucket.assign(count, kNoId);
	next.assign(count, kNoId);
	chain.clear();
	for (uint32_t v = 0; v < count; ++v)
		semi[v] = label[v] = v;
	auto eval = [&](uint32_t v) {
		if (ancestor[v] == kNoId)
			return v;
		chain.clear();
		for (uint32_t x = v; ancestor[ancestor[x]] != kNoId; x = ancestor[x])
			chain.push_back(x);
		for (size_t j = chain.size(); j-- > 0;)
		{
			const uint32_t y = chain[j], a = ancestor[y];
			if (semi[label[a]] < semi[label[y]])
				label[y] = label[a];
			ancestor[y] = ancestor[a];
		}
		return label[v];
	};
	for (uint32_t w = count; w-- > 1;)
	{
		const BlockId b = vertex[w];
		auto meet = [&](BlockId p) {
			if (number[p] == kNoId)
				return;
			const uint32_t u = eval(number[p]);
			if (semi[u] < semi[w])
				semi[w] = semi[u];
		};
		for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1]; ++k)
			meet(cfg.preds[k]);
		for (uint32_t k = hfirst.empty() ? 0 : hfirst[b]; k < (hfirst.empty() ? 0 : hfirst[b + 1]); ++k)
			meet(hall[k]);
		next[w] = bucket[semi[w]];
		bucket[semi[w]] = w;
		const uint32_t p = parent[w];
		ancestor[w] = p;
		for (uint32_t v = bucket[p]; v != kNoId; v = next[v])
		{
			const uint32_t u = eval(v);
			idom[v] = semi[u] < semi[v] ? u : p;
		}
		bucket[p] = kNoId;
	}
	for (uint32_t w = 1; w < count; ++w)
	{
		if (idom[w] != semi[w])
			idom[w] = idom[idom[w]];
		cfg.idom[vertex[w]] = vertex[idom[w]];
	}
}

void build_intervals(Cfg& cfg)
{
	const uint32_t n = cfg.block_count;
	cfg.dom_in.assign(n, 0);
	cfg.dom_out.assign(n, 0);
	if (cfg.rpo.empty())
		return;
	DomChildren& tree = cfg.tree;
	build_dom_children(cfg, tree);
	uint32_t clock = 0;
	std::vector<std::pair<BlockId, uint32_t>>& stack = cfg.scratch_stack;
	stack.assign(1, std::make_pair(cfg.rpo[0], 0u));
	cfg.dom_in[cfg.rpo[0]] = clock++;
	while (!stack.empty())
	{
		const BlockId b = stack.back().first;
		const uint32_t k = stack.back().second++;
		if (tree.first[b] + k >= tree.first[b + 1])
		{
			cfg.dom_out[b] = clock++;
			stack.pop_back();
			continue;
		}
		const BlockId c = tree.children[tree.first[b] + k];
		cfg.dom_in[c] = clock++;
		stack.push_back(std::make_pair(c, 0u));
	}
}

} // namespace

void build_cfg_light(const Program& program, const FunctionBody& body, Cfg& cfg)
{
	const uint32_t n = static_cast<uint32_t>(body.blocks.size());
	cfg.block_count = n;
	cfg.succ_first.assign(n + 1, 0);
	cfg.handler_first.assign(n + 1, 0);
	cfg.succs.clear();
	cfg.succs.reserve(2 * static_cast<size_t>(n)); // (most blocks branch at most two ways)
	cfg.handlers.clear();
	cfg.flags.assign(n, 0);
	cfg.rpo.clear();
	build_edges(program, body, cfg);
	build_order(body, cfg);
}

void complete_cfg(Cfg& cfg)
{
	build_dominators(cfg);
	build_intervals(cfg);
}

void build_cfg(const Program& program, const FunctionBody& body, Cfg& cfg)
{
	build_cfg_light(program, body, cfg);
	complete_cfg(cfg);
}

void build_dom_children(const Cfg& cfg, DomChildren& out)
{
	const uint32_t n = cfg.block_count;
	out.first.assign(n + 1, 0);
	for (BlockId b : cfg.rpo)
		if (cfg.idom[b] != kNoId)
			++out.first[cfg.idom[b] + 1];
	for (uint32_t b = 0; b < n; ++b)
		out.first[b + 1] += out.first[b];
	out.children.assign(out.first[n], 0);
	// Each child at its parent's cursor, first[] (which ends at the next
	// range's start and shifts back after).
	for (BlockId b : cfg.rpo)
		if (cfg.idom[b] != kNoId)
			out.children[out.first[cfg.idom[b]]++] = b;
	for (uint32_t b = n; b > 0; --b)
		out.first[b] = out.first[b - 1];
	out.first[0] = 0;
}

void map_instruction_blocks(const FunctionBody& body, std::vector<BlockId>& block_of)
{
	block_of.assign(body.instructions.size(), kNoId);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			block_of[i] = b;
}

void count_uses(const FunctionBody& body, std::vector<uint32_t>& uses)
{
	uses.assign(body.temps.size(), 0);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			for_each_operand(body, body.instructions[i], [&](const Value& v) {
				if (v.is(ValueKind::Temp))
					++uses[v.index];
			});
}

InstId append_instruction(FunctionBody& body, const Instruction& inst)
{
	body.instructions.push_back(inst);
	body.instructions.back().next = kNoId;
	return static_cast<InstId>(body.instructions.size() - 1);
}

void copy_argument_classes(FunctionBody& body, const FunctionBody& from, InstId source, InstId copy)
{
	// (Indexes: `from` may be `body`.)
	const std::vector<ArgumentClasses>& table = from.argument_classes;
	size_t k = static_cast<size_t>(std::lower_bound(table.begin(), table.end(), source,
									   [](const ArgumentClasses& entry, InstId call) { return entry.call < call; }) -
		table.begin());
	for (; k < from.argument_classes.size() && from.argument_classes[k].call == source; ++k)
	{
		ArgumentClasses entry = from.argument_classes[k];
		entry.call = copy;
		body.argument_classes.push_back(entry);
	}
}

TempId append_temp(FunctionBody& body, Type type, InstId def)
{
	Temp temp;
	temp.name = 0;
	temp.type = type;
	temp.def = def;
	temp.kind = TempKind::Plain;
	body.temps.push_back(temp);
	return static_cast<TempId>(body.temps.size() - 1);
}

Value integer_value(FunctionBody& body, Literal bits)
{
	return literal_value(body, ValueKind::Integer, TypeKind::Void, bits);
}

Value literal_value(FunctionBody& body, ValueKind kind, TypeKind float_kind, Literal bits)
{
	body.literals.push_back(bits);
	Value value = Value::make(kind, static_cast<uint32_t>(body.literals.size() - 1));
	value.float_kind = kind == ValueKind::Float ? float_kind : TypeKind::Void;
	return value;
}

BlockId append_block(FunctionBody& body, BlockKind kind)
{
	Block block;
	block.name = 0;
	block.first = kNoId;
	block.last = kNoId;
	block.kind = kind;
	body.blocks.push_back(block);
	return static_cast<BlockId>(body.blocks.size() - 1);
}

uint32_t append_operands(FunctionBody& body, const Value* values, uint32_t count)
{
	const uint32_t first = static_cast<uint32_t>(body.operands.size());
	for (uint32_t k = 0; k < count; ++k)
		body.operands.push_back(values[k]);
	return first;
}

void insert_after(FunctionBody& body, BlockId block, InstId after, InstId inst)
{
	Block& b = body.blocks[block];
	if (after == kNoId)
	{
		body.instructions[inst].next = b.first;
		b.first = inst;
		if (b.last == kNoId)
			b.last = inst;
		return;
	}
	body.instructions[inst].next = body.instructions[after].next;
	body.instructions[after].next = inst;
	if (b.last == after)
		b.last = inst;
}

void unlink_after(FunctionBody& body, BlockId block, InstId prev, InstId inst)
{
	Block& b = body.blocks[block];
	const InstId next = body.instructions[inst].next;
	if (prev == kNoId)
		b.first = next;
	else
		body.instructions[prev].next = next;
	if (b.last == inst)
		b.last = prev;
	body.instructions[inst].next = kNoId;
}

InstId previous_in_block(const FunctionBody& body, BlockId block, InstId inst)
{
	InstId prev = kNoId;
	for (InstId i = body.blocks[block].first; i != inst; i = body.instructions[i].next)
		prev = i;
	return prev;
}

Value phi_input(FunctionBody& body, Type type, Value value)
{
	if (!type.is_float() || !value.is(ValueKind::Integer))
		return value;
	const Literal bits = body.literals[value.index];
	const __int128 v = static_cast<__int128>((static_cast<unsigned __int128>(bits.high) << 64) | bits.low);
	return literal_value(body, ValueKind::Float, type.kind(), float_literal(type.kind(), static_cast<long double>(v)));
}

void remove_phi_incoming(FunctionBody& body, BlockId block, BlockId pred)
{
	for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
	{
		Instruction& phi = body.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		Value* pairs = &body.operands[phi.extra_first];
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
		{
			if (pairs[k].index != pred)
				continue;
			for (uint32_t j = k + 2; j < phi.extra_count; ++j)
				pairs[j - 2] = pairs[j];
			phi.extra_count -= 2;
			break;
		}
	}
}

void rename_phi_incoming(FunctionBody& body, BlockId block, BlockId from, BlockId to)
{
	for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
	{
		Instruction& phi = body.instructions[i];
		if (phi.op != Opcode::Phi)
			break;
		for (uint32_t k = 0; k < phi.extra_count; k += 2)
			if (body.operands[phi.extra_first + k].index == from)
				body.operands[phi.extra_first + k].index = to;
	}
}

Value phi_incoming(const FunctionBody& body, const Instruction& phi, BlockId pred)
{
	for (uint32_t k = 0; k < phi.extra_count; k += 2)
		if (body.operands[phi.extra_first + k].index == pred)
			return body.operands[phi.extra_first + k + 1];
	return Value::none();
}

void replace_all_uses(FunctionBody& body, TempId from, Value to)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			for_each_operand(body, body.instructions[i], [&](Value& v) {
				if (v.is(ValueKind::Temp) && v.index == from)
					v = to;
			});
}

namespace {

struct NameHash
{
	size_t operator()(NameId name) const { return name; }
};

// Whether an entity's name is its spelling (not when it has a presentation
// kind: lowir/local_kinds.h).
bool explicit_name(const Temp& temp)
{
	return temp.name != 0 && temp.kind == TempKind::Plain;
}
bool explicit_name(const Slot& slot)
{
	return slot.name != 0 && slot.kind == SlotKind::Plain;
}
bool explicit_name(const Block& block)
{
	return block.name != 0 && block.kind == BlockKind::Plain;
}

// Keeps one kind of a body's explicit names distinct: a later entity
// repeating an earlier one's loses it (and prints by id; the writer keeps
// unnamed entities' spellings free of explicit names, lowir/local_names.h).
// The first `fixed` entities (parameters, which the signature also prints)
// keep theirs. O(entities), in a map sized by the body.
template<typename Entity>
void unique_names(std::vector<Entity>& entities, uint32_t fixed, FlatMap<NameId, uint8_t, NameHash>& marks)
{
	marks.clear();
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		if (!explicit_name(entities[i]))
			continue;
		if (marks.find(entities[i].name) == nullptr)
			marks.insert(entities[i].name, 1);
		else if (i >= fixed)
			entities[i].name = 0;
	}
}

} // namespace

void compact_body(Program& program, FunctionBody& body)
{
	const uint32_t parameters = body.parameter_count;
	std::vector<InstId> inst_map(body.instructions.size(), kNoId);
	std::vector<TempId> temp_map(body.temps.size(), kNoId);
	std::vector<uint32_t> slot_map(body.slots.size(), kNoId);
	std::vector<uint32_t> block_map(body.blocks.size(), kNoId);
	std::vector<uint32_t> literal_map(body.literals.size(), kNoId);
	FunctionBody out;
	out.parameter_count = parameters;
	for (TempId t = 0; t < parameters; ++t)
	{
		temp_map[t] = t;
		out.temps.push_back(body.temps[t]);
	}
	for (BlockId b : body.layout)
	{
		block_map[b] = static_cast<uint32_t>(out.blocks.size());
		Block block = body.blocks[b];
		block.first = block.last = kNoId;
		out.blocks.push_back(block);
		out.layout.push_back(block_map[b]);
	}
	auto map_value = [&](Value v) {
		switch (v.kind)
		{
		case ValueKind::Temp:
			v.index = temp_map[v.index];
			break;
		case ValueKind::Block:
			v.index = block_map[v.index];
			break;
		case ValueKind::Slot:
			if (slot_map[v.index] == kNoId)
			{
				slot_map[v.index] = static_cast<uint32_t>(out.slots.size());
				out.slots.push_back(body.slots[v.index]);
			}
			v.index = slot_map[v.index];
			break;
		case ValueKind::Integer:
		case ValueKind::Float:
			if (literal_map[v.index] == kNoId)
			{
				literal_map[v.index] = static_cast<uint32_t>(out.literals.size());
				out.literals.push_back(body.literals[v.index]);
			}
			v.index = literal_map[v.index];
			break;
		default:
			break;
		}
		return v;
	};
	// Results first, so phi operands naming later definitions map (the
	// counts size the new arrays).
	uint32_t instructions = 0, temps = parameters, operands = 0;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			inst_map[i] = instructions++;
			if (inst.result != kNoId)
				temp_map[inst.result] = temps++;
			operands += inst.extra_count;
		}
	out.instructions.reserve(instructions);
	out.temps.reserve(temps);
	out.operands.reserve(operands);
	for (BlockId b : body.layout)
	{
		Block& block = out.blocks[block_map[b]];
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction inst = body.instructions[i];
			if (inst.result != kNoId)
			{
				Temp temp = body.temps[inst.result];
				temp.def = inst_map[i];
				out.temps.push_back(temp);
				inst.result = temp_map[inst.result];
			}
			for (Value& v : inst.ops)
				v = map_value(v);
			const uint32_t first = static_cast<uint32_t>(out.operands.size());
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				out.operands.push_back(map_value(body.operands[inst.extra_first + k]));
			inst.extra_first = inst.extra_count != 0 ? first : 0;
			inst.next = kNoId;
			if (block.first == kNoId)
				block.first = inst_map[i];
			else
				out.instructions[block.last].next = inst_map[i];
			block.last = inst_map[i];
			out.instructions.push_back(inst);
		}
	}
	out.locations.swap(body.locations);
	out.argument_classes.swap(body.argument_classes);
	for (ArgumentClasses& classes : out.argument_classes)
		classes.call = classes.call < inst_map.size() ? inst_map[classes.call] : kNoId;
	std::vector<ArgumentClasses> kept;
	for (const ArgumentClasses& classes : out.argument_classes)
		if (classes.call != kNoId)
			kept.push_back(classes);
	// Renumbered in layout order (an inlined or cloned call may now come
	// before older ones): back in instruction order.
	std::stable_sort(
		kept.begin(), kept.end(), [](const ArgumentClasses& a, const ArgumentClasses& b) { return a.call < b.call; });
	out.argument_classes.swap(kept);
	// Slots nothing names any more are dropped; their storage was unused.
	FlatMap<NameId, uint8_t, NameHash> marks;
	unique_names(out.temps, parameters, marks);
	unique_names(out.slots, 0, marks);
	unique_names(out.blocks, 0, marks);
	body.temps.swap(out.temps);
	body.slots.swap(out.slots);
	body.blocks.swap(out.blocks);
	body.layout.swap(out.layout);
	body.instructions.swap(out.instructions);
	body.operands.swap(out.operands);
	body.literals.swap(out.literals);
	body.locations.swap(out.locations);
	body.argument_classes.swap(out.argument_classes);
}

bool constant_contents(const Program& program, SymbolId symbol, const std::vector<uint8_t>* unwritten)
{
	const Symbol& s = program.symbols[symbol];
	return s.kind == SymbolKind::GlobalDefinition && (s.metadata.flags & SF_REPLACEABLE) == 0 &&
		(s.metadata.storage == ir_model::GSM_READONLY ||
			(unwritten != nullptr && symbol < unwritten->size() && (*unwritten)[symbol] != 0));
}

void find_unwritten_globals(const Program& program, std::vector<uint8_t>& unwritten)
{
	const SymbolId symbols = static_cast<SymbolId>(program.symbols.size());
	unwritten.assign(symbols, 0);
	for (SymbolId s = 0; s < symbols; ++s)
	{
		const Symbol& symbol = program.symbols[s];
		unwritten[s] = symbol.kind == SymbolKind::GlobalDefinition &&
				symbol.metadata.binding == ir_model::SBM_INTERNAL && symbol.metadata.storage == ir_model::GSM_DEFAULT &&
				(symbol.metadata.flags & SF_REPLACEABLE) == 0
			? 1
			: 0;
	}
	// Another global's data holding the address.
	for (SymbolId s = 0; s < symbols; ++s)
	{
		const Symbol& symbol = program.symbols[s];
		if (symbol.kind != SymbolKind::GlobalDefinition)
			continue;
		const Global& g = program.globals[symbol.detail];
		if (g.init == GlobalInit::Address && g.address < symbols && g.address != s)
			unwritten[g.address] = 0;
		if (g.init == GlobalInit::Data)
			for (uint32_t k = 0; k < g.item_count; ++k)
			{
				const DataItem& item = program.data_items[g.first_item + k];
				if (item.kind == DataItemKind::Address && item.address < symbols && item.address != s)
					unwritten[item.address] = 0;
			}
	}
	// Each body: the values carrying a candidate's address (from `addr`,
	// through index, copy and phi, and private slots), then every use of
	// one, which must read through it.
	std::vector<SymbolId> carried, held;
	std::vector<uint8_t> taken;
	for (SymbolId f = 0; f < symbols; ++f)
	{
		const Symbol& symbol = program.symbols[f];
		if (symbol.kind != SymbolKind::FunctionDefinition || program.functions[symbol.detail].body == nullptr)
			continue;
		const FunctionBody& body = *program.functions[symbol.detail].body;
		const SymbolId kConflict = kNoId - 1;
		carried.assign(body.temps.size(), kNoId);
		held.assign(body.slots.size(), kNoId);
		taken.assign(body.slots.size(), 0);
		auto candidate = [&](Value v) -> SymbolId {
			if (v.is(ValueKind::Symbol))
				return v.index < symbols && unwritten[v.index] ? v.index : kNoId;
			if (v.is(ValueKind::Temp) && v.index < carried.size() && carried[v.index] != kConflict)
				return carried[v.index];
			return kNoId;
		};
		auto carry = [&](TempId t, SymbolId s, bool& changed) {
			if (t == kNoId || s == kNoId || carried[t] == s || carried[t] == kConflict)
				return;
			if (carried[t] != kNoId)
			{
				unwritten[carried[t]] = 0;
				unwritten[s] = 0;
				carried[t] = kConflict;
			}
			else
				carried[t] = s;
			changed = true;
		};
		// A private slot a candidate's address is stored in (two stored
		// there leave both written).
		auto hold = [&](SlotId slot, SymbolId s, bool& changed) {
			SymbolId& h = held[slot];
			if (s == kNoId || h == s || h == kConflict)
				return;
			if (h != kNoId)
			{
				unwritten[h] = unwritten[s] = 0;
				h = kConflict;
			}
			else
				h = s;
			changed = true;
		};
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				const bool direct = (inst.op == Opcode::Load && inst.ops[0].is(ValueKind::Slot)) ||
					(inst.op == Opcode::Store && inst.ops[1].is(ValueKind::Slot) && !inst.ops[0].is(ValueKind::Slot));
				for_each_operand(body, inst, [&](const Value& v) {
					if (v.is(ValueKind::Slot) && !direct)
						taken[v.index] = 1;
				});
			}
		for (unsigned round = 0; round < 8; ++round)
		{
			bool changed = false;
			for (BlockId b : body.layout)
				for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				{
					const Instruction& inst = body.instructions[i];
					switch (inst.op)
					{
					case Opcode::Addr:
					case Opcode::Copy:
					case Opcode::Convert:
						carry(inst.result, candidate(inst.ops[0]), changed);
						break;
					case Opcode::Index:
					case Opcode::Binary:
						carry(inst.result, candidate(inst.ops[0]), changed);
						carry(inst.result, candidate(inst.ops[1]), changed);
						break;
					case Opcode::Phi:
						for (uint32_t k = 1; k < inst.extra_count; k += 2)
							carry(inst.result, candidate(body.operands[inst.extra_first + k]), changed);
						break;
					case Opcode::Store:
						if (inst.ops[1].is(ValueKind::Slot) && !taken[inst.ops[1].index])
							hold(inst.ops[1].index, candidate(inst.ops[0]), changed);
						break;
					case Opcode::Load:
						if (inst.ops[0].is(ValueKind::Slot) && held[inst.ops[0].index] != kNoId &&
							held[inst.ops[0].index] != kConflict)
							carry(inst.result, held[inst.ops[0].index], changed);
						break;
					default:
						break;
					}
				}
			if (!changed)
				break;
		}
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				uint32_t position = 0;
				for_each_operand(body, inst, [&](const Value& v) {
					const uint32_t at = position++;
					const SymbolId s = candidate(v);
					if (s == kNoId)
						return;
					bool reads = false;
					switch (inst.op)
					{
					case Opcode::Load: // the address read through
					case Opcode::Compare:
					case Opcode::Addr: // the carriers (their results are checked in turn)
					case Opcode::Copy:
					case Opcode::Convert:
					case Opcode::Index:
					case Opcode::Binary:
					case Opcode::Phi:
						reads = true;
						break;
					case Opcode::CopyObject:
						reads = at == 0; // the source
						break;
					case Opcode::Store:
						reads = at == 0 && inst.ops[1].is(ValueKind::Slot) && !taken[inst.ops[1].index];
						break;
					default:
						break;
					}
					if (!reads)
						unwritten[s] = 0;
				});
			}
	}
}

bool read_readonly_address(const Program& program, SymbolId symbol, int64_t offset,
	const std::vector<uint8_t>* unwritten, SymbolId& target, int64_t* addend)
{
	if (!constant_contents(program, symbol, unwritten) || offset < 0)
		return false;
	const Global& g = program.globals[program.symbols[symbol].detail];
	if (g.init != GlobalInit::Data || g.item_count > 4096)
		return false;
	const uint64_t want = static_cast<uint64_t>(offset);
	uint64_t at = 0;
	for (uint32_t i = 0; i < g.item_count && at <= want; ++i)
	{
		const DataItem& item = program.data_items[g.first_item + i];
		if (item.kind == DataItemKind::Zero)
		{
			at += static_cast<uint64_t>(item.amount);
			continue;
		}
		const Type t = item.kind == DataItemKind::Address ? Type::scalar(TypeKind::Ptr) : item.type;
		const uint64_t bytes = storage_bytes(t);
		const uint64_t align = bytes >= 16 ? 16 : bytes == 0 ? 1 : bytes;
		at = (at + align - 1) & ~(align - 1);
		if (want == at)
		{
			if (item.kind != DataItemKind::Address || (item.amount != 0 && addend == nullptr) ||
				item.address >= program.symbols.size())
				return false;
			target = item.address;
			if (addend != nullptr)
				*addend = item.amount;
			return true;
		}
		at += bytes;
	}
	return false;
}

bool read_readonly_integer(const Program& program, SymbolId symbol, int64_t offset, Type type, Constant& out,
	const std::vector<uint8_t>* unwritten)
{
	const Symbol& s = program.symbols[symbol];
	if (!constant_contents(program, symbol, unwritten) || !type.is_integer() || offset < 0)
		return false;
	const Global& g = program.globals[s.detail];
	const uint64_t want = static_cast<uint64_t>(offset), size = storage_bytes(type);
	out.kind = ValueKind::Integer;
	out.float_kind = TypeKind::Void;
	out.bits = Literal{ 0, 0 };
	if (g.init == GlobalInit::Zero)
		return true;
	if (g.init == GlobalInit::Constant)
	{
		if (!g.typed || want != 0 || storage_bytes(g.type) != size || !g.type.is_integer() ||
			g.constant.kind != ValueKind::Integer)
			return false;
		out.bits = g.constant.bits;
		return true;
	}
	if (g.init != GlobalInit::Data || g.item_count > 4096 || size == 0 || size > 16)
		return false;
	// The bytes of [want, want + size): every one inside an integer item
	// (little-endian) or a zero run.
	uint8_t bytes_of[16] = {};
	uint32_t covered = 0;
	uint64_t at = 0;
	for (uint32_t i = 0; i < g.item_count && at < want + size; ++i)
	{
		const DataItem& item = program.data_items[g.first_item + i];
		if (item.kind == DataItemKind::Zero)
		{
			const uint64_t end = at + static_cast<uint64_t>(item.amount);
			for (uint64_t b = at > want ? at : want; b < end && b < want + size; ++b)
				++covered; // zero
			at = end;
			continue;
		}
		const Type t = item.kind == DataItemKind::Address ? Type::scalar(TypeKind::Ptr) : item.type;
		const uint64_t bytes = storage_bytes(t);
		const uint64_t align = bytes >= 16 ? 16 : bytes == 0 ? 1 : bytes;
		at = (at + align - 1) & ~(align - 1);
		if (want < at + bytes && want + size > at)
		{
			if (item.kind != DataItemKind::Scalar || !t.is_integer() || item.constant.kind != ValueKind::Integer)
				return false;
			for (uint64_t b = at > want ? at : want; b < at + bytes && b < want + size; ++b)
			{
				const uint64_t k = b - at;
				const uint64_t word = k < 8 ? item.constant.bits.low : item.constant.bits.high;
				bytes_of[b - want] = static_cast<uint8_t>(word >> (8 * (k % 8)));
				++covered;
			}
		}
		at += bytes;
	}
	if (covered != size)
		return false;
	for (uint64_t b = 0; b < size; ++b)
		(b < 8 ? out.bits.low : out.bits.high) |= static_cast<uint64_t>(bytes_of[b]) << (8 * (b % 8));
	return true;
}

uint64_t instruction_count(const FunctionBody& body)
{
	uint64_t count = 0;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			++count;
	return count;
}

} // namespace opt
