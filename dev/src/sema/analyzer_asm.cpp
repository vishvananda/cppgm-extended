// GNU asm statements (sema/asm_statements.h): the operands typed as their
// constraints bind them, and the template lifted to the operations the
// lowering emits.

#include "sema/analyzer.h"
#include "asm/packet.h"
#include "asm/parser.h"

#include <cstdlib>
#include <cstring>

#include "lex/identifier_table.h"
#include "sema/asm_statements.h"
#include "sema/constant_interpreter.h"

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

template<typename T>
assembly::View<T> preserve(Arena& arena, const std::vector<T>& values)
{
	assembly::View<T> view;
	view.size = static_cast<uint32_t>(values.size());
	view.data = values.empty() ? nullptr : arena.copy_array(values.data(), values.size());
	return view;
}

const char* prepare_operands(Analyzer& analyzer, const syntax::AsmStmt& stmt, Scope* scope,
	assembly::PacketBuilder& packet, std::vector<ConstValue>& addresses)
{
	using namespace asm_model;
	packet.constants.resize(packet.operands.size());
	addresses.resize(packet.operands.size());
	bool captured_outputs = false;
	for (const syntax::AsmOperand& output : stmt.outputs)
		captured_outputs = captured_outputs || (output.expression->facts->flags & kBitField) != 0;
	for (uint32_t i = 0; i < stmt.outputs.size + stmt.inputs.size; ++i)
	{
		asm_model::Operand& operand = packet.operands[i];
		const syntax::Expr& expr = *(
			i < stmt.outputs.size ? stmt.outputs[i].expression : stmt.inputs[i - stmt.outputs.size].expression);
		uint8_t locations = 0;
		for (uint32_t j = 0; j < operand.alternative_count; ++j)
		{
			const Alternative& alternative = packet.alternatives[operand.first_alternative + j];
			locations |= alternative.locations;
			if (alternative.matching != asm_model::kNoId)
				locations |= AL_REGISTER;
		}
		const bool output = (operand.flags & AO_WRITE) != 0;
		if (output && ((expr.facts->flags & kBitField) != 0 || (captured_outputs && (locations & AL_REGISTER) != 0)))
			operand.flags |= AO_NO_MEMORY;
		assembly::InputConstant& constant = packet.constants[i];
		if (!output && (locations & AL_IMMEDIATE) != 0)
		{
			Constant value;
			constant.present = analyzer.integral_constant(expr, scope, value, constant.bits.high);
			if (constant.present)
			{
				constant.bits.low = value.bits;
				IntegralKind mode;
				const unsigned bits = IntegralKindOf(value.type, mode) ? mode.bits : 0;
				if (bits != 0 && bits <= 64)
				{
					// GNU integer constraints see the operand's machine-mode
					// constant: sign-extend its bits, including unsigned types.
					if (bits < 64)
					{
						const uint64_t sign = uint64_t(1) << (bits - 1);
						constant.bits.low = ((constant.bits.low & ((sign << 1) - 1)) ^ sign) - sign;
					}
					constant.bits.high = static_cast<int64_t>(constant.bits.low) < 0 ? ~uint64_t(0) : 0;
				}
			}
		}
		if (!output && !constant.present && (locations & AL_SYMBOL) != 0)
		{
			ConstantInterpreter interpreter(analyzer);
			ConstValue value = ConstValue();
			if (interpreter.evaluate(expr, true, value) && value.kind == ValueKind::Pointer &&
				value.root != RootKind::Null && value.root != RootKind::Object)
				addresses[i] = value;
		}
		const bool symbol = addresses[i].kind == ValueKind::Pointer;
		const bool addressable = expr.facts->category == ValueCategory::LValue && (expr.facts->flags & kBitField) == 0;
		operand.input_storage = (operand.flags & AO_READ) != 0 && (locations & AL_MEMORY) != 0 &&
			(operand.flags & AO_NO_MEMORY) == 0 && addressable && !constant.present && !symbol;
		const Type* type = expr.facts->type->unqualified;
		const bool scalar = IsScalar(type) || type->kind == TypeKind::Vector;
		if (!scalar && !symbol && ((locations & AL_MEMORY) == 0 || !addressable))
			return "assembly operand requires scalar/vector data or addressable memory";
		if (!output && !constant.present && !symbol && !operand.input_storage && (locations & AL_REGISTER) == 0)
			return "assembly operand requires an addressable memory object or compile-time constant";
		if ((locations & AL_MEMORY) != 0)
		{
			unsigned effect = static_cast<unsigned>(packet.definition.memory);
			if ((operand.flags & AO_READ) != 0)
				effect |= static_cast<unsigned>(Memory::Read);
			if (output)
				effect |= static_cast<unsigned>(Memory::Write);
			packet.definition.memory = static_cast<Memory>(effect);
		}
	}
	return nullptr;
}

} // namespace

void Analyzer::asm_statement(const syntax::AsmStmt& stmt, Scope* scope)
{
	const uint32_t count = stmt.outputs.size + stmt.inputs.size;
	if (stmt.is_goto)
	{
		if (!full_asm_)
			error(stmt.location, "asm goto requires the optional full assembly profile");
		if (stmt.labels.empty())
			error(stmt.location, "asm goto requires outgoing labels");
		for (const syntax::GotoStmt* label : stmt.labels)
			statement(*label, scope);
	}
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
	if (full_asm_)
	{
		full_asm_statement(stmt, scope);
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
		info.fixed = 0;
		info.flag = AsmFlag::None;
		const bool flag_output = output && constraint.compare(0, 4, "=@cc") == 0;
		if (flag_output)
		{
			const std::string condition = constraint.substr(4);
			info.flag = condition == "c" || condition == "b" || condition == "nae" ? AsmFlag::Carry
				: condition == "nc" || condition == "ae" || condition == "nb"      ? AsmFlag::NotCarry
				: condition == "z" || condition == "e"                             ? AsmFlag::Zero
				: condition == "nz" || condition == "ne"                           ? AsmFlag::NotZero
																				   : AsmFlag::None;
		}
		uint32_t classes = 0;
		for (char c : constraint)
		{
			if (c == '=' || c == '+' || c == '&' || c == '%')
				continue;
			++classes;
			if (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'S' || c == 'D' || c == 'x')
				info.fixed = c;
		}
		if (classes != 1)
			info.fixed = 0;
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
			if (operands[tie].fixed == 'x')
				info.fixed = 'x';
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
		if (flag_output)
			info.binding = AsmOperandClass::Flag;
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
		IntegralKind flag_kind;
		if (flag_output && !IntegralKindOf(type, flag_kind))
			error(operand.location, "an asm flag output of no integer type");
		uint64_t size = 0, alignment = 0;
		if (!type_size(type, size, alignment, operand.location))
			error(operand.location, "an asm operand of incomplete type");
		if (info.binding == AsmOperandClass::Register && !output)
		{
			if (!IsScalar(type) && !(type->kind == TypeKind::Vector && size == 16 && info.fixed == 'x'))
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
	program->full = nullptr;
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
	program->reads = 0;
	program->immediates = 0;
	uint8_t* tied = nullptr;
	for (uint32_t i = 0; i < count; ++i)
	{
		program->immediates |= operands[i].binding == AsmOperandClass::Immediate ? 1u << i : 0u;
		program->reads |= operands[i].read ? 1u << i : 0u;
		program->registers |=
			(operands[i].binding == AsmOperandClass::Register || operands[i].binding == AsmOperandClass::Flag) ? 1u << i
																											   : 0u;
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

void Analyzer::full_asm_statement(const syntax::AsmStmt& stmt, Scope* scope)
{
	std::string text;
	if (!LiteralText(stmt.text, text))
		error(stmt.location, "assembly template requires an ordinary string literal");
	std::vector<assembly::GnuOperand> operands;
	const uint32_t count = stmt.outputs.size + stmt.inputs.size;
	for (uint32_t i = 0; i < count; ++i)
	{
		const bool output = i < stmt.outputs.size;
		const syntax::AsmOperand& source = output ? stmt.outputs[i] : stmt.inputs[i - stmt.outputs.size];
		assembly::GnuOperand operand;
		operand.output = output;
		if (source.name != nullptr)
			operand.name = source.name->spelling;
		for (const assembly::GnuOperand& prior : operands)
			if (!operand.name.empty() && prior.name == operand.name)
				error(source.location, "duplicate named assembly operand");
		if (!LiteralText(source.constraint, operand.constraint))
			error(source.location, "assembly constraint requires an ordinary string literal");
		const ExprFacts* facts = typed(*source.expression, scope);
		if (facts == nullptr)
			return;
		if (output && (!modifiable_lvalue(facts) || facts->category != ValueCategory::LValue))
			error(source.location, "assembly output requires a modifiable lvalue");
		const Type* type = facts->type->unqualified;
		IntegralKind flag_kind;
		if (operand.constraint.compare(0, 4, "=@cc") == 0 && !IntegralKindOf(type, flag_kind))
			error(source.location, "assembly flag output requires an integer type");
		uint64_t bytes = 0, alignment = 0;
		if (!type_size(type, bytes, alignment, source.location))
			error(source.location, "assembly operand has incomplete or unsupported size");
		operand.bits = bytes <= UINT16_MAX / 8 ? static_cast<unsigned>(bytes * 8) : 0;
		operand.is_volatile = (facts->type->cv & kVolatile) != 0;
		if (!output && (IsScalar(type) || type->kind == TypeKind::Vector))
			convert_operand(*source.expression, type);
		operands.push_back(operand);
	}
	for (const syntax::GotoStmt* target : stmt.labels)
	{
		assembly::GnuOperand label;
		label.label = true;
		label.name = target->label->spelling;
		for (const assembly::GnuOperand& prior : operands)
			if (prior.name == label.name)
				error(target->location, "duplicate assembly operand or label name");
		operands.push_back(label);
	}
	std::vector<std::string> clobbers;
	for (const TokenSpelling& spelling : stmt.clobbers)
	{
		std::string name;
		if (!LiteralText(spelling, name))
			error(stmt.location, "assembly clobber requires an ordinary string literal");
		clobbers.push_back(name);
	}
	assembly::PacketBuilder built;
	std::vector<ConstValue> addresses;
	try
	{
		built = assembly::gnu_packet(text, operands, clobbers, stmt.is_volatile || stmt.is_goto, !stmt.is_extended);
		if (const char* message = prepare_operands(*this, stmt, scope, built, addresses))
			error(stmt.location, message);
	}
	catch (const assembly::AssemblyError& failure)
	{
		error(stmt.location, copy_label(failure.message.data(), failure.message.size()));
	}
	assembly::Packet* packet = arena_.allocate_array<assembly::Packet>(1);
	packet->definition = built.definition;
	if (stmt.is_inline)
		packet->definition.properties |= asm_model::AP_INLINE_COST_SMALL;
	packet->text = arena_.copy_string(built.text.data(), built.text.size());
	packet->text_size = static_cast<uint32_t>(built.text.size());
	packet->operands = preserve(arena_, built.operands);
	packet->alternatives = preserve(arena_, built.alternatives);
	packet->groups = preserve(arena_, built.groups);
	packet->immediates = preserve(arena_, built.immediates);
	packet->pieces = preserve(arena_, built.pieces);
	packet->registers = preserve(arena_, built.registers);
	packet->clobbers = preserve(arena_, built.clobbers);
	packet->constants = preserve(arena_, built.constants);
	AsmProgram* program = arena_.allocate_array<AsmProgram>(1);
	*program = AsmProgram();
	program->full = packet;
	program->addresses = addresses.empty() ? nullptr : arena_.copy_array(addresses.data(), addresses.size());
	stmt.program = program;
}

} // namespace sema
