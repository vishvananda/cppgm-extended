#include "wasm/encoder.h"

#include <sstream>

namespace wasm {

ValueType value_type(lowir::Type type)
{
	using lowir::TypeKind;
	switch (type.kind())
	{
	case TypeKind::Void:
		return ValueType::Void;
	case TypeKind::I1:
	case TypeKind::I8:
	case TypeKind::U8:
	case TypeKind::I16:
	case TypeKind::U16:
	case TypeKind::I32:
	case TypeKind::U32:
		return ValueType::I32;
	case TypeKind::Ptr:
		if (type.pointer_bytes() != 4)
			throw WasmError("Wasm requires a 32-bit pointer layout");
		return ValueType::I32;
	case TypeKind::I64:
		return ValueType::I64;
	case TypeKind::F32:
		return ValueType::F32;
	case TypeKind::F64:
		return ValueType::F64;
	case TypeKind::V128:
		return ValueType::V128;
	default:
		throw WasmError("Wasm value requires wide/aggregate legalization");
	}
}

void encode_function(const Function& function, Bytes& output)
{
	Bytes body;
	uint32_t groups = 0;
	for (uint32_t i = function.parameter_count; i < function.locals.size(); ++i)
		if (i == function.parameter_count || function.locals[i] != function.locals[i - 1])
			++groups;
	body.unsigned_leb(groups);
	for (uint32_t i = function.parameter_count; i < function.locals.size();)
	{
		const uint32_t first = i;
		while (i < function.locals.size() && function.locals[i] == function.locals[first])
			++i;
		body.unsigned_leb(i - first);
		body.byte(static_cast<uint8_t>(function.locals[first]));
	}
	for (const Instruction& inst : function.instructions)
	{
		const unsigned opcode = static_cast<unsigned>(inst.op);
		if (opcode > 0xff)
		{
			body.byte(opcode >> 8);
			body.unsigned_leb(opcode & 0xff);
		}
		else
			body.byte(opcode);
		const uint64_t* operands = inst.operands.count == 0 ? nullptr : function.operands.data() + inst.operands.first;
		switch (immediate_form(inst.op))
		{
		case Immediate::None:
			break;
		case Immediate::Signed:
			body.signed_leb(static_cast<int64_t>(operands[0]));
			break;
		case Immediate::Float32:
			body.fixed(operands[0], 4);
			break;
		case Immediate::Float64:
			body.fixed(operands[0], 8);
			break;
		case Immediate::BlockType:
			body.byte(static_cast<uint8_t>(operands[0]));
			break;
		case Immediate::Table:
			body.unsigned_leb(inst.operands.count - 1);
			for (uint32_t i = 0; i < inst.operands.count; ++i)
				body.unsigned_leb(operands[i]);
			break;
		default:
			for (uint32_t i = 0; i < inst.operands.count; ++i)
				body.unsigned_leb(operands[i]);
		}
	}
	output.unsigned_leb(body.data.size());
	output.append(body);
}

void dump_function(const Function& function, const char* name, std::string& output)
{
	std::ostringstream stream;
	stream << "function " << name << " frame_bytes " << function.frame_bytes << '\n';
	for (uint32_t i = 0; i < function.locals.size(); ++i)
		stream << "  local " << i << ' ' << spelling(function.locals[i]) << '\n';
	for (const Instruction& inst : function.instructions)
	{
		stream << "  " << spelling(inst.op);
		for (uint32_t i = 0; i < inst.operands.count; ++i)
			stream << ' ' << function.operands[inst.operands.first + i];
		stream << '\n';
	}
	stream << "end_function\n";
	output += stream.str();
}

} // namespace wasm
