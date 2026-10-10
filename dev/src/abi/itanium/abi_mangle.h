#pragma once

// The Itanium C++ ABI naming vocabulary (doc/itanium-mangling.txt, 5.1) and
// the words of PA9's ABI fact files, each as an enum with its codes and
// spellings.  The course solution builds on this header exactly as it is
// shipped: its `abimangle` fact reader classifies words with the fact
// tables below, and its encoder (shared with the compiler, which builds ABI
// targets directly and never writes fact text) spells names with the
// Itanium tables.
//
// What is deliberately not here: the typed model of an ABI entity (names,
// types, template arguments, expressions), the substitution dictionary and
// the encoder.  Those are PA9's design work.  Keep the model structured
// (pa9/README.md "Design Notes"): store these enums, not fact words or
// mangled fragments, and compare structure, not spellings.
//
// Conventions: the Itanium vocabulary has abi_* functions (the names the
// earlier scaffold declared): *_code() gives the Itanium code, *_word() the
// fact word, abi_*_kind() / abi_find_*() read a word.  The fact-file enums
// use spelling(value) and from_spelling(text, length, &value), as the LowIR
// scaffolds do.

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace abi_mangle {

namespace detail {

inline bool same_word(const char * word, const char * text, std::size_t length)
{
  return std::strlen(word) == length && std::memcmp(word, text, length) == 0;
}

template <typename E>
bool find_word(const char * text, std::size_t length, unsigned first, unsigned count, E * value)
{
  for(unsigned i = first; i < count; ++i) {
    const char * word = spelling(static_cast<E>(i));
    if(word[0] != '\0' && same_word(word, text, length)) {
      *value = static_cast<E>(i);
      return true;
    }
  }
  return false;
}

}  // namespace detail

// ===========================================================================
// Itanium vocabulary

// <builtin-type> (5.1.5.2) and the GNU/vendor builtins the course names.
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
  ABI_BUILTIN_TYPE_FLOAT128,  // GNU __float128
  ABI_BUILTIN_TYPE_COMPLEX_FLOAT,
  ABI_BUILTIN_TYPE_COMPLEX_DOUBLE,
  ABI_BUILTIN_TYPE_COMPLEX_LONG_DOUBLE,
  ABI_BUILTIN_TYPE_NULLPTR,
  ABI_BUILTIN_TYPE_BFLOAT16,  // __bf16, std::bfloat16_t
  ABI_BUILTIN_TYPE_COUNT
};

struct AbiBuiltinTypeEntry
{
  const char * word;       // the fact-file word: "int", "ulonglong", ...
  const char * code;       // the Itanium code: "i", "y", ...
  unsigned integral_bits;  // the width of an integral type; 0 otherwise
  bool is_unsigned;
};

inline const AbiBuiltinTypeEntry & abi_builtin_type_entry(AbiBuiltinTypeKind kind)
{
  static const AbiBuiltinTypeEntry kBuiltins[ABI_BUILTIN_TYPE_COUNT] = {
    { "", "", 0, false }, { "void", "v", 0, false }, { "bool", "b", 1, true },
    { "char", "c", 8, false }, { "schar", "a", 8, false }, { "uchar", "h", 8, true },
    { "wchar", "w", 32, false }, { "char16", "Ds", 16, true }, { "char32", "Di", 32, true },
    { "short", "s", 16, false }, { "ushort", "t", 16, true }, { "int", "i", 32, false },
    { "uint", "j", 32, true }, { "long", "l", 64, false }, { "ulong", "m", 64, true },
    { "longlong", "x", 64, false }, { "ulonglong", "y", 64, true }, { "int128", "n", 0, false },
    { "uint128", "o", 0, true }, { "float", "f", 0, false }, { "double", "d", 0, false },
    { "longdouble", "e", 0, false }, { "float16", "DF16_", 0, false },
    { "float32", "DF32_", 0, false }, { "float32x", "DF32x", 0, false },
    { "float64", "DF64_", 0, false }, { "float64x", "DF64x", 0, false },
    { "stdfloat128", "DF128_", 0, false }, { "float128", "g", 0, false },
    { "complex-float", "Cf", 0, false }, { "complex-double", "Cd", 0, false },
    { "complex-longdouble", "Ce", 0, false }, { "nullptr", "Dn", 0, false },
    { "bfloat16", "DF16b", 0, false } };
  return kBuiltins[kind];
}

inline const char * abi_builtin_type_word(AbiBuiltinTypeKind kind) { return abi_builtin_type_entry(kind).word; }
inline const char * abi_builtin_type_code(AbiBuiltinTypeKind kind) { return abi_builtin_type_entry(kind).code; }
inline bool abi_builtin_type_is_unsigned(AbiBuiltinTypeKind kind) { return abi_builtin_type_entry(kind).is_unsigned; }

// Width in bits of an integral builtin; 0 for other kinds.
inline unsigned abi_builtin_type_integral_bits(AbiBuiltinTypeKind kind)
{
  return abi_builtin_type_entry(kind).integral_bits;
}

// The builtin a fact word names, or ABI_BUILTIN_TYPE_NONE.
inline AbiBuiltinTypeKind abi_builtin_type_kind(const char * word, std::size_t length)
{
  for(unsigned kind = ABI_BUILTIN_TYPE_VOID; kind < ABI_BUILTIN_TYPE_COUNT; ++kind) {
    if(detail::same_word(abi_builtin_type_entry(static_cast<AbiBuiltinTypeKind>(kind)).word, word, length)) {
      return static_cast<AbiBuiltinTypeKind>(kind);
    }
  }
  return ABI_BUILTIN_TYPE_NONE;
}

// The complex types are compound types for substitution (5.1.10 excludes
// only <builtin-type>); the other builtins are never candidates.
inline bool abi_builtin_type_is_substitutable(AbiBuiltinTypeKind kind)
{
  return kind >= ABI_BUILTIN_TYPE_COMPLEX_FLOAT && kind <= ABI_BUILTIN_TYPE_COMPLEX_LONG_DOUBLE;
}

// The standard abbreviations (5.1.10): Sa, Sb, Ss, Si, So, Sd.
enum AbiStandardSubstitutionKind : std::uint8_t
{
  ABI_STANDARD_SUBSTITUTION_NONE,
  ABI_STANDARD_SUBSTITUTION_ALLOCATOR,     // Sa ::std::allocator
  ABI_STANDARD_SUBSTITUTION_BASIC_STRING,  // Sb ::std::basic_string
  ABI_STANDARD_SUBSTITUTION_STRING,        // Ss ::std::basic_string<char, ...>
  ABI_STANDARD_SUBSTITUTION_ISTREAM,       // Si ::std::basic_istream<char, ...>
  ABI_STANDARD_SUBSTITUTION_OSTREAM,       // So ::std::basic_ostream<char, ...>
  ABI_STANDARD_SUBSTITUTION_IOSTREAM,      // Sd ::std::basic_iostream<char, ...>
  ABI_STANDARD_SUBSTITUTION_COUNT
};

inline const char * abi_standard_substitution_code(AbiStandardSubstitutionKind kind)
{
  static const char * const kCodes[ABI_STANDARD_SUBSTITUTION_COUNT] = { "", "Sa", "Sb", "Ss", "Si", "So", "Sd" };
  return kCodes[kind];
}

// The abbreviation of a two-letter code such as "Sa", or
// ABI_STANDARD_SUBSTITUTION_NONE.
inline AbiStandardSubstitutionKind abi_standard_substitution_kind(const char * word, std::size_t length)
{
  for(unsigned kind = ABI_STANDARD_SUBSTITUTION_ALLOCATOR; kind < ABI_STANDARD_SUBSTITUTION_COUNT; ++kind) {
    if(detail::same_word(abi_standard_substitution_code(static_cast<AbiStandardSubstitutionKind>(kind)), word, length)) {
      return static_cast<AbiStandardSubstitutionKind>(kind);
    }
  }
  return ABI_STANDARD_SUBSTITUTION_NONE;
}

// Function-name terminals: constructor and destructor variants (5.1.4.3)
// and operator functions (5.1.3).  PLUS and MINUS are the fact words whose
// code depends on the operand count (abi_terminal_for_operands).
enum AbiTerminalKind : std::uint8_t
{
  ABI_TERMINAL_NONE,
  ABI_TERMINAL_CONSTRUCTOR_COMPLETE,
  ABI_TERMINAL_CONSTRUCTOR_BASE,
  ABI_TERMINAL_DESTRUCTOR_DELETING,
  ABI_TERMINAL_DESTRUCTOR_COMPLETE,
  ABI_TERMINAL_DESTRUCTOR_BASE,
  ABI_TERMINAL_PLUS,
  ABI_TERMINAL_MINUS,
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

struct AbiTerminalEntry
{
  const char * word;  // the fact word: "constructor-complete", "plus", ...
  const char * code;  // the Itanium code: "C1", "pl", ...; "" for PLUS and MINUS
};

inline const AbiTerminalEntry & abi_terminal_entry(AbiTerminalKind kind)
{
  static const AbiTerminalEntry kTerminals[ABI_TERMINAL_COUNT] = { { "", "" },
    { "constructor-complete", "C1" }, { "constructor-base", "C2" }, { "destructor-deleting", "D0" },
    { "destructor-complete", "D1" }, { "destructor-base", "D2" }, { "plus", "" }, { "minus", "" },
    { "unary-plus", "ps" }, { "binary-plus", "pl" }, { "unary-minus", "ng" },
    { "binary-minus", "mi" }, { "address-of", "ad" }, { "deref", "de" }, { "new", "nw" },
    { "new-array", "na" }, { "delete", "dl" }, { "delete-array", "da" }, { "multiply", "ml" },
    { "divide", "dv" }, { "remainder", "rm" }, { "bit-and", "an" }, { "bit-or", "or" },
    { "bit-xor", "eo" }, { "complement", "co" }, { "assign", "aS" }, { "plus-assign", "pL" },
    { "minus-assign", "mI" }, { "multiply-assign", "mL" }, { "divide-assign", "dV" },
    { "remainder-assign", "rM" }, { "and-assign", "aN" }, { "or-assign", "oR" },
    { "xor-assign", "eO" }, { "left-shift", "ls" }, { "right-shift", "rs" },
    { "left-shift-assign", "lS" }, { "right-shift-assign", "rS" }, { "equal", "eq" },
    { "not-equal", "ne" }, { "less", "lt" }, { "greater", "gt" }, { "less-equal", "le" },
    { "greater-equal", "ge" }, { "logical-not", "nt" }, { "logical-and", "aa" },
    { "logical-or", "oo" }, { "increment", "pp" }, { "decrement", "mm" }, { "comma", "cm" },
    { "member-pointer", "pm" }, { "arrow", "pt" }, { "call", "cl" }, { "index", "ix" } };
  return kTerminals[kind];
}

inline const char * abi_terminal_word(AbiTerminalKind kind) { return abi_terminal_entry(kind).word; }
inline const char * abi_terminal_code(AbiTerminalKind kind) { return abi_terminal_entry(kind).code; }

// The terminal a fact word names; `operator-call` (the call operator of a
// local or lambda context) is another word for ABI_TERMINAL_CALL.
inline bool abi_find_terminal_kind(const char * word, std::size_t length, AbiTerminalKind * kind)
{
  if(detail::same_word("operator-call", word, length)) {
    *kind = ABI_TERMINAL_CALL;
    return true;
  }
  for(unsigned candidate = ABI_TERMINAL_CONSTRUCTOR_COMPLETE; candidate < ABI_TERMINAL_COUNT; ++candidate) {
    if(detail::same_word(abi_terminal_entry(static_cast<AbiTerminalKind>(candidate)).word, word, length)) {
      *kind = static_cast<AbiTerminalKind>(candidate);
      return true;
    }
  }
  return false;
}

inline bool abi_terminal_is_constructor_or_destructor(AbiTerminalKind kind)
{
  return kind >= ABI_TERMINAL_CONSTRUCTOR_COMPLETE && kind <= ABI_TERMINAL_DESTRUCTOR_BASE;
}

// The fixed-code terminal an operator takes for `operand_count` operands
// (counting a member's implicit object): plus and minus become unary or
// binary, a unary `*` and `&` are deref and address-of (5.1.3).
inline AbiTerminalKind abi_terminal_for_operands(AbiTerminalKind kind, std::size_t operand_count)
{
  if(kind == ABI_TERMINAL_PLUS) {
    return operand_count == 1 ? ABI_TERMINAL_UNARY_PLUS : ABI_TERMINAL_BINARY_PLUS;
  }
  if(kind == ABI_TERMINAL_MINUS) {
    return operand_count == 1 ? ABI_TERMINAL_UNARY_MINUS : ABI_TERMINAL_BINARY_MINUS;
  }
  if(kind == ABI_TERMINAL_MULTIPLY && operand_count == 1) {
    return ABI_TERMINAL_DEREFERENCE;
  }
  if(kind == ABI_TERMINAL_BIT_AND && operand_count == 1) {
    return ABI_TERMINAL_ADDRESS_OF;
  }
  return kind;
}

// Dependent-expression operations (5.1.6).  An operation is its
// two-character Itanium code packed into 16 bits; the table validates a
// code against the operand shape it is used with.
enum AbiExpressionShape : std::uint8_t
{
  ABI_EXPRESSION_SHAPE_UNARY = 1,
  ABI_EXPRESSION_SHAPE_BINARY = 2,
  ABI_EXPRESSION_SHAPE_TERNARY = 4,
  ABI_EXPRESSION_SHAPE_CAST = 8,           // <code> <type> <expression>
  ABI_EXPRESSION_SHAPE_MEMBER_ACCESS = 16  // <code> <expression> <unresolved-name>
};

inline std::uint16_t abi_pack_operation_code(char first, char second)
{
  return static_cast<std::uint16_t>((static_cast<unsigned char>(first) << 8) | static_cast<unsigned char>(second));
}

struct AbiOperationEntry
{
  char code[3];
  unsigned char shapes;
};

inline const AbiOperationEntry * abi_operation_entries(std::size_t * count)
{
  static const AbiOperationEntry kOperations[] = {
    { "ps", ABI_EXPRESSION_SHAPE_UNARY }, { "ng", ABI_EXPRESSION_SHAPE_UNARY },
    { "ad", ABI_EXPRESSION_SHAPE_UNARY }, { "de", ABI_EXPRESSION_SHAPE_UNARY },
    { "co", ABI_EXPRESSION_SHAPE_UNARY }, { "nt", ABI_EXPRESSION_SHAPE_UNARY },
    { "pp", ABI_EXPRESSION_SHAPE_UNARY }, { "mm", ABI_EXPRESSION_SHAPE_UNARY },
    { "sz", ABI_EXPRESSION_SHAPE_UNARY }, { "az", ABI_EXPRESSION_SHAPE_UNARY },
    { "te", ABI_EXPRESSION_SHAPE_UNARY }, { "nx", ABI_EXPRESSION_SHAPE_UNARY },
    { "tw", ABI_EXPRESSION_SHAPE_UNARY }, { "dl", ABI_EXPRESSION_SHAPE_UNARY },
    { "da", ABI_EXPRESSION_SHAPE_UNARY }, { "sp", ABI_EXPRESSION_SHAPE_UNARY },
    { "sZ", ABI_EXPRESSION_SHAPE_UNARY }, { "pl", ABI_EXPRESSION_SHAPE_BINARY },
    { "mi", ABI_EXPRESSION_SHAPE_BINARY }, { "ml", ABI_EXPRESSION_SHAPE_BINARY },
    { "dv", ABI_EXPRESSION_SHAPE_BINARY }, { "rm", ABI_EXPRESSION_SHAPE_BINARY },
    { "an", ABI_EXPRESSION_SHAPE_BINARY }, { "or", ABI_EXPRESSION_SHAPE_BINARY },
    { "eo", ABI_EXPRESSION_SHAPE_BINARY }, { "aS", ABI_EXPRESSION_SHAPE_BINARY },
    { "pL", ABI_EXPRESSION_SHAPE_BINARY }, { "mI", ABI_EXPRESSION_SHAPE_BINARY },
    { "mL", ABI_EXPRESSION_SHAPE_BINARY }, { "dV", ABI_EXPRESSION_SHAPE_BINARY },
    { "rM", ABI_EXPRESSION_SHAPE_BINARY }, { "aN", ABI_EXPRESSION_SHAPE_BINARY },
    { "oR", ABI_EXPRESSION_SHAPE_BINARY }, { "eO", ABI_EXPRESSION_SHAPE_BINARY },
    { "ls", ABI_EXPRESSION_SHAPE_BINARY }, { "rs", ABI_EXPRESSION_SHAPE_BINARY },
    { "lS", ABI_EXPRESSION_SHAPE_BINARY }, { "rS", ABI_EXPRESSION_SHAPE_BINARY },
    { "eq", ABI_EXPRESSION_SHAPE_BINARY }, { "ne", ABI_EXPRESSION_SHAPE_BINARY },
    { "lt", ABI_EXPRESSION_SHAPE_BINARY }, { "gt", ABI_EXPRESSION_SHAPE_BINARY },
    { "le", ABI_EXPRESSION_SHAPE_BINARY }, { "ge", ABI_EXPRESSION_SHAPE_BINARY },
    { "ss", ABI_EXPRESSION_SHAPE_BINARY }, { "aa", ABI_EXPRESSION_SHAPE_BINARY },
    { "oo", ABI_EXPRESSION_SHAPE_BINARY }, { "cm", ABI_EXPRESSION_SHAPE_BINARY },
    { "pm", ABI_EXPRESSION_SHAPE_BINARY }, { "ds", ABI_EXPRESSION_SHAPE_BINARY },
    { "ix", ABI_EXPRESSION_SHAPE_BINARY }, { "qu", ABI_EXPRESSION_SHAPE_TERNARY },
    { "sc", ABI_EXPRESSION_SHAPE_CAST }, { "dc", ABI_EXPRESSION_SHAPE_CAST },
    { "cc", ABI_EXPRESSION_SHAPE_CAST }, { "rc", ABI_EXPRESSION_SHAPE_CAST },
    { "cv", ABI_EXPRESSION_SHAPE_CAST }, { "dt", ABI_EXPRESSION_SHAPE_MEMBER_ACCESS },
    { "pt", ABI_EXPRESSION_SHAPE_MEMBER_ACCESS } };
  *count = sizeof kOperations / sizeof kOperations[0];
  return kOperations;
}

// The packed code of a two-character operation word used with `shape`,
// or 0 if it is not an Itanium operation of that shape.
inline std::uint16_t abi_expression_operation_code(const char * word, std::size_t length, AbiExpressionShape shape)
{
  std::size_t count = 0;
  const AbiOperationEntry * entries = abi_operation_entries(&count);
  if(length != 2) {
    return 0;
  }
  for(std::size_t i = 0; i < count; ++i) {
    if(entries[i].code[0] == word[0] && entries[i].code[1] == word[1] && (entries[i].shapes & shape) != 0) {
      return abi_pack_operation_code(word[0], word[1]);
    }
  }
  return 0;
}

// <CV-qualifiers> and <ref-qualifier> as bits (5.1.5.1, 5.1.5.3); the
// encoding writes them in the canonical order r V K, then R or O.
enum AbiQualifier : std::uint16_t
{
  ABI_QUALIFIER_CONST = 1,       // K
  ABI_QUALIFIER_VOLATILE = 2,    // V
  ABI_QUALIFIER_RESTRICT = 4,    // r
  ABI_QUALIFIER_LVALUE_REF = 8,  // R
  ABI_QUALIFIER_RVALUE_REF = 16  // O
};

// An array type's bound (5.1.5.4): none (A_), a number, an expression, or
// a vendor vector's lane count (Dv <number> _).
enum AbiArrayBound : std::uint8_t
{
  ABI_ARRAY_BOUND_NONE,
  ABI_ARRAY_BOUND_VALUE,
  ABI_ARRAY_BOUND_EXPRESSION,
  ABI_ARRAY_BOUND_VECTOR
};

// <template-param-decl>: Ty, Tn <type>, Tt <template-param-decl>* E; a
// pack's is preceded by Tp.
enum AbiTemplateParamDeclKind : std::uint16_t
{
  ABI_PARAM_DECL_TYPE,
  ABI_PARAM_DECL_NON_TYPE,
  ABI_PARAM_DECL_TEMPLATE
};

// What a mangled name names: a type's bare encoding, an entity, a C name,
// or one of the special names (5.1.4).
enum AbiTargetKind : std::uint8_t
{
  ABI_TARGET_TYPE,                 // <type>
  ABI_TARGET_ENTITY,               // _Z <encoding>
  ABI_TARGET_C_NAME,               // an unmangled C-linkage name
  ABI_TARGET_TYPEINFO,             // _ZTI <type>
  ABI_TARGET_TYPEINFO_NAME,        // _ZTS <type>
  ABI_TARGET_VTABLE,               // _ZTV <type>
  ABI_TARGET_VTT,                  // _ZTT <type>
  ABI_TARGET_CONSTRUCTION_VTABLE,  // _ZTC <derived> <offset> _ <base>
  ABI_TARGET_TLS_WRAPPER,          // _ZTW <name>
  ABI_TARGET_THUNK,                // _ZT <call-offset> [<call-offset>] <encoding>
  ABI_TARGET_TLS_INIT,             // _ZTH <name>
  ABI_TARGET_GUARD_VARIABLE,       // _ZGV <name>
  ABI_TARGET_COUNT
};

// The prefix the encoding of a target starts with ("" for a type or a C name).
inline const char * abi_target_prefix(AbiTargetKind kind)
{
  static const char * const kPrefixes[ABI_TARGET_COUNT] = { "", "_Z", "", "_ZTI", "_ZTS", "_ZTV", "_ZTT",
    "_ZTC", "_ZTW", "_ZT", "_ZTH", "_ZGV" };
  return kPrefixes[kind];
}

// A <call-offset> (5.1.4.2): h <offset> _, or v <offset> _ <vcall offset> _.
struct AbiCallOffset
{
  bool is_virtual = false;
  long long offset = 0;
  long long vcall_offset = 0;
};

// ===========================================================================
// PA9 fact-file words (pa9/README.md "ABI Fact Files")

// `let-type <id> ...` and the other definitions.
enum AbiFactDefinition : std::uint8_t
{
  ABI_FACT_DEFINITION_TYPE,
  ABI_FACT_DEFINITION_ARGUMENT,
  ABI_FACT_DEFINITION_EXPRESSION,
  ABI_FACT_DEFINITION_CONTEXT,
  ABI_FACT_DEFINITION_ENTITY
};

const unsigned ABI_FACT_DEFINITION_COUNT = ABI_FACT_DEFINITION_ENTITY + 1;

inline const char * spelling(AbiFactDefinition value)
{
  static const char * const kWords[ABI_FACT_DEFINITION_COUNT] = { "let-type", "let-arg", "let-expr",
    "let-context", "let-entity" };
  return kWords[value];
}

// The first word of a case's target line.
enum AbiFactTarget : std::uint8_t
{
  ABI_FACT_TARGET_TYPE,
  ABI_FACT_TARGET_FUNCTION,
  ABI_FACT_TARGET_C_FUNCTION,
  ABI_FACT_TARGET_VARIABLE,
  ABI_FACT_TARGET_TYPEINFO,
  ABI_FACT_TARGET_TYPEINFO_NAME,
  ABI_FACT_TARGET_VTABLE,
  ABI_FACT_TARGET_VTT,
  ABI_FACT_TARGET_CONSTRUCTION_VTABLE,
  ABI_FACT_TARGET_TLS_WRAPPER,
  ABI_FACT_TARGET_THUNK,
  ABI_FACT_TARGET_VIRTUAL_BASE_THUNK
};

const unsigned ABI_FACT_TARGET_COUNT = ABI_FACT_TARGET_VIRTUAL_BASE_THUNK + 1;

inline const char * spelling(AbiFactTarget value)
{
  static const char * const kWords[ABI_FACT_TARGET_COUNT] = { "type", "function", "c-function",
    "variable", "typeinfo", "typeinfo-name", "vtable", "vtt", "construction-vtable", "tls-wrapper",
    "thunk", "virtual-base-thunk" };
  return kWords[value];
}

// The target kind a fact target produces.
inline AbiTargetKind abi_target_kind(AbiFactTarget target)
{
  static const AbiTargetKind kKinds[ABI_FACT_TARGET_COUNT] = { ABI_TARGET_TYPE, ABI_TARGET_ENTITY,
    ABI_TARGET_C_NAME, ABI_TARGET_ENTITY, ABI_TARGET_TYPEINFO, ABI_TARGET_TYPEINFO_NAME,
    ABI_TARGET_VTABLE, ABI_TARGET_VTT, ABI_TARGET_CONSTRUCTION_VTABLE, ABI_TARGET_TLS_WRAPPER,
    ABI_TARGET_THUNK, ABI_TARGET_THUNK };
  return kKinds[target];
}

// The form word after `function` (`function path ns::f`); a function
// target without one names a plain function.
enum AbiFactFunctionForm : std::uint8_t
{
  ABI_FACT_FUNCTION_PATH,
  ABI_FACT_FUNCTION_ENCODING,
  ABI_FACT_FUNCTION_LOCAL,
  ABI_FACT_FUNCTION_LAMBDA,
  ABI_FACT_FUNCTION_NAMESPACE_LAMBDA
};

const unsigned ABI_FACT_FUNCTION_COUNT = ABI_FACT_FUNCTION_NAMESPACE_LAMBDA + 1;

inline const char * spelling(AbiFactFunctionForm value)
{
  static const char * const kWords[ABI_FACT_FUNCTION_COUNT] = { "path", "encoding", "local", "lambda",
    "namespace-lambda" };
  return kWords[value];
}

// The lines that follow a function target.
enum AbiFactRecord : std::uint8_t
{
  ABI_FACT_RECORD_PARAM,
  ABI_FACT_RECORD_RESULT,
  ABI_FACT_RECORD_VARIADIC,
  ABI_FACT_RECORD_ABI_TAG,
  ABI_FACT_RECORD_QUALIFIER,
  ABI_FACT_RECORD_FUNCTION_QUALIFIER,
  ABI_FACT_RECORD_FUNCTION_TEMPLATE_ARG,
  ABI_FACT_RECORD_FUNCTION_TEMPLATE_PREFIX,
  ABI_FACT_RECORD_OPERATOR_TERMINAL,
  ABI_FACT_RECORD_CONVERSION_TERMINAL,
  ABI_FACT_RECORD_TERMINAL,
  ABI_FACT_RECORD_TERMINAL_SOURCE,
  ABI_FACT_RECORD_NAME_SOURCE,
  ABI_FACT_RECORD_NAME_STD,
  ABI_FACT_RECORD_NAME_TEMPLATE,
  ABI_FACT_RECORD_COMPONENT_ABI_TAG,
  ABI_FACT_RECORD_LOCAL_CONTEXT,
  ABI_FACT_RECORD_LAMBDA_CONTEXT,
  ABI_FACT_RECORD_NAMESPACE_LAMBDA_CONTEXT
};

const unsigned ABI_FACT_RECORD_COUNT = ABI_FACT_RECORD_NAMESPACE_LAMBDA_CONTEXT + 1;

inline const char * spelling(AbiFactRecord value)
{
  static const char * const kWords[ABI_FACT_RECORD_COUNT] = { "param", "result", "variadic", "abi-tag",
    "qualifier", "function-qualifier", "function-template-arg", "function-template-prefix",
    "operator-terminal", "conversion-terminal", "terminal", "terminal-source", "name-source",
    "name-std", "name-template", "component-abi-tag", "local-context", "lambda-context",
    "namespace-lambda-context" };
  return kWords[value];
}

// The words of a `qualifier` or `function-qualifier` record.
enum AbiFactQualifier : std::uint8_t
{
  ABI_FACT_QUALIFIER_CONST,
  ABI_FACT_QUALIFIER_VOLATILE,
  ABI_FACT_QUALIFIER_RESTRICT,
  ABI_FACT_QUALIFIER_LVALUE_REF,
  ABI_FACT_QUALIFIER_RVALUE_REF
};

const unsigned ABI_FACT_QUALIFIER_COUNT = ABI_FACT_QUALIFIER_RVALUE_REF + 1;

inline const char * spelling(AbiFactQualifier value)
{
  static const char * const kWords[ABI_FACT_QUALIFIER_COUNT] = { "const", "volatile", "restrict",
    "lvalue-ref", "rvalue-ref" };
  return kWords[value];
}

inline AbiQualifier abi_qualifier(AbiFactQualifier value)
{
  return static_cast<AbiQualifier>(value == ABI_FACT_QUALIFIER_CONST ? ABI_QUALIFIER_CONST
    : value == ABI_FACT_QUALIFIER_VOLATILE ? ABI_QUALIFIER_VOLATILE
    : value == ABI_FACT_QUALIFIER_RESTRICT ? ABI_QUALIFIER_RESTRICT
    : value == ABI_FACT_QUALIFIER_LVALUE_REF ? ABI_QUALIFIER_LVALUE_REF : ABI_QUALIFIER_RVALUE_REF);
}

// The terminal forms beyond AbiTerminalKind's words:
// `terminal constructor-inherited-complete <base-type>` (CI1),
// `terminal constructor-inherited-base <base-type>` (CI2) and
// `operator-terminal literal <suffix>` (li).
enum AbiFactSpecialTerminal : std::uint8_t
{
  ABI_FACT_TERMINAL_INHERITED_COMPLETE,
  ABI_FACT_TERMINAL_INHERITED_BASE,
  ABI_FACT_TERMINAL_LITERAL
};

const unsigned ABI_FACT_TERMINAL_COUNT = ABI_FACT_TERMINAL_LITERAL + 1;

inline const char * spelling(AbiFactSpecialTerminal value)
{
  static const char * const kWords[ABI_FACT_TERMINAL_COUNT] = { "constructor-inherited-complete",
    "constructor-inherited-base", "literal" };
  return kWords[value];
}

// The first word of a multiword type form (`let-type T template ns::X A`).
enum AbiFactTypeForm : std::uint8_t
{
  ABI_FACT_TYPE_PTR,
  ABI_FACT_TYPE_REF,
  ABI_FACT_TYPE_RREF,
  ABI_FACT_TYPE_PACK,
  ABI_FACT_TYPE_CONST,
  ABI_FACT_TYPE_VOLATILE,
  ABI_FACT_TYPE_VENDOR,
  ABI_FACT_TYPE_BUILTIN_TRANSFORM,
  ABI_FACT_TYPE_NAMED,
  ABI_FACT_TYPE_NAME,
  ABI_FACT_TYPE_TEMPLATE,
  ABI_FACT_TYPE_STD_TEMPLATE,
  ABI_FACT_TYPE_TEMPLATE_PARAM,
  ABI_FACT_TYPE_TEMPLATE_PARAM_SUBST,
  ABI_FACT_TYPE_TEMPLATE_PARAM_TEMPLATE,
  ABI_FACT_TYPE_MEMBER,
  ABI_FACT_TYPE_MEMBER_TEMPLATE,
  ABI_FACT_TYPE_DECLTYPE,
  ABI_FACT_TYPE_DECLTYPE_ID,
  ABI_FACT_TYPE_LAMBDA_CLOSURE,
  ABI_FACT_TYPE_LOCAL_TYPE,
  ABI_FACT_TYPE_NAMESPACE_LAMBDA,
  ABI_FACT_TYPE_TAGGED,
  ABI_FACT_TYPE_FUNCTION_TYPE,
  ABI_FACT_TYPE_FUNCTION_TYPE_VARIADIC,
  ABI_FACT_TYPE_MEMBER_POINTER
};

const unsigned ABI_FACT_TYPE_COUNT = ABI_FACT_TYPE_MEMBER_POINTER + 1;

inline const char * spelling(AbiFactTypeForm value)
{
  static const char * const kWords[ABI_FACT_TYPE_COUNT] = { "ptr", "ref", "rref", "pack", "const",
    "volatile", "vendor", "builtin-transform", "named", "name", "template", "std-template",
    "template-param", "template-param-subst", "template-param-template", "member",
    "member-template", "decltype", "decltype-id", "lambda-closure", "local-type",
    "namespace-lambda", "tagged", "function-type", "function-type-variadic", "member-pointer" };
  return kWords[value];
}

// The prefixes of a compact type word (`ptr:const:int`, `array:4:int`),
// and its two compact heads, `named:<name>` and `memberptr:<owner>:<type>`.
enum AbiFactTypePrefix : std::uint8_t
{
  ABI_FACT_PREFIX_PTR,
  ABI_FACT_PREFIX_REF,
  ABI_FACT_PREFIX_RREF,
  ABI_FACT_PREFIX_CONST,
  ABI_FACT_PREFIX_VOLATILE,
  ABI_FACT_PREFIX_PACK,
  ABI_FACT_PREFIX_ARRAY,  // array:<bound>: with a number or an expression id
  ABI_FACT_PREFIX_NAMED,
  ABI_FACT_PREFIX_MEMBERPTR
};

const unsigned ABI_FACT_PREFIX_COUNT = ABI_FACT_PREFIX_MEMBERPTR + 1;

// The prefix with its colon: "ptr:", "array:", "named:", ...
inline const char * spelling(AbiFactTypePrefix value)
{
  static const char * const kWords[ABI_FACT_PREFIX_COUNT] = { "ptr:", "ref:", "rref:", "const:",
    "volatile:", "pack:", "array:", "named:", "memberptr:" };
  return kWords[value];
}

// The first word of a template-argument definition (`let-arg A type T`).
enum AbiFactArgumentForm : std::uint8_t
{
  ABI_FACT_ARGUMENT_TYPE,
  ABI_FACT_ARGUMENT_VALUE,
  ABI_FACT_ARGUMENT_DEPENDENT_VALUE,
  ABI_FACT_ARGUMENT_EXPRESSION,
  ABI_FACT_ARGUMENT_TEMPLATE_ENTITY,
  ABI_FACT_ARGUMENT_MEMBER_TEMPLATE_ENTITY,
  ABI_FACT_ARGUMENT_TEMPLATE_PARAM_TEMPLATE,
  ABI_FACT_ARGUMENT_ENTITY_ADDRESS,
  ABI_FACT_ARGUMENT_ENTITY_REFERENCE,
  ABI_FACT_ARGUMENT_MEMBER_EXTERNAL_ADDRESS,
  ABI_FACT_ARGUMENT_PACK
};

const unsigned ABI_FACT_ARGUMENT_COUNT = ABI_FACT_ARGUMENT_PACK + 1;

inline const char * spelling(AbiFactArgumentForm value)
{
  static const char * const kWords[ABI_FACT_ARGUMENT_COUNT] = { "type", "value", "dependent-value",
    "expression", "template-entity", "member-template-entity", "template-param-template",
    "entity-address", "entity-reference", "member-external-address", "pack" };
  return kWords[value];
}

// The first word of a dependent-expression definition (`let-expr E binary pl A B`).
enum AbiFactExpressionForm : std::uint8_t
{
  ABI_FACT_EXPRESSION_TEMPLATE_PARAM,
  ABI_FACT_EXPRESSION_FUNCTION_PARAM,
  ABI_FACT_EXPRESSION_LITERAL,
  ABI_FACT_EXPRESSION_SIZEOF_TYPE,
  ABI_FACT_EXPRESSION_PACK,
  ABI_FACT_EXPRESSION_ENTITY_REFERENCE,
  ABI_FACT_EXPRESSION_MEMBER,
  ABI_FACT_EXPRESSION_SOURCE_NAME,
  ABI_FACT_EXPRESSION_TEMPLATE_ID,
  ABI_FACT_EXPRESSION_TYPE_TRAIT,
  ABI_FACT_EXPRESSION_CAST,
  ABI_FACT_EXPRESSION_OBJECT_MEMBER,
  ABI_FACT_EXPRESSION_UNARY,
  ABI_FACT_EXPRESSION_BINARY,
  ABI_FACT_EXPRESSION_CONDITIONAL,
  ABI_FACT_EXPRESSION_CALL
};

const unsigned ABI_FACT_EXPRESSION_COUNT = ABI_FACT_EXPRESSION_CALL + 1;

inline const char * spelling(AbiFactExpressionForm value)
{
  static const char * const kWords[ABI_FACT_EXPRESSION_COUNT] = { "template-param", "function-param",
    "literal", "sizeof-type", "pack", "entity-reference", "member", "source-name", "template-id",
    "type-trait", "cast", "object-member", "unary", "binary", "conditional", "call" };
  return kWords[value];
}

// `let-context <id> raw <fragment>` and `let-context <id> function ...`.
enum AbiFactContextForm : std::uint8_t
{
  ABI_FACT_CONTEXT_RAW,
  ABI_FACT_CONTEXT_FUNCTION
};

const unsigned ABI_FACT_CONTEXT_COUNT = ABI_FACT_CONTEXT_FUNCTION + 1;

inline const char * spelling(AbiFactContextForm value)
{
  static const char * const kWords[ABI_FACT_CONTEXT_COUNT] = { "raw", "function" };
  return kWords[value];
}

// The first word of an entity definition (`let-entity V variable ns::v`).
enum AbiFactEntityForm : std::uint8_t
{
  ABI_FACT_ENTITY_VARIABLE,
  ABI_FACT_ENTITY_INTERNAL_VARIABLE,
  ABI_FACT_ENTITY_SYMBOL,
  ABI_FACT_ENTITY_FUNCTION
};

const unsigned ABI_FACT_ENTITY_COUNT = ABI_FACT_ENTITY_FUNCTION + 1;

inline const char * spelling(AbiFactEntityForm value)
{
  static const char * const kWords[ABI_FACT_ENTITY_COUNT] = { "variable", "internal-variable", "symbol",
    "function" };
  return kWords[value];
}

// The remaining words: the case separator, the thunk forms' markers, the
// placeholders of an unnamed final component and the flag values.
enum AbiFactWord : std::uint8_t
{
  ABI_FACT_WORD_CASE,            // `case [<label>]` starts a case
  ABI_FACT_WORD_VIRTUAL_RESULT,  // thunk <this> virtual-result <fixed> <vcall> function ...
  ABI_FACT_WORD_PLACEHOLDER,     // `-`: no name (replaced by a terminal, no closure number)
  ABI_FACT_WORD_OPERATOR,        // `operator`: a name replaced by an operator terminal
  ABI_FACT_WORD_STD              // the `std` namespace of a qualified name
};

const unsigned ABI_FACT_WORD_COUNT = ABI_FACT_WORD_STD + 1;

inline const char * spelling(AbiFactWord value)
{
  static const char * const kWords[ABI_FACT_WORD_COUNT] = { "case", "virtual-result", "-", "operator", "std" };
  return kWords[value];
}

// A flag operand: yes, true or 1; no, false or 0.
inline bool abi_fact_flag_from_spelling(const char * text, std::size_t length, bool * value)
{
  if(detail::same_word("yes", text, length) || detail::same_word("true", text, length) ||
     detail::same_word("1", text, length)) {
    *value = true;
    return true;
  }
  if(detail::same_word("no", text, length) || detail::same_word("false", text, length) ||
     detail::same_word("0", text, length)) {
    *value = false;
    return true;
  }
  return false;
}

#define ABI_MANGLE_FROM_SPELLING(Enum, Count)                                           \
  inline bool from_spelling(const char * text, std::size_t length, Enum * value)       \
  {                                                                             \
    return detail::find_word(text, length, 0, Count, value);                   \
  }
ABI_MANGLE_FROM_SPELLING(AbiFactDefinition, ABI_FACT_DEFINITION_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactTarget, ABI_FACT_TARGET_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactFunctionForm, ABI_FACT_FUNCTION_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactRecord, ABI_FACT_RECORD_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactQualifier, ABI_FACT_QUALIFIER_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactSpecialTerminal, ABI_FACT_TERMINAL_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactTypeForm, ABI_FACT_TYPE_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactArgumentForm, ABI_FACT_ARGUMENT_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactExpressionForm, ABI_FACT_EXPRESSION_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactContextForm, ABI_FACT_CONTEXT_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactEntityForm, ABI_FACT_ENTITY_COUNT)
ABI_MANGLE_FROM_SPELLING(AbiFactWord, ABI_FACT_WORD_COUNT)
#undef ABI_MANGLE_FROM_SPELLING

// A compact type prefix at the start of `text` (`ptr:` of `ptr:int`).
inline bool prefix_from_spelling(const char * text, std::size_t length, AbiFactTypePrefix * value)
{
  for(unsigned i = 0; i < ABI_FACT_PREFIX_COUNT; ++i) {
    const char * word = spelling(static_cast<AbiFactTypePrefix>(i));
    const std::size_t n = std::strlen(word);
    if(n <= length && std::memcmp(word, text, n) == 0) {
      *value = static_cast<AbiFactTypePrefix>(i);
      return true;
    }
  }
  return false;
}

}  // namespace abi_mangle
