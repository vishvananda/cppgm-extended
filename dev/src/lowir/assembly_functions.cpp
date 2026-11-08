#include "lowir/assembly.h"

namespace lowir {

const AssemblyFunction* find_assembly_function(const Program& program, SymbolId symbol)
{
	if (program.assembly)
		for (const AssemblyFunction& function : program.assembly->functions)
			if (function.symbol == symbol)
				return &function;
	return nullptr;
}

void retain_assembly_function(Program& program, SymbolId symbol)
{
	if (!program.assembly)
		program.assembly.reset(new AssemblyModule());
	Symbol& declaration = program.symbols[symbol];
	AssemblyFunction function;
	function.symbol = symbol;
	function.body = std::move(program.functions[declaration.detail].body);
	declaration.kind = SymbolKind::FunctionDeclaration;
	program.assembly->functions.push_back(std::move(function));
}

bool validate_assembly_functions(const Program& program, const char*& error)
{
	if (!program.assembly || program.assembly->functions.empty())
		return true;
	const auto fail = [&](const char* message) {
		error = message;
		return false;
	};
	std::vector<bool> seen(program.symbols.size(), false);
	for (const AssemblyFunction& function : program.assembly->functions)
	{
		if (function.symbol >= program.symbols.size() || seen[function.symbol])
			return fail("invalid or duplicate assembly function symbol");
		seen[function.symbol] = true;
		const Symbol& symbol = program.symbols[function.symbol];
		if (symbol.kind != SymbolKind::FunctionDeclaration || symbol.detail >= program.functions.size() ||
			program.functions[symbol.detail].body || !function.body)
			return fail("assembly function requires an opaque function declaration");
		const Function& declaration = program.functions[symbol.detail];
		if (declaration.signature >= program.signatures.size())
			return fail("invalid assembly function signature");
		const Signature& signature = program.signatures[declaration.signature];
		const FunctionBody& body = *function.body;
		if (body.parameter_count != signature.parameter_count ||
			uint64_t(signature.first_parameter) + signature.parameter_count > program.parameters.size())
			return fail("assembly function parameters must retain their declared ABI");
		if (!body.slots.empty() || body.temps.size() != body.parameter_count || body.blocks.size() != 1 ||
			body.layout.size() != 1 || body.layout[0] != 0 || body.instructions.size() < 2 ||
			body.blocks[0].first != 0 || body.blocks[0].last != body.instructions.size() - 1)
			return fail("assembly function requires one raw assembly block");
		for (InstId id = 0; id < body.instructions.size(); ++id)
		{
			const Instruction& inst = body.instructions[id];
			if (inst.location > body.locations.size() || inst.ops[0].kind != ValueKind::None ||
				inst.ops[1].kind != ValueKind::None || inst.ops[2].kind != ValueKind::None)
				return fail("invalid assembly function instruction operands or location");
			const bool last = id + 1 == body.instructions.size();
			if (inst.op != (last ? Opcode::Unreachable : Opcode::Asm) || inst.result != kNoId ||
				inst.next != (last ? kNoId : id + 1) || !inst.type.is_void() ||
				uint64_t(inst.extra_first) + inst.extra_count > body.operands.size())
				return fail("assembly function permits only asm followed by unreachable");
			if (last)
				continue;
			if (inst.immediate < 0 || uint64_t(inst.immediate) >= program.assembly->templates.size())
				return fail("invalid assembly function template");
			const AssemblyTemplate& definition = program.assembly->templates[inst.immediate];
			for (uint32_t i = 0; i < definition.operand_count; ++i)
			{
				const AssemblyOperand& operand = program.assembly->operands[definition.first_operand + i];
				if (operand.flags != asm_model::AO_READ || operand.input_storage || operand.output != kNoId)
					return fail("assembly function operands must be constant or symbol inputs");
				for (uint32_t j = 0; j < operand.alternative_count; ++j)
					if ((program.assembly->alternatives[operand.first_alternative + j].locations &
							~(asm_model::AL_IMMEDIATE | asm_model::AL_SYMBOL)) != 0)
						return fail("assembly function cannot require compiler register setup");
			}
			for (uint32_t i = 0; i < inst.extra_count; ++i)
			{
				const Value value = body.operands[inst.extra_first + i];
				if ((!value.is_literal() && value.kind != ValueKind::Symbol) ||
					(value.is_literal() && value.index >= body.literals.size()) ||
					(value.kind == ValueKind::Symbol && value.index >= program.symbols.size()))
					return fail("assembly function argument must be a constant or symbol");
			}
		}
	}
	return true;
}

} // namespace lowir
