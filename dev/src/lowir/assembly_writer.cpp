#include "lowir/assembly_writer.h"

#include "asm/registers.h"
#include "lowir/assembly.h"

namespace lowir {
namespace {

void number(std::string& out, uint64_t value)
{
	out += std::to_string(value);
	out += ' ';
}

void index(std::string& out, uint32_t value)
{
	if (value == kNoId)
		out += "-1 ";
	else
		number(out, value);
}

void quoted(std::string& out, const char* text, uint32_t size)
{
	out += '"';
	for (uint32_t i = 0; i < size; ++i)
	{
		const char c = text[i];
		if (c == '\n')
			out += "\\n";
		else if (c == '\t')
			out += "\\t";
		else if (c == '\r')
			out += "\\r";
		else
		{
			if (c == '\\' || c == '"')
				out += '\\';
			out += c;
		}
	}
	out += '"';
}

void reg(std::string& out, AssemblyRegister value)
{
	std::string name = assembly::register_name(value);
	if (!name.empty())
	{
		out += name;
		return;
	}
	const uint16_t bits = value.bits;
	const uint8_t offset = value.offset;
	using asm_model::RegisterBank;
	value.bits = value.bank == RegisterBank::Vector ? 128
		: value.bank == RegisterBank::X87           ? 80
		: value.bank == RegisterBank::Tile          ? 8192
		: value.bank == RegisterBank::Segment       ? 16
													: 64;
	value.offset = 0;
	out += assembly::register_name(value);
	out += ':';
	out += std::to_string(bits);
	if (offset != 0)
	{
		out += ':';
		out += std::to_string(offset);
	}
}

void alternative(std::string& out, const AssemblyModule& module, const AssemblyAlternative& alt)
{
	out += "    alternative ";
	number(out, alt.locations);
	number(out, alt.early_write);
	index(out, alt.matching);
	out += "{\n";
	for (uint32_t i = 0; i < alt.group_count; ++i)
	{
		const AssemblyRegisterGroup& group = module.groups[alt.first_group + i];
		out += "      group (";
		for (uint32_t j = 0; j < group.register_count; ++j)
		{
			if (j != 0)
				out += ", ";
			reg(out, module.registers[group.first_register + j]);
		}
		out += ")\n";
	}
	for (uint32_t i = 0; i < alt.immediate_count; ++i)
	{
		const AssemblyImmediate& value = module.immediates[alt.first_immediate + i];
		out += "      range ";
		number(out, static_cast<unsigned>(value.kind));
		number(out, value.is_signed);
		number(out, value.low.low);
		number(out, value.low.high);
		number(out, value.high.low);
		number(out, value.high.high);
		out += '\n';
	}
	out += "    }\n";
}

} // namespace

void append_assembly_reference(std::string& out, const Program& program, uint32_t id)
{
	out += '@';
	const NameId name = program.assembly->templates[id].name;
	if (name != 0)
		out += program.spelling(name);
	else
		out += std::to_string(id);
}

void append_assembly_definitions(std::string& out, const Program& program)
{
	if (!program.assembly)
		return;
	const AssemblyModule& module = *program.assembly;
	for (uint32_t id = 0; id < module.templates.size(); ++id)
	{
		const AssemblyTemplate& definition = module.templates[id];
		out += "assembly ";
		append_assembly_reference(out, program, id);
		out += ' ';
		out += asm_model::spelling(definition.dialect);
		out += ' ';
		out += asm_model::spelling(definition.memory);
		out += ' ';
		number(out, definition.properties);
		number(out, definition.state_read);
		number(out, definition.state_write);
		number(out, definition.argument_count);
		out += "{\n";
		for (uint32_t i = 0; i < definition.operand_count; ++i)
		{
			const AssemblyOperand& operand = module.operands[definition.first_operand + i];
			out += "  operand ";
			out += spelling(operand.type.kind());
			if (operand.type.is_object())
				out += '<' + std::to_string(operand.type.bytes) + 'x' + std::to_string(1u << operand.type.align_log2) +
					'>';
			out += ' ';
			index(out, operand.input);
			index(out, operand.output);
			number(out, operand.flags);
			number(out, operand.input_storage);
			number(out, operand.condition);
			out += "{\n";
			for (uint32_t j = 0; j < operand.alternative_count; ++j)
				alternative(out, module, module.alternatives[operand.first_alternative + j]);
			out += "  }\n";
		}
		for (uint32_t i = 0; i < definition.clobber_count; ++i)
		{
			out += "  clobber ";
			reg(out, module.clobbers[definition.first_clobber + i]);
			out += '\n';
		}
		for (uint32_t i = 0; i < definition.piece_count; ++i)
		{
			const AssemblyPiece& piece = module.pieces[definition.first_piece + i];
			if (piece.kind == asm_model::PieceKind::Text)
			{
				out += "  text ";
				quoted(out, module.text.data() + piece.first, piece.count);
			}
			else if (piece.kind == asm_model::PieceKind::Unique)
				out += "  unique";
			else
			{
				out += "  use ";
				number(out, piece.operand);
				number(out, static_cast<unsigned>(piece.format));
				number(out, piece.bits);
				number(out, piece.offset);
			}
			out += '\n';
		}
		out += '}';
		if (definition.location.present())
		{
			out += " !dbg(";
			out += program.spelling(definition.location.file);
			out += ", " + std::to_string(definition.location.line) + ", " + std::to_string(definition.location.column);
			if (definition.location.scope != 0)
				out += ", " + std::to_string(definition.location.scope);
			out += ')';
		}
		out += "\n\n";
	}
}

} // namespace lowir
