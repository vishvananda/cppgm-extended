#include "asm/parser.h"

#include <algorithm>
#include <climits>
#include <map>

#include "asm/registers.h"

namespace assembly {
namespace {

class Parser
{
public:
	Parser(const std::string& source, asm_model::Dialect dialect) : source_(source), dialect_(dialect) {}
	std::vector<Instruction> run();

private:
	char current() const { return pos_ < source_.size() ? source_[pos_] : '\0'; }
	void step()
	{
		if (current() == '\n')
		{
			++line_;
			column_ = 1;
		}
		else
			++column_;
		++pos_;
	}
	void space()
	{
		while (current() == ' ' || current() == '\t' || current() == '\r')
			step();
	}
	bool take(char c)
	{
		space();
		if (current() != c)
			return false;
		step();
		return true;
	}
	void expect(char c)
	{
		if (!take(c))
			fail(std::string("expected '") + c + "'");
	}
	[[noreturn]] void fail(const std::string& message) const { throw AssemblyError{ message, line_, column_ }; }
	std::string word(bool fold = true);
	Operand target();
	void resolve_labels(std::vector<Instruction>& instructions);
	uint64_t integer();
	asm_model::Register reg();
	Operand operand();
	Operand intel_memory(uint16_t bits);
	Operand att_memory(int64_t displacement);
	void address_register(const asm_model::Register& reg) const;

	const std::string& source_;
	asm_model::Dialect dialect_;
	std::size_t pos_ = 0;
	uint32_t line_ = 1;
	uint32_t column_ = 1;
};

bool alpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '.';
}

std::string Parser::word(bool fold)
{
	space();
	std::string result;
	while (alpha(current()) || (current() >= '0' && current() <= '9'))
	{
		const char c = current();
		result += fold && c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : c;
		step();
	}
	if (result.empty())
		fail("expected assembly identifier");
	return result;
}

uint64_t Parser::integer()
{
	space();
	const bool negative = take('-');
	if (!negative)
		take('+');
	unsigned base = 10;
	if (current() == '0' && pos_ + 1 < source_.size() && (source_[pos_ + 1] == 'x' || source_[pos_ + 1] == 'X'))
	{
		base = 16;
		step();
		step();
	}
	uint64_t value = 0;
	bool any = false;
	for (;;)
	{
		const char c = current();
		const unsigned digit = c >= '0' && c <= '9' ? static_cast<unsigned>(c - '0')
			: c >= 'a' && c <= 'f'                  ? static_cast<unsigned>(c - 'a' + 10)
			: c >= 'A' && c <= 'F'                  ? static_cast<unsigned>(c - 'A' + 10)
													: 16;
		if (digit >= base)
			break;
		if (value > (UINT64_MAX - digit) / base)
			fail("assembly integer exceeds 64 bits");
		value = value * base + digit;
		any = true;
		step();
	}
	if (!any)
		fail("expected assembly integer");
	if (negative && value > (uint64_t(1) << 63))
		fail("negative assembly integer exceeds 64 bits");
	return negative ? uint64_t(0) - value : value;
}

asm_model::Register Parser::reg()
{
	space();
	if (dialect_ == asm_model::Dialect::Att)
		expect('%');
	const std::string name = word();
	asm_model::Register result;
	if (!parse_register(name.data(), name.size(), result))
		fail("unknown architectural register: " + name);
	return result;
}

void Parser::address_register(const asm_model::Register& reg) const
{
	if (reg.bank != asm_model::RegisterBank::Gpr || reg.bits != 64 || reg.offset != 0)
		fail("this assembly address requires a 64-bit general register");
}

Operand Parser::intel_memory(uint16_t bits)
{
	Operand result;
	result.kind = OperandKind::Memory;
	result.bits = bits;
	expect('[');
	bool first = true;
	int64_t displacement = 0;
	do
	{
		bool minus = false;
		if (!first)
		{
			minus = take('-');
			if (!minus)
				expect('+');
		}
		space();
		if (alpha(current()))
		{
			if (minus)
				fail("an assembly address cannot subtract a register");
			const asm_model::Register value = reg();
			address_register(value);
			const bool scaled = take('*');
			uint64_t scale = scaled ? integer() : 1;
			if (scale != 1 && scale != 2 && scale != 4 && scale != 8)
				fail("assembly address scale must be 1, 2, 4 or 8");
			if (!scaled && !result.address.has_base)
			{
				result.address.base = value;
				result.address.has_base = true;
			}
			else if (!result.address.has_index && value.number != 4)
			{
				result.address.index = value;
				result.address.has_index = true;
				result.address.scale = static_cast<uint8_t>(scale);
			}
			else
				fail("invalid or repeated assembly address index");
		}
		else
		{
			const int64_t term = static_cast<int64_t>(integer());
			// The encoded displacement is signed 32-bit. Keep every partial
			// expression in that range to avoid signed arithmetic overflow.
			if (term < INT32_MIN || term > INT32_MAX)
				fail("assembly address displacement exceeds 32 bits");
			displacement += minus ? -term : term;
			if (displacement < INT32_MIN || displacement > INT32_MAX)
				fail("assembly address displacement exceeds 32 bits");
		}
		first = false;
		space();
	} while (current() != ']' && current() != '\0' && current() != '\n');
	expect(']');
	result.address.displacement = static_cast<int32_t>(displacement);
	return result;
}

Operand Parser::att_memory(int64_t displacement)
{
	if (displacement < INT32_MIN || displacement > INT32_MAX)
		fail("assembly address displacement exceeds 32 bits");
	Operand result;
	result.kind = OperandKind::Memory;
	result.address.displacement = static_cast<int32_t>(displacement);
	if (!take('('))
		return result;
	space();
	if (current() == '%')
	{
		result.address.base = reg();
		address_register(result.address.base);
		result.address.has_base = true;
	}
	if (take(','))
	{
		result.address.index = reg();
		address_register(result.address.index);
		if (result.address.index.number == 4)
			fail("rsp cannot be an assembly address index");
		result.address.has_index = true;
		if (take(','))
		{
			const uint64_t scale = integer();
			if (scale != 1 && scale != 2 && scale != 4 && scale != 8)
				fail("assembly address scale must be 1, 2, 4 or 8");
			result.address.scale = static_cast<uint8_t>(scale);
		}
	}
	expect(')');
	if (!result.address.has_base && !result.address.has_index)
		fail("empty assembly address");
	return result;
}

Operand Parser::operand()
{
	space();
	Operand result;
	if (dialect_ == asm_model::Dialect::Att)
	{
		if (take('$'))
			result.immediate = integer();
		else if (current() == '%')
		{
			result.kind = OperandKind::Register;
			result.reg = reg();
			result.bits = result.reg.bits;
		}
		else
			return att_memory(current() == '(' ? 0 : static_cast<int64_t>(integer()));
		return result;
	}
	if (current() == '[')
		return intel_memory(0);
	if (alpha(current()))
	{
		const std::string name = word();
		if (parse_register(name.data(), name.size(), result.reg))
		{
			result.kind = OperandKind::Register;
			result.bits = result.reg.bits;
			return result;
		}
		const char* const sizes[] = { "byte", "word", "dword", "qword", "tbyte", "xmmword", "ymmword", "zmmword" };
		const uint16_t bits[] = { 8, 16, 32, 64, 80, 128, 256, 512 };
		for (unsigned i = 0; i < 8; ++i)
			if (name == sizes[i])
			{
				if (word() != "ptr")
					fail("expected ptr after assembly memory width");
				return intel_memory(bits[i]);
			}
		fail("unsupported assembly symbol expression: " + name);
	}
	result.immediate = integer();
	return result;
}

Operand Parser::target()
{
	Operand result;
	result.kind = OperandKind::Label;
	result.target = word(false);
	return result;
}

void Parser::resolve_labels(std::vector<Instruction>& instructions)
{
	std::map<std::string, uint32_t> named;
	std::map<std::string, std::vector<uint32_t>> numbered;
	for (uint32_t i = 0; i < instructions.size(); ++i)
	{
		const Instruction& inst = instructions[i];
		if (!inst.label)
			continue;
		if (inst.mnemonic.find_first_not_of("0123456789") == std::string::npos)
			numbered[inst.mnemonic].push_back(i);
		else if (!named.emplace(inst.mnemonic, i).second)
			throw AssemblyError{ "duplicate assembly label: " + inst.mnemonic, inst.line, inst.column };
	}
	for (uint32_t i = 0; i < instructions.size(); ++i)
		for (Operand& operand : instructions[i].operands)
		{
			if (operand.kind != OperandKind::Label)
				continue;
			const std::string& name = operand.target;
			uint32_t selected = UINT32_MAX;
			if (name.size() >= 2 && (name.back() == 'f' || name.back() == 'b') &&
				name.find_first_not_of("0123456789") == name.size() - 1)
			{
				const auto found = numbered.find(name.substr(0, name.size() - 1));
				if (found != numbered.end())
					for (uint32_t candidate : found->second)
						if ((name.back() == 'f' && candidate > i && selected == UINT32_MAX) ||
							(name.back() == 'b' && candidate < i))
							selected = candidate;
			}
			else
			{
				const auto found = named.find(name);
				if (found != named.end())
					selected = found->second;
			}
			if (selected == UINT32_MAX)
				throw AssemblyError{ "undefined assembly-local label: " + name, instructions[i].line,
					instructions[i].column };
			operand.label = selected;
		}
}

std::vector<Instruction> Parser::run()
{
	std::vector<Instruction> result;
	while (pos_ < source_.size())
	{
		space();
		if (current() == '#')
			while (pos_ < source_.size() && current() != '\n')
				step();
		if (current() == '\n' || current() == ';')
		{
			step();
			continue;
		}
		if (pos_ == source_.size())
			break;
		if (current() == '\0')
			fail("NUL byte in assembly source");
		Instruction inst;
		inst.line = line_;
		inst.column = column_;
		inst.mnemonic = word(false);
		space();
		if (take(':'))
		{
			inst.label = true;
			result.push_back(inst);
			continue;
		}
		for (char& c : inst.mnemonic)
			if (c >= 'A' && c <= 'Z')
				c = static_cast<char>(c + ('a' - 'A'));
		const bool branch = inst.mnemonic[0] == 'j' || inst.mnemonic == "call" || inst.mnemonic == "callq";
		if (current() != '\n' && current() != ';' && current() != '#' && pos_ != source_.size())
		{
			do
			{
				inst.operands.push_back(branch && current() != '*' ? target() : operand());
				if (inst.operands.size() > 5)
					fail("too many assembly instruction operands");
			} while (take(','));
		}
		space();
		if (pos_ != source_.size() && current() != '\n' && current() != ';' && current() != '#')
			fail("unexpected token after assembly instruction");
		if (dialect_ == asm_model::Dialect::Att)
			std::reverse(inst.operands.begin(), inst.operands.end());
		result.push_back(inst);
	}
	resolve_labels(result);
	return result;
}

} // namespace

std::vector<Instruction> parse(const std::string& source, asm_model::Dialect dialect)
{
	return Parser(source, dialect).run();
}

} // namespace assembly
