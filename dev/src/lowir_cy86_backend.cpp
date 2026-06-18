#include "lowir_cy86_backend.h"

#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "eh_runtime.h"
#include "lowir_internal.h"
#include "types.h"

namespace {

namespace lir = lowir_internal;
namespace cy = cy86_internal;

using lir::Block;
using lir::Function;
using lir::GlobalDefinition;
using lir::Instruction;
using lir::LowType;
using lir::ParseError;
using lir::Program;
using lir::is_sign_extended_integer_type;
using lir::type_size;
using cy::AddressExpr;
using cy::LiteralValue;
using cy::Statement;

struct FunctionLayout
{
  struct AbiLocation
  {
    bool in_reg = true;
    size_t reg_index = 0;
    long long stack_offset = 0;
  };

  map<string, long long> storage_offset;
  map<string, LowType> storage_type;
  vector<string> params;
  vector<string> slots;
  vector<string> temps;
  size_t frame_slots = 0;
  bool has_f80_scratch = false;
  bool has_hidden_return_ptr = false;
  long long hidden_return_ptr_offset = 0;
  vector<AbiLocation> param_abi;
};

LiteralValue integer_literal(long long value)
{
  LiteralValue literal;
  literal.type = FT_LONG_LONG_INT;
  literal.data = cy::bytes_of<long long>(value);
  literal.num_elements = 0;
  literal.is_array = false;
  literal.negated = false;
  return literal;
}

bool is_float_type(const LowType & type)
{
  return type.text == "f32" || type.text == "f64" || type.text == "f80";
}

bool is_register_float_type(const LowType & type)
{
  return type.text == "f32" || type.text == "f64";
}

bool is_f80_type(const LowType & type)
{
  return type.text == "f80";
}

bool is_direct_object_type(const LowType & type)
{
  return lir::is_object_type(type);
}

bool uses_hidden_return_ptr(const LowType & type)
{
  return is_f80_type(type) || is_direct_object_type(type);
}

bool uses_storage_address_passing(lir::ParamPassingMode passing)
{
  return passing == lir::PPM_INDIRECT_RESULT ||
         passing == lir::PPM_BY_ADDRESS ||
         passing == lir::PPM_REFERENCE ||
         passing == lir::PPM_DECAY;
}

bool is_atomic_scalar_type(const LowType & type)
{
  return type.text == "i1" || type.text == "i8" || type.text == "u8" ||
         type.text == "i16" || type.text == "u16" || type.text == "i32" ||
         type.text == "u32" || type.text == "i64" || type.text == "ptr";
}

bool is_integer_scalar_type(const LowType & type)
{
  return type.text == "i1" || type.text == "i8" || type.text == "u8" ||
         type.text == "i16" || type.text == "u16" || type.text == "i32" ||
         type.text == "u32" || type.text == "i64";
}

long long atomic_order_value(const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_INTEGER) {
    throw ParseError("atomic order must be an integer literal");
  }
  return operand.int_value;
}

size_t type_storage_bytes(const LowType & type)
{
  return type_size(type);
}

size_t type_storage_slots(const LowType & type)
{
  const size_t bytes = type_storage_bytes(type);
  return bytes == 0 ? 0 : (bytes + 7) / 8;
}

size_t type_exec_width_bytes(const LowType & type)
{
  return is_f80_type(type) ? 10 : type_size(type);
}

bool is_negative_literal_text(const string & source)
{
  return !source.empty() && source[0] == '-';
}

string unsigned_literal_text(const string & source)
{
  if(!source.empty() && (source[0] == '-' || source[0] == '+')) {
    return source.substr(1);
  }
  return source;
}

bool signaling_nan_literal_bytes(const string & type,
                                 const string & source,
                                 vector<unsigned char> & out)
{
  const string text = unsigned_literal_text(source);
  const bool negative = is_negative_literal_text(source);
  if(type == "f32" && text == "snanf") {
    const uint64_t bits = (negative ? 0x80000000ULL : 0) | 0x7FA00000ULL;
    out = cy::encode_uint64(bits, 4);
    return true;
  }
  if(type == "f64" && text == "snan") {
    const uint64_t bits =
        (negative ? 0x8000000000000000ULL : 0) | 0x7FF4000000000000ULL;
    out = cy::encode_uint64(bits, 8);
    return true;
  }
  if(type == "f80" && text == "snanL") {
    out.assign(10, 0);
    out[7] = 0xA0;
    out[8] = 0xFF;
    out[9] = negative ? 0xFF : 0x7F;
    return true;
  }
  return false;
}

LiteralValue floating_literal(long double value,
                              const LowType & type,
                              const string & source)
{
  LiteralValue literal;
  literal.source = source;
  vector<unsigned char> snan_bytes;
  if(signaling_nan_literal_bytes(type.text, source, snan_bytes)) {
    if(type.text == "f32") {
      literal.type = FT_FLOAT;
    } else if(type.text == "f64") {
      literal.type = FT_DOUBLE;
    } else {
      literal.type = FT_LONG_DOUBLE;
    }
    literal.data = snan_bytes;
    return literal;
  }
  if(type.text == "f32") {
    literal.type = FT_FLOAT;
    literal.data = cy::bytes_of(static_cast<float>(value));
    return literal;
  }
  if(type.text == "f64") {
    literal.type = FT_DOUBLE;
    literal.data = cy::bytes_of(static_cast<double>(value));
    return literal;
  }
  if(type.text == "f80") {
    literal.type = FT_LONG_DOUBLE;
    literal.data = cy::store_float80(value);
    return literal;
  }
  throw ParseError("unsupported floating literal type " + type.text);
}

LiteralValue scalar_literal(const lir::Operand & operand, const LowType & type)
{
  if(!is_float_type(type)) {
    throw ParseError("scalar_literal helper is only valid for floating types");
  }
  if(operand.kind == lir::Operand::OP_FLOAT) {
    return floating_literal(operand.float_value, type, operand.text);
  }
  if(operand.kind == lir::Operand::OP_INTEGER) {
    return floating_literal(static_cast<long double>(operand.int_value),
                            type,
                            to_string(operand.int_value));
  }
  throw ParseError("floating scalar literal requires integer or floating operand");
}

AddressExpr literal_expr(long long value)
{
  AddressExpr expr;
  expr.base_kind = cy::EB_LITERAL;
  expr.literal = integer_literal(value);
  return expr;
}

AddressExpr label_expr(const string & label)
{
  AddressExpr expr;
  expr.base_kind = cy::EB_LABEL;
  expr.name = label;
  return expr;
}

AddressExpr register_expr(const string & reg, long long offset = 0)
{
  AddressExpr expr;
  expr.base_kind = cy::EB_REGISTER;
  expr.name = reg;
  if(offset != 0) {
    expr.has_offset = true;
    expr.subtract_offset = offset < 0;
    expr.offset = integer_literal(offset < 0 ? -offset : offset);
  }
  return expr;
}

cy::Operand register_operand(const string & reg)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_REGISTER;
  operand.reg = reg;
  return operand;
}

cy::Operand immediate_integer(long long value)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_IMMEDIATE;
  operand.expr = literal_expr(value);
  return operand;
}

cy::Operand immediate_literal(const LiteralValue & literal)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_IMMEDIATE;
  operand.expr.base_kind = cy::EB_LITERAL;
  operand.expr.literal = literal;
  return operand;
}

cy::Operand immediate_label(const string & label)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_IMMEDIATE;
  operand.expr = label_expr(label);
  return operand;
}

cy::Operand memory_reg(const string & reg, long long offset = 0)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_MEMORY;
  operand.expr = register_expr(reg, offset);
  return operand;
}

cy::Operand memory_label(const string & label)
{
  cy::Operand operand;
  operand.kind = cy::OPERAND_MEMORY;
  operand.expr = label_expr(label);
  return operand;
}

struct ProgramBuilder
{
  vector<Statement> statements;
  vector<string> pending_labels;

  void label(const string & name)
  {
    pending_labels.push_back(name);
  }

  void opcode(const string & name, const vector<cy::Operand> & operands)
  {
    Statement statement;
    statement.kind = cy::SK_OPCODE;
    statement.opcode = name;
    statement.operands = operands;
    statement.labels.swap(pending_labels);
    statements.push_back(statement);
  }

  void data_integer(size_t width_bytes, long long value)
  {
    Statement statement;
    statement.kind = cy::SK_OPCODE;
    if(width_bytes == 8) {
      statement.opcode = "data64";
    } else {
      statement.opcode = string("data") + to_string(width_bytes * 8);
    }
    statement.operands.push_back(immediate_integer(value));
    statement.labels.swap(pending_labels);
    statements.push_back(statement);
  }

  void data_literal(const LiteralValue & literal)
  {
    Statement statement;
    statement.kind = cy::SK_OPCODE;
    const size_t width_bytes = literal.data.size();
    statement.opcode = string("data") + to_string(width_bytes * 8);
    statement.operands.push_back(immediate_literal(literal));
    statement.labels.swap(pending_labels);
    statements.push_back(statement);
  }

  void data_label(const string & label_name)
  {
    Statement statement;
    statement.kind = cy::SK_OPCODE;
    statement.opcode = "data64";
    statement.operands.push_back(immediate_label(label_name));
    statement.labels.swap(pending_labels);
    statements.push_back(statement);
  }

  void zero_bytes(size_t count)
  {
    for(size_t i = 0; i < count; ++i) {
      data_integer(1, 0);
    }
  }

  cy::Program finish()
  {
    return cy::finalize_program(std::move(statements));
  }
};

struct CY86Translator
{
  Program program_;
  map<string, string> function_symbols_;
  set<string> defined_function_names_;
  map<string, lowir_internal::SymbolRole> function_roles_;
  map<string, string> global_symbols_;
  set<string> defined_global_names_;
  map<string, LowType> global_types_;
  ProgramBuilder builder_;
  bool uses_eh_runtime_ = false;
  size_t internal_label_counter_ = 0;

  explicit CY86Translator(const Program & program)
    : program_(program)
  {
    for(size_t i = 0; i < program_.function_declarations.size(); ++i) {
      const lir::FunctionDeclaration & declaration = program_.function_declarations[i];
      const string & name = declaration.name;
      if(eh_runtime::is_reserved_symbol(name)) {
        throw ParseError("reserved runtime symbol " + name + " may not be declared in LowIR");
      }
      register_function_role(name, declaration.metadata.role);
      if(function_symbols_.count(name) == 0) {
        function_symbols_[name] = string("fn__") + lir::mangle_name(name);
      }
    }
    for(size_t i = 0; i < program_.functions.size(); ++i) {
      const Function & function = program_.functions[i];
      const string & name = function.name;
      if(eh_runtime::is_reserved_symbol(name)) {
        throw ParseError("reserved runtime symbol " + name + " may not be defined in LowIR");
      }
      if(defined_function_names_.count(name) != 0) {
        throw ParseError("duplicate function " + name);
      }
      defined_function_names_.insert(name);
      register_function_role(name, function.metadata.role);
      function_symbols_[name] = string("fn__") + lir::mangle_name(name);
    }
    for(size_t i = 0; i < program_.global_declarations.size(); ++i) {
      const string & name = program_.global_declarations[i].name;
      if(eh_runtime::is_reserved_symbol(name)) {
        throw ParseError("reserved runtime symbol " + name + " may not be declared in LowIR");
      }
      if(global_symbols_.count(name) == 0) {
        global_symbols_[name] = string("g__") + lir::mangle_name(name);
      }
      if(program_.global_declarations[i].has_type) {
        global_types_[name] = program_.global_declarations[i].type;
      }
    }
    for(size_t i = 0; i < program_.globals.size(); ++i) {
      const string & name = program_.globals[i].name;
      if(eh_runtime::is_reserved_symbol(name)) {
        throw ParseError("reserved runtime symbol " + name + " may not be defined in LowIR");
      }
      if(defined_global_names_.count(name) != 0) {
        throw ParseError("duplicate global " + name);
      }
      defined_global_names_.insert(name);
      global_symbols_[name] = string("g__") + lir::mangle_name(name);
      if(!program_.globals[i].structured) {
        global_types_[name] = program_.globals[i].type;
      }
    }
    for(size_t fi = 0; fi < program_.functions.size(); ++fi) {
      const Function & function = program_.functions[fi];
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
          switch(function.blocks[bi].instructions[ii].kind) {
            case Instruction::IK_EH_TRY:
            case Instruction::IK_EH_CLEANUP:
            case Instruction::IK_EH_END:
            case Instruction::IK_THROW:
            case Instruction::IK_EXCEPTION:
            case Instruction::IK_RESUME:
              uses_eh_runtime_ = true;
              break;
            default:
              break;
          }
        }
      }
    }
    if(uses_eh_runtime_) {
      global_symbols_[eh_runtime::kEhTopSymbol] =
          string("g__") + lir::mangle_name(eh_runtime::kEhTopSymbol);
      global_symbols_[eh_runtime::kEhValueSymbol] =
          string("g__") + lir::mangle_name(eh_runtime::kEhValueSymbol);
      LowType top_type;
      top_type.text = "ptr";
      global_types_[eh_runtime::kEhTopSymbol] = top_type;
      LowType value_type;
      value_type.text = "i64";
      global_types_[eh_runtime::kEhValueSymbol] = value_type;
      function_symbols_[eh_runtime::kEhUnhandledSymbol] =
          string("fn__") + lir::mangle_name(eh_runtime::kEhUnhandledSymbol);
    }
    if(entry_function_name().empty()) {
      throw ParseError("missing LowIR entry function");
    }
  }

  void register_function_role(const string & name, lowir_internal::SymbolRole role)
  {
    if(role == lowir_internal::SR_NONE) {
      return;
    }
    map<string, lowir_internal::SymbolRole>::iterator found = function_roles_.find(name);
    if(found != function_roles_.end()) {
      if(found->second != role) {
        throw ParseError("conflicting LowIR function roles for " + name);
      }
      return;
    }
    if(!lowir_internal::is_function_symbol_role(role)) {
      throw ParseError("invalid function role on " + name);
    }
    for(map<string, lowir_internal::SymbolRole>::const_iterator it = function_roles_.begin();
        it != function_roles_.end(); ++it) {
      if(it->second == role && it->first != name) {
        throw ParseError("duplicate LowIR function role " +
                         string(lowir_internal::symbol_role_text(role)));
      }
    }
    function_roles_[name] = role;
  }

  string first_defined_function_with_role(lowir_internal::SymbolRole role) const
  {
    for(map<string, lowir_internal::SymbolRole>::const_iterator it = function_roles_.begin();
        it != function_roles_.end(); ++it) {
      if(it->second == role &&
         defined_function_names_.count(it->first) != 0) {
        return it->first;
      }
    }
    return string();
  }

  string entry_function_name() const
  {
    const string explicit_entry = first_defined_function_with_role(lowir_internal::SR_ENTRY);
    if(!explicit_entry.empty()) {
      return explicit_entry;
    }
    return defined_function_names_.count("@main") != 0 ? "@main" : string();
  }

  string init_function_name() const
  {
    const string explicit_init = first_defined_function_with_role(lowir_internal::SR_INIT);
    if(!explicit_init.empty()) {
      return explicit_init;
    }
    return defined_function_names_.count("@__cppgm_init") != 0 ? "@__cppgm_init" : string();
  }

  string fini_function_name() const
  {
    const string explicit_fini = first_defined_function_with_role(lowir_internal::SR_FINI);
    if(!explicit_fini.empty()) {
      return explicit_fini;
    }
    return defined_function_names_.count("@__cppgm_fini") != 0 ? "@__cppgm_fini" : string();
  }

  string make_internal_label(const string & prefix)
  {
    return prefix + "__" + to_string(internal_label_counter_++);
  }

  static const char * abi_reg(size_t index)
  {
    static const char * regs[] = {"x64", "y64", "z64", "t64"};
    if(index >= 4) {
      throw ParseError("PA13 internal ABI supports at most 4 register arguments");
    }
    return regs[index];
  }

  FunctionLayout::AbiLocation abi_location(size_t index) const
  {
    FunctionLayout::AbiLocation loc;
    if(index < 4) {
      loc.in_reg = true;
      loc.reg_index = index;
      return loc;
    }
    loc.in_reg = false;
    loc.stack_offset = 16 + static_cast<long long>(index - 4) * 8;
    return loc;
  }

  bool function_uses_f80(const Function & function) const
  {
    if(is_f80_type(function.return_type)) {
      return true;
    }
    for(size_t i = 0; i < function.params.size(); ++i) {
      if(is_f80_type(function.params[i].type)) {
        return true;
      }
    }
    for(size_t i = 0; i < function.slots.size(); ++i) {
      if(is_f80_type(function.slots[i].second)) {
        return true;
      }
    }
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.kind == Instruction::IK_CONVERT) {
          return true;
        }
        if(is_f80_type(inst.type)) {
          return true;
        }
        if(inst.first.literal_type.text == "f80" ||
           inst.second.literal_type.text == "f80" ||
           inst.third.literal_type.text == "f80") {
          return true;
        }
      }
    }
    return false;
  }

  FunctionLayout build_layout(const Function & function) const
  {
    FunctionLayout layout;
    layout.has_f80_scratch = function_uses_f80(function);

    size_t abi_index = 0;
    if(uses_hidden_return_ptr(function.return_type)) {
      layout.has_hidden_return_ptr = true;
      layout.hidden_return_ptr_offset = -8;
      layout.frame_slots += 1;
      ++abi_index;
    }
    for(size_t i = 0; i < function.params.size(); ++i) {
      const string & name = function.params[i].name;
      if(layout.storage_offset.count(name) != 0) {
        throw ParseError("duplicate storage name " + name + " in " + function.name);
      }
      layout.param_abi.push_back(abi_location(abi_index++));
      layout.frame_slots += type_storage_slots(function.params[i].type);
      layout.storage_offset[name] = -static_cast<long long>(layout.frame_slots * 8);
      layout.storage_type[name] = function.params[i].type;
      layout.params.push_back(name);
    }
    for(size_t i = 0; i < function.slots.size(); ++i) {
      const string & name = function.slots[i].first;
      if(layout.storage_offset.count(name) != 0) {
        throw ParseError("duplicate storage name " + name + " in " + function.name);
      }
      layout.frame_slots += type_storage_slots(function.slots[i].second);
      layout.storage_offset[name] = -static_cast<long long>(layout.frame_slots * 8);
      layout.storage_type[name] = function.slots[i].second;
      layout.slots.push_back(name);
    }
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.dest.empty()) {
          continue;
        }
        if(layout.storage_offset.count(inst.dest) == 0) {
          layout.frame_slots += type_storage_slots(inst.type);
          layout.storage_offset[inst.dest] = -static_cast<long long>(layout.frame_slots * 8);
          layout.storage_type[inst.dest] = inst.type;
          layout.temps.push_back(inst.dest);
        }
      }
    }
    return layout;
  }

  long long slot_offset(const FunctionLayout & layout, const string & name) const
  {
    map<string, long long>::const_iterator found = layout.storage_offset.find(name);
    if(found == layout.storage_offset.end()) {
      throw ParseError("unknown storage " + name);
    }
    return found->second;
  }

  size_t frame_slots(const FunctionLayout & layout) const
  {
    return layout.frame_slots + (layout.has_f80_scratch ? 8 : 0);
  }

  long long scratch_offset(const FunctionLayout & layout, size_t index) const
  {
    if(!layout.has_f80_scratch || index >= 4) {
      throw ParseError("invalid f80 scratch index");
    }
    return -static_cast<long long>((layout.frame_slots + (index + 1) * 2) * 8);
  }

  string function_symbol(const string & name) const
  {
    map<string, string>::const_iterator found = function_symbols_.find(name);
    if(found == function_symbols_.end()) {
      throw ParseError("unknown function " + name);
    }
    return found->second;
  }

  string global_symbol(const string & name) const
  {
    map<string, string>::const_iterator found = global_symbols_.find(name);
    if(found == global_symbols_.end()) {
      throw ParseError("unknown global " + name);
    }
    return found->second;
  }

  string block_symbol(const Function & function, const string & label) const
  {
    return function_symbol(function.name) + "__" + lir::mangle_name(label);
  }

  LowType operand_type(const FunctionLayout & layout, const lir::Operand & operand) const
  {
    LowType type;
    type.text = "i64";
    switch(operand.kind) {
      case lir::Operand::OP_TEMP:
      case lir::Operand::OP_SLOT: {
        map<string, LowType>::const_iterator found = layout.storage_type.find(operand.text);
        if(found == layout.storage_type.end()) {
          throw ParseError("unknown storage " + operand.text);
        }
        return found->second;
      }
      case lir::Operand::OP_GLOBAL: {
        map<string, LowType>::const_iterator found = global_types_.find(operand.text);
        if(found != global_types_.end()) {
          return found->second;
        }
        type.text = "ptr";
        return type;
      }
      case lir::Operand::OP_INTEGER:
      case lir::Operand::OP_LABEL:
        return type;
      case lir::Operand::OP_FLOAT:
        return operand.literal_type;
    }
    return type;
  }

  void move_zero_register(char base)
  {
    builder_.opcode("move64",
                    vector<cy::Operand>(1, register_operand(string(1, base) + "64")));
    builder_.statements.back().operands.push_back(immediate_integer(0));
  }

  void sign_extend_to_i64(char base, size_t source_width)
  {
    if(source_width >= 8) {
      return;
    }
    const char shift_base = base == 't' ? 'z' : 't';
    const string shift_reg = string(1, shift_base) + "8";
    if(source_width == 1) {
      builder_.opcode("move8", {register_operand(shift_reg), immediate_integer(56)});
    } else if(source_width == 2) {
      builder_.opcode("move8", {register_operand(shift_reg), immediate_integer(48)});
    } else if(source_width == 4) {
      builder_.opcode("move8", {register_operand(shift_reg), immediate_integer(32)});
    } else {
      throw ParseError("unsupported sign extension width");
    }
    builder_.opcode("lshift64",
                    {register_operand(string(1, base) + "64"),
                     register_operand(string(1, base) + "64"),
                     register_operand(shift_reg)});
    builder_.opcode("srshift64",
                    {register_operand(string(1, base) + "64"),
                     register_operand(string(1, base) + "64"),
                     register_operand(shift_reg)});
  }

  string reg_alias(char base, size_t width_bytes) const
  {
    if(width_bytes != 1 && width_bytes != 2 &&
       width_bytes != 4 && width_bytes != 8) {
      throw ParseError("unsupported CY86 register width");
    }
    return string(1, base) +
           (width_bytes == 1 ? "8" : width_bytes == 2 ? "16" :
            width_bytes == 4 ? "32" : "64");
  }

  string move_opcode(size_t width_bytes) const
  {
    if(width_bytes == 10) {
      return "move80";
    }
    return string("move") +
           (width_bytes == 1 ? "8" : width_bytes == 2 ? "16" :
            width_bytes == 4 ? "32" : "64");
  }

  string opcode_with_width(const string & base, size_t width_bytes) const
  {
    if(width_bytes == 10) {
      return base + "80";
    }
    return base +
           (width_bytes == 1 ? "8" : width_bytes == 2 ? "16" :
            width_bytes == 4 ? "32" : "64");
  }

  void emit_load_value(const FunctionLayout & layout,
                       const lir::Operand & operand,
                       const LowType & desired_type,
                       char reg_base)
  {
    if(is_f80_type(desired_type)) {
      throw ParseError("f80 values must be materialized through scratch storage");
    }
    const size_t desired_width = type_exec_width_bytes(desired_type);
    const string desired_reg = reg_alias(reg_base, desired_width);
    const string reg64 = reg_alias(reg_base, 8);
    switch(operand.kind) {
      case lir::Operand::OP_INTEGER:
        if(is_register_float_type(desired_type)) {
          builder_.opcode(move_opcode(desired_width),
                          {register_operand(desired_reg),
                           immediate_literal(floating_literal(
                               static_cast<long double>(operand.int_value),
                               desired_type,
                               to_string(operand.int_value)))});
          return;
        }
        builder_.opcode("move64",
                        {register_operand(reg64), immediate_integer(operand.int_value)});
        return;
      case lir::Operand::OP_FLOAT:
        if(!is_register_float_type(desired_type)) {
          throw ParseError("floating literal used with non-floating type");
        }
        builder_.opcode(move_opcode(desired_width),
                        {register_operand(desired_reg),
                         immediate_literal(floating_literal(operand.float_value,
                                                           desired_type,
                                                           operand.text))});
        return;
      case lir::Operand::OP_TEMP:
      case lir::Operand::OP_SLOT: {
        const LowType source_type = operand_type(layout, operand);
        const size_t source_width = type_exec_width_bytes(source_type);
        if(source_width >= 8) {
          builder_.opcode("move64",
                          {register_operand(reg64),
                           memory_reg("bp", slot_offset(layout, operand.text))});
        } else {
          if(source_width < 4) {
            move_zero_register(reg_base);
          }
          builder_.opcode(move_opcode(source_width),
                          {register_operand(reg_alias(reg_base, source_width)),
                           memory_reg("bp", slot_offset(layout, operand.text))});
          if(desired_width >= 8 &&
             is_sign_extended_integer_type(source_type)) {
            sign_extend_to_i64(reg_base, source_width);
          }
        }
        return;
      }
      case lir::Operand::OP_GLOBAL: {
        if(function_symbols_.count(operand.text) != 0) {
          builder_.opcode("move64",
                          {register_operand(reg64),
                           immediate_label(function_symbol(operand.text))});
          return;
        }
        const LowType source_type = operand_type(layout, operand);
        const size_t source_width = type_exec_width_bytes(source_type);
        if(source_width >= 8) {
          builder_.opcode("move64",
                          {register_operand(reg64),
                           memory_label(global_symbol(operand.text))});
        } else {
          if(source_width < 4) {
            move_zero_register(reg_base);
          }
          builder_.opcode(move_opcode(source_width),
                          {register_operand(reg_alias(reg_base, source_width)),
                           memory_label(global_symbol(operand.text))});
          if(desired_width >= 8 &&
             is_sign_extended_integer_type(source_type)) {
            sign_extend_to_i64(reg_base, source_width);
          }
        }
        return;
      }
      default:
        break;
    }
    throw ParseError("invalid value operand " + operand.text);
  }

  void emit_load_address(const FunctionLayout & layout,
                         const lir::Operand & operand,
                         const string & reg)
  {
    if(operand.kind == lir::Operand::OP_SLOT || operand.kind == lir::Operand::OP_TEMP) {
      builder_.opcode("isub64",
                      {register_operand(reg),
                       register_operand("bp"),
                       immediate_integer(-slot_offset(layout, operand.text))});
      return;
    }
    if(operand.kind == lir::Operand::OP_GLOBAL) {
      const string symbol = function_symbols_.count(operand.text) != 0
          ? function_symbol(operand.text)
          : global_symbol(operand.text);
      builder_.opcode("move64",
                      {register_operand(reg),
                       immediate_label(symbol)});
      return;
    }
    throw ParseError("invalid address operand " + operand.text);
  }

  void emit_bp_relative_address(long long offset, const string & reg)
  {
    builder_.opcode("isub64",
                    {register_operand(reg),
                     register_operand("bp"),
                     immediate_integer(-offset)});
  }

  void emit_store_temp(const FunctionLayout & layout,
                       const string & dest,
                       char reg_base)
  {
    map<string, LowType>::const_iterator found = layout.storage_type.find(dest);
    if(found == layout.storage_type.end()) {
      throw ParseError("unknown storage " + dest);
    }
    if(is_f80_type(found->second)) {
      throw ParseError("f80 temporaries must be stored through scratch storage");
    }
    const size_t width = type_exec_width_bytes(found->second);
    builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                    {memory_reg("bp", slot_offset(layout, dest)),
                     register_operand(reg_alias(reg_base, width >= 8 ? 8 : width))});
  }

  void emit_load_dereference_or_storage(const FunctionLayout & layout,
                                        const lir::Operand & storage,
                                        const LowType & type,
                                        char reg_base)
  {
    if(is_f80_type(type)) {
      throw ParseError("f80 loads must be materialized through scratch storage");
    }
    const size_t width = type_exec_width_bytes(type);
    const string reg64 = reg_alias(reg_base, 8);
    if(storage.kind == lir::Operand::OP_SLOT || storage.kind == lir::Operand::OP_GLOBAL) {
      emit_load_value(layout, storage, type, reg_base);
      return;
    }
    if(storage.kind == lir::Operand::OP_TEMP) {
      builder_.opcode("move64",
                      {register_operand(reg64),
                       memory_reg("bp", slot_offset(layout, storage.text))});
      if(width >= 8) {
        builder_.opcode("move64",
                        {register_operand(reg64),
                         memory_reg(reg64)});
      } else {
        if(width < 4) {
          move_zero_register(reg_base);
          builder_.opcode(move_opcode(width),
                          {register_operand(reg_alias(reg_base, width)),
                           memory_reg(reg64)});
        } else {
          builder_.opcode(move_opcode(width),
                          {register_operand(reg_alias(reg_base, width)),
                           memory_reg(reg64)});
        }
        if(is_sign_extended_integer_type(type)) {
          sign_extend_to_i64(reg_base, width);
        }
      }
      return;
    }
    throw ParseError("invalid load operand " + storage.text);
  }

  void emit_store_to_storage(const FunctionLayout & layout,
                             const lir::Operand & storage,
                             const LowType & type,
                             char reg_base)
  {
    if(is_f80_type(type)) {
      throw ParseError("f80 stores must be materialized through scratch storage");
    }
    const size_t width = type_exec_width_bytes(type);
    const string op = width >= 8 ? "move64" : move_opcode(width);
    const cy::Operand src = register_operand(reg_alias(reg_base, width >= 8 ? 8 : width));
    if(storage.kind == lir::Operand::OP_SLOT || storage.kind == lir::Operand::OP_TEMP) {
      builder_.opcode(op, {memory_reg("bp", slot_offset(layout, storage.text)), src});
      return;
    }
    if(storage.kind == lir::Operand::OP_GLOBAL) {
      builder_.opcode(op, {memory_label(global_symbol(storage.text)), src});
      return;
    }
    throw ParseError("invalid store operand " + storage.text);
  }

  void emit_advance_pointer(const string & reg, size_t amount)
  {
    if(amount != 0) {
      builder_.opcode("iadd64",
                      {register_operand(reg),
                       register_operand(reg),
                       immediate_integer(static_cast<long long>(amount))});
    }
  }

  void emit_copy_chunk(size_t chunk_bytes,
                       const string & dst_reg,
                       const string & src_reg)
  {
    if(chunk_bytes == 8) {
      builder_.opcode("move64", {register_operand("z64"), memory_reg(src_reg)});
      builder_.opcode("move64", {memory_reg(dst_reg), register_operand("z64")});
      return;
    }
    if(chunk_bytes == 4) {
      builder_.opcode("move32", {register_operand("z32"), memory_reg(src_reg)});
      builder_.opcode("move32", {memory_reg(dst_reg), register_operand("z32")});
      return;
    }
    if(chunk_bytes == 2) {
      builder_.opcode("move16", {register_operand("z16"), memory_reg(src_reg)});
      builder_.opcode("move16", {memory_reg(dst_reg), register_operand("z16")});
      return;
    }
    if(chunk_bytes == 1) {
      builder_.opcode("move8", {register_operand("z8"), memory_reg(src_reg)});
      builder_.opcode("move8", {memory_reg(dst_reg), register_operand("z8")});
      return;
    }
    throw ParseError("unsupported copy chunk size");
  }

  void emit_zero_chunk(size_t chunk_bytes, const string & dst_reg)
  {
    if(chunk_bytes == 8) {
      builder_.opcode("move64", {memory_reg(dst_reg), register_operand("z64")});
      return;
    }
    if(chunk_bytes == 4) {
      builder_.opcode("move32", {memory_reg(dst_reg), register_operand("z32")});
      return;
    }
    if(chunk_bytes == 2) {
      builder_.opcode("move16", {memory_reg(dst_reg), register_operand("z16")});
      return;
    }
    if(chunk_bytes == 1) {
      builder_.opcode("move8", {memory_reg(dst_reg), register_operand("z8")});
      return;
    }
    throw ParseError("unsupported zero chunk size");
  }

  void emit_copy_memory(const string & dst_reg,
                        long long dst_offset,
                        const string & src_reg,
                        long long src_offset,
                        size_t bytes)
  {
    static const size_t chunks[] = {8, 4, 2, 1};
    size_t copied = 0;
    while(copied < bytes) {
      size_t chunk = 0;
      for(size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if(bytes - copied >= chunks[i]) {
          chunk = chunks[i];
          break;
        }
      }
      if(chunk == 8) {
        builder_.opcode("move64",
                        {register_operand("z64"),
                         memory_reg(src_reg, src_offset + static_cast<long long>(copied))});
        builder_.opcode("move64",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z64")});
      } else if(chunk == 4) {
        builder_.opcode("move32",
                        {register_operand("z32"),
                         memory_reg(src_reg, src_offset + static_cast<long long>(copied))});
        builder_.opcode("move32",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z32")});
      } else if(chunk == 2) {
        builder_.opcode("move16",
                        {register_operand("z16"),
                         memory_reg(src_reg, src_offset + static_cast<long long>(copied))});
        builder_.opcode("move16",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z16")});
      } else {
        builder_.opcode("move8",
                        {register_operand("z8"),
                         memory_reg(src_reg, src_offset + static_cast<long long>(copied))});
        builder_.opcode("move8",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z8")});
      }
      copied += chunk;
    }
  }

  void emit_zero_memory(const string & dst_reg, long long dst_offset, size_t bytes)
  {
    builder_.opcode("move64", {register_operand("z64"), immediate_integer(0)});
    static const size_t chunks[] = {8, 4, 2, 1};
    size_t copied = 0;
    while(copied < bytes) {
      size_t chunk = 0;
      for(size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if(bytes - copied >= chunks[i]) {
          chunk = chunks[i];
          break;
        }
      }
      if(chunk == 8) {
        builder_.opcode("move64",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z64")});
      } else if(chunk == 4) {
        builder_.opcode("move32",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z32")});
      } else if(chunk == 2) {
        builder_.opcode("move16",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z16")});
      } else {
        builder_.opcode("move8",
                        {memory_reg(dst_reg, dst_offset + static_cast<long long>(copied)),
                         register_operand("z8")});
      }
      copied += chunk;
    }
  }

  void emit_zero_f80_padding_at(const string & dst_reg, long long dst_offset)
  {
    emit_zero_memory(dst_reg, dst_offset + 10, 6);
  }

  void emit_load_storage_address(const FunctionLayout & layout,
                                 const lir::Operand & storage,
                                 const string & reg)
  {
    LowType ptr_type;
    ptr_type.text = "ptr";
    if(storage.kind == lir::Operand::OP_SLOT || storage.kind == lir::Operand::OP_GLOBAL) {
      emit_load_address(layout, storage, reg);
      return;
    }
    if(storage.kind == lir::Operand::OP_TEMP) {
      map<string, LowType>::const_iterator found = layout.storage_type.find(storage.text);
      if(found != layout.storage_type.end() && found->second.text == "ptr") {
        emit_load_value(layout, storage, ptr_type, reg[0]);
      } else {
        emit_load_address(layout, storage, reg);
      }
      return;
    }
    throw ParseError("invalid storage address operand " + storage.text);
  }

  void emit_materialize_f80_value(const FunctionLayout & layout,
                                  const lir::Operand & operand,
                                  size_t scratch_index)
  {
    const long long dst = scratch_offset(layout, scratch_index);
    if(operand.kind == lir::Operand::OP_FLOAT || operand.kind == lir::Operand::OP_INTEGER) {
      LowType f80_type;
      f80_type.text = "f80";
      builder_.opcode("move80",
                      {memory_reg("bp", dst),
                       immediate_literal(scalar_literal(operand, f80_type))});
      emit_zero_f80_padding_at("bp", dst);
      return;
    }
    if(operand.kind == lir::Operand::OP_TEMP || operand.kind == lir::Operand::OP_SLOT ||
       operand.kind == lir::Operand::OP_GLOBAL) {
      emit_load_storage_address(layout, operand, "x64");
      emit_copy_memory("bp", dst, "x64", 0, 16);
      return;
    }
    throw ParseError("unsupported f80 value operand " + operand.text);
  }

  void emit_load_f80_from_storage(const FunctionLayout & layout,
                                  const lir::Operand & storage,
                                  size_t scratch_index)
  {
    const long long dst = scratch_offset(layout, scratch_index);
    emit_load_storage_address(layout, storage, "x64");
    emit_copy_memory("bp", dst, "x64", 0, 16);
  }

  void emit_store_f80_to_storage(const FunctionLayout & layout,
                                 const lir::Operand & storage,
                                 size_t scratch_index)
  {
    emit_load_storage_address(layout, storage, "x64");
    emit_copy_memory("x64", 0, "bp", scratch_offset(layout, scratch_index), 16);
  }

  void emit_store_f80_to_temp(const FunctionLayout & layout,
                              const string & dest,
                              size_t scratch_index)
  {
    emit_copy_memory("bp",
                     slot_offset(layout, dest),
                     "bp",
                     scratch_offset(layout, scratch_index),
                     16);
  }

  void emit_materialize_convert_source_to_f80(const FunctionLayout & layout,
                                              const Instruction & inst,
                                              size_t scratch_index)
  {
    const long long dst = scratch_offset(layout, scratch_index);
    const size_t source_width = type_exec_width_bytes(inst.source_type);
    if(inst.op == "sitofp" || inst.op == "uitofp") {
      emit_load_value(layout, inst.first, inst.source_type, 'x');
      builder_.opcode(string(inst.op == "sitofp" ? "s" : "u") +
                          to_string(source_width * 8) + "convf80",
                      {memory_reg("bp", dst),
                       register_operand(reg_alias('x',
                                                  source_width >= 8 ? 8 :
                                                  source_width))});
      emit_zero_f80_padding_at("bp", dst);
      return;
    }

    if(is_f80_type(inst.source_type)) {
      emit_materialize_f80_value(layout, inst.first, scratch_index);
      return;
    }

    if(inst.op == "fpext" || inst.op == "fptrunc" ||
       inst.op == "fptosi" || inst.op == "fptoui") {
      emit_load_value(layout, inst.first, inst.source_type, 'x');
      builder_.opcode(string("f") + to_string(source_width * 8) + "convf80",
                      {memory_reg("bp", dst),
                       register_operand(reg_alias('x',
                                                  source_width >= 8 ? 8 :
                                                  source_width))});
      emit_zero_f80_padding_at("bp", dst);
      return;
    }

    throw ParseError("unsupported conversion op " + inst.op);
  }

  void emit_copy_bytes(const FunctionLayout & layout,
                       const lir::Operand & dst,
                       const lir::Operand & src,
                       size_t bytes)
  {
    emit_load_storage_address(layout, dst, "x64");
    emit_load_storage_address(layout, src, "y64");
    static const size_t chunks[] = {8, 4, 2, 1};
    size_t remaining = bytes;
    while(remaining != 0) {
      size_t chunk = 0;
      for(size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if(remaining >= chunks[i]) {
          chunk = chunks[i];
          break;
        }
      }
      emit_copy_chunk(chunk, "x64", "y64");
      remaining -= chunk;
      if(remaining != 0) {
        emit_advance_pointer("x64", chunk);
        emit_advance_pointer("y64", chunk);
      }
    }
  }

  void emit_zero_bytes(const FunctionLayout & layout,
                       const lir::Operand & dst,
                       size_t bytes)
  {
    emit_load_storage_address(layout, dst, "x64");
    builder_.opcode("move64", {register_operand("z64"), immediate_integer(0)});
    static const size_t chunks[] = {8, 4, 2, 1};
    size_t remaining = bytes;
    while(remaining != 0) {
      size_t chunk = 0;
      for(size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if(remaining >= chunks[i]) {
          chunk = chunks[i];
          break;
        }
      }
      emit_zero_chunk(chunk, "x64");
      remaining -= chunk;
      if(remaining != 0) {
        emit_advance_pointer("x64", chunk);
      }
    }
  }

  void emit_store_to_pointer(const FunctionLayout & layout,
                             const lir::Operand & pointer,
                             const LowType & type,
                             char reg_base)
  {
    if(pointer.kind != lir::Operand::OP_TEMP) {
      throw ParseError("pointer store requires temp");
    }
    builder_.opcode("move64",
                    {register_operand("y64"),
                     memory_reg("bp", slot_offset(layout, pointer.text))});
    const size_t width = type_size(type);
    builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                    {memory_reg("y64"),
                     register_operand(reg_alias(reg_base, width >= 8 ? 8 : width))});
  }

  void emit_load_exception_value(const LowType & type, char reg_base)
  {
    const size_t width = type_size(type);
    if(width < 4) {
      move_zero_register(reg_base);
    }
    builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                    {register_operand(reg_alias(reg_base, width >= 8 ? 8 : width)),
                     memory_label(global_symbol(eh_runtime::kEhValueSymbol))});
    if(width < 8 && is_sign_extended_integer_type(type)) {
      sign_extend_to_i64(reg_base, width);
    }
  }

  void emit_load_exception_selector(const LowType & type, char reg_base)
  {
    if(type.text != "i32" && type.text != "i64") {
      throw ParseError("exception selector requires i32 or i64 type");
    }
    move_zero_register(reg_base);
  }

  void emit_store_exception_value(const LowType & type, char reg_base)
  {
    const size_t width = type_size(type);
    builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                    {memory_label(global_symbol(eh_runtime::kEhValueSymbol)),
                     register_operand(reg_alias(reg_base, width >= 8 ? 8 : width))});
  }

  void emit_eh_push_record(const Function & function, const string & handler_label)
  {
    const string handler_symbol = block_symbol(function, handler_label);
    builder_.opcode("isub64",
                    {register_operand("sp"), register_operand("sp"), immediate_integer(32)});
    builder_.opcode("move64",
                    {register_operand("z64"),
                     memory_label(global_symbol(eh_runtime::kEhTopSymbol))});
    builder_.opcode("move64", {memory_reg("sp"), register_operand("z64")});
    builder_.opcode("move64", {register_operand("z64"), immediate_label(handler_symbol)});
    builder_.opcode("move64", {memory_reg("sp", 8), register_operand("z64")});
    builder_.opcode("move64", {memory_reg("sp", 16), register_operand("bp")});
    builder_.opcode("move64", {register_operand("z64"), register_operand("sp")});
    builder_.opcode("iadd64",
                    {register_operand("z64"), register_operand("z64"), immediate_integer(32)});
    builder_.opcode("move64", {memory_reg("sp", 24), register_operand("z64")});
    builder_.opcode("move64", {register_operand("z64"), register_operand("sp")});
    builder_.opcode("move64",
                    {memory_label(global_symbol(eh_runtime::kEhTopSymbol)),
                     register_operand("z64")});
  }

  void emit_eh_pop_record()
  {
    builder_.opcode("move64",
                    {register_operand("x64"),
                     memory_label(global_symbol(eh_runtime::kEhTopSymbol))});
    builder_.opcode("move64", {register_operand("y64"), memory_reg("x64")});
    builder_.opcode("move64",
                    {memory_label(global_symbol(eh_runtime::kEhTopSymbol)),
                     register_operand("y64")});
    builder_.opcode("move64", {register_operand("sp"), register_operand("x64")});
    builder_.opcode("iadd64",
                    {register_operand("sp"), register_operand("sp"), immediate_integer(32)});
  }

  void emit_eh_transfer(bool keep_current_value)
  {
    const string has_handler_label = make_internal_label("__eh_handler");
    const string no_handler_label = make_internal_label("__eh_unhandled");

    (void)keep_current_value;
    builder_.opcode("move64",
                    {register_operand("x64"),
                     memory_label(global_symbol(eh_runtime::kEhTopSymbol))});
    builder_.opcode("ieq64",
                    {register_operand("z8"), register_operand("x64"), immediate_integer(0)});
    builder_.opcode("jumpif",
                    {register_operand("z8"), immediate_label(no_handler_label)});

    builder_.label(has_handler_label);
    builder_.opcode("move64", {register_operand("y64"), memory_reg("x64")});
    builder_.opcode("move64",
                    {memory_label(global_symbol(eh_runtime::kEhTopSymbol)),
                     register_operand("y64")});
    builder_.opcode("move64", {register_operand("z64"), memory_reg("x64", 8)});
    builder_.opcode("move64", {register_operand("bp"), memory_reg("x64", 16)});
    builder_.opcode("move64", {register_operand("sp"), memory_reg("x64", 24)});
    builder_.opcode("jump", {register_operand("z64")});

    builder_.label(no_handler_label);
    builder_.opcode("move64",
                    {register_operand("x64"),
                     memory_label(global_symbol(eh_runtime::kEhValueSymbol))});
    builder_.opcode("call", {immediate_label(function_symbol(eh_runtime::kEhUnhandledSymbol))});
    builder_.opcode("syscall1",
                    {register_operand("t64"),
                     immediate_integer(60),
                     register_operand("x64")});
  }

  void emit_eh_unhandled_function()
  {
    const string symbol = function_symbol(eh_runtime::kEhUnhandledSymbol);
    builder_.label(symbol);
    builder_.opcode("syscall1",
                    {register_operand("t64"),
                     immediate_integer(60),
                     register_operand("x64")});
  }

  void emit_eh_runtime_globals()
  {
    builder_.label(global_symbol(eh_runtime::kEhTopSymbol));
    builder_.data_integer(8, 0);
    builder_.label(global_symbol(eh_runtime::kEhValueSymbol));
    builder_.data_integer(8, 0);
  }

  const Function * lookup_function(const string & name) const
  {
    for(size_t i = 0; i < program_.functions.size(); ++i) {
      if(program_.functions[i].name == name) {
        return &program_.functions[i];
      }
    }
    return NULL;
  }

  void emit_load_abi_value(const FunctionLayout::AbiLocation & abi,
                           size_t width_bytes,
                           char reg_base)
  {
    if(width_bytes > 8) {
      throw ParseError("unsupported ABI value width");
    }
    if(abi.in_reg) {
      const string src = reg_alias(abi_reg(abi.reg_index)[0], width_bytes == 8 ? 8 : width_bytes);
      const string dst = reg_alias(reg_base, width_bytes >= 8 ? 8 : width_bytes);
      if(src != dst) {
        builder_.opcode(width_bytes >= 8 ? "move64" : move_opcode(width_bytes),
                        {register_operand(dst), register_operand(src)});
      }
      return;
    }
    if(width_bytes < 4) {
      move_zero_register(reg_base);
    }
    builder_.opcode(width_bytes >= 8 ? "move64" : move_opcode(width_bytes),
                    {register_operand(reg_alias(reg_base, width_bytes >= 8 ? 8 : width_bytes)),
                     memory_reg("bp", abi.stack_offset)});
  }

  void emit_store_abi_stack_value(size_t stack_index, size_t width_bytes, char reg_base)
  {
    const long long offset = static_cast<long long>(stack_index * 8);
    builder_.opcode("move64",
                    {memory_reg("sp", offset), immediate_integer(0)});
    builder_.opcode(width_bytes >= 8 ? "move64" : move_opcode(width_bytes),
                    {memory_reg("sp", offset),
                     register_operand(reg_alias(reg_base, width_bytes >= 8 ? 8 : width_bytes))});
  }

  void emit_load_f80_pointer_arg(const FunctionLayout::AbiLocation & abi, const string & reg)
  {
    if(abi.in_reg) {
      builder_.opcode("move64",
                      {register_operand(reg), register_operand(abi_reg(abi.reg_index))});
      return;
    }
    builder_.opcode("move64",
                    {register_operand(reg), memory_reg("bp", abi.stack_offset)});
  }

  void emit_instruction(const Function & function,
                        const FunctionLayout & layout,
                        const Instruction & inst,
                        const string & epilogue_label)
  {
    switch(inst.kind) {
      case Instruction::IK_CONST:
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          emit_store_f80_to_temp(layout, inst.dest, 0);
          return;
        }
        emit_load_value(layout, inst.first, inst.type, 'x');
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_COPY:
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          emit_store_f80_to_temp(layout, inst.dest, 0);
          return;
        }
        emit_load_value(layout, inst.first, inst.type, 'x');
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_ADDR:
        emit_load_address(layout, inst.first, "x64");
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_LOAD:
        if(is_f80_type(inst.type)) {
          emit_load_f80_from_storage(layout, inst.first, 0);
          emit_store_f80_to_temp(layout, inst.dest, 0);
          return;
        }
        emit_load_dereference_or_storage(layout, inst.first, inst.type, 'x');
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_ATOMIC_LOAD:
        if(!is_atomic_scalar_type(inst.type)) {
          throw ParseError("unsupported atomic scalar type " + inst.type.text);
        }
        emit_load_value(layout, inst.first, LowType{"ptr"}, 'y');
        if(type_exec_width_bytes(inst.type) >= 8) {
          builder_.opcode("move64", {register_operand("x64"), memory_reg("y64")});
        } else {
          move_zero_register('x');
          builder_.opcode(move_opcode(type_exec_width_bytes(inst.type)),
                          {register_operand(reg_alias('x', type_exec_width_bytes(inst.type))),
                           memory_reg("y64")});
          if(is_sign_extended_integer_type(inst.type)) {
            sign_extend_to_i64('x', type_exec_width_bytes(inst.type));
          }
        }
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_STORE:
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          emit_store_f80_to_storage(layout, inst.second, 0);
          return;
        }
        emit_load_value(layout, inst.first, inst.type, 'x');
        if(inst.second.kind == lir::Operand::OP_TEMP &&
           layout.storage_type.find(inst.second.text) != layout.storage_type.end() &&
           layout.storage_type.find(inst.second.text)->second.text == "ptr") {
          emit_store_to_pointer(layout, inst.second, inst.type, 'x');
        } else {
          emit_store_to_storage(layout, inst.second, inst.type, 'x');
        }
        return;
      case Instruction::IK_ATOMIC_STORE:
        if(!is_atomic_scalar_type(inst.type)) {
          throw ParseError("unsupported atomic scalar type " + inst.type.text);
        }
        emit_load_value(layout, inst.second, LowType{"ptr"}, 'y');
        emit_load_value(layout, inst.first, inst.type, 'x');
        builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                            move_opcode(type_exec_width_bytes(inst.type)),
                        {memory_reg("y64"),
                         register_operand(reg_alias('x',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type)))});
        return;
      case Instruction::IK_ATOMIC_EXCHANGE:
        if(!is_atomic_scalar_type(inst.type)) {
          throw ParseError("unsupported atomic scalar type " + inst.type.text);
        }
        emit_load_value(layout, inst.first, LowType{"ptr"}, 'y');
        emit_load_value(layout, inst.second, inst.type, 'x');
        builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                            move_opcode(type_exec_width_bytes(inst.type)),
                        {register_operand(reg_alias('t',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type))),
                         memory_reg("y64")});
        builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                            move_opcode(type_exec_width_bytes(inst.type)),
                        {memory_reg("y64"),
                         register_operand(reg_alias('x',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type)))});
        move_zero_register('x');
        builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                            move_opcode(type_exec_width_bytes(inst.type)),
                        {register_operand(reg_alias('x',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type))),
                         register_operand(reg_alias('t',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type)))});
        if(type_exec_width_bytes(inst.type) < 8 &&
           is_sign_extended_integer_type(inst.type)) {
          sign_extend_to_i64('x', type_exec_width_bytes(inst.type));
        }
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_INDEX: {
        LowType ptr_type;
        ptr_type.text = "ptr";
        LowType index_type;
        index_type.text = "i64";
        emit_load_value(layout, inst.first, ptr_type, 'y');
        emit_load_value(layout, inst.second, index_type, 'x');
        LowType element_type;
        element_type.text = inst.op;
        const size_t scale = type_size(element_type);
        if(scale != 1) {
          builder_.opcode("move64", {register_operand("z64"),
                                     immediate_integer(static_cast<long long>(scale))});
          builder_.opcode("smul64",
                          {register_operand("x64"), register_operand("x64"), register_operand("z64")});
        }
        builder_.opcode("iadd64",
                        {register_operand("x64"), register_operand("y64"), register_operand("x64")});
        emit_store_temp(layout, inst.dest, 'x');
        return;
      }
      case Instruction::IK_UNARY: {
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          if(inst.op == "neg") {
            LowType f80_type;
            f80_type.text = "f80";
            builder_.opcode("move80",
                            {memory_reg("bp", scratch_offset(layout, 1)),
                             immediate_literal(floating_literal(0.0L, f80_type, "0.0L"))});
            emit_zero_f80_padding_at("bp", scratch_offset(layout, 1));
            builder_.opcode("fsub80",
                            {memory_reg("bp", scratch_offset(layout, 2)),
                             memory_reg("bp", scratch_offset(layout, 1)),
                             memory_reg("bp", scratch_offset(layout, 0))});
            emit_zero_f80_padding_at("bp", scratch_offset(layout, 2));
            emit_store_f80_to_temp(layout, inst.dest, 2);
            return;
          }
          if(inst.op == "not") {
            LowType f80_type;
            f80_type.text = "f80";
            builder_.opcode("feq80",
                            {register_operand("z8"),
                             memory_reg("bp", scratch_offset(layout, 0)),
                             immediate_literal(floating_literal(0.0L, f80_type, "0.0L"))});
            builder_.opcode("move64", {register_operand("x64"), immediate_integer(0)});
            builder_.opcode("move8", {register_operand("x8"), register_operand("z8")});
            emit_store_temp(layout, inst.dest, 'x');
            return;
          }
          throw ParseError("bitnot unsupported on floating type");
        }
        emit_load_value(layout, inst.first, inst.type, 'x');
        const size_t width = type_size(inst.type);
        const string xreg = reg_alias('x', width);
        const string yreg = reg_alias('y', width);
        if(inst.op == "decay") {
          if(inst.type.text != "ptr") {
            throw ParseError("decay unsupported on this type");
          }
        } else if(inst.op == "neg") {
          if(is_float_type(inst.type)) {
            builder_.opcode(move_opcode(width),
                            {register_operand(yreg),
                             immediate_literal(floating_literal(0.0L,
                                                               inst.type,
                                                               inst.type.text == "f32" ? "0.0f" : "0.0"))});
            builder_.opcode(opcode_with_width("fsub", width),
                            {register_operand(xreg), register_operand(yreg), register_operand(xreg)});
          } else {
            builder_.opcode("move64", {register_operand("y64"), immediate_integer(0)});
            builder_.opcode(opcode_with_width("isub", width),
                            {register_operand(xreg), register_operand(yreg), register_operand(xreg)});
          }
        } else if(inst.op == "not") {
          if(is_float_type(inst.type)) {
            builder_.opcode(opcode_with_width("feq", width),
                            {register_operand("z8"),
                             register_operand(xreg),
                             immediate_literal(floating_literal(0.0L,
                                                               inst.type,
                                                               inst.type.text == "f32" ? "0.0f" : "0.0"))});
          } else {
            builder_.opcode(opcode_with_width("ieq", width),
                            {register_operand("z8"), register_operand(xreg), immediate_integer(0)});
          }
            builder_.opcode("move64", {register_operand("x64"), immediate_integer(0)});
            builder_.opcode("move8", {register_operand("x8"), register_operand("z8")});
        } else if(inst.op == "bswap") {
          if(is_float_type(inst.type) || width == 1) {
            throw ParseError("bswap unsupported on this type");
          }
          builder_.opcode(opcode_with_width("bswap", width),
                          {register_operand(xreg), register_operand(xreg)});
        } else if(inst.op == "bitnot") {
          if(is_float_type(inst.type)) {
            throw ParseError("bitnot unsupported on floating type");
          }
          builder_.opcode(opcode_with_width("not", width),
                          {register_operand(xreg), register_operand(xreg)});
        } else {
          throw ParseError("unsupported unary op " + inst.op);
        }
        emit_store_temp(layout, inst.dest, 'x');
        return;
      }
      case Instruction::IK_BINARY: {
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          emit_materialize_f80_value(layout, inst.second, 1);
          string op;
          if(inst.op == "add") op = "fadd80";
          else if(inst.op == "sub") op = "fsub80";
          else if(inst.op == "mul") op = "fmul80";
          else if(inst.op == "div") op = "fdiv80";
          else throw ParseError("unsupported floating binary op " + inst.op);
          builder_.opcode(op,
                          {memory_reg("bp", scratch_offset(layout, 2)),
                           memory_reg("bp", scratch_offset(layout, 0)),
                           memory_reg("bp", scratch_offset(layout, 1))});
          emit_zero_f80_padding_at("bp", scratch_offset(layout, 2));
          emit_store_f80_to_temp(layout, inst.dest, 2);
          return;
        }
        emit_load_value(layout, inst.first, inst.type, 'y');
        emit_load_value(layout, inst.second, inst.type, 'x');
        const size_t width = type_size(inst.type);
        const string xreg = reg_alias('x', width);
        const string yreg = reg_alias('y', width);
        if(is_float_type(inst.type)) {
          string base;
          if(inst.op == "add") base = "fadd";
          else if(inst.op == "sub") base = "fsub";
          else if(inst.op == "mul") base = "fmul";
          else if(inst.op == "div") base = "fdiv";
          else throw ParseError("unsupported floating binary op " + inst.op);
          builder_.opcode(opcode_with_width(base, width),
                          {register_operand(xreg), register_operand(yreg), register_operand(xreg)});
        } else if(inst.op == "add" || inst.op == "sub" || inst.op == "mul" ||
           inst.op == "div" || inst.op == "mod" ||
           inst.op == "udiv" || inst.op == "umod" ||
           inst.op == "and" || inst.op == "or" || inst.op == "xor") {
          string base;
          if(inst.op == "add") base = "iadd";
          else if(inst.op == "sub") base = "isub";
          else if(inst.op == "mul") base = "smul";
          else if(inst.op == "div") base = "sdiv";
          else if(inst.op == "mod") base = "smod";
          else if(inst.op == "udiv") base = "udiv";
          else if(inst.op == "umod") base = "umod";
          else base = inst.op;
          builder_.opcode(opcode_with_width(base, width),
                          {register_operand(xreg), register_operand(yreg), register_operand(xreg)});
        } else if(inst.op == "shl" || inst.op == "shr" || inst.op == "ushr") {
          builder_.opcode("move64", {register_operand("z64"), register_operand("x64")});
          builder_.opcode("move8", {register_operand("x8"), register_operand("z8")});
          builder_.opcode(opcode_with_width(inst.op == "shl" ? "lshift" :
                                                inst.op == "ushr" ? "urshift" : "srshift",
                                            width),
                          {register_operand(xreg), register_operand(yreg), register_operand("x8")});
        } else {
          throw ParseError("unsupported binary op " + inst.op);
        }
        emit_store_temp(layout, inst.dest, 'x');
        return;
      }
      case Instruction::IK_CMP: {
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          emit_materialize_f80_value(layout, inst.second, 1);
          string op;
          if(inst.op == "eq") op = "feq80";
          else if(inst.op == "ne") op = "fne80";
          else if(inst.op == "lt") op = "flt80";
          else if(inst.op == "le") op = "fle80";
          else if(inst.op == "gt") op = "fgt80";
          else if(inst.op == "ge") op = "fge80";
          else throw ParseError("unsupported compare predicate " + inst.op);
          builder_.opcode(op,
                          {register_operand("z8"),
                           memory_reg("bp", scratch_offset(layout, 0)),
                           memory_reg("bp", scratch_offset(layout, 1))});
          builder_.opcode("move64", {register_operand("x64"), immediate_integer(0)});
          builder_.opcode("move8", {register_operand("x8"), register_operand("z8")});
          emit_store_temp(layout, inst.dest, 'x');
          return;
        }
        emit_load_value(layout, inst.first, inst.type, 'y');
        emit_load_value(layout, inst.second, inst.type, 'x');
        const size_t width = type_size(inst.type);
        string base;
        if(is_float_type(inst.type)) {
          if(inst.op == "eq") base = "feq";
          else if(inst.op == "ne") base = "fne";
          else if(inst.op == "lt") base = "flt";
          else if(inst.op == "le") base = "fle";
          else if(inst.op == "gt") base = "fgt";
          else if(inst.op == "ge") base = "fge";
        } else if(inst.op == "eq") base = "ieq";
        else if(inst.op == "ne") base = "ine";
        else if(inst.op == "lt") base = "slt";
        else if(inst.op == "le") base = "sle";
        else if(inst.op == "gt") base = "sgt";
        else if(inst.op == "ge") base = "sge";
        else if(inst.op == "ult") base = "ult";
        else if(inst.op == "ule") base = "ule";
        else if(inst.op == "ugt") base = "ugt";
        else if(inst.op == "uge") base = "uge";
        else throw ParseError("unsupported compare predicate " + inst.op);
        builder_.opcode(opcode_with_width(base, width),
                        {register_operand("z8"),
                         register_operand(reg_alias('y', width)),
                         register_operand(reg_alias('x', width))});
        builder_.opcode("move64", {register_operand("x64"), immediate_integer(0)});
        builder_.opcode("move8", {register_operand("x8"), register_operand("z8")});
        emit_store_temp(layout, inst.dest, 'x');
        return;
      }
      case Instruction::IK_CONVERT: {
        const bool to_float = inst.op == "sitofp" || inst.op == "uitofp";
        const bool to_int = inst.op == "fptosi" || inst.op == "fptoui";
        const bool float_widen = inst.op == "fpext";
        const bool float_narrow = inst.op == "fptrunc";
        const bool integer_sign_extend = inst.op == "sext";
        const bool integer_zero_extend = inst.op == "zext";
        const bool integer_trunc = inst.op == "trunc";
        if(!(to_float || to_int || float_widen || float_narrow ||
             integer_sign_extend || integer_zero_extend || integer_trunc)) {
          throw ParseError("unsupported conversion op " + inst.op);
        }
        if((to_float && (!is_float_type(inst.type) ||
                         !is_integer_scalar_type(inst.source_type))) ||
           (to_int && (!is_float_type(inst.source_type) ||
                       !is_integer_scalar_type(inst.type))) ||
           ((integer_sign_extend || integer_zero_extend || integer_trunc) &&
            (!is_integer_scalar_type(inst.type) ||
             !is_integer_scalar_type(inst.source_type))) ||
           ((float_widen || float_narrow) &&
            (!is_float_type(inst.type) || !is_float_type(inst.source_type)))) {
          throw ParseError("invalid conversion signature for " + inst.op);
        }
        if(float_widen &&
           type_exec_width_bytes(inst.source_type) >= type_exec_width_bytes(inst.type)) {
          throw ParseError("fpext requires wider destination type");
        }
        if(float_narrow &&
           type_exec_width_bytes(inst.source_type) <= type_exec_width_bytes(inst.type)) {
          throw ParseError("fptrunc requires narrower destination type");
        }
        if((integer_sign_extend || integer_zero_extend) &&
           type_exec_width_bytes(inst.source_type) >= type_exec_width_bytes(inst.type)) {
          throw ParseError(inst.op + " requires wider destination type");
        }
        if(integer_trunc &&
           type_exec_width_bytes(inst.source_type) <= type_exec_width_bytes(inst.type)) {
          throw ParseError("trunc requires narrower destination type");
        }
        if(integer_sign_extend || integer_zero_extend || integer_trunc) {
          emit_load_value(layout, inst.first, inst.source_type, 'x');
          if(integer_sign_extend) {
            sign_extend_to_i64('x', type_exec_width_bytes(inst.source_type));
          }
          emit_store_temp(layout, inst.dest, 'x');
          return;
        }
        emit_materialize_convert_source_to_f80(layout, inst, 0);
        if(inst.op == "sitofp" || inst.op == "uitofp" ||
           inst.op == "fpext" || inst.op == "fptrunc") {
          if(is_f80_type(inst.type)) {
            emit_store_f80_to_temp(layout, inst.dest, 0);
            return;
          }
          builder_.opcode(string("f80convf") +
                              to_string(type_exec_width_bytes(inst.type) * 8),
                          {memory_reg("bp", slot_offset(layout, inst.dest)),
                           memory_reg("bp", scratch_offset(layout, 0))});
          return;
        }
        if(inst.op == "fptosi" || inst.op == "fptoui") {
          builder_.opcode(string("f80conv") + (inst.op == "fptosi" ? "s" : "u") +
                              to_string(type_exec_width_bytes(inst.type) * 8),
                          {memory_reg("bp", slot_offset(layout, inst.dest)),
                           memory_reg("bp", scratch_offset(layout, 0))});
          return;
        }
        throw ParseError("unsupported conversion op " + inst.op);
      }
      case Instruction::IK_ATOMIC_ADD_FETCH:
        if(!is_atomic_scalar_type(inst.type)) {
          throw ParseError("unsupported atomic scalar type " + inst.type.text);
        }
        emit_load_value(layout, inst.first, LowType{"ptr"}, 'y');
        builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                            move_opcode(type_exec_width_bytes(inst.type)),
                        {register_operand(reg_alias('x',
                                                    type_exec_width_bytes(inst.type) >= 8 ?
                                                        8 :
                                                        type_exec_width_bytes(inst.type))),
                         memory_reg("y64")});
        emit_load_value(layout, inst.second, inst.type, 'z');
        if(inst.type.text == "ptr") {
          builder_.opcode("iadd64",
                          {register_operand("x64"),
                           register_operand("x64"),
                           register_operand("z64")});
          builder_.opcode("move64", {memory_reg("y64"), register_operand("x64")});
        } else {
          builder_.opcode(opcode_with_width("iadd", type_exec_width_bytes(inst.type)),
                          {register_operand(reg_alias('x', type_exec_width_bytes(inst.type))),
                           register_operand(reg_alias('x', type_exec_width_bytes(inst.type))),
                           register_operand(reg_alias('z', type_exec_width_bytes(inst.type)))});
          builder_.opcode(type_exec_width_bytes(inst.type) >= 8 ? "move64" :
                              move_opcode(type_exec_width_bytes(inst.type)),
                          {memory_reg("y64"),
                           register_operand(reg_alias('x',
                                                      type_exec_width_bytes(inst.type) >= 8 ?
                                                          8 :
                                                          type_exec_width_bytes(inst.type)))});
        }
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_ATOMIC_COMPARE_EXCHANGE: {
        if(!is_atomic_scalar_type(inst.type)) {
          throw ParseError("unsupported atomic scalar type " + inst.type.text);
        }
        const size_t width = type_exec_width_bytes(inst.type);
        const string width_move = width >= 8 ? "move64" : move_opcode(width);
        const string xreg = reg_alias('x', width >= 8 ? 8 : width);
        const string treg = reg_alias('t', width >= 8 ? 8 : width);
        const string zreg = reg_alias('z', width >= 8 ? 8 : width);
        const string success_label = make_internal_label("__atomic_cmpxchg_success");
        const string end_label = make_internal_label("__atomic_cmpxchg_end");
        emit_load_value(layout, inst.first, LowType{"ptr"}, 'y');
        emit_load_value(layout, inst.second, LowType{"ptr"}, 'z');
        builder_.opcode(width_move, {register_operand(treg), memory_reg("y64")});
        builder_.opcode(width_move, {register_operand(xreg), memory_reg("z64")});
        builder_.opcode(opcode_with_width("ieq", width),
                        {register_operand("x8"),
                         register_operand(treg),
                         register_operand(xreg)});
        builder_.opcode("jumpif",
                        {register_operand("x8"), immediate_label(success_label)});

        builder_.opcode(width_move, {memory_reg("z64"), register_operand(treg)});
        builder_.opcode("move64", {register_operand("x64"), immediate_integer(0)});
        emit_store_temp(layout, inst.dest, 'x');
        builder_.opcode("jump", {immediate_label(end_label)});

        builder_.label(success_label);
        emit_load_value(layout, inst.third, inst.type, 'x');
        builder_.opcode(width_move, {memory_reg("y64"), register_operand(xreg)});
        builder_.opcode("move64", {register_operand("x64"), immediate_integer(1)});
        emit_store_temp(layout, inst.dest, 'x');
        builder_.label(end_label);
        return;
      }
      case Instruction::IK_ATOMIC_THREAD_FENCE:
      case Instruction::IK_ATOMIC_SIGNAL_FENCE:
        (void)atomic_order_value(inst.first);
        return;
      case Instruction::IK_VA_START:
      case Instruction::IK_VA_ARG:
        throw ParseError("host varargs are unsupported by the CY86 backend");
      case Instruction::IK_STACK_ALLOC:
        emit_load_value(layout, inst.first, LowType{"i64"}, 'x');
        builder_.opcode("iadd64",
                        {register_operand("x64"),
                         register_operand("x64"),
                         immediate_integer(7)});
        builder_.opcode("move64",
                        {register_operand("t64"), immediate_integer(-8)});
        builder_.opcode("and64",
                        {register_operand("x64"),
                         register_operand("x64"),
                         register_operand("t64")});
        builder_.opcode("isub64",
                        {register_operand("sp"),
                         register_operand("sp"),
                         register_operand("x64")});
        builder_.opcode("move64", {register_operand("x64"), register_operand("sp")});
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_CALL: {
        const bool indirect = inst.first.kind != lir::Operand::OP_GLOBAL ||
                              function_symbols_.count(inst.first.text) == 0;
        const Function * target = indirect ? NULL : lookup_function(inst.first.text);
        vector<LowType> arg_types;
        vector<lir::ParamPassingMode> arg_passing;
        bool returns_hidden_result = false;
        if(target != NULL) {
          returns_hidden_result = uses_hidden_return_ptr(target->return_type);
          for(size_t i = 0; i < target->params.size(); ++i) {
            arg_types.push_back(target->params[i].type);
            arg_passing.push_back(target->params[i].metadata.passing);
          }
        } else if(inst.has_call_signature) {
          returns_hidden_result = uses_hidden_return_ptr(inst.call_return_type);
          for(size_t i = 0; i < inst.call_params.size(); ++i) {
            arg_types.push_back(inst.call_params[i].type);
            arg_passing.push_back(inst.call_params[i].metadata.passing);
          }
        } else {
          returns_hidden_result = false;
          for(size_t i = 0; i < inst.args.size(); ++i) {
            LowType arg_type;
            arg_type.text = "i64";
            arg_types.push_back(arg_type);
            arg_passing.push_back(lir::PPM_DIRECT);
          }
        }
        for(size_t i = arg_types.size(); i < inst.args.size(); ++i) {
          arg_types.push_back(operand_type(layout, inst.args[i]));
          arg_passing.push_back(lir::PPM_DIRECT);
        }
        vector<FunctionLayout::AbiLocation> abi_args;
        size_t abi_count = returns_hidden_result ? 1 : 0;
        for(size_t i = 0; i < arg_types.size(); ++i) {
          abi_args.push_back(abi_location(abi_count++));
        }
        const size_t stack_arg_count = abi_count > 4 ? abi_count - 4 : 0;
        if(stack_arg_count != 0) {
          builder_.opcode("isub64",
                          {register_operand("sp"),
                           register_operand("sp"),
                           immediate_integer(static_cast<long long>(stack_arg_count * 8))});
        }
        if(returns_hidden_result && !inst.call_returns_void) {
          FunctionLayout::AbiLocation retloc = abi_location(0);
          emit_bp_relative_address(slot_offset(layout, inst.dest), "x64");
          if(retloc.in_reg) {
            builder_.opcode("move64",
                            {register_operand(abi_reg(retloc.reg_index)),
                             register_operand("x64")});
          } else {
            builder_.opcode("move64",
                            {memory_reg("sp", (retloc.stack_offset - 16)),
                             register_operand("x64")});
          }
        }
        if(indirect) {
          LowType ptr_type;
          ptr_type.text = "ptr";
          emit_load_value(layout, inst.first, ptr_type, 'x');
          builder_.opcode("isub64",
                          {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
          builder_.opcode("move64", {memory_reg("sp"), register_operand("x64")});
        }
        for(size_t i = 0; i < inst.args.size(); ++i) {
          const LowType & arg_type = arg_types[i];
          const lir::ParamPassingMode passing = arg_passing[i];
          const FunctionLayout::AbiLocation & abi = abi_args[i];
          if(is_direct_object_type(arg_type)) {
            emit_load_storage_address(layout, inst.args[i], "x64");
            if(abi.in_reg) {
              builder_.opcode("move64",
                              {register_operand(abi_reg(abi.reg_index)),
                               register_operand("x64")});
            } else {
              builder_.opcode("move64",
                              {memory_reg("sp", (abi.stack_offset - 16)),
                               register_operand("x64")});
            }
            continue;
          }
          if(uses_storage_address_passing(passing)) {
            emit_load_storage_address(layout, inst.args[i], "x64");
            if(abi.in_reg) {
              builder_.opcode("move64",
                              {register_operand(abi_reg(abi.reg_index)),
                               register_operand("x64")});
            } else {
              builder_.opcode("move64",
                              {memory_reg("sp", (abi.stack_offset - 16)),
                               register_operand("x64")});
            }
            continue;
          }
          if(is_f80_type(arg_type)) {
            if(inst.args[i].kind == lir::Operand::OP_INTEGER ||
               inst.args[i].kind == lir::Operand::OP_FLOAT) {
              const size_t scratch_index = i < 4 ? i : 3;
              emit_materialize_f80_value(layout, inst.args[i], scratch_index);
              emit_bp_relative_address(scratch_offset(layout, scratch_index), "x64");
            } else {
              emit_load_storage_address(layout, inst.args[i], "x64");
            }
            if(abi.in_reg) {
              builder_.opcode("move64",
                              {register_operand(abi_reg(abi.reg_index)),
                               register_operand("x64")});
            } else {
              builder_.opcode("move64",
                              {memory_reg("sp", (abi.stack_offset - 16)),
                               register_operand("x64")});
            }
            continue;
          }
          if(abi.in_reg) {
            emit_load_value(layout, inst.args[i], arg_type, abi_reg(abi.reg_index)[0]);
          } else {
            emit_load_value(layout, inst.args[i], arg_type, 'x');
            emit_store_abi_stack_value(static_cast<size_t>((abi.stack_offset - 16) / 8),
                                       type_exec_width_bytes(arg_type),
                                       'x');
          }
        }
        if(indirect) {
          builder_.opcode("call", {memory_reg("sp")});
          builder_.opcode("iadd64",
                          {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
        } else {
          builder_.opcode("call", {immediate_label(function_symbol(inst.first.text))});
        }
        if(stack_arg_count != 0) {
          builder_.opcode("iadd64",
                          {register_operand("sp"),
                           register_operand("sp"),
                           immediate_integer(static_cast<long long>(stack_arg_count * 8))});
        }
        if(!inst.call_returns_void && !returns_hidden_result) {
          emit_store_temp(layout, inst.dest, 'x');
        }
        return;
      }
      case Instruction::IK_COPYOBJ:
        emit_copy_bytes(layout, inst.second, inst.first, inst.byte_count);
        return;
      case Instruction::IK_ZEROINIT:
        emit_zero_bytes(layout, inst.first, inst.byte_count);
        return;
      case Instruction::IK_EH_TRY:
      case Instruction::IK_EH_CLEANUP:
        emit_eh_push_record(function, inst.first.text);
        return;
      case Instruction::IK_EH_CATCH:
      case Instruction::IK_EH_CLEANUP_CLAUSE:
      case Instruction::IK_EH_FILTER:
      case Instruction::IK_EH_CATCH_ALL:
        return;
      case Instruction::IK_EH_END:
        emit_eh_pop_record();
        return;
      case Instruction::IK_THROW:
        emit_load_value(layout, inst.first, inst.type, 'x');
        emit_store_exception_value(inst.type, 'x');
        emit_eh_transfer(false);
        return;
      case Instruction::IK_EXCEPTION:
        emit_load_exception_value(inst.type, 'x');
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_EXCEPTION_SELECTOR:
        emit_load_exception_selector(inst.type, 'x');
        emit_store_temp(layout, inst.dest, 'x');
        return;
      case Instruction::IK_RESUME:
        emit_eh_transfer(true);
        return;
      case Instruction::IK_JUMP:
        builder_.opcode("jump", {immediate_label(block_symbol(function, inst.first.text))});
        return;
      case Instruction::IK_BRANCH: {
        LowType cond_type;
        cond_type.text = "i64";
        emit_load_value(layout, inst.first, cond_type, 'x');
        builder_.opcode("ieq64",
                        {register_operand("z8"), register_operand("x64"), immediate_integer(0)});
        builder_.opcode("jumpif",
                        {register_operand("z8"),
                         immediate_label(block_symbol(function, inst.third.text))});
        builder_.opcode("jump",
                        {immediate_label(block_symbol(function, inst.second.text))});
        return;
      }
      case Instruction::IK_SWITCH: {
        LowType selector_type = operand_type(layout, inst.first);
        if(!is_integer_scalar_type(selector_type)) {
          throw ParseError("switch requires integer selector");
        }
        LowType compare_type;
        compare_type.text = "i64";
        emit_load_value(layout, inst.first, compare_type, 'x');
        for(size_t i = 0; i + 1 < inst.args.size(); i += 2) {
          emit_load_value(layout, inst.args[i], compare_type, 't');
          builder_.opcode("ieq64",
                          {register_operand("z8"),
                           register_operand("x64"),
                           register_operand("t64")});
          builder_.opcode("jumpif",
                          {register_operand("z8"),
                           immediate_label(block_symbol(function, inst.args[i + 1].text))});
        }
        builder_.opcode("jump",
                        {immediate_label(block_symbol(function, inst.second.text))});
        return;
      }
      case Instruction::IK_RETURN:
        if(is_f80_type(inst.type)) {
          emit_materialize_f80_value(layout, inst.first, 0);
          builder_.opcode("move64",
                          {register_operand("x64"),
                           memory_reg("bp", layout.hidden_return_ptr_offset)});
          emit_copy_memory("x64", 0, "bp", scratch_offset(layout, 0), 16);
          builder_.opcode("jump", {immediate_label(epilogue_label)});
          return;
        }
        if(is_direct_object_type(inst.type)) {
          emit_load_storage_address(layout, inst.first, "x64");
          builder_.opcode("move64",
                          {register_operand("y64"),
                           memory_reg("bp", layout.hidden_return_ptr_offset)});
          emit_copy_memory("y64", 0, "x64", 0, type_storage_bytes(inst.type));
          builder_.opcode("jump", {immediate_label(epilogue_label)});
          return;
        }
        if(inst.type.text != "void") {
          emit_load_value(layout, inst.first, inst.type, 'x');
        }
        builder_.opcode("jump", {immediate_label(epilogue_label)});
        return;
    }
  }

  void validate_function(const Function & function) const
  {
    set<string> block_names;
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      if(!block_names.insert(function.blocks[i].label).second) {
        throw ParseError("duplicate block " + function.blocks[i].label + " in " + function.name);
      }
      if(function.blocks[i].instructions.empty()) {
        throw ParseError("empty block " + function.blocks[i].label + " in " + function.name);
      }
      const Instruction & tail = function.blocks[i].instructions.back();
      const bool terminates =
          tail.kind == Instruction::IK_JUMP ||
          tail.kind == Instruction::IK_BRANCH ||
          tail.kind == Instruction::IK_SWITCH ||
          tail.kind == Instruction::IK_RETURN ||
          tail.kind == Instruction::IK_THROW ||
          tail.kind == Instruction::IK_RESUME;
      if(!terminates) {
        throw ParseError("block " + function.blocks[i].label + " in " + function.name +
                         " is missing a terminator");
      }
    }
  }

  void emit_data_bytes(const vector<unsigned char> & bytes)
  {
    size_t offset = 0;
    static const size_t chunks[] = {8, 4, 2, 1};
    while(offset < bytes.size()) {
      size_t chunk = 0;
      for(size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if(bytes.size() - offset >= chunks[i]) {
          chunk = chunks[i];
          break;
        }
      }
      unsigned long long value = 0;
      for(size_t i = 0; i < chunk; ++i) {
        value |= static_cast<unsigned long long>(bytes[offset + i]) << (8 * i);
      }
      builder_.data_integer(chunk, static_cast<long long>(value));
      offset += chunk;
    }
  }

  void emit_f80_literal_object(const lir::Operand & operand)
  {
    LowType f80_type;
    f80_type.text = "f80";
    const vector<unsigned char> payload = cy::evaluated_literal_bytes(
        scalar_literal(operand, f80_type));
    emit_data_bytes(payload);
    builder_.zero_bytes(6);
  }

  void emit_function(const Function & function)
  {
    validate_function(function);
    const FunctionLayout layout = build_layout(function);
    const string symbol = function_symbol(function.name);
    const string epilogue_label = symbol + "__epilogue";

    builder_.label(symbol);
    builder_.opcode("isub64",
                    {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
    builder_.opcode("move64", {memory_reg("sp"), register_operand("bp")});
    builder_.opcode("move64", {register_operand("bp"), register_operand("sp")});
    if(frame_slots(layout) != 0) {
      builder_.opcode("isub64",
                      {register_operand("sp"), register_operand("sp"),
                       immediate_integer(static_cast<long long>(frame_slots(layout) * 8))});
    }
    if(layout.has_hidden_return_ptr) {
      const FunctionLayout::AbiLocation hidden_loc = abi_location(0);
      if(hidden_loc.in_reg) {
        builder_.opcode("move64",
                        {memory_reg("bp", layout.hidden_return_ptr_offset),
                         register_operand(abi_reg(hidden_loc.reg_index))});
      } else {
        builder_.opcode("move64",
                        {register_operand("x64"),
                         memory_reg("bp", hidden_loc.stack_offset)});
        builder_.opcode("move64",
                        {memory_reg("bp", layout.hidden_return_ptr_offset),
                         register_operand("x64")});
      }
    }
    for(size_t i = 0; i < layout.params.size(); ++i) {
      const LowType & type = layout.storage_type.find(layout.params[i])->second;
      const FunctionLayout::AbiLocation & abi = layout.param_abi[i];
      if(is_direct_object_type(type)) {
        if(abi.in_reg) {
          builder_.opcode("move64",
                          {register_operand("x64"),
                           register_operand(abi_reg(abi.reg_index))});
        } else {
          builder_.opcode("move64",
                          {register_operand("x64"),
                           memory_reg("bp", abi.stack_offset)});
        }
        emit_copy_memory("bp",
                         slot_offset(layout, layout.params[i]),
                         "x64",
                         0,
                         type_storage_bytes(type));
        continue;
      }
      if(is_f80_type(type)) {
        emit_load_f80_pointer_arg(abi, "x64");
        emit_copy_memory("bp", slot_offset(layout, layout.params[i]), "x64", 0, 16);
        continue;
      }
      const size_t width = type_exec_width_bytes(type);
      if(abi.in_reg) {
        builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                        {memory_reg("bp", slot_offset(layout, layout.params[i])),
                         register_operand(reg_alias(abi_reg(abi.reg_index)[0],
                                                    width >= 8 ? 8 : width))});
      } else {
        emit_load_abi_value(abi, width, 'x');
        builder_.opcode(width >= 8 ? "move64" : move_opcode(width),
                        {memory_reg("bp", slot_offset(layout, layout.params[i])),
                         register_operand(reg_alias('x', width >= 8 ? 8 : width))});
      }
    }

    for(size_t i = 0; i < function.blocks.size(); ++i) {
      builder_.label(block_symbol(function, function.blocks[i].label));
      for(size_t j = 0; j < function.blocks[i].instructions.size(); ++j) {
        emit_instruction(function, layout, function.blocks[i].instructions[j], epilogue_label);
      }
    }

    builder_.label(epilogue_label);
    builder_.opcode("move64", {register_operand("sp"), register_operand("bp")});
    builder_.opcode("move64", {register_operand("bp"), memory_reg("sp")});
    builder_.opcode("iadd64",
                    {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
    builder_.opcode("ret", {});
  }

  void emit_global(const GlobalDefinition & global)
  {
    builder_.label(global_symbol(global.name));
    if(global.structured) {
      size_t offset = 0;
      for(size_t i = 0; i < global.data_items.size(); ++i) {
        const GlobalDefinition::DataItem & item = global.data_items[i];
        if(item.kind == GlobalDefinition::DataItem::ITEM_ZERO) {
          builder_.zero_bytes(item.zero_bytes);
          offset += item.zero_bytes;
          continue;
        }

        const size_t width =
            item.kind == GlobalDefinition::DataItem::ITEM_ADDR ? 8
                                                               : type_size(item.type);
        while(width > 1 && (offset % width) != 0) {
          builder_.zero_bytes(1);
          ++offset;
        }

        if(item.kind == GlobalDefinition::DataItem::ITEM_ADDR) {
          const string symbol = function_symbols_.count(item.symbol) != 0
              ? function_symbol(item.symbol)
              : global_symbol(item.symbol);
          builder_.data_label(symbol);
          offset += 8;
          continue;
        }

        if(is_f80_type(item.type)) {
          emit_f80_literal_object(item.literal_operand);
        } else if(is_float_type(item.type)) {
          builder_.data_literal(scalar_literal(item.literal_operand, item.type));
        } else {
          builder_.data_integer(width, item.literal_operand.int_value);
        }
        offset += width;
      }
      return;
    }

    const size_t width = type_size(global.type);
    if(global.init_kind == GlobalDefinition::INIT_ZERO) {
      if(is_f80_type(global.type)) {
        builder_.zero_bytes(16);
      } else {
        builder_.data_integer(width, 0);
      }
      return;
    }
    if(global.init_kind == GlobalDefinition::INIT_INTEGER) {
      if(is_f80_type(global.type)) {
        emit_f80_literal_object(global.init_operand);
      } else if(is_float_type(global.type)) {
        builder_.data_literal(scalar_literal(global.init_operand, global.type));
      } else {
        builder_.data_integer(width, global.init_operand.int_value);
      }
      return;
    }
    const string symbol = function_symbols_.count(global.init_operand.text) != 0
        ? function_symbol(global.init_operand.text)
        : global_symbol(global.init_operand.text);
    builder_.data_label(symbol);
  }

  cy86_internal::Program emit()
  {
    const string init_name = init_function_name();
    const string entry_name = entry_function_name();
    const string fini_name = fini_function_name();
    builder_.label("start");
    builder_.opcode("move64", {register_operand("bp"), register_operand("sp")});
    if(!init_name.empty()) {
      builder_.opcode("call", {immediate_label(function_symbol(init_name))});
    }
    builder_.opcode("call", {immediate_label(function_symbol(entry_name))});
    if(!fini_name.empty()) {
      builder_.opcode("isub64",
                      {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
      builder_.opcode("move64", {memory_reg("sp"), register_operand("x64")});
      builder_.opcode("call", {immediate_label(function_symbol(fini_name))});
      builder_.opcode("move64", {register_operand("x64"), memory_reg("sp")});
      builder_.opcode("iadd64",
                      {register_operand("sp"), register_operand("sp"), immediate_integer(8)});
    }
    builder_.opcode("syscall1",
                    {register_operand("t64"),
                     immediate_integer(60),
                     register_operand("x64")});
    for(size_t i = 0; i < program_.functions.size(); ++i) {
      emit_function(program_.functions[i]);
    }
    if(uses_eh_runtime_) {
      emit_eh_unhandled_function();
    }
    for(size_t i = 0; i < program_.globals.size(); ++i) {
      emit_global(program_.globals[i]);
    }
    if(uses_eh_runtime_) {
      emit_eh_runtime_globals();
    }
    return builder_.finish();
  }
};

}  // namespace

cy86_internal::Program build_lowir_cy86_program(const vector<string> & srcfiles)
{
  return build_lowir_cy86_program(lir::parse_program(srcfiles));
}

cy86_internal::Program build_lowir_cy86_program(const Program & program)
{
  return CY86Translator(program).emit();
}
