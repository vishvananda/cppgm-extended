#include "wasm/object_reader.h"

namespace wasm {

namespace {

void types(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		if (input.byte() != 0x60)
			throw WasmError("Wasm object type is not a function signature");
		Signature signature;
		const uint32_t parameters = input.u32();
		for (uint32_t j = 0; j < parameters; ++j)
			signature.parameters.push_back(read_object_type(input));
		const uint32_t results = input.u32();
		for (uint32_t j = 0; j < results; ++j)
			signature.results.push_back(read_object_type(input));
		object.types.push_back(std::move(signature));
	}
}

void limits(Cursor& input, uint32_t maximum)
{
	const uint32_t flags = input.u32();
	const uint32_t minimum = input.u32();
	if (flags > 1 || minimum > maximum)
		throw WasmError("unsupported Wasm memory/table limits");
	if (flags == 1)
	{
		const uint32_t limit = input.u32();
		if (limit < minimum || limit > maximum)
			throw WasmError("invalid Wasm maximum memory/table limit");
	}
}

void imports(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		const std::string module = input.string(), name = input.string();
		const uint8_t kind = input.byte();
		if (kind == 0)
		{
			ObjectFunction function;
			function.type = input.u32();
			function.module = module;
			function.name = name;
			function.imported = true;
			object.functions.push_back(std::move(function));
		}
		else if (kind == 1)
		{
			if (module != "env" || name != "__indirect_function_table" || input.byte() != 0x70)
				throw WasmError("unsupported Wasm table import");
			limits(input, UINT32_MAX);
			if (++object.table_count != 1)
				throw WasmError("multiple Wasm tables are outside the target profile");
		}
		else if (kind == 2)
		{
			if (module != "env" || name != "__linear_memory")
				throw WasmError("unsupported Wasm memory import");
			limits(input, 65536);
			if (++object.memory_count != 1)
				throw WasmError("multiple Wasm memories are outside the target profile");
		}
		else if (kind == 3)
		{
			const ValueType type = read_object_type(input);
			const uint8_t mutable_value = input.byte();
			if (mutable_value > 1)
				throw WasmError("invalid Wasm imported-global mutability");
			object.global_imports.push_back(ObjectGlobal{ module, name, type, mutable_value != 0 });
		}
		else if (kind == 4)
		{
			if (input.byte() != 0)
				throw WasmError("unsupported Wasm exception tag attribute");
			ObjectTag tag;
			tag.type = input.u32();
			tag.module = module;
			tag.name = name;
			tag.imported = true;
			object.tags.push_back(std::move(tag));
		}
		else
			throw WasmError("unsupported Wasm import kind");
	}
}

void functions(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		ObjectFunction function;
		function.type = input.u32();
		object.functions.push_back(std::move(function));
	}
}

void tags(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		if (input.byte() != 0)
			throw WasmError("unsupported Wasm exception tag attribute");
		ObjectTag tag;
		tag.type = input.u32();
		object.tags.push_back(std::move(tag));
	}
}

void exports(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		const std::string name = input.string();
		const uint8_t kind = input.byte();
		if (kind != 0 && kind != 3 && kind != 4)
			throw WasmError("unsupported Wasm object export kind");
		object.exports.push_back(ObjectExport{ name, input.u32(), kind });
	}
}

void code(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	uint32_t definitions = 0;
	for (auto& function : object.functions)
		if (!function.imported)
		{
			++definitions;
			const uint32_t size = input.u32();
			function.code.section_offset = input.position();
			function.code.bytes = input.take(size).bytes();
		}
	if (count != definitions)
		throw WasmError("Wasm object function/code count mismatch");
}

void data(Cursor& input, Object& object)
{
	const uint32_t count = input.u32();
	for (uint32_t i = 0; i < count; ++i)
	{
		ObjectData segment;
		const uint32_t flags = input.u32();
		if (flags != 0)
			throw WasmError("unsupported Wasm data segment mode");
		if (flags == 0)
		{
			if (input.byte() != 0x41)
				throw WasmError("Wasm object data offset is not an i32 constant");
			input.signed_leb();
			if (input.byte() != 0x0b)
				throw WasmError("unterminated Wasm data-offset expression");
		}
		const uint32_t size = input.u32();
		segment.contents.section_offset = input.position();
		segment.contents.bytes = input.take(size).bytes();
		object.data.push_back(std::move(segment));
	}
}

} // namespace

void read_object(const std::string& path, Object& object)
{
	object.path = path;
	read_object_bytes(read_link_file(path), object);
}

static void parse_object_bytes(const Bytes& bytes, Object& object)
{
	const std::string& path = object.path;
	Cursor input(bytes);
	if (input.take(8).bytes().data != std::vector<uint8_t>({ 0, 'a', 's', 'm', 1, 0, 0, 0 }))
		throw WasmError("input is not a wasm32 object: " + path);
	std::vector<uint8_t> sections;
	uint32_t seen = 0, last = 0, data_count = lowir::kNoId;
	const uint8_t rank[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 12, 13, 11, 6 };
	std::vector<std::pair<std::string, Bytes>> customs;
	while (!input.empty())
	{
		const uint8_t kind = input.byte();
		const uint32_t size = input.u32();
		Cursor section = input.take(size);
		if (kind > 13 || (kind != 0 && (seen & (1u << kind)) != 0))
			throw WasmError("invalid or duplicate Wasm object section");
		if (kind != 0)
		{
			if (rank[kind] <= last)
				throw WasmError("Wasm object sections are out of order");
			last = rank[kind];
		}
		seen |= 1u << kind;
		if (kind == 0)
		{
			const std::string name = section.string();
			if (name == "dylink" || name == "dylink.0")
				throw WasmError("dynamic Wasm linking metadata is outside the target profile");
			if (name == "linking" || name == "target_features" || name.compare(0, 6, "reloc.") == 0)
				customs.push_back({ name, section.bytes() });
			else
				section.bytes();
		}
		else if (kind == 1)
			types(section, object);
		else if (kind == 2)
			imports(section, object);
		else if (kind == 3)
			functions(section, object);
		else if (kind == 13)
			tags(section, object);
		else if (kind == 6)
			read_global_definitions(section, object);
		else if (kind == 7)
			exports(section, object);
		else if (kind == 9)
			read_object_elements(section, object);
		else if (kind == 10)
			code(section, object);
		else if (kind == 11)
			data(section, object);
		else if (kind == 12)
			data_count = section.u32();
		else
			throw WasmError("unsupported section in Wasm relocatable object");
		section.done();
		sections.push_back(kind);
	}
	if ((seen & (1u << 12)) != 0 && data_count != object.data.size())
		throw WasmError("Wasm object data-count section does not match data segments");
	read_object_customs(customs, sections, object);
	validate_object(object);
	if (!object.archive_member)
		validate_object_code(object);
}

void read_object_bytes(const Bytes& bytes, Object& object)
{
	try
	{
		parse_object_bytes(bytes, object);
	}
	catch (const WasmError& failure)
	{
		throw WasmError(object.path + ": " + failure.what());
	}
}

} // namespace wasm
