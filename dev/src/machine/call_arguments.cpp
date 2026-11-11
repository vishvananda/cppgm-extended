#include "machine/call_arguments.h"

#include <algorithm>

namespace machine {

ir_model::AggregateShape argument_shape(
	const lowir::FunctionBody& body, const lowir::Instruction& instruction, uint32_t argument)
{
	const auto id = static_cast<lowir::InstId>(&instruction - body.instructions.data());
	auto fact = std::lower_bound(body.argument_facts.begin(), body.argument_facts.end(), id,
		[](const lowir::ArgumentFacts& entry, lowir::InstId call) { return entry.call < call; });
	for (; fact != body.argument_facts.end() && fact->call == id; ++fact)
		if (fact->argument == argument && fact->shape().element != ir_model::AggregateElement::None)
			return fact->shape();
	return ir_model::AggregateShape();
}

void call_argument_types(const lowir::FunctionBody& body, const lowir::Instruction& inst,
	const lowir::Signature& signature, const lowir::Parameter* parameters, std::vector<lowir::Type>& output)
{
	using namespace lowir;
	output.clear();
	for (uint32_t argument = 0; argument < inst.extra_count; ++argument)
	{
		const Value value = body.operands[inst.extra_first + argument];
		Type type = value.is(ValueKind::Temp) ? body.temps[value.index].type : body.pointer_type();
		if (argument < signature.parameter_count)
			type = parameters[argument].type;
		else if (value.is(ValueKind::Float))
			type = Type::scalar(value.float_kind == TypeKind::F80 ? TypeKind::F80 : TypeKind::F64);
		else if (value.is(ValueKind::Integer))
			type = Type::scalar(TypeKind::I64);
		else if (value.is(ValueKind::Slot))
			type = body.slots[value.index].type;
		output.push_back(type);
	}
	apply_argument_types(body, inst, output);
}

void apply_argument_types(
	const lowir::FunctionBody& body, const lowir::Instruction& inst, std::vector<lowir::Type>& output)
{
	using namespace lowir;
	if (body.argument_facts.empty())
		return;
	const InstId id = static_cast<InstId>(&inst - body.instructions.data());
	auto fact = std::lower_bound(body.argument_facts.begin(), body.argument_facts.end(), id,
		[](const ArgumentFacts& entry, InstId instruction) { return entry.call < instruction; });
	for (; fact != body.argument_facts.end() && fact->call == id; ++fact)
		if (fact->type() != TypeKind::Void && fact->argument < output.size())
			output[fact->argument] = fact->type() == TypeKind::Ptr ? body.pointer_type() : Type::scalar(fact->type());
}

} // namespace machine
