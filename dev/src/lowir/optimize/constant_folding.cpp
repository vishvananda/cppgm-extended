// Constant folding of single LowIR instructions: literal operands folded
// through the operation with the operand type's width and signedness, and
// the algebraic identities that drop an operation.  Split from pipeline.cpp.
#include "lowir/optimize/constant_folding.h"
#include "lowir/optimize/scalar_rules.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace lowir_opt {
namespace {

using lowir_model::Instruction;
using lowir_model::LowOperation;
using lowir_model::LowType;
using lowir_model::LowTypeKind;
using lowir_model::Operand;

}  // namespace

namespace {

Operand integer_operand(long long value, const LowType & type)
{
  Operand result;
  result.kind = Operand::OP_INTEGER;
  result.has_int_value = true;
  result.int_value = value;
  result.int_high = value < 0 ? ~UINT64_C(0) : 0;
  result.literal_type = type;
  return result;
}

Operand floating_operand(long double value, const LowType & type)
{
  Operand result;
  result.kind = Operand::OP_FLOAT;
  result.literal_type = type;
  lowir_model::lowir_floating_value_bits(
    value, type, &result.literal_low, &result.literal_high);
  result.has_float_bits = true;
  return result;
}

bool is_integer_type(const LowType & type)
{
  return type.kind == lowir_model::LTK_I1 ||
    (type.kind >= lowir_model::LTK_I8 && type.kind <= lowir_model::LTK_I64) ||
    type.kind == lowir_model::LTK_I128;
}

bool is_float_type(const LowType & type)
{
  return type.kind >= lowir_model::LTK_F32 && type.kind <= lowir_model::LTK_F80;
}

typedef __int128 WideSigned;

typedef unsigned __int128 WideUnsigned;

WideUnsigned wide_mask(const LowType & type)
{
  const std::size_t width = lowir_model::lowir_type_bit_width(type);
  return width >= 128 ? ~static_cast<WideUnsigned>(0) :
    (static_cast<WideUnsigned>(1) << width) - 1;
}

WideUnsigned wide_integer(long long value)
{
  return static_cast<WideUnsigned>(static_cast<WideSigned>(value));
}

bool representable_wide_integer(WideUnsigned value, const LowType & type,
                                Operand * result)
{
  const WideSigned signed_value = static_cast<WideSigned>(value);
  if(signed_value < static_cast<WideSigned>(std::numeric_limits<long long>::min()) ||
     signed_value > static_cast<WideSigned>(std::numeric_limits<long long>::max()))
    return false;
  *result = integer_operand(static_cast<long long>(signed_value), type);
  return true;
}

std::uint64_t width_mask(const LowType & type)
{
  const std::size_t width = lowir_model::lowir_type_bit_width(type);
  return width >= 64 ? ~UINT64_C(0) : (UINT64_C(1) << width) - 1;
}

long long normalize_integer(std::uint64_t value, const LowType & type)
{
  value &= width_mask(type);
  if(type.kind == lowir_model::LTK_U8 || type.kind == lowir_model::LTK_U16 ||
     type.kind == lowir_model::LTK_U32 || type.kind == lowir_model::LTK_PTR)
    return static_cast<long long>(value);
  const std::size_t width = lowir_model::lowir_type_bit_width(type);
  if(width && width < 64 && (value & (UINT64_C(1) << (width - 1))))
    value |= ~width_mask(type);
  return static_cast<long long>(value);
}

}  // namespace

bool commutative(LowOperation op)
{
  return op.kind == LowOperation::LOP_ADD || op.kind == LowOperation::LOP_MUL || op.kind == LowOperation::LOP_AND || op.kind == LowOperation::LOP_OR ||
    op.kind == LowOperation::LOP_XOR;
}

bool fold_unary(const Instruction & ins, Operand * result)
{
  if(ins.first.kind != Operand::OP_INTEGER || !ins.first.has_int_value ||
     !is_integer_type(ins.type)) return false;
  if(lowir_model::lowir_type_bit_width(ins.type) > 64) {
    const WideUnsigned value = wide_integer(ins.first.int_value);
    WideUnsigned folded = 0;
    if(ins.op.kind == LowOperation::LOP_NEG) folded = -value;
    else if(ins.op.kind == LowOperation::LOP_BITNOT) folded = ~value;
    else if(ins.op.kind == LowOperation::LOP_NOT)
      return (*result = integer_operand(value == 0, ins.type), true);
    else return false;
    return representable_wide_integer(folded, ins.type, result);
  }
  const std::uint64_t value = static_cast<std::uint64_t>(ins.first.int_value);
  if(ins.op.kind == LowOperation::LOP_NEG)
    *result = integer_operand(normalize_integer(UINT64_C(0) - value, ins.type), ins.type);
  else if(ins.op.kind == LowOperation::LOP_BITNOT)
    *result = integer_operand(normalize_integer(~value, ins.type), ins.type);
  else if(ins.op.kind == LowOperation::LOP_NOT)
    *result = integer_operand(value == 0, ins.type);
  else return false;
  return true;
}

bool fold_binary(const Instruction & ins, Operand * result)
{
  if(ins.first.kind != Operand::OP_INTEGER || !ins.first.has_int_value ||
     ins.second.kind != Operand::OP_INTEGER || !ins.second.has_int_value ||
     !is_integer_type(ins.type)) return false;
  if(lowir_model::lowir_type_bit_width(ins.type) > 64) {
    const WideUnsigned a = wide_integer(ins.first.int_value);
    const WideUnsigned b = wide_integer(ins.second.int_value);
    WideUnsigned value = 0;
    if(ins.op.kind == LowOperation::LOP_ADD) value = a + b;
    else if(ins.op.kind == LowOperation::LOP_SUB) value = a - b;
    else if(ins.op.kind == LowOperation::LOP_MUL) value = a * b;
    else if(ins.op.kind == LowOperation::LOP_AND) value = a & b;
    else if(ins.op.kind == LowOperation::LOP_OR) value = a | b;
    else if(ins.op.kind == LowOperation::LOP_XOR) value = a ^ b;
    else if(ins.op.kind == LowOperation::LOP_SHL && b < 128)
      value = a << static_cast<unsigned>(b);
    else if(ins.op.kind == LowOperation::LOP_USHR && b < 128)
      value = a >> static_cast<unsigned>(b);
    else if(ins.op.kind == LowOperation::LOP_SHR && b < 128)
      value = static_cast<WideUnsigned>(static_cast<WideSigned>(a) >>
                                       static_cast<unsigned>(b));
    else if((ins.op.kind == LowOperation::LOP_UDIV || ins.op.kind == LowOperation::LOP_UMOD) && b)
      value = ins.op.kind == LowOperation::LOP_UDIV ? a / b : a % b;
    else if((ins.op.kind == LowOperation::LOP_DIV || ins.op.kind == LowOperation::LOP_MOD) && b) {
      const WideSigned signed_a = static_cast<WideSigned>(a);
      const WideSigned signed_b = static_cast<WideSigned>(b);
      value = static_cast<WideUnsigned>(ins.op.kind == LowOperation::LOP_DIV ?
        signed_a / signed_b : signed_a % signed_b);
    } else return false;
    return representable_wide_integer(value, ins.type, result);
  }
  const std::uint64_t a = static_cast<std::uint64_t>(ins.first.int_value);
  const std::uint64_t b = static_cast<std::uint64_t>(ins.second.int_value);
  std::uint64_t value = 0;
  if(ins.op.kind == LowOperation::LOP_ADD) value = a + b;
  else if(ins.op.kind == LowOperation::LOP_SUB) value = a - b;
  else if(ins.op.kind == LowOperation::LOP_MUL) value = a * b;
  else if(ins.op.kind == LowOperation::LOP_AND) value = a & b;
  else if(ins.op.kind == LowOperation::LOP_OR) value = a | b;
  else if(ins.op.kind == LowOperation::LOP_XOR) value = a ^ b;
  else if(ins.op.kind == LowOperation::LOP_SHL && b < 64) value = a << b;
  else if(ins.op.kind == LowOperation::LOP_USHR && b < 64) value = a >> b;
  else if(ins.op.kind == LowOperation::LOP_SHR && b < 64)
    value = static_cast<std::uint64_t>(ins.first.int_value >> b);
  else if((ins.op.kind == LowOperation::LOP_UDIV || ins.op.kind == LowOperation::LOP_UMOD) && b)
    value = ins.op.kind == LowOperation::LOP_UDIV ? a / b : a % b;
  else if((ins.op.kind == LowOperation::LOP_DIV || ins.op.kind == LowOperation::LOP_MOD) && ins.second.int_value &&
          !(ins.first.int_value == std::numeric_limits<long long>::min() &&
            ins.second.int_value == -1))
    value = static_cast<std::uint64_t>(ins.op.kind == LowOperation::LOP_DIV ?
      ins.first.int_value / ins.second.int_value :
      ins.first.int_value % ins.second.int_value);
  else return false;
  *result = integer_operand(normalize_integer(value, ins.type), ins.type);
  return true;
}

bool fold_compare(const Instruction & ins, Operand * result)
{
  bool value = false;
  if(ins.first.kind == Operand::OP_INTEGER && ins.first.has_int_value &&
     ins.second.kind == Operand::OP_INTEGER && ins.second.has_int_value) {
    const long long a = ins.first.int_value;
    const long long b = ins.second.int_value;
    if(lowir_model::lowir_type_bit_width(ins.type) > 64) {
      const WideSigned signed_a = static_cast<WideSigned>(a);
      const WideSigned signed_b = static_cast<WideSigned>(b);
      const WideUnsigned unsigned_a = static_cast<WideUnsigned>(signed_a);
      const WideUnsigned unsigned_b = static_cast<WideUnsigned>(signed_b);
      if(ins.op.kind == LowOperation::LOP_EQ) value = unsigned_a == unsigned_b;
      else if(ins.op.kind == LowOperation::LOP_NE) value = unsigned_a != unsigned_b;
      else if(ins.op.kind == LowOperation::LOP_LT) value = signed_a < signed_b;
      else if(ins.op.kind == LowOperation::LOP_LE) value = signed_a <= signed_b;
      else if(ins.op.kind == LowOperation::LOP_GT) value = signed_a > signed_b;
      else if(ins.op.kind == LowOperation::LOP_GE) value = signed_a >= signed_b;
      else if(ins.op.kind == LowOperation::LOP_ULT) value = unsigned_a < unsigned_b;
      else if(ins.op.kind == LowOperation::LOP_ULE) value = unsigned_a <= unsigned_b;
      else if(ins.op.kind == LowOperation::LOP_UGT) value = unsigned_a > unsigned_b;
      else if(ins.op.kind == LowOperation::LOP_UGE) value = unsigned_a >= unsigned_b;
      else return false;
      *result = integer_operand(value ? 1 : 0,
        lowir_model::builtin_lowir_type(lowir_model::LTK_I64));
      return true;
    }
    const std::uint64_t ua = static_cast<std::uint64_t>(a) & width_mask(ins.type);
    const std::uint64_t ub = static_cast<std::uint64_t>(b) & width_mask(ins.type);
    if(ins.op.kind == LowOperation::LOP_EQ) value = ua == ub;
    else if(ins.op.kind == LowOperation::LOP_NE) value = ua != ub;
    else if(ins.op.kind == LowOperation::LOP_LT) value = a < b;
    else if(ins.op.kind == LowOperation::LOP_LE) value = a <= b;
    else if(ins.op.kind == LowOperation::LOP_GT) value = a > b;
    else if(ins.op.kind == LowOperation::LOP_GE) value = a >= b;
    else if(ins.op.kind == LowOperation::LOP_ULT) value = ua < ub;
    else if(ins.op.kind == LowOperation::LOP_ULE) value = ua <= ub;
    else if(ins.op.kind == LowOperation::LOP_UGT) value = ua > ub;
    else if(ins.op.kind == LowOperation::LOP_UGE) value = ua >= ub;
    else return false;
  } else if(ins.first.kind == Operand::OP_FLOAT &&
            ins.second.kind == Operand::OP_FLOAT) {
    const long double a = lowir_model::lowir_floating_value(
      ins.first.literal_low, ins.first.literal_high, ins.first.literal_type);
    const long double b = lowir_model::lowir_floating_value(
      ins.second.literal_low, ins.second.literal_high, ins.second.literal_type);
    if(ins.op.kind == LowOperation::LOP_EQ) value = a == b;
    else if(ins.op.kind == LowOperation::LOP_NE) value = a != b;
    else if(ins.op.kind == LowOperation::LOP_LT) value = a < b;
    else if(ins.op.kind == LowOperation::LOP_LE) value = a <= b;
    else if(ins.op.kind == LowOperation::LOP_GT) value = a > b;
    else if(ins.op.kind == LowOperation::LOP_GE) value = a >= b;
    else return false;
  } else if(!is_float_type(ins.type) &&
            same_operand(ins.first, ins.second)) {
    if(ins.op.kind == LowOperation::LOP_EQ || ins.op.kind == LowOperation::LOP_LE || ins.op.kind == LowOperation::LOP_GE ||
       ins.op.kind == LowOperation::LOP_ULE || ins.op.kind == LowOperation::LOP_UGE) value = true;
    else if(ins.op.kind == LowOperation::LOP_NE || ins.op.kind == LowOperation::LOP_LT || ins.op.kind == LowOperation::LOP_GT ||
            ins.op.kind == LowOperation::LOP_ULT || ins.op.kind == LowOperation::LOP_UGT) value = false;
    else return false;
  } else if(!is_float_type(ins.type) &&
            ((is_zero(ins.first) && ins.op.kind == LowOperation::LOP_UGT) ||
             (is_zero(ins.second) && ins.op.kind == LowOperation::LOP_ULT))) {
    // No unsigned value is below zero.
    value = false;
  } else if(!is_float_type(ins.type) &&
            ((is_zero(ins.first) && ins.op.kind == LowOperation::LOP_ULE) ||
             (is_zero(ins.second) && ins.op.kind == LowOperation::LOP_UGE))) {
    value = true;
  } else return false;
  *result = integer_operand(value ? 1 : 0,
    lowir_model::builtin_lowir_type(lowir_model::LTK_I64));
  return true;
}

bool fold_convert(const Instruction & ins, Operand * result)
{
  if(lowir_model::same_lowir_type(ins.type, ins.source_type)) {
    *result = ins.first;
    return true;
  }
  if(ins.first.kind == Operand::OP_INTEGER && ins.first.has_int_value) {
    if(is_integer_type(ins.type)) {
      if(lowir_model::lowir_type_bit_width(ins.type) > 64) {
        WideUnsigned value = wide_integer(ins.first.int_value);
        if(ins.op.kind == LowOperation::LOP_ZEXT) value &= wide_mask(ins.source_type);
        else if(ins.op.kind == LowOperation::LOP_SEXT &&
                lowir_model::lowir_type_bit_width(ins.source_type) < 128) {
          const WideUnsigned mask = wide_mask(ins.source_type);
          value &= mask;
          const std::size_t source_width =
            lowir_model::lowir_type_bit_width(ins.source_type);
          if(source_width &&
             (value & (static_cast<WideUnsigned>(1) <<
                       (source_width - 1))))
            value |= ~mask;
        } else return false;
        return representable_wide_integer(value, ins.type, result);
      }
      std::uint64_t value = static_cast<std::uint64_t>(ins.first.int_value);
      if(ins.op.kind == LowOperation::LOP_ZEXT) value &= width_mask(ins.source_type);
      *result = integer_operand(normalize_integer(value, ins.type), ins.type);
      return true;
    }
    if(is_float_type(ins.type) &&
       lowir_model::lowir_type_bit_width(ins.source_type) <= 64 &&
       (ins.op.kind == LowOperation::LOP_SITOFP || ins.op.kind == LowOperation::LOP_UITOFP)) {
      const long double value = ins.op.kind == LowOperation::LOP_UITOFP ?
        static_cast<long double>(static_cast<std::uint64_t>(ins.first.int_value) &
                                 width_mask(ins.source_type)) :
        static_cast<long double>(ins.first.int_value);
      *result = floating_operand(value, ins.type);
      return true;
    }
  }
  if(ins.first.kind == Operand::OP_FLOAT && is_float_type(ins.type) &&
     (ins.op.kind == LowOperation::LOP_FPEXT || ins.op.kind == LowOperation::LOP_FPTRUNC)) {
    *result = floating_operand(lowir_model::lowir_floating_value(
      ins.first.literal_low, ins.first.literal_high, ins.first.literal_type),
      ins.type);
    return true;
  }
  return false;
}

bool algebraic_identity(const Instruction & ins, Operand * result)
{
  if(ins.kind != Instruction::IK_BINARY) return false;
  if((ins.op.kind == LowOperation::LOP_ADD || ins.op.kind == LowOperation::LOP_OR || ins.op.kind == LowOperation::LOP_XOR) && is_zero(ins.second))
    *result = ins.first;
  else if(ins.op.kind == LowOperation::LOP_ADD && is_zero(ins.first)) *result = ins.second;
  else if(ins.op.kind == LowOperation::LOP_SUB && is_zero(ins.second)) *result = ins.first;
  else if((ins.op.kind == LowOperation::LOP_MUL || ins.op.kind == LowOperation::LOP_DIV || ins.op.kind == LowOperation::LOP_UDIV) &&
          is_one(ins.second)) *result = ins.first;
  else if(ins.op.kind == LowOperation::LOP_MUL && is_one(ins.first)) *result = ins.second;
  else if(ins.op.kind == LowOperation::LOP_AND && is_minus_one(ins.second)) *result = ins.first;
  else if(ins.op.kind == LowOperation::LOP_AND && is_minus_one(ins.first)) *result = ins.second;
  else return false;
  return true;
}

}  // namespace lowir_opt
