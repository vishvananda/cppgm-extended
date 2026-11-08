#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

// Optional PA36 assembly vocabulary (pa36/assembly.md), supplied as a header
// scaffold. These records describe the public boundary; program containers,
// parsing, constraint selection and emission remain the student's design.
//
// Frontend-neutral assembly vocabulary and checked boundary records. Source
// frontends attach their own types; LowIR adds its types and argument values.
// Register identities and constraints never depend on a C++ or Rust AST.
namespace asm_model {

const uint32_t kNoId = 0xffffffffu;

namespace detail {

template<typename E>
bool find_spelling(const char* text, std::size_t length, unsigned count, E* value)
{
	for (unsigned i = 0; i < count; ++i)
	{
		const char* word = spelling(static_cast<E>(i));
		if (std::strlen(word) == length && std::memcmp(word, text, length) == 0)
		{
			*value = static_cast<E>(i);
			return true;
		}
	}
	return false;
}

} // namespace detail

struct Bits
{
	uint64_t low;
	uint64_t high;
};

enum class Dialect : uint8_t
{
	Att,
	Intel
};
enum class RegisterBank : uint8_t
{
	Gpr,
	Vector,
	Mask,
	X87,
	Mmx,
	Tile,
	Segment,
	Control,
	Debug
};
enum class Memory : uint8_t
{
	None,
	Read,
	Write,
	ReadWrite
};
enum class Format : uint8_t
{
	Default,
	Bare,
	Negated,
	Label,
	Suffix
};
enum class PieceKind : uint8_t
{
	Text,
	Operand,
	Unique
};

enum class Record : uint8_t
{
	Operand,
	Alternative,
	Group,
	Range,
	Clobber,
	Text,
	Use,
	Unique
};

const unsigned kRecordCount = static_cast<unsigned>(Record::Unique) + 1;
const unsigned kDialectCount = static_cast<unsigned>(Dialect::Intel) + 1;
const unsigned kMemoryCount = static_cast<unsigned>(Memory::ReadWrite) + 1;

inline const char* spelling(Record record)
{
	static const char* const kRecords[kRecordCount] = { "operand", "alternative", "group", "range", "clobber", "text",
		"use", "unique" };
	return kRecords[static_cast<unsigned>(record)];
}

inline const char* spelling(Dialect dialect)
{
	static const char* const kDialects[kDialectCount] = { "att", "intel" };
	return kDialects[static_cast<unsigned>(dialect)];
}

inline const char* spelling(Memory memory)
{
	static const char* const kMemories[kMemoryCount] = { "none", "read", "write", "readwrite" };
	return kMemories[static_cast<unsigned>(memory)];
}

inline bool from_spelling(const char* text, std::size_t length, Record* record)
{
	return detail::find_spelling(text, length, kRecordCount, record);
}

inline bool from_spelling(const char* text, std::size_t length, Dialect* dialect)
{
	return detail::find_spelling(text, length, kDialectCount, dialect);
}

inline bool from_spelling(const char* text, std::size_t length, Memory* memory)
{
	return detail::find_spelling(text, length, kMemoryCount, memory);
}

enum class ImmediateKind : uint8_t
{
	IntegerRange,
	FloatBits,
	Relocatable
};

enum Location : uint8_t
{
	AL_REGISTER = 1,
	AL_MEMORY = 2,
	AL_IMMEDIATE = 4,
	AL_SYMBOL = 8,
	AL_LABEL = 16
};

enum OperandFlag : uint8_t
{
	AO_READ = 1,
	AO_WRITE = 2,
	AO_COMMUTATIVE = 4,
	AO_FLAG = 8,
	AO_VOLATILE = 16,
	AO_NO_MEMORY = 32,
	AO_EXIT_TAG = 64 // i32 output selecting fallthrough (0) or a label's tag
};

enum Property : uint16_t
{
	AP_SIDE_EFFECT = 1,
	AP_DUPLICABLE = 2,
	AP_STACK = 4,
	AP_RED_ZONE = 8,
	AP_UNWIND = 16,
	AP_NORETURN = 32,
	AP_INLINE_COST_SMALL = 64
};

enum State : uint32_t
{
	AS_CONDITION_FLAGS = 1,
	AS_X87_STATUS = 2,
	AS_MXCSR_STATUS = 4,
	AS_X87_CONTROL = 8,
	AS_MXCSR_CONTROL = 16,
	AS_DIRECTION_FLAG = 32
};

// A producer's symbol identity and constant byte displacement.
struct SymbolReference
{
	uint32_t symbol = kNoId;
	int64_t addend = 0;
};

// A view of one architectural register. Overlap is determined by bank and
// number, not by spelling: e.g. AH and RAX, or XMM3 and ZMM3. Zero width in a
// constraint domain means the operand's width; encoded operands have a width.
struct Register
{
	RegisterBank bank;
	uint8_t number;
	uint16_t bits;
	uint8_t offset;
};

struct RegisterGroup
{
	uint32_t first_register = 0;
	uint16_t register_count = 0;
};

struct Immediate
{
	ImmediateKind kind = ImmediateKind::IntegerRange;
	bool is_signed = false;
	Bits low = {};
	Bits high = {};
};

// Every operand selects the same alternative column. A column can allow more
// than one location kind. Each register group is one complete binding (usually
// one register; a wide integer can require a pair). An empty immediate domain
// permits any constant of the operand's type. Matching and early-write rules
// belong to a column: GNU alternatives can differ in both respects.
struct Alternative
{
	uint32_t first_group = 0;
	uint32_t first_immediate = 0;
	uint32_t matching = kNoId;
	uint16_t group_count = 0;
	uint16_t immediate_count = 0;
	uint8_t locations = 0;
	bool early_write = false;
};

// input/output are indices in this invocation's argument list, not TempIds.
// An input is either a captured value or an explicit storage address. Keeping
// that distinction prevents a memory operand from acquiring an extra load.
// Register outputs are captured before ordered writes to output addresses;
// discarded outputs have no address. Read/write operands state both boundaries.
struct Operand
{
	uint32_t input = kNoId;
	uint32_t output = kNoId;
	uint32_t first_alternative = 0;
	uint16_t alternative_count = 0;
	uint8_t flags = 0;
	bool input_storage = false;
	uint8_t condition = 0;
};

struct Piece
{
	PieceKind kind = PieceKind::Text;
	Format format = Format::Default;
	uint16_t bits = 0;
	uint8_t offset = 0;
	uint32_t operand = kNoId;
	uint32_t first = 0;
	uint32_t count = 0;
};

struct Template
{
	Dialect dialect = Dialect::Att;
	Memory memory = Memory::ReadWrite;
	uint16_t properties = AP_SIDE_EFFECT | AP_STACK | AP_RED_ZONE;
	uint32_t first_operand = 0;
	uint32_t operand_count = 0;
	uint32_t argument_count = 0;
	uint32_t first_piece = 0;
	uint32_t piece_count = 0;
	uint32_t first_clobber = 0;
	uint32_t clobber_count = 0;
	uint32_t state_read = 0;
	uint32_t state_write = AS_CONDITION_FLAGS | AS_X87_STATUS | AS_MXCSR_STATUS;
};

} // namespace asm_model
