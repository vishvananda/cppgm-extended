#include "wasm/object_reader.h"

#include <map>
#include "wasm/validation.h"

namespace wasm {

namespace {

bool compatible(RelocationKind actual, RelocationKind expected)
{
	if (expected == RelocationKind::MemoryAddressSigned)
		return actual == expected || actual == RelocationKind::TableIndex ||
			actual == RelocationKind::MemoryAddressRelative || actual == RelocationKind::TableIndexRelative;
	return actual == expected;
}

struct CodeField
{
	RelocationKind kind;
	uint32_t operand;
};

std::vector<uint32_t> global_indices(const Object& object)
{
	std::map<std::pair<std::string, std::string>, uint32_t> imports;
	for (uint32_t i = 0; i < object.global_imports.size(); ++i)
		imports.emplace(std::make_pair(object.global_imports[i].module, object.global_imports[i].name), i);
	std::vector<uint32_t> indices(object.symbols.size(), lowir::kNoId);
	for (uint32_t i = 0; i < object.symbols.size(); ++i)
	{
		const ObjectSymbol& symbol = object.symbols[i];
		if (symbol.kind == 2)
			indices[i] = symbol.index;
		else if (symbol.kind <= 1)
		{
			const auto found = imports.find({ symbol.kind == 0 ? "GOT.func" : "GOT.mem", symbol.name });
			if (found != imports.end())
				indices[i] = found->second;
		}
	}
	return indices;
}

} // namespace

void decode_object_chunk(const ObjectChunk& chunk, const Signature& signature, const Object& object,
	const std::vector<uint32_t>& globals, Function& function, bool expression, bool declaration)
{
	Cursor input(chunk.bytes);
	function.locals = signature.parameters;
	function.parameter_count = static_cast<uint32_t>(function.locals.size());
	const uint32_t groups = expression ? 0 : input.u32();
	for (uint32_t i = 0; i < groups; ++i)
	{
		const uint32_t count = input.u32();
		const auto type = static_cast<ValueType>(input.byte());
		if (type == ValueType::Void || *spelling(type) == '\0' || function.locals.size() > 1000000 ||
			count > 1000000 - function.locals.size())
			throw WasmError("invalid or excessive Wasm local declarations");
		function.locals.insert(function.locals.end(), count, type);
	}
	std::map<uint32_t, CodeField> fields;
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
		if (form == Immediate::TryTable)
		{
			const uint32_t offset = input.position();
			const int64_t type = input.signed_leb(33);
			function.operands.push_back(static_cast<uint64_t>(type));
			if (type >= 0)
				fields.emplace(offset, CodeField{ RelocationKind::TypeIndex, first });
			const uint32_t catches = input.u32();
			function.operands.push_back(catches);
			for (uint32_t i = 0; i < catches; ++i)
			{
				const uint8_t kind = input.byte();
				if (kind > static_cast<unsigned>(CatchKind::AllRef))
					throw WasmError("invalid Wasm try_table catch kind");
				function.operands.push_back(kind);
				if (kind <= static_cast<unsigned>(CatchKind::TagRef))
				{
					const uint32_t field = input.position();
					const uint32_t operand = static_cast<uint32_t>(function.operands.size());
					function.operands.push_back(input.u32());
					fields.emplace(field, CodeField{ RelocationKind::TagIndex, operand });
				}
				function.operands.push_back(input.u32());
			}
			const uint32_t count = static_cast<uint32_t>(function.operands.size()) - first;
			function.instructions.push_back(Instruction{ op, machine_model::Range{ first, count }, 0 });
			continue;
		}
		const uint32_t count = form == Immediate::Bytes16 || form == Immediate::Shuffle ? 16
			: form == Immediate::MemoryLane                                             ? 3
			: form == Immediate::None                                                   ? 0
			: form == Immediate::Indices || form == Immediate::Memory                   ? 2
			: form == Immediate::Table                                                  ? input.u32() + 1
			: form == Immediate::Types                                                  ? input.u32()
																						: 1;
		if (form == Immediate::Table && count == 0)
			throw WasmError("Wasm branch table count overflow");
		if (form == Immediate::Types && count != 1)
			throw WasmError("Wasm typed select requires exactly one type");
		for (uint32_t i = 0; i < count; ++i)
		{
			const uint32_t offset = input.position();
			uint64_t value = 0;
			if (form == Immediate::Signed)
				value = static_cast<uint64_t>(input.signed_leb(op == Op::I64Const ? 64 : op == Op::RefNull ? 33 : 32));
			else if (form == Immediate::Float32 || form == Immediate::Float64)
				for (unsigned byte = 0; byte < (form == Immediate::Float32 ? 4u : 8u); ++byte)
					value |= uint64_t(input.byte()) << (byte * 8);
			else if (form == Immediate::BlockType)
				value = static_cast<uint64_t>(input.signed_leb(33));
			else if (form == Immediate::Types || form == Immediate::Bytes16 || form == Immediate::Shuffle ||
				form == Immediate::Lane || (form == Immediate::MemoryLane && i == 2))
				value = input.byte();
			else
				value = input.u32();
			const uint32_t operand = static_cast<uint32_t>(function.operands.size());
			function.operands.push_back(value);
			const auto field = [&](RelocationKind kind) { fields.emplace(offset, CodeField{ kind, operand }); };
			if (op == Op::I32Const)
				field(RelocationKind::MemoryAddressSigned);
			else if ((form == Immediate::Memory || form == Immediate::MemoryLane) && i == 1)
				field(RelocationKind::MemoryAddress);
			else if (op == Op::Call || op == Op::RefFunc)
				field(RelocationKind::FunctionIndex);
			else if (op == Op::GlobalGet || op == Op::GlobalSet)
				field(RelocationKind::GlobalIndex);
			else if (op == Op::Throw)
				field(RelocationKind::TagIndex);
			else if (op == Op::TableGet || op == Op::TableSet || op == Op::TableGrow || op == Op::TableSize ||
				op == Op::TableFill || op == Op::TableCopy)
				field(RelocationKind::TableNumber);
			else if (op == Op::CallIndirect)
				field(i == 0 ? RelocationKind::TypeIndex : RelocationKind::TableNumber);
			else if (form == Immediate::BlockType && static_cast<int64_t>(value) >= 0)
				field(RelocationKind::TypeIndex);
		}
		function.instructions.push_back(Instruction{ op, machine_model::Range{ first, count }, 0 });
	}
	for (const Relocation& relocation : chunk.relocations)
	{
		const auto found = fields.find(relocation.offset);
		if (found == fields.end() || !compatible(relocation.kind, found->second.kind))
			throw WasmError("Wasm code relocation does not match an instruction operand");
		uint64_t& operand = function.operands[found->second.operand];
		if (relocation.kind == RelocationKind::TypeIndex)
			operand = relocation.index;
		else if (relocation.kind == RelocationKind::FunctionIndex)
			operand = object.symbols[relocation.index].index;
		else if (relocation.kind == RelocationKind::GlobalIndex)
		{
			operand = globals[relocation.index];
			if (operand == lowir::kNoId)
				throw WasmError("Wasm global relocation has no typed global");
		}
		else if (relocation.kind == RelocationKind::TableNumber)
			operand = 0;
		else if (relocation.kind == RelocationKind::TagIndex)
			operand = object.symbols[relocation.index].index;
		fields.erase(found);
	}
	for (const auto& field : fields)
		if (!declaration &&
			(field.second.kind == RelocationKind::FunctionIndex || field.second.kind == RelocationKind::TypeIndex ||
				field.second.kind == RelocationKind::GlobalIndex || field.second.kind == RelocationKind::TagIndex))
			throw WasmError("Wasm object index operand lacks a relocation");
}

void validate_object_code(const Object& object)
{
	const std::vector<uint32_t> globals = global_indices(object);
	Module module;
	module.signatures = object.types;
	module.memory_count = object.memory_count;
	module.table_count = object.table_count;
	module.declared_functions = object.declared_functions;
	module.declared_functions.resize(object.functions.size());
	for (const ObjectExport& entry : object.exports)
		if (entry.kind == 0)
			module.declared_functions[entry.index] = 1;
	module.global_types.clear();
	module.mutable_globals.clear();
	for (const auto& global : object.global_imports)
	{
		module.global_types.push_back(global.type);
		module.mutable_globals.push_back(global.mutable_value);
	}
	for (const auto& global : object.global_definitions)
	{
		module.global_types.push_back(global.type);
		module.mutable_globals.push_back(global.mutable_value);
	}
	for (const auto& function : object.functions)
		module.definition_types.push_back(function.type);
	for (const ObjectTag& tag : object.tags)
		module.tag_types.push_back(tag.type);
	std::vector<Function> expressions;
	const auto decode_expression = [&](const ObjectChunk& chunk, ValueType type, bool declaration) {
		expressions.emplace_back();
		decode_object_chunk(chunk, Signature{ {}, { type } }, object, globals, expressions.back(), true, declaration);
		for (const Instruction& inst : expressions.back().instructions)
			if (inst.op == Op::RefFunc)
			{
				const uint64_t index = expressions.back().operands[inst.operands.first];
				if (index >= module.declared_functions.size())
					throw WasmError("invalid constant function reference");
				module.declared_functions[index] = 1;
			}
	};
	for (const auto& global : object.global_definitions)
		decode_expression(global.initializer, global.type, false);
	for (const auto& element : object.elements)
		for (const auto& value : element.values)
		{
			decode_expression(value, element.type, true);
			if (!element.declaration &&
				(expressions.back().instructions.size() != 2 || expressions.back().instructions[0].op != Op::RefFunc))
				throw WasmError("independent Wasm table initialization is outside the static object convention");
		}
	uint32_t expression = 0;
	for (const auto& global : object.global_definitions)
	{
		validate_constant_function(expressions[expression], global.type,
			static_cast<uint32_t>(object.global_imports.size()) + expression, module);
		++expression;
	}
	for (const auto& element : object.elements)
		for (size_t i = 0; i < element.values.size(); ++i)
			validate_constant_function(
				expressions[expression++], element.type, static_cast<uint32_t>(module.global_types.size()), module);
	for (const auto& source : object.functions)
		if (!source.imported)
		{
			Function function;
			decode_object_chunk(source.code, object.types[source.type], object, globals, function);
			validate_function(function, object.types[source.type], module);
		}
}

} // namespace wasm
