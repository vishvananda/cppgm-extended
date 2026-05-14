#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "posttokenizer.h"
#include "types.h"

namespace cy86_internal {

using LabelMap = std::unordered_map<std::string, std::uint64_t>;
using ExecutableStatementMap = std::unordered_map<std::uint64_t, std::size_t>;

static const std::size_t kPageSize = 4096;
static const std::uint64_t kHeapBase = 0x100000000ULL;
static const std::uint64_t kStackBase = 0x700000000000ULL;
static const std::uint64_t kStackSize = 0x100000ULL;

bool starts_with(const std::string & value, const std::string & prefix);
bool ends_with(const std::string & value, const std::string & suffix);
bool is_floating_type(EFundamentalType type);
bool is_integral_type(EFundamentalType type);
bool is_register_name(const std::string & name);
std::size_t register_width_bytes(const std::string & name);
std::uint64_t mask_for_width(std::size_t width_bytes);
std::uint64_t align_up(std::uint64_t value, std::uint64_t alignment);
std::vector<unsigned char> encode_uint64(std::uint64_t value,
                                         std::size_t width_bytes);
std::uint64_t decode_uint64(const std::vector<unsigned char> & bytes,
                            std::size_t width_bytes);
std::int64_t sign_extend_uint(std::uint64_t value, std::size_t width_bytes);

template<typename T>
std::vector<unsigned char> bytes_of(const T & value)
{
  std::vector<unsigned char> result(sizeof(T), 0);
  std::memcpy(result.data(), &value, sizeof(T));
  return result;
}

template<typename T>
T scalar_from_bytes(const std::vector<unsigned char> & bytes,
                    std::size_t width_bytes)
{
  T value = T();
  std::size_t limit = std::min(width_bytes, std::min(sizeof(T), bytes.size()));
  if(limit != 0) {
    std::memcpy(&value, bytes.data(), limit);
  }
  return value;
}

long double load_float80(const std::vector<unsigned char> & bytes);
std::vector<unsigned char> store_float80(long double value);

struct LiteralValue
{
  LiteralValue()
    : type(FT_INT),
      num_elements(0),
      is_array(false),
      negated(false)
  {}

  EFundamentalType type;
  std::vector<unsigned char> data;
  std::size_t num_elements;
  bool is_array;
  bool negated;
  std::string source;
};

enum ExprBaseKind
{
  EB_LITERAL,
  EB_LABEL,
  EB_REGISTER
};

struct AddressExpr
{
  AddressExpr()
    : base_kind(EB_LITERAL),
      has_offset(false),
      subtract_offset(false)
  {}

  ExprBaseKind base_kind;
  LiteralValue literal;
  std::string name;
  bool has_offset;
  bool subtract_offset;
  LiteralValue offset;
};

enum OperandKind
{
  OPERAND_REGISTER,
  OPERAND_IMMEDIATE,
  OPERAND_MEMORY
};

struct Operand
{
  OperandKind kind;
  std::string reg;
  AddressExpr expr;
};

enum StatementKind
{
  SK_OPCODE,
  SK_LITERAL_DATA,
  SK_DATA_OPCODE
};

enum ExecKind
{
  EK_NONE,
  EK_JUMP,
  EK_JUMPIF,
  EK_CALL,
  EK_RET,
  EK_SYSCALL,
  EK_MOVE,
  EK_NOT,
  EK_BSWAP,
  EK_AND,
  EK_OR,
  EK_XOR,
  EK_LSHIFT,
  EK_SRSHIFT,
  EK_URSHIFT,
  EK_IADD,
  EK_ISUB,
  EK_SMUL,
  EK_UMUL,
  EK_SDIV,
  EK_UDIV,
  EK_SMOD,
  EK_UMOD,
  EK_IEQ,
  EK_INE,
  EK_SLT,
  EK_ULT,
  EK_SGT,
  EK_UGT,
  EK_SLE,
  EK_ULE,
  EK_SGE,
  EK_UGE,
  EK_FADD,
  EK_FSUB,
  EK_FMUL,
  EK_FDIV,
  EK_FEQ,
  EK_FNE,
  EK_FLT,
  EK_FGT,
  EK_FLE,
  EK_FGE,
  EK_CONV_TO80,
  EK_CONV_FROM80
};

struct Statement
{
  Statement()
    : kind(SK_OPCODE),
      exec_kind(EK_NONE),
      width_bits(0),
      syscall_arity(0),
      category('\0'),
      address(0),
      size(0),
      alignment(1)
  {}

  std::vector<std::string> labels;
  StatementKind kind;
  ExecKind exec_kind;
  int width_bits;
  int syscall_arity;
  char category;
  std::string opcode;
  std::vector<Operand> operands;
  LiteralValue literal;
  std::uint64_t address;
  std::size_t size;
  std::size_t alignment;
};

struct Program
{
  std::vector<Statement> statements;
  LabelMap labels;
  ExecutableStatementMap executable_statements;
  std::vector<unsigned char> image;
  std::uint64_t entry;
  bool has_entry;

  Program() : entry(0), has_entry(false) {}
};

enum ProgramOutputTarget
{
  POT_NATIVE,
  POT_LINUX,
  POT_MACOS
};

enum NativeTarget
{
  NT_LINUX,
  NT_MACOS
};

std::vector<unsigned char> convert_literal_to_width(const LiteralValue & literal,
                                                    std::size_t width_bytes);
std::uint64_t convert_literal_to_uint64(const LiteralValue & literal);
std::vector<unsigned char> evaluated_literal_bytes(const LiteralValue & literal);
std::size_t literal_alignment(const LiteralValue & literal);
bool parse_data_width(const std::string & opcode, int * width_bits);
bool is_known_opcode_name(const std::string & opcode);
bool decode_exec_kind(Statement & statement);
std::size_t opcode_width_bytes(int width_bits);
Program finalize_program(std::vector<Statement> statements);
Program build_program_from_tokens(const std::vector<PostToken> & tokens);
Program build_program(const std::vector<std::string> & srcfiles);
ProgramOutputTarget parse_output_target(const std::string & output_target);
NativeTarget native_target_for_output(ProgramOutputTarget target);
void write_native_program(const Program & program,
                          const std::string & outfile,
                          NativeTarget target);

}  // namespace cy86_internal
