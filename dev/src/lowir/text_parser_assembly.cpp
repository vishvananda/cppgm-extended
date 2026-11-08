#include "lowir/text_parser.h"

#include "asm/registers.h"
#include "lowir/assembly.h"

namespace lowir {
namespace text {

uint32_t Parser::assembly_number(uint32_t limit, const char* what, bool absent)
{
	if (absent && accept(TokenKind::Minus))
	{
		if (!at(TokenKind::Number) || token().length != 1 || token().text[0] != '1')
			fail("an absent assembly index must be -1");
		advance();
		return kNoId;
	}
	if (!at(TokenKind::Number))
		fail(std::string("expected ") + what);
	const uint64_t value = parse_natural(token().text, token().length);
	if (value > limit)
		fail(std::string(what) + " is out of range");
	advance();
	return static_cast<uint32_t>(value);
}

AssemblyRegister Parser::parse_assembly_register()
{
	AssemblyRegister reg;
	if (!at(TokenKind::Word) || !assembly::parse_register(token().text, token().length, reg))
		fail("expected architectural register name");
	advance();
	if (accept(TokenKind::Colon))
	{
		reg.bits = static_cast<uint16_t>(assembly_number(UINT16_MAX, "assembly register width"));
		if (accept(TokenKind::Colon))
			reg.offset = static_cast<uint8_t>(assembly_number(UINT8_MAX, "assembly register offset"));
	}
	return reg;
}

void Parser::parse_assembly_alternative()
{
	AssemblyModule& module = *program_.assembly;
	AssemblyAlternative alt;
	alt.locations = static_cast<uint8_t>(assembly_number(31, "assembly locations"));
	alt.early_write = assembly_number(1, "assembly early-write flag") != 0;
	alt.matching = assembly_number(kNoId - 1, "assembly matching operand", true);
	alt.first_group = static_cast<uint32_t>(module.groups.size());
	alt.first_immediate = static_cast<uint32_t>(module.immediates.size());
	expect(TokenKind::LBrace, "{");
	while (!accept(TokenKind::RBrace))
	{
		if (at_word("group"))
		{
			advance();
			AssemblyRegisterGroup group;
			group.first_register = static_cast<uint32_t>(module.registers.size());
			expect(TokenKind::LParen, "(");
			do
			{
				const AssemblyRegister reg = parse_assembly_register();
				module.registers.push_back(reg);
				if (group.register_count == UINT16_MAX)
					fail("too many registers in assembly group");
				++group.register_count;
			} while (accept(TokenKind::Comma));
			expect(TokenKind::RParen, ")");
			if (alt.group_count == UINT16_MAX)
				fail("too many assembly register groups");
			++alt.group_count;
			module.groups.push_back(group);
		}
		else if (at_word("range"))
		{
			advance();
			AssemblyImmediate immediate;
			immediate.kind = static_cast<asm_model::ImmediateKind>(assembly_number(2, "assembly immediate kind"));
			immediate.is_signed = assembly_number(1, "assembly immediate signed flag") != 0;
			auto bits = [&]() {
				if (!at(TokenKind::Number))
					fail("expected unsigned assembly bits");
				const uint64_t value = parse_natural(token().text, token().length);
				advance();
				return value;
			};
			immediate.low.low = bits();
			immediate.low.high = bits();
			immediate.high.low = bits();
			immediate.high.high = bits();
			if (alt.immediate_count == UINT16_MAX)
				fail("too many assembly immediate ranges");
			++alt.immediate_count;
			module.immediates.push_back(immediate);
		}
		else
			fail("expected assembly group or range");
	}
	module.alternatives.push_back(alt);
}

void Parser::parse_assembly_operand()
{
	AssemblyModule& module = *program_.assembly;
	AssemblyOperand operand;
	operand.type = parse_type();
	operand.input = assembly_number(kNoId - 1, "assembly input argument", true);
	operand.output = assembly_number(kNoId - 1, "assembly output argument", true);
	operand.flags = static_cast<uint8_t>(assembly_number(127, "assembly operand flags"));
	operand.input_storage = assembly_number(1, "assembly storage flag") != 0;
	operand.condition = static_cast<uint8_t>(assembly_number(15, "assembly flag condition"));
	operand.first_alternative = static_cast<uint32_t>(module.alternatives.size());
	expect(TokenKind::LBrace, "{");
	while (!accept(TokenKind::RBrace))
	{
		if (!at_word("alternative"))
			fail("expected assembly alternative");
		advance();
		if (operand.alternative_count == UINT16_MAX)
			fail("too many assembly alternatives");
		++operand.alternative_count;
		parse_assembly_alternative();
	}
	module.operands.push_back(operand);
}

void Parser::parse_assembly()
{
	if (!program_.assembly)
		program_.assembly.reset(new AssemblyModule());
	AssemblyModule& module = *program_.assembly;
	AssemblyTemplate definition;
	definition.name = expect_name(TokenKind::Global, "assembly template name");
	if (definition.name >= assemblies_.size())
		assemblies_.resize(definition.name + 1, kNoId);
	if (assemblies_[definition.name] != kNoId)
		fail("duplicate assembly template");
	if (!at(TokenKind::Word) || !asm_model::from_spelling(token().text, token().length, &definition.dialect))
		fail("expected intel or att assembly dialect");
	advance();
	if (!at(TokenKind::Word) || !asm_model::from_spelling(token().text, token().length, &definition.memory))
		fail("expected assembly memory effect");
	advance();
	definition.properties = static_cast<uint16_t>(assembly_number(127, "assembly properties"));
	definition.state_read = assembly_number(63, "assembly state reads");
	definition.state_write = assembly_number(63, "assembly state writes");
	definition.argument_count = assembly_number(kNoId - 1, "assembly argument count");
	definition.first_operand = static_cast<uint32_t>(module.operands.size());
	definition.first_piece = static_cast<uint32_t>(module.pieces.size());
	definition.first_clobber = static_cast<uint32_t>(module.clobbers.size());
	expect(TokenKind::LBrace, "{");
	while (!accept(TokenKind::RBrace))
	{
		if (at_word("operand"))
		{
			advance();
			parse_assembly_operand();
			++definition.operand_count;
		}
		else if (at_word("clobber"))
		{
			advance();
			const AssemblyRegister reg = parse_assembly_register();
			module.clobbers.push_back(reg);
			++definition.clobber_count;
		}
		else
		{
			AssemblyPiece piece;
			if (at_word("text"))
			{
				advance();
				if (!at(TokenKind::String))
					fail("expected quoted assembly text");
				piece.first = static_cast<uint32_t>(module.text.size());
				piece.count = program_.names.length(token().name);
				module.text.append(program_.spelling(token().name), piece.count);
				advance();
			}
			else if (at_word("use"))
			{
				advance();
				piece.kind = asm_model::PieceKind::Operand;
				piece.operand = assembly_number(kNoId - 1, "assembly template operand");
				piece.format = static_cast<asm_model::Format>(assembly_number(4, "assembly operand format"));
				piece.bits = static_cast<uint16_t>(assembly_number(UINT16_MAX, "assembly operand width"));
				piece.offset = static_cast<uint8_t>(assembly_number(UINT8_MAX, "assembly operand offset"));
			}
			else if (at_word("unique"))
			{
				advance();
				piece.kind = asm_model::PieceKind::Unique;
			}
			else
				fail("expected assembly operand, clobber or template piece");
			module.pieces.push_back(piece);
			++definition.piece_count;
		}
	}
	if (at(TokenKind::Debug))
		definition.location = parse_debug_location();
	assemblies_[definition.name] = static_cast<uint32_t>(module.templates.size());
	module.templates.push_back(definition);
}

void Parser::parse_assembly_call(FunctionBuilder& builder, Instruction& inst)
{
	const NameId name = expect_name(TokenKind::Global, "assembly template name");
	if (!program_.assembly || name >= assemblies_.size() || assemblies_[name] == kNoId)
		fail("assembly template must be defined before its invocation");
	inst.op = Opcode::Asm;
	inst.immediate = assemblies_[name];
	const AssemblyModule& module = *program_.assembly;
	const AssemblyTemplate& definition = module.templates[inst.immediate];
	if (uint64_t(definition.argument_count) > uint64_t(definition.operand_count) * 2)
		fail("too many assembly arguments for its operands");
	std::vector<Type> types(definition.argument_count, Type::scalar(TypeKind::Void));
	for (uint32_t i = 0; i < definition.operand_count; ++i)
	{
		const AssemblyOperand& operand = module.operands[definition.first_operand + i];
		if ((operand.input != kNoId && operand.input >= types.size()) ||
			(operand.output != kNoId && operand.output >= types.size()))
			fail("assembly argument index is out of range");
		if (operand.input != kNoId)
			types[operand.input] = operand.input_storage ? Type::scalar(TypeKind::Ptr) : operand.type;
		if (operand.output != kNoId)
			types[operand.output] = Type::scalar(TypeKind::Ptr);
	}
	expect(TokenKind::LParen, "(");
	std::vector<Value> values;
	for (uint32_t i = 0; i < definition.argument_count; ++i)
	{
		if (i != 0)
			expect(TokenKind::Comma, ",");
		values.push_back(parse_value(builder, types[i].kind()));
	}
	expect(TokenKind::RParen, ")");
	inst.extra_count = static_cast<uint32_t>(values.size());
	inst.extra_first = builder.add_operands(values.data(), inst.extra_count);
}

void Parser::parse_assembly_function()
{
	const NameId name = expect_name(TokenKind::Global, "assembly function name");
	const SymbolId symbol = program_.find_symbol(name);
	if (symbol == kNoId || program_.symbols[symbol].kind != SymbolKind::FunctionDeclaration ||
		find_assembly_function(program_, symbol))
		fail("assembly function requires a prior, undefined function declaration");
	program_.functions[program_.symbols[symbol].detail].body.reset(new FunctionBody());
	expect(TokenKind::LBrace, "{");
	parse_body(symbol);
	retain_assembly_function(program_, symbol);
}

} // namespace text
} // namespace lowir
