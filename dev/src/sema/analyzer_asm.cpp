// GNU assembly: type operands and preserve the shared assembly packet.
// Instruction parsing and encoding belong to the integrated assembler.

#include "sema/analyzer.h"
#include "asm/packet.h"
#include "asm/parser.h"

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
	if (stmt.is_goto)
	{
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
	program->packet = packet;
	program->addresses = addresses.empty() ? nullptr : arena_.copy_array(addresses.data(), addresses.size());
	stmt.program = program;
}

} // namespace sema
