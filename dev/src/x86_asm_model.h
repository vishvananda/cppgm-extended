#pragma once

// Optional PA36's initial x86-64 encoding vocabulary and instruction facts.
// These are architectural forms, independent of a frontend, LowIR containers,
// register allocator or object writer. See pa36/README.md and assembly.md.
// The course solution consumes this header as shipped; students may use these
// facts with their own parser, form selection and encoding design.

#include <cstddef>
#include <cstdint>

namespace x86_asm_model {

struct Branch
{
	const char* name;
	uint8_t condition;
};

const Branch branches[] = { { "jo", 0 }, { "jno", 1 }, { "jb", 2 }, { "jc", 2 }, { "jnae", 2 }, { "jae", 3 },
	{ "jnb", 3 }, { "jnc", 3 }, { "je", 4 }, { "jz", 4 }, { "jne", 5 }, { "jnz", 5 }, { "jbe", 6 }, { "jna", 6 },
	{ "ja", 7 }, { "jnbe", 7 }, { "js", 8 }, { "jns", 9 }, { "jp", 10 }, { "jpe", 10 }, { "jnp", 11 }, { "jpo", 11 },
	{ "jl", 12 }, { "jnge", 12 }, { "jge", 13 }, { "jnl", 13 }, { "jle", 14 }, { "jng", 14 }, { "jg", 15 },
	{ "jnle", 15 } };

enum class Encoding : std::uint8_t
{
	Zero,
	Move,
	Lea,
	Exchange,
	Alu,
	VectorRM,
	VectorMR,
	Unary,
	Shift,
	Test,
	Multiply,
	Atomic,
	String,
	Control,
	ConditionMove,
	ConditionSet,
	BitScan,
	BitSwap,
	BitTest,
	DoubleShift,
	WideAtomic,
	ExtendMove
};

struct Form
{
	const char* mnemonic;
	Encoding encoding;
	std::uint32_t opcode;
	std::uint8_t prefix;
	std::uint16_t bits;
	bool memory_only;
	std::uint16_t source_bits; // mixed-width moves: zero derives the source view
};

struct GnuIntegerRange
{
	char constraint;
	std::int64_t low;
	std::int64_t high;
};

// Target constraint facts; repeated letters denote a union of ranges.
inline const GnuIntegerRange* gnu_integer_ranges(std::size_t& count)
{
	static const GnuIntegerRange kRanges[] = { { 'I', 0, 31 }, { 'J', 0, 63 }, { 'K', -128, 127 }, { 'L', 255, 255 },
		{ 'L', 65535, 65535 }, { 'M', 0, 3 }, { 'N', 0, 255 }, { 'e', INT32_MIN, INT32_MAX }, { 'Z', 0, UINT32_MAX } };
	count = sizeof kRanges / sizeof kRanges[0];
	return kRanges;
}

// Opcode bytes are most significant first. ALU forms carry their group digit.
// bits=0 uses the operand width (the destination for mixed-width moves).
// source_bits=0 derives a mixed-width source's view. RM/MR distinguish register
// destination/source; mandatory prefix and memory-only restrictions are facts.
inline const Form* forms(std::size_t& count)
{
	using E = Encoding;
	static const Form kForms[] = {
		{ "ret", E::Control, 0xc3, 0, 64, false, 0 },
		{ "push", E::Control, 0x50, 0, 0, false, 0 },
		{ "pop", E::Control, 0x58, 0, 0, false, 0 },
		{ "cmovo", E::ConditionMove, 0x0f40, 0, 0, false, 0 },
		{ "seto", E::ConditionSet, 0x0f90, 0, 8, false, 0 },
		{ "cmovno", E::ConditionMove, 0x0f41, 0, 0, false, 0 },
		{ "setno", E::ConditionSet, 0x0f91, 0, 8, false, 0 },
		{ "cmovb", E::ConditionMove, 0x0f42, 0, 0, false, 0 },
		{ "setb", E::ConditionSet, 0x0f92, 0, 8, false, 0 },
		{ "cmovc", E::ConditionMove, 0x0f42, 0, 0, false, 0 },
		{ "setc", E::ConditionSet, 0x0f92, 0, 8, false, 0 },
		{ "cmovnae", E::ConditionMove, 0x0f42, 0, 0, false, 0 },
		{ "setnae", E::ConditionSet, 0x0f92, 0, 8, false, 0 },
		{ "cmovae", E::ConditionMove, 0x0f43, 0, 0, false, 0 },
		{ "setae", E::ConditionSet, 0x0f93, 0, 8, false, 0 },
		{ "cmovnb", E::ConditionMove, 0x0f43, 0, 0, false, 0 },
		{ "setnb", E::ConditionSet, 0x0f93, 0, 8, false, 0 },
		{ "cmovnc", E::ConditionMove, 0x0f43, 0, 0, false, 0 },
		{ "setnc", E::ConditionSet, 0x0f93, 0, 8, false, 0 },
		{ "cmove", E::ConditionMove, 0x0f44, 0, 0, false, 0 },
		{ "sete", E::ConditionSet, 0x0f94, 0, 8, false, 0 },
		{ "cmovz", E::ConditionMove, 0x0f44, 0, 0, false, 0 },
		{ "setz", E::ConditionSet, 0x0f94, 0, 8, false, 0 },
		{ "cmovne", E::ConditionMove, 0x0f45, 0, 0, false, 0 },
		{ "setne", E::ConditionSet, 0x0f95, 0, 8, false, 0 },
		{ "cmovnz", E::ConditionMove, 0x0f45, 0, 0, false, 0 },
		{ "setnz", E::ConditionSet, 0x0f95, 0, 8, false, 0 },
		{ "cmovbe", E::ConditionMove, 0x0f46, 0, 0, false, 0 },
		{ "setbe", E::ConditionSet, 0x0f96, 0, 8, false, 0 },
		{ "cmovna", E::ConditionMove, 0x0f46, 0, 0, false, 0 },
		{ "setna", E::ConditionSet, 0x0f96, 0, 8, false, 0 },
		{ "cmova", E::ConditionMove, 0x0f47, 0, 0, false, 0 },
		{ "seta", E::ConditionSet, 0x0f97, 0, 8, false, 0 },
		{ "cmovnbe", E::ConditionMove, 0x0f47, 0, 0, false, 0 },
		{ "setnbe", E::ConditionSet, 0x0f97, 0, 8, false, 0 },
		{ "cmovs", E::ConditionMove, 0x0f48, 0, 0, false, 0 },
		{ "sets", E::ConditionSet, 0x0f98, 0, 8, false, 0 },
		{ "cmovns", E::ConditionMove, 0x0f49, 0, 0, false, 0 },
		{ "setns", E::ConditionSet, 0x0f99, 0, 8, false, 0 },
		{ "cmovp", E::ConditionMove, 0x0f4a, 0, 0, false, 0 },
		{ "setp", E::ConditionSet, 0x0f9a, 0, 8, false, 0 },
		{ "cmovpe", E::ConditionMove, 0x0f4a, 0, 0, false, 0 },
		{ "setpe", E::ConditionSet, 0x0f9a, 0, 8, false, 0 },
		{ "cmovnp", E::ConditionMove, 0x0f4b, 0, 0, false, 0 },
		{ "setnp", E::ConditionSet, 0x0f9b, 0, 8, false, 0 },
		{ "cmovpo", E::ConditionMove, 0x0f4b, 0, 0, false, 0 },
		{ "setpo", E::ConditionSet, 0x0f9b, 0, 8, false, 0 },
		{ "cmovl", E::ConditionMove, 0x0f4c, 0, 0, false, 0 },
		{ "setl", E::ConditionSet, 0x0f9c, 0, 8, false, 0 },
		{ "cmovnge", E::ConditionMove, 0x0f4c, 0, 0, false, 0 },
		{ "setnge", E::ConditionSet, 0x0f9c, 0, 8, false, 0 },
		{ "cmovge", E::ConditionMove, 0x0f4d, 0, 0, false, 0 },
		{ "setge", E::ConditionSet, 0x0f9d, 0, 8, false, 0 },
		{ "cmovnl", E::ConditionMove, 0x0f4d, 0, 0, false, 0 },
		{ "setnl", E::ConditionSet, 0x0f9d, 0, 8, false, 0 },
		{ "cmovle", E::ConditionMove, 0x0f4e, 0, 0, false, 0 },
		{ "setle", E::ConditionSet, 0x0f9e, 0, 8, false, 0 },
		{ "cmovng", E::ConditionMove, 0x0f4e, 0, 0, false, 0 },
		{ "setng", E::ConditionSet, 0x0f9e, 0, 8, false, 0 },
		{ "cmovg", E::ConditionMove, 0x0f4f, 0, 0, false, 0 },
		{ "setg", E::ConditionSet, 0x0f9f, 0, 8, false, 0 },
		{ "cmovnle", E::ConditionMove, 0x0f4f, 0, 0, false, 0 },
		{ "setnle", E::ConditionSet, 0x0f9f, 0, 8, false, 0 },
		{ "bsf", E::BitScan, 0x0fbc, 0x00, 0, false, 0 },
		{ "bsr", E::BitScan, 0x0fbd, 0x00, 0, false, 0 },
		{ "popcnt", E::BitScan, 0x0fb8, 0xf3, 0, false, 0 },
		{ "tzcnt", E::BitScan, 0x0fbc, 0xf3, 0, false, 0 },
		{ "lzcnt", E::BitScan, 0x0fbd, 0xf3, 0, false, 0 },
		{ "bswap", E::BitSwap, 0x0fc8, 0, 0, false, 0 },
		{ "bt", E::BitTest, 4, 0, 0, false, 0 },
		{ "bts", E::BitTest, 5, 0, 0, false, 0 },
		{ "btr", E::BitTest, 6, 0, 0, false, 0 },
		{ "btc", E::BitTest, 7, 0, 0, false, 0 },
		{ "shld", E::DoubleShift, 0x0fa4, 0, 0, false, 0 },
		{ "shrd", E::DoubleShift, 0x0fac, 0, 0, false, 0 },
		{ "cmpxchg8b", E::WideAtomic, 0x0fc7, 0, 64, true, 0 },
		{ "cmpxchg16b", E::WideAtomic, 0x0fc7, 0, 128, true, 0 },
		{ "movsx", E::ExtendMove, 0x0fbe, 0, 0, false, 0 },
		{ "movzx", E::ExtendMove, 0x0fb6, 0, 0, false, 0 },
		{ "movsxd", E::ExtendMove, 0x0063, 0, 64, false, 32 },
		{ "movsbw", E::ExtendMove, 0x0fbe, 0, 16, false, 8 },
		{ "movsbl", E::ExtendMove, 0x0fbe, 0, 32, false, 8 },
		{ "movsbq", E::ExtendMove, 0x0fbe, 0, 64, false, 8 },
		{ "movswl", E::ExtendMove, 0x0fbf, 0, 32, false, 16 },
		{ "movswq", E::ExtendMove, 0x0fbf, 0, 64, false, 16 },
		{ "movslq", E::ExtendMove, 0x0063, 0, 64, false, 32 },
		{ "movzbw", E::ExtendMove, 0x0fb6, 0, 16, false, 8 },
		{ "movzbl", E::ExtendMove, 0x0fb6, 0, 32, false, 8 },
		{ "movzbq", E::ExtendMove, 0x0fb6, 0, 64, false, 8 },
		{ "movzwl", E::ExtendMove, 0x0fb7, 0, 32, false, 16 },
		{ "movzwq", E::ExtendMove, 0x0fb7, 0, 64, false, 16 },
		{ "mov", E::Move, 0, 0, 0, false, 0 },
		{ "movabs", E::Move, 0, 0, 64, false, 0 },
		{ "lea", E::Lea, 0x8d, 0, 0, true, 0 },
		{ "xchg", E::Exchange, 0x87, 0, 0, false, 0 },
		{ "add", E::Alu, 0, 0, 0, false, 0 },
		{ "or", E::Alu, 1, 0, 0, false, 0 },
		{ "adc", E::Alu, 2, 0, 0, false, 0 },
		{ "sbb", E::Alu, 3, 0, 0, false, 0 },
		{ "and", E::Alu, 4, 0, 0, false, 0 },
		{ "sub", E::Alu, 5, 0, 0, false, 0 },
		{ "xor", E::Alu, 6, 0, 0, false, 0 },
		{ "cmp", E::Alu, 7, 0, 0, false, 0 },
		{ "inc", E::Unary, 0, 0, 0, false, 0 },
		{ "dec", E::Unary, 1, 0, 0, false, 0 },
		{ "not", E::Unary, 2, 0, 0, false, 0 },
		{ "neg", E::Unary, 3, 0, 0, false, 0 },
		{ "mul", E::Unary, 4, 0, 0, false, 0 },
		{ "div", E::Unary, 6, 0, 0, false, 0 },
		{ "idiv", E::Unary, 7, 0, 0, false, 0 },
		{ "rol", E::Shift, 0, 0, 0, false, 0 },
		{ "ror", E::Shift, 1, 0, 0, false, 0 },
		{ "rcl", E::Shift, 2, 0, 0, false, 0 },
		{ "rcr", E::Shift, 3, 0, 0, false, 0 },
		{ "shl", E::Shift, 4, 0, 0, false, 0 },
		{ "sal", E::Shift, 4, 0, 0, false, 0 },
		{ "shr", E::Shift, 5, 0, 0, false, 0 },
		{ "sar", E::Shift, 7, 0, 0, false, 0 },
		{ "test", E::Test, 0, 0, 0, false, 0 },
		{ "imul", E::Multiply, 0, 0, 0, false, 0 },
		{ "xadd", E::Atomic, 0x0fc0, 0, 0, false, 0 },
		{ "cmpxchg", E::Atomic, 0x0fb0, 0, 0, false, 0 },
		{ "movsb", E::String, 0xa4, 0, 0, false, 0 },
		{ "movsw", E::String, 0x66a5, 0, 0, false, 0 },
		{ "movsl", E::String, 0xa5, 0, 0, false, 0 },
		{ "movsq", E::String, 0x48a5, 0, 0, false, 0 },
		{ "stosb", E::String, 0xaa, 0, 0, false, 0 },
		{ "stosw", E::String, 0x66ab, 0, 0, false, 0 },
		{ "stosl", E::String, 0xab, 0, 0, false, 0 },
		{ "stosq", E::String, 0x48ab, 0, 0, false, 0 },
		{ "cmpsb", E::String, 0xa6, 0, 0, false, 0 },
		{ "cmpsw", E::String, 0x66a7, 0, 0, false, 0 },
		{ "cmpsl", E::String, 0xa7, 0, 0, false, 0 },
		{ "cmpsq", E::String, 0x48a7, 0, 0, false, 0 },
		{ "scasb", E::String, 0xae, 0, 0, false, 0 },
		{ "scasw", E::String, 0x66af, 0, 0, false, 0 },
		{ "scasl", E::String, 0xaf, 0, 0, false, 0 },
		{ "scasq", E::String, 0x48af, 0, 0, false, 0 },
		{ "lodsb", E::String, 0xac, 0, 0, false, 0 },
		{ "lodsw", E::String, 0x66ad, 0, 0, false, 0 },
		{ "lodsl", E::String, 0xad, 0, 0, false, 0 },
		{ "lodsq", E::String, 0x48ad, 0, 0, false, 0 },
		{ "nop", E::Zero, 0x90, 0, 0, false, 0 },
		{ "pause", E::Zero, 0xf390, 0, 0, false, 0 },
		{ "cpuid", E::Zero, 0x0fa2, 0, 0, false, 0 },
		{ "xgetbv", E::Zero, 0x0f01d0, 0, 0, false, 0 },
		{ "rdtsc", E::Zero, 0x0f31, 0, 0, false, 0 },
		{ "rdtscp", E::Zero, 0x0f01f9, 0, 0, false, 0 },
		{ "lfence", E::Zero, 0x0faee8, 0, 0, false, 0 },
		{ "mfence", E::Zero, 0x0faef0, 0, 0, false, 0 },
		{ "sfence", E::Zero, 0x0faef8, 0, 0, false, 0 },
		{ "cld", E::Zero, 0xfc, 0, 0, false, 0 },
		{ "std", E::Zero, 0xfd, 0, 0, false, 0 },
		{ "clc", E::Zero, 0xf8, 0, 0, false, 0 },
		{ "stc", E::Zero, 0xf9, 0, 0, false, 0 },
		{ "cmc", E::Zero, 0xf5, 0, 0, false, 0 },
		{ "ud2", E::Zero, 0x0f0b, 0, 0, false, 0 },
		{ "emms", E::Zero, 0x0f77, 0, 0, false, 0 },
		{ "movaps", E::VectorRM, 0x0f28, 0, 128, false, 0 },
		{ "movups", E::VectorRM, 0x0f10, 0, 128, false, 0 },
		{ "movapd", E::VectorRM, 0x0f28, 0x66, 128, false, 0 },
		{ "movupd", E::VectorRM, 0x0f10, 0x66, 128, false, 0 },
		{ "movdqa", E::VectorRM, 0x0f6f, 0x66, 128, false, 0 },
		{ "movdqu", E::VectorRM, 0x0f6f, 0xf3, 128, false, 0 },
		{ "movntps", E::VectorMR, 0x0f2b, 0, 128, true, 0 },
		{ "movntpd", E::VectorMR, 0x0f2b, 0x66, 128, true, 0 },
		{ "movntdq", E::VectorMR, 0x0fe7, 0x66, 128, true, 0 },
		{ "addps", E::VectorRM, 0x0f58, 0, 128, false, 0 },
		{ "addpd", E::VectorRM, 0x0f58, 0x66, 128, false, 0 },
		{ "subps", E::VectorRM, 0x0f5c, 0, 128, false, 0 },
		{ "subpd", E::VectorRM, 0x0f5c, 0x66, 128, false, 0 },
		{ "mulps", E::VectorRM, 0x0f59, 0, 128, false, 0 },
		{ "mulpd", E::VectorRM, 0x0f59, 0x66, 128, false, 0 },
		{ "divps", E::VectorRM, 0x0f5e, 0, 128, false, 0 },
		{ "divpd", E::VectorRM, 0x0f5e, 0x66, 128, false, 0 },
		{ "andps", E::VectorRM, 0x0f54, 0, 128, false, 0 },
		{ "andpd", E::VectorRM, 0x0f54, 0x66, 128, false, 0 },
		{ "andnps", E::VectorRM, 0x0f55, 0, 128, false, 0 },
		{ "andnpd", E::VectorRM, 0x0f55, 0x66, 128, false, 0 },
		{ "orps", E::VectorRM, 0x0f56, 0, 128, false, 0 },
		{ "orpd", E::VectorRM, 0x0f56, 0x66, 128, false, 0 },
		{ "xorps", E::VectorRM, 0x0f57, 0, 128, false, 0 },
		{ "xorpd", E::VectorRM, 0x0f57, 0x66, 128, false, 0 },
		{ "paddb", E::VectorRM, 0x0ffc, 0x66, 128, false, 0 },
		{ "paddw", E::VectorRM, 0x0ffd, 0x66, 128, false, 0 },
		{ "paddd", E::VectorRM, 0x0ffe, 0x66, 128, false, 0 },
		{ "paddq", E::VectorRM, 0x0fd4, 0x66, 128, false, 0 },
		{ "psubb", E::VectorRM, 0x0ff8, 0x66, 128, false, 0 },
		{ "psubw", E::VectorRM, 0x0ff9, 0x66, 128, false, 0 },
		{ "psubd", E::VectorRM, 0x0ffa, 0x66, 128, false, 0 },
		{ "psubq", E::VectorRM, 0x0ffb, 0x66, 128, false, 0 },
		{ "pand", E::VectorRM, 0x0fdb, 0x66, 128, false, 0 },
		{ "pandn", E::VectorRM, 0x0fdf, 0x66, 128, false, 0 },
		{ "por", E::VectorRM, 0x0feb, 0x66, 128, false, 0 },
		{ "pxor", E::VectorRM, 0x0fef, 0x66, 128, false, 0 },
		{ "pcmpeqb", E::VectorRM, 0x0f74, 0x66, 128, false, 0 },
		{ "pcmpeqw", E::VectorRM, 0x0f75, 0x66, 128, false, 0 },
		{ "pcmpeqd", E::VectorRM, 0x0f76, 0x66, 128, false, 0 },
		{ "pcmpgtb", E::VectorRM, 0x0f64, 0x66, 128, false, 0 },
		{ "pcmpgtw", E::VectorRM, 0x0f65, 0x66, 128, false, 0 },
		{ "pcmpgtd", E::VectorRM, 0x0f66, 0x66, 128, false, 0 },
	};

	count = sizeof kForms / sizeof kForms[0];
	return kForms;
}

} // namespace x86_asm_model
