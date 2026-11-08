#include "asm/registers.h"

#include <cstring>

namespace assembly {
namespace {

using asm_model::Register;
using asm_model::RegisterBank;

const char* const kGpr[16][4] = { { "al", "ax", "eax", "rax" }, { "cl", "cx", "ecx", "rcx" },
	{ "dl", "dx", "edx", "rdx" }, { "bl", "bx", "ebx", "rbx" }, { "spl", "sp", "esp", "rsp" },
	{ "bpl", "bp", "ebp", "rbp" }, { "sil", "si", "esi", "rsi" }, { "dil", "di", "edi", "rdi" },
	{ "r8b", "r8w", "r8d", "r8" }, { "r9b", "r9w", "r9d", "r9" }, { "r10b", "r10w", "r10d", "r10" },
	{ "r11b", "r11w", "r11d", "r11" }, { "r12b", "r12w", "r12d", "r12" }, { "r13b", "r13w", "r13d", "r13" },
	{ "r14b", "r14w", "r14d", "r14" }, { "r15b", "r15w", "r15d", "r15" } };
const char* const kHigh[4] = { "ah", "ch", "dh", "bh" };
const char* const kSegment[6] = { "es", "cs", "ss", "ds", "fs", "gs" };

struct Family
{
	const char* prefix;
	RegisterBank bank;
	unsigned count;
	unsigned bits;
};

const Family kFamilies[] = { { "xmm", RegisterBank::Vector, 32, 128 }, { "ymm", RegisterBank::Vector, 32, 256 },
	{ "zmm", RegisterBank::Vector, 32, 512 }, { "mm", RegisterBank::Mmx, 8, 64 }, { "k", RegisterBank::Mask, 8, 64 },
	{ "tmm", RegisterBank::Tile, 8, 8192 }, { "cr", RegisterBank::Control, 16, 64 },
	{ "dr", RegisterBank::Debug, 16, 64 } };

bool number(const char* text, unsigned& value)
{
	if (*text == '\0' || (*text == '0' && text[1] != '\0'))
		return false;
	value = 0;
	for (; *text != '\0'; ++text)
	{
		if (*text < '0' || *text > '9' || value >= 32)
			return false;
		value = value * 10 + static_cast<unsigned>(*text - '0');
	}
	return value < 32;
}

Register view(RegisterBank bank, unsigned number, unsigned bits, unsigned offset = 0)
{
	return Register{ bank, static_cast<uint8_t>(number), static_cast<uint16_t>(bits), static_cast<uint8_t>(offset) };
}

} // namespace

bool parse_register(const char* name, std::size_t size, Register& reg)
{
	if (size == 0 || size >= 9)
		return false;
	char text[9];
	for (std::size_t i = 0; i < size; ++i)
	{
		if (name[i] == '\0')
			return false;
		text[i] = name[i] >= 'A' && name[i] <= 'Z' ? static_cast<char>(name[i] + ('a' - 'A')) : name[i];
	}
	text[size] = '\0';
	for (unsigned i = 0; i < 16; ++i)
		for (unsigned width = 0; width < 4; ++width)
			if (std::strcmp(text, kGpr[i][width]) == 0)
			{
				reg = view(RegisterBank::Gpr, i, 8u << width);
				return true;
			}
	for (unsigned i = 0; i < 4; ++i)
		if (std::strcmp(text, kHigh[i]) == 0)
		{
			reg = view(RegisterBank::Gpr, i, 8, 8);
			return true;
		}
	for (unsigned i = 0; i < 6; ++i)
		if (std::strcmp(text, kSegment[i]) == 0)
		{
			reg = view(RegisterBank::Segment, i, 16);
			return true;
		}
	for (const Family& family : kFamilies)
	{
		const std::size_t prefix = std::strlen(family.prefix);
		unsigned n = 0;
		if (size > prefix && std::memcmp(text, family.prefix, prefix) == 0 && number(text + prefix, n) &&
			n < family.count)
		{
			reg = view(family.bank, n, family.bits);
			return true;
		}
	}
	if (text[0] == 'r' && text[1] >= '1' && text[1] <= '3')
	{
		unsigned bits = 64, n = 0;
		const char last = text[size - 1];
		if (last == 'b' || last == 'w' || last == 'd')
		{
			bits = last == 'b' ? 8 : last == 'w' ? 16 : 32;
			text[size - 1] = '\0';
		}
		if (number(text + 1, n) && n >= 16)
		{
			reg = view(RegisterBank::Gpr, n, bits);
			return true;
		}
	}
	if (std::strcmp(text, "st") == 0)
	{
		reg = view(RegisterBank::X87, 0, 80);
		return true;
	}
	if ((size == 3 && text[0] == 's' && text[1] == 't' && text[2] >= '0' && text[2] <= '7') ||
		(size == 5 && text[0] == 's' && text[1] == 't' && text[2] == '(' && text[3] >= '0' && text[3] <= '7' &&
			text[4] == ')'))
	{
		reg = view(RegisterBank::X87, static_cast<unsigned>(text[size == 3 ? 2 : 3] - '0'), 80);
		return true;
	}
	return false;
}

std::string register_name(const Register& reg)
{
	if (reg.bank == RegisterBank::Gpr && reg.number < 32)
	{
		if (reg.bits == 8 && reg.offset == 8 && reg.number < 4)
			return kHigh[reg.number];
		const unsigned width = reg.bits == 8 ? 0 : reg.bits == 16 ? 1 : reg.bits == 32 ? 2 : 3;
		if (reg.offset != 0 || (8u << width) != reg.bits)
			return std::string();
		if (reg.number < 16)
			return kGpr[reg.number][width];
		return "r" + std::to_string(reg.number) + (width == 0 ? "b" : width == 1 ? "w" : width == 2 ? "d" : "");
	}
	if (reg.offset != 0)
		return std::string();
	if (reg.bank == RegisterBank::Segment && reg.bits == 16 && reg.number < 6)
		return kSegment[reg.number];
	if (reg.bank == RegisterBank::X87 && reg.bits == 80 && reg.number < 8)
		return "st(" + std::to_string(reg.number) + ")";
	for (const Family& family : kFamilies)
		if (reg.bank == family.bank && reg.bits == family.bits && reg.number < family.count)
			return family.prefix + std::to_string(reg.number);
	return std::string();
}

} // namespace assembly
