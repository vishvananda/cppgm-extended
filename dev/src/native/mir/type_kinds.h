#pragma once

#include <cstdint>

#include "lowir/types.h"

// Classes of the scalar type kinds MIR instructions carry, shared by
// selection and emission. Registers hold narrow integers normalized to 64
// bits: sign-extended for the signed narrow kinds, zero-extended for the
// unsigned ones.
namespace native {

// The bytes a typed MIR load, store or move of `kind` touches in memory.
inline unsigned register_bytes(lowir::TypeKind kind)
{
	switch (kind)
	{
	case lowir::TypeKind::I1:
	case lowir::TypeKind::I8:
	case lowir::TypeKind::U8:
		return 1;
	case lowir::TypeKind::I16:
	case lowir::TypeKind::U16:
		return 2;
	case lowir::TypeKind::I32:
	case lowir::TypeKind::U32:
	case lowir::TypeKind::F32:
		return 4;
	default:
		return 8;
	}
}

inline bool signed_narrow(lowir::TypeKind kind)
{
	return kind == lowir::TypeKind::I8 || kind == lowir::TypeKind::I16 || kind == lowir::TypeKind::I32;
}

inline bool unsigned_narrow(lowir::TypeKind kind)
{
	return kind == lowir::TypeKind::U8 || kind == lowir::TypeKind::U16 || kind == lowir::TypeKind::U32;
}

// Integers narrower than a register, including i1.
inline bool narrow_integer(lowir::TypeKind kind)
{
	return kind == lowir::TypeKind::I1 || signed_narrow(kind) || unsigned_narrow(kind);
}

inline bool signed_integer(lowir::TypeKind kind)
{
	return signed_narrow(kind) || kind == lowir::TypeKind::I64 || kind == lowir::TypeKind::I128;
}

// The low bits a non-negative integer of `kind` may occupy while its
// canonical register form (sign- or zero-extended by the type) is the value
// itself (0: not such an integer).
inline unsigned canonical_bits(lowir::TypeKind kind)
{
	switch (kind)
	{
	case lowir::TypeKind::I1:
		return 1;
	case lowir::TypeKind::I8:
		return 7;
	case lowir::TypeKind::U8:
		return 8;
	case lowir::TypeKind::I16:
		return 15;
	case lowir::TypeKind::U16:
		return 16;
	case lowir::TypeKind::I32:
		return 31;
	case lowir::TypeKind::U32:
		return 32;
	case lowir::TypeKind::I64:
		return 63;
	default:
		return 0;
	}
}

inline bool power_of_two(uint64_t value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

inline unsigned log2_of(uint64_t value)
{
	return value == 0 ? 0 : 63 - __builtin_clzll(value);
}

// The unsigned type of the same width (for zero-extension) and the signed
// one (for sign extension).
inline lowir::TypeKind unsigned_of(lowir::TypeKind kind)
{
	switch (kind)
	{
	case lowir::TypeKind::I8:
		return lowir::TypeKind::U8;
	case lowir::TypeKind::I16:
		return lowir::TypeKind::U16;
	case lowir::TypeKind::I32:
		return lowir::TypeKind::U32;
	default:
		return kind;
	}
}

inline lowir::TypeKind signed_of(lowir::TypeKind kind)
{
	switch (kind)
	{
	case lowir::TypeKind::U8:
		return lowir::TypeKind::I8;
	case lowir::TypeKind::U16:
		return lowir::TypeKind::I16;
	case lowir::TypeKind::U32:
		return lowir::TypeKind::I32;
	default:
		return kind;
	}
}

} // namespace native
