#pragma once

// The x86-64 registers and condition codes of the machine IR, with their
// MIR spellings and the System V AMD64 ABI facts about them (psABI 3.2).
// The course solution builds on this header exactly as it is shipped.
//
// A register is a small number: 0-15 are the general-purpose registers in
// their x86 encoding order (the low three bits are the ModRM field, bit 3
// the REX bit), 16-31 are XMM0-XMM15.  Register sets are 32-bit masks of
// reg_bit().  How a backend reserves scratch registers or orders its
// allocation is its own design and is not here.

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace mir_model {

typedef std::uint8_t Reg;

enum : Reg
{
	RAX = 0,
	RCX,
	RDX,
	RBX,
	RSP,
	RBP,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	XMM0 = 16,
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
	XMM8,
	XMM9,
	XMM10,
	XMM11,
	XMM12,
	XMM13,
	XMM14,
	XMM15
};

const unsigned kGprCount = 16;
const unsigned kXmmCount = 16;
// No register: a stack location, an absent base or index.
const Reg kNoReg = 0xff;
// The x87 top of stack, where an f80 result is returned.
const Reg kSt0 = 0xfe;

inline bool is_gpr(Reg reg)
{
	return reg < kGprCount;
}
inline bool is_xmm(Reg reg)
{
	return reg >= XMM0 && reg < XMM0 + kXmmCount;
}
inline std::uint32_t reg_bit(Reg reg)
{
	return 1u << reg;
}

// The MIR spelling: "rax" ... "r15", "xmm0" ... "xmm15", "st0"; null for
// any other number.
inline const char* register_spelling(Reg reg)
{
	static const char* const kRegisters[kGprCount + kXmmCount] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi",
		"rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
		"xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
	return reg < kGprCount + kXmmCount ? kRegisters[reg] : reg == kSt0 ? "st0" : 0;
}

inline bool register_from_spelling(const char* text, std::size_t length, Reg* reg)
{
	for (unsigned r = 0; r < kGprCount + kXmmCount; ++r)
	{
		const char* name = register_spelling(static_cast<Reg>(r));
		if (std::strlen(name) == length && std::memcmp(name, text, length) == 0)
		{
			*reg = static_cast<Reg>(r);
			return true;
		}
	}
	if (length == 3 && std::memcmp(text, "st0", 3) == 0)
	{
		*reg = kSt0;
		return true;
	}
	return false;
}

// System V AMD64 ABI: integer arguments in RDI, RSI, RDX, RCX, R8, R9;
// floating arguments in XMM0-XMM7; RBX, RBP and R12-R15 survive a call;
// every XMM register is clobbered by one.
const Reg kArgGprs[6] = { RDI, RSI, RDX, RCX, R8, R9 };
const unsigned kArgXmmCount = 8;
const std::uint32_t kCallerSavedGprs = (1u << RAX) | (1u << RCX) | (1u << RDX) | (1u << RSI) | (1u << RDI) |
	(1u << R8) | (1u << R9) | (1u << R10) | (1u << R11);
const std::uint32_t kCalleeSavedGprs = (1u << RBX) | (1u << R12) | (1u << R13) | (1u << R14) | (1u << R15);
const std::uint32_t kAllXmms = 0xffff0000u;

// x86 condition codes: the low nibble of Jcc, SETcc and CMOVcc.  A code and
// its inverse differ in bit 0.
enum Cond : std::uint8_t
{
	CC_O = 0,
	CC_NO,
	CC_B,
	CC_AE,
	CC_E,
	CC_NE,
	CC_BE,
	CC_A,
	CC_S,
	CC_NS,
	CC_P,
	CC_NP,
	CC_L,
	CC_GE,
	CC_LE,
	CC_G
};

const unsigned kCondCount = CC_G + 1;

inline Cond invert(Cond cond)
{
	return static_cast<Cond>(cond ^ 1);
}

// The suffix of `j`, `set` and `cmov`: "o", "no", "b", ...
inline const char* spelling(Cond cond)
{
	static const char* const kConds[kCondCount] = { "o", "no", "b", "ae", "e", "ne", "be", "a", "s", "ns", "p", "np",
		"l", "ge", "le", "g" };
	return kConds[cond];
}

inline bool from_spelling(const char* text, std::size_t length, Cond* cond)
{
	for (unsigned c = 0; c < kCondCount; ++c)
	{
		const char* name = spelling(static_cast<Cond>(c));
		if (std::strlen(name) == length && std::memcmp(name, text, length) == 0)
		{
			*cond = static_cast<Cond>(c);
			return true;
		}
	}
	return false;
}

} // namespace mir_model
