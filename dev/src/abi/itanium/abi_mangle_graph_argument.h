#pragma once

// Canonical template-argument and dependent-expression records used by the
// numeric ABI mangling graph.

#include "abi/itanium/abi_mangle_facts.h"

#include <cstddef>
#include <vector>

namespace abi_mangle {
namespace detail {

struct ExpressionNode
{
  AbiExpressionKind kind = ABI_EXPRESSION_LITERAL;
  std::size_t symbol = ABI_NO_RESOLVED_REFERENCE;
  std::size_t op = ABI_NO_RESOLVED_REFERENCE;
  std::size_t type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t value_type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t entity = ABI_NO_RESOLVED_REFERENCE;
  std::size_t index = 0;
  long long value = 0;
  bool close_member_owner = false;
  bool address_of = false;
  bool entity_resolved = false;
  bool uses_case_facts = false;
  AbiExpressionOperationKind operation = ABI_EXPRESSION_OPERATION_TEXT;
  std::vector<std::size_t> expressions;
  std::vector<std::size_t> arguments;
  std::vector<std::size_t> types;

  bool operator==(const ExpressionNode & other) const
  {
    return kind == other.kind && symbol == other.symbol && op == other.op && type == other.type
           && value_type == other.value_type && entity == other.entity
           && index == other.index && value == other.value
           && close_member_owner == other.close_member_owner
           && address_of == other.address_of
           && entity_resolved == other.entity_resolved
           && operation == other.operation
           && expressions == other.expressions
           && arguments == other.arguments && types == other.types;
  }
};


struct ArgumentNode
{
  AbiTemplateArgumentKind kind = ABI_TEMPLATE_ARGUMENT_TYPE;
  std::size_t type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t value_type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t owner_type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t expression = ABI_NO_RESOLVED_REFERENCE;
  std::size_t entity = ABI_NO_RESOLVED_REFERENCE;
  std::size_t name = ABI_NO_RESOLVED_REFERENCE;
  std::size_t substitution = ABI_NO_RESOLVED_REFERENCE;
  std::size_t symbol = ABI_NO_RESOLVED_REFERENCE;
  std::size_t index = 0;
  long long value = 0;
  bool has_value_type = false;
  bool address_of = false;
  bool pack_expansion = false;
  bool member_is_function = false;
  bool member_const = false;
  bool member_volatile = false;
  bool member_lvalue_ref = false;
  bool member_rvalue_ref = false;
  bool member_variadic = false;
  AbiMemberFunctionTerminalKind member_terminal_kind =
    ABI_MEMBER_FUNCTION_TERMINAL_SOURCE;
  AbiTerminalKind member_terminal_code = ABI_TERMINAL_NONE;
  std::size_t member_literal_suffix = ABI_NO_RESOLVED_REFERENCE;
  std::size_t member_conversion_type = ABI_NO_RESOLVED_REFERENCE;
  std::size_t member_result_type = ABI_NO_RESOLVED_REFERENCE;
  bool member_has_result_type = false;
  bool substitution_resolved = false;
  bool entity_resolved = false;
  bool uses_case_facts = false;
  std::vector<std::size_t> parameters;
  std::vector<std::size_t> arguments;

  bool operator==(const ArgumentNode & other) const;
};

std::size_t argument_node_hash(const ArgumentNode & argument);

}  // namespace detail
}  // namespace abi_mangle
