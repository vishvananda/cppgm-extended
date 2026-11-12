#include "wasm/selector.h"

#include "machine/call_arguments.h"

namespace wasm {

using namespace lowir;

const VariadicPlan& Selector::variadic_plan(const lowir::Instruction& inst, const lowir::Signature& signature)
{
	if (variadic_plans_.entry.empty())
		variadic_plans_.reset(body_.instructions.size());
	const uint32_t id = static_cast<uint32_t>(&inst - body_.instructions.data());
	if (const auto* known = variadic_plans_.known(id))
		return *known;
	auto& plan = variadic_plans_.add(id);
	plan.arguments.clear();
	plan.alignment_log = 2;
	machine::call_argument_types(body_, inst, signature, program_.parameters_of(signature), argument_types_);
	for (uint32_t index = signature.parameter_count; index < inst.extra_count; ++index)
	{
		const Type type = argument_types_[index];
		const auto passing = classify_value(type,
			type.is_object() ? machine::argument_shape(body_, inst, index) : ir_model::AggregateShape(),
			program_.pointer_type());
		const Type storage = variadic_storage(type, passing);
		plan.arguments.push_back(VariadicArgument{ type, passing, storage });
		if (passing.passing != Passing::Ignore)
			while ((uint32_t(1) << plan.alignment_log) < data_alignment(storage))
				++plan.alignment_log;
	}
	return plan;
}

void Selector::pack_variadic(const lowir::Instruction& inst, const lowir::Signature& signature)
{
	if (inst.extra_count == signature.parameter_count)
	{
		out_.emit(Op::I32Const, { 0 });
		return;
	}
	const auto& plan = variadic_plan(inst, signature);
	for (uint32_t index = signature.parameter_count; index < inst.extra_count; ++index)
	{
		const auto& argument = plan.arguments[index - signature.parameter_count];
		const Type type = argument.type;
		const Value source = body_.operands[inst.extra_first + index];
		const auto passing = argument.passing;
		if (passing.passing == Passing::Ignore)
			continue;
		if (passing.passing == Passing::Indirect)
		{
			frame_address(variadic_copies_[inst.extra_first + index]);
			address(source);
			out_.emit(Op::I32Const, { type.bytes });
			out_.emit(Op::MemoryCopy, { 0, 0 });
			frame_address(call_areas_[inst.extra_first + index]);
			frame_address(variadic_copies_[inst.extra_first + index]);
			out_.emit(Op::I32Store, { 0, 0 });
		}
		else
		{
			frame_address(call_areas_[inst.extra_first + index]);
			if (type.kind() == TypeKind::I128)
			{
				wide_part(source, 0);
				out_.emit(Op::I64Store, { 0, 0 });
				frame_address(call_areas_[inst.extra_first + index]);
				wide_part(source, 1);
				out_.emit(Op::I64Store, { 0, 8 });
			}
			else if (type.is_object())
			{
				address(source);
				out_.emit(Op::I32Const, { type.bytes });
				out_.emit(Op::MemoryCopy, { 0, 0 });
			}
			else
			{
				value(source, type);
				out_.emit(memory_op(type, true), { 0, 0 });
			}
		}
	}
	const uint32_t area = variadic_areas_[&inst - body_.instructions.data()];
	if (area == kNoId)
		out_.emit(Op::I32Const, { 0 });
	else
		frame_address(area);
}

void Selector::variadic_start(const lowir::Instruction& inst)
{
	if (variadic_parameter_ == kNoId)
		throw WasmError("Wasm va_start requires a variadic function");
	address(inst.ops[0]);
	out_.emit(Op::LocalGet, { variadic_parameter_ });
	out_.emit(Op::I32Store, { 0, 0 });
}

void Selector::variadic_argument(const lowir::Instruction& inst)
{
	const auto passing = classify_value(inst.type,
		inst.type.is_object() ? machine::argument_shape(body_, inst, 0) : ir_model::AggregateShape(),
		program_.pointer_type());
	if (passing.passing == Passing::Ignore)
		return;
	const Type storage = variadic_storage(inst.type, passing);
	const uint32_t cursor = add_local(ValueType::I32);
	const uint32_t alignment = data_alignment(storage);
	address(inst.ops[0]);
	address(inst.ops[0]);
	out_.emit(Op::I32Load, { 0, 0 });
	if (alignment > 1)
	{
		out_.emit(Op::I32Const, { alignment - 1 });
		out_.emit(Op::I32Add);
		out_.emit(Op::I32Const, { static_cast<uint64_t>(-int64_t(alignment)) });
		out_.emit(Op::I32And);
	}
	out_.emit(Op::LocalTee, { cursor });
	out_.emit(Op::I32Const, { storage_bytes(storage) });
	out_.emit(Op::I32Add);
	out_.emit(Op::I32Store, { 0, 0 });
	if (memory_value(inst.type))
	{
		address(Value::temp(inst.result));
		out_.emit(Op::LocalGet, { cursor });
		if (passing.passing == Passing::Indirect)
			out_.emit(Op::I32Load, { 0, 0 });
		out_.emit(Op::I32Const, { storage_bytes(inst.type) });
		out_.emit(Op::MemoryCopy, { 0, 0 });
	}
	else
	{
		out_.emit(Op::LocalGet, { cursor });
		out_.emit(memory_op(inst.type, false), { 0, 0 });
		normalize(inst.type);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
	}
}

} // namespace wasm
