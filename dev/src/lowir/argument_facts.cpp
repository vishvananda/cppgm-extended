#include "lowir/argument_facts.h"

#include <map>

namespace lowir {

bool valid_aggregate_shape(Type type, ir_model::AggregateShape shape)
{
	using ir_model::AggregateElement;
	if (static_cast<unsigned>(shape.element) >= ir_model::kAggregateElementCount || shape.reserved != 0)
		return false;
	if (shape.element == AggregateElement::None)
		return shape.count == 0;
	return type.is_object() && ((shape.element > AggregateElement::Mixed) == (shape.count != 0));
}

bool valid_this_return(const Program& program, const Signature& signature)
{
	return !signature.boundary.this_return ||
		(target_model::cxx_abi(program.target).structor_returns_this && signature.result.is_void() &&
			signature.parameter_count != 0 && program.parameters_of(signature)[0].type == program.pointer_type() &&
			program.parameters_of(signature)[0].pass != ir_model::PPM_INDIRECT_RESULT);
}

const char* validate_argument_facts(const Program& program, const FunctionBody& body, InstId& instruction)
{
	std::map<std::pair<InstId, uint32_t>, uint8_t> seen;
	InstId previous = 0;
	for (const ArgumentFacts& fact : body.argument_facts)
	{
		instruction = fact.call;
		if (fact.call >= body.instructions.size() || fact.call < previous)
			return "invalid or unordered argument fact instruction";
		previous = fact.call;
		const Instruction& call = body.instructions[fact.call];
		if ((call.op != Opcode::Call && call.op != Opcode::VaArg) ||
			(call.op == Opcode::Call ? fact.argument >= call.extra_count : fact.argument != 0))
			return "argument fact does not name a call argument or va_arg result";
		if (fact.classes != 0 && !program.object_facts)
			return "argument classes require object facts";
		if (fact.type() != TypeKind::Void)
		{
			if (static_cast<unsigned>(fact.type()) >= static_cast<unsigned>(TypeKind::Obj))
				return "argument_type requires a scalar value type";
			const Signature* signature = program.call_boundary(call);
			if (call.op != Opcode::Call || signature == nullptr ||
				signature->boundary.arity != ir_model::CAM_VARIADIC || fact.argument < signature->parameter_count)
				return "argument_type requires an unnamed variadic call argument";
			const Value value = body.operands[call.extra_first + fact.argument];
			const Type passed = fact.type() == TypeKind::Ptr ? body.pointer_type() : Type::scalar(fact.type());
			if (value.is(ValueKind::Temp) || value.is(ValueKind::Slot))
			{
				const Type actual = value.is(ValueKind::Temp) ? body.temps[value.index].type
															  : body.slots[value.index].type;
				if (actual != passed && !(actual.is_integer() && passed.is_integer()))
					return "argument_type does not match its value representation";
			}
			else if ((value.is(ValueKind::Float) && (!passed.is_float() || value.float_kind != passed.kind())) ||
				(value.is(ValueKind::Symbol) && !passed.is_pointer()) ||
				(value.is(ValueKind::Integer) && !(passed.is_integer() || passed.is_pointer())))
				return "argument_type does not match its literal representation";
		}
		const auto shape = fact.shape();
		if (shape.element != ir_model::AggregateElement::None)
		{
			Type type = call.type;
			if (call.op == Opcode::Call)
			{
				const Signature* signature = program.call_boundary(call);
				if (signature == nullptr || signature->boundary.arity != ir_model::CAM_VARIADIC ||
					fact.argument < signature->parameter_count)
					return "argument_shape requires an unnamed variadic argument";
				const Value value = body.operands[call.extra_first + fact.argument];
				if (!value.is(ValueKind::Temp) && !value.is(ValueKind::Slot))
					return "argument_shape requires an aggregate operand";
				type = value.is(ValueKind::Temp) ? body.temps[value.index].type : body.slots[value.index].type;
			}
			if (!valid_aggregate_shape(type, shape))
				return "invalid aggregate argument or va_arg shape";
		}
		const uint8_t fields = (fact.classes != 0 ? 1 : 0) | (fact.type() != TypeKind::Void ? 2 : 0) |
			(shape.element != ir_model::AggregateElement::None ? 4 : 0);
		uint8_t& prior = seen[{ fact.call, fact.argument }];
		if (fields == 0 || (prior & fields) != 0)
			return "empty or duplicate argument fact";
		prior |= fields;
	}
	instruction = kNoId;
	return nullptr;
}

} // namespace lowir
