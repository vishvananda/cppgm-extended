#include "opt/interprocedural.h"

#include <map>
#include <string>
#include <cstring>

#include "lowir/builder.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// Bound specialization work independently of the number of callers. One
// write-only-parameter clone is shared by all sites needing the same slice.
const uint32_t kPrivateCopies = 32;
const uint32_t kPrivateBody = 512;
const uint64_t kPrivateCopyBudget = 8192;

bool library_call(const Program& program, const Instruction& inst, const char* name, uint32_t arguments)
{
	if (inst.op != Opcode::Call || (inst.flags & IF_SIGNATURE) != 0 || !inst.ops[0].is(ValueKind::Symbol) ||
		inst.extra_count != arguments)
		return false;
	const Symbol& symbol = program.symbols[inst.ops[0].index];
	if (symbol.kind != SymbolKind::FunctionDeclaration || std::strcmp(program.spelling(symbol.name), name) != 0)
		return false;
	const Signature* signature = program.call_boundary(inst);
	if (signature == nullptr || signature->boundary.arity != ir_model::CAM_FIXED ||
		signature->parameter_count != arguments)
		return false;
	for (uint32_t p = 0; p < arguments; ++p)
		if (program.parameters_of(*signature)[p].type.kind() !=
			(std::strcmp(name, "free") == 0 ? TypeKind::Ptr : TypeKind::I64))
			return false;
	return true;
}

// Exact roots through pointer copies and indexing. A merge is deliberately
// not an exact root: its incoming uses are observations that disqualify each
// private root. Path compression makes long frontend alias chains linear.
class PointerRoots
{
public:
	explicit PointerRoots(const FunctionBody& body) : body_(body), root_(body.temps.size(), kNoId - 1) {}
	void seed(TempId temp) { root_[temp] = temp; }
	TempId of(Value value)
	{
		path_.clear();
		TempId root = kNoId;
		while (value.is(ValueKind::Temp) && body_.temps[value.index].type.is_pointer())
		{
			if (root_[value.index] != kNoId - 1)
			{
				root = root_[value.index];
				break;
			}
			path_.push_back(value.index);
			const InstId id = body_.temps[value.index].def;
			if (id == kNoId)
				break;
			const Instruction& inst = body_.instructions[id];
			if (inst.op != Opcode::Index && !(inst.op == Opcode::Copy && inst.type.is_pointer()))
				break;
			value = inst.ops[0];
		}
		for (TempId temp : path_)
			root_[temp] = root;
		return root;
	}

private:
	const FunctionBody& body_;
	std::vector<TempId> root_, path_;
};

bool address_step(const Instruction& inst)
{
	return inst.op == Opcode::Index || (inst.op == Opcode::Copy && inst.type.is_pointer());
}

bool private_write(const Instruction& inst, unsigned operand)
{
	return (inst.flags & IF_VOLATILE) == 0 &&
		(((inst.op == Opcode::Store || inst.op == Opcode::CopyObject) && operand == 1) ||
			(inst.op == Opcode::ZeroInit && operand == 0));
}

// Dropping private writes needs no promise about errno. Dropping the allocator
// does: an unrelated load or call can observe allocation failure. Keep the
// allocation/free pair unless the remaining caller and its sliced helpers
// contain only value computations and ordinary control flow. This deliberately
// favors a retained cheap pair over guessing which unknown pointer is errno.
bool value_only(const FunctionBody& body)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (!is_pure(inst) && inst.op != Opcode::Return && inst.op != Opcode::Jump && inst.op != Opcode::Branch &&
				inst.op != Opcode::Switch)
				return false;
		}
	return true;
}

bool allocator_observed(const Program& program, const FunctionBody& body, const std::vector<uint8_t>& discard)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (discard[i] || library_call(program, inst, "malloc", 1) || library_call(program, inst, "calloc", 2) ||
				library_call(program, inst, "free", 1))
				continue;
			if (inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Symbol))
			{
				const Symbol& callee = program.symbols[inst.ops[0].index];
				if (callee.kind == SymbolKind::FunctionDefinition &&
					callee.metadata.binding == ir_model::SBM_INTERNAL &&
					(callee.metadata.flags & SF_REPLACEABLE) == 0 && program.functions[callee.detail].body != nullptr &&
					value_only(*program.functions[callee.detail].body))
					continue;
			}
			if (!is_pure(inst) && inst.op != Opcode::Return && inst.op != Opcode::Jump && inst.op != Opcode::Branch &&
				inst.op != Opcode::Switch)
				return true;
		}
	return false;
}

struct Slice
{
	bool valid = false;
	SymbolId copy = kNoId;
	std::vector<InstId> writes;
};

// A parameter may be used only to form destination addresses and write
// ordinary bytes. Loads, tests, returns, captures, merges and opaque calls
// all prevent slicing. Other parameters' reads and all other effects remain.
Slice write_only_parameter(const Program& program, SymbolId symbol, uint32_t parameter)
{
	Slice out;
	const Symbol& declared = program.symbols[symbol];
	if (declared.kind != SymbolKind::FunctionDefinition || declared.metadata.binding != ir_model::SBM_INTERNAL ||
		(declared.metadata.flags & SF_REPLACEABLE) != 0)
		return out;
	const FunctionBody& body = *program.functions[declared.detail].body;
	const Signature& signature = program.signatures[program.functions[declared.detail].signature];
	if (!body.code_labels.empty() || instruction_count(body) > kPrivateBody || parameter >= body.parameter_count ||
		!body.temps[parameter].type.is_pointer() || signature.boundary.arity != ir_model::CAM_FIXED ||
		program.parameters_of(signature)[parameter].pass != ir_model::PPM_DIRECT)
		return out;
	PointerRoots roots(body);
	roots.seed(parameter);
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Asm || is_eh_instruction(inst.op))
				return out;
			bool bad = false, write = false;
			unsigned operand = 0;
			for_each_operand(body, inst, [&](const Value& value) {
				const unsigned at = operand++;
				if (roots.of(value) != parameter)
					return;
				if (address_step(inst) && at == 0)
					return;
				if (private_write(inst, at))
					write = true;
				else
					bad = true;
			});
			if (bad)
				return out;
			if (write)
				out.writes.push_back(i);
		}
	out.valid = !out.writes.empty();
	return out;
}

struct PrivateCall
{
	InstId instruction;
	SymbolId callee;
	uint32_t parameter;
};

struct Allocation
{
	TempId value;
	InstId instruction;
	bool valid;
	std::vector<InstId> writes, frees;
	std::vector<PrivateCall> calls;
};

class AllocationElision
{
public:
	AllocationElision(Program& program, OptimizerStats& stats, const std::function<void(SymbolId)>& simplify)
		: program_(program), stats_(stats), simplify_(simplify)
	{}
	bool run();

private:
	Slice& slice(SymbolId symbol, uint32_t parameter);
	SymbolId make_slice(SymbolId symbol, uint32_t parameter, Slice& slice);
	bool caller(SymbolId symbol);
	void inspect(FunctionBody& body, PointerRoots& roots, std::vector<Allocation>& allocations,
		const std::vector<uint32_t>& index);
	Program& program_;
	OptimizerStats& stats_;
	const std::function<void(SymbolId)>& simplify_;
	std::map<std::pair<SymbolId, uint32_t>, Slice> slices_;
	uint32_t names_ = 0, copies_ = 0;
	uint64_t copied_ = 0;
};

Slice& AllocationElision::slice(SymbolId symbol, uint32_t parameter)
{
	const auto key = std::make_pair(symbol, parameter);
	auto found = slices_.find(key);
	if (found == slices_.end())
		found = slices_.emplace(key, write_only_parameter(program_, symbol, parameter)).first;
	return found->second;
}

SymbolId AllocationElision::make_slice(SymbolId source, uint32_t parameter, Slice& slice)
{
	if (slice.copy != kNoId)
		return slice.copy;
	const uint32_t detail = program_.symbols[source].detail;
	const Function& original = program_.functions[detail];
	const SymbolMetadata metadata = program_.symbols[source].metadata;
	const uint64_t size = instruction_count(*original.body);
	if (copies_ >= kPrivateCopies || size > kPrivateCopyBudget - copied_)
		return kNoId;
	std::string name;
	NameId id;
	do
	{
		name = "__private_output" + std::to_string(names_++);
		id = program_.names.find(name.data(), static_cast<uint32_t>(name.size()));
	} while (id != 0 && program_.find_symbol(id) != kNoId);
	Function function;
	function.signature = original.signature;
	function.location = original.location;
	function.body.reset(new FunctionBody(*original.body));
	// The discarded parameter will receive null. Its copy has no incoming
	// object/alias promise; all other parameters and ABI facts stay intact.
	Signature signature = program_.signatures[function.signature];
	std::vector<Parameter> parameters(
		program_.parameters_of(signature), program_.parameters_of(signature) + signature.parameter_count);
	parameters[parameter].object_bytes = 0;
	parameters[parameter].alias = ir_model::PALM_DEFAULT;
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	program_.parameters.insert(program_.parameters.end(), parameters.begin(), parameters.end());
	function.signature = program_.add_signature(signature);
	const SymbolId copy = program_.symbol_for(program_.names.intern(name.data(), static_cast<uint32_t>(name.size())));
	Symbol& symbol = program_.symbols[copy];
	symbol.kind = SymbolKind::FunctionDefinition;
	symbol.detail = static_cast<uint32_t>(program_.functions.size());
	symbol.metadata = metadata;
	symbol.metadata.binding = ir_model::SBM_INTERNAL;
	symbol.metadata.object = 0;
	symbol.metadata.flags &= static_cast<uint8_t>(~(SF_OBJECT_ROOT | SF_KEEP_ALIAS));
	program_.functions.push_back(std::move(function));
	program_.order.push_back(copy);
	FunctionBody& body = *program_.functions[program_.symbols[copy].detail].body;
	std::vector<uint8_t> discard(body.instructions.size(), 0);
	for (InstId i : slice.writes)
		discard[i] = 1;
	for (BlockId b : body.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId;)
		{
			const InstId next = body.instructions[i].next;
			if (discard[i])
				unlink_after(body, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
	slice.copy = copy;
	stats_.stores_removed += slice.writes.size();
	++copies_;
	copied_ += size;
	simplify_(copy);
	return copy;
}

void AllocationElision::inspect(
	FunctionBody& body, PointerRoots& roots, std::vector<Allocation>& allocations, const std::vector<uint32_t>& index)
{
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Call)
			{
				unsigned private_arguments = 0;
				for (uint32_t p = 0; p < inst.extra_count; ++p)
					private_arguments += roots.of(body.operands[inst.extra_first + p]) != kNoId;
				for (uint32_t p = 0; p < inst.extra_count; ++p)
				{
					const TempId root = roots.of(body.operands[inst.extra_first + p]);
					if (root == kNoId)
						continue;
					Allocation& allocation = allocations[index[root]];
					if (private_arguments != 1)
						allocation.valid = false;
					else if (library_call(program_, inst, "free", 1) && inst.type.is_void())
						allocation.frees.push_back(i);
					else if ((inst.flags & IF_SIGNATURE) == 0 && inst.ops[0].is(ValueKind::Symbol) &&
						program_.call_boundary(inst) != nullptr &&
						program_.call_boundary(inst)->parameter_count == inst.extra_count &&
						slice(inst.ops[0].index, p).valid)
						allocation.calls.push_back(PrivateCall{ i, inst.ops[0].index, p });
					else
						allocation.valid = false;
				}
				const TempId target = roots.of(inst.ops[0]);
				if (target != kNoId)
					allocations[index[target]].valid = false;
				continue;
			}
			unsigned operand = 0;
			for_each_operand(body, inst, [&](const Value& value) {
				const unsigned at = operand++;
				const TempId root = roots.of(value);
				if (root == kNoId)
					return;
				Allocation& allocation = allocations[index[root]];
				if (address_step(inst) && at == 0)
					return;
				if (private_write(inst, at))
					allocation.writes.push_back(i);
				else
					allocation.valid = false;
			});
		}
}

bool AllocationElision::caller(SymbolId symbol)
{
	FunctionBody& body = *program_.functions[program_.symbols[symbol].detail].body;
	if (!body.code_labels.empty())
		return false;
	std::vector<Allocation> allocations;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (inst.result != kNoId && inst.type.is_pointer() &&
				(library_call(program_, inst, "malloc", 1) || library_call(program_, inst, "calloc", 2)))
				allocations.push_back(Allocation{ inst.result, i, true, {}, {}, {} });
		}
	if (allocations.empty())
		return false;
	PointerRoots roots(body);
	std::vector<uint32_t> index(body.temps.size(), kNoId);
	for (uint32_t k = 0; k < allocations.size(); ++k)
	{
		roots.seed(allocations[k].value);
		index[allocations[k].value] = k;
	}
	inspect(body, roots, allocations, index);
	std::vector<uint8_t> discard(body.instructions.size(), 0);
	bool changed = false;
	for (Allocation& allocation : allocations)
	{
		if (!allocation.valid || allocation.frees.empty())
			continue;
		for (const PrivateCall& call : allocation.calls)
			if (make_slice(call.callee, call.parameter, slice(call.callee, call.parameter)) == kNoId)
				allocation.valid = false;
		if (!allocation.valid)
			continue;
		for (const PrivateCall& call : allocation.calls)
		{
			Instruction& inst = body.instructions[call.instruction];
			inst.ops[0] = Value::symbol(slice(call.callee, call.parameter).copy);
			body.operands[inst.extra_first + call.parameter] = integer_value(body, integer_literal(0));
		}
		for (InstId i : allocation.writes)
			discard[i] = 1;
		changed = changed || !allocation.writes.empty() || !allocation.calls.empty();
	}
	if (!allocator_observed(program_, body, discard))
		for (const Allocation& allocation : allocations)
		{
			if (!allocation.valid || allocation.frees.empty())
				continue;
			discard[allocation.instruction] = 1;
			for (InstId i : allocation.frees)
				discard[i] = 1;
			changed = true;
		}
	if (!changed)
		return false;
	for (BlockId b : body.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId;)
		{
			const InstId next = body.instructions[i].next;
			if (discard[i])
				unlink_after(body, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
	simplify_(symbol);
	// A cached unmaterialized slice referred to this body's instruction ids.
	// A previously made copy remains valid and can still be shared.
	for (auto it = slices_.lower_bound(std::make_pair(symbol, 0u)); it != slices_.end() && it->first.first == symbol;)
		if (it->second.copy == kNoId)
			it = slices_.erase(it);
		else
			++it;
	return true;
}

bool AllocationElision::run()
{
	bool changed = false;
	const SymbolId count = static_cast<SymbolId>(program_.symbols.size());
	for (SymbolId s = 0; s < count; ++s)
		if (program_.symbols[s].kind == SymbolKind::FunctionDefinition &&
			program_.functions[program_.symbols[s].detail].body != nullptr)
			changed = caller(s) || changed;
	return changed;
}

} // namespace

bool elide_private_allocations(Program& program, OptimizerStats& stats, const std::function<void(SymbolId)>& simplify)
{
	AllocationElision pass(program, stats, simplify);
	return pass.run();
}

} // namespace opt
