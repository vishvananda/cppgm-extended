#include "sema/asm_statements.h"

#include <cstring>
#include <unordered_map>

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
	{ "xchg", AsmOperation::Exchange, 2, true }, { "bswap", AsmOperation::ByteSwap, 1, true },
	{ "bts", AsmOperation::BitSet, 2, true }, { "btr", AsmOperation::BitReset, 2, true },
	{ "btc", AsmOperation::BitComplement, 2, true }, { "pand", AsmOperation::PackedAnd, 2, false } };

// A parsed instruction operand: an asm operand, or an immediate.
struct Operand
{
	bool immediate;
	uint8_t index;
	uint8_t bytes;
	uint64_t value;
};

// The size of the register GCC prints for an operand bound to one: its
// type's (the low register of a 16-byte pair).
uint32_t RegisterBytes(const AsmOperandInfo& operand, const Operand& parsed)
{
	return parsed.bytes != 0 ? parsed.bytes : operand.bytes == 16 && operand.fixed != 'x' ? 8 : operand.bytes;
}

uint32_t ByteMask(uint32_t bytes)
{
	return bytes >= 32 ? ~uint32_t(0) : (uint32_t(1) << bytes) - 1;
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
	{
		for (uint32_t i = 0; i < count_; ++i)
			initialized_[i] = !operands_[i].output || operands_[i].read ? ByteMask(operands_[i].bytes) : 0;
	}

	bool line(const std::string& text);

	bool finish()
	{
		if (lock_ || rep_)
			return fail("a prefix without its instruction");
		for (uint32_t i = 0; i < count_; ++i)
		{
			if (operands_[i].binding != AsmOperandClass::Flag)
				continue;
			if (!capture(operands_[i].flag))
				return false;
			AsmInstruction instruction = AsmInstruction();
			instruction.operation = set_operation(operands_[i].flag);
			instruction.destination = static_cast<uint8_t>(i);
			instruction.bytes = static_cast<uint8_t>(operands_[i].bytes);
			out_.push_back(instruction);
			written(i, operands_[i].bytes);
		}
		for (uint32_t i = 0; i < count_; ++i)
			if (operands_[i].output && operands_[i].binding == AsmOperandClass::Register &&
				(initialized_[i] & ByteMask(operands_[i].bytes)) != ByteMask(operands_[i].bytes))
				return fail("a register output with unwritten bytes");
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
	bool control(const std::string& mnemonic, const std::string& text, size_t at);
	bool transfer(const std::string& mnemonic, const std::string& text, size_t at);
	bool cpu_query(const std::string& text, size_t at);
	bool set_flag(AsmFlag flag, const std::string& text, size_t at);
	bool capture(AsmFlag flag)
	{
		const bool carry = flag == AsmFlag::Carry || flag == AsmFlag::NotCarry;
		const uint32_t producer = carry ? carry_source_ : zero_source_;
		if (flag == AsmFlag::None || producer == ~uint32_t(0))
			return fail("a condition flag without a supported producer");
		out_[producer].flags |= carry ? kAsmCaptureCarry : kAsmCaptureZero;
		return true;
	}
	static AsmOperation set_operation(AsmFlag flag)
	{
		return flag == AsmFlag::Carry   ? AsmOperation::SetCarry
			: flag == AsmFlag::NotCarry ? AsmOperation::SetNotCarry
			: flag == AsmFlag::Zero     ? AsmOperation::SetZero
										: AsmOperation::SetNotZero;
	}
	bool read(const Operand& operand, uint32_t bytes)
	{
		return operand.immediate || operands_[operand.index].binding == AsmOperandClass::Immediate ||
			(initialized_[operand.index] & ByteMask(bytes)) == ByteMask(bytes) ||
			fail("a read of an unwritten asm register");
	}
	void written(uint32_t index, uint32_t bytes)
	{
		if (operands_[index].binding == AsmOperandClass::Register && bytes == 4 && operands_[index].bytes >= 8)
			bytes = 8;
		initialized_[index] |= ByteMask(bytes);
	}

	const AsmOperandInfo* operands_;
	uint32_t count_;
	std::vector<AsmInstruction>& out_;
	std::string& error_;
	bool lock_ = false;
	bool rep_ = false;
	uint32_t zero_source_ = ~uint32_t(0);
	uint32_t carry_source_ = ~uint32_t(0);
	uint32_t initialized_[30] = {};
	std::unordered_map<uint32_t, uint32_t> labels_;
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
	while (at < text.size() && IsSpace(text[at]))
		++at;
	if (at < text.size() && text[at] >= '0' && text[at] <= '9')
	{
		uint32_t label = 0;
		while (at < text.size() && text[at] >= '0' && text[at] <= '9')
		{
			const uint32_t digit = static_cast<uint32_t>(text[at++] - '0');
			if (label > (~uint32_t(0) - digit) / 10)
				return fail("a numeric label beyond 32 bits");
			label = label * 10 + digit;
		}
		if (at >= text.size() || text[at++] != ':' || lock_ || rep_)
			return fail("an invalid numeric label");
		labels_[label] = static_cast<uint32_t>(out_.size());
	}
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
	if (mnemonic == "cpuid")
		return cpu_query(text, at);
	if (mnemonic == ".align" || mnemonic == ".p2align" || mnemonic == "jne" || mnemonic == "jnz" || mnemonic == "je" ||
		mnemonic == "jz")
		return control(mnemonic, text, at);
	if (mnemonic == "setc" || mnemonic == "setb" || mnemonic == "setnae")
		return set_flag(AsmFlag::Carry, text, at);
	if (mnemonic == "setnc" || mnemonic == "setae" || mnemonic == "setnb")
		return set_flag(AsmFlag::NotCarry, text, at);
	if (mnemonic == "setz" || mnemonic == "sete")
		return set_flag(AsmFlag::Zero, text, at);
	if (mnemonic == "setnz" || mnemonic == "setne")
		return set_flag(AsmFlag::NotZero, text, at);
	if (mnemonic.compare(0, 3, "mov") == 0 || mnemonic.compare(0, 7, "cmpxchg") == 0)
		return transfer(mnemonic, text, at);
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
	if (destination.immediate || !operands_[destination.index].output)
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
	const uint32_t register_bytes = !memory ? RegisterBytes(target, destination)
		: source_register                   ? RegisterBytes(operands_[source->index], *source)
											: 0;
	if (bytes == 0 && register_bytes == 0)
		return fail("an instruction of no size (no suffix and no register operand)");
	if (bytes == 0)
		bytes = register_bytes;
	const bool packed = instruction.operation == AsmOperation::PackedAnd;
	if (packed &&
		(instruction.locked || memory || target.fixed != 'x' || !source_register ||
			operands_[source->index].fixed != 'x'))
		return fail("a packed AND without two unlocked XMM register operands");
	if ((register_bytes != 0 && bytes != register_bytes) ||
		(packed ? bytes != 16 : (bytes != 1 && bytes != 2 && bytes != 4 && bytes != 8)))
		return fail("an operand size that disagrees with its instruction");
	if (source_register && RegisterBytes(operands_[source->index], *source) != bytes)
		return fail("a source operand that is no register of the instruction's size");
	if (memory && bytes > target.bytes)
		return fail("an operation wider than its memory operand");
	instruction.bytes = static_cast<uint8_t>(bytes);
	if (!read(destination, bytes) || (source_register && !read(*source, bytes)))
		return false;
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
	const bool bit = instruction.operation == AsmOperation::BitSet || instruction.operation == AsmOperation::BitReset ||
		instruction.operation == AsmOperation::BitComplement;
	if (bit && bytes == 1)
		return fail("a bit-test instruction of byte size");
	if (instruction.operation == AsmOperation::ByteSwap && (memory || (bytes != 4 && bytes != 8)))
		return fail("a byte swap of no 32- or 64-bit register");
	if (instruction.locked && !memory)
		return fail("a locked instruction of no memory destination");
	written(destination.index, bytes);
	if (bit)
		carry_source_ = static_cast<uint32_t>(out_.size());
	else if (instruction.operation != AsmOperation::Not && instruction.operation != AsmOperation::Exchange &&
		instruction.operation != AsmOperation::ByteSwap && !packed)
	{
		zero_source_ = static_cast<uint32_t>(out_.size());
		if (mnemonic.compare(0, 3, "inc") != 0 && mnemonic.compare(0, 3, "dec") != 0)
			carry_source_ = static_cast<uint32_t>(out_.size());
	}
	out_.push_back(instruction);
	return true;
}

bool Lifter::cpu_query(const std::string& text, size_t at)
{
	if (lock_ || !rest_empty(text, at))
		return fail("a prefixed CPUID or one with explicit operands");
	const char registers[] = { 'a', 'b', 'c', 'd' };
	AsmInstruction instruction = AsmInstruction();
	instruction.operation = AsmOperation::CpuId;
	instruction.bytes = 4;
	for (uint32_t reg = 0; reg < 4; ++reg)
	{
		uint32_t found = count_;
		for (uint32_t i = 0; i < count_; ++i)
		{
			if (operands_[i].fixed != registers[reg] || !operands_[i].output)
				continue;
			if (found != count_)
				return fail("ambiguous CPUID register outputs");
			found = i;
		}
		if (found == count_ || operands_[found].binding != AsmOperandClass::Register ||
			(operands_[found].bytes != 4 && operands_[found].bytes != 8))
			return fail("CPUID without four 32- or 64-bit fixed register outputs");
		Operand input = Operand();
		input.index = static_cast<uint8_t>(found);
		if ((reg == 0 || reg == 2) && !read(input, 4))
			return false;
		written(found, 4);
		instruction.immediate |= static_cast<uint64_t>(found) << (reg * 8);
	}
	// CPUID leaves condition flags unchanged.
	out_.push_back(instruction);
	return true;
}

bool Lifter::control(const std::string& mnemonic, const std::string& text, size_t at)
{
	if (lock_ || rep_)
		return fail("a prefixed branch or alignment directive");
	while (at < text.size() && IsSpace(text[at]))
		++at;
	const size_t start = at;
	uint32_t number = 0;
	while (at < text.size() && text[at] >= '0' && text[at] <= '9')
	{
		const uint32_t digit = static_cast<uint32_t>(text[at++] - '0');
		if (number > (~uint32_t(0) - digit) / 10)
			return fail("a branch or alignment operand beyond 32 bits");
		number = number * 10 + digit;
	}
	if (at == start)
		return fail("a branch or alignment operand without digits");
	if (mnemonic == ".align" || mnemonic == ".p2align")
	{
		if ((mnemonic == ".align" && (number == 0 || number > 4096 || (number & (number - 1)) != 0)) ||
			(mnemonic == ".p2align" && number > 12))
			return fail("an unsupported code alignment");
		// Alignment directs assembly placement, not program behavior. The
		// native backend owns placement of these lifted operations.
		return rest_empty(text, at);
	}
	const auto found = labels_.find(number);
	if (at >= text.size() || text[at++] != 'b' || found == labels_.end())
		return fail("a branch without a backward label or supported condition flags");
	if (!rest_empty(text, at) || !capture(AsmFlag::Zero))
		return false;
	AsmInstruction instruction = AsmInstruction();
	instruction.operation = mnemonic == "je" || mnemonic == "jz" ? AsmOperation::JumpZero : AsmOperation::JumpNotZero;
	instruction.immediate = found->second;
	out_.push_back(instruction);
	return true;
}

bool Lifter::transfer(const std::string& mnemonic, const std::string& text, size_t at)
{
	const bool exchange = mnemonic.compare(0, 7, "cmpxchg") == 0;
	uint32_t bytes = 0, source_bytes = 0;
	if (exchange)
	{
		if (mnemonic != "cmpxchg" && (mnemonic.size() != 8 || SuffixBytes(mnemonic[7]) == 0))
			return fail("an unsupported compare-exchange mnemonic");
		if (mnemonic.size() == 8)
			bytes = source_bytes = SuffixBytes(mnemonic[7]);
	}
	else if (mnemonic.size() == 6 && mnemonic.compare(0, 4, "movz") == 0)
	{
		source_bytes = SuffixBytes(mnemonic[4]);
		bytes = SuffixBytes(mnemonic[5]);
		if (source_bytes == 0 || source_bytes >= bytes)
			return fail("an invalid zero-extending move");
	}
	else if (mnemonic.size() == 4)
		bytes = source_bytes = SuffixBytes(mnemonic[3]);
	else if (mnemonic != "mov")
		return fail("an unsupported move or compare-exchange mnemonic");
	Operand source, target;
	if (!operand(text, at, source))
		return false;
	while (at < text.size() && IsSpace(text[at]))
		++at;
	if (at >= text.size() || text[at++] != ',' || !operand(text, at, target) || !rest_empty(text, at))
		return fail("a move or compare-exchange without two operands");
	if (target.immediate || !operands_[target.index].output ||
		operands_[target.index].binding == AsmOperandClass::Immediate)
		return fail("a move or compare-exchange without a writable destination");
	const bool memory = operands_[target.index].binding == AsmOperandClass::Memory;
	const bool source_memory = !source.immediate && operands_[source.index].binding == AsmOperandClass::Memory;
	if (bytes == 0)
	{
		if (!memory)
			bytes = RegisterBytes(operands_[target.index], target);
		else if (!source.immediate && !source_memory)
			bytes = RegisterBytes(operands_[source.index], source);
		source_bytes = bytes;
	}
	if ((bytes != 1 && bytes != 2 && bytes != 4 && bytes != 8) || source_bytes == 0 ||
		bytes > operands_[target.index].bytes || (!memory && RegisterBytes(operands_[target.index], target) != bytes) ||
		(memory && source_memory) || (source_memory && source_bytes > operands_[source.index].bytes))
		return fail("an invalid move or compare-exchange size or memory pair");
	if (!source.immediate && operands_[source.index].binding == AsmOperandClass::Register &&
		(source_bytes > operands_[source.index].bytes ||
			RegisterBytes(operands_[source.index], source) != source_bytes))
		return fail("a source register of another size than its instruction");
	if (!read(source, source_bytes))
		return false;
	AsmInstruction instruction = AsmInstruction();
	instruction.operation = exchange ? AsmOperation::CompareExchange : AsmOperation::Move;
	instruction.destination = target.index;
	instruction.source = source.immediate || operands_[source.index].binding == AsmOperandClass::Immediate
		? kAsmImmediateSource
		: source.index;
	instruction.immediate = source.immediate ? source.value : operands_[source.index].value;
	instruction.bytes = static_cast<uint8_t>(bytes);
	instruction.source_bytes = static_cast<uint8_t>(source_bytes);
	instruction.locked = lock_;
	lock_ = false;
	if (instruction.source == kAsmImmediateSource && !ImmediateFits(instruction.immediate, bytes))
		return fail("an immediate beyond its instruction's field");
	if (exchange)
	{
		uint32_t accumulator = count_;
		for (uint32_t i = 0; i < count_; ++i)
			if (operands_[i].fixed == 'a' && operands_[i].binding == AsmOperandClass::Register && operands_[i].output)
			{
				if (accumulator != count_)
					return fail("a compare-exchange with ambiguous accumulator bindings");
				accumulator = i;
			}
		if (!instruction.locked || !memory || source_memory || instruction.source == kAsmImmediateSource ||
			accumulator == count_ || operands_[accumulator].bytes != bytes || !read(target, bytes))
			return fail("an unsupported compare-exchange binding");
		Operand expected = Operand();
		expected.index = static_cast<uint8_t>(accumulator);
		if (!read(expected, bytes))
			return false;
		instruction.accumulator = expected.index;
		written(accumulator, bytes);
		zero_source_ = static_cast<uint32_t>(out_.size());
		carry_source_ = static_cast<uint32_t>(out_.size());
	}
	else if (instruction.locked)
		return fail("a locked move");
	written(target.index, bytes);
	out_.push_back(instruction);
	return true;
}

bool Lifter::set_flag(AsmFlag flag, const std::string& text, size_t at)
{
	if (lock_ || rep_ || !capture(flag))
		return fail("an invalid flag-setting instruction");
	Operand destination;
	if (!operand(text, at, destination) || !rest_empty(text, at))
		return false;
	if (destination.immediate || !operands_[destination.index].output ||
		(operands_[destination.index].binding == AsmOperandClass::Register &&
			RegisterBytes(operands_[destination.index], destination) != 1))
		return fail("a setcc without a writable byte operand");
	AsmInstruction instruction = AsmInstruction();
	instruction.operation = set_operation(flag);
	instruction.destination = destination.index;
	instruction.bytes = 1;
	written(destination.index, 1);
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
	if (at < text.size() && text[at] == '%')
	{
		++at;
		const size_t start = at;
		while (at < text.size() && ((text[at] >= 'a' && text[at] <= 'z') || (text[at] >= '0' && text[at] <= '9')))
			++at;
		const std::string name = text.substr(start, at - start);
		char fixed = 0;
		if (name == "al" || name == "ax" || name == "eax" || name == "rax")
			fixed = 'a';
		else if (name == "bl" || name == "bx" || name == "ebx" || name == "rbx")
			fixed = 'b';
		else if (name == "cl" || name == "cx" || name == "ecx" || name == "rcx")
			fixed = 'c';
		else if (name == "dl" || name == "dx" || name == "edx" || name == "rdx")
			fixed = 'd';
		if (fixed == 0)
			return fail("an unbound physical register");
		out.bytes = static_cast<uint8_t>(name.size() == 2 ? (name[1] == 'l' ? 1 : 2) : name[0] == 'e' ? 4 : 8);
		uint32_t found = count_;
		for (uint32_t i = 0; i < count_; ++i)
			if (operands_[i].fixed == fixed && operands_[i].binding == AsmOperandClass::Register)
			{
				if (found != count_)
					return fail("an ambiguously bound physical register");
				found = i;
			}
		if (found == count_ || out.bytes > operands_[found].bytes)
			return fail("a physical register without a matching constraint");
		out.index = static_cast<uint8_t>(found);
		return true;
	}
	if (at < text.size() && (text[at] == 'b' || text[at] == 'w' || text[at] == 'k' || text[at] == 'q'))
	{
		out.bytes = static_cast<uint8_t>(text[at] == 'b' ? 1 : text[at] == 'w' ? 2 : text[at] == 'k' ? 4 : 8);
		++at;
	}
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
				if (operands_[i].binding == AsmOperandClass::Flag)
					return fail("a flag output referenced as an asm operand");
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
	if (operands_[index].binding == AsmOperandClass::Flag)
		return fail("a flag output referenced as an asm operand");
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
