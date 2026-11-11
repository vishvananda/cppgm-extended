#include "wasm/module.h"

#include <algorithm>
#include <map>

#include "machine/global_data.h"
#include "wasm/selector.h"

namespace wasm {

namespace {

uint32_t aligned(uint64_t bytes, uint32_t alignment)
{
	const uint64_t result = (bytes + alignment - 1) & ~uint64_t(alignment - 1);
	if (result > 0x7ff00000u)
		throw WasmError("Wasm data exceeds the supported linear-memory range");
	return static_cast<uint32_t>(result);
}

void function_types(const lowir::Program& program, Module& module)
{
	std::map<std::vector<uint8_t>, uint32_t> known;
	for (const lowir::Signature& source : program.signatures)
	{
		Signature signature;
		std::vector<uint8_t> key;
		const lowir::Parameter* parameters = program.parameters_of(source);
		for (uint32_t i = 0; i < source.parameter_count; ++i)
		{
			const ValueType type = value_type(parameters[i].type);
			signature.parameters.push_back(type);
			key.push_back(static_cast<uint8_t>(type));
		}
		signature.result = value_type(source.result);
		key.push_back(static_cast<uint8_t>(signature.result));
		auto inserted = known.emplace(key, static_cast<uint32_t>(module.signatures.size()));
		if (inserted.second)
			module.signatures.push_back(std::move(signature));
		module.signature_types.push_back(inserted.first->second);
	}
}

void exports(const lowir::Program& program, const Module& module, const backend::Options& options, Bytes& output)
{
	Bytes entries;
	uint32_t count = 1;
	entries.string("memory");
	entries.byte(2);
	entries.unsigned_leb(0);
	for (lowir::SymbolId id : module.definitions)
	{
		const auto& symbol = program.symbols[id];
		const char* name = program.spelling(symbol.metadata.object != 0 ? symbol.metadata.object : symbol.name);
		const bool requested = std::find(options.exports.begin(), options.exports.end(), name) != options.exports.end();
		if (!requested && symbol.metadata.role != ir_model::SR_ENTRY)
			continue;
		entries.string(name);
		entries.byte(0);
		entries.unsigned_leb(module.function_indices[id]);
		++count;
	}
	for (const std::string& name : options.exports)
	{
		bool found = false;
		for (lowir::SymbolId id : module.definitions)
		{
			const auto& symbol = program.symbols[id];
			found = found ||
				name == program.spelling(symbol.metadata.object != 0 ? symbol.metadata.object : symbol.name);
		}
		if (!found)
			throw WasmError("undefined Wasm export: " + name);
	}
	Bytes section;
	section.unsigned_leb(count);
	section.append(entries);
	output.section(7, section);
}

} // namespace

void prepare_module(const lowir::Program& program, Module& module)
{
	module.function_indices.assign(program.symbols.size(), lowir::kNoId);
	module.addresses.assign(program.symbols.size(), lowir::kNoId);
	function_types(program, module);
	for (lowir::SymbolId id : program.order)
	{
		if (program.symbols[id].kind == lowir::SymbolKind::FunctionDefinition)
		{
			module.function_indices[id] = static_cast<uint32_t>(module.definitions.size());
			module.definitions.push_back(id);
		}
	}
	uint32_t end = 1024;
	for (lowir::SymbolId id : program.order)
	{
		const auto& symbol = program.symbols[id];
		if (symbol.kind != lowir::SymbolKind::GlobalDefinition)
			continue;
		if (symbol.metadata.storage == ir_model::GSM_THREAD_LOCAL)
			throw WasmError("thread-local storage is not supported by this Wasm profile");
		uint32_t alignment = 1;
		const uint64_t bytes = machine::global_size(program, program.globals[symbol.detail], alignment);
		const uint32_t base = aligned(end, alignment);
		module.addresses[id] = base;
		end = aligned(uint64_t(base) + bytes, 1);
	}
	module.data.data.resize(end, 0);
	std::vector<machine::GlobalAddress> fixups;
	for (lowir::SymbolId id : program.order)
	{
		const auto& symbol = program.symbols[id];
		if (symbol.kind != lowir::SymbolKind::GlobalDefinition)
			continue;
		fixups.clear();
		machine::write_global(program, program.globals[symbol.detail], module.data.data, module.addresses[id], fixups);
		for (const auto& fixup : fixups)
		{
			uint32_t address = module.addresses[fixup.target];
			if (lowir::is_function_kind(program.symbols[fixup.target].kind))
				address = module.function_indices[fixup.target] == lowir::kNoId
					? lowir::kNoId
					: module.function_indices[fixup.target] + 1;
			if (address == lowir::kNoId)
				throw WasmError("undefined Wasm data relocation");
			address += fixup.addend;
			for (unsigned i = 0; i < 4; ++i)
				module.data.data[module.addresses[id] + fixup.offset + i] = static_cast<uint8_t>(address >> (i * 8));
		}
	}
	module.stack_floor = aligned(end, 16);
	module.stack_top = aligned(uint64_t(end) + 1024 * 1024, 16);
	while (!module.data.data.empty() && module.data.data.back() == 0)
		module.data.data.pop_back();
}

void encode_module(lowir::Program& program, const backend::Options& options, Bytes& output, std::string& dump)
{
	Module module;
	prepare_module(program, module);
	output.fixed(0x6d736100, 4);
	output.fixed(1, 4);
	Bytes types;
	types.unsigned_leb(module.signatures.size());
	for (const Signature& signature : module.signatures)
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
	Bytes functions;
	functions.unsigned_leb(module.definitions.size());
	for (auto id : module.definitions)
		functions.unsigned_leb(module.signature_types[program.functions[program.symbols[id].detail].signature]);
	output.section(3, functions);
	Bytes table;
	table.unsigned_leb(1);
	table.byte(0x70);
	table.byte(0);
	table.unsigned_leb(module.definitions.size() + 1);
	output.section(4, table);
	Bytes memory;
	memory.unsigned_leb(1);
	memory.byte(0);
	memory.unsigned_leb((module.stack_top + 65535) / 65536);
	output.section(5, memory);
	Bytes globals;
	globals.unsigned_leb(1);
	globals.byte(static_cast<uint8_t>(ValueType::I32));
	globals.byte(1);
	globals.byte(static_cast<uint8_t>(Op::I32Const));
	globals.signed_leb(module.stack_top);
	globals.byte(static_cast<uint8_t>(Op::End));
	output.section(6, globals);
	exports(program, module, options, output);
	Bytes elements;
	elements.unsigned_leb(1);
	elements.byte(0);
	elements.byte(static_cast<uint8_t>(Op::I32Const));
	elements.signed_leb(1);
	elements.byte(static_cast<uint8_t>(Op::End));
	elements.unsigned_leb(module.definitions.size());
	for (uint32_t i = 0; i < module.definitions.size(); ++i)
		elements.unsigned_leb(i);
	output.section(9, elements);
	Bytes code;
	code.unsigned_leb(module.definitions.size());
	for (auto id : module.definitions)
	{
		Function function;
		Selector(program, module, id, function).run();
		if (!options.machine_ir.empty())
			dump_function(function, program.spelling(program.symbols[id].name), dump);
		encode_function(function, code);
	}
	output.section(10, code);
	if (!module.data.data.empty())
	{
		Bytes data;
		data.unsigned_leb(1);
		data.byte(0);
		data.byte(static_cast<uint8_t>(Op::I32Const));
		data.signed_leb(0);
		data.byte(static_cast<uint8_t>(Op::End));
		data.unsigned_leb(module.data.data.size());
		data.append(module.data);
		output.section(11, data);
	}
}

} // namespace wasm
