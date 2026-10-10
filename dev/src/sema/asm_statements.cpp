#include "sema/asm_statements.h"

#include <cstring>

namespace sema {

namespace {

bool IsSpace(char c)
{
	return c == ' ' || c == '\t';
}

// The size an AT&T mnemonic's suffix names (b, w, l, q), or 0.
uint32_t SuffixBytes(char c)
{
	return c == 'b' ? 1 : c == 'w' ? 2 : c == 'l' ? 4 : c == 'q' ? 8 : 0;
}

// An instruction's mnemonic: its operation, operand count, and whether a
// size suffix may follow.
struct Mnemonic
{
	const char* name;
	AsmOperation operation;
	uint32_t operands;
	bool sized;
};

const Mnemonic kMnemonics[] = { { "mfence", AsmOperation::Fence, 0, false },
	{ "lfence", AsmOperation::Fence, 0, false }, { "sfence", AsmOperation::Fence, 0, false },
	{ "inc", AsmOperation::Add, 1, true }, { "dec", AsmOperation::Sub, 1, true }, { "not", AsmOperation::Not, 1, true },
	{ "neg", AsmOperation::Neg, 1, true }, { "add", AsmOperation::Add, 2, true }, { "sub", AsmOperation::Sub, 2, true },
	{ "and", AsmOperation::And, 2, true }, { "or", AsmOperation::Or, 2, true }, { "xor", AsmOperation::Xor, 2, true },
	{ "xchg", AsmOperation::Exchange, 2, true }, { "bswap", AsmOperation::ByteSwap, 1, true } };

// A parsed instruction operand: an asm operand, or an immediate.
struct Operand
{
	bool immediate;
	uint8_t index;
	uint64_t value;
};

// The size of the register GCC prints for an operand bound to one: its
// type's (the low register of a 16-byte pair).
uint32_t RegisterBytes(const AsmOperandInfo& operand)
{
	return operand.bytes == 16 ? 8 : operand.bytes;
}

// Whether GAS assembles an immediate for an instruction of `bytes`: a q
// instruction's field is a sign-extended 32-bit one (another value is an
// error); a narrower one's value is truncated to its width (a warning).
bool ImmediateFits(uint64_t value, uint32_t bytes)
{
	const int64_t signed_value = static_cast<int64_t>(value);
	return bytes != 8 || (signed_value >= -(int64_t(1) << 31) && signed_value < (int64_t(1) << 31));
}

class Lifter
{
public:
	Lifter(const AsmOperandInfo* operands, uint32_t count, std::vector<AsmInstruction>& out, std::string& error)
		: operands_(operands), count_(count), out_(out), error_(error)
	{}

	bool line(const std::string& text);

	bool finish()
	{
		if (lock_ || rep_)
			return fail("a prefix without its instruction");
		return true;
	}

private:
	bool fail(const char* message)
	{
		error_ = std::string("unsupported GNU asm template: ") + message;
		return false;
	}

	bool rest_empty(const std::string& text, size_t at)
	{
		while (at < text.size() && IsSpace(text[at]))
			++at;
		return at == text.size() || fail("trailing text after an instruction");
	}

	bool operand(const std::string& text, size_t& at, Operand& out);

	const AsmOperandInfo* operands_;
	uint32_t count_;
	std::vector<AsmInstruction>& out_;
	std::string& error_;
	bool lock_ = false;
	bool rep_ = false;
};

bool Lifter::line(const std::string& text)
{
	size_t at = 0;
	const auto word = [&](std::string& out) {
		while (at < text.size() && IsSpace(text[at]))
			++at;
		const size_t start = at;
		while (at < text.size() && !IsSpace(text[at]) && text[at] != ',')
			++at;
		out = text.substr(start, at - start);
	};
	std::string mnemonic;
	word(mnemonic);
	// Prefixes: `lock` applies to the next instruction, on this line or
	// the next; `rep` only as `rep; nop` (pause).
	while (mnemonic == "lock" || mnemonic == "rep" || mnemonic == "repe" || mnemonic == "repz")
	{
		if (mnemonic == "lock")
			lock_ = true;
		else
			rep_ = true;
		word(mnemonic);
	}
	if (mnemonic.empty())
		return true;
	if (mnemonic == "nop" || mnemonic == "pause")
	{
		// A spin hint: no effect on the program.
		if (lock_)
			return fail("a locked nop");
		rep_ = false;
		return rest_empty(text, at);
	}
	if (rep_)
		return fail("a repeated instruction other than nop");
	const Mnemonic* found = nullptr;
	uint32_t bytes = 0;
	for (const Mnemonic& candidate : kMnemonics)
	{
		const size_t size = std::strlen(candidate.name);
		if (mnemonic.compare(0, size, candidate.name) != 0)
			continue;
		if (mnemonic.size() == size)
			found = &candidate;
		else if (candidate.sized && mnemonic.size() == size + 1 && SuffixBytes(mnemonic[size]) != 0)
		{
			found = &candidate;
			bytes = SuffixBytes(mnemonic[size]);
		}
		if (found != nullptr)
			break;
	}
	if (found == nullptr)
		return fail("an instruction outside the lifted set");
	Operand parsed[2];
	for (uint32_t i = 0; i < found->operands; ++i)
	{
		if (i != 0)
		{
			while (at < text.size() && IsSpace(text[at]))
				++at;
			if (at >= text.size() || text[at] != ',')
				return fail("too few instruction operands");
			++at;
		}
		if (!operand(text, at, parsed[i]))
			return false;
	}
	if (!rest_empty(text, at))
		return false;
	AsmInstruction instruction = AsmInstruction();
	instruction.operation = found->operation;
	instruction.locked = lock_;
	instruction.source = kAsmImmediateSource;
	lock_ = false;
	if (found->operation == AsmOperation::Fence)
	{
		if (instruction.locked)
			return fail("a locked fence");
		out_.push_back(instruction);
		return true;
	}
	// AT&T order: the source first; the destination last. An exchange's
	// operands are interchangeable: its memory one, if any, last.
	if (found->operation == AsmOperation::Exchange && !parsed[0].immediate && !parsed[1].immediate &&
		operands_[parsed[0].index].binding == AsmOperandClass::Memory &&
		operands_[parsed[1].index].binding == AsmOperandClass::Register)
	{
		const Operand first = parsed[0];
		parsed[0] = parsed[1];
		parsed[1] = first;
	}
	const Operand& destination = parsed[found->operands - 1];
	if (destination.immediate || !operands_[destination.index].output || !operands_[destination.index].read)
		return fail("a destination that is no read-write operand");
	const AsmOperandInfo& target = operands_[destination.index];
	if (target.binding == AsmOperandClass::Immediate)
		return fail("an immediate destination");
	instruction.destination = destination.index;
	const bool memory = target.binding == AsmOperandClass::Memory;
	// The operation's size: the suffix's, which a register operand's
	// size must equal (GAS), else a register operand's. GAS assumes l
	// for a memory destination with no register operand (warning) and
	// Clang rejects it: so do we. A memory destination is the low bytes
	// of its object.
	const Operand* source = found->operands == 2 ? &parsed[0] : nullptr;
	const bool source_register = source != nullptr && !source->immediate &&
		operands_[source->index].binding != AsmOperandClass::Immediate;
	if (source_register && operands_[source->index].binding != AsmOperandClass::Register)
		return fail("a source operand that is no register");
	const uint32_t register_bytes = !memory ? RegisterBytes(target)
		: source_register                   ? RegisterBytes(operands_[source->index])
											: 0;
	if (bytes == 0 && register_bytes == 0)
		return fail("an instruction of no size (no suffix and no register operand)");
	if (bytes == 0)
		bytes = register_bytes;
	if ((register_bytes != 0 && bytes != register_bytes) || (bytes != 1 && bytes != 2 && bytes != 4 && bytes != 8))
		return fail("an operand size that disagrees with its instruction");
	if (source_register && RegisterBytes(operands_[source->index]) != bytes)
		return fail("a source operand that is no register of the instruction's size");
	if (memory && bytes > target.bytes)
		return fail("an operation wider than its memory operand");
	instruction.bytes = static_cast<uint8_t>(bytes);
	if (mnemonic.compare(0, 3, "inc") == 0 || mnemonic.compare(0, 3, "dec") == 0)
		instruction.immediate = 1;
	if (source != nullptr)
	{
		if (source_register)
			instruction.source = source->index;
		else
		{
			instruction.immediate = source->immediate ? source->value : operands_[source->index].value;
			if (!ImmediateFits(instruction.immediate, bytes))
				return fail("an immediate beyond its instruction's immediate field");
		}
	}
	if (instruction.operation == AsmOperation::Exchange)
	{
		// One register and the destination; with memory, implicitly
		// locked.
		if (instruction.source == kAsmImmediateSource || !operands_[instruction.source].output)
			return fail("an exchange of no two read-write operands");
		instruction.locked = memory;
	}
	if (instruction.operation == AsmOperation::ByteSwap && (memory || (bytes != 4 && bytes != 8)))
		return fail("a byte swap of no 32- or 64-bit register");
	if (instruction.locked && !memory)
		return fail("a locked instruction of no memory destination");
	out_.push_back(instruction);
	return true;
}

bool Lifter::operand(const std::string& text, size_t& at, Operand& out)
{
	while (at < text.size() && IsSpace(text[at]))
		++at;
	out = Operand();
	if (at < text.size() && text[at] == '$')
	{
		// $digits, as GAS reads them: 0x hexadecimal, 0b binary, a
		// leading 0 octal, else decimal; at most 64 bits.
		++at;
		const bool negative = at < text.size() && text[at] == '-';
		if (negative)
			++at;
		unsigned base = 10;
		if (at + 1 < text.size() && text[at] == '0' && (text[at + 1] == 'x' || text[at + 1] == 'X'))
		{
			base = 16;
			at += 2;
		}
		else if (at + 1 < text.size() && text[at] == '0' && (text[at + 1] == 'b' || text[at + 1] == 'B'))
		{
			base = 2;
			at += 2;
		}
		else if (at + 1 < text.size() && text[at] == '0' && text[at + 1] >= '0' && text[at + 1] <= '9')
		{
			base = 8;
			++at;
		}
		const size_t start = at;
		uint64_t value = 0;
		for (; at < text.size(); ++at)
		{
			const char c = text[at];
			const unsigned digit = c >= '0' && c <= '9' ? static_cast<unsigned>(c - '0')
				: c >= 'a' && c <= 'f'                  ? static_cast<unsigned>(c - 'a' + 10)
				: c >= 'A' && c <= 'F'                  ? static_cast<unsigned>(c - 'A' + 10)
														: 16u;
			if (digit == 16)
				break;
			if (digit >= base)
				return fail("an immediate with a digit outside its base");
			if (value > (~uint64_t(0) - digit) / base)
				return fail("an immediate beyond 64 bits");
			value = value * base + digit;
		}
		if (at == start)
			return fail("an immediate without digits");
		out.immediate = true;
		out.value = negative ? 0 - value : value;
		return true;
	}
	if (at >= text.size() || text[at] != '%')
		return fail("an operand that is no asm operand or immediate");
	++at;
	if (at < text.size() && text[at] == '[')
	{
		const size_t close = text.find(']', at);
		if (close == std::string::npos)
			return fail("an unterminated operand name");
		const std::string name = text.substr(at + 1, close - at - 1);
		at = close + 1;
		for (uint32_t i = 0; i < count_; ++i)
		{
			if (operands_[i].name != nullptr && name == operands_[i].name)
			{
				// A matching input names its output's register.
				out.index = static_cast<uint8_t>(operands_[i].tied != 0xff ? operands_[i].tied : i);
				return true;
			}
		}
		return fail("an operand name no operand has");
	}
	if (at >= text.size() || text[at] < '0' || text[at] > '9')
		return fail("a register or operand modifier");
	uint32_t index = 0;
	while (at < text.size() && text[at] >= '0' && text[at] <= '9')
		index = index * 10 + static_cast<uint32_t>(text[at++] - '0');
	if (index >= count_)
		return fail("an operand number beyond the operands");
	out.index = static_cast<uint8_t>(operands_[index].tied != 0xff ? operands_[index].tied : index);
	return true;
}

} // namespace

bool LiftAsmTemplate(const std::string& text, const AsmOperandInfo* operands, uint32_t count,
	std::vector<AsmInstruction>& out, std::string& error)
{
	// Instructions end at `;` or a new-line.
	if (count > 30)
	{
		error = "unsupported GNU asm template: too many operands";
		return false;
	}
	Lifter lifter(operands, count, out, error);
	size_t start = 0;
	for (size_t i = 0; i <= text.size(); ++i)
	{
		if (i == text.size() || text[i] == ';' || text[i] == '\n')
		{
			if (!lifter.line(text.substr(start, i - start)))
				return false;
			start = i + 1;
		}
	}
	return lifter.finish();
}

} // namespace sema
