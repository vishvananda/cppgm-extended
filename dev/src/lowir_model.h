#pragma once

// The LowIR vocabulary (pa8/lowir.md): every enumerable concept of the
// format as an enum, with its spellings, parsers and the facts the format
// defines for it, plus Type, the value that names a LowIR type.  The
// metadata vocabulary is in ir_symbol_model.h.  The course solution builds
// on this header exactly as it is shipped.
//
// What is deliberately not here: program containers.  How a program,
// function, block, instruction and operand are stored is your design, and
// PA32 will rewrite it in place, one function at a time (see the PA8
// handout).  Store these enums in it rather than text: an instruction is
// an Opcode, not the word `binary`; an operand's kind is a ValueKind, not
// its sigil.  Names (temporaries, slots, blocks, symbols) are the only
// text, and how you store them (strings, interned ids) is also yours.
//
// The spelling() and from_spelling() conventions are those of
// ir_symbol_model.h.

#include <cstddef>
#include <cstdint>

#include "ir_symbol_model.h"

namespace lowir_model {

using namespace ir_model;

// ---------------------------------------------------------------------------
// Types (pa8/lowir.md "Types")

enum class TypeKind : std::uint8_t
{
	Void,
	I1,
	I8,
	U8,
	I16,
	U16,
	I32,
	U32,
	I64,
	I128,
	F32,
	F64,
	F80,
	Ptr,
	Obj // obj<bytes x align>
};

const unsigned kTypeKindCount = static_cast<unsigned>(TypeKind::Obj) + 1;

// A LowIR type as one 8-byte value: the scalar kinds carry no payload;
// `obj<bytes x align>` carries its size and the base-2 logarithm of its
// alignment.  Types compare field by field, so equality needs no interning.
struct Type
{
	std::uint64_t kind_bits : 8;
	std::uint64_t align_log2 : 8;
	std::uint64_t bytes : 48;

	TypeKind kind() const { return static_cast<TypeKind>(kind_bits); }

	static Type scalar(TypeKind kind)
	{
		Type type;
		type.kind_bits = static_cast<std::uint64_t>(kind);
		type.align_log2 = 0;
		type.bytes = 0;
		return type;
	}

	static Type object(std::uint64_t bytes, unsigned align_log2)
	{
		Type type;
		type.kind_bits = static_cast<std::uint64_t>(TypeKind::Obj);
		type.align_log2 = align_log2;
		type.bytes = bytes;
		return type;
	}

	bool is_void() const { return kind() == TypeKind::Void; }
	bool is_integer() const { return kind() >= TypeKind::I1 && kind() <= TypeKind::I128; }
	bool is_float() const { return kind() >= TypeKind::F32 && kind() <= TypeKind::F80; }
	bool is_pointer() const { return kind() == TypeKind::Ptr; }
	bool is_object() const { return kind() == TypeKind::Obj; }

	bool operator==(const Type& other) const
	{
		return kind_bits == other.kind_bits && align_log2 == other.align_log2 && bytes == other.bytes;
	}
	bool operator!=(const Type& other) const { return !(*this == other); }
};

static_assert(sizeof(Type) == 8, "a Type is 8 bytes");

// The word of a kind: "i64", "void", ...; "obj" for Obj, which the text
// writes as `obj<bytes x align>`.
inline const char* spelling(TypeKind kind)
{
	static const char* const kTypeKinds[kTypeKindCount] = { "void", "i1", "i8", "u8", "i16", "u16", "i32", "u32", "i64",
		"i128", "f32", "f64", "f80", "ptr", "obj" };
	return kTypeKinds[static_cast<unsigned>(kind)];
}

// A scalar type word (not `obj`, which takes its <bytes x align> suffix).
inline bool from_spelling(const char* text, std::size_t length, TypeKind* kind)
{
	return detail::find_spelling(text, length, 0, static_cast<unsigned>(TypeKind::Obj), kind);
}

// Bits of an integer kind (i1 is one bit); 0 for other kinds.
inline unsigned integer_bits(TypeKind kind)
{
	switch (kind)
	{
	case TypeKind::I1:
		return 1;
	case TypeKind::I8:
	case TypeKind::U8:
		return 8;
	case TypeKind::I16:
	case TypeKind::U16:
		return 16;
	case TypeKind::I32:
	case TypeKind::U32:
		return 32;
	case TypeKind::I64:
		return 64;
	case TypeKind::I128:
		return 128;
	default:
		return 0;
	}
}

inline bool is_unsigned(TypeKind kind)
{
	return kind == TypeKind::U8 || kind == TypeKind::U16 || kind == TypeKind::U32;
}

// 1, 2, 3 for f32, f64, f80; 0 for other kinds.
inline unsigned float_rank(TypeKind kind)
{
	return kind == TypeKind::F32 ? 1 : kind == TypeKind::F64 ? 2 : kind == TypeKind::F80 ? 3 : 0;
}

// The alignment of a value of `type` in memory: an object's own; a
// scalar's size, 16 for i128 and for f80 (whose storage is 16 bytes).
inline unsigned data_alignment(Type type)
{
	switch (type.kind())
	{
	case TypeKind::Void:
	case TypeKind::I1:
	case TypeKind::I8:
	case TypeKind::U8:
		return 1;
	case TypeKind::I16:
	case TypeKind::U16:
		return 2;
	case TypeKind::I32:
	case TypeKind::U32:
	case TypeKind::F32:
		return 4;
	case TypeKind::I128:
	case TypeKind::F80:
		return 16;
	case TypeKind::Obj:
		return 1u << type.align_log2;
	default:
		return 8;
	}
}

// ---------------------------------------------------------------------------
// Instructions (pa8/lowir.md "Required PA8 Instructions", "Exception
// Instructions")
//
// The order is part of the vocabulary: value-producing operations first,
// then effects, then terminators.

enum class Opcode : std::uint8_t
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
	Call, // a value unless its result type is void
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

const unsigned kOpcodeCount = static_cast<unsigned>(Opcode::Unreachable) + 1;

inline bool is_terminator(Opcode op)
{
	return op >= Opcode::Throw;
}

// Whether the instruction defines a temporary (a call only with a
// non-void result type).
inline bool defines_value(Opcode op)
{
	return op <= Opcode::ExceptionSelector;
}

// The handler-stack markers eh_try ... eh_end.
inline bool is_eh_marker(Opcode op)
{
	return op >= Opcode::EhTry && op <= Opcode::EhEnd;
}

// The instruction's head word: "const", "binary", "copyobj", ...
inline const char* spelling(Opcode op)
{
	static const char* const kOpcodes[kOpcodeCount] = { "const", "copy", "phi", "addr", "load", "atomic_load", "index",
		"unary", "binary", "cmp", "select", "convert", "atomic_add_fetch", "atomic_exchange", "atomic_compare_exchange",
		"va_arg", "stack_alloc", "call", "exception", "exception_selector", "store", "atomic_store",
		"atomic_thread_fence", "atomic_signal_fence", "va_start", "copyobj", "zeroinit", "eh_try", "eh_cleanup",
		"eh_catch", "eh_filter", "eh_catch_all", "eh_end", "throw", "resume", "jump", "branch", "switch", "return",
		"unreachable" };
	return kOpcodes[static_cast<unsigned>(op)];
}

inline bool from_spelling(const char* text, std::size_t length, Opcode* op)
{
	return detail::find_spelling(text, length, 0, kOpcodeCount, op);
}

// The operation of `unary`.  ctz/clz: the index of the lowest (highest)
// set bit of a 32- or 64-bit integer, counted from bit 0 (from the top);
// undefined for 0, as GCC's builtins.
enum class UnaryOp : std::uint8_t
{
	Neg,
	Not,
	BitNot,
	ByteSwap,
	CountTrailingZeros,
	CountLeadingZeros
};

const unsigned kUnaryOpCount = static_cast<unsigned>(UnaryOp::CountLeadingZeros) + 1;

enum class BinaryOp : std::uint8_t
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

// The predicate of `cmp` (also of the floating comparisons, where the
// signed and unsigned forms mean the same).
enum class ComparePredicate : std::uint8_t
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

enum class ConvertOp : std::uint8_t
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

// The memory order operand of the atomic instructions, written as its
// number: the GNU/Clang __ATOMIC_* encoding (pa8/lowir.md "Memory and
// Addressing").
enum class MemoryOrder : std::uint8_t
{
	Relaxed,
	Consume,
	Acquire,
	Release,
	AcqRel,
	SeqCst
};

const unsigned kMemoryOrderCount = static_cast<unsigned>(MemoryOrder::SeqCst) + 1;

inline const char* spelling(UnaryOp op)
{
	static const char* const kUnaryOps[kUnaryOpCount] = { "neg", "not", "bitnot", "bswap", "ctz", "clz" };
	return kUnaryOps[static_cast<unsigned>(op)];
}

inline const char* spelling(BinaryOp op)
{
	static const char* const kBinaryOps[kBinaryOpCount] = { "add", "sub", "mul", "div", "mod", "udiv", "umod", "and",
		"or", "xor", "shl", "shr", "ushr" };
	return kBinaryOps[static_cast<unsigned>(op)];
}

inline const char* spelling(ComparePredicate predicate)
{
	static const char* const kComparePredicates[kComparePredicateCount] = { "eq", "ne", "lt", "le", "gt", "ge", "ult",
		"ule", "ugt", "uge" };
	return kComparePredicates[static_cast<unsigned>(predicate)];
}

inline const char* spelling(ConvertOp op)
{
	static const char* const kConvertOps[kConvertOpCount] = { "sext", "zext", "trunc", "sitofp", "uitofp", "fptosi",
		"fptoui", "fpext", "fptrunc" };
	return kConvertOps[static_cast<unsigned>(op)];
}

inline bool from_spelling(const char* text, std::size_t length, UnaryOp* op)
{
	return detail::find_spelling(text, length, 0, kUnaryOpCount, op);
}

inline bool from_spelling(const char* text, std::size_t length, BinaryOp* op)
{
	return detail::find_spelling(text, length, 0, kBinaryOpCount, op);
}

inline bool from_spelling(const char* text, std::size_t length, ComparePredicate* predicate)
{
	return detail::find_spelling(text, length, 0, kComparePredicateCount, predicate);
}

inline bool from_spelling(const char* text, std::size_t length, ConvertOp* op)
{
	return detail::find_spelling(text, length, 0, kConvertOpCount, op);
}

// The predicate that holds with the operands exchanged (a < b is b > a).
inline ComparePredicate swapped(ComparePredicate predicate)
{
	static const ComparePredicate kSwapped[kComparePredicateCount] = { ComparePredicate::Eq, ComparePredicate::Ne,
		ComparePredicate::Gt, ComparePredicate::Ge, ComparePredicate::Lt, ComparePredicate::Le, ComparePredicate::Ugt,
		ComparePredicate::Uge, ComparePredicate::Ult, ComparePredicate::Ule };
	return kSwapped[static_cast<unsigned>(predicate)];
}

// The integer predicate that holds exactly when `predicate` does not
// (a < b is !(a >= b)).  Not the negation of a floating comparison, which
// is false for both when an operand is a NaN.
inline ComparePredicate inverted(ComparePredicate predicate)
{
	static const ComparePredicate kInverted[kComparePredicateCount] = { ComparePredicate::Ne, ComparePredicate::Eq,
		ComparePredicate::Ge, ComparePredicate::Gt, ComparePredicate::Le, ComparePredicate::Lt, ComparePredicate::Uge,
		ComparePredicate::Ugt, ComparePredicate::Ule, ComparePredicate::Ult };
	return kInverted[static_cast<unsigned>(predicate)];
}

// ---------------------------------------------------------------------------
// Instruction flags: the optional parts of an instruction's text, as bits.
//
// - IF_VOLATILE: `load volatile`, `store volatile`.
// - IF_SIGNATURE: a call with an explicit `as (...) -> T` signature.
// - IF_COPY_ELISION: `call ... [elision=copy]`.
// - IF_SELECTOR: eh_catch, eh_filter or eh_catch_all states a selector.
// - The instruction facts (pa8/lowir.md "Instruction Facts"), true whenever
//   the instruction runs; dropping one is always correct: IF_EXACT (`exact`
//   on div/udiv/shr/ushr), IF_NO_WRAP (`nowrap` on add/sub/mul/shl),
//   IF_TYPED (`typed` on load/store).

enum InstructionFlag : std::uint8_t
{
	IF_VOLATILE = 1,
	IF_SIGNATURE = 2,
	IF_COPY_ELISION = 4,
	IF_SELECTOR = 8,
	IF_EXACT = 16,
	IF_NO_WRAP = 32,
	IF_TYPED = 64
};

// The keyword of a flag the text spells as a word (`volatile`, `exact`,
// `nowrap`, `typed`); null for the others, which are spelled by syntax
// (`as`, `elision=copy`, a selector operand).
inline const char* spelling(InstructionFlag flag)
{
	switch (flag)
	{
	case IF_VOLATILE:
		return "volatile";
	case IF_EXACT:
		return "exact";
	case IF_NO_WRAP:
		return "nowrap";
	case IF_TYPED:
		return "typed";
	default:
		return 0;
	}
}

inline bool from_spelling(const char* text, std::size_t length, InstructionFlag* flag)
{
	const InstructionFlag words[] = { IF_VOLATILE, IF_EXACT, IF_NO_WRAP, IF_TYPED };
	for (unsigned i = 0; i < 4; ++i)
	{
		if (detail::same_spelling(spelling(words[i]), text, length))
		{
			*flag = words[i];
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// Operands (pa8/lowir.md "Operands")

enum class ValueKind : std::uint8_t
{
	None,
	Temp,    // %name
	Slot,    // $name
	Symbol,  // @name (the address of a global or function)
	Block,   // ^name
	Integer, // an integer literal
	Float    // a floating literal
};

const unsigned kValueKindCount = static_cast<unsigned>(ValueKind::Float) + 1;

// The sigil of a named operand; 0 for the others.
inline char sigil(ValueKind kind)
{
	static const char kSigils[kValueKindCount] = { 0, '%', '$', '@', '^', 0, 0 };
	return kSigils[static_cast<unsigned>(kind)];
}

inline ValueKind value_kind_of_sigil(char c)
{
	return c == '%' ? ValueKind::Temp
		: c == '$'  ? ValueKind::Slot
		: c == '@'  ? ValueKind::Symbol
		: c == '^'  ? ValueKind::Block
					: ValueKind::None;
}

// The literals written as words: `nullptr` (a ptr), `inf`, `nan`, `snan`
// (also `INFINITY`, `NAN`, `SNAN`; the floating words may take an `f`, `F`,
// `l` or `L` suffix for f32 or f80).
enum class NamedLiteral : std::uint8_t
{
	Nullptr,
	Infinity,
	QuietNaN,
	SignalingNaN
};

const unsigned kNamedLiteralCount = static_cast<unsigned>(NamedLiteral::SignalingNaN) + 1;

inline const char* spelling(NamedLiteral literal)
{
	static const char* const kNamedLiterals[kNamedLiteralCount] = { "nullptr", "inf", "nan", "snan" };
	return kNamedLiterals[static_cast<unsigned>(literal)];
}

// The uppercase alternative (`INFINITY`, `NAN`, `SNAN`); null for nullptr.
inline const char* alternate_spelling(NamedLiteral literal)
{
	static const char* const kAlternates[kNamedLiteralCount] = { 0, "INFINITY", "NAN", "SNAN" };
	return kAlternates[static_cast<unsigned>(literal)];
}

// Either spelling, without a suffix.
inline bool from_spelling(const char* text, std::size_t length, NamedLiteral* literal)
{
	for (unsigned i = 0; i < kNamedLiteralCount; ++i)
	{
		const NamedLiteral candidate = static_cast<NamedLiteral>(i);
		const char* alternate = alternate_spelling(candidate);
		if (detail::same_spelling(spelling(candidate), text, length) ||
			(alternate != 0 && detail::same_spelling(alternate, text, length)))
		{
			*literal = candidate;
			return true;
		}
	}
	return false;
}

// The floating kind a literal suffix selects: F32 for f/F, F80 for l/L,
// Void for none (the context's type), Obj for anything else.
inline TypeKind float_suffix_kind(const char* suffix, std::size_t length)
{
	if (length == 0)
	{
		return TypeKind::Void;
	}
	if (length == 1 && (suffix[0] == 'f' || suffix[0] == 'F'))
	{
		return TypeKind::F32;
	}
	if (length == 1 && (suffix[0] == 'l' || suffix[0] == 'L'))
	{
		return TypeKind::F80;
	}
	return TypeKind::Obj;
}

// The suffix a writer gives a floating literal of `kind`: "f" for f32,
// "L" for f80, "" for f64.
inline const char* float_suffix(TypeKind kind)
{
	return kind == TypeKind::F32 ? "f" : kind == TypeKind::F80 ? "L" : "";
}

// ---------------------------------------------------------------------------
// Program structure (pa8/lowir.md "Program Structure", "Globals")

// The structural words of the text other than instruction heads, types
// and metadata.
enum class Keyword : std::uint8_t
{
	Declare, // declare global, declare function
	Global,
	Function,
	Alias, // alias object <name> = @target
	Object,
	Slot,
	Block,
	Zero, // `= zero`, the `zero <bytes>` data item
	As,   // call ... as (<parameters>) -> <type>
	DebugType,
	DebugScope,
	DebugLocal
};

const unsigned kKeywordCount = static_cast<unsigned>(Keyword::DebugLocal) + 1;

inline const char* spelling(Keyword keyword)
{
	static const char* const kKeywords[kKeywordCount] = { "declare", "global", "function", "alias", "object", "slot",
		"block", "zero", "as", "debug_type", "debug_scope", "debug_local" };
	return kKeywords[static_cast<unsigned>(keyword)];
}

inline bool from_spelling(const char* text, std::size_t length, Keyword* keyword)
{
	return detail::find_spelling(text, length, 0, kKeywordCount, keyword);
}

// A global's initializer: none (a declaration), `= zero`, `= <constant>`,
// `= addr @symbol [+ n]`, or `= { <data items> }`.
enum class GlobalInit : std::uint8_t
{
	None,
	Zero,
	Constant,
	Address,
	Data
};

// A structured global's data item: `<type> <constant>`, `ptr addr @symbol
// [+ n]`, `zero <bytes>`.
enum class DataItemKind : std::uint8_t
{
	Scalar,
	Address,
	Zero
};

} // namespace lowir_model
