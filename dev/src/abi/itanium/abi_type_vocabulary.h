#pragma once

// Fixed vocabulary of ABI type facts: builtin types and the Itanium
// standard-substitution abbreviations (5.1.5.2, 5.1.10). Producers build
// these kinds directly; the PA9 fact adapter maps its words onto them once.
//
// Replaces the course's scaffold abi/itanium/abi_mangle_type_vocabulary.h, which ships to
// students unchanged. The ABI scaffold headers define some of these names
// differently, so the implementation includes none of them (the
// scaffold-header check enforces this).

#include <cstddef>
#include <cstdint>

namespace abi_mangle {

enum AbiBuiltinTypeKind : std::uint8_t
{
	ABI_BUILTIN_TYPE_NONE,
	ABI_BUILTIN_TYPE_VOID,
	ABI_BUILTIN_TYPE_BOOL,
	ABI_BUILTIN_TYPE_CHAR,
	ABI_BUILTIN_TYPE_SIGNED_CHAR,
	ABI_BUILTIN_TYPE_UNSIGNED_CHAR,
	ABI_BUILTIN_TYPE_WCHAR,
	ABI_BUILTIN_TYPE_CHAR16,
	ABI_BUILTIN_TYPE_CHAR32,
	ABI_BUILTIN_TYPE_SHORT,
	ABI_BUILTIN_TYPE_UNSIGNED_SHORT,
	ABI_BUILTIN_TYPE_INT,
	ABI_BUILTIN_TYPE_UNSIGNED_INT,
	ABI_BUILTIN_TYPE_LONG,
	ABI_BUILTIN_TYPE_UNSIGNED_LONG,
	ABI_BUILTIN_TYPE_LONG_LONG,
	ABI_BUILTIN_TYPE_UNSIGNED_LONG_LONG,
	ABI_BUILTIN_TYPE_INT128,
	ABI_BUILTIN_TYPE_UINT128,
	ABI_BUILTIN_TYPE_FLOAT,
	ABI_BUILTIN_TYPE_DOUBLE,
	ABI_BUILTIN_TYPE_LONG_DOUBLE,
	ABI_BUILTIN_TYPE_FLOAT16,
	ABI_BUILTIN_TYPE_FLOAT32,
	ABI_BUILTIN_TYPE_FLOAT32X,
	ABI_BUILTIN_TYPE_FLOAT64,
	ABI_BUILTIN_TYPE_FLOAT64X,
	ABI_BUILTIN_TYPE_STDFLOAT128,
	ABI_BUILTIN_TYPE_FLOAT128,
	ABI_BUILTIN_TYPE_COMPLEX_FLOAT,
	ABI_BUILTIN_TYPE_COMPLEX_DOUBLE,
	ABI_BUILTIN_TYPE_COMPLEX_LONG_DOUBLE,
	ABI_BUILTIN_TYPE_NULLPTR,
	ABI_BUILTIN_TYPE_COUNT
};

enum AbiStandardSubstitutionKind : std::uint8_t
{
	ABI_STANDARD_SUBSTITUTION_NONE,
	ABI_STANDARD_SUBSTITUTION_ALLOCATOR,    // Sa ::std::allocator
	ABI_STANDARD_SUBSTITUTION_BASIC_STRING, // Sb ::std::basic_string
	ABI_STANDARD_SUBSTITUTION_STRING,       // Ss ::std::basic_string<char, ...>
	ABI_STANDARD_SUBSTITUTION_ISTREAM,      // Si ::std::basic_istream<char, ...>
	ABI_STANDARD_SUBSTITUTION_OSTREAM,      // So ::std::basic_ostream<char, ...>
	ABI_STANDARD_SUBSTITUTION_IOSTREAM,     // Sd ::std::basic_iostream<char, ...>
	ABI_STANDARD_SUBSTITUTION_COUNT
};

// The fact-file word for a builtin, or ABI_BUILTIN_TYPE_NONE.
AbiBuiltinTypeKind abi_builtin_type_kind(const char* word, std::size_t length);
const char* abi_builtin_type_code(AbiBuiltinTypeKind kind);
const char* abi_builtin_type_word(AbiBuiltinTypeKind kind);
// C <type> (complex) builtins are compound types for substitution
// (5.1.10 excludes only <builtin-type>); the others are never candidates.
bool abi_builtin_type_is_substitutable(AbiBuiltinTypeKind kind);
bool abi_builtin_type_is_unsigned(AbiBuiltinTypeKind kind);
// Width in bits of an integral builtin, 0 for other kinds.
unsigned abi_builtin_type_integral_bits(AbiBuiltinTypeKind kind);

// The abbreviation for a two-letter code such as "Sa", or
// ABI_STANDARD_SUBSTITUTION_NONE.
AbiStandardSubstitutionKind abi_standard_substitution_kind(const char* word, std::size_t length);
const char* abi_standard_substitution_code(AbiStandardSubstitutionKind kind);

} // namespace abi_mangle
