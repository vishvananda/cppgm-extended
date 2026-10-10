#include "opt/interprocedural.h"

#include "opt/function_ir.h"
#include "opt/scalar_fold.h"

namespace opt {

using namespace lowir;

namespace {

// What every direct call passes for one parameter: nothing seen yet, one
// literal or symbol, or disagreement.
struct Agreement
{
	bool seen = false;
	bool varies = false;
	Value value;
	Constant constant;
};

// A direct call of a candidate: its caller (a function index) and
// instruction. Compaction renumbers a body's instructions, so a site holds
// while its caller's generation is the one recorded; a candidate's body is
// compacted when it is simplified or loses parameters, and its calls are
// then recorded again.
struct Site
{
	uint32_t caller;
	uint32_t generation;
	InstId inst;
};

struct Specializer
{
	Program& program;
	const CallGraph& graph;
	std::vector<uint8_t> ok;
	std::vector<std::vector<Agreement>> agree;
	std::vector<std::vector<Site>> sites; // per candidate
	std::vector<uint32_t> generation;     // per function

	Specializer(Program& p, const CallGraph& g) : program(p), graph(g) {}

	FunctionBody& body_of(uint32_t f) const
	{
		return *program.functions[program.symbols[graph.functions[f]].detail].body;
	}
	const Signature& signature_of(uint32_t f) const
	{
		return program.signatures[program.functions[program.symbols[graph.functions[f]].detail].signature];
	}
	bool candidate(uint32_t f) const;
	// Records `g`'s calls of candidates at its current generation; with
	// `agreeing`, also what each passes (the first scan).
	void record(uint32_t g, bool agreeing);
	uint32_t substitute(uint32_t f);
	void remove_parameters(uint32_t f, const std::vector<uint8_t>& removed);
};

bool Specializer::candidate(uint32_t f) const
{
	const Symbol& symbol = program.symbols[graph.functions[f]];
	const Signature& sig = signature_of(f);
	return symbol.metadata.binding == ir_model::SBM_INTERNAL && !graph.observed[f] && graph.calls_in[f] > 0 &&
		!graph.recursive[f] && sig.boundary.arity == ir_model::CAM_FIXED && sig.parameter_count > 0 &&
		sig.boundary.query == ir_model::CQM_DEFAULT;
}

void Specializer::record(uint32_t g, bool agreeing)
{
	const FunctionBody& body = body_of(g);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol))
				continue;
			const uint32_t f = graph.index_of[inst.ops[0].index];
			if (f == kNoId || !ok[f])
				continue;
			sites[f].push_back(Site{ g, generation[g], i });
			if (!agreeing)
				continue;
			const Signature& sig = signature_of(f);
			if ((inst.flags & IF_SIGNATURE) != 0 || inst.extra_count != sig.parameter_count)
			{
				ok[f] = 0;
				continue;
			}
			for (uint32_t p = 0; p < sig.parameter_count; ++p)
			{
				Agreement& a = agree[f][p];
				const Value v = body.operands[inst.extra_first + p];
				const Type type = program.parameters_of(sig)[p].type;
				Constant c;
				const bool literal = literal_constant(body, v, c) && !type.is_object();
				if (!literal && !v.is(ValueKind::Symbol))
				{
					a.varies = true;
					continue;
				}
				if (!a.seen)
				{
					a.seen = true;
					a.value = v;
					a.constant = c;
					continue;
				}
				const bool same = literal ? a.value.is_literal() && same_constant(type, a.constant, c) : a.value == v;
				a.varies = a.varies || !same;
			}
		}
}

// The uses of each parameter every call agrees on take the agreed literal
// (in `f`'s own pool) or symbol, in one scan; a literal never becomes a
// callee. Returns the number of uses replaced.
uint32_t Specializer::substitute(uint32_t f)
{
	FunctionBody& body = body_of(f);
	const std::vector<Agreement>& a = agree[f];
	std::vector<Value> made(a.size(), Value::none());
	uint32_t replaced = 0;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction& inst = body.instructions[i];
			uint32_t position = 0;
			for_each_operand(body, inst, [&](Value& v) {
				const bool callee = inst.op == Opcode::Call && position++ == 0;
				if (!v.is(ValueKind::Temp) || v.index >= a.size())
					return;
				const Agreement& p = a[v.index];
				if (!p.seen || p.varies || (callee && p.value.is_literal()))
					return;
				Value& value = made[v.index];
				if (value.is(ValueKind::None))
					value = p.value.is_literal()
						? literal_value(body, p.constant.kind, p.constant.float_kind, p.constant.bits)
						: p.value;
				v = value;
				++replaced;
			});
		}
	return replaced;
}

// Drops the parameters `removed` marks from `f`'s signature and body
// (temporaries renumbered, then the body compacted so every temporary
// still prints uniquely: one renumbered onto a removed parameter's id would
// otherwise print as a named temporary already spelled that way) and from
// every call naming it (its recorded sites).
void Specializer::remove_parameters(uint32_t f, const std::vector<uint8_t>& removed)
{
	Function& function = program.functions[program.symbols[graph.functions[f]].detail];
	Signature sig = program.signatures[function.signature];
	const uint32_t count = sig.parameter_count;
	std::vector<Parameter> kept;
	for (uint32_t p = 0; p < count; ++p)
		if (!removed[p])
			kept.push_back(program.parameters_of(sig)[p]);
	sig.first_parameter = static_cast<uint32_t>(program.parameters.size());
	sig.parameter_count = static_cast<uint32_t>(kept.size());
	program.parameters.insert(program.parameters.end(), kept.begin(), kept.end());
	function.signature = program.add_signature(sig);
	FunctionBody& body = *function.body;
	std::vector<TempId> map(body.temps.size(), kNoId);
	std::vector<Temp> temps;
	for (TempId t = 0; t < body.temps.size(); ++t)
	{
		if (t < count && removed[t])
			continue;
		map[t] = static_cast<TempId>(temps.size());
		temps.push_back(body.temps[t]);
	}
	body.temps.swap(temps);
	body.parameter_count = sig.parameter_count;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction& inst = body.instructions[i];
			if (inst.result != kNoId)
				inst.result = map[inst.result];
			for_each_operand(body, inst, [&](Value& v) {
				if (v.is(ValueKind::Temp))
					v.index = map[v.index];
			});
		}
	compact_body(program, body);
	const SymbolId symbol = graph.functions[f];
	for (const Site& site : sites[f])
	{
		if (site.generation != generation[site.caller])
			continue;
		FunctionBody& cb = body_of(site.caller);
		Instruction& inst = cb.instructions[site.inst];
		if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol) || inst.ops[0].index != symbol)
			continue;
		uint32_t out = 0;
		for (uint32_t p = 0; p < count; ++p)
			if (!removed[p])
				cb.operands[inst.extra_first + out++] = cb.operands[inst.extra_first + p];
		inst.extra_count = out;
	}
}

} // namespace

bool specialize_arguments(Program& program, OptimizerStats& stats, const std::function<void(SymbolId)>& simplify)
{
	CallGraph graph;
	build_call_graph(program, graph);
	const uint32_t n = static_cast<uint32_t>(graph.functions.size());
	Specializer sp(program, graph);
	sp.ok.assign(n, 0);
	sp.agree.resize(n);
	sp.sites.resize(n);
	sp.generation.assign(n, 0);
	for (uint32_t f = 0; f < n; ++f)
		if (sp.candidate(f))
		{
			sp.ok[f] = 1;
			sp.agree[f].resize(sp.signature_of(f).parameter_count);
		}
	for (uint32_t g = 0; g < n; ++g)
		sp.record(g, true);
	// The callers of each function, as the graph found them.
	std::vector<uint32_t> caller_first(n + 1, 0), callers(graph.callees.size());
	for (uint32_t callee : graph.callees)
		++caller_first[callee + 1];
	for (uint32_t f = 0; f < n; ++f)
		caller_first[f + 1] += caller_first[f];
	{
		std::vector<uint32_t> at(caller_first.begin(), caller_first.end() - 1);
		for (uint32_t g = 0; g < n; ++g)
			for (uint32_t k = graph.callee_first[g]; k < graph.callee_first[g + 1]; ++k)
				callers[at[graph.callees[k]]++] = g;
	}
	std::vector<uint8_t> changed_callers(n, 0);
	bool any = false;
	for (uint32_t f = 0; f < n; ++f)
	{
		if (!sp.ok[f])
			continue;
		const SymbolId s = graph.functions[f];
		const bool substituted = sp.substitute(f) != 0;
		if (substituted)
		{
			simplify(s);
			++sp.generation[f];
			sp.record(f, false);
		}
		// Parameters the body no longer reads.
		std::vector<uint32_t> uses;
		count_uses(sp.body_of(f), uses);
		std::vector<uint8_t> removed(sp.agree[f].size(), 0);
		bool remove = false;
		for (uint32_t p = 0; p < sp.agree[f].size(); ++p)
			if (uses[p] == 0)
				removed[p] = remove = true;
		if (!remove && !substituted)
			continue;
		if (remove)
		{
			sp.remove_parameters(f, removed);
			++sp.generation[f];
			sp.record(f, false);
			for (uint32_t k = caller_first[f]; k < caller_first[f + 1]; ++k)
				changed_callers[callers[k]] = 1;
		}
		++stats.specialized;
		any = true;
	}
	for (uint32_t g = 0; g < n; ++g)
		if (changed_callers[g])
			simplify(graph.functions[g]);
	return any;
}

} // namespace opt
