#include "wasm/exceptions.h"

#include <cstring>

namespace wasm {

namespace {

using namespace lowir;

bool needs_regions(const Program& program)
{
	for (SymbolId id : program.order)
		if (program.symbols[id].kind == SymbolKind::FunctionDefinition)
			for (BlockId block : program.functions[program.symbols[id].detail].body->layout)
			{
				const auto& body = *program.functions[program.symbols[id].detail].body;
				for (InstId inst = body.blocks[block].first; inst != kNoId; inst = body.instructions[inst].next)
					if ((body.instructions[inst].op == Opcode::EhTry ||
							body.instructions[inst].op == Opcode::EhCleanup) &&
						body.instructions[inst].ops[0].is(ValueKind::Block))
						return true;
			}
	return false;
}

SymbolId runtime_symbol(Program& program, const char* name, const char* object)
{
	const SymbolId id = program.symbol_for(program.names.intern(name, static_cast<uint32_t>(std::strlen(name))));
	if (program.symbols[id].kind != SymbolKind::Unresolved)
		throw WasmError("conflicting Wasm exception-runtime identity");
	program.symbols[id].metadata.object = program.names.intern(object, static_cast<uint32_t>(std::strlen(object)));
	program.symbols[id].metadata.linkage = ir_model::LLM_C;
	return id;
}

} // namespace

void prepare_exception_runtime(lowir::Program& program, Module& module)
{
	using namespace lowir;
	if (!needs_regions(program))
		return;
	lowir::Signature type{};
	type.result = Type::scalar(TypeKind::I32);
	type.parameter_count = 1;
	type.first_parameter = static_cast<uint32_t>(program.parameters.size());
	type.boundary.unwind = ir_model::CUM_NO;
	Parameter argument{};
	argument.type = program.pointer_type();
	program.parameters.push_back(argument);
	module.personality_symbol = runtime_symbol(program, "__cppgm_wasm_personality", "_Unwind_CallPersonality");
	program.declare_function(module.personality_symbol, SymbolKind::FunctionDeclaration, program.add_signature(type));
	module.exception_context = runtime_symbol(program, "__cppgm_wasm_exception_context", "__wasm_lpad_context");
	Global context{};
	context.typed = true;
	context.type = Type::object(12, 2);
	program.declare_global(module.exception_context, SymbolKind::GlobalDeclaration, context);
}

} // namespace wasm
