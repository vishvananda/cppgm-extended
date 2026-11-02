#include <algorithm>

#include "lowir/builder.h"
#include "opt/function_ir.h"
#include "opt/interprocedural.h"

namespace opt {

using namespace lowir;

namespace {

// A target the direct path may then copy (the late wave's caps at a loop
// site are larger).
const uint32_t kSpeculatedTargetCap = 64;
const uint32_t kSpeculatedCalls = 8; // per function

// A store of a readonly table's address (`index i8 @vtable, 16`) into an
// object: its construction.
struct TableStore
{
	Value object;
	SymbolId table;
	int64_t offset;
	BlockId block;
	uint32_t order;
};

struct Speculation
{
	InstId call;
	BlockId block;
	uint32_t order;
	SymbolId target;
};

class VirtualCalls
{
public:
	VirtualCalls(Program& program, FunctionBody& body) : program_(program), body_(body) {}

	uint32_t run();

private:
	const Instruction* producer(Value v) const
	{
		return v.is(ValueKind::Temp) && body_.temps[v.index].def != kNoId
			? &body_.instructions[body_.temps[v.index].def]
			: nullptr;
	}
	void address_of(Value v, Value& base, int64_t& offset) const;
	SymbolId target_of(const Instruction& call, BlockId block, uint32_t order) const;
	void speculate(BlockId block, InstId before, InstId id, SymbolId target);

	Program& program_;
	FunctionBody& body_;
	Cfg cfg_;
	std::vector<TableStore> stores_;
};

// The base and constant offset of an address (through `index` steps by
// literals, pointer copies and `addr` of a symbol).
void VirtualCalls::address_of(Value v, Value& base, int64_t& offset) const
{
	base = v;
	offset = 0;
	for (unsigned step = 0; step < 8; ++step)
	{
		const Instruction* def = producer(base);
		if (def == nullptr)
			return;
		if (def->op == Opcode::Index && def->ops[1].is(ValueKind::Integer))
			offset += static_cast<int64_t>(body_.literals[def->ops[1].index].low) *
				static_cast<int64_t>(storage_bytes(def->type));
		else if (!((def->op == Opcode::Copy && def->type.is_pointer()) ||
					 (def->op == Opcode::Addr && def->ops[0].is(ValueKind::Symbol))))
			return;
		base = def->ops[0];
	}
}

// The function the latest table store dominating the call (`order` in
// `block`) names in the slot the call loads its callee from, when the call
// passes that object first; kNoId otherwise.
SymbolId VirtualCalls::target_of(const Instruction& call, BlockId block, uint32_t order) const
{
	const Instruction* slot = producer(call.ops[0]);
	if (slot == nullptr || slot->op != Opcode::Load || (slot->flags & IF_VOLATILE) != 0 || !slot->type.is_pointer())
		return kNoId;
	Value table;
	int64_t at = 0;
	address_of(slot->ops[0], table, at);
	const Instruction* vptr = producer(table);
	if (vptr == nullptr || vptr->op != Opcode::Load || (vptr->flags & IF_VOLATILE) != 0 || !vptr->type.is_pointer() ||
		call.extra_count == 0 || !(body_.operands[call.extra_first] == vptr->ops[0]))
		return kNoId;
	const TableStore* best = nullptr;
	for (const TableStore& s : stores_)
	{
		if (!(s.object == vptr->ops[0]) || !cfg_.reachable(s.block) ||
			!(s.block == block ? s.order < order : cfg_.dominates(s.block, block)))
			continue;
		// (The dominating stores lie on one chain: the deepest is the latest.)
		if (best == nullptr || cfg_.dom_in[s.block] > cfg_.dom_in[best->block] ||
			(s.block == best->block && s.order > best->order))
			best = &s;
	}
	SymbolId target = kNoId;
	if (best == nullptr || !read_readonly_address(program_, best->table, best->offset + at, nullptr, target))
		return kNoId;
	const Symbol& symbol = program_.symbols[target];
	if (symbol.kind != SymbolKind::FunctionDefinition || (symbol.metadata.flags & (SF_NO_INLINE | SF_REPLACEABLE)) != 0)
		return kNoId;
	const Function& function = program_.functions[symbol.detail];
	const Signature& sig = program_.signatures[function.signature];
	if (function.body == nullptr || sig.parameter_count != call.extra_count || sig.result != call.type ||
		instruction_count(*function.body) > kSpeculatedTargetCap)
		return kNoId;
	return target;
}

// `block` ends at a branch on the callee being `target`: a direct call of
// it, or the call as it was; `rest` holds what followed, starting with a phi
// of the two results under the call's own name.
void VirtualCalls::speculate(BlockId block, InstId before, InstId id, SymbolId target)
{
	const Instruction call = body_.instructions[id];
	const BlockId rest = append_block(body_, BlockKind::Split);
	const BlockId direct = append_block(body_, BlockKind::Split);
	const BlockId indirect = append_block(body_, BlockKind::Split);
	const InstId last = body_.blocks[block].last;
	body_.blocks[rest].first = call.next;
	body_.blocks[rest].last = last;
	for_each_successor(
		body_, body_.instructions[last], [&](BlockId& to) { rename_phi_incoming(body_, to, block, rest); });
	body_.blocks[block].last = before;
	if (before == kNoId)
		body_.blocks[block].first = kNoId;
	else
		body_.instructions[before].next = kNoId;
	auto emit = [&](BlockId b, InstId& after, InstId made, Type result) {
		body_.instructions[made].location = call.location;
		const TempId temp = result.kind() == TypeKind::Void ? kNoId : append_temp(body_, result, made);
		body_.instructions[made].result = temp;
		insert_after(body_, b, after, made);
		after = made;
		return Value::temp(temp);
	};
	auto jump = [&](BlockId b, InstId& after) {
		Instruction back = make_instruction(Opcode::Jump, Type::scalar(TypeKind::Void));
		back.ops[0] = Value::block(rest);
		emit(b, after, append_instruction(body_, back), back.type);
	};
	InstId at = before;
	Instruction same = make_instruction(Opcode::Compare, Type::scalar(TypeKind::Ptr));
	same.sub = static_cast<uint8_t>(ComparePredicate::Eq);
	same.ops[0] = call.ops[0];
	same.ops[1] = Value::symbol(target);
	const Value known = emit(block, at, append_instruction(body_, same), Type::scalar(TypeKind::I64));
	Instruction branch = make_instruction(Opcode::Branch, Type::scalar(TypeKind::Void));
	branch.ops[0] = known;
	branch.ops[1] = Value::block(direct);
	branch.ops[2] = Value::block(indirect);
	emit(block, at, append_instruction(body_, branch), branch.type);
	// The direct call, with its own argument list.
	Instruction copy = call;
	copy.ops[0] = Value::symbol(target);
	copy.next = kNoId;
	const std::vector<Value> arguments(
		body_.operands.begin() + call.extra_first, body_.operands.begin() + call.extra_first + call.extra_count);
	copy.extra_first = append_operands(body_, arguments.data(), call.extra_count);
	InstId tail = kNoId;
	const Value fast = emit(direct, tail, append_copy(body_, body_, id, copy), call.type);
	jump(direct, tail);
	// The call through the slot, under a new name.
	const TempId result = call.result;
	body_.instructions[id].next = kNoId;
	tail = kNoId;
	insert_after(body_, indirect, tail, id);
	tail = id;
	Value slow;
	if (result != kNoId)
	{
		slow = Value::temp(append_temp(body_, call.type, id));
		body_.instructions[id].result = slow.index;
	}
	jump(indirect, tail);
	if (result != kNoId)
	{
		const Value pairs[4] = { Value::block(direct), fast, Value::block(indirect), slow };
		Instruction phi = make_instruction(Opcode::Phi, call.type);
		phi.extra_first = append_operands(body_, pairs, 4);
		phi.extra_count = 4;
		phi.result = result;
		phi.location = call.location;
		const InstId merge = append_instruction(body_, phi);
		body_.temps[result].def = merge;
		insert_after(body_, rest, kNoId, merge);
	}
	// The direct path falls through to what followed; the other goes last.
	std::vector<BlockId> layout;
	layout.reserve(body_.layout.size() + 3);
	for (BlockId b : body_.layout)
	{
		layout.push_back(b);
		if (b == block)
		{
			layout.push_back(direct);
			layout.push_back(rest);
		}
	}
	layout.push_back(indirect);
	body_.layout.swap(layout);
}

uint32_t VirtualCalls::run()
{
	std::vector<Speculation> found;
	bool calls = false;
	uint32_t order = 0;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next, ++order)
		{
			const Instruction& inst = body_.instructions[i];
			calls = calls || (inst.op == Opcode::Call && inst.ops[0].is(ValueKind::Temp));
			if (inst.op != Opcode::Store || (inst.flags & IF_VOLATILE) != 0 || !inst.type.is_pointer() ||
				!inst.ops[1].is(ValueKind::Temp))
				continue;
			Value table;
			int64_t offset = 0;
			address_of(inst.ops[0], table, offset);
			if (table.is(ValueKind::Symbol) && program_.symbols[table.index].kind == SymbolKind::GlobalDefinition &&
				constant_contents(program_, table.index, nullptr))
				stores_.push_back(TableStore{ inst.ops[1], table.index, offset, b, order });
		}
	if (stores_.empty() || !calls)
		return 0;
	build_cfg(program_, body_, cfg_);
	order = 0;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && found.size() < kSpeculatedCalls;
			i = body_.instructions[i].next, ++order)
		{
			const Instruction& inst = body_.instructions[i];
			if (inst.op != Opcode::Call || !inst.ops[0].is(ValueKind::Temp) || !cfg_.reachable(b))
				continue;
			const SymbolId target = target_of(inst, b, order);
			if (target != kNoId)
				found.push_back(Speculation{ i, b, order, target });
		}
	// Later calls of a block first: a split moves what follows its call.
	std::sort(found.begin(), found.end(), [](const Speculation& a, const Speculation& b) {
		return a.block != b.block ? a.block < b.block : a.order > b.order;
	});
	for (const Speculation& s : found)
		speculate(s.block, previous_in_block(body_, s.block, s.call), s.call, s.target);
	return static_cast<uint32_t>(found.size());
}

} // namespace

bool speculate_virtual_calls(Program& program, OptimizerStats& stats)
{
	uint32_t total = 0;
	for (SymbolId s = 0; s < program.symbols.size(); ++s)
	{
		const Symbol& symbol = program.symbols[s];
		if (symbol.kind != SymbolKind::FunctionDefinition || program.functions[symbol.detail].body == nullptr)
			continue;
		VirtualCalls pass(program, *program.functions[symbol.detail].body);
		total += pass.run();
	}
	stats.calls_speculated += total;
	return total != 0;
}

} // namespace opt
