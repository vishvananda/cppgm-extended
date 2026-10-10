// GNU asm statements (sema/asm_statements.h): the operands typed as their
// constraints bind them, and the template lifted to the operations the
// lowering emits.

#include "sema/analyzer.h"

#include <cstdlib>
#include <cstring>

#include "lex/identifier_table.h"
#include "sema/asm_statements.h"

namespace sema {

namespace {

// A string literal's characters (its code units before the null).
bool LiteralText(const TokenSpelling& spelling, std::string& out)
{
	StringLiteralFacts facts;
	std::vector<uint64_t> units;
	if (!StringLiteralUnits(spelling, facts, units) || units.empty())
		return false;
	out.clear();
	for (size_t i = 0; i + 1 < units.size(); ++i)
		out.push_back(static_cast<char>(units[i]));
	return true;
}

} // namespace

void Analyzer::asm_statement(const syntax::AsmStmt& stmt, Scope* scope)
{
	const uint32_t count = stmt.outputs.size + stmt.inputs.size;
	// A template's definition checks the operands' names; its instances,
	// and other code, type them.
	if (body_ == nullptr || body_->pattern || !semantic() || template_depth_ != 0)
	{
		for (const syntax::AsmOperand& operand : stmt.outputs)
			expression(operand.expression, scope);
		for (const syntax::AsmOperand& operand : stmt.inputs)
			expression(operand.expression, scope);
		return;
	}
	std::string text;
	if (!LiteralText(stmt.text, text))
		error(stmt.location, "an asm template that is no ordinary string literal");
	std::vector<AsmOperandInfo> operands(count);
	std::vector<std::string> constraints(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		const bool output = i < stmt.outputs.size;
		const syntax::AsmOperand& operand = output ? stmt.outputs[i] : stmt.inputs[i - stmt.outputs.size];
		std::string& constraint = constraints[i];
		if (!LiteralText(operand.constraint, constraint))
			error(operand.location, "an asm constraint that is no ordinary string literal");
		AsmOperandInfo& info = operands[i];
		info.name = operand.name != nullptr ? operand.name->spelling : nullptr;
		for (uint32_t j = 0; j < i && info.name != nullptr; ++j)
			if (operands[j].name != nullptr && std::strcmp(operands[j].name, info.name) == 0)
				error(operand.location, "a duplicate asm operand name");
		info.output = output;
		info.tied = 0xff;
		// An input's matching constraint (digits, or an output's [name]):
		// it starts in that output's register, which is then read too.
		uint32_t tie = count;
		if (!output && !constraint.empty() && constraint.find_first_not_of("0123456789") == std::string::npos)
			tie = static_cast<uint32_t>(std::strtoul(constraint.c_str(), nullptr, 10));
		else if (!output && constraint.size() > 2 && constraint[0] == '[' && constraint.back() == ']')
			for (uint32_t j = 0; j < stmt.outputs.size; ++j)
				if (operands[j].name != nullptr && constraint.compare(1, constraint.size() - 2, operands[j].name) == 0)
					tie = j;
		if (!output && !constraint.empty() && (constraint[0] == '[' || (constraint[0] >= '0' && constraint[0] <= '9')))
		{
			bool matched = false;
			for (uint32_t j = stmt.outputs.size; j < i; ++j)
				matched = matched || operands[j].tied == tie;
			if (tie >= stmt.outputs.size || operands[tie].binding != AsmOperandClass::Register || operands[tie].read ||
				matched)
				error(
					operand.location, "an asm matching constraint of no `=` register output (or of one matched twice)");
			info.tied = static_cast<uint8_t>(tie);
			operands[tie].read = true;
		}
		// Modifiers (= + &) and then the operand's classes: m (memory), r
		// and the register letters, i n (constants), g (any: a register
		// here), a digit (an output's place: a register).
		bool memory = false, immediate = false, any = false;
		for (char c : constraint)
		{
			if (c == '+')
				info.read = true;
			else if (c == 'm' || c == 'o' || c == 'V')
				memory = true;
			else if (c == 'i' || c == 'n')
				immediate = true;
			else if (c != '=' && c != '&' && c != '%')
				any = true;
		}
		info.binding = memory && !any && !immediate ? AsmOperandClass::Memory
			: immediate && !any && !memory          ? AsmOperandClass::Immediate
													: AsmOperandClass::Register;
		if (output && (constraint.empty() || (constraint[0] != '=' && constraint[0] != '+')))
			error(operand.location, "an asm output whose constraint is no `=` or `+` one");
		const ExprFacts* facts = typed(*operand.expression, scope);
		if (facts == nullptr)
			return;
		if (info.binding == AsmOperandClass::Immediate)
		{
			Constant value;
			if (output || !integral_constant(*operand.expression, scope, value))
				error(operand.location, "an asm immediate operand that is no integral constant");
			info.value = value.bits;
			continue;
		}
		if ((output || info.binding == AsmOperandClass::Memory) && facts->category != ValueCategory::LValue)
			error(operand.location, "an asm memory or output operand that is no lvalue");
		if (output && !modifiable_lvalue(facts))
			error(operand.location, "an asm output operand that is no modifiable lvalue");
		const Type* type = facts->type->unqualified;
		uint64_t size = 0, alignment = 0;
		if (!type_size(type, size, alignment, operand.location))
			error(operand.location, "an asm operand of incomplete type");
		if (info.binding == AsmOperandClass::Register && !output)
		{
			if (!IsScalar(type))
				error(operand.location, "an asm register operand of no scalar type");
			convert_operand(*operand.expression, type);
		}
		info.bytes = static_cast<uint32_t>(size);
		// A matching input fills its output's register: its size.
		if (info.tied != 0xff && info.bytes != operands[info.tied].bytes)
			error(operand.location, "an asm matching input of another size than its output");
	}
	std::vector<AsmInstruction> instructions;
	std::string message;
	AsmProgram* program = static_cast<AsmProgram*>(arena_.allocate(sizeof(AsmProgram), alignof(AsmProgram)));
	program->unlifted = nullptr;
	if (!LiftAsmTemplate(text, operands.data(), count, instructions, message))
	{
		// A template outside the lifted set is this compiler's limit, not
		// an error in the program. Only emitted code reaches an assembler,
		// so in a function emitted only where odr-used (an inline one, as
		// GCC's gnu_inline intrinsic wrappers are, or an internal
		// always_inline one: ProgramLowering::emitted_at_definition) the
		// lowering reports it if it emits the statement.
		const FunctionEntity* function = body_->function;
		if (function == nullptr || !(function->is_inline || (function->internal_linkage && function->always_inline)))
			error(stmt.location, copy_label(message.data(), message.size()));
		program->unlifted = copy_label(message.data(), message.size());
		instructions.clear();
	}
	program->count = static_cast<uint32_t>(instructions.size());
	program->instructions = instructions.empty() ? nullptr
												 : arena_.copy_array(instructions.data(), instructions.size());
	program->registers = 0;
	uint8_t* tied = nullptr;
	for (uint32_t i = 0; i < count; ++i)
	{
		program->registers |= operands[i].binding == AsmOperandClass::Register ? 1u << i : 0u;
		if (operands[i].tied == 0xff)
			continue;
		if (tied == nullptr)
		{
			tied = static_cast<uint8_t*>(arena_.allocate(stmt.outputs.size, 1));
			std::memset(tied, 0xff, stmt.outputs.size);
		}
		tied[operands[i].tied] = static_cast<uint8_t>(i);
	}
	program->tied = tied;
	// A "memory" clobber makes the statement a compiler barrier.
	program->barrier = false;
	for (const TokenSpelling& clobber : stmt.clobbers)
	{
		std::string name;
		program->barrier = program->barrier || (LiteralText(clobber, name) && name == "memory");
	}
	stmt.program = program;
}

} // namespace sema
