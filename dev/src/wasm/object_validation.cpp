#include "wasm/object_reader.h"

#include <algorithm>
#include <set>

namespace wasm {

namespace {

void validate_chunk(const ObjectChunk& chunk, const Object& object)
{
	std::vector<std::pair<uint32_t, uint32_t>> ranges;
	for (const Relocation& relocation : chunk.relocations)
	{
		const unsigned kind = static_cast<unsigned>(relocation.kind);
		const bool fixed = kind == 2 || kind == 5 || kind == 13 || kind == 26;
		const uint32_t width = fixed ? 4 : 5;
		if (relocation.offset > chunk.bytes.data.size() || width > chunk.bytes.data.size() - relocation.offset)
			throw WasmError("Wasm relocation exceeds its code/data body");
		if (!fixed)
		{
			Cursor value(chunk.bytes.data.data() + relocation.offset, width);
			if (kind == 1 || kind == 4 || kind == 11 || kind == 12)
				value.signed_leb();
			else
				value.u32();
			value.done();
		}
		ranges.push_back({ relocation.offset, relocation.offset + width });
		if (kind == 6)
		{
			if (relocation.index >= object.types.size())
				throw WasmError("invalid Wasm type relocation index");
			continue;
		}
		if (relocation.index >= object.symbols.size())
			throw WasmError("invalid Wasm symbol relocation index");
		const uint8_t expected = kind <= 2 || kind == 12 || kind == 26 ? 0
			: kind <= 5 || kind == 11                                  ? 1
			: kind == 7 || kind == 13                                  ? 2
			: kind == 10                                               ? 4
																	   : 5;
		const ObjectSymbol& symbol = object.symbols[relocation.index];
		if ((kind == 7 || kind == 13) && symbol.kind <= 1)
		{
			bool matched = false;
			for (const ObjectGlobal& global : object.global_imports)
				matched = matched ||
					(global.module == (symbol.kind == 0 ? "GOT.func" : "GOT.mem") && global.name == symbol.name &&
						global.type == ValueType::I32);
			if (!matched)
				throw WasmError("Wasm address-global relocation lacks its typed GOT import");
		}
		else if (symbol.kind != expected)
			throw WasmError("Wasm relocation kind does not match its symbol");
	}
	std::sort(ranges.begin(), ranges.end());
	for (size_t i = 1; i < ranges.size(); ++i)
		if (ranges[i - 1].second > ranges[i].first)
			throw WasmError("overlapping Wasm relocations");
}

} // namespace

void validate_object(const Object& object)
{
	const std::set<std::string> supported{ "mutable-globals", "sign-ext", "nontrapping-fptoint", "bulk-memory",
		"bulk-memory-opt", "extended-const", "multivalue", "reference-types", "call-indirect-overlong", "simd128",
		"exception-handling" };
	for (const auto& feature : object.features)
		if ((feature.first != '+' && feature.first != '-' && feature.first != '=') ||
			(feature.first != '-' && supported.count(feature.second) == 0))
			throw WasmError("unsupported Wasm object feature: " + feature.second);
	for (const auto& function : object.functions)
	{
		if (function.type >= object.types.size())
			throw WasmError("Wasm function references an invalid signature");
		if (!function.imported && (function.code.bytes.data.empty() || function.code.bytes.data.back() != 0x0b))
			throw WasmError("Wasm function body is empty or unterminated");
		validate_chunk(function.code, object);
	}
	for (const auto& data : object.data)
		validate_chunk(data.contents, object);
	std::set<std::string> names;
	for (const ObjectExport& entry : object.exports)
	{
		if (!names.insert(entry.name).second || entry.function >= object.functions.size())
			throw WasmError("duplicate or invalid Wasm object export");
		bool found = false;
		for (const ObjectSymbol& symbol : object.symbols)
			found = found || (symbol.kind == 0 && symbol.index == entry.function);
		if (!found)
			throw WasmError("Wasm exported function has no linking symbol");
	}
	names.clear();
	for (const ObjectSymbol& symbol : object.symbols)
	{
		if (!symbol.local() && symbol.kind != 3 && !names.insert(symbol.name).second)
			throw WasmError("duplicate name in Wasm object symbol table: " + symbol.name);
		if (symbol.local() && symbol.undefined())
			throw WasmError("local Wasm symbol cannot be undefined");
		if (symbol.kind == 0)
		{
			if (symbol.index >= object.functions.size() ||
				object.functions[symbol.index].imported != symbol.undefined())
				throw WasmError("Wasm function symbol/import mismatch");
		}
		else if (symbol.kind == 1 && !symbol.undefined())
		{
			if (symbol.index >= object.data.size())
				throw WasmError("invalid Wasm data symbol segment");
			const uint64_t size = object.data[symbol.index].contents.bytes.data.size();
			if (symbol.offset > size || symbol.size > size - symbol.offset)
				throw WasmError("Wasm data symbol exceeds its segment");
		}
		else if (symbol.kind == 2 && (symbol.index >= object.global_imports.size() || !symbol.undefined()))
			throw WasmError("invalid Wasm global symbol");
		else if (symbol.kind == 5 && (symbol.index != 0 || !symbol.undefined()))
			throw WasmError("invalid Wasm table symbol");
	}
	for (const auto& initializer : object.initializers)
	{
		if (initializer.second >= object.symbols.size())
			throw WasmError("invalid Wasm initializer symbol");
		const auto& symbol = object.symbols[initializer.second];
		if (symbol.kind != 0 || symbol.undefined())
			throw WasmError("Wasm initializer is not a defined function");
		const Signature& type = object.types[object.functions[symbol.index].type];
		if (!type.parameters.empty() || type.result != ValueType::Void)
			throw WasmError("Wasm initializer signature is not void()");
	}
}

} // namespace wasm
