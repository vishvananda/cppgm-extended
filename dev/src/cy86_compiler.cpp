#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <limits.h>
#include <unistd.h>

using namespace std;

#include "cy86_compiler.h"
#include "cy86_internal.h"
#include "types.h"

namespace cy86_internal {

bool starts_with(const string & value, const string & prefix)
{
  return value.size() >= prefix.size() &&
         value.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const string & value, const string & suffix)
{
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool is_floating_type(EFundamentalType type)
{
  return type == FT_FLOAT || type == FT_DOUBLE || type == FT_LONG_DOUBLE;
}

bool is_integral_type(EFundamentalType type)
{
  return !is_floating_type(type) &&
         type != FT_VOID &&
         type != FT_NULLPTR_T;
}

bool is_register_name(const string & name)
{
  static const char * const kRegisters[] =
  {
    "sp", "bp",
    "x8", "x16", "x32", "x64",
    "y8", "y16", "y32", "y64",
    "z8", "z16", "z32", "z64",
    "t8", "t16", "t32", "t64"
  };

  for(size_t i = 0; i < sizeof(kRegisters) / sizeof(kRegisters[0]); ++i) {
    if(name == kRegisters[i]) {
      return true;
    }
  }
  return false;
}

size_t register_width_bytes(const string & name)
{
  if(name == "sp" || name == "bp") {
    return 8;
  }
  string width = name.substr(1);
  if(width == "8") return 1;
  if(width == "16") return 2;
  if(width == "32") return 4;
  if(width == "64") return 8;
  throw logic_error("invalid register: " + name);
}

uint64_t mask_for_width(size_t width_bytes)
{
  if(width_bytes >= 8) {
    return numeric_limits<uint64_t>::max();
  }
  return (uint64_t(1) << (width_bytes * 8)) - 1;
}

uint64_t align_up(uint64_t value, uint64_t alignment)
{
  if(alignment <= 1) {
    return value;
  }
  uint64_t remainder = value % alignment;
  return remainder == 0 ? value : value + (alignment - remainder);
}

vector<unsigned char> encode_uint64(uint64_t value, size_t width_bytes)
{
  vector<unsigned char> result(width_bytes, 0);
  size_t limit = min(width_bytes, static_cast<size_t>(8));
  for(size_t i = 0; i < limit; ++i) {
    result[i] = static_cast<unsigned char>((value >> (8 * i)) & 0xFF);
  }
  return result;
}

uint64_t decode_uint64(const vector<unsigned char> & bytes, size_t width_bytes)
{
  uint64_t result = 0;
  size_t limit = min(width_bytes, static_cast<size_t>(8));
  for(size_t i = 0; i < limit && i < bytes.size(); ++i) {
    result |= (uint64_t(bytes[i]) << (8 * i));
  }
  return result;
}

vector<unsigned char> store_float80(long double value)
{
  if(sizeof(long double) < 10) {
    throw logic_error("host long double does not support float80");
  }

  unsigned char storage[sizeof(long double)];
  memcpy(storage, &value, sizeof(storage));
  return vector<unsigned char>(storage, storage + 10);
}

bool literal_is_arithmetic(const LiteralValue & literal)
{
  return !literal.is_array &&
         (is_integral_type(literal.type) || is_floating_type(literal.type));
}

vector<unsigned char> evaluated_literal_bytes(const LiteralValue & literal)
{
  if(!literal.negated) {
    return literal.data;
  }

  if(!literal_is_arithmetic(literal)) {
    throw logic_error("unary minus requires arithmetic literal");
  }

  if(is_floating_type(literal.type)) {
    if(literal.type == FT_FLOAT) {
      float value = scalar_from_bytes<float>(literal.data, literal.data.size());
      value = -value;
      return bytes_of(value);
    }
    if(literal.type == FT_DOUBLE) {
      double value = scalar_from_bytes<double>(literal.data, literal.data.size());
      value = -value;
      return bytes_of(value);
    }
    long double value = scalar_from_bytes<long double>(literal.data,
                                                       literal.data.size());
    value = -value;
    return bytes_of(value);
  }

  size_t width = literal.data.size();
  uint64_t value = decode_uint64(literal.data, width);
  value = (~value + 1) & mask_for_width(min(width, static_cast<size_t>(8)));
  return encode_uint64(value, width);
}

vector<unsigned char> convert_literal_to_width(const LiteralValue & literal,
                                               size_t width_bytes)
{
  vector<unsigned char> raw = evaluated_literal_bytes(literal);
  if(raw.size() >= width_bytes) {
    return vector<unsigned char>(raw.begin(), raw.begin() + width_bytes);
  }

  unsigned char fill = 0;
  if(!literal.is_array &&
     is_integral_type(literal.type) &&
     type_is_signed(literal.type) &&
     !raw.empty() &&
     (raw.back() & 0x80) != 0) {
    fill = 0xFF;
  }

  raw.resize(width_bytes, fill);
  return raw;
}

uint64_t convert_literal_to_uint64(const LiteralValue & literal)
{
  return decode_uint64(convert_literal_to_width(literal, 8), 8);
}

size_t literal_alignment(const LiteralValue & literal)
{
  size_t alignment = type_to_size(literal.type);
  return alignment == 0 ? 1 : alignment;
}

bool parse_width_suffix(const string & opcode,
                        const string & prefix,
                        const int * widths,
                        size_t nwidths,
                        int * width_bits)
{
  if(!starts_with(opcode, prefix)) {
    return false;
  }

  string suffix = opcode.substr(prefix.size());
  for(size_t i = 0; i < nwidths; ++i) {
    if(suffix == to_string(widths[i])) {
      if(width_bits) {
        *width_bits = widths[i];
      }
      return true;
    }
  }
  return false;
}

bool parse_data_width(const string & opcode, int * width_bits)
{
  static const int kWidths[] = {8, 16, 32, 64};
  return parse_width_suffix(opcode, "data", kWidths, 4, width_bits);
}

bool parse_move_width(const string & opcode, int * width_bits)
{
  static const int kWidths[] = {8, 16, 32, 64, 80};
  return parse_width_suffix(opcode, "move", kWidths, 5, width_bits);
}

bool parse_int_width(const string & opcode,
                     const string & prefix,
                     int * width_bits)
{
  static const int kWidths[] = {8, 16, 32, 64};
  return parse_width_suffix(opcode, prefix, kWidths, 4, width_bits);
}

bool parse_float_width(const string & opcode,
                       const string & prefix,
                       int * width_bits)
{
  static const int kWidths[] = {32, 64, 80};
  return parse_width_suffix(opcode, prefix, kWidths, 3, width_bits);
}

bool parse_float_convert_to80(const string & opcode,
                              char * category,
                              int * width_bits)
{
  if(ends_with(opcode, "convf80")) {
    string middle = opcode.substr(1, opcode.size() - 1 - 7);
    if(opcode[0] == 's' || opcode[0] == 'u') {
      if(middle == "8" || middle == "16" || middle == "32" || middle == "64") {
        *category = opcode[0];
        *width_bits = atoi(middle.c_str());
        return true;
      }
    }
    if(opcode[0] == 'f' && (middle == "32" || middle == "64")) {
      *category = 'f';
      *width_bits = atoi(middle.c_str());
      return true;
    }
  }
  return false;
}

bool parse_float_convert_from80(const string & opcode,
                                char * category,
                                int * width_bits)
{
  static const int kIntWidths[] = {8, 16, 32, 64};
  static const char kIntCats[] = {'s', 'u'};
  for(size_t c = 0; c < 2; ++c) {
    if(starts_with(opcode, string("f80conv") + kIntCats[c])) {
      string suffix = opcode.substr(8);
      for(size_t i = 0; i < 4; ++i) {
        if(suffix == to_string(kIntWidths[i])) {
          *category = kIntCats[c];
          *width_bits = kIntWidths[i];
          return true;
        }
      }
    }
  }
  if(starts_with(opcode, "f80convf")) {
    string suffix = opcode.substr(8);
    if(suffix == "32") { *category = 'f'; *width_bits = 32; return true; }
    if(suffix == "64") { *category = 'f'; *width_bits = 64; return true; }
  }
  return false;
}

bool parse_syscall_arity(const string & opcode, int * arity)
{
  if(!starts_with(opcode, "syscall")) {
    return false;
  }
  string suffix = opcode.substr(7);
  if(suffix.size() != 1 || suffix[0] < '0' || suffix[0] > '6') {
    return false;
  }
  *arity = suffix[0] - '0';
  return true;
}

bool is_known_opcode_name(const string & opcode)
{
  Statement s;
  s.kind = SK_OPCODE;
  s.opcode = opcode;
  return decode_exec_kind(s);
}

bool decode_exec_kind(Statement & statement)
{
  int width_bits = 0;
  int arity = 0;
  char category = '\0';

  statement.exec_kind = EK_NONE;
  statement.width_bits = 0;
  statement.syscall_arity = 0;
  statement.category = '\0';

  if(statement.kind != SK_OPCODE) {
    return false;
  }

  if(statement.opcode == "jump") { statement.exec_kind = EK_JUMP; return true; }
  if(statement.opcode == "jumpif") { statement.exec_kind = EK_JUMPIF; return true; }
  if(statement.opcode == "call") { statement.exec_kind = EK_CALL; return true; }
  if(statement.opcode == "ret") { statement.exec_kind = EK_RET; return true; }

  if(parse_syscall_arity(statement.opcode, &arity)) {
    statement.exec_kind = EK_SYSCALL;
    statement.syscall_arity = arity;
    return true;
  }
  if(parse_move_width(statement.opcode, &width_bits)) {
    statement.exec_kind = EK_MOVE;
    statement.width_bits = width_bits;
    return true;
  }

  static const struct { const char * prefix; ExecKind kind; } kIntOps[] = {
    { "not",     EK_NOT     }, { "bswap",   EK_BSWAP   }, { "and",     EK_AND     },
    { "or",      EK_OR      }, { "xor",     EK_XOR     }, { "lshift",  EK_LSHIFT  },
    { "srshift", EK_SRSHIFT }, { "urshift", EK_URSHIFT }, { "iadd",    EK_IADD    },
    { "isub",    EK_ISUB    }, { "smul",    EK_SMUL    }, { "umul",    EK_UMUL    },
    { "sdiv",    EK_SDIV    }, { "udiv",    EK_UDIV    }, { "smod",    EK_SMOD    },
    { "umod",    EK_UMOD    }, { "ieq",     EK_IEQ     }, { "ine",     EK_INE     },
    { "slt",     EK_SLT     }, { "ult",     EK_ULT     }, { "sgt",     EK_SGT     },
    { "ugt",     EK_UGT     }, { "sle",     EK_SLE     }, { "ule",     EK_ULE     },
    { "sge",     EK_SGE     }, { "uge",     EK_UGE     },
  };
  for(size_t i = 0; i < sizeof(kIntOps) / sizeof(kIntOps[0]); ++i) {
    if(parse_int_width(statement.opcode, kIntOps[i].prefix, &width_bits)) {
      statement.exec_kind = kIntOps[i].kind;
      statement.width_bits = width_bits;
      return true;
    }
  }

  static const struct { const char * prefix; ExecKind kind; } kFloatOps[] = {
    { "fadd", EK_FADD }, { "fsub", EK_FSUB }, { "fmul", EK_FMUL }, { "fdiv", EK_FDIV },
    { "feq",  EK_FEQ  }, { "fne",  EK_FNE  }, { "flt",  EK_FLT  }, { "fgt",  EK_FGT  },
    { "fle",  EK_FLE  }, { "fge",  EK_FGE  },
  };
  for(size_t i = 0; i < sizeof(kFloatOps) / sizeof(kFloatOps[0]); ++i) {
    if(parse_float_width(statement.opcode, kFloatOps[i].prefix, &width_bits)) {
      statement.exec_kind = kFloatOps[i].kind;
      statement.width_bits = width_bits;
      return true;
    }
  }

  if(parse_float_convert_to80(statement.opcode, &category, &width_bits)) {
    statement.exec_kind = EK_CONV_TO80;
    statement.width_bits = width_bits;
    statement.category = category;
    return true;
  }
  if(parse_float_convert_from80(statement.opcode, &category, &width_bits)) {
    statement.exec_kind = EK_CONV_FROM80;
    statement.width_bits = width_bits;
    statement.category = category;
    return true;
  }

  return false;
}

size_t opcode_width_bytes(int width_bits)
{
  return width_bits == 80 ? 10 : static_cast<size_t>(width_bits / 8);
}

}  // namespace cy86_internal

string absolute_path(const string & path)
{
  char * resolved = realpath(path.c_str(), NULL);
  if(resolved) {
    string result(resolved);
    free(resolved);
    return result;
  }

  if(!path.empty() && path[0] == '/') {
    return path;
  }

  char cwd[PATH_MAX];
  if(!getcwd(cwd, sizeof(cwd))) {
    throw logic_error("unable to determine current directory");
  }

  return string(cwd) + "/" + path;
}

void build_cy86_program(const std::vector<std::string> & srcfiles,
                        const std::string & outfile,
                        const std::string & output_target)
{
  cy86_internal::Program program = cy86_internal::build_program(srcfiles);
  string outfile_host = absolute_path(outfile);
  cy86_internal::ProgramOutputTarget target =
      cy86_internal::parse_output_target(output_target);

  cy86_internal::write_native_program(
      program,
      outfile_host,
      cy86_internal::native_target_for_output(target));
}
