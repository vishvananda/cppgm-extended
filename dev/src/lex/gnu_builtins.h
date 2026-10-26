#pragma once

#include <cstdint>

#include "lex/token_kind.h"

// The GNU builtins this implementation provides to hosted source (PA26,
// PA29): the builtin functions of the registry below (__builtin_memcpy ...)
// and the type traits and transforms (__is_class ...). Hosted headers ask
// `__has_builtin` before using an optional one; it answers from these
// registries, which the parser and analyzer read the names from.
bool IsProvidedBuiltin(const char* spelling, uint32_t length);

// The builtin functions (GCC's and Clang's): the analyzer declares each in
// the global namespace from its entry when a call first names it.
enum class GnuBuiltin : uint8_t
{
	None,
	// Functions of their signature whose meaning the lowering knows
	// (sema::BuiltinFunction).
	Abort,
	Memcpy,
	Memmove,
	Strlen,
	Unreachable,
	Expect,
	Trap,
	// GCC's builtin form of a C library function: a function of its
	// signature called as the C function its name without `__builtin_`
	// names (the libm family in its double, float and long double
	// spellings).
	Library,
	// Builtins the call analysis checks and types itself and the lowering
	// expands in place (sema::Intrinsic); `variant` selects among a
	// family: FloatClassify isfinite, isinf, isnan, isnormal, fpclassify,
	// signbit, isinf_sign, issignaling; FloatCompare isgreater, isgreaterequal, isless,
	// islessequal, islessgreater, isunordered; FloatConstant 3 * (nan,
	// nans, inf) + (double, float, long double); BitCount 3 * (bswap, clz,
	// ctz, popcount) + the operand type by suffix (16/32/64, or unsigned,
	// long, long long); GenericBitCount clzg, ctzg, popcountg; VaList
	// start, end, copy; Overflow add, sub, mul; Atomic an index of
	// kGnuAtomicOperations; Sync an index of kGnuSyncOperations; C11Atomic
	// an index of kGnuC11Operations; SourceLocation FILE, LINE, FUNCTION,
	// COLUMN.
	FloatClassify,
	FloatCompare,
	FloatConstant,
	AddressOf,
	BitCount,
	GenericBitCount,
	Alloca,
	VaList,
	Prefetch,
	AssumeAligned,
	FltRounds,
	// GCC's __builtin_complex(re, im).
	Complex,
	// Clang's __builtin_reduce_and/or/xor/add/mul/max/min(vector), by
	// variant in that order.
	Reduce,
	// GCC's x86 lane builtins __builtin_ia32_vec_init/ext/set_<mode>: an
	// index of kGnuVectorLaneOperations.
	VectorLanes,
	Overflow,
	OperatorNew,
	OperatorDelete,
	Atomic,
	Sync,
	C11Atomic,
	// Queries the call analysis answers (ObjectSize: object_size and
	// dynamic_object_size, lowered as the answer, sema::Intrinsic).
	ConstantP,
	ObjectSize,
	Invoke,
	IsConstantEvaluated,
	SourceLocation
};

struct GnuBuiltinEntry
{
	GnuBuiltin builtin;
	uint8_t variant;
	// Function and Library: the type, result first and then the
	// parameters, one letter each -- v void, c char, i int, l long, x long
	// long, z size_t (unsigned long), f float, d double, e long double, V
	// __builtin_va_list, T the libm family's floating type, Z its complex
	// type -- `C` before a letter making it const and `*` after it a
	// pointer to it.
	const char* signature;
	// The libm family's floating type letter for `T` (d, f or e).
	char floating;
};

// The entry `spelling` names; false for none.
bool FindGnuBuiltin(const char* spelling, uint32_t length, GnuBuiltinEntry& entry);

// GCC's __atomic builtins by their names' suffixes, in sema::Intrinsic's
// order from AtomicLoadN (the lock-free queries last): the operand roles --
// p the object's address, r a pointer to a value of its type, v a value of
// its type, b bool, o a memory order, s a size -- and the result, the
// object's type (T), void (-), bool (B) or a constant (C).
struct GnuAtomicOperation
{
	const char* suffix;
	const char* roles;
	char result;
};
extern const GnuAtomicOperation kGnuAtomicOperations[];
extern const unsigned kGnuAtomicOperationCount;
// GCC's x86 builtins on a vector's lanes (MMX and SSE2, as its default
// target provides them): `init` makes a vector of its `lanes` scalar
// operands, `ext` reads the lane a constant selects, `set` gives the vector
// with that lane replaced. The lane type's letter: a signed char (the
// course reference's V8QI; GCC's is char), c char, s short, i int, x long
// long, f float, d double.
struct GnuVectorLaneOperation
{
	char kind;
	uint8_t lanes;
	char lane;
};
extern const GnuVectorLaneOperation kGnuVectorLaneOperations[];
// GCC's legacy __sync builtins: the __atomic operation each performs
// (an index of kGnuAtomicOperations, or kGnuSyncCompareSwap for the
// compare-and-swap forms), its operand roles and result as above, and the
// memory order it implies (GCC's encoding: 5 seq_cst, 2 acquire, 3
// release).
struct GnuSyncOperation
{
	const char* suffix;
	uint8_t atomic;
	const char* roles;
	char result;
	uint8_t order;
};
const uint8_t kGnuSyncCompareSwap = 0xff;
extern const GnuSyncOperation kGnuSyncOperations[];
extern const unsigned kGnuSyncOperationCount;
// Clang's __c11_atomic builtins on _Atomic objects: the __atomic operation
// each performs, roles and result as above (an object of any type of 1, 2,
// 4 or 8 bytes but for the arithmetic), the memory order it implies
// (init: relaxed, its encoding + 1; 0 when an operand gives it), and for
// a compare-exchange its weakness (0, 1; 0xff for none).
struct GnuC11Operation
{
	const char* suffix;
	uint8_t atomic;
	const char* roles;
	char result;
	uint8_t implied;
	uint8_t weak;
};
extern const GnuC11Operation kGnuC11Operations[];
extern const unsigned kGnuC11OperationCount;

// GCC's alternative keyword spellings, names reserved to the
// implementation (17.6.4.3.2): `__inline`, `__const__` ... spell the
// keyword `kind` (`__thread` thread_local); `__restrict`, `__restrict__` and `__extension__` are
// dropped (`dropped`). The parser reads them so; `__is_identifier` (a
// hosted preprocessor probe) answers that they are not identifiers.
struct GnuKeyword
{
	const char* spelling;
	TokenKind kind;
	bool dropped;
};
extern const GnuKeyword kGnuKeywords[];
extern const unsigned kGnuKeywordCount;
bool IsGnuKeyword(const char* spelling, uint32_t length);

// The GNU type traits and transforms, `__is_class ( type-id )` ... (GCC's
// and Clang's "Type Traits" extensions; C++11's 20.9.4 and 20.9.7 traits by
// name, and some of later standards').
enum class GnuTrait : uint8_t
{
	// One type-id: a bool constant about its shape alone, which an
	// incomplete class has too.
	IsClass,
	IsEnum,
	IsUnion,
	IsVoid,
	IsNullPointer,
	IsIntegral,
	IsFloatingPoint,
	IsArray,
	IsBoundedArray,
	IsUnboundedArray,
	IsPointer,
	IsLvalueReference,
	IsRvalueReference,
	IsReference,
	IsMemberObjectPointer,
	IsMemberFunctionPointer,
	IsMemberPointer,
	IsFunction,
	IsArithmetic,
	IsFundamental,
	IsObject,
	IsScalar,
	IsCompound,
	IsConst,
	IsVolatile,
	IsSigned,
	IsUnsigned,
	IsReferenceable,
	IsScopedEnum,
	// One type-id: a bool constant about a class that must be complete (or
	// an array of one).
	IsEmpty,
	IsPolymorphic,
	IsAbstract,
	IsFinal,
	IsAggregate,
	IsTrivial,
	IsTriviallyCopyable,
	HasUniqueObjectRepresentations,
	IsStandardLayout,
	IsPod,
	IsLiteralType,
	HasTrivialDestructor,
	HasVirtualDestructor,
	IsDestructible,
	IsNothrowDestructible,
	IsTriviallyDestructible,
	// GCC's pre-C++11 traits: the trivial or non-throwing default
	// constructor, copy constructor or copy assignment.
	HasTrivialConstructor,
	HasTrivialCopy,
	HasTrivialAssign,
	HasNothrowConstructor,
	HasNothrowCopy,
	HasNothrowAssign,
	// One type-id: Embarcadero's `__array_rank`, a std::size_t constant.
	ArrayRank,
	// `__builtin_offsetof ( type-id , member-designator )` (GCC's, behind
	// <cstddef>'s offsetof): a std::size_t constant; the designator -- an
	// identifier, then `. identifier` and `[ expression ]` parts -- is the
	// operand, member accesses and subscripts of an OffsetofBase.
	Offsetof,
	// Two type-ids.
	IsSame,
	IsBaseOf,
	IsAssignable,
	IsNothrowAssignable,
	IsTriviallyAssignable,
	IsConvertible,
	IsConvertibleTo,
	IsNothrowConvertible,
	ReferenceBindsToTemporary,
	ReferenceConstructsFromTemporary,
	ReferenceConvertsFromTemporary,
	// A type-id and any number more (pack expansions among them).
	IsConstructible,
	IsNothrowConstructible,
	IsTriviallyConstructible,
	IsInvocable,
	IsNothrowInvocable,
	// One type-id, a type: `__underlying_type` and the other type
	// transforms (Clang's set, C++11's 20.9.7 by name, and GCC's spelling
	// `__remove_reference`), parsed as the decltype-specifier of an operand
	// of the resulting type.
	UnderlyingType,
	AddLvalueReference,
	AddPointer,
	AddRvalueReference,
	Decay,
	MakeSigned,
	MakeUnsigned,
	RemoveAllExtents,
	RemoveConst,
	RemoveCv,
	RemoveCvref,
	RemoveExtent,
	RemovePointer,
	RemoveReference,
	RemoveReferenceGnu,
	RemoveRestrict,
	RemoveVolatile,
	// C11's `_Atomic ( type-name )` type specifier (Clang's C++ extension):
	// the atomic-qualified type (sema::kAtomic).
	AddAtomic,
	// C23's `_BitInt ( constant-expression )` (Clang's C++ extension; an
	// expression operand, `unsigned` before it): parsed, and dependent in a
	// template's definition; its types are outside PA29.
	BitInt,
	// `__integer_pack ( expression )`: the pack 0, 1, ..., N-1 of N's type,
	// the whole pattern of a template argument pack expansion.
	IntegerPack,
	// `__builtin_va_arg ( list , type-id )`: the next variadic argument of
	// the list (operand) as the type (pa28).
	VaArg,
	// Clang's `__builtin_convertvector ( expression , type-id )`: the vector
	// operand's lanes converted to the type-id's, a vector of as many lanes.
	ConvertVector,
	// `__builtin_bit_cast ( type-id , expression )` (GCC's and Clang's, for
	// std::bit_cast): the operand's bytes as a value of the type.
	BitCast,
	// Not spelled: an operand the analysis makes, a value of the type as
	// std::declval<T>() gives it (its category by the reference).
	Declval,
	// Not spelled: the operand of GNU `__typeof__ ( expression )`, parsed as
	// a decltype-specifier of it: a prvalue of the expression's type (cv
	// kept, no reference).
	TypeofOperand,
	// Not spelled: a prvalue of a non-reference type, the operand of a
	// temporary-binding trait (VAL<T>).
	DeclvalPrvalue,
	// Not spelled: an lvalue of an offsetof's type-id, its designator's
	// object.
	OffsetofBase,
	// GNU `__real__ e` and `__imag__ e` (also `__real`, `__imag`), unary
	// operators parsed as keywords (operand): a complex value's parts; of
	// a real operand, the operand and zero.
	RealPart,
	ImagPart
};

// The trait `spelling` names; false for none.
bool FindGnuTrait(const char* spelling, uint32_t length, GnuTrait& trait);
// A trait whose result is a type (`__underlying_type`, `__decay` ...).
inline bool IsGnuTypeTransform(GnuTrait trait)
{
	return trait >= GnuTrait::UnderlyingType && trait <= GnuTrait::BitInt;
}
// The number of type-ids the trait takes: 1, 2, or 0 for one or more.
uint32_t GnuTraitArity(GnuTrait trait);
// A trait about a class that requires it complete (14.7.1p1).
inline bool GnuTraitNeedsComplete(GnuTrait trait)
{
	return trait >= GnuTrait::IsEmpty && trait <= GnuTrait::HasNothrowAssign;
}
const char* GnuTraitName(GnuTrait trait);
