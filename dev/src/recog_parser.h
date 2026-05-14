#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>

#include "recog_token.h"
#include "recog_token_cursor.h"

struct RecogParser : RecogTokenCursor
{
  explicit RecogParser(const std::vector<RecogToken> & tokens);
  explicit RecogParser(IRecogTokenSequence & tokens);
  bool parse_translation_unit();
  const std::string & error() const { return error_msg; }

protected:
  bool parse_pa6_balanced_clause(ETokenType open, ETokenType close);
  bool parse_pa6_balanced_token_sequence(
      std::initializer_list<ETokenType> terminators,
      bool allow_empty);
  bool parse_declaration();
  bool parse_empty_declaration();
  bool parse_declaration_suffix(bool saw_declarator);
  bool parse_generic_declaration();
  bool parse_decl_specifier_seq();
  bool parse_declarator();
  bool parse_parameters_and_qualifiers();
  bool parse_function_body();
  bool parse_compound_statement();
  bool parse_statement();
  bool parse_control_statement();
  bool parse_labeled_statement();
  bool parse_generic_semicolon_statement();
  bool parse_expression();
  void set_error(const std::string & error);

  std::string error_msg;
};
