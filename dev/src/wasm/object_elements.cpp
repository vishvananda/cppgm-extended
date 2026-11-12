#include "wasm/object_reader.h"

namespace wasm {

void read_object_elements(Cursor& input, Object& object)
{
	object.declared_functions.resize(object.functions.size());
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		const uint32_t flags = input.u32();
		if (flags > 7 || (flags & 3) == 1)
			throw WasmError("unsupported Wasm element segment mode");
		if ((flags & 1) == 0)
		{
			if (object.table_count != 1 || ((flags & 2) != 0 && input.u32() != 0))
				throw WasmError("invalid Wasm element table index");
			ObjectChunk offset = read_constant_expression(input);
			Cursor expression(offset.bytes);
			if (expression.byte() != 0x41)
				throw WasmError("Wasm C function-table declaration needs a constant offset");
			const int64_t base = expression.signed_leb();
			if ((base != 0 && base != 1) || expression.byte() != 0x0b)
				throw WasmError("independent Wasm table initialization is outside the static object convention");
			expression.done();
		}
		ObjectElement element;
		element.type = ValueType::FuncRef;
		element.declaration = (flags & 1) != 0;
		if ((flags & 4) != 0 && flags != 4)
			element.type = read_object_type(input);
		else if (flags != 0 && (flags & 4) == 0 && input.byte() != 0)
			throw WasmError("invalid Wasm element kind");
		if ((element.type != ValueType::FuncRef && element.type != ValueType::ExternRef) ||
			((flags & 1) == 0 && element.type != ValueType::FuncRef))
			throw WasmError("invalid Wasm element reference type");
		const uint32_t values = input.u32();
		for (uint32_t j = 0; j < values; ++j)
		{
			if ((flags & 4) != 0)
				element.values.push_back(read_constant_expression(input));
			else
			{
				const uint32_t function = input.u32();
				if (function >= object.functions.size())
					throw WasmError("invalid Wasm element function index");
				object.declared_functions[function] = 1;
			}
		}
		object.elements.push_back(std::move(element));
	}
}

} // namespace wasm
