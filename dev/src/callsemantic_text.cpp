#include "callsemantic_internal.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <stdexcept>

#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "types.h"

using namespace std;

namespace callsemantic_internal {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;

std::string append_parenthesized_type_spelling_prefix(const std::string & before,
                                                      const std::string & declarator)
{
  const std::string trimmed = trim_space(before);
  return trimmed.empty() ? declarator : trimmed + " " + declarator;
}

std::string collapse_reparseable_scope_operators(const std::string & text)
{
  bool needs_collapse = false;
  for(size_t i = 0; i < text.size();) {
    if(text[i] != ':') {
      ++i;
      continue;
    }
    size_t j = i + 1;
    while(j < text.size() && text[j] == ':') {
      ++j;
    }
    if(j - i > 2) {
      needs_collapse = true;
      break;
    }
    i = j;
  }
  if(!needs_collapse) {
    return text;
  }

  std::string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size();) {
    if(text[i] == ':') {
      size_t j = i;
      while(j < text.size() && text[j] == ':') {
        ++j;
      }
      if(j - i >= 2) {
        out += "::";
      } else {
        out += ':';
      }
      i = j;
      continue;
    }
    out += text[i++];
  }
  return out;
}

struct TopLevelCvText
{
  TypePtr base;
  bool cv_const = false;
  bool cv_volatile = false;
};

TopLevelCvText collect_top_level_cv_text(const TypePtr & type)
{
  TopLevelCvText out;
  out.base = type;
  while(out.base && out.base->kind == Type::TK_CV) {
    out.cv_const = out.cv_const || out.base->cv_const;
    out.cv_volatile = out.cv_volatile || out.base->cv_volatile;
    out.base = out.base->inner;
  }
  return out;
}

std::string cv_qualifier_text(bool cv_const, bool cv_volatile)
{
  std::string out;
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

TypeSpellingParts spell_reparseable_type_argument(const TypePtr & type)
{
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return TypeSpellingParts{type_to_string(type->fundamental) + " ", ""};

  case Type::TK_NAMED:
  {
    std::string text = type->named_key.empty() ? type->named_display : type->named_key;
    if(text.compare(0, 19, "template-parameter ") == 0 ||
       text.compare(0, 10, "dependent ") == 0 ||
       text.compare(0, 10, "$dqmember:") == 0) {
      text = type->named_display;
    }
    text = collapse_reparseable_scope_operators(strip_elaborated_type_prefix(text));
    return TypeSpellingParts{text + " ", ""};
  }

  case Type::TK_CV:
  {
    TopLevelCvText cv = collect_top_level_cv_text(type);
    TypeSpellingParts inner = spell_reparseable_type_argument(cv.base);
    const std::string qualifier = cv_qualifier_text(cv.cv_const, cv.cv_volatile);
    if(!inner.after.empty()) {
      inner.before = trim_space(inner.before) + qualifier;
    } else {
      inner.before = trim_space(inner.before) + " " + qualifier + " ";
    }
    return inner;
  }

  case Type::TK_ATOMIC:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    return TypeSpellingParts{"_Atomic(" +
                                 trim_space(inner.before) +
                                 inner.after + ") ",
                             ""};
  }

  case Type::TK_POINTER:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    if(!inner.after.empty()) {
      return TypeSpellingParts{
          append_parenthesized_type_spelling_prefix(inner.before, "(*"),
          ")" + inner.after};
    }
    return TypeSpellingParts{trim_space(inner.before) + " * ", ""};
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    const std::string owner_text =
        trim_space(spell_reparseable_type_argument(type->owner).before);
    if(!inner.after.empty()) {
      return TypeSpellingParts{
          append_parenthesized_type_spelling_prefix(inner.before,
                                                    "(" + owner_text + "::*"),
          ")" + inner.after};
    }
    return TypeSpellingParts{trim_space(inner.before) + " " + owner_text + "::* ", ""};
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    if(!inner.after.empty()) {
      return TypeSpellingParts{
          append_parenthesized_type_spelling_prefix(inner.before, "(^"),
          ")" + inner.after};
    }
    return TypeSpellingParts{trim_space(inner.before) + " ^ ", ""};
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    if(!inner.after.empty()) {
      return TypeSpellingParts{
          append_parenthesized_type_spelling_prefix(inner.before, "(&"),
          ")" + inner.after};
    }
    return TypeSpellingParts{trim_space(inner.before) + " & ", ""};
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    if(!inner.after.empty()) {
      return TypeSpellingParts{
          append_parenthesized_type_spelling_prefix(inner.before, "(&&"),
          ")" + inner.after};
    }
    return TypeSpellingParts{trim_space(inner.before) + " && ", ""};
  }

  case Type::TK_ARRAY:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    if(type->has_bound) {
      std::ostringstream out;
      out << "[" << type->bound << "]";
      inner.after += out.str();
    } else if(!type->bound_text.empty()) {
      inner.after += "[" + type->bound_text + "]";
    } else {
      inner.after += "[]";
    }
    return inner;
  }

  case Type::TK_FUNCTION:
  {
    TypeSpellingParts inner = spell_reparseable_type_argument(type->inner);
    inner.before = trim_space(inner.before);
    std::ostringstream out;
    out << "(";
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      TypeSpellingParts param = spell_reparseable_type_argument(type->params[i]);
      out << trim_space(param.before + param.after);
    }
    if(type->variadic) {
      if(!type->params.empty()) {
        out << ", ";
      }
      out << "...";
    } else if(type->prototype_relaxed) {
      if(!type->params.empty()) {
        out << ", ";
      }
      out << "<prototype_relaxed>";
    }
    out << ")";
    if(type->function_const) {
      out << " const";
    }
    if(type->function_volatile) {
      out << " volatile";
    }
    if(type->function_ref_qualifier == FTRQ_LVALUE) {
      out << " &";
    } else if(type->function_ref_qualifier == FTRQ_RVALUE) {
      out << " &&";
    }
    inner.after += out.str();
    return inner;
  }
  }

  throw logic_error("unknown type kind in spell_reparseable_type_argument");
}

std::string reparseable_type_argument_text(const TypePtr & type)
{
  if(!type) {
    return std::string();
  }
  const TypeSpellingParts parts = spell_reparseable_type_argument(type);
  return trim_space(parts.before + parts.after);
}

void snapshot_function_template_debug_info(SemanticContext & ctx,
                                           FunctionTemplateDecl & decl)
{
  decl.debug_decl_location = semantic_trace::template_decl_primary_location(ctx, &decl);
  decl.debug_decl_location_details = semantic_trace::template_decl_location_details(ctx, &decl);
  decl.debug_scope_name =
      decl.declaring_scope ? semantic_trace::scope_name_for_diagnostic(*decl.declaring_scope) :
                             std::string("<none>");
  decl.debug_signature = semantic_trace::function_template_signature_for_diagnostic(decl);
}

string normalize_type_lookup_name(const string & text)
{
  string out = trim_space(text);
  const string typename_prefix = "typename ";
  if(out.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    out.erase(0, typename_prefix.size());
  }

  const string template_marker = "::template ";
  size_t pos = 0;
  while((pos = out.find(template_marker, pos)) != string::npos) {
    out.erase(pos + 2, template_marker.size() - 2);
    pos += 2;
  }
  out = strip_elaborated_type_prefix(out);
  if(out == "unsigned") {
    return "unsigned int";
  }
  if(out == "signed") {
    return "int";
  }
  return out;
}

string normalize_qualified_name_spacing(const string & text)
{
  string out;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(!std::isspace(static_cast<unsigned char>(ch))) {
      out += ch;
      continue;
    }

    size_t next = i + 1;
    while(next < text.size() &&
          std::isspace(static_cast<unsigned char>(text[next]))) {
      ++next;
    }

    const char next_ch = next < text.size() ? text[next] : '\0';
    const char prev_ch = out.empty() ? '\0' : out.back();
    if(next_ch == '>' || next_ch == ':' || prev_ch == ':') {
      continue;
    }

    if(!out.empty() && out.back() != ' ') {
      out += ' ';
    }
    i = next - 1;
  }
  return trim_space(out);
}

bool has_top_level_declarator_syntax(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(angle_depth != 0 || paren_depth != 0) {
      continue;
    }
    if(ch == '*' || ch == '&' || ch == '[' || ch == ']') {
      return true;
    }
  }
  return false;
}

bool has_invalid_top_level_qualified_owner_syntax(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(ch == '[') {
      ++bracket_depth;
      continue;
    }
    if(ch == ']' && bracket_depth > 0) {
      --bracket_depth;
      continue;
    }
    if(ch == '{') {
      ++brace_depth;
      continue;
    }
    if(ch == '}' && brace_depth > 0) {
      --brace_depth;
      continue;
    }
    if(angle_depth != 0 || paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(ch == '*' || ch == '&') {
      size_t next = i + 1;
      while(next < text.size() &&
            isspace(static_cast<unsigned char>(text[next]))) {
        ++next;
      }
      if(next + 1 < text.size() &&
         text[next] == ':' &&
         text[next + 1] == ':') {
        return true;
      }
    }
  }
  return false;
}

bool strip_top_level_cv_text(string text,
                             string & core,
                             bool & cv_const,
                             bool & cv_volatile)
{
  cv_const = false;
  cv_volatile = false;
  core = trim_space(text);
  if(core.empty() || has_top_level_declarator_syntax(core)) {
    return false;
  }

  bool changed = false;
  bool progress = true;
  while(progress) {
    progress = false;
    if(core.compare(0, 6, "const ") == 0) {
      core.erase(0, 6);
      core = trim_space(core);
      cv_const = true;
      changed = progress = true;
      continue;
    }
    if(core.compare(0, 9, "volatile ") == 0) {
      core.erase(0, 9);
      core = trim_space(core);
      cv_volatile = true;
      changed = progress = true;
      continue;
    }
    if(core.size() > 6 && core.compare(core.size() - 6, 6, " const") == 0) {
      core.erase(core.size() - 6);
      core = trim_space(core);
      cv_const = true;
      changed = progress = true;
      continue;
    }
    if(core.size() > 9 && core.compare(core.size() - 9, 9, " volatile") == 0) {
      core.erase(core.size() - 9);
      core = trim_space(core);
      cv_volatile = true;
      changed = progress = true;
      continue;
    }
  }

  return changed && !core.empty();
}

TypePtr match_wrapped_type_text(const string & text,
                                const string & base_text,
                                const TypePtr & base_type)
{
  string remaining = trim_space(text);
  bool cv_const = false;
  bool cv_volatile = false;
  bool changed = true;
  while(changed) {
    changed = false;
    if(remaining.compare(0, 6, "const ") == 0) {
      cv_const = true;
      remaining = trim_space(remaining.substr(6));
      changed = true;
    }
    if(remaining.compare(0, 9, "volatile ") == 0) {
      cv_volatile = true;
      remaining = trim_space(remaining.substr(9));
      changed = true;
    }
  }

  enum RefKind { RK_NONE, RK_LVALUE, RK_RVALUE };
  RefKind ref_kind = RK_NONE;
  if(remaining.size() >= 2 &&
     remaining.compare(remaining.size() - 2, 2, "&&") == 0) {
    ref_kind = RK_RVALUE;
    remaining = trim_space(remaining.substr(0, remaining.size() - 2));
  } else if(!remaining.empty() && remaining[remaining.size() - 1] == '&') {
    ref_kind = RK_LVALUE;
    remaining = trim_space(remaining.substr(0, remaining.size() - 1));
  }

  size_t pointer_depth = 0;
  while(!remaining.empty()) {
    size_t end = remaining.size();
    while(end > 0 &&
          isspace(static_cast<unsigned char>(remaining[end - 1]))) {
      --end;
    }
    if(end == 0 || remaining[end - 1] != '*') {
      break;
    }
    ++pointer_depth;
    remaining = trim_space(remaining.substr(0, end - 1));
  }

  string cv_core;
  bool cv_suffix_const = false;
  bool cv_suffix_volatile = false;
  if(strip_top_level_cv_text(remaining,
                             cv_core,
                             cv_suffix_const,
                             cv_suffix_volatile)) {
    remaining = cv_core;
    cv_const = cv_const || cv_suffix_const;
    cv_volatile = cv_volatile || cv_suffix_volatile;
  }

  const string normalized_remaining =
      remove_space_chars(normalize_type_lookup_name(remaining));
  const string normalized_base =
      remove_space_chars(normalize_type_lookup_name(base_text));
  if(normalized_remaining != normalized_base) {
    return TypePtr();
  }

  TypePtr matched = base_type;
  if(cv_const || cv_volatile) {
    matched = make_cv(matched, cv_const, cv_volatile);
  }
  while(pointer_depth != 0) {
    matched = make_pointer(matched);
    --pointer_depth;
  }
  if(ref_kind == RK_LVALUE) {
    matched = make_lvalue_reference_raw(matched);
  } else if(ref_kind == RK_RVALUE) {
    matched = make_rvalue_reference_raw(matched);
  }
  return matched;
}

bool parse_elaborated_class_lookup_name(const string & text,
                                        string & class_kind,
                                        string & declared_name)
{
  string out = trim_space(text);
  const string typename_prefix = "typename ";
  if(out.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    out.erase(0, typename_prefix.size());
  }

  const string template_marker = "::template ";
  size_t pos = 0;
  while((pos = out.find(template_marker, pos)) != string::npos) {
    out.erase(pos + 2, template_marker.size() - 2);
    pos += 2;
  }

  if(out.compare(0, 6, "class ") == 0) {
    class_kind = "class";
    declared_name = trim_space(out.substr(6));
    return !declared_name.empty();
  }
  if(out.compare(0, 7, "struct ") == 0) {
    class_kind = "struct";
    declared_name = trim_space(out.substr(7));
    return !declared_name.empty();
  }
  if(out.compare(0, 6, "union ") == 0) {
    class_kind = "union";
    declared_name = trim_space(out.substr(6));
    return !declared_name.empty();
  }

  return false;
}

bool declarator_has_parameter_pack(const CppAstNode & declarator)
{
  if(cpp_decl::find_child(declarator, CppAstKind::parameter_pack)) {
    return true;
  }
  const CppAstNode * nested = cpp_decl::find_child(declarator, CppAstKind::nested_declarator);
  return nested && !nested->children.empty() && declarator_has_parameter_pack(nested->children[0]);
}

bool declarator_has_trailing_function_parameter_pack(const CppAstNode & declarator)
{
  const CppAstNode * parameter_clause =
      cpp_decl::find_child(declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || parameter_clause->children.empty()) {
    return false;
  }
  const CppAstNode & last = parameter_clause->children.back();
  if(last.kind != CppAstKind::parameter_declaration) {
    return false;
  }
  const CppAstNode * declarator_child = cpp_decl::find_child(last, CppAstKind::declarator);
  const CppAstNode * abstract = cpp_decl::find_child(last, CppAstKind::abstract_declarator);
  if(!(declarator_child && declarator_has_parameter_pack(*declarator_child)) &&
     !(abstract && declarator_has_parameter_pack(*abstract))) {
    return false;
  }
  return true;
}

bool is_pure_virtual_initializer(const CppAstNode & initializer)
{
  if(initializer.kind != CppAstKind::initializer ||
     initializer.children.size() != 1) {
    return false;
  }

  const CppAstNode & child = initializer.children[0];
  return child.kind == CppAstKind::literal &&
         semantic_utils::trim_space(node_text(child)) == "0";
}

bool subtree_contains_pure_virtual_initializer(const CppAstNode & node)
{
  if(is_pure_virtual_initializer(node)) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(subtree_contains_pure_virtual_initializer(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool declaration_node_is_pure_virtual(const CppAstNode * declaration_node)
{
  if(!declaration_node) {
    return false;
  }

  const CppAstNode * initializer =
      cpp_decl::find_child(*declaration_node, CppAstKind::initializer);
  if(initializer && is_pure_virtual_initializer(*initializer)) {
    return true;
  }

  std::string compact = semantic_utils::trim_space(node_text(*declaration_node));
  compact = remove_space_chars(compact);
  if(!compact.empty() && compact[compact.size() - 1] == ';') {
    compact.resize(compact.size() - 1);
  }
  return compact.size() >= 2 &&
         compact.compare(compact.size() - 2, 2, "=0") == 0;
}

bool contains_identifier_token(const string & text, const string & name)
{
  if(name.empty()) {
    return false;
  }

  size_t pos = 0;
  while((pos = text.find(name, pos)) != string::npos) {
    const bool left_ok =
        pos == 0 ||
        !(isalnum(static_cast<unsigned char>(text[pos - 1])) || text[pos - 1] == '_');
    const size_t end = pos + name.size();
    const bool right_ok =
        end == text.size() ||
        !(isalnum(static_cast<unsigned char>(text[end])) || text[end] == '_');
    if(left_ok && right_ok) {
      return true;
    }
    pos = end;
  }
  return false;
}

bool identifier_is_qualified_component(const string & text, size_t pos)
{
  while(pos > 0) {
    const char ch = text[pos - 1];
    if(isspace(static_cast<unsigned char>(ch))) {
      --pos;
      continue;
    }
    if(ch != ':') {
      return false;
    }
    if(pos < 2) {
      return false;
    }
    size_t previous = pos - 1;
    while(previous > 0 &&
          isspace(static_cast<unsigned char>(text[previous - 1]))) {
      --previous;
    }
    return previous > 0 && text[previous - 1] == ':';
  }
  return false;
}

bool is_identifier_text(const string & text)
{
  if(text.empty() ||
     !(isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_')) {
    return false;
  }
  for(size_t i = 1; i < text.size(); ++i) {
    if(!(isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_')) {
      return false;
    }
  }
  return true;
}

IdentifierTokenSet collect_identifier_tokens(const string & text)
{
  IdentifierTokenSet out;
  out.reserve(text.size() / 4 + 1);
  size_t i = 0;
  while(i < text.size()) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!(isalpha(ch) || text[i] == '_')) {
      ++i;
      continue;
    }
    const size_t start = i++;
    while(i < text.size()) {
      const unsigned char inner = static_cast<unsigned char>(text[i]);
      if(!(isalnum(inner) || text[i] == '_')) {
        break;
      }
      ++i;
    }
    out.insert(intern_text_atom(text.data() + start, i - start));
  }
  return out;
}

void maybe_complete_sizeof_type(SemanticContext & ctx, const TypePtr & type)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return;
  }
  if(base->kind == Type::TK_ARRAY) {
    maybe_complete_sizeof_type(ctx, base->inner);
    return;
  }
  if(base->kind == Type::TK_NAMED && !base->named_has_layout) {
    ClassInfo * info = ctx.complete_class_type(base);
    if(info && info->type) {
      base->named_complete = info->type->named_complete;
      base->named_has_layout = info->type->named_has_layout;
      base->named_alignment = info->type->named_alignment;
      base->named_size = info->type->named_size;
      base->named_is_empty = info->type->named_is_empty;
      base->named_host_abi_chunks = info->type->named_host_abi_chunks;
      base->set_named_lambda_mangle(info->type->named_lambda_mangle());
      base->named_class_template_specialization_mangle_info =
          info->type->named_class_template_specialization_mangle_info;
    }
  }
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer)
{
  if(initializer && initializer->kind == CppAstKind::initializer &&
     initializer->children.size() == 1) {
    return &initializer->children[0];
  }
  return initializer;
}

bool count_braced_initializer_bound_elements(SemanticContext * ctx,
                                             Scope * scope,
                                             const CppAstNode & payload,
                                             size_t & out_bound)
{
  out_bound = 0;
  for(size_t i = 0; i < payload.children.size(); ++i) {
    const CppAstNode & child = payload.children[i];
    if(child.kind == CppAstKind::pack_expansion_expression && ctx && scope) {
      vector<CppAstNode> expanded_nodes;
      if(ctx->expand_pack_argument_node(*scope, child, expanded_nodes)) {
        out_bound += expanded_nodes.size();
        continue;
      }
    }
    ++out_bound;
  }
  return true;
}

bool infer_unknown_bound_array_size_impl(SemanticContext * ctx,
                                         Scope * scope,
                                         const TypePtr & type,
                                         const CppAstNode * initializer,
                                         size_t & out_bound)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_ARRAY || base->has_bound || !base->inner || !initializer) {
    return false;
  }

  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(!payload) {
    return false;
  }

  const auto infer_string_literal_bound =
      [&](const CppAstNode & literal_node) -> bool
      {
        if(literal_node.kind != CppAstKind::literal) {
          return false;
        }
        try
        {
          QuoteLiteralData literal = parse_quote_literal(literal_node.value);
          if(literal.quote != '"' || !literal.ud_suffix.empty()) {
            return false;
          }
          const TypePtr element_type = strip_top_level_cv(base->inner);
          if(!element_type || element_type->kind != Type::TK_FUNDAMENTAL) {
            return false;
          }
          const EFundamentalType literal_type = string_literal_element_type(literal);
          const EFundamentalType element_fundamental = element_type->fundamental;
          if(literal_type == FT_CHAR) {
            if(element_fundamental != FT_CHAR &&
               element_fundamental != FT_SIGNED_CHAR &&
               element_fundamental != FT_UNSIGNED_CHAR) {
              return false;
            }
          } else if(element_fundamental != literal_type) {
            return false;
          }
          out_bound = quote_literal_string_unit_count(literal) + 1;
          return true;
        }
        catch(const logic_error &)
        {
          return false;
        }
      };

  if(payload->kind == CppAstKind::braced_init_list) {
    if(payload->children.size() == 1 &&
       infer_string_literal_bound(payload->children[0])) {
      return true;
    }
    return count_braced_initializer_bound_elements(ctx, scope, *payload, out_bound);
  }

  if(payload->kind == CppAstKind::literal) {
    return infer_string_literal_bound(*payload);
  }

  return false;
}

bool infer_unknown_bound_array_size(const TypePtr & type,
                                    const CppAstNode * initializer,
                                    size_t & out_bound)
{
  return infer_unknown_bound_array_size_impl(nullptr, nullptr, type, initializer, out_bound);
}

bool infer_unknown_bound_array_size(SemanticContext & ctx,
                                    Scope & scope,
                                    const TypePtr & type,
                                    const CppAstNode * initializer,
                                    size_t & out_bound)
{
  return infer_unknown_bound_array_size_impl(&ctx, &scope, type, initializer, out_bound);
}

TypePtr apply_initializer_array_bound(const TypePtr & type,
                                      const CppAstNode * initializer)
{
  size_t bound = 0;
  if(!infer_unknown_bound_array_size(type, initializer, bound)) {
    return type;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return make_array(base->inner, true, bound);
}

TypePtr apply_initializer_array_bound(SemanticContext & ctx,
                                      Scope & scope,
                                      const TypePtr & type,
                                      const CppAstNode * initializer)
{
  size_t bound = 0;
  if(!infer_unknown_bound_array_size(ctx, scope, type, initializer, bound)) {
    return type;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return make_array(base->inner, true, bound);
}

string recog_token_text_for_span(const RecogToken & token)
{
  if(token.is_rshift_piece()) {
    return ">";
  }
  return token.source;
}

bool recog_token_text_needs_separator(const RecogToken & lhs,
                                      const RecogToken & rhs)
{
  const auto is_word_like = [](const RecogToken & token) -> bool
  {
    if(token.kind == RT_IDENTIFIER || token.kind == RT_LITERAL) {
      return true;
    }
    if(token.kind != RT_SIMPLE || token.source.empty()) {
      return false;
    }
    const char c = token.source[0];
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '_';
  };

  return is_word_like(lhs) && is_word_like(rhs);
}

string remove_space_chars(string text)
{
  // Match std::isspace under the C locale (the only locale cppgm++ runs in)
  // with a direct ASCII test, avoiding the locale-table lookup per character.
  text.erase(remove_if(text.begin(), text.end(),
                       [](unsigned char ch) {
                         return ch == ' ' || ch == '\t' || ch == '\n' ||
                                ch == '\r' || ch == '\f' || ch == '\v';
                       }),
             text.end());
  return text;
}

bool is_literal_operator_function_name(const string & name)
{
  const string compact = remove_space_chars(unqualified_member_name(name));
  const string literal_operator_prefix = "operator\"\"";
  return compact.compare(0,
                         literal_operator_prefix.size(),
                         literal_operator_prefix) == 0 &&
         compact.size() > literal_operator_prefix.size();
}

bool is_builtin_operator_function_name(const string & name)
{
  const string compact = remove_space_chars(unqualified_member_name(name));
  if(compact.compare(0, 8, "operator") != 0) {
    return false;
  }
  if(is_literal_operator_function_name(name)) {
    return true;
  }

  static const set<string> operator_names = {
      "operator=", "operator()", "operator[]",
      "operator+", "operator-", "operator*", "operator/", "operator%",
      "operator^", "operator&", "operator|", "operator~", "operator!",
      "operator<", "operator>", "operator+=", "operator-=", "operator*=",
      "operator/=", "operator%=", "operator^=", "operator&=", "operator|=",
      "operator<<", "operator>>", "operator>>=", "operator<<=",
      "operator==", "operator!=", "operator<=", "operator>=",
      "operator&&", "operator||", "operator++", "operator--", "operator,",
      "operator->*", "operator->", "operatornew", "operatordelete",
      "operatornew[]", "operatordelete[]"};
  return operator_names.find(compact) != operator_names.end();
}

size_t conversion_operator_name_start(const string & name)
{
  if(name.compare(0, 8, "operator") == 0) {
    return 0;
  }
  // No "operator" substring anywhere means no top-level "::operator" either, so
  // skip the per-character depth scan for the common (non-operator) name.
  if(name.find("operator") == string::npos) {
    return string::npos;
  }

  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t i = 0; i + 1 < name.size(); ++i) {
    switch(name[i]) {
    case '<':
      ++angle_depth;
      break;
    case '>':
      if(angle_depth > 0) {
        --angle_depth;
      }
      break;
    case '(':
      ++paren_depth;
      break;
    case ')':
      if(paren_depth > 0) {
        --paren_depth;
      }
      break;
    case '[':
      ++bracket_depth;
      break;
    case ']':
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      break;
    case '{':
      ++brace_depth;
      break;
    case '}':
      if(brace_depth > 0) {
        --brace_depth;
      }
      break;
    case ':':
      if(angle_depth == 0 && paren_depth == 0 && bracket_depth == 0 &&
         brace_depth == 0 && name[i + 1] == ':' &&
         name.compare(i + 2, 8, "operator") == 0) {
        return i + 2;
      }
      ++i;
      break;
    default:
      break;
    }
  }
  return string::npos;
}

bool is_conversion_function_name(const string & name)
{
  const size_t start = conversion_operator_name_start(name);
  if(start == string::npos) {
    return false;
  }
  return !is_builtin_operator_function_name(name.substr(start));
}

string append_diagnostic_context_message(const string & message)
{
  string full_message = message;
  parser_trace::append_to_error(full_message);

  const bool has_context = full_message.find("\nDiagnostic context:\n") != string::npos;
  const string full_context = has_context ? string() : DiagnosticContext::format_stack();
  const string compact_context =
      has_context ? string() :
                    DiagnosticContext::format_stack_compact(
                        semantic_trace::diagnostic_max_stack_frames(),
                        semantic_trace::diagnostic_max_line_chars());

  string full_output = full_message;
  if(!full_context.empty()) {
    full_output += "\nDiagnostic context:\n" + full_context;
  }

  if(!semantic_trace::diagnostic_mode_compact()) {
    return full_output;
  }

  string compact_output = full_message;
  if(!compact_context.empty()) {
    compact_output += "\nDiagnostic context:\n" + compact_context;
  }
  compact_output = semantic_trace::compact_diagnostic_message(
      compact_output,
      semantic_trace::diagnostic_max_line_chars());

  if(semantic_trace::diagnostic_mode_sidecar() &&
     compact_output != full_output) {
    const string sidecar =
        semantic_trace::write_diagnostic_sidecar(full_message, full_context);
    if(!sidecar.empty()) {
      compact_output += "\nDiagnostic detail: " + sidecar;
    }
  }
  return compact_output;
}

CallValueCategory to_call_value_category(ValueCategory category)
{
  switch(category) {
  case VC_LVALUE: return CVC_LVALUE;
  case VC_PRVALUE: return CVC_PRVALUE;
  case VC_XVALUE: return CVC_XVALUE;
  }

  throw logic_error("unknown value category");
}

const CppAstNode * find_child_kind(const CppAstNode & node,
                                   CppAstKind kind,
                                   size_t ordinal)
{
  return find_child(node, kind, ordinal);
}

MemberAccess default_access_for_class_kind(const string & class_kind)
{
  return semantic_class_model::default_access_for_class_kind(class_kind);
}

MemberAccess access_from_node(const CppAstNode & node)
{
  if(node_has_simple_type(node, KW_PUBLIC)) {
    return MA_PUBLIC;
  }
  if(node_has_simple_type(node, KW_PROTECTED)) {
    return MA_PROTECTED;
  }
  return MA_PRIVATE;
}

string describe_expression_for_diagnostic(const CppAstNode & node)
{
  if(!node.children.empty()) {
    if(node.kind == CppAstKind::unary_expression && node.children.size() == 1) {
      return node.value + describe_expression_for_diagnostic(node.children[0]);
    }
    if(node.kind == CppAstKind::binary_expression && node.children.size() == 2) {
      return describe_expression_for_diagnostic(node.children[0]) + " " + node.value + " " +
             describe_expression_for_diagnostic(node.children[1]);
    }
    if(node.kind == CppAstKind::call_expression && node.children.size() == 2 &&
       (node.children[1].kind == CppAstKind::argument_list ||
        node.children[1].kind == CppAstKind::paren_argument_list)) {
      if(node.children[1].kind == CppAstKind::argument_list &&
         node.children[1].children.size() == 1 &&
         node.children[1].children[0].kind == CppAstKind::braced_init_list) {
        return describe_expression_for_diagnostic(node.children[0]) +
               describe_expression_for_diagnostic(node.children[1].children[0]);
      }
      ostringstream out;
      out << describe_expression_for_diagnostic(node.children[0]) << "(";
      for(size_t i = 0; i < node.children[1].children.size(); ++i) {
        if(i != 0) {
          out << ", ";
        }
        out << describe_expression_for_diagnostic(node.children[1].children[i]);
      }
      out << ")";
      return out.str();
    }
    if(node.kind == CppAstKind::parenthesized_expression && node.children.size() == 1) {
      return "(" + describe_expression_for_diagnostic(node.children[0]) + ")";
    }
    if(node.kind == CppAstKind::fold_expression) {
      if(node.children.size() == 2) {
        if(node.children[0].kind == CppAstKind::ellipsis) {
          return "(... " + node.value + " " +
                 describe_expression_for_diagnostic(node.children[1]) + ")";
        }
        if(node.children[1].kind == CppAstKind::ellipsis) {
          return "(" + describe_expression_for_diagnostic(node.children[0]) + " " +
                 node.value + " ...)";
        }
      }
      if(node.children.size() == 3 && node.children[1].kind == CppAstKind::ellipsis) {
        return "(" + describe_expression_for_diagnostic(node.children[0]) + " " +
               node.value + " ... " + node.value + " " +
               describe_expression_for_diagnostic(node.children[2]) + ")";
      }
    }
    if(node.kind == CppAstKind::member_expression && node.children.size() == 2) {
      return describe_expression_for_diagnostic(node.children[0]) + node.value +
             describe_expression_for_diagnostic(node.children[1]);
    }
    if(node.kind == CppAstKind::type_trait_expression && !node.children.empty()) {
      ostringstream out;
      out << node.value << "(";
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(i != 0) {
          out << ", ";
        }
        out << describe_expression_for_diagnostic(node.children[i]);
      }
      out << ")";
      return out.str();
    }
    if(node.kind == CppAstKind::sizeof_pack_expression &&
       node.children.size() == 1) {
      return "sizeof...(" + describe_expression_for_diagnostic(node.children[0]) + ")";
    }
    if(node.kind == CppAstKind::sizeof_expression &&
       node.children.size() == 1) {
      return "sizeof(" + describe_expression_for_diagnostic(node.children[0]) + ")";
    }
    if(node.kind == CppAstKind::braced_init_list) {
      ostringstream out;
      out << "{";
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(i != 0) {
          out << ", ";
        }
        out << describe_expression_for_diagnostic(node.children[i]);
      }
      out << "}";
      return out.str();
    }
    if(node.kind == CppAstKind::type_id) {
      return node_text(node);
    }
  }

  if(!node.value.empty()) {
    return node.value;
  }
  return node_text(node);
}

string describe_scope_bindings_for_diagnostic(const Scope & scope)
{
  return semantic_trace::scope_bindings_for_diagnostic(scope);
}

void set_expr_metadata(DumpNode & node,
                       const TypePtr & type,
                       ValueCategory category)
{
  node.semantic_type = type;
  set_callsem_materialization_source_type(node, TypePtr());
  set_callsem_conversion_source_type(node, TypePtr());
  node.value_category = to_call_value_category(category);
}

}  // namespace callsemantic_internal
