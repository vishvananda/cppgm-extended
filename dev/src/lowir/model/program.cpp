// The typed LowIR model's own text and identity helpers: operation and type
// spellings, the builtin type table, and type identity.  They belong to the
// model rather than to the reader or the writer, so a consumer of program.h
// links them without either.
#include "lowir/model/program.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <functional>
#include <ostream>
#include <string>

namespace lowir_model {
namespace {

LowOperation::Kind operation_kind(const std::string & text)
{
  static const std::pair<const char *, LowOperation::Kind> operations[] = {
    {"neg", LowOperation::LOP_NEG},
    {"not", LowOperation::LOP_NOT},
    {"bitnot", LowOperation::LOP_BITNOT},
    {"bswap", LowOperation::LOP_BSWAP},
    {"add", LowOperation::LOP_ADD},
    {"sub", LowOperation::LOP_SUB},
    {"mul", LowOperation::LOP_MUL},
    {"div", LowOperation::LOP_DIV},
    {"udiv", LowOperation::LOP_UDIV},
    {"mod", LowOperation::LOP_MOD},
    {"umod", LowOperation::LOP_UMOD},
    {"and", LowOperation::LOP_AND},
    {"or", LowOperation::LOP_OR},
    {"xor", LowOperation::LOP_XOR},
    {"shl", LowOperation::LOP_SHL},
    {"shr", LowOperation::LOP_SHR},
    {"ushr", LowOperation::LOP_USHR},
    {"eq", LowOperation::LOP_EQ},
    {"ne", LowOperation::LOP_NE},
    {"lt", LowOperation::LOP_LT},
    {"ult", LowOperation::LOP_ULT},
    {"le", LowOperation::LOP_LE},
    {"ule", LowOperation::LOP_ULE},
    {"gt", LowOperation::LOP_GT},
    {"ugt", LowOperation::LOP_UGT},
    {"ge", LowOperation::LOP_GE},
    {"uge", LowOperation::LOP_UGE},
    {"trunc", LowOperation::LOP_TRUNC},
    {"sext", LowOperation::LOP_SEXT},
    {"zext", LowOperation::LOP_ZEXT},
    {"sitofp", LowOperation::LOP_SITOFP},
    {"uitofp", LowOperation::LOP_UITOFP},
    {"fptosi", LowOperation::LOP_FPTOSI},
    {"fptoui", LowOperation::LOP_FPTOUI},
    {"fptrunc", LowOperation::LOP_FPTRUNC},
    {"fpext", LowOperation::LOP_FPEXT}
  };
  for(std::size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); ++i)
    if(text == operations[i].first) return operations[i].second;
  if(text.empty()) return LowOperation::LOP_NONE;
  ThrowLowirInputError("unknown LowIR operation: " + text);
}

LowType make_builtin_type(LowTypeKind kind, std::size_t storage_size,
                          std::uint32_t alignment)
{
  LowType result;
  result.kind = kind;
  result.storage_size = storage_size;
  result.alignment = alignment;
  return result;
}

}  // namespace

LowOperation parse_lowir_operation(const std::string & text)
{
  return LowOperation(operation_kind(text));
}

const char * lowir_operation_text(LowOperation operation)
{
  switch(operation.kind) {
  case LowOperation::LOP_NONE: return "";
  case LowOperation::LOP_NEG: return "neg";
  case LowOperation::LOP_NOT: return "not";
  case LowOperation::LOP_BITNOT: return "bitnot";
  case LowOperation::LOP_BSWAP: return "bswap";
  case LowOperation::LOP_ADD: return "add";
  case LowOperation::LOP_SUB: return "sub";
  case LowOperation::LOP_MUL: return "mul";
  case LowOperation::LOP_DIV: return "div";
  case LowOperation::LOP_UDIV: return "udiv";
  case LowOperation::LOP_MOD: return "mod";
  case LowOperation::LOP_UMOD: return "umod";
  case LowOperation::LOP_AND: return "and";
  case LowOperation::LOP_OR: return "or";
  case LowOperation::LOP_XOR: return "xor";
  case LowOperation::LOP_SHL: return "shl";
  case LowOperation::LOP_SHR: return "shr";
  case LowOperation::LOP_USHR: return "ushr";
  case LowOperation::LOP_EQ: return "eq";
  case LowOperation::LOP_NE: return "ne";
  case LowOperation::LOP_LT: return "lt";
  case LowOperation::LOP_ULT: return "ult";
  case LowOperation::LOP_LE: return "le";
  case LowOperation::LOP_ULE: return "ule";
  case LowOperation::LOP_GT: return "gt";
  case LowOperation::LOP_UGT: return "ugt";
  case LowOperation::LOP_GE: return "ge";
  case LowOperation::LOP_UGE: return "uge";
  case LowOperation::LOP_TRUNC: return "trunc";
  case LowOperation::LOP_SEXT: return "sext";
  case LowOperation::LOP_ZEXT: return "zext";
  case LowOperation::LOP_SITOFP: return "sitofp";
  case LowOperation::LOP_UITOFP: return "uitofp";
  case LowOperation::LOP_FPTOSI: return "fptosi";
  case LowOperation::LOP_FPTOUI: return "fptoui";
  case LowOperation::LOP_FPTRUNC: return "fptrunc";
  case LowOperation::LOP_FPEXT: return "fpext";
  }
  ThrowLowirInternalError("invalid compact LowIR operation identity");
}

bool operator==(LowOperation left, LowOperation right)
{
  return left.kind == right.kind;
}

bool operator!=(LowOperation left, LowOperation right)
{
  return !(left == right);
}

std::ostream & operator<<(std::ostream & out, LowOperation operation)
{
  return out << lowir_operation_text(operation);
}

std::size_t lowir_operation_hash(LowOperation operation)
{
  return static_cast<std::size_t>(operation.kind);
}

const LowType & builtin_lowir_type(LowTypeKind kind)
{
  static const LowType void_type = make_builtin_type(LTK_VOID, 0, 1);
  static const LowType i1_type = make_builtin_type(LTK_I1, 1, 1);
  static const LowType i8_type = make_builtin_type(LTK_I8, 1, 1);
  static const LowType u8_type = make_builtin_type(LTK_U8, 1, 1);
  static const LowType i16_type = make_builtin_type(LTK_I16, 2, 2);
  static const LowType u16_type = make_builtin_type(LTK_U16, 2, 2);
  static const LowType i32_type = make_builtin_type(LTK_I32, 4, 4);
  static const LowType u32_type = make_builtin_type(LTK_U32, 4, 4);
  static const LowType i64_type = make_builtin_type(LTK_I64, 8, 8);
  static const LowType i128_type = make_builtin_type(LTK_I128, 16, 16);
  static const LowType f32_type = make_builtin_type(LTK_F32, 4, 4);
  static const LowType f64_type = make_builtin_type(LTK_F64, 8, 8);
  static const LowType f80_type = make_builtin_type(LTK_F80, 16, 16);
  static const LowType ptr_type = make_builtin_type(LTK_PTR, 8, 8);

  switch(kind) {
  case LTK_VOID: return void_type;
  case LTK_I1: return i1_type;
  case LTK_I8: return i8_type;
  case LTK_U8: return u8_type;
  case LTK_I16: return i16_type;
  case LTK_U16: return u16_type;
  case LTK_I32: return i32_type;
  case LTK_U32: return u32_type;
  case LTK_I64: return i64_type;
  case LTK_I128: return i128_type;
  case LTK_F32: return f32_type;
  case LTK_F64: return f64_type;
  case LTK_F80: return f80_type;
  case LTK_PTR: return ptr_type;
  default: ThrowLowirInternalError("invalid built-in LowIR type identity");
  }
}

std::string lowir_type_text(const LowType & type)
{
  switch(type.kind) {
  case LTK_INVALID: return std::string();
  case LTK_VOID: return "void";
  case LTK_I1: return "i1";
  case LTK_I8: return "i8";
  case LTK_U8: return "u8";
  case LTK_I16: return "i16";
  case LTK_U16: return "u16";
  case LTK_I32: return "i32";
  case LTK_U32: return "u32";
  case LTK_I64: return "i64";
  case LTK_I128: return "i128";
  case LTK_F32: return "f32";
  case LTK_F64: return "f64";
  case LTK_F80: return "f80";
  case LTK_PTR: return "ptr";
  case LTK_OBJECT:
    return "obj<" + std::to_string(type.storage_size) + "x" +
      std::to_string(type.alignment) + ">";
  }
  ThrowLowirInternalError("invalid compact LowIR type identity");
}

bool InstructionDebugLocation::present() const
{
  return file.valid() && line != 0 && column != 0;
}

bool same_lowir_type(const LowType & left, const LowType & right)
{
  if(left.kind != right.kind) return false;
  if(left.kind != LTK_OBJECT) return true;
  return left.storage_size == right.storage_size && left.alignment == right.alignment;
}

bool operator==(const LowType & left, const LowType & right)
{
  return same_lowir_type(left, right);
}

bool operator!=(const LowType & left, const LowType & right)
{
  return !same_lowir_type(left, right);
}

}  // namespace lowir_model
