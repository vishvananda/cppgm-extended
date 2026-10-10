#pragma once

#include <cstdint>

namespace lowir {

// LowIR value and storage types (lowir.md "Types"). The scalar kinds carry
// no payload; `obj<bytes x align>` carries its size and alignment. A Type is
// an 8-byte value compared field by field, so equality needs no interning.
enum class TypeKind : uint8_t
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
	Obj
};

struct Type
{
	uint64_t kind_bits : 8;
	uint64_t align_log2 : 8;
	uint64_t bytes : 48;

	TypeKind kind() const { return static_cast<TypeKind>(kind_bits); }

	static Type scalar(TypeKind kind)
	{
		Type type;
		type.kind_bits = static_cast<uint64_t>(kind);
		type.align_log2 = 0;
		type.bytes = 0;
		return type;
	}

	// `align_log2` is the base-2 logarithm of the alignment.
	static Type object(uint64_t bytes, unsigned align_log2)
	{
		Type type;
		type.kind_bits = static_cast<uint64_t>(TypeKind::Obj);
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

// Bits of an integer kind (i1 is one bit); 0 for other kinds.
unsigned integer_bits(TypeKind kind);

// 1, 2, 3 for f32, f64, f80; 0 for other kinds.
unsigned float_rank(TypeKind kind);

// The alignment of a value of `type` in memory (a data item's or a storage
// type's): an object's own, a scalar's size (16 for f80's storage).
unsigned data_alignment(Type type);

// The spelling of a non-object kind, such as "i64" or "void".
const char* scalar_type_spelling(TypeKind kind);

} // namespace lowir
