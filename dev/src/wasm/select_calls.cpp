#include "wasm/selector.h"

namespace wasm {

using namespace lowir;

void Selector::call(const lowir::Instruction& inst)
{
	const lowir::Signature* signature = program_.call_boundary(inst);
	if (signature == nullptr ||
		(signature->boundary.arity == ir_model::CAM_FIXED ? signature->parameter_count != inst.extra_count
														  : signature->parameter_count > inst.extra_count))
		throw WasmError("Wasm call requires a matching typed boundary");
	if (builtin_call(inst, *signature))
		return;
	const uint32_t signature_id = static_cast<uint32_t>(signature - program_.signatures.data());
	const Boundary& boundary = module_.boundaries[signature_id];
	const Parameter* parameters = program_.parameters_of(*signature);
	// A coerced result is stored after the call; an indirect result's
	// destination is the first ABI argument. Both use the SSA value's storage.
	if (boundary.result.passing == Passing::Coerce || boundary.result.passing == Passing::Indirect)
		address(Value::temp(inst.result));
	for (uint32_t i = 0; i < signature->parameter_count; ++i)
	{
		const AbiValue argument = boundary.parameters[i];
		const Value source = body_.operands[inst.extra_first + i];
		if (argument.passing == Passing::Ignore)
			continue;
		if (argument.passing == Passing::Indirect)
		{
			const uint32_t offset = call_areas_[inst.extra_first + i];
			frame_address(offset);
			address(source);
			out_.emit(Op::I32Const, { parameters[i].type.bytes });
			out_.emit(Op::MemoryCopy, { 0, 0 });
			frame_address(offset);
		}
		else if (argument.passing == Passing::Coerce)
		{
			address(source);
			out_.emit(memory_op(argument.scalar, false), { 0, 0 });
		}
		else if (argument.passing == Passing::Expand)
		{
			address(source);
			out_.emit(Op::I64Load, { 0, 0 });
			address(source);
			out_.emit(Op::I64Load, { 0, 8 });
		}
		else
			value(source, argument.scalar);
	}
	if (signature->boundary.arity == ir_model::CAM_VARIADIC)
		pack_variadic(inst, *signature);
	if (inst.ops[0].is(ValueKind::Symbol))
	{
		const uint32_t function = module_.function_indices[inst.ops[0].index];
		if (function == kNoId)
			throw WasmError(
				"undefined Wasm function: " + std::string(program_.spelling(program_.symbols[inst.ops[0].index].name)));
		out_.emit(Op::Call, { function });
		out_.relocate(RelocationKind::FunctionIndex, inst.ops[0].index);
	}
	else
	{
		value(inst.ops[0], program_.pointer_type());
		out_.emit(Op::CallIndirect, { module_.signature_types[signature_id], 0 });
		out_.relocate(RelocationKind::TypeIndex, module_.signature_types[signature_id]);
		out_.relocate(RelocationKind::TableNumber, module_.table_symbol, 1);
	}
	if (signature->boundary.returns == ir_model::CRM_NORETURN)
	{
		out_.emit(Op::Unreachable);
		return;
	}
	if (boundary.result.passing == Passing::Coerce)
		out_.emit(memory_op(boundary.result.scalar, true), { 0, 0 });
	else if (!inst.type.is_void() && !inst.type.is_object())
	{
		normalize(inst.type);
		out_.emit(Op::LocalSet, { locals_[inst.result] });
	}
}

void Selector::return_value(const lowir::Instruction& inst)
{
	if (boundary_.result.passing == Passing::Indirect)
	{
		out_.emit(Op::LocalGet, { hidden_result_ });
		address(inst.ops[0]);
		out_.emit(Op::I32Const, { signature_.result.bytes });
		out_.emit(Op::MemoryCopy, { 0, 0 });
	}
	else if (boundary_.result.passing == Passing::Coerce)
	{
		address(inst.ops[0]);
		out_.emit(memory_op(boundary_.result.scalar, false), { 0, 0 });
	}
	else if (boundary_.result.passing == Passing::Direct && !inst.type.is_void())
		value(inst.ops[0], inst.type);
	restore_stack();
	out_.emit(Op::Return);
}

} // namespace wasm
