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
    std::string text =
        type->named_key.empty() ? named_type_display_text(type) :
                                  type->named_key;
    if(text.compare(0, 19, "template-parameter ") == 0 ||
       text.compare(0, 10, "dependent ") == 0 ||
       text.compare(0, 10, "$dqmember:") == 0) {
      text = named_type_display_text(type);
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
  out.reserve(text.size());
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
    if(next_ch == '\0' ||
       next_ch == '>' ||
       next_ch == ':' ||
       prev_ch == ':') {
      continue;
    }

    if(!out.empty() && out.back() != ' ') {
      out += ' ';
    }
    i = next - 1;
  }
  return out;
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

bool is_pure_virtual_initializer(const CppAstNode & initializer)
{
  if(initializer.kind != CppAstKind::initializer ||
     initializer.children.size() != 1) {
    return false;
  }

  const CppAstNode & child = initializer.children[0];
  return child.kind == CppAstKind::literal &&
         child.value == "0";
}

bool declaration_node_is_pure_virtual(const CppAstNode * declaration_node)
{
  if(!declaration_node) {
    return false;
  }

  const CppAstNode * special_definition =
      cpp_decl::find_child(*declaration_node, CppAstKind::special_definition);
  if(special_definition && special_definition->value == "0") {
    return true;
  }

  const CppAstNode * initializer =
      cpp_decl::find_child(*declaration_node, CppAstKind::initializer);
  return initializer && is_pure_virtual_initializer(*initializer);
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
  out.reserve(std::min<std::size_t>(text.size() / 4 + 1, 32));
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
    out.insert(text.data() + start, i - start);
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
      base->mutable_named_rare_metadata()
          .named_class_template_specialization_mangle_info =
              info->type->named_rare()
                  .named_class_template_specialization_mangle_info;
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

namespace {

using StructuredTypeNameReplacements = map<string, string>;

string render_structured_expression(const CppAstNode & node,
                                    bool template_argument_policy,
                                    const StructuredTypeNameReplacements *
                                        type_name_replacements,
                                    bool final_template_disambiguator = false);
string render_structured_type(
    const CppAstNode & node,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool final_template_disambiguator = false);
string render_structured_template_argument(
    const TemplateArgumentSyntax & syntax,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool preserve_source_template_disambiguator = true);

bool structured_expression_contains_qualified_id(const CppAstNode & node)
{
  if(node.qualified_name_syntax &&
     (!node.qualified_name_syntax->qualifiers.empty() ||
      node.qualified_name_syntax->rooted)) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(structured_expression_contains_qualified_id(node.children[i])) {
      return true;
    }
  }
  return false;
}

string render_structured_template_id_with_prefix_drop(
    const TemplateIdSyntax & syntax,
    size_t prefix_drop,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool preserve_final_template_disambiguator = true);

string render_structured_qualified_name(
    const QualifiedName & name,
    const CppAstLazyVector<TemplateIdSyntax> * node_qualifier_template_ids,
    const vector<TemplateIdSyntax> * template_qualifier_template_ids,
    size_t prefix_drop,
    bool final_template_disambiguator,
    const StructuredTypeNameReplacements * type_name_replacements)
{
  ostringstream out;
  if(name.rooted && prefix_drop == 0) {
    out << "::";
  }
  for(size_t i = prefix_drop; i < name.qualifiers.size(); ++i) {
    if(i != prefix_drop) {
      out << "::";
    }
    const TemplateIdSyntax * component = nullptr;
    if(node_qualifier_template_ids &&
       i < node_qualifier_template_ids->size() &&
       !(*node_qualifier_template_ids)[i].name.name.empty()) {
      component = &(*node_qualifier_template_ids)[i];
    } else if(template_qualifier_template_ids &&
              i < template_qualifier_template_ids->size() &&
              !(*template_qualifier_template_ids)[i].name.name.empty()) {
      component = &(*template_qualifier_template_ids)[i];
    }
    if(component) {
      out << render_structured_template_id_with_prefix_drop(
          *component, i, type_name_replacements);
    } else {
      out << name.qualifiers[i];
    }
  }
  if(prefix_drop < name.qualifiers.size()) {
    out << "::";
  }
  if(final_template_disambiguator) {
    out << "template ";
  }
  out << name.name;
  return out.str();
}

string render_structured_template_id_with_prefix_drop(
    const TemplateIdSyntax & syntax,
    size_t prefix_drop,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool preserve_final_template_disambiguator)
{
  ostringstream out;
  out << render_structured_qualified_name(
             syntax.name,
             nullptr,
             &syntax.qualifier_template_id_syntaxes,
             prefix_drop,
             preserve_final_template_disambiguator &&
                 syntax.name_has_template_disambiguator,
             type_name_replacements)
      << "<";
  const size_t count = !syntax.argument_syntaxes.empty() ?
      syntax.argument_syntaxes.size() : syntax.arguments.size();
  for(size_t i = 0; i < count; ++i) {
    if(i != 0) {
      out << ", ";
    }
    if(i < syntax.argument_syntaxes.size()) {
      out << render_structured_template_argument(
          syntax.argument_syntaxes[i], type_name_replacements);
    } else {
      out << trim_space(syntax.arguments[i]);
    }
  }
  out << ">";
  return out.str();
}

string render_structured_node_name(
    const CppAstNode & node,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool permit_type_name_replacement,
    bool final_template_disambiguator = false)
{
  string text;
  if(node.template_id_syntax) {
    text = render_structured_template_id_with_prefix_drop(
        *node.template_id_syntax, 0, type_name_replacements);
  } else if(node.qualified_name_syntax) {
    const QualifiedName & name = *node.qualified_name_syntax;
    StructuredTypeNameReplacements::const_iterator replacement =
        type_name_replacements ?
            type_name_replacements->find(name.name) :
            StructuredTypeNameReplacements::const_iterator();
    if(permit_type_name_replacement &&
       type_name_replacements &&
       !name.rooted &&
       name.qualifiers.empty() &&
       replacement != type_name_replacements->end()) {
      text = replacement->second;
    } else {
      text = render_structured_qualified_name(
          name,
          &node.qualifier_template_id_syntaxes,
          nullptr,
          0,
          final_template_disambiguator,
          type_name_replacements);
    }
  } else {
    StructuredTypeNameReplacements::const_iterator replacement =
        type_name_replacements ?
            type_name_replacements->find(node.value) :
            StructuredTypeNameReplacements::const_iterator();
    text = permit_type_name_replacement &&
               type_name_replacements &&
               replacement != type_name_replacements->end() ?
        replacement->second : node.value;
  }
  if(node.has_leading_typename &&
     text.compare(0, 9, "typename ") != 0) {
    text = "typename " + text;
  }
  return text;
}

string render_structured_type_sequence(
    const CppAstNode & node,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool final_template_disambiguator = false)
{
  ostringstream out;
  // QualType::print places top-level cv qualifiers before the base type even
  // when the declaration grammar accepted them after it.  Preserve the AST
  // distinction instead of repairing the completed binding as text.
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind != CppAstKind::cv_qualifier) {
      continue;
    }
    const string child = render_structured_type(
        node.children[i], type_name_replacements);
    if(child.empty()) {
      continue;
    }
    if(out.tellp() > 0) {
      out << " ";
    }
    out << child;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::cv_qualifier) {
      continue;
    }
    const string child = render_structured_type(
        node.children[i],
        type_name_replacements,
        final_template_disambiguator);
    if(child.empty()) {
      continue;
    }
    if(out.tellp() > 0) {
      out << " ";
    }
    out << child;
  }
  if(out.tellp() > 0) {
    return out.str();
  }
  return render_structured_node_name(node,
                                     type_name_replacements,
                                     true,
                                     final_template_disambiguator);
}

string render_structured_parameter_declaration(
    const CppAstNode & node,
    const StructuredTypeNameReplacements * type_name_replacements)
{
  string base;
  const CppAstNode * declarator = nullptr;
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::decl_specifier_seq ||
       node.children[i].kind == CppAstKind::type_specifier_seq) {
      base = render_structured_type_sequence(node.children[i],
                                             type_name_replacements);
    } else if(node.children[i].kind == CppAstKind::declarator ||
              node.children[i].kind == CppAstKind::abstract_declarator) {
      declarator = &node.children[i];
    }
  }
  if(!declarator) {
    return base.empty() ?
        render_structured_node_name(node, type_name_replacements, true) :
        base;
  }

  string out = base;
  for(size_t i = 0; i < declarator->children.size(); ++i) {
    const CppAstNode & child = declarator->children[i];
    if(child.kind == CppAstKind::parameter_pack) {
      out += "...";
    } else if(child.kind == CppAstKind::ptr_operator) {
      out += (out.empty() ? string() : string(" ")) + child.value;
    } else {
      const string suffix = render_structured_type(child,
                                                   type_name_replacements);
      if(!suffix.empty()) {
        out += (out.empty() ? string() : string(" ")) + suffix;
      }
    }
  }
  return out;
}

string apply_structured_declarator(
    string base,
    const CppAstNode & declarator,
    const StructuredTypeNameReplacements * type_name_replacements)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    switch(child.kind) {
    case CppAstKind::ptr_operator:
      if(!base.empty() && base[base.size() - 1] != '*' &&
         base[base.size() - 1] != '&') {
        base += " ";
      }
      base += child.value;
      break;
    case CppAstKind::cv_qualifier:
      if(!base.empty() && base[base.size() - 1] != '*') {
        base += " ";
      }
      base += child.value;
      break;
    case CppAstKind::parameter_pack:
    case CppAstKind::ellipsis:
      base += "...";
      break;
    case CppAstKind::parameter_clause:
    {
      ostringstream parameters;
      parameters << "(";
      for(size_t j = 0; j < child.children.size(); ++j) {
        if(j != 0) {
          parameters << ", ";
        }
        parameters << render_structured_parameter_declaration(
            child.children[j], type_name_replacements);
      }
      parameters << ")";
      if(!base.empty()) {
        base += " ";
      }
      base += parameters.str();
      break;
    }
    case CppAstKind::array_suffix:
      base += render_structured_type(child, type_name_replacements);
      break;
    case CppAstKind::nested_declarator:
      if(!child.children.empty()) {
        base = "(" + apply_structured_declarator(
            base, child.children[0], type_name_replacements) + ")";
      }
      break;
    default:
    {
      const string part = render_structured_type(child,
                                                 type_name_replacements);
      if(!part.empty()) {
        if(!base.empty()) {
          base += " ";
        }
        base += part;
      }
      break;
    }
    }
  }
  return base;
}

string render_structured_type(
    const CppAstNode & node,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool final_template_disambiguator)
{
  switch(node.kind) {
  case CppAstKind::type_id:
  {
    string base;
    const CppAstNode * declarator = nullptr;
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::type_specifier_seq ||
         node.children[i].kind == CppAstKind::decl_specifier_seq) {
        base = render_structured_type_sequence(node.children[i],
                                               type_name_replacements,
                                               final_template_disambiguator);
      } else if(node.children[i].kind == CppAstKind::declarator ||
                node.children[i].kind == CppAstKind::abstract_declarator) {
        declarator = &node.children[i];
      }
    }
    return declarator ?
        apply_structured_declarator(base,
                                    *declarator,
                                    type_name_replacements) :
        base;
  }
  case CppAstKind::type_specifier_seq:
  case CppAstKind::decl_specifier_seq:
    return render_structured_type_sequence(node,
                                           type_name_replacements,
                                           final_template_disambiguator);
  case CppAstKind::type_specifier:
  case CppAstKind::decl_specifier:
  case CppAstKind::type_name:
  case CppAstKind::base_name:
  case CppAstKind::identifier:
    return render_structured_node_name(node,
                                       type_name_replacements,
                                       true,
                                       final_template_disambiguator);
  case CppAstKind::class_forward_declaration:
  {
    string key;
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::class_key) {
        key = node.children[i].value;
        break;
      }
    }
    const string name = render_structured_node_name(
        node,
        type_name_replacements,
        true,
        final_template_disambiguator);
    return key.empty() ? name : key + " " + name;
  }
  case CppAstKind::decltype_specifier:
    if(node.children.size() == 1) {
      return string(node.is_typeof_specifier ? "__typeof__(" : "decltype(") +
          render_structured_expression(node.children[0],
                                       true,
                                       type_name_replacements) + ")";
    }
    return render_structured_node_name(node,
                                       type_name_replacements,
                                       true);
  case CppAstKind::parameter_declaration:
    return render_structured_parameter_declaration(node,
                                                   type_name_replacements);
  case CppAstKind::parameter_clause:
  {
    ostringstream out;
    out << "(";
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << render_structured_parameter_declaration(
          node.children[i], type_name_replacements);
    }
    out << ")";
    return out.str();
  }
  case CppAstKind::ptr_operator:
  case CppAstKind::cv_qualifier:
    return node.value;
  case CppAstKind::parameter_pack:
  case CppAstKind::ellipsis:
    return "...";
  case CppAstKind::array_suffix:
    if(node.children.size() == 1) {
      return "[" + render_structured_expression(node.children[0],
                                                 true,
                                                 type_name_replacements) + "]";
    }
    return node.value.empty() ? "[]" : node.value;
  case CppAstKind::abstract_declarator:
  case CppAstKind::declarator:
    return apply_structured_declarator(string(),
                                       node,
                                       type_name_replacements);
  default:
    if(node.template_id_syntax || node.qualified_name_syntax ||
       !node.value.empty()) {
      return render_structured_node_name(node,
                                         type_name_replacements,
                                         true);
    }
    return string();
  }
}

string render_structured_template_argument(
    const TemplateArgumentSyntax & syntax,
    const StructuredTypeNameReplacements * type_name_replacements,
    bool preserve_source_template_disambiguator)
{
  string out;
  const bool final_template_disambiguator =
      preserve_source_template_disambiguator &&
      syntax.name_has_template_disambiguator;
  // A directly parsed template-id is the complete argument spelling.  Prefer
  // it to the accompanying type fragment so nested template arguments and
  // qualifier-template syntax are retained by the structured printer.
  if(syntax.template_id) {
    out = render_structured_template_id_with_prefix_drop(
        *syntax.template_id,
        0,
        type_name_replacements,
        preserve_source_template_disambiguator);
  } else if(syntax.type_id &&
            syntax.expression &&
            structured_expression_contains_qualified_id(*syntax.expression)) {
    // When the fragment parser retains both parses, the expression grammar
    // wins unless semantic template-id resolution has already selected the
    // direct template-id case above.  This covers dependent functional casts
    // such as `bool(Bn::value)`.  Keep a function type such as
    // `void(Tail...)` on the type branch; its expression alternative has no
    // qualified value-id.
    out = render_structured_expression(*syntax.expression,
                                       true,
                                       type_name_replacements,
                                       final_template_disambiguator);
  } else if(syntax.type_id) {
    out = render_structured_type(*syntax.type_id,
                                 type_name_replacements,
                                 final_template_disambiguator);
  } else if(syntax.expression) {
    out = render_structured_expression(*syntax.expression,
                                       true,
                                       type_name_replacements,
                                       final_template_disambiguator);
  } else {
    out = trim_space(syntax.text);
  }
  if(syntax.pack_expansion &&
     (out.size() < 3 || out.compare(out.size() - 3, 3, "...") != 0)) {
    out += "...";
  }
  return out;
}

string render_structured_expression(const CppAstNode & node,
                                    bool template_argument_policy,
                                    const StructuredTypeNameReplacements *
                                        type_name_replacements,
                                    bool final_template_disambiguator)
{
  if(node.kind == CppAstKind::cast_expression &&
     node.children.size() == 2) {
    const string type = render_structured_type(node.children[0],
                                               type_name_replacements);
    const string operand = render_structured_expression(
        node.children[1], template_argument_policy, type_name_replacements);
    if(!node.value.empty()) {
      return node.value + "<" + type + ">(" + operand + ")";
    }
    return "(" + type + ")" + operand;
  }
  if(node.kind == CppAstKind::pack_expansion_expression &&
     node.children.size() == 1) {
    return render_structured_expression(node.children[0],
                                        template_argument_policy,
                                        type_name_replacements) +
           "...";
  }
  if(node.kind == CppAstKind::decltype_specifier &&
     node.children.size() == 1) {
    return string(node.is_typeof_specifier ? "__typeof__(" : "decltype(") +
           render_structured_expression(node.children[0],
                                        template_argument_policy,
                                        type_name_replacements) +
           ")";
  }
  if(node.kind == CppAstKind::braced_init_list) {
    ostringstream out;
    out << "{";
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << render_structured_expression(node.children[i],
                                          template_argument_policy,
                                          type_name_replacements);
    }
    out << "}";
    return out.str();
  }
  if(!node.children.empty()) {
    if(node.kind == CppAstKind::unary_expression && node.children.size() == 1) {
      return node.value +
          render_structured_expression(node.children[0],
                                       template_argument_policy,
                                       type_name_replacements);
    }
    if(node.kind == CppAstKind::binary_expression && node.children.size() == 2) {
      return render_structured_expression(node.children[0],
                                          template_argument_policy,
                                          type_name_replacements) +
             " " + node.value + " " +
             render_structured_expression(node.children[1],
                                          template_argument_policy,
                                          type_name_replacements);
    }
    if(node.kind == CppAstKind::conditional_expression &&
       node.children.size() == 3) {
      return render_structured_expression(node.children[0],
                                          template_argument_policy,
                                          type_name_replacements) +
             " ? " +
             render_structured_expression(node.children[1],
                                          template_argument_policy,
                                          type_name_replacements) +
             " : " +
             render_structured_expression(node.children[2],
                                          template_argument_policy,
                                          type_name_replacements);
    }
    if(node.kind == CppAstKind::call_expression && node.children.size() == 2 &&
       (node.children[1].kind == CppAstKind::argument_list ||
        node.children[1].kind == CppAstKind::paren_argument_list)) {
      if(node.children[1].kind == CppAstKind::argument_list &&
         node.children[1].children.size() == 1 &&
         node.children[1].children[0].kind == CppAstKind::braced_init_list) {
        return render_structured_expression(node.children[0],
                                            template_argument_policy,
                                            type_name_replacements) +
               render_structured_expression(node.children[1].children[0],
                                            template_argument_policy,
                                            type_name_replacements);
      }
      ostringstream out;
      out << render_structured_expression(node.children[0],
                                          template_argument_policy,
                                          type_name_replacements)
          << "(";
      for(size_t i = 0; i < node.children[1].children.size(); ++i) {
        if(i != 0) {
          out << ", ";
        }
        out << render_structured_expression(node.children[1].children[i],
                                            template_argument_policy,
                                            type_name_replacements);
      }
      out << ")";
      return out.str();
    }
    if(node.kind == CppAstKind::parenthesized_expression && node.children.size() == 1) {
      return "(" + render_structured_expression(node.children[0],
                                                 template_argument_policy,
                                                 type_name_replacements) +
             ")";
    }
    if(node.kind == CppAstKind::fold_expression) {
      if(node.children.size() == 2) {
        if(node.children[0].kind == CppAstKind::ellipsis) {
          return "(... " + node.value + " " +
                 render_structured_expression(node.children[1],
                                              template_argument_policy,
                                              type_name_replacements) +
                 ")";
        }
        if(node.children[1].kind == CppAstKind::ellipsis) {
          return "(" +
                 render_structured_expression(node.children[0],
                                              template_argument_policy,
                                              type_name_replacements) +
                 " " +
                 node.value + " ...)";
        }
      }
      if(node.children.size() == 3 && node.children[1].kind == CppAstKind::ellipsis) {
        return "(" +
               render_structured_expression(node.children[0],
                                            template_argument_policy,
                                            type_name_replacements) +
               " " +
               node.value + " ... " + node.value + " " +
               render_structured_expression(node.children[2],
                                            template_argument_policy,
                                            type_name_replacements) +
               ")";
      }
    }
    if(node.kind == CppAstKind::member_expression && node.children.size() == 2) {
      return render_structured_expression(node.children[0],
                                          template_argument_policy,
                                          type_name_replacements) +
             node.value +
             render_structured_expression(node.children[1],
                                          template_argument_policy,
                                          type_name_replacements);
    }
    if(node.kind == CppAstKind::type_trait_expression && !node.children.empty()) {
      ostringstream out;
      out << node.value << "(";
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(i != 0) {
          out << ", ";
        }
        out << render_structured_expression(node.children[i],
                                            template_argument_policy,
                                            type_name_replacements);
      }
      out << ")";
      return out.str();
    }
    if(node.kind == CppAstKind::sizeof_pack_expression &&
       node.children.size() == 1) {
      return "sizeof...(" +
             render_structured_expression(node.children[0],
                                          template_argument_policy,
                                          type_name_replacements) +
             ")";
    }
    if(node.kind == CppAstKind::sizeof_expression &&
       node.children.size() == 1) {
      const bool type_operand = node.children[0].kind == CppAstKind::type_id;
      return string(template_argument_policy && !type_operand ?
                        "sizeof (" : "sizeof(") +
             (type_operand ?
                  render_structured_type(node.children[0],
                                         type_name_replacements) :
                  render_structured_expression(node.children[0],
                                               template_argument_policy,
                                               type_name_replacements)) +
             ")";
    }
    if(node.kind == CppAstKind::type_id) {
      return render_structured_type(node, type_name_replacements);
    }
  }

  if(node.template_id_syntax || node.qualified_name_syntax) {
    return render_structured_node_name(node,
                                       type_name_replacements,
                                       false,
                                       final_template_disambiguator);
  }

  if(!node.value.empty()) {
    return node.value;
  }
  return node_text(node);
}

}  // namespace

string describe_expression_for_diagnostic(const CppAstNode & node)
{
  return render_structured_expression(node, false, nullptr);
}

string render_template_argument_expression(const CppAstNode & node)
{
  return render_structured_expression(node, true, nullptr);
}

string render_template_argument_expression(
    const CppAstNode & node,
    const map<string, string> & type_name_replacements)
{
  return render_structured_expression(node,
                                      true,
                                      &type_name_replacements);
}

string render_template_argument_syntax(
    const TemplateArgumentSyntax & syntax)
{
  return render_structured_template_argument(syntax, nullptr);
}

string render_template_argument_syntax(
    const TemplateArgumentSyntax & syntax,
    const map<string, string> & type_name_replacements)
{
  return render_structured_template_argument(syntax,
                                             &type_name_replacements);
}

string render_template_argument_syntax(
    const TemplateArgumentSyntax & syntax,
    const map<string, string> & type_name_replacements,
    bool preserve_source_template_disambiguator)
{
  return render_structured_template_argument(
      syntax,
      &type_name_replacements,
      preserve_source_template_disambiguator);
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
