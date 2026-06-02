#include "typesemantic.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cpp_decl_bridge.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_model.h"
#include "cpp_scope_lookup.h"
#include "cppast_dump.h"
#include "cppast_parser.h"
#include "constexpr_eval.h"
#include "cpp_syntax.h"
#include "semantic_consteval.h"
#include "semantic_class_model.h"
#include "semantic_utils.h"

namespace {

using namespace cpp_decl;

const CppAstNode * find_function_body_node(const CppAstNode & node)
{
  if(const CppAstNode * body = find_child(node, CppAstKind::compound_statement)) {
    return body;
  }
  if(const CppAstNode * body = find_child(node, CppAstKind::lazy_function_body)) {
    return body;
  }
  return find_child(node, CppAstKind::try_block);
}

struct Scope;

enum BindingKind
{
  BK_TYPE,
  BK_TYPE_ALIAS,
  BK_ENUMERATOR,
  BK_FUNCTION,
  BK_VARIABLE,
  BK_PARAMETER
};

enum NamedTypeKind
{
  NTK_TYPE,
  NTK_TYPE_ALIAS
};

const char * binding_kind_text(BindingKind kind)
{
  switch(kind) {
  case BK_TYPE: return "type";
  case BK_TYPE_ALIAS: return "type-alias";
  case BK_ENUMERATOR: return "enumerator";
  case BK_FUNCTION: return "function";
  case BK_VARIABLE: return "variable";
  case BK_PARAMETER: return "parameter";
  }

  throw logic_error("unknown binding kind");
}

bool is_value_binding_kind(BindingKind kind)
{
  return kind == BK_VARIABLE || kind == BK_FUNCTION ||
         kind == BK_ENUMERATOR || kind == BK_PARAMETER;
}

BindingKind binding_kind_for_named_type_kind(NamedTypeKind kind)
{
  switch(kind) {
  case NTK_TYPE: return BK_TYPE;
  case NTK_TYPE_ALIAS: return BK_TYPE_ALIAS;
  }

  throw logic_error("unknown named type kind");
}

struct Binding
{
  BindingKind kind;
  string name;
  TypePtr type;
  bool has_value;
  long long value;

  Binding(BindingKind kind, const string & name, const TypePtr & type)
    : kind(kind), name(name), type(type), has_value(false), value(0)
  {}
};

struct NamedTypeBinding
{
  NamedTypeKind kind;
  TypePtr type;

  NamedTypeBinding() : kind(NTK_TYPE) {}

  NamedTypeBinding(NamedTypeKind kind, const TypePtr & type)
    : kind(kind), type(type)
  {}
};

struct Scope
{
  enum Kind
  {
    SK_NAMESPACE,
    SK_TEMPLATE_PARAMETERS,
    SK_CLASS,
    SK_ENUM,
    SK_FUNCTION,
    SK_BLOCK
  };

  Scope(Kind kind, const string & name, Scope * parent)
    : kind(kind), name(name), parent(parent)
  {}

  Kind kind;
  string name;
  Scope * parent;
  vector<Binding> bindings;
  vector<unique_ptr<Scope> > children;
  map<string, NamedTypeBinding> named_types;
  map<string, Scope *> namespace_bindings;
  map<string, Scope *> type_scopes;
  map<string, long long> constant_values;
  vector<Scope *> using_directives;
};

void inject_anonymous_union_member_bindings(Scope & scope, const Scope & union_scope)
{
  for(size_t i = 0; i < union_scope.bindings.size(); ++i) {
    const Binding & binding = union_scope.bindings[i];
    if(binding.kind != BK_VARIABLE) {
      continue;
    }
    for(size_t j = 0; j < scope.bindings.size(); ++j) {
      if(scope.bindings[j].name == binding.name) {
        throw logic_error("duplicate anonymous union member " + binding.name);
      }
    }
    scope.bindings.push_back(binding);
    if(binding.has_value) {
      scope.constant_values[binding.name] = binding.value;
    }
  }
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  return find_child(node, kind);
}

void indent(ostringstream & out, size_t depth)
{
  for(size_t i = 0; i < depth; ++i) {
    out << "  ";
  }
}

using semantic_utils::trim_space;

struct ConstantLookupResult
{
  bool found;
  long long value;

  ConstantLookupResult() : found(false), value(0) {}
  explicit ConstantLookupResult(long long value) : found(true), value(value) {}
};

inline bool operator==(const ConstantLookupResult & lhs,
                       const ConstantLookupResult & rhs)
{
  return lhs.found == rhs.found && (!lhs.found || lhs.value == rhs.value);
}

inline bool has_lookup_result(const TypePtr & result)
{
  return static_cast<bool>(result);
}

template<typename T>
inline bool has_lookup_result(T * result)
{
  return result != nullptr;
}

inline bool has_lookup_result(const ConstantLookupResult & result)
{
  return result.found;
}

template<typename Result>
bool lookup_result_present(const Result & result)
{
  return has_lookup_result(result);
}

class Analyzer
{
public:
  explicit Analyzer(const CppAstNode & ast)
    : ast(ast)
  {
    root_scope.reset(new Scope(Scope::SK_NAMESPACE, "<global>", nullptr));
  }

  string describe()
  {
    if(ast.kind != CppAstKind::translation_unit) {
      throw logic_error("expected translation-unit");
    }

    for(size_t i = 0; i < ast.children.size(); ++i) {
      analyze_declaration(*root_scope, ast.children[i]);
    }

    ostringstream out;
    out << "translation-unit\n";
    describe_scope(out, *root_scope, 1);
    return out.str();
  }

private:
  const CppAstNode & ast;
  unique_ptr<Scope> root_scope;
  map<string, Scope *> named_type_scopes;
  vector<unique_ptr<CppAstNode> > synthetic_ast_nodes;

  const CppAstNode * own_synthetic_ast(CppAstNode node)
  {
    synthetic_ast_nodes.push_back(
        unique_ptr<CppAstNode>(new CppAstNode(std::move(node))));
    return synthetic_ast_nodes.back().get();
  }

  void describe_scope(ostringstream & out, const Scope & scope, size_t depth) const
  {
    indent(out, depth);
    if(scope.kind == Scope::SK_NAMESPACE) {
      out << "scope namespace " << scope.name << "\n";
    } else if(scope.kind == Scope::SK_TEMPLATE_PARAMETERS) {
      out << "scope template-parameters\n";
    } else if(scope.kind == Scope::SK_CLASS) {
      out << "scope class " << scope.name << "\n";
    } else if(scope.kind == Scope::SK_ENUM) {
      out << "scope enum " << scope.name << "\n";
    } else if(scope.kind == Scope::SK_FUNCTION) {
      out << "scope function " << scope.name << "\n";
    } else {
      out << "scope block\n";
    }

    for(size_t i = 0; i < scope.bindings.size(); ++i) {
      indent(out, depth + 1);
      out << binding_kind_text(scope.bindings[i].kind) << " "
          << scope.bindings[i].name;
      if(scope.bindings[i].type) {
        out << " " << describe_type(scope.bindings[i].type);
      }
      if(scope.bindings[i].has_value) {
        out << " " << scope.bindings[i].value;
      }
      out << "\n";
    }

    for(size_t i = 0; i < scope.children.size(); ++i) {
      describe_scope(out, *scope.children[i], depth + 1);
    }
  }

  Scope & append_child_scope(Scope & parent, Scope::Kind kind, const string & name)
  {
    parent.children.emplace_back(new Scope(kind, name, &parent));
    return *parent.children.back();
  }

  Scope * find_named_namespace_child(Scope & scope, const string & name)
  {
    for(size_t i = 0; i < scope.children.size(); ++i) {
      Scope * child = scope.children[i].get();
      if(child->kind == Scope::SK_NAMESPACE && child->name == name) {
        return child;
      }
    }
    return nullptr;
  }

  Scope * resolve_direct_namespace(Scope & scope, const string & name)
  {
    map<string, Scope *>::iterator it = scope.namespace_bindings.find(name);
    return it == scope.namespace_bindings.end() ? nullptr : it->second;
  }

  Scope * resolve_direct_type_scope(Scope & scope, const string & name)
  {
    map<string, Scope *>::iterator it = scope.type_scopes.find(name);
    return it == scope.type_scopes.end() ? nullptr : it->second;
  }

  Scope * resolve_direct_qualifier(Scope & scope, const string & name)
  {
    Scope * direct_namespace = resolve_direct_namespace(scope, name);
    if(direct_namespace) {
      return direct_namespace;
    }
    return resolve_direct_type_scope(scope, name);
  }

  TypePtr lookup_named_type_direct(Scope & scope, const string & name)
  {
    map<string, NamedTypeBinding>::iterator it = scope.named_types.find(name);
    return it == scope.named_types.end() ? TypePtr() : it->second.type;
  }

  const NamedTypeBinding * lookup_named_type_binding_direct(const Scope & scope,
                                                            const string & name)
  {
    map<string, NamedTypeBinding>::const_iterator it = scope.named_types.find(name);
    return it == scope.named_types.end() ? nullptr : &it->second;
  }

  ConstantLookupResult lookup_constant_direct(Scope & scope, const string & name)
  {
    map<string, long long>::iterator it = scope.constant_values.find(name);
    if(it == scope.constant_values.end()) {
      return ConstantLookupResult();
    }
    return ConstantLookupResult(it->second);
  }

  template<typename Result, typename DirectLookup>
  Result lookup_unqualified_generic(Scope & scope,
                                    const string & name,
                                    const DirectLookup & direct_lookup)
  {
    return cpp_scope_lookup::lookup_unqualified<Result>(
        scope, name, direct_lookup, lookup_result_present<Result>);
  }

  template<typename Result, typename FinalLookup>
  Result lookup_qualified_generic(Scope & scope,
                                  const QualifiedName & qualified,
                                  const FinalLookup & final_lookup)
  {
    return cpp_scope_lookup::lookup_qualified<Result>(
        *root_scope, qualified,
        [this, &scope](const string & name) -> Scope *
        {
          return lookup_unqualified_generic<Scope *>(
              scope, name,
              [this](Scope & target, const string & lookup_name) -> Scope *
              {
                return resolve_direct_qualifier(target, lookup_name);
              });
        },
        [this](Scope & target, const string & lookup_name) -> Scope *
        {
          return resolve_direct_qualifier(target, lookup_name);
        },
        final_lookup);
  }

  string scope_qualified_name(const Scope & scope, const string & name) const
  {
    vector<string> parts;
    parts.push_back(name);
    for(const Scope * current = &scope; current; current = current->parent) {
      if(current->kind == Scope::SK_NAMESPACE && current->name != "<global>" &&
         current->name != "<unnamed>") {
        parts.push_back(current->name);
      } else if(current->kind == Scope::SK_CLASS && !current->name.empty()) {
        parts.push_back(current->name);
      }
    }

    string out;
    for(size_t i = parts.size(); i > 0; --i) {
      if(!out.empty()) {
        out += "::";
      }
      out += parts[i - 1];
    }
    return out;
  }

  TypePtr make_class_type(Scope & scope,
                          const string & class_key,
                          const string & name,
                          bool complete)
  {
    string display = class_key + " " + name;
    string key = class_key + " " + scope_qualified_name(scope, name);
    return make_named(display, key, complete);
  }

  TypePtr make_enum_type(Scope & scope,
                         const string & enum_prefix,
                         const string & name,
                         bool complete,
                         size_t alignment = 4,
                         size_t size = 4)
  {
    string display = enum_prefix + " " + name;
    string key = enum_prefix + " " + scope_qualified_name(scope, name);
    return make_named(display, key, complete, true, alignment, size);
  }

  Scope * lookup_namespace_unqualified(Scope & scope, const string & name)
  {
    return lookup_unqualified_generic<Scope *>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> Scope *
        {
          return resolve_direct_namespace(target, lookup_name);
        });
  }

  Scope * lookup_namespace_from_using_directives_in_scope(Scope & scope,
                                                          const string & name)
  {
    bool found = false;
    bool ambiguous = false;
    Scope * value = nullptr;
    set<const Scope *> visited;
    cpp_scope_lookup::collect_lookup_from_using_directives<Scope *>(
        scope,
        name,
        visited,
        [this](Scope & target, const string & lookup_name) -> Scope *
        {
          return resolve_direct_namespace(target, lookup_name);
        },
        lookup_result_present<Scope *>,
        found,
        value,
        ambiguous);
    return ambiguous ? nullptr : value;
  }

  Scope * lookup_namespace_member_in_qualified_scope(Scope & scope,
                                                     const string & name)
  {
    Scope * direct = resolve_direct_namespace(scope, name);
    if(direct) {
      return direct;
    }
    return lookup_namespace_from_using_directives_in_scope(scope, name);
  }

  Scope * lookup_type_scope_unqualified(Scope & scope, const string & name)
  {
    return lookup_unqualified_generic<Scope *>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> Scope *
        {
          return resolve_direct_type_scope(target, lookup_name);
        });
  }

  Scope * lookup_qualifier_unqualified(Scope & scope, const string & name)
  {
    return lookup_unqualified_generic<Scope *>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> Scope *
        {
          return resolve_direct_qualifier(target, lookup_name);
        });
  }

  Scope * lookup_namespace_name(Scope & scope, const QualifiedName & qualified)
  {
    if(!qualified.rooted && qualified.qualifiers.empty()) {
      QualifiedName split_name;
      if(qualified.name.find("::") != string::npos &&
         semantic_utils::split_qualified_name_text(qualified.name, split_name) &&
         (split_name.rooted || !split_name.qualifiers.empty())) {
        return lookup_namespace_name(scope, split_name);
      }
      return lookup_namespace_unqualified(scope, qualified.name);
    }

    Scope * current = qualified.rooted ? root_scope.get() :
        lookup_namespace_unqualified(scope, qualified.qualifiers[0]);
    size_t next = qualified.rooted ? 0 : 1;
    if(!current) {
      return nullptr;
    }

    while(next < qualified.qualifiers.size()) {
      current = lookup_namespace_member_in_qualified_scope(
          *current,
          qualified.qualifiers[next]);
      if(!current) {
        return nullptr;
      }
      ++next;
    }

    if(qualified.name.empty()) {
      return current;
    }

    return lookup_namespace_member_in_qualified_scope(*current, qualified.name);
  }

  TypePtr lookup_qualified_type(Scope & scope, const QualifiedName & qualified)
  {
    const NamedTypeBinding * binding = lookup_qualified_generic<const NamedTypeBinding *>(
        scope, qualified,
        [this](Scope & target, const string & lookup_name) -> const NamedTypeBinding *
        {
          return lookup_named_type_binding_direct(target, lookup_name);
        });
    return binding ? binding->type : TypePtr();
  }

  TypePtr lookup_type(Scope & scope, const QualifiedName & qualified)
  {
    if(qualified.rooted || !qualified.qualifiers.empty()) {
      return lookup_qualified_type(scope, qualified);
    }
    return lookup_unqualified_generic<TypePtr>(
        scope, qualified.name,
        [this](Scope & target, const string & lookup_name) -> TypePtr
        {
          return lookup_named_type_direct(target, lookup_name);
        });
  }

  TypePtr lookup_type(Scope & scope, const string & name)
  {
    QualifiedName qualified;
    if(semantic_utils::split_qualified_name_text(name, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      return lookup_type(scope, qualified);
    }

    return lookup_unqualified_generic<TypePtr>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> TypePtr
        {
          return lookup_named_type_direct(target, lookup_name);
        });
  }

  Scope * lookup_type_scope(Scope & scope, const QualifiedName & qualified)
  {
    if(qualified.rooted || !qualified.qualifiers.empty()) {
      return lookup_qualified_generic<Scope *>(
          scope, qualified,
          [this](Scope & target, const string & lookup_name) -> Scope *
          {
            return resolve_direct_type_scope(target, lookup_name);
          });
    }

    return lookup_type_scope_unqualified(scope, qualified.name);
  }

  Scope * lookup_type_scope(Scope & scope, const string & name)
  {
    QualifiedName qualified;
    if(semantic_utils::split_qualified_name_text(name, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      return lookup_type_scope(scope, qualified);
    }

    return lookup_type_scope_unqualified(scope, name);
  }

  bool lookup_constant_unqualified(Scope & scope, const string & name, long long & out)
  {
    ConstantLookupResult result = lookup_unqualified_generic<ConstantLookupResult>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> ConstantLookupResult
        {
          return lookup_constant_direct(target, lookup_name);
        });
    if(!result.found) {
      return false;
    }
    out = result.value;
    return true;
  }

  bool lookup_qualified_constant(Scope & scope,
                                 const QualifiedName & qualified,
                                 long long & out)
  {
    ConstantLookupResult result = lookup_qualified_generic<ConstantLookupResult>(
        scope, qualified,
        [this](Scope & target, const string & lookup_name) -> ConstantLookupResult
        {
          return lookup_constant_direct(target, lookup_name);
        });
    if(!result.found) {
      return false;
    }
    out = result.value;
    return true;
  }

  bool lookup_constant_value(Scope & scope,
                             const QualifiedName & qualified,
                             long long & out)
  {
    if(qualified.rooted || !qualified.qualifiers.empty()) {
      return lookup_qualified_constant(scope, qualified, out);
    }
    return lookup_constant_unqualified(scope, qualified.name, out);
  }

  bool lookup_constant_value(Scope & scope, const string & name, long long & out)
  {
    QualifiedName qualified;
    if(semantic_utils::split_qualified_name_text(name, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      return lookup_constant_value(scope, qualified, out);
    }

    return lookup_constant_unqualified(scope, name, out);
  }

  const Binding *lookup_value_binding_in_scope(const Scope & scope, const string & name)
  {
    for(size_t i = scope.bindings.size(); i > 0; --i) {
      const Binding & binding = scope.bindings[i - 1];
      if(binding.name == name && is_value_binding_kind(binding.kind)) {
        return &binding;
      }
    }
    return nullptr;
  }

  const Binding *lookup_value_binding_unqualified(Scope & scope, const string & name)
  {
    return lookup_unqualified_generic<const Binding *>(
        scope, name,
        [this](Scope & target, const string & lookup_name) -> const Binding *
        {
          return lookup_value_binding_in_scope(target, lookup_name);
        });
  }

  const Binding *lookup_qualified_value_binding(Scope & scope,
                                                const QualifiedName & qualified)
  {
    return lookup_qualified_generic<const Binding *>(
        scope, qualified,
        [this](Scope & target, const string & lookup_name) -> const Binding *
        {
          return lookup_value_binding_in_scope(target, lookup_name);
        });
  }

  TypePtr lookup_value_in_scope(const Scope & scope, const string & name)
  {
    const Binding *binding = lookup_value_binding_in_scope(scope, name);
    return binding ? binding->type : TypePtr();
  }

  TypePtr lookup_qualified_value_type(Scope & scope, const QualifiedName & qualified)
  {
    const Binding *binding = lookup_qualified_value_binding(scope, qualified);
    return binding ? binding->type : TypePtr();
  }

  TypePtr lookup_value_type(Scope & scope, const QualifiedName & qualified)
  {
    if(qualified.rooted || !qualified.qualifiers.empty()) {
      return lookup_qualified_value_type(scope, qualified);
    }

    const Binding *binding = lookup_value_binding_unqualified(scope, qualified.name);
    return binding ? binding->type : TypePtr();
  }

  TypePtr lookup_value_type(Scope & scope, const string & name)
  {
    QualifiedName qualified;
    if(semantic_utils::split_qualified_name_text(name, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      return lookup_value_type(scope, qualified);
    }

    const Binding *binding = lookup_value_binding_unqualified(scope, name);
    return binding ? binding->type : TypePtr();
  }

  Scope * scope_for_type(const TypePtr & type)
  {
    TypePtr base = strip_top_level_cv(type);
    if(!base || base->kind != Type::TK_NAMED) {
      return nullptr;
    }

    map<string, Scope *>::iterator it = named_type_scopes.find(base->named_key);
    return it == named_type_scopes.end() ? nullptr : it->second;
  }

  void maybe_add_type_scope_binding(Scope & scope,
                                    const string & name,
                                    const TypePtr & type)
  {
    Scope * target_scope = scope_for_type(type);
    if(target_scope) {
      scope.type_scopes[name] = target_scope;
    }
  }

  constant_eval::Hooks make_consteval_hooks(Scope & scope)
  {
  constant_eval::Hooks hooks;
  hooks.lookup_external_value =
        [&scope, this](const string & name,
                       const CppAstNode *,
                       constant_eval::ConstexprValue & value)
        {
          long long raw = 0;
          if(!lookup_constant_value(scope, name, raw)) {
            return false;
          }
          value = constant_eval::make_integral_value(raw, lookup_value_type(scope, name));
          return true;
        };
    hooks.lookup_type = [this, &scope](const string & name) {
      return lookup_type(scope, name);
    };
    hooks.parse_type_id = [this, &scope](const CppAstNode & type_id, TypePtr & type) {
      return parse_type_id(scope, type_id, type);
    };
    hooks.evaluate_sizeof_operand =
        [this, &scope](const CppAstNode & expr, size_t & size)
        {
          if(expr.kind != CppAstKind::id_expression) {
            return false;
          }
          TypePtr type = lookup_type(scope, expr.value);
          if(!type) {
            return false;
          }
          size = type_size(type);
          return true;
        };
    return hooks;
  }

  bool evaluate_initializer_constant(Scope & scope,
                                     const CppAstNode & initializer,
                                     long long & value)
  {
    constant_eval::Evaluator evaluator(make_consteval_hooks(scope));
    constant_eval::ConstexprValue out;
    return evaluator.eval_initializer(initializer, out) &&
           constant_eval::constexpr_value_to_integral(out, value);
  }

  bool parse_decltype_specifier(Scope & scope,
                                const string & text,
                                TypePtr & out)
  {
    const string decltype_prefix = "decltype(";
    const string decltype_gnu_prefix = "__decltype(";
    const string decltype_gnu_alt_prefix = "__decltype__(";
    const string typeof_prefix = "__typeof(";
    const string typeof_alt_prefix = "__typeof__(";

    size_t prefix_size = 0;
    bool is_typeof = false;
    if(text.size() >= decltype_prefix.size() + 1 &&
       text.compare(0, decltype_prefix.size(), decltype_prefix) == 0 &&
       text[text.size() - 1] == ')') {
      prefix_size = decltype_prefix.size();
    } else if(text.size() >= decltype_gnu_prefix.size() + 1 &&
              text.compare(0, decltype_gnu_prefix.size(), decltype_gnu_prefix) == 0 &&
              text[text.size() - 1] == ')') {
      prefix_size = decltype_gnu_prefix.size();
    } else if(text.size() >= decltype_gnu_alt_prefix.size() + 1 &&
              text.compare(0, decltype_gnu_alt_prefix.size(), decltype_gnu_alt_prefix) == 0 &&
              text[text.size() - 1] == ')') {
      prefix_size = decltype_gnu_alt_prefix.size();
    } else if(text.size() >= typeof_prefix.size() + 1 &&
              text.compare(0, typeof_prefix.size(), typeof_prefix) == 0 &&
              text[text.size() - 1] == ')') {
      prefix_size = typeof_prefix.size();
      is_typeof = true;
    } else if(text.size() >= typeof_alt_prefix.size() + 1 &&
              text.compare(0, typeof_alt_prefix.size(), typeof_alt_prefix) == 0 &&
              text[text.size() - 1] == ')') {
      prefix_size = typeof_alt_prefix.size();
      is_typeof = true;
    } else {
      return false;
    }

    string inner = trim_space(text.substr(prefix_size, text.size() - prefix_size - 1));
    bool parenthesized = false;
    if(!is_typeof && semantic_utils::is_wrapped_in_balanced_parens(inner)) {
      parenthesized = true;
      inner = trim_space(inner.substr(1, inner.size() - 2));
    }

    if(inner.empty()) {
      return false;
    }

    if(is_typeof) {
      TypePtr alias = lookup_type(scope, inner);
      if(alias) {
        out = alias;
        return true;
      }
    }

    TypePtr value_type = lookup_value_type(scope, inner);
    if(!value_type) {
      return false;
    }

    out = (parenthesized && !is_typeof) ? make_lvalue_reference_raw(value_type) : value_type;
    return true;
  }

  bool evaluate_constant_expression(Scope & scope,
                                    const CppAstNode & node,
                                    long long & out)
  {
    constant_eval::Evaluator evaluator(make_consteval_hooks(scope));
    constant_eval::ConstexprValue value;
    return evaluator.eval_expr(node, value) &&
           constant_eval::constexpr_value_to_integral(value, out);
  }

  AstDeclHooks make_decl_hooks(Scope & scope)
  {
    AstDeclHooks hooks;
    hooks.lookup_type = [this, &scope](const string & name) {
      return lookup_type(scope, name);
    };
    hooks.parse_decltype_specifier = [this, &scope](const CppAstNode & node, TypePtr & out) {
      return parse_decltype_specifier(scope, node.value, out);
    };
    hooks.evaluate_constant_expression = [this, &scope](const CppAstNode & node, long long & out) {
      return evaluate_constant_expression(scope, node, out);
    };
    hooks.allow_virtual_specifier = true;
    return hooks;
  }

  bool parse_type_specifier_seq(Scope & scope,
                                const CppAstNode & node,
                                TypePtr & out)
  {
    return parse_type_specifier_seq_ast(node, make_decl_hooks(scope), out);
  }

  bool parse_decl_spec(const CppAstNode & node,
                       Scope & scope,
                       bool & is_typedef,
                       TypePtr & out)
  {
    return parse_decl_spec_ast(node, make_decl_hooks(scope), is_typedef, out);
  }

  bool parse_parameter_clause(Scope & scope,
                              const CppAstNode & node,
                              vector<pair<string, TypePtr> > & params)
  {
    return parse_parameter_clause_ast(node, make_decl_hooks(scope), params);
  }

  bool parse_declarator(Scope & scope,
                        const CppAstNode & node,
                        const TypePtr & base,
                        string & name,
                        TypePtr & out)
  {
    return parse_declarator_ast(node, make_decl_hooks(scope), base, name, out);
  }

  bool parse_type_id(Scope & scope,
                     const CppAstNode & node,
                     TypePtr & out)
  {
    return parse_type_id_ast(node, make_decl_hooks(scope), out);
  }

  void add_binding(Scope & scope, BindingKind kind, const string & name, const TypePtr & type)
  {
    scope.bindings.push_back(Binding(kind, name, type));
  }

  void add_value_binding(Scope & scope,
                         BindingKind kind,
                         const string & name,
                         const TypePtr & type,
                         long long value)
  {
    scope.bindings.push_back(Binding(kind, name, type));
    scope.bindings.back().has_value = true;
    scope.bindings.back().value = value;
  }

  void analyze_simple_declaration(Scope & scope, const CppAstNode & node)
  {
    if(analyze_anonymous_union_declaration(scope, node)) {
      return;
    }
    const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
    const CppAstNode * declarators = find_child_kind(node, CppAstKind::init_declarator_list);
    if(!specifiers) {
      throw logic_error("simple-declaration missing decl-specifier-seq");
    }

    bool is_typedef = false;
    TypePtr base;
    if(!parse_decl_spec(*specifiers, scope, is_typedef, base)) {
      throw logic_error("unsupported decl-specifier-seq");
    }

    const bool is_constexpr_object = decl_spec_contains_token(*specifiers, KW_CONSTEXPR);
    const bool is_const_object =
        decl_spec_contains_token(*specifiers, KW_CONST) ||
        is_constexpr_object;

    if(!declarators) {
      return;
    }

    for(size_t i = 0; i < declarators->children.size(); ++i) {
      const CppAstNode & init_decl = declarators->children[i];
      if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
        throw logic_error("unsupported init-declarator");
      }

      string name;
      TypePtr type;
      if(!parse_declarator(scope, init_decl.children[0], base, name, type)) {
        throw logic_error("unsupported declarator");
      }

      if(type && strip_top_level_cv(type)->kind != Type::TK_FUNCTION && is_constexpr_object) {
        type = apply_cv(type, true, false);
      }

      if(is_typedef) {
        scope.named_types[name] = NamedTypeBinding(NTK_TYPE_ALIAS, type);
        maybe_add_type_scope_binding(scope, name, type);
        add_binding(scope, BK_TYPE_ALIAS, name, type);
      } else if(type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
        add_binding(scope, BK_FUNCTION, name, type);
      } else {
        add_binding(scope, BK_VARIABLE, name, type);
        if(is_const_object && init_decl.children.size() > 1 &&
           (is_integral_type(type) || strip_top_level_cv(type)->kind == Type::TK_NAMED)) {
          long long value = 0;
          if(evaluate_initializer_constant(scope, init_decl.children[1], value)) {
            scope.constant_values[name] = value;
          }
        }
      }
    }
  }

  void analyze_bit_field_declaration(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
    if(!specifiers) {
      throw logic_error("bit-field declaration missing decl-specifier-seq");
    }

    bool is_typedef = false;
    TypePtr base;
    if(!parse_decl_spec(*specifiers, scope, is_typedef, base) || is_typedef || !base) {
      throw logic_error("unsupported bit-field decl-specifier-seq");
    }

    for(size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & child = node.children[i];
      if(child.kind != CppAstKind::bit_field_declarator) {
        continue;
      }

      const CppAstNode * declarator = find_child_kind(child, CppAstKind::declarator);
      if(!declarator) {
        continue;
      }

      string name;
      TypePtr type;
      if(!parse_declarator(scope, *declarator, base, name, type)) {
        throw logic_error("unsupported bit-field declarator");
      }
      add_binding(scope, BK_VARIABLE, name, type);
    }
  }

  void analyze_alias_declaration(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * type_id = find_child_kind(node, CppAstKind::type_id);
    if(!type_id) {
      throw logic_error("alias-declaration missing type-id");
    }

    TypePtr type;
    if(!parse_type_id(scope, *type_id, type)) {
      throw logic_error("unsupported alias type-id");
    }

    scope.named_types[node.value] = NamedTypeBinding(NTK_TYPE_ALIAS, type);
    maybe_add_type_scope_binding(scope, node.value, type);
    add_binding(scope, BK_TYPE_ALIAS, node.value, type);
  }

  void analyze_enum_declaration(Scope & scope, const CppAstNode & node)
  {
    if(node.value.empty()) {
      throw logic_error("anonymous enums unsupported");
    }

    const CppAstNode * enum_key = find_child_kind(node, CppAstKind::enum_key);
    const bool scoped = enum_key != nullptr;
    size_t enumerator_count = 0;
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::enumerator) {
        ++enumerator_count;
      }
    }
    const bool has_enumerators = enumerator_count > 0;
    if(!scoped && !has_enumerators) {
      throw logic_error("opaque unscoped enums unsupported");
    }
    string enum_prefix = "enum";
    if(enum_key) {
      enum_prefix += " ";
      enum_prefix += node_text(*enum_key);
    }

    TypePtr enum_underlying_type = make_fundamental(FT_INT);
    size_t enum_alignment = 4;
    size_t enum_size = 4;
    if(const CppAstNode * underlying = find_child_kind(node, CppAstKind::type_id)) {
      if(!parse_type_id(scope, *underlying, enum_underlying_type)) {
        throw logic_error("unsupported enum underlying type");
      }
      enum_alignment = type_alignment(enum_underlying_type);
      enum_size = type_size(enum_underlying_type);
    }

    TypePtr enum_type = make_enum_type(scope, enum_prefix, node.value, true,
                                       enum_alignment, enum_size);
    enum_type->named_enum_underlying_type = enum_underlying_type;
    scope.named_types[node.value] = NamedTypeBinding(NTK_TYPE, enum_type);
    add_binding(scope, BK_TYPE, node.value, enum_type);

    Scope * enumerator_scope = &scope;
    if(scoped) {
      Scope & enum_scope = append_child_scope(scope, Scope::SK_ENUM, node.value);
      scope.type_scopes[node.value] = &enum_scope;
      named_type_scopes[enum_type->named_key] = &enum_scope;
      enumerator_scope = &enum_scope;
    }

    long long next_value = -1;
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & child = node.children[i];
      if(child.kind == CppAstKind::enum_key) {
        continue;
      }
      if(child.kind == CppAstKind::type_id) {
        continue;
      }
      if(child.kind != CppAstKind::enumerator) {
        throw logic_error("unsupported enum child");
      }

      long long value = next_value + 1;
      if(!child.children.empty()) {
        if(child.children.size() != 1 ||
           !evaluate_constant_expression(*enumerator_scope, child.children[0], value)) {
          throw logic_error("unsupported enumerator value");
        }
      }

      enumerator_scope->constant_values[child.value] = value;
      add_value_binding(*enumerator_scope, BK_ENUMERATOR, child.value, enum_type, value);
      next_value = value;
    }
  }

  void analyze_static_assert_declaration(Scope & scope, const CppAstNode & node)
  {
    if(node.children.empty()) {
      throw logic_error("static_assert missing condition");
    }

    long long value = 0;
    if(!evaluate_constant_expression(scope, node.children[0], value) || !value) {
      throw logic_error("static_assert failed");
    }
  }

  void analyze_compound_statement(Scope & parent, const CppAstNode & node, bool create_block_scope)
  {
    Scope * target = &parent;
    if(create_block_scope) {
      target = &append_child_scope(parent, Scope::SK_BLOCK, "");
    }

    for(size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & child = node.children[i];
      if(child.kind == CppAstKind::simple_declaration) {
        analyze_simple_declaration(*target, child);
      } else if(child.kind == CppAstKind::enum_specifier) {
        analyze_enum_declaration(*target, child);
      } else if(child.kind == CppAstKind::alias_declaration) {
        analyze_alias_declaration(*target, child);
      } else if(child.kind == CppAstKind::static_assert_declaration) {
        analyze_static_assert_declaration(*target, child);
      } else if(child.kind == CppAstKind::using_directive) {
        analyze_using_directive(*target, child);
      } else if(child.kind == CppAstKind::using_declaration) {
        analyze_using_declaration(*target, child);
      } else if(child.kind == CppAstKind::compound_statement) {
        analyze_compound_statement(*target, child, true);
      }
    }
  }

  void analyze_function_body(Scope & parent, const CppAstNode & node)
  {
    if(node.kind == CppAstKind::compound_statement) {
      analyze_compound_statement(parent, node, true);
      return;
    }
    if(node.kind != CppAstKind::try_block || node.children.empty()) {
      throw logic_error("unsupported function body");
    }

    analyze_compound_statement(parent, node.children[0], true);
    for(size_t i = 1; i < node.children.size(); ++i) {
      const CppAstNode & handler = node.children[i];
      if(handler.kind != CppAstKind::handler || handler.children.size() != 2) {
        throw logic_error("unsupported function try-block handler");
      }
      analyze_compound_statement(parent, handler.children[1], true);
    }
  }

  void analyze_function_definition(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
    const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
    const CppAstNode * body = find_function_body_node(node);
    if(!specifiers || !declarator || !body) {
      throw logic_error("function-definition missing children");
    }

    bool is_typedef = false;
    TypePtr base;
    if(!parse_decl_spec(*specifiers, scope, is_typedef, base) || is_typedef) {
      throw logic_error("unsupported function decl-specifier-seq");
    }

    string name;
    TypePtr type;
    if(!parse_declarator(scope, *declarator, base, name, type) ||
       !type || strip_top_level_cv(type)->kind != Type::TK_FUNCTION) {
      throw logic_error("unsupported function declarator");
    }

    add_binding(scope, BK_FUNCTION, name, type);

    Scope & function_scope = append_child_scope(scope, Scope::SK_FUNCTION, name);

    const TypePtr function_type = strip_top_level_cv(type);
    const CppAstNode * parameter_clause = find_child_kind(*declarator, CppAstKind::parameter_clause);
    vector<pair<string, TypePtr> > params;
    if(parameter_clause && !parse_parameter_clause(scope, *parameter_clause, params)) {
      throw logic_error("unsupported function parameter-clause");
    }
    for(size_t i = 0; i < params.size(); ++i) {
      function_scope.bindings.push_back(Binding(BK_PARAMETER, params[i].first,
                                                params[i].second));
    }

    (void)function_type;
    analyze_function_body(function_scope, *body);
  }

  void analyze_template_declaration(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * parameters = find_child_kind(node, CppAstKind::template_parameter_clause);
    if(!parameters || node.children.size() < 2) {
      throw logic_error("template-declaration missing children");
    }

    Scope & template_scope = append_child_scope(scope, Scope::SK_TEMPLATE_PARAMETERS, "");

    const CppAstNode * parameter_list = find_child_kind(*parameters, CppAstKind::template_parameter_list);
    if(parameter_list) {
      for(size_t i = 0; i < parameter_list->children.size(); ++i) {
        const CppAstNode & parameter = parameter_list->children[i];
        if(parameter.kind != CppAstKind::type_parameter) {
          continue;
        }

        const CppAstNode * identifier = find_child_kind(parameter, CppAstKind::identifier);
        if(!identifier) {
          continue;
        }

        string display = "typename " + identifier->value;
        if(find_child_kind(parameter, CppAstKind::template_template_parameter)) {
          display = "template-parameter " + identifier->value;
        }
        string key = "template-parameter " + scope_qualified_name(template_scope,
                                                                  identifier->value);
        TypePtr param_type = make_named(display, key, true);
        template_scope.named_types[identifier->value] = NamedTypeBinding(NTK_TYPE, param_type);
        add_binding(template_scope, BK_TYPE, identifier->value, param_type);
      }
    }

    analyze_declaration(template_scope, node.children.back());
  }

  bool analyze_anonymous_union_declaration(Scope & scope, const CppAstNode & node)
  {
    CppAstNode synthetic_decl;
    string type_name;
    string storage_name;
    if(!semantic_class_model::synthesize_anonymous_union_storage_declaration(node,
                                                                             synthetic_decl,
                                                                             type_name,
                                                                             storage_name)) {
      return false;
    }
    const CppAstNode * owned_synthetic_decl = own_synthetic_ast(std::move(synthetic_decl));

    const CppAstNode * synthetic_specifiers =
        find_child_kind(*owned_synthetic_decl, CppAstKind::decl_specifier_seq);
    if(!synthetic_specifiers) {
      throw logic_error("anonymous union synthesis missing specifiers");
    }

    const CppAstNode * named_union = nullptr;
    for(size_t i = 0; i < synthetic_specifiers->children.size(); ++i) {
      if(synthetic_specifiers->children[i].kind == CppAstKind::class_specifier) {
        named_union = &synthetic_specifiers->children[i];
        break;
      }
    }
    if(!named_union) {
      throw logic_error("anonymous union synthesis missing named union");
    }

    analyze_class_declaration(scope, *named_union);
    TypePtr union_type = lookup_type(scope, type_name);
    Scope * union_scope = scope_for_type(union_type);
    if(!union_scope) {
      throw logic_error("missing anonymous union scope");
    }
    inject_anonymous_union_member_bindings(scope, *union_scope);
    scope.named_types.erase(type_name);
    scope.type_scopes.erase(type_name);
    for(vector<Binding>::iterator it = scope.bindings.begin();
        it != scope.bindings.end();
        ++it) {
      if(it->kind == BK_TYPE && it->name == type_name) {
        scope.bindings.erase(it);
        break;
      }
    }
    return true;
  }

  void analyze_class_declaration(Scope & scope, const CppAstNode & node)
  {
    if(analyze_anonymous_union_declaration(scope, node)) {
      return;
    }
    if(node.value.empty()) {
      throw logic_error("anonymous classes unsupported");
    }

    const CppAstNode * class_key = find_child_kind(node, CppAstKind::class_key);
    if(!class_key || !class_key->has_token) {
      throw logic_error("class declaration missing class-key");
    }

    const string class_kind = node_text(*class_key);
    const bool complete = node.kind == CppAstKind::class_specifier;
    TypePtr class_type = make_class_type(scope, class_kind, node.value, complete);
    scope.named_types[node.value] = NamedTypeBinding(NTK_TYPE, class_type);
    add_binding(scope, BK_TYPE, node.value, class_type);

    if(!complete) {
      return;
    }

    Scope & class_scope = append_child_scope(scope, Scope::SK_CLASS, node.value);
    class_scope.named_types[node.value] = NamedTypeBinding(NTK_TYPE, class_type);
    scope.type_scopes[node.value] = &class_scope;
    named_type_scopes[class_type->named_key] = &class_scope;

    for(size_t i = 0; i < node.children.size(); ++i) {
      const CppAstNode & child = node.children[i];
      if(child.kind == CppAstKind::class_key || child.kind == CppAstKind::base_clause ||
         child.kind == CppAstKind::access_specifier) {
        continue;
      }
      analyze_declaration(class_scope, child);
    }
  }

  void analyze_namespace_alias_definition(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * target = find_child_kind(node, CppAstKind::target);
    if(!target) {
      throw logic_error("namespace-alias-definition missing target");
    }

    const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
    if(!target_name) {
      throw logic_error("namespace-alias-definition target missing structured name");
    }

    Scope * target_namespace = lookup_namespace_name(scope, *target_name);
    if(!target_namespace) {
      throw logic_error("unknown namespace alias target");
    }

    scope.namespace_bindings[node.value] = target_namespace;
  }

  void analyze_using_directive(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * target = find_child_kind(node, CppAstKind::target);
    if(!target) {
      throw logic_error("using-directive missing target");
    }

    const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
    if(!target_name) {
      throw logic_error("using-directive target missing structured name");
    }

    Scope * target_namespace = lookup_namespace_name(scope, *target_name);
    if(!target_namespace) {
      throw logic_error("unknown using-directive target");
    }

    if(find(scope.using_directives.begin(),
            scope.using_directives.end(),
            target_namespace) == scope.using_directives.end()) {
      scope.using_directives.push_back(target_namespace);
    }
  }

  void analyze_using_declaration(Scope & scope, const CppAstNode & node)
  {
    const CppAstNode * target = find_child_kind(node, CppAstKind::target);
    if(!target) {
      throw logic_error("using-declaration missing target");
    }

    const QualifiedName * qualified = cppast_qualified_name_syntax(*target);
    if(!qualified) {
      throw logic_error("using-declaration target missing structured name");
    }

    TypePtr type = lookup_type(scope, *qualified);
    if(type) {
      NamedTypeKind binding_kind = NTK_TYPE_ALIAS;
      if(qualified->rooted || !qualified->qualifiers.empty()) {
        const NamedTypeBinding * existing =
            lookup_qualified_generic<const NamedTypeBinding *>(
                scope, *qualified,
                [this](Scope & target, const string & lookup_name) -> const NamedTypeBinding *
                {
                  return lookup_named_type_binding_direct(target, lookup_name);
                });
        if(existing) {
          binding_kind = existing->kind;
        }
      }
      scope.named_types[qualified->name] = NamedTypeBinding(binding_kind, type);
      maybe_add_type_scope_binding(scope, qualified->name, type);
      add_binding(scope, binding_kind_for_named_type_kind(binding_kind), qualified->name, type);
      return;
    }

    Scope * target_namespace = lookup_namespace_name(scope, *qualified);
    if(target_namespace) {
      scope.namespace_bindings[qualified->name] = target_namespace;
      return;
    }

    const Binding *value_binding = lookup_qualified_value_binding(scope, *qualified);
    if(value_binding) {
      scope.bindings.push_back(Binding(value_binding->kind, qualified->name, value_binding->type));
      if(value_binding->has_value) {
        scope.bindings.back().has_value = true;
        scope.bindings.back().value = value_binding->value;
      }

      long long constant_value = 0;
      if(value_binding->has_value ||
         lookup_qualified_constant(scope, *qualified, constant_value)) {
        if(value_binding->has_value) {
          constant_value = value_binding->value;
        }
        scope.constant_values[qualified->name] = constant_value;
      }
      return;
    }

    throw logic_error("unknown using-declaration target");
  }

  void analyze_namespace_definition(Scope & scope, const CppAstNode & node)
  {
    Scope * target = nullptr;
    if(node.value == "<unnamed>") {
      target = &append_child_scope(scope, Scope::SK_NAMESPACE, node.value);
      scope.namespace_bindings["_GLOBAL__N_1"] = target;
    } else {
      target = find_named_namespace_child(scope, node.value);
      if(!target) {
        target = &append_child_scope(scope, Scope::SK_NAMESPACE, node.value);
      }
      scope.namespace_bindings[node.value] = target;
    }

    Scope & ns = *target;

    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::inline_node) {
        continue;
      }
      analyze_declaration(ns, node.children[i]);
    }
  }

  void analyze_declaration(Scope & scope, const CppAstNode & node)
  {
    if(node.kind == CppAstKind::empty_declaration) {
      return;
    }
    if(node.kind == CppAstKind::namespace_definition) {
      analyze_namespace_definition(scope, node);
      return;
    }
    if(node.kind == CppAstKind::class_specifier || node.kind == CppAstKind::class_forward_declaration) {
      analyze_class_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::template_declaration) {
      analyze_template_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::namespace_alias_definition) {
      analyze_namespace_alias_definition(scope, node);
      return;
    }
    if(node.kind == CppAstKind::using_directive) {
      analyze_using_directive(scope, node);
      return;
    }
    if(node.kind == CppAstKind::using_declaration) {
      analyze_using_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::enum_specifier) {
      analyze_enum_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::static_assert_declaration) {
      analyze_static_assert_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::simple_declaration) {
      analyze_simple_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::bit_field_declaration) {
      analyze_bit_field_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::alias_declaration) {
      analyze_alias_declaration(scope, node);
      return;
    }
    if(node.kind == CppAstKind::explicit_instantiation_declaration ||
       node.kind == CppAstKind::explicit_instantiation_definition) {
      return;
    }
    if(node.kind == CppAstKind::function_definition) {
      analyze_function_definition(scope, node);
      return;
    }
    throw logic_error(string("unsupported declaration kind ") + cppast_kind_text(node.kind));
  }
};

}  // namespace

string describe_types_translation_unit(IRecogTokenSequence & tokens)
{
  CppAstParser parser(tokens);
  CppAstNode ast;
  if(!parser.parse_translation_unit(ast)) {
    throw logic_error(parser.error());
  }

  Analyzer analyzer(ast);
  return analyzer.describe();
}

string describe_types_translation_unit(const vector<RecogToken> & tokens)
{
  CppAstParser parser(tokens);
  CppAstNode ast;
  if(!parser.parse_translation_unit(ast)) {
    throw logic_error(parser.error());
  }

  Analyzer analyzer(ast);
  return analyzer.describe();
}
