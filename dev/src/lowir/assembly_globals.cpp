#include "lowir/assembly.h"

namespace lowir {

bool validate_assembly_globals(const Program& program, const char*& error)
{
	if (!program.assembly)
		return true;
	const auto fail = [&](const char* message) {
		error = message;
		return false;
	};
	const AssemblyModule& module = *program.assembly;
	for (const AssemblyGlobal& item : module.globals)
	{
		if (item.definition >= module.templates.size())
			return fail("invalid module assembly template");
		const AssemblyTemplate& definition = module.templates[item.definition];
		if (item.arguments.size() != definition.argument_count)
			return fail("invalid module assembly argument count");
		for (uint32_t i = 0; i < definition.operand_count; ++i)
		{
			const AssemblyOperand& operand = module.operands[definition.first_operand + i];
			if (operand.flags != asm_model::AO_READ || operand.input_storage || operand.output != kNoId)
				return fail("module assembly requires constant or symbol inputs");
			for (uint32_t j = 0; j < operand.alternative_count; ++j)
				if ((module.alternatives[operand.first_alternative + j].locations &
						~(asm_model::AL_IMMEDIATE | asm_model::AL_SYMBOL)) != 0)
					return fail("module assembly cannot require runtime register setup");
			const Value value = item.arguments[operand.input];
			if (value.kind == ValueKind::Symbol)
			{
				if (value.index >= program.symbols.size() || !operand.type.is_pointer())
					return fail("invalid module assembly symbol argument");
			}
			else if (!value.is_literal() || value.index >= item.literals.size() ||
				(value.kind == ValueKind::Integer && !operand.type.is_integer()) ||
				(value.kind == ValueKind::Float && value.float_kind != operand.type.kind()))
				return fail("invalid module assembly literal argument");
		}
	}
	return true;
}

} // namespace lowir
