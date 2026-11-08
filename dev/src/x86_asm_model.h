#pragma once

// Optional PA36's initial x86-64 encoding vocabulary and instruction facts.
// These are architectural forms, independent of a frontend, LowIR containers,
// register allocator or object writer. See pa36/README.md and assembly.md.
// The course solution consumes this header as shipped; students may use these
// facts with their own parser, form selection and encoding design.

#include <cstddef>
#include <cstdint>

namespace x86_asm_model {

enum class Encoding : std::uint8_t
{
	Zero,
	Move,
	Lea,
	Exchange,
	Alu,
	VectorRM,
	VectorMR
};

struct Form
{
	const char* mnemonic;
	Encoding encoding;
	std::uint32_t opcode;
	std::uint8_t prefix;
	std::uint16_t bits;
	bool memory_only;
};

// Opcode bytes are most significant first. ALU forms carry their group digit.
// bits=0 uses the operand width. RM/MR distinguish register destination/source;
// mandatory prefix and memory-only restrictions are part of the form.
inline const Form* forms(std::size_t& count)
{
	using E = Encoding;
	static const Form kForms[] = {
		{ "mov", E::Move, 0, 0, 0, false },
		{ "movabs", E::Move, 0, 0, 64, false },
		{ "lea", E::Lea, 0x8d, 0, 0, true },
		{ "xchg", E::Exchange, 0x87, 0, 0, false },
		{ "add", E::Alu, 0, 0, 0, false },
		{ "or", E::Alu, 1, 0, 0, false },
		{ "adc", E::Alu, 2, 0, 0, false },
		{ "sbb", E::Alu, 3, 0, 0, false },
		{ "and", E::Alu, 4, 0, 0, false },
		{ "sub", E::Alu, 5, 0, 0, false },
		{ "xor", E::Alu, 6, 0, 0, false },
		{ "cmp", E::Alu, 7, 0, 0, false },
		{ "nop", E::Zero, 0x90, 0, 0, false },
		{ "pause", E::Zero, 0xf390, 0, 0, false },
		{ "cpuid", E::Zero, 0x0fa2, 0, 0, false },
		{ "xgetbv", E::Zero, 0x0f01d0, 0, 0, false },
		{ "rdtsc", E::Zero, 0x0f31, 0, 0, false },
		{ "rdtscp", E::Zero, 0x0f01f9, 0, 0, false },
		{ "lfence", E::Zero, 0x0faee8, 0, 0, false },
		{ "mfence", E::Zero, 0x0faef0, 0, 0, false },
		{ "sfence", E::Zero, 0x0faef8, 0, 0, false },
		{ "cld", E::Zero, 0xfc, 0, 0, false },
		{ "std", E::Zero, 0xfd, 0, 0, false },
		{ "ud2", E::Zero, 0x0f0b, 0, 0, false },
		{ "emms", E::Zero, 0x0f77, 0, 0, false },
		{ "movaps", E::VectorRM, 0x0f28, 0, 128, false },
		{ "movups", E::VectorRM, 0x0f10, 0, 128, false },
		{ "movapd", E::VectorRM, 0x0f28, 0x66, 128, false },
		{ "movupd", E::VectorRM, 0x0f10, 0x66, 128, false },
		{ "movdqa", E::VectorRM, 0x0f6f, 0x66, 128, false },
		{ "movdqu", E::VectorRM, 0x0f6f, 0xf3, 128, false },
		{ "movntps", E::VectorMR, 0x0f2b, 0, 128, true },
		{ "movntpd", E::VectorMR, 0x0f2b, 0x66, 128, true },
		{ "movntdq", E::VectorMR, 0x0fe7, 0x66, 128, true },
		{ "addps", E::VectorRM, 0x0f58, 0, 128, false },
		{ "addpd", E::VectorRM, 0x0f58, 0x66, 128, false },
		{ "subps", E::VectorRM, 0x0f5c, 0, 128, false },
		{ "subpd", E::VectorRM, 0x0f5c, 0x66, 128, false },
		{ "mulps", E::VectorRM, 0x0f59, 0, 128, false },
		{ "mulpd", E::VectorRM, 0x0f59, 0x66, 128, false },
		{ "divps", E::VectorRM, 0x0f5e, 0, 128, false },
		{ "divpd", E::VectorRM, 0x0f5e, 0x66, 128, false },
		{ "andps", E::VectorRM, 0x0f54, 0, 128, false },
		{ "andpd", E::VectorRM, 0x0f54, 0x66, 128, false },
		{ "andnps", E::VectorRM, 0x0f55, 0, 128, false },
		{ "andnpd", E::VectorRM, 0x0f55, 0x66, 128, false },
		{ "orps", E::VectorRM, 0x0f56, 0, 128, false },
		{ "orpd", E::VectorRM, 0x0f56, 0x66, 128, false },
		{ "xorps", E::VectorRM, 0x0f57, 0, 128, false },
		{ "xorpd", E::VectorRM, 0x0f57, 0x66, 128, false },
		{ "paddb", E::VectorRM, 0x0ffc, 0x66, 128, false },
		{ "paddw", E::VectorRM, 0x0ffd, 0x66, 128, false },
		{ "paddd", E::VectorRM, 0x0ffe, 0x66, 128, false },
		{ "paddq", E::VectorRM, 0x0fd4, 0x66, 128, false },
		{ "psubb", E::VectorRM, 0x0ff8, 0x66, 128, false },
		{ "psubw", E::VectorRM, 0x0ff9, 0x66, 128, false },
		{ "psubd", E::VectorRM, 0x0ffa, 0x66, 128, false },
		{ "psubq", E::VectorRM, 0x0ffb, 0x66, 128, false },
		{ "pand", E::VectorRM, 0x0fdb, 0x66, 128, false },
		{ "pandn", E::VectorRM, 0x0fdf, 0x66, 128, false },
		{ "por", E::VectorRM, 0x0feb, 0x66, 128, false },
		{ "pxor", E::VectorRM, 0x0fef, 0x66, 128, false },
		{ "pcmpeqb", E::VectorRM, 0x0f74, 0x66, 128, false },
		{ "pcmpeqw", E::VectorRM, 0x0f75, 0x66, 128, false },
		{ "pcmpeqd", E::VectorRM, 0x0f76, 0x66, 128, false },
		{ "pcmpgtb", E::VectorRM, 0x0f64, 0x66, 128, false },
		{ "pcmpgtw", E::VectorRM, 0x0f65, 0x66, 128, false },
		{ "pcmpgtd", E::VectorRM, 0x0f66, 0x66, 128, false },
	};

	count = sizeof kForms / sizeof kForms[0];
	return kForms;
}

} // namespace x86_asm_model
