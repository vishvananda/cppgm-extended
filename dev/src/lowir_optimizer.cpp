#include "lowir_optimizer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#include "lowir_internal.h"
#include "optimization_level.h"

namespace lir = lowir_internal;
namespace lowir = lowir_model;

namespace {

typedef unordered_map<string, lir::FunctionBoundaryMetadata> FunctionBoundaryMap;
typedef unordered_map<string, const lir::Function *> FunctionDefinitionMap;

size_t function_instruction_count(const lir::Function & function);

bool phase_timing_enabled()
{
  static const bool enabled = []()
  {
    const char * value = std::getenv("CPPGM_PHASE_TIMING");
    return value && *value && std::string(value) != "0";
  }();
  return enabled;
}

class PhaseTimer
{
public:
  PhaseTimer(const char * name, const string & detail = string())
    : name_(name),
      detail_(detail),
      enabled_(phase_timing_enabled()),
      start_(enabled_ ? Clock::now() : Clock::time_point())
  {
  }

  ~PhaseTimer()
  {
    if(!enabled_) {
      return;
    }
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_);
    std::cerr << "phase-timing"
              << " name=" << name_;
    if(!detail_.empty()) {
      std::cerr << " detail=" << detail_;
    }
    std::cerr << " ms=" << elapsed.count() << "\n";
  }

private:
  using Clock = std::chrono::steady_clock;

  const char * name_;
  string detail_;
  bool enabled_;
  Clock::time_point start_;
};

bool is_float_type(const string & type)
{
  return type == "f32" || type == "f64" || type == "f80";
}

bool is_generated_debug_value_temp(const string & name)
{
  string source_name;
  return lir::lowir_debug_value_source_name(name, source_name);
}

bool is_signed_integer_type(const string & type)
{
  return type == "i8" || type == "i16" || type == "i32" || type == "i64";
}

bool is_unsigned_integer_type(const string & type)
{
  return type == "u8" || type == "u16" || type == "u32";
}

bool is_foldable_integer_type(const string & type)
{
  return type == "i1" || is_signed_integer_type(type) ||
         is_unsigned_integer_type(type) || type == "ptr";
}

bool is_foldable_scalar_type(const string & type)
{
  return is_foldable_integer_type(type) || is_float_type(type);
}

bool is_slot_promotion_type(const string & type)
{
  return is_foldable_scalar_type(type);
}

bool operand_equal(const lir::Operand & lhs, const lir::Operand & rhs)
{
  return lhs.kind == rhs.kind &&
         lhs.text == rhs.text &&
         lhs.int_value == rhs.int_value &&
         lhs.float_value == rhs.float_value &&
         lhs.literal_type.text == rhs.literal_type.text;
}

void merge_boundary_metadata(lir::FunctionBoundaryMetadata & dst,
                             const lir::FunctionBoundaryMetadata & src)
{
  if(dst.arity == lir::CAM_FIXED &&
     src.arity != lir::CAM_FIXED) {
    dst.arity = src.arity;
  }
  if(dst.effects == lir::CFXM_DEFAULT &&
     src.effects != lir::CFXM_DEFAULT) {
    dst.effects = src.effects;
  }
  if(dst.unwind == lir::CUM_DEFAULT &&
     src.unwind != lir::CUM_DEFAULT) {
    dst.unwind = src.unwind;
  }
  if(dst.returns == lir::CRM_DEFAULT &&
     src.returns != lir::CRM_DEFAULT) {
    dst.returns = src.returns;
  }
}

size_t scalar_bit_width(const string & type)
{
  if(type == "i1") {
    return 1;
  }
  return lir::type_size(lir::LowType{type}) * 8;
}

uint64_t width_mask(size_t width)
{
  if(width == 0) {
    return 0;
  }
  if(width >= 64) {
    return numeric_limits<uint64_t>::max();
  }
  return (uint64_t(1) << width) - 1;
}

uint64_t normalize_bits(uint64_t bits, size_t width)
{
  return bits & width_mask(width);
}

long long sign_extend_bits(uint64_t bits, size_t width)
{
  bits = normalize_bits(bits, width);
  if(width == 0) {
    return 0;
  }
  if(width >= 64) {
    return static_cast<long long>(bits);
  }
  const uint64_t sign_bit = uint64_t(1) << (width - 1);
  if((bits & sign_bit) == 0) {
    return static_cast<long long>(bits);
  }
  return static_cast<long long>(bits | ~width_mask(width));
}

bool is_known_truth_value(const lir::Operand & operand, bool & truthy)
{
  if(operand.kind == lir::Operand::OP_INTEGER) {
    truthy = operand.int_value != 0;
    return true;
  }
  if(operand.kind == lir::Operand::OP_FLOAT) {
    truthy = operand.float_value != 0.0L;
    return true;
  }
  return false;
}

bool is_known_integer_value(const lir::Operand & operand, long long & value)
{
  if(operand.kind != lir::Operand::OP_INTEGER) {
    return false;
  }
  value = operand.int_value;
  return true;
}

lir::Operand make_label_operand(const string & label)
{
  lir::Operand operand;
  operand.kind = lir::Operand::OP_LABEL;
  operand.text = label;
  return operand;
}

lir::Operand make_temp_operand(const string & temp)
{
  lir::Operand operand;
  operand.kind = lir::Operand::OP_TEMP;
  operand.text = temp;
  return operand;
}

lir::Operand make_slot_operand(const string & slot)
{
  lir::Operand operand;
  operand.kind = lir::Operand::OP_SLOT;
  operand.text = slot;
  return operand;
}

lir::Operand make_integer_operand_for_type(const string & type, uint64_t bits)
{
  lir::Operand operand;
  operand.kind = lir::Operand::OP_INTEGER;
  const size_t width = scalar_bit_width(type);
  bits = normalize_bits(bits, width);
  if(is_signed_integer_type(type)) {
    operand.int_value = sign_extend_bits(bits, width);
  } else if(width < 64) {
    operand.int_value = static_cast<long long>(bits);
  } else {
    operand.int_value = static_cast<long long>(bits);
  }
  return operand;
}

lir::Operand make_float_operand_for_type(const string & type, long double value)
{
  lir::Operand operand;
  operand.kind = lir::Operand::OP_FLOAT;
  operand.literal_type.text = type;
  if(type == "f32") {
    operand.float_value = static_cast<float>(value);
  } else if(type == "f64") {
    operand.float_value = static_cast<double>(value);
  } else {
    operand.float_value = value;
  }
  return operand;
}

void copy_instruction_debug_location(lir::Instruction & instruction,
                                     const lir::Instruction * source)
{
  if(source != nullptr) {
    instruction.debug_location = source->debug_location;
  }
}

lir::Instruction make_const_instruction(const string & dest,
                                        const string & type,
                                        const lir::Operand & value,
                                        const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_CONST;
  instruction.dest = dest;
  instruction.type.text = type;
  instruction.first = value;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_copy_instruction(const string & dest,
                                       const string & type,
                                       const lir::Operand & value,
                                       const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_COPY;
  instruction.dest = dest;
  instruction.type.text = type;
  instruction.first = value;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_unary_instruction(const string & dest,
                                        const string & type,
                                        const string & op,
                                        const lir::Operand & value,
                                        const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_UNARY;
  instruction.dest = dest;
  instruction.type.text = type;
  instruction.op = op;
  instruction.first = value;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_jump_instruction(const string & target,
                                       const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_JUMP;
  instruction.first = make_label_operand(target);
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_load_instruction(const string & dest,
                                       const string & type,
                                       const lir::Operand & source_operand,
                                       const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_LOAD;
  instruction.dest = dest;
  instruction.type.text = type;
  instruction.first = source_operand;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_store_instruction(const string & type,
                                        const lir::Operand & value,
                                        const lir::Operand & dest_operand,
                                        const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_STORE;
  instruction.type.text = type;
  instruction.first = value;
  instruction.second = dest_operand;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

lir::Instruction make_copyobj_instruction(size_t byte_count,
                                          size_t byte_alignment,
                                          const lir::Operand & source_operand,
                                          const lir::Operand & dest_operand,
                                          const lir::Instruction * source = nullptr)
{
  lir::Instruction instruction;
  instruction.kind = lir::Instruction::IK_COPYOBJ;
  instruction.byte_count = byte_count;
  instruction.byte_alignment = byte_alignment;
  instruction.first = source_operand;
  instruction.second = dest_operand;
  copy_instruction_debug_location(instruction, source);
  return instruction;
}

bool try_get_integer_bits(const lir::Operand & operand,
                          const string & type,
                          uint64_t & bits)
{
  if(!is_foldable_integer_type(type) || operand.kind != lir::Operand::OP_INTEGER) {
    return false;
  }
  bits = normalize_bits(static_cast<uint64_t>(operand.int_value), scalar_bit_width(type));
  return true;
}

bool try_get_float_value(const lir::Operand & operand,
                         const string & type,
                         long double & value)
{
  if(!is_float_type(type)) {
    return false;
  }
  if(operand.kind == lir::Operand::OP_INTEGER) {
    value = static_cast<long double>(operand.int_value);
  } else if(operand.kind == lir::Operand::OP_FLOAT) {
    value = operand.float_value;
  } else {
    return false;
  }

  if(type == "f32") {
    value = static_cast<float>(value);
  } else if(type == "f64") {
    value = static_cast<double>(value);
  }
  return true;
}

bool try_make_typed_constant_operand(const string & type,
                                     const lir::Operand & source,
                                     lir::Operand & out)
{
  if(is_foldable_integer_type(type)) {
    uint64_t bits = 0;
    if(!try_get_integer_bits(source, type, bits)) {
      return false;
    }
    out = make_integer_operand_for_type(type, bits);
    return true;
  }
  if(is_float_type(type)) {
    long double value = 0.0L;
    if(!try_get_float_value(source, type, value)) {
      return false;
    }
    out = make_float_operand_for_type(type, value);
    return true;
  }
  return false;
}

bool operand_is_zero_for_type(const lir::Operand & operand, const string & type)
{
  if(is_foldable_integer_type(type)) {
    uint64_t bits = 0;
    return try_get_integer_bits(operand, type, bits) && bits == 0;
  }
  if(is_float_type(type)) {
    long double value = 0.0L;
    return try_get_float_value(operand, type, value) && value == 0.0L;
  }
  return false;
}

bool operand_is_one_for_type(const lir::Operand & operand, const string & type)
{
  if(is_foldable_integer_type(type)) {
    uint64_t bits = 0;
    return try_get_integer_bits(operand, type, bits) && bits == 1;
  }
  if(is_float_type(type)) {
    long double value = 0.0L;
    return try_get_float_value(operand, type, value) && value == 1.0L;
  }
  return false;
}

bool operand_is_all_ones_for_type(const lir::Operand & operand, const string & type)
{
  if(!is_foldable_integer_type(type)) {
    return false;
  }
  uint64_t bits = 0;
  return try_get_integer_bits(operand, type, bits) &&
         bits == width_mask(scalar_bit_width(type));
}

bool is_safe_shift_amount(const lir::Operand & operand, const string & type)
{
  if(operand.kind != lir::Operand::OP_INTEGER || operand.int_value < 0) {
    return false;
  }
  return static_cast<size_t>(operand.int_value) < scalar_bit_width(type);
}

bool try_fold_integer_unary(const lir::Instruction & instruction,
                            lir::Operand & value)
{
  uint64_t bits = 0;
  if(!try_get_integer_bits(instruction.first, instruction.type.text, bits)) {
    return false;
  }
  const size_t width = scalar_bit_width(instruction.type.text);
  if(instruction.op == "neg") {
    value = make_integer_operand_for_type(instruction.type.text,
                                          normalize_bits(~bits + 1, width));
    return true;
  }
  if(instruction.op == "bitnot") {
    value = make_integer_operand_for_type(instruction.type.text, ~bits);
    return true;
  }
  if(instruction.op == "not") {
    value = make_integer_operand_for_type(instruction.type.text, bits == 0 ? 1 : 0);
    return true;
  }
  return false;
}

bool try_fold_float_unary(const lir::Instruction & instruction,
                          lir::Operand & value)
{
  long double operand = 0.0L;
  if(!try_get_float_value(instruction.first, instruction.type.text, operand)) {
    return false;
  }
  if(std::isnan(operand)) {
    return false;
  }
  if(instruction.op == "neg") {
    value = make_float_operand_for_type(instruction.type.text, -operand);
    return true;
  }
  return false;
}

bool try_fold_integer_binary(const lir::Instruction & instruction,
                             lir::Operand & value)
{
  uint64_t lhs_bits = 0;
  uint64_t rhs_bits = 0;
  if(!try_get_integer_bits(instruction.first, instruction.type.text, lhs_bits) ||
     !try_get_integer_bits(instruction.second, instruction.type.text, rhs_bits)) {
    return false;
  }

  const string & type = instruction.type.text;
  const size_t width = scalar_bit_width(type);

  if(instruction.op == "add") {
    value = make_integer_operand_for_type(type, lhs_bits + rhs_bits);
    return true;
  }
  if(instruction.op == "sub") {
    value = make_integer_operand_for_type(type, lhs_bits - rhs_bits);
    return true;
  }
  if(instruction.op == "mul") {
    value = make_integer_operand_for_type(type, lhs_bits * rhs_bits);
    return true;
  }
  if(instruction.op == "and") {
    value = make_integer_operand_for_type(type, lhs_bits & rhs_bits);
    return true;
  }
  if(instruction.op == "or") {
    value = make_integer_operand_for_type(type, lhs_bits | rhs_bits);
    return true;
  }
  if(instruction.op == "xor") {
    value = make_integer_operand_for_type(type, lhs_bits ^ rhs_bits);
    return true;
  }
  if((instruction.op == "shl" || instruction.op == "shr" || instruction.op == "ushr") &&
     !is_safe_shift_amount(instruction.second, type)) {
    return false;
  }
  if(instruction.op == "shl") {
    value = make_integer_operand_for_type(
        type,
        normalize_bits(lhs_bits << static_cast<unsigned>(instruction.second.int_value), width));
    return true;
  }
  if(instruction.op == "ushr") {
    value = make_integer_operand_for_type(
        type,
        normalize_bits(lhs_bits >> static_cast<unsigned>(instruction.second.int_value), width));
    return true;
  }
  if(instruction.op == "shr") {
    const long long lhs = sign_extend_bits(lhs_bits, width);
    value = make_integer_operand_for_type(
        type,
        static_cast<uint64_t>(lhs >> static_cast<unsigned>(instruction.second.int_value)));
    return true;
  }

  if(instruction.op == "udiv" || instruction.op == "umod") {
    if(rhs_bits == 0) {
      return false;
    }
    if(instruction.op == "udiv") {
      value = make_integer_operand_for_type(type, lhs_bits / rhs_bits);
    } else {
      value = make_integer_operand_for_type(type, lhs_bits % rhs_bits);
    }
    return true;
  }

  if(instruction.op == "div" || instruction.op == "mod") {
    const long long lhs = sign_extend_bits(lhs_bits, width);
    const long long rhs = sign_extend_bits(rhs_bits, width);
    if(rhs == 0) {
      return false;
    }
    const long long min_value =
        width >= 64 ? numeric_limits<long long>::min() :
                      -static_cast<long long>(uint64_t(1) << (width - 1));
    if(lhs == min_value && rhs == -1) {
      return false;
    }
    if(instruction.op == "div") {
      value = make_integer_operand_for_type(type, static_cast<uint64_t>(lhs / rhs));
    } else {
      value = make_integer_operand_for_type(type, static_cast<uint64_t>(lhs % rhs));
    }
    return true;
  }

  return false;
}

bool try_fold_float_binary(const lir::Instruction & instruction,
                           lir::Operand & value)
{
  long double lhs = 0.0L;
  long double rhs = 0.0L;
  if(!try_get_float_value(instruction.first, instruction.type.text, lhs) ||
     !try_get_float_value(instruction.second, instruction.type.text, rhs)) {
    return false;
  }

  if(instruction.op == "add") {
    value = make_float_operand_for_type(instruction.type.text, lhs + rhs);
    return true;
  }
  if(instruction.op == "sub") {
    value = make_float_operand_for_type(instruction.type.text, lhs - rhs);
    return true;
  }
  if(instruction.op == "mul") {
    value = make_float_operand_for_type(instruction.type.text, lhs * rhs);
    return true;
  }
  if(instruction.op == "div") {
    if(rhs == 0.0L) {
      return false;
    }
    value = make_float_operand_for_type(instruction.type.text, lhs / rhs);
    return true;
  }
  return false;
}

bool try_fold_integer_cmp(const lir::Instruction & instruction,
                          lir::Operand & value)
{
  uint64_t lhs_bits = 0;
  uint64_t rhs_bits = 0;
  if(!try_get_integer_bits(instruction.first, instruction.type.text, lhs_bits) ||
     !try_get_integer_bits(instruction.second, instruction.type.text, rhs_bits)) {
    return false;
  }

  bool result = false;
  if(instruction.op == "eq") result = lhs_bits == rhs_bits;
  else if(instruction.op == "ne") result = lhs_bits != rhs_bits;
  else if(instruction.op == "ult") result = lhs_bits < rhs_bits;
  else if(instruction.op == "ugt") result = lhs_bits > rhs_bits;
  else if(instruction.op == "ule") result = lhs_bits <= rhs_bits;
  else if(instruction.op == "uge") result = lhs_bits >= rhs_bits;
  else {
    const long long lhs = sign_extend_bits(lhs_bits, scalar_bit_width(instruction.type.text));
    const long long rhs = sign_extend_bits(rhs_bits, scalar_bit_width(instruction.type.text));
    if(instruction.op == "lt") result = lhs < rhs;
    else if(instruction.op == "gt") result = lhs > rhs;
    else if(instruction.op == "le") result = lhs <= rhs;
    else if(instruction.op == "ge") result = lhs >= rhs;
    else return false;
  }

  value = make_integer_operand_for_type("i64", result ? 1 : 0);
  return true;
}

bool try_fold_float_cmp(const lir::Instruction & instruction,
                        lir::Operand & value)
{
  long double lhs = 0.0L;
  long double rhs = 0.0L;
  if(!try_get_float_value(instruction.first, instruction.type.text, lhs) ||
     !try_get_float_value(instruction.second, instruction.type.text, rhs)) {
    return false;
  }

  bool result = false;
  if(instruction.op == "eq") result = lhs == rhs;
  else if(instruction.op == "ne") result = lhs != rhs;
  else if(instruction.op == "lt") result = lhs < rhs;
  else if(instruction.op == "gt") result = lhs > rhs;
  else if(instruction.op == "le") result = lhs <= rhs;
  else if(instruction.op == "ge") result = lhs >= rhs;
  else return false;

  value = make_integer_operand_for_type("i64", result ? 1 : 0);
  return true;
}

bool try_fold_integer_convert(const lir::Instruction & instruction,
                              lir::Operand & value)
{
  uint64_t source_bits = 0;
  if(!try_get_integer_bits(instruction.first, instruction.source_type.text, source_bits)) {
    return false;
  }

  if(instruction.op == "sext") {
    const long long signed_source =
        sign_extend_bits(source_bits, scalar_bit_width(instruction.source_type.text));
    value = make_integer_operand_for_type(instruction.type.text,
                                          static_cast<uint64_t>(signed_source));
    return true;
  }
  if(instruction.op == "zext" || instruction.op == "trunc") {
    value = make_integer_operand_for_type(instruction.type.text, source_bits);
    return true;
  }
  return false;
}

bool try_fold_int_to_float_convert(const lir::Instruction & instruction,
                                   lir::Operand & value)
{
  if(!is_float_type(instruction.type.text) ||
     !is_foldable_integer_type(instruction.source_type.text)) {
    return false;
  }

  uint64_t bits = 0;
  if(!try_get_integer_bits(instruction.first, instruction.source_type.text, bits)) {
    return false;
  }

  long double numeric = 0.0L;
  if(instruction.op == "sitofp") {
    numeric = static_cast<long double>(
        sign_extend_bits(bits, scalar_bit_width(instruction.source_type.text)));
  } else if(instruction.op == "uitofp") {
    numeric = static_cast<long double>(bits);
  } else {
    return false;
  }

  value = make_float_operand_for_type(instruction.type.text, numeric);
  return true;
}

bool try_fold_float_to_int_convert(const lir::Instruction & instruction,
                                   lir::Operand & value)
{
  if(!is_float_type(instruction.source_type.text) ||
     !is_foldable_integer_type(instruction.type.text)) {
    return false;
  }

  long double numeric = 0.0L;
  if(!try_get_float_value(instruction.first, instruction.source_type.text, numeric) ||
     !std::isfinite(numeric)) {
    return false;
  }

  const long double truncated = std::trunc(numeric);
  const size_t width = scalar_bit_width(instruction.type.text);

  if(instruction.op == "fptosi") {
    long double min_value = 0.0L;
    long double max_value = 0.0L;
    if(is_signed_integer_type(instruction.type.text)) {
      if(width >= 64) {
        min_value = static_cast<long double>(numeric_limits<long long>::min());
        max_value = static_cast<long double>(numeric_limits<long long>::max());
      } else {
        min_value = -static_cast<long double>(uint64_t(1) << (width - 1));
        max_value = static_cast<long double>((uint64_t(1) << (width - 1)) - 1);
      }
    } else {
      min_value = 0.0L;
      max_value = static_cast<long double>(width_mask(width));
    }
    if(truncated < min_value || truncated > max_value) {
      return false;
    }
    value = make_integer_operand_for_type(instruction.type.text,
                                          static_cast<uint64_t>(
                                              static_cast<long long>(truncated)));
    return true;
  }

  if(instruction.op == "fptoui") {
    const long double max_value = static_cast<long double>(width_mask(width));
    if(truncated < 0.0L || truncated > max_value) {
      return false;
    }
    value = make_integer_operand_for_type(instruction.type.text,
                                          static_cast<uint64_t>(truncated));
    return true;
  }

  return false;
}

bool try_fold_float_convert(const lir::Instruction & instruction,
                            lir::Operand & value)
{
  if(!is_float_type(instruction.type.text) ||
     !is_float_type(instruction.source_type.text)) {
    return false;
  }
  long double numeric = 0.0L;
  if(!try_get_float_value(instruction.first, instruction.source_type.text, numeric)) {
    return false;
  }
  if(instruction.op != "fpext" && instruction.op != "fptrunc") {
    return false;
  }
  value = make_float_operand_for_type(instruction.type.text, numeric);
  return true;
}

bool simplify_copy_instruction(lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_COPY) {
    return false;
  }

  lir::Operand constant;
  if(try_make_typed_constant_operand(instruction.type.text, instruction.first, constant)) {
    instruction = make_const_instruction(instruction.dest, instruction.type.text, constant,
                                         &instruction);
    return true;
  }
  return false;
}

bool simplify_unary_instruction(lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_UNARY) {
    return false;
  }

  if(instruction.op == "decay" && instruction.type.text == "ptr") {
    instruction = make_copy_instruction(instruction.dest, instruction.type.text,
                                        instruction.first, &instruction);
    simplify_copy_instruction(instruction);
    return true;
  }

  lir::Operand folded;
  const bool folded_ok =
      is_float_type(instruction.type.text) ?
          try_fold_float_unary(instruction, folded) :
          try_fold_integer_unary(instruction, folded);
  if(!folded_ok) {
    return false;
  }

  instruction = make_const_instruction(instruction.dest, instruction.type.text, folded,
                                       &instruction);
  return true;
}

bool simplify_binary_instruction(lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_BINARY) {
    return false;
  }

  lir::Operand folded;
  const bool folded_ok =
      is_float_type(instruction.type.text) ?
          try_fold_float_binary(instruction, folded) :
          try_fold_integer_binary(instruction, folded);
  if(folded_ok) {
    instruction = make_const_instruction(instruction.dest, instruction.type.text, folded,
                                         &instruction);
    return true;
  }

  const string & type = instruction.type.text;
  if(instruction.op == "add") {
    if(operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_is_zero_for_type(instruction.first, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.second,
                                          &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
  } else if(instruction.op == "sub") {
    if(operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(!is_float_type(type) && operand_equal(instruction.first, instruction.second)) {
      instruction = make_const_instruction(instruction.dest,
                                           type,
                                           make_integer_operand_for_type(type, 0),
                                           &instruction);
      return true;
    }
  } else if(instruction.op == "mul") {
    if(operand_is_one_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_is_one_for_type(instruction.first, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.second,
                                          &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(!is_float_type(type) &&
       (operand_is_zero_for_type(instruction.first, type) ||
        operand_is_zero_for_type(instruction.second, type))) {
      instruction = make_const_instruction(instruction.dest,
                                           type,
                                           make_integer_operand_for_type(type, 0),
                                           &instruction);
      return true;
    }
  } else if(instruction.op == "and") {
    if(operand_is_zero_for_type(instruction.first, type) ||
       operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_const_instruction(instruction.dest,
                                           type,
                                           make_integer_operand_for_type(type, 0),
                                           &instruction);
      return true;
    }
    if(operand_is_all_ones_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_is_all_ones_for_type(instruction.first, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.second,
                                          &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_equal(instruction.first, instruction.second)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
  } else if(instruction.op == "or") {
    if(operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_is_zero_for_type(instruction.first, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.second,
                                          &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_equal(instruction.first, instruction.second)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
  } else if(instruction.op == "xor") {
    if(operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_is_zero_for_type(instruction.first, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.second,
                                          &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
    if(operand_equal(instruction.first, instruction.second)) {
      instruction = make_const_instruction(instruction.dest,
                                           type,
                                           make_integer_operand_for_type(type, 0),
                                           &instruction);
      return true;
    }
  } else if(instruction.op == "shl" ||
            instruction.op == "shr" ||
            instruction.op == "ushr") {
    if(operand_is_zero_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
  } else if(instruction.op == "div" || instruction.op == "udiv") {
    if(operand_is_one_for_type(instruction.second, type)) {
      instruction = make_copy_instruction(instruction.dest, type, instruction.first, &instruction);
      simplify_copy_instruction(instruction);
      return true;
    }
  } else if(instruction.op == "mod" || instruction.op == "umod") {
    if(operand_is_one_for_type(instruction.second, type)) {
      instruction = make_const_instruction(instruction.dest,
                                           type,
                                           make_integer_operand_for_type(type, 0),
                                           &instruction);
      return true;
    }
  }

  return false;
}

bool simplify_cmp_instruction(lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_CMP) {
    return false;
  }

  lir::Operand folded;
  const bool folded_ok =
      is_float_type(instruction.type.text) ?
          try_fold_float_cmp(instruction, folded) :
          try_fold_integer_cmp(instruction, folded);
  if(folded_ok) {
    instruction = make_const_instruction(instruction.dest, "i64", folded, &instruction);
    return true;
  }

  if(!is_float_type(instruction.type.text) &&
     operand_equal(instruction.first, instruction.second)) {
    bool truth = false;
    if(instruction.op == "eq" || instruction.op == "le" || instruction.op == "ge" ||
       instruction.op == "ule" || instruction.op == "uge") {
      truth = true;
    } else if(instruction.op == "ne" || instruction.op == "lt" || instruction.op == "gt" ||
              instruction.op == "ult" || instruction.op == "ugt") {
      truth = false;
    } else {
      return false;
    }
    instruction = make_const_instruction(
        instruction.dest, "i64", make_integer_operand_for_type("i64", truth ? 1 : 0),
        &instruction);
    return true;
  }

  return false;
}

bool simplify_convert_instruction(lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_CONVERT) {
    return false;
  }

  if(instruction.type.text == instruction.source_type.text) {
    instruction = make_copy_instruction(instruction.dest, instruction.type.text,
                                        instruction.first, &instruction);
    simplify_copy_instruction(instruction);
    return true;
  }

  lir::Operand folded;
  bool folded_ok = false;
  if(instruction.op == "sext" || instruction.op == "zext" || instruction.op == "trunc") {
    folded_ok = try_fold_integer_convert(instruction, folded);
  } else if(instruction.op == "sitofp" || instruction.op == "uitofp") {
    folded_ok = try_fold_int_to_float_convert(instruction, folded);
  } else if(instruction.op == "fptosi" || instruction.op == "fptoui") {
    folded_ok = try_fold_float_to_int_convert(instruction, folded);
  } else if(instruction.op == "fpext" || instruction.op == "fptrunc") {
    folded_ok = try_fold_float_convert(instruction, folded);
  }

  if(!folded_ok) {
    return false;
  }
  instruction = make_const_instruction(instruction.dest, instruction.type.text, folded,
                                       &instruction);
  return true;
}

bool simplify_instruction(lir::Instruction & instruction)
{
  if(simplify_copy_instruction(instruction)) {
    return true;
  }
  if(simplify_unary_instruction(instruction)) {
    return true;
  }
  if(simplify_binary_instruction(instruction)) {
    return true;
  }
  if(simplify_cmp_instruction(instruction)) {
    return true;
  }
  if(simplify_convert_instruction(instruction)) {
    return true;
  }
  return false;
}

typedef unordered_map<string, size_t> BlockIndexMap;

BlockIndexMap build_block_index_map(const lir::Function & function)
{
  BlockIndexMap block_index;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    block_index[function.blocks[i].label] = i;
  }
  return block_index;
}

string resolve_trivial_jump_target(const lir::Function & function,
                                   const BlockIndexMap & block_index,
                                   const string & label)
{
  string current = label;
  set<string> seen;
  while(seen.insert(current).second) {
    const BlockIndexMap::const_iterator found = block_index.find(current);
    if(found == block_index.end()) {
      return current;
    }
    const lir::Block & block = function.blocks[found->second];
    if(block.instructions.size() != 1) {
      return current;
    }
    const lir::Instruction & terminator = block.instructions[0];
    if(terminator.kind != lir::Instruction::IK_JUMP ||
       terminator.first.kind != lir::Operand::OP_LABEL ||
       terminator.first.text == current) {
      return current;
    }
    current = terminator.first.text;
  }
  return current;
}

bool rewrite_label_operand(const lir::Function & function,
                           const BlockIndexMap & block_index,
                           lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_LABEL) {
    return false;
  }
  const string resolved = resolve_trivial_jump_target(function, block_index, operand.text);
  if(resolved == operand.text) {
    return false;
  }
  operand.text = resolved;
  return true;
}

typedef vector<string> TempUseList;

TempUseList instruction_temp_uses(const lir::Instruction & instruction);
unordered_set<string> collect_executable_successor_labels(const lir::Instruction & instruction);
vector<string> collect_nonterminator_structural_successor_labels(const lir::Block & block);

typedef unordered_map<string, lir::Operand> ValueEnvironment;
typedef unordered_map<string, lir::InstructionDebugLocation> ValueDebugLocationEnvironment;
typedef unordered_set<string> BooleanTempSet;
struct FunctionOptimizationContext;
struct ReassociableIntegerProducer
{
  string op;
  string type;
  lir::Operand base_operand;
  uint64_t constant_bits = 0;
};
typedef unordered_map<string, ReassociableIntegerProducer> ProducerEnvironment;
struct CachedExpression
{
  string result_temp;
  TempUseList temp_uses;
  bool block_boundary_safe = false;
};
typedef unordered_map<string, CachedExpression> ExpressionCache;
void apply_branch_edge_assumptions(const string & assumed_temp,
                                   bool assumed_truth,
                                   ValueEnvironment & environment,
                                   ValueDebugLocationEnvironment & debug_locations,
                                   BooleanTempSet & boolean_temps,
                                   bool track_debug_locations = true);
struct ValueBlockAnalysis
{
  ValueEnvironment out_environment;
  ValueDebugLocationEnvironment out_debug_locations;
  BooleanTempSet out_boolean_temps;
  ExpressionCache out_expression_cache;
  unordered_set<string> executable_successors;
};

struct ValueDataflowState
{
  enum InputKind
  {
    DIK_UNREACHABLE,
    DIK_ENTRY,
    DIK_EXECUTABLE_EMPTY,
    DIK_SINGLE_PREDECESSOR,
    DIK_MERGED
  };

  vector<ValueEnvironment> in_environments;
  vector<ValueDebugLocationEnvironment> in_debug_locations;
  vector<BooleanTempSet> in_boolean_temps;
  vector<ExpressionCache> in_expression_caches;
  vector<ValueBlockAnalysis> analyses;
  vector<bool> executable;
  vector<InputKind> input_kinds;
  vector<size_t> input_single_predecessors;
  vector<size_t> input_single_predecessor_versions;
  vector<bool> input_has_branch_assumptions;
  vector<string> input_assumed_temps;
  vector<bool> input_assumed_truths;
};

struct IncomingValueState
{
  size_t predecessor = 0;
  const ValueEnvironment * environment = nullptr;
  const ValueDebugLocationEnvironment * debug_locations = nullptr;
  const BooleanTempSet * boolean_temps = nullptr;
  const ExpressionCache * expression_cache = nullptr;
  bool use_owned_value_state = false;
  ValueEnvironment owned_environment;
  ValueDebugLocationEnvironment owned_debug_locations;
  BooleanTempSet owned_boolean_temps;
  bool has_branch_assumption = false;
  string assumed_temp;
  bool assumed_truth = false;

  const ValueEnvironment & in_environment() const
  {
    return use_owned_value_state ? owned_environment : *environment;
  }

  const ValueDebugLocationEnvironment & in_debug_locations() const
  {
    return use_owned_value_state ? owned_debug_locations : *debug_locations;
  }

  const BooleanTempSet & in_boolean_temps() const
  {
    return use_owned_value_state ? owned_boolean_temps : *boolean_temps;
  }

  const ExpressionCache & in_expression_cache() const
  {
    return *expression_cache;
  }
};

struct ResolvedValueInputState
{
  const ValueEnvironment * environment = nullptr;
  const ValueDebugLocationEnvironment * debug_locations = nullptr;
  const BooleanTempSet * boolean_temps = nullptr;
  const ExpressionCache * expression_cache = nullptr;
  ValueEnvironment owned_environment;
  ValueDebugLocationEnvironment owned_debug_locations;
  BooleanTempSet owned_boolean_temps;

  const ValueEnvironment & in_environment() const
  {
    return *environment;
  }

  const ValueDebugLocationEnvironment & in_debug_locations() const
  {
    return *debug_locations;
  }

  const BooleanTempSet & in_boolean_temps() const
  {
    return *boolean_temps;
  }

  const ExpressionCache & in_expression_cache() const
  {
    return *expression_cache;
  }
};

ValueDataflowState compute_value_dataflow(const lir::Function & function,
                                         const FunctionOptimizationContext * context = nullptr);

bool value_environment_equal(const ValueEnvironment & lhs,
                             const ValueEnvironment & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(ValueEnvironment::const_iterator it = lhs.begin(); it != lhs.end(); ++it) {
    const ValueEnvironment::const_iterator found = rhs.find(it->first);
    if(found == rhs.end() || !operand_equal(it->second, found->second)) {
      return false;
    }
  }
  return true;
}

bool instruction_debug_location_equal(const lir::InstructionDebugLocation & lhs,
                                      const lir::InstructionDebugLocation & rhs)
{
  return lhs.file == rhs.file &&
         lhs.line == rhs.line &&
         lhs.column == rhs.column;
}

bool value_debug_location_environment_equal(const ValueDebugLocationEnvironment & lhs,
                                            const ValueDebugLocationEnvironment & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(ValueDebugLocationEnvironment::const_iterator it = lhs.begin(); it != lhs.end(); ++it) {
    const ValueDebugLocationEnvironment::const_iterator found = rhs.find(it->first);
    if(found == rhs.end() ||
       !instruction_debug_location_equal(it->second, found->second)) {
      return false;
    }
  }
  return true;
}

bool cached_expression_equal(const CachedExpression & lhs,
                             const CachedExpression & rhs)
{
  return lhs.result_temp == rhs.result_temp &&
         lhs.temp_uses == rhs.temp_uses &&
         lhs.block_boundary_safe == rhs.block_boundary_safe;
}

pair<ValueEnvironment, ValueDebugLocationEnvironment> meet_incoming_value_environments(
    const vector<IncomingValueState> & incoming,
    bool track_debug_locations = true)
{
  if(incoming.empty()) {
    return make_pair(ValueEnvironment(), ValueDebugLocationEnvironment());
  }

  size_t seed = 0;
  for(size_t i = 1; i < incoming.size(); ++i) {
    if(incoming[i].in_environment().size() < incoming[seed].in_environment().size()) {
      seed = i;
    }
  }

  const ValueEnvironment & seed_environment = incoming[seed].in_environment();
  ValueEnvironment result;
  ValueDebugLocationEnvironment result_debug_locations;
  result.reserve(seed_environment.size());
  for(ValueEnvironment::const_iterator it = seed_environment.begin();
      it != seed_environment.end();
      ++it) {
    bool keep = true;
    for(size_t i = 0; i < incoming.size(); ++i) {
      if(i == seed) {
        continue;
      }
      const ValueEnvironment::const_iterator found = incoming[i].in_environment().find(it->first);
      if(found == incoming[i].in_environment().end() ||
         !operand_equal(it->second, found->second)) {
        keep = false;
        break;
      }
    }
    if(keep) {
      result.insert(*it);
      if(track_debug_locations) {
        const ValueDebugLocationEnvironment::const_iterator debug_found =
            incoming[seed].in_debug_locations().find(it->first);
        if(debug_found != incoming[seed].in_debug_locations().end()) {
          bool keep_debug = true;
          for(size_t i = 0; i < incoming.size(); ++i) {
            if(i == seed) {
              continue;
            }
            const ValueDebugLocationEnvironment::const_iterator incoming_debug =
                incoming[i].in_debug_locations().find(it->first);
            if(incoming_debug == incoming[i].in_debug_locations().end() ||
               !instruction_debug_location_equal(debug_found->second,
                                                incoming_debug->second)) {
              keep_debug = false;
              break;
            }
          }
          if(keep_debug) {
            result_debug_locations.insert(*debug_found);
          }
        }
      }
    }
  }
  return make_pair(result, result_debug_locations);
}

ExpressionCache meet_incoming_expression_caches(const vector<IncomingValueState> & incoming)
{
  if(incoming.empty()) {
    return ExpressionCache();
  }

  size_t seed = 0;
  for(size_t i = 1; i < incoming.size(); ++i) {
    if(incoming[i].in_expression_cache().size() <
       incoming[seed].in_expression_cache().size()) {
      seed = i;
    }
  }

  const ExpressionCache & seed_cache = incoming[seed].in_expression_cache();
  ExpressionCache result;
  result.reserve(seed_cache.size());
  for(ExpressionCache::const_iterator it = seed_cache.begin();
      it != seed_cache.end();
      ++it) {
    if(!it->second.block_boundary_safe) {
      continue;
    }
    bool keep = true;
    for(size_t i = 0; i < incoming.size(); ++i) {
      if(i == seed) {
        continue;
      }
      const ExpressionCache::const_iterator found =
          incoming[i].in_expression_cache().find(it->first);
      if(found == incoming[i].in_expression_cache().end() ||
         !cached_expression_equal(it->second, found->second)) {
        keep = false;
        break;
      }
    }
    if(keep) {
      result.insert(*it);
    }
  }
  return result;
}

BooleanTempSet meet_incoming_boolean_temp_sets(const vector<IncomingValueState> & incoming)
{
  if(incoming.empty()) {
    return BooleanTempSet();
  }

  size_t seed = 0;
  for(size_t i = 1; i < incoming.size(); ++i) {
    if(incoming[i].in_boolean_temps().size() < incoming[seed].in_boolean_temps().size()) {
      seed = i;
    }
  }

  const BooleanTempSet & seed_boolean_temps = incoming[seed].in_boolean_temps();
  BooleanTempSet result;
  result.reserve(seed_boolean_temps.size());
  for(BooleanTempSet::const_iterator it = seed_boolean_temps.begin();
      it != seed_boolean_temps.end();
      ++it) {
    bool keep = true;
    for(size_t i = 0; i < incoming.size(); ++i) {
      if(i == seed) {
        continue;
      }
      if(incoming[i].in_boolean_temps().count(*it) == 0) {
        keep = false;
        break;
      }
    }
    if(keep) {
      result.insert(*it);
    }
  }
  return result;
}

bool expression_cache_equal(const ExpressionCache & lhs,
                            const ExpressionCache & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(ExpressionCache::const_iterator it = lhs.begin(); it != lhs.end(); ++it) {
    const ExpressionCache::const_iterator found = rhs.find(it->first);
    if(found == rhs.end() || !cached_expression_equal(it->second, found->second)) {
      return false;
    }
  }
  return true;
}

ResolvedValueInputState resolve_dataflow_input_state(const ValueDataflowState & dataflow,
                                                     size_t block_index,
                                                     bool track_debug_locations = true)
{
  ResolvedValueInputState input;
  static const ValueEnvironment empty_environment;
  static const ValueDebugLocationEnvironment empty_debug_locations;
  static const BooleanTempSet empty_boolean_temps;
  static const ExpressionCache empty_expression_cache;

  switch(dataflow.input_kinds[block_index]) {
    case ValueDataflowState::DIK_UNREACHABLE:
    case ValueDataflowState::DIK_ENTRY:
    case ValueDataflowState::DIK_EXECUTABLE_EMPTY:
      input.environment = &empty_environment;
      input.debug_locations = &empty_debug_locations;
      input.boolean_temps = &empty_boolean_temps;
      input.expression_cache = &empty_expression_cache;
      return input;

    case ValueDataflowState::DIK_MERGED:
      input.environment = &dataflow.in_environments[block_index];
      input.debug_locations = &dataflow.in_debug_locations[block_index];
      input.boolean_temps = &dataflow.in_boolean_temps[block_index];
      input.expression_cache = &dataflow.in_expression_caches[block_index];
      return input;

    case ValueDataflowState::DIK_SINGLE_PREDECESSOR: {
      const size_t predecessor = dataflow.input_single_predecessors[block_index];
      input.environment = &dataflow.analyses[predecessor].out_environment;
      input.debug_locations = &dataflow.analyses[predecessor].out_debug_locations;
      input.boolean_temps = &dataflow.analyses[predecessor].out_boolean_temps;
      input.expression_cache = &dataflow.analyses[predecessor].out_expression_cache;
      if(dataflow.input_has_branch_assumptions[block_index]) {
        input.owned_environment = *input.environment;
        if(track_debug_locations) {
          input.owned_debug_locations = *input.debug_locations;
        }
        input.owned_boolean_temps = *input.boolean_temps;
        apply_branch_edge_assumptions(dataflow.input_assumed_temps[block_index],
                                      dataflow.input_assumed_truths[block_index],
                                      input.owned_environment,
                                      input.owned_debug_locations,
                                      input.owned_boolean_temps,
                                      track_debug_locations);
        input.environment = &input.owned_environment;
        input.debug_locations =
            track_debug_locations ? &input.owned_debug_locations : &empty_debug_locations;
        input.boolean_temps = &input.owned_boolean_temps;
      }
      return input;
    }
  }

  input.environment = &empty_environment;
  input.debug_locations = &empty_debug_locations;
  input.boolean_temps = &empty_boolean_temps;
  input.expression_cache = &empty_expression_cache;
  return input;
}

bool value_block_analysis_equal(const ValueBlockAnalysis & lhs,
                                const ValueBlockAnalysis & rhs)
{
  return value_environment_equal(lhs.out_environment, rhs.out_environment) &&
         value_debug_location_environment_equal(lhs.out_debug_locations,
                                               rhs.out_debug_locations) &&
         lhs.out_boolean_temps == rhs.out_boolean_temps &&
         expression_cache_equal(lhs.out_expression_cache, rhs.out_expression_cache) &&
         lhs.executable_successors == rhs.executable_successors;
}

bool operand_has_stable_storage_kind(const lir::Operand & operand);

const lir::Operand * follow_environment_operand(const ValueEnvironment & environment,
                                                const lir::Operand & operand,
                                                bool require_stable_storage)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return nullptr;
  }
  const ValueEnvironment::const_iterator found = environment.find(operand.text);
  if(found == environment.end() ||
     (require_stable_storage && !operand_has_stable_storage_kind(found->second))) {
    return nullptr;
  }
  return &found->second;
}

const lir::Operand * resolve_rewritten_operand(const ValueEnvironment & environment,
                                               const lir::Operand & operand,
                                               bool require_stable_storage)
{
  if(operand.kind != lir::Operand::OP_TEMP || environment.empty()) {
    return &operand;
  }

  const lir::Operand * next =
      follow_environment_operand(environment, operand, require_stable_storage);
  if(next == nullptr) {
    return &operand;
  }

  const lir::Operand * rewritten = next;
  const string * inline_seen[8];
  size_t inline_seen_count = 1;
  inline_seen[0] = &operand.text;
  vector<const string *> overflow_seen;

  while(rewritten->kind == lir::Operand::OP_TEMP) {
    bool already_seen = false;
    for(size_t i = 0; i < inline_seen_count; ++i) {
      if(*inline_seen[i] == rewritten->text) {
        already_seen = true;
        break;
      }
    }
    if(!already_seen) {
      for(size_t i = 0; i < overflow_seen.size(); ++i) {
        if(*overflow_seen[i] == rewritten->text) {
          already_seen = true;
          break;
        }
      }
    }
    if(already_seen) {
      break;
    }
    if(inline_seen_count < sizeof(inline_seen) / sizeof(inline_seen[0])) {
      inline_seen[inline_seen_count++] = &rewritten->text;
    } else {
      overflow_seen.push_back(&rewritten->text);
    }

    const lir::Operand * next =
        follow_environment_operand(environment, *rewritten, require_stable_storage);
    if(next == nullptr) {
      break;
    }
    rewritten = next;
  }

  return rewritten;
}

bool rewrite_operand_from_environment(const ValueEnvironment & environment,
                                      const ValueDebugLocationEnvironment & debug_locations,
                                      lir::Operand & operand,
                                      lir::Instruction * instruction = nullptr,
                                      const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                      bool track_debug_locations = true)
{
  if(operand.kind != lir::Operand::OP_TEMP || environment.empty()) {
    return false;
  }

  const string * original_temp = instruction != nullptr ? &operand.text : nullptr;
  const lir::Operand * rewritten = resolve_rewritten_operand(environment, operand, false);

  if(rewritten == &operand) {
    return false;
  }
  if(operand_equal(operand, *rewritten)) {
    return false;
  }
  if(track_debug_locations && instruction != nullptr) {
    const ValueDebugLocationEnvironment::const_iterator debug_found =
        debug_locations.find(*original_temp);
    if(debug_found != debug_locations.end() &&
       (!instruction->debug_location.present() ||
        (fallback_debug_location != nullptr &&
         instruction_debug_location_equal(instruction->debug_location,
                                          *fallback_debug_location) &&
         !instruction_debug_location_equal(instruction->debug_location,
                                           debug_found->second)))) {
      instruction->debug_location = debug_found->second;
    }
  }
  operand = *rewritten;
  return true;
}

bool operand_has_stable_storage_kind(const lir::Operand & operand)
{
  return operand.kind == lir::Operand::OP_TEMP ||
         operand.kind == lir::Operand::OP_SLOT ||
         operand.kind == lir::Operand::OP_GLOBAL;
}

bool rewrite_storage_operand_from_environment(const ValueEnvironment & environment,
                                              const ValueDebugLocationEnvironment & debug_locations,
                                              lir::Operand & operand,
                                              lir::Instruction * instruction = nullptr,
                                              const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                              bool track_debug_locations = true)
{
  if(operand.kind != lir::Operand::OP_TEMP || environment.empty()) {
    return false;
  }

  const string * original_temp = instruction != nullptr ? &operand.text : nullptr;
  const lir::Operand * rewritten = resolve_rewritten_operand(environment, operand, true);

  if(rewritten == &operand) {
    return false;
  }
  if(operand_equal(operand, *rewritten)) {
    return false;
  }
  if(track_debug_locations && instruction != nullptr) {
    const ValueDebugLocationEnvironment::const_iterator debug_found =
        debug_locations.find(*original_temp);
    if(debug_found != debug_locations.end() &&
       (!instruction->debug_location.present() ||
        (fallback_debug_location != nullptr &&
         instruction_debug_location_equal(instruction->debug_location,
                                          *fallback_debug_location) &&
         !instruction_debug_location_equal(instruction->debug_location,
                                           debug_found->second)))) {
      instruction->debug_location = debug_found->second;
    }
  }
  operand = *rewritten;
  return true;
}

bool rewrite_operand_from_environment_no_debug(const ValueEnvironment & environment,
                                               lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP || environment.empty()) {
    return false;
  }

  const lir::Operand * rewritten =
      resolve_rewritten_operand(environment, operand, false);
  if(rewritten == &operand || operand_equal(operand, *rewritten)) {
    return false;
  }
  operand = *rewritten;
  return true;
}

bool rewrite_storage_operand_from_environment_no_debug(const ValueEnvironment & environment,
                                                       lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP || environment.empty()) {
    return false;
  }

  const lir::Operand * rewritten =
      resolve_rewritten_operand(environment, operand, true);
  if(rewritten == &operand || operand_equal(operand, *rewritten)) {
    return false;
  }
  operand = *rewritten;
  return true;
}

bool rewrite_instruction_operands(const ValueEnvironment & environment,
                                  const ValueDebugLocationEnvironment & debug_locations,
                                  lir::Instruction & instruction,
                                  const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                  bool track_debug_locations = true)
{
  bool changed = false;
  switch(instruction.kind) {
    case lir::Instruction::IK_STORE:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      changed |= rewrite_storage_operand_from_environment(environment,
                                                         debug_locations,
                                                         instruction.second,
                                                         &instruction,
                                                         fallback_debug_location,
                                                         track_debug_locations);
      break;
    case lir::Instruction::IK_ATOMIC_STORE:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.second,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_VA_START:
    case lir::Instruction::IK_VA_ARG:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_STACK_ALLOC:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_CALL:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        changed |= rewrite_storage_operand_from_environment(environment,
                                                           debug_locations,
                                                           instruction.args[i],
                                                           &instruction,
                                                           fallback_debug_location,
                                                           track_debug_locations);
      }
      break;
    case lir::Instruction::IK_COPYOBJ:
      changed |= rewrite_storage_operand_from_environment(environment,
                                                         debug_locations,
                                                         instruction.first,
                                                         &instruction,
                                                         fallback_debug_location,
                                                         track_debug_locations);
      changed |= rewrite_storage_operand_from_environment(environment,
                                                         debug_locations,
                                                         instruction.second,
                                                         &instruction,
                                                         fallback_debug_location,
                                                         track_debug_locations);
      break;
    case lir::Instruction::IK_ZEROINIT:
      changed |= rewrite_storage_operand_from_environment(environment,
                                                         debug_locations,
                                                         instruction.first,
                                                         &instruction,
                                                         fallback_debug_location,
                                                         track_debug_locations);
      break;
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_JUMP:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
      break;
    case lir::Instruction::IK_EH_FILTER:
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        changed |= rewrite_operand_from_environment(environment,
                                                   debug_locations,
                                                   instruction.args[i],
                                                   &instruction,
                                                   fallback_debug_location,
                                                   track_debug_locations);
      }
      break;
    case lir::Instruction::IK_BRANCH:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_SWITCH:
      changed |= rewrite_operand_from_environment(environment,
                                                 debug_locations,
                                                 instruction.first,
                                                 &instruction,
                                                 fallback_debug_location,
                                                 track_debug_locations);
      break;
    case lir::Instruction::IK_RETURN:
      if(instruction.type.text != "void") {
        changed |= rewrite_operand_from_environment(environment,
                                                   debug_locations,
                                                   instruction.first,
                                                   &instruction,
                                                   fallback_debug_location,
                                                   track_debug_locations);
      }
      break;
    case lir::Instruction::IK_CONST:
    case lir::Instruction::IK_COPY:
    case lir::Instruction::IK_LOAD:
    case lir::Instruction::IK_ATOMIC_LOAD:
    case lir::Instruction::IK_INDEX:
    case lir::Instruction::IK_UNARY:
    case lir::Instruction::IK_BINARY:
    case lir::Instruction::IK_CMP:
    case lir::Instruction::IK_CONVERT:
    case lir::Instruction::IK_ATOMIC_ADD_FETCH:
    case lir::Instruction::IK_ATOMIC_EXCHANGE:
    case lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE:
      break;
    default:
      break;
  }

  if(instruction.kind == lir::Instruction::IK_CONST ||
     instruction.kind == lir::Instruction::IK_COPY ||
     instruction.kind == lir::Instruction::IK_ATOMIC_LOAD ||
     instruction.kind == lir::Instruction::IK_UNARY ||
     instruction.kind == lir::Instruction::IK_CONVERT) {
    changed |= rewrite_operand_from_environment(environment,
                                               debug_locations,
                                               instruction.first,
                                               &instruction,
                                               fallback_debug_location,
                                               track_debug_locations);
  }

  if(instruction.kind == lir::Instruction::IK_LOAD) {
    changed |= rewrite_storage_operand_from_environment(environment,
                                                       debug_locations,
                                                       instruction.first,
                                                       &instruction,
                                                       fallback_debug_location,
                                                       track_debug_locations);
  }

  if(instruction.kind == lir::Instruction::IK_INDEX ||
     instruction.kind == lir::Instruction::IK_BINARY ||
     instruction.kind == lir::Instruction::IK_CMP ||
     instruction.kind == lir::Instruction::IK_ATOMIC_ADD_FETCH ||
     instruction.kind == lir::Instruction::IK_ATOMIC_EXCHANGE ||
     instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    changed |= rewrite_operand_from_environment(environment,
                                               debug_locations,
                                               instruction.first,
                                               &instruction,
                                               fallback_debug_location,
                                               track_debug_locations);
    changed |= rewrite_operand_from_environment(environment,
                                               debug_locations,
                                               instruction.second,
                                               &instruction,
                                               fallback_debug_location,
                                               track_debug_locations);
  }

  if(instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    changed |= rewrite_operand_from_environment(environment,
                                               debug_locations,
                                               instruction.third,
                                               &instruction,
                                               fallback_debug_location,
                                               track_debug_locations);
  }

  return changed;
}

bool rewrite_instruction_operands_no_debug(const ValueEnvironment & environment,
                                           lir::Instruction & instruction)
{
  bool changed = false;
  switch(instruction.kind) {
    case lir::Instruction::IK_STORE:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      changed |= rewrite_storage_operand_from_environment_no_debug(environment, instruction.second);
      break;
    case lir::Instruction::IK_ATOMIC_STORE:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.second);
      break;
    case lir::Instruction::IK_VA_START:
    case lir::Instruction::IK_VA_ARG:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      break;
    case lir::Instruction::IK_STACK_ALLOC:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      break;
    case lir::Instruction::IK_CALL:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        changed |= rewrite_storage_operand_from_environment_no_debug(environment,
                                                                     instruction.args[i]);
      }
      break;
    case lir::Instruction::IK_COPYOBJ:
      changed |= rewrite_storage_operand_from_environment_no_debug(environment, instruction.first);
      changed |= rewrite_storage_operand_from_environment_no_debug(environment, instruction.second);
      break;
    case lir::Instruction::IK_ZEROINIT:
      changed |= rewrite_storage_operand_from_environment_no_debug(environment, instruction.first);
      break;
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_JUMP:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      break;
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
      break;
    case lir::Instruction::IK_EH_FILTER:
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        changed |= rewrite_operand_from_environment_no_debug(environment, instruction.args[i]);
      }
      break;
    case lir::Instruction::IK_BRANCH:
    case lir::Instruction::IK_SWITCH:
      changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      break;
    case lir::Instruction::IK_RETURN:
      if(instruction.type.text != "void") {
        changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
      }
      break;
    default:
      break;
  }

  if(instruction.kind == lir::Instruction::IK_CONST ||
     instruction.kind == lir::Instruction::IK_COPY ||
     instruction.kind == lir::Instruction::IK_ATOMIC_LOAD ||
     instruction.kind == lir::Instruction::IK_UNARY ||
     instruction.kind == lir::Instruction::IK_CONVERT) {
    changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
  }

  if(instruction.kind == lir::Instruction::IK_LOAD) {
    changed |= rewrite_storage_operand_from_environment_no_debug(environment, instruction.first);
  }

  if(instruction.kind == lir::Instruction::IK_INDEX ||
     instruction.kind == lir::Instruction::IK_BINARY ||
     instruction.kind == lir::Instruction::IK_CMP ||
     instruction.kind == lir::Instruction::IK_ATOMIC_ADD_FETCH ||
     instruction.kind == lir::Instruction::IK_ATOMIC_EXCHANGE ||
     instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    changed |= rewrite_operand_from_environment_no_debug(environment, instruction.first);
    changed |= rewrite_operand_from_environment_no_debug(environment, instruction.second);
  }

  if(instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    changed |= rewrite_operand_from_environment_no_debug(environment, instruction.third);
  }

  return changed;
}

bool simplify_boolean_compare_instruction(const BooleanTempSet & boolean_temps,
                                          lir::Instruction & instruction);
bool instruction_is_local_cse_candidate(const lir::Instruction & instruction);
string instruction_expression_key(const lir::Instruction & instruction);
void invalidate_expression_cache(ExpressionCache & cache, const string & temp);
bool instruction_produces_i64_boolean_value(const BooleanTempSet & boolean_temps,
                                            const lir::Instruction & instruction);
bool reassociate_integer_binary_instruction(const ProducerEnvironment & producers,
                                            lir::Instruction & instruction);
bool match_reassociable_binary_with_constant(const lir::Instruction & instruction,
                                             lir::Operand & base_operand,
                                             uint64_t & constant_bits);

bool process_instruction_no_debug(ValueEnvironment & environment,
                                  BooleanTempSet & boolean_temps,
                                  ProducerEnvironment & producers,
                                  ExpressionCache & expression_cache,
                                  lir::Instruction & instruction,
                                  bool track_expression_cache = true,
                                  bool track_reassociation = true)
{
  bool changed = false;
  changed |= rewrite_instruction_operands_no_debug(environment, instruction);
  changed |= simplify_boolean_compare_instruction(boolean_temps, instruction);
  changed |= simplify_instruction(instruction);
  if(track_reassociation &&
     reassociate_integer_binary_instruction(producers, instruction)) {
    changed = true;
    changed |= simplify_instruction(instruction);
  }

  string expression_key;
  bool have_expression_key = false;
  const auto ensure_expression_key = [&]() -> const string & {
    if(!have_expression_key) {
      expression_key = instruction_expression_key(instruction);
      have_expression_key = true;
    }
    return expression_key;
  };

  if(track_expression_cache && instruction_is_local_cse_candidate(instruction)) {
    const ExpressionCache::const_iterator found =
        expression_cache.find(ensure_expression_key());
    if(found != expression_cache.end()) {
      instruction = make_copy_instruction(instruction.dest,
                                          instruction.type.text,
                                          make_temp_operand(found->second.result_temp),
                                          &instruction);
      simplify_copy_instruction(instruction);
      changed = true;
    }
  }

  if(!instruction.dest.empty()) {
    environment.erase(instruction.dest);
    boolean_temps.erase(instruction.dest);
    if(track_reassociation) {
      producers.erase(instruction.dest);
    }
    if(track_expression_cache) {
      invalidate_expression_cache(expression_cache, instruction.dest);
    }
    if((instruction.kind == lir::Instruction::IK_CONST ||
        instruction.kind == lir::Instruction::IK_COPY) &&
       !is_generated_debug_value_temp(instruction.dest)) {
      environment[instruction.dest] = instruction.first;
    }
    if(instruction_produces_i64_boolean_value(boolean_temps, instruction)) {
      boolean_temps.insert(instruction.dest);
    }
    if(track_reassociation &&
       instruction.kind == lir::Instruction::IK_BINARY) {
      ReassociableIntegerProducer producer;
      if(match_reassociable_binary_with_constant(instruction,
                                                 producer.base_operand,
                                                 producer.constant_bits)) {
        producer.op = instruction.op;
        producer.type = instruction.type.text;
        producers[instruction.dest] = std::move(producer);
      }
    }
  }
  if(track_expression_cache && instruction_is_local_cse_candidate(instruction)) {
    CachedExpression cached;
    cached.result_temp = instruction.dest;
    cached.temp_uses = instruction_temp_uses(instruction);
    cached.block_boundary_safe = true;
    expression_cache[have_expression_key ? expression_key : instruction_expression_key(instruction)] =
        std::move(cached);
  }
  return changed;
}

bool operand_is_i64_boolean_constant(const lir::Operand & operand, bool & value)
{
  if(operand.kind != lir::Operand::OP_INTEGER) {
    return false;
  }
  if(operand.int_value == 0) {
    value = false;
    return true;
  }
  if(operand.int_value == 1) {
    value = true;
    return true;
  }
  return false;
}

bool operand_is_known_i64_boolean(const BooleanTempSet & boolean_temps,
                                  const lir::Operand & operand)
{
  bool value = false;
  return operand_is_i64_boolean_constant(operand, value) ||
         (operand.kind == lir::Operand::OP_TEMP &&
          boolean_temps.count(operand.text) != 0);
}

bool simplify_boolean_compare_instruction(const BooleanTempSet & boolean_temps,
                                          lir::Instruction & instruction)
{
  if(instruction.kind != lir::Instruction::IK_CMP ||
     instruction.type.text != "i64" ||
     (instruction.op != "eq" && instruction.op != "ne")) {
    return false;
  }

  const lir::Operand * boolean_operand = NULL;
  bool constant_value = false;
  if(operand_is_known_i64_boolean(boolean_temps, instruction.first) &&
     operand_is_i64_boolean_constant(instruction.second, constant_value)) {
    boolean_operand = &instruction.first;
  } else if(operand_is_known_i64_boolean(boolean_temps, instruction.second) &&
            operand_is_i64_boolean_constant(instruction.first, constant_value)) {
    boolean_operand = &instruction.second;
  } else {
    return false;
  }

  if((instruction.op == "ne" && !constant_value) ||
     (instruction.op == "eq" && constant_value)) {
    instruction = make_copy_instruction(instruction.dest, "i64", *boolean_operand, &instruction);
    simplify_copy_instruction(instruction);
    return true;
  }

  instruction = make_unary_instruction(instruction.dest, "i64", "not", *boolean_operand,
                                       &instruction);
  simplify_unary_instruction(instruction);
  return true;
}

bool instruction_is_local_cse_candidate(const lir::Instruction & instruction)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_ADDR:
    case lir::Instruction::IK_INDEX:
    case lir::Instruction::IK_UNARY:
    case lir::Instruction::IK_BINARY:
    case lir::Instruction::IK_CMP:
    case lir::Instruction::IK_CONVERT:
      return !instruction.dest.empty();
    default:
      return false;
  }
}

size_t count_block_local_cse_candidates(const lir::Block & block)
{
  size_t count = 0;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    if(instruction_is_local_cse_candidate(block.instructions[i])) {
      ++count;
    }
  }
  return count;
}

bool is_reassociable_integer_type(const string & type)
{
  return is_signed_integer_type(type) ||
         is_unsigned_integer_type(type) ||
         type == "i1";
}

bool instruction_is_reassociable_integer_binary(const lir::Instruction & instruction)
{
  return instruction.kind == lir::Instruction::IK_BINARY &&
         is_reassociable_integer_type(instruction.type.text) &&
         (instruction.op == "add" ||
          instruction.op == "mul" ||
          instruction.op == "and" ||
          instruction.op == "or" ||
          instruction.op == "xor");
}

bool match_reassociable_binary_with_constant(const lir::Instruction & instruction,
                                             lir::Operand & base_operand,
                                             uint64_t & constant_bits)
{
  if(!instruction_is_reassociable_integer_binary(instruction)) {
    return false;
  }

  uint64_t first_bits = 0;
  uint64_t second_bits = 0;
  const bool first_constant = try_get_integer_bits(instruction.first, instruction.type.text, first_bits);
  const bool second_constant = try_get_integer_bits(instruction.second, instruction.type.text, second_bits);
  if(first_constant == second_constant) {
    return false;
  }

  if(first_constant) {
    constant_bits = first_bits;
    base_operand = instruction.second;
  } else {
    constant_bits = second_bits;
    base_operand = instruction.first;
  }
  return true;
}

uint64_t combine_reassociated_integer_constants(const string & op,
                                                const string & type,
                                                uint64_t lhs_bits,
                                                uint64_t rhs_bits)
{
  const size_t width = scalar_bit_width(type);
  if(op == "add") {
    return normalize_bits(lhs_bits + rhs_bits, width);
  }
  if(op == "mul") {
    return normalize_bits(lhs_bits * rhs_bits, width);
  }
  if(op == "and") {
    return normalize_bits(lhs_bits & rhs_bits, width);
  }
  if(op == "or") {
    return normalize_bits(lhs_bits | rhs_bits, width);
  }
  return normalize_bits(lhs_bits ^ rhs_bits, width);
}

bool reassociate_integer_binary_instruction(const ProducerEnvironment & producers,
                                            lir::Instruction & instruction)
{
  lir::Operand outer_base;
  uint64_t outer_constant = 0;
  if(!match_reassociable_binary_with_constant(instruction, outer_base, outer_constant) ||
     outer_base.kind != lir::Operand::OP_TEMP) {
    return false;
  }

  const ProducerEnvironment::const_iterator found = producers.find(outer_base.text);
  if(found == producers.end()) {
    return false;
  }

  if(found->second.op != instruction.op ||
     found->second.type != instruction.type.text) {
    return false;
  }

  instruction.first = found->second.base_operand;
  instruction.second = make_integer_operand_for_type(
      instruction.type.text,
      combine_reassociated_integer_constants(
          instruction.op,
          instruction.type.text,
          found->second.constant_bits,
          outer_constant));
  return true;
}

void append_decimal_unsigned(string & out, unsigned long long value)
{
  char buffer[32];
  char * it = buffer + sizeof(buffer);
  do {
    *--it = static_cast<char>('0' + (value % 10));
    value /= 10;
  } while(value != 0);
  out.append(it, static_cast<size_t>(buffer + sizeof(buffer) - it));
}

void append_decimal_signed(string & out, long long value)
{
  if(value < 0) {
    out.push_back('-');
    append_decimal_unsigned(out, static_cast<unsigned long long>(-(value + 1)) + 1);
    return;
  }
  append_decimal_unsigned(out, static_cast<unsigned long long>(value));
}

void append_key_text(string & out, const string & value)
{
  append_decimal_unsigned(out, static_cast<unsigned long long>(value.size()));
  out.push_back(':');
  out.append(value);
  out.push_back(';');
}

void append_key_number(string & out, long long value)
{
  append_decimal_signed(out, value);
  out.push_back(';');
}

void append_key_size(string & out, size_t value)
{
  append_decimal_unsigned(out, static_cast<unsigned long long>(value));
  out.push_back(';');
}

long double normalize_float_key_value(long double value)
{
  return value == 0.0L ? 0.0L : value;
}

void append_key_float(string & out, long double value, const string & type)
{
  value = normalize_float_key_value(value);
  if(std::isnan(value)) {
    out.append("nan;");
    return;
  }

  if(type == "f32") {
    const float narrowed = static_cast<float>(value);
    uint32_t bits = 0;
    std::memcpy(&bits, &narrowed, sizeof(bits));
    append_decimal_unsigned(out, static_cast<unsigned long long>(bits));
    out.push_back(';');
    return;
  }

  if(type == "f64") {
    const double narrowed = static_cast<double>(value);
    uint64_t bits = 0;
    std::memcpy(&bits, &narrowed, sizeof(bits));
    append_decimal_unsigned(out, static_cast<unsigned long long>(bits));
    out.push_back(';');
    return;
  }

  append_key_text(out, to_string(value));
}

int compare_float_key_value(long double lhs, long double rhs)
{
  lhs = normalize_float_key_value(lhs);
  rhs = normalize_float_key_value(rhs);
  const bool lhs_nan = std::isnan(lhs);
  const bool rhs_nan = std::isnan(rhs);
  if(lhs_nan || rhs_nan) {
    if(lhs_nan == rhs_nan) {
      return 0;
    }
    return lhs_nan ? -1 : 1;
  }
  if(lhs < rhs) {
    return -1;
  }
  if(lhs > rhs) {
    return 1;
  }
  return 0;
}

int compare_operand_for_key(const lir::Operand & lhs, const lir::Operand & rhs)
{
  if(lhs.kind != rhs.kind) {
    return lhs.kind < rhs.kind ? -1 : 1;
  }
  if(lhs.text != rhs.text) {
    return lhs.text < rhs.text ? -1 : 1;
  }
  if(lhs.int_value != rhs.int_value) {
    return lhs.int_value < rhs.int_value ? -1 : 1;
  }
  const int float_cmp = compare_float_key_value(lhs.float_value, rhs.float_value);
  if(float_cmp != 0) {
    return float_cmp;
  }
  if(lhs.literal_type.text != rhs.literal_type.text) {
    return lhs.literal_type.text < rhs.literal_type.text ? -1 : 1;
  }
  return 0;
}

size_t operand_key_reserve_hint(const lir::Operand & operand)
{
  size_t hint = 32 + operand.text.size() + operand.literal_type.text.size();
  if(operand.kind == lir::Operand::OP_FLOAT) {
    hint += 16;
  }
  return hint;
}

void append_key_operand(string & out, const lir::Operand & operand)
{
  append_key_number(out, static_cast<long long>(operand.kind));
  switch(operand.kind) {
    case lir::Operand::OP_TEMP:
    case lir::Operand::OP_SLOT:
    case lir::Operand::OP_GLOBAL:
    case lir::Operand::OP_LABEL:
      append_key_text(out, operand.text);
      break;
    case lir::Operand::OP_INTEGER:
      append_key_number(out, operand.int_value);
      break;
    case lir::Operand::OP_FLOAT:
      append_key_float(out, operand.float_value, operand.literal_type.text);
      append_key_text(out, operand.literal_type.text);
      break;
  }
}

bool instruction_has_commutative_expression_key(const lir::Instruction & instruction)
{
  if(instruction.kind == lir::Instruction::IK_BINARY) {
    if(!is_signed_integer_type(instruction.type.text) &&
       !is_unsigned_integer_type(instruction.type.text) &&
       instruction.type.text != "i1") {
      return false;
    }
    return instruction.op == "add" ||
           instruction.op == "mul" ||
           instruction.op == "and" ||
           instruction.op == "or" ||
           instruction.op == "xor";
  }

  return instruction.kind == lir::Instruction::IK_CMP &&
         (instruction.op == "eq" || instruction.op == "ne");
}

string canonical_cmp_expression_op(const string & op, bool & swap_operands)
{
  swap_operands = false;
  if(op == "gt") {
    swap_operands = true;
    return "lt";
  }
  if(op == "ge") {
    swap_operands = true;
    return "le";
  }
  if(op == "ugt") {
    swap_operands = true;
    return "ult";
  }
  if(op == "uge") {
    swap_operands = true;
    return "ule";
  }
  return op;
}

string instruction_expression_key(const lir::Instruction & instruction)
{
  const lir::Operand * first = &instruction.first;
  const lir::Operand * second = &instruction.second;
  const string * op = &instruction.op;
  string canonical_op;

  if(instruction.kind == lir::Instruction::IK_CMP) {
    bool swap_operands = false;
    canonical_op = canonical_cmp_expression_op(*op, swap_operands);
    op = &canonical_op;
    if(swap_operands) {
      std::swap(first, second);
    }
  }

  string key;
  key.reserve(96 + instruction.type.text.size() + instruction.source_type.text.size() +
              op->size() + operand_key_reserve_hint(*first) +
              operand_key_reserve_hint(*second) +
              operand_key_reserve_hint(instruction.third) +
              instruction.args.size() * 24);
  append_key_number(key, static_cast<long long>(instruction.kind));
  append_key_text(key, instruction.type.text);
  append_key_text(key, instruction.source_type.text);
  append_key_text(key, *op);
  append_key_size(key, instruction.byte_count);
  append_key_size(key, instruction.byte_alignment);
  append_key_number(key, static_cast<long long>(instruction.index_projection));
  if(instruction_has_commutative_expression_key(instruction) &&
     compare_operand_for_key(*second, *first) < 0) {
    append_key_operand(key, *second);
    append_key_operand(key, *first);
  } else {
    append_key_operand(key, *first);
    append_key_operand(key, *second);
  }
  append_key_operand(key, instruction.third);
  append_key_size(key, instruction.args.size());
  for(size_t i = 0; i < instruction.args.size(); ++i) {
    append_key_operand(key, instruction.args[i]);
  }
  return key;
}

void invalidate_expression_cache(ExpressionCache & cache, const string & temp)
{
  for(ExpressionCache::iterator it = cache.begin(); it != cache.end();) {
    if(it->second.result_temp == temp ||
       find(it->second.temp_uses.begin(), it->second.temp_uses.end(), temp) !=
           it->second.temp_uses.end()) {
      it = cache.erase(it);
    } else {
      ++it;
    }
  }
}

bool instruction_produces_i64_boolean_value(const BooleanTempSet & boolean_temps,
                                            const lir::Instruction & instruction)
{
  if(instruction.kind == lir::Instruction::IK_CMP) {
    return true;
  }
  if(instruction.kind == lir::Instruction::IK_CONST &&
     instruction.type.text == "i64") {
    bool value = false;
    return operand_is_i64_boolean_constant(instruction.first, value);
  }
  if(instruction.kind == lir::Instruction::IK_COPY &&
     instruction.type.text == "i64") {
    return operand_is_known_i64_boolean(boolean_temps, instruction.first);
  }
  if(instruction.kind == lir::Instruction::IK_UNARY &&
     instruction.type.text == "i64" &&
     instruction.op == "not") {
    return true;
  }
  if(instruction.kind == lir::Instruction::IK_BINARY &&
     instruction.type.text == "i64" &&
     (instruction.op == "and" ||
      instruction.op == "or" ||
      instruction.op == "xor")) {
    return operand_is_known_i64_boolean(boolean_temps, instruction.first) &&
           operand_is_known_i64_boolean(boolean_temps, instruction.second);
  }
  return false;
}

bool process_instruction(ValueEnvironment & environment,
                         ValueDebugLocationEnvironment & debug_locations,
                         BooleanTempSet & boolean_temps,
                         ProducerEnvironment & producers,
                         ExpressionCache & expression_cache,
                         lir::Instruction & instruction,
                         const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                         bool track_expression_cache = true,
                         bool track_reassociation = true,
                         bool track_debug_locations = true)
{
  if(!track_debug_locations) {
    return process_instruction_no_debug(environment,
                                        boolean_temps,
                                        producers,
                                        expression_cache,
                                        instruction,
                                        track_expression_cache,
                                        track_reassociation);
  }

  bool changed = false;
  changed |= rewrite_instruction_operands(environment,
                                          debug_locations,
                                          instruction,
                                          fallback_debug_location,
                                          track_debug_locations);
  changed |= simplify_boolean_compare_instruction(boolean_temps, instruction);
  changed |= simplify_instruction(instruction);
  if(track_reassociation &&
     reassociate_integer_binary_instruction(producers, instruction)) {
    changed = true;
    changed |= simplify_instruction(instruction);
  }

  string expression_key;
  bool have_expression_key = false;
  const auto ensure_expression_key = [&]() -> const string & {
    if(!have_expression_key) {
      expression_key = instruction_expression_key(instruction);
      have_expression_key = true;
    }
    return expression_key;
  };

  if(track_expression_cache && instruction_is_local_cse_candidate(instruction)) {
    const ExpressionCache::const_iterator found =
        expression_cache.find(ensure_expression_key());
    if(found != expression_cache.end()) {
      instruction = make_copy_instruction(instruction.dest,
                                          instruction.type.text,
                                          make_temp_operand(found->second.result_temp),
                                          &instruction);
      simplify_copy_instruction(instruction);
      changed = true;
    }
  }

  if(!instruction.dest.empty()) {
    environment.erase(instruction.dest);
    if(track_debug_locations) {
      debug_locations.erase(instruction.dest);
    }
    boolean_temps.erase(instruction.dest);
    if(track_reassociation) {
      producers.erase(instruction.dest);
    }
    if(track_expression_cache) {
      invalidate_expression_cache(expression_cache, instruction.dest);
    }
    if((instruction.kind == lir::Instruction::IK_CONST ||
        instruction.kind == lir::Instruction::IK_COPY) &&
       !is_generated_debug_value_temp(instruction.dest)) {
      environment[instruction.dest] = instruction.first;
      if(track_debug_locations && instruction.debug_location.present()) {
        debug_locations[instruction.dest] = instruction.debug_location;
      }
    }
    if(instruction_produces_i64_boolean_value(boolean_temps, instruction)) {
      boolean_temps.insert(instruction.dest);
    }
    if(track_reassociation &&
       instruction.kind == lir::Instruction::IK_BINARY) {
      ReassociableIntegerProducer producer;
      if(match_reassociable_binary_with_constant(instruction,
                                                 producer.base_operand,
                                                 producer.constant_bits)) {
        producer.op = instruction.op;
        producer.type = instruction.type.text;
        producers[instruction.dest] = std::move(producer);
      }
    }
  }
  if(track_expression_cache && instruction_is_local_cse_candidate(instruction)) {
    CachedExpression cached;
    cached.result_temp = instruction.dest;
    cached.temp_uses = instruction_temp_uses(instruction);
    cached.block_boundary_safe = true;
    expression_cache[have_expression_key ? expression_key : instruction_expression_key(instruction)] =
        std::move(cached);
  }
  return changed;
}

bool try_get_branch_edge_assumption(const lir::Block & predecessor_block,
                                    const string & successor_label,
                                    const ValueEnvironment & environment,
                                    const ValueDebugLocationEnvironment & debug_locations,
                                    const BooleanTempSet & boolean_temps,
                                    string & assumed_temp,
                                    bool & assumed_truth,
                                    const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                    bool track_debug_locations = true)
{
  if(predecessor_block.instructions.empty()) {
    return false;
  }

  lir::Instruction terminator = predecessor_block.instructions.back();
  rewrite_instruction_operands(environment,
                               debug_locations,
                               terminator,
                               fallback_debug_location,
                               track_debug_locations);
  simplify_boolean_compare_instruction(boolean_temps, terminator);
  simplify_instruction(terminator);
  if(terminator.kind != lir::Instruction::IK_BRANCH ||
     terminator.first.kind != lir::Operand::OP_TEMP ||
     terminator.second.kind != lir::Operand::OP_LABEL ||
     terminator.third.kind != lir::Operand::OP_LABEL ||
     terminator.second.text == terminator.third.text) {
    return false;
  }
  if(!operand_is_known_i64_boolean(boolean_temps, terminator.first)) {
    return false;
  }

  if(successor_label == terminator.second.text) {
    assumed_truth = true;
  } else if(successor_label == terminator.third.text) {
    assumed_truth = false;
  } else {
    return false;
  }

  assumed_temp = terminator.first.text;
  return true;
}

void apply_branch_edge_assumptions(const string & assumed_temp,
                                   bool assumed_truth,
                                   ValueEnvironment & environment,
                                   ValueDebugLocationEnvironment & debug_locations,
                                   BooleanTempSet & boolean_temps,
                                   bool track_debug_locations)
{
  environment[assumed_temp] =
      make_integer_operand_for_type("i64", assumed_truth ? 1 : 0);
  if(track_debug_locations) {
    debug_locations.erase(assumed_temp);
  }
  boolean_temps.insert(assumed_temp);
}

ValueBlockAnalysis analyze_value_block(const lir::Block & block,
                                       const ValueEnvironment & input_environment,
                                       const ValueDebugLocationEnvironment & input_debug_locations,
                                       const BooleanTempSet & input_boolean_temps,
                                       const ExpressionCache & input_cache,
                                       const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                       bool track_expression_cache = true,
                                       bool track_reassociation = true,
                                       bool track_debug_locations = true,
                                       const vector<string> * nonterminator_structural_successors = nullptr,
                                       size_t cse_candidate_count = 0)
{
  ValueBlockAnalysis analysis;
  analysis.out_environment = input_environment;
  if(track_debug_locations) {
    analysis.out_debug_locations = input_debug_locations;
  }
  analysis.out_boolean_temps = input_boolean_temps;
  analysis.out_expression_cache = track_expression_cache ? input_cache : ExpressionCache();
  const size_t effective_cse_candidate_count =
      track_expression_cache ? cse_candidate_count : 0;
  if(track_expression_cache && effective_cse_candidate_count == 0) {
    // Recompute on demand when no collected count is available for this block.
    analysis.out_expression_cache.reserve(analysis.out_expression_cache.size() +
                                          count_block_local_cse_candidates(block));
  } else if(effective_cse_candidate_count != 0) {
    analysis.out_expression_cache.reserve(analysis.out_expression_cache.size() +
                                          effective_cse_candidate_count);
  }
  {
    const vector<string> & structural_successors =
        nonterminator_structural_successors != nullptr ?
            *nonterminator_structural_successors :
            collect_nonterminator_structural_successor_labels(block);
    for(size_t i = 0; i < structural_successors.size(); ++i) {
      analysis.executable_successors.insert(structural_successors[i]);
    }
  }
  ProducerEnvironment producers;
  if(track_reassociation && !block.instructions.empty()) {
    producers.reserve(block.instructions.size());
  }

  for(size_t i = 0; i < block.instructions.size(); ++i) {
    lir::Instruction simulated = block.instructions[i];
    process_instruction(analysis.out_environment,
                        analysis.out_debug_locations,
                        analysis.out_boolean_temps,
                        producers,
                        analysis.out_expression_cache,
                        simulated,
                        fallback_debug_location,
                        track_expression_cache,
                        track_reassociation,
                        track_debug_locations);
    if(i + 1 == block.instructions.size()) {
      const unordered_set<string> terminator_successors =
          collect_executable_successor_labels(simulated);
      analysis.executable_successors.insert(terminator_successors.begin(),
                                            terminator_successors.end());
    }
  }

  return analysis;
}

bool simplify_block_instructions(lir::Block & block,
                                 const ValueEnvironment & input_environment,
                                 const ValueDebugLocationEnvironment & input_debug_locations,
                                 const BooleanTempSet & input_boolean_temps,
                                 const ExpressionCache & input_expression_cache,
                                 const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                 bool track_expression_cache = true,
                                 bool track_debug_locations = true,
                                 size_t cse_candidate_count = 0)
{
  ValueEnvironment environment = input_environment;
  ValueDebugLocationEnvironment debug_locations;
  if(track_debug_locations) {
    debug_locations = input_debug_locations;
  }
  BooleanTempSet boolean_temps = input_boolean_temps;
  ExpressionCache expression_cache =
      track_expression_cache ? input_expression_cache : ExpressionCache();
  ProducerEnvironment producers;
  const size_t effective_cse_candidate_count =
      track_expression_cache ? cse_candidate_count : 0;
  if(track_expression_cache && effective_cse_candidate_count == 0) {
    expression_cache.reserve(expression_cache.size() +
                             count_block_local_cse_candidates(block));
  } else if(effective_cse_candidate_count != 0) {
    expression_cache.reserve(expression_cache.size() + cse_candidate_count);
  }
  if(!block.instructions.empty()) {
    producers.reserve(block.instructions.size());
  }
  bool changed = false;

  for(size_t i = 0; i < block.instructions.size(); ++i) {
    lir::Instruction & instruction = block.instructions[i];
    changed |= process_instruction(
        environment,
        debug_locations,
        boolean_temps,
        producers,
        expression_cache,
        instruction,
        fallback_debug_location,
        track_expression_cache,
        true,
        track_debug_locations);
  }

  return changed;
}

bool all_switch_targets_match(const lir::Instruction & instruction)
{
  if(instruction.second.kind != lir::Operand::OP_LABEL) {
    return false;
  }
  const string & target = instruction.second.text;
  for(size_t i = 1; i < instruction.args.size(); i += 2) {
    if(instruction.args[i].kind != lir::Operand::OP_LABEL ||
       instruction.args[i].text != target) {
      return false;
    }
  }
  return !instruction.args.empty();
}

vector<string> collect_successor_labels(const lir::Instruction & instruction)
{
  vector<string> labels;
  switch(instruction.kind) {
    case lir::Instruction::IK_JUMP:
      if(instruction.first.kind == lir::Operand::OP_LABEL) {
        labels.push_back(instruction.first.text);
      }
      break;
    case lir::Instruction::IK_BRANCH:
      if(instruction.second.kind == lir::Operand::OP_LABEL) {
        labels.push_back(instruction.second.text);
      }
      if(instruction.third.kind == lir::Operand::OP_LABEL) {
        labels.push_back(instruction.third.text);
      }
      break;
    case lir::Instruction::IK_SWITCH:
      if(instruction.second.kind == lir::Operand::OP_LABEL) {
        labels.push_back(instruction.second.text);
      }
      for(size_t i = 1; i < instruction.args.size(); i += 2) {
        if(instruction.args[i].kind == lir::Operand::OP_LABEL) {
          labels.push_back(instruction.args[i].text);
        }
      }
      break;
    default:
      break;
  }
  return labels;
}

void append_unique_label(vector<string> & labels, const string & label)
{
  if(find(labels.begin(), labels.end(), label) == labels.end()) {
    labels.push_back(label);
  }
}

vector<string> collect_nonterminator_structural_successor_labels(const lir::Block & block)
{
  vector<string> labels;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    const lir::Instruction & instruction = block.instructions[i];
    if((instruction.kind == lir::Instruction::IK_EH_TRY ||
        instruction.kind == lir::Instruction::IK_EH_CLEANUP) &&
       instruction.first.kind == lir::Operand::OP_LABEL) {
      append_unique_label(labels, instruction.first.text);
    }
  }
  return labels;
}

unordered_set<string> collect_executable_successor_labels(const lir::Instruction & instruction)
{
  unordered_set<string> labels;
  switch(instruction.kind) {
    case lir::Instruction::IK_JUMP:
      if(instruction.first.kind == lir::Operand::OP_LABEL) {
        labels.insert(instruction.first.text);
      }
      break;
    case lir::Instruction::IK_BRANCH: {
      if(instruction.second.kind != lir::Operand::OP_LABEL ||
         instruction.third.kind != lir::Operand::OP_LABEL) {
        break;
      }
      if(instruction.second.text == instruction.third.text) {
        labels.insert(instruction.second.text);
        break;
      }
      bool truthy = false;
      if(is_known_truth_value(instruction.first, truthy)) {
        labels.insert(truthy ? instruction.second.text : instruction.third.text);
        break;
      }
      labels.insert(instruction.second.text);
      labels.insert(instruction.third.text);
      break;
    }
    case lir::Instruction::IK_SWITCH: {
      if(instruction.second.kind != lir::Operand::OP_LABEL) {
        break;
      }
      if(all_switch_targets_match(instruction)) {
        labels.insert(instruction.second.text);
        break;
      }
      long long selector = 0;
      if(is_known_integer_value(instruction.first, selector)) {
        string target = instruction.second.text;
        for(size_t i = 0; i + 1 < instruction.args.size(); i += 2) {
          long long case_value = 0;
          if(!is_known_integer_value(instruction.args[i], case_value)) {
            continue;
          }
          if(case_value == selector &&
             instruction.args[i + 1].kind == lir::Operand::OP_LABEL) {
            target = instruction.args[i + 1].text;
            break;
          }
        }
        labels.insert(target);
        break;
      }
      labels.insert(instruction.second.text);
      for(size_t i = 1; i < instruction.args.size(); i += 2) {
        if(instruction.args[i].kind == lir::Operand::OP_LABEL) {
          labels.insert(instruction.args[i].text);
        }
      }
      break;
    }
    default:
      break;
  }
  return labels;
}

bool simplify_block_terminator(lir::Function & function,
                               const BlockIndexMap & block_index,
                               size_t block_index_value)
{
  lir::Block & block = function.blocks[block_index_value];
  if(block.instructions.empty()) {
    return false;
  }

  lir::Instruction & terminator = block.instructions.back();
  bool changed = false;

  switch(terminator.kind) {
    case lir::Instruction::IK_JUMP:
      changed |= rewrite_label_operand(function, block_index, terminator.first);
      break;
    case lir::Instruction::IK_BRANCH: {
      changed |= rewrite_label_operand(function, block_index, terminator.second);
      changed |= rewrite_label_operand(function, block_index, terminator.third);
      if(terminator.second.kind == lir::Operand::OP_LABEL &&
         terminator.third.kind == lir::Operand::OP_LABEL &&
         terminator.second.text == terminator.third.text) {
        terminator = make_jump_instruction(terminator.second.text, &terminator);
        return true;
      }
      bool truthy = false;
      if(is_known_truth_value(terminator.first, truthy) &&
         terminator.second.kind == lir::Operand::OP_LABEL &&
         terminator.third.kind == lir::Operand::OP_LABEL) {
        terminator = make_jump_instruction(truthy ?
                                           terminator.second.text :
                                           terminator.third.text,
                                           &terminator);
        changed = true;
        changed |= rewrite_label_operand(function, block_index, terminator.first);
      }
      break;
    }
    case lir::Instruction::IK_SWITCH: {
      changed |= rewrite_label_operand(function, block_index, terminator.second);
      for(size_t i = 1; i < terminator.args.size(); i += 2) {
        changed |= rewrite_label_operand(function, block_index, terminator.args[i]);
      }
      if(all_switch_targets_match(terminator)) {
        terminator = make_jump_instruction(terminator.second.text, &terminator);
        return true;
      }
      long long selector = 0;
      if(is_known_integer_value(terminator.first, selector) &&
         terminator.second.kind == lir::Operand::OP_LABEL) {
        string target = terminator.second.text;
        for(size_t i = 0; i + 1 < terminator.args.size(); i += 2) {
          long long case_value = 0;
          if(!is_known_integer_value(terminator.args[i], case_value)) {
            continue;
          }
          if(case_value == selector &&
             terminator.args[i + 1].kind == lir::Operand::OP_LABEL) {
            target = terminator.args[i + 1].text;
            break;
          }
        }
        terminator = make_jump_instruction(target, &terminator);
        changed = true;
        changed |= rewrite_label_operand(function, block_index, terminator.first);
      }
      break;
    }
    default:
      break;
  }

  return changed;
}

struct IncomingBlockEdge
{
  size_t predecessor = 0;
  bool has_eh_edge = false;
};

struct FunctionControlFlow
{
  vector<vector<IncomingBlockEdge> > incoming_edges;
  vector<vector<size_t> > successors;
};

struct FunctionOptimizationContext
{
  BlockIndexMap block_index;
  FunctionControlFlow control_flow;
  size_t function_instruction_total = 0;
  bool track_analysis_expression_cache = false;
  bool track_analysis_reassociation = false;
  bool track_cleanup_expression_cache = false;
  bool track_debug_locations = false;
};

FunctionControlFlow build_function_control_flow(const lir::Function & function,
                                                const BlockIndexMap & block_index);
FunctionControlFlow build_function_control_flow(const lir::Function & function);
FunctionOptimizationContext collect_function_optimization_context(const lir::Function & function);

bool remove_unreachable_blocks(lir::Function & function,
                               const FunctionControlFlow * control_flow = nullptr)
{
  if(function.blocks.empty()) {
    return false;
  }

  FunctionControlFlow local_control_flow;
  if(control_flow == nullptr) {
    local_control_flow = build_function_control_flow(function);
    control_flow = &local_control_flow;
  }
  vector<bool> reachable(function.blocks.size(), false);
  vector<size_t> worklist(1, 0);

  while(!worklist.empty()) {
    const size_t block_i = worklist.back();
    worklist.pop_back();
    if(reachable[block_i]) {
      continue;
    }
    reachable[block_i] = true;
    for(size_t i = 0; i < control_flow->successors[block_i].size(); ++i) {
      const size_t successor = control_flow->successors[block_i][i];
      if(!reachable[successor]) {
        worklist.push_back(successor);
      }
    }
  }

  bool removed_any = false;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    if(!reachable[i]) {
      removed_any = true;
      break;
    }
  }
  if(!removed_any) {
    return false;
  }

  vector<lir::Block> kept_blocks;
  kept_blocks.reserve(function.blocks.size());
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    if(reachable[i]) {
      kept_blocks.push_back(std::move(function.blocks[i]));
    }
  }
  function.blocks.swap(kept_blocks);
  return true;
}

bool instruction_is_eh_related(const lir::Instruction & instruction)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_EH_FILTER:
    case lir::Instruction::IK_EH_CATCH_ALL:
    case lir::Instruction::IK_EH_END:
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_EXCEPTION:
    case lir::Instruction::IK_EXCEPTION_SELECTOR:
    case lir::Instruction::IK_RESUME:
      return true;
    default:
      return false;
  }
}

bool block_contains_eh_related_instruction(const lir::Block & block)
{
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    if(instruction_is_eh_related(block.instructions[i])) {
      return true;
    }
  }
  return false;
}

FunctionControlFlow build_function_control_flow(const lir::Function & function,
                                                const BlockIndexMap & block_index)
{
  FunctionControlFlow control_flow;
  control_flow.incoming_edges.resize(function.blocks.size());
  control_flow.successors.resize(function.blocks.size());

  for(size_t i = 0; i < function.blocks.size(); ++i) {
    const lir::Block & block = function.blocks[i];
    vector<size_t> & block_successors = control_flow.successors[i];
    const auto add_edge = [&](const string & label, bool has_eh_edge)
    {
      const BlockIndexMap::const_iterator found = block_index.find(label);
      if(found == block_index.end()) {
        return;
      }
      const size_t successor = found->second;
      if(find(block_successors.begin(), block_successors.end(), successor) ==
         block_successors.end()) {
        block_successors.push_back(successor);
      }
      vector<IncomingBlockEdge> & incoming = control_flow.incoming_edges[successor];
      for(size_t pi = 0; pi < incoming.size(); ++pi) {
        if(incoming[pi].predecessor == i) {
          incoming[pi].has_eh_edge = incoming[pi].has_eh_edge || has_eh_edge;
          return;
        }
      }
      IncomingBlockEdge edge;
      edge.predecessor = i;
      edge.has_eh_edge = has_eh_edge;
      incoming.push_back(edge);
    };

    for(size_t j = 0; j < block.instructions.size(); ++j) {
      const lir::Instruction & instruction = block.instructions[j];
      if((instruction.kind == lir::Instruction::IK_EH_TRY ||
          instruction.kind == lir::Instruction::IK_EH_CLEANUP) &&
         instruction.first.kind == lir::Operand::OP_LABEL) {
        add_edge(instruction.first.text, true);
      }
    }
    if(block.instructions.empty()) {
      continue;
    }
    const vector<string> terminator_successors =
        collect_successor_labels(block.instructions.back());
    for(size_t j = 0; j < terminator_successors.size(); ++j) {
      add_edge(terminator_successors[j], false);
    }
  }

  return control_flow;
}

FunctionControlFlow build_function_control_flow(const lir::Function & function)
{
  const BlockIndexMap block_index = build_block_index_map(function);
  return build_function_control_flow(function, block_index);
}

bool function_has_debug_locations(const lir::Function & function)
{
  if(function.debug_location.present()) {
    return true;
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      if(function.blocks[bi].instructions[ii].debug_location.present()) {
        return true;
      }
    }
  }
  return false;
}

FunctionOptimizationContext collect_function_optimization_context(const lir::Function & function)
{
  FunctionOptimizationContext context;
  context.block_index = build_block_index_map(function);
  context.control_flow = build_function_control_flow(function, context.block_index);

  for(size_t i = 0; i < function.blocks.size(); ++i) {
    context.function_instruction_total += function.blocks[i].instructions.size();
  }

  context.track_analysis_expression_cache =
      function.blocks.size() <= 8 && context.function_instruction_total <= 128;
  context.track_analysis_reassociation =
      function.blocks.size() <= 12 && context.function_instruction_total <= 192;
  context.track_cleanup_expression_cache =
      function.blocks.size() <= 12 && context.function_instruction_total <= 192;
  context.track_debug_locations = function_has_debug_locations(function);
  return context;
}

bool merge_straight_line_blocks(lir::Function & function,
                                const FunctionControlFlow * control_flow = nullptr,
                                const BlockIndexMap * block_index = nullptr)
{
  if(function.blocks.size() < 2) {
    return false;
  }

  FunctionControlFlow local_control_flow;
  if(control_flow == nullptr) {
    local_control_flow = build_function_control_flow(function);
    control_flow = &local_control_flow;
  }
  BlockIndexMap local_block_index;
  if(block_index == nullptr) {
    local_block_index = build_block_index_map(function);
    block_index = &local_block_index;
  }

  for(size_t i = 0; i < function.blocks.size(); ++i) {
    lir::Block & block = function.blocks[i];
    if(block.instructions.empty()) {
      continue;
    }
    const lir::Instruction & terminator = block.instructions.back();
    if(terminator.kind != lir::Instruction::IK_JUMP ||
       terminator.first.kind != lir::Operand::OP_LABEL) {
      continue;
    }

    const BlockIndexMap::const_iterator found = block_index->find(terminator.first.text);
    if(found == block_index->end() || found->second == i) {
      continue;
    }
    const size_t target_index = found->second;
    if(control_flow->incoming_edges[target_index].size() != 1 ||
       control_flow->incoming_edges[target_index][0].predecessor != i) {
      continue;
    }
    if(block_contains_eh_related_instruction(block) ||
       block_contains_eh_related_instruction(function.blocks[target_index])) {
      continue;
    }

    lir::Block merged = function.blocks[i];
    merged.instructions.pop_back();
    merged.instructions.insert(merged.instructions.end(),
                               function.blocks[target_index].instructions.begin(),
                               function.blocks[target_index].instructions.end());

    vector<lir::Block> updated;
    updated.reserve(function.blocks.size() - 1);
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      if(bi == i) {
        updated.push_back(merged);
      } else if(bi != target_index) {
        updated.push_back(function.blocks[bi]);
      }
    }
    function.blocks.swap(updated);
    return true;
  }

  return false;
}

void collect_temp_uses(const lir::Operand & operand, TempUseList & out)
{
  if(operand.kind == lir::Operand::OP_TEMP &&
     find(out.begin(), out.end(), operand.text) == out.end()) {
    out.push_back(operand.text);
  }
}

bool indexed_slot_family_root(const string & slot_name, string & out)
{
  const size_t pos = slot_name.rfind("__");
  if(pos == string::npos || pos + 2 >= slot_name.size()) {
    return false;
  }
  for(size_t i = pos + 2; i < slot_name.size(); ++i) {
    if(slot_name[i] < '0' || slot_name[i] > '9') {
      return false;
    }
  }
  out = slot_name.substr(0, pos);
  return true;
}

TempUseList instruction_temp_uses(const lir::Instruction & instruction)
{
  TempUseList out;
  out.reserve(4);
  switch(instruction.kind) {
    case lir::Instruction::IK_STORE:
      collect_temp_uses(instruction.first, out);
      collect_temp_uses(instruction.second, out);
      break;
    case lir::Instruction::IK_ATOMIC_STORE:
      collect_temp_uses(instruction.first, out);
      collect_temp_uses(instruction.second, out);
      break;
    case lir::Instruction::IK_VA_START:
    case lir::Instruction::IK_VA_ARG:
      collect_temp_uses(instruction.first, out);
      break;
    case lir::Instruction::IK_STACK_ALLOC:
      collect_temp_uses(instruction.first, out);
      break;
    case lir::Instruction::IK_CALL:
      collect_temp_uses(instruction.first, out);
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        collect_temp_uses(instruction.args[i], out);
      }
      break;
    case lir::Instruction::IK_COPYOBJ:
      collect_temp_uses(instruction.first, out);
      collect_temp_uses(instruction.second, out);
      break;
    case lir::Instruction::IK_ZEROINIT:
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_JUMP:
      collect_temp_uses(instruction.first, out);
      break;
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
      break;
    case lir::Instruction::IK_EH_FILTER:
      for(size_t i = 0; i < instruction.args.size(); ++i) {
        collect_temp_uses(instruction.args[i], out);
      }
      break;
    case lir::Instruction::IK_BRANCH:
      collect_temp_uses(instruction.first, out);
      break;
    case lir::Instruction::IK_SWITCH:
      collect_temp_uses(instruction.first, out);
      break;
    case lir::Instruction::IK_RETURN:
      if(instruction.type.text != "void") {
        collect_temp_uses(instruction.first, out);
      }
      break;
    default:
      break;
  }

  if(instruction.kind == lir::Instruction::IK_CONST ||
     instruction.kind == lir::Instruction::IK_COPY ||
     instruction.kind == lir::Instruction::IK_LOAD ||
     instruction.kind == lir::Instruction::IK_ATOMIC_LOAD ||
     instruction.kind == lir::Instruction::IK_UNARY ||
     instruction.kind == lir::Instruction::IK_CONVERT) {
    collect_temp_uses(instruction.first, out);
  }

  if(instruction.kind == lir::Instruction::IK_INDEX ||
     instruction.kind == lir::Instruction::IK_BINARY ||
     instruction.kind == lir::Instruction::IK_CMP ||
     instruction.kind == lir::Instruction::IK_ATOMIC_ADD_FETCH ||
     instruction.kind == lir::Instruction::IK_ATOMIC_EXCHANGE ||
     instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    collect_temp_uses(instruction.first, out);
    collect_temp_uses(instruction.second, out);
  }

  if(instruction.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) {
    collect_temp_uses(instruction.third, out);
  }

  if(out.size() > 1) {
    sort(out.begin(), out.end());
  }
  return out;
}

bool instruction_is_dead_code_candidate(const lir::Instruction & instruction)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_CONST:
    case lir::Instruction::IK_COPY:
    case lir::Instruction::IK_ADDR:
    case lir::Instruction::IK_INDEX:
    case lir::Instruction::IK_UNARY:
    case lir::Instruction::IK_BINARY:
    case lir::Instruction::IK_CMP:
    case lir::Instruction::IK_CONVERT:
      return true;
    default:
      return false;
  }
}

bool block_has_incoming_eh_edge(const FunctionControlFlow & control_flow,
                                size_t block_index)
{
  if(block_index >= control_flow.incoming_edges.size()) {
    return false;
  }
  const vector<IncomingBlockEdge> & incoming_edges =
      control_flow.incoming_edges[block_index];
  for(size_t i = 0; i < incoming_edges.size(); ++i) {
    if(incoming_edges[i].has_eh_edge) {
      return true;
    }
  }
  return false;
}

bool resolve_plain_jump_target(const lir::Function & function,
                               const BlockIndexMap & block_index,
                               const FunctionControlFlow & control_flow,
                               const string & label,
                               string & resolved_label)
{
  resolved_label = label;
  set<string> seen_labels;
  while(seen_labels.insert(resolved_label).second) {
    const BlockIndexMap::const_iterator found = block_index.find(resolved_label);
    if(found == block_index.end()) {
      return false;
    }
    const size_t target_index = found->second;
    const lir::Block & target_block = function.blocks[target_index];
    if(block_has_incoming_eh_edge(control_flow, target_index) ||
       block_contains_eh_related_instruction(target_block) ||
       target_block.instructions.size() != 1) {
      return true;
    }
    const lir::Instruction & only_instruction = target_block.instructions.front();
    if(only_instruction.kind != lir::Instruction::IK_JUMP ||
       only_instruction.first.kind != lir::Operand::OP_LABEL) {
      return true;
    }
    resolved_label = only_instruction.first.text;
  }
  return false;
}

bool collapse_empty_branch_diamonds(lir::Function & function,
                                    const BlockIndexMap & block_index,
                                    const FunctionControlFlow & control_flow)
{
  bool changed = false;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    lir::Block & block = function.blocks[i];
    if(block.instructions.empty()) {
      continue;
    }
    lir::Instruction & terminator = block.instructions.back();
    if(terminator.kind != lir::Instruction::IK_BRANCH ||
       terminator.second.kind != lir::Operand::OP_LABEL ||
       terminator.third.kind != lir::Operand::OP_LABEL) {
      continue;
    }

    string true_target;
    string false_target;
    if(!resolve_plain_jump_target(function,
                                  block_index,
                                  control_flow,
                                  terminator.second.text,
                                  true_target) ||
       !resolve_plain_jump_target(function,
                                  block_index,
                                  control_flow,
                                  terminator.third.text,
                                  false_target) ||
       true_target != false_target) {
      continue;
    }

    terminator = make_jump_instruction(true_target, &terminator);
    changed = true;
  }
  return changed;
}

pair<set<string>, set<string> > block_use_def_sets(const lir::Block & block)
{
  set<string> use_set;
  set<string> def_set;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    const TempUseList uses = instruction_temp_uses(block.instructions[i]);
    for(TempUseList::const_iterator it = uses.begin(); it != uses.end(); ++it) {
      if(def_set.count(*it) == 0) {
        use_set.insert(*it);
      }
    }
    if(!block.instructions[i].dest.empty()) {
      def_set.insert(block.instructions[i].dest);
    }
  }
  return make_pair(use_set, def_set);
}

lir::FunctionBoundaryMetadata resolved_call_boundary(
    const lir::Instruction & instruction,
    const FunctionBoundaryMap & function_boundaries)
{
  lir::FunctionBoundaryMetadata boundary;
  if(instruction.kind != lir::Instruction::IK_CALL) {
    return boundary;
  }
  if(instruction.first.kind == lir::Operand::OP_GLOBAL) {
    const FunctionBoundaryMap::const_iterator found =
        function_boundaries.find(instruction.first.text);
    if(found != function_boundaries.end()) {
      merge_boundary_metadata(boundary, found->second);
    }
  }
  if(instruction.has_call_signature) {
    merge_boundary_metadata(boundary, instruction.call_boundary);
  }
  return boundary;
}

bool instruction_is_dead_readnone_call(const lir::Instruction & instruction,
                                       const FunctionBoundaryMap & function_boundaries)
{
  if(instruction.kind != lir::Instruction::IK_CALL) {
    return false;
  }
  const lir::FunctionBoundaryMetadata boundary =
      resolved_call_boundary(instruction, function_boundaries);
  return boundary.effects == lir::CFXM_READNONE &&
         boundary.unwind == lir::CUM_NO &&
         boundary.returns != lir::CRM_NORETURN;
}

bool instruction_may_unwind(const lir::Instruction & instruction,
                            const FunctionBoundaryMap & function_boundaries)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_CALL: {
      const lir::FunctionBoundaryMetadata boundary =
          resolved_call_boundary(instruction, function_boundaries);
      return boundary.unwind != lir::CUM_NO;
    }
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_RESUME:
      return true;
    default:
      return false;
  }
}

bool instruction_starts_eh_region(const lir::Instruction & instruction)
{
  return (instruction.kind == lir::Instruction::IK_EH_TRY ||
          instruction.kind == lir::Instruction::IK_EH_CLEANUP) &&
         instruction.first.kind == lir::Operand::OP_LABEL;
}

bool instruction_is_allowed_inside_nonthrowing_eh_region(
    const lir::Instruction & instruction)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_END:
      return true;
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_EH_FILTER:
    case lir::Instruction::IK_EH_CATCH_ALL:
    case lir::Instruction::IK_EXCEPTION:
    case lir::Instruction::IK_EXCEPTION_SELECTOR:
    case lir::Instruction::IK_RESUME:
      return false;
    default:
      return true;
  }
}

struct EhInstructionPosition
{
  size_t block_index = 0;
  size_t instruction_index = 0;

  bool operator==(const EhInstructionPosition & rhs) const
  {
    return block_index == rhs.block_index &&
           instruction_index == rhs.instruction_index;
  }

  bool operator<(const EhInstructionPosition & rhs) const
  {
    if(block_index != rhs.block_index) {
      return block_index < rhs.block_index;
    }
    return instruction_index < rhs.instruction_index;
  }
};

struct EhRegionScanState
{
  size_t block_index = 0;
  size_t instruction_index = 0;
  size_t depth = 0;

  bool operator<(const EhRegionScanState & rhs) const
  {
    if(block_index != rhs.block_index) {
      return block_index < rhs.block_index;
    }
    if(instruction_index != rhs.instruction_index) {
      return instruction_index < rhs.instruction_index;
    }
    return depth < rhs.depth;
  }
};

bool find_nonthrowing_eh_region_ends(
    const lir::Function & function,
    const BlockIndexMap & block_index,
    const FunctionBoundaryMap & function_boundaries,
    size_t start_block_index,
    size_t start_instruction_index,
    vector<EhInstructionPosition> & end_positions)
{
  if(start_block_index >= function.blocks.size() ||
     start_instruction_index >= function.blocks[start_block_index].instructions.size()) {
    return false;
  }

  const lir::Instruction & start_instruction =
      function.blocks[start_block_index].instructions[start_instruction_index];
  if(!instruction_starts_eh_region(start_instruction)) {
    return false;
  }

  const size_t max_scan_states =
      std::max<size_t>(64, function_instruction_count(function) * 8);
  vector<EhRegionScanState> worklist;
  set<EhRegionScanState> seen;
  EhRegionScanState start_state;
  start_state.block_index = start_block_index;
  start_state.instruction_index = start_instruction_index + 1;
  start_state.depth = 1;
  worklist.push_back(start_state);

  while(!worklist.empty()) {
    const EhRegionScanState state = worklist.back();
    worklist.pop_back();
    if(state.block_index >= function.blocks.size() ||
       !seen.insert(state).second) {
      continue;
    }
    if(seen.size() > max_scan_states || state.depth > 64) {
      return false;
    }

    const lir::Block & block = function.blocks[state.block_index];
    size_t depth = state.depth;
    bool closed_region_on_this_path = false;
    for(size_t ii = state.instruction_index; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & instruction = block.instructions[ii];
      if(!instruction_is_allowed_inside_nonthrowing_eh_region(instruction) ||
         instruction_may_unwind(instruction, function_boundaries)) {
        return false;
      }

      if(instruction_starts_eh_region(instruction)) {
        ++depth;
        if(depth > 64) {
          return false;
        }
        continue;
      }

      if(instruction.kind == lir::Instruction::IK_EH_END) {
        if(depth == 0) {
          return false;
        }
        --depth;
        if(depth == 0) {
          EhInstructionPosition end_position;
          end_position.block_index = state.block_index;
          end_position.instruction_index = ii;
          if(find(end_positions.begin(), end_positions.end(), end_position) ==
             end_positions.end()) {
            end_positions.push_back(end_position);
          }
          closed_region_on_this_path = true;
          break;
        }
      }
    }

    if(closed_region_on_this_path) {
      continue;
    }

    const vector<string> successor_labels =
        block.instructions.empty() ? vector<string>() :
                                      collect_successor_labels(block.instructions.back());
    if(successor_labels.empty()) {
      return false;
    }
    for(size_t si = 0; si < successor_labels.size(); ++si) {
      const BlockIndexMap::const_iterator found =
          block_index.find(successor_labels[si]);
      if(found == block_index.end()) {
        return false;
      }
      EhRegionScanState successor_state;
      successor_state.block_index = found->second;
      successor_state.instruction_index = 0;
      successor_state.depth = depth;
      worklist.push_back(successor_state);
    }
  }

  return !end_positions.empty();
}

bool remove_nonthrowing_eh_markers(lir::Function & function,
                                   const FunctionBoundaryMap & function_boundaries)
{
  if(function.boundary.unwind != lir::CUM_NO) {
    return false;
  }

  const size_t instruction_count = function_instruction_count(function);
  if(instruction_count > 256) {
    return false;
  }

  const BlockIndexMap block_index = build_block_index_map(function);
  vector<vector<bool> > remove_instruction(function.blocks.size());
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    remove_instruction[bi].assign(function.blocks[bi].instructions.size(), false);
  }

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    const lir::Block & block = function.blocks[bi];
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & instruction = block.instructions[ii];
      if(!instruction_starts_eh_region(instruction)) {
        continue;
      }

      vector<EhInstructionPosition> end_positions;
      if(!find_nonthrowing_eh_region_ends(function,
                                          block_index,
                                          function_boundaries,
                                          bi,
                                          ii,
                                          end_positions)) {
        continue;
      }
      remove_instruction[bi][ii] = true;
      for(size_t ei = 0; ei < end_positions.size(); ++ei) {
        remove_instruction[end_positions[ei].block_index]
                          [end_positions[ei].instruction_index] = true;
      }
    }
  }

  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    size_t remove_count = 0;
    for(size_t ii = 0; ii < remove_instruction[bi].size(); ++ii) {
      if(remove_instruction[bi][ii]) {
        ++remove_count;
      }
    }
    if(remove_count == 0) {
      continue;
    }

    lir::Block & block = function.blocks[bi];
    vector<lir::Instruction> kept;
    kept.reserve(block.instructions.size() - remove_count);
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      if(!remove_instruction[bi][ii]) {
        kept.push_back(block.instructions[ii]);
      }
    }
    block.instructions.swap(kept);
    changed = true;
  }

  return changed;
}

bool eliminate_dead_code(lir::Function & function,
                         const FunctionBoundaryMap & function_boundaries,
                         const FunctionControlFlow * control_flow = nullptr)
{
  if(function.blocks.empty()) {
    return false;
  }

  FunctionControlFlow local_control_flow;
  if(control_flow == nullptr) {
    local_control_flow = build_function_control_flow(function);
    control_flow = &local_control_flow;
  }
  vector<set<string> > block_use(function.blocks.size());
  vector<set<string> > block_def(function.blocks.size());
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    const pair<set<string>, set<string> > use_def = block_use_def_sets(function.blocks[i]);
    block_use[i] = use_def.first;
    block_def[i] = use_def.second;
  }

  vector<set<string> > live_in(function.blocks.size());
  vector<set<string> > live_out(function.blocks.size());
  bool dataflow_changed = false;
  do {
    dataflow_changed = false;
    for(size_t offset = function.blocks.size(); offset-- != 0;) {
      set<string> new_live_out;
      for(size_t si = 0; si < control_flow->successors[offset].size(); ++si) {
        const size_t successor = control_flow->successors[offset][si];
        new_live_out.insert(live_in[successor].begin(),
                            live_in[successor].end());
      }

      set<string> new_live_in = block_use[offset];
      for(set<string>::const_iterator it = new_live_out.begin();
          it != new_live_out.end();
          ++it) {
        if(block_def[offset].count(*it) == 0) {
          new_live_in.insert(*it);
        }
      }

      if(new_live_out != live_out[offset] || new_live_in != live_in[offset]) {
        live_out[offset] = new_live_out;
        live_in[offset] = new_live_in;
        dataflow_changed = true;
      }
    }
  } while(dataflow_changed);

  bool changed = false;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    set<string> live = live_out[i];
    vector<lir::Instruction> kept_reversed;
    for(size_t pos = function.blocks[i].instructions.size(); pos-- != 0;) {
      const lir::Instruction & instruction = function.blocks[i].instructions[pos];
      const bool removable =
          instruction_is_dead_readnone_call(instruction, function_boundaries) &&
          (instruction.dest.empty() || live.count(instruction.dest) == 0);
      const bool removable_dead_temp =
          !instruction.dest.empty() &&
          instruction_is_dead_code_candidate(instruction) &&
          live.count(instruction.dest) == 0;
      const bool removable_dead_slot_load =
          !instruction.dest.empty() &&
          instruction.kind == lir::Instruction::IK_LOAD &&
          instruction.first.kind == lir::Operand::OP_SLOT &&
          live.count(instruction.dest) == 0;
      if(removable || removable_dead_temp || removable_dead_slot_load) {
        changed = true;
        continue;
      }

      if(!instruction.dest.empty()) {
        live.erase(instruction.dest);
      }
      const TempUseList uses = instruction_temp_uses(instruction);
      live.insert(uses.begin(), uses.end());
      kept_reversed.push_back(instruction);
    }

    vector<lir::Instruction> kept;
    kept.reserve(kept_reversed.size());
    for(size_t pos = kept_reversed.size(); pos-- != 0;) {
      kept.push_back(kept_reversed[pos]);
    }
    function.blocks[i].instructions.swap(kept);
  }

  return changed;
}

struct SlotStoreUseSummary
{
  bool saw_store = false;
  bool saw_load = false;
  bool saw_other_use = false;
};

void note_slot_other_use(map<string, SlotStoreUseSummary> & slot_uses,
                         const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_SLOT) {
    return;
  }
  map<string, SlotStoreUseSummary>::iterator found = slot_uses.find(operand.text);
  if(found != slot_uses.end()) {
    found->second.saw_other_use = true;
  }
}

bool remove_stores_to_unread_slots(lir::Function & function)
{
  if(function.slots.empty()) {
    return false;
  }

  map<string, SlotStoreUseSummary> slot_uses;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    slot_uses[function.slots[i].first] = SlotStoreUseSummary();
  }

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    const lir::Block & block = function.blocks[bi];
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & instruction = block.instructions[ii];
      if(instruction.kind == lir::Instruction::IK_STORE) {
        note_slot_other_use(slot_uses, instruction.first);
        if(instruction.second.kind == lir::Operand::OP_SLOT) {
          map<string, SlotStoreUseSummary>::iterator found =
              slot_uses.find(instruction.second.text);
          if(found != slot_uses.end()) {
            found->second.saw_store = true;
          }
        } else {
          note_slot_other_use(slot_uses, instruction.second);
        }
        continue;
      }

      if(instruction.kind == lir::Instruction::IK_LOAD &&
         instruction.first.kind == lir::Operand::OP_SLOT) {
        map<string, SlotStoreUseSummary>::iterator found =
            slot_uses.find(instruction.first.text);
        if(found != slot_uses.end()) {
          found->second.saw_load = true;
        }
        continue;
      }

      note_slot_other_use(slot_uses, instruction.first);
      note_slot_other_use(slot_uses, instruction.second);
      note_slot_other_use(slot_uses, instruction.third);
      for(size_t ai = 0; ai < instruction.args.size(); ++ai) {
        note_slot_other_use(slot_uses, instruction.args[ai]);
      }
    }
  }

  set<string> unread_store_slots;
  for(map<string, SlotStoreUseSummary>::const_iterator it = slot_uses.begin();
      it != slot_uses.end();
      ++it) {
    if(it->second.saw_store &&
       !it->second.saw_load &&
       !it->second.saw_other_use) {
      unread_store_slots.insert(it->first);
    }
  }
  if(unread_store_slots.empty()) {
    return false;
  }

  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    lir::Block & block = function.blocks[bi];
    vector<lir::Instruction> kept;
    kept.reserve(block.instructions.size());
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & instruction = block.instructions[ii];
      if(instruction.kind == lir::Instruction::IK_STORE &&
         instruction.second.kind == lir::Operand::OP_SLOT &&
         unread_store_slots.count(instruction.second.text) != 0) {
        changed = true;
        continue;
      }
      kept.push_back(instruction);
    }
    if(kept.size() != block.instructions.size()) {
      block.instructions.swap(kept);
    }
  }
  return changed;
}

bool remove_unused_slots(lir::Function & function)
{
  set<string> used_slots;
  set<string> used_indexed_slot_families;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & instruction = function.blocks[bi].instructions[ii];
      const auto note_slot = [&used_slots, &used_indexed_slot_families](const lir::Operand & operand) {
        if(operand.kind == lir::Operand::OP_SLOT) {
          used_slots.insert(operand.text);
          string family_root;
          if(indexed_slot_family_root(operand.text, family_root)) {
            used_indexed_slot_families.insert(family_root);
          }
        }
      };

      note_slot(instruction.first);
      note_slot(instruction.second);
      note_slot(instruction.third);
      for(size_t ai = 0; ai < instruction.args.size(); ++ai) {
        note_slot(instruction.args[ai]);
      }
    }
  }

  vector<pair<string, lir::LowType> > kept_slots;
  bool changed = false;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    string family_root;
    if(used_slots.count(function.slots[i].first) != 0 ||
       (indexed_slot_family_root(function.slots[i].first, family_root) &&
        used_indexed_slot_families.count(family_root) != 0)) {
      kept_slots.push_back(function.slots[i]);
    } else {
      changed = true;
    }
  }
  if(changed) {
    function.slots.swap(kept_slots);
  }
  return changed;
}

bool propagate_known_values_across_blocks(lir::Function & function,
                                         const FunctionOptimizationContext * context = nullptr)
{
  if(function.blocks.empty()) {
    return false;
  }

  FunctionOptimizationContext local_context;
  if(context == nullptr) {
    local_context = collect_function_optimization_context(function);
    context = &local_context;
  }

  const ValueDataflowState dataflow = compute_value_dataflow(function, context);
  const lir::InstructionDebugLocation * fallback_debug_location =
      function.debug_location.present() ? &function.debug_location : nullptr;
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    if(!dataflow.executable[bi]) {
      continue;
    }
    const ResolvedValueInputState input =
        resolve_dataflow_input_state(dataflow, bi, context->track_debug_locations);
    changed |= simplify_block_instructions(function.blocks[bi],
                                           input.in_environment(),
                                           input.in_debug_locations(),
                                           input.in_boolean_temps(),
                                           input.in_expression_cache(),
                                           fallback_debug_location,
                                           context->track_cleanup_expression_cache,
                                           context->track_debug_locations);
  }
  return changed;
}

ValueDataflowState compute_value_dataflow(const lir::Function & function,
                                         const FunctionOptimizationContext * context)
{
  ValueDataflowState state;
  state.in_environments.resize(function.blocks.size());
  state.in_debug_locations.resize(function.blocks.size());
  state.in_boolean_temps.resize(function.blocks.size());
  state.in_expression_caches.resize(function.blocks.size());
  state.analyses.resize(function.blocks.size());
  state.executable.assign(function.blocks.size(), false);
  state.input_kinds.assign(function.blocks.size(), ValueDataflowState::DIK_UNREACHABLE);
  state.input_single_predecessors.assign(function.blocks.size(), 0);
  state.input_single_predecessor_versions.assign(function.blocks.size(), 0);
  state.input_has_branch_assumptions.assign(function.blocks.size(), false);
  state.input_assumed_temps.resize(function.blocks.size());
  state.input_assumed_truths.assign(function.blocks.size(), false);
  if(function.blocks.empty()) {
    return state;
  }

  FunctionOptimizationContext local_context;
  if(context == nullptr) {
    local_context = collect_function_optimization_context(function);
    context = &local_context;
  }

  const size_t no_predecessor = function.blocks.size();
  const lir::InstructionDebugLocation * fallback_debug_location =
      function.debug_location.present() ? &function.debug_location : nullptr;
  const ValueEnvironment empty_environment;
  const ValueDebugLocationEnvironment empty_debug_locations;
  const BooleanTempSet empty_boolean_temps;
  const ExpressionCache empty_expression_cache;
  vector<size_t> worklist(1, 0);
  vector<bool> queued(function.blocks.size(), false);
  vector<size_t> analysis_versions(function.blocks.size(), 0);
  queued[0] = true;

  while(!worklist.empty()) {
    const size_t bi = worklist.back();
    worklist.pop_back();
    queued[bi] = false;

    vector<IncomingValueState> incoming;
    ValueEnvironment merged_in;
    ValueDebugLocationEnvironment merged_in_debug_locations;
    BooleanTempSet merged_in_boolean;
    ExpressionCache merged_in_cache;
    ValueEnvironment single_in_environment;
    ValueDebugLocationEnvironment single_in_debug_locations;
    BooleanTempSet single_in_boolean;
    bool new_executable = false;
    ValueDataflowState::InputKind new_input_kind = ValueDataflowState::DIK_UNREACHABLE;
    size_t new_input_single_predecessor = no_predecessor;
    size_t new_input_single_predecessor_version = 0;
    bool new_input_has_branch_assumption = false;
    string new_input_assumed_temp;
    bool new_input_assumed_truth = false;
    const ValueEnvironment * input_environment = &empty_environment;
    const ValueDebugLocationEnvironment * input_debug_locations = &empty_debug_locations;
    const BooleanTempSet * input_boolean_temps = &empty_boolean_temps;
    const ExpressionCache * input_expression_cache = &empty_expression_cache;
    if(bi == 0) {
      new_executable = true;
      new_input_kind = ValueDataflowState::DIK_ENTRY;
      if(state.executable[bi] && state.input_kinds[bi] == ValueDataflowState::DIK_ENTRY) {
        continue;
      }
    } else {
      const vector<IncomingBlockEdge> & incoming_edges =
          context->control_flow.incoming_edges[bi];
      const string & block_label = function.blocks[bi].label;
      size_t first_predecessor = no_predecessor;
      bool have_any_predecessor = false;
      bool have_multiple_predecessors = false;
      bool have_eh_predecessor = false;
      for(size_t pi = 0; pi < incoming_edges.size(); ++pi) {
        const IncomingBlockEdge & edge = incoming_edges[pi];
        const size_t predecessor = edge.predecessor;
        if(!state.executable[predecessor] ||
           state.analyses[predecessor].executable_successors.count(block_label) == 0) {
          continue;
        }
        have_any_predecessor = true;
        if(edge.has_eh_edge) {
          have_eh_predecessor = true;
          break;
        }
        if(first_predecessor == no_predecessor) {
          first_predecessor = predecessor;
        } else {
          have_multiple_predecessors = true;
        }
      }

      if(!have_any_predecessor) {
        if(!state.executable[bi] &&
           state.input_kinds[bi] == ValueDataflowState::DIK_UNREACHABLE) {
          continue;
        }
      } else if(have_eh_predecessor) {
        new_executable = true;
        new_input_kind = ValueDataflowState::DIK_EXECUTABLE_EMPTY;
        if(state.executable[bi] &&
           state.input_kinds[bi] == ValueDataflowState::DIK_EXECUTABLE_EMPTY) {
          continue;
        }
      } else if(!have_multiple_predecessors) {
        const ValueEnvironment * predecessor_environment =
            &state.analyses[first_predecessor].out_environment;
        const ValueDebugLocationEnvironment * predecessor_debug_locations =
            context->track_debug_locations ?
                &state.analyses[first_predecessor].out_debug_locations :
                &empty_debug_locations;
        const BooleanTempSet * predecessor_boolean_temps =
            &state.analyses[first_predecessor].out_boolean_temps;
        const ExpressionCache * predecessor_expression_cache =
            &state.analyses[first_predecessor].out_expression_cache;
        new_executable = true;
        new_input_kind = ValueDataflowState::DIK_SINGLE_PREDECESSOR;
        new_input_single_predecessor = first_predecessor;
        new_input_single_predecessor_version = analysis_versions[first_predecessor];
        if(try_get_branch_edge_assumption(function.blocks[first_predecessor],
                                          block_label,
                                          *predecessor_environment,
                                          *predecessor_debug_locations,
                                          *predecessor_boolean_temps,
                                          new_input_assumed_temp,
                                          new_input_assumed_truth,
                                          fallback_debug_location,
                                          context->track_debug_locations)) {
          new_input_has_branch_assumption = true;
        }
        if(state.executable[bi] &&
           state.input_kinds[bi] == ValueDataflowState::DIK_SINGLE_PREDECESSOR &&
           state.input_single_predecessors[bi] == new_input_single_predecessor &&
           state.input_single_predecessor_versions[bi] ==
               new_input_single_predecessor_version &&
           state.input_has_branch_assumptions[bi] == new_input_has_branch_assumption &&
           (!new_input_has_branch_assumption ||
            (state.input_assumed_temps[bi] == new_input_assumed_temp &&
             state.input_assumed_truths[bi] == new_input_assumed_truth))) {
          continue;
        }
        if(new_input_has_branch_assumption) {
          single_in_environment = *predecessor_environment;
          if(context->track_debug_locations) {
            single_in_debug_locations = *predecessor_debug_locations;
          }
          single_in_boolean = *predecessor_boolean_temps;
          apply_branch_edge_assumptions(new_input_assumed_temp,
                                        new_input_assumed_truth,
                                        single_in_environment,
                                        single_in_debug_locations,
                                        single_in_boolean,
                                        context->track_debug_locations);
          input_environment = &single_in_environment;
          input_debug_locations =
              context->track_debug_locations ? &single_in_debug_locations
                                             : &empty_debug_locations;
          input_boolean_temps = &single_in_boolean;
        } else {
          input_environment = predecessor_environment;
          input_debug_locations = predecessor_debug_locations;
          input_boolean_temps = predecessor_boolean_temps;
        }
        input_expression_cache = predecessor_expression_cache;
      } else {
        incoming.reserve(incoming_edges.size());
        for(size_t pi = 0; pi < incoming_edges.size(); ++pi) {
          const IncomingBlockEdge & edge = incoming_edges[pi];
          const size_t predecessor = edge.predecessor;
          if(!state.executable[predecessor] ||
             state.analyses[predecessor].executable_successors.count(block_label) == 0) {
            continue;
          }

          IncomingValueState adjusted;
          adjusted.predecessor = predecessor;
          adjusted.environment = &state.analyses[predecessor].out_environment;
          adjusted.debug_locations =
              context->track_debug_locations ?
                  &state.analyses[predecessor].out_debug_locations :
                  &empty_debug_locations;
          adjusted.boolean_temps = &state.analyses[predecessor].out_boolean_temps;
          adjusted.expression_cache = &state.analyses[predecessor].out_expression_cache;
          string assumed_temp;
          bool assumed_truth = false;
          if(try_get_branch_edge_assumption(function.blocks[predecessor],
                                            block_label,
                                            *adjusted.environment,
                                            *adjusted.debug_locations,
                                            *adjusted.boolean_temps,
                                            assumed_temp,
                                            assumed_truth,
                                            fallback_debug_location,
                                            context->track_debug_locations)) {
            adjusted.has_branch_assumption = true;
            adjusted.assumed_temp = assumed_temp;
            adjusted.assumed_truth = assumed_truth;
            adjusted.use_owned_value_state = true;
            adjusted.owned_environment = *adjusted.environment;
            if(context->track_debug_locations) {
              adjusted.owned_debug_locations = *adjusted.debug_locations;
            }
            adjusted.owned_boolean_temps = *adjusted.boolean_temps;
            apply_branch_edge_assumptions(assumed_temp,
                                          assumed_truth,
                                          adjusted.owned_environment,
                                          adjusted.owned_debug_locations,
                                          adjusted.owned_boolean_temps,
                                          context->track_debug_locations);
          }
          incoming.push_back(std::move(adjusted));
        }
        new_executable = true;
        new_input_kind = ValueDataflowState::DIK_MERGED;
        pair<ValueEnvironment, ValueDebugLocationEnvironment> merged =
            meet_incoming_value_environments(incoming, context->track_debug_locations);
        merged_in = std::move(merged.first);
        if(context->track_debug_locations) {
          merged_in_debug_locations = std::move(merged.second);
        }
        merged_in_boolean = meet_incoming_boolean_temp_sets(incoming);
        if(context->track_analysis_expression_cache) {
          merged_in_cache = meet_incoming_expression_caches(incoming);
        }
        if(state.executable[bi] &&
           state.input_kinds[bi] == ValueDataflowState::DIK_MERGED &&
           value_environment_equal(merged_in, state.in_environments[bi]) &&
           (!context->track_debug_locations ||
            value_debug_location_environment_equal(merged_in_debug_locations,
                                                  state.in_debug_locations[bi])) &&
           merged_in_boolean == state.in_boolean_temps[bi] &&
           (!context->track_analysis_expression_cache ||
            expression_cache_equal(merged_in_cache, state.in_expression_caches[bi]))) {
          continue;
        }
        input_environment = &merged_in;
        input_debug_locations =
            context->track_debug_locations ? &merged_in_debug_locations : &empty_debug_locations;
        input_boolean_temps = &merged_in_boolean;
        input_expression_cache =
            context->track_analysis_expression_cache ? &merged_in_cache : &empty_expression_cache;
      }
    }

    const bool old_executable = state.executable[bi];
    state.executable[bi] = new_executable;
    state.input_kinds[bi] = new_input_kind;
    state.input_single_predecessors[bi] = new_input_single_predecessor;
    state.input_single_predecessor_versions[bi] = new_input_single_predecessor_version;
    state.input_has_branch_assumptions[bi] = new_input_has_branch_assumption;
    state.input_assumed_temps[bi] = new_input_assumed_temp;
    state.input_assumed_truths[bi] = new_input_assumed_truth;
    if(new_input_kind == ValueDataflowState::DIK_MERGED) {
      state.in_environments[bi] = std::move(merged_in);
      if(context->track_debug_locations) {
        state.in_debug_locations[bi] = std::move(merged_in_debug_locations);
      } else if(!state.in_debug_locations[bi].empty()) {
        state.in_debug_locations[bi].clear();
      }
      state.in_boolean_temps[bi] = std::move(merged_in_boolean);
      state.in_expression_caches[bi] = std::move(merged_in_cache);
    } else if(new_input_kind != ValueDataflowState::DIK_MERGED &&
              (!state.in_environments[bi].empty() ||
               !state.in_debug_locations[bi].empty() ||
               !state.in_boolean_temps[bi].empty() ||
               !state.in_expression_caches[bi].empty())) {
      state.in_environments[bi].clear();
      state.in_debug_locations[bi].clear();
      state.in_boolean_temps[bi].clear();
      state.in_expression_caches[bi].clear();
    }

    ValueBlockAnalysis new_analysis;
    if(state.executable[bi]) {
      new_analysis = analyze_value_block(
          function.blocks[bi],
          *input_environment,
          *input_debug_locations,
          *input_boolean_temps,
          *input_expression_cache,
          fallback_debug_location,
          context->track_analysis_expression_cache,
          context->track_analysis_reassociation,
          context->track_debug_locations);
    }

    const bool outputs_changed = !value_block_analysis_equal(new_analysis, state.analyses[bi]);
    state.analyses[bi] = std::move(new_analysis);
    if(old_executable != state.executable[bi] || outputs_changed) {
      ++analysis_versions[bi];
    }

    if(old_executable != state.executable[bi] || outputs_changed) {
      for(size_t si = 0; si < context->control_flow.successors[bi].size(); ++si) {
        const size_t successor = context->control_flow.successors[bi][si];
        if(!queued[successor]) {
          queued[successor] = true;
          worklist.push_back(successor);
        }
      }
    }
  }

  return state;
}

bool instruction_has_inline_unsupported_semantics(const lir::Instruction & instruction)
{
  switch(instruction.kind) {
    case lir::Instruction::IK_EH_TRY:
    case lir::Instruction::IK_EH_CLEANUP:
    case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
    case lir::Instruction::IK_EH_CATCH:
    case lir::Instruction::IK_EH_FILTER:
    case lir::Instruction::IK_EH_CATCH_ALL:
    case lir::Instruction::IK_EH_END:
    case lir::Instruction::IK_VA_START:
    case lir::Instruction::IK_VA_ARG:
    case lir::Instruction::IK_STACK_ALLOC:
    case lir::Instruction::IK_THROW:
    case lir::Instruction::IK_EXCEPTION:
    case lir::Instruction::IK_EXCEPTION_SELECTOR:
    case lir::Instruction::IK_RESUME:
      return true;
    default:
      return false;
  }
}

bool block_has_inline_unsupported_semantics(const lir::Block & block)
{
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    if(instruction_has_inline_unsupported_semantics(block.instructions[i])) {
      return true;
    }
  }
  return false;
}

bool block_has_only_eh_try_end_inline_unsupported_semantics(const lir::Block & block)
{
  bool saw_eh_marker = false;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    if(!instruction_has_inline_unsupported_semantics(block.instructions[i])) {
      continue;
    }
    if(block.instructions[i].kind != lir::Instruction::IK_EH_TRY &&
       block.instructions[i].kind != lir::Instruction::IK_EH_END) {
      return false;
    }
    saw_eh_marker = true;
  }
  return saw_eh_marker;
}

size_t block_eh_depth_before_instruction(const lir::Block & block,
                                         size_t instruction_index)
{
  size_t depth = 0;
  const size_t end = std::min(instruction_index, block.instructions.size());
  for(size_t i = 0; i < end; ++i) {
    const lir::Instruction & instruction = block.instructions[i];
    if(instruction_starts_eh_region(instruction)) {
      ++depth;
    } else if(instruction.kind == lir::Instruction::IK_EH_END && depth != 0) {
      --depth;
    }
  }
  return depth;
}

bool block_eh_region_suffix_closes_without_may_unwind(
    const lir::Block & block,
    size_t instruction_index,
    size_t depth,
    const FunctionBoundaryMap & function_boundaries)
{
  for(size_t i = instruction_index; i < block.instructions.size() && depth != 0; ++i) {
    const lir::Instruction & instruction = block.instructions[i];
    if(instruction_may_unwind(instruction, function_boundaries)) {
      return false;
    }
    if(instruction_starts_eh_region(instruction)) {
      ++depth;
    } else if(instruction.kind == lir::Instruction::IK_EH_END) {
      --depth;
    }
  }
  return depth == 0;
}

bool function_allows_eh_region_multi_block_inline(const lir::Function & function,
                                                  const lir::Block & block,
                                                  const lir::Function & callee)
{
  return callee.blocks.size() <= 4 &&
         function.blocks.size() <= 8 &&
         function_instruction_count(function) <= 96 &&
         block.instructions.size() <= 8;
}

size_t function_instruction_count(const lir::Function & function)
{
  size_t count = 0;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    count += function.blocks[i].instructions.size();
  }
  return count;
}

size_t function_return_count(const lir::Function & function)
{
  size_t count = 0;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    for(size_t j = 0; j < function.blocks[i].instructions.size(); ++j) {
      if(function.blocks[i].instructions[j].kind == lir::Instruction::IK_RETURN) {
        ++count;
      }
    }
  }
  return count;
}

bool function_has_cfg_cycle(const lir::Function & function)
{
  if(function.blocks.empty()) {
    return false;
  }

  const FunctionOptimizationContext context = collect_function_optimization_context(function);

  vector<int> color(function.blocks.size(), 0);
  const std::function<bool(size_t)> visit = [&](size_t bi) -> bool
  {
    color[bi] = 1;
    for(size_t si = 0; si < context.control_flow.successors[bi].size(); ++si) {
      const size_t successor = context.control_flow.successors[bi][si];
      if(color[successor] == 1) {
        return true;
      }
      if(color[successor] == 0 && visit(successor)) {
        return true;
      }
    }
    color[bi] = 2;
    return false;
  };

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    if(color[bi] == 0 && visit(bi)) {
      return true;
    }
  }
  return false;
}

bool function_is_effectively_nonthrowing_inline_candidate(
    const lir::Function & function,
    const FunctionBoundaryMap & function_boundaries)
{
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & instruction = function.blocks[bi].instructions[ii];
      if(instruction.kind == lir::Instruction::IK_CALL) {
        const lir::FunctionBoundaryMetadata boundary =
            resolved_call_boundary(instruction, function_boundaries);
        if(boundary.unwind != lir::CUM_NO ||
           boundary.returns == lir::CRM_NORETURN) {
          return false;
        }
        continue;
      }
      if(instruction_has_inline_unsupported_semantics(instruction)) {
        return false;
      }
    }
  }
  return true;
}

bool function_is_small_inline_candidate(const lir::Function & function)
{
  size_t max_inline_blocks = 4;
  size_t max_inline_instructions = 24;

  // Prefer-local helpers are typically small accessors or wrappers that are
  // intended to stay intra-object. Give them a modestly larger O1 budget so
  // branchy stdlib accessors and tiny hash helpers like basic_string::data or
  // __constrain_hash can inline.
  if(function.metadata.prefer_local_object_binding) {
    max_inline_blocks = 7;
    max_inline_instructions = 40;
  } else if(function.boundary.unwind == lir::CUM_NO) {
    max_inline_blocks = 5;
    max_inline_instructions = 32;
  }

  if(function.blocks.empty() ||
     function.blocks.size() > max_inline_blocks ||
     function_instruction_count(function) > max_inline_instructions ||
     function_return_count(function) == 0 ||
     function_has_cfg_cycle(function)) {
    return false;
  }

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    if(function.blocks[bi].instructions.empty() ||
       block_has_inline_unsupported_semantics(function.blocks[bi])) {
      return false;
    }
  }
  return true;
}

unordered_set<string> find_recursive_function_names(
    const vector<lir::Function> & functions)
{
  unordered_map<string, size_t> function_indexes;
  for(size_t i = 0; i < functions.size(); ++i) {
    function_indexes[functions[i].name] = i;
  }

  vector<vector<size_t> > outgoing(functions.size());
  vector<bool> has_self_edge(functions.size(), false);
  for(size_t i = 0; i < functions.size(); ++i) {
    unordered_set<size_t> seen_targets;
    for(size_t bi = 0; bi < functions[i].blocks.size(); ++bi) {
      for(size_t ii = 0; ii < functions[i].blocks[bi].instructions.size(); ++ii) {
        const lir::Instruction & instruction = functions[i].blocks[bi].instructions[ii];
        if(instruction.kind != lir::Instruction::IK_CALL ||
           instruction.first.kind != lir::Operand::OP_GLOBAL) {
          continue;
        }
        const unordered_map<string, size_t>::const_iterator found =
            function_indexes.find(instruction.first.text);
        if(found == function_indexes.end()) {
          continue;
        }
        if(found->second == i) {
          has_self_edge[i] = true;
        }
        if(seen_targets.insert(found->second).second) {
          outgoing[i].push_back(found->second);
        }
      }
    }
  }

  vector<int> indexes(functions.size(), -1);
  vector<int> lowlinks(functions.size(), 0);
  vector<bool> on_stack(functions.size(), false);
  vector<size_t> stack;
  unordered_set<string> recursive_functions;
  int next_index = 0;

  const function<void(size_t)> strongconnect = [&](size_t v)
  {
    indexes[v] = next_index;
    lowlinks[v] = next_index;
    ++next_index;
    stack.push_back(v);
    on_stack[v] = true;

    for(size_t i = 0; i < outgoing[v].size(); ++i) {
      const size_t w = outgoing[v][i];
      if(indexes[w] == -1) {
        strongconnect(w);
        lowlinks[v] = std::min(lowlinks[v], lowlinks[w]);
      } else if(on_stack[w]) {
        lowlinks[v] = std::min(lowlinks[v], indexes[w]);
      }
    }

    if(lowlinks[v] != indexes[v]) {
      return;
    }

    vector<size_t> component;
    while(!stack.empty()) {
      const size_t w = stack.back();
      stack.pop_back();
      on_stack[w] = false;
      component.push_back(w);
      if(w == v) {
        break;
      }
    }

    if(component.size() > 1) {
      for(size_t i = 0; i < component.size(); ++i) {
        recursive_functions.insert(functions[component[i]].name);
      }
      return;
    }

    if(component.size() == 1 && has_self_edge[component[0]]) {
      recursive_functions.insert(functions[component[0]].name);
    }
  };

  for(size_t i = 0; i < functions.size(); ++i) {
    if(indexes[i] == -1) {
      strongconnect(i);
    }
  }
  return recursive_functions;
}

bool function_allows_o1_inline_expansion(const lir::Function & function)
{
  size_t max_inline_caller_blocks = 96;
  size_t max_inline_caller_instructions = 768;

  // Prefer-local helpers are often tiny wrappers around object-local accessors,
  // so let them inline into slightly larger callers before cutting off growth.
  if(function.metadata.prefer_local_object_binding) {
    max_inline_caller_blocks = 128;
    max_inline_caller_instructions = 1024;
  }

  return function.blocks.size() <= max_inline_caller_blocks &&
         function_instruction_count(function) <= max_inline_caller_instructions;
}

bool call_has_inlineable_object_copy_consumer(const lir::Instruction & call,
                                              const lir::Instruction * next_instruction)
{
  if(call.kind != lir::Instruction::IK_CALL ||
     call.call_returns_void ||
     call.dest.empty() ||
     !lir::is_object_type(call.type) ||
     next_instruction == nullptr ||
     next_instruction->kind != lir::Instruction::IK_COPYOBJ ||
     next_instruction->first.kind != lir::Operand::OP_TEMP ||
     next_instruction->first.text != call.dest) {
    return false;
  }

  return next_instruction->byte_count == lir::type_size(call.type) &&
         next_instruction->byte_alignment == lir::type_alignment(call.type);
}

bool call_matches_inline_callee(const lir::Instruction & call,
                                const lir::Instruction * next_instruction,
                                const lir::Function & callee,
                                const unordered_set<string> & recursive_functions)
{
  if(call.kind != lir::Instruction::IK_CALL ||
     call.first.kind != lir::Operand::OP_GLOBAL ||
     call.has_call_signature ||
     callee.boundary.arity != lir::CAM_FIXED ||
     call.args.size() != callee.params.size() ||
     callee.boundary.returns == lir::CRM_NORETURN) {
    return false;
  }

  if(call.call_returns_void != (callee.return_type.text == "void")) {
    return false;
  }

  if(!call.call_returns_void) {
    if(call.type.text != callee.return_type.text) {
      return false;
    }
    if(lir::is_object_type(callee.return_type) &&
       !call_has_inlineable_object_copy_consumer(call, next_instruction)) {
      return false;
    }
  }

  if(recursive_functions.count(callee.name) != 0) {
    return false;
  }

  return function_is_small_inline_candidate(callee);
}

string make_inline_name(const string & original, size_t inline_site_id)
{
  if(original.empty()) {
    return original;
  }
  return string(1, original[0]) + "__o1inl" + to_string(inline_site_id) +
         "__" + original.substr(1);
}

string make_inline_continuation_label(size_t inline_site_id)
{
  return "^__o1inl" + to_string(inline_site_id) + "__cont";
}

void rewrite_inlined_operand(
    lir::Operand & operand,
    const unordered_map<string, lir::Operand> & parameter_operands,
    const unordered_map<string, string> & renamed_temps,
    const unordered_map<string, string> & renamed_slots,
    const unordered_map<string, string> & renamed_labels)
{
  switch(operand.kind) {
    case lir::Operand::OP_TEMP: {
      const unordered_map<string, lir::Operand>::const_iterator param_found =
          parameter_operands.find(operand.text);
      if(param_found != parameter_operands.end()) {
        operand = param_found->second;
        return;
      }
      const unordered_map<string, string>::const_iterator temp_found =
          renamed_temps.find(operand.text);
      if(temp_found != renamed_temps.end()) {
        operand.text = temp_found->second;
      }
      return;
    }
    case lir::Operand::OP_SLOT: {
      const unordered_map<string, string>::const_iterator slot_found =
          renamed_slots.find(operand.text);
      if(slot_found != renamed_slots.end()) {
        operand.text = slot_found->second;
      }
      return;
    }
    case lir::Operand::OP_LABEL: {
      const unordered_map<string, string>::const_iterator label_found =
          renamed_labels.find(operand.text);
      if(label_found != renamed_labels.end()) {
        operand.text = label_found->second;
      }
      return;
    }
    default:
      return;
  }
}

void rewrite_inlined_instruction(
    lir::Instruction & instruction,
    const unordered_map<string, lir::Operand> & parameter_operands,
    const unordered_map<string, string> & renamed_temps,
    const unordered_map<string, string> & renamed_slots,
    const unordered_map<string, string> & renamed_labels)
{
  if(!instruction.dest.empty()) {
    const unordered_map<string, string>::const_iterator dest_found =
        renamed_temps.find(instruction.dest);
    if(dest_found != renamed_temps.end()) {
      instruction.dest = dest_found->second;
    }
  }

  rewrite_inlined_operand(instruction.first,
                          parameter_operands,
                          renamed_temps,
                          renamed_slots,
                          renamed_labels);
  rewrite_inlined_operand(instruction.second,
                          parameter_operands,
                          renamed_temps,
                          renamed_slots,
                          renamed_labels);
  rewrite_inlined_operand(instruction.third,
                          parameter_operands,
                          renamed_temps,
                          renamed_slots,
                          renamed_labels);
  for(size_t i = 0; i < instruction.args.size(); ++i) {
    rewrite_inlined_operand(instruction.args[i],
                            parameter_operands,
                            renamed_temps,
                            renamed_slots,
                            renamed_labels);
  }
}

bool inline_direct_call_at(lir::Function & function,
                           size_t block_index,
                           size_t instruction_index,
                           const lir::Function & callee,
                           const FunctionBoundaryMap & function_boundaries,
                           size_t inline_site_id,
                           bool block_has_incoming_eh_edge)
{
  const lir::Instruction call = function.blocks[block_index].instructions[instruction_index];
  const lir::Block original_block = function.blocks[block_index];
  if(instruction_index + 1 >= original_block.instructions.size()) {
    return false;
  }
  // Landing-pad blocks have an implicit EH region in the object backend.
  // The general inliner splits the caller block, which can move the matching
  // eh_end into a continuation block and make that implicit pop underflow.
  if(block_has_incoming_eh_edge) {
    return false;
  }

  const bool returns_object =
      !call.call_returns_void && lir::is_object_type(call.type);
  const bool has_object_copy_consumer =
      returns_object &&
      call_has_inlineable_object_copy_consumer(call,
                                               &original_block.instructions[instruction_index + 1]);
  if(returns_object && !has_object_copy_consumer) {
    return false;
  }
  const size_t consumed_instruction_count = has_object_copy_consumer ? 2 : 1;
  if(instruction_index + consumed_instruction_count > original_block.instructions.size()) {
    return false;
  }
  const lir::Instruction * object_copy_consumer =
      has_object_copy_consumer ? &original_block.instructions[instruction_index + 1] : nullptr;
  const lir::Operand object_copy_target =
      object_copy_consumer != nullptr ? object_copy_consumer->second : lir::Operand();

  unordered_map<string, lir::Operand> parameter_operands;
  for(size_t i = 0; i < callee.params.size(); ++i) {
    parameter_operands[callee.params[i].name] = call.args[i];
  }

  unordered_map<string, string> renamed_slots;
  unordered_set<string> used_renamed_slot_names;
  for(size_t i = 0; i < callee.slots.size(); ++i) {
    const string renamed_slot_name =
        make_inline_name(callee.slots[i].first, inline_site_id);
    renamed_slots[callee.slots[i].first] = renamed_slot_name;
    used_renamed_slot_names.insert(renamed_slot_name);
  }

  unordered_map<string, string> renamed_labels;
  for(size_t i = 0; i < callee.blocks.size(); ++i) {
    renamed_labels[callee.blocks[i].label] =
        make_inline_name(callee.blocks[i].label, inline_site_id);
  }

  unordered_map<string, string> renamed_temps;
  for(size_t bi = 0; bi < callee.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < callee.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & instruction = callee.blocks[bi].instructions[ii];
      if(!instruction.dest.empty()) {
        renamed_temps[instruction.dest] =
            make_inline_name(instruction.dest, inline_site_id);
      }
    }
  }

  if(block_has_inline_unsupported_semantics(original_block)) {
    if(!block_has_only_eh_try_end_inline_unsupported_semantics(original_block)) {
      return false;
    }

    const size_t eh_depth_at_call =
        block_eh_depth_before_instruction(original_block, instruction_index);
    const bool call_is_in_eh_region = eh_depth_at_call != 0;
    const bool callee_is_effectively_nonthrowing =
        function_is_effectively_nonthrowing_inline_candidate(callee,
                                                            function_boundaries);
    if(call_is_in_eh_region &&
       callee.blocks.size() == 1 &&
       function_return_count(callee) == 1 &&
       callee_is_effectively_nonthrowing) {
      vector<pair<string, lir::LowType> > appended_slots;
      appended_slots.reserve(callee.slots.size());
      for(size_t i = 0; i < callee.slots.size(); ++i) {
        appended_slots.push_back(
            make_pair(renamed_slots[callee.slots[i].first], callee.slots[i].second));
      }

      vector<lir::Instruction> spliced_instructions;
      const lir::Block & callee_block = callee.blocks.front();
      spliced_instructions.reserve(callee_block.instructions.size());
      for(size_t ii = 0; ii < callee_block.instructions.size(); ++ii) {
        const lir::Instruction & original_instruction = callee_block.instructions[ii];
        if(original_instruction.kind == lir::Instruction::IK_RETURN) {
          if(has_object_copy_consumer) {
            lir::Instruction copyobj =
                make_copyobj_instruction(object_copy_consumer->byte_count,
                                         object_copy_consumer->byte_alignment,
                                         original_instruction.first,
                                         object_copy_target,
                                         &original_instruction);
            rewrite_inlined_instruction(copyobj,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            // The inlined aggregate return materializes directly into caller
            // storage, so the destination must remain the caller operand.
            copyobj.second = object_copy_target;
            spliced_instructions.push_back(copyobj);
          } else if(!call.call_returns_void && !call.dest.empty()) {
            lir::Instruction copy =
                make_copy_instruction(call.dest, call.type.text, original_instruction.first,
                                      &original_instruction);
            rewrite_inlined_instruction(copy,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            // The inlined scalar return writes back into the caller's
            // destination temp, so it must not be remapped through callee
            // renaming even when the names collide.
            copy.dest = call.dest;
            simplify_copy_instruction(copy);
            spliced_instructions.push_back(copy);
          }
          continue;
        }

        lir::Instruction cloned_instruction = original_instruction;
        rewrite_inlined_instruction(cloned_instruction,
                                    parameter_operands,
                                    renamed_temps,
                                    renamed_slots,
                                    renamed_labels);
        spliced_instructions.push_back(cloned_instruction);
      }

      lir::Block rebuilt_block;
      rebuilt_block.label = original_block.label;
      rebuilt_block.instructions.reserve(original_block.instructions.size() +
                                         spliced_instructions.size());
      rebuilt_block.instructions.insert(rebuilt_block.instructions.end(),
                                        original_block.instructions.begin(),
                                        original_block.instructions.begin() +
                                            instruction_index);
      rebuilt_block.instructions.insert(rebuilt_block.instructions.end(),
                                        spliced_instructions.begin(),
                                        spliced_instructions.end());
      rebuilt_block.instructions.insert(rebuilt_block.instructions.end(),
                                        original_block.instructions.begin() +
                                            instruction_index + consumed_instruction_count,
                                        original_block.instructions.end());

      function.slots.insert(function.slots.end(), appended_slots.begin(), appended_slots.end());
      function.blocks[block_index] = rebuilt_block;
      return true;
    }

    if(call_is_in_eh_region &&
       (!callee_is_effectively_nonthrowing ||
        !function_allows_eh_region_multi_block_inline(function,
                                                      original_block,
                                                      callee) ||
        !block_eh_region_suffix_closes_without_may_unwind(
            original_block,
            instruction_index + consumed_instruction_count,
            eh_depth_at_call,
            function_boundaries))) {
      return false;
    }
  }

  vector<pair<string, lir::LowType> > appended_slots;
  appended_slots.reserve(callee.slots.size());
  for(size_t i = 0; i < callee.slots.size(); ++i) {
    appended_slots.push_back(
        make_pair(renamed_slots[callee.slots[i].first], callee.slots[i].second));
  }

  const bool use_scalar_return_slot =
      !call.call_returns_void && !returns_object && !call.dest.empty() &&
      function_return_count(callee) > 1;
  const bool use_object_return_slot =
      has_object_copy_consumer && function_return_count(callee) > 1;
  string return_slot_name;
  if(use_scalar_return_slot || use_object_return_slot) {
    const string return_slot_base =
        returns_object ? "$retmergeobj__1" : "$retmerge__1";
    return_slot_name = make_inline_name(return_slot_base, inline_site_id);
    for(size_t disambiguator = 1;
        used_renamed_slot_names.count(return_slot_name) != 0;
        ++disambiguator) {
      return_slot_name =
          make_inline_name(return_slot_base, inline_site_id) +
          "__" + to_string(disambiguator);
    }
    appended_slots.push_back(make_pair(return_slot_name, call.type));
  }

  const string continuation_label = make_inline_continuation_label(inline_site_id);
  vector<lir::Block> inlined_blocks;
  inlined_blocks.reserve(callee.blocks.size());
  for(size_t bi = 0; bi < callee.blocks.size(); ++bi) {
    lir::Block cloned_block;
    cloned_block.label = renamed_labels[callee.blocks[bi].label];
    for(size_t ii = 0; ii < callee.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & original_instruction = callee.blocks[bi].instructions[ii];
      if(original_instruction.kind == lir::Instruction::IK_RETURN) {
        if(has_object_copy_consumer) {
          if(use_object_return_slot) {
            lir::Instruction copyobj =
                make_copyobj_instruction(object_copy_consumer->byte_count,
                                         object_copy_consumer->byte_alignment,
                                         original_instruction.first,
                                         make_slot_operand(return_slot_name),
                                         &original_instruction);
            rewrite_inlined_instruction(copyobj,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            cloned_block.instructions.push_back(copyobj);
          } else {
            lir::Instruction copyobj =
                make_copyobj_instruction(object_copy_consumer->byte_count,
                                         object_copy_consumer->byte_alignment,
                                         original_instruction.first,
                                         object_copy_target,
                                         &original_instruction);
            rewrite_inlined_instruction(copyobj,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            // The synthetic return-to-caller copy writes into caller storage,
            // so the destination must not be remapped through callee renaming.
            copyobj.second = object_copy_target;
            cloned_block.instructions.push_back(copyobj);
          }
        } else if(!call.call_returns_void && !call.dest.empty()) {
          if(use_scalar_return_slot) {
            lir::Instruction store =
                make_store_instruction(call.type.text,
                                       original_instruction.first,
                                       make_slot_operand(return_slot_name),
                                       &original_instruction);
            rewrite_inlined_instruction(store,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            cloned_block.instructions.push_back(store);
          } else {
            lir::Instruction copy =
                make_copy_instruction(call.dest, call.type.text, original_instruction.first,
                                      &original_instruction);
            rewrite_inlined_instruction(copy,
                                        parameter_operands,
                                        renamed_temps,
                                        renamed_slots,
                                        renamed_labels);
            // The synthetic return-to-call-dest copy writes back into the
            // caller's destination temp, so it must not be remapped through
            // callee temp renaming even when the names collide.
            copy.dest = call.dest;
            simplify_copy_instruction(copy);
            cloned_block.instructions.push_back(copy);
          }
        }
        cloned_block.instructions.push_back(
            make_jump_instruction(continuation_label, &original_instruction));
        continue;
      }

      lir::Instruction cloned_instruction = original_instruction;
      rewrite_inlined_instruction(cloned_instruction,
                                  parameter_operands,
                                  renamed_temps,
                                  renamed_slots,
                                  renamed_labels);
      cloned_block.instructions.push_back(cloned_instruction);
    }
    inlined_blocks.push_back(cloned_block);
  }

  lir::Block prefix_block = original_block;
  prefix_block.instructions.resize(instruction_index);
  prefix_block.instructions.push_back(
      make_jump_instruction(inlined_blocks.front().label, &call));

  lir::Block continuation_block;
  continuation_block.label = continuation_label;
  if(use_scalar_return_slot) {
    continuation_block.instructions.push_back(
        make_load_instruction(call.dest, call.type.text, make_slot_operand(return_slot_name),
                              &call));
  }
  if(use_object_return_slot) {
    continuation_block.instructions.push_back(
        make_copyobj_instruction(object_copy_consumer->byte_count,
                                 object_copy_consumer->byte_alignment,
                                 make_slot_operand(return_slot_name),
                                 object_copy_target,
                                 object_copy_consumer));
  }
  continuation_block.instructions.insert(continuation_block.instructions.end(),
                                         original_block.instructions.begin() +
                                             instruction_index + consumed_instruction_count,
                                         original_block.instructions.end());

  vector<lir::Block> rebuilt_blocks;
  rebuilt_blocks.reserve(function.blocks.size() + inlined_blocks.size() + 1);
  for(size_t i = 0; i < block_index; ++i) {
    rebuilt_blocks.push_back(function.blocks[i]);
  }
  rebuilt_blocks.push_back(prefix_block);
  rebuilt_blocks.insert(rebuilt_blocks.end(), inlined_blocks.begin(), inlined_blocks.end());
  rebuilt_blocks.push_back(continuation_block);
  for(size_t i = block_index + 1; i < function.blocks.size(); ++i) {
    rebuilt_blocks.push_back(function.blocks[i]);
  }

  function.slots.insert(function.slots.end(), appended_slots.begin(), appended_slots.end());
  function.blocks.swap(rebuilt_blocks);
  return true;
}

bool inline_small_direct_calls(lir::Function & function,
                               const FunctionBoundaryMap & function_boundaries,
                               const FunctionDefinitionMap & function_definitions,
                               const unordered_set<string> & recursive_functions,
                               size_t & next_inline_site_id)
{
  const FunctionControlFlow control_flow = build_function_control_flow(function);
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    const lir::Block & block = function.blocks[bi];
    bool block_has_incoming_eh_edge = false;
    if(bi < control_flow.incoming_edges.size()) {
      const vector<IncomingBlockEdge> & incoming_edges =
          control_flow.incoming_edges[bi];
      for(size_t ei = 0; ei < incoming_edges.size(); ++ei) {
        if(incoming_edges[ei].has_eh_edge) {
          block_has_incoming_eh_edge = true;
          break;
        }
      }
    }
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & instruction = block.instructions[ii];
      if(instruction.kind != lir::Instruction::IK_CALL ||
         instruction.first.kind != lir::Operand::OP_GLOBAL) {
        continue;
      }
      const lir::Instruction * next_instruction =
          ii + 1 < block.instructions.size() ? &block.instructions[ii + 1] : nullptr;

      const FunctionDefinitionMap::const_iterator found =
          function_definitions.find(instruction.first.text);
      if(found == function_definitions.end() ||
         found->second == nullptr ||
         found->second->name == function.name ||
         !call_matches_inline_callee(instruction,
                                     next_instruction,
                                     *found->second,
                                     recursive_functions)) {
        continue;
      }

      if(inline_direct_call_at(function,
                               bi,
                               ii,
                               *found->second,
                               function_boundaries,
                               next_inline_site_id,
                               block_has_incoming_eh_edge)) {
        ++next_inline_site_id;
        return true;
      }
    }
  }
  return false;
}

bool run_o1_cleanup_pipeline(lir::Function & function,
                             const FunctionBoundaryMap & function_boundaries)
{
  bool changed = false;
  bool local_change = false;
  do {
    local_change = false;
    bool propagation_or_cfg_change = false;
    propagation_or_cfg_change |=
        remove_nonthrowing_eh_markers(function, function_boundaries);
    const FunctionOptimizationContext context =
        collect_function_optimization_context(function);
    propagation_or_cfg_change |= propagate_known_values_across_blocks(function, &context);
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      propagation_or_cfg_change |= simplify_block_terminator(function, context.block_index, i);
    }
    BlockIndexMap structural_block_index = build_block_index_map(function);
    FunctionControlFlow structural_control_flow =
        build_function_control_flow(function, structural_block_index);
    const bool collapsed_empty_branches =
        collapse_empty_branch_diamonds(function,
                                       structural_block_index,
                                       structural_control_flow);
    propagation_or_cfg_change |= collapsed_empty_branches;
    if(collapsed_empty_branches) {
      structural_block_index = build_block_index_map(function);
      structural_control_flow =
          build_function_control_flow(function, structural_block_index);
    }
    const bool removed_unreachable =
        remove_unreachable_blocks(function, &structural_control_flow);
    propagation_or_cfg_change |= removed_unreachable;
    if(removed_unreachable) {
      structural_block_index = build_block_index_map(function);
      structural_control_flow =
          build_function_control_flow(function, structural_block_index);
    }
    const bool merged_blocks =
        merge_straight_line_blocks(function,
                                   &structural_control_flow,
                                   &structural_block_index);
    propagation_or_cfg_change |= merged_blocks;
    if(merged_blocks) {
      structural_control_flow = build_function_control_flow(function);
    }
    const bool dead_code_change =
        eliminate_dead_code(function, function_boundaries, &structural_control_flow);
    const bool dead_slot_store_change = remove_stores_to_unread_slots(function);
    const bool unused_slot_change = remove_unused_slots(function);
    const bool cleanup_change =
        dead_code_change || dead_slot_store_change || unused_slot_change;
    local_change = propagation_or_cfg_change || cleanup_change;
    changed |= local_change;
  } while(local_change);
  return changed;
}

struct SlotState
{
  enum Kind
  {
    SS_UNSET,
    SS_KNOWN,
    SS_UNKNOWN
  } kind = SS_UNSET;

  lir::Operand value;
};

bool slot_state_equal(const SlotState & lhs, const SlotState & rhs)
{
  return lhs.kind == rhs.kind &&
         (lhs.kind != SlotState::SS_KNOWN || operand_equal(lhs.value, rhs.value));
}

SlotState unknown_slot_state()
{
  SlotState state;
  state.kind = SlotState::SS_UNKNOWN;
  return state;
}

SlotState known_slot_state(const lir::Operand & value)
{
  SlotState state;
  state.kind = SlotState::SS_KNOWN;
  state.value = value;
  return state;
}

SlotState meet_slot_states(const SlotState & lhs, const SlotState & rhs)
{
  if(lhs.kind == SlotState::SS_UNKNOWN || rhs.kind == SlotState::SS_UNKNOWN) {
    return unknown_slot_state();
  }
  if(lhs.kind == SlotState::SS_UNSET) {
    return rhs.kind == SlotState::SS_UNSET ? lhs : unknown_slot_state();
  }
  if(rhs.kind == SlotState::SS_UNSET) {
    return unknown_slot_state();
  }
  if(operand_equal(lhs.value, rhs.value)) {
    return lhs;
  }
  return unknown_slot_state();
}

bool is_stable_slot_value_operand(const lir::Operand & operand,
                                  const set<string> & function_symbols)
{
  if(operand.kind == lir::Operand::OP_TEMP ||
     operand.kind == lir::Operand::OP_INTEGER ||
     operand.kind == lir::Operand::OP_FLOAT ||
     operand.kind == lir::Operand::OP_LABEL) {
    return true;
  }
  return operand.kind == lir::Operand::OP_GLOBAL &&
         function_symbols.count(operand.text) != 0;
}

struct SlotUsageSummary
{
  bool promotable = true;
  bool seen_any_use = false;
};

enum SlotPromotionScope
{
  SPS_ALL,
  SPS_INLINE_ONLY
};

bool is_inline_generated_slot_name(const string & slot_name)
{
  return slot_name.find("__o1inl") != string::npos;
}

bool slot_allowed_for_scope(const string & slot_name,
                            SlotPromotionScope scope)
{
  return scope == SPS_ALL || is_inline_generated_slot_name(slot_name);
}

bool function_has_inline_generated_slots(const lir::Function & function)
{
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(is_inline_generated_slot_name(function.slots[i].first)) {
      return true;
    }
  }
  return false;
}

size_t function_inline_generated_slot_count(const lir::Function & function)
{
  size_t count = 0;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(is_inline_generated_slot_name(function.slots[i].first)) {
      ++count;
    }
  }
  return count;
}

bool should_limit_full_slot_promotion(const lir::Function & function)
{
  const size_t block_count = function.blocks.size();
  const size_t slot_count = function.slots.size();
  const size_t instruction_count = function_instruction_count(function);
  return block_count > 6 ||
         slot_count > 12 ||
         instruction_count > 96 ||
         slot_count * instruction_count > 384;
}

bool should_skip_inline_slot_promotion(const lir::Function & function)
{
  const size_t block_count = function.blocks.size();
  const size_t instruction_count = function_instruction_count(function);
  const size_t inline_slot_count = function_inline_generated_slot_count(function);
  return inline_slot_count == 0 ||
         block_count > 16 ||
         inline_slot_count > 12 ||
         instruction_count > 256 ||
         inline_slot_count * instruction_count > 768;
}

struct SlotBlockAnalysis
{
  SlotState out_state;
  unordered_set<string> executable_successors;
  map<string, SlotState> eh_edge_states;
};

bool eh_edge_states_equal(const map<string, SlotState> & lhs,
                          const map<string, SlotState> & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  map<string, SlotState>::const_iterator lit = lhs.begin();
  map<string, SlotState>::const_iterator rit = rhs.begin();
  while(lit != lhs.end()) {
    if(lit->first != rit->first || !slot_state_equal(lit->second, rit->second)) {
      return false;
    }
    ++lit;
    ++rit;
  }
  return true;
}

bool slot_block_analysis_equal(const SlotBlockAnalysis & lhs,
                               const SlotBlockAnalysis & rhs)
{
  return slot_state_equal(lhs.out_state, rhs.out_state) &&
         lhs.executable_successors == rhs.executable_successors &&
         eh_edge_states_equal(lhs.eh_edge_states, rhs.eh_edge_states);
}

void note_slot_usage(SlotUsageSummary & summary,
                     bool allowed_use)
{
  summary.seen_any_use = true;
  if(!allowed_use) {
    summary.promotable = false;
  }
}

set<string> find_promotable_slots(const lir::Function & function,
                                  const set<string> & function_symbols,
                                  SlotPromotionScope scope)
{
  map<string, lir::LowType> slot_types;
  map<string, SlotUsageSummary> slot_usage;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(!slot_allowed_for_scope(function.slots[i].first, scope)) {
      continue;
    }
    slot_types[function.slots[i].first] = function.slots[i].second;
  }

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & instruction = function.blocks[bi].instructions[ii];

      if(instruction.kind == lir::Instruction::IK_LOAD &&
         instruction.first.kind == lir::Operand::OP_SLOT &&
         slot_types.count(instruction.first.text) != 0) {
        note_slot_usage(slot_usage[instruction.first.text],
                        instruction.type.text == slot_types[instruction.first.text].text &&
                        is_slot_promotion_type(slot_types[instruction.first.text].text));
      }

      if(instruction.kind == lir::Instruction::IK_STORE &&
         instruction.second.kind == lir::Operand::OP_SLOT &&
         slot_types.count(instruction.second.text) != 0) {
        note_slot_usage(slot_usage[instruction.second.text],
                        instruction.type.text == slot_types[instruction.second.text].text &&
                        is_slot_promotion_type(slot_types[instruction.second.text].text) &&
                        is_stable_slot_value_operand(instruction.first, function_symbols));
      }

      const auto note_other_slot_use = [&slot_types, &slot_usage](const lir::Operand & operand) {
        if(operand.kind == lir::Operand::OP_SLOT &&
           slot_types.count(operand.text) != 0) {
          note_slot_usage(slot_usage[operand.text], false);
        }
      };

      switch(instruction.kind) {
        case lir::Instruction::IK_STORE:
          note_other_slot_use(instruction.first);
          break;
        case lir::Instruction::IK_LOAD:
          break;
        default:
          note_other_slot_use(instruction.first);
          note_other_slot_use(instruction.second);
          note_other_slot_use(instruction.third);
          for(size_t ai = 0; ai < instruction.args.size(); ++ai) {
            note_other_slot_use(instruction.args[ai]);
          }
          break;
      }
    }
  }

  set<string> promotable;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    const string & slot_name = function.slots[i].first;
    if(!slot_allowed_for_scope(slot_name, scope)) {
      continue;
    }
    const map<string, SlotUsageSummary>::const_iterator found = slot_usage.find(slot_name);
    if(found != slot_usage.end() && found->second.promotable && found->second.seen_any_use) {
      promotable.insert(slot_name);
    }
  }
  return promotable;
}

SlotBlockAnalysis analyze_slot_block(const lir::Block & block,
                                     const string & slot_name,
                                     const SlotState & input,
                                     const set<string> & function_symbols,
                                     const ValueEnvironment & input_environment,
                                     const ValueDebugLocationEnvironment & input_debug_locations,
                                     const BooleanTempSet & input_boolean_temps,
                                     const lir::InstructionDebugLocation * fallback_debug_location = nullptr,
                                     bool track_debug_locations = true)
{
  SlotBlockAnalysis analysis;
  analysis.out_state = input;
  {
    const vector<string> structural_successors =
        collect_nonterminator_structural_successor_labels(block);
    for(size_t i = 0; i < structural_successors.size(); ++i) {
      analysis.executable_successors.insert(structural_successors[i]);
    }
  }

  ValueEnvironment environment = input_environment;
  ValueDebugLocationEnvironment debug_locations;
  if(track_debug_locations) {
    debug_locations = input_debug_locations;
  }
  BooleanTempSet boolean_temps = input_boolean_temps;
  ProducerEnvironment producers;
  ExpressionCache expression_cache;
  vector<string> active_eh_labels;

  for(size_t i = 0; i < block.instructions.size(); ++i) {
    lir::Instruction simulated = block.instructions[i];
    if((simulated.kind == lir::Instruction::IK_EH_TRY ||
        simulated.kind == lir::Instruction::IK_EH_CLEANUP) &&
       simulated.first.kind == lir::Operand::OP_LABEL) {
      active_eh_labels.push_back(simulated.first.text);
    } else if(simulated.kind == lir::Instruction::IK_EH_END &&
              !active_eh_labels.empty()) {
      active_eh_labels.pop_back();
    }

    if((simulated.kind == lir::Instruction::IK_CALL ||
        simulated.kind == lir::Instruction::IK_THROW) &&
       !active_eh_labels.empty()) {
      for(size_t ai = 0; ai < active_eh_labels.size(); ++ai) {
        analysis.eh_edge_states[active_eh_labels[ai]] = analysis.out_state;
      }
    }

    if(simulated.kind == lir::Instruction::IK_LOAD &&
       simulated.first.kind == lir::Operand::OP_SLOT &&
       simulated.first.text == slot_name &&
       analysis.out_state.kind == SlotState::SS_KNOWN) {
      simulated = make_copy_instruction(simulated.dest, simulated.type.text,
                                        analysis.out_state.value, &simulated);
      simplify_copy_instruction(simulated);
    }

    process_instruction(environment,
                        debug_locations,
                        boolean_temps,
                        producers,
                        expression_cache,
                        simulated,
                        fallback_debug_location,
                        true,
                        true,
                        track_debug_locations);

    if(simulated.kind == lir::Instruction::IK_STORE &&
       simulated.second.kind == lir::Operand::OP_SLOT &&
       simulated.second.text == slot_name) {
      if(is_stable_slot_value_operand(simulated.first, function_symbols)) {
        analysis.out_state = known_slot_state(simulated.first);
      } else {
        analysis.out_state = unknown_slot_state();
      }
    }

    if(i + 1 == block.instructions.size()) {
      const unordered_set<string> terminator_successors =
          collect_executable_successor_labels(simulated);
      analysis.executable_successors.insert(terminator_successors.begin(),
                                            terminator_successors.end());
    }
  }

  return analysis;
}

bool remove_dead_promoted_slot_stores(lir::Function & function,
                                      const set<string> & promotable_slots)
{
  if(promotable_slots.empty() || function.blocks.empty()) {
    return false;
  }

  vector<set<string> > block_use(function.blocks.size());
  vector<set<string> > block_def(function.blocks.size());
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & instruction = function.blocks[bi].instructions[ii];
      if(instruction.kind == lir::Instruction::IK_LOAD &&
         instruction.first.kind == lir::Operand::OP_SLOT &&
         promotable_slots.count(instruction.first.text) != 0) {
        if(block_def[bi].count(instruction.first.text) == 0) {
          block_use[bi].insert(instruction.first.text);
        }
      } else if(instruction.kind == lir::Instruction::IK_STORE &&
                instruction.second.kind == lir::Operand::OP_SLOT &&
                promotable_slots.count(instruction.second.text) != 0) {
        block_def[bi].insert(instruction.second.text);
      }
    }
  }

  const FunctionControlFlow control_flow = build_function_control_flow(function);
  vector<set<string> > live_in(function.blocks.size());
  vector<set<string> > live_out(function.blocks.size());
  bool dataflow_changed = false;
  do {
    dataflow_changed = false;
    for(size_t offset = function.blocks.size(); offset-- != 0;) {
      set<string> new_live_out;
      for(size_t si = 0; si < control_flow.successors[offset].size(); ++si) {
        const size_t successor = control_flow.successors[offset][si];
        new_live_out.insert(live_in[successor].begin(),
                            live_in[successor].end());
      }

      set<string> new_live_in = block_use[offset];
      for(set<string>::const_iterator it = new_live_out.begin();
          it != new_live_out.end();
          ++it) {
        if(block_def[offset].count(*it) == 0) {
          new_live_in.insert(*it);
        }
      }

      if(new_live_out != live_out[offset] || new_live_in != live_in[offset]) {
        live_out[offset] = new_live_out;
        live_in[offset] = new_live_in;
        dataflow_changed = true;
      }
    }
  } while(dataflow_changed);

  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    set<string> live = live_out[bi];
    vector<lir::Instruction> kept_reversed;
    kept_reversed.reserve(function.blocks[bi].instructions.size());
    for(size_t ii = function.blocks[bi].instructions.size(); ii-- != 0;) {
      const lir::Instruction & instruction = function.blocks[bi].instructions[ii];
      if(instruction.kind == lir::Instruction::IK_LOAD &&
         instruction.first.kind == lir::Operand::OP_SLOT &&
         promotable_slots.count(instruction.first.text) != 0) {
        live.insert(instruction.first.text);
      }
      if(instruction.kind == lir::Instruction::IK_STORE &&
         instruction.second.kind == lir::Operand::OP_SLOT &&
         promotable_slots.count(instruction.second.text) != 0) {
        if(live.count(instruction.second.text) == 0) {
          changed = true;
          continue;
        }
        live.erase(instruction.second.text);
      }
      kept_reversed.push_back(instruction);
    }

    vector<lir::Instruction> kept;
    kept.reserve(kept_reversed.size());
    for(size_t ii = kept_reversed.size(); ii-- != 0;) {
      kept.push_back(kept_reversed[ii]);
    }
    function.blocks[bi].instructions.swap(kept);
  }
  return changed;
}

bool promote_simple_slots(lir::Function & function,
                          SlotPromotionScope scope = SPS_ALL)
{
  if(function.slots.empty() || function.blocks.empty()) {
    return false;
  }
  if(scope == SPS_INLINE_ONLY &&
     should_skip_inline_slot_promotion(function)) {
    return false;
  }

  set<string> function_symbols;
  function_symbols.insert(function.name);

  const set<string> promotable_slots =
      find_promotable_slots(function, function_symbols, scope);
  if(promotable_slots.empty()) {
    return false;
  }

  const FunctionOptimizationContext context =
      collect_function_optimization_context(function);
  const ValueDataflowState value_dataflow = compute_value_dataflow(function, &context);
  const lir::InstructionDebugLocation * fallback_debug_location =
      function.debug_location.present() ? &function.debug_location : nullptr;
  bool changed = false;

  for(set<string>::const_iterator slot_it = promotable_slots.begin();
      slot_it != promotable_slots.end();
      ++slot_it) {
    const string & slot_name = *slot_it;
    vector<SlotState> in_states(function.blocks.size());
    vector<SlotBlockAnalysis> analyses(function.blocks.size());
    vector<bool> executable(function.blocks.size(), false);
    vector<size_t> worklist(1, 0);
    vector<bool> queued(function.blocks.size(), false);
    queued[0] = true;

    while(!worklist.empty()) {
      const size_t bi = worklist.back();
      worklist.pop_back();
      queued[bi] = false;

      SlotState new_in;
      bool new_executable = false;
      if(bi == 0) {
        new_in.kind = SlotState::SS_UNSET;
        new_executable = true;
      } else if(context.control_flow.incoming_edges[bi].empty()) {
        new_in.kind = SlotState::SS_UNSET;
      } else {
        bool have_executable_pred = false;
        for(size_t pi = 0; pi < context.control_flow.incoming_edges[bi].size(); ++pi) {
          const IncomingBlockEdge & edge = context.control_flow.incoming_edges[bi][pi];
          const size_t predecessor = edge.predecessor;
          if(!executable[predecessor] ||
             analyses[predecessor].executable_successors.count(function.blocks[bi].label) == 0) {
            continue;
          }
          SlotState predecessor_state;
          predecessor_state.kind = SlotState::SS_UNSET;
          if(edge.has_eh_edge) {
            const map<string, SlotState>::const_iterator found =
                analyses[predecessor].eh_edge_states.find(function.blocks[bi].label);
            if(found != analyses[predecessor].eh_edge_states.end()) {
              predecessor_state = found->second;
            }
          } else {
            predecessor_state = analyses[predecessor].out_state;
          }
          if(!have_executable_pred) {
            new_in = predecessor_state;
            have_executable_pred = true;
          } else {
            new_in = meet_slot_states(new_in, predecessor_state);
          }
        }
        if(have_executable_pred) {
          new_executable = true;
        } else {
          new_in.kind = SlotState::SS_UNSET;
        }
      }

      if(new_executable == executable[bi] &&
         slot_state_equal(new_in, in_states[bi])) {
        continue;
      }

      const bool old_executable = executable[bi];
      executable[bi] = new_executable;
      in_states[bi] = new_in;

      SlotBlockAnalysis new_analysis;
      if(executable[bi]) {
        const ResolvedValueInputState input =
            resolve_dataflow_input_state(value_dataflow, bi, context.track_debug_locations);
        new_analysis = analyze_slot_block(function.blocks[bi],
                                          slot_name,
                                          in_states[bi],
                                          function_symbols,
                                          input.in_environment(),
                                          input.in_debug_locations(),
                                          input.in_boolean_temps(),
                                          fallback_debug_location,
                                          context.track_debug_locations);
      } else {
        new_analysis.out_state.kind = SlotState::SS_UNSET;
      }

      const bool outputs_changed = !slot_block_analysis_equal(new_analysis, analyses[bi]);
      analyses[bi] = std::move(new_analysis);

      if(old_executable != executable[bi] || outputs_changed) {
        for(size_t si = 0; si < context.control_flow.successors[bi].size(); ++si) {
          const size_t successor = context.control_flow.successors[bi][si];
          if(!queued[successor]) {
            queued[successor] = true;
            worklist.push_back(successor);
          }
        }
      }
    }

    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      if(!executable[bi]) {
        continue;
      }
      SlotState state = in_states[bi];
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        lir::Instruction & instruction = function.blocks[bi].instructions[ii];
        if(instruction.kind == lir::Instruction::IK_LOAD &&
           instruction.first.kind == lir::Operand::OP_SLOT &&
           instruction.first.text == slot_name &&
           state.kind == SlotState::SS_KNOWN) {
          instruction =
              make_copy_instruction(instruction.dest, instruction.type.text, state.value,
                                    &instruction);
          simplify_copy_instruction(instruction);
          changed = true;
        }

        if(instruction.kind == lir::Instruction::IK_STORE &&
           instruction.second.kind == lir::Operand::OP_SLOT &&
           instruction.second.text == slot_name) {
          if(is_stable_slot_value_operand(instruction.first, function_symbols)) {
            state = known_slot_state(instruction.first);
          } else {
            state = unknown_slot_state();
          }
        }
      }
    }
  }

  changed |= remove_dead_promoted_slot_stores(function, promotable_slots);
  return changed;
}

bool run_o1_inline_pipeline(lir::Function & function,
                            const FunctionBoundaryMap & function_boundaries,
                            const FunctionDefinitionMap & function_definitions,
                            const unordered_set<string> & recursive_functions,
                            size_t & next_inline_site_id)
{
  bool changed = false;
  bool batch_changed = false;
  size_t batch_inline_count = 0;
  const size_t max_o1_inlines_per_cleanup_batch = 8;
  const auto flush_inline_cleanup = [&]()
  {
    if(!batch_changed) {
      return;
    }
    changed = true;
    run_o1_cleanup_pipeline(function, function_boundaries);
    const SlotPromotionScope scope =
        should_limit_full_slot_promotion(function) ? SPS_INLINE_ONLY : SPS_ALL;
    if(promote_simple_slots(function, scope)) {
      run_o1_cleanup_pipeline(function, function_boundaries);
    }
    batch_changed = false;
    batch_inline_count = 0;
  };

  while(function_allows_o1_inline_expansion(function) &&
        inline_small_direct_calls(function,
                                  function_boundaries,
                                  function_definitions,
                                  recursive_functions,
                                  next_inline_site_id)) {
    batch_changed = true;
    ++batch_inline_count;
    if(batch_inline_count >= max_o1_inlines_per_cleanup_batch ||
       !function_allows_o1_inline_expansion(function)) {
      flush_inline_cleanup();
    }
  }
  flush_inline_cleanup();
  return changed;
}

void run_o2_slot_promotion_pipeline(lir::Function & function,
                                    const FunctionBoundaryMap & function_boundaries)
{
  const bool limit_full_promotion = should_limit_full_slot_promotion(function);
  const bool has_inline_slots = function_has_inline_generated_slots(function);
  if(limit_full_promotion && !has_inline_slots) {
    return;
  }
  const SlotPromotionScope scope =
      limit_full_promotion ? SPS_INLINE_ONLY : SPS_ALL;
  bool changed = false;
  do {
    const bool promotion_changed = promote_simple_slots(function, scope);
    const bool cleanup_changed =
        promotion_changed && run_o1_cleanup_pipeline(function, function_boundaries);
    changed = promotion_changed || cleanup_changed;
  } while(changed);
}

}  // namespace

lowir::LowirProgram optimize_lowir_program(const lowir::LowirProgram & program,
                                           int optimization_level)
{
  lowir::LowirProgram optimized = program;
  const int level = normalize_optimization_level(optimization_level);
  if(level <= 0) {
    return optimized;
  }

  const PhaseTimer timer("optimize_lowir_program",
                         string("level=") + to_string(level) +
                             " functions=" + to_string(optimized.functions.size()));

  FunctionBoundaryMap function_boundaries;
  for(size_t i = 0; i < optimized.function_declarations.size(); ++i) {
    merge_boundary_metadata(function_boundaries[optimized.function_declarations[i].name],
                            optimized.function_declarations[i].boundary);
  }
  for(size_t i = 0; i < optimized.functions.size(); ++i) {
    merge_boundary_metadata(function_boundaries[optimized.functions[i].name],
                            optimized.functions[i].boundary);
  }

  FunctionDefinitionMap function_definitions;
  for(size_t i = 0; i < optimized.functions.size(); ++i) {
    function_definitions[optimized.functions[i].name] = &optimized.functions[i];
  }

  for(size_t i = 0; i < optimized.functions.size(); ++i) {
    run_o1_cleanup_pipeline(optimized.functions[i], function_boundaries);
  }

  // Earlier callers may only become inlineable after later callees are
  // simplified in the same O1 run, so iterate a small whole-program fixpoint.
  vector<size_t> next_inline_site_ids(optimized.functions.size(), 0);
  const size_t max_o1_inline_rounds = 4;
  for(size_t round = 0; round < max_o1_inline_rounds; ++round) {
    bool round_changed = false;
    const unordered_set<string> recursive_functions =
        find_recursive_function_names(optimized.functions);
    for(size_t i = 0; i < optimized.functions.size(); ++i) {
      round_changed |= run_o1_inline_pipeline(optimized.functions[i],
                                              function_boundaries,
                                              function_definitions,
                                              recursive_functions,
                                              next_inline_site_ids[i]);
    }
    if(!round_changed) {
      break;
    }
  }

  if(level < 2) {
    return optimized;
  }

  for(size_t i = 0; i < optimized.functions.size(); ++i) {
    run_o2_slot_promotion_pipeline(optimized.functions[i], function_boundaries);
  }
  return optimized;
}

string optimize_lowir_text(const vector<string> & srcfiles,
                           int optimization_level)
{
  return lowir::serialize_lowir_program(
      optimize_lowir_program(lowir::parse_lowir_program_files(srcfiles),
                             optimization_level));
}
