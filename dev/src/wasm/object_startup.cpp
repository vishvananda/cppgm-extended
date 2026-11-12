#include "wasm/object.h"

namespace wasm {

void adapt_wasi_entry(Object& object, uint32_t index)
{
	ObjectSymbol& entry = object.symbols.at(index);
	ObjectFunction& function = object.functions.at(entry.index);
	const Signature& type = object.types.at(function.type);
	const bool arguments = type.parameters.size() == 2;
	if (type.results != std::vector<ValueType>{ ValueType::I32 } || (!arguments && !type.parameters.empty()) ||
		(arguments && type.parameters != std::vector<ValueType>{ ValueType::I32, ValueType::I32 }))
		throw WasmError("unsupported WASI main signature");
	entry.name = arguments ? "__main_argc_argv" : "__main_void";
	entry.flags &= ~uint32_t(0x20);
	function.name = entry.name;
	if (arguments || entry.undefined())
		return;
	const uint32_t entry_function = entry.index;
	ObjectSymbol alias = entry;
	alias.name = "__original_main";
	object.symbols.push_back(std::move(alias));
	ObjectFunction compatibility;
	compatibility.name = "main";
	compatibility.type = static_cast<uint32_t>(object.types.size());
	object.types.push_back(Signature{ { ValueType::I32, ValueType::I32 }, { ValueType::I32 } });
	compatibility.code.bytes.byte(0); // No locals; the compatibility parameters are ignored.
	compatibility.code.bytes.byte(0x10);
	compatibility.code.relocations.push_back(Relocation{ RelocationKind::FunctionIndex, 2, index, 0 });
	for (unsigned byte = 0; byte < 5; ++byte)
		compatibility.code.bytes.byte(((entry_function >> (7 * byte)) & 0x7f) | (byte == 4 ? 0 : 0x80));
	compatibility.code.bytes.byte(0x0b);
	ObjectSymbol symbol;
	symbol.name = "main";
	symbol.index = static_cast<uint32_t>(object.functions.size());
	object.functions.push_back(std::move(compatibility));
	object.symbols.push_back(std::move(symbol));
}

} // namespace wasm
