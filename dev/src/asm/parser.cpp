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
	Parser(const std::string& source, asm_model::Dialect dialect, const std::vector<SymbolUse>& symbols,
		const std::vector<LabelUse>& labels, SymbolLookup* lookup, Syntax* state)
		: source_(source), symbols_(symbols), labels_(labels), dialect_(state ? state->dialect : dialect),
		  register_prefix_(state ? state->register_prefix : dialect == asm_model::Dialect::Att), lookup_(lookup),
		  state_(state)
	{}
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
	bool symbol(Operand& operand);
	void rip_register(Address& address);
	void resolve_labels(std::vector<Instruction>& instructions);
	uint64_t integer();
	bool syntax(const std::string& name);
	bool directive(Instruction& inst);
	asm_model::Register reg();
	Operand operand();
	void decorators(Operand& operand);
	Operand intel_memory(uint16_t bits);
	Operand att_memory(int64_t displacement);
	void address_register(const asm_model::Register& reg) const;

	const std::string& source_;
	const std::vector<SymbolUse>& symbols_;
	const std::vector<LabelUse>& labels_;
	asm_model::Dialect dialect_;
	bool register_prefix_;
	SymbolLookup* lookup_;
	Syntax* state_;
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
	if (register_prefix_)
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
		Operand bound;
		if (symbol(bound))
		{
			if (minus || result.address.symbol.symbol != asm_model::kNoId)
				fail("invalid assembly symbol expression");
			result.address.symbol = bound.address.symbol;
			result.address.local = bound.address.local;
			result.address.relocation = bound.address.relocation;
			displacement += bound.address.displacement;
		}
		else if (source_.compare(pos_, register_prefix_ ? 4 : 3, register_prefix_ ? "%rip" : "rip") == 0)
		{
			if (minus)
				fail("an assembly address cannot subtract RIP");
			if (register_prefix_)
				step();
			rip_register(result.address);
		}
		else if (register_prefix_ ? current() == '%' : alpha(current()))
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
		if (displacement < INT32_MIN || displacement > INT32_MAX)
			fail("assembly address displacement exceeds 32 bits");
		first = false;
		space();
	} while (current() != ']' && current() != '\0' && current() != '\n');
	expect(']');
	if (displacement < INT32_MIN || displacement > INT32_MAX)
		fail("assembly address displacement exceeds 32 bits");
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
	if (register_prefix_ ? current() == '%' : alpha(current()))
	{
		if (source_.compare(pos_, register_prefix_ ? 4 : 3, register_prefix_ ? "%rip" : "rip") == 0)
		{
			if (register_prefix_)
				step();
			rip_register(result.address);
		}
		else
		{
			result.address.base = reg();
			address_register(result.address.base);
			result.address.has_base = true;
		}
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
	if (!result.address.has_base && !result.address.has_index && !result.address.rip)
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
		{
			if (!symbol(result))
				result.immediate = integer();
		}
		else if (symbol(result))
		{
			const Address binding = result.address;
			result = att_memory(binding.displacement);
			result.address.symbol = binding.symbol;
			result.address.local = binding.local;
			result.address.relocation = binding.relocation;
		}
		else if (register_prefix_ ? current() == '%' : alpha(current()))
		{
			result.kind = OperandKind::Register;
			result.reg = reg();
			result.bits = result.reg.bits;
		}
		else
			return att_memory(current() == '(' ? 0 : static_cast<int64_t>(integer()));
		return result;
	}
	if (register_prefix_ && current() == '%')
	{
		result.kind = OperandKind::Register;
		result.reg = reg();
		result.bits = result.reg.bits;
		return result;
	}
	if (current() == '[')
		return intel_memory(0);
	if (symbol(result))
		return result;
	if (alpha(current()))
	{
		const std::string name = word();
		if (parse_register(name.data(), name.size(), result.reg))
		{
			if (register_prefix_)
				fail("assembly register requires a percent prefix");
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

void Parser::decorators(Operand& operand)
{
	while (take('{'))
	{
		take('%');
		const std::string name = word();
		asm_model::Register reg;
		if (parse_register(name.data(), name.size(), reg) && reg.bank == asm_model::RegisterBank::Mask)
		{
			if (reg.number == 0 || operand.mask != 0)
				fail("invalid or repeated assembly write mask");
			operand.mask = reg.number;
		}
		else if (name == "z")
		{
			if (operand.zero)
				fail("repeated zero-mask modifier");
			operand.zero = true;
		}
		else
		{
			const char* counts[] = { "1to2", "1to4", "1to8", "1to16", "1to32", "1to64" };
			bool found = false;
			for (unsigned i = 0; i < 6; ++i)
				if (name == counts[i])
				{
					if (operand.broadcast)
						fail("repeated broadcast modifier");
					operand.broadcast = static_cast<uint8_t>(2u << i);
					found = true;
				}
			if (!found)
				fail("assembly decorator is not implemented: " + name);
		}
		expect('}');
	}
}

void Parser::rip_register(Address& address)
{
	if (word() != "rip" || address.rip)
		fail("invalid RIP-relative assembly address");
	address.rip = true;
}

bool Parser::symbol(Operand& operand)
{
	space();
	const auto found = std::lower_bound(symbols_.begin(), symbols_.end(), pos_,
		[](const SymbolUse& use, size_t position) { return use.first < position; });
	asm_model::SymbolReference reference;
	if (found != symbols_.end() && found->first == pos_)
	{
		reference = found->reference;
		for (uint32_t i = 0; i < found->count; ++i)
			step();
	}
	else
	{
		const size_t begin = pos_;
		size_t end = begin;
		const bool numeric = current() >= '0' && current() <= '9';
		if (numeric)
		{
			while (end < source_.size() && source_[end] >= '0' && source_[end] <= '9')
				++end;
			if (end == source_.size() || (source_[end] != 'f' && source_[end] != 'b'))
				return false;
			++end;
			if (end < source_.size() && (alpha(source_[end]) || (source_[end] >= '0' && source_[end] <= '9')))
				return false;
		}
		else
		{
			if (!alpha(current()))
				return false;
			while (end < source_.size() && (alpha(source_[end]) || (source_[end] >= '0' && source_[end] <= '9')))
				++end;
		}
		const std::string name = source_.substr(begin, end - begin);
		asm_model::Register reg;
		if (name == "rip" || name == "byte" || name == "word" || name == "dword" || name == "qword" ||
			name == "xmmword" || name == "ymmword" || name == "zmmword" || name == "tbyte" ||
			parse_register(name.data(), name.size(), reg))
			return false;
		if (lookup_ != nullptr && !numeric)
		{
			if (!lookup_->lookup(name, reference))
				return false;
		}
		else
			operand.address.local = name;
		while (pos_ < end)
			step();
	}
	operand.kind = OperandKind::Symbol;
	operand.address.symbol = reference;
	int64_t displacement = reference.addend;
	space();
	if (take('@'))
	{
		const std::string relocation = word();
		operand.address.relocation = relocation == "gotpcrel" ? Relocation::Got
			: relocation == "gottpoff"                        ? Relocation::GotTpOff
			: relocation == "tlvp"                            ? Relocation::Tlv
			: relocation == "plt"                             ? Relocation::Plt
															  : Relocation::Address;
		if (operand.address.relocation == Relocation::Address)
			fail("unsupported assembly symbol relocation");
	}
	space();
	if (current() == '+' || current() == '-')
	{
		size_t next = pos_ + 1;
		while (next < source_.size() && (source_[next] == ' ' || source_[next] == '\t'))
			++next;
		if (next < source_.size() && source_[next] >= '0' && source_[next] <= '9')
		{
			const int64_t term = static_cast<int64_t>(integer());
			if (term < INT32_MIN || term > INT32_MAX)
				fail("assembly symbol displacement exceeds 32 bits");
			displacement += term;
		}
	}
	if (displacement < INT32_MIN || displacement > INT32_MAX)
		fail("assembly symbol displacement exceeds 32 bits");
	operand.address.displacement = static_cast<int32_t>(displacement);
	return true;
}

Operand Parser::target()
{
	space();
	Operand result;
	const auto label = std::lower_bound(labels_.begin(), labels_.end(), pos_,
		[](const LabelUse& use, size_t position) { return use.first < position; });
	if (label != labels_.end() && label->first == pos_)
	{
		result.kind = OperandKind::Label;
		result.label = label->operand;
		result.external_label = true;
		for (uint32_t i = 0; i < label->count; ++i)
			step();
		return result;
	}
	if (std::any_of(symbols_.begin(), symbols_.end(), [&](const SymbolUse& use) { return use.first == pos_; }))
		if (symbol(result))
			return result;
	result.kind = OperandKind::Label;
	if (dialect_ == asm_model::Dialect::Intel && current() == '[')
		return intel_memory(0);
	if (dialect_ == asm_model::Dialect::Intel && register_prefix_ && current() == '%')
	{
		result.kind = OperandKind::Register;
		result.reg = reg();
		result.bits = result.reg.bits;
		return result;
	}
	result.target = word(false);
	if (lookup_ != nullptr && take('@'))
	{
		if (word() != "plt" || !lookup_->lookup(result.target, result.address.symbol))
			fail("unsupported direct module branch relocation");
		result.kind = OperandKind::Symbol;
		result.address.relocation = Relocation::Plt;
		return result;
	}
	if (dialect_ == asm_model::Dialect::Intel)
	{
		if (parse_register(result.target.data(), result.target.size(), result.reg))
		{
			if (register_prefix_)
				fail("assembly register requires a percent prefix");
			result.kind = OperandKind::Register;
			result.bits = result.reg.bits;
		}
		else if (result.target == "qword")
		{
			if (word() != "ptr")
				fail("expected ptr after assembly branch width");
			return intel_memory(64);
		}
	}
	return result;
}

void Parser::resolve_labels(std::vector<Instruction>& instructions)
{
	std::map<std::string, uint32_t> named;
	std::map<std::string, std::vector<uint32_t>> numbered;
	for (uint32_t i = 0; i < instructions.size(); ++i)
	{
		const Instruction& inst = instructions[i];
		if (!inst.label || inst.symbol_definition != asm_model::kNoId)
			continue;
		if (inst.mnemonic.find_first_not_of("0123456789") == std::string::npos)
			numbered[inst.mnemonic].push_back(i);
		else if (!named.emplace(inst.mnemonic, i).second)
			throw AssemblyError{ "duplicate assembly label: " + inst.mnemonic, inst.line, inst.column };
	}
	for (uint32_t i = 0; i < instructions.size(); ++i)
		for (Operand& operand : instructions[i].operands)
		{
			if (operand.kind == OperandKind::Symbol && !operand.address.local.empty())
				throw AssemblyError{ "local code address requires RIP-relative memory", instructions[i].line,
					instructions[i].column };
			const bool memory = operand.kind == OperandKind::Memory && !operand.address.local.empty();
			if ((!memory && operand.kind != OperandKind::Label) || operand.external_label)
				continue;
			const std::string& name = memory ? operand.address.local : operand.target;
			if (lookup_ != nullptr && lookup_->interposable(name) && lookup_->lookup(name, operand.address.symbol))
			{
				operand.kind = OperandKind::Symbol;
				continue;
			}
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
			if (selected == UINT32_MAX && lookup_ != nullptr && lookup_->lookup(name, operand.address.symbol))
			{
				operand.kind = OperandKind::Symbol;
				continue;
			}
			if (selected == UINT32_MAX)
				throw AssemblyError{ "undefined assembly-local label: " + name, instructions[i].line,
					instructions[i].column };
			if (memory)
				operand.address.label = selected;
			else
				operand.label = selected;
		}
}

bool Parser::directive(Instruction& inst)
{
	const std::string& name = inst.mnemonic;
	if (name == ".p2align" || name == ".balign")
	{
		uint64_t alignment = integer();
		if (name == ".p2align")
		{
			if (alignment > 20)
				fail("assembly alignment exponent exceeds limit");
			alignment = uint64_t(1) << alignment;
		}
		if (alignment == 0 || alignment > (1u << 20) || (alignment & (alignment - 1)))
			fail("assembly alignment requires a bounded power of two");
		inst.alignment = alignment;
		if (take(','))
		{
			if (current() != ',')
			{
				const uint64_t fill = integer();
				if (fill > 255)
					fail("assembly alignment fill exceeds one byte");
				inst.fill = fill;
			}
			if (take(','))
			{
				const uint64_t limit = integer();
				if (limit > UINT32_MAX)
					fail("assembly padding limit exceeds range");
				inst.max_padding = limit;
			}
		}
		return true;
	}
	const unsigned width = name == ".byte"    ? 1
		: name == ".short" || name == ".word" ? 2
		: name == ".long"                     ? 4
		: name == ".quad"                     ? 8
											  : 0;
	if (width != 0)
	{
		inst.data = true;
		do
		{
			const uint64_t value = integer();
			if (inst.bytes.size() + width > (1u << 20))
				fail("assembly data exceeds byte limit");
			for (unsigned i = 0; i < width; ++i)
				inst.bytes.push_back(value >> (8 * i));
		} while (take(','));
		return true;
	}
	if (name != ".zero" && name != ".space" && name != ".skip")
		return false;
	const uint64_t count = integer();
	if (count > (1u << 20))
		fail("assembly data exceeds byte limit");
	const uint64_t fill = take(',') ? integer() : 0;
	if (fill > 255)
		fail("assembly data fill exceeds one byte");
	inst.data = true;
	inst.bytes.assign(count, fill);
	return true;
}

bool Parser::syntax(const std::string& name)
{
	if (name != ".intel_syntax" && name != ".att_syntax")
		return false;
	dialect_ = name == ".intel_syntax" ? asm_model::Dialect::Intel : asm_model::Dialect::Att;
	register_prefix_ = true;
	space();
	if (current() != '\n' && current() != ';' && current() != '#' && pos_ != source_.size())
	{
		const std::string mode = word();
		if (mode != "prefix" && mode != "noprefix")
			fail("unknown assembly register-prefix mode");
		register_prefix_ = mode == "prefix";
	}
	space();
	if (current() != '\n' && current() != ';' && current() != '#' && pos_ != source_.size())
		fail("unexpected token after assembly syntax directive");
	return true;
}

std::vector<Instruction> Parser::run()
{
	std::vector<Instruction> result;
	uint8_t pending_prefix = 0;
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
		const size_t definition_first = pos_;
		inst.mnemonic = word(false);
		space();
		if (take(':'))
		{
			if (pending_prefix != 0)
				fail("assembly prefix cannot precede a label");
			inst.label = true;
			if (lookup_ != nullptr)
				for (const SymbolUse& use : symbols_)
					if (use.first == definition_first)
					{
						if (use.reference.addend != 0)
							fail("module label cannot define an address addend");
						inst.symbol_definition = use.reference.symbol;
					}
			result.push_back(inst);
			continue;
		}
		for (char& c : inst.mnemonic)
			if (c >= 'A' && c <= 'Z')
				c = static_cast<char>(c + ('a' - 'A'));
		if (syntax(inst.mnemonic))
		{
			if (pending_prefix != 0)
				fail("assembly prefix cannot precede a syntax directive");
			continue;
		}
		const uint8_t prefix = inst.mnemonic == "lock"                                     ? 0xf0
			: inst.mnemonic == "rep" || inst.mnemonic == "repe" || inst.mnemonic == "repz" ? 0xf3
			: inst.mnemonic == "repne" || inst.mnemonic == "repnz"                         ? 0xf2
																						   : 0;
		if (prefix != 0)
		{
			if (pending_prefix != 0)
				fail("conflicting assembly instruction prefixes");
			pending_prefix = prefix;
			continue;
		}
		inst.prefix = pending_prefix;
		pending_prefix = 0;
		if (directive(inst))
		{
			if (inst.prefix)
				fail("assembly prefix cannot precede data or alignment");
			space();
			if (pos_ != source_.size() && current() != '\n' && current() != ';' && current() != '#')
				fail("unexpected token after assembly directive");
			result.push_back(std::move(inst));
			continue;
		}
		const bool branch = inst.mnemonic[0] == 'j' || inst.mnemonic == "call" || inst.mnemonic == "callq";
		if (current() != '\n' && current() != ';' && current() != '#' && pos_ != source_.size())
		{
			do
			{
				const bool indirect = branch && dialect_ == asm_model::Dialect::Att && take('*');
				Operand value = branch && !indirect ? target() : operand();
				decorators(value);
				inst.operands.push_back(std::move(value));
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
	if (pending_prefix != 0)
		fail("assembly prefix has no instruction");
	resolve_labels(result);
	if (state_)
	{
		state_->dialect = dialect_;
		state_->register_prefix = register_prefix_;
	}
	return result;
}

} // namespace

std::vector<Instruction> parse(const std::string& source, asm_model::Dialect dialect,
	const std::vector<SymbolUse>& symbols, const std::vector<LabelUse>& labels, SymbolLookup* lookup, Syntax* state)
{
	return Parser(source, dialect, symbols, labels, lookup, state).run();
}

} // namespace assembly
