#include "wasm/abi.h"

#include <cstring>
#include <map>

#include "lowir/builder.h"

namespace wasm {

namespace {

using namespace lowir;

struct Helper
{
	const char* name;
	Type argument;
	unsigned count;
};

Helper helper_for(const lowir::Instruction& inst)
{
	const Type wide = Type::scalar(TypeKind::I128);
	if (inst.op == Opcode::Binary && inst.type == wide)
	{
		const char* name = nullptr;
		switch (static_cast<BinaryOp>(inst.sub))
		{
		case BinaryOp::Mul:
			name = "__multi3";
			break;
		case BinaryOp::Div:
			name = "__divti3";
			break;
		case BinaryOp::UDiv:
			name = "__udivti3";
			break;
		case BinaryOp::Mod:
			name = "__modti3";
			break;
		case BinaryOp::UMod:
			name = "__umodti3";
			break;
		default:
			break;
		}
		return Helper{ name, wide, 2 };
	}
	if (inst.op != Opcode::Convert)
		return Helper{ nullptr, wide, 0 };
	const Type from = convert_source_type(inst);
	const auto conversion = static_cast<ConvertOp>(inst.sub);
	if (from == wide && (inst.type.kind() == TypeKind::F32 || inst.type.kind() == TypeKind::F64))
	{
		const bool single = inst.type.kind() == TypeKind::F32;
		const char* name = conversion == ConvertOp::UIToFP ? (single ? "__floatuntisf" : "__floatuntidf")
														   : (single ? "__floattisf" : "__floattidf");
		return Helper{ name, wide, 1 };
	}
	if (inst.type == wide && (from.kind() == TypeKind::F32 || from.kind() == TypeKind::F64))
	{
		const bool single = from.kind() == TypeKind::F32;
		const char* name = conversion == ConvertOp::FPToUI ? (single ? "__fixunssfti" : "__fixunsdfti")
														   : (single ? "__fixsfti" : "__fixdfti");
		return Helper{ name, from, 1 };
	}
	return Helper{ nullptr, wide, 0 };
}

SymbolId declare_helper(Program& program, Helper helper, Type result)
{
	const std::string internal = std::string("__cppgm_wasm_runtime_") + helper.name;
	const NameId name = program.names.intern(internal.data(), static_cast<uint32_t>(internal.size()));
	const SymbolId symbol = program.symbol_for(name);
	if (program.symbols[symbol].kind != SymbolKind::Unresolved)
		throw WasmError("conflicting Wasm arithmetic helper identity");
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program.parameters.size());
	signature.parameter_count = helper.count;
	signature.result = result;
	signature.boundary.unwind = ir_model::CUM_NO;
	for (unsigned i = 0; i < helper.count; ++i)
	{
		Parameter parameter = Parameter();
		parameter.type = helper.argument;
		program.parameters.push_back(parameter);
	}
	program.declare_function(symbol, SymbolKind::FunctionDeclaration, program.add_signature(signature));
	auto& metadata = program.symbols[symbol].metadata;
	metadata.linkage = ir_model::LLM_C;
	metadata.binding = ir_model::SBM_STRONG;
	metadata.object = program.names.intern(helper.name, static_cast<uint32_t>(std::strlen(helper.name)));
	return symbol;
}

} // namespace

void legalize_wide_helpers(lowir::Program& program)
{
	using namespace lowir;
	std::map<std::string, SymbolId> helpers;
	const size_t functions = program.functions.size();
	for (size_t f = 0; f < functions; ++f)
	{
		FunctionBody* body = program.functions[f].body.get();
		if (body == nullptr)
			continue;
		for (BlockId block : body->layout)
			for (InstId id = body->blocks[block].first; id != kNoId; id = body->instructions[id].next)
			{
				auto& inst = body->instructions[id];
				const Helper helper = helper_for(inst);
				if (helper.name == nullptr)
					continue;
				auto found = helpers.find(helper.name);
				const SymbolId symbol = found == helpers.end()
					? helpers.emplace(helper.name, declare_helper(program, helper, inst.type)).first->second
					: found->second;
				const auto first = static_cast<uint32_t>(body->operands.size());
				for (unsigned i = 0; i < helper.count; ++i)
					body->operands.push_back(inst.ops[i]);
				lowir::Instruction call = make_instruction(Opcode::Call, inst.type);
				call.ops[0] = Value::symbol(symbol);
				call.extra_first = first;
				call.extra_count = helper.count;
				call.result = inst.result;
				call.next = inst.next;
				call.location = inst.location;
				inst = call;
			}
	}
}

} // namespace wasm
