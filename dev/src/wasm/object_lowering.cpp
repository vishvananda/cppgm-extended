#include "wasm/object.h"

#include "machine/global_data.h"
#include "wasm/object_cursor.h"
#include "wasm/exceptions.h"
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

void relocate_symbols(std::vector<Relocation>& relocations, const std::vector<uint32_t>& symbols, bool& uses_table)
{
	for (Relocation& relocation : relocations)
		if (relocation.kind != RelocationKind::TypeIndex)
		{
			uses_table = uses_table || relocation.kind == RelocationKind::TableIndex ||
				relocation.kind == RelocationKind::TableIndexI32 ||
				relocation.kind == RelocationKind::TableIndexRelative || relocation.kind == RelocationKind::TableNumber;
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

// A constant's declaration can survive after all its uses folded. Wasm
// linking cannot encode an undefined local symbol; omit it when no typed
// reference remains, while retaining a referenced one for the diagnostic.
void discard_unused_local_declarations(lowir::Program& program)
{
	using namespace lowir;
	bool any = false;
	for (SymbolId id : program.order)
	{
		const Symbol& s = program.symbols[id];
		any = any ||
			(s.metadata.binding == ir_model::SBM_INTERNAL &&
				(s.kind == SymbolKind::FunctionDeclaration || s.kind == SymbolKind::GlobalDeclaration));
	}
	if (!any)
		return;
	std::vector<uint8_t> used(program.symbols.size(), 0);
	auto mark = [&](Value value) {
		if (value.kind == ValueKind::Symbol)
			used[value.index] = 1;
	};
	for (SymbolId id : program.order)
	{
		const Symbol& s = program.symbols[id];
		if (s.kind == SymbolKind::FunctionDefinition)
		{
			const FunctionBody& body = *program.functions[s.detail].body;
			for (BlockId block : body.layout)
				for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
				{
					const lowir::Instruction& inst = body.instructions[i];
					for (Value value : inst.ops)
						mark(value);
					for (uint32_t j = 0; j < inst.extra_count; ++j)
						mark(body.operands[inst.extra_first + j]);
				}
		}
		if (s.kind == SymbolKind::GlobalDefinition)
		{
			const Global& global = program.globals[s.detail];
			if (global.init == GlobalInit::Address)
				used[global.address] = 1;
			for (uint32_t j = 0; global.init == GlobalInit::Data && j < global.item_count; ++j)
			{
				const DataItem& item = program.data_items[global.first_item + j];
				if (item.kind == DataItemKind::Address)
					used[item.address] = 1;
			}
		}
	}
	for (const ObjectAlias& alias : program.aliases)
		used[alias.target] = 1;
	for (const FunctionBinding& binding : program.function_bindings)
		used[binding.symbol] = 1;
	size_t count = 0;
	for (SymbolId id : program.order)
	{
		const Symbol& s = program.symbols[id];
		if (used[id] || s.metadata.binding != ir_model::SBM_INTERNAL ||
			(s.kind != SymbolKind::FunctionDeclaration && s.kind != SymbolKind::GlobalDeclaration))
			program.order[count++] = id;
	}
	program.order.resize(count);
}

} // namespace

void make_object(lowir::Program& program, const backend::Options& options, Object& object, std::string& dump)
{
	discard_unused_local_declarations(program);
	bool uses_table = false;
	Module module;
	module.relocatable = true;
	prepare_exception_runtime(program, module);
	prepare_module(program, module);
	if (!options.machine_ir.empty())
		dump_module(program, module, dump);
	object.table_count = 1;
	object.memory_count = 1;
	object.types = module.signatures;
	object.functions.resize(module.definition_types.size());
	std::vector<uint32_t> function_symbols(object.functions.size(), lowir::kNoId);
	std::vector<uint32_t> symbols(program.symbols.size() + (module.tag_types.empty() ? 3 : 4), lowir::kNoId);
	std::vector<uint32_t> groups(program.symbols.size(), lowir::kNoId);
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
			if (function_symbols[symbol.index] != lowir::kNoId)
			{
				symbols[id] = function_symbols[symbol.index];
				ObjectSymbol& previous = object.symbols[symbols[id]];
				if (!symbol.weak())
					previous.flags &= ~1u;
				previous.flags |= symbol.flags & 0x80;
				continue;
			}
			function_symbols[symbol.index] = static_cast<uint32_t>(object.symbols.size());
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
			groups[id] = static_cast<uint32_t>(object.groups.size());
			object.groups.push_back(std::move(group));
		}
		symbols[id] = static_cast<uint32_t>(object.symbols.size());
		object.symbols.push_back(std::move(symbol));
		if (source.metadata.role == ir_model::SR_INIT)
			object.initializers.push_back({ 65535, symbols[id] });
	}
	for (const lowir::FunctionBinding& binding : program.function_bindings)
	{
		ObjectSymbol& symbol = object.symbols[symbols[binding.symbol]];
		ObjectFunction& function = object.functions[symbol.index];
		if (binding.imported)
		{
			if ((symbol.flags & 0x40) != 0 &&
				(function.module != program.spelling(binding.module) ||
					function.name != program.spelling(binding.name)))
				throw WasmError("conflicting Wasm imports for one object function");
			function.module = program.spelling(binding.module);
			function.name = program.spelling(binding.name);
			symbol.flags |= 0x40;
		}
		else
		{
			if (symbol.local())
			{
				// Object-local definitions need a unique export-facing
				// linking identity; relocations still name the same index.
				symbol.flags = (symbol.flags & ~3u) | 4u;
				symbol.name = std::string("__cppgm_export.") + program.spelling(binding.name);
				function.name = symbol.name;
			}
			symbol.flags |= 0xa0;
			object.exports.push_back(ObjectExport{ program.spelling(binding.name), symbol.index, 0 });
		}
	}
	symbols[module.stack_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(2, 0, "__stack_pointer"));
	object.global_imports.push_back(ObjectGlobal{ "env", "__stack_pointer", ValueType::I32, true });
	symbols[module.stack_low_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(1, 0, "__stack_low"));
	symbols[module.table_symbol] = static_cast<uint32_t>(object.symbols.size());
	object.symbols.push_back(imported_symbol(5, 0, "__indirect_function_table"));
	if (!module.tag_types.empty())
	{
		symbols[module.exception_tag_symbol] = static_cast<uint32_t>(object.symbols.size());
		const bool cxx = module.exception_abi == lowir::ExceptionAbi::Cxx;
		ObjectTag tag;
		tag.type = module.tag_types[0];
		tag.module = "env";
		tag.name = cxx ? "__cpp_exception" : "__cppgm_opaque_exception";
		tag.imported = cxx;
		ObjectSymbol identity = imported_symbol(4, 0, tag.name.c_str());
		// Opaque payloads use one coalesced private tag across translation units.
		if (!cxx)
			identity.flags = 1;
		object.symbols.push_back(std::move(identity));
		object.tags.push_back(std::move(tag));
	}
	bool uses_simd = false;
	for (const Signature& type : module.signatures)
	{
		for (ValueType value : type.parameters)
			uses_simd = uses_simd || value == ValueType::V128;
		for (ValueType value : type.results)
			uses_simd = uses_simd || value == ValueType::V128;
	}
	for (lowir::SymbolId id : module.definitions)
	{
		ExceptionPlan exceptions;
		if (module.exception_regions)
			prepare_exception_plan(program, id, object, symbols, groups[id], exceptions);
		Function function;
		Selector(program, module, id, function, &exceptions).run();
		cleanup_locals(function, options.optimization_level != 0);
		for (ValueType local : function.locals)
			uses_simd = uses_simd || local == ValueType::V128;
		for (const auto& inst : function.instructions)
			uses_simd = uses_simd || (static_cast<unsigned>(inst.op) >> 8) == 0xfd;
		ObjectFunction& destination = object.functions[module.function_indices[id]];
		try
		{
			validate_function(function, object.types[destination.type], module);
		}
		catch (const WasmError& failure)
		{
			throw WasmError(std::string(program.spelling(program.symbols[id].name)) + ": " + failure.what());
		}
		if (!options.machine_ir.empty())
			dump_function(function, program, id, dump);
		Bytes encoded;
		encode_function(function, encoded, &destination.code.relocations);
		Cursor body(encoded);
		body.u32();
		const uint32_t prefix = body.position();
		destination.code.bytes = body.bytes();
		for (Relocation& relocation : destination.code.relocations)
			relocation.offset -= prefix;
		relocate_symbols(destination.code.relocations, symbols, uses_table);
	}
	for (ObjectData& data : object.data)
		relocate_symbols(data.contents.relocations, symbols, uses_table);
	if (uses_table)
		object.symbols[symbols[module.table_symbol]].flags |= 0x80;
	if (options.target == target_model::Target::Wasi)
		for (lowir::SymbolId id : program.order)
			if (program.symbols[id].metadata.role == ir_model::SR_ENTRY)
				adapt_wasi_entry(object, symbols[id]);
	object.features = { { '+', "mutable-globals" }, { '+', "sign-ext" }, { '+', "bulk-memory" },
		{ '+', "nontrapping-fptoint" }, { '+', "reference-types" } };
	if (uses_simd)
		object.features.push_back({ '+', "simd128" });
	if (!module.tag_types.empty())
	{
		object.features.push_back({ '+', "exception-handling" });
		object.features.push_back({ '+', "multivalue" });
	}
}

} // namespace wasm
