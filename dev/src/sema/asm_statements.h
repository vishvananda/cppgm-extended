#pragma once

#include <cstdint>
#include <string>
#include <vector>

// GNU asm statements: their AT&T-syntax templates are not
// assembled. Each instruction of the small set hosted headers write --
// fences and spin hints, the read-modify-writes of an operand (locked: an
// atomic one), exchanges and byte swaps -- is lifted to the operation it
// performs on the statement's operands, which the lowering emits as LowIR;
// any other template is rejected.

namespace sema {

// How an operand is bound (its constraint's class): an object in memory,
// a value in a register, an integral constant.
enum class AsmOperandClass : uint8_t
{
	Memory,
	Register,
	Immediate
};

// One asm operand as its constraint and type give it: outputs first, then
// inputs (GCC's %N numbering); a `+` output is read too.
struct AsmOperandInfo
{
	const char* name;
	AsmOperandClass binding;
	bool output;
	bool read;
	// The operand's size in bytes (its type's); an immediate's value.
	uint32_t bytes;
	uint64_t value;
	// An input with a matching constraint ("0", "[name]"): the output
	// whose register it starts that output in (0xff: none). The template
	// names that register by either operand.
	uint8_t tied;
};

enum class AsmOperation : uint8_t
{
	// mfence, lfence, sfence: a sequentially consistent fence.
	Fence,
	Add,
	Sub,
	And,
	Or,
	Xor,
	Not,
	Neg,
	// xchg: the two operands' values swapped (memory with a register:
	// atomically).
	Exchange,
	// bswap of a register operand.
	ByteSwap
};

const uint8_t kAsmImmediateSource = 0xff;

// One lifted instruction, on the low `bytes` of its operands (an integer
// of that size whatever their types: GAS and the CPU see bits).
struct AsmInstruction
{
	AsmOperation operation;
	// A `lock` prefix: an atomic read-modify-write of a memory destination.
	bool locked;
	uint8_t destination;
	// An operand index, or kAsmImmediateSource for `immediate`.
	uint8_t source;
	// The operation's size in bytes: 1, 2, 4 or 8.
	uint8_t bytes;
	uint64_t immediate;
};

// A lifted template: its instructions in order, then (a "memory"
// clobber) a compiler barrier. Bit i of `registers`: operand i is bound to
// a register (a copy of its value, which an output's object receives after
// the template). `tied` (null when no input is): for each output, the
// input whose value its register starts with, or 0xff. `unlifted` (null
// when lifted): why the template is outside the set, in a function emitted
// only where odr-used, whose emission reports it (Analyzer::asm_statement).
struct AsmProgram
{
	const AsmInstruction* instructions;
	uint32_t count;
	uint32_t registers;
	const uint8_t* tied;
	bool barrier;
	const char* unlifted;
};

// Lifts `text` (the template's characters) over the operands; false, with
// `error`, for a template outside the set.
bool LiftAsmTemplate(const std::string& text, const AsmOperandInfo* operands, uint32_t count,
	std::vector<AsmInstruction>& out, std::string& error);

} // namespace sema
