#include "semantic_parameter_recovery.h"

#include <sstream>
#include <string>
#include <vector>

#include "cpp_decl_bridge.h"

using namespace std;

namespace semantic_parameter_recovery {
namespace {

bool recover_expression(const CppAstNode & node, CppAstNode & out);
bool recover_declarator_expression(const CppAstNode & node, CppAstNode * base, CppAstNode & out);
bool recover_parameter_expression(const CppAstNode & node, CppAstNode & out);

bool is_expression_kind(CppAstKind kind)
{
  switch(kind) {
  case CppAstKind::assignment_expression:
  case CppAstKind::binary_expression:
  case CppAstKind::call_expression:
  case CppAstKind::cast_expression:
  case CppAstKind::conditional_expression:
  case CppAstKind::delete_expression:
  case CppAstKind::fold_expression:
  case CppAstKind::id_expression:
  case CppAstKind::keyword_literal:
  case CppAstKind::lambda_expression:
  case CppAstKind::literal:
  case CppAstKind::member_expression:
  case CppAstKind::new_expression:
  case CppAstKind::pack_expansion_expression:
  case CppAstKind::parenthesized_expression:
  case CppAstKind::postfix_expression:
  case CppAstKind::sizeof_expression:
  case CppAstKind::sizeof_pack_expression:
  case CppAstKind::statement_expression:
  case CppAstKind::subscript_expression:
  case CppAstKind::throw_statement:
  case CppAstKind::type_trait_expression:
  case CppAstKind::unary_expression:
    return true;
  default:
    return false;
  }
}

bool is_identifier_like_leaf(const CppAstNode & node)
{
  switch(node.kind) {
  case CppAstKind::identifier:
  case CppAstKind::decl_specifier:
  case CppAstKind::type_specifier:
  case CppAstKind::type_name:
    break;
  default:
    return false;
  }

  if(node.value.empty()) {
    return false;
  }

  if(node.has_token) {
    return node.token_kind == RT_IDENTIFIER;
  }

  return node.qualified_name_syntax ||
         node.template_id_syntax ||
         node.value.find("::") != string::npos ||
         node.value.find('<') != string::npos;
}

CppAstNode make_id_expression(CppAstNode node)
{
  node.kind = CppAstKind::id_expression;
  node.children.clear();
  return node;
}

void copy_operator_token(CppAstNode & target, const CppAstNode & token_node)
{
  target.has_token = token_node.has_token;
  target.token_kind = token_node.token_kind;
  target.simple_type = token_node.simple_type;
  target.token_start = token_node.token_start;
  target.token_end = token_node.token_end;
}

CppAstNode make_unary_expression(const CppAstNode & op, CppAstNode operand)
{
  CppAstNode out;
  out.kind = CppAstKind::unary_expression;
  out.value = op.value;
  copy_operator_token(out, op);
  out.children.push_back(operand);
  return out;
}

CppAstNode make_binary_expression(const CppAstNode & op, CppAstNode lhs, CppAstNode rhs)
{
  CppAstNode out;
  out.kind = CppAstKind::binary_expression;
  out.value = op.value;
  copy_operator_token(out, op);
  out.children.push_back(lhs);
  out.children.push_back(rhs);
  return out;
}

CppAstNode make_assignment_expression(CppAstNode lhs, CppAstNode rhs)
{
  CppAstNode out;
  out.kind = CppAstKind::assignment_expression;
  out.value = "=";
  out.has_token = true;
  out.token_kind = RT_SIMPLE;
  out.simple_type = OP_ASS;
  out.children.push_back(lhs);
  out.children.push_back(rhs);
  return out;
}

CppAstNode make_parenthesized_expression(CppAstNode child)
{
  CppAstNode out;
  out.kind = CppAstKind::parenthesized_expression;
  out.children.push_back(child);
  return out;
}

CppAstNode make_call_expression(CppAstNode callee, vector<CppAstNode> args)
{
  CppAstNode out;
  out.kind = CppAstKind::call_expression;
  out.children.push_back(callee);

  CppAstNode argument_list;
  argument_list.kind = CppAstKind::argument_list;
  argument_list.children.swap(args);
  out.children.push_back(argument_list);
  return out;
}

CppAstNode make_pack_expansion_expression(CppAstNode pattern)
{
  CppAstNode out;
  out.kind = CppAstKind::pack_expansion_expression;
  out.children.push_back(pattern);
  return out;
}

CppAstNode make_subscript_expression(CppAstNode base, CppAstNode index)
{
  CppAstNode out;
  out.kind = CppAstKind::subscript_expression;
  out.children.push_back(base);
  out.children.push_back(index);
  return out;
}

bool is_declarator_binary_operator(const CppAstNode & node)
{
  return node.kind == CppAstKind::ptr_operator &&
         (node_has_simple_type(node, OP_STAR) ||
          node_has_simple_type(node, OP_AMP) ||
          node_has_simple_type(node, OP_LAND));
}

bool recover_sequence_expression(const CppAstNode & node, CppAstNode & out)
{
  if(node.children.size() != 1) {
    return false;
  }
  return recover_expression(node.children[0], out);
}

bool recover_initializer_expression(const CppAstNode & node, CppAstNode & out)
{
  if(node.kind != CppAstKind::initializer || node.children.size() != 1) {
    return false;
  }
  const CppAstNode & child = node.children[0];
  if(child.kind == CppAstKind::paren_initializer &&
     child.children.size() == 1) {
    return recover_expression(child.children[0], out);
  }
  return recover_expression(child, out);
}

bool recover_parameter_clause_arguments(const CppAstNode & node, vector<CppAstNode> & out)
{
  if(node.kind != CppAstKind::parameter_clause) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    CppAstNode arg;
    if(!recover_parameter_expression(node.children[i], arg)) {
      return false;
    }
    out.push_back(arg);
  }
  return true;
}

bool recover_declarator_from_index(const CppAstNode & node,
                                   size_t index,
                                   CppAstNode * base,
                                   CppAstNode & out)
{
  if(index >= node.children.size()) {
    if(base) {
      out = *base;
      return true;
    }
    return false;
  }

  const CppAstNode & child = node.children[index];
  if(child.kind == CppAstKind::nullability_qualifier) {
    return recover_declarator_from_index(node, index + 1, base, out);
  }

  if(child.kind == CppAstKind::parameter_pack ||
     child.kind == CppAstKind::ellipsis) {
    if(!base) {
      return false;
    }
    CppAstNode current = make_pack_expansion_expression(*base);
    return recover_declarator_from_index(node, index + 1, &current, out);
  }

  if(is_declarator_binary_operator(child)) {
    CppAstNode rhs;
    if(!recover_declarator_from_index(node, index + 1, nullptr, rhs)) {
      return false;
    }
    if(base) {
      out = make_binary_expression(child, *base, rhs);
    } else {
      out = make_unary_expression(child, rhs);
    }
    return true;
  }

  CppAstNode current;
  if(base) {
    current = *base;
  }

  if(child.kind == CppAstKind::nested_declarator) {
    if(child.children.size() != 1) {
      return false;
    }
    CppAstNode nested;
    if(!recover_declarator_expression(child.children[0], nullptr, nested)) {
      return false;
    }
    current = base ? make_call_expression(current, vector<CppAstNode>(1, nested))
                   : make_parenthesized_expression(nested);
  } else if(child.kind == CppAstKind::array_suffix) {
    if(!base || child.children.size() != 1) {
      return false;
    }
    CppAstNode index_expr;
    if(!recover_expression(child.children[0], index_expr)) {
      return false;
    }
    current = make_subscript_expression(current, index_expr);
  } else if(child.kind == CppAstKind::parameter_clause) {
    if(!base) {
      return false;
    }
    vector<CppAstNode> args;
    if(!recover_parameter_clause_arguments(child, args)) {
      return false;
    }
    current = make_call_expression(current, args);
  } else {
    CppAstNode recovered;
    if(!recover_expression(child, recovered)) {
      return false;
    }
    if(base) {
      return false;
    }
    current = recovered;
  }

  return recover_declarator_from_index(node, index + 1, &current, out);
}

bool recover_declarator_expression(const CppAstNode & node, CppAstNode * base, CppAstNode & out)
{
  if(node.kind != CppAstKind::declarator &&
     node.kind != CppAstKind::abstract_declarator) {
    return false;
  }
  return recover_declarator_from_index(node, 0, base, out);
}

bool recover_parameter_expression(const CppAstNode & node, CppAstNode & out)
{
  if(node.kind != CppAstKind::parameter_declaration) {
    return recover_expression(node, out);
  }

  CppAstNode current;
  bool have_current = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::declarator ||
       child.kind == CppAstKind::abstract_declarator) {
      CppAstNode next;
      CppAstNode * base = have_current ? &current : nullptr;
      if(!recover_declarator_expression(child, base, next)) {
        return false;
      }
      current = next;
      have_current = true;
    } else if(child.kind == CppAstKind::default_argument) {
      if(!have_current || child.children.size() != 1) {
        return false;
      }
      CppAstNode rhs;
      if(!recover_initializer_expression(child.children[0], rhs)) {
        return false;
      }
      current = make_assignment_expression(current, rhs);
    } else {
      CppAstNode next;
      if(!recover_expression(child, next)) {
        return false;
      }
      if(have_current) {
        return false;
      }
      current = next;
      have_current = true;
    }
  }

  if(!have_current) {
    return false;
  }
  out = current;
  return true;
}

bool recover_expression(const CppAstNode & node, CppAstNode & out)
{
  if(is_expression_kind(node.kind)) {
    out = node;
    return true;
  }

  if(is_identifier_like_leaf(node)) {
    out = make_id_expression(node);
    return true;
  }

  switch(node.kind) {
  case CppAstKind::decl_specifier_seq:
  case CppAstKind::type_specifier_seq:
    return recover_sequence_expression(node, out);

  case CppAstKind::declarator:
  case CppAstKind::abstract_declarator:
    return recover_declarator_expression(node, nullptr, out);

  case CppAstKind::nested_declarator:
    if(node.children.size() != 1) {
      return false;
    }
    if(!recover_declarator_expression(node.children[0], nullptr, out)) {
      return false;
    }
    out = make_parenthesized_expression(out);
    return true;

  case CppAstKind::initializer:
    return recover_initializer_expression(node, out);

  default:
    return false;
  }
}

}  // namespace

bool recover_parameter_clause_initializer(
    const CppAstNode & parameter_clause,
    CppAstNode & initializer,
    string & error)
{
  if(parameter_clause.kind != CppAstKind::parameter_clause) {
    error = "expected parameter-clause";
    return false;
  }

  CppAstNode paren;
  paren.kind = CppAstKind::paren_initializer;
  for(size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind == CppAstKind::parameter_pack ||
       parameter_clause.children[i].kind == CppAstKind::ellipsis) {
      if(paren.children.empty()) {
        error = "ellipsis has no preceding initializer expression";
        return false;
      }
      paren.children.back() =
          make_pack_expansion_expression(paren.children.back());
      continue;
    }
    CppAstNode expr;
    if(!recover_parameter_expression(parameter_clause.children[i], expr)) {
      ostringstream out;
      out << "failed structured parameter-clause expression recovery";
      out << " [index " << i << "]";
      out << " [parameter " << cpp_decl::node_text(parameter_clause.children[i]) << "]";
      error = out.str();
      return false;
    }
    paren.children.push_back(expr);
  }

  initializer.kind = CppAstKind::initializer;
  initializer.children.clear();
  initializer.children.push_back(paren);
  return true;
}

bool recover_function_style_initializer_declarator(
    const CppAstNode & declarator,
    CppAstNode & stripped_declarator,
    CppAstNode & initializer,
    string & error)
{
  const CppAstNode * parameter_clause =
      cppast_find_child_kind(declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || parameter_clause->children.empty()) {
    error = "no parameter-clause children";
    return false;
  }

  stripped_declarator = declarator;
  vector<CppAstNode> kept;
  for(size_t i = 0; i < stripped_declarator.children.size(); ++i) {
    if(stripped_declarator.children[i].kind != CppAstKind::parameter_clause) {
      kept.push_back(stripped_declarator.children[i]);
    }
  }
  stripped_declarator.children.swap(kept);

  return recover_parameter_clause_initializer(*parameter_clause, initializer, error);
}

}  // namespace semantic_parameter_recovery
