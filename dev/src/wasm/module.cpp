#include "wasm/module.h"

#include <map>

namespace wasm {

namespace {

void function_types(const lowir::Program& program, Module& module)
{
	std::map<std::vector<uint8_t>, uint32_t> known;
	for (const lowir::Signature& source : program.signatures)
	{
		Signature signature;
		Boundary boundary;
		std::vector<uint8_t> key;
		boundary.result = classify_value(source.result, source.boundary.result_shape, program.pointer_type());
		if (boundary.result.passing == Passing::Expand)
			boundary.result.passing = Passing::Indirect;
		if (boundary.result.passing == Passing::Indirect)
			signature.parameters.push_back(ValueType::I32);
		const lowir::Parameter* parameters = program.parameters_of(source);
		for (uint32_t i = 0; i < source.parameter_count; ++i)
		{
			const AbiValue argument = classify_value(parameters[i].type, parameters[i].shape, program.pointer_type());
			boundary.parameters.push_back(argument);
			if (argument.passing != Passing::Ignore)
				signature.parameters.push_back(value_type(argument.scalar));
			if (argument.passing == Passing::Expand)
				signature.parameters.push_back(value_type(argument.scalar));
		}
		if (source.boundary.arity == ir_model::CAM_VARIADIC)
			signature.parameters.push_back(ValueType::I32);
		const ValueType result = boundary.result.passing == Passing::Indirect ||
				boundary.result.passing == Passing::Ignore
			? ValueType::Void
			: value_type(boundary.result.scalar);
		if (source.boundary.this_return)
			signature.results.push_back(ValueType::I32);
		else if (result != ValueType::Void)
			signature.results.push_back(result);
		for (ValueType parameter : signature.parameters)
			key.push_back(static_cast<uint8_t>(parameter));
		key.push_back(0);
		for (ValueType returned : signature.results)
			key.push_back(static_cast<uint8_t>(returned));
		auto inserted = known.emplace(key, static_cast<uint32_t>(module.signatures.size()));
		if (inserted.second)
			module.signatures.push_back(std::move(signature));
		module.signature_types.push_back(inserted.first->second);
		module.boundaries.push_back(std::move(boundary));
	}
}

} // namespace

void prepare_module(const lowir::Program& program, Module& module)
{
	module.function_indices.assign(program.symbols.size(), lowir::kNoId);
	module.addresses.assign(program.symbols.size(), lowir::kNoId);
	function_types(program, module);
	module.stack_symbol = static_cast<uint32_t>(program.symbols.size());
	module.stack_low_symbol = module.stack_symbol + 1;
	module.table_symbol = module.stack_symbol + 2;
	if (module.exception_regions)
	{
		module.exception_tag_symbol = module.stack_symbol + 3;
		Signature tag;
		tag.parameters = module.exception_abi == lowir::ExceptionAbi::Cxx
			? std::vector<ValueType>{ ValueType::I32 }
			: std::vector<ValueType>{ ValueType::I64, ValueType::I64 };
		module.tag_types.push_back(static_cast<uint32_t>(module.signatures.size()));
		module.signatures.push_back(std::move(tag));
		Signature pair;
		pair.results = module.signatures.back().parameters;
		pair.results.push_back(ValueType::ExnRef);
		module.exception_pair_type = static_cast<uint32_t>(module.signatures.size());
		module.signatures.push_back(std::move(pair));
	}
	for (lowir::SymbolId id : program.order)
	{
		const auto& symbol = program.symbols[id];
		if (symbol.kind != lowir::SymbolKind::FunctionDeclaration)
			continue;
		module.function_indices[id] = static_cast<uint32_t>(module.definition_types.size());
		module.definition_types.push_back(module.signature_types[program.functions[symbol.detail].signature]);
	}
	for (lowir::SymbolId id : program.order)
	{
		if (program.symbols[id].kind == lowir::SymbolKind::FunctionDefinition)
		{
			const auto& symbol = program.symbols[id];
			if (symbol.metadata.role == ir_model::SR_INIT)
			{
				const auto& signature = program.signatures[program.functions[symbol.detail].signature];
				if (module.initializer != lowir::kNoId || signature.parameter_count != 0 || !signature.result.is_void())
					throw WasmError("invalid or duplicate Wasm initializer");
				module.initializer = static_cast<uint32_t>(module.definition_types.size());
			}
			module.function_indices[id] = static_cast<uint32_t>(module.definition_types.size());
			module.definitions.push_back(id);
			module.definition_types.push_back(
				module.signature_types[program.functions[program.symbols[id].detail].signature]);
		}
	}
}

} // namespace wasm
