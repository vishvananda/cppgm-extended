#pragma once

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "recog_token_buffer.h"
#include "recog_token.h"

// Shared token-stream cursor for recognizers, AST parsers, and semantic parsers.
struct RecogTokenCursor
{
  explicit RecogTokenCursor(const std::vector<RecogToken> & tokens);
  explicit RecogTokenCursor(IRecogTokenSequence & tokens);

protected:
  const RecogToken & peek(std::size_t offset = 0) const;
  bool at_eof() const;
  bool consume_simple(ETokenType type);
  bool consume_close_angle_bracket();
  bool consume_identifier();
  bool consume_literal();
  bool parse_balanced_clause(ETokenType open, ETokenType close);
  // Legacy PA6 recognizer helper. New parser fixes should land in the shared
  // template-angle parser, not here.
  bool parse_legacy_template_id();
  bool parse_balanced_token_sequence(
      std::initializer_list<ETokenType> terminators,
      bool allow_empty);
  std::string token_label(const RecogToken & token) const;
  std::string token_location(const RecogToken & token) const;
  std::string token_location_suffix(const RecogToken & token) const;
  std::string token_span_text(std::size_t start, std::size_t end) const;

  std::unique_ptr<IRecogTokenSequence> owned_tokens;
  IRecogTokenSequence & tokens;
  std::size_t pos;
};
