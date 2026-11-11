#include "wasm/object.h"

#include <cstring>
#include "lowir/builder.h"

namespace wasm {

namespace {

using namespace lowir;

void require_void_entry(const Program& program, SymbolId id)
{
	const auto& type = program.signatures[program.functions[program.symbols[id].detail].signature];
	if (!type.result.is_void() || type.parameter_count != 0 || type.boundary.arity != ir_model::CAM_FIXED)
		throw WasmError("Wasm lifetime entry must have type void()");
}

SymbolId registrar(Program& program)
{
	for (SymbolId id : program.order)
	{
		const auto& symbol = program.symbols[id];
		if (!is_function_kind(symbol.kind) ||
			std::strcmp(
				program.spelling(symbol.metadata.object != 0 ? symbol.metadata.object : symbol.name), "atexit") != 0)
			continue;
		const auto& type = program.signatures[program.functions[symbol.detail].signature];
		if (type.result.kind() != TypeKind::I32 || type.parameter_count != 1 ||
			type.boundary.arity != ir_model::CAM_FIXED || program.parameters_of(type)[0].type != program.pointer_type())
			throw WasmError("incompatible Wasm finalizer registration signature");
		return id;
	}
	lowir::Signature type = lowir::Signature();
	type.first_parameter = static_cast<uint32_t>(program.parameters.size());
	type.parameter_count = 1;
	type.result = Type::scalar(TypeKind::I32);
	type.boundary.unwind = ir_model::CUM_NO;
	Parameter argument = Parameter();
	argument.type = program.pointer_type();
	program.parameters.push_back(argument);
	const SymbolId id = program.symbol_for(program.names.intern("__cppgm_wasm_atexit", 19));
	if (program.symbols[id].kind != SymbolKind::Unresolved)
		throw WasmError("conflicting Wasm finalizer registration identity");
	program.declare_function(id, SymbolKind::FunctionDeclaration, program.add_signature(type));
	program.symbols[id].metadata.object = program.names.intern("atexit", 6);
	program.symbols[id].metadata.linkage = ir_model::LLM_C;
	return id;
}

SymbolId initializer(Program& program)
{
	lowir::Signature type = lowir::Signature();
	type.result = Type::scalar(TypeKind::Void);
	const char* name = "__cppgm_wasm_register_finalizers";
	const SymbolId id = program.symbol_for(program.names.intern(name, static_cast<uint32_t>(std::strlen(name))));
	if (program.symbols[id].kind != SymbolKind::Unresolved)
		throw WasmError("conflicting Wasm finalizer initializer identity");
	program.declare_function(id, SymbolKind::FunctionDefinition, program.add_signature(type));
	program.symbols[id].metadata.role = ir_model::SR_INIT;
	program.symbols[id].metadata.binding = ir_model::SBM_INTERNAL;
	FunctionBuilder builder(program, id);
	const BlockId entry = builder.add_block(0);
	builder.place_block(entry);
	builder.set_insertion_block(entry);
	builder.ret(type.result, Value::none());
	return id;
}

} // namespace

void prepare_finalizers(lowir::Program& program)
{
	using namespace lowir;
	SymbolId fini = kNoId, init = kNoId;
	for (SymbolId id : program.order)
		if (program.symbols[id].kind == SymbolKind::FunctionDefinition)
		{
			if (program.symbols[id].metadata.role == ir_model::SR_FINI)
				fini = id;
			if (program.symbols[id].metadata.role == ir_model::SR_INIT)
				init = id;
		}
	if (fini == kNoId)
		return;
	require_void_entry(program, fini);
	if (init == kNoId)
		init = initializer(program);
	require_void_entry(program, init);
	const SymbolId registration = registrar(program);
	FunctionBuilder builder(program, init);
	auto& body = builder.body();
	for (BlockId block : body.layout)
	{
		InstId previous = kNoId;
		for (InstId id = body.blocks[block].first; id != kNoId; id = body.instructions[id].next)
		{
			if (body.instructions[id].op == Opcode::Return)
			{
				lowir::Instruction call = make_instruction(Opcode::Call, Type::scalar(TypeKind::I32));
				call.ops[0] = Value::symbol(registration);
				const Value callback = Value::symbol(fini);
				call.extra_first = builder.add_operands(&callback, 1);
				call.extra_count = 1;
				call.location = body.instructions[id].location;
				call.result = builder.add_temp(0);
				const InstId inserted = builder.insert_after(block, previous, call);
				builder.bind(call.result, call.type, inserted);
			}
			previous = id;
		}
	}
	builder.finish();
}

} // namespace wasm
