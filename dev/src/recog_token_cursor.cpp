#include <string>

using namespace std;

#include "recog_token_cursor.h"
#include "template_angle_lookup.h"
#include "template_angle_parser.h"

namespace {

struct VectorRecogTokenSequence : IRecogTokenSequence
{
  explicit VectorRecogTokenSequence(const vector<RecogToken> & tokens) :
    tokens(tokens)
  {}

  virtual const RecogToken & operator[](size_t index) const
  {
    return tokens[index];
  }

  virtual const RecogToken & peek(size_t index) const
  {
    if(index >= tokens.size()) {
      return tokens.back();
    }
    return tokens[index];
  }

  virtual size_t size() const
  {
    return tokens.size();
  }

  virtual const RecogToken & back() const
  {
    return tokens.back();
  }

  virtual vector<RecogToken> slice(size_t start, size_t end) const
  {
    return vector<RecogToken>(tokens.begin() + start, tokens.begin() + end);
  }

  virtual string span_text(size_t start, size_t end) const
  {
    string text;
    text.reserve((end - start) * 8);
    for(size_t i = start; i < end; ++i) {
      text += tokens[i].is_rshift_piece() ? ">" : tokens[i].source;
    }
    return text;
  }

  virtual const SourceLocationTable * source_locations() const
  {
    return nullptr;
  }

  const vector<RecogToken> & tokens;
};

}  // namespace

RecogTokenCursor::RecogTokenCursor(const vector<RecogToken> & input_tokens) :
  owned_tokens(new VectorRecogTokenSequence(input_tokens)),
  tokens(*owned_tokens),
  pos(0)
{}

RecogTokenCursor::RecogTokenCursor(IRecogTokenSequence & tokens) :
  tokens(tokens),
  pos(0)
{}

const RecogToken & RecogTokenCursor::peek(size_t offset) const
{
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  if(const RecogTokenBuffer * buffer = dynamic_cast<const RecogTokenBuffer *>(&tokens)) {
    buffer->debug_note_peek(pos, offset);
  }
#endif
  return tokens.peek(pos + offset);
}

bool RecogTokenCursor::at_eof() const
{
  return peek().is_eof();
}

bool RecogTokenCursor::consume_simple(ETokenType type)
{
  if(!peek().is_simple(type)) {
    return false;
  }
  ++pos;
  return true;
}

bool RecogTokenCursor::consume_close_angle_bracket()
{
  if(!peek().is_close_angle_bracket()) {
    return false;
  }
  ++pos;
  return true;
}

bool RecogTokenCursor::consume_identifier()
{
  if(!peek().is_identifier()) {
    return false;
  }
  ++pos;
  return true;
}

bool RecogTokenCursor::consume_literal()
{
  if(!peek().is_literal()) {
    return false;
  }
  ++pos;
  return true;
}

bool RecogTokenCursor::parse_balanced_clause(ETokenType open, ETokenType close)
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
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LSQUARE)) {
      if(!parse_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
        pos = start;
        return false;
      }
      continue;
    }

    if(peek().is_simple(OP_LBRACE)) {
      if(!parse_balanced_clause(OP_LBRACE, OP_RBRACE)) {
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

bool RecogTokenCursor::parse_legacy_template_id()
{
  size_t start = pos;
  if(!peek().is_identifier() || !peek(1).is_simple(OP_LT)) {
    return false;
  }

  static const template_angle_lookup::NameSetLookup lookup =
      template_angle_lookup::make_permissive_lookup();
  size_t end = 0;
  std::vector<std::pair<size_t, size_t> > arg_ranges;
  if(!template_angle::parse_template_id_suffix_ranges(tokens,
                                                      pos + 1,
                                                      lookup,
                                                      end,
                                                      arg_ranges)) {
    pos = start;
    return false;
  }

  pos = end;
  return true;
}

bool RecogTokenCursor::parse_balanced_token_sequence(
    std::initializer_list<ETokenType> terminators,
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
      if(!parse_balanced_clause(OP_LPAREN, OP_RPAREN)) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_simple(OP_LSQUARE)) {
      if(!parse_balanced_clause(OP_LSQUARE, OP_RSQUARE)) {
        pos = start;
        return false;
      }
      saw_tokens = true;
      continue;
    }

    if(peek().is_simple(OP_LBRACE)) {
      if(!parse_balanced_clause(OP_LBRACE, OP_RBRACE)) {
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

string RecogTokenCursor::token_label(const RecogToken & token) const
{
  if(token.is_invalid()) {
    return string("TT_INVALID:") + token.source;
  }
  if(token.is_eof()) {
    return "EOF";
  }
  if(token.kind == RT_RSHIFT_1) {
    return "ST_RSHIFT_1";
  }
  if(token.kind == RT_RSHIFT_2) {
    return "ST_RSHIFT_2";
  }
  if(token.kind == RT_SIMPLE) {
    return string(token_type_to_string(token.simple_type)) + ":" + token.source;
  }
  if(token.kind == RT_IDENTIFIER) {
    return string("TT_IDENTIFIER:") + token.source;
  }
  return string("TT_LITERAL:") + token.source;
}

string RecogTokenCursor::token_location(const RecogToken & token) const
{
  const SourceLocationTable * locations = tokens.source_locations();
  if(locations == nullptr || token.location_id == 0) {
    return string();
  }
  return locations->describe(token.location_id);
}

string RecogTokenCursor::token_location_suffix(const RecogToken & token) const
{
  const string location = token_location(token);
  if(location.empty() || location == "<unknown>") {
    return string();
  }
  return string(" at ") + location;
}

string RecogTokenCursor::token_span_text(size_t start, size_t end) const
{
  return tokens.span_text(start, end);
}
