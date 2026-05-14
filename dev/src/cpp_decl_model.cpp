#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <stdexcept>

using namespace std;

#include "cppast_ast.h"
#include "cpp_decl_model.h"

namespace cpp_decl {

namespace {

string trim_trailing_space(const string & text)
{
  size_t end = text.size();
  while(end > 0 && isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(0, end);
}

string strip_elaborated_type_prefix(const string & text)
{
  static const char * prefixes[] = {
      "enum class ",
      "enum struct ",
      "class ",
      "struct ",
      "union ",
      "enum "};
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix = prefixes[i];
    if(text.compare(0, prefix.size(), prefix) == 0) {
      return text.substr(prefix.size());
    }
  }
  return text;
}

struct LeadingCvNormalization
{
  string normalized_text;
  bool cv_const = false;
  bool cv_volatile = false;
};

LeadingCvNormalization normalize_leading_cv_spelling(const string & text)
{
  static const char * semantic_prefixes[] = {
      "dependent type ",
      "dependent alias ",
      "dependent decltype ",
      "dependent typeof "
  };

  LeadingCvNormalization result;
  string prefix;
  string remaining = text;
  for(size_t i = 0; i < sizeof(semantic_prefixes) / sizeof(semantic_prefixes[0]); ++i) {
    const string semantic_prefix = semantic_prefixes[i];
    if(remaining.compare(0, semantic_prefix.size(), semantic_prefix) == 0) {
      prefix = semantic_prefix;
      remaining = remaining.substr(semantic_prefix.size());
      break;
    }
  }

  bool changed = false;
  while(true) {
    while(!remaining.empty() && isspace(static_cast<unsigned char>(remaining[0]))) {
      remaining.erase(remaining.begin());
      changed = true;
    }
    if(remaining.compare(0, 6, "const ") == 0) {
      result.cv_const = true;
      remaining.erase(0, 6);
      changed = true;
      continue;
    }
    if(remaining.compare(0, 9, "volatile ") == 0) {
      result.cv_volatile = true;
      remaining.erase(0, 9);
      changed = true;
      continue;
    }
    break;
  }

  result.normalized_text = changed ? prefix + remaining : text;
  return result;
}

struct NamedSemanticClassification
{
  Type::NamedSemanticKind kind = Type::NSK_ORDINARY;
  string payload;
};

NamedSemanticClassification classify_named_semantic_key(const string & key)
{
  static const struct
  {
    const char * prefix;
    Type::NamedSemanticKind kind;
  } prefixes[] = {
      {"template-parameter ", Type::NSK_TEMPLATE_PARAMETER},
      {"partial-order ", Type::NSK_PARTIAL_ORDER},
      {"dependent type ", Type::NSK_DEPENDENT_TYPE},
      {"dependent alias ", Type::NSK_DEPENDENT_ALIAS},
      {"dependent decltype ", Type::NSK_DEPENDENT_DECLTYPE},
      {"dependent typeof ", Type::NSK_DEPENDENT_TYPEOF},
  };

  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const char * prefix = prefixes[i].prefix;
    const size_t prefix_size = strlen(prefix);
    if(key.compare(0, prefix_size, prefix) == 0) {
      NamedSemanticClassification out;
      out.kind = prefixes[i].kind;
      out.payload = key.substr(prefix_size);
      return out;
    }
  }

  NamedSemanticClassification out;
  return out;
}

const char * semantic_prefix_for_kind(Type::NamedSemanticKind kind)
{
  switch(kind) {
  case Type::NSK_TEMPLATE_PARAMETER:
    return "template-parameter ";
  case Type::NSK_PARTIAL_ORDER:
    return "partial-order ";
  case Type::NSK_DEPENDENT_TYPE:
    return "dependent type ";
  case Type::NSK_DEPENDENT_ALIAS:
    return "dependent alias ";
  case Type::NSK_DEPENDENT_DECLTYPE:
    return "dependent decltype ";
  case Type::NSK_DEPENDENT_TYPEOF:
    return "dependent typeof ";
  case Type::NSK_ORDINARY:
    return "";
  }
  return "";
}

struct TypeSpelling
{
  string before;
  string after;
};

struct TopLevelCv
{
  TypePtr base;
  bool cv_const = false;
  bool cv_volatile = false;
};

TypeSpelling spell_template_argument_type(const TypePtr & type);

TopLevelCv collect_top_level_cv(const TypePtr & type)
{
  TopLevelCv out;
  out.base = type;
  while(out.base && out.base->kind == Type::TK_CV) {
    out.cv_const = out.cv_const || out.base->cv_const;
    out.cv_volatile = out.cv_volatile || out.base->cv_volatile;
    out.base = out.base->inner;
  }
  return out;
}

string cv_qualifier_suffix(bool cv_const, bool cv_volatile)
{
  string out;
  if(cv_const) {
    out += "const";
  }
  if(cv_volatile) {
    if(!out.empty()) {
      out += " ";
    }
    out += "volatile";
  }
  return out;
}

string join_function_params(const vector<TypePtr> & params,
                           bool variadic,
                           bool prototype_relaxed)
{
  string out;
  for(size_t i = 0; i < params.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += template_argument_type_text(params[i]);
  }
  if(variadic) {
    if(!params.empty()) {
      out += ", ";
    }
    out += "...";
  } else if(prototype_relaxed) {
    if(!params.empty()) {
      out += ", ";
    }
    out += "<prototype_relaxed>";
  }
  return out;
}

TypeSpelling spell_template_argument_type(const TypePtr & type)
{
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return TypeSpelling{type_to_string(type->fundamental) + " ", ""};

  case Type::TK_NAMED:
    return TypeSpelling{strip_elaborated_type_prefix(type->named_display) + " ", ""};

  case Type::TK_CV:
  {
    TopLevelCv cv = collect_top_level_cv(type);
    TypeSpelling inner = spell_template_argument_type(cv.base);
    string qualifier_text = cv_qualifier_suffix(cv.cv_const, cv.cv_volatile);
    if(!inner.after.empty()) {
      inner.before = trim_trailing_space(inner.before) + qualifier_text;
    } else {
      inner.before = trim_trailing_space(inner.before) + " " + qualifier_text + " ";
    }
    return inner;
  }

  case Type::TK_ATOMIC:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    return TypeSpelling{"_Atomic(" +
                            trim_trailing_space(inner.before) +
                            inner.after + ") ",
                        ""};
  }

  case Type::TK_POINTER:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    if(!inner.after.empty()) {
      return TypeSpelling{inner.before + "(*", ")" + inner.after};
    }
    return TypeSpelling{trim_trailing_space(inner.before) + " * ", ""};
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    const string owner_text = template_argument_type_text(type->owner);
    if(!inner.after.empty()) {
      return TypeSpelling{inner.before + "(" + owner_text + "::*",
                          ")" + inner.after};
    }
    return TypeSpelling{trim_trailing_space(inner.before) + " " + owner_text + "::* ", ""};
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    if(!inner.after.empty()) {
      return TypeSpelling{inner.before + "(^", ")" + inner.after};
    }
    return TypeSpelling{trim_trailing_space(inner.before) + " ^ ", ""};
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    if(!inner.after.empty()) {
      return TypeSpelling{inner.before + "(&", ")" + inner.after};
    }
    return TypeSpelling{trim_trailing_space(inner.before) + " & ", ""};
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    if(!inner.after.empty()) {
      return TypeSpelling{inner.before + "(&&", ")" + inner.after};
    }
    return TypeSpelling{trim_trailing_space(inner.before) + " && ", ""};
  }

  case Type::TK_ARRAY:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    ostringstream out;
    if(type->has_bound) {
      out << "[" << type->bound << "]";
    } else if(!type->bound_text.empty()) {
      out << "[" << type->bound_text << "]";
    } else {
      out << "[]";
    }
    inner.after += out.str();
    return inner;
  }

  case Type::TK_FUNCTION:
  {
    TypeSpelling inner = spell_template_argument_type(type->inner);
    inner.after += "(" +
                   join_function_params(type->params,
                                        type->variadic,
                                        type->prototype_relaxed) +
                   ")";
    if(type->function_const) {
      inner.after += " const";
    }
    if(type->function_volatile) {
      inner.after += " volatile";
    }
    return inner;
  }
  }

  throw logic_error("unknown type kind in template_argument_type_text");
}

}  // namespace

TypePtr make_fundamental(EFundamentalType type)
{
  TypePtr result(new Type(Type::TK_FUNDAMENTAL));
  result->fundamental = type;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_named(const string & display_name,
                   const string & unique_key,
                   bool complete,
                   bool has_layout,
                   size_t alignment,
                   size_t size)
{
  LeadingCvNormalization display = normalize_leading_cv_spelling(display_name);
  LeadingCvNormalization key = normalize_leading_cv_spelling(unique_key);

  TypePtr result(new Type(Type::TK_NAMED));
  result->named_display = display.normalized_text;
  result->named_key = key.normalized_text;
  const NamedSemanticClassification semantic =
      classify_named_semantic_key(result->named_key);
  result->named_semantic_kind = semantic.kind;
  result->named_semantic_payload = semantic.payload;
  result->named_complete = complete;
  result->named_has_layout = has_layout;
  result->named_alignment = alignment;
  result->named_size = size;
  result->named_is_empty = false;
  return make_cv(result,
                 display.cv_const || key.cv_const,
                 display.cv_volatile || key.cv_volatile);
}

TypePtr make_semantic_named(const string & display_name,
                            Type::NamedSemanticKind semantic_kind,
                            const string & semantic_payload,
                            bool complete,
                            bool has_layout,
                            size_t alignment,
                            size_t size)
{
  const string unique_key =
      string(semantic_prefix_for_kind(semantic_kind)) + semantic_payload;
  TypePtr result =
      make_named(display_name, unique_key, complete, has_layout, alignment, size);
  TypePtr base = strip_top_level_cv(result);
  if(base && base->kind == Type::TK_NAMED) {
    base->named_semantic_kind = semantic_kind;
    base->named_semantic_payload =
        normalize_leading_cv_spelling(semantic_payload).normalized_text;
  }
  return result;
}

TypePtr make_dependent_type_expression_type(const string & display_name,
                                            Type::NamedSemanticKind semantic_kind,
                                            const string & semantic_payload,
                                            const CppAstNode & expression_node)
{
  TypePtr result =
      make_semantic_named(display_name, semantic_kind, semantic_payload, true);
  TypePtr base = strip_top_level_cv(result);
  if(base && base->kind == Type::TK_NAMED &&
     (semantic_kind == Type::NSK_DEPENDENT_DECLTYPE ||
      semantic_kind == Type::NSK_DEPENDENT_TYPEOF)) {
    base->named_dependent_type_expression_node.reset(new CppAstNode(expression_node));
  }
  return result;
}

TypePtr make_dependent_alias_type(
    const string & display_name,
    const string & semantic_payload,
    void * alias_template_decl,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  TypePtr result = make_semantic_named(display_name,
                                       Type::NSK_DEPENDENT_ALIAS,
                                       semantic_payload,
                                       true);
  TypePtr base = strip_top_level_cv(result);
  if(base && base->kind == Type::TK_NAMED) {
    base->named_dependent_alias_template_decl = alias_template_decl;
    base->named_dependent_alias_arguments = arguments;
  }
  return result;
}

namespace {

TypePtr named_base(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_NAMED ? base : TypePtr();
}

bool named_type_has_kind(const TypePtr & type, Type::NamedSemanticKind kind)
{
  TypePtr base = named_base(type);
  return base && base->named_semantic_kind == kind;
}

}  // namespace

TypePtr make_dependent_qualified_member_type(
    const string & display_name,
    const TypePtr & owner,
    const vector<string> & members,
    bool leading_typename,
    const vector<TemplateIdSyntax> & member_template_ids)
{
  ostringstream key;
  key << "$dqmember:" << template_argument_type_text(owner);
  for(size_t i = 0; i < members.size(); ++i) {
    key << "::" << members[i];
  }
  TypePtr result = make_named(display_name, key.str(), true);
  TypePtr base = named_base(result);
  if(base && base->kind == Type::TK_NAMED) {
    base->named_semantic_kind = Type::NSK_DEPENDENT_TYPE;
    base->named_semantic_payload = key.str();
    base->named_dependent_qualified_owner = owner;
    base->named_dependent_qualified_members = members;
    base->named_dependent_qualified_member_template_ids = member_template_ids;
    base->named_dependent_qualified_leading_typename = leading_typename;
  }
  return result;
}

bool named_type_is_template_parameter(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_TEMPLATE_PARAMETER);
}

bool named_type_is_partial_order_placeholder(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_PARTIAL_ORDER);
}

bool named_type_is_dependent_alias(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_DEPENDENT_ALIAS);
}

bool named_type_is_dependent_type(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_DEPENDENT_TYPE);
}

bool named_type_is_dependent_decltype(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_DEPENDENT_DECLTYPE);
}

bool named_type_is_dependent_typeof(const TypePtr & type)
{
  return named_type_has_kind(type, Type::NSK_DEPENDENT_TYPEOF);
}

bool named_type_has_dependent_semantic(const TypePtr & type)
{
  TypePtr base = named_base(type);
  if(!base) {
    return false;
  }
  switch(base->named_semantic_kind) {
  case Type::NSK_TEMPLATE_PARAMETER:
  case Type::NSK_PARTIAL_ORDER:
  case Type::NSK_DEPENDENT_TYPE:
  case Type::NSK_DEPENDENT_ALIAS:
  case Type::NSK_DEPENDENT_DECLTYPE:
  case Type::NSK_DEPENDENT_TYPEOF:
    return true;
  case Type::NSK_ORDINARY:
    return false;
  }
  return false;
}

bool named_type_key_contains_dependent_semantic(const TypePtr & type)
{
  TypePtr base = named_base(type);
  if(!base || base->named_key.find("builtin ") != 0) {
    return false;
  }
  return base->named_key.find("template-parameter ") != string::npos ||
         base->named_key.find("partial-order ") != string::npos ||
         base->named_key.find("dependent alias ") != string::npos ||
         base->named_key.find("dependent type ") != string::npos ||
         base->named_key.find("dependent decltype ") != string::npos ||
         base->named_key.find("dependent typeof ") != string::npos;
}

bool named_type_key_contains_partial_order_placeholder(const TypePtr & type)
{
  TypePtr base = named_base(type);
  return base &&
         base->named_key.find("builtin ") == 0 &&
         base->named_key.find("partial-order ") != string::npos;
}

std::string named_type_semantic_payload(const TypePtr & type)
{
  TypePtr base = named_base(type);
  return base ? base->named_semantic_payload : string();
}

const CppAstNode * named_type_dependent_type_expression_node(const TypePtr & type)
{
  TypePtr base = named_base(type);
  if(!base ||
     (base->named_semantic_kind != Type::NSK_DEPENDENT_DECLTYPE &&
      base->named_semantic_kind != Type::NSK_DEPENDENT_TYPEOF)) {
    return nullptr;
  }
  return base->named_dependent_type_expression_node.get();
}

bool named_type_dependent_alias_template(
    const TypePtr & type,
    void *& alias_template_decl,
    vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  TypePtr base = named_base(type);
  if(!base ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_ALIAS ||
     !base->named_dependent_alias_template_decl) {
    alias_template_decl = nullptr;
    arguments.clear();
    return false;
  }
  alias_template_decl = base->named_dependent_alias_template_decl;
  arguments = base->named_dependent_alias_arguments;
  return true;
}

void set_named_type_dependent_class_template(
    const TypePtr & type,
    void * class_template_decl,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  TypePtr base = named_base(type);
  if(!base) {
    return;
  }
  base->named_dependent_class_template_decl = class_template_decl;
  base->named_dependent_class_arguments =
      class_template_decl ? arguments :
                            vector<DependentAliasTemplateArgumentSyntax>();
}

bool named_type_dependent_class_template(
    const TypePtr & type,
    void *& class_template_decl,
    vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  TypePtr base = named_base(type);
  if(!base || !base->named_dependent_class_template_decl) {
    class_template_decl = nullptr;
    arguments.clear();
    return false;
  }
  class_template_decl = base->named_dependent_class_template_decl;
  arguments = base->named_dependent_class_arguments;
  return true;
}

bool named_type_dependent_qualified_member(
    const TypePtr & type,
    TypePtr & owner,
    vector<string> & members,
    bool & leading_typename,
    vector<TemplateIdSyntax> * member_template_ids)
{
  owner.reset();
  members.clear();
  leading_typename = false;
  if(member_template_ids) {
    member_template_ids->clear();
  }
  TypePtr base = named_base(type);
  if(!base ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_TYPE ||
     !base->named_dependent_qualified_owner ||
     base->named_dependent_qualified_members.empty()) {
    return false;
  }
  owner = base->named_dependent_qualified_owner;
  members = base->named_dependent_qualified_members;
  if(member_template_ids) {
    *member_template_ids = base->named_dependent_qualified_member_template_ids;
  }
  leading_typename = base->named_dependent_qualified_leading_typename;
  return true;
}

bool type_is_definitely_not_class(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->definitely_not_class;
}

TypePtr make_cv(const TypePtr & base, bool cv_const, bool cv_volatile)
{
  if(!cv_const && !cv_volatile) {
    return base;
  }

  if(base->kind == Type::TK_CV) {
    TypePtr result(new Type(Type::TK_CV));
    result->cv_const = base->cv_const || cv_const;
    result->cv_volatile = base->cv_volatile || cv_volatile;
    result->inner = base->inner;
    result->definitely_not_class = base->definitely_not_class;
    return result;
  }

  TypePtr result(new Type(Type::TK_CV));
  result->cv_const = cv_const;
  result->cv_volatile = cv_volatile;
  result->inner = base;
  result->definitely_not_class = base->definitely_not_class;
  return result;
}

TypePtr make_atomic(const TypePtr & base)
{
  TypePtr result(new Type(Type::TK_ATOMIC));
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_pointer(const TypePtr & base)
{
  TypePtr result(new Type(Type::TK_POINTER));
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_member_pointer(const TypePtr & owner, const TypePtr & base)
{
  TypePtr result(new Type(Type::TK_MEMBER_POINTER));
  result->owner = owner;
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_block_pointer(const TypePtr & base)
{
  TypePtr result(new Type(Type::TK_BLOCK_POINTER));
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_array(const TypePtr & element,
                   bool has_bound,
                   size_t bound,
                   const string & bound_text)
{
  TypePtr result(new Type(Type::TK_ARRAY));
  result->inner = element;
  result->has_bound = has_bound;
  result->bound = bound;
  result->bound_text = bound_text;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_function(const TypePtr & result_type,
                      const vector<TypePtr> & params,
                      bool variadic,
                      bool function_const,
                      bool function_volatile,
                      bool prototype_relaxed)
{
  TypePtr result(new Type(Type::TK_FUNCTION));
  result->inner = result_type;
  result->params = params;
  result->variadic = variadic;
  result->prototype_relaxed = prototype_relaxed;
  result->function_const = function_const;
  result->function_volatile = function_volatile;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_lvalue_reference_raw(const TypePtr & base)
{
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    TypePtr result(new Type(Type::TK_LVALUE_REFERENCE));
    result->inner = base->inner;
    result->definitely_not_class = true;
    return result;
  }
  TypePtr result(new Type(Type::TK_LVALUE_REFERENCE));
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr make_rvalue_reference_raw(const TypePtr & base)
{
  if(base->kind == Type::TK_LVALUE_REFERENCE) {
    TypePtr result(new Type(Type::TK_LVALUE_REFERENCE));
    result->inner = base->inner;
    result->definitely_not_class = true;
    return result;
  }
  if(base->kind == Type::TK_RVALUE_REFERENCE) {
    TypePtr result(new Type(Type::TK_RVALUE_REFERENCE));
    result->inner = base->inner;
    result->definitely_not_class = true;
    return result;
  }
  TypePtr result(new Type(Type::TK_RVALUE_REFERENCE));
  result->inner = base;
  result->definitely_not_class = true;
  return result;
}

TypePtr apply_cv(const TypePtr & base, bool cv_const, bool cv_volatile)
{
  if(!cv_const && !cv_volatile) {
    return base;
  }

  if(base->kind == Type::TK_ARRAY) {
    return make_array(apply_cv(base->inner, cv_const, cv_volatile),
                      base->has_bound, base->bound, base->bound_text);
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return base;
  }

  return make_cv(base, cv_const, cv_volatile);
}

TypePtr strip_top_level_cv(const TypePtr & type)
{
  if(!type) {
    return type;
  }
  if(type->kind == Type::TK_CV) {
    return type->inner;
  }
  return type;
}

TypePtr normalize_parameter_type(const TypePtr & type)
{
  if(type->kind == Type::TK_LVALUE_REFERENCE ||
     type->kind == Type::TK_RVALUE_REFERENCE) {
    return type;
  }

  if(type->kind == Type::TK_ARRAY) {
    return make_pointer(type->inner);
  }

  if(type->kind == Type::TK_FUNCTION) {
    return make_pointer(type);
  }

  return strip_top_level_cv(type);
}

bool type_equals(const TypePtr & lhs, const TypePtr & rhs)
{
  if(lhs.get() == rhs.get()) {
    return true;
  }

  if(!lhs || !rhs || lhs->kind != rhs->kind) {
    return false;
  }

  switch(lhs->kind) {
  case Type::TK_FUNDAMENTAL:
    return lhs->fundamental == rhs->fundamental;

  case Type::TK_NAMED:
    return lhs->named_key == rhs->named_key &&
           lhs->named_complete == rhs->named_complete;

  case Type::TK_CV:
    return lhs->cv_const == rhs->cv_const &&
           lhs->cv_volatile == rhs->cv_volatile &&
           type_equals(lhs->inner, rhs->inner);

  case Type::TK_ATOMIC:
    return type_equals(lhs->inner, rhs->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return type_equals(lhs->inner, rhs->inner);

  case Type::TK_MEMBER_POINTER:
    return type_equals(lhs->owner, rhs->owner) &&
           type_equals(lhs->inner, rhs->inner);

  case Type::TK_ARRAY:
    return lhs->has_bound == rhs->has_bound &&
           lhs->bound == rhs->bound &&
           lhs->bound_text == rhs->bound_text &&
           type_equals(lhs->inner, rhs->inner);

  case Type::TK_FUNCTION:
    if(lhs->variadic != rhs->variadic ||
       lhs->prototype_relaxed != rhs->prototype_relaxed ||
       lhs->function_const != rhs->function_const ||
       lhs->function_volatile != rhs->function_volatile ||
       lhs->params.size() != rhs->params.size() ||
       !type_equals(lhs->inner, rhs->inner)) {
      return false;
    }
    for(size_t i = 0; i < lhs->params.size(); ++i) {
      if(!type_equals(lhs->params[i], rhs->params[i])) {
        return false;
      }
    }
    return true;
  }

  return false;
}

TypePtr merge_types(const TypePtr & lhs, const TypePtr & rhs)
{
  if(type_equals(lhs, rhs)) {
    return lhs;
  }

  if(lhs->kind != Type::TK_ARRAY || rhs->kind != Type::TK_ARRAY) {
    return TypePtr();
  }

  TypePtr merged_inner = merge_types(lhs->inner, rhs->inner);
  if(!merged_inner) {
    return TypePtr();
  }

  if(lhs->has_bound && rhs->has_bound) {
    if(lhs->bound != rhs->bound) {
      return TypePtr();
    }
    return make_array(merged_inner, true, lhs->bound, lhs->bound_text);
  }

  if(lhs->has_bound) {
    return make_array(merged_inner, true, lhs->bound, lhs->bound_text);
  }

  if(rhs->has_bound) {
    return make_array(merged_inner, true, rhs->bound, rhs->bound_text);
  }

  return make_array(merged_inner, false, 0, lhs->bound_text.empty() ? rhs->bound_text :
                                                           lhs->bound_text);
}

TypePtr remove_reference_type(const TypePtr & type)
{
  if(type->kind == Type::TK_LVALUE_REFERENCE ||
     type->kind == Type::TK_RVALUE_REFERENCE) {
    return type->inner;
  }
  return type;
}

bool is_integral_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }
  switch(base->fundamental) {
  case FT_SIGNED_CHAR:
  case FT_SHORT_INT:
  case FT_INT:
  case FT_LONG_INT:
  case FT_LONG_LONG_INT:
  case FT_INT128:
  case FT_UNSIGNED_CHAR:
  case FT_UNSIGNED_SHORT_INT:
  case FT_UNSIGNED_INT:
  case FT_UNSIGNED_LONG_INT:
  case FT_UNSIGNED_LONG_LONG_INT:
  case FT_UINT128:
  case FT_WCHAR_T:
  case FT_CHAR:
  case FT_CHAR16_T:
  case FT_CHAR32_T:
  case FT_BOOL:
    return true;
  default:
    return false;
  }
}

bool is_floating_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(base->kind == Type::TK_NAMED) {
    return base->named_key == "builtin _Float16" ||
           base->named_key == "builtin _Float32" ||
           base->named_key == "builtin _Float32x" ||
           base->named_key == "builtin _Float64" ||
           base->named_key == "builtin _Float64x" ||
           base->named_key == "builtin _Float128" ||
           base->named_key == "builtin __float128" ||
           base->named_key == "builtin __ibm128";
  }
  if(base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }
  return base->fundamental == FT_FLOAT ||
         base->fundamental == FT_DOUBLE ||
         base->fundamental == FT_LONG_DOUBLE;
}

bool is_function_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_FUNCTION;
}

bool is_block_pointer_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_BLOCK_POINTER;
}

bool is_pointer_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_POINTER;
}

bool is_reference_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base->kind == Type::TK_LVALUE_REFERENCE ||
         base->kind == Type::TK_RVALUE_REFERENCE;
}

bool is_array_type(const TypePtr & type)
{
  return strip_top_level_cv(type)->kind == Type::TK_ARRAY;
}

bool is_bool_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_BOOL;
}

bool is_unsigned_integral_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }
  switch(base->fundamental) {
  case FT_UNSIGNED_CHAR:
  case FT_UNSIGNED_SHORT_INT:
  case FT_UNSIGNED_INT:
  case FT_UNSIGNED_LONG_INT:
  case FT_UNSIGNED_LONG_LONG_INT:
  case FT_UINT128:
  case FT_BOOL:
  case FT_CHAR16_T:
  case FT_CHAR32_T:
  case FT_WCHAR_T:
    return true;

  default:
    return false;
  }
}

bool resolve_callable_function_type(const TypePtr & type, TypePtr & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_FUNCTION) {
    out = base;
    return true;
  }
  if((base->kind == Type::TK_POINTER || base->kind == Type::TK_BLOCK_POINTER) &&
     is_function_type(base->inner)) {
    out = strip_top_level_cv(base->inner);
    return true;
  }
  if((base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE) &&
     is_function_type(base->inner)) {
    out = strip_top_level_cv(base->inner);
    return true;
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    TypePtr referred = strip_top_level_cv(base->inner);
    if(referred &&
       (referred->kind == Type::TK_POINTER ||
        referred->kind == Type::TK_BLOCK_POINTER) &&
       is_function_type(referred->inner)) {
      out = strip_top_level_cv(referred->inner);
      return true;
    }
  }
  return false;
}

bool is_void_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_VOID;
}

bool type_is_const_object(const TypePtr & type)
{
  TypePtr base = type;
  if(base->kind == Type::TK_ARRAY) {
    return type_is_const_object(base->inner);
  }
  if(base->kind == Type::TK_CV) {
    return base->cv_const;
  }
  return false;
}

bool type_is_complete(const TypePtr & type)
{
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return true;

  case Type::TK_NAMED:
    return type->named_complete;

  case Type::TK_CV:
  case Type::TK_ATOMIC:
    return type_is_complete(type->inner);

  case Type::TK_ARRAY:
    return type->has_bound && type_is_complete(type->inner);

  case Type::TK_FUNCTION:
    return true;
  }

  return false;
}

size_t type_alignment(const TypePtr & type)
{
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return max<size_t>(1, type_to_size(type->fundamental));

  case Type::TK_NAMED:
    if(type->named_has_layout) {
      return type->named_alignment;
    }
    throw logic_error("named type alignment unavailable");

  case Type::TK_CV:
  case Type::TK_ATOMIC:
    return type_alignment(type->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return 8;

  case Type::TK_MEMBER_POINTER:
    return 8;

  case Type::TK_ARRAY:
    return type_alignment(type->inner);

  case Type::TK_FUNCTION:
    return 1;
  }

  throw logic_error("unknown type alignment");
}

size_t type_size(const TypePtr & type)
{
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return type_to_size(type->fundamental);

  case Type::TK_NAMED:
    if(type->named_has_layout) {
      return type->named_size;
    }
    throw logic_error(string("named type size unavailable: ") + type->named_display +
                      " [key " + type->named_key + "]");

  case Type::TK_CV:
  case Type::TK_ATOMIC:
    return type_size(type->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return 8;

  case Type::TK_MEMBER_POINTER:
    return is_function_type(type->inner) ? 16 : 8;

  case Type::TK_ARRAY:
    if(!type->has_bound) {
      throw logic_error("incomplete array type");
    }
    return type->bound * type_size(type->inner);

  case Type::TK_FUNCTION:
    return 0;
  }

  throw logic_error("unknown type size");
}

string describe_type(const TypePtr & type)
{
  if(!type) {
    return "<null-type>";
  }
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return type_to_string(type->fundamental);

  case Type::TK_NAMED:
    return type->named_display;

  case Type::TK_CV:
    if(type->cv_const && type->cv_volatile) {
      return string("const volatile ") + describe_type(type->inner);
    }
    if(type->cv_const) {
      return string("const ") + describe_type(type->inner);
    }
    return string("volatile ") + describe_type(type->inner);

  case Type::TK_ATOMIC:
    return string("_Atomic(") + template_argument_type_text(type->inner) + ")";

  case Type::TK_POINTER:
    return string("pointer to ") + describe_type(type->inner);

  case Type::TK_MEMBER_POINTER:
    return string("member-pointer of ") + describe_type(type->owner) +
           " to " + describe_type(type->inner);

  case Type::TK_BLOCK_POINTER:
    return string("block-pointer to ") + describe_type(type->inner);

  case Type::TK_LVALUE_REFERENCE:
    return string("lvalue-reference to ") + describe_type(type->inner);

  case Type::TK_RVALUE_REFERENCE:
    return string("rvalue-reference to ") + describe_type(type->inner);

  case Type::TK_ARRAY:
  {
    ostringstream out;
    if(type->has_bound) {
      out << "array of " << type->bound << " " << describe_type(type->inner);
    } else {
      out << "array of unknown bound of " << describe_type(type->inner);
    }
    return out.str();
  }

  case Type::TK_FUNCTION:
  {
    string result = "function of (";
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(i != 0) {
        result += ", ";
      }
      result += describe_type(type->params[i]);
    }
    if(type->variadic) {
      if(!type->params.empty()) {
        result += ", ";
      }
      result += "...";
    } else if(type->prototype_relaxed) {
      if(!type->params.empty()) {
        result += ", ";
      }
      result += "<prototype_relaxed>";
    }
    result += ")";
    if(type->function_const) {
      result += " const";
    }
    if(type->function_volatile) {
      result += " volatile";
    }
    result += " returning ";
    result += describe_type(type->inner);
    return result;
  }
  default:
    throw logic_error("unknown type");
  }
}

string template_argument_type_text(const TypePtr & type)
{
  TypeSpelling spelling = spell_template_argument_type(type);
  return trim_trailing_space(spelling.before) + spelling.after;
}

bool finalize_fundamental_type_specifiers(int signed_count,
                                          int unsigned_count,
                                          int short_count,
                                          int long_count,
                                          bool saw_int,
                                          bool saw_char,
                                          bool saw_char16,
                                          bool saw_char32,
                                          bool saw_wchar,
                                          bool saw_bool,
                                          bool saw_float,
                                          bool saw_double,
                                          bool saw_void,
                                          TypePtr & out)
{
  if(saw_char) {
    if(unsigned_count && !signed_count) {
      out = make_fundamental(FT_UNSIGNED_CHAR);
    } else if(signed_count && !unsigned_count) {
      out = make_fundamental(FT_SIGNED_CHAR);
    } else if(!signed_count && !unsigned_count) {
      out = make_fundamental(FT_CHAR);
    } else {
      return false;
    }
    return true;
  }

  if(saw_char16) {
    out = make_fundamental(FT_CHAR16_T);
    return true;
  }

  if(saw_char32) {
    out = make_fundamental(FT_CHAR32_T);
    return true;
  }

  if(saw_wchar) {
    out = make_fundamental(FT_WCHAR_T);
    return true;
  }

  if(saw_bool) {
    if(signed_count || unsigned_count || short_count || long_count || saw_int) {
      return false;
    }
    out = make_fundamental(FT_BOOL);
    return true;
  }

  if(saw_float) {
    if(signed_count || unsigned_count || short_count || long_count || saw_int) {
      return false;
    }
    out = make_fundamental(FT_FLOAT);
    return true;
  }

  if(saw_double) {
    if(short_count || signed_count || unsigned_count || saw_int || long_count > 1) {
      return false;
    }
    out = make_fundamental(long_count > 0 ? FT_LONG_DOUBLE : FT_DOUBLE);
    return true;
  }

  if(saw_void) {
    if(signed_count || unsigned_count || short_count || long_count || saw_int) {
      return false;
    }
    out = make_fundamental(FT_VOID);
    return true;
  }

  if(short_count > 1 || long_count > 2 || signed_count > 1 || unsigned_count > 1) {
    return false;
  }
  if(signed_count && unsigned_count) {
    return false;
  }

  const bool saw_integer_family =
      saw_int || signed_count > 0 || unsigned_count > 0 ||
      short_count > 0 || long_count > 0;
  if(!saw_integer_family) {
    return false;
  }

  const bool is_unsigned = unsigned_count > 0;
  if(short_count > 0) {
    out = make_fundamental(is_unsigned ? FT_UNSIGNED_SHORT_INT : FT_SHORT_INT);
  } else if(long_count >= 2) {
    out = make_fundamental(is_unsigned ? FT_UNSIGNED_LONG_LONG_INT :
                                        FT_LONG_LONG_INT);
  } else if(long_count == 1) {
    out = make_fundamental(is_unsigned ? FT_UNSIGNED_LONG_INT :
                                        FT_LONG_INT);
  } else {
    out = make_fundamental(is_unsigned ? FT_UNSIGNED_INT : FT_INT);
  }

  return true;
}

bool TypeSpecifierAccumulator::has_type_content() const
{
  return saw_named_type || signed_count || unsigned_count || short_count ||
         long_count || saw_int || saw_char || saw_char16 || saw_char32 ||
         saw_wchar || saw_bool || saw_float || saw_double || saw_void;
}

bool TypeSpecifierAccumulator::add_cv(ETokenType type)
{
  if(type == KW_CONST) {
    cv_const = true;
    return true;
  }
  if(type == KW_VOLATILE) {
    cv_volatile = true;
    return true;
  }
  return false;
}

bool TypeSpecifierAccumulator::add_simple_type(ETokenType type)
{
  if(saw_named_type) {
    const bool int128_named_type =
        named_type &&
        named_type->kind == Type::TK_FUNDAMENTAL &&
        (named_type->fundamental == FT_INT128 ||
         named_type->fundamental == FT_UINT128);
    if(int128_named_type &&
       (type == KW_SIGNED || type == KW_UNSIGNED) &&
       signed_count + unsigned_count == 0) {
      if(type == KW_SIGNED) {
        ++signed_count;
      } else {
        ++unsigned_count;
      }
      return true;
    }
    return false;
  }

  switch(type) {
  case KW_SIGNED: ++signed_count; return true;
  case KW_UNSIGNED: ++unsigned_count; return true;
  case KW_SHORT: ++short_count; return true;
  case KW_LONG: ++long_count; return true;
  case KW_INT: saw_int = true; return true;
  case KW_CHAR: saw_char = true; return true;
  case KW_CHAR16_T: saw_char16 = true; return true;
  case KW_CHAR32_T: saw_char32 = true; return true;
  case KW_WCHAR_T: saw_wchar = true; return true;
  case KW_BOOL: saw_bool = true; return true;
  case KW_FLOAT: saw_float = true; return true;
  case KW_DOUBLE: saw_double = true; return true;
  case KW_VOID: saw_void = true; return true;
  default: return false;
  }
}

bool TypeSpecifierAccumulator::set_named_type(const TypePtr & type)
{
  const bool sign_only_content =
      !short_count && !long_count && !saw_int && !saw_char && !saw_char16 &&
      !saw_char32 && !saw_wchar && !saw_bool && !saw_float && !saw_double &&
      !saw_void && (signed_count || unsigned_count);
  const bool int128_named_type =
      type &&
      type->kind == Type::TK_FUNDAMENTAL &&
      (type->fundamental == FT_INT128 || type->fundamental == FT_UINT128);
  if(has_type_content() && !(sign_only_content && int128_named_type)) {
    return false;
  }
  named_type = type;
  saw_named_type = true;
  return true;
}

bool TypeSpecifierAccumulator::finalize(TypePtr & out) const
{
  if(saw_named_type) {
    const bool int128_named_type =
        named_type &&
        named_type->kind == Type::TK_FUNDAMENTAL &&
        (named_type->fundamental == FT_INT128 || named_type->fundamental == FT_UINT128);
    if(signed_count || unsigned_count || short_count || long_count || saw_int ||
       saw_char || saw_char16 || saw_char32 || saw_wchar || saw_bool || saw_float ||
       saw_double || saw_void) {
      if(!int128_named_type ||
         signed_count > 1 || unsigned_count > 1 || signed_count + unsigned_count > 1 ||
         short_count || long_count || saw_int || saw_char || saw_char16 || saw_char32 ||
         saw_wchar || saw_bool || saw_float || saw_double || saw_void) {
        return false;
      }
      out = apply_cv(make_fundamental(unsigned_count ? FT_UINT128 : FT_INT128),
                     cv_const,
                     cv_volatile);
      return true;
    }
    out = apply_cv(named_type, cv_const, cv_volatile);
    return true;
  }

  TypePtr fundamental;
  if(!finalize_fundamental_type_specifiers(signed_count, unsigned_count,
                                           short_count, long_count, saw_int,
                                           saw_char, saw_char16, saw_char32,
                                           saw_wchar, saw_bool, saw_float,
                                           saw_double, saw_void,
                                           fundamental)) {
    return false;
  }

  out = apply_cv(fundamental, cv_const, cv_volatile);
  return true;
}

}  // namespace cpp_decl
