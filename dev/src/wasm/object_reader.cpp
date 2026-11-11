#include "wasm/object_reader.h"

#include <fstream>
#include <iterator>

namespace wasm {

namespace {

ValueType read_type(Cursor& input)
{
	const auto type = static_cast<ValueType>(input.byte());
	if (type != ValueType::I32 && type != ValueType::I64 && type != ValueType::F32 && type != ValueType::F64 &&
		type != ValueType::V128)
		throw WasmError("unsupported Wasm object value type");
	return type;
}

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
			signature.parameters.push_back(read_type(input));
		const uint32_t results = input.u32();
		if (results > 1)
			throw WasmError("Wasm object multi-result signatures need legalization");
		signature.result = results == 0 ? ValueType::Void : read_type(input);
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
		}
		else if (kind == 2)
		{
			if (module != "env" || name != "__linear_memory")
				throw WasmError("unsupported Wasm memory import");
			limits(input, 65536);
		}
		else if (kind == 3)
		{
			const ValueType type = read_type(input);
			const uint8_t mutable_value = input.byte();
			if (mutable_value > 1)
				throw WasmError("invalid Wasm imported-global mutability");
			object.global_imports.push_back(ObjectGlobal{ module, name, type, mutable_value != 0 });
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
		if (flags != 0 && flags != 1)
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
	std::ifstream file(path, std::ios::binary);
	if (!file)
		throw WasmError("cannot read Wasm object: " + path);
	Bytes bytes;
	bytes.data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	read_object_bytes(bytes, object);
}

static void parse_object_bytes(const Bytes& bytes, Object& object)
{
	const std::string& path = object.path;
	Cursor input(bytes);
	if (input.take(8).bytes().data != std::vector<uint8_t>({ 0, 'a', 's', 'm', 1, 0, 0, 0 }))
		throw WasmError("input is not a wasm32 object: " + path);
	std::vector<uint8_t> sections;
	uint32_t seen = 0;
	bool linking = false;
	while (!input.empty())
	{
		const uint8_t kind = input.byte();
		const uint32_t size = input.u32();
		Cursor section = input.take(size);
		if (kind > 13 || (kind != 0 && (seen & (1u << kind)) != 0))
			throw WasmError("invalid or duplicate Wasm object section");
		seen |= 1u << kind;
		if (kind == 0)
		{
			const std::string name = section.string();
			if (name == "linking")
			{
				if (linking)
					throw WasmError("duplicate Wasm linking metadata");
				linking = true;
				read_linking(section, object);
			}
			else if (name.compare(0, 6, "reloc.") == 0)
			{
				const uint32_t target = section.u32();
				if (target >= sections.size())
					throw WasmError("invalid Wasm relocation target section");
				if (sections[target] == 0)
					section.bytes(); // Custom/debug payloads are not retained in the linked module.
				else
					read_relocations(section, object, sections[target]);
			}
			else if (name == "target_features")
			{
				const uint32_t count = section.u32();
				for (uint32_t i = 0; i < count; ++i)
				{
					const char prefix = section.byte();
					object.features.push_back({ prefix, section.string() });
				}
			}
			else
				section.bytes();
		}
		else if (kind == 1)
			types(section, object);
		else if (kind == 2)
			imports(section, object);
		else if (kind == 3)
			functions(section, object);
		else if (kind == 9)
			section.bytes();
		else if (kind == 10)
			code(section, object);
		else if (kind == 11)
			data(section, object);
		else if (kind == 12)
			section.u32();
		else
			throw WasmError("unsupported section in Wasm relocatable object");
		section.done();
		sections.push_back(kind);
	}
	if (!linking)
		throw WasmError("Wasm input lacks relocatable-object linking metadata");
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
