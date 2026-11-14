#include "wasm/object_places.h"

#include "opt/loops.h"

namespace wasm {

using namespace lowir;

namespace {

struct SlotUse
{
	bool escaped = false;   // its address is a value somewhere
	bool read = false;      // read other than by one by-value argument
	uint32_t first = kNoId; // the first access
	uint32_t writes = 0;
	BlockId write_block = kNoId;
	bool spread = false; // written in more than one block
	uint32_t last_write = 0;
	uint32_t calls = 0;
	BlockId call_block = kNoId;
	uint32_t call = 0;
	uint32_t returns = 0;
};

class Planner
{
public:
	Planner(const Program& program, const FunctionBody& body, const lowir::Signature& signature,
		const Boundary& boundary, const std::vector<uint32_t>* eh_sites)
		: program_(program), body_(body), signature_(signature), boundary_(boundary), eh_sites_(eh_sites)
	{}
	ObjectPlaces run();

private:
	void touch(const lowir::Instruction& inst, Value value, uint32_t operand, BlockId block, uint32_t position);
	// The slot that a complete copy of `bytes` bytes writes; kNoId otherwise.
	uint32_t whole_slot(Value destination, uint64_t bytes) const;
	void parameters(ObjectPlaces& places) const;
	void result(ObjectPlaces& places) const;
	void call_results(ObjectPlaces& places) const;

	const Program& program_;
	const FunctionBody& body_;
	const lowir::Signature& signature_;
	const Boundary& boundary_;
	const std::vector<uint32_t>* eh_sites_;
	std::vector<SlotUse> uses_;
	std::vector<uint32_t> reads_;     // per temporary: its uses
	std::vector<uint32_t> positions_; // per instruction
};

void Planner::touch(const lowir::Instruction& inst, Value value, uint32_t operand, BlockId block, uint32_t position)
{
	if (value.is(ValueKind::Temp))
		++reads_[value.index];
	else if (!value.is(ValueKind::Slot))
		return;
	const auto address = opt::locate(body_, value);
	if (!address.root.is(ValueKind::Slot) || !body_.slots[address.root.index].type.is_object())
		return;
	SlotUse& use = uses_[address.root.index];
	// A step from the address is no access: its result is located again.
	if ((inst.op == Opcode::Addr || inst.op == Opcode::Index) && operand == 0)
		return;
	if (inst.op == Opcode::Copy && operand == 0 && inst.type.is_pointer() && value.is(ValueKind::Temp))
		return;
	if (use.first == kNoId)
		use.first = position;
	const bool write = (inst.op == Opcode::Store && operand == 1) || (inst.op == Opcode::CopyObject && operand == 1) ||
		(inst.op == Opcode::ZeroInit && operand == 0);
	if (write)
	{
		use.spread = use.spread || (use.write_block != kNoId && use.write_block != block);
		use.write_block = block;
		use.last_write = position;
		++use.writes;
	}
	else if (inst.op == Opcode::Load || (inst.op == Opcode::CopyObject && operand == 0) ||
		(value.is(ValueKind::Slot) && (inst.op == Opcode::Copy || inst.op == Opcode::Store)))
		use.read = true;
	else if (inst.op == Opcode::Return && value.is(ValueKind::Slot))
	{
		use.read = true;
		++use.returns;
	}
	else if (inst.op == Opcode::Call && operand >= 3)
	{
		const auto* signature = program_.call_boundary(inst);
		const uint32_t argument = operand - 3;
		if (signature != nullptr && argument < signature->parameter_count && address.known && address.offset == 0 &&
			program_.parameters_of(*signature)[argument].type == body_.slots[address.root.index].type)
		{
			++use.calls;
			use.call_block = block;
			use.call = position;
		}
		else
			use.escaped = true;
	}
	else
		use.escaped = true;
}

uint32_t Planner::whole_slot(Value destination, uint64_t bytes) const
{
	const auto address = opt::locate(body_, destination);
	if (!address.root.is(ValueKind::Slot) || !address.known || address.offset != 0)
		return kNoId;
	const Type type = body_.slots[address.root.index].type;
	return type.is_object() && storage_bytes(type) == bytes ? address.root.index : kNoId;
}

// A by-value parameter that passes through memory, and that the entry block
// copies whole into a slot before anything else touches the slot.
void Planner::parameters(ObjectPlaces& places) const
{
	if (body_.layout.empty())
		return;
	for (InstId i = body_.blocks[body_.layout[0]].first; i != kNoId; i = body_.instructions[i].next)
	{
		const auto& inst = body_.instructions[i];
		if (inst.op != Opcode::CopyObject || !inst.ops[0].is(ValueKind::Temp) ||
			inst.ops[0].index >= body_.parameter_count)
			continue;
		const uint32_t parameter = inst.ops[0].index;
		const Type type = body_.temps[parameter].type;
		if (boundary_.parameters[parameter].passing != Passing::Indirect || !type.is_object() ||
			reads_[parameter] != 1 || inst.immediate < 0 ||
			static_cast<uint64_t>(inst.immediate) != storage_bytes(type))
			continue;
		const uint32_t slot = whole_slot(inst.ops[1], storage_bytes(type));
		if (slot != kNoId && places.parameter_of[slot] == kNoId && uses_[slot].first == positions_[i])
			places.parameter_of[slot] = parameter;
	}
}

// The slot that most returns give back is the result object.
void Planner::result(ObjectPlaces& places) const
{
	if (boundary_.result.passing != Passing::Indirect || !signature_.result.is_object())
		return;
	uint32_t returns = 0;
	for (uint32_t s = 0; s < uses_.size(); ++s)
		if (uses_[s].returns > returns && places.parameter_of[s] == kNoId &&
			storage_bytes(body_.slots[s].type) == storage_bytes(signature_.result))
		{
			places.result = s;
			returns = uses_[s].returns;
		}
}

// The result of a call that only a complete copy into a slot reads, with no
// handler for the call in this function, is written into that slot: nothing
// else reaches a slot whose address is no value.
void Planner::call_results(ObjectPlaces& places) const
{
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next)
		{
			const auto& call = body_.instructions[i];
			if (call.op != Opcode::Call || call.result == kNoId || !call.type.is_object() || reads_[call.result] != 1 ||
				(eh_sites_ != nullptr && (*eh_sites_)[i] != 0))
				continue;
			InstId next = call.next;
			while (next != kNoId && body_.instructions[next].op == Opcode::Addr)
				next = body_.instructions[next].next;
			if (next == kNoId)
				continue;
			const auto& copy = body_.instructions[next];
			if (copy.op != Opcode::CopyObject || !(copy.ops[0] == Value::temp(call.result)) || copy.immediate < 0 ||
				static_cast<uint64_t>(copy.immediate) != storage_bytes(call.type))
				continue;
			const uint32_t slot = whole_slot(copy.ops[1], storage_bytes(call.type));
			if (slot == kNoId || uses_[slot].escaped)
				continue;
			places.slot_of[call.result] = slot;
			places.target[slot] = 1;
		}
}

ObjectPlaces Planner::run()
{
	ObjectPlaces places;
	places.parameter_of.assign(body_.slots.size(), kNoId);
	places.argument.assign(body_.slots.size(), 0);
	places.target.assign(body_.slots.size(), 0);
	places.slot_of.assign(body_.temps.size(), kNoId);
	uses_.assign(body_.slots.size(), SlotUse());
	reads_.assign(body_.temps.size(), 0);
	positions_.assign(body_.instructions.size(), 0);
	uint32_t position = 0;
	for (BlockId block : body_.layout)
		for (InstId i = body_.blocks[block].first; i != kNoId; i = body_.instructions[i].next, ++position)
		{
			const auto& inst = body_.instructions[i];
			positions_[i] = position;
			for (uint32_t k = 0; k < 3; ++k)
				touch(inst, inst.ops[k], k, block, position);
			for (uint32_t k = 0; k < inst.extra_count; ++k)
				touch(inst, body_.operands[inst.extra_first + k], k + 3, block, position);
		}
	for (uint32_t s = 0; s < uses_.size(); ++s)
	{
		const SlotUse& use = uses_[s];
		places.argument[s] = !use.escaped && !use.read && use.calls == 1 && use.writes != 0 && !use.spread &&
			use.write_block == use.call_block && use.last_write < use.call;
	}
	parameters(places);
	result(places);
	call_results(places);
	return places;
}

} // namespace

ObjectPlaces plan_object_places(const Program& program, const FunctionBody& body, const lowir::Signature& signature,
	const Boundary& boundary, const std::vector<uint32_t>* eh_sites)
{
	return Planner(program, body, signature, boundary, eh_sites).run();
}

} // namespace wasm
