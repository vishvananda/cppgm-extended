#include "lower/program_lowering.h"

#include <cstring>

namespace lower {

void ProgramLowering::function_bindings(
	const sema::FunctionEntity* function, lowir::SymbolId symbol, const std::string& object)
{
	const sema::SemanticGraph::WasmBinding binding = graph_->wasm_binding(function);
	const auto name = [&](const char* text) { return program_.names.intern(text, std::strlen(text)); };
	if (!function->defined && (binding.module != nullptr || binding.import_name != nullptr))
		program_.function_bindings.push_back(
			lowir::FunctionBinding{ symbol, name(binding.module != nullptr ? binding.module : "env"),
				name(binding.import_name != nullptr ? binding.import_name : object.c_str()), true });
	if (binding.export_name != nullptr)
		program_.function_bindings.push_back(lowir::FunctionBinding{ symbol, 0, name(binding.export_name), false });
}

} // namespace lower
