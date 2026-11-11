#include "wasm/object.h"

#include "machine/global_data.h"
#include "wasm/object_cursor.h"
#include "wasm/selector.h"
#include "wasm/validation.h"

namespace wasm {

namespace {

uint32_t flags(const lowir::Symbol& symbol)
{
	return symbol.metadata.binding == ir_model::SBM_INTERNAL ? 2
		: symbol.metadata.binding == ir_model::SBM_WEAK      ? 1
															 : 0;
}

void relocate_symbols(std::vector<Relocation>& relocations, const std::vector<uint32_t>& symbols)
{
	for (Relocation& relocation : relocations)
		if (relocation.kind != RelocationKind::TypeIndex)
		{
			if (relocation.index >= symbols.size() || symbols[relocation.index] == lowir::kNoId)
				throw WasmError("Wasm relocation refers to a symbol without an object identity");
			relocation.index = symbols[relocation.index];
		}
}

ObjectSymbol imported_symbol(uint8_t kind, uint32_t index, const char* name)
{
	ObjectSymbol symbol;
	symbol.kind = kind;
	symbol.flags = 0x10;
	symbol.index = index;
	symbol.name = name;
	return symbol;
}

} // namespace

void make_object(lowir::Program& program, const backend::Options& options, Object& object, std::string& dump)
{
	Module module;
	module.relocatable = true;
	prepare_module(program, module);
	object.types = module.signatures;
	object.functions.resize(module.definition_types.size());
	std::vector<uint32_t> symbols(program.symbols.size() + 3, lowir::kNoId);
	for (lowir::SymbolId id : program.order)
	{
		const auto& source = program.symbols[id];
		if (!lowir::is_function_kind(source.kind) && !lowir::is_global_kind(source.kind))
			continue;
		ObjectSymbol symbol;
		symbol.name = program.spelling(source.metadata.object != 0 ? source.metadata.object : source.name);
		symbol.flags = flags(source);
		if ((source.metadata.flags & ir_model::SF_OBJECT_ROOT) != 0)
			symbol.flags |= 0x80;
		if (source.metadata.role == ir_model::SR_ENTRY)
			symbol.flags |= 0x20;
		if (lowir::is_function_kind(source.kind))
		{
			symbol.index = module.function_indices[id];
			ObjectFunction& function = object.functions[symbol.index];
			function.type = module.definition_types[symbol.index];
			function.name = symbol.name;
			function.imported = source.kind == lowir::SymbolKind::FunctionDeclaration;
			if (function.imported)
			{
				function.module = "env";
				symbol.flags |= 0x10;
			}
		}
		else
		{
			symbol.kind = 1;
			if (source.kind == lowir::SymbolKind::GlobalDeclaration)
				symbol.flags |= 0x10;
			else
			{
				if (source.metadata.storage == ir_model::GSM_THREAD_LOCAL)
					throw WasmError("thread-local data is not supported by the Wasm profile");
				ObjectData data;
				data.name = symbol.name;
				uint32_t alignment = 1;
				symbol.size = machine::global_size(program, program.globals[source.detail], alignment);
				while ((uint32_t(1) << data.alignment) < alignment)
					++data.alignment;
				data.contents.bytes.data.resize(symbol.size);
				std::vector<machine::GlobalAddress> fixups;
				machine::write_global(program, program.globals[source.detail], data.contents.bytes.data, 0, fixups);
				for (const auto& fixup : fixups)
					data.contents.relocations.push_back(Relocation{
						lowir::is_function_kind(program.symbols[fixup.target].kind) ? RelocationKind::TableIndexI32
																					: RelocationKind::MemoryAddressI32,
						static_cast<uint32_t>(fixup.offset), fixup.target, fixup.addend });
				symbol.index = static_cast<uint32_t>(object.data.size());
				object.data.push_back(std::move(data));
			}
		}
		if (symbol.weak() && !symbol.undefined() && (source.metadata.flags & ir_model::SF_REPLACEABLE) == 0)
		{
			ObjectGroup group;
			group.name = symbol.name;
			group.members.push_back({ symbol.kind == 0 ? uint8_t(1) : uint8_t(0), symbol.index });
			object.groups.push_back(std::move(group));
		}
		symbols[id] = static_cast<uint32_t>(object.symbols.size());
		object.symbols.push_back(std::move(symbol));
		if (source.metadata.role == ir_model::SR_INIT)
			object.initializers.push_back({ 65535, symbols[id] });
	}
	symbols[module.stack_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(2, 0, "__stack_pointer"));
	object.global_imports.push_back(ObjectGlobal{ "env", "__stack_pointer", ValueType::I32, true });
	symbols[module.stack_low_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(1, 0, "__stack_low"));
	symbols[module.table_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(5, 0, "__indirect_function_table"));
	for (lowir::SymbolId id : module.definitions)
	{
		Function function;
		Selector(program, module, id, function).run();
		ObjectFunction& destination = object.functions[module.function_indices[id]];
		validate_function(function, object.types[destination.type], module);
		if (!options.machine_ir.empty())
			dump_function(function, program.spelling(program.symbols[id].name), dump);
		Bytes encoded;
		encode_function(function, encoded, &destination.code.relocations);
		Cursor body(encoded);
		body.u32();
		const uint32_t prefix = body.position();
		destination.code.bytes = body.bytes();
		for (Relocation& relocation : destination.code.relocations)
			relocation.offset -= prefix;
		relocate_symbols(destination.code.relocations, symbols);
	}
	for (ObjectData& data : object.data)
		relocate_symbols(data.contents.relocations, symbols);
	object.features = { { '+', "mutable-globals" }, { '+', "sign-ext" }, { '+', "bulk-memory" },
		{ '+', "nontrapping-fptoint" }, { '+', "reference-types" } };
}

} // namespace wasm
