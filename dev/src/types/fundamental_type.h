#pragma once

#include <cstdint>

// The fundamental types of [basic.fundamental] 3.9.1.
enum class FundamentalType : uint8_t
{
	// 3.9.1p2
	SignedChar,
	ShortInt,
	Int,
	LongInt,
	LongLongInt,

	// 3.9.1p3
	UnsignedChar,
	UnsignedShortInt,
	UnsignedInt,
	UnsignedLongInt,
	UnsignedLongLongInt,

	// 3.9.1p1, 3.9.1p5
	WcharT,
	Char,
	Char16T,
	Char32T,

	// 3.9.1p6
	Bool,

	// 3.9.1p8
	Float,
	Double,
	LongDouble,

	// 3.9.1p9
	Void,

	// 3.9.1p10
	NullptrT,

	// GCC's 128-bit integer extension (__int128, unsigned __int128): an
	// extended signed integer type and its unsigned counterpart (3.9.1p2,
	// p3), ranked above long long (4.13).
	Int128,
	UnsignedInt128,

	// GCC's extended floating types (PA29; C++23's 6.8.3 as GCC implements
	// them in every mode): _Float16 (binary16), _Float32, _Float64,
	// _Float128 (binary128), _Float32x (binary64), _Float64x (x87's
	// extended format), and the GNU __float128 (binary128, distinct from
	// _Float128). Each is distinct from the standard floating types.
	Float16,
	Float32,
	Float64,
	Float128,
	Float32x,
	Float64x,
	GnuFloat128,

	// Clang's __bf16 (x86-64, PA34): bfloat16 storage (2 bytes, 2-byte
	// aligned: an 8-bit exponent and an 8-bit significand), whose
	// arithmetic is performed in float; of a lower floating-point rank
	// than _Float16, and converting to or from it only by the usual
	// arithmetic conversions.
	BFloat16
};

const unsigned kFundamentalTypeCount = static_cast<unsigned>(FundamentalType::BFloat16) + 1;

// The type's spelling as a simple-type-specifier sequence, e.g. "unsigned long int".
const char* FundamentalTypeName(FundamentalType type);

// sizeof(type) under the System V AMD64 ABI (Table 3.1); 0 for void.
unsigned FundamentalTypeSize(FundamentalType type);

// A standard or extended floating type.
bool IsFloatingFundamental(FundamentalType type);
// The standard floating type of a floating type's representation (float,
// double or long double), or _Float16, _Float128 and __bf16 for binary16,
// binary128 and bfloat16, which no standard type has.
FundamentalType FloatingRepresentation(FundamentalType type);
// The floating-point conversion rank as GCC and Clang order it (1
// bfloat16, 2 binary16, 3 float, 4 double, 5 long double, 6 binary128; 0
// for a non-floating type); an extended type has the greater subrank of an
// equal rank (C++23 6.8.6p2).
unsigned FloatingRank(FundamentalType type);
