#include "wasm/object_reader.h"

namespace wasm {

ValueType global_type(const Object& object, uint32_t index)
{
	return index < object.global_imports.size() ? object.global_imports[index].type
												: object.global_definitions[index - object.global_imports.size()].type;
}

bool mutable_global(const Object& object, uint32_t index)
{
	return index < object.global_imports.size()
		? object.global_imports[index].mutable_value
		: object.global_definitions[index - object.global_imports.size()].mutable_value;
}

ValueType read_object_type(Cursor& input)
{
	const auto type = static_cast<ValueType>(input.byte());
	if (type == ValueType::Void || *spelling(type) == '\0')
		throw WasmError("unsupported Wasm object value type");
	return type;
}

ObjectChunk read_constant_expression(Cursor& input)
{
	ObjectChunk chunk;
	chunk.section_offset = input.position();
	for (;;)
	{
		const auto op = static_cast<Op>(input.byte());
		switch (op)
		{
		case Op::End:
			chunk.bytes = input.consumed(chunk.section_offset);
			return chunk;
		case Op::I32Const:
			input.signed_leb();
			break;
		case Op::I64Const:
			input.signed_leb(64);
			break;
		case Op::RefNull:
			input.signed_leb(33);
			break;
		case Op::F32Const:
			input.take(4);
			break;
		case Op::F64Const:
			input.take(8);
			break;
		case Op::GlobalGet:
		case Op::RefFunc:
			input.u32();
			break;
		case Op::I32Add:
		case Op::I32Sub:
		case Op::I32Mul:
		case Op::I64Add:
		case Op::I64Sub:
		case Op::I64Mul:
			break;
		default:
			if (static_cast<unsigned>(op) != 0xfd || input.u32() != 12)
				throw WasmError("unsupported instruction in Wasm constant expression");
			input.take(16);
			break;
		}
	}
}

void read_global_definitions(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		const ValueType type = read_object_type(input);
		const uint8_t mutability = input.byte();
		if (mutability > 1)
			throw WasmError("invalid Wasm global mutability");
		ObjectGlobalDefinition global;
		global.type = type;
		global.mutable_value = mutability != 0;
		global.initializer = read_constant_expression(input);
		object.global_definitions.push_back(std::move(global));
	}
}

} // namespace wasm
