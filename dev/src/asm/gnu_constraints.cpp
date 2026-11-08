#include "asm/packet.h"

#include <cstdlib>

#include "asm/parser.h"
#include "asm/registers.h"

namespace assembly {
namespace {

using namespace asm_model;

[[noreturn]] void fail(const std::string& message)
{
	throw AssemblyError{ message, 1, 1 };
}

void registers(PacketBuilder& packet, Alternative& alt, RegisterBank bank, unsigned mask)
{
	for (unsigned r = 0; r < 16; ++r)
		if (((mask >> r) & 1u) != 0)
		{
			RegisterGroup group;
			group.first_register = static_cast<uint32_t>(packet.registers.size());
			group.register_count = 1;
			packet.registers.push_back(Register{ bank, static_cast<uint8_t>(r), 0, 0 });
			packet.groups.push_back(group);
			++alt.group_count;
		}
	alt.locations |= AL_REGISTER;
}

void constraints(PacketBuilder& packet, const std::vector<GnuOperand>& source)
{
	for (uint32_t index = 0; index < source.size(); ++index)
	{
		const GnuOperand& input = source[index];
		const std::string& constraint = input.constraint;
		asm_model::Operand operand;
		operand.flags = input.output ? AO_WRITE : AO_READ;
		if (input.is_volatile)
			operand.flags |= AO_VOLATILE;
		std::size_t pos = 0;
		if (input.output)
		{
			if (constraint.empty() || (constraint[0] != '=' && constraint[0] != '+'))
				fail("assembly output requires = or + constraint");
			if (constraint[0] == '+')
				operand.flags |= AO_READ;
			++pos;
		}
		if ((operand.flags & AO_READ) != 0)
			operand.input = packet.definition.argument_count++;
		if ((operand.flags & AO_WRITE) != 0)
			operand.output = packet.definition.argument_count++;
		operand.first_alternative = static_cast<uint32_t>(packet.alternatives.size());
		if (input.output && constraint.compare(0, 4, "=@cc") == 0)
		{
			const char* const conditions[16][3] = { { "o", "o", "o" }, { "no", "no", "no" }, { "b", "c", "nae" },
				{ "ae", "nb", "nc" }, { "e", "z", "e" }, { "ne", "nz", "ne" }, { "be", "na", "be" },
				{ "a", "nbe", "a" }, { "s", "s", "s" }, { "ns", "ns", "ns" }, { "p", "pe", "p" }, { "np", "po", "np" },
				{ "l", "nge", "l" }, { "ge", "nl", "ge" }, { "le", "ng", "le" }, { "g", "nle", "g" } };
			bool found = false;
			for (uint8_t cc = 0; cc < 16; ++cc)
				for (const char* name : conditions[cc])
					if (constraint.substr(4) == name)
					{
						operand.condition = cc;
						found = true;
					}
			if (!found)
				fail("unknown assembly flag-output condition");
			operand.flags |= AO_FLAG;
			packet.operands.push_back(operand);
			continue;
		}
		for (;;)
		{
			bool early = false;
			Alternative alternative;
			alternative.first_group = static_cast<uint32_t>(packet.groups.size());
			alternative.first_immediate = static_cast<uint32_t>(packet.immediates.size());
			for (; pos < constraint.size() && constraint[pos] != ','; ++pos)
			{
				const char c = constraint[pos];
				if (c == '&')
				{
					if (!input.output)
						fail("early-clobber modifier requires an output");
					early = true;
					continue;
				}
				if (c == '%')
				{
					operand.flags |= AO_COMMUTATIVE;
					continue;
				}
				if (c == '[')
				{
					const std::size_t first = ++pos;
					while (pos < constraint.size() && constraint[pos] != ']')
						++pos;
					if (pos == constraint.size() || input.output)
						fail("invalid named assembly matching constraint");
					const std::string name = constraint.substr(first, pos - first);
					for (uint32_t i = 0; i < source.size(); ++i)
						if (source[i].output && !name.empty() && source[i].name == name)
							alternative.matching = i;
					if (alternative.matching == kNoId)
						fail("named assembly matching constraint does not name an output");
					continue;
				}
				if (c >= '0' && c <= '9')
				{
					if (input.output)
						fail("assembly output cannot have a matching constraint");
					uint64_t matching = 0;
					do
					{
						matching = matching * 10 + static_cast<unsigned>(constraint[pos] - '0');
						if (matching >= source.size())
							fail("assembly matching operand is out of range");
						++pos;
					} while (pos < constraint.size() && constraint[pos] >= '0' && constraint[pos] <= '9');
					--pos;
					if (!source[matching].output)
						fail("assembly matching constraint does not name an output");
					alternative.matching = static_cast<uint32_t>(matching);
					continue;
				}
				const char* fixed = c == 'a' ? "rax"
					: c == 'b'               ? "rbx"
					: c == 'c'               ? "rcx"
					: c == 'd'               ? "rdx"
					: c == 'S'               ? "rsi"
					: c == 'D'               ? "rdi"
											 : nullptr;
				if (fixed != nullptr)
				{
					Register reg;
					parse_register(fixed, 3, reg);
					registers(packet, alternative, RegisterBank::Gpr, 1u << reg.number);
				}
				else if (c == 'r' || c == 'q')
					registers(packet, alternative, RegisterBank::Gpr, 0xffefu); // exclude RSP
				else if (c == 'Q')
					registers(packet, alternative, RegisterBank::Gpr, 0xf);
				else if (c == 'R')
					registers(packet, alternative, RegisterBank::Gpr, 0xef);
				else if (c == 'x')
					registers(packet, alternative, RegisterBank::Vector, 0xffff);
				else
					fail(std::string("GNU assembly constraint is not implemented yet: ") + c);
			}
			alternative.early_write = early;
			if (alternative.locations == 0 && alternative.matching == kNoId)
				fail("empty assembly constraint alternative");
			packet.alternatives.push_back(alternative);
			++operand.alternative_count;
			if (pos == constraint.size())
				break;
			++pos;
		}
		packet.operands.push_back(operand);
	}
}

} // namespace

void gnu_constraints(PacketBuilder& packet, const std::vector<GnuOperand>& operands)
{
	constraints(packet, operands);
}

} // namespace assembly
