#pragma once

// Operation codes of dependent expressions (5.1.6). An operation is kept as
// its two-character Itanium code packed into 16 bits; the table below
// validates each code against the operand shape it is used with.
//
// Replaces the course's scaffold abi/itanium/abi_mangle_expression.h, which ships to
// students unchanged. The ABI scaffold headers define some of these names
// differently, so v4opt includes none of them (scripts/check_scaffold_headers.sh).

#include <cstddef>
#include <cstdint>

namespace abi_mangle {

enum AbiExpressionShape : std::uint8_t
{
	ABI_EXPRESSION_SHAPE_UNARY = 1,
	ABI_EXPRESSION_SHAPE_BINARY = 2,
	ABI_EXPRESSION_SHAPE_TERNARY = 4,
	ABI_EXPRESSION_SHAPE_CAST = 8,          // <code> <type> <expression>
	ABI_EXPRESSION_SHAPE_MEMBER_ACCESS = 16 // <code> <expression> <unresolved-name>
};

inline std::uint16_t abi_pack_operation_code(char first, char second)
{
	return static_cast<std::uint16_t>((static_cast<unsigned char>(first) << 8) | static_cast<unsigned char>(second));
}

// The packed code for a two-character spelling used with `shape`, or 0 when
// the code is not an Itanium operation of that shape.
std::uint16_t abi_expression_operation_code(const char* word, std::size_t length, AbiExpressionShape shape);

} // namespace abi_mangle
