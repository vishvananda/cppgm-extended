#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cpp_decl_ast.h"

#include "cpp_decl_bridge.h"
#include "types.h"

namespace cpp_decl {

namespace {

bool hosted_decl_specifier_alias(const CppAstNode & node,
                                 ETokenType & mapped,
                                 bool & ignore)
{
  ignore = false;
  if((node.kind != CppAstKind::decl_specifier &&
      node.kind != CppAstKind::type_specifier &&
      node.kind != CppAstKind::cv_qualifier) ||
     node.value.empty()) {
    return false;
  }

  const string & text = node.value;
  if(text == "__signed" || text == "__signed__") {
    mapped = KW_SIGNED;
    return true;
  }
  if(text == "__unsigned" || text == "__unsigned__") {
    mapped = KW_UNSIGNED;
    return true;
  }
  if(text == "__const" || text == "__const__") {
    mapped = KW_CONST;
    return true;
  }
  if(text == "__volatile" || text == "__volatile__") {
    mapped = KW_VOLATILE;
    return true;
  }
  if(text == "__inline" || text == "__inline__") {
    mapped = KW_INLINE;
    return true;
  }
  if(text == "__thread") {
    mapped = KW_THREAD_LOCAL;
    return true;
  }
  if(text == "__extension__") {
    ignore = true;
    return true;
  }
  return false;
}

string ast_named_type_lookup_text(const CppAstNode & node)
{
  if(node.kind == CppAstKind::type_name && !node.value.empty()) {
    return node.value;
  }
  return node_text(node);
}

TypePtr lookup_type_from_ast_node(const AstDeclHooks & hooks,
                                  const CppAstNode & node)
{
  if(node.semantic_type &&
     !(hooks.ignore_semantic_type && hooks.ignore_semantic_type(node.semantic_type))) {
    return node.semantic_type;
  }
  const bool has_structured_lookup_syntax =
      node.qualified_name_syntax ||
      node.template_id_syntax ||
      !node.qualifier_template_id_syntaxes.empty() ||
      !node.qualifier_type_syntaxes.empty();
  if(hooks.lookup_type_node) {
    TypePtr type = hooks.lookup_type_node(node);
    if(type || has_structured_lookup_syntax || !hooks.lookup_type) {
      return type;
    }
  }
  return hooks.lookup_type ? hooks.lookup_type(ast_named_type_lookup_text(node)) :
                             TypePtr();
}

bool try_parse_decltype_specifier(const AstDeclHooks & hooks,
                                  const CppAstNode & node,
                                  TypePtr & out)
{
  if(!hooks.parse_decltype_specifier) {
    return false;
  }

  if(node.kind == CppAstKind::decltype_specifier) {
    return hooks.parse_decltype_specifier(node, out);
  }

  if((node.kind == CppAstKind::decl_specifier || node.kind == CppAstKind::type_specifier) &&
     node.value.find('(') != string::npos) {
    return hooks.parse_decltype_specifier(node, out);
  }

  return false;
}

bool try_parse_atomic_specifier(const AstDeclHooks & hooks,
                                const CppAstNode & node,
                                TypePtr & out)
{
  if((node.kind != CppAstKind::decl_specifier &&
      node.kind != CppAstKind::type_specifier) ||
     node.value != "_Atomic" ||
     node.children.size() != 1) {
    return false;
  }

  TypePtr inner;
  if(!parse_type_id_ast(node.children[0], hooks, inner) || !inner) {
    return false;
  }

  out = make_atomic(inner);
  return true;
}

bool parse_array_bound_fallback(const CppAstNode & node, unsigned long long & bound)
{
  if(node.kind != CppAstKind::literal || node.value.empty()) {
    return false;
  }

  for(size_t i = 0; i < node.value.size(); ++i) {
    if(node.value[i] < '0' || node.value[i] > '9') {
      return false;
    }
  }

  bound = 0;
  string ud_suffix;
  classify_int(node.value, bound, ud_suffix);
  return ud_suffix.empty();
}

bool parse_declarator_core(const CppAstNode & node,
                           const AstDeclHooks & hooks,
                           const TypePtr & base,
                           string & name,
                           TypePtr & out,
                           bool require_name);

bool node_contains_parameter_pack(const CppAstNode & node)
{
  if(node.kind == CppAstKind::parameter_pack) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node_contains_parameter_pack(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool node_has_direct_parameter_pack(const CppAstNode * node)
{
  if(!node) {
    return false;
  }
  for(size_t i = 0; i < node->children.size(); ++i) {
    if(node->children[i].kind == CppAstKind::parameter_pack) {
      return true;
    }
  }
  return false;
}

bool declarator_has_parameter_pack(const CppAstNode * declarator)
{
  return declarator && node_contains_parameter_pack(*declarator);
}

bool node_names_parameter_pack_type(const AstDeclHooks & hooks,
                                    const CppAstNode & node)
{
  if(hooks.type_name_is_parameter_pack &&
     (node.kind == CppAstKind::decl_specifier ||
      node.kind == CppAstKind::type_specifier ||
      node.kind == CppAstKind::type_name) &&
     !node.value.empty() &&
     hooks.type_name_is_parameter_pack(node.value)) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node_names_parameter_pack_type(hooks, node.children[i])) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool decl_spec_contains_token(const CppAstNode & node, ETokenType token)
{
  if(node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::member_specifiers) {
    return false;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if((child.kind == CppAstKind::decl_specifier ||
        child.kind == CppAstKind::specifier) &&
       child.has_token &&
       child.token_kind == RT_SIMPLE &&
       child.simple_type == token) {
      return true;
    }
  }

  return false;
}

bool parse_type_specifier_seq_ast(const CppAstNode & node,
                                  const AstDeclHooks & hooks,
                                  TypePtr & out)
{
  if(node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::type_specifier_seq) {
    return false;
  }

  TypeSpecifierAccumulator acc;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    ETokenType mapped = KW_CONST;
    bool ignore = false;
    if(hosted_decl_specifier_alias(child, mapped, ignore)) {
      if(ignore) {
        continue;
      }
      if(acc.add_cv(mapped) || acc.add_simple_type(mapped)) {
        continue;
      }
      return false;
    }

    if((child.kind == CppAstKind::decl_specifier || child.kind == CppAstKind::type_specifier ||
        child.kind == CppAstKind::cv_qualifier) &&
       child.has_token &&
       (acc.add_cv(child.simple_type) || acc.add_simple_type(child.simple_type))) {
      continue;
    }

    if(child.kind == CppAstKind::type_name) {
      TypePtr alias = lookup_type_from_ast_node(hooks, child);
      if(!alias || !acc.set_named_type(alias)) {
        return false;
      }
      continue;
    }

    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration ||
       child.kind == CppAstKind::enum_specifier) {
      TypePtr alias = lookup_type_from_ast_node(hooks, child);
      if(!alias || !acc.set_named_type(alias)) {
        return false;
      }
      continue;
    }

    TypePtr decltype_type;
    if(try_parse_decltype_specifier(hooks, child, decltype_type)) {
      if(!decltype_type || !acc.set_named_type(decltype_type)) {
        return false;
      }
      continue;
    }

    TypePtr atomic_type;
    if(try_parse_atomic_specifier(hooks, child, atomic_type)) {
      if(!atomic_type || !acc.set_named_type(atomic_type)) {
        return false;
      }
      continue;
    }

    TypePtr alias = lookup_type_from_ast_node(hooks, child);
    if(!alias || !acc.set_named_type(alias)) {
      return false;
    }
  }

  return acc.finalize(out);
}

bool parse_decl_spec_ast(const CppAstNode & node,
                         const AstDeclHooks & hooks,
                         bool & is_typedef,
                         TypePtr & out)
{
  if(node.kind != CppAstKind::decl_specifier_seq) {
    return false;
  }

  is_typedef = false;
  TypeSpecifierAccumulator acc;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    ETokenType mapped = KW_CONST;
    bool ignore = false;
    if(child.kind != CppAstKind::decl_specifier &&
       child.kind != CppAstKind::class_specifier &&
       child.kind != CppAstKind::class_forward_declaration &&
       child.kind != CppAstKind::enum_specifier) {
      return false;
    }

    if(hosted_decl_specifier_alias(child, mapped, ignore)) {
      if(ignore) {
        continue;
      }
      if(acc.add_cv(mapped) || acc.add_simple_type(mapped)) {
        continue;
      }
      if(mapped == KW_INLINE || mapped == KW_THREAD_LOCAL) {
        continue;
      }
      return false;
    }

    if(child.kind == CppAstKind::decl_specifier &&
       child.has_token && child.token_kind == RT_SIMPLE &&
       child.simple_type == KW_TYPEDEF) {
      is_typedef = true;
      continue;
    }

    if(child.kind == CppAstKind::decl_specifier &&
       child.has_token && child.token_kind == RT_SIMPLE &&
       (child.simple_type == KW_EXTERN || child.simple_type == KW_STATIC ||
        child.simple_type == KW_THREAD_LOCAL || child.simple_type == KW_CONSTEXPR ||
        child.simple_type == KW_INLINE ||
        (hooks.allow_virtual_specifier && child.simple_type == KW_VIRTUAL))) {
      continue;
    }

    if(child.kind == CppAstKind::decl_specifier &&
       child.has_token &&
       (acc.add_cv(child.simple_type) || acc.add_simple_type(child.simple_type))) {
      continue;
    }

    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration ||
       child.kind == CppAstKind::enum_specifier) {
      TypePtr alias = lookup_type_from_ast_node(hooks, child);
      if(!alias || !acc.set_named_type(alias)) {
        return false;
      }
      continue;
    }

    TypePtr decltype_type;
    if(try_parse_decltype_specifier(hooks, child, decltype_type)) {
      if(!decltype_type || !acc.set_named_type(decltype_type)) {
        return false;
      }
      continue;
    }

    TypePtr atomic_type;
    if(try_parse_atomic_specifier(hooks, child, atomic_type)) {
      if(!atomic_type || !acc.set_named_type(atomic_type)) {
        return false;
      }
      continue;
    }

    TypePtr alias = lookup_type_from_ast_node(hooks, child);
    if(!alias || !acc.set_named_type(alias)) {
      return false;
    }
  }

  return acc.finalize(out);
}

bool parse_parameter_clause_ast(
    const CppAstNode & node,
    const AstDeclHooks & hooks,
    vector<pair<string, TypePtr> > & params,
    vector<const CppAstNode *> * default_args_out,
    bool * variadic_out)
{
  const auto try_expanded_pack_clause =
      [&]() -> bool
  {
    if(!hooks.expand_parameter_clause_packs) {
      return false;
    }
    CppAstNode expanded_clause;
    vector<const CppAstNode *> expanded_default_args;
    if(!hooks.expand_parameter_clause_packs(
           node,
           expanded_clause,
           default_args_out ? &expanded_default_args : nullptr)) {
      return false;
    }
    vector<pair<string, TypePtr> > expanded_params;
    bool expanded_variadic = false;
    if(!parse_parameter_clause_ast(expanded_clause,
                                   hooks,
                                   expanded_params,
                                   nullptr,
                                   variadic_out ? &expanded_variadic : nullptr)) {
      return false;
    }
    params.swap(expanded_params);
    if(default_args_out) {
      if(expanded_default_args.size() != params.size()) {
        return false;
      }
      default_args_out->swap(expanded_default_args);
    }
    if(variadic_out) {
      *variadic_out = expanded_variadic;
    }
    return true;
  };

  if(node.kind != CppAstKind::parameter_clause) {
    return false;
  }

  params.clear();
  if(default_args_out) {
    default_args_out->clear();
  }
  bool variadic = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::parameter_pack) {
      if(i + 1 != node.children.size()) {
        return false;
      }
      variadic = true;
      break;
    }
    if(child.kind != CppAstKind::parameter_declaration) {
      return false;
    }

    const CppAstNode * specifiers = find_child(child, CppAstKind::decl_specifier_seq);
    const CppAstNode * declarator = find_child(child, CppAstKind::declarator);
    if(!declarator) {
      declarator = find_child(child, CppAstKind::abstract_declarator);
    }
    const bool declarator_has_pack = declarator_has_parameter_pack(declarator);
    const bool declarator_has_direct_pack = node_has_direct_parameter_pack(declarator);
    if(declarator_has_pack) {
      if(hooks.expand_parameter_clause_packs && try_expanded_pack_clause()) {
        return true;
      }
    }
    if(!specifiers) {
      return false;
    }

    TypePtr base;
    bool is_typedef = false;
    try
    {
      if(!parse_decl_spec_ast(*specifiers, hooks, is_typedef, base) || is_typedef) {
        return false;
      }
    }
    catch(const std::logic_error &)
    {
      throw;
    }

    if(declarator_has_direct_pack &&
       !node_names_parameter_pack_type(hooks, *specifiers)) {
      if(i + 1 != node.children.size()) {
        return false;
      }
      variadic = true;
    }

    string name;
    TypePtr type = base;
    if(declarator) {
      try
      {
        if(!parse_declarator_core(*declarator, hooks, base, name, type, false)) {
          return false;
        }
      }
      catch(const std::logic_error &)
      {
        throw;
      }
    }

    if(!type) {
      throw logic_error("parse_parameter_clause_ast: null parameter type");
    }

    params.push_back(
        make_pair(name,
                  hooks.normalize_function_parameters ? normalize_parameter_type(type) :
                                                        type));
    if(hooks.bind_parameter_name) {
      hooks.bind_parameter_name(params.back().first, params.back().second);
    }
    if(default_args_out) {
      default_args_out->push_back(find_child(child, CppAstKind::default_argument));
    }
  }

  if(!variadic &&
     params.size() == 1 &&
     params[0].first.empty() &&
     params[0].second &&
     params[0].second->kind == Type::TK_FUNDAMENTAL &&
     params[0].second->fundamental == FT_VOID) {
    params.clear();
    if(default_args_out) {
      default_args_out->clear();
    }
  }

  if(variadic_out) {
    *variadic_out = variadic;
  }
  return true;
}

bool parse_declarator_ast(const CppAstNode & node,
                          const AstDeclHooks & hooks,
                          const TypePtr & base,
                          string & name,
                          TypePtr & out)
{
  return parse_declarator_core(node, hooks, base, name, out, true);
}

bool parse_abstract_declarator_ast(const CppAstNode & node,
                                   const AstDeclHooks & hooks,
                                   const TypePtr & base,
                                   TypePtr & out)
{
  string ignored_name;
  return parse_declarator_core(node, hooks, base, ignored_name, out, false);
}

bool parse_type_id_ast(const CppAstNode & node,
                       const AstDeclHooks & hooks,
                       TypePtr & out)
{
  if(node.kind != CppAstKind::type_id || node.children.empty()) {
    return false;
  }

  if(!parse_type_specifier_seq_ast(node.children[0], hooks, out)) {
    return false;
  }

  const CppAstNode * abstract = find_child(node, CppAstKind::abstract_declarator);
  if(!abstract) {
    return true;
  }

  string ignored_name;
  return parse_declarator_core(*abstract, hooks, out, ignored_name, out, false);
}

namespace {

bool array_bound_mentions_id_expression(const CppAstNode & node)
{
  if(node.kind == CppAstKind::id_expression) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(array_bound_mentions_id_expression(node.children[i])) {
      return true;
    }
  }
  return false;
}

string trim_space_copy(const string & text)
{
  size_t start = 0;
  while(start < text.size() && isspace(static_cast<unsigned char>(text[start]))) {
    ++start;
  }
  size_t end = text.size();
  while(end > start && isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(start, end - start);
}

bool parse_declarator_core(const CppAstNode & node,
                           const AstDeclHooks & hooks,
                           const TypePtr & base,
                           string & name,
                           TypePtr & out,
                           bool require_name)
{
  if(node.kind != CppAstKind::declarator && node.kind != CppAstKind::abstract_declarator) {
    return false;
  }

  name.clear();
  out = base;
  vector<PtrOperator> prefixes;
  vector<DeclaratorSuffix> suffixes;
  const CppAstNode * nested = nullptr;

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::ptr_operator) {
      if(!child.has_token) {
        const string text = trim_space_copy(child.value);
        if(text.size() <= 3 || text.compare(text.size() - 3, 3, "::*") != 0 ||
           !hooks.lookup_type) {
          return false;
        }
        const string owner_text = trim_space_copy(text.substr(0, text.size() - 3));
        TypePtr owner_type;
        if(hooks.lookup_type_node) {
          CppAstNode owner_node = child;
          owner_node.kind = CppAstKind::type_name;
          owner_node.value = owner_text;
          owner_type = hooks.lookup_type_node(owner_node);
        }
        if(!owner_type) {
          owner_type = hooks.lookup_type(owner_text);
        }
        if(!owner_type) {
          return false;
        }
        PtrOperator op(PtrOperator::PK_MEMBER_POINTER);
        op.owner = owner_type;
        prefixes.push_back(op);
        continue;
      }
      if(child.simple_type == OP_STAR) {
        prefixes.push_back(PtrOperator(PtrOperator::PK_POINTER));
      } else if(child.simple_type == OP_XOR) {
        prefixes.push_back(PtrOperator(PtrOperator::PK_BLOCK_POINTER));
      } else if(child.simple_type == OP_AMP) {
        prefixes.push_back(PtrOperator(PtrOperator::PK_LVALUE_REFERENCE));
      } else if(child.simple_type == OP_LAND) {
        prefixes.push_back(PtrOperator(PtrOperator::PK_RVALUE_REFERENCE));
      } else {
        return false;
      }
      continue;
    }

    if(child.kind == CppAstKind::cv_qualifier) {
      if(!child.has_token) {
        return false;
      }
      if(!prefixes.empty() &&
         (prefixes.back().kind == PtrOperator::PK_POINTER ||
          prefixes.back().kind == PtrOperator::PK_MEMBER_POINTER ||
          prefixes.back().kind == PtrOperator::PK_BLOCK_POINTER)) {
        if(child.simple_type == KW_CONST) {
          prefixes.back().cv_const = true;
        } else if(child.simple_type == KW_VOLATILE) {
          prefixes.back().cv_volatile = true;
        } else {
          return false;
        }
        continue;
      }
      if(!suffixes.empty() && suffixes.back().kind == DeclaratorSuffix::SK_FUNCTION) {
        if(child.simple_type == KW_CONST) {
          suffixes.back().function_const = true;
        } else if(child.simple_type == KW_VOLATILE) {
          suffixes.back().function_volatile = true;
        } else {
          return false;
        }
        continue;
      }
      return false;
    }

    if(child.kind == CppAstKind::nullability_qualifier) {
      if(prefixes.empty()) {
        return false;
      }
      continue;
    }

    if(child.kind == CppAstKind::identifier) {
      if(!name.empty()) {
        return false;
      }
      name = child.value;
      continue;
    }

    if(child.kind == CppAstKind::parameter_pack) {
      continue;
    }

    if(child.kind == CppAstKind::nested_declarator) {
      if(child.children.size() != 1 || nested) {
        return false;
      }
      nested = &child.children[0];
      continue;
    }

    if(child.kind == CppAstKind::parameter_clause) {
      DeclaratorSuffix suffix(DeclaratorSuffix::SK_FUNCTION);
      vector<pair<string, TypePtr> > params;
      if(!parse_parameter_clause_ast(child, hooks, params, nullptr, &suffix.variadic)) {
        return false;
      }
      for(size_t j = 0; j < params.size(); ++j) {
        suffix.params.push_back(
            hooks.normalize_function_parameters ? normalize_parameter_type(params[j].second) :
                                                  params[j].second);
      }
      suffixes.push_back(suffix);
      continue;
    }

    if(child.kind == CppAstKind::array_suffix) {
      DeclaratorSuffix suffix(DeclaratorSuffix::SK_ARRAY);
      if(!child.children.empty()) {
        if(child.children.size() != 1) {
          return false;
        }
        long long bound_value = 0;
        unsigned long long literal_bound = 0;
        bool evaluated_bound = false;
        if(hooks.evaluate_constant_expression) {
          try {
            evaluated_bound = hooks.evaluate_constant_expression(child.children[0], bound_value);
          } catch(const logic_error &) {
            evaluated_bound = false;
          }
        }
        if(evaluated_bound && bound_value >= 0) {
          suffix.has_bound = true;
          suffix.has_evaluated_bound = true;
          suffix.bound_value = static_cast<unsigned long long>(bound_value);
        } else if(parse_array_bound_fallback(child.children[0], literal_bound)) {
          suffix.has_bound = true;
          suffix.has_evaluated_bound = true;
          suffix.bound_value = literal_bound;
        } else if((hooks.array_bound_is_dependent &&
                   hooks.array_bound_is_dependent(child.children[0])) ||
                  array_bound_mentions_id_expression(child.children[0])) {
          suffix.has_bound = false;
          suffix.bound_text = node_text(child.children[0]);
        } else {
          return false;
        }
      }
      suffixes.push_back(suffix);
      continue;
    }

    return false;
  }

  for(size_t i = 0; i < prefixes.size(); ++i) {
    if(prefixes[i].kind == PtrOperator::PK_POINTER) {
      if(out->kind == Type::TK_LVALUE_REFERENCE ||
         out->kind == Type::TK_RVALUE_REFERENCE) {
        return false;
      }
      out = apply_cv(make_pointer(out), prefixes[i].cv_const, prefixes[i].cv_volatile);
    } else if(prefixes[i].kind == PtrOperator::PK_MEMBER_POINTER) {
      out = apply_cv(make_member_pointer(prefixes[i].owner, out),
                     prefixes[i].cv_const,
                     prefixes[i].cv_volatile);
    } else if(prefixes[i].kind == PtrOperator::PK_BLOCK_POINTER) {
      out = apply_cv(make_block_pointer(out), prefixes[i].cv_const, prefixes[i].cv_volatile);
    } else if(prefixes[i].kind == PtrOperator::PK_LVALUE_REFERENCE) {
      out = make_lvalue_reference_raw(out);
    } else {
      out = make_rvalue_reference_raw(out);
    }
  }

  for(size_t i = suffixes.size(); i > 0; --i) {
    const DeclaratorSuffix & suffix = suffixes[i - 1];
    if(suffix.kind == DeclaratorSuffix::SK_FUNCTION) {
      out = make_function(out,
                          suffix.params,
                          suffix.variadic,
                          suffix.function_const,
                          suffix.function_volatile);
    } else {
      out = make_array(out, suffix.has_bound,
                       suffix.has_bound ? static_cast<size_t>(suffix.bound_value) : 0,
                       suffix.bound_text);
    }
  }

  if(nested) {
    return parse_declarator_core(*nested, hooks, out, name, out, require_name);
  }

  return !require_name || !name.empty();
}

}  // namespace

}  // namespace cpp_decl
