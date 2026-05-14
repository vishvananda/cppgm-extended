#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "cpp_syntax.h"
#include "recog_parser.h"

bool PA6_IsClassName(const string& identifier)
{
  return identifier.find('C') != string::npos;
}

bool PA6_IsTemplateName(const string& identifier)
{
  return identifier.find('T') != string::npos;
}

bool PA6_IsTypedefName(const string& identifier)
{
  return identifier.find('Y') != string::npos;
}

bool PA6_IsEnumName(const string& identifier)
{
  return identifier.find('E') != string::npos;
}

bool PA6_IsNamespaceName(const string& identifier)
{
  return identifier.find('N') != string::npos;
}

bool RecogToken::is_simple(ETokenType type) const
{
  return kind == RT_SIMPLE && simple_type == type;
}

bool RecogToken::is_identifier() const
{
  return kind == RT_IDENTIFIER;
}

bool RecogToken::is_literal() const
{
  return kind == RT_LITERAL;
}

bool RecogToken::is_eof() const
{
  return kind == RT_EOF;
}

bool RecogToken::is_rshift_piece() const
{
  return kind == RT_RSHIFT_1 || kind == RT_RSHIFT_2;
}

bool RecogToken::is_close_angle_bracket() const
{
  return is_simple(OP_GT) || is_rshift_piece();
}

bool RecogToken::is_nonparen() const
{
  if(is_eof()) {
    return false;
  }
  if(kind != RT_SIMPLE) {
    return true;
  }
  switch(simple_type) {
  case OP_LPAREN:
  case OP_RPAREN:
  case OP_LSQUARE:
  case OP_RSQUARE:
  case OP_LBRACE:
  case OP_RBRACE:
    return false;
  default:
    return true;
  }
}

bool RecogToken::is_empty_string() const
{
  return is_literal() && source == "\"\"";
}

bool RecogToken::is_zero() const
{
  return is_literal() && source == "0";
}

bool RecogToken::is_final() const
{
  return is_identifier() && source == "final";
}

bool RecogToken::is_override() const
{
  return is_identifier() && source == "override";
}

bool RecogToken::is_class_name() const
{
  return is_identifier() && PA6_IsClassName(source);
}

bool RecogToken::is_template_name() const
{
  return is_identifier() && PA6_IsTemplateName(source);
}

bool RecogToken::is_typedef_name() const
{
  return is_identifier() && PA6_IsTypedefName(source);
}

bool RecogToken::is_enum_name() const
{
  return is_identifier() && PA6_IsEnumName(source);
}

bool RecogToken::is_namespace_name() const
{
  return is_identifier() && PA6_IsNamespaceName(source);
}

bool RecogToken::is_invalid() const
{
  return kind == RT_INVALID;
}

RecogTokenizer::RecogTokenizer(IPostTokenSource & input) : input(input) {}

RecogToken RecogTokenizer::get()
{
  if(!pending.empty()) {
    RecogToken token = std::move(pending.back());
    pending.pop_back();
    return token;
  }

  auto token = input.get();
  if(token.kind == PT_INVALID) {
    error_msg = string("invalid token: ") + token.source;
    return {RT_INVALID, token.source, static_cast<ETokenType>(0), token.location_id};
  }

  vector<RecogToken> output;
  append(token, output);
  if(output.empty()) {
    return {RT_INVALID, string(), static_cast<ETokenType>(0), token.location_id};
  }

  for(size_t i = output.size(); i > 1; --i) {
    pending.push_back(std::move(output[i - 1]));
  }
  return std::move(output[0]);
}

void RecogTokenizer::get_many(vector<RecogToken> & out, size_t max_tokens)
{
  while(out.size() < max_tokens) {
    if(!pending.empty()) {
      RecogToken token = std::move(pending.back());
      pending.pop_back();
      const bool done = token.is_eof() || token.is_invalid();
      out.push_back(std::move(token));
      if(done) {
        return;
      }
      continue;
    }

    vector<PostToken> post_tokens;
    post_tokens.reserve(max_tokens - out.size());
    input.get_many(post_tokens, max_tokens - out.size());
    for(vector<PostToken>::iterator it = post_tokens.begin();
        it != post_tokens.end(); ++it) {
      PostToken & token = *it;
      if(token.kind == PT_INVALID) {
        error_msg = string("invalid token: ") + token.source;
        out.push_back(
            RecogToken{RT_INVALID, token.source, static_cast<ETokenType>(0),
                       token.location_id});
        return;
      }

      const size_t before = out.size();
      append(token, out);
      if(out.size() == before) {
        out.push_back(
            RecogToken{RT_INVALID, string(), static_cast<ETokenType>(0),
                       token.location_id});
        return;
      }
      if(out.back().is_eof() || out.back().is_invalid()) {
        return;
      }
    }
  }
}

void RecogTokenizer::append(PostToken & token, vector<RecogToken> & output)
{
  switch(token.kind) {
  case PT_SIMPLE:
    if(token.token_type == OP_RSHIFT) {
      output.push_back(RecogToken{RT_RSHIFT_1, token.source, token.token_type, token.location_id});
      output.push_back(RecogToken{RT_RSHIFT_2, std::move(token.source), token.token_type, token.location_id});
    } else {
      output.push_back(RecogToken{RT_SIMPLE, std::move(token.source), token.token_type, token.location_id});
    }
    break;
  case PT_IDENTIFIER:
    output.push_back(RecogToken{RT_IDENTIFIER, std::move(token.source), static_cast<ETokenType>(0),
                                token.location_id});
    break;
  case PT_LITERAL:
  case PT_LITERAL_ARRAY:
  case PT_USER_DEFINED_LITERAL_CHARACTER:
  case PT_USER_DEFINED_LITERAL_STRING_ARRAY:
  case PT_USER_DEFINED_LITERAL_INTEGER:
  case PT_USER_DEFINED_LITERAL_FLOATING:
    output.push_back(RecogToken{RT_LITERAL, std::move(token.source), static_cast<ETokenType>(0),
                                token.location_id});
    break;
  case PT_EOF:
    output.push_back(RecogToken{RT_EOF, string(), static_cast<ETokenType>(0),
                                token.location_id});
    break;
  case PT_INVALID:
    break;
  }
}

RecogParser::RecogParser(const vector<RecogToken> & tokens) :
  RecogTokenCursor(tokens)
{}

RecogParser::RecogParser(IRecogTokenSequence & tokens) :
  RecogTokenCursor(tokens)
{}

bool RecogParser::parse_pa6_balanced_clause(ETokenType open, ETokenType close)
{
  size_t start = pos;
  if(!consume_simple(open)) {
    pos = start;
    return false;
  }

  while(!at_eof()) {
    if(consume_simple(close)) {
      return true;
    }

    if(peek().is_simple(OP_LPAREN)) {
      if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LSQUARE)) {
      if(!parse_pa6_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LBRACE)) {
      if(!parse_pa6_balanced_clause(OP_LBRACE, OP_RBRACE)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_template_name() && peek(1).is_simple(OP_LT)) {
      if(!parse_legacy_template_id()) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_RPAREN) || peek().is_simple(OP_RSQUARE) ||
       peek().is_simple(OP_RBRACE)) {
      pos = start;
      return false;
    }

    ++pos;
  }

  pos = start;
  return false;
}

bool RecogParser::parse_pa6_balanced_token_sequence(
    initializer_list<ETokenType> terminators,
    bool allow_empty)
{
  size_t start = pos;
  bool saw_tokens = false;

  auto is_terminator = [&](const RecogToken & token) {
    if(token.kind != RT_SIMPLE) {
      return false;
    }

    for(auto terminator : terminators) {
      if(token.simple_type == terminator) {
        return true;
      }
    }

    return false;
  };

  while(!at_eof()) {
    if(is_terminator(peek())) {
      break;
    }

    if(peek().is_simple(OP_LPAREN)) {
      if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_simple(OP_LSQUARE)) {
      if(!parse_pa6_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_simple(OP_LBRACE)) {
      if(!parse_pa6_balanced_clause(OP_LBRACE, OP_RBRACE)) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_template_name() && peek(1).is_simple(OP_LT)) {
      if(!parse_legacy_template_id()) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_simple(OP_RPAREN) || peek().is_simple(OP_RSQUARE) ||
       peek().is_simple(OP_RBRACE)) {
      pos = start;
      return false;
    }

    ++pos;
    saw_tokens = true;
  }

  if(!allow_empty && !saw_tokens) {
    pos = start;
    return false;
  }

  return true;
}

bool RecogParser::parse_translation_unit()
{
  if(peek().is_invalid()) {
    set_error(string("invalid token: ") + peek().source);
    return false;
  }

  while(!at_eof()) {
    if(peek().is_invalid()) {
      set_error(string("invalid token: ") + peek().source);
      return false;
    }
    if(!parse_declaration()) {
      if(peek().is_invalid()) {
        set_error(string("invalid token: ") + peek().source);
        return false;
      }
      set_error("expected declaration");
      return false;
    }
  }
  return true;
}

bool RecogParser::parse_declaration()
{
  size_t start = pos;
  if(parse_empty_declaration()) {
    return true;
  }

  if(is_cv_qualifier(peek()) || is_simple_type_specifier(peek()) ||
     is_decl_specifier_keyword(peek()) || is_type_name_identifier(peek())) {
    if(!parse_decl_specifier_seq()) {
      pos = start;
      return false;
    }

    bool saw_declarator = parse_declarator();
    if(!parse_declaration_suffix(saw_declarator)) {
      pos = start;
      return false;
    }
    return true;
  }

  if(parse_generic_declaration()) {
    return true;
  }

  pos = start;
  return false;
}

bool RecogParser::parse_empty_declaration()
{
  return consume_simple(OP_SEMICOLON);
}

bool RecogParser::parse_declaration_suffix(bool saw_declarator)
{
  if(saw_declarator && peek().is_simple(OP_LBRACE)) {
    return parse_function_body();
  }
  return parse_generic_declaration();
}

bool RecogParser::parse_generic_declaration()
{
  size_t start = pos;
  while(!at_eof()) {
    if(peek().is_simple(OP_SEMICOLON)) {
      ++pos;
      return true;
    }

    if(peek().is_simple(OP_LPAREN)) {
      if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LSQUARE)) {
      if(!parse_pa6_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LBRACE)) {
      if(!parse_pa6_balanced_clause(OP_LBRACE, OP_RBRACE)) {
        pos = start;
        return false;
      }

      while(peek().is_simple(KW_CATCH)) {
        ++pos;
        if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN) ||
           !parse_compound_statement()) {
          pos = start;
          return false;
        }
      }

      consume_simple(OP_SEMICOLON);
      return true;
    }

    if(peek().is_template_name() && peek(1).is_simple(OP_LT)) {
      if(!parse_legacy_template_id()) {
        pos = start;
        return false;
      }
      continue;
    }

    ++pos;
  }

  pos = start;
  return false;
}

bool RecogParser::parse_decl_specifier_seq()
{
  size_t start = pos;
  bool matched = false;
  bool saw_type_name = false;

  for(;;) {
    const auto & token = peek();
    if(token.is_identifier() && token.source == "__extension__") {
      ++pos;
      continue;
    }
    if(is_cv_qualifier(token)) {
      ++pos;
      matched = true;
      continue;
    }
    if(is_simple_type_specifier(token)) {
      ++pos;
      matched = true;
      saw_type_name = true;
      continue;
    }
    if(is_decl_specifier_keyword(token)) {
      ++pos;
      matched = true;
      continue;
    }
    if(!saw_type_name && is_type_name_identifier(token)) {
      ++pos;
      matched = true;
      saw_type_name = true;
      continue;
    }
    break;
  }

  if(!matched) {
    pos = start;
  }
  return matched;
}

bool RecogParser::parse_declarator()
{
  size_t start = pos;
  if(!consume_identifier()) {
    pos = start;
    return false;
  }
  if(!parse_parameters_and_qualifiers()) {
    pos = start;
    return false;
  }
  return true;
}

bool RecogParser::parse_parameters_and_qualifiers()
{
  return parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN);
}

bool RecogParser::parse_function_body()
{
  return parse_compound_statement();
}

bool RecogParser::parse_compound_statement()
{
  size_t start = pos;
  if(!consume_simple(OP_LBRACE)) {
    pos = start;
    return false;
  }
  while(!peek().is_eof() && !peek().is_simple(OP_RBRACE)) {
    if(!parse_statement()) {
      pos = start;
      return false;
    }
  }
  if(!consume_simple(OP_RBRACE)) {
    pos = start;
    return false;
  }
  return true;
}

bool RecogParser::parse_statement()
{
  if(peek().is_simple(OP_LBRACE)) {
    return parse_compound_statement();
  }

  if(peek().is_simple(KW_IF) || peek().is_simple(KW_SWITCH) ||
     peek().is_simple(KW_WHILE) || peek().is_simple(KW_DO) ||
     peek().is_simple(KW_FOR) || peek().is_simple(KW_TRY)) {
    return parse_control_statement();
  }

  if(peek().is_simple(KW_CASE) || peek().is_simple(KW_DEFAULT) ||
     (peek().is_identifier() && peek(1).is_simple(OP_COLON))) {
    return parse_labeled_statement();
  }

  return parse_generic_semicolon_statement();
}

bool RecogParser::parse_control_statement()
{
  size_t start = pos;

  if(peek().is_simple(KW_IF) || peek().is_simple(KW_SWITCH) ||
     peek().is_simple(KW_WHILE)) {
    ++pos;
    if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN) || !parse_statement()) {
      pos = start;
      return false;
    }
    if(tokens[start].is_simple(KW_IF) && peek().is_simple(KW_ELSE)) {
      ++pos;
      if(!parse_statement()) {
        pos = start;
        return false;
      }
    }
    return true;
  }

  if(peek().is_simple(KW_DO)) {
    ++pos;
    if(!parse_statement() || !consume_simple(KW_WHILE) ||
       !parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN) ||
       !consume_simple(OP_SEMICOLON)) {
      pos = start;
      return false;
    }
    return true;
  }

  if(peek().is_simple(KW_FOR)) {
    ++pos;
    if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN) || !parse_statement()) {
      pos = start;
      return false;
    }
    return true;
  }

  if(peek().is_simple(KW_TRY)) {
    ++pos;
    if(!parse_compound_statement()) {
      pos = start;
      return false;
    }
    if(!peek().is_simple(KW_CATCH)) {
      pos = start;
      return false;
    }
    do {
      ++pos;
      if(!parse_pa6_balanced_clause(OP_LPAREN, OP_RPAREN) ||
         !parse_compound_statement()) {
        pos = start;
        return false;
      }
    } while(peek().is_simple(KW_CATCH));
    return true;
  }

  pos = start;
  return false;
}

bool RecogParser::parse_labeled_statement()
{
  size_t start = pos;

  if(peek().is_identifier() && peek(1).is_simple(OP_COLON)) {
    pos += 2;
    if(!parse_statement()) {
      pos = start;
      return false;
    }
    return true;
  }

  if(peek().is_simple(KW_DEFAULT)) {
    ++pos;
    if(!consume_simple(OP_COLON) || !parse_statement()) {
      pos = start;
      return false;
    }
    return true;
  }

  if(peek().is_simple(KW_CASE)) {
    ++pos;
    if(!parse_pa6_balanced_token_sequence({OP_COLON}, false) ||
       !consume_simple(OP_COLON) || !parse_statement()) {
      pos = start;
      return false;
    }
    return true;
  }

  pos = start;
  return false;
}

bool RecogParser::parse_generic_semicolon_statement()
{
  size_t start = pos;

  if(consume_simple(OP_SEMICOLON)) {
    return true;
  }

  if(!parse_pa6_balanced_token_sequence({OP_SEMICOLON}, false) ||
     !consume_simple(OP_SEMICOLON)) {
    pos = start;
    return false;
  }

  return true;
}

bool RecogParser::parse_expression()
{
  size_t start = pos;
  if(!parse_pa6_balanced_token_sequence(
         {OP_SEMICOLON, OP_RPAREN, OP_RSQUARE, OP_RBRACE}, false)) {
    pos = start;
    return false;
  }
  return true;
}

void RecogParser::set_error(const string & error)
{
  if(error_msg.empty()) {
    error_msg = error + " near " + describe_recog_token(peek()) +
                token_location_suffix(peek());
  }
}
