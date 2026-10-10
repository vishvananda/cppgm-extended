#pragma once

#include <cstdint>

#include "sema/semantic_types.h"

namespace syntax {
struct LiteralExpr;
}

// The typed values of constant evaluation (5.19): scalars, pointers
// and references, arrays and class objects. The constant evaluator
// (sema/constant_interpreter.h) computes them; a variable's constant value
// is published into the semantic arena, where lookup, template arguments
// and the lowering read it by identity.
namespace sema {

struct DataMember;
struct VariableEntity;
struct FunctionEntity;

enum class ValueKind : uint8_t
{
	// An object not yet initialized (8.5p6-p11 leave it indeterminate):
	// reading it is not a constant expression.
	Indeterminate,
	// An integral or enumeration value (bits as Constant keeps them).
	Integer,
	// A floating value in its type's precision.
	Floating,
	// A pointer, or the object or function a reference is bound to.
	Pointer,
	// A pointer to member (5.3.1p3, 4.11): `address.entity` the member (a
	// non-static data member or member function; null for the null
	// value), `address.offset` the offset of the member's class subobject
	// in the pointer's class (the this-adjustment; a non-anonymous class
	// holding a data member).
	MemberPointer,
	// An array's elements, or a class object's subobjects: its direct
	// bases in declaration order, then its non-static data members by
	// ordinal (a union's members, one of them active).
	Array,
	Class
};

// The storage a pointer designates.
enum class RootKind : uint8_t
{
	// The null pointer value (4.10).
	Null,
	// A variable: its constant value, when it has one (the one being
	// initialized, by the evaluation itself).
	Variable,
	// A string literal's array (2.14.5p8).
	Literal,
	// A function (a pointer to it, or a reference bound to it).
	Function,
	// An object the evaluation created: a parameter, an automatic variable
	// or a temporary of a constexpr call being evaluated.
	Object,
	// The temporary a variable's reference binds and extends (12.2p5): its
	// published value (`entity`), stated as its own object by the lowering.
	Temporary,
	// The std::type_info object a typeid of a static type designates
	// (5.2.8p4-p5, an address constant expression, 5.19p3): `entity` is the
	// type, canonical and cv-unqualified. It is never read here.
	TypeInfo
};

// ConstValue::flags.
enum : uint8_t
{
	// Zero-initialized as part of a value-initialization (8.5p5, p7), not
	// by an initializer of its own (the lowering's presentation of such
	// bytes).
	kValueZeroed = 1,
	// A pointer one past the end of its array (or of an object as an array
	// of one, 5.7p4): no object is there to read (its address may be
	// another's); arithmetic moves within the array before it.
	kPastEnd = 2
};

// A glvalue's designation: the root storage, the byte offset of the
// designated object in it, and the object's type (cv-qualified). An
// evaluation object is identified by its slot and the slot's generation,
// so a reference that outlives its object is recognized. A bit-field's
// glvalue designates the member `bit_field` of the class object at
// `offset`.
struct ConstPlace
{
	RootKind root;
	// One past the end of an array (kPastEnd).
	bool past_end;
	uint32_t object;
	uint32_t generation;
	// The innermost union member a member access named on the way (null
	// for none or a place made otherwise) and the offset of that union's
	// object (within an object's bounded storage): a read must find it
	// active (5.19p2), which the object's offset and type alone cannot
	// tell from a member sharing both.
	uint32_t variant_offset;
	const void* entity;
	int64_t offset;
	const Type* type;
	const VariableEntity* bit_field;
	const DataMember* variant;
};

// One value. `type` is the value's unqualified type (the pointer type of a
// pointer, the referenced type's reference for a bound reference member).
struct ConstValue
{
	ValueKind kind;
	RootKind root;
	uint8_t flags;
	// Array and Class: the number of elements; Pointer to an evaluation
	// object: its slot.
	uint32_t count;
	// Pointer to an evaluation object: the slot's generation; a union: its
	// active member's index plus one (0 for none).
	uint32_t extra;
	const Type* type;
	// Integer of a 128-bit type: the value's high 64 bits (`bits` the low
	// ones; see WideOf). Unused for narrower types.
	uint64_t high;
	union
	{
		uint64_t bits;
		long double floating;
		ConstValue* elements;
		struct
		{
			const void* entity;
			int64_t offset;
		} address;
	};
};

inline ConstPlace PlaceOf(const ConstValue& pointer, const Type* designated)
{
	ConstPlace place = ConstPlace();
	place.root = pointer.root;
	place.past_end = (pointer.flags & kPastEnd) != 0;
	place.object = pointer.count;
	place.generation = pointer.extra;
	place.entity = pointer.address.entity;
	place.offset = pointer.address.offset;
	place.type = designated;
	return place;
}

inline ConstValue PointerTo(const ConstPlace& place, const Type* type)
{
	ConstValue value = ConstValue();
	value.kind = ValueKind::Pointer;
	value.root = place.root;
	value.flags = place.past_end ? kPastEnd : 0;
	value.count = place.object;
	value.extra = place.generation;
	value.type = type;
	value.address.entity = place.entity;
	value.address.offset = place.offset;
	return value;
}

// A pointer template argument (PA18, a TypeKind::Value of pointer or
// pointer to member type) as the pointer value it is
// (sema/template_addresses.cpp).
ConstValue ArgumentPointer(const Type* argument);

inline ConstValue NullMemberPointer(const Type* type)
{
	ConstValue value = ConstValue();
	value.kind = ValueKind::MemberPointer;
	value.type = type;
	return value;
}

// The high half of a value of `type` whose low 64 bits, extended by the
// type's signedness, are `bits` (0 for a type of at most 64 bits).
uint64_t WideHigh(const Type* type, uint64_t bits);

inline ConstValue IntegerValue(const Type* type, uint64_t bits)
{
	ConstValue value = ConstValue();
	value.kind = ValueKind::Integer;
	value.type = type;
	value.bits = bits;
	// Only a 128-bit type (or an enumeration, through its underlying type)
	// has a high half.
	const Type* bare = type != nullptr ? type->unqualified : nullptr;
	if (bare != nullptr &&
		(bare->kind == TypeKind::Enum ||
			(bare->kind == TypeKind::Fundamental &&
				(bare->fundamental == FundamentalType::Int128 ||
					bare->fundamental == FundamentalType::UnsignedInt128))))
		value.high = WideHigh(type, bits);
	return value;
}

// A 128-bit integer value (GCC's unsigned __int128 in this implementation)
// of an integral type's constant: a 128-bit type's whole value, another's
// extended by its signedness.
typedef unsigned __int128 WideBits;

// An integer value a Constant holds: any of a narrower type, a 128-bit
// type's whose high half extends its low one (sema/integral_arithmetic.h).
inline bool HeldAsConstant(const ConstValue& value)
{
	return value.high == WideHigh(value.type, value.bits);
}

inline WideBits WideOf(const ConstValue& value)
{
	return (static_cast<WideBits>(value.high) << 64) | value.bits;
}

inline ConstValue WideValue(const Type* type, WideBits bits)
{
	ConstValue value = IntegerValue(type, static_cast<uint64_t>(bits));
	value.high = static_cast<uint64_t>(bits >> 64);
	return value;
}

inline ConstValue FloatingValue(const Type* type, long double floating)
{
	ConstValue value = ConstValue();
	value.kind = ValueKind::Floating;
	value.type = type;
	value.floating = floating;
	return value;
}

// A GNU vector whose constant value is held lane by lane: an Array value of
// the vector type with one element per lane (as a complex value holds its
// two parts). A bool vector's packed lanes, and a vector whose size depends
// on template parameters, are not (their values stay opaque).
inline bool LaneVector(const Type* type)
{
	type = type->unqualified;
	if (type->kind != TypeKind::Vector || type->owner != nullptr || type->bound == 0)
		return false;
	const Type* lane = type->element->unqualified;
	return lane->kind != TypeKind::Fundamental || lane->fundamental != FundamentalType::Bool;
}

} // namespace sema
