#pragma once

#include "native/errors.h"
#include "native/lowering/abi.h"
#include "native/mir/construction.h"
#include "native/lowering/selection.h"
#include "native/lowering/wide.h"

#include <vector>

namespace lowir_native
{
namespace return_detail
{

template <class Derived>
class ReturnLowering
{
protected:
	void emit_return(const lowir_model::Instruction& instruction,
		std::vector<mir_model::MirInstruction>& out)
	{
		using namespace build;
		Derived& lowerer = static_cast<Derived&>(*this);
		const lowir_model::LowType& source_type =
			instruction.type.kind == lowir_model::LTK_VOID ?
			instruction.type : lowerer.operand_type(instruction.first);
		mir_model::MirOperand scalar_return = reg_operand(XR_RAX);
		if (wide::is_integer(instruction.type))
		{
			const wide::Value value = lowerer.wide_value(instruction.first);
			wide::append_word_to_register(value, 0, XR_RAX, XR_R11, out);
			wide::append_word_to_register(value, 1, XR_RDX, XR_R11, out);
		}
		else if (instruction.type.kind == lowir_model::LTK_OBJECT)
		{
			if (instruction.type.storage_size > 16)
				native_errors::ThrowSource(
					"direct object return exceeds two SysV eightbytes");
			const std::size_t chunks =
				(instruction.type.storage_size + 7) / 8;
			// INTEGER eightbytes go in rax then rdx, SSE ones in xmm0 then
			// xmm1, as the function's result_classes name them.
			const std::uint8_t classes =
				lowerer.source_.boundary.result_classes;
			if (classes == lowir_model::kObjectClassesMemory)
				native_errors::ThrowSource(
					"direct object return in memory has no register form");
			const bool zero =
				instruction.first.kind == lowir_model::Operand::OP_INTEGER &&
				selection::integer_value(instruction.first) == 0;
			mir_model::MirOperand direct_storage;
			const bool direct = !zero && lowerer.direct_object_chunk_storage(
				instruction.first, 0, &direct_storage);
			if (!zero && !direct)
				lowerer.emit_operand_address(out, XR_R11, instruction.first);
			std::size_t gprs = 0, xmms = 0;
			for (std::size_t chunk = 0; chunk < chunks; ++chunk)
			{
				const unsigned eightbyte =
					lowir_model::object_eightbyte_class(classes, chunk);
				if (eightbyte == 1) continue;
				mir_model::MirOperand storage = direct ? direct_storage :
					dereference(XR_R11);
				storage.offset += static_cast<long long>(chunk * 8);
				if (eightbyte == 3)
				{
					lowerer.uses_scalar_float_ = true;
					const lowir_model::LowType& chunk_type =
						abi::object_sse_chunk_type(
							instruction.type.storage_size - chunk * 8);
					const mir_model::MirOperand target =
						xmm_operand(xmms++ ? XMM_1 : XMM_0);
					if (zero)
						append_float_move(out, target, float_immediate(0, 0,
							lowir_model::StringId()), chunk_type);
					else append_float_move(out, target, storage, chunk_type);
					continue;
				}
				const mir_model::MirOperand target =
					reg_operand(gprs++ ? XR_RDX : XR_RAX);
				if (zero) append_move(out, target, immediate(0));
				else append_load(out, target, storage,
					abi::object_chunk_type(
						instruction.type.storage_size - chunk * 8));
			}
		}
		else if (selection::is_extended_float(instruction.type))
		{
			lowerer.uses_scalar_float_ = true;
			mir_model::MirOperand source = lowerer.resolve(instruction.first);
			if (selection::is_floating(source_type) &&
				!lowir_model::same_lowir_type(source_type, instruction.type))
			{
				const mir_model::MirOperand converted =
					lowerer.allocate_named_temp_home(
						lowir_model::FPN_F80_RETURN, instruction.type);
				lowerer.append_float_width_conversion(out, converted, source,
					source_type, instruction.type);
				source = converted;
			}
			mir_model::MirInstruction result = machine_instruction(
				mir_model::MirInstruction::MI_FRET, instruction.type);
			append_operand(result, source);
			out.push_back(result);
			lowerer.consume(instruction.first);
			return;
		}
		else if (selection::is_scalar_float(instruction.type))
		{
			lowerer.uses_scalar_float_ = true;
			if (selection::is_extended_float(source_type))
				lowerer.append_float_width_conversion(out, xmm_operand(XMM_0),
					lowerer.resolve(instruction.first), source_type,
					instruction.type);
			else
				append_float_move(out, xmm_operand(XMM_0),
					lowerer.resolve(instruction.first), instruction.type);
		}
		else if (instruction.type.kind != lowir_model::LTK_VOID)
		{
			if (lowerer.is_frame_address(instruction.first)) {
				lowerer.emit_operand_address(out, XR_RAX, instruction.first);
				scalar_return = reg_operand(XR_RAX);
			}
			else
			{
				scalar_return = lowerer.resolve(instruction.first);
				if (scalar_return.kind != mir_model::MirOperand::OP_REG)
				{
					lowerer.move_value_to_register(out, XR_RAX, scalar_return,
						instruction.type);
					scalar_return = reg_operand(XR_RAX);
				}
			}
		}
		mir_model::MirInstruction ret =
			machine_instruction(mir_model::MirInstruction::MI_RET);
		if (instruction.type.kind != lowir_model::LTK_VOID &&
			!selection::is_scalar_float(instruction.type) &&
			!selection::is_extended_float(instruction.type) &&
			instruction.type.kind != lowir_model::LTK_OBJECT)
			append_operand(ret, scalar_return);
		out.push_back(ret);
		lowerer.consume(instruction.first);
	}
};

}
}
