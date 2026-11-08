// Ordinary LowIR fallbacks for selectors that remain variable. Constant
// callers are replaced with vector operations after required source inlining.
#include "lower/function_lowering.h"

namespace lower {

namespace {

uint32_t LaneBytes(lowir::VectorOp operation)
{
	using lowir::VectorOp;
	switch (operation)
	{
	case VectorOp::Palignr:
	case VectorOp::Pslldq:
	case VectorOp::Psrldq:
		return 1;
	case VectorOp::Pblendw:
	case VectorOp::Pshuflw:
	case VectorOp::Pshufhw:
		return 2;
	case VectorOp::Blendpd:
	case VectorOp::Shufpd:
	case VectorOp::Pclmulqdq:
		return 8;
	case VectorOp::Blendps:
	case VectorOp::Shufps:
	case VectorOp::Pshufd:
		return 4;
	default:
		throw LoweringError(0, "invalid deferred vector operation");
	}
}

} // namespace

RValue FunctionLowering::deferred_vector(
	lowir::VectorOp operation, const RValue& first, const RValue& second, const RValue& count)
{
	const lowir::SymbolId function = lowering_.source_vector_function(operation);
	const size_t base = arguments_.size();
	arguments_.push_back(first.value);
	arguments_.push_back(second.value);
	arguments_.push_back(count.value);
	RValue result;
	result.type = lowir::Type::scalar(lowir::TypeKind::V128);
	result.value = emit_call(result.type, lowir::Value::symbol(function), base, lowir::kNoId, false);
	return result;
}

RValue FunctionLowering::vector_helper_offset(
	lowir::VectorOp operation, uint32_t lane, const RValue& count, RValue& valid)
{
	using lowir::VectorOp;
	const lowir::Type word = count.type;
	const RValue position = integer(word, lane);
	if (operation == VectorOp::Palignr || operation == VectorOp::Psrldq || operation == VectorOp::Pslldq)
	{
		const bool left = operation == VectorOp::Pslldq;
		const bool pair = operation == VectorOp::Palignr;
		const RValue at = binary(left ? lowir::BinaryOp::Sub : lowir::BinaryOp::Add, word, position, count);
		valid = compare(left ? lowir::ComparePredicate::Uge : lowir::ComparePredicate::Ult, word, left ? position : at,
			left ? count : integer(word, pair ? 32 : 16));
		RValue offset = binary(lowir::BinaryOp::And, word, at, integer(word, pair ? 31 : 15));
		return pair ? binary(lowir::BinaryOp::Xor, word, offset, integer(word, 16)) : offset;
	}
	const uint32_t bytes = LaneBytes(operation);
	if (operation == VectorOp::Pblendw || operation == VectorOp::Blendps || operation == VectorOp::Blendpd)
	{
		const RValue selected = binary(
			lowir::BinaryOp::And, word, binary(lowir::BinaryOp::UShr, word, count, position), integer(word, 1));
		return binary(lowir::BinaryOp::Add, word, integer(word, lane * bytes),
			binary(lowir::BinaryOp::Mul, word, selected, integer(word, 16)));
	}
	const bool high = operation == VectorOp::Pshufhw;
	if ((high && lane < 4) || (operation == VectorOp::Pshuflw && lane >= 4))
		return integer(word, lane * bytes);
	const bool paired = operation == VectorOp::Shufps || operation == VectorOp::Shufpd;
	const uint32_t shift = (lane - (high ? 4 : 0)) * (bytes == 8 ? 1 : 2);
	const RValue picked = binary(lowir::BinaryOp::And, word,
		binary(lowir::BinaryOp::UShr, word, count, integer(word, shift)), integer(word, bytes == 8 ? 1 : 3));
	return binary(lowir::BinaryOp::Add, word, binary(lowir::BinaryOp::Mul, word, picked, integer(word, bytes)),
		integer(word,
			high                              ? 8
				: paired && lane >= 8 / bytes ? 16
											  : 0));
}

void FunctionLowering::lower_vector_helper(lowir::VectorOp operation)
{
	if (immediate_vector_helper(operation))
		return;
	const lowir::Type vector = lowir::Type::scalar(lowir::TypeKind::V128);
	const lowir::Type word = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue inputs[3];
	for (uint32_t i = 0; i < 3; ++i)
	{
		inputs[i].type = i < 2 ? vector : lowir::Type::scalar(lowir::TypeKind::I32);
		inputs[i].value = lowir::Value::temp(builder_.parameter(i));
	}
	const RValue count = convert_op(lowir::ConvertOp::ZExt, word, inputs[2]);
	Place pool = Place();
	pool.untyped = true;
	pool.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, lowir::Type::object(32, 4)));
	const lowir::Value base = address_of(pool);
	const auto at = [&](const RValue& offset) {
		Place place = Place();
		place.untyped = true;
		place.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, base, offset).value;
		return place;
	};
	store(vector, inputs[0], at(integer(word, 0)));
	store(vector, inputs[1], at(integer(word, 16)));
	RValue result;
	if (operation == lowir::VectorOp::Pclmulqdq)
	{
		const RValue low = binary(
			lowir::BinaryOp::Mul, word, binary(lowir::BinaryOp::And, word, count, integer(word, 1)), integer(word, 8));
		const RValue high = binary(lowir::BinaryOp::Add, word, integer(word, 16),
			binary(lowir::BinaryOp::UShr, word, binary(lowir::BinaryOp::And, word, count, integer(word, 16)),
				integer(word, 1)));
		const RValue a = load(word, at(low)), b = load(word, at(high));
		const RValue av = vector_instruction(lowir::VectorOp::Fromi64, &a);
		const RValue bv = vector_instruction(lowir::VectorOp::Fromi64, &b);
		result = vector_instruction(operation, &av, &bv, nullptr, 0);
	}
	else
	{
		const uint32_t bytes = LaneBytes(operation);
		const lowir::Type lane = lowir::Type::scalar(bytes == 1 ? lowir::TypeKind::I8
				: bytes == 2                                    ? lowir::TypeKind::I16
				: bytes == 4                                    ? lowir::TypeKind::I32
																: lowir::TypeKind::I64);
		Place out = Place();
		out.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Vector, vector));
		const lowir::Value destination = address_of(out);
		for (uint32_t i = 0; i < 16 / bytes; ++i)
		{
			RValue valid = RValue();
			const RValue offset = vector_helper_offset(operation, i, count, valid);
			RValue value = load(lane, at(offset));
			if (valid.value.kind != lowir::ValueKind::None)
				value = select_value(valid, value, integer(lane, 0));
			Place element = Place();
			element.untyped = true;
			element.storage = index(
				lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, destination, integer(word, i * bytes))
								  .value;
			store(lane, value, element);
		}
		result = load(vector, out);
	}
	lowir::Instruction ret = lowir::make_instruction(lowir::Opcode::Return, vector);
	ret.ops[0] = result.value;
	terminate(ret);
	if (!lowering_.streaming())
		builder_.finish();
}

} // namespace lower
