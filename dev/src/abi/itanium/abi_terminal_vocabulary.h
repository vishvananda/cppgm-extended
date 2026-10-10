#pragma once

// Semantic function-name terminals: constructor and destructor variants
// (5.1.4.3) and operator functions (5.1.3). Producers construct the kind
// directly; the PA9 fact adapter classifies its words once.
//
// Replaces the course's scaffold abi/itanium/abi_mangle_terminal.h, which ships to
// students unchanged. The ABI scaffold headers define some of these names
// differently, so the implementation includes none of them (the
// scaffold-header check enforces this).

#include <cstddef>
#include <cstdint>

namespace abi_mangle {

enum AbiTerminalKind : std::uint8_t
{
	ABI_TERMINAL_NONE,
	ABI_TERMINAL_CONSTRUCTOR_COMPLETE,
	ABI_TERMINAL_CONSTRUCTOR_BASE,
	ABI_TERMINAL_DESTRUCTOR_DELETING,
	ABI_TERMINAL_DESTRUCTOR_COMPLETE,
	ABI_TERMINAL_DESTRUCTOR_BASE,
	// Operators whose code depends on the operand count.
	ABI_TERMINAL_PLUS,
	ABI_TERMINAL_MINUS,
	// Operators with one fixed code.
	ABI_TERMINAL_UNARY_PLUS,
	ABI_TERMINAL_BINARY_PLUS,
	ABI_TERMINAL_UNARY_MINUS,
	ABI_TERMINAL_BINARY_MINUS,
	ABI_TERMINAL_ADDRESS_OF,
	ABI_TERMINAL_DEREFERENCE,
	ABI_TERMINAL_NEW,
	ABI_TERMINAL_NEW_ARRAY,
	ABI_TERMINAL_DELETE,
	ABI_TERMINAL_DELETE_ARRAY,
	ABI_TERMINAL_MULTIPLY,
	ABI_TERMINAL_DIVIDE,
	ABI_TERMINAL_REMAINDER,
	ABI_TERMINAL_BIT_AND,
	ABI_TERMINAL_BIT_OR,
	ABI_TERMINAL_BIT_XOR,
	ABI_TERMINAL_COMPLEMENT,
	ABI_TERMINAL_ASSIGN,
	ABI_TERMINAL_PLUS_ASSIGN,
	ABI_TERMINAL_MINUS_ASSIGN,
	ABI_TERMINAL_MULTIPLY_ASSIGN,
	ABI_TERMINAL_DIVIDE_ASSIGN,
	ABI_TERMINAL_REMAINDER_ASSIGN,
	ABI_TERMINAL_AND_ASSIGN,
	ABI_TERMINAL_OR_ASSIGN,
	ABI_TERMINAL_XOR_ASSIGN,
	ABI_TERMINAL_LEFT_SHIFT,
	ABI_TERMINAL_RIGHT_SHIFT,
	ABI_TERMINAL_LEFT_SHIFT_ASSIGN,
	ABI_TERMINAL_RIGHT_SHIFT_ASSIGN,
	ABI_TERMINAL_EQUAL,
	ABI_TERMINAL_NOT_EQUAL,
	ABI_TERMINAL_LESS,
	ABI_TERMINAL_GREATER,
	ABI_TERMINAL_LESS_EQUAL,
	ABI_TERMINAL_GREATER_EQUAL,
	ABI_TERMINAL_LOGICAL_NOT,
	ABI_TERMINAL_LOGICAL_AND,
	ABI_TERMINAL_LOGICAL_OR,
	ABI_TERMINAL_INCREMENT,
	ABI_TERMINAL_DECREMENT,
	ABI_TERMINAL_COMMA,
	ABI_TERMINAL_MEMBER_POINTER,
	ABI_TERMINAL_ARROW,
	ABI_TERMINAL_CALL,
	ABI_TERMINAL_INDEX,
	ABI_TERMINAL_COUNT
};

// The fact-file word for a terminal ("constructor-complete", "plus",
// "operator-call", ...), or false.
bool abi_find_terminal_kind(const char* word, std::size_t length, AbiTerminalKind* kind);
bool abi_terminal_is_constructor_or_destructor(AbiTerminalKind kind);
// The fixed-code kind an operator takes for `operand_count` operands
// (counting the implicit object of a member): PLUS/MINUS become the unary
// or binary kind; other kinds are returned unchanged.
AbiTerminalKind abi_terminal_for_operands(AbiTerminalKind kind, std::size_t operand_count);
// The Itanium code of a fixed-code kind: "C1", "D0", "pl", "cl", ...
const char* abi_terminal_code(AbiTerminalKind kind);

} // namespace abi_mangle
