#pragma once

// Optional assignment-facing ABI fact scaffold for the PA31 standalone
// abimangle tool. Solutions may use these records directly or replace them, as
// long as they accept the PA31 fact files and emit the required Itanium ABI
// names.

#include <cstddef>
#include <string>
#include <vector>

namespace abi_mangle {

enum AbiBuiltinType
{
  ABI_BUILTIN_INVALID,
  ABI_BUILTIN_VOID,
  ABI_BUILTIN_BOOL,
  ABI_BUILTIN_CHAR,
  ABI_BUILTIN_SIGNED_CHAR,
  ABI_BUILTIN_UNSIGNED_CHAR,
  ABI_BUILTIN_SHORT,
  ABI_BUILTIN_UNSIGNED_SHORT,
  ABI_BUILTIN_INT,
  ABI_BUILTIN_UNSIGNED_INT,
  ABI_BUILTIN_LONG,
  ABI_BUILTIN_UNSIGNED_LONG,
  ABI_BUILTIN_LONG_LONG,
  ABI_BUILTIN_UNSIGNED_LONG_LONG,
  ABI_BUILTIN_INT128,
  ABI_BUILTIN_UINT128,
  ABI_BUILTIN_WCHAR,
  ABI_BUILTIN_CHAR16,
  ABI_BUILTIN_CHAR32,
  ABI_BUILTIN_FLOAT,
  ABI_BUILTIN_DOUBLE,
  ABI_BUILTIN_LONG_DOUBLE,
  ABI_BUILTIN_NULLPTR
};

enum AbiVendorQualifier
{
  ABI_VENDOR_QUALIFIER_NONE,
  ABI_VENDOR_QUALIFIER_ATOMIC
};

enum AbiStdSubstitution
{
  ABI_STD_SUBSTITUTION_NONE,
  ABI_STD_SUBSTITUTION_STD,
  ABI_STD_SUBSTITUTION_ALLOCATOR,
  ABI_STD_SUBSTITUTION_BASIC_STRING,
  ABI_STD_SUBSTITUTION_STRING,
  ABI_STD_SUBSTITUTION_ISTREAM,
  ABI_STD_SUBSTITUTION_OSTREAM,
  ABI_STD_SUBSTITUTION_IOSTREAM
};

enum AbiExpressionOperator
{
  ABI_EXPR_OP_INVALID,
  ABI_EXPR_OP_DEREFERENCE,
  ABI_EXPR_OP_ADDRESS_OF,
  ABI_EXPR_OP_UNARY_PLUS,
  ABI_EXPR_OP_UNARY_MINUS,
  ABI_EXPR_OP_NOT,
  ABI_EXPR_OP_COMPLEMENT,
  ABI_EXPR_OP_ADD,
  ABI_EXPR_OP_DIVIDE,
  ABI_EXPR_OP_REMAINDER,
  ABI_EXPR_OP_EQUAL
};

enum AbiArrayBoundKind
{
  ABI_ARRAY_BOUND_NONE,
  ABI_ARRAY_BOUND_INTEGER,
  ABI_ARRAY_BOUND_EXPRESSION
};

struct AbiArrayBound
{
  AbiArrayBoundKind kind = ABI_ARRAY_BOUND_NONE;
  unsigned long long integer_value = 0;
  std::string expression_reference;
};

enum AbiTypeKind
{
  ABI_TYPE_REFERENCE,
  ABI_TYPE_BUILTIN,
  ABI_TYPE_TEMPLATE_PARAMETER,
  ABI_TYPE_POINTER,
  ABI_TYPE_LVALUE_REFERENCE,
  ABI_TYPE_RVALUE_REFERENCE,
  ABI_TYPE_CONST,
  ABI_TYPE_VOLATILE,
  ABI_TYPE_VENDOR_QUALIFIED,
  ABI_TYPE_BUILTIN_TYPE_TRANSFORM,
  ABI_TYPE_PACK_EXPANSION,
  ABI_TYPE_ARRAY,
  ABI_TYPE_FUNCTION,
  ABI_TYPE_MEMBER_POINTER,
  ABI_TYPE_NAMED,
  ABI_TYPE_CLASS_TEMPLATE,
  ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE,
  ABI_TYPE_STD_CLASS_TEMPLATE,
  ABI_TYPE_MEMBER_TYPE,
  ABI_TYPE_MEMBER_CLASS_TEMPLATE,
  ABI_TYPE_DECLTYPE,
  ABI_TYPE_LAMBDA_CLOSURE,
  ABI_TYPE_LOCAL_TYPE
};

struct AbiType
{
  AbiTypeKind kind = ABI_TYPE_REFERENCE;
  std::string reference;
  AbiBuiltinType builtin_type = ABI_BUILTIN_INVALID;
  AbiVendorQualifier vendor_qualifier = ABI_VENDOR_QUALIFIER_NONE;
  std::string name;
  AbiArrayBound array_bound;
  std::size_t template_parameter_index = 0;
  bool substitutable_template_parameter = false;
  AbiStdSubstitution std_substitution = ABI_STD_SUBSTITUTION_NONE;
  bool std_substitution_includes_template_arguments = false;
  std::string context_reference;
  std::string discriminator;
  std::string source_name;
  std::string expression_reference;
  bool variadic = false;
  std::vector<AbiType> child_types;
  std::vector<std::string> template_argument_references;
};

enum AbiTemplateArgumentKind
{
  ABI_TEMPLATE_ARG_TYPE,
  ABI_TEMPLATE_ARG_INTEGRAL_VALUE,
  ABI_TEMPLATE_ARG_DEPENDENT_INTEGRAL_VALUE,
  ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE,
  ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION,
  ABI_TEMPLATE_ARG_TEMPLATE_ENTITY,
  ABI_TEMPLATE_ARG_TEMPLATE_PARAMETER_ENTITY,
  ABI_TEMPLATE_ARG_EXTERNAL_ENTITY,
  ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY,
  ABI_TEMPLATE_ARG_ENTITY_ADDRESS,
  ABI_TEMPLATE_ARG_ENTITY_REFERENCE,
  ABI_TEMPLATE_ARG_PACK
};

struct AbiTemplateArg
{
  AbiTemplateArgumentKind kind = ABI_TEMPLATE_ARG_TYPE;
  AbiType type;
  AbiType parameter_type;
  AbiType owner_type;
  long long integer_value = 0;
  std::string expression_reference;
  std::string entity_reference;
  std::string symbol;
  std::string member_name;
  std::vector<AbiType> parameter_types;
  bool address_of = false;
  bool member_is_function = false;
  bool member_function_const = false;
  bool member_function_volatile = false;
  bool member_function_lvalue_ref = false;
  bool member_function_rvalue_ref = false;
  bool member_function_variadic = false;
  std::size_t template_parameter_index = 0;
  std::vector<std::string> pack_argument_references;
};

enum AbiDependentExpressionKind
{
  ABI_EXPR_TEMPLATE_PARAMETER,
  ABI_EXPR_FUNCTION_PARAMETER,
  ABI_EXPR_LITERAL,
  ABI_EXPR_INTEGRAL_VALUE,
  ABI_EXPR_OBJECT_MEMBER,
  ABI_EXPR_UNARY,
  ABI_EXPR_BINARY,
  ABI_EXPR_CONDITIONAL,
  ABI_EXPR_PACK_EXPANSION,
  ABI_EXPR_CALL,
  ABI_EXPR_CONVERSION,
  ABI_EXPR_TEMPLATE_ID,
  ABI_EXPR_TYPE_TRAIT,
  ABI_EXPR_SIZEOF_TYPE,
  ABI_EXPR_MEMBER,
  ABI_EXPR_EXTERNAL_ENTITY,
  ABI_EXPR_ENTITY_ADDRESS,
  ABI_EXPR_ENTITY_REFERENCE
};

struct AbiDependentExpr
{
  AbiDependentExpressionKind kind = ABI_EXPR_LITERAL;
  std::size_t index = 0;
  std::string literal;
  AbiExpressionOperator expression_operator = ABI_EXPR_OP_INVALID;
  std::string expression_operator_code;
  std::string first_reference;
  std::string second_reference;
  std::string third_reference;
  AbiType owner_type;
  AbiType value_type;
  bool close_template_arguments = false;
  std::string member_name;
  std::string entity_reference;
  std::string symbol;
  bool address_of = false;
  std::vector<std::string> argument_references;
  std::vector<std::string> template_argument_references;
  std::vector<AbiType> type_arguments;
};

struct AbiFunctionPath
{
  std::string qualified_name;
  std::vector<std::string> template_argument_references;
  bool has_result_type = false;
  AbiType result_type;
  std::vector<AbiType> parameter_types;
  bool variadic = false;
};

enum AbiEntityKind
{
  ABI_ENTITY_FUNCTION,
  ABI_ENTITY_VARIABLE
};

struct AbiEntity
{
  AbiEntityKind kind = ABI_ENTITY_VARIABLE;
  AbiFunctionPath function;
  std::string qualified_name;
};

enum AbiFactKind
{
  ABI_FACT_TYPE,
  ABI_FACT_TEMPLATE_ARGUMENT,
  ABI_FACT_EXPRESSION,
  ABI_FACT_LOCAL_CONTEXT,
  ABI_FACT_ENTITY
};

struct AbiFact
{
  AbiFactKind kind = ABI_FACT_TYPE;
  std::string id;
  AbiType type;
  AbiTemplateArg template_argument;
  AbiDependentExpr expression;
  AbiFunctionPath context_function;
  AbiEntity entity;
};

enum AbiFunctionForm
{
  ABI_FUNCTION_PATH,
  ABI_FUNCTION_LAMBDA,
  ABI_FUNCTION_LOCAL
};

enum AbiFunctionTerminal
{
  ABI_FUNCTION_TERMINAL_SOURCE_NAME,
  ABI_FUNCTION_TERMINAL_OPERATOR_CALL,
  ABI_FUNCTION_TERMINAL_OPERATOR_ASSIGN,
  ABI_FUNCTION_TERMINAL_OPERATOR_CODE,
  ABI_FUNCTION_TERMINAL_CONVERSION,
  ABI_FUNCTION_TERMINAL_CONSTRUCTOR_COMPLETE,
  ABI_FUNCTION_TERMINAL_CONSTRUCTOR_BASE,
  ABI_FUNCTION_TERMINAL_DESTRUCTOR_COMPLETE,
  ABI_FUNCTION_TERMINAL_DESTRUCTOR_BASE,
  ABI_FUNCTION_TERMINAL_DESTRUCTOR_DELETING
};

struct AbiFunction
{
  AbiFunctionForm form = ABI_FUNCTION_PATH;
  bool c_linkage = false;
  std::string qualified_name;
  std::vector<std::string> template_argument_references;
  std::string context_reference;
  std::string discriminator;
  std::string source_name;
  AbiFunctionTerminal terminal = ABI_FUNCTION_TERMINAL_SOURCE_NAME;
  std::string terminal_operator_code;
  AbiType conversion_type;
  std::vector<AbiType> lambda_signature_parameter_types;
  bool has_result_type = false;
  AbiType result_type;
  std::vector<AbiType> parameter_types;
  std::vector<std::string> abi_tags;
  bool variadic = false;
  bool nested_const = false;
  bool nested_volatile = false;
  bool nested_lvalue_ref = false;
  bool nested_rvalue_ref = false;
};

enum AbiMangleTargetKind
{
  ABI_MANGLE_NONE,
  ABI_MANGLE_TYPE,
  ABI_MANGLE_FUNCTION,
  ABI_MANGLE_VARIABLE,
  ABI_MANGLE_TYPEINFO,
  ABI_MANGLE_VTABLE,
  ABI_MANGLE_VTT,
  ABI_MANGLE_CONSTRUCTION_VTABLE,
  ABI_MANGLE_THREAD_LOCAL_WRAPPER,
  ABI_MANGLE_THUNK,
  ABI_MANGLE_VIRTUAL_BASE_THUNK
};

struct AbiMangleTarget
{
  AbiMangleTargetKind kind = ABI_MANGLE_NONE;
  AbiType type;
  AbiType base_type;
  AbiFunction function;
  std::string qualified_name;
  unsigned long long base_offset = 0;
  long long this_adjust = 0;
  bool has_result_adjust = false;
  long long result_adjust = 0;
  long long vcall_offset = 0;
  bool c_linkage = false;
};

struct Invocation
{
  std::string outfile;
  std::vector<std::string> inputs;
};

struct AbiFactCase
{
  std::string label;
  std::vector<AbiFact> facts;
  AbiMangleTarget target;
};

struct AbiFactFile
{
  std::vector<AbiFactCase> cases;
};

AbiFactFile parse_fact_text(const std::string & text);
std::string serialize_fact_file(const AbiFactFile & file);
std::string mangle_fact_file(const AbiFactFile & file);
std::string mangle_fact_files(const std::vector<std::string> & input_paths);

}  // namespace abi_mangle
