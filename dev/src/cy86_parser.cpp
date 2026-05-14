#include <algorithm>
#include <ctime>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "cy86_compiler.h"
#include "cy86_internal.h"
#include "preprocessor.h"
#include "types.h"

namespace cy86_internal {

LiteralValue literal_from_token(const PostToken & token)
{
  LiteralValue literal;
  literal.type = token.type;
  literal.data = token.data;
  literal.num_elements = token.num_elements;
  literal.is_array = token.kind == PT_LITERAL_ARRAY;
  literal.source = token.source;
  return literal;
}

class TokenCursor
{
public:
  explicit TokenCursor(const vector<PostToken> & tokens) :
    tokens_(&tokens),
    source_(nullptr),
    pos_(0)
  {}

  explicit TokenCursor(IPostTokenSource & source) :
    tokens_(nullptr),
    source_(&source),
    pos_(0)
  {}

  bool eof() const
  {
    if(tokens_) {
      return pos_ >= tokens_->size();
    }
    fill(0);
    return buffered_.empty() || buffered_.front().kind == PT_EOF;
  }

  const PostToken & peek(size_t offset = 0) const
  {
    if(!fill(offset)) {
      throw logic_error("unexpected end of input");
    }
    return tokens_ ? (*tokens_)[pos_ + offset] : buffered_[offset];
  }

  bool peek_simple(const string & spelling, size_t offset = 0) const
  {
    if(!fill(offset)) {
      return false;
    }
    const PostToken & token =
        tokens_ ? (*tokens_)[pos_ + offset] : buffered_[offset];
    return token.kind == PT_SIMPLE && token.source == spelling;
  }

  bool match_simple(const string & spelling)
  {
    if(peek_simple(spelling)) {
      advance();
      return true;
    }
    return false;
  }

  void expect_simple(const string & spelling)
  {
    if(!match_simple(spelling)) {
      throw logic_error("expected '" + spelling + "'");
    }
  }

  bool peek_identifier(size_t offset = 0) const
  {
    if(!fill(offset)) {
      return false;
    }
    const PostToken & token =
        tokens_ ? (*tokens_)[pos_ + offset] : buffered_[offset];
    return token.kind == PT_IDENTIFIER;
  }

  string expect_identifier()
  {
    if(!peek_identifier()) {
      throw logic_error("expected identifier");
    }
    string result = peek().source;
    advance();
    return result;
  }

  bool peek_literal_like(size_t offset = 0) const
  {
    if(!fill(offset)) {
      return false;
    }
    const PostToken & token =
        tokens_ ? (*tokens_)[pos_ + offset] : buffered_[offset];
    return token.kind == PT_LITERAL || token.kind == PT_LITERAL_ARRAY;
  }

  LiteralValue expect_literal_like()
  {
    if(!peek_literal_like()) {
      throw logic_error("expected literal");
    }
    LiteralValue result = literal_from_token(peek());
    advance();
    return result;
  }

private:
  bool fill(size_t offset) const
  {
    if(tokens_) {
      return pos_ + offset < tokens_->size();
    }

    while(buffered_.size() <= offset) {
      if(!source_) {
        return false;
      }
      PostToken token = source_->get();
      buffered_.push_back(std::move(token));
      if(buffered_.back().kind == PT_EOF) {
        break;
      }
    }

    return buffered_.size() > offset && buffered_[offset].kind != PT_EOF;
  }

  void advance()
  {
    if(tokens_) {
      ++pos_;
    } else {
      if(buffered_.empty()) {
        throw logic_error("unexpected end of input");
      }
      buffered_.pop_front();
    }
  }

  const vector<PostToken> * tokens_;
  IPostTokenSource * source_;
  size_t pos_;
  mutable deque<PostToken> buffered_;
};

class TranslationUnitTokenSource : public IPostTokenSource
{
public:
  explicit TranslationUnitTokenSource(const vector<string> & srcfiles) :
    srcfiles_(srcfiles),
    unit_index_(0),
    now_(time(NULL))
  {}

  PostToken get()
  {
    for(;;) {
      if(!tokenizer_) {
        if(unit_index_ >= srcfiles_.size()) {
          return PostToken(PT_EOF,
                           string(),
                           static_cast<ETokenType>(0),
                           FT_VOID,
                           vector<unsigned char>(),
                           0,
                           string(),
                           string(),
                           0);
        }
        preprocessor_.reset(new Preprocessor(srcfiles_[unit_index_], now_));
        tokenizer_.reset(new PostTokenizer(*preprocessor_));
      }

      PostToken token = tokenizer_->get();
      switch(token.kind) {
      case PT_EOF:
        tokenizer_.reset();
        preprocessor_.reset();
        ++unit_index_;
        continue;
      case PT_INVALID:
        throw logic_error("invalid token: " + token.source);
      case PT_USER_DEFINED_LITERAL_CHARACTER:
      case PT_USER_DEFINED_LITERAL_STRING_ARRAY:
      case PT_USER_DEFINED_LITERAL_INTEGER:
      case PT_USER_DEFINED_LITERAL_FLOATING:
        throw logic_error("user-defined literal not allowed: " + token.source);
      default:
        return token;
      }
    }
  }

private:
  const vector<string> & srcfiles_;
  size_t unit_index_;
  time_t now_;
  unique_ptr<Preprocessor> preprocessor_;
  unique_ptr<PostTokenizer> tokenizer_;
};

AddressExpr parse_immediate(TokenCursor & cursor);

AddressExpr parse_memory_expr(TokenCursor & cursor)
{
  cursor.expect_simple("[");

  AddressExpr expr;
  if(cursor.peek_identifier()) {
    string name = cursor.expect_identifier();
    if(is_register_name(name)) {
      expr.base_kind = EB_REGISTER;
      expr.name = name;
    } else {
      expr.base_kind = EB_LABEL;
      expr.name = name;
    }
  } else if(cursor.peek_literal_like()) {
    expr.base_kind = EB_LITERAL;
    expr.literal = cursor.expect_literal_like();
  } else {
    throw logic_error("invalid memory operand");
  }

  if(cursor.match_simple("+")) {
    expr.has_offset = true;
    expr.subtract_offset = false;
    expr.offset = cursor.expect_literal_like();
  } else if(cursor.match_simple("-")) {
    expr.has_offset = true;
    expr.subtract_offset = true;
    expr.offset = cursor.expect_literal_like();
  }

  cursor.expect_simple("]");
  return expr;
}

AddressExpr parse_immediate(TokenCursor & cursor)
{
  AddressExpr expr;
  if(cursor.match_simple("(")) {
    if(cursor.match_simple("-")) {
      expr.base_kind = EB_LITERAL;
      expr.literal = cursor.expect_literal_like();
      expr.literal.negated = true;
      cursor.expect_simple(")");
      return expr;
    }

    if(cursor.peek_identifier()) {
      expr.base_kind = EB_LABEL;
      expr.name = cursor.expect_identifier();
      if(is_register_name(expr.name)) {
        throw logic_error("register requires direct operand form");
      }
      if(cursor.match_simple("+")) {
        expr.has_offset = true;
        expr.subtract_offset = false;
        expr.offset = cursor.expect_literal_like();
      } else if(cursor.match_simple("-")) {
        expr.has_offset = true;
        expr.subtract_offset = true;
        expr.offset = cursor.expect_literal_like();
      }
      cursor.expect_simple(")");
      return expr;
    }

    expr.base_kind = EB_LITERAL;
    expr.literal = cursor.expect_literal_like();
    cursor.expect_simple(")");
    return expr;
  }

  if(cursor.peek_literal_like()) {
    expr.base_kind = EB_LITERAL;
    expr.literal = cursor.expect_literal_like();
    return expr;
  }

  if(cursor.peek_identifier()) {
    expr.base_kind = EB_LABEL;
    expr.name = cursor.expect_identifier();
    if(is_register_name(expr.name)) {
      throw logic_error("register requires direct operand form");
    }
    return expr;
  }

  throw logic_error("invalid immediate operand");
}

Operand parse_operand(TokenCursor & cursor)
{
  Operand operand;
  if(cursor.peek_simple("[")) {
    operand.kind = OPERAND_MEMORY;
    operand.expr = parse_memory_expr(cursor);
    return operand;
  }

  if(cursor.peek_identifier()) {
    string name = cursor.expect_identifier();
    if(is_register_name(name)) {
      operand.kind = OPERAND_REGISTER;
      operand.reg = name;
      return operand;
    }
    operand.kind = OPERAND_IMMEDIATE;
    operand.expr.base_kind = EB_LABEL;
    operand.expr.name = name;
    return operand;
  }

  operand.kind = OPERAND_IMMEDIATE;
  operand.expr = parse_immediate(cursor);
  return operand;
}

Statement parse_statement(TokenCursor & cursor)
{
  Statement statement;

  while(cursor.peek_identifier() && cursor.peek_simple(":", 1)) {
    statement.labels.push_back(cursor.expect_identifier());
    cursor.expect_simple(":");
  }

  if(cursor.peek_literal_like()) {
    statement.kind = SK_LITERAL_DATA;
    statement.literal = cursor.expect_literal_like();
    return statement;
  }

  if(cursor.match_simple("-")) {
    statement.kind = SK_LITERAL_DATA;
    statement.literal = cursor.expect_literal_like();
    statement.literal.negated = true;
    return statement;
  }

  if(!cursor.peek_identifier()) {
    throw logic_error("expected statement");
  }

  statement.kind = SK_OPCODE;
  statement.opcode = cursor.expect_identifier();

  while(!cursor.peek_simple(";")) {
    statement.operands.push_back(parse_operand(cursor));
  }

  return statement;
}

void validate_address_expr(const AddressExpr & expr,
                           const LabelMap & labels)
{
  if(expr.base_kind == EB_LABEL &&
     labels.find(expr.name) == labels.end()) {
    throw logic_error("unknown label: " + expr.name);
  }
  if(expr.base_kind == EB_REGISTER &&
     register_width_bytes(expr.name) != 8) {
    throw logic_error("address expression requires 64-bit register");
  }
}

void validate_operand_width(const Operand & operand,
                            size_t width_bytes,
                            bool write_target,
                            const LabelMap & labels)
{
  if(write_target && operand.kind == OPERAND_IMMEDIATE) {
    throw logic_error("write operand may not be immediate");
  }

  if(operand.kind == OPERAND_REGISTER) {
    if(register_width_bytes(operand.reg) != width_bytes) {
      throw logic_error("register width mismatch");
    }
    return;
  }

  validate_address_expr(operand.expr, labels);
}

Program finalize_program(vector<Statement> statements)
{
  Program program;
  program.statements = std::move(statements);

  program.labels.reserve(program.statements.size());
  program.executable_statements.reserve(program.statements.size());

  uint64_t address = 0;
  for(size_t i = 0; i < program.statements.size(); ++i) {
    Statement & statement = program.statements[i];
    int width_bits = 0;

    if(statement.kind == SK_LITERAL_DATA) {
      statement.alignment = literal_alignment(statement.literal);
      statement.size = evaluated_literal_bytes(statement.literal).size();
    } else if(parse_data_width(statement.opcode, &width_bits)) {
      statement.kind = SK_DATA_OPCODE;
      statement.alignment = opcode_width_bytes(width_bits);
      statement.size = opcode_width_bytes(width_bits);
    } else {
      if(!is_known_opcode_name(statement.opcode)) {
        throw logic_error("unknown opcode: " + statement.opcode);
      }
      decode_exec_kind(statement);
      statement.alignment = 1;
      statement.size = 1;
    }

    address = align_up(address, statement.alignment);
    statement.address = address;
    address += statement.size;

    for(size_t j = 0; j < statement.labels.size(); ++j) {
      const string & label = statement.labels[j];
      if(is_register_name(label) || is_known_opcode_name(label)) {
        throw logic_error("invalid label spelling: " + label);
      }
      if(program.labels.find(label) != program.labels.end()) {
        throw logic_error("duplicate label: " + label);
      }
      program.labels[label] = statement.address;
    }

    if(statement.kind == SK_OPCODE) {
      program.executable_statements[statement.address] = i;
    }
  }

  if(!program.statements.empty()) {
    program.has_entry = true;
    LabelMap::const_iterator start = program.labels.find("start");
    program.entry = start == program.labels.end()
        ? program.statements.front().address
        : start->second;
  }

  for(size_t i = 0; i < program.statements.size(); ++i) {
    const Statement & statement = program.statements[i];

    if(statement.kind == SK_LITERAL_DATA) {
      continue;
    }

    if(statement.kind == SK_DATA_OPCODE) {
      if(statement.operands.size() != 1) {
        throw logic_error("data opcode requires one operand");
      }
      validate_operand_width(statement.operands[0], statement.size, false,
                             program.labels);
      continue;
    }

    size_t width_bytes = opcode_width_bytes(statement.width_bits);
    switch(statement.exec_kind) {
    case EK_JUMP:
      if(statement.operands.size() != 1)
        throw logic_error("jump requires one operand");
      validate_operand_width(statement.operands[0], 8, false, program.labels);
      break;
    case EK_JUMPIF:
      if(statement.operands.size() != 2)
        throw logic_error("jumpif requires two operands");
      validate_operand_width(statement.operands[0], 1, false, program.labels);
      validate_operand_width(statement.operands[1], 8, false, program.labels);
      break;
    case EK_CALL:
      if(statement.operands.size() != 1)
        throw logic_error("call requires one operand");
      validate_operand_width(statement.operands[0], 8, false, program.labels);
      break;
    case EK_RET:
      if(!statement.operands.empty())
        throw logic_error("ret takes no operands");
      break;
    case EK_SYSCALL:
      if(statement.operands.size() !=
         static_cast<size_t>(statement.syscall_arity + 2)) {
        throw logic_error("syscall operand count mismatch");
      }
      validate_operand_width(statement.operands[0], 8, true, program.labels);
      for(size_t j = 1; j < statement.operands.size(); ++j) {
        validate_operand_width(statement.operands[j], 8, false, program.labels);
      }
      break;
    case EK_MOVE:
    case EK_NOT:
    case EK_BSWAP:
      if(statement.operands.size() != 2)
        throw logic_error("opcode operand count mismatch");
      validate_operand_width(statement.operands[0], width_bytes, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      break;
    case EK_AND: case EK_OR: case EK_XOR:
    case EK_IADD: case EK_ISUB:
    case EK_SMUL: case EK_UMUL:
    case EK_SDIV: case EK_UDIV:
    case EK_SMOD: case EK_UMOD:
      if(statement.operands.size() != 3)
        throw logic_error("opcode operand count mismatch");
      validate_operand_width(statement.operands[0], width_bytes, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      validate_operand_width(statement.operands[2], width_bytes, false, program.labels);
      break;
    case EK_LSHIFT: case EK_SRSHIFT: case EK_URSHIFT:
      if(statement.operands.size() != 3)
        throw logic_error("opcode operand count mismatch");
      validate_operand_width(statement.operands[0], width_bytes, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      validate_operand_width(statement.operands[2], 1, false, program.labels);
      break;
    case EK_IEQ: case EK_INE:
    case EK_SLT: case EK_ULT: case EK_SGT: case EK_UGT:
    case EK_SLE: case EK_ULE: case EK_SGE: case EK_UGE:
      if(statement.operands.size() != 3)
        throw logic_error("comparison operand count mismatch");
      validate_operand_width(statement.operands[0], 1, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      validate_operand_width(statement.operands[2], width_bytes, false, program.labels);
      break;
    case EK_FADD: case EK_FSUB: case EK_FMUL: case EK_FDIV:
      if(statement.operands.size() != 3)
        throw logic_error("float opcode operand count mismatch");
      validate_operand_width(statement.operands[0], width_bytes, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      validate_operand_width(statement.operands[2], width_bytes, false, program.labels);
      break;
    case EK_FEQ: case EK_FNE:
    case EK_FLT: case EK_FGT: case EK_FLE: case EK_FGE:
      if(statement.operands.size() != 3)
        throw logic_error("float comparison operand count mismatch");
      validate_operand_width(statement.operands[0], 1, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      validate_operand_width(statement.operands[2], width_bytes, false, program.labels);
      break;
    case EK_CONV_TO80:
      if(statement.operands.size() != 2)
        throw logic_error("float80 conversion operand count mismatch");
      validate_operand_width(statement.operands[0], 10, true, program.labels);
      validate_operand_width(statement.operands[1], width_bytes, false, program.labels);
      break;
    case EK_CONV_FROM80:
      if(statement.operands.size() != 2)
        throw logic_error("float80 conversion operand count mismatch");
      validate_operand_width(statement.operands[0], width_bytes, true, program.labels);
      validate_operand_width(statement.operands[1], 10, false, program.labels);
      break;
    case EK_NONE:
      break;
    }
  }

  program.image.assign(address, 0);
  for(size_t i = 0; i < program.statements.size(); ++i) {
    const Statement & statement = program.statements[i];
    if(statement.kind == SK_LITERAL_DATA) {
      vector<unsigned char> bytes = evaluated_literal_bytes(statement.literal);
      copy(bytes.begin(), bytes.end(), program.image.begin() + statement.address);
    } else if(statement.kind == SK_DATA_OPCODE) {
      int width_bits = 0;
      parse_data_width(statement.opcode, &width_bits);
      vector<unsigned char> bytes;
      if(statement.operands[0].kind == OPERAND_IMMEDIATE &&
         statement.operands[0].expr.base_kind == EB_LITERAL &&
         !statement.operands[0].expr.has_offset) {
        bytes = convert_literal_to_width(statement.operands[0].expr.literal,
                                         opcode_width_bytes(width_bits));
      } else {
        uint64_t value = 0;
        const AddressExpr & expr = statement.operands[0].expr;
        if(expr.base_kind == EB_LABEL) {
          value = program.labels.find(expr.name)->second;
        } else if(expr.base_kind == EB_LITERAL) {
          value = convert_literal_to_uint64(expr.literal);
        } else {
          throw logic_error("data opcode immediate may not use register");
        }
        if(expr.has_offset) {
          uint64_t offset_value = convert_literal_to_uint64(expr.offset);
          value = expr.subtract_offset ? value - offset_value
                                       : value + offset_value;
        }
        bytes = encode_uint64(value, opcode_width_bytes(width_bits));
      }
      copy(bytes.begin(), bytes.end(), program.image.begin() + statement.address);
    }
  }

  return program;
}

Program build_program_from_tokens(const vector<PostToken> & tokens)
{
  TokenCursor cursor(tokens);
  vector<Statement> statements;
  while(!cursor.eof()) {
    Statement statement = parse_statement(cursor);
    cursor.expect_simple(";");
    statements.push_back(std::move(statement));
  }
  return finalize_program(std::move(statements));
}

Program build_program(const vector<string> & srcfiles)
{
  TranslationUnitTokenSource source(srcfiles);
  TokenCursor cursor(source);
  vector<Statement> statements;
  while(!cursor.eof()) {
    Statement statement = parse_statement(cursor);
    cursor.expect_simple(";");
    statements.push_back(std::move(statement));
  }
  return finalize_program(std::move(statements));
}

}  // namespace cy86_internal

std::vector<PostToken> tokenize_cy86_translation_unit(const std::string & srcfile)
{
  time_t now = time(NULL);
  Preprocessor preprocessor(srcfile, now);
  PostTokenizer tokenizer(preprocessor);
  vector<PostToken> tokens;

  for(;;) {
    PostToken token = tokenizer.get();
    switch(token.kind) {
    case PT_EOF:
      return tokens;
    case PT_INVALID:
      throw logic_error("invalid token: " + token.source);
    case PT_USER_DEFINED_LITERAL_CHARACTER:
    case PT_USER_DEFINED_LITERAL_STRING_ARRAY:
    case PT_USER_DEFINED_LITERAL_INTEGER:
    case PT_USER_DEFINED_LITERAL_FLOATING:
      throw logic_error("user-defined literal not allowed: " + token.source);
    default:
      tokens.push_back(std::move(token));
      break;
    }
  }
}
