// A representation probe, not an AArch64 encoder or calling-convention
// implementation. Exercise the common call-fact reader with a four-element
// homogeneous aggregate and variable arity before target ABI classification.
#include "machine/call_arguments.h"
#include "machine_model.h"

#include <cassert>
#include <vector>

int main()
{
	using namespace lowir;
	FunctionBody body;
	body.instructions.resize(1);
	Instruction& call = body.instructions[0];
	call.op = Opcode::Call;
	call.extra_first = 0;
	call.extra_count = 17;
	body.operands.resize(call.extra_count);
	std::vector<Parameter> parameters(call.extra_count);
	for (unsigned i = 0; i < parameters.size(); ++i)
		parameters[i].type = Type::scalar(i % 2 ? TypeKind::F64 : TypeKind::I64);
	parameters[0].type = Type::object(16, 2);
	ir_model::AggregateShape hfa;
	hfa.element = ir_model::AggregateElement::F32;
	hfa.count = 4;
	parameters[0].shape = hfa;
	body.argument_facts.push_back(ArgumentFacts(0, 0, 0, hfa));
	Signature signature = {};
	signature.parameter_count = static_cast<uint32_t>(parameters.size());
	std::vector<Type> types;
	machine::call_argument_types(body, call, signature, parameters.data(), types);
	assert(types.size() == 17 && types[0] == Type::object(16, 2));
	assert(machine::argument_shape(body, call, 0) == hfa);
	assert(machine::argument_shape(body, call, 16).element == ir_model::AggregateElement::None);
	for (unsigned i = 1; i < types.size(); ++i)
		assert(types[i] == parameters[i].type);

	using namespace machine_model;
	// An AArch64 adapter can assign distinct IDs to GPR and vector views and
	// keep alias units in its own register description, outside common facts.
	const Location x0{ LocationKind::Register, 0, 0 };
	const Location w0{ LocationKind::Register, 32, 0 };
	const Location v0{ LocationKind::Register, 64, 0 };
	const unsigned alias_units[] = { 0, 0, 32 };
	assert(x0.index != w0.index && alias_units[0] == alias_units[1]);
	assert(v0.index > 31 && alias_units[2] != alias_units[0]);
	std::vector<Location> parts;
	for (unsigned i = 0; i < hfa.count; ++i)
		parts.push_back(Location{ LocationKind::Register, v0.index + i, 0 });
	const Range result{ 0, static_cast<uint32_t>(parts.size()) };
	assert(result.count == 4 && parts[result.first + 3].index == 67);
	const Location indirect_result{ LocationKind::Register, 8, 0 };
	const Location local{ LocationKind::Local, 400, 0 };
	const Location frame{ LocationKind::FrameObject, 7, -32768 };
	const Location incoming{ LocationKind::IncomingStack, 0, 8192 };
	assert(indirect_result.index == 8 && local.kind != indirect_result.kind);
	assert(frame.kind != incoming.kind && frame.offset < 0 && incoming.offset > 0);
	const FrameObject aligned{ 96, 32, 7 };
	assert(aligned.identity == frame.index && aligned.bytes % aligned.alignment == 0);
	// Target instruction payloads can have distinct destinations and sources,
	// or variable call/branch-table operands, without a shared fixed-size array.
	const Range three_address{ 100, 3 }, call_operands{ 103, 17 };
	assert(three_address.first + three_address.count == call_operands.first);
	assert(call_operands.count == types.size());
}
