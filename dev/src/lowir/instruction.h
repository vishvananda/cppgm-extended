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

// ---------------------------------------------------------------------------
// Operations

enum class Opcode : uint8_t
{
	// Value-producing instructions.
	Const,
	Copy,
	Phi,
	Addr,
	Load,
	AtomicLoad,
	Index,
	Unary,
	Binary,
	Compare,
	Select,
	Convert,
	AtomicAddFetch,
	AtomicExchange,
	AtomicCompareExchange,
	VaArg,
	StackAlloc,
	Call,
	Exception,
	ExceptionSelector,
	// Effects.
	Store,
	AtomicStore,
	AtomicThreadFence,
	AtomicSignalFence,
	VaStart,
	CopyObject,
	ZeroInit,
	EhTry,
	EhCleanup,
	EhCatch,
	EhFilter,
	EhCatchAll,
	EhEnd,
	// Terminators.
	Throw,
	Resume,
	Jump,
	Branch,
	Switch,
	Return,
	Unreachable
};

inline bool is_terminator(Opcode op)
{
	return op >= Opcode::Throw;
}

enum class UnaryOp : uint8_t
{
	Neg,
	Not,
	BitNot,
	ByteSwap,
	// The index of the lowest (highest) set bit of a 32- or 64-bit integer,
	// counted from bit 0 (from the top): undefined for 0, as GCC's builtins.
	CountTrailingZeros,
	CountLeadingZeros
};

const unsigned kUnaryOpCount = static_cast<unsigned>(UnaryOp::CountLeadingZeros) + 1;

enum class BinaryOp : uint8_t
{
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	UDiv,
	UMod,
	And,
	Or,
	Xor,
	Shl,
	Shr,
	UShr
};

const unsigned kBinaryOpCount = static_cast<unsigned>(BinaryOp::UShr) + 1;

enum class ComparePredicate : uint8_t
{
	Eq,
	Ne,
	Lt,
	Le,
	Gt,
	Ge,
	Ult,
	Ule,
	Ugt,
	Uge
};

const unsigned kComparePredicateCount = static_cast<unsigned>(ComparePredicate::Uge) + 1;

enum class ConvertOp : uint8_t
{
	SExt,
	ZExt,
	Trunc,
	SIToFP,
	UIToFP,
	FPToSI,
	FPToUI,
	FPExt,
	FPTrunc
};

const unsigned kConvertOpCount = static_cast<unsigned>(ConvertOp::FPTrunc) + 1;

// GNU/Clang memory-order encoding (lowir.md "Memory and Addressing").
enum class MemoryOrder : uint8_t
{
	Relaxed,
	Consume,
	Acquire,
	Release,
	AcqRel,
	SeqCst
};

const unsigned kMemoryOrderCount = 6;

// ---------------------------------------------------------------------------
// Values

enum class ValueKind : uint8_t
{
	None,
	Temp,
	Slot,
	Symbol,
	Block,
	Integer,
	Float
};

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

	bool present() const { return file != 0; }
};

// ---------------------------------------------------------------------------
// Instructions

enum InstructionFlag : uint8_t
{
	IF_VOLATILE = 1,     // load/store volatile
	IF_SIGNATURE = 2,    // call with an explicit `as (...) -> T` signature
	IF_COPY_ELISION = 4, // call [elision=copy]
	IF_SELECTOR = 8,     // eh_catch/eh_filter/eh_catch_all with a selector
	// Instruction facts (lowir.md "Instruction Facts"): true whenever the
	// instruction runs, or the program's behavior is undefined; dropping one
	// is always correct. Every reader accepts them and every writer prints
	// them.
	//
	// binary div/udiv `exact`: the dividend is a multiple of the divisor (a
	// pointer difference, 5.7p6); shr/ushr: no set bit is shifted out.
	IF_EXACT = 16,
	// binary add/sub/mul/shl `nowrap`: the exact result is representable in
	// the type at its signedness (a loop counter stepped by one while below a
	// bound of its type): computed at 64 bits from canonical operands it is
	// its own canonical register form.
	IF_NO_WRAP = 32,
	// load/store `typed`: a C++ access of an object through a glvalue of the
	// access's own type ([basic.lval]/10), not a character type: two typed
	// accesses of incompatible types (a pointer and a non-pointer, integers
	// of different widths, an integer and a floating type) never read or
	// write the same bytes. The optimizer uses the pointer/non-pointer case.
	IF_TYPED = 64
};

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
