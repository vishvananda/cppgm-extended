#include "wasm/object_reader.h"

#include <map>
#include "wasm/validation.h"

namespace wasm {

namespace {

bool compatible(RelocationKind actual, RelocationKind expected)
{
	if (expected == RelocationKind::MemoryAddressSigned)
		return actual == expected || actual == RelocationKind::TableIndex;
	return actual == expected;
}

void decode(const ObjectFunction& source, const Object& object, Function& function)
{
	Cursor input(source.code.bytes);
	function.locals = object.types[source.type].parameters;
	function.parameter_count = static_cast<uint32_t>(function.locals.size());
	const uint32_t groups = input.u32();
	for (uint32_t i = 0; i < groups; ++i)
	{
		const uint32_t count = input.u32();
		const auto type = static_cast<ValueType>(input.byte());
		if (type == ValueType::Void || *spelling(type) == '\0' || count > 1000000 - function.locals.size())
			throw WasmError("invalid or excessive Wasm local declarations");
		function.locals.insert(function.locals.end(), count, type);
	}
	std::map<uint32_t, RelocationKind> fields;
	while (!input.empty())
	{
		unsigned opcode = input.byte();
		if (opcode == 0xfc || opcode == 0xfd)
		{
			const uint32_t suffix = input.u32();
			if (suffix > 255)
				throw WasmError("unsupported Wasm extended opcode");
			opcode = (opcode << 8) | suffix;
		}
		const Op op = static_cast<Op>(opcode);
		if (*spelling(op) == '\0')
			throw WasmError("unsupported opcode in Wasm object function");
		const Immediate form = immediate_form(op);
		const uint32_t first = static_cast<uint32_t>(function.operands.size());
		const uint32_t count = form == Immediate::None                ? 0
			: form == Immediate::Indices || form == Immediate::Memory ? 2
			: form == Immediate::Table                                ? input.u32() + 1
																	  : 1;
		if (form == Immediate::Table && count == 0)
			throw WasmError("Wasm branch table count overflow");
		for (uint32_t i = 0; i < count; ++i)
		{
			const uint32_t offset = input.position();
			uint64_t value = 0;
			if (form == Immediate::Signed)
				value = static_cast<uint64_t>(input.signed_leb(op == Op::I64Const ? 64 : 32));
			else if (form == Immediate::Float32 || form == Immediate::Float64)
				for (unsigned byte = 0; byte < (form == Immediate::Float32 ? 4u : 8u); ++byte)
					value |= uint64_t(input.byte()) << (byte * 8);
			else if (form == Immediate::BlockType)
				value = input.byte();
			else
				value = input.u32();
			function.operands.push_back(value);
			if (op == Op::I32Const)
				fields.emplace(offset, RelocationKind::MemoryAddressSigned);
			else if (form == Immediate::Memory && i == 1)
				fields.emplace(offset, RelocationKind::MemoryAddress);
			else if (op == Op::Call)
				fields.emplace(offset, RelocationKind::FunctionIndex);
			else if (op == Op::GlobalGet || op == Op::GlobalSet)
				fields.emplace(offset, RelocationKind::GlobalIndex);
			else if (op == Op::CallIndirect)
				fields.emplace(offset, i == 0 ? RelocationKind::TypeIndex : RelocationKind::TableNumber);
		}
		function.instructions.push_back(Instruction{ op, machine_model::Range{ first, count }, 0 });
	}
	for (const Relocation& relocation : source.code.relocations)
	{
		const auto found = fields.find(relocation.offset);
		if (found == fields.end() || !compatible(relocation.kind, found->second))
			throw WasmError("Wasm code relocation does not match an instruction operand");
		fields.erase(found);
	}
	for (const auto& field : fields)
		if (field.second == RelocationKind::FunctionIndex || field.second == RelocationKind::TypeIndex ||
			field.second == RelocationKind::GlobalIndex)
			throw WasmError("Wasm object index operand lacks a relocation");
}

} // namespace

void validate_object_code(const Object& object)
{
	Module module;
	module.signatures = object.types;
	for (const auto& function : object.functions)
		module.definition_types.push_back(function.type);
	for (const auto& source : object.functions)
		if (!source.imported)
		{
			Function function;
			decode(source, object, function);
			validate_function(function, object.types[source.type], module);
		}
}

} // namespace wasm
