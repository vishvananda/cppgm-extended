#include "opt/interprocedural.h"

#include "opt/function_ir.h"
#include "opt/loops.h"
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
		!graph.recursive[f] && body_of(f).code_labels.empty() && sig.boundary.arity == ir_model::CAM_FIXED &&
		sig.parameter_count > 0;
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
			const uint32_t f = graph.function_of(inst.ops[0].index);
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

// Whether the function's own instructions leave its callers' memory alone:
// no store but to a slot whose address stays here, no volatile or atomic
// access, no exception work. Calls to defined functions are the caller's
// question; any other call must promise as much itself.
static bool writes_nothing_outside(const Program& program, const FunctionBody& body, const CallGraph& graph)
{
	std::vector<uint8_t> escaped;
	auto own = [&](Value address) {
		const Located at = locate(body, address);
		if (!at.root.is(ValueKind::Slot))
			return false;
		if (escaped.empty())
			escaped = escaped_slots(body, true);
		return escaped[at.root.index] == 0;
	};
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if ((inst.flags & IF_VOLATILE) != 0 && (inst.op == Opcode::Load || inst.op == Opcode::Store))
				return false;
			if (inst.op == Opcode::Store || inst.op == Opcode::CopyObject)
			{
				if (!own(inst.ops[1]))
					return false;
			}
			else if (inst.op == Opcode::ZeroInit)
			{
				if (!own(inst.ops[0]))
					return false;
			}
			else if (inst.op == Opcode::Call)
			{
				const bool defined = inst.ops[0].is(ValueKind::Symbol) && graph.function_of(inst.ops[0].index) != kNoId;
				if (!defined && call_may_write_memory(program, inst))
					return false;
			}
			else if (may_write_memory(program, inst))
				return false;
		}
	return true;
}

bool infer_read_only_functions(Program& program)
{
	CallGraph graph;
	build_call_graph(program, graph);
	const uint32_t n = static_cast<uint32_t>(graph.functions.size());
	std::vector<uint8_t> writes(n, 0);
	std::vector<uint32_t> work;
	for (uint32_t f = 0; f < n; ++f)
	{
		const Symbol& symbol = program.symbols[graph.functions[f]];
		if ((symbol.metadata.flags & SF_REPLACEABLE) != 0 ||
			!writes_nothing_outside(program, *program.functions[symbol.detail].body, graph))
		{
			writes[f] = 1;
			work.push_back(f);
		}
	}
	// The callers of a function that writes write too; what remains calls
	// only functions that do not, cycles included.
	std::vector<uint32_t> caller_first(n + 1, 0), callers(graph.callees.size());
	for (uint32_t callee : graph.callees)
		++caller_first[callee + 1];
	for (uint32_t f = 0; f < n; ++f)
		caller_first[f + 1] += caller_first[f];
	std::vector<uint32_t> at(caller_first.begin(), caller_first.end() - 1);
	for (uint32_t g = 0; g < n; ++g)
		for (uint32_t k = graph.callee_first[g]; k < graph.callee_first[g + 1]; ++k)
			callers[at[graph.callees[k]]++] = g;
	while (!work.empty())
	{
		const uint32_t f = work.back();
		work.pop_back();
		for (uint32_t k = caller_first[f]; k < caller_first[f + 1]; ++k)
			if (writes[callers[k]] == 0)
			{
				writes[callers[k]] = 1;
				work.push_back(callers[k]);
			}
	}
	bool any = false;
	program.read_only_functions.assign(program.symbols.size(), 0);
	for (uint32_t f = 0; f < n; ++f)
		if (writes[f] == 0)
		{
			program.read_only_functions[graph.functions[f]] = 1;
			any = true;
		}
	return any;
}

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

namespace {

// Callee copies: a function of at most kCalleeCopyCap instructions, at most
// kCalleeCopies copies in a unit, found in at most kCalleeCopyRounds scans
// (a copy passing the function on to another candidate is found by the next).
const uint64_t kCalleeCopyCap = 400;
const uint32_t kCalleeCopies = 32;
const uint32_t kCalleeCopyRounds = 2;

struct CalleeCopy
{
	SymbolId function;
	uint32_t parameter;
	SymbolId argument;
	SymbolId copy;
};

struct CalleeCopier
{
	Program& program;
	std::vector<uint64_t> called; // per symbol: the parameters (below 64) its body calls through
	std::vector<CalleeCopy> copies;
	uint32_t names = 0;

	explicit CalleeCopier(Program& p) : program(p) {}

	FunctionBody* body_of(SymbolId s) const
	{
		const Symbol& symbol = program.symbols[s];
		return symbol.kind == SymbolKind::FunctionDefinition ? program.functions[symbol.detail].body.get() : nullptr;
	}
	void measure(SymbolId s);
	SymbolId copy_for(SymbolId f, uint32_t parameter, SymbolId argument);
	bool scan(SymbolId g, bool make);
};

void CalleeCopier::measure(SymbolId s)
{
	if (s >= called.size())
		called.resize(program.symbols.size(), 0);
	const Symbol& symbol = program.symbols[s];
	const FunctionBody* body = body_of(s);
	if (body == nullptr)
		return;
	const Signature& sig = program.signatures[program.functions[symbol.detail].signature];
	if (!body->code_labels.empty() || (symbol.metadata.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0 ||
		sig.boundary.arity != ir_model::CAM_FIXED || instruction_count(*body) > kCalleeCopyCap)
		return;
	uint64_t mask = 0;
	for (BlockId b : body->layout)
		for (InstId i = body->blocks[b].first; i != kNoId; i = body->instructions[i].next)
		{
			const Instruction& inst = body->instructions[i];
			if (inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Temp) &&
				inst.ops[0].index < body->parameter_count && inst.ops[0].index < 64)
				mask |= uint64_t(1) << inst.ops[0].index;
		}
	called[s] = mask;
}

SymbolId CalleeCopier::copy_for(SymbolId f, uint32_t parameter, SymbolId argument)
{
	for (const CalleeCopy& c : copies)
		if (c.function == f && c.parameter == parameter && c.argument == argument)
			return c.copy;
	if (copies.size() >= kCalleeCopies)
		return kNoId;
	std::string name;
	NameId id = 0;
	do
	{
		name = "__callee_copy" + std::to_string(names++);
		id = program.names.find(name.data(), static_cast<uint32_t>(name.size()));
	} while (id != 0 && program.find_symbol(id) != kNoId);
	// By value: making the copy's symbol may move the symbol table.
	const SymbolMetadata metadata = program.symbols[f].metadata;
	const uint32_t detail = program.symbols[f].detail;
	const SymbolId copy = program.symbol_for(program.names.intern(name.data(), static_cast<uint32_t>(name.size())));
	Function function;
	function.signature = program.functions[detail].signature;
	function.location = program.functions[detail].location;
	function.body.reset(new FunctionBody(*program.functions[detail].body));
	Symbol& symbol = program.symbols[copy];
	symbol.kind = SymbolKind::FunctionDefinition;
	symbol.detail = static_cast<uint32_t>(program.functions.size());
	symbol.metadata = metadata;
	symbol.metadata.binding = ir_model::SBM_INTERNAL;
	symbol.metadata.object = 0;
	symbol.metadata.flags &= static_cast<uint8_t>(~(SF_OBJECT_ROOT | SF_KEEP_ALIAS));
	program.functions.push_back(std::move(function));
	program.order.push_back(copy);
	FunctionBody& body = *program.functions[symbol.detail].body;
	const Value value = Value::symbol(argument);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
			for_each_operand(body, body.instructions[i], [&](Value& v) {
				if (v.is(ValueKind::Temp) && v.index == parameter)
					v = value;
			});
	copies.push_back(CalleeCopy{ f, parameter, argument, copy });
	measure(copy);
	return copy;
}

// Points `g`'s calls that pass a defined function for a parameter their
// callee calls through at the callee's copy for it, making the copy when
// `make`; returns whether it made one.
bool CalleeCopier::scan(SymbolId g, bool make)
{
	bool made = false;
	FunctionBody* body = body_of(g);
	for (BlockId b : body->layout)
		for (InstId i = body->blocks[b].first; i != kNoId; i = body->instructions[i].next)
		{
			const Instruction& inst = body->instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Symbol) || (inst.flags & IF_SIGNATURE) != 0)
				continue;
			const SymbolId f = inst.ops[0].index;
			if (f >= called.size() || called[f] == 0)
				continue;
			for (uint32_t k = 0; k < inst.extra_count && k < 64; ++k)
			{
				Value v = body->operands[inst.extra_first + k];
				if (v.is(ValueKind::Temp) && v.index >= body->parameter_count && body->temps[v.index].def != kNoId &&
					body->instructions[body->temps[v.index].def].op == Opcode::Addr)
					v = body->instructions[body->temps[v.index].def].ops[0];
				if ((called[f] & (uint64_t(1) << k)) == 0 || !v.is(ValueKind::Symbol) || body_of(v.index) == nullptr)
					continue;
				SymbolId copy = kNoId;
				for (const CalleeCopy& c : copies)
					if (c.function == f && c.parameter == k && c.argument == v.index)
						copy = c.copy;
				if (copy == kNoId && make)
				{
					copy = copy_for(f, k, v.index);
					made = made || copy != kNoId;
					// (Copying may move the bodies' table, not this body.)
				}
				if (copy != kNoId)
				{
					body->instructions[i].ops[0] = Value::symbol(copy);
					break;
				}
			}
		}
	return made;
}

} // namespace

bool copy_for_callees(Program& program, OptimizerStats& stats)
{
	CalleeCopier copier(program);
	copier.called.assign(program.symbols.size(), 0);
	for (SymbolId s = 0; s < program.symbols.size(); ++s)
		if (program.symbols[s].kind == SymbolKind::FunctionDefinition)
			copier.measure(s);
	bool any = false;
	for (uint32_t round = 0; round < kCalleeCopyRounds; ++round)
	{
		bool made = false;
		const SymbolId count = static_cast<SymbolId>(program.symbols.size());
		for (SymbolId s = 0; s < count; ++s)
			if (copier.body_of(s) != nullptr)
				made = copier.scan(s, true) || made;
		if (!made)
			break;
		any = true;
	}
	// The copies' own calls passing the function on.
	for (const CalleeCopy& c : copier.copies)
		copier.scan(c.copy, false);
	stats.specialized += static_cast<uint32_t>(copier.copies.size());
	return any;
}

namespace {

struct ForwardArgument
{
	Value value;
	Literal literal;
};

enum class ForwardState : uint8_t
{
	Unseen,
	Visiting,
	Resolved,
	Rejected
};

struct ForwardCall
{
	InstId call = kNoId;
	SymbolId target = kNoId;
	ForwardState state = ForwardState::Unseen;
	std::vector<ForwardArgument> arguments;
};

class CallForwarding
{
public:
	explicit CallForwarding(Program& program) : program_(program), forwards_(program.symbols.size()) {}
	bool run(OptimizerStats& stats);

private:
	const Signature& signature(SymbolId s) const
	{
		return program_.signatures[program_.functions[program_.symbols[s].detail].signature];
	}
	FunctionBody& body(SymbolId s) const { return *program_.functions[program_.symbols[s].detail].body; }
	bool recognize(SymbolId s);
	ForwardArgument argument(const FunctionBody& body, Value value) const;
	bool resolve(SymbolId s);
	void resolve_chains();
	bool rewrite(FunctionBody& body, Instruction& call);

	Program& program_;
	std::vector<ForwardCall> forwards_;
	std::vector<Value> actual_;
};

bool CallForwarding::recognize(SymbolId s)
{
	const Symbol& symbol = program_.symbols[s];
	if (symbol.kind != SymbolKind::FunctionDefinition || (symbol.metadata.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0)
		return false;
	const Signature& sig = signature(s);
	const FunctionBody& b = body(s);
	if (sig.boundary.arity != ir_model::CAM_FIXED || sig.result.is_object() || b.layout.size() != 1)
		return false;
	const Block& block = b.blocks[b.layout[0]];
	if (block.first == kNoId || block.first == block.last)
		return false;
	const Instruction& call = b.instructions[block.first];
	const Instruction& leave = b.instructions[block.last];
	if (call.op != Opcode::Call || !call.ops[0].is(ValueKind::Symbol) ||
		!is_function_kind(program_.symbols[call.ops[0].index].kind) ||
		(call.flags & (IF_SIGNATURE | IF_COPY_ELISION)) != 0 || call.next != block.last || leave.op != Opcode::Return ||
		call.type != sig.result || leave.type != sig.result)
		return false;
	if (sig.result.kind() == TypeKind::Void ? !leave.ops[0].is(ValueKind::None)
											: !(leave.ops[0] == Value::temp(call.result)))
		return false;
	const Signature& target = signature(call.ops[0].index);
	if (target.boundary.arity != ir_model::CAM_FIXED || target.result != sig.result ||
		call.extra_count != target.parameter_count)
		return false;
	forwards_[s].call = block.first;
	return true;
}

ForwardArgument CallForwarding::argument(const FunctionBody& b, Value value) const
{
	ForwardArgument result = { value, { 0, 0 } };
	if (value.is_literal())
		result.literal = b.literals[value.index];
	return result;
}

bool CallForwarding::resolve(SymbolId s)
{
	ForwardCall& forward = forwards_[s];
	const FunctionBody& b = body(s);
	const Instruction& call = b.instructions[forward.call];
	const SymbolId next = call.ops[0].index;
	const ForwardCall& nested = forwards_[next];
	const bool compose = nested.state == ForwardState::Resolved;
	forward.target = compose ? nested.target : next;
	const Signature& target = signature(forward.target);
	const Signature& own = signature(s);
	// Bypassing a frame must not expand the call's argument work. This also
	// bounds composition by the original signatures, including long chains.
	if (target.parameter_count > own.parameter_count)
		return false;
	forward.arguments.reserve(target.parameter_count);
	for (uint32_t p = 0; p < target.parameter_count; ++p)
	{
		ForwardArgument a;
		if (!compose)
			a = argument(b, b.operands[call.extra_first + p]);
		else
		{
			a = nested.arguments[p];
			if (a.value.is(ValueKind::Temp))
				a = argument(b, b.operands[call.extra_first + a.value.index]);
		}
		const Type type = program_.parameters_of(target)[p].type;
		if (a.value.is(ValueKind::Temp))
		{
			if (a.value.index >= b.parameter_count || type.is_object() ||
				program_.parameters_of(own)[a.value.index].type != type)
				return false;
		}
		else if (a.value.is_literal())
		{
			if (type.is_object())
				return false;
		}
		else if (!a.value.is(ValueKind::Symbol) || !type.is_pointer())
			return false;
		forward.arguments.push_back(a);
	}
	return true;
}

void CallForwarding::resolve_chains()
{
	std::vector<SymbolId> chain;
	for (SymbolId s = 0; s < forwards_.size(); ++s)
	{
		if (forwards_[s].call == kNoId || forwards_[s].state != ForwardState::Unseen)
			continue;
		SymbolId at = s;
		while (forwards_[at].call != kNoId && forwards_[at].state == ForwardState::Unseen)
		{
			forwards_[at].state = ForwardState::Visiting;
			chain.push_back(at);
			at = body(at).instructions[forwards_[at].call].ops[0].index;
		}
		const bool cycle = forwards_[at].state == ForwardState::Visiting;
		while (!chain.empty())
		{
			at = chain.back();
			chain.pop_back();
			forwards_[at].state = !cycle && resolve(at) ? ForwardState::Resolved : ForwardState::Rejected;
		}
	}
}

bool CallForwarding::rewrite(FunctionBody& b, Instruction& call)
{
	if (call.op != Opcode::Call || !call.ops[0].is(ValueKind::Symbol) ||
		(call.flags & (IF_SIGNATURE | IF_COPY_ELISION)) != 0)
		return false;
	const SymbolId callee = call.ops[0].index;
	const ForwardCall& forward = forwards_[callee];
	if (forward.state != ForwardState::Resolved || forward.target == callee)
		return false;
	const Signature& sig = signature(callee);
	if (call.extra_count != sig.parameter_count || call.type != sig.result)
		return false;
	actual_.clear();
	for (const ForwardArgument& a : forward.arguments)
	{
		Value value = a.value;
		if (value.is(ValueKind::Temp))
		{
			const Parameter& parameter = program_.parameters_of(sig)[value.index];
			value = b.operands[call.extra_first + value.index];
			if (value.is(ValueKind::Temp))
			{
				if (b.temps[value.index].type != parameter.type)
					return false;
			}
			else if (!(value.is_literal() || (value.is(ValueKind::Symbol) && parameter.type.is_pointer())))
				return false;
			// A storage argument supplied as a value is materialized at the
			// boundary; copying that value would lose its address semantics.
			if (parameter.pass != ir_model::PPM_DIRECT &&
				!(value.is(ValueKind::Symbol) || (value.is(ValueKind::Temp) && b.temps[value.index].type.is_pointer())))
				return false;
		}
		actual_.push_back(value);
	}
	for (uint32_t p = 0; p < actual_.size(); ++p)
	{
		const ForwardArgument& a = forward.arguments[p];
		if (a.value.is_literal())
			actual_[p] = literal_value(b, a.value.kind, a.value.float_kind, a.literal);
		b.operands[call.extra_first + p] = actual_[p];
	}
	call.ops[0] = Value::symbol(forward.target);
	call.extra_count = static_cast<uint32_t>(actual_.size());
	if (actual_.empty())
		call.extra_first = 0;
	return true;
}

bool CallForwarding::run(OptimizerStats& stats)
{
	bool any = false;
	for (SymbolId s = 0; s < program_.symbols.size(); ++s)
		any = recognize(s) || any;
	if (!any)
		return false;
	resolve_chains();
	bool changed = false;
	for (SymbolId s = 0; s < program_.symbols.size(); ++s)
		if (program_.symbols[s].kind == SymbolKind::FunctionDefinition)
		{
			FunctionBody& b = body(s);
			for (BlockId block : b.layout)
				for (InstId i = b.blocks[block].first; i != kNoId; i = b.instructions[i].next)
					if (rewrite(b, b.instructions[i]))
					{
						++stats.calls_forwarded;
						changed = true;
					}
		}
	return changed;
}

} // namespace

bool forward_calls(Program& program, OptimizerStats& stats)
{
	CallForwarding forwarding(program);
	return forwarding.run(stats);
}

} // namespace opt
