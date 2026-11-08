#include "asm/packet.h"

#include "asm/parser.h"
#include "asm/registers.h"

namespace assembly {

void gnu_constraints(PacketBuilder& packet, const std::vector<GnuOperand>& operands);

namespace {

using namespace asm_model;

[[noreturn]] void fail(const std::string& message)
{
	throw AssemblyError{ message, 1, 1 };
}

void text(PacketBuilder& packet, const std::string& value)
{
	if (value.empty())
		return;
	Piece piece;
	piece.first = static_cast<uint32_t>(packet.text.size());
	piece.count = static_cast<uint32_t>(value.size());
	packet.text += value;
	packet.pieces.push_back(piece);
}

uint32_t reference(const std::string& source, std::size_t& pos, const std::vector<GnuOperand>& operands, bool label)
{
	if (pos == source.size())
		fail("missing assembly operand reference");
	if (source[pos] == '[')
	{
		const std::size_t first = ++pos;
		while (pos < source.size() && source[pos] != ']')
			++pos;
		if (pos == source.size())
			fail("unterminated named assembly operand");
		const std::string name = source.substr(first, pos++ - first);
		for (uint32_t i = 0; i < operands.size(); ++i)
			if (!name.empty() && operands[i].name == name)
				return i;
		fail("unknown named assembly operand: " + name);
	}
	uint32_t extra = 0;
	if (label)
		for (const GnuOperand& operand : operands)
			if (operand.output && !operand.constraint.empty() && operand.constraint[0] == '+')
				++extra;
	uint64_t index = 0;
	bool any = false;
	while (pos < source.size() && source[pos] >= '0' && source[pos] <= '9')
	{
		index = index * 10 + static_cast<unsigned>(source[pos++] - '0');
		if (index >= uint64_t(operands.size()) + extra)
			fail("assembly operand reference is out of range");
		any = true;
	}
	if (!any)
		fail("invalid assembly operand reference");
	if (label)
	{
		if (index < extra)
			fail("assembly label reference is out of range");
		index -= extra;
	}
	return static_cast<uint32_t>(index);
}

void pieces(PacketBuilder& packet, const std::string& source, const std::vector<GnuOperand>& operands, bool basic)
{
	std::string literal;
	for (std::size_t pos = 0; pos < source.size();)
	{
		const char c = source[pos++];
		if (c != '%' || basic)
		{
			literal += c;
			continue;
		}
		if (pos == source.size())
			fail("trailing percent in assembly template");
		const char next = source[pos];
		if (next == '%' || next == '{' || next == '}' || next == '|')
		{
			literal += next;
			++pos;
			continue;
		}
		text(packet, literal);
		literal.clear();
		Piece piece;
		piece.kind = PieceKind::Operand;
		if (next == '=')
		{
			++pos;
			piece.kind = PieceKind::Unique;
		}
		else
		{
			if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z'))
			{
				++pos;
				switch (next)
				{
				case 'b':
					piece.bits = 8;
					break;
				case 'h':
					piece.bits = 8;
					piece.offset = 8;
					break;
				case 'w':
					piece.bits = 16;
					break;
				case 'k':
					piece.bits = 32;
					break;
				case 'q':
					piece.bits = 64;
					break;
				case 'x':
					piece.bits = 128;
					break;
				case 't':
					piece.bits = 256;
					break;
				case 'g':
					piece.bits = 512;
					break;
				case 'P':
				case 'c':
					piece.format = Format::Bare;
					break;
				case 'n':
					piece.format = Format::Negated;
					break;
				case 'l':
					piece.format = Format::Label;
					break;
				case 'z':
					piece.format = Format::Suffix;
					break;
				case 'V':
					piece.format = Format::Bare;
					piece.bits = 64;
					break;
				default:
					fail(std::string("assembly operand modifier is not implemented yet: ") + next);
				}
			}
			piece.operand = reference(source, pos, operands, piece.format == Format::Label);
			if (operands[piece.operand].label != (piece.format == Format::Label))
				fail("assembly label references require the label modifier");
		}
		packet.pieces.push_back(piece);
	}
	text(packet, literal);
}

} // namespace

PacketBuilder gnu_packet(const std::string& source, const std::vector<GnuOperand>& operands,
	const std::vector<std::string>& clobbers, bool is_volatile, bool basic)
{
	PacketBuilder packet;
	packet.definition.dialect = Dialect::Att;
	packet.definition.memory = basic ? Memory::ReadWrite : Memory::None;
	bool outputs = false;
	for (const GnuOperand& operand : operands)
		outputs = outputs || operand.output;
	packet.definition.properties = AP_STACK | AP_RED_ZONE |
		(is_volatile || basic || !outputs ? AP_SIDE_EFFECT : AP_DUPLICABLE);
	packet.definition.state_write = AS_CONDITION_FLAGS | AS_X87_STATUS | AS_MXCSR_STATUS;
	gnu_constraints(packet, operands);
	uint32_t columns = 0;
	for (uint32_t i = 0; i < packet.operands.size(); ++i)
	{
		const asm_model::Operand& operand = packet.operands[i];
		if ((operand.flags & AO_COMMUTATIVE) != 0 &&
			((operand.flags & (AO_READ | AO_WRITE)) != AO_READ || i + 1 == operands.size() || operands[i + 1].output ||
				operands[i + 1].label))
			fail("assembly commutative modifier requires two read-only inputs");
		if (operand.alternative_count != 0)
		{
			if (columns != 0 && columns != operand.alternative_count)
				fail("assembly operands have different alternative counts");
			columns = operand.alternative_count;
		}
	}
	pieces(packet, source, operands, basic);
	for (const Piece& piece : packet.pieces)
		if (piece.kind == PieceKind::Operand && (packet.operands[piece.operand].flags & AO_FLAG) != 0)
			fail("assembly flag output cannot appear in its template");
	for (const std::string& name : clobbers)
	{
		if (name == "memory")
			packet.definition.memory = Memory::ReadWrite;
		else if (name == "cc" || name == "flags")
			packet.definition.state_write |= AS_CONDITION_FLAGS;
		else
		{
			Register reg;
			if (!parse_register(name.data(), name.size(), reg))
				fail("unknown assembly clobber register: " + name);
			if (reg.bank == RegisterBank::Gpr && (reg.number == 4 || reg.number == 5))
				fail("assembly cannot clobber the stack or frame register");
			packet.clobbers.push_back(reg);
		}
	}
	packet.definition.operand_count = static_cast<uint32_t>(packet.operands.size());
	packet.definition.piece_count = static_cast<uint32_t>(packet.pieces.size());
	packet.definition.clobber_count = static_cast<uint32_t>(packet.clobbers.size());
	return packet;
}

} // namespace assembly
