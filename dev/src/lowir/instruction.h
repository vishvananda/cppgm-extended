#pragma once

#include <cstdint>
#include <cstring>

#include "lowir/name_table.h"
#include "lowir/types.h"

// The LowIR instruction vocabulary (lowir.md "Required PA8 Instructions",
// "PA17 Exception/Runtime Instructions"): operations, operand values,
// literal bits and the instruction record of a function body.
namespace lowir {

typedef uint32_t SymbolId;
typedef uint32_t SignatureId;
typedef uint32_t TempId;
typedef uint32_t SlotId;
typedef uint32_t BlockId;
typedef uint32_t InstId;

const uint32_t kNoId = 0xffffffffu;

// The operations (Opcode, UnaryOp, BinaryOp, ComparePredicate, ConvertOp,
// MemoryOrder), operand kinds (ValueKind) and instruction flags
// (InstructionFlag) are lowir_model.h's (lowir/types.h).

// ---------------------------------------------------------------------------
// Values

// A literal's bits. Integers are 128-bit two's complement; a literal read
// outside an i128 context is wrapped to 64 bits and sign extended. Floats
// hold the IEEE pattern of their kind: f32 and f64 in `low`, f80 as its
// 64-bit significand in `low` and sign/exponent in the low 16 bits of `high`.
struct Literal
{
	uint64_t low;
	uint64_t high;
};

// An operand: a temporary, slot, top-level symbol (its address), block
// target, or a literal in the owning function's literal pool.
struct Value
{
	ValueKind kind;
	// The kind of a Float literal: F32, F64 or F80.
	TypeKind float_kind;
	uint32_t index;

	static Value none() { return make(ValueKind::None, 0); }
	static Value temp(TempId id) { return make(ValueKind::Temp, id); }
	static Value slot(SlotId id) { return make(ValueKind::Slot, id); }
	static Value symbol(SymbolId id) { return make(ValueKind::Symbol, id); }
	static Value block(BlockId id) { return make(ValueKind::Block, id); }

	bool is(ValueKind k) const { return kind == k; }
	bool is_literal() const { return kind == ValueKind::Integer || kind == ValueKind::Float; }

	bool operator==(const Value& other) const
	{
		return kind == other.kind && float_kind == other.float_kind && index == other.index;
	}

	static Value make(ValueKind kind, uint32_t index)
	{
		Value value;
		value.kind = kind;
		value.float_kind = TypeKind::Void;
		value.index = index;
		return value;
	}
};

// A literal outside a function body (global initializers and data items).
struct Constant
{
	ValueKind kind;      // Integer or Float
	TypeKind float_kind; // for Float
	Literal bits;
};

// A `!dbg(file, line, column)` location; file 0 means absent.
struct DebugLocation
{
	NameId file;
	uint32_t line;
	uint32_t column;
	// Global optional DebugInfo lexical scope id; zero is function scope.
	uint32_t scope;

	bool present() const { return file != 0; }
};

// ---------------------------------------------------------------------------
// Instructions

// One instruction. Operand use by opcode:
//   Const             ops[0] literal
//   Copy              ops[0]
//   Phi               extra: (Block, value) pairs
//   Addr              ops[0] slot or symbol
//   Load              ops[0] storage (temp, slot or symbol); IF_VOLATILE
//   AtomicLoad        ops[0] pointer; order
//   Index             ops[0] base, ops[1] offset; sub = IndexProjectionKind
//   Unary             ops[0]; sub = UnaryOp
//   Binary, Compare   ops[0], ops[1]; sub = BinaryOp / ComparePredicate
//   Select            ops[0] condition (nonzero: ops[1], else ops[2]); type =
//                     result type (an integer or pointer)
//   Convert           ops[0]; type = destination, immediate = source type
//                     (convert_source_type); sub = ConvertOp
//   AtomicAddFetch,
//   AtomicExchange    ops[0] pointer, ops[1] value; order
//   AtomicCompareExchange ops[0] pointer, ops[1] expected pointer,
//                     ops[2] desired; order (success), sub = failure order
//   VaArg             ops[0] list
//   StackAlloc        ops[0] size
//   Call              ops[0] callee; extra: arguments; immediate =
//                     SignatureId when IF_SIGNATURE; type = result type
//   Exception,
//   ExceptionSelector (no operands)
//   Store             ops[0] value, ops[1] storage; IF_VOLATILE
//   AtomicStore       ops[0] value, ops[1] pointer; order
//   AtomicThreadFence,
//   AtomicSignalFence order
//   VaStart           ops[0] list
//   CopyObject        ops[0] source, ops[1] destination; immediate = bytes,
//                     sub = alignment log2
//   ZeroInit          ops[0] destination; immediate = bytes, sub = log2
//   EhTry             ops[0] handler block
//   EhCleanup         ops[0] cleanup block or none
//   EhCatch           ops[0] type-info symbol; immediate = selector
//   EhFilter          extra: type-info symbols; immediate = selector
//   EhCatchAll        immediate = selector
//   Throw             ops[0]; type
//   Jump              ops[0] block
//   Branch            ops[0] condition, ops[1] true block, ops[2] false block
//   Switch            ops[0] selector, ops[1] default block;
//                     extra: (case value, Block) pairs
//   Return            ops[0] value or none; type
// `location` is 1 + the index of the instruction's DebugLocation in its
// body's `locations`, or 0 for none.
struct Instruction
{
	Opcode op;
	uint8_t sub;
	uint8_t flags;
	MemoryOrder order;
	TempId result;
	InstId next;
	uint32_t location;
	Type type;
	Value ops[3];
	uint32_t extra_first;
	uint32_t extra_count;
	int64_t immediate;
};

static_assert(sizeof(Instruction) == 64, "one instruction is 64 bytes");

inline Type convert_source_type(const Instruction& inst)
{
	Type type;
	std::memcpy(&type, &inst.immediate, sizeof type);
	return type;
}

inline void set_convert_source_type(Instruction& inst, Type type)
{
	std::memcpy(&inst.immediate, &type, sizeof type);
}

} // namespace lowir
