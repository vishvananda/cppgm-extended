#include "opt/call_graph.h"

#include <algorithm>
#include <cstring>

#include "opt/function_ir.h"

namespace opt {

using namespace lowir;

namespace {

bool is_definition(const Program& program, SymbolId s)
{
	const Symbol& symbol = program.symbols[s];
	return symbol.kind == SymbolKind::FunctionDefinition && program.functions[symbol.detail].body != nullptr;
}

// Marks function symbols named by globals' initializers and aliases.
template<typename Mark>
void mark_data_references(const Program& program, Mark mark)
{
	for (SymbolId s = 0; s < program.symbols.size(); ++s)
	{
		const Symbol& symbol = program.symbols[s];
		if (symbol.kind != SymbolKind::GlobalDefinition)
			continue;
		const Global& global = program.globals[symbol.detail];
		if (global.init == GlobalInit::Address)
			mark(global.address);
		if (global.init == GlobalInit::Data)
			for (uint32_t k = 0; k < global.item_count; ++k)
			{
				const DataItem& item = program.data_items[global.first_item + k];
				if (item.kind == DataItemKind::Address)
					mark(item.address);
			}
	}
	// An object alias is another name for its target's code: it keeps a
	// target alive only when the target is kept for its own sake (pruning
	// drops the aliases of what it removes). A jump thunk
	// (Program::jump_thunks) keeps its target alive as any function does,
	// through its body's forwarding call: liveness reads only what LowIR
	// text carries.
}

} // namespace

void build_call_graph(const Program& program, CallGraph& graph)
{
	graph.functions.clear();
	graph.index_of.assign(program.symbols.size(), kNoId);
	for (SymbolId s : program.order)
		if (is_definition(program, s) && graph.index_of[s] == kNoId)
		{
			graph.index_of[s] = static_cast<uint32_t>(graph.functions.size());
			graph.functions.push_back(s);
		}
	const uint32_t n = static_cast<uint32_t>(graph.functions.size());
	graph.callee_first.assign(n + 1, 0);
	graph.callees.clear();
	graph.calls_in.assign(n, 0);
	graph.observed.assign(n, 0);
	auto observe = [&](SymbolId s) {
		if (s < graph.index_of.size() && graph.index_of[s] != kNoId)
			graph.observed[graph.index_of[s]] = 1;
	};
	for (uint32_t f = 0; f < n; ++f)
	{
		graph.callee_first[f] = static_cast<uint32_t>(graph.callees.size());
		const Symbol& symbol = program.symbols[graph.functions[f]];
		if (symbol.metadata.role != ir_model::SR_NONE || (symbol.metadata.flags & SF_OBJECT_ROOT) != 0 ||
			(symbol.metadata.flags & SF_REPLACEABLE) != 0)
			graph.observed[f] = 1;
		const FunctionBody& body = *program.functions[symbol.detail].body;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			{
				const Instruction& inst = body.instructions[i];
				uint32_t position = 0;
				for_each_operand(body, inst, [&](const Value& v) {
					const uint32_t at = position++;
					if (!v.is(ValueKind::Symbol))
						return;
					if (inst.op == Opcode::Call && at == 0 && graph.index_of[v.index] != kNoId)
					{
						graph.callees.push_back(graph.index_of[v.index]);
						++graph.calls_in[graph.index_of[v.index]];
					}
					else
						observe(v.index);
				});
			}
	}
	graph.callee_first[n] = static_cast<uint32_t>(graph.callees.size());
	// Distinct calling functions per callee (a function's sites count once).
	graph.callers_in.assign(n, 0);
	{
		std::vector<uint32_t> last(n, kNoId);
		for (uint32_t f = 0; f < n; ++f)
			for (uint32_t k = graph.callee_first[f]; k < graph.callee_first[f + 1]; ++k)
				if (last[graph.callees[k]] != f)
				{
					last[graph.callees[k]] = f;
					++graph.callers_in[graph.callees[k]];
				}
	}
	mark_data_references(program, observe);
	// Tarjan's SCCs, iteratively; components finish callees first.
	graph.component.assign(n, kNoId);
	graph.recursive.assign(n, 0);
	graph.bottom_up.clear();
	std::vector<uint32_t> number(n, kNoId), low(n, 0), stack;
	std::vector<uint8_t> on_stack(n, 0);
	std::vector<std::pair<uint32_t, uint32_t>> frames;
	uint32_t counter = 0, components = 0;
	for (uint32_t root = 0; root < n; ++root)
	{
		if (number[root] != kNoId)
			continue;
		frames.push_back(std::make_pair(root, graph.callee_first[root]));
		number[root] = low[root] = counter++;
		stack.push_back(root);
		on_stack[root] = 1;
		while (!frames.empty())
		{
			const uint32_t f = frames.back().first;
			uint32_t& next = frames.back().second;
			if (next < graph.callee_first[f + 1])
			{
				const uint32_t g = graph.callees[next++];
				if (g == f)
					graph.recursive[f] = 1;
				if (number[g] == kNoId)
				{
					number[g] = low[g] = counter++;
					stack.push_back(g);
					on_stack[g] = 1;
					frames.push_back(std::make_pair(g, graph.callee_first[g]));
				}
				else if (on_stack[g] && number[g] < low[f])
					low[f] = number[g];
				continue;
			}
			frames.pop_back();
			if (!frames.empty() && low[f] < low[frames.back().first])
				low[frames.back().first] = low[f];
			if (low[f] != number[f])
				continue;
			const size_t base = stack.size();
			size_t first = base;
			while (true)
			{
				--first;
				if (stack[first] == f)
					break;
			}
			const bool cycle = base - first > 1;
			for (size_t k = first; k < base; ++k)
			{
				const uint32_t g = stack[k];
				on_stack[g] = 0;
				graph.component[g] = components;
				if (cycle)
					graph.recursive[g] = 1;
			}
			// Members in program order within the component.
			std::vector<uint32_t> members(stack.begin() + first, stack.end());
			std::sort(members.begin(), members.end());
			graph.bottom_up.insert(graph.bottom_up.end(), members.begin(), members.end());
			stack.resize(first);
			++components;
		}
	}
}

bool discardable(const Program& program, SymbolId symbol)
{
	// A thread-local wrapper (tls_for) is an ABI entry other units call
	// without defining it.
	const SymbolMetadata& m = program.symbols[symbol].metadata;
	return (m.binding == ir_model::SBM_WEAK || m.binding == ir_model::SBM_INTERNAL) && m.role == ir_model::SR_NONE &&
		(m.flags & (SF_OBJECT_ROOT | SF_REPLACEABLE)) == 0 && m.tls_for == kNoId;
}

bool this_adjusting_thunk(const Program& program, SymbolId symbol)
{
	const NameId object = program.symbols[symbol].metadata.object;
	if (object == 0 || program.names.length(object) < 4)
		return false;
	const char* spelling = program.spelling(object);
	return std::strncmp(spelling, "_ZTh", 4) == 0 || std::strncmp(spelling, "_ZTv", 4) == 0;
}

uint32_t prune_unreachable(Program& program, OptimizerStats& stats)
{
	std::vector<uint8_t> live(program.symbols.size(), 0);
	std::vector<SymbolId> work;
	auto mark = [&](SymbolId s) {
		if (s < live.size() && !live[s])
		{
			live[s] = 1;
			if (is_definition(program, s))
				work.push_back(s);
		}
	};
	for (SymbolId s : program.order)
		if (is_definition(program, s) && !discardable(program, s))
			mark(s);
	mark_data_references(program, mark);
	while (!work.empty())
	{
		const SymbolId s = work.back();
		work.pop_back();
		const FunctionBody& body = *program.functions[program.symbols[s].detail].body;
		for (BlockId b : body.layout)
			for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
				for_each_operand(body, body.instructions[i], [&](const Value& v) {
					if (v.is(ValueKind::Symbol))
						mark(v.index);
				});
	}
	uint32_t removed = 0;
	std::vector<SymbolId> order;
	for (SymbolId s : program.order)
	{
		if (is_definition(program, s) && !live[s])
		{
			// The definition goes; nothing names the symbol any more.
			Symbol& symbol = program.symbols[s];
			program.functions[symbol.detail].body.reset();
			symbol.kind = SymbolKind::FunctionDeclaration;
			++removed;
			continue;
		}
		order.push_back(s);
	}
	program.order.swap(order);
	// A pruned jump thunk is none any more.
	size_t kept_thunks = 0;
	for (const JumpThunk& thunk : program.jump_thunks)
		if (is_definition(program, thunk.symbol))
			program.jump_thunks[kept_thunks++] = thunk;
	program.jump_thunks.resize(kept_thunks);
	if (removed != 0)
	{
		std::vector<ObjectAlias> aliases;
		for (const ObjectAlias& alias : program.aliases)
			if (program.symbols[alias.target].kind != SymbolKind::FunctionDeclaration ||
				!(alias.target < live.size() && !live[alias.target] && discardable(program, alias.target)))
				aliases.push_back(alias);
		program.aliases.swap(aliases);
	}
	stats.functions_pruned += removed;
	return removed;
}

} // namespace opt
