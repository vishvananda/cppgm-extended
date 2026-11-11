#include "wasm/object_reader.h"

namespace wasm {

namespace {

void symbols(Cursor& input, Object& object)
{
	if (!object.symbols.empty())
		throw WasmError("duplicate Wasm symbol table");
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		ObjectSymbol symbol;
		symbol.kind = input.byte();
		symbol.flags = input.u32();
		if ((symbol.flags & 0x103) == 3 || (symbol.flags & 0x100) != 0)
			throw WasmError("unsupported common or thread-local Wasm symbol");
		if (symbol.kind == 1)
		{
			symbol.name = input.string();
			if (!symbol.undefined())
			{
				symbol.index = input.u32();
				symbol.offset = input.u32();
				symbol.size = input.u32();
			}
		}
		else if (symbol.kind == 0 || symbol.kind == 2 || symbol.kind == 5)
		{
			symbol.index = input.u32();
			if (!symbol.undefined() || (symbol.flags & 0x40) != 0)
				symbol.name = input.string();
			else if (symbol.kind == 0 && symbol.index < object.functions.size())
				symbol.name = object.functions[symbol.index].name;
			else if (symbol.kind == 2 && symbol.index < object.global_imports.size())
				symbol.name = object.global_imports[symbol.index].name;
			else if (symbol.kind == 5 && symbol.index == 0)
				symbol.name = "__indirect_function_table";
			else
				throw WasmError("invalid imported Wasm symbol index");
		}
		else if (symbol.kind == 3)
			symbol.index = input.u32();
		else
			throw WasmError("unsupported Wasm symbol kind");
		object.symbols.push_back(std::move(symbol));
	}
}

ObjectChunk* find_chunk(Object& object, uint8_t section, uint32_t offset)
{
	if (section == 10)
	{
		for (auto& function : object.functions)
			if (!function.imported && offset >= function.code.section_offset &&
				uint64_t(offset) < uint64_t(function.code.section_offset) + function.code.bytes.data.size())
				return &function.code;
	}
	else if (section == 11)
		for (auto& data : object.data)
			if (offset >= data.contents.section_offset &&
				uint64_t(offset) < uint64_t(data.contents.section_offset) + data.contents.bytes.data.size())
				return &data.contents;
	return nullptr;
}

} // namespace

void read_linking(Cursor& input, Object& object)
{
	if (input.u32() != 2)
		throw WasmError("unsupported Wasm linking metadata version");
	while (!input.empty())
	{
		const uint8_t kind = input.byte();
		const uint32_t size = input.u32();
		Cursor subsection = input.take(size);
		if (kind == 8)
			symbols(subsection, object);
		else if (kind == 5)
		{
			const uint32_t count = subsection.u32();
			if (count != object.data.size())
				throw WasmError("Wasm segment metadata count mismatch");
			for (ObjectData& data : object.data)
			{
				data.name = subsection.string();
				data.alignment = subsection.u32();
				data.flags = subsection.u32();
				if (data.alignment > 30 || (data.flags & ~uint32_t(5)) != 0)
					throw WasmError("unsupported Wasm data-segment flags or alignment");
			}
		}
		else if (kind == 6)
		{
			const uint32_t count = subsection.u32();
			for (uint32_t i = 0; i < count; ++i)
			{
				const uint32_t priority = subsection.u32();
				object.initializers.push_back({ priority, subsection.u32() });
			}
		}
		else if (kind == 7)
		{
			const uint32_t count = subsection.u32();
			for (uint32_t i = 0; i < count; ++i)
			{
				ObjectGroup group;
				group.name = subsection.string();
				if (subsection.u32() != 0)
					throw WasmError("unsupported Wasm COMDAT flags");
				const uint32_t members = subsection.u32();
				for (uint32_t j = 0; j < members; ++j)
				{
					const uint8_t type = subsection.byte();
					group.members.push_back({ type, subsection.u32() });
				}
				object.groups.push_back(std::move(group));
			}
		}
		else if (kind == 9)
		{
			if (subsection.string() != "wasm32")
				throw WasmError("Wasm object target architecture mismatch");
		}
		else
			throw WasmError("unsupported Wasm linking subsection");
		subsection.done();
	}
}

void read_relocations(Cursor& input, Object& object, uint8_t section)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		const auto kind = static_cast<RelocationKind>(input.byte());
		const uint32_t offset = input.u32();
		const uint32_t index = input.u32();
		const int64_t addend = relocation_addend(kind) ? input.signed_leb() : 0;
		const unsigned value = static_cast<unsigned>(kind);
		if (value > 7 && value != 10 && value != 11 && value != 12 && value != 13 && value != 20 && value != 26)
			throw WasmError("unsupported Wasm relocation kind: " + std::to_string(value));
		ObjectChunk* chunk = find_chunk(object, section, offset);
		if (chunk == nullptr)
			throw WasmError("Wasm relocation does not lie within a code or data body");
		const uint32_t relative = offset - chunk->section_offset;
		const uint32_t width = kind == RelocationKind::TableIndexI32 || kind == RelocationKind::MemoryAddressI32 ? 4
																												 : 5;
		if (relative > chunk->bytes.data.size() || width > chunk->bytes.data.size() - relative)
			throw WasmError("Wasm relocation crosses a body boundary");
		chunk->relocations.push_back(Relocation{ kind, relative, index, addend });
	}
}

} // namespace wasm
