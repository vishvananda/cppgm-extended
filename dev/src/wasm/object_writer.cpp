#include "wasm/object.h"

namespace wasm {

bool relocation_addend(RelocationKind kind)
{
	return kind == RelocationKind::MemoryAddress || kind == RelocationKind::MemoryAddressSigned ||
		kind == RelocationKind::MemoryAddressI32 || kind == RelocationKind::MemoryAddressRelative;
}

namespace {

void custom(Bytes& output, const char* name, const Bytes& contents)
{
	Bytes section;
	section.string(name);
	section.append(contents);
	output.section(0, section);
}

void relocation_section(
	Bytes& output, const char* name, uint32_t section_index, const std::vector<Relocation>& relocations)
{
	if (relocations.empty())
		return;
	Bytes contents;
	contents.unsigned_leb(section_index);
	contents.unsigned_leb(relocations.size());
	for (const Relocation& relocation : relocations)
	{
		contents.byte(static_cast<uint8_t>(relocation.kind));
		contents.unsigned_leb(relocation.offset);
		contents.unsigned_leb(relocation.index);
		if (relocation_addend(relocation.kind))
			contents.signed_leb(relocation.addend);
	}
	custom(output, name, contents);
}

void append_chunk(const ObjectChunk& chunk, Bytes& section, std::vector<Relocation>& relocations)
{
	for (Relocation relocation : chunk.relocations)
	{
		relocation.offset += static_cast<uint32_t>(section.data.size());
		relocations.push_back(relocation);
	}
	section.append(chunk.bytes);
}

void linking_section(const Object& object, Bytes& output)
{
	Bytes linking;
	linking.unsigned_leb(2);
	Bytes segments;
	segments.unsigned_leb(object.data.size());
	for (const ObjectData& data : object.data)
	{
		segments.string(data.name);
		segments.unsigned_leb(data.alignment);
		segments.unsigned_leb(data.flags);
	}
	linking.section(5, segments);
	Bytes symbols;
	symbols.unsigned_leb(object.symbols.size());
	for (const ObjectSymbol& symbol : object.symbols)
	{
		symbols.byte(symbol.kind);
		symbols.unsigned_leb(symbol.flags);
		if (symbol.kind == 1)
		{
			symbols.string(symbol.name);
			if (!symbol.undefined())
			{
				symbols.unsigned_leb(symbol.index);
				symbols.unsigned_leb(symbol.offset);
				symbols.unsigned_leb(symbol.size);
			}
		}
		else
		{
			symbols.unsigned_leb(symbol.index);
			if (!symbol.undefined() || (symbol.flags & 0x40) != 0)
				symbols.string(symbol.name);
		}
	}
	linking.section(8, symbols);
	if (!object.groups.empty())
	{
		Bytes groups;
		groups.unsigned_leb(object.groups.size());
		for (const auto& group : object.groups)
		{
			groups.string(group.name);
			groups.unsigned_leb(0);
			groups.unsigned_leb(group.members.size());
			for (const auto& member : group.members)
			{
				groups.byte(member.first);
				groups.unsigned_leb(member.second);
			}
		}
		linking.section(7, groups);
	}
	if (!object.initializers.empty())
	{
		Bytes init;
		init.unsigned_leb(object.initializers.size());
		for (const auto& function : object.initializers)
		{
			init.unsigned_leb(function.first);
			init.unsigned_leb(function.second);
		}
		linking.section(6, init);
	}
	custom(output, "linking", linking);
}

} // namespace

void write_object(const Object& object, Bytes& output)
{
	output.fixed(0x6d736100, 4);
	output.fixed(1, 4);
	Bytes types;
	types.unsigned_leb(object.types.size());
	for (const Signature& signature : object.types)
	{
		types.byte(0x60);
		types.unsigned_leb(signature.parameters.size());
		for (ValueType type : signature.parameters)
			types.byte(static_cast<uint8_t>(type));
		types.unsigned_leb(signature.result == ValueType::Void ? 0 : 1);
		if (signature.result != ValueType::Void)
			types.byte(static_cast<uint8_t>(signature.result));
	}
	output.section(1, types);
	Bytes imports;
	uint32_t imported_functions = 0;
	for (const auto& function : object.functions)
		imported_functions += function.imported;
	imports.unsigned_leb(imported_functions + object.global_imports.size() + 2);
	imports.string("env");
	imports.string("__linear_memory");
	imports.byte(2);
	imports.byte(0);
	imports.unsigned_leb(0);
	for (const auto& function : object.functions)
		if (function.imported)
		{
			imports.string(function.module);
			imports.string(function.name);
			imports.byte(0);
			imports.unsigned_leb(function.type);
		}
	for (const ObjectGlobal& global : object.global_imports)
	{
		imports.string(global.module);
		imports.string(global.name);
		imports.byte(3);
		imports.byte(static_cast<uint8_t>(global.type));
		imports.byte(global.mutable_value ? 1 : 0);
	}
	imports.string("env");
	imports.string("__indirect_function_table");
	imports.byte(1);
	imports.byte(0x70);
	imports.byte(0);
	imports.unsigned_leb(object.functions.size() + 1);
	output.section(2, imports);
	Bytes functions;
	functions.unsigned_leb(object.functions.size() - imported_functions);
	for (const auto& function : object.functions)
		if (!function.imported)
			functions.unsigned_leb(function.type);
	output.section(3, functions);
	Bytes elements;
	elements.unsigned_leb(1);
	elements.byte(0);
	elements.byte(0x41);
	elements.signed_leb(1);
	elements.byte(0x0b);
	elements.unsigned_leb(object.functions.size());
	for (uint32_t i = 0; i < object.functions.size(); ++i)
		elements.unsigned_leb(i);
	output.section(9, elements);
	Bytes code;
	std::vector<Relocation> code_relocations;
	code.unsigned_leb(object.functions.size() - imported_functions);
	for (const auto& function : object.functions)
		if (!function.imported)
		{
			code.unsigned_leb(function.code.bytes.data.size());
			append_chunk(function.code, code, code_relocations);
		}
	output.section(10, code);
	Bytes data;
	std::vector<Relocation> data_relocations;
	data.unsigned_leb(object.data.size());
	for (const auto& segment : object.data)
	{
		data.byte(0);
		data.byte(0x41);
		data.signed_leb(0);
		data.byte(0x0b);
		data.unsigned_leb(segment.contents.bytes.data.size());
		append_chunk(segment.contents, data, data_relocations);
	}
	output.section(11, data);
	linking_section(object, output);
	relocation_section(output, "reloc.CODE", 4, code_relocations);
	relocation_section(output, "reloc.DATA", 5, data_relocations);
	Bytes features;
	features.unsigned_leb(object.features.size());
	for (const auto& feature : object.features)
	{
		features.byte(feature.first);
		features.string(feature.second);
	}
	custom(output, "target_features", features);
}

} // namespace wasm
