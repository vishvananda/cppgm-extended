// Read-only Itanium vocabulary tables: builtin type codes, standard
// abbreviations, terminal codes and expression operation codes.

#include "abi/itanium/abi_graph.h"
#include "abi/itanium/abi_expression_vocabulary.h"
#include "abi/itanium/abi_terminal_vocabulary.h"
#include "abi/itanium/abi_type_vocabulary.h"

#include <cstring>

namespace abi_mangle {

namespace {

bool word_equals(const char* word, std::size_t length, const char* text)
{
	return std::strlen(text) == length && std::memcmp(word, text, length) == 0;
}

struct BuiltinEntry
{
	const char* word;
	const char* code;
	unsigned integral_bits;
	bool is_unsigned;
};

// Indexed by AbiBuiltinTypeKind (abi_graph.h's extensions last).
const BuiltinEntry kBuiltins[kAbiBuiltinTypeCount] = {
	{ "", "", 0, false },
	{ "void", "v", 0, false },
	{ "bool", "b", 1, true },
	{ "char", "c", 8, false },
	{ "schar", "a", 8, false },
	{ "uchar", "h", 8, true },
	{ "wchar", "w", 32, false },
	{ "char16", "Ds", 16, true },
	{ "char32", "Di", 32, true },
	{ "short", "s", 16, false },
	{ "ushort", "t", 16, true },
	{ "int", "i", 32, false },
	{ "uint", "j", 32, true },
	{ "long", "l", 64, false },
	{ "ulong", "m", 64, true },
	{ "longlong", "x", 64, false },
	{ "ulonglong", "y", 64, true },
	{ "int128", "n", 0, false },
	{ "uint128", "o", 0, true },
	{ "float", "f", 0, false },
	{ "double", "d", 0, false },
	{ "longdouble", "e", 0, false },
	{ "float16", "DF16_", 0, false },
	{ "float32", "DF32_", 0, false },
	{ "float32x", "DF32x", 0, false },
	{ "float64", "DF64_", 0, false },
	{ "float64x", "DF64x", 0, false },
	{ "stdfloat128", "DF128_", 0, false },
	{ "float128", "g", 0, false },
	{ "complex-float", "Cf", 0, false },
	{ "complex-double", "Cd", 0, false },
	{ "complex-longdouble", "Ce", 0, false },
	{ "nullptr", "Dn", 0, false },
	{ "bfloat16", "DF16b", 0, false },
};

const char* const kStandardSubstitutionCodes[ABI_STANDARD_SUBSTITUTION_COUNT] = {
	"",
	"Sa",
	"Sb",
	"Ss",
	"Si",
	"So",
	"Sd",
};

struct TerminalEntry
{
	const char* word;
	const char* code;
};

// Indexed by AbiTerminalKind. PLUS and MINUS have no fixed code.
const TerminalEntry kTerminals[ABI_TERMINAL_COUNT] = {
	{ "", "" },
	{ "constructor-complete", "C1" },
	{ "constructor-base", "C2" },
	{ "destructor-deleting", "D0" },
	{ "destructor-complete", "D1" },
	{ "destructor-base", "D2" },
	{ "plus", "" },
	{ "minus", "" },
	{ "unary-plus", "ps" },
	{ "binary-plus", "pl" },
	{ "unary-minus", "ng" },
	{ "binary-minus", "mi" },
	{ "address-of", "ad" },
	{ "deref", "de" },
	{ "new", "nw" },
	{ "new-array", "na" },
	{ "delete", "dl" },
	{ "delete-array", "da" },
	{ "multiply", "ml" },
	{ "divide", "dv" },
	{ "remainder", "rm" },
	{ "bit-and", "an" },
	{ "bit-or", "or" },
	{ "bit-xor", "eo" },
	{ "complement", "co" },
	{ "assign", "aS" },
	{ "plus-assign", "pL" },
	{ "minus-assign", "mI" },
	{ "multiply-assign", "mL" },
	{ "divide-assign", "dV" },
	{ "remainder-assign", "rM" },
	{ "and-assign", "aN" },
	{ "or-assign", "oR" },
	{ "xor-assign", "eO" },
	{ "left-shift", "ls" },
	{ "right-shift", "rs" },
	{ "left-shift-assign", "lS" },
	{ "right-shift-assign", "rS" },
	{ "equal", "eq" },
	{ "not-equal", "ne" },
	{ "less", "lt" },
	{ "greater", "gt" },
	{ "less-equal", "le" },
	{ "greater-equal", "ge" },
	{ "logical-not", "nt" },
	{ "logical-and", "aa" },
	{ "logical-or", "oo" },
	{ "increment", "pp" },
	{ "decrement", "mm" },
	{ "comma", "cm" },
	{ "member-pointer", "pm" },
	{ "arrow", "pt" },
	{ "call", "cl" },
	{ "index", "ix" },
};

struct OperationEntry
{
	char code[3];
	unsigned char shapes;
};

// 5.1.3 operator codes and the 5.1.6 expression codes that take the listed
// operand shapes.
const OperationEntry kOperations[] = {
	{ "ps", ABI_EXPRESSION_SHAPE_UNARY },
	{ "ng", ABI_EXPRESSION_SHAPE_UNARY },
	{ "ad", ABI_EXPRESSION_SHAPE_UNARY },
	{ "de", ABI_EXPRESSION_SHAPE_UNARY },
	{ "co", ABI_EXPRESSION_SHAPE_UNARY },
	{ "nt", ABI_EXPRESSION_SHAPE_UNARY },
	{ "pp", ABI_EXPRESSION_SHAPE_UNARY },
	{ "mm", ABI_EXPRESSION_SHAPE_UNARY },
	{ "sz", ABI_EXPRESSION_SHAPE_UNARY },
	{ "az", ABI_EXPRESSION_SHAPE_UNARY },
	{ "te", ABI_EXPRESSION_SHAPE_UNARY },
	{ "nx", ABI_EXPRESSION_SHAPE_UNARY },
	{ "tw", ABI_EXPRESSION_SHAPE_UNARY },
	{ "dl", ABI_EXPRESSION_SHAPE_UNARY },
	{ "da", ABI_EXPRESSION_SHAPE_UNARY },
	{ "sp", ABI_EXPRESSION_SHAPE_UNARY },
	{ "sZ", ABI_EXPRESSION_SHAPE_UNARY },
	{ "pl", ABI_EXPRESSION_SHAPE_BINARY },
	{ "mi", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ml", ABI_EXPRESSION_SHAPE_BINARY },
	{ "dv", ABI_EXPRESSION_SHAPE_BINARY },
	{ "rm", ABI_EXPRESSION_SHAPE_BINARY },
	{ "an", ABI_EXPRESSION_SHAPE_BINARY },
	{ "or", ABI_EXPRESSION_SHAPE_BINARY },
	{ "eo", ABI_EXPRESSION_SHAPE_BINARY },
	{ "aS", ABI_EXPRESSION_SHAPE_BINARY },
	{ "pL", ABI_EXPRESSION_SHAPE_BINARY },
	{ "mI", ABI_EXPRESSION_SHAPE_BINARY },
	{ "mL", ABI_EXPRESSION_SHAPE_BINARY },
	{ "dV", ABI_EXPRESSION_SHAPE_BINARY },
	{ "rM", ABI_EXPRESSION_SHAPE_BINARY },
	{ "aN", ABI_EXPRESSION_SHAPE_BINARY },
	{ "oR", ABI_EXPRESSION_SHAPE_BINARY },
	{ "eO", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ls", ABI_EXPRESSION_SHAPE_BINARY },
	{ "rs", ABI_EXPRESSION_SHAPE_BINARY },
	{ "lS", ABI_EXPRESSION_SHAPE_BINARY },
	{ "rS", ABI_EXPRESSION_SHAPE_BINARY },
	{ "eq", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ne", ABI_EXPRESSION_SHAPE_BINARY },
	{ "lt", ABI_EXPRESSION_SHAPE_BINARY },
	{ "gt", ABI_EXPRESSION_SHAPE_BINARY },
	{ "le", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ge", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ss", ABI_EXPRESSION_SHAPE_BINARY },
	{ "aa", ABI_EXPRESSION_SHAPE_BINARY },
	{ "oo", ABI_EXPRESSION_SHAPE_BINARY },
	{ "cm", ABI_EXPRESSION_SHAPE_BINARY },
	{ "pm", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ds", ABI_EXPRESSION_SHAPE_BINARY },
	{ "ix", ABI_EXPRESSION_SHAPE_BINARY },
	{ "qu", ABI_EXPRESSION_SHAPE_TERNARY },
	{ "sc", ABI_EXPRESSION_SHAPE_CAST },
	{ "dc", ABI_EXPRESSION_SHAPE_CAST },
	{ "cc", ABI_EXPRESSION_SHAPE_CAST },
	{ "rc", ABI_EXPRESSION_SHAPE_CAST },
	{ "cv", ABI_EXPRESSION_SHAPE_CAST },
	{ "dt", ABI_EXPRESSION_SHAPE_MEMBER_ACCESS },
	{ "pt", ABI_EXPRESSION_SHAPE_MEMBER_ACCESS },
};

} // namespace

AbiBuiltinTypeKind abi_builtin_type_kind(const char* word, std::size_t length)
{
	for (unsigned kind = ABI_BUILTIN_TYPE_VOID; kind < kAbiBuiltinTypeCount; ++kind)
	{
		if (word_equals(word, length, kBuiltins[kind].word))
			return static_cast<AbiBuiltinTypeKind>(kind);
	}
	return ABI_BUILTIN_TYPE_NONE;
}

const char* abi_builtin_type_code(AbiBuiltinTypeKind kind)
{
	return kBuiltins[kind].code;
}

const char* abi_builtin_type_word(AbiBuiltinTypeKind kind)
{
	return kBuiltins[kind].word;
}

bool abi_builtin_type_is_substitutable(AbiBuiltinTypeKind kind)
{
	return kind == ABI_BUILTIN_TYPE_COMPLEX_FLOAT || kind == ABI_BUILTIN_TYPE_COMPLEX_DOUBLE ||
		kind == ABI_BUILTIN_TYPE_COMPLEX_LONG_DOUBLE;
}

bool abi_builtin_type_is_unsigned(AbiBuiltinTypeKind kind)
{
	return kBuiltins[kind].is_unsigned;
}

unsigned abi_builtin_type_integral_bits(AbiBuiltinTypeKind kind)
{
	return kBuiltins[kind].integral_bits;
}

AbiStandardSubstitutionKind abi_standard_substitution_kind(const char* word, std::size_t length)
{
	for (unsigned kind = ABI_STANDARD_SUBSTITUTION_ALLOCATOR; kind < ABI_STANDARD_SUBSTITUTION_COUNT; ++kind)
	{
		if (word_equals(word, length, kStandardSubstitutionCodes[kind]))
			return static_cast<AbiStandardSubstitutionKind>(kind);
	}
	return ABI_STANDARD_SUBSTITUTION_NONE;
}

const char* abi_standard_substitution_code(AbiStandardSubstitutionKind kind)
{
	return kStandardSubstitutionCodes[kind];
}

bool abi_find_terminal_kind(const char* word, std::size_t length, AbiTerminalKind* kind)
{
	if (word_equals(word, length, "operator-call"))
	{
		*kind = ABI_TERMINAL_CALL;
		return true;
	}
	for (unsigned candidate = ABI_TERMINAL_CONSTRUCTOR_COMPLETE; candidate < ABI_TERMINAL_COUNT; ++candidate)
	{
		if (word_equals(word, length, kTerminals[candidate].word))
		{
			*kind = static_cast<AbiTerminalKind>(candidate);
			return true;
		}
	}
	return false;
}

bool abi_terminal_is_constructor_or_destructor(AbiTerminalKind kind)
{
	return kind >= ABI_TERMINAL_CONSTRUCTOR_COMPLETE && kind <= ABI_TERMINAL_DESTRUCTOR_BASE;
}

AbiTerminalKind abi_terminal_for_operands(AbiTerminalKind kind, std::size_t operand_count)
{
	if (kind == ABI_TERMINAL_PLUS)
		return operand_count == 1 ? ABI_TERMINAL_UNARY_PLUS : ABI_TERMINAL_BINARY_PLUS;
	if (kind == ABI_TERMINAL_MINUS)
		return operand_count == 1 ? ABI_TERMINAL_UNARY_MINUS : ABI_TERMINAL_BINARY_MINUS;
	// 5.1.3: unary `*` and `&` are `de` and `ad`.
	if (kind == ABI_TERMINAL_MULTIPLY && operand_count == 1)
		return ABI_TERMINAL_DEREFERENCE;
	if (kind == ABI_TERMINAL_BIT_AND && operand_count == 1)
		return ABI_TERMINAL_ADDRESS_OF;
	return kind;
}

const char* abi_terminal_code(AbiTerminalKind kind)
{
	return kTerminals[kind].code;
}

std::uint16_t abi_expression_operation_code(const char* word, std::size_t length, AbiExpressionShape shape)
{
	if (length != 2)
		return 0;
	for (const OperationEntry& entry : kOperations)
	{
		if (entry.code[0] == word[0] && entry.code[1] == word[1] && (entry.shapes & shape) != 0)
			return abi_pack_operation_code(word[0], word[1]);
	}
	return 0;
}

} // namespace abi_mangle
