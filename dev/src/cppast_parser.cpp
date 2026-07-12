#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "builtin_type_transforms.h"
#include "cpp_syntax.h"
#include "cppast_parser.h"
#include "file_timing.h"
#include "parser_trace.h"
#include "qualified_name_parser.h"
#include "template_angle_parser.h"
#include "types.h"

namespace {

struct RecogTokenSuffixSequence : IRecogTokenSequence
{
  explicit RecogTokenSuffixSequence(const IRecogTokenSequence & tokens,
                                    size_t start) :
    tokens(tokens),
    start(start)
  {}

  virtual const RecogToken & peek(size_t index) const
  {
    return tokens.peek(start + index);
  }

  virtual const RecogToken & operator[](size_t index) const
  {
    return peek(index);
  }

  virtual size_t size() const
  {
    size_t i = 0;
    for(;; ++i) {
      if(tokens.peek(start + i).is_eof()) {
        return i + 1;
      }
    }
  }

  virtual const RecogToken & back() const
  {
    return tokens.peek(start + size() - 1);
  }

  virtual vector<RecogToken> slice(size_t slice_start, size_t slice_end) const
  {
    if(slice_end < slice_start) {
      throw logic_error("invalid token slice");
    }

    vector<RecogToken> out;
    out.reserve(slice_end - slice_start);
    for(size_t i = slice_start; i < slice_end; ++i) {
      out.push_back(peek(i));
    }
    return out;
  }

  virtual string span_text(size_t span_start, size_t span_end) const
  {
    if(span_end < span_start) {
      throw logic_error("invalid token span");
    }

    string text;
    text.reserve((span_end - span_start) * 8);
    for(size_t i = span_start; i < span_end; ++i) {
      const RecogToken & token = peek(i);
      text += token.is_rshift_piece() ? ">" : token.source;
    }
    return text;
  }

  virtual const SourceLocationTable * source_locations() const
  {
    return tokens.source_locations();
  }

  const IRecogTokenSequence & tokens;
  size_t start;
};

bool is_bare_parameter_pack_abstract_declarator(const CppAstNode & node)
{
  return (node.kind == CppAstKind::declarator ||
          node.kind == CppAstKind::abstract_declarator) &&
         node.children.size() == 1 &&
         node.children[0].kind == CppAstKind::parameter_pack;
}

struct RecogTokenRangeSequence : IRecogTokenSequence
{
  RecogTokenRangeSequence(const IRecogTokenSequence & tokens,
                          size_t start,
                          size_t end) :
    tokens(tokens),
    start(start),
    end(end),
    eof_token(RT_EOF, string(), static_cast<ETokenType>(0), 0)
  {
    if(end < start) {
      throw logic_error("invalid token range");
    }
  }

  virtual const RecogToken & peek(size_t index) const
  {
    if(index < token_count()) {
      return tokens.peek(start + index);
    }
    return eof_token;
  }

  virtual const RecogToken & operator[](size_t index) const
  {
    return peek(index);
  }

  virtual size_t size() const
  {
    return token_count() + 1;
  }

  virtual const RecogToken & back() const
  {
    return eof_token;
  }

  virtual vector<RecogToken> slice(size_t slice_start, size_t slice_end) const
  {
    if(slice_end < slice_start || slice_end > size()) {
      throw logic_error("invalid token slice");
    }

    vector<RecogToken> out;
    out.reserve(slice_end - slice_start);
    for(size_t i = slice_start; i < slice_end; ++i) {
      out.push_back(peek(i));
    }
    return out;
  }

  virtual string span_text(size_t span_start, size_t span_end) const
  {
    if(span_end < span_start || span_end > size()) {
      throw logic_error("invalid token span");
    }

    string text;
    text.reserve((span_end - span_start) * 8);
    for(size_t i = span_start; i < span_end; ++i) {
      const RecogToken & token = peek(i);
      if(token.is_eof()) {
        break;
      }
      text += token.is_rshift_piece() ? ">" : token.source;
    }
    return text;
  }

  virtual const SourceLocationTable * source_locations() const
  {
    return tokens.source_locations();
  }

  virtual const string & primary_source_file() const
  {
    return tokens.primary_source_file();
  }

private:
  size_t token_count() const
  {
    return end - start;
  }

  const IRecogTokenSequence & tokens;
  size_t start;
  size_t end;
  RecogToken eof_token;
};

CppAstNode make_node(CppAstKind kind, const string & value = string())
{
  CppAstNode node;
  node.kind = kind;
  node.value = value;
  return node;
}

CppAstNode make_token_node(CppAstKind kind, const RecogToken & token)
{
  CppAstNode node = make_node(kind, token.source);
  node.has_token = true;
  node.token_kind = token.kind;
  node.simple_type = token.simple_type;
  node.source_location_id = token.location_id;
  return node;
}

void set_node_token(CppAstNode & node, const RecogToken & token)
{
  node.has_token = true;
  node.token_kind = token.kind;
  node.simple_type = token.simple_type;
  node.source_location_id = token.location_id;
}

void set_node_simple_type(CppAstNode & node, ETokenType type)
{
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = type;
}

cpp_decl::QualifiedName build_qualified_name_syntax(
    const IRecogTokenSequence & tokens,
    const qualified_name_parser::QualifiedNameParseResult & parsed)
{
  cpp_decl::QualifiedName out;
  out.rooted = parsed.rooted;
  for(size_t i = 0; i < parsed.qualifiers.size(); ++i) {
    out.qualifiers.push_back(
        template_angle::token_span_text_spaced(tokens,
                                               parsed.qualifiers[i].first,
                                               parsed.qualifiers[i].second));
  }
  if(parsed.name_kind == qualified_name_parser::UNQ_COMPONENT) {
    const size_t name_begin =
        parsed.name_template_head_component.second >
                parsed.name_template_head_component.first ?
            parsed.name_template_head_component.first :
            parsed.name_component.first;
    out.name = template_angle::token_span_text_spaced(tokens,
                                                      name_begin,
                                                      parsed.name_component.second);
  } else {
    out.name = tokens.span_text(parsed.name_component.first, parsed.name_component.second);
  }
  return out;
}

bool build_template_id_syntax(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const qualified_name_parser::QualifiedNameParseResult & parsed,
    cpp_decl::TemplateIdSyntax & out,
    CppAstParser * parser_context);
bool build_template_id_syntax_from_range(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    cpp_decl::TemplateIdSyntax & out,
    CppAstParser * parser_context);
bool qualified_name_has_qualifier_template_id(
    const qualified_name_parser::QualifiedNameParseResult & parsed);
void build_qualifier_template_id_syntaxes(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const qualified_name_parser::QualifiedNameParseResult & parsed,
    std::vector<cpp_decl::TemplateIdSyntax> & out,
    CppAstParser * parser_context);
bool build_qualified_id_expression_syntax_from_range(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    CppAstNode & out,
    CppAstParser * parser_context);
bool is_builtin_type_trait_expression_start(const IRecogTokenSequence & tokens,
                                            size_t pos);
bool is_alignof_type_trait_identifier(const RecogToken & token);

bool token_is_decltype_or_typeof_specifier_start(const RecogToken & token)
{
  return token.is_simple(KW_DECLTYPE) ||
         (token.is_identifier() &&
          (token.source == "__decltype" ||
           token.source == "__decltype__" ||
           token.source == "__typeof" ||
           token.source == "__typeof__"));
}

bool expected_close_for_open_token(const RecogToken & token, ETokenType & close)
{
  if(token.is_simple(OP_LPAREN)) {
    close = OP_RPAREN;
    return true;
  }
  if(token.is_simple(OP_LSQUARE)) {
    close = OP_RSQUARE;
    return true;
  }
  if(token.is_simple(OP_LBRACE)) {
    close = OP_RBRACE;
    return true;
  }
  return false;
}

struct BalancedCloseStack
{
  static const size_t inline_capacity = 64;

  BalancedCloseStack() : count(0) {}

  void push(ETokenType value)
  {
    if(count < inline_capacity) {
      inline_values[count] = value;
    } else {
      overflow.push_back(value);
    }
    ++count;
  }

  void pop()
  {
    if(count == 0) {
      return;
    }
    if(count > inline_capacity) {
      overflow.pop_back();
    }
    --count;
  }

  bool empty() const
  {
    return count == 0;
  }

  ETokenType back() const
  {
    if(count <= inline_capacity) {
      return inline_values[count - 1];
    }
    return overflow[count - inline_capacity - 1];
  }

  ETokenType inline_values[inline_capacity];
  vector<ETokenType> overflow;
  size_t count;
};

bool skip_balanced_group_in_range(IRecogTokenSequence & tokens,
                                  size_t start,
                                  size_t end,
                                  size_t & next)
{
  ETokenType close = static_cast<ETokenType>(0);
  if(start >= end || !expected_close_for_open_token(tokens.peek(start), close)) {
    return false;
  }

  BalancedCloseStack expected_closes;
  expected_closes.push(close);
  for(size_t i = start + 1; i < end; ++i) {
    ETokenType nested_close = static_cast<ETokenType>(0);
    if(expected_close_for_open_token(tokens.peek(i), nested_close)) {
      expected_closes.push(nested_close);
      continue;
    }
    if(!expected_closes.empty() && tokens.peek(i).is_simple(expected_closes.back())) {
      expected_closes.pop();
      if(expected_closes.empty()) {
        next = i + 1;
        return true;
      }
      continue;
    }
    if(tokens.peek(i).is_simple(OP_RPAREN) ||
       tokens.peek(i).is_simple(OP_RSQUARE) ||
       tokens.peek(i).is_simple(OP_RBRACE)) {
      return false;
    }
  }
  return false;
}

bool skip_balanced_group_from(IRecogTokenSequence & tokens,
                              size_t start,
                              size_t & next)
{
  ETokenType close = static_cast<ETokenType>(0);
  if(!expected_close_for_open_token(tokens.peek(start), close)) {
    return false;
  }

  BalancedCloseStack expected_closes;
  expected_closes.push(close);
  for(size_t i = start + 1;; ++i) {
    const RecogToken & token = tokens.peek(i);
    if(token.is_eof()) {
      return false;
    }
    ETokenType nested_close = static_cast<ETokenType>(0);
    if(expected_close_for_open_token(token, nested_close)) {
      expected_closes.push(nested_close);
      continue;
    }
    if(!expected_closes.empty() && token.is_simple(expected_closes.back())) {
      expected_closes.pop();
      if(expected_closes.empty()) {
        next = i + 1;
        return true;
      }
      continue;
    }
    if(token.is_simple(OP_RPAREN) ||
       token.is_simple(OP_RSQUARE) ||
       token.is_simple(OP_RBRACE)) {
      return false;
    }
  }
}

bool env_flag_enabled(const char * name)
{
  const char * value = std::getenv(name);
  return value != nullptr && value[0] != '\0' &&
         !(value[0] == '0' && value[1] == '\0') &&
         !(value[0] == 'n' && value[1] == 'o' && value[2] == '\0') &&
         !(value[0] == 'N' && value[1] == 'O' && value[2] == '\0');
}

bool lazy_header_function_bodies_enabled()
{
  return true;
}

bool lazy_body_stats_enabled()
{
  static const bool enabled = env_flag_enabled("CPPGM_LAZY_BODY_STATS");
  return enabled;
}

struct LazyHeaderFunctionBodyStats
{
  size_t eager_function_bodies = 0;
  size_t skipped_header_bodies = 0;
  size_t skipped_header_body_tokens = 0;
  size_t skip_failures = 0;

  ~LazyHeaderFunctionBodyStats()
  {
    if(!lazy_body_stats_enabled()) {
      return;
    }
    cerr << "cppgm lazy-body stats:"
         << " eager_function_bodies=" << eager_function_bodies
         << " skipped_header_bodies=" << skipped_header_bodies
         << " skipped_header_body_tokens=" << skipped_header_body_tokens
         << " skip_failures=" << skip_failures
         << '\n';
  }
};

LazyHeaderFunctionBodyStats & lazy_body_stats()
{
  static LazyHeaderFunctionBodyStats stats;
  return stats;
}

string format_name_set_for_trace(const template_angle_lookup::NameSet & names)
{
  vector<string> values;
  for(template_angle_lookup::NameSet::const_iterator it = names.begin();
      it != names.end();
      ++it) {
    if(*it) {
      values.push_back(**it);
    }
  }
  sort(values.begin(), values.end());

  ostringstream out;
  out << '{';
  const size_t limit = 16;
  for(size_t i = 0; i < values.size() && i < limit; ++i) {
    if(i != 0) {
      out << ',';
    }
    out << values[i];
  }
  if(values.size() > limit) {
    out << ",...";
  }
  out << '}';
  return out.str();
}

bool token_is_from_non_primary_source_file(const IRecogTokenSequence & tokens,
                                           size_t index)
{
  const SourceLocationTable * table = tokens.source_locations();
  if(table == nullptr) {
    return false;
  }

  const uint32_t location_id = tokens.peek(index).location_id;
  if(location_id == 0 || location_id >= table->locations.size()) {
    return false;
  }

  const SourceLocation & location = table->locations[location_id];
  const string & primary_source_file = tokens.primary_source_file();
  if(!primary_source_file.empty() &&
     location.file_index < table->files.size()) {
    return table->files[location.file_index] != primary_source_file;
  }
  return location.file_index > 1;
}

bool token_forces_template_argument_expression_syntax(const RecogToken & token)
{
  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_DOTS:
  case OP_DOT:
  case OP_DOTSTAR:
  case OP_ARROW:
  case OP_ARROWSTAR:
  case OP_PLUS:
  case OP_MINUS:
  case OP_STAR:
  case OP_DIV:
  case OP_MOD:
  case OP_XOR:
  case OP_AMP:
  case OP_BOR:
  case OP_ASS:
  case OP_LT:
  case OP_GT:
  case OP_PLUSASS:
  case OP_MINUSASS:
  case OP_STARASS:
  case OP_DIVASS:
  case OP_MODASS:
  case OP_XORASS:
  case OP_BANDASS:
  case OP_BORASS:
  case OP_LSHIFT:
  case OP_RSHIFT:
  case OP_RSHIFTASS:
  case OP_LSHIFTASS:
  case OP_EQ:
  case OP_NE:
  case OP_LE:
  case OP_GE:
  case OP_LAND:
  case OP_LOR:
  case OP_INC:
  case OP_DEC:
  case OP_QMARK:
    return true;
  default:
    return false;
  }
}

bool template_argument_range_fragment_mode(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    const CppAstParser * parser_context,
    CppAstParser::TemplateArgumentFragmentMode & mode)
{
  if(range.second <= range.first) {
    return false;
  }

  std::size_t effective_end = range.second;
  if(effective_end > range.first && tokens.peek(effective_end - 1).is_simple(OP_DOTS)) {
    --effective_end;
  }
  const auto has_postfix_initializer_after_qualified_id = [&]() -> bool
  {
    qualified_name_parser::QualifiedNameParseResult parsed_name;
    if(!qualified_name_parser::parse_qualified_name(
           tokens,
           range.first,
           lookup,
           qualified_name_parser::UnqualifiedNameOptions(),
           parsed_name) ||
       parsed_name.end >= effective_end ||
       (!tokens.peek(parsed_name.end).is_simple(OP_LPAREN) &&
        !tokens.peek(parsed_name.end).is_simple(OP_LBRACE))) {
      return false;
    }
    size_t call_end = parsed_name.end;
    return skip_balanced_group_in_range(tokens,
                                        parsed_name.end,
                                        effective_end,
                                        call_end) &&
           call_end == effective_end;
  };
  const auto has_template_id_qualifier = [&]() -> bool
  {
    qualified_name_parser::QualifiedNameParseResult parsed_name;
    if(!qualified_name_parser::parse_qualified_name(
           tokens,
           range.first,
           lookup,
           qualified_name_parser::UnqualifiedNameOptions(),
           parsed_name) ||
       parsed_name.end != effective_end) {
      return false;
    }
    for(size_t i = 0; i < parsed_name.qualifier_components.size(); ++i) {
      if(parsed_name.qualifier_components[i].has_template_suffix) {
        return true;
      }
    }
    return false;
  };
  const auto has_template_id_prefix_with_cv_tail = [&]() -> bool
  {
    for(size_t open = range.first; open < effective_end; ++open) {
      if(!tokens.peek(open).is_simple(OP_LT)) {
        continue;
      }

      size_t suffix_end = open;
      vector<pair<size_t, size_t> > arg_ranges;
      if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                          open,
                                                          lookup,
                                                          suffix_end,
                                                          arg_ranges) ||
         suffix_end >= effective_end) {
        continue;
      }

      bool saw_cv = false;
      size_t tail = suffix_end;
      while(tail < effective_end && is_cv_qualifier(tokens.peek(tail))) {
        saw_cv = true;
        ++tail;
      }
      if(!saw_cv || tail != effective_end) {
        continue;
      }

      RecogTokenRangeSequence head_tokens(tokens, range.first, open);
      qualified_name_parser::QualifiedNameParseResult head_parsed;
      if(qualified_name_parser::parse_qualified_name(
             head_tokens,
             0,
             lookup,
             qualified_name_parser::UnqualifiedNameOptions(),
             head_parsed) &&
         head_parsed.end == open - range.first) {
        return true;
      }
    }
    return false;
  };
  const auto has_function_style_type_cast = [&]() -> bool
  {
    size_t pos = range.first;
    bool saw_type = false;
    while(pos < effective_end) {
      const RecogToken & token = tokens.peek(pos);
      if(is_cv_qualifier(token)) {
        ++pos;
        continue;
      }
      if(is_simple_type_specifier(token)) {
        saw_type = true;
        ++pos;
        continue;
      }
      break;
    }
    if(!saw_type ||
       pos >= effective_end ||
       (!tokens.peek(pos).is_simple(OP_LPAREN) &&
        !tokens.peek(pos).is_simple(OP_LBRACE))) {
      return false;
    }
    size_t call_end = pos;
    if(!skip_balanced_group_in_range(tokens, pos, effective_end, call_end)) {
      return false;
    }
    return call_end == effective_end ||
           (call_end < effective_end &&
            token_forces_template_argument_expression_syntax(tokens.peek(call_end)));
  };

  const RecogToken & first = tokens.peek(range.first);
  if(is_cv_qualifier(first) ||
         is_simple_type_specifier(first) ||
         is_class_key(first) ||
         first.is_simple(KW_ENUM) ||
         token_is_decltype_or_typeof_specifier_start(first) ||
         first.is_simple(KW_TYPENAME)) {
    mode = has_function_style_type_cast() ?
        CppAstParser::TAF_PARSE_BOTH :
        CppAstParser::TAF_PARSE_TYPE_ONLY;
    return true;
  }
  if(has_postfix_initializer_after_qualified_id()) {
    mode = CppAstParser::TAF_PARSE_BOTH;
    return true;
  }
  if(has_template_id_qualifier()) {
    mode = CppAstParser::TAF_PARSE_BOTH;
    return true;
  }
  if(has_template_id_prefix_with_cv_tail()) {
    mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
    return true;
  }
  if(first.is_identifier() &&
     (lookup.is_known_type_name_identifier(first) ||
      (parser_context &&
       parser_context->is_template_type_parameter_name(first)) ||
      lookup.is_known_template_name_identifier(first)) &&
     !lookup.is_known_value_template_parameter_identifier(first) &&
     !lookup.is_known_value_name_identifier(first)) {
    mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
    return true;
  }

  if(first.is_literal() ||
         first.is_simple(KW_TRUE) ||
         first.is_simple(KW_FALSE) ||
         first.is_simple(KW_NULLPTR) ||
         first.is_simple(KW_THIS) ||
         first.is_simple(KW_SIZEOF) ||
         first.is_simple(KW_ALIGNOF) ||
         first.is_simple(KW_NOEXCEPT) ||
         first.is_simple(OP_LPAREN) ||
         first.is_simple(OP_PLUS) ||
         first.is_simple(OP_MINUS) ||
         first.is_simple(OP_LNOT) ||
         is_alignof_type_trait_identifier(first) ||
         is_builtin_type_trait_expression_start(tokens, range.first)) {
    mode = CppAstParser::TAF_PARSE_EXPRESSION_ONLY;
    return true;
  }

  qualified_name_parser::QualifiedNameParseResult parsed_name;
  if(qualified_name_parser::parse_qualified_name(
         tokens,
         range.first,
         lookup,
         qualified_name_parser::UnqualifiedNameOptions(),
         parsed_name) &&
     parsed_name.end <= effective_end) {
    if(!parsed_name.name_has_template_suffix) {
      if(parsed_name.end == effective_end) {
        mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
        return true;
      }
      const RecogToken & next = tokens.peek(parsed_name.end);
      if(next.is_simple(OP_STAR) ||
         next.is_simple(OP_AMP) ||
         next.is_simple(OP_LAND) ||
         next.is_simple(OP_LPAREN) ||
         next.is_simple(OP_DOTS) ||
         is_cv_qualifier(next)) {
        mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
        return true;
      }
      if(next.is_simple(OP_LSQUARE)) {
        mode = CppAstParser::TAF_PARSE_BOTH;
        return true;
      }
    }
    if(parsed_name.name_has_template_suffix &&
       parsed_name.name_template_head_component.first <
           parsed_name.name_template_head_component.second) {
    if(parsed_name.end == effective_end) {
      const RecogToken & head =
          tokens.peek(parsed_name.name_template_head_component.first);
      const bool known_value =
          lookup.is_known_value_template_parameter_identifier(head) ||
          lookup.is_known_value_name_identifier(head);
      const bool known_type =
          lookup.is_known_template_name_identifier(head) ||
          lookup.is_known_type_name_identifier(head) ||
          (parser_context &&
           parser_context->is_template_type_parameter_name(head));
      mode = (known_value && !known_type) ?
          CppAstParser::TAF_PARSE_EXPRESSION_ONLY :
          CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
      return true;
    }
    const RecogToken & next = tokens.peek(parsed_name.end);
    if(next.is_simple(OP_STAR) ||
       next.is_simple(OP_AMP) ||
       next.is_simple(OP_LAND) ||
       next.is_simple(OP_DOTS) ||
       next.is_simple(OP_LPAREN) ||
       next.is_simple(OP_LBRACE) ||
       is_cv_qualifier(next)) {
      mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
      return true;
    }
    if(next.is_simple(OP_LSQUARE)) {
      mode = CppAstParser::TAF_PARSE_BOTH;
      return true;
    }
    }
  }

  for(std::size_t i = range.first; i < range.second;) {
    size_t skip_end = i;
    if(skip_balanced_group_in_range(tokens, i, range.second, skip_end)) {
      i = skip_end;
      continue;
    }

    if(tokens.peek(i).is_identifier() &&
       i + 1 < range.second &&
       tokens.peek(i + 1).is_simple(OP_LT)) {
      size_t template_suffix_end = i + 1;
      std::vector<std::pair<size_t, size_t> > nested_arg_ranges;
      if(template_angle::parse_template_id_suffix_ranges(tokens,
                                                         i + 1,
                                                         lookup,
                                                         template_suffix_end,
                                                         nested_arg_ranges) &&
         template_suffix_end <= range.second) {
        i = template_suffix_end;
        continue;
      }
    }

    const RecogToken & token = tokens.peek(i);
    if(!token_forces_template_argument_expression_syntax(token)) {
      ++i;
      continue;
    }
    if(token.is_simple(OP_STAR) ||
       token.is_simple(OP_AMP) ||
       token.is_simple(OP_DOTS)) {
      mode = CppAstParser::TAF_PARSE_TYPE_THEN_EXPRESSION;
    } else {
      mode = CppAstParser::TAF_PARSE_EXPRESSION_ONLY;
    }
    return true;
  }
  return false;
}

void attach_template_id_syntax_to_direct_type_id(
    CppAstNode & type_id,
    const cpp_decl::TemplateIdSyntax & template_id)
{
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.empty() ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq) {
    return;
  }

  CppAstNode & specifiers = type_id.children[0];
  CppAstNode * type_name = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    CppAstNode & child = specifiers.children[i];
    if(child.kind == CppAstKind::type_name) {
      if(type_name) {
        return;
      }
      type_name = &child;
      continue;
    }
    if(child.kind != CppAstKind::cv_qualifier) {
      return;
    }
  }
  if(!type_name) {
    return;
  }

  set_cppast_template_id_syntax(*type_name, template_id);
}

void attach_qualifier_template_id_syntaxes_to_direct_type_id(
    CppAstNode & type_id,
    const std::vector<cpp_decl::TemplateIdSyntax> & qualifier_template_ids)
{
  if(qualifier_template_ids.empty() ||
     type_id.kind != CppAstKind::type_id ||
     type_id.children.empty() ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq) {
    return;
  }

  CppAstNode & specifiers = type_id.children[0];
  CppAstNode * type_name = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    CppAstNode & child = specifiers.children[i];
    if(child.kind == CppAstKind::type_name) {
      if(type_name) {
        return;
      }
      type_name = &child;
      continue;
    }
    if(child.kind != CppAstKind::cv_qualifier) {
      return;
    }
  }
  if(!type_name) {
    return;
  }

  set_cppast_qualifier_template_id_syntaxes(*type_name,
                                            qualifier_template_ids);
}

CppAstNode make_template_id_expression_syntax_from_range(
    IRecogTokenSequence & tokens,
    const std::pair<std::size_t, std::size_t> & range,
    const std::shared_ptr<cpp_decl::TemplateIdSyntax> & template_id,
    const std::vector<cpp_decl::TemplateIdSyntax> & qualifier_template_ids,
    const std::string & text)
{
  CppAstNode expression = make_node(CppAstKind::id_expression, text);
  expression.token_start = range.first;
  expression.token_end = range.second;
  expression.source_location_id = tokens[range.first].location_id;
  if(template_id) {
    expression.qualified_name_syntax.reset(
        new cpp_decl::QualifiedName(template_id->name));
    expression.template_id_syntax = template_id;
  }
  if(!qualifier_template_ids.empty()) {
    set_cppast_qualifier_template_id_syntaxes(expression,
                                              qualifier_template_ids);
  }
  return expression;
}

bool simple_type_specifier_token_for_name(const std::string & name,
                                          ETokenType & out)
{
  if(name == "auto") { out = KW_AUTO; return true; }
  if(name == "bool") { out = KW_BOOL; return true; }
  if(name == "char") { out = KW_CHAR; return true; }
  if(name == "char16_t") { out = KW_CHAR16_T; return true; }
  if(name == "char32_t") { out = KW_CHAR32_T; return true; }
  if(name == "double") { out = KW_DOUBLE; return true; }
  if(name == "float") { out = KW_FLOAT; return true; }
  if(name == "int") { out = KW_INT; return true; }
  if(name == "long") { out = KW_LONG; return true; }
  if(name == "short") { out = KW_SHORT; return true; }
  if(name == "signed") { out = KW_SIGNED; return true; }
  if(name == "unsigned") { out = KW_UNSIGNED; return true; }
  if(name == "void") { out = KW_VOID; return true; }
  if(name == "wchar_t") { out = KW_WCHAR_T; return true; }
  return false;
}

bool build_empty_function_type_id_from_call_expression(const CppAstNode & expr,
                                                       CppAstNode & out)
{
  if(expr.kind != CppAstKind::call_expression ||
     expr.children.size() != 2 ||
     expr.children[0].kind != CppAstKind::id_expression ||
     expr.children[1].kind != CppAstKind::paren_argument_list ||
     !expr.children[1].children.empty()) {
    return false;
  }

  CppAstNode type_name;
  ETokenType simple_type = KW_VOID;
  if(simple_type_specifier_token_for_name(expr.children[0].value, simple_type)) {
    RecogToken token(RT_SIMPLE,
                     expr.children[0].value,
                     simple_type,
                     expr.children[0].source_location_id);
    type_name = make_token_node(CppAstKind::type_specifier, token);
  } else {
    type_name = expr.children[0];
    type_name.kind = CppAstKind::type_name;
  }

  CppAstNode specifiers = make_node(CppAstKind::type_specifier_seq,
                                    type_name.value);
  specifiers.children.push_back(std::move(type_name));

  CppAstNode parameter_clause = make_node(CppAstKind::parameter_clause);
  CppAstNode declarator = make_node(CppAstKind::abstract_declarator);
  declarator.children.push_back(std::move(parameter_clause));

  out = make_node(CppAstKind::type_id, expr.value);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarator));
  return true;
}

void build_template_argument_syntax_from_range(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    CppAstParser * parser_context,
    cpp_decl::TemplateArgumentSyntax & argument)
{
  argument = cpp_decl::TemplateArgumentSyntax();
  argument.has_source_token_start = true;
  argument.source_token_start = range.first;
  argument.source_location_id = tokens[range.first].location_id;
  argument.text =
      template_angle::token_span_text_spaced(tokens, range.first, range.second);

  std::pair<std::size_t, std::size_t> template_id_range = range;
  if(template_id_range.second > template_id_range.first &&
     tokens.peek(template_id_range.second - 1).is_simple(OP_DOTS)) {
    --template_id_range.second;
  }

  cpp_decl::TemplateIdSyntax nested_template_id;
  if(build_template_id_syntax_from_range(tokens,
                                         lookup,
                                         template_id_range,
                                         nested_template_id,
                                         parser_context)) {
    argument.template_id.reset(
        new cpp_decl::TemplateIdSyntax(std::move(nested_template_id)));
    qualified_name_parser::QualifiedNameParseResult parsed;
    const bool parsed_qualified_name =
        qualified_name_parser::parse_qualified_name(
            tokens,
            range.first,
            lookup,
            qualified_name_parser::UnqualifiedNameOptions(),
            parsed) &&
        parsed.end == range.second;
    std::vector<cpp_decl::TemplateIdSyntax> qualifier_template_ids;
    if(parsed_qualified_name &&
       qualified_name_has_qualifier_template_id(parsed)) {
      build_qualifier_template_id_syntaxes(tokens,
                                           lookup,
                                           parsed,
                                           qualifier_template_ids,
                                           parser_context);
      if(argument.template_id && !qualifier_template_ids.empty()) {
        argument.template_id->qualifier_template_id_syntaxes =
            qualifier_template_ids;
      }
    }
    argument.expression.reset(
        new CppAstNode(
            make_template_id_expression_syntax_from_range(tokens,
                                                          template_id_range,
                                                          argument.template_id,
                                                          qualifier_template_ids,
                                                          argument.text)));
    const RecogToken & head = tokens.peek(range.first);
    const bool value_only_template_id =
        head.is_identifier() &&
        (lookup.is_known_value_template_parameter_identifier(head) ||
         lookup.is_known_value_name_identifier(head)) &&
         !lookup.is_known_template_name_identifier(head) &&
         !lookup.is_known_type_name_identifier(head);
    if(!value_only_template_id) {
      if(parser_context) {
        cpp_decl::TemplateArgumentSyntax parsed_type_argument;
        if(parser_context->parse_template_argument_fragment_syntax(
               range.first,
               range.second,
               parsed_type_argument,
               CppAstParser::TAF_PARSE_TYPE_ONLY,
               false)) {
          parsed_type_argument.text = argument.text;
          parsed_type_argument.has_source_token_start = true;
          parsed_type_argument.source_token_start = argument.source_token_start;
          parsed_type_argument.source_location_id = argument.source_location_id;
          parsed_type_argument.template_id = argument.template_id;
          if(argument.expression && !parsed_type_argument.expression) {
            parsed_type_argument.expression = argument.expression;
          }
          if(parsed_type_argument.type_id && argument.template_id) {
            attach_template_id_syntax_to_direct_type_id(
                *parsed_type_argument.type_id,
                *argument.template_id);
            attach_qualifier_template_id_syntaxes_to_direct_type_id(
                *parsed_type_argument.type_id,
                qualifier_template_ids);
          }
          argument = std::move(parsed_type_argument);
        }
      }
      return;
    }
  }

  CppAstNode qualified_id_expression;
  if(build_qualified_id_expression_syntax_from_range(tokens,
                                                     lookup,
                                                     range,
                                                     qualified_id_expression,
                                                     parser_context)) {
    argument.expression.reset(
        new CppAstNode(std::move(qualified_id_expression)));
  }

  if(argument.type_id || argument.expression) {
    return;
  }

  CppAstParser::TemplateArgumentFragmentMode fragment_mode =
      CppAstParser::TAF_PARSE_BOTH;
  if(parser_context &&
     template_argument_range_fragment_mode(tokens,
                                           lookup,
                                           range,
                                           parser_context,
                                           fragment_mode)) {
    cpp_decl::TemplateArgumentSyntax parsed_argument;
    if(parser_context->parse_template_argument_fragment_syntax(range.first,
                                                               range.second,
                                                               parsed_argument,
                                                               fragment_mode)) {
      parsed_argument.text = argument.text;
      parsed_argument.has_source_token_start = true;
      parsed_argument.source_token_start = argument.source_token_start;
      parsed_argument.source_location_id = argument.source_location_id;
      argument = std::move(parsed_argument);
    }
  }
}

struct TemplateArgumentFragmentNameLookup : template_angle::NameLookup
{
  TemplateArgumentFragmentNameLookup(
      const CppAstParser & parser,
      const template_angle_lookup::ScopedNameLookup & scoped_lookup) :
    parser(parser),
    scoped_lookup(scoped_lookup)
  {}

  virtual bool is_known_template_name_identifier(const RecogToken & token) const
  {
    return scoped_lookup.is_known_template_name_identifier(token);
  }

  virtual bool is_known_type_name_identifier(const RecogToken & token) const
  {
    return parser.is_template_type_parameter_name(token) ||
           scoped_lookup.is_known_type_name_identifier(token);
  }

  virtual bool is_known_value_template_parameter_identifier(
      const RecogToken & token) const
  {
    return scoped_lookup.is_known_value_template_parameter_identifier(token);
  }

  virtual bool is_known_value_name_identifier(const RecogToken & token) const
  {
    return scoped_lookup.is_known_value_name_identifier(token);
  }

  virtual bool is_template_type_parameter_identifier(
      const RecogToken & token) const
  {
    return parser.is_template_type_parameter_name(token) ||
           scoped_lookup.is_template_type_parameter_identifier(token);
  }

  virtual bool prefer_template_id_for_unknown_identifiers() const
  {
    return scoped_lookup.prefer_template_id_for_unknown_identifiers();
  }

  const CppAstParser & parser;
  template_angle_lookup::ScopedNameLookup scoped_lookup;
};

bool build_template_id_syntax_from_range(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    cpp_decl::TemplateIdSyntax & out,
    CppAstParser * parser_context)
{
  qualified_name_parser::QualifiedNameParseResult parsed;
  if(qualified_name_parser::parse_qualified_name(
         tokens,
         range.first,
         lookup,
         qualified_name_parser::UnqualifiedNameOptions(),
         parsed) &&
     parsed.end == range.second) {
    return build_template_id_syntax(tokens, lookup, parsed, out, parser_context);
  }

  for(size_t open = range.first; open < range.second; ++open) {
    if(!tokens.peek(open).is_simple(OP_LT)) {
      continue;
    }

    size_t suffix_end = open;
    vector<pair<size_t, size_t> > arg_ranges;
    if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                        open,
                                                        lookup,
                                                        suffix_end,
                                                        arg_ranges) ||
       suffix_end != range.second) {
      continue;
    }

    RecogTokenRangeSequence head_tokens(tokens, range.first, open);
    qualified_name_parser::QualifiedNameParseResult head_parsed;
    if(!qualified_name_parser::parse_qualified_name(
           head_tokens,
           0,
           lookup,
           qualified_name_parser::UnqualifiedNameOptions(),
           head_parsed) ||
       head_parsed.end != open - range.first) {
      continue;
    }

    out = cpp_decl::TemplateIdSyntax();
    out.name = build_qualified_name_syntax(head_tokens, head_parsed);
    out.source_location_id = tokens[range.first].location_id;
    qualified_name_parser::QualifiedNameParseResult original_head_parsed;
    if(qualified_name_parser::parse_qualified_name(
           tokens,
           range.first,
           lookup,
           qualified_name_parser::UnqualifiedNameOptions(),
           original_head_parsed) &&
       original_head_parsed.end == open &&
       qualified_name_has_qualifier_template_id(original_head_parsed)) {
      build_qualifier_template_id_syntaxes(tokens,
                                           lookup,
                                           original_head_parsed,
                                           out.qualifier_template_id_syntaxes,
                                           parser_context);
    }
    for(size_t i = 0; i < arg_ranges.size(); ++i) {
      cpp_decl::TemplateArgumentSyntax argument;
      build_template_argument_syntax_from_range(tokens,
                                                lookup,
                                                arg_ranges[i],
                                                parser_context,
                                                argument);
      out.arguments.push_back(argument.text);
      out.argument_syntaxes.push_back(argument);
    }
    return !out.name.name.empty();
  }

  return false;
}

bool build_template_id_syntax(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const qualified_name_parser::QualifiedNameParseResult & parsed,
    cpp_decl::TemplateIdSyntax & out,
    CppAstParser * parser_context)
{
  if(!parsed.name_has_template_suffix ||
     parsed.name_template_head_component.second <=
         parsed.name_template_head_component.first) {
    return false;
  }

  out = cpp_decl::TemplateIdSyntax();
  out.name.rooted = parsed.rooted;
  out.source_location_id =
      tokens[parsed.name_template_head_component.first].location_id;
  for(size_t i = 0; i < parsed.qualifiers.size(); ++i) {
    out.name.qualifiers.push_back(
        template_angle::token_span_text_spaced(tokens,
                                               parsed.qualifiers[i].first,
                                               parsed.qualifiers[i].second));
  }
  if(parsed.name_kind == qualified_name_parser::UNQ_COMPONENT) {
    out.name.name =
        template_angle::token_span_text_spaced(
            tokens,
            parsed.name_template_head_component.first,
            parsed.name_template_head_component.second);
  } else {
    out.name.name =
        tokens.span_text(parsed.name_template_head_component.first,
                         parsed.name_template_head_component.second);
  }

  for(size_t i = 0; i < parsed.name_template_arg_ranges.size(); ++i) {
    cpp_decl::TemplateArgumentSyntax argument;
    build_template_argument_syntax_from_range(tokens,
                                              lookup,
                                              parsed.name_template_arg_ranges[i],
                                              parser_context,
                                              argument);
    out.arguments.push_back(argument.text);
    out.argument_syntaxes.push_back(argument);
  }
  if(qualified_name_has_qualifier_template_id(parsed)) {
    std::vector<cpp_decl::TemplateIdSyntax> qualifier_template_ids;
    build_qualifier_template_id_syntaxes(tokens,
                                         lookup,
                                         parsed,
                                         qualifier_template_ids,
                                         parser_context);
    out.qualifier_template_id_syntaxes = std::move(qualifier_template_ids);
  }
  return !out.name.name.empty();
}

bool qualified_name_has_qualifier_template_id(
    const qualified_name_parser::QualifiedNameParseResult & parsed)
{
  for(size_t i = 0; i < parsed.qualifier_components.size(); ++i) {
    const qualified_name_parser::NameComponentParseResult & component =
        parsed.qualifier_components[i];
    if(component.has_template_suffix &&
       component.name_component.second > component.name_component.first) {
      return true;
    }
  }
  return false;
}

void build_qualifier_template_id_syntaxes(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const qualified_name_parser::QualifiedNameParseResult & parsed,
    std::vector<cpp_decl::TemplateIdSyntax> & out,
    CppAstParser * parser_context)
{
  out.clear();
  out.resize(parsed.qualifier_components.size());
  bool any_template_id = false;
  for(size_t i = 0; i < parsed.qualifier_components.size(); ++i) {
    const qualified_name_parser::NameComponentParseResult & component =
        parsed.qualifier_components[i];
    if(!component.has_template_suffix ||
       component.name_component.second <= component.name_component.first) {
      continue;
    }

    cpp_decl::TemplateIdSyntax syntax;
    syntax.name.rooted = parsed.rooted;
    syntax.source_location_id =
        tokens[component.name_component.first].location_id;
    for(size_t qualifier_index = 0; qualifier_index < i; ++qualifier_index) {
      syntax.name.qualifiers.push_back(
          template_angle::token_span_text_spaced(
              tokens,
              parsed.qualifiers[qualifier_index].first,
              parsed.qualifiers[qualifier_index].second));
    }
    syntax.name.name =
        template_angle::token_span_text_spaced(tokens,
                                               component.name_component.first,
                                               component.name_component.second);
    for(size_t arg_index = 0; arg_index < component.template_arg_ranges.size(); ++arg_index) {
      cpp_decl::TemplateArgumentSyntax argument;
      build_template_argument_syntax_from_range(
          tokens,
          lookup,
          component.template_arg_ranges[arg_index],
          parser_context,
          argument);
      syntax.arguments.push_back(argument.text);
      syntax.argument_syntaxes.push_back(argument);
    }
    if(any_template_id) {
      syntax.qualifier_template_id_syntaxes.assign(out.begin(), out.begin() + i);
    }
    if(!syntax.name.name.empty()) {
      out[i] = syntax;
      any_template_id = true;
    }
  }
  if(!any_template_id) {
    out.clear();
  }
}

bool build_qualified_id_expression_syntax_from_range(
    IRecogTokenSequence & tokens,
    const qualified_name_parser::NameLookup & lookup,
    const std::pair<std::size_t, std::size_t> & range,
    CppAstNode & out,
    CppAstParser * parser_context)
{
  if(range.first >= range.second) {
    return false;
  }

  qualified_name_parser::QualifiedNameParseResult parsed;
  if(!qualified_name_parser::parse_qualified_name(
         tokens,
         range.first,
         lookup,
         qualified_name_parser::UnqualifiedNameOptions(),
         parsed) ||
     parsed.end != range.second) {
    return false;
  }

  if(!parsed.rooted && parsed.qualifiers.empty() &&
     !parsed.name_has_template_suffix) {
    const RecogToken & token = tokens.peek(range.first);
    const bool known_value =
        lookup.is_known_value_template_parameter_identifier(token) ||
        lookup.is_known_value_name_identifier(token);
    const bool known_type =
        lookup.is_known_template_name_identifier(token) ||
        lookup.is_known_type_name_identifier(token);
    if(!known_value || known_type) {
      return false;
    }
  }

  CppAstNode expression = make_node(
      CppAstKind::id_expression,
      template_angle::token_span_text_spaced(tokens, range.first, range.second));
  expression.token_start = range.first;
  expression.token_end = range.second;
  expression.source_location_id = tokens[range.first].location_id;
  set_cppast_qualified_name_syntax(
      expression,
      build_qualified_name_syntax(tokens, parsed));

  cpp_decl::TemplateIdSyntax template_id_syntax;
  if(build_template_id_syntax(tokens,
                              lookup,
                              parsed,
                              template_id_syntax,
                              parser_context) &&
     !template_id_syntax.name.name.empty()) {
    set_cppast_template_id_syntax(expression, std::move(template_id_syntax));
  }

  std::vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  build_qualifier_template_id_syntaxes(tokens,
                                       lookup,
                                       parsed,
                                       qualifier_template_id_syntaxes,
                                       parser_context);
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(
        expression,
        std::move(qualifier_template_id_syntaxes));
  }

  out = std::move(expression);
  return true;
}

bool is_fold_operator_token(const RecogToken & token)
{
  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_PLUS:
  case OP_MINUS:
  case OP_STAR:
  case OP_DIV:
  case OP_MOD:
  case OP_XOR:
  case OP_AMP:
  case OP_BOR:
  case OP_ASS:
  case OP_LT:
  case OP_GT:
  case OP_LSHIFT:
  case OP_RSHIFT:
  case OP_PLUSASS:
  case OP_MINUSASS:
  case OP_STARASS:
  case OP_DIVASS:
  case OP_MODASS:
  case OP_XORASS:
  case OP_BANDASS:
  case OP_BORASS:
  case OP_LSHIFTASS:
  case OP_RSHIFTASS:
  case OP_EQ:
  case OP_NE:
  case OP_LE:
  case OP_GE:
  case OP_LAND:
  case OP_LOR:
  case OP_COMMA:
  case OP_DOTSTAR:
  case OP_ARROWSTAR:
    return true;
  default:
    return false;
  }
}

void offset_node_token_spans(CppAstNode & node, size_t offset)
{
  node.token_start += offset;
  node.token_end += offset;
  for(size_t i = 0; i < node.children.size(); ++i) {
    offset_node_token_spans(node.children[i], offset);
  }
}

bool is_builtin_type_trait_identifier(const RecogToken & token)
{
  if(!token.is_identifier()) {
    return false;
  }

  return token.source == "__array_extent" ||
         token.source == "__array_rank" ||
         token.source == "__is_trivially_destructible" ||
         token.source == "__is_trivially_constructible" ||
         token.source == "__is_trivially_assignable" ||
         token.source == "__is_trivially_copyable" ||
         token.source == "__is_pod" ||
         token.source == "__is_trivial" ||
         token.source == "__has_trivial_constructor" ||
         token.source == "__has_trivial_destructor" ||
         token.source == "__has_virtual_destructor" ||
         token.source == "__has_unique_object_representations" ||
         token.source == "__is_abstract" ||
         token.source == "__is_aggregate" ||
         token.source == "__is_bounded_array" ||
         token.source == "__is_integral" ||
         token.source == "__is_floating_point" ||
         token.source == "__is_arithmetic" ||
         token.source == "__is_signed" ||
         token.source == "__is_unsigned" ||
         token.source == "__is_const" ||
         token.source == "__is_volatile" ||
         token.source == "__is_reference" ||
         token.source == "__is_lvalue_reference" ||
         token.source == "__is_rvalue_reference" ||
         token.source == "__is_void" ||
         token.source == "__is_array" ||
         token.source == "__is_pointer" ||
         token.source == "__is_enum" ||
         token.source == "__is_union" ||
         token.source == "__is_class" ||
         token.source == "__is_empty" ||
         token.source == "__is_fundamental" ||
         token.source == "__is_scalar" ||
         token.source == "__is_compound" ||
         token.source == "__is_object" ||
         token.source == "__is_standard_layout" ||
         token.source == "__is_destructible" ||
         token.source == "__is_nothrow_destructible" ||
         token.source == "__is_same" ||
         token.source == "__is_assignable" ||
         token.source == "__is_nothrow_assignable" ||
         token.source == "__is_convertible" ||
         token.source == "__is_nothrow_convertible" ||
         token.source == "__is_constructible" ||
         token.source == "__is_nothrow_constructible" ||
         token.source == "__is_base_of" ||
         token.source == "__is_final" ||
         token.source == "__is_literal_type" ||
         token.source == "__is_function" ||
         token.source == "__is_member_function_pointer" ||
         token.source == "__is_member_object_pointer" ||
         token.source == "__is_member_pointer" ||
         token.source == "__is_polymorphic" ||
         token.source == "__is_referenceable" ||
         token.source == "__is_scoped_enum" ||
         token.source == "__is_trivially_equality_comparable" ||
         token.source == "__is_trivially_relocatable" ||
         token.source == "__is_unbounded_array" ||
         token.source == "__reference_constructs_from_temporary" ||
         token.source == "__reference_binds_to_temporary" ||
         token.source == "__reference_converts_from_temporary";
}

bool is_builtin_type_trait_expression_start(const IRecogTokenSequence & tokens,
                                            size_t pos)
{
  return is_builtin_type_trait_identifier(tokens.peek(pos)) &&
         tokens.peek(pos + 1).is_simple(OP_LPAREN);
}

bool is_builtin_type_transform_identifier(const RecogToken & token)
{
  if(!token.is_identifier()) {
    return false;
  }

  return builtin_type_transforms::is_supported_name(token.source);
}

void annotate_builtin_type_transform_node(CppAstNode & node,
                                          const IRecogTokenSequence & tokens,
                                          size_t name_start)
{
  const RecogToken & identifier = tokens.peek(name_start);
  if(is_builtin_type_transform_identifier(identifier) &&
     tokens.peek(name_start + 1).is_simple(OP_LPAREN)) {
    node.builtin_type_transform_name = identifier.source;
  }
}

bool is_alignof_type_trait_identifier(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__alignof" || token.source == "__alignof__");
}

bool is_alignas_expression_preferred_start(const IRecogTokenSequence & tokens,
                                           size_t pos)
{
  const RecogToken & token = tokens.peek(pos);
  return token.is_simple(KW_ALIGNOF) ||
         token.is_simple(KW_NOEXCEPT) ||
         is_alignof_type_trait_identifier(token) ||
         is_builtin_type_trait_expression_start(tokens, pos);
}

bool is_gnu_complex_type_keyword(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "_Complex" ||
          token.source == "__complex" ||
          token.source == "__complex__");
}

bool match_gnu_complex_type_specifier(const IRecogTokenSequence & tokens,
                                      size_t start,
                                      size_t & end)
{
  if(!is_gnu_complex_type_keyword(tokens.peek(start))) {
    return false;
  }

  if(tokens.peek(start + 1).is_simple(KW_FLOAT) ||
     tokens.peek(start + 1).is_simple(KW_DOUBLE)) {
    end = start + 2;
    return true;
  }

  if(tokens.peek(start + 1).is_simple(KW_LONG) &&
     tokens.peek(start + 2).is_simple(KW_DOUBLE)) {
    end = start + 3;
    return true;
  }

  return false;
}

bool is_gnu_complex_unary_operator_name(const string & text)
{
  return text == "__real" || text == "__real__" ||
         text == "__imag" || text == "__imag__";
}

bool is_coroutine_contextual_keyword(const RecogToken & token,
                                     const char * spelling)
{
  return token.is_identifier() && token.source == spelling;
}

bool can_parse_coroutine_contextual_keyword_in_template(
    const RecogToken & token,
    const char * spelling,
    size_t template_declaration_depth,
    bool known_value_name)
{
  return template_declaration_depth > 0 &&
         is_coroutine_contextual_keyword(token, spelling) &&
         !known_value_name;
}

void apply_leading_declaration_attributes(CppAstNode & node,
                                          const CppAstNode & attributes)
{
  if(attributes.has_no_unique_address) {
    node.has_no_unique_address = true;
  }
  if(attributes.has_using_if_exists) {
    node.has_using_if_exists = true;
  }
  if(attributes.has_exclude_from_explicit_instantiation) {
    node.has_exclude_from_explicit_instantiation = true;
  }
  append_cppast_abi_tags(node.abi_tags, attributes);
  append_cppast_alignment_specifiers(node, attributes);

  if(node.kind != CppAstKind::simple_declaration) {
    return;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::init_declarator_list) {
      continue;
    }
    for(size_t j = 0; j < child.children.size(); ++j) {
      if(child.children[j].kind == CppAstKind::init_declarator) {
        if(attributes.has_no_unique_address) {
          child.children[j].has_no_unique_address = true;
        }
        if(attributes.has_exclude_from_explicit_instantiation) {
          child.children[j].has_exclude_from_explicit_instantiation = true;
        }
        append_cppast_abi_tags(child.children[j].abi_tags, attributes);
        append_cppast_alignment_specifiers(child.children[j], attributes);
      }
    }
  }
}

bool is_abi_tag_attribute_name(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "abi_tag" ||
          token.source == "__abi_tag" ||
          token.source == "__abi_tag__");
}

bool is_string_literal_attribute_token(const RecogToken & token)
{
  return token.is_literal() && token.source.find('"') != std::string::npos;
}

std::string abi_tag_literal_value(const std::string & source)
{
  const std::size_t first_quote = source.find('"');
  const std::size_t last_quote = source.rfind('"');
  if(first_quote == std::string::npos || last_quote == std::string::npos ||
     last_quote <= first_quote) {
    return std::string();
  }
  return source.substr(first_quote + 1, last_quote - first_quote - 1);
}

std::string gnu_asm_label_literal_value(const IRecogTokenSequence & tokens,
                                        std::size_t start,
                                        std::size_t end)
{
  if(end != start + 4 ||
     !tokens.peek(start + 1).is_simple(OP_LPAREN) ||
     !tokens.peek(start + 2).is_literal() ||
     !tokens.peek(start + 3).is_simple(OP_RPAREN)) {
    return std::string();
  }
  const QuoteLiteralData literal =
      parse_quote_literal(tokens.peek(start + 2).source);
  if(literal.quote != '"') {
    return std::string();
  }

  std::string label;
  label.reserve(literal.contents.size());
  for(char32_t ch : literal.contents) {
    if(ch > 0xff) {
      return std::string();
    }
    label.push_back(static_cast<char>(ch));
  }
  return label;
}

bool is_gnu_asm_token(const RecogToken & token)
{
  return token.is_simple(KW_ASM) ||
         (token.is_identifier() &&
          (token.source == "__asm" || token.source == "__asm__"));
}

bool is_gnu_asm_qualifier_token(const RecogToken & token)
{
  if(token.is_simple(KW_VOLATILE) || token.is_simple(KW_INLINE) || token.is_simple(KW_GOTO)) {
    return true;
  }

  return token.is_identifier() &&
         (token.source == "__volatile" || token.source == "__volatile__" ||
          token.source == "__inline" || token.source == "__inline__" ||
          token.source == "__goto" || token.source == "__goto__");
}

bool is_nullability_qualifier_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "_Nonnull" ||
          token.source == "_Nullable" ||
          token.source == "_Null_unspecified" ||
          token.source == "_Nullable_result");
}

bool is_gnu_restrict_qualifier_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__restrict" ||
          token.source == "__restrict__" ||
          token.source == "restrict");
}

bool is_gnu_typeof_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__typeof" || token.source == "__typeof__");
}

bool is_gnu_decltype_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__decltype" || token.source == "__decltype__");
}

bool is_decltype_token(const RecogToken & token)
{
  return token.is_simple(KW_DECLTYPE) || is_gnu_decltype_token(token);
}

bool is_gnu_decl_specifier_identifier(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__inline" ||
          token.source == "__inline__" ||
          token.source == "__forceinline" ||
          token.source == "__volatile" ||
          token.source == "__volatile__" ||
          token.source == "__const" ||
          token.source == "__const__" ||
          token.source == "__signed" ||
          token.source == "__signed__" ||
          token.source == "__unsigned" ||
          token.source == "__unsigned__");
}

bool is_gnu_float_type_specifier_identifier(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__float128" ||
          token.source == "_Float128" ||
          token.source == "_Float64" ||
          token.source == "_Float32");
}

bool is_gnu_int128_type_specifier_identifier(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__int128" ||
          token.source == "__int128_t" ||
          token.source == "__uint128_t");
}

bool is_gnu_extension_token(const RecogToken & token)
{
  return token.is_identifier() && token.source == "__extension__";
}

bool is_gnu_attribute_name_token(const RecogToken & token)
{
  return token.is_identifier() &&
         (token.source == "__attribute__" || token.source == "__attribute");
}

size_t find_last_top_level_colon2(const string & text)
{
  size_t angle_depth = 0;
  size_t last_colon2 = string::npos;
  for(size_t i = 0; i + 1 < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>') {
      if(angle_depth > 0) {
        --angle_depth;
      }
      continue;
    }
    if(angle_depth == 0 && ch == ':' && text[i + 1] == ':') {
      last_colon2 = i;
      ++i;
    }
  }
  return last_colon2;
}

size_t find_first_top_level_angle(const string & text)
{
  size_t angle_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      if(angle_depth == 0) {
        return i;
      }
      ++angle_depth;
      continue;
    }
    if(ch == '>') {
      if(angle_depth > 0) {
        --angle_depth;
      }
    }
  }
  return string::npos;
}

string unqualified_name_text(const string & name)
{
  const size_t last_colon2 = find_last_top_level_colon2(name);
  if(last_colon2 == string::npos) {
    return name;
  }
  return name.substr(last_colon2 + 2);
}

string primary_name_text(const string & name)
{
  const string unqualified = unqualified_name_text(name);
  const size_t angle = find_first_top_level_angle(unqualified);
  if(angle == string::npos) {
    return unqualified;
  }
  return unqualified.substr(0, angle);
}

string current_class_trace_label(const vector<string> & class_name_stack)
{
  if(class_name_stack.empty()) {
    return "<none>";
  }
  return class_name_stack.back();
}

int nearest_name_scope_index(const template_angle_lookup::NameSetStack * primary,
                             const template_angle_lookup::NameSetStack * inherited,
                             text_intern::Atom atom)
{
  if(!atom) {
    return -1;
  }

  const int inherited_size =
      inherited ? static_cast<int>(inherited->size()) : 0;
  if(primary) {
    for(size_t i = primary->size(); i > 0; --i) {
      if((*primary)[i - 1].count(atom) != 0) {
        return inherited_size + static_cast<int>(i - 1);
      }
    }
  }
  if(inherited) {
    for(size_t i = inherited->size(); i > 0; --i) {
      if((*inherited)[i - 1].count(atom) != 0) {
        return static_cast<int>(i - 1);
      }
    }
  }
  return -1;
}

}  // namespace

CppAstParser::CppAstParser(const vector<RecogToken> & tokens) :
  RecogTokenCursor(tokens)
{
  template_value_parameter_scopes.push_back(NameSet());
  template_name_scopes.push_back(NameSet());
  type_name_scopes.push_back(NameSet());
  value_name_scopes.push_back(NameSet());
}

CppAstParser::CppAstParser(IRecogTokenSequence & tokens) :
  RecogTokenCursor(tokens)
{
  template_value_parameter_scopes.push_back(NameSet());
  template_name_scopes.push_back(NameSet());
  type_name_scopes.push_back(NameSet());
  value_name_scopes.push_back(NameSet());
}

size_t CppAstParser::NamedDeclSpecifierSeqCacheKeyHash::operator()(
    const NamedDeclSpecifierSeqCacheKey & key) const
{
  size_t hash = key.pos;
  hash ^= key.template_type_parameter_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.template_value_parameter_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.template_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.type_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.value_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  return hash;
}

size_t CppAstParser::DeclarationStartProbeCacheKeyHash::operator()(
    const DeclarationStartProbeCacheKey & key) const
{
  size_t hash = key.pos;
  hash ^= key.template_type_parameter_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.template_value_parameter_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.template_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.type_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.value_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  hash ^= key.class_name_depth + 0x9e3779b97f4a7c15ULL +
          (hash << 6) + (hash >> 2);
  return hash;
}

CppAstParser::NamedDeclSpecifierSeqCacheKey
CppAstParser::make_named_decl_specifier_seq_cache_key() const
{
  NamedDeclSpecifierSeqCacheKey key;
  key.pos = pos;
  key.template_type_parameter_depth = template_type_parameter_scopes.size();
  key.template_value_parameter_depth = template_value_parameter_scopes.size();
  key.template_name_depth = template_name_scopes.size();
  key.type_name_depth = type_name_scopes.size();
  key.value_name_depth = value_name_scopes.size();
  return key;
}

CppAstParser::DeclarationStartProbeCacheKey
CppAstParser::make_declaration_start_probe_cache_key(size_t probe_pos) const
{
  DeclarationStartProbeCacheKey key;
  key.pos = probe_pos;
  key.template_type_parameter_depth = template_type_parameter_scopes.size();
  key.template_value_parameter_depth = template_value_parameter_scopes.size();
  key.template_name_depth = template_name_scopes.size();
  key.type_name_depth = type_name_scopes.size();
  key.value_name_depth = value_name_scopes.size();
  key.class_name_depth = class_name_stack.size();
  return key;
}

void CppAstParser::note_name_lookup_mutation()
{
  named_decl_specifier_seq_cache.clear();
  declaration_start_probe_cache.clear();
}

void CppAstParser::inherit_name_lookup_state_from(const CppAstParser & parent)
{
  const auto inherit_stack = [](const vector<NameSet> & local,
                                const vector<NameSet> * inherited,
                                vector<NameSet> & materialized,
                                const vector<NameSet> *& out) -> void
  {
    if(inherited == nullptr) {
      materialized.clear();
      out = &local;
      return;
    }
    if(local.empty()) {
      materialized.clear();
      out = inherited;
      return;
    }
    materialized = *inherited;
    materialized.insert(materialized.end(), local.begin(), local.end());
    out = &materialized;
  };

  inherit_stack(parent.template_type_parameter_scopes,
                parent.inherited_template_type_parameter_scopes,
                materialized_inherited_template_type_parameter_scopes,
                inherited_template_type_parameter_scopes);
  inherit_stack(parent.template_value_parameter_scopes,
                parent.inherited_template_value_parameter_scopes,
                materialized_inherited_template_value_parameter_scopes,
                inherited_template_value_parameter_scopes);
  inherit_stack(parent.template_name_scopes,
                parent.inherited_template_name_scopes,
                materialized_inherited_template_name_scopes,
                inherited_template_name_scopes);
  inherit_stack(parent.type_name_scopes,
                parent.inherited_type_name_scopes,
                materialized_inherited_type_name_scopes,
                inherited_type_name_scopes);
  inherit_stack(parent.value_name_scopes,
                parent.inherited_value_name_scopes,
                materialized_inherited_value_name_scopes,
                inherited_value_name_scopes);
  borrowed_template_parameter_lookup =
      parent.borrowed_template_parameter_lookup;
  external_name_lookup = parent.external_name_lookup;
  suppress_template_argument_fragment_syntax =
      parent.suppress_template_argument_fragment_syntax;
}

shared_ptr<const CppAstNameLookupSnapshot>
CppAstParser::snapshot_name_lookup_state(const NameSet * used_names) const
{
  const auto inherit_stack = [](const vector<NameSet> & local,
                                const vector<NameSet> * inherited,
                                const NameSet * used_names,
                                vector<NameSet> & out) -> void
  {
    out.clear();
    const auto append_scope = [&](const NameSet & source) -> void
    {
      if(!used_names) {
        out.push_back(source);
        return;
      }
      NameSet filtered;
      if(used_names->size() < source.size()) {
        for(NameSet::const_iterator it = used_names->begin();
            it != used_names->end();
            ++it) {
          if(source.count(*it) != 0) {
            filtered.insert(*it);
          }
        }
      } else {
        for(NameSet::const_iterator it = source.begin(); it != source.end(); ++it) {
          if(used_names->count(*it) != 0) {
            filtered.insert(*it);
          }
        }
      }
      if(!filtered.empty()) {
        out.push_back(filtered);
      }
    };

    if(inherited) {
      for(size_t i = 0; i < inherited->size(); ++i) {
        append_scope((*inherited)[i]);
      }
    }
    for(size_t i = 0; i < local.size(); ++i) {
      append_scope(local[i]);
    }
  };

  shared_ptr<CppAstNameLookupSnapshot> snapshot(new CppAstNameLookupSnapshot);
  inherit_stack(template_type_parameter_scopes,
                inherited_template_type_parameter_scopes,
                used_names,
                snapshot->template_type_parameter_scopes);
  inherit_stack(template_value_parameter_scopes,
                inherited_template_value_parameter_scopes,
                used_names,
                snapshot->template_value_parameter_scopes);
  inherit_stack(template_name_scopes,
                inherited_template_name_scopes,
                used_names,
                snapshot->template_name_scopes);
  inherit_stack(type_name_scopes,
                inherited_type_name_scopes,
                used_names,
                snapshot->type_name_scopes);
  inherit_stack(value_name_scopes,
                inherited_value_name_scopes,
                used_names,
                snapshot->value_name_scopes);
  snapshot->class_name_stack = class_name_stack;
  snapshot->namespace_path_stack = namespace_path_stack;
  snapshot->namespace_inline_stack = namespace_inline_stack;
  return snapshot;
}

void CppAstParser::restore_name_lookup_state_from(
    const CppAstNameLookupSnapshot & snapshot)
{
  template_type_parameter_scopes = snapshot.template_type_parameter_scopes;
  template_value_parameter_scopes = snapshot.template_value_parameter_scopes;
  template_name_scopes = snapshot.template_name_scopes;
  type_name_scopes = snapshot.type_name_scopes;
  value_name_scopes = snapshot.value_name_scopes;
  class_name_stack = snapshot.class_name_stack;
  namespace_path_stack = snapshot.namespace_path_stack;
  namespace_inline_stack = snapshot.namespace_inline_stack;

  materialized_inherited_template_type_parameter_scopes.clear();
  materialized_inherited_template_value_parameter_scopes.clear();
  materialized_inherited_template_name_scopes.clear();
  materialized_inherited_type_name_scopes.clear();
  materialized_inherited_value_name_scopes.clear();
  inherited_template_type_parameter_scopes = nullptr;
  inherited_template_value_parameter_scopes = nullptr;
  inherited_template_name_scopes = nullptr;
  inherited_type_name_scopes = nullptr;
  inherited_value_name_scopes = nullptr;
  borrowed_template_parameter_lookup = nullptr;
  external_name_lookup = nullptr;
  note_name_lookup_mutation();
}

void CppAstParser::seed_known_type_names(const NameSet & names)
{
  if(type_name_scopes.empty()) {
    type_name_scopes.push_back(NameSet());
  }
  type_name_scopes.back().insert(names.begin(), names.end());
  note_name_lookup_mutation();
}

void CppAstParser::seed_known_template_names(const NameSet & names)
{
  if(template_name_scopes.empty()) {
    template_name_scopes.push_back(NameSet());
  }
  template_name_scopes.back().insert(names.begin(), names.end());
  note_name_lookup_mutation();
}

void CppAstParser::seed_known_value_names(const NameSet & names)
{
  if(value_name_scopes.empty()) {
    value_name_scopes.push_back(NameSet());
  }
  value_name_scopes.back().insert(names.begin(), names.end());
  note_name_lookup_mutation();
}

void CppAstParser::seed_known_template_value_names(const NameSet & names)
{
  if(template_value_parameter_scopes.empty()) {
    template_value_parameter_scopes.push_back(NameSet());
  }
  template_value_parameter_scopes.back().insert(names.begin(), names.end());
  note_name_lookup_mutation();
}

void CppAstParser::set_external_name_lookup(const template_angle::NameLookup * lookup)
{
  external_name_lookup = lookup;
  note_name_lookup_mutation();
}

string CppAstParser::normalized_name_without_template_args(const string & text) const
{
  string out;
  size_t angle_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(isspace(static_cast<unsigned char>(ch))) {
      continue;
    }
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>') {
      if(angle_depth > 0) {
        --angle_depth;
      }
      continue;
    }
    if(angle_depth != 0) {
      continue;
    }
    out.push_back(ch);
  }
  return out;
}

string CppAstParser::normalized_lookup_name(const string & text) const
{
  string out = normalized_name_without_template_args(text);
  const string typename_prefix = "typename";
  if(out.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    out.erase(0, typename_prefix.size());
  }
  while(out.compare(0, 2, "::") == 0) {
    out.erase(0, 2);
  }
  return out;
}

bool CppAstParser::namespace_scope_exists(const string & key) const
{
  return namespace_template_name_scopes.count(key) != 0 ||
         namespace_template_value_name_scopes.count(key) != 0 ||
         namespace_type_name_scopes.count(key) != 0 ||
         namespace_value_name_scopes.count(key) != 0 ||
         namespace_alias_targets.count(key) != 0;
}

string CppAstParser::resolve_namespace_alias_chain(const string & key) const
{
  string current = key;
  for(size_t i = 0; i < 32; ++i) {
    const auto found = namespace_alias_targets.find(current);
    if(found == namespace_alias_targets.end() || found->second.empty() ||
       found->second == current) {
      return current;
    }
    current = found->second;
  }
  return current;
}

string CppAstParser::resolve_visible_namespace_scope_key(const string & text) const
{
  const string normalized = normalized_lookup_name(text);
  if(normalized.empty()) {
    return string();
  }

  vector<string> candidates;
  if(normalized.find("::") != string::npos) {
    vector<string> prefixes = namespace_path_stack;
    for(size_t depth = prefixes.size(); ; --depth) {
      string candidate;
      for(size_t i = 0; i < depth; ++i) {
        if(!candidate.empty()) {
          candidate += "::";
        }
        candidate += prefixes[i];
      }
      if(!candidate.empty()) {
        candidate += "::";
      }
      candidate += normalized;
      candidates.push_back(candidate);
      if(depth == 0) {
        break;
      }
    }
  } else {
    vector<string> prefixes = namespace_path_stack;
    for(size_t depth = prefixes.size(); ; --depth) {
      string candidate;
      for(size_t i = 0; i < depth; ++i) {
        if(!candidate.empty()) {
          candidate += "::";
        }
        candidate += prefixes[i];
      }
      if(!candidate.empty()) {
        candidate += "::";
      }
      candidate += normalized;
      candidates.push_back(candidate);
      if(depth == 0) {
        break;
      }
    }
  }

  for(size_t i = 0; i < candidates.size(); ++i) {
    const string resolved = resolve_namespace_alias_chain(candidates[i]);
    if(namespace_scope_exists(resolved)) {
      return resolved;
    }
  }
  return resolve_namespace_alias_chain(normalized);
}

string CppAstParser::class_scope_definition_key(const string & name) const
{
  const string normalized = normalized_name_without_template_args(name);
  if(normalized.empty()) {
    return normalized;
  }
  if(normalized.find("::") != string::npos) {
    return normalized;
  }

  string out = current_namespace_path_key();
  for(size_t i = 0; i < class_name_stack.size(); ++i) {
    if(!out.empty()) {
      out += "::";
    }
    out += class_name_stack[i];
  }
  if(!out.empty()) {
    out += "::";
  }
  out += normalized;
  return out;
}

string CppAstParser::owner_class_scope_key(const string & qualified_name) const
{
  const string normalized = normalized_name_without_template_args(qualified_name);
  const size_t split = normalized.rfind("::");
  if(split == string::npos) {
    return string();
  }
  return class_scope_definition_key(normalized.substr(0, split));
}

string CppAstParser::first_identifier_text(const CppAstNode & node) const
{
  if(node.kind == CppAstKind::identifier) {
    return node.value;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    const string child = first_identifier_text(node.children[i]);
    if(!child.empty()) {
      return child;
    }
  }
  return string();
}

bool CppAstParser::resolve_declarator_owner_class_scope_key(const CppAstNode & declarator,
                                                            string & out) const
{
  out = owner_class_scope_key(first_identifier_text(declarator));
  return !out.empty();
}

CppAstParser::SeededClassNameScopes CppAstParser::push_class_member_name_hints(
    const string & class_key)
{
  SeededClassNameScopes seeded;
  const string current_namespace = current_namespace_path_key();
  const auto find_scopes =
      [&](const string & key) -> const ClassMemberNameScopes *
  {
    std::unordered_map<string, ClassMemberNameScopes>::const_iterator found =
        class_member_name_scopes.find(key);
    if(found != class_member_name_scopes.end()) {
      return &found->second;
    }
    if(!current_namespace.empty()) {
      const string qualified = current_namespace + "::" + key;
      found = class_member_name_scopes.find(qualified);
      if(found != class_member_name_scopes.end()) {
        return &found->second;
      }
    }
    return nullptr;
  };

  vector<string> keys;
  for(string key = class_key; !key.empty();) {
    keys.push_back(key);
    const size_t split = key.rfind("::");
    if(split == string::npos) {
      break;
    }
    key = key.substr(0, split);
  }

  for(vector<string>::const_reverse_iterator it = keys.rbegin();
      it != keys.rend();
      ++it) {
    const ClassMemberNameScopes * found = find_scopes(*it);
    if(!found) {
      continue;
    }

    if(!found->template_names.empty()) {
      template_name_scopes.push_back(found->template_names);
      ++seeded.template_names;
    }
    if(!found->type_names.empty()) {
      type_name_scopes.push_back(found->type_names);
      ++seeded.type_names;
    }
    if(!found->value_names.empty()) {
      value_name_scopes.push_back(found->value_names);
      ++seeded.value_names;
    }
  }
  return seeded;
}

void CppAstParser::pop_class_member_name_hints(const SeededClassNameScopes & seeded)
{
  for(size_t i = 0; i < seeded.value_names; ++i) {
    value_name_scopes.pop_back();
  }
  for(size_t i = 0; i < seeded.type_names; ++i) {
    type_name_scopes.pop_back();
  }
  for(size_t i = 0; i < seeded.template_names; ++i) {
    template_name_scopes.pop_back();
  }
}

string CppAstParser::token_span_text_spaced(size_t start, size_t end) const
{
  return template_angle::token_span_text_spaced(tokens, start, end);
}

bool CppAstParser::token_text_needs_separator(const RecogToken & lhs,
                                              const RecogToken & rhs) const
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

bool CppAstParser::parse_translation_unit(CppAstNode & out)
{
  size_t start = pos;
  out = make_node(CppAstKind::translation_unit);
  while(!at_eof()) {
    CppAstNode declaration;
    if(!parse_declaration(declaration)) {
      if(error_msg.empty()) {
        set_error("expected declaration near " + token_label(peek()) +
                  " at token " + to_string(pos));
      }
      return false;
    }
    note_visible_names_after_declaration(declaration);
    out.children.push_back(std::move(declaration));
  }
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_compound_statement_fragment(CppAstNode & out)
{
  const size_t start = pos;
  if(!parse_compound_statement(out)) {
    if(error_msg.empty()) {
      set_error("expected compound-statement fragment near " + token_label(peek()));
    }
    pos = start;
    return false;
  }
  if(!at_eof()) {
    set_error("unexpected token after compound-statement fragment near " + token_label(peek()));
    pos = start;
    return false;
  }
  return true;
}

bool CppAstParser::parse_class_specifier_fragment(CppAstNode & out)
{
  const size_t start = pos;
  if(!parse_class_specifier(out)) {
    if(error_msg.empty()) {
      set_error("expected class-specifier fragment near " + token_label(peek()));
    }
    pos = start;
    return false;
  }
  if(peek().is_simple(OP_SEMICOLON)) {
    ++pos;
  }
  if(!at_eof()) {
    set_error("unexpected token after class-specifier fragment near " + token_label(peek()));
    pos = start;
    return false;
  }
  return true;
}

bool CppAstParser::parse_declaration(CppAstNode & out)
{
  size_t start = pos;
  std::string timing_location;
  if(file_timing::enabled()) {
    const SourceLocationTable * table = tokens.source_locations();
    if(table) {
      const RecogToken & token = peek();
      if(token.location_id != 0) {
        timing_location = table->describe(token.location_id);
      }
    }
  }
  file_timing::ScopedTimer decl_timer("parser.declaration", timing_location);
  const auto can_start_namespace_declaration_at = [&](size_t index) -> bool
  {
    return tokens.peek(index).is_simple(KW_NAMESPACE) ||
           (tokens.peek(index).is_simple(KW_INLINE) &&
            tokens.peek(index + 1).is_simple(KW_NAMESPACE));
  };
  const auto can_start_explicit_instantiation_at = [&](size_t index) -> bool
  {
    return (tokens.peek(index).is_simple(KW_TEMPLATE) &&
            !tokens.peek(index + 1).is_simple(OP_LT)) ||
           (tokens.peek(index).is_simple(KW_EXTERN) &&
            tokens.peek(index + 1).is_simple(KW_TEMPLATE));
  };
  const auto can_start_linkage_specification_at = [&](size_t index) -> bool
  {
    return tokens.peek(index).is_simple(KW_EXTERN) &&
           tokens.peek(index + 1).is_literal();
  };
  const auto try_core = [&](size_t core_start) -> bool
  {
    pos = core_start;
    if(parse_empty_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_namespace_declaration_at(core_start) &&
       parse_namespace_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_explicit_instantiation_at(core_start) &&
       parse_explicit_instantiation(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_linkage_specification_at(core_start) &&
       parse_linkage_specification(out)) {
      return true;
    }

    pos = core_start;
    if(parse_using_or_alias_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(parse_template_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(parse_class_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(parse_enum_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(parse_static_assert_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_deduction_guide_declaration() &&
       parse_deduction_guide_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_qualified_implicit_type_function_candidate() &&
       parse_qualified_special_member_definition(out)) {
      return true;
    }

    pos = core_start;
    if(can_start_qualified_implicit_type_function_candidate() &&
       parse_qualified_special_member_declaration(out)) {
      return true;
    }

    pos = core_start;
    if(parse_decl_specifier_leading_declaration(out)) {
      return true;
    }

    pos = core_start;
    return false;
  };

  const auto skip_gnu_extensions = [&](size_t cursor) -> size_t
  {
    while(is_gnu_extension_token(tokens.peek(cursor))) {
      ++cursor;
    }
    return cursor;
  };

  const size_t gnu_start = skip_gnu_extensions(start);
  if(try_core(gnu_start)) {
    return true;
  }

  pos = start;
  CppAstNode attributes = make_node(CppAstKind::specifier);
  if(!skip_attribute_specifier_seq(&attributes)) {
    pos = start;
    return false;
  }
  const size_t attribute_start = skip_gnu_extensions(pos);
  if(attribute_start != start && try_core(attribute_start)) {
    apply_leading_declaration_attributes(out, attributes);
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_explicit_instantiation(CppAstNode & out)
{
  size_t start = pos;
  const bool is_extern = consume_simple(KW_EXTERN);
  if(!consume_simple(KW_TEMPLATE)) {
    pos = start;
    return false;
  }

  CppAstNode declaration;
  if(!parse_explicit_instantiation_target(declaration)) {
    pos = start;
    return false;
  }

  out = make_node(is_extern ? CppAstKind::explicit_instantiation_declaration :
                              CppAstKind::explicit_instantiation_definition);
  out.children.push_back(std::move(declaration));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_explicit_instantiation_target(CppAstNode & out)
{
  size_t start = pos;
  if(parse_explicit_class_instantiation_target(out)) {
    return true;
  }

  pos = start;
  if(parse_class_declaration(out)) {
    return true;
  }

  pos = start;
  if(can_start_qualified_implicit_type_function_candidate() &&
     parse_qualified_special_member_declaration(out)) {
    return true;
  }

  pos = start;
  if(parse_decl_specifier_leading_declaration(out)) {
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_explicit_class_instantiation_target(CppAstNode & out)
{
  size_t start = pos;
  if(!is_class_key(peek())) {
    pos = start;
    return false;
  }

  RecogToken class_key_token = peek();
  ++pos;

  CppAstNode attributes = make_node(CppAstKind::specifier);
  if(!skip_attribute_specifier_seq(&attributes)) {
    pos = start;
    return false;
  }

  string name;
  cpp_decl::QualifiedName name_syntax;
  cpp_decl::TemplateIdSyntax template_id_syntax;
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  vector<CppAstNode> qualifier_type_syntaxes;
  if(!parse_qualified_name_text(name,
                                &name_syntax,
                                &template_id_syntax,
                                &qualifier_template_id_syntaxes,
                                &qualifier_type_syntaxes,
                                true,
                                false,
                                false,
                                true) ||
     template_id_syntax.name.name.empty() ||
     !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::class_forward_declaration, name);
  set_cppast_qualified_name_syntax(out, std::move(name_syntax));
  set_cppast_template_id_syntax(out, std::move(template_id_syntax));
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(
        out,
        std::move(qualifier_template_id_syntaxes));
  }
  if(!qualifier_type_syntaxes.empty()) {
    set_cppast_qualifier_type_syntaxes(out,
                                       std::move(qualifier_type_syntaxes));
  }
  out.children.push_back(make_token_node(CppAstKind::class_key,
                                         class_key_token));
  apply_leading_declaration_attributes(out, attributes);
  set_span(out, start);
  return true;
}

bool CppAstParser::can_start_decl_specifier_seq() const
{
  size_t cursor = 0;
  while(is_gnu_extension_token(peek(cursor))) {
    ++cursor;
  }

  const RecogToken & token = peek(cursor);
  const RecogToken & next = peek(cursor + 1);
  const bool known_value_template =
      is_known_value_template_parameter_identifier(token);
  const bool known_value = is_known_value_name_identifier(token);
  const bool known_template = is_known_template_name_identifier(token);
  const bool value_name_preferred =
      unqualified_identifier_prefers_value_name(token);
  const bool known_type_name =
      is_known_type_name_identifier(token) ||
      is_template_type_parameter_name(token);
  const bool named_candidate =
      token.is_simple(OP_COLON2) ||
      (token.is_identifier() &&
       (next.is_simple(OP_COLON2) ||
        ((known_template || !value_name_preferred) &&
         next.is_simple(OP_LT))));
  const bool result = is_decltype_token(token) ||
         (is_gnu_typeof_token(token) && next.is_simple(OP_LPAREN)) ||
         (token.is_identifier() && token.source == "_Atomic" &&
          next.is_simple(OP_LPAREN)) ||
         (token.is_identifier() && token.source == "_BitInt" &&
          next.is_simple(OP_LPAREN)) ||
         token.is_simple(KW_TYPENAME) ||
         is_cv_qualifier(token) ||
         is_simple_type_specifier(token) ||
         is_decl_specifier_keyword(token) ||
         is_gnu_float_type_specifier_identifier(token) ||
         is_gnu_int128_type_specifier_identifier(token) ||
         is_gnu_decl_specifier_identifier(token) ||
         (named_candidate && can_start_named_decl_specifier_seq()) ||
         (known_type_name &&
          !value_name_preferred &&
          !next.is_simple(OP_COLON2) &&
          !next.is_simple(OP_LT)) ||
         (token.is_identifier() &&
          !known_value_template &&
          !known_value &&
          (!next.is_simple(OP_COLON2)) &&
          (!next.is_simple(OP_LT)) &&
          (next.is_identifier() || is_cv_qualifier(next) ||
           next.is_simple(OP_STAR) || next.is_simple(OP_AMP) ||
           next.is_simple(OP_LAND)));
  if(parser_trace::enabled("parser.decl") &&
     (result || (token.is_identifier() &&
                 (next.is_identifier() ||
                  next.is_simple(OP_LT) ||
                  next.is_simple(OP_COLON2) ||
                  next.is_simple(OP_LPAREN))))) {
    std::ostringstream trace;
    trace << "can-start-decl-specifier-seq token=" << token_label(token)
          << " next=" << token_label(next)
          << " result=" << (result ? "yes" : "no")
          << " named-candidate=" << (named_candidate ? "yes" : "no")
          << " known-template=" << (known_template ? "yes" : "no")
          << " known-type=" << (known_type_name ? "yes" : "no")
          << " known-value-template=" << (known_value_template ? "yes" : "no")
          << " known-value=" << (known_value ? "yes" : "no")
          << " value-preferred=" << (value_name_preferred ? "yes" : "no");
    parser_trace::note("parser.decl", tokens, pos, trace.str());
  }
  return result;
}

bool CppAstParser::can_start_named_decl_specifier_seq() const
{
  const NamedDeclSpecifierSeqCacheKey cache_key =
      make_named_decl_specifier_seq_cache_key();
  const auto cached = named_decl_specifier_seq_cache.find(cache_key);
  if(cached != named_decl_specifier_seq_cache.end()) {
    return cached->second;
  }

  std::size_t end = pos;
  if(!scan_named_decl_specifier_seq_end(end)) {
    qualified_name_parser::QualifiedNameParseResult parsed;
    qualified_name_parser::UnqualifiedNameOptions options;
    const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
    options.allow_operator = false;
    options.allow_destructor = false;
    if(!qualified_name_parser::parse_qualified_name(tokens, pos,
                                                    lookup,
                                                    options,
                                                    parsed)) {
      named_decl_specifier_seq_cache[cache_key] = false;
      return false;
    }
    end = parsed.end;
  }

  const RecogToken & after = tokens.peek(end);
  const bool result = after.is_identifier() ||
      is_cv_qualifier(after) ||
      after.is_simple(OP_STAR) ||
      after.is_simple(OP_AMP) ||
      after.is_simple(OP_LAND);
  named_decl_specifier_seq_cache[cache_key] = result;
  return result;
}

bool CppAstParser::scan_named_decl_specifier_seq_end(size_t & end) const
{
  size_t cursor = pos;
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup(true);
  std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;

  if(tokens.peek(cursor).is_simple(OP_COLON2)) {
    ++cursor;
  }

  bool saw_component = false;
  while(true) {
    if(tokens.peek(cursor).is_simple(KW_TEMPLATE)) {
      ++cursor;
    }

    if(!tokens.peek(cursor).is_identifier()) {
      return false;
    }
    ++cursor;
    saw_component = true;

    if(tokens.peek(cursor).is_simple(OP_LT)) {
      arg_ranges.clear();
      if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                          cursor,
                                                          lookup,
                                                          cursor,
                                                          arg_ranges)) {
        return false;
      }
    }

    if(!tokens.peek(cursor).is_simple(OP_COLON2)) {
      end = cursor;
      return saw_component;
    }
    ++cursor;
  }
}

bool CppAstParser::can_start_attributed_decl_specifier_seq()
{
  const size_t start = pos;
  if(!skip_attribute_specifier_seq() || pos == start) {
    pos = start;
    return false;
  }
  const bool result = can_start_decl_specifier_seq();
  pos = start;
  return result;
}

bool CppAstParser::can_start_type_id() const
{
  const RecogToken & token = peek();
  const bool known_value_template =
      is_known_value_template_parameter_identifier(token);
  const bool known_value = is_known_value_name_identifier(token);
  return is_decltype_token(token) ||
         (is_gnu_typeof_token(token) && peek(1).is_simple(OP_LPAREN)) ||
         (token.is_identifier() && token.source == "_Atomic" &&
          peek(1).is_simple(OP_LPAREN)) ||
         (token.is_identifier() && token.source == "_BitInt" &&
          peek(1).is_simple(OP_LPAREN)) ||
         token.is_simple(KW_TYPENAME) ||
         token.is_simple(KW_TEMPLATE) ||
         is_class_key(token) ||
         token.is_simple(KW_ENUM) ||
         is_cv_qualifier(token) ||
         is_simple_type_specifier(token) ||
         is_gnu_attribute_name_token(token) ||
         is_gnu_int128_type_specifier_identifier(token) ||
         is_known_type_name_identifier(token) ||
         is_template_type_parameter_name(token) ||
         token.is_simple(OP_COLON2) ||
         (token.is_identifier() &&
          !known_value_template &&
          !known_value &&
          (peek(1).is_simple(OP_COLON2) ||
           peek(1).is_simple(OP_LT) ||
           is_cv_qualifier(peek(1)) ||
           peek(1).is_simple(OP_STAR) ||
           peek(1).is_simple(OP_AMP) ||
           peek(1).is_simple(OP_LAND)));
}

bool CppAstParser::qualified_name_span_prefers_expression(std::size_t begin,
                                                          std::size_t end) const
{
  if(begin >= end) {
    return false;
  }

  const template_angle_lookup::ScopedNameLookup lookup =
      make_template_angle_lookup();
  qualified_name_parser::QualifiedNameParseResult parsed;
  qualified_name_parser::UnqualifiedNameOptions options;
  options.allow_operator = false;
  if(!qualified_name_parser::parse_qualified_name(tokens,
                                                  begin,
                                                  lookup,
                                                  options,
                                                  parsed) ||
     parsed.end != end ||
     parsed.name_kind != qualified_name_parser::UNQ_COMPONENT) {
    return false;
  }

  const std::pair<std::size_t, std::size_t> name_range =
      parsed.name_has_template_suffix ?
          parsed.name_template_head_component :
          parsed.name_component;
  if(name_range.first >= name_range.second) {
    return false;
  }
  const std::string name =
      primary_name_text(token_span_text_spaced(name_range.first,
                                               name_range.second));
  if(name.empty()) {
    return false;
  }

  if(!parsed.rooted && parsed.qualifiers.empty()) {
    const RecogToken & token = tokens.peek(name_range.first);
    const bool known_type =
        is_known_type_name_identifier(token) ||
        is_template_type_parameter_name(token);
    const bool known_template = is_known_template_name_identifier(token);
    const bool known_value_template =
        is_known_value_template_parameter_identifier(token);
    const bool known_value = is_known_value_name_identifier(token);
    return (known_template || known_value_template || known_value) &&
           !known_type;
  }

  std::string qualifier_text = parsed.rooted ? std::string("::") : std::string();
  for(std::size_t i = 0; i < parsed.qualifiers.size(); ++i) {
    if(!qualifier_text.empty() && qualifier_text != "::") {
      qualifier_text += "::";
    }
    qualifier_text += token_span_text_spaced(parsed.qualifiers[i].first,
                                             parsed.qualifiers[i].second);
  }
  const std::string namespace_key =
      resolve_visible_namespace_scope_key(qualifier_text);
  const auto qualifier_names_current_namespace =
      [&]() -> bool
      {
        const std::string normalized = normalized_lookup_name(qualifier_text);
        const std::string current_key = current_namespace_path_key();
        if(normalized.empty() || current_key.empty()) {
          return false;
        }
        if(parsed.rooted) {
          return normalized == current_key;
        }

        for(std::size_t depth = namespace_path_stack.size(); ; --depth) {
          std::string candidate;
          for(std::size_t i = 0; i < depth; ++i) {
            if(!candidate.empty()) {
              candidate += "::";
            }
            candidate += namespace_path_stack[i];
          }
          if(!candidate.empty()) {
            candidate += "::";
          }
          candidate += normalized;
          if(candidate == current_key) {
            return true;
          }
          if(depth == 0) {
            break;
          }
        }
        return false;
      };
  const bool active_namespace_qualifier = qualifier_names_current_namespace();
  if(namespace_key.empty() ||
     (!namespace_scope_exists(namespace_key) && !active_namespace_qualifier)) {
    return false;
  }

  const RecogToken & token = tokens.peek(name_range.first);
  const bool scoped_known_type =
      is_known_type_name_identifier(token) ||
      is_template_type_parameter_name(token);
  const bool scoped_known_template = is_known_template_name_identifier(token);
  const bool scoped_known_value_template =
      is_known_value_template_parameter_identifier(token);
  const bool scoped_known_value = is_known_value_name_identifier(token);

  const auto type_found = namespace_type_name_scopes.find(namespace_key);
  bool known_type =
      type_found != namespace_type_name_scopes.end() &&
      type_found->second.count(name) != 0;
  const auto template_found = namespace_template_name_scopes.find(namespace_key);
  bool known_template =
      template_found != namespace_template_name_scopes.end() &&
      template_found->second.count(name) != 0;
  const auto value_found = namespace_value_name_scopes.find(namespace_key);
  bool known_value =
      value_found != namespace_value_name_scopes.end() &&
      value_found->second.count(name) != 0;
  if(active_namespace_qualifier) {
    const std::size_t namespace_scope_index = namespace_path_stack.size();
    known_type =
        known_type ||
        (namespace_scope_index < type_name_scopes.size() &&
         type_name_scopes[namespace_scope_index].count(name) != 0);
    known_template =
        known_template ||
        (namespace_scope_index < template_name_scopes.size() &&
         template_name_scopes[namespace_scope_index].count(name) != 0);
    known_value =
        known_value ||
        (namespace_scope_index < value_name_scopes.size() &&
         value_name_scopes[namespace_scope_index].count(name) != 0);
  }

  if(known_type || known_template || known_value) {
    return (known_template || known_value) && !known_type;
  }

  return (scoped_known_template || scoped_known_value_template || scoped_known_value) &&
         !scoped_known_type;
}

bool CppAstParser::qualified_name_span_names_known_type(std::size_t begin,
                                                        std::size_t end) const
{
  if(begin >= end) {
    return false;
  }

  const template_angle_lookup::ScopedNameLookup lookup =
      make_template_angle_lookup();
  qualified_name_parser::QualifiedNameParseResult parsed;
  qualified_name_parser::UnqualifiedNameOptions options;
  options.allow_operator = false;
  if(!qualified_name_parser::parse_qualified_name(tokens,
                                                  begin,
                                                  lookup,
                                                  options,
                                                  parsed) ||
     parsed.end != end ||
     parsed.name_kind != qualified_name_parser::UNQ_COMPONENT) {
    return false;
  }

  const std::pair<std::size_t, std::size_t> name_range =
      parsed.name_has_template_suffix ?
          parsed.name_template_head_component :
          parsed.name_component;
  if(name_range.first >= name_range.second) {
    return false;
  }
  const std::string name =
      primary_name_text(token_span_text_spaced(name_range.first,
                                               name_range.second));
  if(name.empty()) {
    return false;
  }

  const RecogToken & token = tokens.peek(name_range.first);
  const bool scoped_known_type =
      is_known_type_name_identifier(token) ||
      is_template_type_parameter_name(token);
  if(!parsed.rooted && parsed.qualifiers.empty()) {
    return scoped_known_type;
  }

  std::string qualifier_text = parsed.rooted ? std::string("::") : std::string();
  for(std::size_t i = 0; i < parsed.qualifiers.size(); ++i) {
    if(!qualifier_text.empty() && qualifier_text != "::") {
      qualifier_text += "::";
    }
    qualifier_text += token_span_text_spaced(parsed.qualifiers[i].first,
                                             parsed.qualifiers[i].second);
  }
  const std::string namespace_key =
      resolve_visible_namespace_scope_key(qualifier_text);
  const auto qualifier_names_current_namespace =
      [&]() -> bool
      {
        const std::string normalized = normalized_lookup_name(qualifier_text);
        const std::string current_key = current_namespace_path_key();
        if(normalized.empty() || current_key.empty()) {
          return false;
        }
        if(parsed.rooted) {
          return normalized == current_key;
        }

        for(std::size_t depth = namespace_path_stack.size(); ; --depth) {
          std::string candidate;
          for(std::size_t i = 0; i < depth; ++i) {
            if(!candidate.empty()) {
              candidate += "::";
            }
            candidate += namespace_path_stack[i];
          }
          if(!candidate.empty()) {
            candidate += "::";
          }
          candidate += normalized;
          if(candidate == current_key) {
            return true;
          }
          if(depth == 0) {
            break;
          }
        }
        return false;
      };
  const bool active_namespace_qualifier = qualifier_names_current_namespace();
  if(namespace_key.empty() ||
     (!namespace_scope_exists(namespace_key) && !active_namespace_qualifier)) {
    return scoped_known_type;
  }

  const auto type_found = namespace_type_name_scopes.find(namespace_key);
  bool known_type =
      type_found != namespace_type_name_scopes.end() &&
      type_found->second.count(name) != 0;
  if(active_namespace_qualifier) {
    const std::size_t namespace_scope_index = namespace_path_stack.size();
    known_type =
        known_type ||
        (namespace_scope_index < type_name_scopes.size() &&
         type_name_scopes[namespace_scope_index].count(name) != 0);
  }

  return known_type || scoped_known_type;
}

bool CppAstParser::qualified_template_id_span_has_head_expression_lookup(
    std::size_t begin,
    std::size_t end) const
{
  if(begin >= end) {
    return false;
  }

  const template_angle_lookup::ScopedNameLookup lookup =
      make_template_angle_lookup();
  for(std::size_t open = begin; open < end; ++open) {
    if(!tokens.peek(open).is_simple(OP_LT)) {
      continue;
    }

    std::size_t suffix_end = open;
    std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
    if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                        open,
                                                        lookup,
                                                        suffix_end,
                                                        arg_ranges) ||
       suffix_end != end) {
      continue;
    }

    if(qualified_name_span_prefers_expression(begin, open) ||
       (!qualified_name_span_names_known_type(begin, open) &&
        peek().is_simple(OP_LPAREN))) {
      return true;
    }
  }
  return false;
}

bool CppAstParser::parenthesized_type_id_prefers_expression(
    const CppAstNode & type_id) const
{
  if(qualified_name_span_prefers_expression(type_id.token_start,
                                            type_id.token_end)) {
    return true;
  }
  if(qualified_template_id_span_has_head_expression_lookup(type_id.token_start,
                                                           type_id.token_end)) {
    return true;
  }

  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.empty() ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq ||
     type_id.children[0].children.size() != 1 ||
     type_id.children[0].children[0].kind != CppAstKind::type_name) {
    return false;
  }

  const CppAstNode & head = type_id.children[0].children[0];
  const bool function_like_abstract_declarator =
      type_id_has_function_style_abstract_declarator(type_id);
  if(qualified_template_id_span_has_head_expression_lookup(head.token_start,
                                                           head.token_end)) {
    return true;
  }
  const cpp_decl::QualifiedName * head_syntax = cppast_qualified_name_syntax(head);
  if(head_syntax &&
     (head_syntax->rooted || !head_syntax->qualifiers.empty()) &&
     !function_like_abstract_declarator) {
    return false;
  }
  for(std::size_t i = type_id.token_start; i < type_id.token_end; ++i) {
    if(tokens.peek(i).is_simple(OP_COLON2) && !function_like_abstract_declarator) {
      return false;
    }
  }
  return qualified_name_span_prefers_expression(head.token_start, head.token_end);
}

bool CppAstParser::can_start_id_expression() const
{
  const RecogToken & token = peek();
  return token.is_identifier() ||
         is_decltype_token(token) ||
         token.is_simple(OP_COLON2) ||
         token.is_simple(KW_TEMPLATE) ||
         token.is_simple(OP_COMPL) ||
         token.is_simple(KW_OPERATOR);
}

bool CppAstParser::can_start_primary_expression() const
{
  const RecogToken & token = peek();
  return token.is_simple(OP_LSQUARE) ||
         token.is_simple(OP_LBRACE) ||
         token.is_literal() ||
         can_start_id_expression() ||
         token.is_simple(KW_TRUE) ||
         token.is_simple(KW_FALSE) ||
         token.is_simple(KW_NULLPTR) ||
         token.is_simple(KW_THIS) ||
         token.is_simple(OP_LPAREN);
}

bool CppAstParser::can_start_block_declaration()
{
  size_t cursor = pos;
  while(is_gnu_extension_token(tokens.peek(cursor))) {
    ++cursor;
  }
  const RecogToken & token = tokens.peek(cursor);
  return token.is_simple(OP_SEMICOLON) ||
         token.is_simple(KW_NAMESPACE) ||
         token.is_simple(KW_USING) ||
         token.is_simple(KW_TEMPLATE) ||
         is_class_key(token) ||
         token.is_simple(KW_ENUM) ||
         token.is_simple(KW_STATIC_ASSERT) ||
         can_start_decl_specifier_seq() ||
         can_start_attributed_decl_specifier_seq();
}

bool CppAstParser::can_start_range_declaration()
{
  size_t cursor = pos;
  while(is_gnu_extension_token(tokens.peek(cursor))) {
    ++cursor;
  }
  if(cursor != pos) {
    return tokens.peek(cursor).is_simple(KW_USING) ||
           tokens.peek(cursor).is_simple(KW_TEMPLATE) ||
           is_class_key(tokens.peek(cursor)) ||
           tokens.peek(cursor).is_simple(KW_ENUM) ||
           can_start_decl_specifier_seq();
  }
  return can_start_decl_specifier_seq() ||
         can_start_attributed_decl_specifier_seq();
}

bool CppAstParser::is_current_class_name_identifier(const RecogToken & token) const
{
  return token.is_identifier() && !class_name_stack.empty() &&
         token.source == class_name_stack.back();
}

bool CppAstParser::can_start_special_member_candidate() const
{
  const RecogToken & token = peek();
  return token.is_simple(KW_EXPLICIT) ||
         is_current_class_name_identifier(token) ||
         token.is_simple(OP_COMPL) ||
         token.is_simple(KW_OPERATOR) ||
         is_member_function_specifier(token);
}

bool CppAstParser::find_template_parameter_clause_end(size_t template_pos,
                                                      size_t & end) const
{
  if(!tokens.peek(template_pos).is_simple(KW_TEMPLATE) ||
     !tokens.peek(template_pos + 1).is_simple(OP_LT)) {
    return false;
  }

  int angle_depth = 1;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t cursor = template_pos + 2; !tokens.peek(cursor).is_eof(); ++cursor) {
    const RecogToken & token = tokens.peek(cursor);
    if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
      continue;
    }
    if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
      continue;
    }
    if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
      continue;
    }
    if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        return false;
      }
      --bracket_depth;
      continue;
    }
    if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
      continue;
    }
    if(token.is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        return false;
      }
      --brace_depth;
      continue;
    }
    if(paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(token.is_simple(OP_LT)) {
      ++angle_depth;
      continue;
    }
    if(token.is_close_angle_bracket()) {
      --angle_depth;
      if(angle_depth == 0) {
        end = cursor + 1;
        return true;
      }
      if(angle_depth < 0) {
        return false;
      }
    }
  }
  return false;
}

bool CppAstParser::can_start_unqualified_implicit_type_function_candidate_at(
    size_t cursor) const
{
  for(;;) {
    const size_t prefix_start = cursor;

    while(is_gnu_attribute_name_token(tokens.peek(cursor))) {
      ++cursor;
      if(!tokens.peek(cursor).is_simple(OP_LPAREN)) {
        return false;
      }
      int depth = 0;
      do {
        const RecogToken & token = tokens.peek(cursor);
        if(token.is_eof()) {
          return false;
        }
        if(token.is_simple(OP_LPAREN)) {
          ++depth;
        }
        else if(token.is_simple(OP_RPAREN)) {
          --depth;
        }
        ++cursor;
      } while(depth > 0);
    }

    while(tokens.peek(cursor).is_simple(OP_LSQUARE) &&
          tokens.peek(cursor + 1).is_simple(OP_LSQUARE)) {
      cursor += 2;
      int depth = 1;
      while(depth > 0) {
        const RecogToken & token = tokens.peek(cursor);
        if(token.is_eof()) {
          return false;
        }
        if(token.is_simple(OP_LSQUARE) && tokens.peek(cursor + 1).is_simple(OP_LSQUARE)) {
          cursor += 2;
          ++depth;
          continue;
        }
        if(token.is_simple(OP_RSQUARE) && tokens.peek(cursor + 1).is_simple(OP_RSQUARE)) {
          cursor += 2;
          --depth;
          continue;
        }
        ++cursor;
      }
    }

    if(tokens.peek(cursor).is_simple(KW_ALIGNAS)) {
      ++cursor;
      if(!tokens.peek(cursor).is_simple(OP_LPAREN)) {
        return false;
      }
      int depth = 0;
      do {
        const RecogToken & token = tokens.peek(cursor);
        if(token.is_eof()) {
          return false;
        }
        if(token.is_simple(OP_LPAREN)) {
          ++depth;
        }
        else if(token.is_simple(OP_RPAREN)) {
          --depth;
        }
        ++cursor;
      } while(depth > 0);
      continue;
    }

    if(tokens.peek(cursor).is_simple(KW_EXPLICIT)) {
      ++cursor;
      if(tokens.peek(cursor).is_simple(OP_LPAREN)) {
        int depth = 0;
        do {
          const RecogToken & token = tokens.peek(cursor);
          if(token.is_eof()) {
            return false;
          }
          if(token.is_simple(OP_LPAREN)) {
            ++depth;
          }
          else if(token.is_simple(OP_RPAREN)) {
            --depth;
          }
          ++cursor;
        } while(depth > 0);
      }
      continue;
    }

    if(is_member_function_specifier(tokens.peek(cursor))) {
      ++cursor;
      continue;
    }

    if(cursor == prefix_start) {
      break;
    }
  }

  return is_current_class_name_identifier(tokens.peek(cursor)) ||
         tokens.peek(cursor).is_simple(OP_COMPL) ||
         tokens.peek(cursor).is_simple(KW_OPERATOR);
}

bool CppAstParser::can_start_unqualified_implicit_type_function_candidate() const
{
  return can_start_unqualified_implicit_type_function_candidate_at(pos);
}

bool CppAstParser::can_start_template_special_member_candidate() const
{
  size_t after_parameters = pos;
  if(!find_template_parameter_clause_end(pos, after_parameters)) {
    return true;
  }
  return can_start_unqualified_implicit_type_function_candidate_at(after_parameters);
}

bool CppAstParser::can_start_qualified_implicit_type_function_candidate() const
{
  const auto skip_prefix = [&](size_t & cursor) -> bool
  {
    for(;;) {
      bool advanced = false;
      while(is_gnu_attribute_name_token(tokens.peek(cursor))) {
        ++cursor;
        if(!tokens.peek(cursor).is_simple(OP_LPAREN)) {
          return false;
        }
        int depth = 0;
        do {
          const RecogToken & token = tokens.peek(cursor);
          if(token.is_eof()) {
            return false;
          }
          if(token.is_simple(OP_LPAREN)) {
            ++depth;
          }
          else if(token.is_simple(OP_RPAREN)) {
            --depth;
          }
          ++cursor;
        } while(depth > 0);
        advanced = true;
      }

      while(tokens.peek(cursor).is_simple(OP_LSQUARE) &&
            tokens.peek(cursor + 1).is_simple(OP_LSQUARE)) {
        cursor += 2;
        int depth = 1;
        while(depth > 0) {
          const RecogToken & token = tokens.peek(cursor);
          if(token.is_eof()) {
            return false;
          }
          if(token.is_simple(OP_LSQUARE) && tokens.peek(cursor + 1).is_simple(OP_LSQUARE)) {
            cursor += 2;
            ++depth;
            continue;
          }
          if(token.is_simple(OP_RSQUARE) && tokens.peek(cursor + 1).is_simple(OP_RSQUARE)) {
            cursor += 2;
            --depth;
            continue;
          }
          ++cursor;
        }
        advanced = true;
      }

      if(tokens.peek(cursor).is_simple(KW_ALIGNAS)) {
        ++cursor;
        if(!tokens.peek(cursor).is_simple(OP_LPAREN)) {
          return false;
        }
        int depth = 0;
        do {
          const RecogToken & token = tokens.peek(cursor);
          if(token.is_eof()) {
            return false;
          }
          if(token.is_simple(OP_LPAREN)) {
            ++depth;
          }
          else if(token.is_simple(OP_RPAREN)) {
            --depth;
          }
          ++cursor;
        } while(depth > 0);
        advanced = true;
        continue;
      }

      if(tokens.peek(cursor).is_simple(KW_EXPLICIT)) {
        ++cursor;
        if(tokens.peek(cursor).is_simple(OP_LPAREN)) {
          int depth = 0;
          do {
            const RecogToken & token = tokens.peek(cursor);
            if(token.is_eof()) {
              return false;
            }
            if(token.is_simple(OP_LPAREN)) {
              ++depth;
            }
            else if(token.is_simple(OP_RPAREN)) {
              --depth;
            }
            ++cursor;
          } while(depth > 0);
        }
        advanced = true;
        continue;
      }

      if(is_member_function_specifier(tokens.peek(cursor))) {
        ++cursor;
        advanced = true;
        continue;
      }

      if(!advanced) {
        break;
      }
    }
    return true;
  };

  const auto parse_component = [&](size_t & cursor, std::string & base_name) -> bool
  {
    if(tokens.peek(cursor).is_simple(KW_TEMPLATE)) {
      ++cursor;
    }
    if(!tokens.peek(cursor).is_identifier()) {
      return false;
    }
    base_name = tokens.peek(cursor).source;
    ++cursor;
    if(tokens.peek(cursor).is_simple(OP_LT)) {
      const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
      std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
      if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                          cursor,
                                                          lookup,
                                                          cursor,
                                                          arg_ranges)) {
        return false;
      }
    }
    return true;
  };

  size_t cursor = pos;
  if(!skip_prefix(cursor)) {
    return false;
  }
  if(tokens.peek(cursor).is_simple(OP_COLON2)) {
    ++cursor;
  }

  std::string owner_name;
  if(!parse_component(cursor, owner_name) || !tokens.peek(cursor).is_simple(OP_COLON2)) {
    return false;
  }

  while(tokens.peek(cursor).is_simple(OP_COLON2)) {
    ++cursor;
    if(tokens.peek(cursor).is_simple(KW_OPERATOR)) {
      return true;
    }
    if(tokens.peek(cursor).is_simple(OP_COMPL)) {
      ++cursor;
      return tokens.peek(cursor).is_identifier() && tokens.peek(cursor).source == owner_name;
    }

    std::string next_name;
    if(!parse_component(cursor, next_name)) {
      return false;
    }
    if(tokens.peek(cursor).is_simple(OP_LPAREN)) {
      return next_name == owner_name;
    }
    owner_name = next_name;
  }

  return false;
}

bool CppAstParser::can_start_deduction_guide_declaration() const
{
  size_t cursor = pos;
  while(is_gnu_extension_token(tokens.peek(cursor))) {
    ++cursor;
  }

  if(tokens.peek(cursor).is_simple(KW_EXPLICIT)) {
    ++cursor;
    if(tokens.peek(cursor).is_simple(OP_LPAREN)) {
      int depth = 0;
      do {
        const RecogToken & token = tokens.peek(cursor);
        if(token.is_eof()) {
          return false;
        }
        if(token.is_simple(OP_LPAREN)) {
          ++depth;
        } else if(token.is_simple(OP_RPAREN)) {
          --depth;
        }
        ++cursor;
      } while(depth > 0);
    }
  }

  if(!tokens.peek(cursor).is_identifier()) {
    return false;
  }
  ++cursor;
  if(!tokens.peek(cursor).is_simple(OP_LPAREN)) {
    return false;
  }

  int paren_depth = 0;
  int brace_depth = 0;
  int bracket_depth = 0;
  for(;; ++cursor) {
    const RecogToken & token = tokens.peek(cursor);
    if(token.is_eof()) {
      return false;
    }

    const bool top_level =
        paren_depth == 0 && brace_depth == 0 && bracket_depth == 0;
    if(top_level && token.is_simple(OP_SEMICOLON)) {
      return false;
    }
    if(top_level && token.is_simple(OP_ARROW)) {
      return true;
    }
    if(top_level &&
       (token.is_simple(OP_LBRACE) ||
        token.is_simple(OP_COLON) ||
        token.is_simple(OP_ASS))) {
      return false;
    }

    if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
    } else if(token.is_simple(OP_RPAREN)) {
      if(paren_depth > 0) {
        --paren_depth;
      }
    } else if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
    } else if(token.is_simple(OP_RBRACE)) {
      if(brace_depth > 0) {
        --brace_depth;
      }
    } else if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
    } else if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
    }
  }
}

bool CppAstParser::can_start_statement_as_label() const
{
  return (peek().is_identifier() && peek(1).is_simple(OP_COLON)) ||
         peek().is_simple(KW_CASE) ||
         peek().is_simple(KW_DEFAULT);
}

bool CppAstParser::can_start_structured_binding_declarator() const
{
  return peek().is_simple(OP_LSQUARE) ||
         ((peek().is_simple(OP_AMP) || peek().is_simple(OP_LAND)) &&
          peek(1).is_simple(OP_LSQUARE));
}

bool CppAstParser::declarator_has_parameter_clause(const CppAstNode & node) const
{
  if(node.kind == CppAstKind::parameter_clause) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(declarator_has_parameter_clause(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool CppAstParser::type_id_has_function_style_abstract_declarator(
    const CppAstNode & type_id) const
{
  return type_id.kind == CppAstKind::type_id &&
         type_id.children.size() == 2 &&
         type_id.children[1].kind == CppAstKind::abstract_declarator &&
         declarator_has_parameter_clause(type_id.children[1]);
}

bool CppAstParser::parse_empty_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_SEMICOLON)) {
    return false;
  }

  out = make_node(CppAstKind::empty_declaration);
  set_span(out, start);
  return true;
}

bool CppAstParser::skip_gnu_attribute_specifier_seq(CppAstNode * annotated)
{
  while(is_gnu_attribute_name_token(peek())) {
    size_t start = pos;
    ++pos;
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }
    note_attribute_specifier(annotated, start, pos);
  }
  return true;
}

bool CppAstParser::skip_standard_attribute_specifier(CppAstNode * annotated)
{
  if(!peek().is_simple(OP_LSQUARE) || !peek(1).is_simple(OP_LSQUARE)) {
    return false;
  }

  const size_t start = pos;
  pos += 2;
  int depth = 1;
  while(!at_eof()) {
    if(peek().is_simple(OP_LSQUARE) && peek(1).is_simple(OP_LSQUARE)) {
      ++depth;
      pos += 2;
      continue;
    }
    if(peek().is_simple(OP_RSQUARE) && peek(1).is_simple(OP_RSQUARE)) {
      --depth;
      pos += 2;
      if(depth == 0) {
        note_attribute_specifier(annotated, start, pos);
        return true;
      }
      continue;
    }
    ++pos;
  }

  return false;
}

void CppAstParser::note_attribute_specifier(CppAstNode * annotated,
                                            std::size_t start,
                                            std::size_t end)
{
  if(!annotated) {
    return;
  }

  const std::string text = token_span_text_spaced(start, end);
  if(text.find("__using_if_exists__") != std::string::npos) {
    annotated->has_using_if_exists = true;
  }
  if(text.find("exclude_from_explicit_instantiation") != std::string::npos) {
    annotated->has_exclude_from_explicit_instantiation = true;
  }
  if(text.find("no_unique_address") != std::string::npos) {
    annotated->has_no_unique_address = true;
  }

  bool collecting_abi_tag = false;
  int abi_tag_paren_depth = 0;
  for(std::size_t i = start; i < end; ++i) {
    const RecogToken & token = tokens.peek(i);
    if(!collecting_abi_tag) {
      if(is_abi_tag_attribute_name(token)) {
        collecting_abi_tag = true;
        abi_tag_paren_depth = 0;
      }
      continue;
    }
    if(token.is_simple(OP_LPAREN)) {
      ++abi_tag_paren_depth;
      continue;
    }
    if(token.is_simple(OP_RPAREN)) {
      if(abi_tag_paren_depth > 0) {
        --abi_tag_paren_depth;
      }
      if(abi_tag_paren_depth == 0) {
        collecting_abi_tag = false;
      }
      continue;
    }
    if(abi_tag_paren_depth > 0 && is_string_literal_attribute_token(token)) {
      append_unique_cppast_text(annotated->abi_tags, abi_tag_literal_value(token.source));
    }
  }
}

bool CppAstParser::skip_attribute_specifier_seq(CppAstNode * annotated)
{
  while(true) {
    size_t start = pos;
    if(!skip_gnu_attribute_specifier_seq(annotated)) {
      pos = start;
      return false;
    }
    if(pos != start) {
      continue;
    }

    if(peek().is_simple(OP_LSQUARE) && peek(1).is_simple(OP_LSQUARE)) {
      if(!skip_standard_attribute_specifier(annotated)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(KW_ALIGNAS)) {
      const std::size_t alignas_start = pos;
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      if(annotated &&
         pos >= alignas_start + 2 &&
         tokens.peek(alignas_start + 1).is_simple(OP_LPAREN)) {
        const std::size_t operand_start = alignas_start + 2;
        const std::size_t operand_end = pos - 1;
        CppAstNode operand;
        bool have_operand = false;
        const std::size_t saved_pos = pos;
        if(operand_start < operand_end) {
          const RecogToken & operand_first = tokens.peek(operand_start);
          const bool known_value_operand =
              operand_first.is_identifier() &&
              (is_known_value_template_parameter_identifier(operand_first) ||
               is_known_value_name_identifier(operand_first)) &&
              !is_template_type_parameter_name(operand_first) &&
              !is_known_type_name_identifier(operand_first) &&
              !is_known_template_name_identifier(operand_first);
          const bool prefer_expression =
              known_value_operand ||
              is_alignas_expression_preferred_start(tokens, operand_start);
          if(prefer_expression) {
            pos = operand_start;
            CppAstNode expr;
            if(parse_assignment_expression(expr) && pos == operand_end) {
              operand = expr;
              have_operand = true;
            }
          }
          if(!have_operand) {
            pos = operand_start;
            CppAstNode type_id;
            if(parse_type_id(type_id) && pos == operand_end) {
              operand = type_id;
              have_operand = true;
            }
          }
          if(!have_operand && !prefer_expression) {
            pos = operand_start;
            CppAstNode expr;
            if(parse_assignment_expression(expr) && pos == operand_end) {
              operand = expr;
              have_operand = true;
            }
          }
        }
        pos = saved_pos;
        append_cppast_alignment_specifier(
            *annotated,
            token_span_text_spaced(operand_start, operand_end),
            have_operand ? &operand : nullptr);
      }
      continue;
    }

    break;
  }

  return true;
}

bool CppAstParser::skip_trailing_declarator_extensions(CppAstNode * annotated)
{
  while(true) {
    size_t start = pos;
    if(!skip_attribute_specifier_seq(annotated)) {
      pos = start;
      return false;
    }
    if(pos != start) {
      continue;
    }

    if(is_gnu_asm_token(peek())) {
      const std::size_t asm_start = pos;
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      if(annotated) {
        const std::string label =
            gnu_asm_label_literal_value(tokens, asm_start, pos);
        if(!label.empty()) {
          annotated->asm_label = label;
        }
      }
      continue;
    }

    break;
  }

  return true;
}

void apply_trailing_declarator_extensions(CppAstNode & target,
                                          const CppAstNode & extensions)
{
  if(!extensions.asm_label.empty()) {
    target.asm_label = extensions.asm_label;
  }
  append_cppast_abi_tags(target.abi_tags, extensions);
  append_cppast_alignment_specifiers(target, extensions);
}

bool CppAstParser::parse_namespace_declaration(CppAstNode & out)
{
  size_t start = pos;
  bool is_inline = consume_simple(KW_INLINE);
  if(!consume_simple(KW_NAMESPACE)) {
    pos = start;
    return false;
  }

  if(!skip_attribute_specifier_seq()) {
    pos = start;
    return false;
  }

  string name;
  if(peek().is_identifier()) {
    name = peek().source;
    ++pos;
  }

  if(!skip_attribute_specifier_seq()) {
    pos = start;
    return false;
  }

  if(consume_simple(OP_ASS)) {
    string target;
    cpp_decl::QualifiedName target_syntax;
    if(name.empty() || !parse_qualified_name_text(target, &target_syntax) ||
       !consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::namespace_alias_definition, name);
    CppAstNode target_node = make_node(CppAstKind::target, target);
    set_cppast_qualified_name_syntax(target_node, std::move(target_syntax));
    out.children.push_back(std::move(target_node));
    set_span(out, start);
    return true;
  }

  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::namespace_definition, name.empty() ? "<unnamed>" : name);
  if(is_inline) {
    out.children.push_back(make_node(CppAstKind::inline_node));
  }

  push_namespace_name_scopes(name, is_inline);
  while(!at_eof() && !peek().is_simple(OP_RBRACE)) {
    CppAstNode declaration;
    if(!parse_declaration(declaration)) {
      if(error_msg.empty()) {
        set_error("expected declaration in namespace " +
                  (name.empty() ? string("<unnamed>") : name) +
                  " near " + token_label(peek()) +
                  " at token " + to_string(pos) +
                  " [template-scope-depth=" +
                  to_string(template_type_parameter_scopes.size()) + "]");
      }
      pop_namespace_name_scopes(false);
      pos = start;
      return false;
    }
    note_visible_names_after_declaration(declaration);
    out.children.push_back(std::move(declaration));
  }

  if(!consume_simple(OP_RBRACE)) {
    pop_namespace_name_scopes(false);
    pos = start;
    return false;
  }

  pop_namespace_name_scopes(true);
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_linkage_specification(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_EXTERN)) {
    return false;
  }
  if(!peek().is_literal()) {
    pos = start;
    return false;
  }
  const string linkage = peek().source == "\"C\"" ? "C" :
                         peek().source == "\"C++\"" ? "C++" : string();
  if(linkage.empty()) {
    pos = start;
    return false;
  }
  ++pos;

  out = make_node(CppAstKind::linkage_specification, linkage);
  if(consume_simple(OP_LBRACE)) {
    out.linkage_has_braces = true;
    while(!peek().is_simple(OP_RBRACE)) {
      CppAstNode declaration;
      if(!parse_declaration(declaration)) {
        set_error("expected declaration inside linkage-specification near " +
                  token_label(peek()) + " at token " + to_string(pos));
        pos = start;
        return false;
      }
      note_visible_names_after_declaration(declaration);
      out.children.push_back(std::move(declaration));
    }
    if(!consume_simple(OP_RBRACE)) {
      pos = start;
      return false;
    }
  } else {
    CppAstNode declaration;
    if(!parse_declaration(declaration)) {
      set_error("expected declaration inside linkage-specification near " +
                token_label(peek()) + " at token " + to_string(pos));
      pos = start;
      return false;
    }
    note_visible_names_after_declaration(declaration);
    out.children.push_back(std::move(declaration));
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_using_or_alias_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_USING)) {
    pos = start;
    return false;
  }

  if(consume_simple(KW_NAMESPACE)) {
    string target;
    cpp_decl::QualifiedName target_syntax;
    if(!parse_qualified_name_text(target, &target_syntax) || !consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::using_directive);
    CppAstNode target_node = make_node(CppAstKind::target, target);
    set_cppast_qualified_name_syntax(target_node, std::move(target_syntax));
    out.children.push_back(std::move(target_node));
    set_span(out, start);
    return true;
  }

  if(peek().is_identifier()) {
    string alias = peek().source;
    ++pos;
    if(!skip_attribute_specifier_seq()) {
      pos = start;
      return false;
    }
    if(consume_simple(OP_ASS)) {
      CppAstNode type_id;
      if(!parse_type_id(type_id) || !consume_simple(OP_SEMICOLON)) {
        pos = start;
        return false;
      }
      out = make_node(CppAstKind::alias_declaration, alias);
      out.children.push_back(std::move(type_id));
      set_span(out, start);
      return true;
    }
    pos = start + 1;
  }

  const size_t target_start = pos;
  consume_simple(KW_TYPENAME);
  string target;
  cpp_decl::QualifiedName target_syntax;
  cpp_decl::TemplateIdSyntax target_template_id_syntax;
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  vector<CppAstNode> qualifier_type_syntaxes;
  if(!parse_qualified_name_text(target,
                                &target_syntax,
                                &target_template_id_syntax,
                                &qualifier_template_id_syntaxes,
                                &qualifier_type_syntaxes,
                                false,
                                false,
                                true)) {
    pos = start;
    return false;
  }
  const size_t target_end = pos;
  CppAstNode attributes = make_node(CppAstKind::specifier);
  if(!skip_attribute_specifier_seq(&attributes) ||
     !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  target = token_span_text_spaced(target_start, target_end);
  if(target.empty()) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::using_declaration);
  out.has_using_if_exists = attributes.has_using_if_exists;
  CppAstNode target_node = make_node(CppAstKind::target, target);
  set_cppast_qualified_name_syntax(target_node, std::move(target_syntax));
  if(!target_template_id_syntax.name.name.empty()) {
    set_cppast_template_id_syntax(target_node,
                                  std::move(target_template_id_syntax));
  }
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(
        target_node,
        std::move(qualifier_template_id_syntaxes));
  }
  if(!qualifier_type_syntaxes.empty()) {
    set_cppast_qualifier_type_syntaxes(target_node,
                                       std::move(qualifier_type_syntaxes));
  }
  out.children.push_back(std::move(target_node));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_template_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_TEMPLATE)) {
    pos = start;
    return false;
  }

  CppAstNode parameters;
  if(!parse_template_parameter_clause(parameters)) {
    pos = start;
    return false;
  }

  CppAstNode declaration;
  NameSet parameter_names;
  NameSet parameter_value_names;
  NameSet parameter_template_names;
  collect_template_parameter_names(parameters, parameter_names);
  collect_template_parameter_value_names(parameters, parameter_value_names);
  collect_template_parameter_template_names(parameters, parameter_template_names);
  template_type_parameter_scopes.push_back(parameter_names);
  template_value_parameter_scopes.push_back(parameter_value_names);
  template_name_scopes.push_back(parameter_template_names);
  value_name_scopes.push_back(parameter_value_names);
  ++template_declaration_depth;
  bool ok = parse_declaration(declaration);
  --template_declaration_depth;
  value_name_scopes.pop_back();
  template_name_scopes.pop_back();
  template_value_parameter_scopes.pop_back();
  template_type_parameter_scopes.pop_back();
  if(!ok) {
    if(error_msg.empty()) {
      set_error("expected declaration after template-parameter-clause near " +
                token_label(peek()) + " at token " + to_string(pos) +
                " [template-scope-depth=" +
                to_string(template_type_parameter_scopes.size()) + "]");
    }
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::template_declaration);
  out.children.push_back(std::move(parameters));
  out.children.push_back(std::move(declaration));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_class_specifier(CppAstNode & out)
{
  size_t start = pos;
  if(!is_class_key(peek())) {
    pos = start;
    return false;
  }

  RecogToken class_key_token = peek();
  ++pos;

  CppAstNode attributes = make_node(CppAstKind::specifier);
  if(!skip_attribute_specifier_seq(&attributes)) {
    pos = start;
    return false;
  }

  string name;
  cpp_decl::QualifiedName name_syntax;
  cpp_decl::TemplateIdSyntax template_id_syntax;
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  vector<CppAstNode> qualifier_type_syntaxes;
  if(parse_qualified_name_text(name,
                               &name_syntax,
                               &template_id_syntax,
                               &qualifier_template_id_syntaxes,
                               &qualifier_type_syntaxes,
                               true)) {
    // handled by helper
  }

  if(!name.empty() && template_declaration_depth > 0 &&
     !template_name_scopes.empty()) {
    template_name_scopes.back().insert(unqualified_name_text(name));
    note_name_lookup_mutation();
  }

  out = make_node(CppAstKind::class_specifier, name);
  if(!name.empty()) {
    set_cppast_qualified_name_syntax(out, std::move(name_syntax));
    if(!template_id_syntax.name.name.empty()) {
      set_cppast_template_id_syntax(out, std::move(template_id_syntax));
    }
    if(!qualifier_template_id_syntaxes.empty()) {
      set_cppast_qualifier_template_id_syntaxes(out,
                                                std::move(qualifier_template_id_syntaxes));
    }
    if(!qualifier_type_syntaxes.empty()) {
      set_cppast_qualifier_type_syntaxes(out, std::move(qualifier_type_syntaxes));
    }
  }
  out.children.push_back(make_token_node(CppAstKind::class_key, class_key_token));
  apply_leading_declaration_attributes(out, attributes);
  if(peek().is_final()) {
    out.is_final_specifier = true;
    ++pos;
  }

  bool saw_base_clause = false;
  CppAstNode bases;
  if(parse_base_clause(bases)) {
    saw_base_clause = true;
    out.children.push_back(std::move(bases));
  }

  if(!saw_base_clause && !peek().is_simple(OP_LBRACE)) {
    if(name.empty() || out.is_final_specifier) {
      pos = start;
      return false;
    }
    out.kind = CppAstKind::class_forward_declaration;
    set_span(out, start);
    return true;
  }

  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  template_name_scopes.push_back(NameSet());
  type_name_scopes.push_back(NameSet());
  value_name_scopes.push_back(NameSet());
  string class_key;
  bool pushed_class_scope = false;
  if(!name.empty()) {
    type_name_scopes.back().insert(unqualified_name_text(name));
    if(template_declaration_depth > 0) {
      template_name_scopes.back().insert(unqualified_name_text(name));
    }
    note_name_lookup_mutation();
    class_key = class_scope_definition_key(name);
    const string class_component = primary_name_text(name);
    if(!class_component.empty()) {
      class_name_stack.push_back(class_component);
      pushed_class_scope = true;
    }
  }
  while(!at_eof() && !peek().is_simple(OP_RBRACE)) {
    CppAstNode member;
    if(!parse_class_member(member)) {
      if(parser_trace::enabled("parser.decl")) {
        std::ostringstream trace;
        const size_t member_start = pos;
        const bool had_attributes = skip_attribute_specifier_seq();
        const bool can_start_decl =
            had_attributes && member_start != pos && can_start_decl_specifier_seq();
        const std::string after_attributes =
            had_attributes && member_start != pos ? token_label(peek()) : std::string("<none>");
        pos = member_start;
        trace << "class member parse failed near "
              << token_label(peek())
              << " [current-class " << current_class_trace_label(class_name_stack) << "]"
              << " [after-attributes " << after_attributes << "]"
              << " [can-start-decl " << (can_start_decl ? "yes" : "no") << "]";
        parser_trace::note("parser.decl", tokens, pos, trace.str());
      }
      if(pushed_class_scope) {
        class_name_stack.pop_back();
      }
      value_name_scopes.pop_back();
      type_name_scopes.pop_back();
      template_name_scopes.pop_back();
      pos = start;
      return false;
    }
    note_declared_template_names(member);
    note_declared_type_names(member);
    note_declared_value_names(member);
    out.children.push_back(std::move(member));
  }

  if(!consume_simple(OP_RBRACE)) {
    if(pushed_class_scope) {
      class_name_stack.pop_back();
    }
    value_name_scopes.pop_back();
    type_name_scopes.pop_back();
    template_name_scopes.pop_back();
    pos = start;
    return false;
  }
  if(!class_key.empty()) {
    ClassMemberNameScopes & stored = class_member_name_scopes[class_key];
    stored.template_names = template_name_scopes.back();
    stored.type_names = type_name_scopes.back();
    stored.value_names = value_name_scopes.back();
    refresh_lazy_function_body_snapshots_for_class(out, stored);
  }
  if(pushed_class_scope) {
    class_name_stack.pop_back();
  }
  value_name_scopes.pop_back();
  type_name_scopes.pop_back();
  template_name_scopes.pop_back();
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_class_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_class_specifier(out) || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_enum_specifier(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_ENUM)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::enum_specifier);
  if(consume_simple(KW_CLASS)) {
    out.children.push_back(make_token_node(CppAstKind::enum_key, tokens[pos - 1]));
  }
  else if(consume_simple(KW_STRUCT)) {
    out.children.push_back(make_token_node(CppAstKind::enum_key, tokens[pos - 1]));
  }

  CppAstNode attributes = make_node(CppAstKind::specifier);
  if(!skip_attribute_specifier_seq(&attributes)) {
    pos = start;
    return false;
  }
  apply_leading_declaration_attributes(out, attributes);

  string name;
  cpp_decl::QualifiedName name_syntax;
  if(parse_qualified_name_text(name, &name_syntax)) {
    out.value = name;
    set_cppast_qualified_name_syntax(out, std::move(name_syntax));
  }

  if(!skip_attribute_specifier_seq(&out)) {
    pos = start;
    return false;
  }

  if(consume_simple(OP_COLON)) {
    CppAstNode underlying = make_node(CppAstKind::type_id);
    CppAstNode specifiers;
    if(!parse_type_specifier_seq(specifiers)) {
      pos = start;
      return false;
    }
    underlying.children.push_back(std::move(specifiers));
    out.children.push_back(std::move(underlying));
  }

  if(!peek().is_simple(OP_LBRACE)) {
    if(out.value.empty()) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  while(!at_eof() && !peek().is_simple(OP_RBRACE)) {
    if(!consume_identifier()) {
      pos = start;
      return false;
    }

    CppAstNode enumerator = make_node(CppAstKind::enumerator, tokens[pos - 1].source);
    if(!skip_attribute_specifier_seq(&enumerator)) {
      pos = start;
      return false;
    }
    if(consume_simple(OP_ASS)) {
      CppAstNode expr;
      if(!parse_assignment_expression(expr)) {
        pos = start;
        return false;
      }
      enumerator.children.push_back(std::move(expr));
    }
    out.children.push_back(std::move(enumerator));

    if(!consume_simple(OP_COMMA)) {
      break;
    }
  }

  if(!consume_simple(OP_RBRACE)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_enum_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_enum_specifier(out) || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_static_assert_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_STATIC_ASSERT) || !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::static_assert_declaration);

  CppAstNode condition;
  if(!parse_assignment_expression(condition)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(condition));

  if(consume_simple(OP_COMMA)) {
    size_t message_start = pos;
    if(!parse_balanced_token_sequence({OP_RPAREN}, false)) {
      pos = start;
      return false;
    }
    out.children.push_back(make_node(CppAstKind::message, token_span_text(message_start, pos)));
  }

  if(!consume_simple(OP_RPAREN) || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_special_member_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers = make_node(CppAstKind::member_specifiers);
  for(;;) {
    const size_t prefix_start = pos;
    skip_attribute_specifier_seq(&specifiers);
    if(pos != prefix_start) {
      continue;
    }

    if(consume_simple(KW_EXPLICIT)) {
      const size_t explicit_start = pos - 1;
      if(peek().is_simple(OP_LPAREN) &&
         !parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      specifiers.children.push_back(
          make_node(CppAstKind::specifier, token_span_text_spaced(explicit_start, pos)));
      continue;
    }

    if(is_member_function_specifier(peek())) {
      specifiers.children.push_back(make_token_node(CppAstKind::specifier, peek()));
      ++pos;
      continue;
    }

    break;
  }

  string name;
  const size_t name_start = pos;
  if(!parse_unqualified_name_text(name, true, true, true) ||
     !peek().is_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode declarator = make_node(CppAstKind::declarator);
  CppAstNode identifier = make_node(CppAstKind::identifier, name);
  CppAstNode conversion_type_id;
  if(name.compare(0, 8, "operator") == 0 &&
     parse_conversion_operator_type_id(name_start, pos, conversion_type_id)) {
    set_cppast_conversion_type_id_syntax(identifier, std::move(conversion_type_id));
  }
  declarator.children.push_back(std::move(identifier));

  CppAstNode parameters;
  if(!parse_parameter_clause(parameters)) {
    pos = start;
    return false;
  }
  declarator.children.push_back(std::move(parameters));
  if(!parse_function_suffixes(declarator)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::special_member_declaration, name);
  if(!specifiers.children.empty() ||
     specifiers.has_exclude_from_explicit_instantiation ||
     specifiers.has_using_if_exists ||
     specifiers.has_no_unique_address ||
     !specifiers.abi_tags.empty() ||
     !specifiers.alignment_specifiers.empty()) {
    out.children.push_back(std::move(specifiers));
  }
  out.children.push_back(std::move(declarator));

  if(consume_simple(OP_ASS)) {
    size_t marker_start = pos;
    if(consume_simple(KW_DEFAULT) || consume_simple(KW_DELETE)) {
      out.children.push_back(
          make_node(CppAstKind::special_definition, token_span_text(marker_start, pos)));
    }
    else if(peek().is_zero()) {
      ++pos;
      out.children.push_back(
          make_node(CppAstKind::special_definition, token_span_text(marker_start, pos)));
    }
    else {
      pos = start;
      return false;
    }

    if(!consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(consume_simple(OP_SEMICOLON)) {
    set_span(out, start);
    return true;
  }

  CppAstNode ctor_initializer;
  CppAstNode body;
  NameSet parameter_value_names;
  collect_outer_parameter_value_names(declarator, parameter_value_names);
  if(!parameter_value_names.empty()) {
    value_name_scopes.push_back(parameter_value_names);
  }
  const bool parsed_body = peek().is_simple(KW_TRY) ?
      parse_function_try_body(body, &ctor_initializer) :
      ([&]() -> bool {
        if(parse_ctor_initializer(ctor_initializer)) {
          return parse_function_body(body);
        }
        return parse_function_body(body);
      })();
  if(!parameter_value_names.empty()) {
    value_name_scopes.pop_back();
  }
  if(!parsed_body) {
    pos = start;
    return false;
  }

  out.kind = CppAstKind::special_member_definition;
  if(ctor_initializer.kind != CppAstKind::invalid) {
    out.children.push_back(std::move(ctor_initializer));
  }
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_deduction_guide_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers = make_node(CppAstKind::member_specifiers);
  for(;;) {
    const size_t prefix_start = pos;
    skip_attribute_specifier_seq(&specifiers);
    if(pos != prefix_start) {
      continue;
    }

    if(consume_simple(KW_EXPLICIT)) {
      const size_t explicit_start = pos - 1;
      if(peek().is_simple(OP_LPAREN) &&
         !parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      specifiers.children.push_back(
          make_node(CppAstKind::specifier, token_span_text_spaced(explicit_start, pos)));
      continue;
    }

    break;
  }

  string name;
  if(!parse_unqualified_name_text(name, false, false, false) ||
     !peek().is_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode declarator = make_node(CppAstKind::declarator);
  declarator.children.push_back(make_node(CppAstKind::identifier, name));

  CppAstNode parameters;
  if(!parse_parameter_clause(parameters)) {
    pos = start;
    return false;
  }
  declarator.children.push_back(std::move(parameters));
  if(!parse_function_suffixes(declarator)) {
    pos = start;
    return false;
  }

  bool have_trailing_return = false;
  for(const CppAstNode & child : declarator.children) {
    if(child.kind == CppAstKind::trailing_return_type) {
      have_trailing_return = true;
      break;
    }
  }
  if(!have_trailing_return || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::deduction_guide_declaration, name);
  if(!specifiers.children.empty() ||
     specifiers.has_exclude_from_explicit_instantiation ||
     specifiers.has_using_if_exists ||
     specifiers.has_no_unique_address ||
     !specifiers.abi_tags.empty() ||
     !specifiers.alignment_specifiers.empty()) {
    out.children.push_back(std::move(specifiers));
  }
  out.children.push_back(std::move(declarator));
  set_span(out, start);
  return true;
}

namespace {

const RecogToken * first_name_component_identifier(const IRecogTokenSequence & tokens,
                                                   std::pair<std::size_t, std::size_t> range)
{
  std::size_t cursor = range.first;
  if(tokens.peek(cursor).is_simple(KW_TEMPLATE)) {
    ++cursor;
  }
  return tokens.peek(cursor).is_identifier() ? &tokens.peek(cursor) : nullptr;
}

bool is_qualified_special_member_or_conversion_name(
    const IRecogTokenSequence & tokens,
    const qualified_name_parser::QualifiedNameParseResult & parsed)
{
  if(parsed.qualifiers.empty()) {
    return false;
  }

  const RecogToken * class_name =
      first_name_component_identifier(tokens, parsed.qualifiers.back());
  if(!class_name) {
    return false;
  }

  switch(parsed.name_kind) {
  case qualified_name_parser::UNQ_COMPONENT: {
    const RecogToken * simple_name =
        first_name_component_identifier(tokens, parsed.name_component);
    return simple_name && simple_name->source == class_name->source;
  }
  case qualified_name_parser::UNQ_DESTRUCTOR: {
    const std::pair<std::size_t, std::size_t> destructor_name(
        parsed.name_component.first + 1,
        parsed.name_component.second);
    const RecogToken * simple_name =
        first_name_component_identifier(tokens, destructor_name);
    return simple_name && simple_name->source == class_name->source;
  }
  case qualified_name_parser::UNQ_OPERATOR:
    return parsed.operator_is_conversion;
  }

  return false;
}

}  // namespace

bool CppAstParser::parse_conversion_operator_type_id(std::size_t name_start,
                                                     std::size_t name_end,
                                                     CppAstNode & out)
{
  out = CppAstNode();
  if(name_end <= name_start) {
    return false;
  }

  std::size_t operator_pos = name_end;
  for(std::size_t i = name_start; i < name_end; ++i) {
    if(tokens.peek(i).is_simple(KW_OPERATOR)) {
      operator_pos = i;
    }
  }
  if(operator_pos == name_end || operator_pos + 1 >= name_end) {
    return false;
  }

  CppAstNode parsed;
  bool is_type_id = false;
  if(!parse_template_argument_fragment_node(operator_pos + 1,
                                            name_end,
                                            parsed,
                                            is_type_id) ||
     !is_type_id) {
    return false;
  }
  out = parsed;
  return true;
}

bool CppAstParser::parse_qualified_special_member_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers = make_node(CppAstKind::member_specifiers);
  skip_attribute_specifier_seq(&specifiers);
  for(;;) {
    const size_t prefix_start = pos;
    skip_attribute_specifier_seq(&specifiers);
    if(pos != prefix_start) {
      continue;
    }

    if(consume_simple(KW_EXPLICIT)) {
      const size_t explicit_start = pos - 1;
      if(peek().is_simple(OP_LPAREN) &&
         !parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      specifiers.children.push_back(
          make_node(CppAstKind::specifier, token_span_text_spaced(explicit_start, pos)));
      continue;
    }

    if(is_member_function_specifier(peek())) {
      specifiers.children.push_back(make_token_node(CppAstKind::specifier, peek()));
      ++pos;
      continue;
    }

    break;
  }

  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup(true);
  const size_t name_start = pos;
  qualified_name_parser::QualifiedNameParseResult parsed_name;
  if(!qualified_name_parser::parse_qualified_name(tokens,
                                                  name_start,
                                                  lookup,
                                                  qualified_name_parser::UnqualifiedNameOptions(),
                                                  parsed_name) ||
     parsed_name.qualifiers.empty()) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "qualified implicit-type declaration rejected before parameter-clause near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, start, trace.str());
    }
    pos = start;
    return false;
  }
  pos = parsed_name.end;

  const string name = token_span_text_spaced(name_start, pos);
  const cpp_decl::QualifiedName name_syntax = build_qualified_name_syntax(tokens, parsed_name);
  if(!peek().is_simple(OP_LPAREN)) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "qualified implicit-type declaration rejected before parameter-clause near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, start, trace.str());
    }
    pos = start;
    return false;
  }

  if(!is_qualified_special_member_or_conversion_name(tokens, parsed_name)) {
    if(parser_trace::enabled("parser.decl")) {
      parser_trace::note("parser.decl",
                         tokens,
                         start,
                         "qualified implicit-type declaration rejected by name " + name);
    }
    pos = start;
    return false;
  }

  CppAstNode declarator = make_node(CppAstKind::declarator);
  CppAstNode identifier = make_node(CppAstKind::identifier, name);
  set_cppast_qualified_name_syntax(identifier, name_syntax);
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  build_qualifier_template_id_syntaxes(tokens,
                                       lookup,
                                       parsed_name,
                                       qualifier_template_id_syntaxes,
                                       this);
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(
        identifier,
        std::move(qualifier_template_id_syntaxes));
  }
  CppAstNode conversion_type_id;
  if(parsed_name.operator_is_conversion &&
     parse_conversion_operator_type_id(name_start, pos, conversion_type_id)) {
    set_cppast_conversion_type_id_syntax(identifier, std::move(conversion_type_id));
  }
  declarator.children.push_back(std::move(identifier));

  CppAstNode parameters;
  if(!parse_parameter_clause(parameters)) {
    pos = start;
    return false;
  }
  declarator.children.push_back(std::move(parameters));
  if(!parse_function_suffixes(declarator)) {
    pos = start;
    return false;
  }
  if(consume_simple(OP_ASS)) {
    size_t marker_start = pos;
    if(consume_simple(KW_DEFAULT) || consume_simple(KW_DELETE)) {
      out = make_node(CppAstKind::special_member_declaration, name);
      set_cppast_qualified_name_syntax(out, std::move(name_syntax));
      if(!specifiers.children.empty() ||
         specifiers.has_exclude_from_explicit_instantiation ||
         specifiers.has_using_if_exists ||
         specifiers.has_no_unique_address ||
         !specifiers.abi_tags.empty() ||
         !specifiers.alignment_specifiers.empty()) {
        out.children.push_back(std::move(specifiers));
      }
      out.children.push_back(std::move(declarator));
      out.children.push_back(
          make_node(CppAstKind::special_definition, token_span_text(marker_start, pos)));
      if(!consume_simple(OP_SEMICOLON)) {
        pos = start;
        return false;
      }
      set_span(out, start);
      return true;
    }
    pos = start;
    return false;
  }
  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::special_member_declaration, name);
  set_cppast_qualified_name_syntax(out, std::move(name_syntax));
  if(!specifiers.children.empty() ||
     specifiers.has_exclude_from_explicit_instantiation ||
     specifiers.has_using_if_exists ||
     specifiers.has_no_unique_address ||
     !specifiers.abi_tags.empty() ||
     !specifiers.alignment_specifiers.empty()) {
    out.children.push_back(std::move(specifiers));
  }
  out.children.push_back(std::move(declarator));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_qualified_special_member_definition(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers = make_node(CppAstKind::member_specifiers);
  skip_attribute_specifier_seq(&specifiers);
  for(;;) {
    const size_t prefix_start = pos;
    skip_attribute_specifier_seq(&specifiers);
    if(pos != prefix_start) {
      continue;
    }

    if(consume_simple(KW_EXPLICIT)) {
      const size_t explicit_start = pos - 1;
      if(peek().is_simple(OP_LPAREN) &&
         !parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      specifiers.children.push_back(
          make_node(CppAstKind::specifier, token_span_text_spaced(explicit_start, pos)));
      continue;
    }

    if(is_member_function_specifier(peek())) {
      specifiers.children.push_back(make_token_node(CppAstKind::specifier, peek()));
      ++pos;
      continue;
    }

    break;
  }

  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
  const size_t name_start = pos;
  qualified_name_parser::QualifiedNameParseResult parsed_name;
  if(!qualified_name_parser::parse_qualified_name(tokens,
                                                  name_start,
                                                  lookup,
                                                  qualified_name_parser::UnqualifiedNameOptions(),
                                                  parsed_name) ||
     parsed_name.qualifiers.empty()) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "qualified implicit-type definition rejected before parameter-clause near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, start, trace.str());
    }
    pos = start;
    return false;
  }
  pos = parsed_name.end;

  const string name = token_span_text_spaced(name_start, pos);
  const cpp_decl::QualifiedName name_syntax = build_qualified_name_syntax(tokens, parsed_name);
  if(!peek().is_simple(OP_LPAREN)) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "qualified implicit-type definition rejected before parameter-clause near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, start, trace.str());
    }
    pos = start;
    return false;
  }

  if(!is_qualified_special_member_or_conversion_name(tokens, parsed_name)) {
    if(parser_trace::enabled("parser.decl")) {
      parser_trace::note("parser.decl",
                         tokens,
                         start,
                         "qualified implicit-type definition rejected by name " + name);
    }
    pos = start;
    return false;
  }

  CppAstNode declarator = make_node(CppAstKind::declarator);
  CppAstNode identifier = make_node(CppAstKind::identifier, name);
  set_cppast_qualified_name_syntax(identifier, name_syntax);
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  build_qualifier_template_id_syntaxes(tokens,
                                       lookup,
                                       parsed_name,
                                       qualifier_template_id_syntaxes,
                                       this);
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(
        identifier,
        std::move(qualifier_template_id_syntaxes));
  }
  CppAstNode conversion_type_id;
  if(parsed_name.operator_is_conversion &&
     parse_conversion_operator_type_id(name_start, pos, conversion_type_id)) {
    set_cppast_conversion_type_id_syntax(identifier, std::move(conversion_type_id));
  }
  declarator.children.push_back(std::move(identifier));

  CppAstNode parameters;
  if(!parse_parameter_clause(parameters)) {
    pos = start;
    return false;
  }
  declarator.children.push_back(std::move(parameters));
  if(!parse_function_suffixes(declarator)) {
    pos = start;
    return false;
  }

  CppAstNode ctor_initializer;
  const string class_key = owner_class_scope_key(name);
  const SeededClassNameScopes seeded_class_names = push_class_member_name_hints(class_key);
  CppAstNode body;
  NameSet parameter_value_names;
  collect_outer_parameter_value_names(declarator, parameter_value_names);
  if(!parameter_value_names.empty()) {
    value_name_scopes.push_back(parameter_value_names);
  }
  const bool parsed_body = peek().is_simple(KW_TRY) ?
      parse_function_try_body(body, &ctor_initializer) :
      ([&]() -> bool {
        if(parse_ctor_initializer(ctor_initializer)) {
          return parse_function_body(body);
        }
        return parse_function_body(body);
      })();
  if(!parameter_value_names.empty()) {
    value_name_scopes.pop_back();
  }
  pop_class_member_name_hints(seeded_class_names);
  if(!parsed_body) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::special_member_definition, name);
  set_cppast_qualified_name_syntax(out, std::move(name_syntax));
  if(!specifiers.children.empty() ||
     specifiers.has_exclude_from_explicit_instantiation ||
     specifiers.has_using_if_exists ||
     specifiers.has_no_unique_address ||
     !specifiers.abi_tags.empty() ||
     !specifiers.alignment_specifiers.empty()) {
    out.children.push_back(std::move(specifiers));
  }
  out.children.push_back(std::move(declarator));
  if(ctor_initializer.kind != CppAstKind::invalid) {
    out.children.push_back(std::move(ctor_initializer));
  }
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_template_special_member_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_TEMPLATE)) {
    pos = start;
    return false;
  }

  CppAstNode parameters;
  if(!parse_template_parameter_clause(parameters)) {
    pos = start;
    return false;
  }

  CppAstNode declaration;
  NameSet parameter_names;
  NameSet parameter_value_names;
  collect_template_parameter_names(parameters, parameter_names);
  collect_template_parameter_value_names(parameters, parameter_value_names);
  template_type_parameter_scopes.push_back(parameter_names);
  template_value_parameter_scopes.push_back(parameter_value_names);
  value_name_scopes.push_back(parameter_value_names);
  const bool ok = parse_special_member_declaration(declaration);
  value_name_scopes.pop_back();
  template_value_parameter_scopes.pop_back();
  template_type_parameter_scopes.pop_back();
  if(!ok) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::template_declaration);
  out.children.push_back(std::move(parameters));
  out.children.push_back(std::move(declaration));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_bit_field_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::bit_field_declaration);
  out.children.push_back(std::move(specifiers));

  for(;;) {
    size_t declarator_start = pos;
    CppAstNode declarator;
    bool has_declarator = parse_declarator(declarator);
    if(!has_declarator) {
      pos = declarator_start;
    }

    if(!skip_trailing_declarator_extensions() || !consume_simple(OP_COLON)) {
      pos = start;
      return false;
    }

    CppAstNode width;
    if(!parse_assignment_expression(width)) {
      pos = start;
      return false;
    }

    CppAstNode field = make_node(CppAstKind::bit_field_declarator);
    if(has_declarator) {
      field.children.push_back(std::move(declarator));
    }
    field.children.push_back(std::move(width));
    out.children.push_back(std::move(field));

    if(!consume_simple(OP_COMMA)) {
      break;
    }
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_function_definition(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers;
  CppAstNode declarator;
  CppAstNode body;

  CppAstNode trailing_extensions;
  if(!parse_decl_specifier_seq(specifiers) || !parse_declarator(declarator) ||
     !skip_trailing_declarator_extensions(&trailing_extensions) ||
     !declarator_has_parameter_clause(declarator) ||
     !(peek().is_simple(OP_LBRACE) || peek().is_simple(KW_TRY))) {
    pos = start;
    return false;
  }
  apply_trailing_declarator_extensions(declarator, trailing_extensions);

  NameSet signature_type_hints;
  collect_signature_type_hint_names(specifiers, signature_type_hints);
  collect_signature_type_hint_names(declarator, signature_type_hints);
  NameSet parameter_value_names;
  string class_key;
  SeededClassNameScopes seeded_class_names;
  resolve_declarator_owner_class_scope_key(declarator, class_key);
  collect_outer_parameter_value_names(declarator, parameter_value_names);
  if(!class_key.empty()) {
    seeded_class_names = push_class_member_name_hints(class_key);
  }
  type_name_scopes.push_back(signature_type_hints);
  if(!parameter_value_names.empty()) {
    value_name_scopes.push_back(parameter_value_names);
  }
  const bool parsed_body = parse_function_body(body);
  if(!parameter_value_names.empty()) {
    value_name_scopes.pop_back();
  }
  type_name_scopes.pop_back();
  if(!class_key.empty()) {
    pop_class_member_name_hints(seeded_class_names);
  }
  if(!parsed_body) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::function_definition);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarator));
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_simple_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode attributes = make_node(CppAstKind::specifier);
  skip_attribute_specifier_seq(&attributes);
  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }
  if(!parse_simple_declaration_after_specifiers(out, specifiers, start)) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "simple-declaration rejected after specifiers={"
            << token_span_text_spaced(specifiers.token_start, specifiers.token_end)
            << "} next=" << token_label(peek());
      parser_trace::note("parser.decl", tokens, pos, trace.str());
    }
    return false;
  }
  apply_leading_declaration_attributes(out, attributes);
  return true;
}

bool CppAstParser::parse_decl_specifier_leading_declaration(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode attributes = make_node(CppAstKind::specifier);
  skip_attribute_specifier_seq(&attributes);
  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  size_t declarator_start = pos;
  CppAstNode declarator;
  if(!parse_declarator(declarator)) {
    pos = declarator_start;
    if(peek().is_simple(OP_SEMICOLON) || can_start_structured_binding_declarator()) {
      if(!parse_simple_declaration_after_specifiers(out, specifiers, start)) {
        return false;
      }
      apply_leading_declaration_attributes(out, attributes);
      return true;
    }
    pos = start;
    return false;
  }

  CppAstNode trailing_extensions;
  if(!skip_trailing_declarator_extensions(&trailing_extensions)) {
    pos = start;
    return false;
  }
  apply_trailing_declarator_extensions(declarator, trailing_extensions);

  if(declarator_has_parameter_clause(declarator) &&
     (peek().is_simple(OP_LBRACE) || peek().is_simple(KW_TRY))) {
    CppAstNode body;
    NameSet signature_type_hints;
    collect_signature_type_hint_names(specifiers, signature_type_hints);
    collect_signature_type_hint_names(declarator, signature_type_hints);
    NameSet parameter_value_names;
    string class_key;
    SeededClassNameScopes seeded_class_names;
    resolve_declarator_owner_class_scope_key(declarator, class_key);
    collect_outer_parameter_value_names(declarator, parameter_value_names);
    if(!class_key.empty()) {
      seeded_class_names = push_class_member_name_hints(class_key);
    }
    type_name_scopes.push_back(signature_type_hints);
    if(!parameter_value_names.empty()) {
      value_name_scopes.push_back(parameter_value_names);
    }
    const bool parsed_body = parse_function_body(body);
    if(!parameter_value_names.empty()) {
      value_name_scopes.pop_back();
    }
    type_name_scopes.pop_back();
    if(!class_key.empty()) {
      pop_class_member_name_hints(seeded_class_names);
    }
    if(!parsed_body) {
      pos = start;
      return false;
    }

    out = make_node(CppAstKind::function_definition);
    out.children.push_back(std::move(specifiers));
    out.children.push_back(std::move(declarator));
    out.children.push_back(std::move(body));
    set_span(out, start);
    apply_leading_declaration_attributes(out, attributes);
    return true;
  }

  if(!parse_simple_declaration_after_specifiers(out, specifiers, start,
                                                &declarator, declarator_start)) {
    return false;
  }
  apply_leading_declaration_attributes(out, attributes);
  return true;
}

bool CppAstParser::parse_simple_declaration_after_specifiers(
    CppAstNode & out, CppAstNode & specifiers, size_t start,
    CppAstNode * first_declarator, size_t first_declarator_start)
{
  if(first_declarator == nullptr && consume_simple(OP_SEMICOLON)) {
    out = make_node(CppAstKind::simple_declaration);
    out.children.push_back(std::move(specifiers));
    set_span(out, start);
    return true;
  }

  if(first_declarator == nullptr && can_start_structured_binding_declarator()) {
    if(!parse_structured_binding_declaration_after_specifiers(out, specifiers, start)) {
      pos = start;
      return false;
    }
    return true;
  }

  CppAstNode declarators = make_node(CppAstKind::init_declarator_list);
  CppAstNode first;
  if(first_declarator != nullptr) {
    if(!parse_init_declarator_after_declarator(
           first, *first_declarator, first_declarator_start)) {
      pos = start;
      return false;
    }
  }
  else if(!parse_init_declarator(first)) {
    pos = start;
    return false;
  }
  declarators.children.push_back(std::move(first));

  while(consume_simple(OP_COMMA)) {
    CppAstNode next;
    if(!parse_init_declarator(next)) {
      pos = start;
      return false;
    }
    declarators.children.push_back(std::move(next));
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::simple_declaration);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarators));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_structured_binding_declaration_after_specifiers(
    CppAstNode & out, CppAstNode & specifiers, size_t start)
{
  CppAstNode declarator;
  if(!parse_structured_binding_declarator(declarator)) {
    pos = start;
    return false;
  }

  CppAstNode initializer;
  if(!parse_initializer(initializer) || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::structured_binding_declaration);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarator));
  out.children.push_back(std::move(initializer));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_init_declarator_after_declarator(
    CppAstNode & out, CppAstNode & declarator, size_t start)
{
  out = make_node(CppAstKind::init_declarator);
  out.has_no_unique_address = declarator.has_no_unique_address;
  out.children.push_back(std::move(declarator));

  if(!skip_trailing_declarator_extensions(&out)) {
    pos = start;
    return false;
  }
  if(!out.asm_label.empty() && !out.children.empty()) {
    out.children[0].asm_label = out.asm_label;
  }

  if(peek().is_simple(OP_ASS) || peek().is_simple(OP_LBRACE) ||
     peek().is_simple(OP_LPAREN)) {
    CppAstNode initializer;
    if(!parse_initializer(initializer)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(initializer));
  }

  if(!skip_trailing_declarator_extensions()) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_type_id(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers;
  if(!parse_type_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::type_id);
  out.children.push_back(std::move(specifiers));

  CppAstNode declarator;
  if(parse_abstract_declarator(declarator)) {
    declarator.kind = CppAstKind::abstract_declarator;
    out.children.push_back(std::move(declarator));
  }

  set_span(out, start);

  return true;
}

bool CppAstParser::parse_new_type_id(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode specifiers;
  if(!parse_type_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::type_id);
  out.children.push_back(std::move(specifiers));

  CppAstNode declarator;
  if(parse_new_abstract_declarator(declarator)) {
    declarator.kind = CppAstKind::abstract_declarator;
    out.children.push_back(std::move(declarator));
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_type_specifier_seq(CppAstNode & out)
{
  size_t start = pos;
  bool matched = false;

  out = make_node(CppAstKind::type_specifier_seq);

  for(;;) {
    size_t attribute_start = pos;
    if(!skip_gnu_attribute_specifier_seq(&out)) {
      pos = start;
      return false;
    }
    if(pos != attribute_start) {
      continue;
    }

    const auto & token = peek();
    if(is_gnu_extension_token(token)) {
      ++pos;
      continue;
    }
    if(is_cv_qualifier(token)) {
      out.children.push_back(make_token_node(CppAstKind::cv_qualifier, token));
      ++pos;
      matched = true;
      continue;
    }

    if(is_simple_type_specifier(token)) {
      out.children.push_back(make_token_node(CppAstKind::type_specifier, token));
      ++pos;
      matched = true;
      continue;
    }

    if(is_gnu_int128_type_specifier_identifier(token)) {
      out.children.push_back(make_node(CppAstKind::type_specifier, token.source));
      ++pos;
      matched = true;
      continue;
    }

    if(is_decltype_token(token) ||
       (is_gnu_typeof_token(token) && peek(1).is_simple(OP_LPAREN))) {
      size_t decltype_start = pos;
      const bool is_typeof = is_gnu_typeof_token(token);
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      CppAstNode decltype_spec =
          make_node(CppAstKind::decltype_specifier, token_span_text_spaced(decltype_start, pos));
      decltype_spec.is_typeof_specifier = is_typeof;
      CppAstNode operand;
      if(parse_decltype_or_typeof_operand_node(decltype_start, pos, is_typeof, operand)) {
        decltype_spec.children.push_back(std::move(operand));
      }
      out.children.push_back(std::move(decltype_spec));
      matched = true;
      continue;
    }

    if(token.is_identifier() &&
       token.source == "_BitInt" &&
       peek(1).is_simple(OP_LPAREN)) {
      const size_t bitint_start = pos;
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      out.children.push_back(
          make_node(CppAstKind::type_specifier, token_span_text_spaced(bitint_start, pos)));
      matched = true;
      continue;
    }

    {
      const size_t complex_start = pos;
      size_t complex_end = pos;
      if(match_gnu_complex_type_specifier(tokens, pos, complex_end)) {
        pos = complex_end;
        out.children.push_back(
            make_node(CppAstKind::type_specifier, token_span_text_spaced(complex_start, pos)));
        matched = true;
        continue;
      }
    }

    if(token.is_simple(KW_TYPENAME)) {
      ++pos;
      const size_t name_start = pos;
      string name;
      if(!parse_type_name_text(name)) {
        pos = start;
        return false;
      }
      CppAstNode type_name = make_node(CppAstKind::type_name, name);
      annotate_builtin_type_transform_node(type_name, tokens, name_start);
      attach_builtin_type_transform_syntax_from_span(type_name, name_start, pos);
      type_name.has_leading_typename = true;
      attach_qualified_name_syntax_from_span(type_name, name_start, pos);
      set_span(type_name, name_start);
      out.children.push_back(std::move(type_name));
      matched = true;
      continue;
    }

    {
      size_t specifier_start = pos;
      CppAstNode specifier;
      if(parse_class_specifier(specifier) || parse_enum_specifier(specifier)) {
        out.children.push_back(std::move(specifier));
        matched = true;
        continue;
      }
      pos = specifier_start;
    }

    string name;
    const size_t name_start = pos;
    if(parse_type_name_text(name)) {
      CppAstNode type_name = make_node(CppAstKind::type_name, name);
      annotate_builtin_type_transform_node(type_name, tokens, name_start);
      attach_builtin_type_transform_syntax_from_span(type_name, name_start, pos);
      attach_qualified_name_syntax_from_span(type_name, name_start, pos);
      set_span(type_name, name_start);
      out.children.push_back(std::move(type_name));
      matched = true;
      continue;
    }

    break;
  }

  if(!matched) {
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "decl-specifier-seq rejected near " << token_label(peek());
      parser_trace::note("parser.decl", tokens, pos, trace.str());
    }
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_decl_specifier_seq(CppAstNode & out)
{
  size_t start = pos;
  bool matched = false;
  bool saw_type_name = false;

  out = make_node(CppAstKind::decl_specifier_seq);

  for(;;) {
    size_t attribute_start = pos;
    if(!skip_gnu_attribute_specifier_seq(&out)) {
      pos = start;
      return false;
    }
    if(pos != attribute_start) {
      continue;
    }

    const auto & token = peek();
    if(is_gnu_extension_token(token)) {
      ++pos;
      continue;
    }
    if(is_decltype_token(token) ||
       (is_gnu_typeof_token(token) && peek(1).is_simple(OP_LPAREN))) {
      size_t decltype_start = pos;
      const bool is_typeof = is_gnu_typeof_token(token);
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      CppAstNode decltype_spec =
          make_node(CppAstKind::decl_specifier, token_span_text_spaced(decltype_start, pos));
      decltype_spec.is_typeof_specifier = is_typeof;
      CppAstNode operand;
      if(parse_decltype_or_typeof_operand_node(decltype_start, pos, is_typeof, operand)) {
        decltype_spec.children.push_back(std::move(operand));
      }
      set_span(decltype_spec, decltype_start);
      out.children.push_back(std::move(decltype_spec));
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(!saw_type_name &&
       token.is_identifier() &&
       token.source == "_BitInt" &&
       peek(1).is_simple(OP_LPAREN)) {
      const size_t bitint_start = pos;
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      out.children.push_back(
          make_node(CppAstKind::decl_specifier, token_span_text_spaced(bitint_start, pos)));
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(!saw_type_name &&
       token.is_identifier() &&
       token.source == "_Atomic" &&
       peek(1).is_simple(OP_LPAREN)) {
      const size_t atomic_start = pos;
      ++pos;
      if(!consume_simple(OP_LPAREN)) {
        pos = start;
        return false;
      }
      CppAstNode type_id;
      if(!parse_type_id(type_id) || !consume_simple(OP_RPAREN)) {
        pos = start;
        return false;
      }
      CppAstNode atomic = make_node(CppAstKind::decl_specifier, "_Atomic");
      atomic.children.push_back(std::move(type_id));
      atomic.token_start = atomic_start;
      atomic.token_end = pos;
      out.children.push_back(std::move(atomic));
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(!saw_type_name) {
      const size_t complex_start = pos;
      size_t complex_end = pos;
      if(match_gnu_complex_type_specifier(tokens, pos, complex_end)) {
        pos = complex_end;
        out.children.push_back(
            make_node(CppAstKind::decl_specifier, token_span_text_spaced(complex_start, pos)));
        matched = true;
        saw_type_name = true;
        continue;
      }
    }
    if(!saw_type_name && token.is_simple(KW_TYPENAME)) {
      ++pos;
      const size_t name_start = pos;
      string name;
      if(!parse_type_name_text(name)) {
        pos = start;
        return false;
      }
      CppAstNode specifier = make_node(CppAstKind::decl_specifier, name);
      specifier.has_leading_typename = true;
      annotate_builtin_type_transform_node(specifier, tokens, name_start);
      attach_builtin_type_transform_syntax_from_span(specifier, name_start, pos);
      attach_qualified_name_syntax_from_span(specifier, name_start, pos);
      set_span(specifier, name_start);
      out.children.push_back(std::move(specifier));
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(!saw_type_name) {
      size_t specifier_start = pos;
      CppAstNode specifier;
      if(parse_class_specifier(specifier) || parse_enum_specifier(specifier)) {
        out.children.push_back(std::move(specifier));
        matched = true;
        saw_type_name = true;
        continue;
      }
      pos = specifier_start;
    }
    if(!saw_type_name && is_gnu_float_type_specifier_identifier(token)) {
      out.children.push_back(make_node(CppAstKind::decl_specifier, token.source));
      ++pos;
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(is_gnu_int128_type_specifier_identifier(token)) {
      out.children.push_back(make_node(CppAstKind::decl_specifier, token.source));
      ++pos;
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(is_gnu_decl_specifier_identifier(token)) {
      out.children.push_back(make_node(CppAstKind::decl_specifier, token.source));
      ++pos;
      matched = true;
      continue;
    }
    if(!saw_type_name &&
       (token.is_simple(OP_COLON2) ||
        token.is_simple(KW_TEMPLATE) ||
        token.is_identifier())) {
      size_t type_name_start = pos;
      string name;
      if(parse_type_name_text(name)) {
        if(pos == type_name_start + 1 && tokens[type_name_start].is_identifier()) {
          out.children.push_back(make_token_node(CppAstKind::decl_specifier,
                                                 tokens[type_name_start]));
        } else {
          CppAstNode specifier = make_node(CppAstKind::decl_specifier, name);
          annotate_builtin_type_transform_node(specifier, tokens, type_name_start);
          attach_builtin_type_transform_syntax_from_span(
              specifier, type_name_start, pos);
          attach_qualified_name_syntax_from_span(specifier, type_name_start, pos);
          set_span(specifier, type_name_start);
          out.children.push_back(std::move(specifier));
        }
        matched = true;
        saw_type_name = true;
        continue;
      }
    }
    if(is_cv_qualifier(token) || is_simple_type_specifier(token) ||
       is_decl_specifier_keyword(token) ||
       (!saw_type_name && is_template_type_parameter_name(token))) {
      if(is_simple_type_specifier(token) || is_template_type_parameter_name(token)) {
        saw_type_name = true;
      }
      out.children.push_back(make_token_node(CppAstKind::decl_specifier, token));
      ++pos;
      matched = true;
      continue;
    }
    break;
  }

  if(!matched) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_init_declarator_list(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode first;
  if(!parse_init_declarator(first)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::init_declarator_list);
  out.children.push_back(std::move(first));

  while(consume_simple(OP_COMMA)) {
    CppAstNode next;
    if(!parse_init_declarator(next)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(next));
  }

  return true;
}

bool CppAstParser::parse_init_declarator(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode declarator;
  if(!parse_declarator(declarator)) {
    pos = start;
    return false;
  }
  return parse_init_declarator_after_declarator(out, declarator, start);
}

bool CppAstParser::parse_initializer(CppAstNode & out)
{
  size_t start = pos;
  out = make_node(CppAstKind::initializer);

  if(consume_simple(OP_ASS)) {
    out.uses_assignment_form = true;
    if(consume_simple(KW_DEFAULT) || consume_simple(KW_DELETE)) {
      out.children.push_back(
          make_node(CppAstKind::special_initializer, token_span_text(pos - 1, pos)));
      set_span(out, start);
      return true;
    }

    CppAstNode value;
    if(!parse_braced_init_list(value) && !parse_assignment_expression(value)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(value));
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(OP_LBRACE)) {
    CppAstNode value;
    if(!parse_braced_init_list(value)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(value));
    set_span(out, start);
    return true;
  }

  if(consume_simple(OP_LPAREN)) {
    CppAstNode paren = make_node(CppAstKind::paren_initializer);
    if(!consume_simple(OP_RPAREN)) {
      while(true) {
        CppAstNode element;
        if(!parse_braced_init_list(element) &&
           !parse_assignment_expression(element)) {
          pos = start;
          return false;
        }
        if(consume_simple(OP_DOTS)) {
          CppAstNode expanded = make_node(CppAstKind::pack_expansion_expression);
          expanded.children.push_back(std::move(element));
          element = std::move(expanded);
        }
        paren.children.push_back(std::move(element));

        if(consume_simple(OP_RPAREN)) {
          break;
        }

        if(!consume_simple(OP_COMMA)) {
          pos = start;
          return false;
        }
      }
    }
    out.children.push_back(std::move(paren));
    set_span(out, start);
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_declarator(CppAstNode & out, bool require_parameters)
{
  size_t start = pos;
  out = make_node(CppAstKind::declarator);
  bool saw_parameters = false;

  while(true) {
    CppAstNode ptr_operator;
    if(!parse_ptr_operator_node(ptr_operator)) {
      break;
    }
    out.children.push_back(std::move(ptr_operator));
    while(is_cv_qualifier(peek()) || is_nullability_qualifier_token(peek()) ||
          is_gnu_restrict_qualifier_token(peek())) {
      if(is_gnu_restrict_qualifier_token(peek())) {
        ++pos;
        continue;
      }
      if(is_cv_qualifier(peek())) {
        out.children.push_back(make_token_node(CppAstKind::cv_qualifier, peek()));
      } else {
        out.children.push_back(make_token_node(CppAstKind::nullability_qualifier, peek()));
      }
      ++pos;
    }
  }

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
  }

  if(!skip_attribute_specifier_seq(&out)) {
    pos = start;
    return false;
  }

  if(consume_simple(OP_LPAREN)) {
    CppAstNode nested;
    if(!parse_declarator(nested) || !consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }
    CppAstNode nested_node = make_node(CppAstKind::nested_declarator);
    nested_node.children.push_back(std::move(nested));
    out.children.push_back(std::move(nested_node));
  }
  else {
    string name;
    cpp_decl::QualifiedName name_syntax;
    cpp_decl::TemplateIdSyntax template_id_syntax;
    vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
    vector<CppAstNode> qualifier_type_syntaxes;
    const size_t name_start = pos;
    if(!parse_qualified_name_text(name,
                                  &name_syntax,
                                  &template_id_syntax,
                                  &qualifier_template_id_syntaxes,
                                  &qualifier_type_syntaxes,
                                  false,
                                  false,
                                  false,
                                  true)) {
      pos = start;
      return false;
    }
    CppAstNode identifier = make_node(CppAstKind::identifier, name);
    set_cppast_qualified_name_syntax(identifier, std::move(name_syntax));
    if(!template_id_syntax.name.name.empty()) {
      set_cppast_template_id_syntax(identifier, std::move(template_id_syntax));
    }
    if(!qualifier_template_id_syntaxes.empty()) {
      set_cppast_qualifier_template_id_syntaxes(identifier,
                                                std::move(qualifier_template_id_syntaxes));
    }
    if(!qualifier_type_syntaxes.empty()) {
      set_cppast_qualifier_type_syntaxes(identifier, std::move(qualifier_type_syntaxes));
    }
    identifier.token_start = name_start;
    identifier.token_end = pos;
    out.children.push_back(std::move(identifier));
  }

  while(true) {
    if(peek().is_simple(OP_LPAREN)) {
      size_t parameters_start = pos;
      CppAstNode parameters;
      if(!parse_parameter_clause(parameters)) {
        pos = parameters_start;
        break;
      }
      out.children.push_back(std::move(parameters));
      if(!parse_function_suffixes(out)) {
        pos = start;
        return false;
      }
      saw_parameters = true;
      continue;
    }

    if(peek().is_simple(OP_LSQUARE) && peek(1).is_simple(OP_LSQUARE)) {
      break;
    }

    if(consume_simple(OP_LSQUARE)) {
      CppAstNode suffix = make_node(CppAstKind::array_suffix);
      if(!peek().is_simple(OP_RSQUARE)) {
        CppAstNode bound;
        if(!parse_expression(bound)) {
          pos = start;
          return false;
        }
        suffix.children.push_back(std::move(bound));
      }
      if(!consume_simple(OP_RSQUARE)) {
        pos = start;
        return false;
      }
      out.children.push_back(std::move(suffix));
      continue;
    }

    break;
  }

  if(require_parameters && !saw_parameters) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_abstract_declarator(CppAstNode & out,
                                             bool require_parameters)
{
  size_t start = pos;
  bool consumed = false;
  bool saw_parameters = false;

  out = make_node(CppAstKind::declarator);

  while(true) {
    CppAstNode ptr_operator;
    if(!parse_ptr_operator_node(ptr_operator)) {
      break;
    }
    out.children.push_back(std::move(ptr_operator));
    while(is_cv_qualifier(peek()) || is_nullability_qualifier_token(peek()) ||
          is_gnu_restrict_qualifier_token(peek())) {
      if(is_gnu_restrict_qualifier_token(peek())) {
        ++pos;
        continue;
      }
      if(is_cv_qualifier(peek())) {
        out.children.push_back(make_token_node(CppAstKind::cv_qualifier, peek()));
      } else {
        out.children.push_back(make_token_node(CppAstKind::nullability_qualifier, peek()));
      }
      ++pos;
    }
    consumed = true;
  }

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
    consumed = true;
  }

  if(!skip_attribute_specifier_seq()) {
    pos = start;
    return false;
  }

  size_t nested_start = pos;
  if(consume_simple(OP_LPAREN)) {
    CppAstNode nested;
    if(parse_abstract_declarator(nested) &&
       !is_bare_parameter_pack_abstract_declarator(nested) &&
       consume_simple(OP_RPAREN)) {
      CppAstNode nested_node = make_node(CppAstKind::nested_declarator);
      nested_node.children.push_back(std::move(nested));
      out.children.push_back(std::move(nested_node));
      consumed = true;
    } else {
      pos = nested_start;
    }
  }

  while(true) {
    if(peek().is_simple(OP_LPAREN)) {
      size_t parameters_start = pos;
      CppAstNode parameters;
      if(!parse_parameter_clause(parameters)) {
        pos = parameters_start;
        break;
      }
      out.children.push_back(std::move(parameters));
      if(!parse_function_suffixes(out)) {
        pos = start;
        return false;
      }
      consumed = true;
      saw_parameters = true;
      continue;
    }

    if(peek().is_simple(OP_LSQUARE) && peek(1).is_simple(OP_LSQUARE)) {
      break;
    }

    if(consume_simple(OP_LSQUARE)) {
      CppAstNode suffix = make_node(CppAstKind::array_suffix);
      if(!peek().is_simple(OP_RSQUARE)) {
        CppAstNode bound;
        if(!parse_expression(bound)) {
          pos = start;
          return false;
        }
        suffix.children.push_back(std::move(bound));
      }
      if(!consume_simple(OP_RSQUARE)) {
        pos = start;
        return false;
      }
      out.children.push_back(std::move(suffix));
      consumed = true;
      continue;
    }

    break;
  }

  if(!consumed || (require_parameters && !saw_parameters)) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_new_abstract_declarator(CppAstNode & out)
{
  size_t start = pos;
  bool consumed = false;

  out = make_node(CppAstKind::declarator);

  while(true) {
    CppAstNode ptr_operator;
    if(!parse_ptr_operator_node(ptr_operator)) {
      break;
    }
    out.children.push_back(std::move(ptr_operator));
    while(is_cv_qualifier(peek()) || is_nullability_qualifier_token(peek()) ||
          is_gnu_restrict_qualifier_token(peek())) {
      if(is_gnu_restrict_qualifier_token(peek())) {
        ++pos;
        continue;
      }
      if(is_cv_qualifier(peek())) {
        out.children.push_back(make_token_node(CppAstKind::cv_qualifier, peek()));
      } else {
        out.children.push_back(make_token_node(CppAstKind::nullability_qualifier, peek()));
      }
      ++pos;
    }
    consumed = true;
  }

  while(!(peek().is_simple(OP_LSQUARE) && peek(1).is_simple(OP_LSQUARE)) &&
        consume_simple(OP_LSQUARE)) {
    CppAstNode suffix = make_node(CppAstKind::array_suffix);
    if(!peek().is_simple(OP_RSQUARE)) {
      CppAstNode bound;
      if(!parse_expression(bound)) {
        pos = start;
        return false;
      }
      suffix.children.push_back(std::move(bound));
    }
    if(!consume_simple(OP_RSQUARE)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(suffix));
    consumed = true;
  }

  if(!consumed) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_ptr_operator_node(CppAstNode & out)
{
  size_t start = pos;
  if(peek().is_simple(OP_STAR) || peek().is_simple(OP_XOR) ||
     peek().is_simple(OP_AMP) ||
     peek().is_simple(OP_LAND)) {
    out = make_token_node(CppAstKind::ptr_operator, peek());
    ++pos;
    return true;
  }

  cpp_decl::QualifiedName qualified_syntax;
  cpp_decl::TemplateIdSyntax template_id_syntax;
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  vector<CppAstNode> qualifier_type_syntaxes;
  string qualified_text;
  if(parse_qualified_name_text(qualified_text,
                               &qualified_syntax,
                               &template_id_syntax,
                               &qualifier_template_id_syntaxes,
                               &qualifier_type_syntaxes,
                               true) &&
     consume_simple(OP_COLON2) &&
     consume_simple(OP_STAR)) {
    out = make_node(CppAstKind::ptr_operator, token_span_text_spaced(start, pos));
    set_cppast_qualified_name_syntax(out, std::move(qualified_syntax));
    if(!template_id_syntax.name.name.empty()) {
      set_cppast_template_id_syntax(out, std::move(template_id_syntax));
    }
    if(!qualifier_template_id_syntaxes.empty()) {
      set_cppast_qualifier_template_id_syntaxes(
          out,
          std::move(qualifier_template_id_syntaxes));
    }
    if(!qualifier_type_syntaxes.empty()) {
      set_cppast_qualifier_type_syntaxes(out, std::move(qualifier_type_syntaxes));
    }
    return true;
  }
  pos = start;

  size_t qualifier_start = pos;
  const bool rooted = consume_simple(OP_COLON2);
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
  vector<qualified_name_parser::NameComponentParseResult> components;
  vector<size_t> component_starts;
  while(true) {
    const size_t component_start = pos;
    qualified_name_parser::NameComponentParseResult component;
    if(!qualified_name_parser::parse_name_component(tokens,
                                                     component_start,
                                                     lookup,
                                                     component)) {
      break;
    }
    pos = component.end;
    if(!consume_simple(OP_COLON2)) {
      pos = component_start;
      break;
    }
    component_starts.push_back(component_start);
    components.push_back(std::move(component));
  }
  if(!components.empty() && consume_simple(OP_STAR)) {
    out = make_node(CppAstKind::ptr_operator, token_span_text_spaced(start, pos));

    qualified_name_parser::QualifiedNameParseResult owner;
    owner.rooted = rooted;
    for(size_t i = 0; i + 1 < components.size(); ++i) {
      owner.qualifiers.push_back(
          make_pair(component_starts[i], components[i].end));
      owner.qualifier_components.push_back(components[i]);
    }
    const size_t final_index = components.size() - 1;
    owner.name_component =
        make_pair(component_starts[final_index], components[final_index].end);
    owner.name_template_head_component = components[final_index].name_component;
    owner.name_kind = qualified_name_parser::UNQ_COMPONENT;
    owner.name_has_template_suffix = components[final_index].has_template_suffix;
    owner.name_template_arg_ranges = components[final_index].template_arg_ranges;
    owner.end = components[final_index].end;

    set_cppast_qualified_name_syntax(out,
                                     build_qualified_name_syntax(tokens, owner));
    CppAstParser * template_argument_parser_context =
        suppress_template_argument_fragment_syntax ? nullptr : this;
    if(owner.name_has_template_suffix) {
      cpp_decl::TemplateIdSyntax owner_template_id;
      if(build_template_id_syntax(tokens,
                                  lookup,
                                  owner,
                                  owner_template_id,
                                  template_argument_parser_context)) {
        set_cppast_template_id_syntax(out, std::move(owner_template_id));
      }
    }
    vector<cpp_decl::TemplateIdSyntax> owner_qualifier_template_ids;
    build_qualifier_template_id_syntaxes(tokens,
                                         lookup,
                                         owner,
                                         owner_qualifier_template_ids,
                                         template_argument_parser_context);
    if(!owner_qualifier_template_ids.empty()) {
      set_cppast_qualifier_template_id_syntaxes(
          out,
          std::move(owner_qualifier_template_ids));
    }
    return true;
  }

  pos = qualifier_start;
  return false;
}

bool CppAstParser::parse_function_suffixes(CppAstNode & out)
{
  size_t start = pos;

  while(true) {
    size_t attribute_start = pos;
    if(!skip_attribute_specifier_seq(&out)) {
      pos = start;
      return false;
    }
    if(pos != attribute_start) {
      continue;
    }

    if(is_cv_qualifier(peek())) {
      out.children.push_back(make_token_node(CppAstKind::cv_qualifier, peek()));
      ++pos;
      continue;
    }

    if(peek().is_simple(OP_AMP) || peek().is_simple(OP_LAND)) {
      out.children.push_back(make_token_node(CppAstKind::ref_qualifier, peek()));
      ++pos;
      continue;
    }

    if(peek().is_simple(KW_NOEXCEPT) || peek().is_simple(KW_THROW)) {
      size_t qualifier_start = pos;
      const bool is_noexcept = peek().is_simple(KW_NOEXCEPT);
      CppAstNode qualifier_node = make_node(CppAstKind::function_qualifier);
      ++pos;
      if(is_noexcept && consume_simple(OP_LPAREN)) {
        CppAstNode expression;
        if(!parse_expression(expression) || !consume_simple(OP_RPAREN)) {
          pos = start;
          return false;
        }
        qualifier_node.children.push_back(std::move(expression));
      } else if(peek().is_simple(OP_LPAREN)) {
        const size_t paren_start = pos;
        std::vector<CppAstNode> exception_type_ids;
        bool parsed_exception_types = false;
        ++pos;
        if(consume_simple(OP_RPAREN)) {
          parsed_exception_types = true;
        } else {
          for(;;) {
            CppAstNode type_id;
            if(!parse_type_id(type_id)) {
              break;
            }
            exception_type_ids.push_back(std::move(type_id));
            if(consume_simple(OP_RPAREN)) {
              parsed_exception_types = true;
              break;
            }
            if(!consume_simple(OP_COMMA)) {
              break;
            }
          }
        }
        if(parsed_exception_types) {
          set_cppast_exception_type_id_syntaxes(qualifier_node,
                                                std::move(exception_type_ids));
        } else {
          pos = paren_start;
          if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
            pos = start;
            return false;
          }
        }
      }
      qualifier_node.value = token_span_text(qualifier_start, pos);
      out.children.push_back(std::move(qualifier_node));
      continue;
    }

    if(peek().is_override() || peek().is_final()) {
      out.children.push_back(make_token_node(CppAstKind::virt_specifier, peek()));
      ++pos;
      continue;
    }

    if(consume_simple(OP_ARROW)) {
      size_t type_start = pos;
      CppAstNode trailing_type;
      if(!parse_type_id(trailing_type)) {
        pos = start;
        return false;
      }
      CppAstNode trailing = make_node(CppAstKind::trailing_return_type,
                                      token_span_text_spaced(type_start, pos));
      attach_qualified_name_syntax_from_span(trailing, type_start, pos);
      trailing.children.push_back(std::move(trailing_type));
      set_span(trailing, type_start);
      out.children.push_back(std::move(trailing));
      continue;
    }

    break;
  }

  return true;
}

bool CppAstParser::parse_template_parameter_clause(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LT)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::template_parameter_clause);

  CppAstNode parameters;
  template_type_parameter_scopes.push_back(NameSet());
  template_value_parameter_scopes.push_back(NameSet());
  template_name_scopes.push_back(NameSet());
  value_name_scopes.push_back(NameSet());
  const bool have_parameters = parse_template_parameter_list(parameters);
  value_name_scopes.pop_back();
  template_name_scopes.pop_back();
  template_value_parameter_scopes.pop_back();
  template_type_parameter_scopes.pop_back();
  if(have_parameters) {
    out.children.push_back(std::move(parameters));
  }

  if(!consume_close_angle_bracket()) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_template_parameter_list(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode parameter;
  if(!parse_template_parameter(parameter)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::template_parameter_list);
  if(!template_type_parameter_scopes.empty()) {
    collect_template_parameter_names(parameter, template_type_parameter_scopes.back());
  }
  if(!template_value_parameter_scopes.empty()) {
    collect_template_parameter_value_names(parameter,
                                           template_value_parameter_scopes.back());
  }
  if(!value_name_scopes.empty()) {
    collect_template_parameter_value_names(parameter,
                                           value_name_scopes.back());
  }
  if(!template_name_scopes.empty()) {
    collect_template_parameter_template_names(parameter, template_name_scopes.back());
  }
  note_name_lookup_mutation();
  out.children.push_back(std::move(parameter));

  while(consume_simple(OP_COMMA)) {
    CppAstNode next;
    if(!parse_template_parameter(next)) {
      pos = start;
      return false;
    }
    if(!template_type_parameter_scopes.empty()) {
      collect_template_parameter_names(next, template_type_parameter_scopes.back());
    }
    if(!template_value_parameter_scopes.empty()) {
      collect_template_parameter_value_names(next,
                                             template_value_parameter_scopes.back());
    }
    if(!value_name_scopes.empty()) {
      collect_template_parameter_value_names(next,
                                             value_name_scopes.back());
    }
    if(!template_name_scopes.empty()) {
      collect_template_parameter_template_names(next, template_name_scopes.back());
    }
    note_name_lookup_mutation();
    out.children.push_back(std::move(next));
  }

  return true;
}

bool CppAstParser::parse_template_parameter(CppAstNode & out)
{
  size_t start = pos;
  if(parse_type_parameter(out)) {
    return true;
  }

  pos = start;
  if(parse_non_type_template_parameter(out)) {
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_type_parameter(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(KW_CLASS) || consume_simple(KW_TYPENAME)) {
    out = make_node(CppAstKind::type_parameter);
    out.children.push_back(make_token_node(CppAstKind::parameter_key, tokens[pos - 1]));

    if(consume_simple(OP_DOTS)) {
      out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
    }

    if(peek().is_identifier()) {
      out.children.push_back(make_node(CppAstKind::identifier, peek().source));
      ++pos;
      if(!(peek().is_simple(OP_ASS) ||
           peek().is_simple(OP_COMMA) ||
           peek().is_close_angle_bracket())) {
        pos = start;
        return false;
      }
    } else if(!(peek().is_simple(OP_ASS) ||
                peek().is_simple(OP_COMMA) ||
                peek().is_close_angle_bracket())) {
      pos = start;
      return false;
    }

    CppAstNode default_argument;
    if(parse_default_template_argument(default_argument)) {
      out.children.push_back(std::move(default_argument));
    }

    return true;
  }

  pos = start;
  if(!consume_simple(KW_TEMPLATE)) {
    pos = start;
    return false;
  }

  CppAstNode inner_clause;
  if(!parse_template_parameter_clause(inner_clause)) {
    pos = start;
    return false;
  }

  if(!(consume_simple(KW_CLASS) || consume_simple(KW_TYPENAME))) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::type_parameter);
  out.children.push_back(make_node(CppAstKind::template_template_parameter));
  out.children.push_back(std::move(inner_clause));
  out.children.push_back(make_token_node(CppAstKind::parameter_key, tokens[pos - 1]));

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
  }

  if(peek().is_identifier()) {
    out.children.push_back(make_node(CppAstKind::identifier, peek().source));
    ++pos;
  }

  CppAstNode default_argument;
  if(parse_default_template_argument(default_argument)) {
    out.children.push_back(std::move(default_argument));
  }

  return true;
}

bool CppAstParser::parse_non_type_template_parameter(CppAstNode & out)
{
  size_t start = pos;

  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::non_type_template_parameter);
  out.children.push_back(std::move(specifiers));

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
  }

  CppAstNode declarator;
  if(parse_declarator(declarator)) {
    out.children.push_back(std::move(declarator));
  } else if(parse_abstract_declarator(declarator)) {
    declarator.kind = CppAstKind::abstract_declarator;
    out.children.push_back(std::move(declarator));
  }

  if(consume_simple(OP_ASS)) {
    CppAstNode expr;
    if(!parse_non_type_template_default_argument(expr)) {
      pos = start;
      return false;
    }
    CppAstNode default_argument = make_node(CppAstKind::default_template_argument);
    default_argument.children.push_back(std::move(expr));
    out.children.push_back(std::move(default_argument));
  }

  return true;
}

bool CppAstParser::parse_non_type_template_default_argument(CppAstNode & out)
{
  size_t start = pos;
  const auto can_maybe_start_declaration = [&](size_t index) -> bool
  {
    const RecogToken & token = tokens.peek(index);
    if(token.is_eof() || token.is_literal()) {
      return false;
    }
    if(token.is_identifier()) {
      return true;
    }
    if(token.is_simple(OP_LSQUARE)) {
      return tokens.peek(index + 1).is_simple(OP_LSQUARE);
    }
    if(token.is_simple(OP_SEMICOLON) ||
       token.is_simple(OP_COLON2) ||
       token.is_simple(KW_NAMESPACE) ||
       token.is_simple(KW_EXTERN) ||
       token.is_simple(KW_USING) ||
       token.is_simple(KW_TEMPLATE) ||
       token.is_simple(KW_ENUM) ||
       token.is_simple(KW_STATIC_ASSERT) ||
       token.is_simple(KW_TYPENAME) ||
       token.is_simple(KW_DECLTYPE) ||
       token.is_simple(KW_EXPLICIT) ||
       token.is_simple(KW_OPERATOR) ||
       token.is_simple(KW_ALIGNAS) ||
       token.is_simple(OP_COMPL)) {
      return true;
    }
    return is_class_key(token) ||
           is_cv_qualifier(token) ||
           is_simple_type_specifier(token) ||
           is_decl_specifier_keyword(token) ||
           is_member_function_specifier(token);
  };
  const auto next_starts_declaration = [&](size_t index) -> bool
  {
    if(tokens.peek(index).is_eof()) {
      return false;
    }
    if(!can_maybe_start_declaration(index)) {
      return false;
    }
    const DeclarationStartProbeCacheKey cache_key =
        make_declaration_start_probe_cache_key(index);
    const auto cached = declaration_start_probe_cache.find(cache_key);
    if(cached != declaration_start_probe_cache.end()) {
      return cached->second;
    }

    RecogTokenSuffixSequence fragment(tokens, index);
    CppAstParser parser(fragment);
    parser.inherit_name_lookup_state_from(*this);
    parser.suppress_template_argument_fragment_syntax = true;

    CppAstNode declaration;
    const bool result = parser.parse_declaration(declaration);
    declaration_start_probe_cache[cache_key] = result;
    return result;
  };

  pos = start;
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t boundary = pos; !tokens.peek(boundary).is_eof(); ++boundary) {
    const RecogToken & token = tokens.peek(boundary);
    if(token.is_simple(OP_LT)) {
      ++angle_depth;
    } else if(token.is_close_angle_bracket()) {
      if(angle_depth > 0) {
        --angle_depth;
      }
    } else if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
    } else if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        break;
      }
      --paren_depth;
    } else if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
    } else if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        break;
      }
      --bracket_depth;
    } else if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
    } else if(token.is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        break;
      }
      --brace_depth;
    }

    const bool top_level = angle_depth == 0 &&
                           paren_depth == 0 &&
                           bracket_depth == 0 &&
                           brace_depth == 0;
    const bool close_angle_boundary =
        top_level &&
        (token.is_close_angle_bracket() || token.is_simple(OP_GT)) &&
        next_starts_declaration(boundary + 1);
    if(top_level && (token.is_simple(OP_COMMA) || close_angle_boundary)) {
      vector<size_t> candidates;
      if(close_angle_boundary) {
        candidates.push_back(boundary + 1);
      }
      candidates.push_back(boundary);
      for(size_t k = 0; k < candidates.size(); ++k) {
        size_t candidate = candidates[k];
        if(candidate <= start) {
          continue;
        }
        RecogTokenRangeSequence fragment(tokens, start, candidate);
        CppAstParser parser(fragment);
        parser.inherit_name_lookup_state_from(*this);
        if(parser.parse_assignment_expression(out) && parser.at_eof()) {
          pos = candidate;
          return true;
        }
      }
      if(token.is_simple(OP_COMMA)) {
        break;
      }
    }
  }

  pos = start;
  if(peek().is_literal()) {
    out = make_token_node(CppAstKind::literal, peek());
    ++pos;
    return true;
  }

  if(peek().is_simple(KW_TRUE) ||
     peek().is_simple(KW_FALSE) ||
     peek().is_simple(KW_NULLPTR)) {
    out = make_token_node(CppAstKind::keyword_literal, peek());
    ++pos;
    return true;
  }

  pos = start;
  if(error_msg.empty()) {
    ostringstream diag;
    diag << "failed non-type template default argument near "
         << token_label(peek()) << " with template type scopes=[";
    for(size_t i = 0; i < template_type_parameter_scopes.size(); ++i) {
      if(i != 0) {
        diag << "; ";
      }
      vector<string> names;
      for(NameSet::const_iterator it = template_type_parameter_scopes[i].begin();
          it != template_type_parameter_scopes[i].end();
          ++it) {
        names.push_back(**it);
      }
      sort(names.begin(), names.end());
      bool first = true;
      for(const auto & name : names) {
        if(!first) {
          diag << ",";
        }
        diag << name;
        first = false;
      }
    }
    diag << "]";
    set_error(diag.str());
  }
  return false;
}

bool CppAstParser::parse_default_template_argument(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_ASS)) {
    pos = start;
    return false;
  }

  CppAstNode type_id;
  if(!parse_type_id(type_id)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::default_template_argument);
  out.children.push_back(std::move(type_id));
  return true;
}

bool CppAstParser::parse_base_clause(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_COLON)) {
    pos = start;
    return false;
  }

  CppAstNode base_specifier;
  if(!parse_base_specifier(base_specifier)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::base_clause);
  out.children.push_back(std::move(base_specifier));

  while(consume_simple(OP_COMMA)) {
    CppAstNode next;
    if(!parse_base_specifier(next)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(next));
  }

  return true;
}

bool CppAstParser::parse_base_specifier(CppAstNode & out)
{
  size_t start = pos;
  out = make_node(CppAstKind::base_specifier);

  bool saw_virtual = false;
  bool saw_access = false;
  while(true) {
    if(!saw_virtual && consume_simple(KW_VIRTUAL)) {
      out.children.push_back(make_token_node(CppAstKind::virtual_node, tokens[pos - 1]));
      saw_virtual = true;
      continue;
    }

    if(!saw_access && is_access_specifier_token(peek())) {
      out.children.push_back(make_token_node(CppAstKind::access_specifier, peek()));
      ++pos;
      saw_access = true;
      continue;
    }

    break;
  }

  size_t name_start = pos;
  if(is_decltype_token(peek()) ||
     is_gnu_typeof_token(peek())) {
    const bool is_typeof = is_gnu_typeof_token(peek());
    ++pos;
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }
    CppAstNode base_name =
        make_node(CppAstKind::base_name, token_span_text(name_start, pos));
    CppAstNode type_spec =
        make_node(CppAstKind::decltype_specifier,
                  token_span_text_spaced(name_start, pos));
    type_spec.is_typeof_specifier = is_typeof;
    type_spec.token_start = name_start;
    type_spec.token_end = pos;
    type_spec.source_location_id = tokens[name_start].location_id;
    CppAstNode operand;
    if(parse_decltype_or_typeof_operand_node(name_start, pos, is_typeof, operand)) {
      type_spec.children.push_back(std::move(operand));
    }
    base_name.base_type_syntax.reset(new CppAstNode(std::move(type_spec)));
    out.children.push_back(std::move(base_name));
  }
  else {
    string name;
    if(!parse_type_name_text(name)) {
      pos = start;
      return false;
    }
    CppAstNode base_name = make_node(CppAstKind::base_name, name);
    attach_qualified_name_syntax_from_span(base_name, name_start, pos);
    out.children.push_back(std::move(base_name));
  }

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::ellipsis, "..."));
  }

  return true;
}

bool CppAstParser::parse_parameter_clause(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::parameter_clause);

  if(consume_simple(OP_RPAREN)) {
    set_span(out, start);
    return true;
  }

  if(consume_simple(OP_DOTS)) {
    out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
    if(!consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  CppAstNode parameter;
  if(!parse_parameter_declaration(parameter)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(parameter));

  while(consume_simple(OP_COMMA)) {
    if(consume_simple(OP_DOTS)) {
      out.children.push_back(make_node(CppAstKind::parameter_pack, "..."));
      break;
    }

    CppAstNode next;
    if(!parse_parameter_declaration(next)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(next));
  }

  if(!consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_parameter_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(OP_DOTS)) {
    out = make_node(CppAstKind::parameter_pack, "...");
    set_span(out, start);
    return true;
  }

  if(!skip_attribute_specifier_seq()) {
    pos = start;
    return false;
  }

  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::parameter_declaration);
  out.children.push_back(std::move(specifiers));

  CppAstNode declarator;
  bool have_declarator = false;
  if(peek().is_simple(OP_LPAREN)) {
    const size_t abstract_start = pos;
    if(parse_abstract_declarator(declarator) &&
       declarator_has_parameter_clause(declarator)) {
      have_declarator = true;
    } else {
      pos = abstract_start;
      declarator = CppAstNode();
    }
  }
  if(!have_declarator && parse_declarator(declarator)) {
    have_declarator = true;
  }
  if(!have_declarator && parse_abstract_declarator(declarator)) {
    have_declarator = true;
  }
  if(have_declarator) {
    out.children.push_back(std::move(declarator));
  }

  if(!skip_attribute_specifier_seq(&out)) {
    pos = start;
    return false;
  }

  if(!skip_trailing_declarator_extensions()) {
    pos = start;
    return false;
  }

  if(peek().is_simple(OP_ASS)) {
    CppAstNode default_argument = make_node(CppAstKind::default_argument);
    CppAstNode initializer;
    if(!parse_initializer(initializer)) {
      pos = start;
      return false;
    }
    const std::size_t initializer_start = initializer.token_start;
    default_argument.children.push_back(std::move(initializer));
    set_span(default_argument, initializer_start);
    out.children.push_back(std::move(default_argument));
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_ctor_initializer(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_COLON)) {
    pos = start;
    return false;
  }

  CppAstNode initializer;
  if(!parse_mem_initializer(initializer)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::ctor_initializer);
  out.children.push_back(std::move(initializer));

  while(consume_simple(OP_COMMA)) {
    CppAstNode next;
    if(!parse_mem_initializer(next)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(next));
  }

  return true;
}

bool CppAstParser::parse_mem_initializer(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode initializer_id;
  if(!parse_mem_initializer_id(initializer_id)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::mem_initializer);
  out.children.push_back(std::move(initializer_id));

  if(peek().is_simple(OP_LPAREN)) {
    CppAstNode arguments;
    if(!parse_paren_argument_list(arguments)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(arguments));
    if(consume_simple(OP_DOTS)) {
      out.children.push_back(make_node(CppAstKind::pack_expansion_expression, "..."));
    }
    return true;
  }

  if(peek().is_simple(OP_LBRACE)) {
    CppAstNode braces;
    if(!parse_braced_init_list(braces)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(braces));
    if(consume_simple(OP_DOTS)) {
      out.children.push_back(make_node(CppAstKind::pack_expansion_expression, "..."));
    }
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_mem_initializer_id(CppAstNode & out)
{
  size_t start = pos;
  size_t name_start = pos;
  if((is_decltype_token(peek()) && (++pos, true)) ||
     (is_gnu_typeof_token(peek()) && (++pos, true))) {
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::mem_initializer_id, token_span_text(name_start, pos));
    return true;
  }

  string name;
  if(!parse_type_name_text(name)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::mem_initializer_id, name);
  attach_qualified_name_syntax_from_span(out, start, pos);
  return true;
}

bool CppAstParser::parse_paren_argument_list(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::paren_argument_list);
  if(consume_simple(OP_RPAREN)) {
    return true;
  }

  while(true) {
    CppAstNode element;
    if(!parse_braced_init_list(element) && !parse_assignment_expression(element)) {
      pos = start;
      return false;
    }
    if(consume_simple(OP_DOTS)) {
      CppAstNode expanded = make_node(CppAstKind::pack_expansion_expression);
      expanded.children.push_back(std::move(element));
      element = std::move(expanded);
    }
    out.children.push_back(std::move(element));

    if(consume_simple(OP_RPAREN)) {
      return true;
    }

    if(!consume_simple(OP_COMMA)) {
      pos = start;
      return false;
    }
  }
}

bool CppAstParser::parse_condition(CppAstNode & out, ETokenType terminator)
{
  size_t start = pos;
  if(can_start_decl_specifier_seq() || can_start_attributed_decl_specifier_seq()) {
    if(parse_condition_declaration_candidate(out)) {
      if(peek().is_simple(terminator)) {
        return true;
      }
      pos = start;
    }
  }

  pos = start;
  if(!parse_expression(out)) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_condition_declaration_candidate(CppAstNode & out)
{
  size_t start = pos;
  skip_attribute_specifier_seq();
  CppAstNode specifiers;
  CppAstNode declarator;
  CppAstNode initializer;
  if(!parse_decl_specifier_seq(specifiers) || !parse_declarator(declarator) ||
     !parse_initializer(initializer)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::condition_declaration);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarator));
  out.children.push_back(std::move(initializer));
  return true;
}

bool CppAstParser::parse_parenthesized_type_id_or_expression(
    CppAstNode & out, bool & is_type_id, bool allow_expression)
{
  size_t start = pos;
  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  if(can_start_type_id()) {
    CppAstNode type_id;
    if(parse_type_id(type_id) && consume_simple(OP_RPAREN)) {
      const size_t type_id_finish = pos;
      if(parenthesized_type_id_prefers_expression(type_id)) {
        if(allow_expression) {
          pos = start + 1;
          CppAstNode expr;
          if(parse_expression(expr) && consume_simple(OP_RPAREN)) {
            out = std::move(expr);
            is_type_id = false;
            return true;
          }
        }
        pos = start;
        return false;
      }
      const bool functional_cast_abstract_declarator =
          allow_expression &&
          type_id_has_function_style_abstract_declarator(type_id);
      if(functional_cast_abstract_declarator) {
        pos = start + 1;
        CppAstNode expr;
        if(parse_expression(expr) && consume_simple(OP_RPAREN)) {
          if(parser_trace::enabled("parser.fragment")) {
            std::ostringstream trace;
            trace << "parenthesized parse preferred expression over functional type-id text="
                  << token_span_text_spaced(expr.token_start, expr.token_end);
            parser_trace::note("parser.fragment", tokens, start, trace.str());
          }
          out = std::move(expr);
          is_type_id = false;
          return true;
        }
        pos = type_id_finish;
      }
      if(parser_trace::enabled("parser.fragment")) {
        std::ostringstream trace;
        trace << "parenthesized parse chose type-id text="
              << token_span_text_spaced(type_id.token_start, type_id.token_end)
              << " allow_expression=" << (allow_expression ? "true" : "false");
        parser_trace::note("parser.fragment", tokens, start, trace.str());
      }
      out = std::move(type_id);
      is_type_id = true;
      return true;
    }
    pos = start + 1;
  }

  if(!allow_expression) {
    pos = start;
    return false;
  }

  CppAstNode expr;
  if(!parse_expression(expr) || !consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  if(parser_trace::enabled("parser.fragment")) {
    std::ostringstream trace;
    trace << "parenthesized parse chose expression text="
          << token_span_text_spaced(expr.token_start, expr.token_end);
    parser_trace::note("parser.fragment", tokens, start, trace.str());
  }
  out = std::move(expr);
  is_type_id = false;
  return true;
}

bool CppAstParser::parse_decltype_or_typeof_operand_node(std::size_t specifier_start,
                                                         std::size_t specifier_end,
                                                         bool is_typeof,
                                                         CppAstNode & out)
{
  if(specifier_end <= specifier_start + 2 ||
     !tokens.peek(specifier_start + 1).is_simple(OP_LPAREN)) {
    return false;
  }

  const std::size_t operand_start = specifier_start + 2;
  const std::size_t operand_end = specifier_end - 1;
  if(operand_start >= operand_end) {
    return false;
  }

  const std::size_t saved_pos = pos;
  bool parsed = false;
  if(is_typeof) {
    pos = operand_start;
    CppAstNode type_id;
    if(parse_type_id(type_id) && pos == operand_end) {
      const bool functional_cast_abstract_declarator =
          type_id_has_function_style_abstract_declarator(type_id);
      if(parenthesized_type_id_prefers_expression(type_id) ||
         functional_cast_abstract_declarator) {
        pos = operand_start;
        CppAstNode expr;
        if(parse_expression(expr) && pos == operand_end) {
          if(parser_trace::enabled("parser.fragment")) {
            std::ostringstream trace;
            trace << "typeof operand preferred expression over type-id text="
                  << token_span_text_spaced(expr.token_start, expr.token_end);
            parser_trace::note("parser.fragment", tokens, operand_start, trace.str());
          }
          out = std::move(expr);
          parsed = true;
        }
      }
      if(!parsed) {
        out = std::move(type_id);
        parsed = true;
      }
    }
  }
  if(!parsed) {
    pos = operand_start;
    CppAstNode expr;
    if(parse_expression(expr) && pos == operand_end) {
      out = std::move(expr);
      parsed = true;
    }
  }
  pos = saved_pos;
  return parsed;
}

bool CppAstParser::parse_compound_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::compound_statement);
  template_name_scopes.push_back(NameSet());
  type_name_scopes.push_back(NameSet());
  value_name_scopes.push_back(NameSet());

  while(!at_eof() && !peek().is_simple(OP_RBRACE)) {
    CppAstNode item;
    if(!parse_block_item(item)) {
      if(error_msg.empty()) {
        set_error("expected block-item near " + token_label(peek()));
      }
      value_name_scopes.pop_back();
      type_name_scopes.pop_back();
      template_name_scopes.pop_back();
      pos = start;
      return false;
    }
    note_visible_names_after_declaration(item);
    out.children.push_back(std::move(item));
  }

  if(!consume_simple(OP_RBRACE)) {
    value_name_scopes.pop_back();
    type_name_scopes.pop_back();
    template_name_scopes.pop_back();
    pos = start;
    return false;
  }

  value_name_scopes.pop_back();
  type_name_scopes.pop_back();
  template_name_scopes.pop_back();
  return true;
}

bool CppAstParser::parse_function_body(CppAstNode & out)
{
  const size_t start = pos;
  if(parse_lazy_header_compound_function_body(out)) {
    return true;
  }
  if(parse_compound_statement(out) || parse_try_statement(out)) {
    ++lazy_body_stats().eager_function_bodies;
    return true;
  }
  pos = start;
  return false;
}

bool CppAstParser::parse_lazy_header_compound_function_body(CppAstNode & out)
{
  const size_t start = pos;
  if(!lazy_header_function_bodies_enabled() ||
     !peek().is_simple(OP_LBRACE) ||
     !token_is_from_non_primary_source_file(tokens, start)) {
    return false;
  }

  size_t next = start;
  if(!skip_balanced_group_from(tokens, start, next)) {
    ++lazy_body_stats().skip_failures;
    return false;
  }

  NameSet used_names;
  for(size_t i = start; i < next; ++i) {
    const RecogToken & token = tokens.peek(i);
    if(token.is_simple(KW_USING)) {
      return false;
    }
    if(token.is_identifier()) {
      used_names.insert(token.cached_identifier_atom());
    }
  }

  pos = next;
  out = make_node(CppAstKind::lazy_function_body);
  set_span(out, start);
  out.name_lookup_snapshot = snapshot_name_lookup_state(&used_names);
  ++lazy_body_stats().skipped_header_bodies;
  lazy_body_stats().skipped_header_body_tokens += pos - start;
  return true;
}

bool CppAstParser::parse_function_try_body(CppAstNode & out, CppAstNode * ctor_initializer)
{
  const size_t start = pos;
  if(!consume_simple(KW_TRY)) {
    pos = start;
    return false;
  }

  if(ctor_initializer) {
    CppAstNode parsed_ctor_initializer;
    if(parse_ctor_initializer(parsed_ctor_initializer)) {
      *ctor_initializer = std::move(parsed_ctor_initializer);
    }
  }

  CppAstNode body;
  if(!parse_compound_statement(body)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::try_block);
  out.children.push_back(std::move(body));

  bool saw_handler = false;
  while(consume_simple(KW_CATCH)) {
    saw_handler = true;
    if(!consume_simple(OP_LPAREN)) {
      pos = start;
      return false;
    }

    CppAstNode handler = make_node(CppAstKind::handler);
    CppAstNode declaration;
    if(!parse_exception_declaration(declaration) || !consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }
    NameSet handler_value_names;
    collect_declared_value_names(declaration, handler_value_names);
    handler.children.push_back(std::move(declaration));

    CppAstNode handler_body;
    if(!handler_value_names.empty()) {
      value_name_scopes.push_back(handler_value_names);
    }
    const bool parsed_handler_body = parse_compound_statement(handler_body);
    if(!handler_value_names.empty()) {
      value_name_scopes.pop_back();
    }
    if(!parsed_handler_body) {
      pos = start;
      return false;
    }
    handler.children.push_back(std::move(handler_body));
    out.children.push_back(std::move(handler));
  }

  if(!saw_handler) {
    pos = start;
    return false;
  }

  return true;
}

bool CppAstParser::parse_block_item(CppAstNode & out)
{
  size_t start = pos;
  {
    size_t cursor = pos;
    while(is_gnu_extension_token(tokens.peek(cursor))) {
      ++cursor;
    }
    if(tokens.peek(cursor).is_simple(KW_USING)) {
      pos = cursor;
      if(parse_using_or_alias_declaration(out)) {
        return true;
      }
      pos = start;
    }
  }

  if(can_start_block_declaration()) {
    if(parse_declaration(out)) {
      return true;
    }
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "block declaration candidate rejected near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, pos, trace.str());
    }
    pos = start;
  }

  if(parse_statement(out)) {
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_class_member(CppAstNode & out)
{
  size_t start = pos;
  if(is_access_specifier_token(peek()) && peek(1).is_simple(OP_COLON)) {
    out = make_token_node(CppAstKind::access_specifier, peek());
    pos += 2;
    return true;
  }

  if(can_start_unqualified_implicit_type_function_candidate() &&
     parse_special_member_declaration(out)) {
    return true;
  }
  if(parser_trace::enabled("parser.decl") &&
     (can_start_unqualified_implicit_type_function_candidate() ||
      can_start_attributed_decl_specifier_seq())) {
    std::ostringstream trace;
    trace << "class special-member candidate rejected near "
          << token_label(peek())
          << " [current-class " << current_class_trace_label(class_name_stack) << "]";
    parser_trace::note("parser.decl", tokens, pos, trace.str());
  }

  pos = start;
  if(peek().is_simple(KW_TEMPLATE) &&
     can_start_template_special_member_candidate() &&
     parse_template_special_member_declaration(out)) {
    return true;
  }
  if(peek().is_simple(KW_TEMPLATE) && parser_trace::enabled("parser.decl")) {
    std::ostringstream trace;
    trace << "class template special-member candidate rejected near "
          << token_label(peek());
    parser_trace::note("parser.decl", tokens, pos, trace.str());
  }

  pos = start;
  if(parse_declaration(out)) {
    return true;
  }
  if(parser_trace::enabled("parser.decl")) {
    std::ostringstream trace;
    trace << "class declaration candidate rejected near "
          << token_label(peek())
          << " [current-class " << current_class_trace_label(class_name_stack) << "]";
    parser_trace::note("parser.decl", tokens, pos, trace.str());
  }

  pos = start;
  if(parse_bit_field_declaration(out)) {
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_statement(CppAstNode & out)
{
  size_t start = pos;
  size_t attribute_start = pos;
  if(skip_attribute_specifier_seq() && pos != attribute_start) {
    const size_t attribute_end = pos;
    CppAstNode statement;
    if(parse_statement(statement)) {
      out = make_node(CppAstKind::attributed_statement,
                      token_span_text_spaced(attribute_start, attribute_end));
      out.children.push_back(std::move(statement));
      set_span(out, start);
      return true;
    }
    pos = start;
  }

  if(peek().is_simple(OP_LBRACE)) {
    return parse_compound_statement(out);
  }
  if(can_start_statement_as_label()) {
    return parse_labeled_statement(out);
  }
  if(peek().is_simple(KW_IF)) {
    return parse_if_statement(out);
  }
  if(peek().is_simple(KW_SWITCH)) {
    return parse_switch_statement(out);
  }
  if(peek().is_simple(KW_WHILE)) {
    return parse_while_statement(out);
  }
  if(peek().is_simple(KW_DO)) {
    return parse_do_statement(out);
  }
  if(peek().is_simple(KW_FOR)) {
    return parse_for_statement(out);
  }
  if(peek().is_simple(KW_BREAK) || peek().is_simple(KW_CONTINUE) ||
     peek().is_simple(KW_GOTO) || peek().is_simple(KW_THROW)) {
    return parse_jump_statement(out);
  }
  if(peek().is_simple(KW_TRY)) {
    return parse_try_statement(out);
  }
  if(is_gnu_asm_token(peek())) {
    return parse_asm_statement(out);
  }
  if(peek().is_simple(KW_RETURN)) {
    return parse_return_statement(out);
  }
  if(can_parse_coroutine_contextual_keyword_in_template(
         peek(),
         "co_return",
         template_declaration_depth,
         is_known_value_name_identifier(peek()))) {
    return parse_coroutine_return_statement(out);
  }
  if(can_start_block_declaration()) {
    if(parse_declaration(out)) {
      return true;
    }
    if(parser_trace::enabled("parser.decl")) {
      std::ostringstream trace;
      trace << "statement declaration candidate rejected near "
            << token_label(peek());
      parser_trace::note("parser.decl", tokens, pos, trace.str());
    }
    pos = start;
  }
  if(parse_expression_statement(out)) {
    return true;
  }
  pos = start;
  return false;
}

bool CppAstParser::parse_labeled_statement(CppAstNode & out)
{
  size_t start = pos;
  const auto parse_labeled_substatement = [&](CppAstNode & child) -> bool
  {
    const size_t substatement_start = pos;
    if(can_start_block_declaration()) {
      if(parse_declaration(child)) {
        return true;
      }
      pos = substatement_start;
    }
    return parse_statement(child);
  };

  if(peek().is_identifier() && peek(1).is_simple(OP_COLON)) {
    string name = peek().source;
    pos += 2;
    CppAstNode statement;
    if(!parse_labeled_substatement(statement)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::labeled_statement, name);
    out.children.push_back(std::move(statement));
    set_span(out, start);
    return true;
  }

  if(consume_simple(KW_CASE)) {
    CppAstNode expr;
    if(!parse_expression(expr) || !consume_simple(OP_COLON)) {
      pos = start;
      return false;
    }
    CppAstNode statement;
    if(!parse_labeled_substatement(statement)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::case_statement);
    out.children.push_back(std::move(expr));
    out.children.push_back(std::move(statement));
    set_span(out, start);
    return true;
  }

  if(consume_simple(KW_DEFAULT)) {
    if(!consume_simple(OP_COLON)) {
      pos = start;
      return false;
    }
    CppAstNode statement;
    if(!parse_labeled_substatement(statement)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::default_statement);
    out.children.push_back(std::move(statement));
    set_span(out, start);
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_if_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_IF)) {
    pos = start;
    return false;
  }

  const bool is_constexpr = consume_simple(KW_CONSTEXPR);
  if(!consume_simple(OP_LPAREN)) {
    set_error("expected '(' after if near " + token_label(peek()));
    pos = start;
    return false;
  }

  size_t after_lparen = pos;
  CppAstNode init_stmt;
  CppAstNode condition_expr;
  const bool has_init =
      parse_for_init_statement(init_stmt) &&
      parse_condition(condition_expr, OP_RPAREN) &&
      consume_simple(OP_RPAREN);
  if(!has_init) {
    pos = after_lparen;
    if(!parse_condition(condition_expr, OP_RPAREN) || !consume_simple(OP_RPAREN)) {
      set_error("expected if condition near " + token_label(peek()));
      pos = start;
      return false;
    }
  }

  CppAstNode then_stmt;
  if(!parse_statement(then_stmt)) {
    set_error("expected if statement body near " + token_label(peek()));
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::if_statement);
  if(is_constexpr) {
    out.value = "constexpr";
  }
  if(has_init) {
    out.children.push_back(std::move(init_stmt));
  }
  CppAstNode condition = make_node(CppAstKind::condition);
  condition.token_start = condition_expr.token_start;
  condition.token_end = condition_expr.token_end;
  condition.children.push_back(std::move(condition_expr));
  out.children.push_back(std::move(condition));

  CppAstNode then_branch = make_node(CppAstKind::then_node);
  then_branch.token_start = then_stmt.token_start;
  then_branch.token_end = then_stmt.token_end;
  then_branch.children.push_back(std::move(then_stmt));
  out.children.push_back(std::move(then_branch));

  if(consume_simple(KW_ELSE)) {
    CppAstNode else_stmt;
    if(!parse_statement(else_stmt)) {
      set_error("expected else statement body near " + token_label(peek()));
      pos = start;
      return false;
    }
    CppAstNode else_branch = make_node(CppAstKind::else_node);
    else_branch.token_start = else_stmt.token_start;
    else_branch.token_end = else_stmt.token_end;
    else_branch.children.push_back(std::move(else_stmt));
    out.children.push_back(std::move(else_branch));
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_switch_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_SWITCH) || !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode condition_expr;
  if(!parse_condition(condition_expr, OP_RPAREN) || !consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!parse_statement(body)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::switch_statement);
  CppAstNode condition = make_node(CppAstKind::condition);
  condition.token_start = condition_expr.token_start;
  condition.token_end = condition_expr.token_end;
  condition.children.push_back(std::move(condition_expr));
  out.children.push_back(std::move(condition));
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_while_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_WHILE) || !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode condition_expr;
  if(!parse_condition(condition_expr, OP_RPAREN) || !consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!parse_statement(body)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::while_statement);
  CppAstNode condition = make_node(CppAstKind::condition);
  condition.token_start = condition_expr.token_start;
  condition.token_end = condition_expr.token_end;
  condition.children.push_back(std::move(condition_expr));
  out.children.push_back(std::move(condition));
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_do_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_DO)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!parse_statement(body) || !consume_simple(KW_WHILE) ||
     !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode condition_expr;
  if(!parse_expression(condition_expr) || !consume_simple(OP_RPAREN) ||
     !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::do_statement);
  out.children.push_back(std::move(body));
  CppAstNode condition = make_node(CppAstKind::condition);
  condition.token_start = condition_expr.token_start;
  condition.token_end = condition_expr.token_end;
  condition.children.push_back(std::move(condition_expr));
  out.children.push_back(std::move(condition));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_for_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_FOR) || !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  size_t after_lparen = pos;
  if(can_start_range_declaration()) {
    CppAstNode range_decl;
    CppAstNode range_init;
    if(parse_range_declaration(range_decl) && consume_simple(OP_COLON) &&
       parse_range_initializer(range_init) && consume_simple(OP_RPAREN)) {
      CppAstNode body;
      if(!parse_statement(body)) {
        pos = start;
        return false;
      }

      out = make_node(CppAstKind::range_for_statement);
      out.children.push_back(std::move(range_decl));
      out.children.push_back(std::move(range_init));
      out.children.push_back(std::move(body));
      set_span(out, start);
      return true;
    }
    pos = after_lparen;
  }

  out = make_node(CppAstKind::for_statement);

  CppAstNode init;
  if(!parse_for_init_statement(init)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(init));

  if(!peek().is_simple(OP_SEMICOLON)) {
    CppAstNode condition_expr;
    if(!parse_condition(condition_expr, OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    CppAstNode condition = make_node(CppAstKind::condition);
    condition.children.push_back(std::move(condition_expr));
    out.children.push_back(std::move(condition));
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  if(!peek().is_simple(OP_RPAREN)) {
    CppAstNode iteration_expr;
    if(!parse_expression(iteration_expr)) {
      pos = start;
      return false;
    }
    CppAstNode iteration = make_node(CppAstKind::iteration);
    iteration.children.push_back(std::move(iteration_expr));
    out.children.push_back(std::move(iteration));
  }

  if(!consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!parse_statement(body)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(body));

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_for_init_statement(CppAstNode & out)
{
  size_t start = pos;
  out = make_node(CppAstKind::for_init_statement);

  if(consume_simple(OP_SEMICOLON)) {
    set_span(out, start);
    return true;
  }

  if(can_start_decl_specifier_seq() || can_start_attributed_decl_specifier_seq()) {
    CppAstNode init_decl;
    if(parse_simple_declaration(init_decl)) {
      out.children.push_back(std::move(init_decl));
      set_span(out, start);
      return true;
    }
    pos = start;
  }

  if(peek().is_simple(KW_USING)) {
    CppAstNode using_or_alias;
    if(parse_using_or_alias_declaration(using_or_alias)) {
      out.children.push_back(std::move(using_or_alias));
      set_span(out, start);
      return true;
    }
    pos = start;
  }

  CppAstNode init_expr;
  if(!parse_expression(init_expr) || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(init_expr));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_range_declaration(CppAstNode & out)
{
  size_t start = pos;
  skip_attribute_specifier_seq();
  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  CppAstNode declarator;
  if(can_start_structured_binding_declarator()) {
    if(!parse_structured_binding_declarator(declarator)) {
      pos = start;
      return false;
    }
  } else if(!parse_declarator(declarator)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::range_declaration);
  out.children.push_back(std::move(specifiers));
  out.children.push_back(std::move(declarator));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_structured_binding_declarator(CppAstNode & out)
{
  const size_t start = pos;
  out = make_node(CppAstKind::structured_binding_declarator);

  if(consume_simple(OP_AMP) || consume_simple(OP_LAND)) {
    out.children.push_back(make_token_node(CppAstKind::ref_qualifier, tokens[pos - 1]));
  }

  CppAstNode identifiers;
  if(!parse_structured_binding_identifier_list(identifiers)) {
    pos = start;
    return false;
  }

  out.children.push_back(std::move(identifiers));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_structured_binding_identifier_list(CppAstNode & out)
{
  const size_t start = pos;
  if(!consume_simple(OP_LSQUARE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::structured_binding_identifier_list);
  while(true) {
    if(!peek().is_identifier()) {
      pos = start;
      return false;
    }

    out.children.push_back(make_token_node(CppAstKind::identifier, peek()));
    ++pos;

    if(!skip_attribute_specifier_seq()) {
      pos = start;
      return false;
    }

    if(consume_simple(OP_RSQUARE)) {
      break;
    }

    if(!consume_simple(OP_COMMA)) {
      pos = start;
      return false;
    }
  }

  if(out.children.empty()) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_range_initializer(CppAstNode & out)
{
  size_t start = pos;
  CppAstNode value;
  if(!parse_braced_init_list(value) && !parse_expression(value)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::range_initializer);
  out.children.push_back(std::move(value));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_jump_statement(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(KW_BREAK)) {
    if(!consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::break_statement);
    set_span(out, start);
    return true;
  }

  if(consume_simple(KW_CONTINUE)) {
    if(!consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::continue_statement);
    set_span(out, start);
    return true;
  }

  if(consume_simple(KW_GOTO)) {
    if(!consume_identifier() || !consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::goto_statement, tokens[pos - 2].source);
    set_span(out, start);
    return true;
  }

  if(consume_simple(KW_THROW)) {
    out = make_node(CppAstKind::throw_statement);
    if(!peek().is_simple(OP_SEMICOLON)) {
      CppAstNode expr;
      if(!parse_expression(expr)) {
        pos = start;
        return false;
      }
      out.children.push_back(std::move(expr));
    }
    if(!consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_try_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_TRY)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!parse_compound_statement(body)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::try_block);
  out.children.push_back(std::move(body));

  bool saw_handler = false;
  while(consume_simple(KW_CATCH)) {
    saw_handler = true;
    const size_t handler_start = pos - 1;
    if(!consume_simple(OP_LPAREN)) {
      pos = start;
      return false;
    }

    CppAstNode handler = make_node(CppAstKind::handler);
    CppAstNode declaration;
    if(!parse_exception_declaration(declaration) || !consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }
    NameSet handler_value_names;
    collect_declared_value_names(declaration, handler_value_names);
    handler.children.push_back(std::move(declaration));

    CppAstNode handler_body;
    if(!handler_value_names.empty()) {
      value_name_scopes.push_back(handler_value_names);
    }
    const bool parsed_handler_body = parse_compound_statement(handler_body);
    if(!handler_value_names.empty()) {
      value_name_scopes.pop_back();
    }
    if(!parsed_handler_body) {
      pos = start;
      return false;
    }
    handler.children.push_back(std::move(handler_body));
    set_span(handler, handler_start);
    out.children.push_back(std::move(handler));
  }

  if(!saw_handler) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_asm_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!is_gnu_asm_token(peek())) {
    pos = start;
    return false;
  }
  ++pos;

  const size_t qualifier_start = pos;
  while(is_gnu_asm_qualifier_token(peek())) {
    ++pos;
  }
  const size_t qualifier_end = pos;

  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::asm_statement,
                  token_span_text_spaced(qualifier_start, qualifier_end));

  size_t clause_start = pos;
  size_t paren_depth = 1;
  size_t bracket_depth = 0;
  size_t brace_depth = 0;
  bool closed = false;

  while(!at_eof()) {
    if(peek().is_simple(OP_LPAREN)) {
      ++paren_depth;
      ++pos;
      continue;
    }
    if(peek().is_simple(OP_RPAREN)) {
      if(paren_depth == 1 && bracket_depth == 0 && brace_depth == 0) {
        CppAstNode clause = make_node(CppAstKind::asm_clause,
                                      token_span_text_spaced(clause_start, pos));
        clause.token_start = clause_start;
        clause.token_end = pos;
        out.children.push_back(std::move(clause));
        ++pos;
        closed = true;
        break;
      }
      if(paren_depth == 0) {
        pos = start;
        return false;
      }
      --paren_depth;
      ++pos;
      continue;
    }
    if(peek().is_simple(OP_LSQUARE)) {
      ++bracket_depth;
      ++pos;
      continue;
    }
    if(peek().is_simple(OP_RSQUARE)) {
      if(bracket_depth == 0) {
        pos = start;
        return false;
      }
      --bracket_depth;
      ++pos;
      continue;
    }
    if(peek().is_simple(OP_LBRACE)) {
      ++brace_depth;
      ++pos;
      continue;
    }
    if(peek().is_simple(OP_RBRACE)) {
      if(brace_depth == 0) {
        pos = start;
        return false;
      }
      --brace_depth;
      ++pos;
      continue;
    }
    if(paren_depth == 1 && bracket_depth == 0 && brace_depth == 0 &&
       peek().is_simple(OP_COLON2)) {
      CppAstNode clause =
          make_node(CppAstKind::asm_clause, token_span_text_spaced(clause_start, pos));
      clause.token_start = clause_start;
      clause.token_end = pos;
      out.children.push_back(std::move(clause));
      CppAstNode empty_clause = make_node(CppAstKind::asm_clause, "");
      empty_clause.token_start = pos;
      empty_clause.token_end = pos;
      out.children.push_back(std::move(empty_clause));
      ++pos;
      clause_start = pos;
      continue;
    }
    if(paren_depth == 1 && bracket_depth == 0 && brace_depth == 0 &&
       peek().is_simple(OP_COLON)) {
      CppAstNode clause =
          make_node(CppAstKind::asm_clause, token_span_text_spaced(clause_start, pos));
      clause.token_start = clause_start;
      clause.token_end = pos;
      out.children.push_back(std::move(clause));
      ++pos;
      clause_start = pos;
      continue;
    }
    ++pos;
  }

  if(!closed || !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_exception_declaration(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(OP_DOTS)) {
    out = make_node(CppAstKind::exception_declaration);
    out.children.push_back(make_node(CppAstKind::ellipsis, "..."));
    return true;
  }

  CppAstNode specifiers;
  if(!parse_decl_specifier_seq(specifiers)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::exception_declaration);
  out.children.push_back(std::move(specifiers));

  CppAstNode declarator;
  if(parse_declarator(declarator)) {
    out.children.push_back(std::move(declarator));
    return true;
  }

  if(parse_abstract_declarator(declarator)) {
    declarator.kind = CppAstKind::abstract_declarator;
    out.children.push_back(std::move(declarator));
  }

  return true;
}

bool CppAstParser::parse_return_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(KW_RETURN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::return_statement);

  if(!peek().is_simple(OP_SEMICOLON)) {
    CppAstNode expression;
    if(!parse_expression(expression)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(expression));
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_coroutine_return_statement(CppAstNode & out)
{
  size_t start = pos;
  if(!can_parse_coroutine_contextual_keyword_in_template(
         peek(),
         "co_return",
         template_declaration_depth,
         is_known_value_name_identifier(peek()))) {
    pos = start;
    return false;
  }
  ++pos;

  out = make_node(CppAstKind::coroutine_return_statement, "co_return");

  if(!peek().is_simple(OP_SEMICOLON)) {
    CppAstNode expression;
    if(!parse_braced_init_list(expression) && !parse_expression(expression)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(expression));
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_expression_statement(CppAstNode & out)
{
  size_t start = pos;
  out = make_node(CppAstKind::expression_statement);

  if(!peek().is_simple(OP_SEMICOLON)) {
    CppAstNode expression;
    if(!parse_expression(expression)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(expression));
  }

  if(!consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_assignment_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_COMMA)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_assignment_expression(rhs)) {
      pos = start;
      return false;
    }

    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_assignment_expression(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(KW_THROW)) {
    out = make_node(CppAstKind::throw_statement);
    const size_t after_throw = pos;
    CppAstNode expr;
    if(parse_assignment_expression(expr)) {
      out.children.push_back(std::move(expr));
    } else {
      pos = after_throw;
    }
    set_span(out, start);
    return true;
  }

  if(can_parse_coroutine_contextual_keyword_in_template(
         peek(),
         "co_yield",
         template_declaration_depth,
         is_known_value_name_identifier(peek()))) {
    ++pos;
    out = make_node(CppAstKind::unary_expression, "co_yield");
    CppAstNode operand;
    if(!parse_braced_init_list(operand) && !parse_assignment_expression(operand)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(operand));
    set_span(out, start);
    return true;
  }

  if(!parse_conditional_expression(out)) {
    pos = start;
    return false;
  }

  if(is_assignment_operator(peek())) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;

    CppAstNode rhs;
    if(!parse_assignment_expression(rhs)) {
      pos = start;
      return false;
    }

    CppAstNode combined = make_node(CppAstKind::assignment_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_conditional_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_logical_or_expression(out)) {
    pos = start;
    return false;
  }

  if(consume_simple(OP_QMARK)) {
    CppAstNode then_expr;
    if(!parse_expression(then_expr) || !consume_simple(OP_COLON)) {
      pos = start;
      return false;
    }

    CppAstNode else_expr;
    if(!parse_assignment_expression(else_expr)) {
      pos = start;
      return false;
    }

    CppAstNode conditional = make_node(CppAstKind::conditional_expression);
    conditional.children.push_back(std::move(out));
    conditional.children.push_back(std::move(then_expr));
    conditional.children.push_back(std::move(else_expr));
    out = std::move(conditional);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_logical_or_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_logical_and_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_LOR)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_logical_and_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_logical_and_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_inclusive_or_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_LAND)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_inclusive_or_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_inclusive_or_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_exclusive_or_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_BOR)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_exclusive_or_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_exclusive_or_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_and_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_XOR)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_and_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_and_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_equality_expression(out)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_AMP)) {
    RecogToken op_token = tokens[pos - 1];
    CppAstNode rhs;
    if(!parse_equality_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op_token.source);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_equality_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_relational_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_EQ) || peek().is_simple(OP_NE)) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;

    CppAstNode rhs;
    if(!parse_relational_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_relational_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_shift_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_LT) || peek().is_simple(OP_GT) ||
        peek().is_simple(OP_LE) || peek().is_simple(OP_GE)) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;

    CppAstNode rhs;
    if(!parse_shift_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_shift_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_additive_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_LSHIFT) || peek().is_simple(OP_RSHIFT) ||
        peek().is_rshift_piece()) {
    string op;
    RecogToken op_token;
    bool has_op_token = false;
    if(peek().is_simple(OP_LSHIFT)) {
      op_token = peek();
      has_op_token = true;
      op = op_token.source;
      ++pos;
    }
    else if(peek().is_simple(OP_RSHIFT)) {
      op_token = peek();
      has_op_token = true;
      op = op_token.source;
      ++pos;
    }
    else {
      ++pos;
      if(!peek().is_rshift_piece()) {
        pos = start;
        return false;
      }
      ++pos;
      op = ">>";
    }

    CppAstNode rhs;
    if(!parse_additive_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    if(has_op_token) {
      set_node_token(combined, op_token);
    } else if(op == ">>") {
      set_node_simple_type(combined, OP_RSHIFT);
    }
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_additive_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_multiplicative_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_PLUS) || peek().is_simple(OP_MINUS)) {
    RecogToken op_token = peek();
    const string op = op_token.source;
    ++pos;

    CppAstNode rhs;
    if(!parse_multiplicative_expression(rhs)) {
      pos = start;
      return false;
    }

    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_multiplicative_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_pm_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_STAR) || peek().is_simple(OP_DIV) ||
        peek().is_simple(OP_MOD)) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;

    CppAstNode rhs;
    if(!parse_pm_expression(rhs)) {
      pos = start;
      return false;
    }

    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_pm_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_unary_expression(out)) {
    pos = start;
    return false;
  }

  while(peek().is_simple(OP_DOTSTAR) || peek().is_simple(OP_ARROWSTAR)) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;
    CppAstNode rhs;
    if(!parse_unary_expression(rhs)) {
      pos = start;
      return false;
    }
    CppAstNode combined = make_node(CppAstKind::binary_expression, op);
    set_node_token(combined, op_token);
    combined.children.push_back(std::move(out));
    combined.children.push_back(std::move(rhs));
    out = std::move(combined);
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_unary_expression(CppAstNode & out)
{
  size_t start = pos;
  if(is_gnu_extension_token(peek())) {
    ++pos;
    if(!parse_unary_expression(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(can_parse_coroutine_contextual_keyword_in_template(
         peek(),
         "co_await",
         template_declaration_depth,
         is_known_value_name_identifier(peek()))) {
    ++pos;
    CppAstNode operand;
    if(!parse_unary_expression(operand)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::unary_expression, "co_await");
    out.children.push_back(std::move(operand));
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(KW_STATIC_CAST) || peek().is_simple(KW_DYNAMIC_CAST) ||
     peek().is_simple(KW_CONST_CAST) || peek().is_simple(KW_REINTERPET_CAST)) {
    if(!parse_keyword_cast_expression(out)) {
      pos = start;
      return false;
    }
    if(!parse_postfix_suffixes(out, start)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(KW_NEW) ||
     (peek().is_simple(OP_COLON2) && peek(1).is_simple(KW_NEW))) {
    if(!parse_new_expression(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(KW_DELETE) ||
     (peek().is_simple(OP_COLON2) && peek(1).is_simple(KW_DELETE))) {
    if(!parse_delete_expression(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(KW_TYPEID) || peek().is_simple(KW_ALIGNOF) ||
     is_alignof_type_trait_identifier(peek()) ||
     peek().is_simple(KW_NOEXCEPT) ||
     is_builtin_type_trait_expression_start(tokens, pos)) {
    if(!parse_type_trait_expression(out)) {
      pos = start;
      return false;
    }
    if(!parse_postfix_suffixes(out, start)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(OP_LPAREN)) {
    CppAstNode type_id;
    bool is_type_id = false;
    if(parse_parenthesized_type_id_or_expression(type_id,
                                                 is_type_id,
                                                 false) &&
       is_type_id) {
      const bool postfix_after_parenthesized_functional_cast =
          type_id_has_function_style_abstract_declarator(type_id) &&
          (peek().is_simple(OP_LPAREN) ||
           peek().is_simple(OP_LSQUARE) ||
           peek().is_simple(OP_DOT) ||
           peek().is_simple(OP_ARROW) ||
           peek().is_simple(OP_INC) ||
           peek().is_simple(OP_DEC));
      if(postfix_after_parenthesized_functional_cast) {
        pos = start;
      } else {
        if(parser_trace::enabled("parser.fragment")) {
          std::ostringstream trace;
          trace << "unary cast candidate type-id="
                << token_span_text_spaced(type_id.token_start, type_id.token_end);
          parser_trace::note("parser.fragment", tokens, start, trace.str());
        }
        CppAstNode operand;
        if(!parse_unary_expression(operand)) {
          if(parser_trace::enabled("parser.fragment")) {
            std::ostringstream trace;
            trace << "unary cast candidate rejected missing operand after type-id="
                  << token_span_text_spaced(type_id.token_start, type_id.token_end)
                  << " next=" << token_label(peek());
            parser_trace::note("parser.fragment", tokens, pos, trace.str());
          }
          pos = start;
        } else {
          out = make_node(CppAstKind::cast_expression);
          set_node_simple_type(out, OP_LPAREN);
          out.children.push_back(std::move(type_id));
          out.children.push_back(std::move(operand));
          if(!parse_postfix_suffixes(out, start)) {
            pos = start;
            return false;
          }
          set_span(out, start);
          return true;
        }
      }
    }
    pos = start;
  }

  if(consume_simple(KW_SIZEOF)) {
    if(consume_simple(OP_DOTS)) {
      if(!consume_simple(OP_LPAREN) || !consume_identifier() || !consume_simple(OP_RPAREN)) {
        pos = start;
        return false;
      }
      out = make_node(CppAstKind::sizeof_pack_expression);
      out.children.push_back(make_node(CppAstKind::identifier, tokens[pos - 2].source));
      set_span(out, start);
      return true;
    }

    out = make_node(CppAstKind::sizeof_expression);

    if(consume_simple(OP_LPAREN)) {
      --pos;
      CppAstNode inner;
      bool is_type_id = false;
      if(!parse_parenthesized_type_id_or_expression(inner, is_type_id, true)) {
        pos = start;
        return false;
      }
      out.children.push_back(std::move(inner));
      set_span(out, start);
      return true;
    }

    CppAstNode operand;
    if(!parse_unary_expression(operand)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(operand));
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(OP_PLUS) || peek().is_simple(OP_MINUS) ||
     peek().is_simple(OP_LNOT) || peek().is_simple(OP_COMPL) ||
     peek().is_simple(OP_STAR) || peek().is_simple(OP_AMP) ||
     peek().is_simple(OP_INC) || peek().is_simple(OP_DEC)) {
    RecogToken op_token = peek();
    string op = op_token.source;
    ++pos;
    CppAstNode operand;
    if(!parse_unary_expression(operand)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::unary_expression, op);
    set_node_token(out, op_token);
    out.children.push_back(std::move(operand));
    set_span(out, start);
    return true;
  }

  if(peek().is_identifier() && is_gnu_complex_unary_operator_name(peek().source)) {
    const string op = peek().source;
    ++pos;
    CppAstNode operand;
    if(!parse_unary_expression(operand)) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::unary_expression, op);
    out.children.push_back(std::move(operand));
    set_span(out, start);
    return true;
  }

  return parse_postfix_expression(out);
}

bool CppAstParser::parse_postfix_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!parse_primary_expression(out)) {
    pos = start;
    return false;
  }

  if(!parse_postfix_suffixes(out, start)) {
    pos = start;
    return false;
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_postfix_suffixes(CppAstNode & out, size_t start)
{
  for(;;) {
    if(consume_simple(OP_LPAREN)) {
      CppAstNode call = make_node(CppAstKind::call_expression);
      call.children.push_back(std::move(out));

      CppAstNode arguments = make_node(CppAstKind::argument_list);
      if(call.children[0].kind == CppAstKind::id_expression &&
         call.children[0].value == "__builtin_va_arg") {
        if(!parse_builtin_va_arg_argument_list(arguments, start)) {
          pos = start;
          return false;
        }
        call.children.push_back(std::move(arguments));
        out = std::move(call);
        continue;
      }
      if(!consume_simple(OP_RPAREN)) {
        CppAstNode arg;
        if(!parse_assignment_expression(arg)) {
          pos = start;
          return false;
        }
        if(consume_simple(OP_DOTS)) {
          CppAstNode expanded = make_node(CppAstKind::pack_expansion_expression);
          expanded.children.push_back(std::move(arg));
          arg = std::move(expanded);
        }
        arguments.children.push_back(std::move(arg));

        while(consume_simple(OP_COMMA)) {
          CppAstNode next;
          if(!parse_assignment_expression(next)) {
            pos = start;
            return false;
          }
          if(consume_simple(OP_DOTS)) {
            CppAstNode expanded = make_node(CppAstKind::pack_expansion_expression);
            expanded.children.push_back(std::move(next));
            next = std::move(expanded);
          }
          arguments.children.push_back(std::move(next));
        }

        if(!consume_simple(OP_RPAREN)) {
          pos = start;
          return false;
        }
      }

      call.children.push_back(std::move(arguments));
      out = std::move(call);
      continue;
    }

    if(consume_simple(OP_LSQUARE)) {
      CppAstNode index;
      if(!parse_expression(index) || !consume_simple(OP_RSQUARE)) {
        pos = start;
        return false;
      }
      CppAstNode subscript = make_node(CppAstKind::subscript_expression);
      subscript.children.push_back(std::move(out));
      subscript.children.push_back(std::move(index));
      out = std::move(subscript);
      continue;
    }

    if(peek().is_simple(OP_DOT) || peek().is_simple(OP_ARROW)) {
      RecogToken op_token = peek();
      string op = op_token.source;
      ++pos;
      const bool member_template_disambiguator = consume_simple(KW_TEMPLATE);
      string member_name;
      cpp_decl::QualifiedName member_name_syntax;
      cpp_decl::TemplateIdSyntax member_template_id_syntax;
      vector<cpp_decl::TemplateIdSyntax> member_qualifier_template_id_syntaxes;
      vector<CppAstNode> member_qualifier_type_syntaxes;
      if(!parse_qualified_name_text(member_name,
                                    &member_name_syntax,
                                    &member_template_id_syntax,
                                    &member_qualifier_template_id_syntaxes,
                                    &member_qualifier_type_syntaxes,
                                    false,
                                    true)) {
        pos = start;
        return false;
      }

      CppAstNode member = make_node(CppAstKind::member_expression, op);
      set_node_token(member, op_token);
      member.children.push_back(std::move(out));
      CppAstNode identifier =
          make_node(CppAstKind::identifier,
                    member_template_disambiguator ?
                        string("template ") + member_name :
                        member_name);
      set_cppast_qualified_name_syntax(identifier, std::move(member_name_syntax));
      if(!member_template_id_syntax.name.name.empty()) {
        set_cppast_template_id_syntax(identifier, std::move(member_template_id_syntax));
      }
      if(!member_qualifier_template_id_syntaxes.empty()) {
        set_cppast_qualifier_template_id_syntaxes(
            identifier,
            std::move(member_qualifier_template_id_syntaxes));
      }
      if(!member_qualifier_type_syntaxes.empty()) {
        set_cppast_qualifier_type_syntaxes(identifier,
                                           std::move(member_qualifier_type_syntaxes));
      }
      member.children.push_back(std::move(identifier));
      out = std::move(member);
      continue;
    }

    if(peek().is_simple(OP_INC) || peek().is_simple(OP_DEC)) {
      RecogToken op_token = peek();
      string op = op_token.source;
      ++pos;
      CppAstNode post = make_node(CppAstKind::postfix_expression, op);
      set_node_token(post, op_token);
      post.children.push_back(std::move(out));
      out = std::move(post);
      continue;
    }

    break;
  }

  return true;
}

bool CppAstParser::parse_builtin_va_arg_argument_list(CppAstNode & out,
                                                      size_t start)
{
  out = make_node(CppAstKind::argument_list);

  CppAstNode list_arg;
  CppAstNode result_type;
  if(!parse_assignment_expression(list_arg) ||
     !consume_simple(OP_COMMA) ||
     !parse_type_id(result_type) ||
     !consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  out.children.push_back(std::move(list_arg));
  out.children.push_back(std::move(result_type));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_primary_expression(CppAstNode & out)
{
  size_t start = pos;

  if(peek().is_simple(OP_LSQUARE)) {
    if(!parse_lambda_expression(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(OP_LBRACE)) {
    if(!parse_braced_init_list(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_literal() && consume_literal()) {
    out = make_node(CppAstKind::literal, tokens[pos - 1].source);
    set_span(out, start);
    return true;
  }

  {
    size_t type_start = pos;
    bool saw_typename = consume_simple(KW_TYPENAME);
    size_t type_name_start = pos;
    string type_name;
    bool simple_type = !saw_typename && parse_function_style_simple_type_text(type_name);
    bool saw_type_name = simple_type || parse_type_name_text(type_name);
    const size_t type_name_end = pos;
    bool saw_paren_init = peek().is_simple(OP_LPAREN);
    bool saw_brace_init = peek().is_simple(OP_LBRACE);
    if(saw_type_name && (saw_paren_init || saw_brace_init)) {
      bool ambiguous_id_expression = false;
      if(!saw_typename && !simple_type && saw_paren_init) {
        size_t type_end = pos;
        string id_name;
        pos = type_start;
        ambiguous_id_expression =
            parse_qualified_name_text(id_name) && pos == type_end;
        if(parser_trace::enabled("parser.fragment")) {
          std::ostringstream trace;
          trace << "functional-cast ambiguity type=" << type_name
                << " id_parse=" << id_name
                << " type_end=" << type_end
                << " id_end=" << pos
                << " result=" << (ambiguous_id_expression ? "id-expression"
                                                           : "type-name");
          parser_trace::note("parser.fragment", tokens, type_start, trace.str());
        }
        pos = type_end;
      }
      if(ambiguous_id_expression) {
        pos = type_start;
      } else {
      CppAstNode conversion_type_id;
      bool has_conversion_type_id = false;
      {
        const size_t saved_pos = pos;
        pos = type_start;
        CppAstNode conversion_specifiers;
        if(parse_type_specifier_seq(conversion_specifiers) &&
           pos == type_name_end) {
          conversion_type_id = make_node(CppAstKind::type_id);
          conversion_type_id.children.push_back(std::move(conversion_specifiers));
          conversion_type_id.token_start = type_start;
          conversion_type_id.token_end = type_name_end;
          has_conversion_type_id = true;
        }
        pos = saved_pos;
      }
      CppAstNode arguments = make_node(CppAstKind::argument_list);
      if(peek().is_simple(OP_LPAREN)) {
        if(!parse_paren_argument_list(arguments)) {
          pos = start;
          return false;
        }
      } else {
        CppAstNode braces;
        if(!parse_braced_init_list(braces)) {
          pos = start;
          return false;
        }
        arguments.children.push_back(std::move(braces));
      }
      out = make_node(CppAstKind::call_expression);
      CppAstNode callee =
          make_node(CppAstKind::id_expression,
                    (saw_typename ? string("typename ") : string()) + type_name);
      callee.has_leading_typename = saw_typename;
      if(saw_type_name && !simple_type) {
        attach_qualified_name_syntax_from_span(callee, type_name_start, type_name_end);
      }
      out.children.push_back(std::move(callee));
      out.children.push_back(std::move(arguments));
      if(has_conversion_type_id) {
        set_cppast_conversion_type_id_syntax(out, std::move(conversion_type_id));
      }
      set_span(out, start);
      return true;
      }
    }
    pos = type_start;
  }

  if(can_start_id_expression()) {
    if(!parse_id_expression(out)) {
      pos = start;
      return false;
    }
    set_span(out, start);
    return true;
  }

  if(peek().is_simple(KW_TRUE) || peek().is_simple(KW_FALSE) ||
     peek().is_simple(KW_NULLPTR) || peek().is_simple(KW_THIS)) {
    out = make_token_node(CppAstKind::keyword_literal, peek());
    ++pos;
    set_span(out, start);
    return true;
  }

  if(parse_fold_expression(out)) {
    set_span(out, start);
    return true;
  }

  if(consume_simple(OP_LPAREN)) {
    if(peek().is_simple(OP_LBRACE)) {
      CppAstNode body;
      if(!parse_compound_statement(body) || !consume_simple(OP_RPAREN)) {
        pos = start;
        return false;
      }
      out = make_node(CppAstKind::statement_expression);
      out.children.push_back(std::move(body));
      set_span(out, start);
      return true;
    }

    CppAstNode expression;
    if(!parse_expression(expression)) {
      if(parser_trace::enabled("parser.fragment")) {
        std::ostringstream trace;
        trace << "primary parenthesized expression inner parse failed near "
              << token_label(peek());
        parser_trace::note("parser.fragment", tokens, pos, trace.str());
      }
      pos = start;
      return false;
    }
    if(parser_trace::enabled("parser.fragment")) {
      std::ostringstream trace;
      trace << "primary parenthesized expression inner parse text="
            << token_span_text_spaced(expression.token_start, expression.token_end)
            << " next=" << token_label(peek());
      parser_trace::note("parser.fragment", tokens, pos, trace.str());
    }
    if(!consume_simple(OP_RPAREN)) {
      if(parser_trace::enabled("parser.fragment")) {
        std::ostringstream trace;
        trace << "primary parenthesized expression rejected near "
              << token_label(peek());
        parser_trace::note("parser.fragment", tokens, pos, trace.str());
      }
      pos = start;
      return false;
    }

    if(parser_trace::enabled("parser.fragment")) {
      std::ostringstream trace;
      trace << "primary parenthesized expression accepted text="
            << token_span_text_spaced(expression.token_start, expression.token_end);
      parser_trace::note("parser.fragment", tokens, start, trace.str());
    }
    out = make_node(CppAstKind::parenthesized_expression);
    out.children.push_back(std::move(expression));
    set_span(out, start);
    return true;
  }

  return false;
}

bool CppAstParser::parse_fold_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  size_t ellipsis_pos = static_cast<size_t>(-1);
  size_t close_pos = static_cast<size_t>(-1);
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t cursor = pos; !tokens.peek(cursor).is_eof(); ++cursor) {
    const RecogToken & token = tokens.peek(cursor);
    if(token.is_simple(OP_LT)) {
      ++angle_depth;
    } else if(token.is_close_angle_bracket() || token.is_simple(OP_GT)) {
      if(angle_depth > 0) {
        --angle_depth;
      }
    } else if(token.is_simple(OP_LPAREN)) {
      ++paren_depth;
    } else if(token.is_simple(OP_RPAREN)) {
      if(paren_depth == 0) {
        close_pos = cursor;
        break;
      }
      --paren_depth;
    } else if(token.is_simple(OP_LSQUARE)) {
      ++bracket_depth;
    } else if(token.is_simple(OP_RSQUARE)) {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
    } else if(token.is_simple(OP_LBRACE)) {
      ++brace_depth;
    } else if(token.is_simple(OP_RBRACE)) {
      if(brace_depth > 0) {
        --brace_depth;
      }
    }

    if(angle_depth == 0 &&
       paren_depth == 0 &&
       bracket_depth == 0 &&
       brace_depth == 0 &&
       token.is_simple(OP_DOTS)) {
      if(ellipsis_pos != static_cast<size_t>(-1)) {
        pos = start;
        return false;
      }
      ellipsis_pos = cursor;
    }
  }

  if(close_pos == static_cast<size_t>(-1) || ellipsis_pos == static_cast<size_t>(-1)) {
    pos = start;
    return false;
  }

  const auto make_ellipsis_node = [&](size_t index) -> CppAstNode
  {
    CppAstNode node = make_token_node(CppAstKind::ellipsis, tokens.peek(index));
    node.token_start = index;
    node.token_end = index + 1;
    return node;
  };

  const auto parse_operand = [&](size_t from, size_t to, CppAstNode & operand) -> bool
  {
    if(to <= from) {
      return false;
    }
    RecogTokenRangeSequence fragment(tokens, from, to);
    CppAstParser parser(fragment);
    parser.inherit_name_lookup_state_from(*this);
    if(!parser.parse_assignment_expression(operand) || !parser.at_eof()) {
      return false;
    }
    offset_node_token_spans(operand, from);
    return true;
  };

  out = make_node(CppAstKind::fold_expression);
  if(ellipsis_pos == pos) {
    if(ellipsis_pos + 1 >= close_pos || !is_fold_operator_token(tokens.peek(ellipsis_pos + 1))) {
      pos = start;
      return false;
    }
    const RecogToken & op_token = tokens.peek(ellipsis_pos + 1);
    CppAstNode operand;
    if(!parse_operand(ellipsis_pos + 2, close_pos, operand)) {
      pos = start;
      return false;
    }
    out.value = op_token.source;
    set_node_token(out, op_token);
    out.children.push_back(make_ellipsis_node(ellipsis_pos));
    out.children.push_back(std::move(operand));
  } else if(ellipsis_pos + 1 == close_pos) {
    if(ellipsis_pos <= pos || !is_fold_operator_token(tokens.peek(ellipsis_pos - 1))) {
      pos = start;
      return false;
    }
    const RecogToken & op_token = tokens.peek(ellipsis_pos - 1);
    CppAstNode operand;
    if(!parse_operand(pos, ellipsis_pos - 1, operand)) {
      pos = start;
      return false;
    }
    out.value = op_token.source;
    set_node_token(out, op_token);
    out.children.push_back(std::move(operand));
    out.children.push_back(make_ellipsis_node(ellipsis_pos));
  } else {
    if(ellipsis_pos <= pos ||
       ellipsis_pos + 1 >= close_pos ||
       !is_fold_operator_token(tokens.peek(ellipsis_pos - 1)) ||
       !is_fold_operator_token(tokens.peek(ellipsis_pos + 1)) ||
       tokens.peek(ellipsis_pos - 1).simple_type != tokens.peek(ellipsis_pos + 1).simple_type) {
      pos = start;
      return false;
    }
    const RecogToken & op_token = tokens.peek(ellipsis_pos - 1);
    CppAstNode lhs;
    CppAstNode rhs;
    if(!parse_operand(pos, ellipsis_pos - 1, lhs) ||
       !parse_operand(ellipsis_pos + 2, close_pos, rhs)) {
      pos = start;
      return false;
    }
    out.value = op_token.source;
    set_node_token(out, op_token);
    out.children.push_back(std::move(lhs));
    out.children.push_back(make_ellipsis_node(ellipsis_pos));
    out.children.push_back(std::move(rhs));
  }

  pos = close_pos + 1;
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_lambda_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!peek().is_simple(OP_LSQUARE)) {
    pos = start;
    return false;
  }

  if(!parse_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::lambda_expression);
  out.children.push_back(make_node(CppAstKind::lambda_introducer, token_span_text(start, pos)));

  CppAstNode declarator;
  NameSet lambda_template_parameter_names;
  NameSet lambda_template_value_parameter_names;
  NameSet lambda_parameter_value_names;
  if(parse_lambda_declarator(declarator)) {
    collect_outer_parameter_value_names(declarator, lambda_parameter_value_names);
    for(size_t i = 0; i < declarator.children.size(); ++i) {
      if(declarator.children[i].kind == CppAstKind::template_parameter_clause) {
        collect_template_parameter_names(declarator.children[i],
                                         lambda_template_parameter_names);
        collect_template_parameter_value_names(declarator.children[i],
                                               lambda_template_value_parameter_names);
      }
    }
    out.children.push_back(std::move(declarator));
  }
  else if(!peek().is_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  CppAstNode body;
  if(!lambda_template_parameter_names.empty()) {
    template_type_parameter_scopes.push_back(lambda_template_parameter_names);
  }
  if(!lambda_template_value_parameter_names.empty()) {
    template_value_parameter_scopes.push_back(lambda_template_value_parameter_names);
    value_name_scopes.push_back(lambda_template_value_parameter_names);
  }
  if(!lambda_parameter_value_names.empty()) {
    value_name_scopes.push_back(lambda_parameter_value_names);
  }
  const bool ok = parse_compound_statement(body);
  if(!lambda_parameter_value_names.empty()) {
    value_name_scopes.pop_back();
  }
  if(!lambda_template_value_parameter_names.empty()) {
    value_name_scopes.pop_back();
    template_value_parameter_scopes.pop_back();
  }
  if(!lambda_template_parameter_names.empty()) {
    template_type_parameter_scopes.pop_back();
  }
  if(!ok) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(body));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_lambda_declarator(CppAstNode & out)
{
  size_t start = pos;
  if(!(peek().is_simple(OP_LT) || peek().is_simple(OP_LPAREN))) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::lambda_declarator);

  NameSet lambda_template_parameter_names;
  NameSet lambda_template_value_parameter_names;
  bool saw_template_parameter_clause = false;
  if(peek().is_simple(OP_LT)) {
    CppAstNode template_parameters;
    if(!parse_template_parameter_clause(template_parameters)) {
      pos = start;
      return false;
    }
    collect_template_parameter_names(template_parameters,
                                     lambda_template_parameter_names);
    collect_template_parameter_value_names(template_parameters,
                                           lambda_template_value_parameter_names);
    out.children.push_back(std::move(template_parameters));
    template_type_parameter_scopes.push_back(lambda_template_parameter_names);
    template_value_parameter_scopes.push_back(lambda_template_value_parameter_names);
    value_name_scopes.push_back(lambda_template_value_parameter_names);
    saw_template_parameter_clause = true;
  }

  CppAstNode parameters;
  if(parse_parameter_clause(parameters)) {
    out.children.push_back(std::move(parameters));
  }
  else if(!saw_template_parameter_clause) {
    pos = start;
    return false;
  }

  if(consume_simple(KW_MUTABLE)) {
    out.children.push_back(make_token_node(CppAstKind::lambda_specifier, tokens[pos - 1]));
  }

  if(peek().is_simple(KW_NOEXCEPT)) {
    ++pos;
    CppAstNode noexcept_node = make_node(CppAstKind::noexcept_specification);
    if(consume_simple(OP_LPAREN)) {
      CppAstNode expression;
      if(!parse_expression(expression) || !consume_simple(OP_RPAREN)) {
        if(saw_template_parameter_clause) {
          value_name_scopes.pop_back();
          template_value_parameter_scopes.pop_back();
          template_type_parameter_scopes.pop_back();
        }
        pos = start;
        return false;
      }
      noexcept_node.children.push_back(std::move(expression));
    }
    out.children.push_back(std::move(noexcept_node));
  }

  if(consume_simple(OP_ARROW)) {
    CppAstNode type_id;
    if(!parse_type_id(type_id)) {
      if(saw_template_parameter_clause) {
        value_name_scopes.pop_back();
        template_value_parameter_scopes.pop_back();
        template_type_parameter_scopes.pop_back();
      }
      pos = start;
      return false;
    }
    CppAstNode trailing = make_node(CppAstKind::trailing_return_type);
    trailing.children.push_back(std::move(type_id));
    out.children.push_back(std::move(trailing));
  }

  if(saw_template_parameter_clause) {
    value_name_scopes.pop_back();
    template_value_parameter_scopes.pop_back();
    template_type_parameter_scopes.pop_back();
  }
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_keyword_cast_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!(peek().is_simple(KW_STATIC_CAST) || peek().is_simple(KW_DYNAMIC_CAST) ||
       peek().is_simple(KW_CONST_CAST) || peek().is_simple(KW_REINTERPET_CAST))) {
    pos = start;
    return false;
  }

  RecogToken cast_token = peek();
  string cast_kind = cast_token.source;
  ++pos;

  if(!consume_simple(OP_LT)) {
    pos = start;
    return false;
  }

  CppAstNode type_id;
  if(!parse_type_id(type_id) || !consume_close_angle_bracket() ||
     !consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  CppAstNode operand;
  if(!parse_expression(operand) || !consume_simple(OP_RPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::cast_expression, cast_kind);
  set_node_token(out, cast_token);
  out.children.push_back(std::move(type_id));
  out.children.push_back(std::move(operand));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_new_expression(CppAstNode & out)
{
  size_t start = pos;
  bool global_scope = consume_simple(OP_COLON2);
  if(!consume_simple(KW_NEW)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::new_expression);
  if(global_scope) {
    out.children.push_back(make_node(CppAstKind::global_scope));
  }

  if(peek().is_simple(OP_LPAREN)) {
    size_t branch_start = pos;
    if(parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      size_t placement_end = pos;
      CppAstNode placement_arguments;
      const size_t after_placement = pos;
      pos = branch_start;
      const bool parsed_placement_arguments =
          parse_paren_argument_list(placement_arguments);
      pos = after_placement;
      CppAstNode type_id;
      if(parsed_placement_arguments && parse_new_type_id(type_id)) {
        CppAstNode placement =
            make_node(CppAstKind::placement, token_span_text(branch_start, placement_end));
        placement.children.push_back(std::move(placement_arguments));
        out.children.push_back(std::move(placement));
        out.children.push_back(std::move(type_id));
      }
      else {
        pos = branch_start;
      }
    }

    if(out.children.empty() || out.children.back().kind == CppAstKind::global_scope) {
      pos = branch_start;
      if(peek().is_simple(OP_LPAREN)) {
        CppAstNode type_id;
        bool is_type_id = false;
        if(!parse_parenthesized_type_id_or_expression(type_id, is_type_id, false) ||
           !is_type_id) {
          pos = start;
          return false;
        }
        out.children.push_back(std::move(type_id));
      }
    }
  }

  if(out.children.empty() || out.children.back().kind == CppAstKind::placement ||
     out.children.back().kind == CppAstKind::global_scope) {
    CppAstNode type_id;
    if(!parse_new_type_id(type_id)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(type_id));
  }

  if(peek().is_simple(OP_LPAREN) || peek().is_simple(OP_LBRACE)) {
    CppAstNode initializer;
    if(!parse_initializer(initializer)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(initializer));
  }

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_delete_expression(CppAstNode & out)
{
  size_t start = pos;
  bool global_scope = consume_simple(OP_COLON2);
  if(!consume_simple(KW_DELETE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::delete_expression);
  if(global_scope) {
    out.children.push_back(make_node(CppAstKind::global_scope));
  }
  if(consume_simple(OP_LSQUARE)) {
    if(!consume_simple(OP_RSQUARE)) {
      pos = start;
      return false;
    }
    out.children.push_back(make_node(CppAstKind::array_delete));
  }

  CppAstNode operand;
  if(!parse_unary_expression(operand)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(operand));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_type_trait_expression(CppAstNode & out)
{
  size_t start = pos;
  if(!(peek().is_simple(KW_TYPEID) || peek().is_simple(KW_ALIGNOF) ||
       is_alignof_type_trait_identifier(peek()) ||
       peek().is_simple(KW_NOEXCEPT) ||
       is_builtin_type_trait_expression_start(tokens, pos))) {
    pos = start;
    return false;
  }

  RecogToken kind_token = peek();
  string kind = kind_token.source;
  ++pos;

  if(!consume_simple(OP_LPAREN)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::type_trait_expression, kind);
  set_node_token(out, kind_token);
  if(is_alignof_type_trait_identifier(kind_token)) {
    set_node_simple_type(out, KW_ALIGNOF);
  }

  if(kind_token.is_simple(KW_NOEXCEPT)) {
    CppAstNode inner_expr;
    if(!parse_expression(inner_expr) || !consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(inner_expr));
    set_span(out, start);
    return true;
  }

  if(kind_token.is_identifier()) {
    CppAstNode type_id;
    if(!parse_type_id(type_id)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(type_id));

    while(consume_simple(OP_COMMA)) {
      CppAstNode next_type_id;
      if(!parse_type_id(next_type_id)) {
        pos = start;
        return false;
      }
      out.children.push_back(std::move(next_type_id));
    }

    if(!consume_simple(OP_RPAREN)) {
      pos = start;
      return false;
    }

    set_span(out, start);
    return true;
  }

  --pos;
  CppAstNode inner;
  bool is_type_id = false;
  if(!parse_parenthesized_type_id_or_expression(
         inner, is_type_id, kind_token.is_simple(KW_TYPEID))) {
    pos = start;
    return false;
  }
  if(!is_type_id && !kind_token.is_simple(KW_TYPEID)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(inner));

  set_span(out, start);
  return true;
}

bool CppAstParser::parse_braced_init_list(CppAstNode & out)
{
  size_t start = pos;
  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::braced_init_list);

  if(consume_simple(OP_RBRACE)) {
    set_span(out, start);
    return true;
  }

  while(true) {
    CppAstNode element;
    if(!parse_designated_initializer(element) &&
       !parse_assignment_expression(element) &&
       !parse_braced_init_list(element)) {
      pos = start;
      return false;
    }
    if(consume_simple(OP_DOTS)) {
      CppAstNode expanded = make_node(CppAstKind::pack_expansion_expression);
      expanded.children.push_back(std::move(element));
      element = std::move(expanded);
    }
    out.children.push_back(std::move(element));

    if(consume_simple(OP_RBRACE)) {
      set_span(out, start);
      return true;
    }

    if(!consume_simple(OP_COMMA)) {
      pos = start;
      return false;
    }

    if(consume_simple(OP_RBRACE)) {
      set_span(out, start);
      return true;
    }
  }
}

bool CppAstParser::parse_designated_initializer(CppAstNode & out)
{
  size_t start = pos;
  if(!(peek().is_simple(OP_DOT) || peek().is_simple(OP_LSQUARE))) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::designated_initializer);
  do
  {
    CppAstNode designator;
    if(!parse_designator(designator)) {
      pos = start;
      return false;
    }
    out.children.push_back(std::move(designator));
  } while(peek().is_simple(OP_DOT) || peek().is_simple(OP_LSQUARE));

  if(!consume_simple(OP_ASS)) {
    pos = start;
    return false;
  }

  CppAstNode payload;
  if(!parse_braced_init_list(payload) && !parse_assignment_expression(payload)) {
    pos = start;
    return false;
  }
  out.children.push_back(std::move(payload));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_designator(CppAstNode & out)
{
  size_t start = pos;
  if(consume_simple(OP_DOT)) {
    if(!consume_identifier()) {
      pos = start;
      return false;
    }
    out = make_node(CppAstKind::designator);
    set_node_simple_type(out, OP_DOT);
    out.children.push_back(make_node(CppAstKind::identifier, tokens[pos - 1].source));
    set_span(out, start);
    return true;
  }

  if(!consume_simple(OP_LSQUARE)) {
    pos = start;
    return false;
  }

  CppAstNode index;
  if(!parse_assignment_expression(index) || !consume_simple(OP_RSQUARE)) {
    pos = start;
    return false;
  }

  out = make_node(CppAstKind::designator);
  set_node_simple_type(out, OP_LSQUARE);
  out.children.push_back(std::move(index));
  set_span(out, start);
  return true;
}

bool CppAstParser::parse_id_expression(CppAstNode & out)
{
  size_t start = pos;

  const auto current_id_has_known_value_template_suffix = [&]() -> bool
  {
    for(size_t cursor = start; !tokens.peek(cursor).is_eof(); ++cursor) {
      const RecogToken & token = tokens.peek(cursor);
      if(token.is_simple(OP_LT)) {
        return cursor > start &&
               tokens.peek(cursor - 1).is_identifier() &&
               is_known_value_template_parameter_identifier(
                   tokens.peek(cursor - 1));
      }
      if(token.is_simple(OP_COLON2) ||
         token.is_identifier() ||
         token.is_simple(KW_TEMPLATE)) {
        continue;
      }
      break;
    }
    return false;
  };

  const auto current_id_has_potential_template_suffix = [&]() -> bool
  {
    for(size_t cursor = start; !tokens.peek(cursor).is_eof(); ++cursor) {
      const RecogToken & token = tokens.peek(cursor);
      if(token.is_simple(OP_LT)) {
        return cursor > start && tokens.peek(cursor - 1).is_identifier();
      }
      if(token.is_simple(OP_COLON2) ||
         token.is_identifier() ||
         token.is_simple(KW_TEMPLATE)) {
        continue;
      }
      break;
    }
    return false;
  };

  const auto current_id_has_qualified_potential_template_suffix = [&]() -> bool
  {
    bool saw_qualifier = false;
    const template_angle_lookup::ScopedNameLookup lookup =
        make_template_angle_lookup(true);
    for(size_t cursor = start; !tokens.peek(cursor).is_eof();) {
      const RecogToken & token = tokens.peek(cursor);
      if(token.is_simple(OP_LT)) {
        if(cursor <= start || !tokens.peek(cursor - 1).is_identifier()) {
          return false;
        }
        if(saw_qualifier) {
          return true;
        }
        size_t suffix_end = cursor;
        vector<pair<size_t, size_t> > arg_ranges;
        if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                            cursor,
                                                            lookup,
                                                            suffix_end,
                                                            arg_ranges)) {
          return false;
        }
        cursor = suffix_end;
        continue;
      }
      if(token.is_simple(OP_COLON2)) {
        saw_qualifier = true;
        ++cursor;
        continue;
      }
      if(token.is_identifier() || token.is_simple(KW_TEMPLATE)) {
        ++cursor;
        continue;
      }
      break;
    }
    return false;
  };

  const auto parse_qualified_id = [&](bool prefer_unknown_template_ids,
                                      bool suppress_template_id_crossing_logical_operator,
                                      bool allow_value_template_id,
                                      bool * parsed_template_suffix) -> bool
  {
    string name;
    cpp_decl::QualifiedName name_syntax;
    cpp_decl::TemplateIdSyntax template_id_syntax;
    vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
    vector<CppAstNode> qualifier_type_syntaxes;
    if(!parse_qualified_name_text(name,
                                  &name_syntax,
                                  &template_id_syntax,
                                  &qualifier_template_id_syntaxes,
                                  &qualifier_type_syntaxes,
                                  prefer_unknown_template_ids,
                                  suppress_template_id_crossing_logical_operator,
                                  false,
                                  allow_value_template_id)) {
      return false;
    }
    if(parsed_template_suffix != nullptr) {
      *parsed_template_suffix =
          !template_id_syntax.name.name.empty() ||
          !qualifier_template_id_syntaxes.empty();
    }
    out = make_node(CppAstKind::id_expression, name);
    set_cppast_qualified_name_syntax(out, std::move(name_syntax));
    if(!template_id_syntax.name.name.empty()) {
      set_cppast_template_id_syntax(out, std::move(template_id_syntax));
    }
    if(!qualifier_template_id_syntaxes.empty()) {
      set_cppast_qualifier_template_id_syntaxes(out,
                                                std::move(qualifier_template_id_syntaxes));
    }
    if(!qualifier_type_syntaxes.empty()) {
      set_cppast_qualifier_type_syntaxes(out, std::move(qualifier_type_syntaxes));
    }
    set_span(out, start);
    return true;
  };

  if(current_id_has_known_value_template_suffix() &&
     parse_qualified_id(false, false, true, nullptr)) {
    return true;
  }

  pos = start;
  if(current_id_has_potential_template_suffix()) {
    bool parsed_template_suffix = false;
    if(parse_qualified_id(true,
                          true,
                          current_id_has_qualified_potential_template_suffix(),
                          &parsed_template_suffix) &&
       parsed_template_suffix) {
      return true;
    }
  }

  pos = start;
  if(parse_qualified_id(false, false, false, nullptr)) {
    return true;
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_unqualified_name_text(string & out,
                                               bool allow_template_id,
                                               bool allow_destructor,
                                               bool allow_operator)
{
  size_t start = pos;

  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  qualified_name_parser::UnqualifiedNameParseResult parsed;
  qualified_name_parser::UnqualifiedNameOptions options;
  options.allow_template_id = allow_template_id;
  options.allow_destructor = allow_destructor;
  options.allow_operator = allow_operator;
  if(!qualified_name_parser::parse_unqualified_name(tokens, start, lookup, options, parsed)) {
    pos = start;
    return false;
  }
  pos = parsed.end;
  out = parsed.kind == qualified_name_parser::UNQ_COMPONENT ?
      token_span_text_spaced(start, pos) :
      token_span_text(start, pos);
  return true;
}

bool CppAstParser::parse_name_component_text(string & out)
{
  size_t start = pos;
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  qualified_name_parser::NameComponentParseResult parsed;
  if(!qualified_name_parser::parse_name_component(tokens, start, lookup, parsed)) {
    pos = start;
    return false;
  }
  pos = parsed.end;
  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_template_argument_fragment_node(std::size_t start,
                                                         std::size_t end,
                                                         CppAstNode & out,
                                                         bool & is_type_id,
                                                         bool * pack_expansion)
{
  out = CppAstNode();
  is_type_id = false;
  if(pack_expansion) {
    *pack_expansion = false;
  }
  cpp_decl::TemplateArgumentSyntax syntax;
  if(!parse_template_argument_fragment_syntax(start, end, syntax)) {
    return false;
  }
  if(syntax.type_id) {
    out = std::move(*syntax.type_id);
    is_type_id = true;
  } else if(syntax.expression) {
    out = std::move(*syntax.expression);
  } else {
    return false;
  }
  if(pack_expansion) {
    *pack_expansion = syntax.pack_expansion;
  }
  return true;
}

bool CppAstParser::parse_template_argument_fragment_syntax(
    std::size_t start,
    std::size_t end,
    cpp_decl::TemplateArgumentSyntax & out,
    TemplateArgumentFragmentMode mode,
    bool suppress_nested_template_argument_syntax)
{
  out = cpp_decl::TemplateArgumentSyntax();
  if(end <= start) {
    return false;
  }

  const auto parsed_to_end = [](CppAstParser & parser, bool ok, bool & is_pack) -> bool
  {
    if(!ok) {
      return false;
    }
    if(parser.consume_simple(OP_DOTS)) {
      is_pack = true;
      return parser.at_eof();
    }
    return parser.at_eof();
  };

  RecogTokenRangeSequence fragment(tokens, start, end);
  out.text = template_angle::token_span_text_spaced(tokens, start, end);
  out.source_location_id = tokens[start].location_id;
  const bool has_trailing_pack_expansion =
      end > start && tokens.peek(end - 1).is_simple(OP_DOTS);

  const auto can_start_type_fragment = [&]() -> bool
  {
    const RecogToken & token = tokens.peek(start);
    if(token.is_eof() ||
       token.is_literal() ||
       token.is_simple(KW_TRUE) ||
       token.is_simple(KW_FALSE) ||
       token.is_simple(KW_NULLPTR) ||
       token.is_simple(KW_THIS)) {
      return false;
    }
    if(token.is_simple(OP_PLUS) ||
       token.is_simple(OP_MINUS) ||
       token.is_simple(OP_LNOT) ||
       token.is_simple(OP_STAR) ||
       token.is_simple(OP_AMP) ||
       token.is_simple(OP_INC) ||
       token.is_simple(OP_DEC)) {
      return false;
    }
    return true;
  };

  const bool allow_type =
      mode == TAF_PARSE_BOTH ||
      mode == TAF_PARSE_TYPE_ONLY ||
      mode == TAF_PARSE_TYPE_THEN_EXPRESSION;
  const bool allow_expression =
      mode == TAF_PARSE_BOTH ||
      mode == TAF_PARSE_EXPRESSION_ONLY ||
      mode == TAF_PARSE_TYPE_THEN_EXPRESSION;
  const template_angle_lookup::ScopedNameLookup scoped_lookup =
      make_template_angle_lookup();
  const TemplateArgumentFragmentNameLookup fragment_lookup(*this, scoped_lookup);
  const auto configure_child_parser = [&](CppAstParser & parser) -> void
  {
    parser.borrowed_template_parameter_lookup = this;
    parser.external_name_lookup = &fragment_lookup;
    parser.suppress_template_argument_fragment_syntax =
        suppress_nested_template_argument_syntax ||
        suppress_template_argument_fragment_syntax;
  };

  if(allow_type && can_start_type_fragment()) {
    CppAstNode type_argument;
    CppAstParser type_parser(fragment);
    configure_child_parser(type_parser);
    bool type_pack_expansion = false;
    if(parsed_to_end(type_parser,
                     type_parser.parse_type_id(type_argument),
                     type_pack_expansion)) {
      out.type_id.reset(new CppAstNode(std::move(type_argument)));
      out.pack_expansion =
          type_pack_expansion || has_trailing_pack_expansion;
      if(mode == TAF_PARSE_TYPE_ONLY || mode == TAF_PARSE_TYPE_THEN_EXPRESSION) {
        return true;
      }
    }
  }

  if(!allow_expression) {
    return static_cast<bool>(out.type_id);
  }

  CppAstParser expr_parser(fragment);
  configure_child_parser(expr_parser);
  CppAstNode expr_argument;
  bool expr_pack_expansion = false;
  if(parsed_to_end(expr_parser,
                   expr_parser.parse_assignment_expression(expr_argument),
                   expr_pack_expansion)) {
    if(!out.type_id) {
      CppAstNode function_type_id;
      if(build_empty_function_type_id_from_call_expression(expr_argument,
                                                           function_type_id)) {
        out.type_id.reset(new CppAstNode(std::move(function_type_id)));
      }
    }
    out.expression.reset(new CppAstNode(std::move(expr_argument)));
    out.pack_expansion =
        out.pack_expansion || expr_pack_expansion || has_trailing_pack_expansion;
  }
  return out.type_id || out.expression;
}

bool CppAstParser::parse_template_argument_text(string & out)
{
  size_t start = pos;
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  const auto parse_fragment = [&](size_t end) -> bool
  {
    CppAstNode argument;
    bool is_type_id = false;
    if(parse_template_argument_fragment_node(start, end, argument, is_type_id)) {
      pos = end;
      out = token_span_text_spaced(start, pos);
      return true;
    }
    return false;
  };

  std::vector<template_angle::Delimiter> delimiters;
  if(!template_angle::collect_template_argument_delimiters(tokens,
                                                           start,
                                                           lookup,
                                                           delimiters)) {
    pos = start;
    return false;
  }
  const bool trace_fragment = parser_trace::enabled("parser.fragment");
  for(std::size_t i = 0; i < delimiters.size(); ++i) {
    if(trace_fragment) {
      std::ostringstream trace;
      trace << "template-argument boundary kind="
            << (delimiters[i].kind == template_angle::DK_COMMA ? "comma" : "close")
            << " candidate={" << token_span_text_spaced(start, delimiters[i].pos) << "}";
      parser_trace::note("parser.fragment", tokens, start, trace.str());
    }
    if(parse_fragment(delimiters[i].pos)) {
      if(trace_fragment) {
        parser_trace::note("parser.fragment",
                           tokens,
                           start,
                           "template-argument accepted");
      }
      return true;
    }
  }

  pos = start;
  return false;
}

bool CppAstParser::parse_decltype_specifier_text(string & out)
{
  size_t start = pos;
  size_t end = start;
  if(!qualified_name_parser::parse_decltype_specifier(tokens, start, end)) {
    pos = start;
    return false;
  }

  pos = end;
  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_template_id_suffix_text(string & out)
{
  size_t start = pos;
  if(!peek().is_simple(OP_LT)) {
    pos = start;
    return false;
  }

  if(start > 0) {
    const RecogToken & prev = tokens.peek(start - 1);
    if(prev.is_identifier()) {
      const bool known_template = is_known_template_name_identifier(prev);
      const bool known_type = is_known_type_name_identifier(prev);
      const bool known_value_template =
          is_known_value_template_parameter_identifier(prev);
      const bool known_value = is_known_value_name_identifier(prev);
      if((known_value_template || known_value) &&
         !known_template && !known_type) {
        pos = start;
        return false;
      }
    }
  }

  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  std::size_t end = start;
  std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
  if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                      start,
                                                      lookup,
                                                      end,
                                                      arg_ranges)) {
    pos = start;
    return false;
  }

  const auto follower_allows_unknown_template_id =
      [](const RecogToken & follower) -> bool
  {
    if(follower.is_eof()) {
      return true;
    }
    if(follower.kind != RT_SIMPLE) {
      return false;
    }

    switch(follower.simple_type) {
    case OP_COLON2:
    case OP_LPAREN:
    case OP_RPAREN:
    case OP_RSQUARE:
    case OP_COMMA:
    case OP_SEMICOLON:
    case OP_COLON:
    case OP_DOT:
    case OP_ARROW:
    case OP_DOTS:
    case OP_RBRACE:
      return true;
    default:
      return false;
    }
  };

  if(start > 0) {
    const RecogToken & prev = tokens.peek(start - 1);
    if(prev.is_identifier()) {
      const bool known_type = lookup.is_known_type_name_identifier(prev);
      const bool known_template = lookup.is_known_template_name_identifier(prev);
      const bool known_value_template =
          lookup.is_known_value_template_parameter_identifier(prev);
      const bool known_value = lookup.is_known_value_name_identifier(prev);
      const bool explicit_template =
          start >= 2 &&
          (tokens.peek(start - 2).is_simple(KW_TEMPLATE) ||
           tokens.peek(start - 2).is_simple(OP_COLON2));

      if(!known_type && !known_template &&
         !known_value_template && !known_value &&
         !explicit_template &&
         !follower_allows_unknown_template_id(tokens.peek(end))) {
        pos = start;
        return false;
      }
    }
  }

  pos = end;
  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_function_style_simple_type_text(string & out)
{
  size_t start = pos;
  bool saw_type = false;

  for(;;) {
    const RecogToken & token = peek();
    if(is_cv_qualifier(token)) {
      ++pos;
      continue;
    }

    if(is_simple_type_specifier(token)) {
      ++pos;
      saw_type = true;
      continue;
    }

    if(is_decltype_token(token) ||
       (is_gnu_typeof_token(token) && peek(1).is_simple(OP_LPAREN))) {
      ++pos;
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      saw_type = true;
      continue;
    }

    break;
  }

  if(!saw_type) {
    pos = start;
    return false;
  }

  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_type_name_component_text(string & out)
{
  size_t start = pos;
  consume_simple(KW_TEMPLATE);

  const RecogToken & identifier = peek();
  if(is_decltype_token(identifier) ||
     (is_gnu_typeof_token(identifier) && peek(1).is_simple(OP_LPAREN))) {
    ++pos;
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }
    out = token_span_text_spaced(start, pos);
    return true;
  }

  if(!identifier.is_identifier()) {
    pos = start;
    return false;
  }

  ++pos;

  const bool qualified_component =
      start > 0 && tokens.peek(start - 1).is_simple(OP_COLON2);
  if(!qualified_component && peek().is_simple(OP_LT)) {
    const bool known_value =
        is_known_value_template_parameter_identifier(identifier) ||
        is_known_value_name_identifier(identifier);
    const bool known_type_or_template =
        is_known_template_name_identifier(identifier) ||
        is_known_type_name_identifier(identifier) ||
        is_template_type_parameter_name(identifier);
    if(known_value && !known_type_or_template) {
      out = token_span_text_spaced(start, pos);
      return true;
    }
  }

  string template_suffix;
  parse_angle_clause_text(template_suffix);

  if(peek().is_simple(OP_LPAREN) &&
     is_builtin_type_transform_identifier(identifier)) {
    if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
      pos = start;
      return false;
    }
    out = token_span_text_spaced(start, pos);
    return true;
  }

  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_type_name_text(string & out)
{
  size_t start = pos;
  consume_simple(OP_COLON2);

  string component;
  if(!parse_type_name_component_text(component)) {
    pos = start;
    return false;
  }

  while(consume_simple(OP_COLON2)) {
    if(!parse_type_name_component_text(component)) {
      pos = start;
      return false;
    }
  }

  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::parse_qualified_name_text(string & out,
                                             cpp_decl::QualifiedName * syntax,
                                             cpp_decl::TemplateIdSyntax * template_id_syntax,
                                             vector<cpp_decl::TemplateIdSyntax> *
                                                 qualifier_template_id_syntaxes,
                                             vector<CppAstNode> *
                                                 qualifier_type_syntaxes,
                                             bool prefer_unknown_template_ids,
                                             bool suppress_unforced_template_id_crossing_logical_operator,
                                             bool allow_conversion_operator_type_without_call,
                                             bool allow_value_template_id_final_component)
{
  size_t start = pos;
  template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
  lookup.prefer_unknown_template_ids =
      lookup.prefer_unknown_template_ids || prefer_unknown_template_ids;

  qualified_name_parser::QualifiedNameParseResult parsed;
  qualified_name_parser::UnqualifiedNameOptions options;
  options.suppress_unforced_template_id_crossing_logical_operator =
      suppress_unforced_template_id_crossing_logical_operator;
  options.allow_conversion_operator_type_without_call =
      allow_conversion_operator_type_without_call;
  options.allow_value_template_id_final_component =
      allow_value_template_id_final_component;
  if(!qualified_name_parser::parse_qualified_name(tokens,
                                                  start,
                                                  lookup,
                                                  options,
                                                  parsed)) {
    pos = start;
    return false;
  }

  pos = parsed.end;
  if(syntax != nullptr) {
    *syntax = build_qualified_name_syntax(tokens, parsed);
  }
  CppAstParser * template_argument_parser_context =
      suppress_template_argument_fragment_syntax ? nullptr : this;
  if(template_id_syntax != nullptr) {
    if(!build_template_id_syntax(tokens,
                                 lookup,
                                 parsed,
                                 *template_id_syntax,
                                 template_argument_parser_context)) {
      *template_id_syntax = cpp_decl::TemplateIdSyntax();
    }
  }
  if(qualifier_template_id_syntaxes != nullptr) {
    build_qualifier_template_id_syntaxes(tokens,
                                         lookup,
                                         parsed,
                                         *qualifier_template_id_syntaxes,
                                         template_argument_parser_context);
    if(template_id_syntax != nullptr &&
       !qualifier_template_id_syntaxes->empty() &&
       !template_id_syntax->name.name.empty()) {
      template_id_syntax->qualifier_template_id_syntaxes =
          *qualifier_template_id_syntaxes;
    }
  }
  if(qualifier_type_syntaxes != nullptr) {
    qualifier_type_syntaxes->clear();
    qualifier_type_syntaxes->resize(parsed.qualifiers.size());
    bool any_qualifier_type_syntax = false;
    for(size_t i = 0; i < parsed.qualifiers.size(); ++i) {
      const size_t qualifier_start = parsed.qualifiers[i].first;
      const size_t qualifier_end = parsed.qualifiers[i].second;
      size_t decltype_end = qualifier_start;
      if(!qualified_name_parser::parse_decltype_specifier(tokens,
                                                          qualifier_start,
                                                          decltype_end) ||
         decltype_end != qualifier_end) {
        continue;
      }
      CppAstNode qualifier =
          make_node(CppAstKind::decltype_specifier,
                    token_span_text_spaced(qualifier_start, qualifier_end));
      qualifier.is_typeof_specifier =
          is_gnu_typeof_token(tokens.peek(qualifier_start));
      qualifier.token_start = qualifier_start;
      qualifier.token_end = qualifier_end;
      qualifier.source_location_id = tokens[qualifier_start].location_id;
      CppAstNode operand;
      if(parse_decltype_or_typeof_operand_node(
             qualifier_start,
             qualifier_end,
             is_gnu_typeof_token(tokens.peek(qualifier_start)),
             operand)) {
        qualifier.children.push_back(std::move(operand));
      }
      (*qualifier_type_syntaxes)[i] = std::move(qualifier);
      any_qualifier_type_syntax = true;
    }
    if(!any_qualifier_type_syntax) {
      qualifier_type_syntaxes->clear();
    }
  }
  if(parsed.rooted || !parsed.qualifiers.empty()) {
    out = token_span_text_spaced(start, pos);
  } else if(parsed.name_kind == qualified_name_parser::UNQ_COMPONENT) {
    out = token_span_text_spaced(start, pos);
  } else {
    out = token_span_text(start, pos);
  }
  return true;
}

void CppAstParser::attach_builtin_type_transform_syntax_from_span(
    CppAstNode & node,
    size_t start,
    size_t end)
{
  const RecogToken & identifier = tokens.peek(start);
  if(!is_builtin_type_transform_identifier(identifier) ||
     !tokens.peek(start + 1).is_simple(OP_LPAREN) ||
     end <= start + 3) {
    return;
  }

  const size_t saved = pos;
  pos = start + 2;
  CppAstNode operand;
  const bool parsed =
      parse_type_id(operand) &&
      consume_simple(OP_RPAREN) &&
      pos == end;
  pos = saved;

  if(!parsed) {
    return;
  }

  node.builtin_type_transform_name = identifier.source;
  node.base_type_syntax.reset(new CppAstNode(std::move(operand)));
}

void CppAstParser::attach_qualified_name_syntax_from_span(CppAstNode & node,
                                                          size_t start,
                                                          size_t end)
{
  const size_t saved = pos;
  pos = start;

  string parsed_name;
  cpp_decl::QualifiedName name_syntax;
  cpp_decl::TemplateIdSyntax template_id_syntax;
  vector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  vector<CppAstNode> qualifier_type_syntaxes;
  const bool parsed =
      parse_qualified_name_text(parsed_name,
                                &name_syntax,
                                &template_id_syntax,
                                &qualifier_template_id_syntaxes,
                                &qualifier_type_syntaxes,
                                true) &&
      pos == end;
  pos = saved;

  if(!parsed) {
    for(size_t open = start; open < end; ++open) {
      if(!tokens.peek(open).is_simple(OP_LT)) {
        continue;
      }

      template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();
      lookup.prefer_unknown_template_ids = true;
      size_t suffix_end = open;
      vector<pair<size_t, size_t> > arg_ranges;
      if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                          open,
                                                          lookup,
                                                          suffix_end,
                                                          arg_ranges) ||
         suffix_end != end) {
        continue;
      }

      RecogTokenRangeSequence head_tokens(tokens, start, open);
      qualified_name_parser::QualifiedNameParseResult head_parsed;
      qualified_name_parser::UnqualifiedNameOptions head_options;
      const bool parsed_head =
          qualified_name_parser::parse_qualified_name(head_tokens,
                                                      0,
                                                      lookup,
                                                      head_options,
                                                      head_parsed) &&
          head_parsed.end == open - start;
      if(!parsed_head) {
        continue;
      }
      cpp_decl::QualifiedName head_syntax =
          build_qualified_name_syntax(head_tokens, head_parsed);

      cpp_decl::TemplateIdSyntax template_id;
      template_id.name = head_syntax;
      template_id.source_location_id = tokens[start].location_id;
      vector<cpp_decl::TemplateIdSyntax> fallback_qualifier_template_ids;
      qualified_name_parser::QualifiedNameParseResult original_head_parsed;
      if(qualified_name_parser::parse_qualified_name(
             tokens,
             start,
             lookup,
             qualified_name_parser::UnqualifiedNameOptions(),
             original_head_parsed) &&
         original_head_parsed.end == open &&
         qualified_name_has_qualifier_template_id(original_head_parsed)) {
        build_qualifier_template_id_syntaxes(tokens,
                                             lookup,
                                             original_head_parsed,
                                             fallback_qualifier_template_ids,
                                             this);
        template_id.qualifier_template_id_syntaxes =
            fallback_qualifier_template_ids;
      }
      for(size_t i = 0; i < arg_ranges.size(); ++i) {
        cpp_decl::TemplateArgumentSyntax argument;
        build_template_argument_syntax_from_range(tokens,
                                                  lookup,
                                                  arg_ranges[i],
                                                  this,
                                                  argument);
        template_id.arguments.push_back(argument.text);
        template_id.argument_syntaxes.push_back(std::move(argument));
      }

      set_cppast_qualified_name_syntax(node, std::move(head_syntax));
      set_cppast_template_id_syntax(node, std::move(template_id));
      if(!fallback_qualifier_template_ids.empty()) {
        set_cppast_qualifier_template_id_syntaxes(
            node,
            std::move(fallback_qualifier_template_ids));
      }
      return;
    }
    return;
  }

  set_cppast_qualified_name_syntax(node, std::move(name_syntax));
  if(!template_id_syntax.name.name.empty()) {
    set_cppast_template_id_syntax(node, std::move(template_id_syntax));
  }
  if(!qualifier_template_id_syntaxes.empty()) {
    set_cppast_qualifier_template_id_syntaxes(node,
                                              std::move(qualifier_template_id_syntaxes));
  }
  if(!qualifier_type_syntaxes.empty()) {
    set_cppast_qualifier_type_syntaxes(node, std::move(qualifier_type_syntaxes));
  }
}

bool CppAstParser::parse_angle_clause_text(string & out)
{
  size_t start = pos;
  if(!peek().is_simple(OP_LT)) {
    pos = start;
    return false;
  }

  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup(true);

  std::size_t end = start;
  std::vector<std::pair<std::size_t, std::size_t> > arg_ranges;
  if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                      start,
                                                      lookup,
                                                      end,
                                                      arg_ranges)) {
    pos = start;
    return false;
  }
  pos = end;
  out = token_span_text_spaced(start, pos);
  return true;
}

bool CppAstParser::can_open_nested_template_angle_at(size_t boundary) const
{
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  return template_angle::can_open_nested_template_angle_at(tokens, boundary, lookup);
}

bool CppAstParser::looks_like_unknown_nested_template_id_at(size_t boundary) const
{
  const template_angle_lookup::ScopedNameLookup lookup = make_template_angle_lookup();

  return template_angle::looks_like_unknown_nested_template_id_at(tokens, boundary, lookup);
}

bool CppAstParser::is_template_type_parameter_name(const RecogToken & token) const
{
  return template_angle_lookup::lookup_in_scoped_names(
      &template_type_parameter_scopes,
      inherited_template_type_parameter_scopes,
      token) ||
         (borrowed_template_parameter_lookup &&
          borrowed_template_parameter_lookup->is_template_type_parameter_name(token));
}

bool CppAstParser::is_known_template_name_identifier(const RecogToken & token) const
{
  return template_angle_lookup::lookup_in_scoped_names(
      &template_name_scopes,
      inherited_template_name_scopes,
      token) ||
         (external_name_lookup &&
          external_name_lookup->is_known_template_name_identifier(token));
}

bool CppAstParser::is_known_type_name_identifier(const RecogToken & token) const
{
  return template_angle_lookup::lookup_in_scoped_names(
      &type_name_scopes,
      inherited_type_name_scopes,
      token) ||
         (external_name_lookup &&
          external_name_lookup->is_known_type_name_identifier(token));
}

template_angle_lookup::ScopedNameLookup CppAstParser::make_template_angle_lookup(
    bool prefer_unknown_template_ids) const
{
  template_angle_lookup::ScopedNameLookup out;
  out.template_name_scopes = &template_name_scopes;
  out.template_type_parameter_scopes = &template_type_parameter_scopes;
  out.type_name_scopes = &type_name_scopes;
  out.template_value_name_scopes = &template_value_parameter_scopes;
  out.value_name_scopes = &value_name_scopes;
  out.inherited_template_name_scopes = inherited_template_name_scopes;
  out.inherited_template_type_parameter_scopes =
      inherited_template_type_parameter_scopes;
  out.inherited_type_name_scopes = inherited_type_name_scopes;
  out.inherited_template_value_name_scopes =
      inherited_template_value_parameter_scopes;
  out.inherited_value_name_scopes = inherited_value_name_scopes;
  out.fallback_lookup = external_name_lookup;
  out.prefer_unknown_template_ids = prefer_unknown_template_ids;
  return out;
}

bool CppAstParser::decl_specifier_seq_has_typedef(const CppAstNode & node) const
{
  if(node.kind != CppAstKind::decl_specifier_seq) {
    return false;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].has_token &&
       node.children[i].token_kind == RT_SIMPLE &&
       node.children[i].simple_type == KW_TYPEDEF) {
      return true;
    }
  }

  return false;
}

bool CppAstParser::is_known_value_template_parameter_identifier(
    const RecogToken & token) const
{
  return template_angle_lookup::lookup_in_scoped_names(
      &template_value_parameter_scopes,
      inherited_template_value_parameter_scopes,
      token) ||
         (external_name_lookup &&
          external_name_lookup->is_known_value_template_parameter_identifier(token));
}

bool CppAstParser::is_known_value_name_identifier(const RecogToken & token) const
{
  return template_angle_lookup::lookup_in_scoped_names(
      &value_name_scopes,
      inherited_value_name_scopes,
      token) ||
         (external_name_lookup &&
          external_name_lookup->is_known_value_name_identifier(token));
}

bool CppAstParser::unqualified_identifier_prefers_value_name(
    const RecogToken & token) const
{
  if(!token.is_identifier()) {
    return false;
  }

  text_intern::Atom atom = token.cached_identifier_atom();
  const int value_index =
      std::max(nearest_name_scope_index(&value_name_scopes,
                                        inherited_value_name_scopes,
                                        atom),
               nearest_name_scope_index(&template_value_parameter_scopes,
                                        inherited_template_value_parameter_scopes,
                                        atom));
  const int type_index =
      std::max(nearest_name_scope_index(&type_name_scopes,
                                        inherited_type_name_scopes,
                                        atom),
               nearest_name_scope_index(&template_type_parameter_scopes,
                                        inherited_template_type_parameter_scopes,
                                        atom));

  if(value_index >= 0 || type_index >= 0) {
    return value_index >= type_index;
  }

  const bool fallback_value =
      external_name_lookup &&
      (external_name_lookup->is_known_value_name_identifier(token) ||
       external_name_lookup->is_known_value_template_parameter_identifier(token));
  const bool fallback_type =
      external_name_lookup &&
      external_name_lookup->is_known_type_name_identifier(token);
  return fallback_value && !fallback_type;
}

void CppAstParser::collect_signature_type_hint_names(const CppAstNode & node,
                                                     NameSet & out) const
{
  if(node.kind == CppAstKind::decl_specifier) {
    if(node.has_token) {
      if(node.token_kind == RT_IDENTIFIER) {
        out.insert(primary_name_text(node.value));
      }
    } else if(node.children.empty() && !node.value.empty()) {
      out.insert(primary_name_text(node.value));
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_signature_type_hint_names(node.children[i], out);
  }
}

void CppAstParser::collect_declarator_identifiers(const CppAstNode & node,
                                                  NameSet & out) const
{
  if(node.kind == CppAstKind::identifier) {
    out.insert(unqualified_name_text(node.value));
    return;
  }

  if(node.kind == CppAstKind::parameter_clause) {
    return;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const size_t size_before = out.size();
    collect_declarator_identifiers(node.children[i], out);
    if(out.size() != size_before) {
      return;
    }
  }
}

bool CppAstParser::collect_outer_parameter_value_names(const CppAstNode & node,
                                                       NameSet & out) const
{
  if(node.kind == CppAstKind::parameter_clause) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind != CppAstKind::parameter_declaration) {
        continue;
      }
      for(size_t j = 0; j < node.children[i].children.size(); ++j) {
        if(node.children[i].children[j].kind == CppAstKind::declarator) {
          collect_declarator_identifiers(node.children[i].children[j], out);
          break;
        }
      }
    }
    return true;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(collect_outer_parameter_value_names(node.children[i], out)) {
      return true;
    }
  }

  return false;
}

void CppAstParser::collect_declared_type_names(const CppAstNode & node,
                                               NameSet & out) const
{
  if((node.kind == CppAstKind::class_specifier ||
      node.kind == CppAstKind::class_forward_declaration ||
      node.kind == CppAstKind::enum_specifier ||
      node.kind == CppAstKind::alias_declaration) &&
     !node.value.empty()) {
    out.insert(unqualified_name_text(node.value));
    return;
  }

  if(node.kind == CppAstKind::template_declaration) {
    if(node.children.size() > 1) {
      collect_declared_type_names(node.children[1], out);
    }
    return;
  }

  if(node.kind == CppAstKind::simple_declaration ||
     node.kind == CppAstKind::bit_field_declaration) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::decl_specifier_seq) {
        for(size_t j = 0; j < node.children[i].children.size(); ++j) {
          collect_declared_type_names(node.children[i].children[j], out);
        }
        if(decl_specifier_seq_has_typedef(node.children[i])) {
          for(size_t j = 0; j < node.children.size(); ++j) {
            if(node.children[j].kind == CppAstKind::init_declarator_list) {
              for(size_t k = 0; k < node.children[j].children.size(); ++k) {
                collect_declarator_identifiers(node.children[j].children[k], out);
              }
            }
          }
        }
        break;
      }
    }
    return;
  }
}

void CppAstParser::collect_declared_value_names(const CppAstNode & node,
                                                NameSet & out) const
{
  if(node.kind == CppAstKind::template_declaration) {
    return;
  }

  if(node.kind == CppAstKind::simple_declaration ||
     node.kind == CppAstKind::bit_field_declaration) {
    bool has_typedef = false;
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::decl_specifier_seq) {
        has_typedef = decl_specifier_seq_has_typedef(node.children[i]);
        break;
      }
    }
    if(has_typedef) {
      return;
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::init_declarator_list) {
        for(size_t j = 0; j < node.children[i].children.size(); ++j) {
          collect_declarator_identifiers(node.children[i].children[j], out);
        }
      }
    }
    return;
  }

  if(node.kind == CppAstKind::condition_declaration ||
     node.kind == CppAstKind::parameter_declaration ||
     node.kind == CppAstKind::exception_declaration) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::declarator) {
        collect_declarator_identifiers(node.children[i], out);
        return;
      }
    }
    return;
  }
}

void CppAstParser::collect_identifier_names_in_token_range(size_t start,
                                                           size_t end,
                                                           NameSet & out) const
{
  for(size_t i = start; i < end; ++i) {
    const RecogToken & token = tokens.peek(i);
    if(token.is_identifier()) {
      out.insert(token.cached_identifier_atom());
    }
  }
}

void CppAstParser::refresh_lazy_function_body_snapshots_for_class(
    CppAstNode & node,
    const ClassMemberNameScopes & scopes,
    bool root_class) const
{
  if(node.kind == CppAstKind::class_specifier && !root_class) {
    return;
  }

  if(node.kind == CppAstKind::lazy_function_body) {
    NameSet used_names;
    collect_identifier_names_in_token_range(node.token_start,
                                            node.token_end,
                                            used_names);

    const auto filter_used_names =
        [&used_names](const NameSet & source) -> NameSet
    {
      NameSet filtered;
      if(source.empty() || used_names.empty()) {
        return filtered;
      }
      if(used_names.size() < source.size()) {
        for(NameSet::const_iterator it = used_names.begin();
            it != used_names.end();
            ++it) {
          if(source.count(*it) != 0) {
            filtered.insert(*it);
          }
        }
      } else {
        for(NameSet::const_iterator it = source.begin();
            it != source.end();
            ++it) {
          if(used_names.count(*it) != 0) {
            filtered.insert(*it);
          }
        }
      }
      return filtered;
    };

    NameSet template_names = filter_used_names(scopes.template_names);
    NameSet type_names = filter_used_names(scopes.type_names);
    NameSet value_names = filter_used_names(scopes.value_names);
    if(!template_names.empty() || !type_names.empty() || !value_names.empty()) {
      if(parser_trace::enabled("parser.decl")) {
        std::ostringstream trace;
        trace << "lazy body class snapshot refresh"
              << " template-names=" << format_name_set_for_trace(template_names)
              << " type-names=" << format_name_set_for_trace(type_names)
              << " value-names=" << format_name_set_for_trace(value_names);
        parser_trace::note("parser.decl", tokens, node.token_start, trace.str());
      }
      shared_ptr<CppAstNameLookupSnapshot> refreshed(
          new CppAstNameLookupSnapshot(
              node.name_lookup_snapshot ? *node.name_lookup_snapshot :
                                           CppAstNameLookupSnapshot()));
      refreshed->template_name_scopes.push_back(template_names);
      refreshed->type_name_scopes.push_back(type_names);
      refreshed->value_name_scopes.push_back(value_names);
      node.name_lookup_snapshot = refreshed;
    }
    return;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    refresh_lazy_function_body_snapshots_for_class(node.children[i],
                                                   scopes,
                                                   false);
  }
}

void CppAstParser::collect_declared_template_names(const CppAstNode & node,
                                                   NameSet & out) const
{
  if(node.kind != CppAstKind::template_declaration || node.children.size() < 2) {
    return;
  }

  const CppAstNode & declaration = node.children[1];
  if((declaration.kind == CppAstKind::class_specifier ||
      declaration.kind == CppAstKind::class_forward_declaration ||
      declaration.kind == CppAstKind::alias_declaration) &&
     !declaration.value.empty()) {
    out.insert(unqualified_name_text(declaration.value));
    return;
  }

  for(size_t i = 0; i < declaration.children.size(); ++i) {
    if(declaration.children[i].kind == CppAstKind::init_declarator_list) {
      for(size_t j = 0; j < declaration.children[i].children.size(); ++j) {
        collect_declarator_identifiers(declaration.children[i].children[j], out);
      }
      if(!out.empty()) {
        return;
      }
    }
  }

  for(size_t i = 0; i < declaration.children.size(); ++i) {
    if(declaration.children[i].kind == CppAstKind::declarator ||
       declaration.children[i].kind == CppAstKind::init_declarator ||
       declaration.children[i].kind == CppAstKind::structured_binding_declarator) {
      collect_declarator_identifiers(declaration.children[i], out);
      if(!out.empty()) {
        return;
      }
    }
  }

  collect_declarator_identifiers(declaration, out);
}

void CppAstParser::collect_declared_template_value_names(const CppAstNode & node,
                                                        NameSet & out) const
{
  if(node.kind != CppAstKind::template_declaration || node.children.size() < 2) {
    return;
  }

  const CppAstNode & declaration = node.children[1];
  if(declaration.kind == CppAstKind::class_specifier ||
     declaration.kind == CppAstKind::class_forward_declaration ||
     declaration.kind == CppAstKind::alias_declaration) {
    return;
  }

  if(declaration.kind == CppAstKind::simple_declaration ||
     declaration.kind == CppAstKind::bit_field_declaration ||
     declaration.kind == CppAstKind::function_definition) {
    collect_declarator_identifiers(declaration, out);
  }
}

void CppAstParser::note_declared_type_names(const CppAstNode & node)
{
  if(type_name_scopes.empty()) {
    return;
  }

  NameSet names;
  collect_declared_type_names(node, names);
  type_name_scopes.back().insert(names.begin(), names.end());
  if(!names.empty()) {
    note_name_lookup_mutation();
  }
}

void CppAstParser::note_declared_template_names(const CppAstNode & node)
{
  if(template_name_scopes.empty()) {
    return;
  }

  NameSet names;
  collect_declared_template_names(node, names);
  template_name_scopes.back().insert(names.begin(), names.end());
  if(!names.empty()) {
    note_name_lookup_mutation();
  }
}

void CppAstParser::note_declared_template_value_names(const CppAstNode & node)
{
  if(template_value_parameter_scopes.empty()) {
    template_value_parameter_scopes.push_back(NameSet());
  }

  NameSet names;
  collect_declared_template_value_names(node, names);
  template_value_parameter_scopes.back().insert(names.begin(), names.end());
  if(!names.empty()) {
    note_name_lookup_mutation();
  }
}

void CppAstParser::note_declared_value_names(const CppAstNode & node)
{
  if(value_name_scopes.empty()) {
    return;
  }

  NameSet names;
  collect_declared_value_names(node, names);
  value_name_scopes.back().insert(names.begin(), names.end());
  if(!names.empty()) {
    note_name_lookup_mutation();
  }
}

void CppAstParser::note_namespace_alias_definition(const CppAstNode & node)
{
  if(node.kind != CppAstKind::namespace_alias_definition || node.value.empty() ||
     node.children.empty()) {
    return;
  }
  const CppAstNode & target = node.children[0];
  if(target.kind != CppAstKind::target || target.value.empty()) {
    return;
  }

  const string alias_key = current_namespace_path_key(node.value);
  const string target_key = resolve_visible_namespace_scope_key(target.value);
  if(alias_key.empty() || target_key.empty()) {
    return;
  }

  namespace_alias_targets[alias_key] = target_key;
  note_name_lookup_mutation();
}

void CppAstParser::note_using_imports(const CppAstNode & node)
{
  if(template_value_parameter_scopes.empty() ||
     template_name_scopes.empty() ||
     type_name_scopes.empty() ||
     value_name_scopes.empty()) {
    return;
  }

  bool changed = false;
  if(node.kind == CppAstKind::using_directive) {
    if(node.children.empty() || node.children[0].kind != CppAstKind::target) {
      return;
    }
    const string target_key = resolve_visible_namespace_scope_key(node.children[0].value);
    if(target_key.empty()) {
      return;
    }

    const auto template_found = namespace_template_name_scopes.find(target_key);
    if(template_found != namespace_template_name_scopes.end()) {
      template_name_scopes.back().insert(template_found->second.begin(),
                                         template_found->second.end());
      changed = true;
    }
    const auto template_value_found =
        namespace_template_value_name_scopes.find(target_key);
    if(template_value_found != namespace_template_value_name_scopes.end()) {
      template_value_parameter_scopes.back().insert(template_value_found->second.begin(),
                                                    template_value_found->second.end());
      changed = true;
    }
    const auto type_found = namespace_type_name_scopes.find(target_key);
    if(type_found != namespace_type_name_scopes.end()) {
      type_name_scopes.back().insert(type_found->second.begin(),
                                     type_found->second.end());
      changed = true;
    }
    const auto value_found = namespace_value_name_scopes.find(target_key);
    if(value_found != namespace_value_name_scopes.end()) {
      value_name_scopes.back().insert(value_found->second.begin(),
                                      value_found->second.end());
      changed = true;
    }
  } else if(node.kind == CppAstKind::using_declaration) {
    if(node.children.empty() || node.children[0].kind != CppAstKind::target) {
      return;
    }
    const string normalized_target = normalized_lookup_name(node.children[0].value);
    const size_t split = normalized_target.rfind("::");
    if(split == string::npos) {
      return;
    }
    const string namespace_key =
        resolve_visible_namespace_scope_key(normalized_target.substr(0, split));
    const string target_name = normalized_target.substr(split + 2);
    if(namespace_key.empty() || target_name.empty()) {
      return;
    }

    const auto template_found = namespace_template_name_scopes.find(namespace_key);
    if(template_found != namespace_template_name_scopes.end() &&
       template_found->second.count(target_name) != 0) {
      template_name_scopes.back().insert(target_name);
      changed = true;
    }
    const auto template_value_found =
        namespace_template_value_name_scopes.find(namespace_key);
    if(template_value_found != namespace_template_value_name_scopes.end() &&
       template_value_found->second.count(target_name) != 0) {
      template_value_parameter_scopes.back().insert(target_name);
      changed = true;
    }
    const auto type_found = namespace_type_name_scopes.find(namespace_key);
    if(type_found != namespace_type_name_scopes.end() &&
       type_found->second.count(target_name) != 0) {
      type_name_scopes.back().insert(target_name);
      changed = true;
    }
    const auto value_found = namespace_value_name_scopes.find(namespace_key);
    if(value_found != namespace_value_name_scopes.end() &&
       value_found->second.count(target_name) != 0) {
      value_name_scopes.back().insert(target_name);
      changed = true;
    }
  }

  if(changed) {
    note_name_lookup_mutation();
  }
}

void CppAstParser::note_visible_names_after_declaration(const CppAstNode & node)
{
  note_declared_template_names(node);
  note_declared_template_value_names(node);
  note_declared_type_names(node);
  note_declared_value_names(node);
  note_namespace_alias_definition(node);
  note_using_imports(node);
}

void CppAstParser::collect_template_parameter_names(const CppAstNode & node,
                                                    NameSet & out) const
{
  if(node.kind == CppAstKind::type_parameter) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::identifier) {
        out.insert(node.children[i].value);
      }
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::template_parameter_clause) {
      continue;
    }
    collect_template_parameter_names(node.children[i], out);
  }
}

void CppAstParser::collect_template_parameter_value_names(
    const CppAstNode & node,
    NameSet & out) const
{
  if(node.kind == CppAstKind::non_type_template_parameter) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::declarator) {
        collect_declarator_identifiers(node.children[i], out);
      }
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::template_parameter_clause) {
      continue;
    }
    collect_template_parameter_value_names(node.children[i], out);
  }
}

void CppAstParser::collect_template_parameter_template_names(
    const CppAstNode & node,
    NameSet & out) const
{
  if(node.kind == CppAstKind::type_parameter) {
    bool is_template_template_parameter = false;
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::template_template_parameter) {
        is_template_template_parameter = true;
        break;
      }
    }
    if(is_template_template_parameter) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind == CppAstKind::identifier) {
          out.insert(node.children[i].value);
        }
      }
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::template_parameter_clause) {
      continue;
    }
    collect_template_parameter_template_names(node.children[i], out);
  }
}

string CppAstParser::current_namespace_path_key(const string & next) const
{
  vector<string> parts = namespace_path_stack;
  if(!next.empty()) {
    parts.push_back(next);
  }

  string key;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i != 0) {
      key += "::";
    }
    key += parts[i];
  }
  return key;
}

void CppAstParser::push_namespace_name_scopes(const string & name, bool is_inline)
{
  const string component = name.empty() ? "<unnamed>" : name;
  const string key = current_namespace_path_key(component);
  namespace_path_stack.push_back(component);
  namespace_inline_stack.push_back(is_inline);

  const auto template_value_found = namespace_template_value_name_scopes.find(key);
  if(template_value_found == namespace_template_value_name_scopes.end()) {
    template_value_parameter_scopes.push_back(NameSet());
  } else {
    template_value_parameter_scopes.push_back(template_value_found->second);
  }

  const auto template_found = namespace_template_name_scopes.find(key);
  if(template_found == namespace_template_name_scopes.end()) {
    template_name_scopes.push_back(NameSet());
  } else {
    template_name_scopes.push_back(template_found->second);
  }

  const auto type_found = namespace_type_name_scopes.find(key);
  if(type_found == namespace_type_name_scopes.end()) {
    type_name_scopes.push_back(NameSet());
  } else {
    type_name_scopes.push_back(type_found->second);
  }

  const auto value_found = namespace_value_name_scopes.find(key);
  if(value_found == namespace_value_name_scopes.end()) {
    value_name_scopes.push_back(NameSet());
  } else {
    value_name_scopes.push_back(value_found->second);
  }
}

void CppAstParser::pop_namespace_name_scopes(bool commit)
{
  if(namespace_path_stack.empty() ||
     namespace_inline_stack.empty() ||
     template_value_parameter_scopes.empty() ||
     template_name_scopes.empty() ||
     type_name_scopes.empty() ||
     value_name_scopes.empty()) {
    return;
  }

  const string key = current_namespace_path_key();
  if(commit) {
    namespace_template_value_name_scopes[key].insert(
        template_value_parameter_scopes.back().begin(),
        template_value_parameter_scopes.back().end());
    namespace_template_name_scopes[key].insert(template_name_scopes.back().begin(),
                                               template_name_scopes.back().end());
    namespace_type_name_scopes[key].insert(type_name_scopes.back().begin(),
                                           type_name_scopes.back().end());
    namespace_value_name_scopes[key].insert(value_name_scopes.back().begin(),
                                            value_name_scopes.back().end());

    if(namespace_inline_stack.back() &&
       template_value_parameter_scopes.size() >= 2 &&
       template_name_scopes.size() >= 2 &&
       type_name_scopes.size() >= 2 &&
       value_name_scopes.size() >= 2) {
      template_value_parameter_scopes[template_value_parameter_scopes.size() - 2].insert(
          template_value_parameter_scopes.back().begin(),
          template_value_parameter_scopes.back().end());
      template_name_scopes[template_name_scopes.size() - 2].insert(
          template_name_scopes.back().begin(), template_name_scopes.back().end());
      type_name_scopes[type_name_scopes.size() - 2].insert(
          type_name_scopes.back().begin(), type_name_scopes.back().end());
      value_name_scopes[value_name_scopes.size() - 2].insert(
          value_name_scopes.back().begin(), value_name_scopes.back().end());

      const std::size_t split = key.rfind("::");
      if(split != std::string::npos) {
        const std::string parent_key = key.substr(0, split);
        namespace_template_value_name_scopes[parent_key].insert(
            template_value_parameter_scopes.back().begin(),
            template_value_parameter_scopes.back().end());
        namespace_template_name_scopes[parent_key].insert(
            template_name_scopes.back().begin(), template_name_scopes.back().end());
        namespace_type_name_scopes[parent_key].insert(
            type_name_scopes.back().begin(), type_name_scopes.back().end());
        namespace_value_name_scopes[parent_key].insert(
            value_name_scopes.back().begin(), value_name_scopes.back().end());
      }
    }
  }

  value_name_scopes.pop_back();
  template_value_parameter_scopes.pop_back();
  template_name_scopes.pop_back();
  type_name_scopes.pop_back();
  namespace_path_stack.pop_back();
  namespace_inline_stack.pop_back();
}

void CppAstParser::set_span(CppAstNode & node, size_t start) const
{
  node.token_start = start;
  node.token_end = pos;
  node.source_location_id = tokens[start].location_id;
}

void CppAstParser::set_error(const string & error)
{
  if(error_msg.empty()) {
    error_msg = error;
    error_msg += token_location_suffix(peek());
    const char * want_context = std::getenv("CPPGM_PARSER_ERROR_CONTEXT");
    if(want_context && *want_context) {
      const size_t start = (pos > 20 ? pos - 20 : 0);
      const size_t end = pos + 20;
      error_msg += " [context={" + token_span_text_spaced(start, end) + "}]";
    }
    parser_trace::append_to_error(error_msg);
  }
}
