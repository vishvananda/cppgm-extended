#include "lowir_machine_ir.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "lowir_internal.h"
#include "machine_ir.h"
#include "runtime_symbol_policy.h"
#include "symbol_linkage.h"

namespace {

namespace lir = lowir_internal;
namespace mir = machine_ir;
namespace lowir = lowir_model;

struct FunctionLayout
{
  string function_name;
  map<string, size_t> storage_offset;
  map<string, string> storage_type;
  map<string, X64Register> temp_register;
  map<string, XmmRegister> float_temp_register;
  map<string, mir::ParamBinding> forwarded_params;
  map<string, string> promoted_param_slots;
  map<string, string> aliased_param_slots;
  map<string, string> aliased_object_return_slots;
  map<string, lir::Instruction> temp_def_instruction;
  map<string, lir::Operand> elided_direct_branch_load_sources;
  set<string> address_taken_temps;
  set<string> direct_branch_temps;
  set<string> dead_call_result_temps;
  set<string> direct_call_arg_index_temps;
  set<string> thread_local_globals;
  vector<string> params;
  vector<string> slots;
  vector<string> temps;
  size_t frame_bytes = 0;
  size_t scratch_bytes = 0;
  bool host_eh_enabled = false;
  size_t host_eh_exception_offset = 0;
  size_t host_eh_selector_offset = 0;
  bool variadic = false;
  size_t va_reg_save_area_offset = 0;
  long long va_overflow_stack_offset = 16;
  unsigned va_gp_offset = 0;
  unsigned va_fp_offset = 48;
  bool has_preserve_spill = false;
  size_t preserve_spill_offset = 0;
  size_t preserve_spill_count = 0;
};

struct PreservedIntegerValue
{
  bool spilled = false;
  X64Register reg = XR_RAX;
  size_t spill_index = 0;
};

struct XmmCallArgMove
{
  string type;
  XmmRegister dst = XMM_0;
  mir::Operand src;
  bool done = false;
};

bool call_source_reg_clobbered_before_call_piece(const vector<bool> & arg_in_reg,
                                                 const vector<size_t> & arg_reg_index,
                                                 size_t index,
                                                 X64Register source_reg,
                                                 bool direct_symbol_call,
                                                 X64Register indirect_target_reg = XR_R10)
{
  static const X64Register kIntegerArgRegs[] = {
    XR_RDI, XR_RSI, XR_RDX, XR_RCX, XR_R8, XR_R9
  };
  if(!direct_symbol_call && source_reg == indirect_target_reg) {
    return true;
  }
  for(size_t i = 0; i < index; ++i) {
    if(arg_in_reg[i] &&
       arg_reg_index[i] < sizeof(kIntegerArgRegs) / sizeof(kIntegerArgRegs[0]) &&
       kIntegerArgRegs[arg_reg_index[i]] == source_reg) {
      return true;
    }
  }
  return false;
}

bool pending_xmm_arg_source_uses_register(const vector<XmmCallArgMove> & moves,
                                          XmmRegister reg)
{
  for(size_t i = 0; i < moves.size(); ++i) {
    if(!moves[i].done &&
       moves[i].src.kind == mir::Operand::OP_XMM &&
       moves[i].src.xmm == reg) {
      return true;
    }
  }
  return false;
}

bool find_spare_xmm_arg_move_register(const vector<XmmCallArgMove> & moves,
                                      XmmRegister & out)
{
  set<XmmRegister> used;
  for(size_t i = 0; i < moves.size(); ++i) {
    // Every destination already holds, or will hold, a live call argument.
    // In particular, a completed self-move reserves its register through the
    // call and cannot serve as cycle-breaking scratch storage.
    used.insert(moves[i].dst);
    if(!moves[i].done && moves[i].src.kind == mir::Operand::OP_XMM) {
      used.insert(moves[i].src.xmm);
    }
  }

  static const XmmRegister candidates[] = {
    XMM_0, XMM_1, XMM_2, XMM_3, XMM_4, XMM_5, XMM_6, XMM_7
  };
  for(size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
    if(used.count(candidates[i]) == 0) {
      out = candidates[i];
      return true;
    }
  }
  return false;
}

bool xmm_arg_register_moves_have_cycle(vector<XmmCallArgMove> moves)
{
  for(size_t i = 0; i < moves.size(); ++i) {
    if(moves[i].src.kind == mir::Operand::OP_XMM &&
       moves[i].src.xmm == moves[i].dst) {
      moves[i].done = true;
    }
  }

  while(true) {
    bool any_pending = false;
    bool made_progress = false;
    for(size_t i = 0; i < moves.size(); ++i) {
      if(moves[i].done) {
        continue;
      }
      any_pending = true;
      if(!pending_xmm_arg_source_uses_register(moves, moves[i].dst)) {
        moves[i].done = true;
        made_progress = true;
      }
    }
    if(!any_pending) {
      return false;
    }
    if(!made_progress) {
      return true;
    }
  }
}

bool xmm_arg_register_moves_need_stack_spill(vector<XmmCallArgMove> moves)
{
  if(!xmm_arg_register_moves_have_cycle(moves)) {
    return false;
  }
  for(size_t i = 0; i < moves.size(); ++i) {
    if(moves[i].src.kind == mir::Operand::OP_XMM &&
       moves[i].src.xmm == moves[i].dst) {
      moves[i].done = true;
    }
  }
  XmmRegister spare = XMM_0;
  return !find_spare_xmm_arg_move_register(moves, spare);
}

string target_text(const string & output_target)
{
  const cy86_internal::ProgramOutputTarget parsed =
      cy86_internal::parse_output_target(output_target);
  switch(cy86_internal::native_target_for_output(parsed)) {
    case cy86_internal::NT_LINUX:
      return "linux";
    case cy86_internal::NT_MACOS:
      return "macos";
  }
  return "unknown";
}

bool is_float_type(const string & type)
{
  return type == "f32" || type == "f64" || type == "f80";
}

bool is_float_type(const lir::LowType & type)
{
  return is_float_type(type.text);
}

bool is_atomic_scalar_type(const string & type)
{
  return type == "i1" || type == "i8" || type == "u8" ||
         type == "i16" || type == "u16" || type == "i32" ||
         type == "u32" || type == "i64" || type == "ptr";
}

bool is_i128_scalar_type(const string & type)
{
  return type == "i128" || type == "u128";
}

bool is_integer_scalar_type(const string & type)
{
  return type == "i1" || type == "i8" || type == "u8" ||
         type == "i16" || type == "u16" || type == "i32" ||
         type == "u32" || type == "i64" ||
         type == "i128" || type == "u128";
}

bool is_object_type(const string & type)
{
  return lir::is_object_type(lir::LowType{type});
}

bool uses_storage_address_passing(lir::ParamPassingMode passing)
{
  return passing == lir::PPM_INDIRECT_RESULT ||
         passing == lir::PPM_BY_ADDRESS ||
         passing == lir::PPM_REFERENCE ||
         passing == lir::PPM_DECAY;
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

string integer_chunk_type_for_size(size_t width)
{
  if(width <= 1) {
    return "i8";
  }
  if(width <= 2) {
    return "i16";
  }
  if(width <= 4) {
    return "i32";
  }
  return "i64";
}

vector<string> object_abi_chunk_types(const string & type)
{
  vector<string> out;
  if(!is_object_type(type)) {
    return out;
  }
  size_t remaining = lir::type_size(lir::LowType{type});
  while(remaining != 0) {
    const size_t chunk = min<size_t>(8, remaining);
    out.push_back(integer_chunk_type_for_size(chunk));
    remaining -= chunk;
  }
  return out;
}

vector<string> scalar_abi_chunk_types(const string & type)
{
  if(is_i128_scalar_type(type)) {
    return vector<string>(2, "i64");
  }
  return object_abi_chunk_types(type);
}

bool is_memory_class_object_abi_type(const string & type)
{
  return is_object_type(type) &&
         lir::type_size(lir::LowType{type}) > 16;
}

size_t frame_storage_size_text(const string & type)
{
  const size_t logical_size = lir::type_size(lir::LowType{type});
  const vector<string> chunk_types = scalar_abi_chunk_types(type);
  if(chunk_types.empty()) {
    return logical_size;
  }
  size_t chunk_bytes = 0;
  for(size_t i = 0; i < chunk_types.size(); ++i) {
    chunk_bytes += lir::type_size(lir::LowType{chunk_types[i]});
  }
  return max(logical_size, chunk_bytes);
}

size_t float_exec_width_bytes(const string & type)
{
  return type == "f80" ? 10 : lir::type_size(lir::LowType{type});
}

bool is_seq_cst_order(long long order)
{
  return order == 5;
}

long long atomic_order_value(const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_INTEGER) {
    throw lir::ParseError("atomic order must be an integer literal");
  }
  return operand.int_value;
}

long double scalar_literal_float(const lir::Operand & operand)
{
  if(operand.kind == lir::Operand::OP_FLOAT) {
    return operand.float_value;
  }
  if(operand.kind == lir::Operand::OP_INTEGER) {
    return static_cast<long double>(operand.int_value);
  }
  throw lir::ParseError("unsupported non-scalar floating literal in native/object backend");
}

long long scalar_literal_bits(const lir::Operand & operand,
                              const string & type)
{
  if(is_float_type(type)) {
    long double numeric = scalar_literal_float(operand);
    if(type == "f32") {
      const float value = static_cast<float>(numeric);
      return static_cast<long long>(
          cy86_internal::decode_uint64(cy86_internal::bytes_of(value), 4));
    }
    if(type == "f64") {
      const double value = static_cast<double>(numeric);
      return static_cast<long long>(
          cy86_internal::decode_uint64(cy86_internal::bytes_of(value), 8));
    }
    throw lir::ParseError("f80 literals do not have a scalar bit-pack form");
  }
  if(operand.kind != lir::Operand::OP_INTEGER) {
    throw lir::ParseError("unsupported non-integer scalar literal in native/object backend");
  }
  return operand.int_value;
}

size_t type_alignment_text(const string & type)
{
  return lir::type_alignment(lir::LowType{type});
}

size_t stack_arg_size(const string & type)
{
  if(type == "f80") {
    return 16;
  }
  return max<size_t>(8, lir::type_size(lir::LowType{type}));
}

size_t align_up_size(size_t value, size_t align)
{
  return (value + align - 1) & ~(align - 1);
}

size_t scratch_bytes_for(const lir::Function & function)
{
  if(is_float_type(function.return_type)) {
    return 48;
  }
  for(size_t i = 0; i < function.params.size(); ++i) {
    if(is_float_type(function.params[i].type)) {
      return 48;
    }
  }
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(is_float_type(function.slots[i].second)) {
      return 48;
    }
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.kind == lir::Instruction::IK_CONVERT) {
        return 48;
      }
      if(is_float_type(inst.type) || is_float_type(inst.op)) {
        return 48;
      }
      if(inst.first.kind == lir::Operand::OP_FLOAT ||
         inst.second.kind == lir::Operand::OP_FLOAT ||
         inst.third.kind == lir::Operand::OP_FLOAT) {
        return 48;
      }
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        if(inst.args[ai].kind == lir::Operand::OP_FLOAT) {
          return 48;
        }
      }
    }
  }
  return 0;
}

bool function_contains_eh_regions(const lir::Function & function,
                                  const map<string, lir::SymbolRole> & function_roles)
{
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      const lir::Instruction::Kind kind = inst.kind;
      if(kind == lir::Instruction::IK_EH_TRY ||
         kind == lir::Instruction::IK_EH_CLEANUP ||
         kind == lir::Instruction::IK_THROW ||
         kind == lir::Instruction::IK_EXCEPTION ||
         kind == lir::Instruction::IK_EXCEPTION_SELECTOR ||
         kind == lir::Instruction::IK_RESUME) {
        return true;
      }
      if(kind == lir::Instruction::IK_CALL &&
         inst.first.kind == lir::Operand::OP_GLOBAL) {
        map<string, lir::SymbolRole>::const_iterator found =
            function_roles.find(inst.first.text);
        if(found != function_roles.end() &&
           lir::is_host_eh_symbol_role(found->second)) {
          return true;
        }
        if(runtime_symbol_policy::is_host_eh_runtime_symbol(inst.first.text)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool is_register_allocatable_temp_type(const string & type)
{
  return is_atomic_scalar_type(type);
}

string instruction_result_storage_type(const lir::Instruction & inst)
{
  if(inst.kind == lir::Instruction::IK_CMP) {
    return "i64";
  }
  if(inst.kind == lir::Instruction::IK_BINARY &&
     inst.op == "sub" &&
     inst.type.text == "ptr") {
    return "i64";
  }
  return inst.type.text;
}

bool is_xmm_allocatable_temp_type(const string & type)
{
  return type == "f32" || type == "f64";
}

string block_symbol(const string & function_name, const string & block_label);

map<string, vector<mir::HostEhClause> > collect_host_eh_clauses(const lir::Function & function)
{
  map<string, vector<mir::HostEhClause> > out;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    const lir::Block & block = function.blocks[bi];
    vector<mir::HostEhClause> clauses;
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const lir::Instruction & inst = block.instructions[ii];
      if(inst.kind == lir::Instruction::IK_EH_CATCH) {
        mir::HostEhClause item;
        item.type_symbol = inst.first.text;
        item.selector = inst.has_eh_selector ? inst.eh_selector : 0;
        clauses.push_back(item);
        continue;
      }
      if(inst.kind == lir::Instruction::IK_EH_CLEANUP_CLAUSE) {
        mir::HostEhClause item;
        item.kind = mir::HostEhClause::HC_CLEANUP;
        clauses.push_back(item);
        continue;
      }
      if(inst.kind == lir::Instruction::IK_EH_FILTER) {
        mir::HostEhClause item;
        item.kind = mir::HostEhClause::HC_FILTER;
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          item.filter_type_symbols.push_back(inst.args[ai].text);
        }
        clauses.push_back(item);
        continue;
      }
      if(inst.kind == lir::Instruction::IK_EH_CATCH_ALL) {
        mir::HostEhClause item;
        item.catch_all = true;
        item.selector = inst.has_eh_selector ? inst.eh_selector : 0;
        clauses.push_back(item);
        continue;
      }
      break;
    }
    if(!clauses.empty()) {
      out[block_symbol(function.name, block.label)] = clauses;
    }
  }
  return out;
}

const vector<X64Register> & caller_saved_temp_registers()
{
  static const vector<X64Register> regs = {
    XR_R8,
    XR_R9
  };
  return regs;
}

const vector<X64Register> & call_setup_preserve_registers()
{
  static const vector<X64Register> regs = {
    XR_R8,
    XR_R9
  };
  return regs;
}

const vector<X64Register> & callee_saved_temp_registers()
{
  static const vector<X64Register> regs = {
    XR_RBX,
    XR_R12,
    XR_R13,
    XR_R14,
    XR_R15
  };
  return regs;
}

bool is_callee_saved_temp_register(X64Register reg)
{
  const vector<X64Register> & regs = callee_saved_temp_registers();
  return find(regs.begin(), regs.end(), reg) != regs.end();
}

bool is_call_clobbered_register(X64Register reg)
{
  return !is_callee_saved_temp_register(reg) && reg != XR_RBP && reg != XR_RSP;
}

bool is_backend_temp_register(X64Register reg)
{
  return find(caller_saved_temp_registers().begin(),
              caller_saved_temp_registers().end(),
              reg) != caller_saved_temp_registers().end() ||
         is_callee_saved_temp_register(reg);
}

bool find_spare_callee_saved_register(const FunctionLayout & layout,
                                      X64Register & out)
{
  set<X64Register> reserved;
  for(map<string, X64Register>::const_iterator it = layout.temp_register.begin();
      it != layout.temp_register.end();
      ++it) {
    reserved.insert(it->second);
  }
  for(size_t i = 0; i < callee_saved_temp_registers().size(); ++i) {
    const X64Register reg = callee_saved_temp_registers()[i];
    if(reserved.count(reg) == 0) {
      out = reg;
      return true;
    }
  }
  return false;
}

struct TempInterval
{
  string name;
  string type;
  size_t start = 0;
  size_t end = 0;
  size_t use_count = 0;
  bool live_across_call = false;
  bool used_in_call_setup = false;
  bool has_def = false;
  bool preferred_reg_present = false;
  X64Register preferred_reg = XR_RAX;
  lir::Instruction::Kind last_use_kind = lir::Instruction::IK_CONST;
};

vector<X64Register> candidate_temp_registers(const TempInterval & interval,
                                            bool allow_callee_saved)
{
  vector<X64Register> out;
  if(interval.live_across_call) {
    if(!allow_callee_saved) {
      return vector<X64Register>();
    }
    out = callee_saved_temp_registers();
  } else {
    out = caller_saved_temp_registers();
    if(allow_callee_saved) {
      const vector<X64Register> & callee = callee_saved_temp_registers();
      out.insert(out.end(), callee.begin(), callee.end());
    }
  }
  if(interval.preferred_reg_present) {
    vector<X64Register>::iterator preferred =
        find(out.begin(), out.end(), interval.preferred_reg);
    if(preferred == out.end()) {
      if(!interval.live_across_call ||
         is_callee_saved_temp_register(interval.preferred_reg)) {
        out.insert(out.begin(), interval.preferred_reg);
      }
    } else if(preferred != out.begin()) {
      out.erase(preferred);
      out.insert(out.begin(), interval.preferred_reg);
    }
  }
  return out;
}

const vector<XmmRegister> & candidate_xmm_temp_registers()
{
  static const vector<XmmRegister> regs = {
    XMM_0, XMM_1, XMM_2, XMM_3, XMM_4, XMM_5, XMM_6, XMM_7
  };
  return regs;
}

struct TempDefInfo
{
  size_t position = 0;
  lir::Instruction::Kind kind = lir::Instruction::IK_CONST;
  string op;
  string type;
  lir::Operand first;
  lir::Operand second;
};

map<string, TempDefInfo> collect_temp_def_info(const lir::Function & function);
vector<mir::ParamBinding> collect_param_bindings(const lir::Function & function);
map<string, mir::ParamBinding> collect_forwarded_register_params(
    const lir::Function & function,
    const vector<mir::ParamBinding> & param_bindings);
map<string, string> collect_promoted_param_slots(const lir::Function & function);
map<string, string> collect_aliased_object_param_slots(const lir::Function & function);
map<string, string> collect_aliased_object_return_slots(const lir::Function & function,
                                                        const FunctionLayout & layout);
map<string, lir::Operand> collect_elided_direct_branch_loads(
    const lir::Function & function,
    const set<string> & direct_branch_temps,
    const map<string, string> & promoted_param_slots,
    const set<string> & thread_local_globals);
set<string> collect_address_taken_temps(
    const lir::Function & function,
    const map<string, vector<lir::Parameter> > & function_params);
bool operand_may_emit_tls_addr(const lir::Operand & operand,
                               const set<string> & thread_local_globals);
bool instruction_may_emit_tls_addr(const lir::Instruction & inst,
                                   const set<string> & thread_local_globals);
bool instruction_may_emit_i128_helper_call(const lir::Instruction & inst);
vector<TempInterval> collect_forwarded_param_intervals(
    const lir::Function & function,
    const map<string, mir::ParamBinding> & forwarded_params,
    const map<string, string> & promoted_param_slots,
    const set<string> & direct_call_arg_index_temps,
    const set<string> & thread_local_globals);

void note_operand_use(map<string, TempInterval> & intervals,
                      const lir::Operand & operand,
                      size_t position,
                      lir::Instruction::Kind use_kind)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return;
  }
  map<string, TempInterval>::iterator found = intervals.find(operand.text);
  if(found == intervals.end()) {
    return;
  }
  found->second.end = max(found->second.end, position);
  ++found->second.use_count;
  found->second.last_use_kind = use_kind;
}

void note_instruction_uses(map<string, TempInterval> & intervals,
                           const lir::Instruction & inst,
                           size_t position)
{
  note_operand_use(intervals, inst.first, position, inst.kind);
  note_operand_use(intervals, inst.second, position, inst.kind);
  note_operand_use(intervals, inst.third, position, inst.kind);
  for(size_t i = 0; i < inst.args.size(); ++i) {
    note_operand_use(intervals, inst.args[i], position, inst.kind);
  }
}

map<string, string> collect_temp_result_types(const lir::Function & function)
{
  map<string, string> out;
  for(size_t i = 0; i < function.params.size(); ++i) {
    out[function.params[i].name] = function.params[i].type.text;
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(!inst.dest.empty()) {
        out[inst.dest] = instruction_result_storage_type(inst);
      }
    }
  }
  return out;
}

void note_temp_address_required(set<string> & out, const lir::Operand & operand)
{
  if(operand.kind == lir::Operand::OP_TEMP) {
    out.insert(operand.text);
  }
}

void note_storage_address_required(set<string> & out,
                                   const map<string, string> & temp_types,
                                   const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return;
  }
  map<string, string>::const_iterator found = temp_types.find(operand.text);
  if(found == temp_types.end() || found->second != "ptr") {
    out.insert(operand.text);
  }
}

vector<lir::Parameter> instruction_call_params(
    const lir::Instruction & inst,
    const map<string, vector<lir::Parameter> > & function_params)
{
  if(inst.has_call_signature) {
    return inst.call_params;
  }
  if(inst.first.kind != lir::Operand::OP_GLOBAL) {
    return vector<lir::Parameter>();
  }
  map<string, vector<lir::Parameter> >::const_iterator found =
      function_params.find(inst.first.text);
  return found == function_params.end() ? vector<lir::Parameter>() : found->second;
}

set<string> collect_address_taken_temps(
    const lir::Function & function,
    const map<string, vector<lir::Parameter> > & function_params)
{
  const map<string, string> temp_types = collect_temp_result_types(function);
  set<string> out;

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      switch(inst.kind) {
        case lir::Instruction::IK_ADDR:
          note_temp_address_required(out, inst.first);
          break;
        case lir::Instruction::IK_LOAD:
        case lir::Instruction::IK_ATOMIC_LOAD:
          note_storage_address_required(out, temp_types, inst.first);
          break;
        case lir::Instruction::IK_STORE:
        case lir::Instruction::IK_ATOMIC_STORE:
          note_storage_address_required(out, temp_types, inst.second);
          break;
        case lir::Instruction::IK_ATOMIC_ADD_FETCH:
        case lir::Instruction::IK_ATOMIC_EXCHANGE:
          note_storage_address_required(out, temp_types, inst.first);
          break;
        case lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE:
          note_storage_address_required(out, temp_types, inst.first);
          note_storage_address_required(out, temp_types, inst.second);
          break;
        case lir::Instruction::IK_COPYOBJ:
          note_storage_address_required(out, temp_types, inst.first);
          note_storage_address_required(out, temp_types, inst.second);
          break;
        case lir::Instruction::IK_ZEROINIT:
          note_storage_address_required(out, temp_types, inst.first);
          break;
        case lir::Instruction::IK_CALL: {
          const vector<lir::Parameter> call_params =
              instruction_call_params(inst, function_params);
          for(size_t ai = 0; ai < inst.args.size(); ++ai) {
            if(ai >= call_params.size()) {
              continue;
            }
            const string & param_type = call_params[ai].type.text;
            if(!scalar_abi_chunk_types(param_type).empty() ||
               uses_storage_address_passing(call_params[ai].metadata.passing)) {
              note_storage_address_required(out, temp_types, inst.args[ai]);
            }
          }
          break;
        }
        default:
          break;
      }
    }
  }

  return out;
}

void note_direct_branch_source_uses(map<string, TempInterval> & intervals,
                                    const map<string, TempDefInfo> & def_info,
                                    const lir::Instruction & inst,
                                    size_t position)
{
  if(inst.kind != lir::Instruction::IK_BRANCH ||
     inst.first.kind != lir::Operand::OP_TEMP) {
    return;
  }

  map<string, TempDefInfo>::const_iterator def = def_info.find(inst.first.text);
  if(def == def_info.end()) {
    return;
  }

  const bool direct_cmp =
      def->second.kind == lir::Instruction::IK_CMP &&
      !is_i128_scalar_type(def->second.type);
  const bool direct_integer_not =
      def->second.kind == lir::Instruction::IK_UNARY &&
      def->second.op == "not" &&
      !is_float_type(def->second.type) &&
      !is_i128_scalar_type(def->second.type);
  if(!(direct_cmp || direct_integer_not)) {
    return;
  }

  note_operand_use(intervals, def->second.first, position, lir::Instruction::IK_BRANCH);
  if(direct_cmp) {
    note_operand_use(intervals, def->second.second, position, lir::Instruction::IK_BRANCH);
  }
}

void note_promoted_slot_load_use(map<string, TempInterval> & intervals,
                                 const map<string, string> & promoted_param_slots,
                                 const lir::Instruction & inst,
                                 size_t position)
{
  if(inst.kind != lir::Instruction::IK_LOAD ||
     inst.first.kind != lir::Operand::OP_SLOT) {
    return;
  }
  map<string, string>::const_iterator promoted =
      promoted_param_slots.find(inst.first.text);
  if(promoted == promoted_param_slots.end()) {
    return;
  }
  lir::Operand forwarded;
  forwarded.kind = lir::Operand::OP_TEMP;
  forwarded.text = promoted->second;
  note_operand_use(intervals, forwarded, position, lir::Instruction::IK_LOAD);
}

void note_direct_call_arg_index_source_uses(
    map<string, TempInterval> & intervals,
    const map<string, TempDefInfo> & def_info,
    const set<string> & direct_call_arg_index_temps,
    const lir::Instruction & inst,
    size_t position)
{
  if(inst.kind != lir::Instruction::IK_CALL) {
    return;
  }

  for(size_t ai = 0; ai < inst.args.size(); ++ai) {
    if(inst.args[ai].kind != lir::Operand::OP_TEMP ||
       direct_call_arg_index_temps.count(inst.args[ai].text) == 0) {
      continue;
    }

    map<string, TempDefInfo>::const_iterator def = def_info.find(inst.args[ai].text);
    if(def == def_info.end() ||
       def->second.kind != lir::Instruction::IK_INDEX ||
       def->second.second.kind != lir::Operand::OP_INTEGER) {
      continue;
    }

    note_operand_use(intervals,
                     def->second.first,
                     position,
                     lir::Instruction::IK_CALL);
  }
}

void note_named_temp_use(set<string> & names,
                         const set<string> & tracked_names,
                         const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP ||
     tracked_names.count(operand.text) == 0) {
    return;
  }
  names.insert(operand.text);
}

void note_named_identity_source_use(set<string> & names,
                                    const set<string> & tracked_names,
                                    const map<string, TempDefInfo> & def_info,
                                    const lir::Operand & operand,
                                    set<string> & visited)
{
  if(operand.kind != lir::Operand::OP_TEMP ||
     !visited.insert(operand.text).second) {
    return;
  }
  if(tracked_names.count(operand.text) != 0) {
    names.insert(operand.text);
    return;
  }

  map<string, TempDefInfo>::const_iterator def = def_info.find(operand.text);
  if(def == def_info.end()) {
    return;
  }

  if(def->second.kind == lir::Instruction::IK_COPY) {
    note_named_identity_source_use(names,
                                   tracked_names,
                                   def_info,
                                   def->second.first,
                                   visited);
    return;
  }

  if(def->second.kind == lir::Instruction::IK_INDEX &&
     def->second.first.kind == lir::Operand::OP_TEMP &&
     def->second.second.kind == lir::Operand::OP_INTEGER &&
     def->second.second.int_value == 0) {
    note_named_identity_source_use(names,
                                   tracked_names,
                                   def_info,
                                   def->second.first,
                                   visited);
  }
}

void note_direct_call_arg_index_source_names(
    set<string> & names,
    const set<string> & tracked_names,
    const map<string, TempDefInfo> & def_info,
    const set<string> & direct_call_arg_index_temps,
    const lir::Instruction & inst)
{
  if(inst.kind != lir::Instruction::IK_CALL) {
    return;
  }

  for(size_t ai = 0; ai < inst.args.size(); ++ai) {
    if(inst.args[ai].kind != lir::Operand::OP_TEMP ||
       direct_call_arg_index_temps.count(inst.args[ai].text) == 0) {
      continue;
    }

    map<string, TempDefInfo>::const_iterator def = def_info.find(inst.args[ai].text);
    if(def == def_info.end() ||
       def->second.kind != lir::Instruction::IK_INDEX ||
       def->second.second.kind != lir::Operand::OP_INTEGER) {
      continue;
    }

    set<string> visited;
    note_named_temp_use(names, tracked_names, def->second.first);
    note_named_identity_source_use(names, tracked_names, def_info, def->second.first, visited);
  }
}

void note_instruction_named_uses(set<string> & names,
                                 const set<string> & tracked_names,
                                 const map<string, TempDefInfo> & def_info,
                                 const lir::Instruction & inst)
{
  set<string> visited;
  note_named_temp_use(names, tracked_names, inst.first);
  note_named_identity_source_use(names, tracked_names, def_info, inst.first, visited);
  visited.clear();
  note_named_temp_use(names, tracked_names, inst.second);
  note_named_identity_source_use(names, tracked_names, def_info, inst.second, visited);
  visited.clear();
  note_named_temp_use(names, tracked_names, inst.third);
  note_named_identity_source_use(names, tracked_names, def_info, inst.third, visited);
  for(size_t i = 0; i < inst.args.size(); ++i) {
    note_named_temp_use(names, tracked_names, inst.args[i]);
    visited.clear();
    note_named_identity_source_use(names, tracked_names, def_info, inst.args[i], visited);
  }
}

void note_direct_branch_source_names(set<string> & names,
                                     const set<string> & tracked_names,
                                     const map<string, TempDefInfo> & def_info,
                                     const lir::Instruction & inst)
{
  if(inst.kind != lir::Instruction::IK_BRANCH ||
     inst.first.kind != lir::Operand::OP_TEMP) {
    return;
  }

  map<string, TempDefInfo>::const_iterator def = def_info.find(inst.first.text);
  if(def == def_info.end()) {
    return;
  }

  const bool direct_cmp =
      def->second.kind == lir::Instruction::IK_CMP &&
      !is_i128_scalar_type(def->second.type);
  const bool direct_integer_not =
      def->second.kind == lir::Instruction::IK_UNARY &&
      def->second.op == "not" &&
      !is_float_type(def->second.type) &&
      !is_i128_scalar_type(def->second.type);
  if(!(direct_cmp || direct_integer_not)) {
    return;
  }

  note_named_temp_use(names, tracked_names, def->second.first);
  if(direct_cmp) {
    note_named_temp_use(names, tracked_names, def->second.second);
  }
}

void note_promoted_slot_load_names(set<string> & names,
                                   const set<string> & tracked_names,
                                   const map<string, string> & promoted_param_slots,
                                   const lir::Instruction & inst)
{
  if(inst.kind != lir::Instruction::IK_LOAD ||
     inst.first.kind != lir::Operand::OP_SLOT) {
    return;
  }
  map<string, string>::const_iterator promoted =
      promoted_param_slots.find(inst.first.text);
  if(promoted == promoted_param_slots.end() ||
     tracked_names.count(promoted->second) == 0) {
    return;
  }
  names.insert(promoted->second);
}

void append_unique_label(vector<string> & labels, const string & label_text)
{
  if(find(labels.begin(), labels.end(), label_text) == labels.end()) {
    labels.push_back(label_text);
  }
}

vector<string> collect_structural_successor_labels(const lir::Block & block)
{
  vector<string> labels;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    const lir::Instruction & inst = block.instructions[i];
    if((inst.kind == lir::Instruction::IK_EH_TRY ||
        inst.kind == lir::Instruction::IK_EH_CLEANUP) &&
       inst.first.kind == lir::Operand::OP_LABEL) {
      append_unique_label(labels, inst.first.text);
    }
  }
  if(block.instructions.empty()) {
    return labels;
  }

  const lir::Instruction & tail = block.instructions.back();
  switch(tail.kind) {
    case lir::Instruction::IK_JUMP:
      if(tail.first.kind == lir::Operand::OP_LABEL) {
        append_unique_label(labels, tail.first.text);
      }
      break;
    case lir::Instruction::IK_BRANCH:
      if(tail.second.kind == lir::Operand::OP_LABEL) {
        append_unique_label(labels, tail.second.text);
      }
      if(tail.third.kind == lir::Operand::OP_LABEL) {
        append_unique_label(labels, tail.third.text);
      }
      break;
    case lir::Instruction::IK_SWITCH:
      if(tail.second.kind == lir::Operand::OP_LABEL) {
        append_unique_label(labels, tail.second.text);
      }
      for(size_t i = 1; i < tail.args.size(); i += 2) {
        if(tail.args[i].kind == lir::Operand::OP_LABEL) {
          append_unique_label(labels, tail.args[i].text);
        }
      }
      break;
    default:
      break;
  }
  return labels;
}

vector<vector<size_t> > build_successor_lists(const lir::Function & function)
{
  map<string, size_t> block_index;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    block_index[function.blocks[i].label] = i;
  }

  vector<vector<size_t> > successors(function.blocks.size());
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    const vector<string> labels =
        collect_structural_successor_labels(function.blocks[i]);
    for(size_t j = 0; j < labels.size(); ++j) {
      map<string, size_t>::const_iterator found = block_index.find(labels[j]);
      if(found == block_index.end()) {
        continue;
      }
      if(find(successors[i].begin(), successors[i].end(), found->second) ==
         successors[i].end()) {
        successors[i].push_back(found->second);
      }
    }
  }
  return successors;
}

struct BlockTempLiveness
{
  set<string> use;
  set<string> def;
  set<string> live_in;
  set<string> live_out;
};

struct BlockValueLiveness
{
  set<string> use;
  set<string> live_in;
  set<string> live_out;
};

void note_block_temp_use(BlockTempLiveness & block, const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return;
  }
  if(block.def.count(operand.text) == 0) {
    block.use.insert(operand.text);
  }
}

vector<BlockTempLiveness> build_block_temp_liveness(const lir::Function & function)
{
  vector<BlockTempLiveness> out(function.blocks.size());
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    BlockTempLiveness & block = out[bi];
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_block_temp_use(block, inst.first);
      note_block_temp_use(block, inst.second);
      note_block_temp_use(block, inst.third);
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        note_block_temp_use(block, inst.args[ai]);
      }
      if(!inst.dest.empty()) {
        block.def.insert(inst.dest);
      }
    }
  }

  const vector<vector<size_t> > successors = build_successor_lists(function);
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t reverse_index = function.blocks.size(); reverse_index > 0; --reverse_index) {
      const size_t bi = reverse_index - 1;
      BlockTempLiveness & block = out[bi];
      set<string> new_out;
      for(size_t si = 0; si < successors[bi].size(); ++si) {
        const set<string> & successor_live_in = out[successors[bi][si]].live_in;
        new_out.insert(successor_live_in.begin(), successor_live_in.end());
      }

      set<string> new_in = block.use;
      for(set<string>::const_iterator it = new_out.begin(); it != new_out.end(); ++it) {
        if(block.def.count(*it) == 0) {
          new_in.insert(*it);
        }
      }

      if(new_out != block.live_out || new_in != block.live_in) {
        block.live_out.swap(new_out);
        block.live_in.swap(new_in);
        changed = true;
      }
    }
  }

  return out;
}

vector<BlockValueLiveness> build_named_value_liveness(
    const lir::Function & function,
    const set<string> & tracked_names,
    const map<string, TempDefInfo> & def_info,
    const map<string, string> & promoted_param_slots,
    const set<string> & direct_call_arg_index_temps)
{
  vector<BlockValueLiveness> out(function.blocks.size());
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    BlockValueLiveness & block = out[bi];
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_instruction_named_uses(block.use, tracked_names, def_info, inst);
      note_direct_branch_source_names(block.use, tracked_names, def_info, inst);
      note_promoted_slot_load_names(block.use, tracked_names, promoted_param_slots, inst);
      note_direct_call_arg_index_source_names(block.use,
                                              tracked_names,
                                              def_info,
                                              direct_call_arg_index_temps,
                                              inst);
    }
  }

  const vector<vector<size_t> > successors = build_successor_lists(function);
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t reverse_index = function.blocks.size(); reverse_index > 0; --reverse_index) {
      const size_t bi = reverse_index - 1;
      BlockValueLiveness & block = out[bi];
      set<string> new_out;
      for(size_t si = 0; si < successors[bi].size(); ++si) {
        const set<string> & successor_live_in = out[successors[bi][si]].live_in;
        new_out.insert(successor_live_in.begin(), successor_live_in.end());
      }

      set<string> new_in = block.use;
      new_in.insert(new_out.begin(), new_out.end());

      if(new_out != block.live_out || new_in != block.live_in) {
        block.live_out.swap(new_out);
        block.live_in.swap(new_in);
        changed = true;
      }
    }
  }

  return out;
}

vector<TempInterval> collect_temp_intervals(
    const lir::Function & function,
    const set<string> & thread_local_globals = set<string>(),
    const set<string> & direct_call_arg_index_temps = set<string>())
{
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  map<string, TempInterval> intervals;
  vector<size_t> call_positions;
  vector<size_t> block_start(function.blocks.size(), 0);
  vector<size_t> block_end(function.blocks.size(), 0);
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.dest.empty()) {
        continue;
      }
      TempInterval & interval = intervals[inst.dest];
      interval.name = inst.dest;
      interval.type = instruction_result_storage_type(inst);
    }
  }

  size_t position = 0;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    block_start[bi] = position;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii, ++position) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(!inst.dest.empty()) {
        TempInterval & interval = intervals[inst.dest];
        if(interval.has_def) {
          throw logic_error("duplicate temp def " + inst.dest + " in " + function.name);
        }
        interval.start = position;
        interval.end = position;
        interval.has_def = true;
      }
      note_instruction_uses(intervals, inst, position);
      note_direct_branch_source_uses(intervals, def_info, inst, position);
      note_direct_call_arg_index_source_uses(intervals,
                                             def_info,
                                             direct_call_arg_index_temps,
                                             inst,
                                             position);
      const bool i128_helper_call = instruction_may_emit_i128_helper_call(inst);
      if(instruction_may_emit_tls_addr(inst, thread_local_globals)) {
        note_operand_use(intervals, inst.first, position, inst.kind);
        note_operand_use(intervals, inst.second, position, inst.kind);
        note_operand_use(intervals, inst.third, position, inst.kind);
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          note_operand_use(intervals, inst.args[ai], position, inst.kind);
        }
        if(inst.first.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.first.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        if(inst.second.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.second.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        if(inst.third.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.third.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          if(inst.args[ai].kind != lir::Operand::OP_TEMP) {
            continue;
          }
          map<string, TempInterval>::iterator found = intervals.find(inst.args[ai].text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        call_positions.push_back(position);
      }
      if(inst.kind == lir::Instruction::IK_CALL) {
        if(inst.first.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator target = intervals.find(inst.first.text);
          if(target != intervals.end()) {
            target->second.used_in_call_setup = true;
          }
        }
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          if(inst.args[ai].kind != lir::Operand::OP_TEMP) {
            continue;
          }
          map<string, TempInterval>::iterator arg = intervals.find(inst.args[ai].text);
          if(arg != intervals.end()) {
            arg->second.used_in_call_setup = true;
          }
        }
        call_positions.push_back(position);
      } else if(i128_helper_call) {
        call_positions.push_back(position);
      }
    }
    block_end[bi] = function.blocks[bi].instructions.empty() ? position : position - 1;
  }

  const vector<BlockTempLiveness> block_liveness = build_block_temp_liveness(function);
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    set<string> block_live = block_liveness[bi].live_in;
    block_live.insert(block_liveness[bi].live_out.begin(),
                      block_liveness[bi].live_out.end());
    for(set<string>::const_iterator it = block_live.begin(); it != block_live.end(); ++it) {
      map<string, TempInterval>::iterator found = intervals.find(*it);
      if(found == intervals.end()) {
        continue;
      }
      found->second.start = min(found->second.start, block_start[bi]);
      found->second.end = max(found->second.end, block_end[bi]);
      found->second.use_count = max<size_t>(found->second.use_count, 1);
    }
  }

  vector<TempInterval> out;
  for(map<string, TempInterval>::iterator it = intervals.begin(); it != intervals.end(); ++it) {
    if(!it->second.has_def) {
      throw logic_error("missing temp def state for " + it->first);
    }
    for(size_t i = 0; i < call_positions.size(); ++i) {
      if(it->second.start < call_positions[i] && call_positions[i] < it->second.end) {
        it->second.live_across_call = true;
        break;
      }
    }
    out.push_back(it->second);
  }
  sort(out.begin(),
       out.end(),
       [](const TempInterval & lhs, const TempInterval & rhs)
       {
         if(lhs.start != rhs.start) {
           return lhs.start < rhs.start;
         }
         return lhs.name < rhs.name;
       });
  return out;
}

set<string> collect_dead_call_result_temps(const lir::Function & function)
{
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  const vector<TempInterval> intervals = collect_temp_intervals(function, set<string>());
  set<string> out;
  for(size_t i = 0; i < intervals.size(); ++i) {
    if(intervals[i].use_count != 0) {
      continue;
    }
    map<string, TempDefInfo>::const_iterator def = def_info.find(intervals[i].name);
    if(def != def_info.end() && def->second.kind == lir::Instruction::IK_CALL) {
      out.insert(intervals[i].name);
    }
  }
  return out;
}

set<string> collect_direct_call_arg_index_temps(
    const lir::Function & function,
    const map<string, vector<lir::Parameter> > & function_params,
    const set<string> & thread_local_globals)
{
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  const vector<TempInterval> intervals = collect_temp_intervals(function,
                                                                thread_local_globals);
  set<string> out;
  for(size_t i = 0; i < intervals.size(); ++i) {
    if(intervals[i].use_count != 1 ||
       intervals[i].last_use_kind != lir::Instruction::IK_CALL ||
       intervals[i].type != "ptr") {
      continue;
    }
    map<string, TempDefInfo>::const_iterator def = def_info.find(intervals[i].name);
    if(def == def_info.end() ||
       def->second.kind != lir::Instruction::IK_INDEX ||
       def->second.second.kind != lir::Operand::OP_INTEGER) {
      continue;
    }
    const size_t scale = lir::type_size(lir::LowType{def->second.op});
    long long scaled_offset = 0;
    if(__builtin_mul_overflow(def->second.second.int_value,
                              static_cast<long long>(scale),
                              &scaled_offset)) {
      continue;
    }

    bool safe_direct_call_arg_use = false;
    for(size_t bi = 0; bi < function.blocks.size() && !safe_direct_call_arg_use; ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const lir::Instruction & call = function.blocks[bi].instructions[ii];
        if(call.kind != lir::Instruction::IK_CALL) {
          continue;
        }
        for(size_t ai = 0; ai < call.args.size(); ++ai) {
          if(call.args[ai].kind != lir::Operand::OP_TEMP ||
             call.args[ai].text != intervals[i].name) {
            continue;
          }
          vector<lir::Parameter> call_params;
          if(call.has_call_signature) {
            call_params = call.call_params;
          } else if(call.first.kind == lir::Operand::OP_GLOBAL) {
            map<string, vector<lir::Parameter> >::const_iterator found =
                function_params.find(call.first.text);
            if(found != function_params.end()) {
              call_params = found->second;
            }
          }
          if(ai >= call_params.size()) {
            break;
          }
          const lir::Parameter & param = call_params[ai];
          if(param.type.text == "ptr" &&
             scalar_abi_chunk_types(param.type.text).empty() &&
             !uses_storage_address_passing(param.metadata.passing)) {
            safe_direct_call_arg_use = true;
          }
          break;
        }
      }
    }
    if(!safe_direct_call_arg_use) {
      continue;
    }
    out.insert(intervals[i].name);
  }
  return out;
}

bool plain_storage_debug_name(const string & storage_name,
                              string & source_name)
{
  if(storage_name.size() <= 1 ||
     (storage_name[0] != '%' && storage_name[0] != '$')) {
    return false;
  }
  const string candidate = storage_name.substr(1);
  if(!lir::is_plain_identifier_text(candidate)) {
    return false;
  }
  source_name = candidate;
  return true;
}

map<string, TempDefInfo> collect_temp_def_info(const lir::Function & function)
{
  map<string, TempDefInfo> out;
  size_t position = 0;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii, ++position) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.dest.empty()) {
        continue;
      }
      TempDefInfo info;
      info.position = position;
      info.kind = inst.kind;
      info.op = inst.op;
      info.type = inst.type.text;
      info.first = inst.first;
      info.second = inst.second;
      out[inst.dest] = info;
    }
  }
  return out;
}

set<string> collect_direct_branch_temps(const lir::Function & function,
                                        const set<string> & thread_local_globals)
{
  const vector<TempInterval> intervals = collect_temp_intervals(function,
                                                                thread_local_globals);
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  set<string> out;
  for(size_t i = 0; i < intervals.size(); ++i) {
    map<string, TempDefInfo>::const_iterator def = def_info.find(intervals[i].name);
    if(def == def_info.end()) {
      continue;
    }
    const bool direct_cmp =
        def->second.kind == lir::Instruction::IK_CMP &&
        !is_i128_scalar_type(def->second.type);
    const bool direct_integer_not =
        def->second.kind == lir::Instruction::IK_UNARY &&
        def->second.op == "not" &&
        !is_float_type(def->second.type) &&
        !is_i128_scalar_type(def->second.type);
    if((direct_cmp || direct_integer_not) &&
       intervals[i].use_count == 1 &&
       intervals[i].last_use_kind == lir::Instruction::IK_BRANCH) {
      out.insert(intervals[i].name);
    }
  }
  return out;
}

map<string, lir::Operand> collect_elided_direct_branch_loads(
    const lir::Function & function,
    const set<string> & direct_branch_temps,
    const map<string, string> & promoted_param_slots,
    const set<string> & thread_local_globals)
{
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  map<string, size_t> raw_use_count;
  map<string, size_t> raw_use_position;
  map<string, size_t> direct_branch_use_position;
  vector<size_t> caller_saved_clobber_positions;
  size_t position = 0;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii, ++position) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      const auto note_raw_use =
          [&](const lir::Operand & operand)
          {
            if(operand.kind == lir::Operand::OP_TEMP) {
              ++raw_use_count[operand.text];
              raw_use_position[operand.text] = position;
            }
          };
      note_raw_use(inst.first);
      note_raw_use(inst.second);
      note_raw_use(inst.third);
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        note_raw_use(inst.args[ai]);
      }
      if(inst.kind == lir::Instruction::IK_BRANCH &&
         inst.first.kind == lir::Operand::OP_TEMP &&
         direct_branch_temps.count(inst.first.text) != 0) {
        direct_branch_use_position[inst.first.text] = position;
      }
      if(inst.kind == lir::Instruction::IK_CALL ||
         instruction_may_emit_tls_addr(inst, thread_local_globals) ||
         instruction_may_emit_i128_helper_call(inst)) {
        caller_saved_clobber_positions.push_back(position);
      }
    }
  }

  const auto caller_saved_clobber_between =
      [&](size_t first, size_t last) -> bool
      {
        if(first >= last) {
          return false;
        }
        vector<size_t>::const_iterator found =
            upper_bound(caller_saved_clobber_positions.begin(),
                        caller_saved_clobber_positions.end(),
                        first);
        return found != caller_saved_clobber_positions.end() && *found < last;
      };

  auto is_elidable_direct_load =
      [&](const string & direct_branch_temp,
          const lir::Operand & operand,
          const string & compare_type)
      {
        if(operand.kind != lir::Operand::OP_TEMP) {
          return false;
        }
        map<string, size_t>::const_iterator uses = raw_use_count.find(operand.text);
        if(uses == raw_use_count.end() || uses->second != 1) {
          return false;
        }
        map<string, TempDefInfo>::const_iterator def =
            def_info.find(operand.text);
        if(def == def_info.end() ||
           def->second.kind != lir::Instruction::IK_LOAD ||
           def->second.type != compare_type) {
          return false;
        }
        if(def->second.first.kind == lir::Operand::OP_GLOBAL &&
           thread_local_globals.count(def->second.first.text) != 0) {
          return false;
        }
        if(def->second.first.kind == lir::Operand::OP_SLOT &&
           promoted_param_slots.count(def->second.first.text) != 0) {
          map<string, size_t>::const_iterator branch_use =
              direct_branch_use_position.find(direct_branch_temp);
          map<string, size_t>::const_iterator raw_use =
              raw_use_position.find(operand.text);
          const size_t use_position =
              branch_use != direct_branch_use_position.end()
                  ? branch_use->second
                  : (raw_use == raw_use_position.end() ? def->second.position : raw_use->second);
          if(caller_saved_clobber_between(def->second.position, use_position)) {
            return false;
          }
        }
        return def->second.first.kind == lir::Operand::OP_SLOT ||
               def->second.first.kind == lir::Operand::OP_GLOBAL;
      };

  map<string, lir::Operand> out;
  for(set<string>::const_iterator it = direct_branch_temps.begin();
      it != direct_branch_temps.end();
      ++it) {
    map<string, TempDefInfo>::const_iterator def = def_info.find(*it);
    if(def == def_info.end()) {
      continue;
    }
    const bool direct_cmp =
        def->second.kind == lir::Instruction::IK_CMP &&
        !is_float_type(def->second.type) &&
        !is_i128_scalar_type(def->second.type);
    const bool direct_integer_not =
        def->second.kind == lir::Instruction::IK_UNARY &&
        def->second.op == "not" &&
        !is_float_type(def->second.type) &&
        !is_i128_scalar_type(def->second.type);
    if(direct_cmp) {
      if(is_elidable_direct_load(*it, def->second.first, def->second.type)) {
        out[def->second.first.text] =
            def_info.find(def->second.first.text)->second.first;
      }
      if(is_elidable_direct_load(*it, def->second.second, def->second.type)) {
        out[def->second.second.text] =
            def_info.find(def->second.second.text)->second.first;
      }
    } else if(direct_integer_not &&
              is_elidable_direct_load(*it, def->second.first, def->second.type)) {
      out[def->second.first.text] =
          def_info.find(def->second.first.text)->second.first;
    }
  }
  return out;
}

const X64Register * reusable_first_input_register(
    const TempInterval & interval,
    const map<string, TempDefInfo> & def_info,
    const map<string, TempInterval> & intervals_by_name,
    const map<string, X64Register> & assigned_regs)
{
  map<string, TempDefInfo>::const_iterator def = def_info.find(interval.name);
  if(def == def_info.end()) {
    return nullptr;
  }
  switch(def->second.kind) {
    case lir::Instruction::IK_COPY:
    case lir::Instruction::IK_UNARY:
    case lir::Instruction::IK_BINARY:
    case lir::Instruction::IK_CMP:
    case lir::Instruction::IK_INDEX:
      break;
    default:
      return nullptr;
  }
  if(def->second.first.kind != lir::Operand::OP_TEMP) {
    return nullptr;
  }
  map<string, TempInterval>::const_iterator source =
      intervals_by_name.find(def->second.first.text);
  if(source == intervals_by_name.end() || source->second.end != interval.start) {
    return nullptr;
  }
  map<string, X64Register>::const_iterator assigned =
      assigned_regs.find(def->second.first.text);
  if(assigned == assigned_regs.end()) {
    return nullptr;
  }
  if(interval.live_across_call && !is_callee_saved_temp_register(assigned->second)) {
    return nullptr;
  }
  return &assigned->second;
}

bool operand_may_emit_tls_addr(const lir::Operand & operand,
                               const set<string> & thread_local_globals)
{
  return operand.kind == lir::Operand::OP_GLOBAL &&
         thread_local_globals.count(operand.text) != 0;
}

bool instruction_may_emit_tls_addr(const lir::Instruction & inst,
                                   const set<string> & thread_local_globals)
{
  if(operand_may_emit_tls_addr(inst.first, thread_local_globals) ||
     operand_may_emit_tls_addr(inst.second, thread_local_globals) ||
     operand_may_emit_tls_addr(inst.third, thread_local_globals)) {
    return true;
  }
  for(size_t ai = 0; ai < inst.args.size(); ++ai) {
    if(operand_may_emit_tls_addr(inst.args[ai], thread_local_globals)) {
      return true;
    }
  }
  return false;
}

bool instruction_may_emit_i128_helper_call(const lir::Instruction & inst)
{
  if(inst.kind != lir::Instruction::IK_BINARY ||
     !is_i128_scalar_type(inst.type.text)) {
    return false;
  }
  return inst.op == "mul" ||
         inst.op == "div" ||
         inst.op == "udiv" ||
         inst.op == "mod" ||
         inst.op == "umod" ||
         inst.op == "shl" ||
         inst.op == "shr" ||
         inst.op == "ushr";
}

void assign_temp_registers(const lir::Function & function,
                           FunctionLayout & layout,
                           bool allow_callee_saved)
{
  struct ActiveInterval
  {
    TempInterval interval;
    X64Register reg = XR_R8;
  };

  vector<TempInterval> intervals =
      collect_temp_intervals(function,
                             layout.thread_local_globals,
                             layout.direct_call_arg_index_temps);
  const vector<TempInterval> forwarded_params =
      collect_forwarded_param_intervals(function,
                                       layout.forwarded_params,
                                       layout.promoted_param_slots,
                                       layout.direct_call_arg_index_temps,
                                       layout.thread_local_globals);
  intervals.insert(intervals.end(), forwarded_params.begin(), forwarded_params.end());
  sort(intervals.begin(),
       intervals.end(),
       [](const TempInterval & lhs, const TempInterval & rhs)
       {
         if(lhs.start != rhs.start) {
           return lhs.start < rhs.start;
         }
         return lhs.name < rhs.name;
       });
  map<string, TempInterval> intervals_by_name;
  for(size_t i = 0; i < intervals.size(); ++i) {
    intervals_by_name[intervals[i].name] = intervals[i];
  }
  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  vector<ActiveInterval> active;
  vector<X64Register> free_regs = candidate_temp_registers(TempInterval{},
                                                           allow_callee_saved);
  for(size_t i = 0; i < intervals.size(); ++i) {
    if(intervals[i].preferred_reg_present &&
       find(free_regs.begin(), free_regs.end(), intervals[i].preferred_reg) == free_regs.end()) {
      free_regs.push_back(intervals[i].preferred_reg);
    }
  }

  for(size_t i = 0; i < intervals.size(); ++i) {
    const TempInterval & interval = intervals[i];
    if(layout.dead_call_result_temps.count(interval.name) != 0 ||
       layout.direct_call_arg_index_temps.count(interval.name) != 0 ||
       layout.address_taken_temps.count(interval.name) != 0) {
      continue;
    }
    if(!is_register_allocatable_temp_type(interval.type)) {
      continue;
    }
    map<string, TempDefInfo>::const_iterator def = def_info.find(interval.name);
    if(def != def_info.end() &&
       def->second.kind == lir::Instruction::IK_ADDR &&
       def->second.first.kind == lir::Operand::OP_SLOT) {
      continue;
    }

    for(size_t ai = 0; ai < active.size();) {
      if(active[ai].interval.end < interval.start) {
        free_regs.push_back(active[ai].reg);
        active.erase(active.begin() + ai);
      } else {
        ++ai;
      }
    }

    vector<X64Register> candidates = candidate_temp_registers(interval,
                                                              allow_callee_saved);
    const X64Register * hint =
        reusable_first_input_register(interval, def_info, intervals_by_name, layout.temp_register);
    if(hint != nullptr) {
      for(size_t ai = 0; ai < active.size(); ++ai) {
        if(active[ai].reg == *hint && active[ai].interval.end == interval.start) {
          free_regs.push_back(active[ai].reg);
          active.erase(active.begin() + ai);
          break;
        }
      }
    }

    X64Register chosen = XR_RAX;
    bool have_choice = false;
    if(hint != nullptr &&
       find(candidates.begin(), candidates.end(), *hint) != candidates.end() &&
       find(free_regs.begin(), free_regs.end(), *hint) != free_regs.end()) {
      chosen = *hint;
      have_choice = true;
    } else {
      for(size_t ci = 0; ci < candidates.size(); ++ci) {
        vector<X64Register>::iterator available =
            find(free_regs.begin(), free_regs.end(), candidates[ci]);
        if(available != free_regs.end()) {
          chosen = *available;
          have_choice = true;
          break;
        }
      }
    }
    if(!have_choice) {
      continue;
    }

    free_regs.erase(find(free_regs.begin(), free_regs.end(), chosen));
    layout.temp_register[interval.name] = chosen;

    ActiveInterval assigned;
    assigned.interval = interval;
    assigned.reg = chosen;
    active.push_back(assigned);
    sort(active.begin(),
         active.end(),
         [](const ActiveInterval & lhs, const ActiveInterval & rhs)
         {
           if(lhs.interval.end != rhs.interval.end) {
             return lhs.interval.end < rhs.interval.end;
           }
           return lhs.interval.name < rhs.interval.name;
         });
  }
}

void assign_float_temp_registers(const lir::Function & function,
                                 FunctionLayout & layout)
{
  struct ActiveInterval
  {
    TempInterval interval;
    XmmRegister reg = XMM_0;
  };

  vector<TempInterval> intervals =
      collect_temp_intervals(function,
                             layout.thread_local_globals,
                             layout.direct_call_arg_index_temps);
  vector<ActiveInterval> active;
  vector<XmmRegister> free_regs(candidate_xmm_temp_registers().begin(),
                                candidate_xmm_temp_registers().end());

  for(size_t i = 0; i < intervals.size(); ++i) {
    const TempInterval & interval = intervals[i];
    if(layout.dead_call_result_temps.count(interval.name) != 0 ||
       layout.direct_call_arg_index_temps.count(interval.name) != 0 ||
       layout.address_taken_temps.count(interval.name) != 0) {
      continue;
    }
    if(!is_xmm_allocatable_temp_type(interval.type) ||
       interval.live_across_call) {
      continue;
    }

    for(size_t ai = 0; ai < active.size();) {
      if(active[ai].interval.end < interval.start) {
        free_regs.push_back(active[ai].reg);
        active.erase(active.begin() + ai);
      } else {
        ++ai;
      }
    }

    if(free_regs.empty()) {
      continue;
    }

    sort(free_regs.begin(), free_regs.end());
    const XmmRegister reg = free_regs.front();
    free_regs.erase(free_regs.begin());
    layout.float_temp_register[interval.name] = reg;

    ActiveInterval assigned;
    assigned.interval = interval;
    assigned.reg = reg;
    active.push_back(assigned);
    sort(active.begin(),
         active.end(),
         [](const ActiveInterval & lhs, const ActiveInterval & rhs)
         {
           if(lhs.interval.end != rhs.interval.end) {
             return lhs.interval.end < rhs.interval.end;
           }
           return lhs.interval.name < rhs.interval.name;
         });
  }
}

X64Register arg_register(size_t index)
{
  static const X64Register regs[] = {
    XR_RDI, XR_RSI, XR_RDX, XR_RCX, XR_R8, XR_R9
  };
  if(index >= sizeof(regs) / sizeof(regs[0])) {
    throw logic_error("machine IR supports at most 6 arguments");
  }
  return regs[index];
}

X64Register integer_return_register(size_t index)
{
  static const X64Register regs[] = {
    XR_RAX, XR_RDX
  };
  if(index >= sizeof(regs) / sizeof(regs[0])) {
    throw logic_error("machine IR supports at most 2 direct integer return chunks");
  }
  return regs[index];
}

XmmRegister float_arg_register(size_t index)
{
  static const XmmRegister regs[] = {
    XMM_0, XMM_1, XMM_2, XMM_3, XMM_4, XMM_5, XMM_6, XMM_7
  };
  if(index >= sizeof(regs) / sizeof(regs[0])) {
    throw logic_error("machine IR supports at most 8 float arguments");
  }
  return regs[index];
}

vector<mir::ParamBinding> collect_param_bindings(const lir::Function & function)
{
  vector<mir::ParamBinding> out;
  size_t next_reg = 0;
  size_t next_xmm = 0;
  size_t next_stack = 16;
  for(size_t i = 0; i < function.params.size(); ++i) {
    const string & param_type = function.params[i].type.text;
    if(is_memory_class_object_abi_type(param_type)) {
      const size_t alignment = min<size_t>(16, lir::type_alignment(lir::LowType{param_type}));
      next_stack = align_up_size(next_stack, max<size_t>(8, alignment));
      mir::ParamBinding param;
      param.name = function.params[i].name;
      param.type = param_type;
      param.location = mir::ParamBinding::PL_STACK;
      param.stack_offset = static_cast<long long>(next_stack);
      next_stack += align_up_size(lir::type_size(lir::LowType{param_type}), 8);
      out.push_back(param);
      continue;
    }
    const vector<string> chunk_types = scalar_abi_chunk_types(param_type);
    if(!chunk_types.empty()) {
      size_t chunk_offset = 0;
      for(size_t chunk_index = 0; chunk_index < chunk_types.size(); ++chunk_index) {
        mir::ParamBinding param;
        param.name = function.params[i].name;
        param.type = chunk_types[chunk_index];
        param.chunk_offset = static_cast<long long>(chunk_offset);
        if(next_reg < 6) {
          param.location = mir::ParamBinding::PL_REG;
          param.reg = arg_register(next_reg++);
        } else {
          param.location = mir::ParamBinding::PL_STACK;
          param.stack_offset = static_cast<long long>(next_stack);
          next_stack += stack_arg_size(param.type);
        }
        out.push_back(param);
        chunk_offset += lir::type_size(lir::LowType{param.type});
      }
      continue;
    }
    mir::ParamBinding param;
    param.name = function.params[i].name;
    param.type = param_type;
    if((param.type == "f32" || param.type == "f64") && next_xmm < 8) {
      param.location = mir::ParamBinding::PL_XMM;
      param.xmm = float_arg_register(next_xmm++);
    } else if(!is_float_type(param.type) && next_reg < 6) {
      param.location = mir::ParamBinding::PL_REG;
      param.reg = arg_register(next_reg++);
    } else {
      param.location = mir::ParamBinding::PL_STACK;
      param.stack_offset = static_cast<long long>(next_stack);
      next_stack += stack_arg_size(param.type);
    }
    out.push_back(param);
  }
  return out;
}

bool register_param_needs_stable_storage(const lir::Function & function,
                                         const string & param_name)
{
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.dest == param_name) {
        return true;
      }
      if(inst.kind == lir::Instruction::IK_ADDR &&
         inst.first.kind == lir::Operand::OP_TEMP &&
         inst.first.text == param_name) {
        return true;
      }
    }
  }
  return false;
}

map<string, mir::ParamBinding> collect_forwarded_register_params(
    const lir::Function & function,
    const vector<mir::ParamBinding> & param_bindings)
{
  map<string, mir::ParamBinding> out;
  set<string> eligible_names;
  for(size_t i = 0; i < function.params.size(); ++i) {
    const lir::Parameter & param = function.params[i];
    if(!is_register_allocatable_temp_type(param.type.text) ||
       is_float_type(param.type.text) ||
       is_i128_scalar_type(param.type.text) ||
       !scalar_abi_chunk_types(param.type.text).empty() ||
       register_param_needs_stable_storage(function, param.name)) {
      continue;
    }
    eligible_names.insert(param.name);
  }

  for(size_t i = 0; i < param_bindings.size(); ++i) {
    const mir::ParamBinding & binding = param_bindings[i];
    if(binding.chunk_offset != 0 ||
       binding.location != mir::ParamBinding::PL_REG ||
       !is_register_allocatable_temp_type(binding.type) ||
       is_float_type(binding.type) ||
       is_i128_scalar_type(binding.type) ||
       eligible_names.count(binding.name) == 0) {
      continue;
    }
    out[binding.name] = binding;
  }
  return out;
}

map<string, string> collect_promoted_param_slots(const lir::Function & function)
{
  set<string> param_names;
  for(size_t i = 0; i < function.params.size(); ++i) {
    param_names.insert(function.params[i].name);
  }
  map<string, string> slot_types;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    slot_types[function.slots[i].first] = function.slots[i].second.text;
  }

  struct Candidate
  {
    bool valid = true;
    bool saw_store = false;
    size_t block_index = static_cast<size_t>(-1);
    string param_name;
  };

  map<string, Candidate> candidates;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    const string & slot_name = function.slots[i].first;
    const string & slot_type = function.slots[i].second.text;
    if(!is_register_allocatable_temp_type(slot_type) ||
       is_float_type(slot_type) ||
       is_i128_scalar_type(slot_type)) {
      continue;
    }
    candidates[slot_name] = Candidate{};
  }

  auto note_slot_use =
      [&](size_t block_index,
          const lir::Instruction & inst,
          const lir::Operand & operand,
          bool is_store_target,
          bool is_load_source)
      {
        if(operand.kind != lir::Operand::OP_SLOT) {
          return;
        }
        map<string, Candidate>::iterator found = candidates.find(operand.text);
        if(found == candidates.end() || !found->second.valid) {
          return;
        }
        Candidate & candidate = found->second;
        if(inst.kind == lir::Instruction::IK_ADDR) {
          candidate.valid = false;
          return;
        }
        if(is_store_target &&
           inst.kind == lir::Instruction::IK_STORE &&
           inst.second.kind == lir::Operand::OP_SLOT &&
           inst.type.text == slot_types.find(operand.text)->second &&
           inst.first.kind == lir::Operand::OP_TEMP &&
           param_names.count(inst.first.text) != 0) {
          if(candidate.saw_store) {
            candidate.valid = false;
            return;
          }
          candidate.saw_store = true;
          candidate.block_index = block_index;
          candidate.param_name = inst.first.text;
          return;
        }
        if(is_load_source &&
           inst.kind == lir::Instruction::IK_LOAD &&
           inst.first.kind == lir::Operand::OP_SLOT &&
           inst.type.text == slot_types.find(operand.text)->second) {
          if(!candidate.saw_store || candidate.block_index != block_index) {
            candidate.valid = false;
          }
          return;
        }
        candidate.valid = false;
      };

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_slot_use(bi, inst, inst.first, false, inst.kind == lir::Instruction::IK_LOAD);
      note_slot_use(bi, inst, inst.second, inst.kind == lir::Instruction::IK_STORE, false);
      note_slot_use(bi, inst, inst.third, false, false);
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        note_slot_use(bi, inst, inst.args[ai], false, false);
      }
    }
  }

  map<string, string> out;
  for(map<string, Candidate>::const_iterator it = candidates.begin();
      it != candidates.end();
      ++it) {
    if(it->second.valid && it->second.saw_store) {
      out[it->first] = it->second.param_name;
    }
  }
  return out;
}

map<string, string> collect_aliased_object_param_slots(const lir::Function & function)
{
  map<string, string> param_types;
  for(size_t i = 0; i < function.params.size(); ++i) {
    if(is_object_type(function.params[i].type.text)) {
      param_types[function.params[i].name] = function.params[i].type.text;
    }
  }
  if(param_types.empty()) {
    return map<string, string>();
  }

  map<string, string> slot_types;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(is_object_type(function.slots[i].second.text)) {
      slot_types[function.slots[i].first] = function.slots[i].second.text;
    }
  }
  if(slot_types.empty()) {
    return map<string, string>();
  }

  map<string, string> addr_slot;
  map<string, size_t> param_use_count;
  auto note_param_use =
      [&](const lir::Operand & operand)
      {
        if(operand.kind == lir::Operand::OP_TEMP &&
           param_types.count(operand.text) != 0) {
          ++param_use_count[operand.text];
        }
      };

  struct Candidate
  {
    bool valid = true;
    bool saw_init = false;
    string param_name;
  };

  map<string, Candidate> candidates;
  for(size_t i = 0; i < function.slots.size(); ++i) {
    if(slot_types.count(function.slots[i].first) != 0) {
      candidates[function.slots[i].first] = Candidate{};
    }
  }

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_param_use(inst.first);
      note_param_use(inst.second);
      note_param_use(inst.third);
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        note_param_use(inst.args[ai]);
      }

      if(inst.kind == lir::Instruction::IK_ADDR &&
         inst.first.kind == lir::Operand::OP_SLOT &&
         slot_types.count(inst.first.text) != 0 &&
         !inst.dest.empty()) {
        addr_slot[inst.dest] = inst.first.text;
        continue;
      }

      if(inst.kind != lir::Instruction::IK_COPYOBJ ||
         inst.first.kind != lir::Operand::OP_TEMP ||
         inst.second.kind != lir::Operand::OP_TEMP) {
        continue;
      }

      map<string, string>::const_iterator param =
          param_types.find(inst.first.text);
      map<string, string>::const_iterator slot = addr_slot.find(inst.second.text);
      if(param == param_types.end() || slot == addr_slot.end()) {
        continue;
      }
      map<string, string>::const_iterator slot_type = slot_types.find(slot->second);
      if(slot_type == slot_types.end() || slot_type->second != param->second) {
        continue;
      }

      Candidate & candidate = candidates[slot->second];
      if(candidate.saw_init &&
         candidate.param_name != inst.first.text) {
        candidate.valid = false;
        continue;
      }
      if(candidate.saw_init) {
        candidate.valid = false;
        continue;
      }
      candidate.saw_init = true;
      candidate.param_name = inst.first.text;
    }
  }

  map<string, string> out;
  for(map<string, Candidate>::const_iterator it = candidates.begin();
      it != candidates.end();
      ++it) {
    if(!it->second.valid || !it->second.saw_init) {
      continue;
    }
    map<string, size_t>::const_iterator use_count =
        param_use_count.find(it->second.param_name);
    if(use_count != param_use_count.end() &&
       use_count->second == 1) {
      out[it->first] = it->second.param_name;
    }
  }
  return out;
}

map<string, string> collect_aliased_object_return_slots(const lir::Function & function,
                                                        const FunctionLayout & layout)
{
  map<string, string> call_result_types;
  map<string, size_t> temp_use_count;
  auto note_temp_use =
      [&](const lir::Operand & operand)
      {
        if(operand.kind == lir::Operand::OP_TEMP) {
          ++temp_use_count[operand.text];
        }
      };

  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_temp_use(inst.first);
      note_temp_use(inst.second);
      note_temp_use(inst.third);
      for(size_t ai = 0; ai < inst.args.size(); ++ai) {
        note_temp_use(inst.args[ai]);
      }

      if(inst.kind == lir::Instruction::IK_CALL &&
         !inst.dest.empty() &&
         is_object_type(inst.type.text) &&
         !scalar_abi_chunk_types(inst.type.text).empty()) {
        call_result_types[inst.dest] = inst.type.text;
      }
    }
  }

  map<string, string> out;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.kind != lir::Instruction::IK_COPYOBJ ||
         inst.first.kind != lir::Operand::OP_TEMP ||
         inst.second.kind != lir::Operand::OP_TEMP) {
        continue;
      }

      map<string, string>::const_iterator call_result =
          call_result_types.find(inst.first.text);
      if(call_result == call_result_types.end()) {
        continue;
      }

      map<string, lir::Instruction>::const_iterator target_addr =
          layout.temp_def_instruction.find(inst.second.text);
      if(target_addr == layout.temp_def_instruction.end() ||
         target_addr->second.kind != lir::Instruction::IK_ADDR ||
         target_addr->second.first.kind != lir::Operand::OP_SLOT) {
        continue;
      }

      map<string, string>::const_iterator slot_type =
          layout.storage_type.find(target_addr->second.first.text);
      if(slot_type == layout.storage_type.end() ||
         slot_type->second != call_result->second) {
        continue;
      }

      map<string, size_t>::const_iterator use_count =
          temp_use_count.find(inst.first.text);
      if(use_count == temp_use_count.end() ||
         use_count->second != 1) {
        continue;
      }

      out[inst.first.text] = target_addr->second.first.text;
    }
  }

  return out;
}

vector<TempInterval> collect_forwarded_param_intervals(
    const lir::Function & function,
    const map<string, mir::ParamBinding> & forwarded_params,
    const map<string, string> & promoted_param_slots,
    const set<string> & direct_call_arg_index_temps,
    const set<string> & thread_local_globals)
{
  map<string, TempInterval> intervals;
  set<string> tracked_names;
  for(map<string, mir::ParamBinding>::const_iterator it = forwarded_params.begin();
      it != forwarded_params.end();
      ++it) {
    TempInterval interval;
    interval.name = it->first;
    interval.type = it->second.type;
    interval.has_def = true;
    if(is_backend_temp_register(it->second.reg)) {
      interval.preferred_reg_present = true;
      interval.preferred_reg = it->second.reg;
    }
    intervals[it->first] = interval;
    tracked_names.insert(it->first);
  }

  const map<string, TempDefInfo> def_info = collect_temp_def_info(function);
  vector<size_t> block_start(function.blocks.size(), 0);
  vector<size_t> block_end(function.blocks.size(), 0);
  size_t position = 0;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    block_start[bi] = position;
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii, ++position) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      note_instruction_uses(intervals, inst, position);
      note_direct_branch_source_uses(intervals, def_info, inst, position);
      note_promoted_slot_load_use(intervals, promoted_param_slots, inst, position);
      note_direct_call_arg_index_source_uses(intervals,
                                             def_info,
                                             direct_call_arg_index_temps,
                                             inst,
                                             position);
      if(instruction_may_emit_tls_addr(inst, thread_local_globals)) {
        if(inst.first.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.first.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        if(inst.second.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.second.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        if(inst.third.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator found = intervals.find(inst.third.text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          if(inst.args[ai].kind != lir::Operand::OP_TEMP) {
            continue;
          }
          map<string, TempInterval>::iterator found = intervals.find(inst.args[ai].text);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
      }
      if(inst.kind == lir::Instruction::IK_CALL) {
        if(inst.first.kind == lir::Operand::OP_TEMP) {
          map<string, TempInterval>::iterator target = intervals.find(inst.first.text);
          if(target != intervals.end()) {
            target->second.used_in_call_setup = true;
          }
        }
        for(size_t ai = 0; ai < inst.args.size(); ++ai) {
          if(inst.args[ai].kind != lir::Operand::OP_TEMP) {
            continue;
          }
          map<string, TempInterval>::iterator arg = intervals.find(inst.args[ai].text);
          if(arg != intervals.end()) {
            arg->second.used_in_call_setup = true;
          }
        }
      }
    }
    block_end[bi] = function.blocks[bi].instructions.empty() ? position : position - 1;
  }

  const vector<BlockValueLiveness> block_liveness =
      build_named_value_liveness(function,
                                 tracked_names,
                                 def_info,
                                 promoted_param_slots,
                                 direct_call_arg_index_temps);
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    set<string> block_live = block_liveness[bi].live_in;
    block_live.insert(block_liveness[bi].live_out.begin(),
                      block_liveness[bi].live_out.end());
    for(set<string>::const_iterator it = block_live.begin(); it != block_live.end(); ++it) {
      map<string, TempInterval>::iterator found = intervals.find(*it);
      if(found == intervals.end()) {
        continue;
      }
      found->second.start = min(found->second.start, block_start[bi]);
      found->second.end = max(found->second.end, block_end[bi]);
      found->second.use_count = max<size_t>(found->second.use_count, 1);
    }

    set<string> live = block_liveness[bi].live_out;
    for(size_t reverse_index = function.blocks[bi].instructions.size();
        reverse_index > 0;
        --reverse_index) {
      const lir::Instruction & inst = function.blocks[bi].instructions[reverse_index - 1];
      const bool tls_call = instruction_may_emit_tls_addr(inst, thread_local_globals);
      const bool i128_helper_call = instruction_may_emit_i128_helper_call(inst);
      if(inst.kind == lir::Instruction::IK_CALL || tls_call || i128_helper_call) {
        set<string> call_live = live;
        if(tls_call) {
          note_instruction_named_uses(call_live, tracked_names, def_info, inst);
          note_direct_branch_source_names(call_live, tracked_names, def_info, inst);
          note_promoted_slot_load_names(call_live, tracked_names, promoted_param_slots, inst);
          note_direct_call_arg_index_source_names(call_live,
                                                  tracked_names,
                                                  def_info,
                                                  direct_call_arg_index_temps,
                                                  inst);
        }
        for(set<string>::const_iterator it = call_live.begin(); it != call_live.end(); ++it) {
          map<string, TempInterval>::iterator found = intervals.find(*it);
          if(found != intervals.end()) {
            found->second.live_across_call = true;
          }
        }
      }
      note_instruction_named_uses(live, tracked_names, def_info, inst);
      note_direct_branch_source_names(live, tracked_names, def_info, inst);
      note_promoted_slot_load_names(live, tracked_names, promoted_param_slots, inst);
      note_direct_call_arg_index_source_names(live,
                                              tracked_names,
                                              def_info,
                                              direct_call_arg_index_temps,
                                              inst);
    }
  }

  vector<TempInterval> out;
  for(map<string, TempInterval>::const_iterator it = intervals.begin();
      it != intervals.end();
      ++it) {
    if(it->second.use_count == 0) {
      continue;
    }
    out.push_back(it->second);
  }
  sort(out.begin(),
       out.end(),
       [](const TempInterval & lhs, const TempInterval & rhs)
       {
         if(lhs.start != rhs.start) {
           return lhs.start < rhs.start;
         }
         return lhs.name < rhs.name;
       });
  return out;
}

FunctionLayout build_layout(const lir::Function & function,
                            bool host_eh_enabled,
                            const map<string, vector<lir::Parameter> > & function_params,
                            const set<string> & thread_local_globals)
{
  FunctionLayout layout;
  layout.function_name = function.name;
  layout.scratch_bytes = scratch_bytes_for(function);
  layout.host_eh_enabled = host_eh_enabled;
  layout.thread_local_globals = thread_local_globals;
  layout.promoted_param_slots = collect_promoted_param_slots(function);
  layout.direct_branch_temps = collect_direct_branch_temps(function,
                                                           thread_local_globals);
  layout.elided_direct_branch_load_sources =
      collect_elided_direct_branch_loads(function,
                                         layout.direct_branch_temps,
                                         layout.promoted_param_slots,
                                         thread_local_globals);
  layout.aliased_param_slots = collect_aliased_object_param_slots(function);
  layout.dead_call_result_temps = collect_dead_call_result_temps(function);
  layout.direct_call_arg_index_temps =
      collect_direct_call_arg_index_temps(function,
                                         function_params,
                                         thread_local_globals);
  layout.address_taken_temps = collect_address_taken_temps(function, function_params);
  const vector<mir::ParamBinding> param_bindings = collect_param_bindings(function);
  layout.forwarded_params = collect_forwarded_register_params(function, param_bindings);
  layout.variadic = function.boundary.arity == lir::CAM_VARIADIC;
  if(layout.variadic) {
    size_t gp_count = 0;
    size_t fp_count = 0;
    size_t next_stack = 16;
    for(size_t i = 0; i < param_bindings.size(); ++i) {
      const mir::ParamBinding & binding = param_bindings[i];
      if(binding.location == mir::ParamBinding::PL_REG) {
        ++gp_count;
      } else if(binding.location == mir::ParamBinding::PL_XMM) {
        ++fp_count;
      } else if(binding.location == mir::ParamBinding::PL_STACK) {
        const size_t end =
            static_cast<size_t>(binding.stack_offset) + stack_arg_size(binding.type);
        next_stack = max(next_stack, end);
      }
    }
    layout.va_gp_offset = static_cast<unsigned>(min<size_t>(gp_count, 6) * 8);
    layout.va_fp_offset = static_cast<unsigned>(48 + min<size_t>(fp_count, 8) * 16);
    layout.va_overflow_stack_offset = static_cast<long long>(next_stack);
  }
  auto note_type =
      [&layout](const string & name, const string & type)
      {
        layout.storage_type[name] = type;
      };
  auto add_storage =
      [&layout, &function](const string & name, const string & type)
      {
        if(layout.storage_offset.count(name) != 0) {
          throw logic_error("duplicate storage name " + name + " in " + function.name);
        }
        const size_t align = type_alignment_text(type);
        layout.frame_bytes = (layout.frame_bytes + align - 1) & ~(align - 1);
        layout.frame_bytes += frame_storage_size_text(type);
        layout.storage_offset[name] = layout.frame_bytes;
        layout.storage_type[name] = type;
      };
  for(size_t i = 0; i < function.params.size(); ++i) {
    note_type(function.params[i].name, function.params[i].type.text);
  }
  for(size_t i = 0; i < function.slots.size(); ++i) {
    note_type(function.slots[i].first, function.slots[i].second.text);
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.dest.empty()) {
        continue;
      }
      layout.temp_def_instruction[inst.dest] = inst;
      if(layout.storage_type.count(inst.dest) == 0) {
        note_type(inst.dest, instruction_result_storage_type(inst));
      }
    }
  }
  layout.aliased_object_return_slots =
      collect_aliased_object_return_slots(function, layout);
  assign_temp_registers(function, layout, !host_eh_enabled);
  assign_float_temp_registers(function, layout);
  for(size_t i = 0; i < function.params.size(); ++i) {
    const string & name = function.params[i].name;
    if(layout.forwarded_params.count(name) != 0 &&
       layout.temp_register.count(name) != 0) {
      continue;
    }
    add_storage(name, function.params[i].type.text);
    layout.params.push_back(name);
  }
  for(size_t i = 0; i < function.slots.size(); ++i) {
    const string & name = function.slots[i].first;
    if(layout.promoted_param_slots.count(name) != 0 ||
       layout.aliased_param_slots.count(name) != 0) {
      continue;
    }
    add_storage(name, function.slots[i].second.text);
    layout.slots.push_back(name);
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.dest.empty()) {
        continue;
      }
      if(layout.dead_call_result_temps.count(inst.dest) != 0) {
        continue;
      }
      if(layout.direct_call_arg_index_temps.count(inst.dest) != 0) {
        continue;
      }
      if(layout.storage_offset.count(inst.dest) == 0 &&
         layout.temp_register.count(inst.dest) == 0 &&
         layout.float_temp_register.count(inst.dest) == 0 &&
         layout.direct_branch_temps.count(inst.dest) == 0 &&
         layout.aliased_object_return_slots.count(inst.dest) == 0 &&
         !(inst.kind == lir::Instruction::IK_ADDR &&
           inst.first.kind == lir::Operand::OP_SLOT)) {
        const string dest_type = layout.storage_type.find(inst.dest)->second;
        add_storage(inst.dest, dest_type);
        layout.temps.push_back(inst.dest);
      }
    }
  }
  if(layout.host_eh_enabled) {
    const size_t align = type_alignment_text("ptr");
    layout.frame_bytes = (layout.frame_bytes + align - 1) & ~(align - 1);
    layout.frame_bytes += lir::type_size(lir::LowType{"ptr"});
    layout.host_eh_exception_offset = layout.frame_bytes;
    const size_t selector_align = type_alignment_text("i32");
    layout.frame_bytes = (layout.frame_bytes + selector_align - 1) & ~(selector_align - 1);
    layout.frame_bytes += lir::type_size(lir::LowType{"i32"});
    layout.host_eh_selector_offset = layout.frame_bytes;
  }
  if(layout.variadic) {
    layout.frame_bytes = align_up_size(layout.frame_bytes, 16);
    layout.frame_bytes += 176;
    layout.va_reg_save_area_offset = layout.frame_bytes;
  }
  return layout;
}

vector<mir::DebugVariable> collect_debug_variables(const lir::Function & function,
                                                   const FunctionLayout & layout)
{
  set<string> frame_debug_names;
  for(size_t i = 0; i < layout.params.size(); ++i) {
    string source_name;
    if(plain_storage_debug_name(layout.params[i], source_name)) {
      frame_debug_names.insert(source_name);
    }
  }
  for(size_t i = 0; i < layout.slots.size(); ++i) {
    string source_name;
    if(plain_storage_debug_name(layout.slots[i], source_name)) {
      frame_debug_names.insert(source_name);
    }
  }

  vector<mir::DebugVariable> out;
  map<string, size_t> variable_index;
  const vector<TempInterval> forwarded_intervals =
      collect_forwarded_param_intervals(function,
                                       layout.forwarded_params,
                                       layout.promoted_param_slots,
                                       layout.direct_call_arg_index_temps,
                                       layout.thread_local_globals);
  for(size_t i = 0; i < forwarded_intervals.size(); ++i) {
    string source_name;
    if(!plain_storage_debug_name(forwarded_intervals[i].name, source_name) ||
       frame_debug_names.count(source_name) != 0) {
      continue;
    }
    map<string, X64Register>::const_iterator reg =
        layout.temp_register.find(forwarded_intervals[i].name);
    if(reg == layout.temp_register.end()) {
      continue;
    }

    mir::DebugVariable::Range range;
    range.start_source_position = forwarded_intervals[i].start;
    range.end_source_position = forwarded_intervals[i].end + 1;
    range.location = mir::DebugVariable::Range::LK_REG;
    range.reg = reg->second;

    map<string, size_t>::const_iterator found = variable_index.find(source_name);
    if(found == variable_index.end()) {
      mir::DebugVariable variable;
      variable.name = source_name;
      variable.type = forwarded_intervals[i].type;
      if(function.debug_location.present()) {
        variable.decl_location.file = function.debug_location.file;
        variable.decl_location.line = function.debug_location.line;
        variable.decl_location.column = function.debug_location.column;
      }
      variable.ranges.push_back(range);
      variable_index[source_name] = out.size();
      out.push_back(variable);
      continue;
    }

    out[found->second].ranges.push_back(range);
  }

  const vector<TempInterval> intervals = collect_temp_intervals(function,
                                                                layout.thread_local_globals);
  for(size_t i = 0; i < intervals.size(); ++i) {
    string source_name;
    if(!lir::lowir_debug_value_source_name(intervals[i].name, source_name) ||
       frame_debug_names.count(source_name) != 0) {
      continue;
    }

    mir::DebugVariable::Range range;
    range.start_source_position = intervals[i].start;
    range.end_source_position = intervals[i].end + 1;
    map<string, X64Register>::const_iterator reg =
        layout.temp_register.find(intervals[i].name);
    if(reg != layout.temp_register.end()) {
      range.location = mir::DebugVariable::Range::LK_REG;
      range.reg = reg->second;
    } else {
      map<string, XmmRegister>::const_iterator xmm =
          layout.float_temp_register.find(intervals[i].name);
      if(xmm != layout.float_temp_register.end()) {
        range.location = mir::DebugVariable::Range::LK_XMM;
        range.xmm = xmm->second;
      } else {
        map<string, size_t>::const_iterator storage =
            layout.storage_offset.find(intervals[i].name);
        if(storage == layout.storage_offset.end()) {
          continue;
        }
        range.location = mir::DebugVariable::Range::LK_FRAME;
        range.frame_offset = -static_cast<long long>(storage->second);
      }
    }

    map<string, size_t>::const_iterator found = variable_index.find(source_name);
    if(found == variable_index.end()) {
      mir::DebugVariable variable;
      variable.name = source_name;
      variable.type = intervals[i].type;
      map<string, lir::Instruction>::const_iterator def =
          layout.temp_def_instruction.find(intervals[i].name);
      if(def != layout.temp_def_instruction.end() &&
         def->second.debug_location.present()) {
        variable.decl_location.file = def->second.debug_location.file;
        variable.decl_location.line = def->second.debug_location.line;
        variable.decl_location.column = def->second.debug_location.column;
      } else if(function.debug_location.present()) {
        variable.decl_location.file = function.debug_location.file;
        variable.decl_location.line = function.debug_location.line;
        variable.decl_location.column = function.debug_location.column;
      }
      variable.ranges.push_back(range);
      variable_index[source_name] = out.size();
      out.push_back(variable);
      continue;
    }

    out[found->second].ranges.push_back(range);
  }

  return out;
}

string unknown_storage_error(const FunctionLayout & layout, const string & name)
{
  return "unknown storage " + name + " in " + layout.function_name;
}

const lir::Instruction * rematerialized_slot_address_def(const FunctionLayout & layout,
                                                         const string & name)
{
  map<string, lir::Instruction>::const_iterator found =
      layout.temp_def_instruction.find(name);
  if(found == layout.temp_def_instruction.end() ||
     found->second.kind != lir::Instruction::IK_ADDR ||
     found->second.first.kind != lir::Operand::OP_SLOT) {
    return nullptr;
  }
  return &found->second;
}

long long slot_offset(const FunctionLayout & layout, const string & name)
{
  map<string, size_t>::const_iterator found = layout.storage_offset.find(name);
  if(found == layout.storage_offset.end()) {
    map<string, string>::const_iterator aliased =
        layout.aliased_param_slots.find(name);
    if(aliased != layout.aliased_param_slots.end()) {
      found = layout.storage_offset.find(aliased->second);
    }
  }
  if(found == layout.storage_offset.end()) {
    throw logic_error(unknown_storage_error(layout, name));
  }
  return -static_cast<long long>(found->second);
}

long long preserve_spill_slot_offset(const FunctionLayout & layout, size_t index)
{
  if(!layout.has_preserve_spill || index >= layout.preserve_spill_count) {
    throw logic_error("invalid preserve spill slot in " + layout.function_name);
  }
  return -static_cast<long long>(layout.preserve_spill_offset + index * 8);
}

const X64Register * temp_register_for(const FunctionLayout & layout,
                                      const string & name)
{
  map<string, X64Register>::const_iterator found = layout.temp_register.find(name);
  return found == layout.temp_register.end() ? nullptr : &found->second;
}

const XmmRegister * float_temp_register_for(const FunctionLayout & layout,
                                            const string & name)
{
  map<string, XmmRegister>::const_iterator found = layout.float_temp_register.find(name);
  return found == layout.float_temp_register.end() ? nullptr : &found->second;
}

const X64Register * direct_integer_temp_register(const FunctionLayout & layout,
                                                 const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return nullptr;
  }
  return temp_register_for(layout, operand.text);
}

const X64Register * direct_pointer_base_register(const FunctionLayout & layout,
                                                 const lir::Operand & operand)
{
  if(operand.kind != lir::Operand::OP_TEMP) {
    return nullptr;
  }
  map<string, string>::const_iterator found = layout.storage_type.find(operand.text);
  if(found == layout.storage_type.end() ||
     found->second != "ptr") {
    return nullptr;
  }
  return temp_register_for(layout, operand.text);
}

X64Register temp_result_register(const FunctionLayout & layout,
                                 const string & dest,
                                 X64Register fallback)
{
  const X64Register * assigned = temp_register_for(layout, dest);
  return assigned ? *assigned : fallback;
}

X64Register alternate_integer_scratch(X64Register primary)
{
  if(primary != XR_RDX) {
    return XR_RDX;
  }
  if(primary != XR_RAX) {
    return XR_RAX;
  }
  return XR_R11;
}

mir::Operand reg(X64Register r)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_REG;
  operand.reg = r;
  return operand;
}

mir::Operand xmm(XmmRegister r)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_XMM;
  operand.xmm = r;
  return operand;
}

mir::Operand imm(long long value)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_IMM;
  operand.imm = value;
  return operand;
}

mir::Operand float_imm(const lir::Operand & operand)
{
  mir::Operand out;
  out.kind = mir::Operand::OP_FLOAT_IMM;
  out.float_imm = scalar_literal_float(operand);
  out.text = operand.text;
  return out;
}

mir::Operand zero_float_imm(const string & type)
{
  mir::Operand out;
  out.kind = mir::Operand::OP_FLOAT_IMM;
  out.float_imm = 0.0L;
  out.text = type == "f32" ? "0.0f" : type == "f80" ? "0.0L" : "0.0";
  return out;
}

mir::Operand symbol(const string & text)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_SYMBOL;
  operand.text = text;
  return operand;
}

mir::Operand label(const string & text)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_LABEL;
  operand.text = text;
  return operand;
}

mir::Operand frame(const FunctionLayout & layout, const string & name)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_FRAME;
  operand.offset = slot_offset(layout, name);
  operand.text = name;
  return operand;
}

mir::Operand float_dest_operand(const FunctionLayout & layout,
                                const string & dest)
{
  if(const XmmRegister * assigned = float_temp_register_for(layout, dest)) {
    return xmm(*assigned);
  }
  return frame(layout, dest);
}

mir::Operand global_ref(const string & name)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_GLOBAL;
  operand.text = name;
  return operand;
}

mir::Operand deref(X64Register r)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_DEREF;
  operand.reg = r;
  return operand;
}

mir::Operand deref_offset(X64Register r, long long offset)
{
  mir::Operand operand;
  operand.kind = mir::Operand::OP_DEREF;
  operand.reg = r;
  operand.offset = offset;
  return operand;
}

string block_symbol(const string & function_name, const string & block_label)
{
  return function_name + "$" + block_label;
}

mir::Instruction::Opcode float_binary_opcode(const string & op)
{
  if(op == "add") return mir::Instruction::MI_FADD;
  if(op == "sub") return mir::Instruction::MI_FSUB;
  if(op == "mul") return mir::Instruction::MI_FMUL;
  if(op == "div") return mir::Instruction::MI_FDIV;
  throw logic_error("unsupported binary op " + op);
}

mir::Instruction::Opcode integer_binary_opcode(const string & op)
{
  if(op == "add") return mir::Instruction::MI_ADD;
  if(op == "sub") return mir::Instruction::MI_SUB;
  if(op == "mul") return mir::Instruction::MI_IMUL;
  if(op == "and") return mir::Instruction::MI_AND;
  if(op == "or") return mir::Instruction::MI_OR;
  if(op == "xor") return mir::Instruction::MI_XOR;
  if(op == "shl") return mir::Instruction::MI_SHL_CL;
  if(op == "shr") return mir::Instruction::MI_SAR_CL;
  if(op == "ushr") return mir::Instruction::MI_SHR_CL;
  if(op == "div" || op == "mod") return mir::Instruction::MI_IDIV;
  if(op == "udiv" || op == "umod") return mir::Instruction::MI_DIV;
  throw logic_error("unsupported binary op " + op);
}

bool integer_binary_immediate_form_supported(const string & op,
                                             const string & type,
                                             long long value)
{
  if(op != "add" && op != "sub" && op != "mul") {
    return false;
  }
  if(op == "mul") {
    return true;
  }
  if(type == "i64" || type == "u64" || type == "ptr") {
    return value >= INT32_MIN && value <= INT32_MAX;
  }
  return true;
}

mir::Instruction::Opcode float_compare_opcode(const string & op)
{
  if(op == "eq") return mir::Instruction::MI_FEQ;
  if(op == "ne") return mir::Instruction::MI_FNE;
  if(op == "lt") return mir::Instruction::MI_FLT;
  if(op == "gt") return mir::Instruction::MI_FGT;
  if(op == "le") return mir::Instruction::MI_FLE;
  if(op == "ge") return mir::Instruction::MI_FGE;
  throw logic_error("unsupported cmp predicate " + op);
}

thread_local const lir::Instruction * g_machine_ir_source_instruction = nullptr;
thread_local size_t g_machine_ir_source_position = 0;
thread_local bool g_machine_ir_has_source_position = false;

void maybe_set_machine_ir_debug_location(mir::Instruction & inst)
{
  if(g_machine_ir_has_source_position) {
    inst.has_source_position = true;
    inst.source_position = g_machine_ir_source_position;
  }
  if(g_machine_ir_source_instruction == nullptr ||
     !g_machine_ir_source_instruction->debug_location.present()) {
    return;
  }

  inst.debug_location.file = g_machine_ir_source_instruction->debug_location.file;
  inst.debug_location.line = g_machine_ir_source_instruction->debug_location.line;
  inst.debug_location.column = g_machine_ir_source_instruction->debug_location.column;
}

mir::Instruction make_instruction(mir::Instruction::Opcode opcode)
{
  mir::Instruction inst;
  inst.opcode = opcode;
  maybe_set_machine_ir_debug_location(inst);
  return inst;
}

void emit_xmm_arg_move(const XmmCallArgMove & move,
                       vector<mir::Instruction> & out)
{
  if(move.src.kind == mir::Operand::OP_XMM && move.src.xmm == move.dst) {
    return;
  }

  mir::Instruction inst = make_instruction(mir::Instruction::MI_FMOV);
  inst.type = move.type;
  inst.operands.push_back(xmm(move.dst));
  inst.operands.push_back(move.src);
  out.push_back(inst);
}

void emit_xmm_arg_register_moves(vector<XmmCallArgMove> moves,
                                 const mir::Operand * spill_slot,
                                 vector<mir::Instruction> & out)
{
  for(size_t i = 0; i < moves.size(); ++i) {
    if(moves[i].src.kind == mir::Operand::OP_XMM &&
       moves[i].src.xmm == moves[i].dst) {
      moves[i].done = true;
    }
  }

  while(true) {
    bool any_pending = false;
    bool made_progress = false;
    for(size_t i = 0; i < moves.size(); ++i) {
      if(moves[i].done) {
        continue;
      }
      any_pending = true;
      if(!pending_xmm_arg_source_uses_register(moves, moves[i].dst)) {
        emit_xmm_arg_move(moves[i], out);
        moves[i].done = true;
        made_progress = true;
      }
    }
    if(!any_pending) {
      return;
    }
    if(made_progress) {
      continue;
    }

    size_t cycle = 0;
    while(cycle < moves.size() && moves[cycle].done) {
      ++cycle;
    }
    if(cycle == moves.size()) {
      return;
    }

    XmmRegister spare = XMM_0;
    if(find_spare_xmm_arg_move_register(moves, spare)) {
      XmmCallArgMove save;
      save.type = moves[cycle].type;
      save.dst = spare;
      save.src = moves[cycle].src;
      emit_xmm_arg_move(save, out);
      moves[cycle].src = xmm(spare);
    } else {
      if(spill_slot == nullptr) {
        throw logic_error("xmm call argument cycle requires spill slot");
      }
      mir::Instruction save = make_instruction(mir::Instruction::MI_FMOV);
      save.type = moves[cycle].type;
      save.operands.push_back(*spill_slot);
      save.operands.push_back(moves[cycle].src);
      out.push_back(save);
      moves[cycle].src = *spill_slot;
    }
  }
}

struct ScopedMachineIRSourceInstruction
{
  ScopedMachineIRSourceInstruction(const lir::Instruction & inst,
                                   size_t source_position)
    : saved_(g_machine_ir_source_instruction)
    , saved_source_position_(g_machine_ir_source_position)
    , saved_has_source_position_(g_machine_ir_has_source_position)
  {
    g_machine_ir_source_instruction = &inst;
    g_machine_ir_source_position = source_position;
    g_machine_ir_has_source_position = true;
  }

  ~ScopedMachineIRSourceInstruction()
  {
    g_machine_ir_source_instruction = saved_;
    g_machine_ir_source_position = saved_source_position_;
    g_machine_ir_has_source_position = saved_has_source_position_;
  }

private:
  const lir::Instruction * saved_;
  size_t saved_source_position_;
  bool saved_has_source_position_;
};

class MachineIRBuilder
{
public:
  MachineIRBuilder(const lir::Program & program,
                   const string & output_target,
                   bool enable_host_eh)
    : program_(program)
  {
    machine_.target = target_text(output_target);
    machine_.exported_symbols = program_.exported_symbols;
    for(size_t i = 0; i < program_.object_aliases.size(); ++i) {
      mir::ObjectAlias alias;
      alias.object_symbol = program_.object_aliases[i].object_symbol;
      alias.target = program_.object_aliases[i].target;
      machine_.object_aliases.push_back(alias);
    }
    for(size_t i = 0; i < program_.function_declarations.size(); ++i) {
      const lir::FunctionDeclaration & declaration = program_.function_declarations[i];
      function_names_.insert(declaration.name);
      function_params_[declaration.name] = declaration.params;
      merge_boundary_metadata(function_boundaries_[declaration.name], declaration.boundary);
      register_function_role(declaration.name, declaration.metadata.role);
      register_thread_local_wrapper(declaration.name,
                                    declaration.metadata.tls_for_symbol);
    }
    for(size_t i = 0; i < program_.functions.size(); ++i) {
      const lir::Function & function = program_.functions[i];
      function_names_.insert(function.name);
      defined_function_names_.insert(function.name);
      function_params_[function.name] = function.params;
      merge_boundary_metadata(function_boundaries_[function.name], function.boundary);
      register_function_role(function.name, function.metadata.role);
      register_thread_local_wrapper(function.name, function.metadata.tls_for_symbol);
    }
    if(enable_host_eh) {
      for(size_t i = 0; i < program_.functions.size(); ++i) {
        if(function_needs_host_eh_enabled(program_.functions[i])) {
          host_eh_requested_ = true;
          break;
        }
      }
    }
    for(size_t i = 0; i < program_.global_declarations.size(); ++i) {
      const lir::GlobalDeclaration & global = program_.global_declarations[i];
      global_names_.insert(global.name);
      if(global.storage == lir::GSM_THREAD_LOCAL) {
        thread_local_globals_.insert(global.name);
      }
      if(global.has_type) {
        scalar_global_types_[global.name] = global.type.text;
      }
    }
    for(size_t i = 0; i < program_.globals.size(); ++i) {
      const lir::GlobalDefinition & global = program_.globals[i];
      global_names_.insert(global.name);
      if(global.storage == lir::GSM_THREAD_LOCAL) {
        thread_local_globals_.insert(global.name);
      }
      if(!global.structured) {
        scalar_global_types_[global.name] = global.type.text;
      }
    }
    validate_thread_local_wrappers();
  }

  mir::Program build()
  {
    return build_internal(true);
  }

  mir::Program build_object()
  {
    return build_internal(false);
  }

private:
  mir::Program build_internal(bool include_startup)
  {
    validate_backend_subset();
    if(include_startup && !entry_function_name().empty()) {
      emit_startup();
    }
    for(size_t i = 0; i < program_.globals.size(); ++i) {
      emit_global(program_.globals[i]);
    }
    for(size_t i = 0; i < program_.functions.size(); ++i) {
      emit_function(program_.functions[i]);
    }
    return machine_;
  }
  lir::Program program_;
  set<string> function_names_;
  set<string> defined_function_names_;
  map<string, vector<lir::Parameter> > function_params_;
  map<string, lir::FunctionBoundaryMetadata> function_boundaries_;
  map<string, lir::SymbolRole> function_roles_;
  set<string> global_names_;
  set<string> thread_local_globals_;
  map<string, string> thread_local_wrapper_symbols_;
  map<string, string> scalar_global_types_;
  mir::Program machine_;
  bool host_eh_requested_ = false;

  void register_function_role(const string & name, lir::SymbolRole role)
  {
    if(role == lir::SR_NONE) {
      return;
    }
    map<string, lir::SymbolRole>::iterator found = function_roles_.find(name);
    if(found != function_roles_.end()) {
      if(found->second != role) {
        throw lir::ParseError("conflicting LowIR function roles for " + name);
      }
      return;
    }
    if(!lir::is_function_symbol_role(role)) {
      throw lir::ParseError("invalid function role on " + name);
    }
    for(map<string, lir::SymbolRole>::const_iterator it = function_roles_.begin();
        it != function_roles_.end(); ++it) {
      if(it->second == role && it->first != name) {
        throw lir::ParseError("duplicate LowIR function role " +
                              string(lir::symbol_role_text(role)));
      }
    }
    function_roles_[name] = role;
  }

  void register_thread_local_wrapper(const string & wrapper_symbol,
                                     const string & target_global)
  {
    if(target_global.empty()) {
      return;
    }
    if(wrapper_symbol == target_global) {
      throw lir::ParseError("tls_for metadata cannot target wrapper function " +
                            wrapper_symbol);
    }
    map<string, string>::const_iterator found =
        thread_local_wrapper_symbols_.find(target_global);
    if(found != thread_local_wrapper_symbols_.end() &&
       found->second != wrapper_symbol) {
      throw lir::ParseError("duplicate thread_local wrapper for " + target_global);
    }
    thread_local_wrapper_symbols_[target_global] = wrapper_symbol;
  }

  void validate_thread_local_wrappers() const
  {
    for(map<string, string>::const_iterator it = thread_local_wrapper_symbols_.begin();
        it != thread_local_wrapper_symbols_.end();
        ++it) {
      if(thread_local_globals_.count(it->first) == 0) {
        throw lir::ParseError("tls_for target is not a thread_local global " +
                              it->first);
      }
    }
  }

  string thread_local_wrapper_symbol_for_global(const string & global_symbol) const
  {
    map<string, string>::const_iterator found =
        thread_local_wrapper_symbols_.find(global_symbol);
    return found == thread_local_wrapper_symbols_.end() ? string() : found->second;
  }

  string first_defined_function_with_role(lir::SymbolRole role) const
  {
    for(map<string, lir::SymbolRole>::const_iterator it = function_roles_.begin();
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
    const string explicit_entry = first_defined_function_with_role(lir::SR_ENTRY);
    if(!explicit_entry.empty()) {
      return explicit_entry;
    }
    return defined_function_names_.count("@main") != 0 ? "@main" : string();
  }

  bool is_thread_local_global_name(const string & name) const
  {
    return thread_local_globals_.count(name) != 0;
  }

  bool is_thread_local_global_operand(const lir::Operand & operand) const
  {
    return operand.kind == lir::Operand::OP_GLOBAL &&
           is_thread_local_global_name(operand.text);
  }

  string init_function_name() const
  {
    const string explicit_init = first_defined_function_with_role(lir::SR_INIT);
    if(!explicit_init.empty()) {
      return explicit_init;
    }
    return defined_function_names_.count("@__cppgm_init") != 0 ? "@__cppgm_init" : string();
  }

  string fini_function_name() const
  {
    const string explicit_fini = first_defined_function_with_role(lir::SR_FINI);
    if(!explicit_fini.empty()) {
      return explicit_fini;
    }
    return defined_function_names_.count("@__cppgm_fini") != 0 ? "@__cppgm_fini" : string();
  }

  void validate_backend_subset() const
  {
    for(size_t gi = 0; gi < program_.globals.size(); ++gi) {
      const lir::GlobalDefinition & global = program_.globals[gi];
      (void)global;
    }

    for(size_t fi = 0; fi < program_.functions.size(); ++fi) {
      const lir::Function & function = program_.functions[fi];
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
          const lir::Instruction & inst = function.blocks[bi].instructions[ii];
          if(inst.kind == lir::Instruction::IK_BINARY &&
             is_float_type(inst.type) &&
             !(inst.op == "add" || inst.op == "sub" ||
               inst.op == "mul" || inst.op == "div")) {
            throw lir::ParseError("unsupported floating binary op " + inst.op);
          }
          if(inst.kind == lir::Instruction::IK_UNARY &&
             is_float_type(inst.type) &&
             !(inst.op == "neg" || inst.op == "not")) {
            throw lir::ParseError("unsupported floating unary op " + inst.op);
          }
          if(inst.kind == lir::Instruction::IK_UNARY &&
             inst.op == "decay" &&
             inst.type.text != "ptr") {
            throw lir::ParseError("decay unsupported on this type");
          }
          if((inst.kind == lir::Instruction::IK_ATOMIC_LOAD ||
              inst.kind == lir::Instruction::IK_ATOMIC_STORE ||
              inst.kind == lir::Instruction::IK_ATOMIC_ADD_FETCH ||
              inst.kind == lir::Instruction::IK_ATOMIC_EXCHANGE ||
              inst.kind == lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE) &&
             !is_atomic_scalar_type(inst.type.text)) {
            throw lir::ParseError("unsupported atomic scalar type " + inst.type.text);
          }
          if(inst.kind == lir::Instruction::IK_CONVERT) {
            const bool to_float = inst.op == "sitofp" || inst.op == "uitofp";
            const bool to_int = inst.op == "fptosi" || inst.op == "fptoui";
            const bool float_widen = inst.op == "fpext";
            const bool float_narrow = inst.op == "fptrunc";
            const bool integer_sign_extend = inst.op == "sext";
            const bool integer_zero_extend = inst.op == "zext";
            const bool integer_trunc = inst.op == "trunc";
            if(!(to_float || to_int || float_widen || float_narrow ||
                 integer_sign_extend || integer_zero_extend || integer_trunc)) {
              throw lir::ParseError("unsupported conversion op " + inst.op);
            }
            if((to_float && (!is_float_type(inst.type.text) ||
                             !is_integer_scalar_type(inst.source_type.text))) ||
               (to_int && (!is_float_type(inst.source_type.text) ||
                           !is_integer_scalar_type(inst.type.text))) ||
               ((integer_sign_extend || integer_zero_extend || integer_trunc) &&
                (!is_integer_scalar_type(inst.type.text) ||
                 !is_integer_scalar_type(inst.source_type.text))) ||
               ((float_widen || float_narrow) &&
                (!is_float_type(inst.type.text) ||
                 !is_float_type(inst.source_type.text)))) {
              throw lir::ParseError("invalid conversion signature for " + inst.op);
            }
            if(float_widen &&
               float_exec_width_bytes(inst.source_type.text) >=
                   float_exec_width_bytes(inst.type.text)) {
              throw lir::ParseError("fpext requires wider destination type");
            }
            if(float_narrow &&
               float_exec_width_bytes(inst.source_type.text) <=
                   float_exec_width_bytes(inst.type.text)) {
              throw lir::ParseError("fptrunc requires narrower destination type");
            }
            if((integer_sign_extend || integer_zero_extend) &&
               lir::type_size(inst.source_type) >= lir::type_size(inst.type)) {
              throw lir::ParseError(inst.op + " requires wider destination type");
            }
            if(integer_trunc &&
               lir::type_size(inst.source_type) <= lir::type_size(inst.type)) {
              throw lir::ParseError("trunc requires narrower destination type");
            }
          }
        }
      }
    }
  }

  void validate_function(const lir::Function & function) const
  {
    set<string> block_labels;
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      if(!block_labels.insert(function.blocks[i].label).second) {
        throw lir::ParseError("duplicate block " + function.blocks[i].label +
                              " in " + function.name);
      }
      if(function.blocks[i].instructions.empty()) {
        throw lir::ParseError("empty block " + function.blocks[i].label +
                              " in " + function.name);
      }
      const lir::Instruction & tail = function.blocks[i].instructions.back();
      const bool terminates =
          tail.kind == lir::Instruction::IK_JUMP ||
          tail.kind == lir::Instruction::IK_BRANCH ||
          tail.kind == lir::Instruction::IK_SWITCH ||
          tail.kind == lir::Instruction::IK_RETURN ||
          tail.kind == lir::Instruction::IK_THROW ||
          tail.kind == lir::Instruction::IK_RESUME;
      if(!terminates) {
        throw lir::ParseError("block " + function.blocks[i].label + " in " +
                              function.name + " is missing a terminator");
      }
    }
  }

  X86Condition integer_cmp_condition(const string & op) const
  {
    if(op == "eq") return XC_E;
    if(op == "ne") return XC_NE;
    if(op == "lt") return XC_L;
    if(op == "gt") return XC_G;
    if(op == "le") return XC_LE;
    if(op == "ge") return XC_GE;
    if(op == "ult") return XC_B;
    if(op == "ugt") return XC_A;
    if(op == "ule") return XC_BE;
    if(op == "uge") return XC_AE;
    throw logic_error("unsupported cmp predicate " + op);
  }

  void emit_jcc(X86Condition cond,
                const string & target,
                vector<mir::Instruction> & out) const
  {
    mir::Instruction mi = make_instruction(mir::Instruction::MI_JCC);
    mi.condition = cond;
    mi.operands.push_back(label(target));
    out.push_back(mi);
  }

  bool is_direct_integer_compare_memory_operand(const mir::Operand & operand) const
  {
    return operand.kind == mir::Operand::OP_FRAME ||
           operand.kind == mir::Operand::OP_DEREF ||
           operand.kind == mir::Operand::OP_GLOBAL;
  }

  bool direct_integer_compare_form_supported(const string & type,
                                             const mir::Operand & lhs,
                                             const mir::Operand & rhs) const
  {
    const bool word_compare =
        type == "i32" || type == "u32" ||
        type == "i64" || type == "u64" ||
        type == "ptr";
    if(lhs.kind == mir::Operand::OP_REG) {
      if(rhs.kind == mir::Operand::OP_REG) {
        return true;
      }
      if(rhs.kind == mir::Operand::OP_IMM) {
        if(type == "i64" || type == "u64" || type == "ptr") {
          return rhs.imm >= INT32_MIN && rhs.imm <= INT32_MAX;
        }
        return word_compare;
      }
      return word_compare && is_direct_integer_compare_memory_operand(rhs);
    }
    if(is_direct_integer_compare_memory_operand(lhs)) {
      if(rhs.kind == mir::Operand::OP_IMM) {
        if(type == "i64" || type == "u64" || type == "ptr") {
          return rhs.imm >= INT32_MIN && rhs.imm <= INT32_MAX;
        }
        return true;
      }
      return word_compare && rhs.kind == mir::Operand::OP_REG;
    }
    return false;
  }

  bool try_direct_integer_compare_operand(const FunctionLayout & layout,
                                          const lir::Operand & operand,
                                          const string & type,
                                          mir::Operand & out) const
  {
    const auto direct_thread_local_global =
        [&](const lir::Operand & source) -> bool {
          return source.kind == lir::Operand::OP_GLOBAL &&
                 is_thread_local_global_name(source.text);
        };
    const auto storage_type_compatible =
        [&](const lir::Operand & source) -> bool {
          if(direct_thread_local_global(source)) {
            return false;
          }
          if(source.kind != lir::Operand::OP_SLOT &&
             source.kind != lir::Operand::OP_GLOBAL &&
             source.kind != lir::Operand::OP_TEMP) {
            return true;
          }
          const string source_type = operand_type(layout, source);
          const size_t source_width = lir::type_size(lir::LowType{source_type});
          const size_t desired_width = lir::type_size(lir::LowType{type});
          return !(source_width < 8 &&
                   source_width < desired_width &&
                   is_atomic_scalar_type(source_type) &&
                   source_type != "ptr");
        };
    if(operand.kind == lir::Operand::OP_SLOT ||
       operand.kind == lir::Operand::OP_GLOBAL ||
       operand.kind == lir::Operand::OP_INTEGER) {
      if(!storage_type_compatible(operand)) {
        return false;
      }
      out = integer_source_operand(layout, operand);
      return true;
    }
    if(operand.kind != lir::Operand::OP_TEMP) {
      return false;
    }
    map<string, lir::Operand>::const_iterator elided =
        layout.elided_direct_branch_load_sources.find(operand.text);
    if(elided != layout.elided_direct_branch_load_sources.end()) {
      const lir::Operand & source = elided->second;
      if(storage_type_compatible(source) &&
         !direct_thread_local_global(source) &&
         (source.kind == lir::Operand::OP_SLOT ||
          source.kind == lir::Operand::OP_GLOBAL)) {
        out = integer_source_operand(layout, source);
        return true;
      }
    }
    if(const X64Register * assigned = temp_register_for(layout, operand.text)) {
      out = reg(*assigned);
      return true;
    }
    if(layout.direct_branch_temps.count(operand.text) == 0) {
      if(layout.storage_offset.count(operand.text) == 0) {
        return false;
      }
      if(!storage_type_compatible(operand)) {
        return false;
      }
      out = frame(layout, operand.text);
      return true;
    }
    return false;
  }

  bool integer_immediate_fits_type(long long imm, const string & type) const
  {
    if(type == "i8") return imm >= INT8_MIN && imm <= INT8_MAX;
    if(type == "u8") return imm >= 0 && imm <= UINT8_MAX;
    if(type == "i16") return imm >= INT16_MIN && imm <= INT16_MAX;
    if(type == "u16") return imm >= 0 && imm <= UINT16_MAX;
    if(type == "i32") return imm >= INT32_MIN && imm <= INT32_MAX;
    if(type == "u32") return imm >= 0 && imm <= INT32_MAX;
    if(type == "i64" || type == "u64" || type == "ptr") return true;
    return false;
  }

  bool try_narrow_direct_integer_compare_type(const FunctionLayout & layout,
                                              const lir::Instruction & cmp,
                                              string & out_type) const
  {
    if(cmp.op != "eq" && cmp.op != "ne") {
      return false;
    }
    const lir::Operand * value = nullptr;
    const lir::Operand * imm = nullptr;
    if(cmp.first.kind == lir::Operand::OP_INTEGER) {
      imm = &cmp.first;
      value = &cmp.second;
    } else if(cmp.second.kind == lir::Operand::OP_INTEGER) {
      value = &cmp.first;
      imm = &cmp.second;
    } else {
      return false;
    }
    if(value->kind != lir::Operand::OP_SLOT &&
       value->kind != lir::Operand::OP_GLOBAL &&
       value->kind != lir::Operand::OP_TEMP) {
      return false;
    }
    const string source_type = operand_type(layout, *value);
    const size_t source_width = lir::type_size(lir::LowType{source_type});
    const size_t desired_width = lir::type_size(lir::LowType{cmp.type.text});
    if(!(source_width < 8 &&
         source_width < desired_width &&
         is_atomic_scalar_type(source_type) &&
         source_type != "ptr")) {
      return false;
    }
    if(!integer_immediate_fits_type(imm->int_value, source_type)) {
      return false;
    }
    out_type = source_type;
    return true;
  }

  void emit_direct_integer_compare_branch(const FunctionLayout & layout,
                                          const lir::Instruction & cmp,
                                          const lir::Instruction & branch,
                                          vector<mir::Instruction> & out) const
  {
    const X64Register lhs = XR_RAX;
    const X64Register rhs = alternate_integer_scratch(lhs);
    mir::Instruction mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = cmp.type.text;
    mir::Operand lhs_operand;
    mir::Operand rhs_operand;
    if(try_direct_integer_compare_operand(layout, cmp.first, cmp.type.text, lhs_operand) &&
       try_direct_integer_compare_operand(layout, cmp.second, cmp.type.text, rhs_operand) &&
       direct_integer_compare_form_supported(cmp.type.text, lhs_operand, rhs_operand)) {
      mi.operands.push_back(lhs_operand);
      mi.operands.push_back(rhs_operand);
    } else {
      emit_load_value(layout, cmp.first, cmp.type.text, lhs, out);
      mi.operands.push_back(reg(lhs));
      if(try_direct_integer_compare_operand(layout, cmp.second, cmp.type.text, rhs_operand) &&
         direct_integer_compare_form_supported(cmp.type.text, reg(lhs), rhs_operand)) {
        mi.operands.push_back(rhs_operand);
      } else {
        emit_load_value(layout, cmp.second, cmp.type.text, rhs, out);
        mi.operands.push_back(reg(rhs));
      }
    }
    out.push_back(mi);
    emit_jcc(integer_cmp_condition(cmp.op), branch.second.text, out);
    mi = make_instruction(mir::Instruction::MI_JMP);
    mi.operands.push_back(label(branch.third.text));
    out.push_back(mi);
  }

  void emit_direct_integer_not_branch(const FunctionLayout & layout,
                                      const lir::Instruction & unary,
                                      const lir::Instruction & branch,
                                      vector<mir::Instruction> & out) const
  {
    const X64Register value = XR_RAX;
    emit_load_value(layout, unary.first, unary.type.text, value, out);
    mir::Instruction mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = unary.type.text;
    mi.operands.push_back(reg(value));
    mi.operands.push_back(imm(0));
    out.push_back(mi);
    emit_jcc(XC_E, branch.second.text, out);
    mi = make_instruction(mir::Instruction::MI_JMP);
    mi.operands.push_back(label(branch.third.text));
    out.push_back(mi);
  }

  void emit_direct_float_compare_branch(const FunctionLayout & layout,
                                        const lir::Instruction & cmp,
                                        const lir::Instruction & branch,
                                        vector<mir::Instruction> & out) const
  {
    mir::Instruction mi = make_instruction(mir::Instruction::MI_FCMP);
    mi.type = cmp.type.text;
    mi.operands.push_back(float_source_operand(layout, cmp.first));
    mi.operands.push_back(float_source_operand(layout, cmp.second));
    out.push_back(mi);

    if(cmp.op == "eq") {
      emit_jcc(XC_P, branch.third.text, out);
      emit_jcc(XC_E, branch.second.text, out);
    } else if(cmp.op == "ne") {
      emit_jcc(XC_P, branch.second.text, out);
      emit_jcc(XC_NE, branch.second.text, out);
    } else if(cmp.op == "lt") {
      emit_jcc(XC_A, branch.second.text, out);
    } else if(cmp.op == "le") {
      emit_jcc(XC_AE, branch.second.text, out);
    } else if(cmp.op == "gt") {
      emit_jcc(XC_P, branch.third.text, out);
      emit_jcc(XC_B, branch.second.text, out);
    } else if(cmp.op == "ge") {
      emit_jcc(XC_P, branch.third.text, out);
      emit_jcc(XC_BE, branch.second.text, out);
    } else {
      throw logic_error("unsupported floating cmp predicate " + cmp.op);
    }

    mi = make_instruction(mir::Instruction::MI_JMP);
    mi.operands.push_back(label(branch.third.text));
    out.push_back(mi);
  }

  bool emit_direct_branch(const FunctionLayout & layout,
                          const lir::Instruction & inst,
                          vector<mir::Instruction> & out) const
  {
    if(inst.first.kind != lir::Operand::OP_TEMP ||
       layout.direct_branch_temps.count(inst.first.text) == 0) {
      return false;
    }
    map<string, lir::Instruction>::const_iterator cmp_it =
        layout.temp_def_instruction.find(inst.first.text);
    if(cmp_it == layout.temp_def_instruction.end()) {
      throw logic_error("missing direct-branch def for " + inst.first.text);
    }
    const lir::Instruction & source = cmp_it->second;
    if(source.kind == lir::Instruction::IK_CMP) {
      if(is_float_type(source.type)) {
        emit_direct_float_compare_branch(layout, source, inst, out);
      } else {
        emit_direct_integer_compare_branch(layout, source, inst, out);
      }
      return true;
    }
    if(source.kind == lir::Instruction::IK_UNARY &&
       source.op == "not" &&
       !is_float_type(source.type)) {
      emit_direct_integer_not_branch(layout, source, inst, out);
      return true;
    }
    return false;
  }

  string operand_type(const FunctionLayout & layout,
                      const lir::Operand & operand) const
  {
    switch(operand.kind) {
      case lir::Operand::OP_TEMP:
      case lir::Operand::OP_SLOT: {
        map<string, string>::const_iterator found =
            layout.storage_type.find(operand.text);
        if(found == layout.storage_type.end()) {
          throw logic_error(unknown_storage_error(layout, operand.text));
        }
        return found->second;
      }
      case lir::Operand::OP_GLOBAL: {
        map<string, string>::const_iterator found =
            scalar_global_types_.find(operand.text);
        if(found != scalar_global_types_.end()) {
          return found->second;
        }
        return "ptr";
      }
      case lir::Operand::OP_INTEGER:
      case lir::Operand::OP_LABEL:
        return "i64";
      case lir::Operand::OP_FLOAT:
        return operand.literal_type.text;
    }
    return "i64";
  }

  void emit_startup()
  {
    const string init_name = init_function_name();
    const string entry_name = entry_function_name();
    const string fini_name = fini_function_name();
    mir::Instruction inst;
    if(!init_name.empty()) {
      inst = make_instruction(mir::Instruction::MI_CALL);
      inst.operands.push_back(symbol(init_name));
      machine_.startup.push_back(inst);
    }

    inst = make_instruction(mir::Instruction::MI_CALL);
    inst.operands.push_back(symbol(entry_name));
    machine_.startup.push_back(inst);

    if(!fini_name.empty()) {
      inst = make_instruction(mir::Instruction::MI_MOV);
      inst.operands.push_back(reg(XR_R12));
      inst.operands.push_back(reg(XR_RAX));
      machine_.startup.push_back(inst);

      inst = make_instruction(mir::Instruction::MI_CALL);
      inst.operands.push_back(symbol(fini_name));
      machine_.startup.push_back(inst);

      inst = make_instruction(mir::Instruction::MI_MOV);
      inst.operands.push_back(reg(XR_RDI));
      inst.operands.push_back(reg(XR_R12));
      machine_.startup.push_back(inst);
    } else {
      inst = make_instruction(mir::Instruction::MI_MOV);
      inst.operands.push_back(reg(XR_RDI));
      inst.operands.push_back(reg(XR_RAX));
      machine_.startup.push_back(inst);
    }

    inst = make_instruction(mir::Instruction::MI_EXIT);
    machine_.startup.push_back(inst);
  }

  void emit_global(const lir::GlobalDefinition & global)
  {
    mir::GlobalDefinition out;
    out.name = global.name;
    out.readonly = global.storage == lir::GSM_READONLY;
    out.thread_local_storage = global.storage == lir::GSM_THREAD_LOCAL;
    out.section_segment = global.metadata.section_segment;
    out.section_name = global.metadata.section_name;
    if(out.thread_local_storage) {
      out.thread_local_wrapper_symbol =
          thread_local_wrapper_symbol_for_global(global.name);
    }
    if(!global.structured) {
      out.storage_kind = mir::GlobalDefinition::GS_SCALAR;
      out.type = global.type.text;
      switch(global.init_kind) {
        case lir::GlobalDefinition::INIT_ZERO:
          out.init_kind = mir::GlobalDefinition::GI_ZERO;
          break;
        case lir::GlobalDefinition::INIT_INTEGER:
          if(is_float_type(out.type)) {
            out.init_kind = mir::GlobalDefinition::GI_FLOAT;
            out.float_value = scalar_literal_float(global.init_operand);
            out.literal_text = global.init_operand.text;
          } else {
            out.init_kind = mir::GlobalDefinition::GI_INTEGER;
            out.int_value = scalar_literal_bits(global.init_operand, out.type);
          }
          break;
        case lir::GlobalDefinition::INIT_ADDR:
          out.init_kind = mir::GlobalDefinition::GI_ADDR;
          out.symbol = global.init_operand.text;
          out.addr_addend = global.addr_addend;
          break;
      }
      machine_.globals.push_back(out);
      return;
    }

    out.storage_kind = mir::GlobalDefinition::GS_DATA;
    for(size_t i = 0; i < global.data_items.size(); ++i) {
      const lir::GlobalDefinition::DataItem & item = global.data_items[i];
      mir::GlobalDefinition::DataItem out_item;
      if(item.kind == lir::GlobalDefinition::DataItem::ITEM_INTEGER) {
        out_item.type = item.type.text;
        if(is_float_type(out_item.type)) {
          out_item.kind = mir::GlobalDefinition::DataItem::ITEM_FLOAT;
          out_item.float_value = scalar_literal_float(item.literal_operand);
          out_item.literal_text = item.literal_operand.text;
        } else {
          out_item.kind = mir::GlobalDefinition::DataItem::ITEM_INTEGER;
          out_item.int_value = scalar_literal_bits(item.literal_operand, out_item.type);
        }
      } else if(item.kind == lir::GlobalDefinition::DataItem::ITEM_ADDR) {
        out_item.kind = mir::GlobalDefinition::DataItem::ITEM_ADDR;
        out_item.symbol = item.symbol;
        out_item.addr_addend = item.addr_addend;
      } else {
        out_item.kind = mir::GlobalDefinition::DataItem::ITEM_ZERO;
        out_item.zero_bytes = item.zero_bytes;
      }
      out.data_items.push_back(out_item);
    }
    machine_.globals.push_back(out);
  }

  void emit_load_value(const FunctionLayout & layout,
                       const lir::Operand & operand,
                       const string & desired_type,
                       X64Register dst,
                       vector<mir::Instruction> & out) const
  {
    if(is_float_type(desired_type)) {
      throw logic_error("emit_load_value only supports integer/pointer types");
    }
    mir::Instruction inst;
    switch(operand.kind) {
      case lir::Operand::OP_INTEGER:
        inst = make_instruction(mir::Instruction::MI_MOV);
        inst.operands.push_back(reg(dst));
        inst.operands.push_back(imm(operand.int_value));
        out.push_back(inst);
        return;
      case lir::Operand::OP_SLOT:
      {
        map<string, string>::const_iterator promoted =
            layout.promoted_param_slots.find(operand.text);
        if(promoted != layout.promoted_param_slots.end()) {
          lir::Operand forwarded;
          forwarded.kind = lir::Operand::OP_TEMP;
          forwarded.text = promoted->second;
          emit_load_value(layout, forwarded, desired_type, dst, out);
          return;
        }
      }
      case lir::Operand::OP_TEMP: {
        if(operand.kind == lir::Operand::OP_TEMP) {
          map<string, lir::Operand>::const_iterator elided =
              layout.elided_direct_branch_load_sources.find(operand.text);
          if(elided != layout.elided_direct_branch_load_sources.end()) {
            emit_load_value(layout, elided->second, desired_type, dst, out);
            return;
          }
          const X64Register * assigned = temp_register_for(layout, operand.text);
          if(assigned) {
            if(*assigned != dst) {
              inst = make_instruction(mir::Instruction::MI_MOV);
              inst.operands.push_back(reg(dst));
              inst.operands.push_back(reg(*assigned));
              out.push_back(inst);
            }
            return;
          }
          if(rematerialized_slot_address_def(layout, operand.text) != nullptr) {
            emit_load_address(layout, operand, dst, out);
            return;
          }
        }
        const string source_type = operand_type(layout, operand);
        const size_t source_width = lir::type_size(lir::LowType{source_type});
        const size_t desired_width = lir::type_size(lir::LowType{desired_type});
        const bool narrow_integer_reload =
            source_width < 8 &&
            source_width < desired_width &&
            is_atomic_scalar_type(source_type) &&
            source_type != "ptr";
        inst = make_instruction(mir::Instruction::MI_LOAD);
        inst.type = narrow_integer_reload ? source_type : desired_type;
        inst.operands.push_back(reg(dst));
        inst.operands.push_back(frame(layout, operand.text));
        out.push_back(inst);
        if(narrow_integer_reload &&
           lir::is_sign_extended_integer_type(lir::LowType{source_type})) {
          emit_sign_extend_to_i64(dst, source_width, out);
        }
        return;
      }
      case lir::Operand::OP_GLOBAL:
        if(function_names_.count(operand.text) != 0) {
          inst = make_instruction(mir::Instruction::MI_MOV);
          inst.operands.push_back(reg(dst));
          inst.operands.push_back(symbol(operand.text));
        } else if(global_names_.count(operand.text) != 0) {
          const string source_type = operand_type(layout, operand);
          const size_t source_width = lir::type_size(lir::LowType{source_type});
          const size_t desired_width = lir::type_size(lir::LowType{desired_type});
          const bool narrow_integer_reload =
              source_width < 8 &&
              source_width < desired_width &&
              is_atomic_scalar_type(source_type) &&
              source_type != "ptr";
          if(is_thread_local_global_name(operand.text)) {
            emit_load_address(layout, operand, XR_R11, out);
            inst = make_instruction(mir::Instruction::MI_LOAD);
            inst.type = narrow_integer_reload ? source_type : desired_type;
            inst.operands.push_back(reg(dst));
            inst.operands.push_back(deref(XR_R11));
            out.push_back(inst);
            if(narrow_integer_reload &&
               lir::is_sign_extended_integer_type(lir::LowType{source_type})) {
              emit_sign_extend_to_i64(dst, source_width, out);
            }
            return;
          }
          inst = make_instruction(mir::Instruction::MI_LOAD);
          inst.type = narrow_integer_reload ? source_type : desired_type;
          inst.operands.push_back(reg(dst));
          inst.operands.push_back(global_ref(operand.text));
          out.push_back(inst);
          if(narrow_integer_reload &&
             lir::is_sign_extended_integer_type(lir::LowType{source_type})) {
            emit_sign_extend_to_i64(dst, source_width, out);
          }
          return;
        } else {
          throw lir::ParseError("unknown symbol " + operand.text);
        }
        out.push_back(inst);
        return;
      case lir::Operand::OP_LABEL:
        inst = make_instruction(mir::Instruction::MI_MOV);
        inst.operands.push_back(reg(dst));
        inst.operands.push_back(label(operand.text));
        out.push_back(inst);
        return;
      case lir::Operand::OP_FLOAT:
        break;
    }
    throw logic_error("invalid non-integer operand kind for emit_load_value");
  }

  void emit_sign_extend_to_i64(X64Register dst,
                               size_t source_width,
                               vector<mir::Instruction> & out) const
  {
    if(source_width == 0 || source_width >= 8) {
      return;
    }
    mir::Instruction inst = make_instruction(mir::Instruction::MI_SEXT);
    inst.byte_count = source_width;
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_zero_extend_to_i64(X64Register dst,
                               size_t source_width,
                               vector<mir::Instruction> & out) const
  {
    if(source_width == 0 || source_width >= 8) {
      return;
    }
    mir::Instruction inst = make_instruction(mir::Instruction::MI_ZEXT);
    inst.byte_count = source_width;
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_normalize_integer_temp(const string & type,
                                   X64Register dst,
                                   vector<mir::Instruction> & out) const
  {
    if(type == "ptr" || type == "i64") {
      return;
    }
    const size_t width = lir::type_size(lir::LowType{type});
    if(width == 0 || width >= 8) {
      return;
    }
    if(lir::is_sign_extended_integer_type(lir::LowType{type})) {
      emit_sign_extend_to_i64(dst, width, out);
    } else {
      emit_zero_extend_to_i64(dst, width, out);
    }
  }

  void emit_load_address(const FunctionLayout & layout,
                         const lir::Operand & operand,
                         X64Register dst,
                         vector<mir::Instruction> & out) const
  {
    mir::Instruction inst;
    if(operand.kind == lir::Operand::OP_SLOT || operand.kind == lir::Operand::OP_TEMP) {
      if(operand.kind == lir::Operand::OP_TEMP) {
        if(const lir::Instruction * rematerialized =
               rematerialized_slot_address_def(layout, operand.text)) {
          inst = make_instruction(mir::Instruction::MI_LEA);
          inst.operands.push_back(reg(dst));
          inst.operands.push_back(frame(layout, rematerialized->first.text));
          out.push_back(inst);
          return;
        }
        if(temp_register_for(layout, operand.text)) {
          throw logic_error("register temp does not have a stable address " + operand.text);
        }
      }
      inst = make_instruction(mir::Instruction::MI_LEA);
      inst.operands.push_back(reg(dst));
      inst.operands.push_back(frame(layout, operand.text));
      out.push_back(inst);
      return;
    }
    if(operand.kind == lir::Operand::OP_GLOBAL) {
      if(thread_local_globals_.count(operand.text) != 0) {
        const string wrapper_symbol =
            thread_local_wrapper_symbol_for_global(operand.text);
        if(wrapper_symbol.empty()) {
          throw lir::ParseError("thread_local address requires tls_for wrapper for " +
                                operand.text);
        }
        inst = make_instruction(mir::Instruction::MI_TLS_ADDR);
        inst.operands.push_back(reg(dst));
        inst.operands.push_back(symbol(wrapper_symbol));
        out.push_back(inst);
        return;
      }
      inst = make_instruction(mir::Instruction::MI_MOV);
      inst.operands.push_back(reg(dst));
      inst.operands.push_back(symbol(operand.text));
      out.push_back(inst);
      return;
    }
    throw logic_error("invalid address operand " + operand.text);
  }

  void emit_zero_register(X64Register dst,
                          vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_MOV);
    inst.operands.push_back(reg(dst));
    inst.operands.push_back(imm(0));
    out.push_back(inst);
  }

  void emit_parallel_integer_param_moves(
      const FunctionLayout & layout,
      const vector<pair<X64Register, X64Register> > & requested_moves,
      vector<mir::Instruction> & out) const
  {
    struct PendingMove
    {
      X64Register dst = XR_RAX;
      X64Register src = XR_RAX;
      bool src_spilled = false;
    };

    vector<PendingMove> pending;
    for(size_t i = 0; i < requested_moves.size(); ++i) {
      if(requested_moves[i].first == requested_moves[i].second) {
        continue;
      }
      PendingMove move;
      move.dst = requested_moves[i].first;
      move.src = requested_moves[i].second;
      pending.push_back(move);
    }

    while(!pending.empty()) {
      bool progressed = false;
      for(size_t i = 0; i < pending.size(); ++i) {
        bool dst_is_live_source = false;
        if(!pending[i].src_spilled) {
          for(size_t j = 0; j < pending.size(); ++j) {
            if(i == j || pending[j].src_spilled) {
              continue;
            }
            if(pending[j].src == pending[i].dst) {
              dst_is_live_source = true;
              break;
            }
          }
        }
        if(dst_is_live_source) {
          continue;
        }

        mir::Instruction inst;
        if(pending[i].src_spilled) {
          inst = make_instruction(mir::Instruction::MI_LOAD);
          inst.type = "i64";
          inst.operands.push_back(reg(pending[i].dst));
          inst.operands.push_back(deref_offset(XR_RBP,
                                               preserve_spill_slot_offset(layout, 0)));
        } else {
          inst = make_instruction(mir::Instruction::MI_MOV);
          inst.operands.push_back(reg(pending[i].dst));
          inst.operands.push_back(reg(pending[i].src));
        }
        out.push_back(inst);
        pending.erase(pending.begin() + i);
        progressed = true;
        break;
      }
      if(progressed) {
        continue;
      }

      if(!layout.has_preserve_spill) {
        throw logic_error("missing preserve spill for entry parameter move cycle");
      }

      mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
      inst.type = "i64";
      inst.operands.push_back(deref_offset(XR_RBP,
                                           preserve_spill_slot_offset(layout, 0)));
      inst.operands.push_back(reg(pending.front().src));
      out.push_back(inst);
      pending.front().src_spilled = true;
    }
  }

  void emit_entry_variadic_register_save_area(const FunctionLayout & layout,
                                              vector<mir::Instruction> & out) const
  {
    if(!layout.variadic) {
      return;
    }
    if(layout.va_reg_save_area_offset == 0) {
      throw logic_error("missing variadic register save area");
    }
    static const X64Register gp_regs[] = {
      XR_RDI, XR_RSI, XR_RDX, XR_RCX, XR_R8, XR_R9
    };
    for(size_t i = 0; i < sizeof(gp_regs) / sizeof(gp_regs[0]); ++i) {
      mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
      inst.type = "i64";
      inst.operands.push_back(deref_offset(
          XR_RBP,
          -static_cast<long long>(layout.va_reg_save_area_offset) +
              static_cast<long long>(i * 8)));
      inst.operands.push_back(reg(gp_regs[i]));
      out.push_back(inst);
    }
    for(size_t i = 0; i < 8; ++i) {
      emit_float_move_or_convert(deref_offset(
                                     XR_RBP,
                                     -static_cast<long long>(layout.va_reg_save_area_offset) +
                                         48 + static_cast<long long>(i * 16)),
                                 "f64",
                                 "f64",
                                 xmm(float_arg_register(i)),
                                 out);
    }
  }

  void emit_entry_param_setup(const FunctionLayout & layout,
                              const vector<mir::ParamBinding> & params,
                              vector<mir::Instruction> & out,
                              set<string> & preinitialized_param_slots) const
  {
    struct PendingStackLoad
    {
      X64Register dst = XR_RAX;
      long long stack_offset = 0;
      string type;
    };

    emit_entry_variadic_register_save_area(layout, out);

    vector<pair<X64Register, X64Register> > integer_reg_moves;
    vector<PendingStackLoad> integer_stack_loads;
    for(size_t i = 0; i < params.size(); ++i) {
      const mir::ParamBinding & param = params[i];
      const bool has_int_temp =
          temp_register_for(layout, param.name) != nullptr &&
          param.chunk_offset == 0;
      const bool has_float_temp =
          float_temp_register_for(layout, param.name) != nullptr &&
          param.chunk_offset == 0;
      if(has_int_temp || has_float_temp || layout.storage_offset.count(param.name) == 0) {
        continue;
      }
      preinitialized_param_slots.insert(param.name);
      const long long local_offset = slot_offset(layout, param.name) + param.chunk_offset;
      if(param.location == mir::ParamBinding::PL_STACK &&
         is_memory_class_object_abi_type(param.type)) {
        size_t copied = 0;
        const size_t byte_count = lir::type_size(lir::LowType{param.type});
        while(copied < byte_count) {
          const size_t chunk_size = min<size_t>(8, byte_count - copied);
          const string chunk_type = integer_chunk_type_for_size(chunk_size);
          mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
          inst.type = chunk_type;
          inst.operands.push_back(reg(XR_RAX));
          inst.operands.push_back(
              deref_offset(XR_RBP, param.stack_offset + static_cast<long long>(copied)));
          out.push_back(inst);

          inst = make_instruction(mir::Instruction::MI_STORE);
          inst.type = chunk_type;
          inst.operands.push_back(
              deref_offset(XR_RBP, local_offset + static_cast<long long>(copied)));
          inst.operands.push_back(reg(XR_RAX));
          out.push_back(inst);
          copied += chunk_size;
        }
        continue;
      }
      if(param.location == mir::ParamBinding::PL_REG) {
        mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
        inst.type = param.type;
        inst.operands.push_back(deref_offset(XR_RBP, local_offset));
        inst.operands.push_back(reg(param.reg));
        out.push_back(inst);
        continue;
      }
      if(param.location == mir::ParamBinding::PL_XMM) {
        emit_float_move_or_convert(deref_offset(XR_RBP, local_offset),
                                   param.type,
                                   param.type,
                                   xmm(param.xmm),
                                   out);
        continue;
      }
      if(param.location == mir::ParamBinding::PL_STACK) {
        if(is_float_type(param.type)) {
          emit_float_move_or_convert(deref_offset(XR_RBP, local_offset),
                                     param.type,
                                     param.type,
                                     deref_offset(XR_RBP, param.stack_offset),
                                     out);
        } else {
          mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
          inst.type = param.type;
          inst.operands.push_back(reg(XR_RAX));
          inst.operands.push_back(deref_offset(XR_RBP, param.stack_offset));
          out.push_back(inst);

          inst = make_instruction(mir::Instruction::MI_STORE);
          inst.type = param.type;
          inst.operands.push_back(deref_offset(XR_RBP, local_offset));
          inst.operands.push_back(reg(XR_RAX));
          out.push_back(inst);
        }
      }
    }

    for(size_t i = 0; i < params.size(); ++i) {
      const mir::ParamBinding & param = params[i];
      const X64Register * assigned = temp_register_for(layout, param.name);
      if(assigned != nullptr && param.chunk_offset == 0) {
        if(param.location == mir::ParamBinding::PL_REG) {
          integer_reg_moves.push_back(make_pair(*assigned, param.reg));
          continue;
        }
        if(param.location == mir::ParamBinding::PL_STACK) {
          PendingStackLoad load;
          load.dst = *assigned;
          load.stack_offset = param.stack_offset;
          load.type = param.type;
          integer_stack_loads.push_back(load);
        }
        continue;
      }

      const XmmRegister * float_assigned = float_temp_register_for(layout, param.name);
      if(float_assigned != nullptr && param.chunk_offset == 0) {
        if(param.location == mir::ParamBinding::PL_XMM) {
          if(*float_assigned != param.xmm) {
            emit_float_move_or_convert(xmm(*float_assigned),
                                       param.type,
                                       param.type,
                                       xmm(param.xmm),
                                       out);
          }
          continue;
        }
        if(param.location == mir::ParamBinding::PL_STACK) {
          emit_float_move_or_convert(xmm(*float_assigned),
                                     param.type,
                                     param.type,
                                     deref_offset(XR_RBP, param.stack_offset),
                                     out);
        }
        continue;
      }
    }

    emit_parallel_integer_param_moves(layout, integer_reg_moves, out);
    for(size_t i = 0; i < integer_stack_loads.size(); ++i) {
      mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
      inst.type = integer_stack_loads[i].type;
      inst.operands.push_back(reg(integer_stack_loads[i].dst));
      inst.operands.push_back(deref_offset(XR_RBP, integer_stack_loads[i].stack_offset));
      out.push_back(inst);
    }
  }

  bool is_redundant_param_materialization(const FunctionLayout & layout,
                                          const lir::Instruction & inst,
                                          const set<string> & preinitialized_param_slots) const
  {
    if(inst.kind == lir::Instruction::IK_STORE &&
       inst.first.kind == lir::Operand::OP_TEMP &&
       inst.second.kind == lir::Operand::OP_SLOT &&
       inst.first.text == inst.second.text &&
       preinitialized_param_slots.count(inst.first.text) != 0) {
      return true;
    }

    if(inst.kind == lir::Instruction::IK_COPYOBJ &&
       inst.first.kind == lir::Operand::OP_TEMP &&
       preinitialized_param_slots.count(inst.first.text) != 0 &&
       inst.second.kind == lir::Operand::OP_TEMP) {
      map<string, lir::Instruction>::const_iterator found =
          layout.temp_def_instruction.find(inst.second.text);
      if(found != layout.temp_def_instruction.end() &&
         found->second.kind == lir::Instruction::IK_ADDR &&
         found->second.first.kind == lir::Operand::OP_SLOT) {
        if(found->second.first.text == inst.first.text) {
          return true;
        }
        map<string, string>::const_iterator aliased =
            layout.aliased_param_slots.find(found->second.first.text);
        if(aliased != layout.aliased_param_slots.end() &&
           aliased->second == inst.first.text) {
          return true;
        }
      }
    }

    if(inst.kind == lir::Instruction::IK_COPYOBJ &&
       inst.first.kind == lir::Operand::OP_TEMP &&
       inst.second.kind == lir::Operand::OP_TEMP) {
      map<string, lir::Instruction>::const_iterator found =
          layout.temp_def_instruction.find(inst.second.text);
      if(found != layout.temp_def_instruction.end() &&
         found->second.kind == lir::Instruction::IK_ADDR &&
         found->second.first.kind == lir::Operand::OP_SLOT) {
        map<string, string>::const_iterator aliased =
            layout.aliased_param_slots.find(found->second.first.text);
        if(aliased != layout.aliased_param_slots.end() &&
           aliased->second == inst.first.text) {
          return true;
        }
      }
    }

    return false;
  }

  bool is_redundant_aliased_object_return_materialization(
      const FunctionLayout & layout,
      const lir::Instruction & inst) const
  {
    if(inst.kind != lir::Instruction::IK_COPYOBJ ||
       inst.first.kind != lir::Operand::OP_TEMP ||
       inst.second.kind != lir::Operand::OP_TEMP) {
      return false;
    }

    map<string, lir::Instruction>::const_iterator found =
        layout.temp_def_instruction.find(inst.second.text);
    if(found == layout.temp_def_instruction.end() ||
       found->second.kind != lir::Instruction::IK_ADDR ||
       found->second.first.kind != lir::Operand::OP_SLOT) {
      return false;
    }

    map<string, string>::const_iterator aliased =
        layout.aliased_object_return_slots.find(inst.first.text);
    return aliased != layout.aliased_object_return_slots.end() &&
           aliased->second == found->second.first.text;
  }

  bool is_promoted_param_slot_store(const FunctionLayout & layout,
                                    const lir::Instruction & inst) const
  {
    if(inst.kind != lir::Instruction::IK_STORE ||
       inst.second.kind != lir::Operand::OP_SLOT ||
       inst.first.kind != lir::Operand::OP_TEMP) {
      return false;
    }
    map<string, string>::const_iterator promoted =
        layout.promoted_param_slots.find(inst.second.text);
    return promoted != layout.promoted_param_slots.end() &&
           inst.first.text == promoted->second;
  }

  void emit_sign_mask_from_register(X64Register src,
                                    X64Register dst,
                                    vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_CMP);
    inst.type = "i64";
    inst.operands.push_back(reg(src));
    inst.operands.push_back(imm(0));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_SETCC);
    inst.condition = XC_L;
    inst.operands.push_back(reg(dst));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_MOVZX);
    inst.operands.push_back(reg(dst));
    inst.operands.push_back(reg(dst));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_NEG);
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_load_i128_from_address_register(X64Register address,
                                            X64Register lo,
                                            X64Register hi,
                                            vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
    inst.type = "i64";
    inst.operands.push_back(reg(lo));
    inst.operands.push_back(deref_offset(address, 0));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_LOAD);
    inst.type = "i64";
    inst.operands.push_back(reg(hi));
    inst.operands.push_back(deref_offset(address, 8));
    out.push_back(inst);
  }

  void emit_store_i128_to_address_register(X64Register address,
                                           X64Register lo,
                                           X64Register hi,
                                           vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
    inst.type = "i64";
    inst.operands.push_back(deref_offset(address, 0));
    inst.operands.push_back(reg(lo));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_STORE);
    inst.type = "i64";
    inst.operands.push_back(deref_offset(address, 8));
    inst.operands.push_back(reg(hi));
    out.push_back(inst);
  }

  void emit_load_i128_literal(const lir::Operand & operand,
                              const string & desired_type,
                              X64Register lo,
                              X64Register hi,
                              vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_MOV);
    inst.operands.push_back(reg(lo));
    inst.operands.push_back(imm(operand.int_value));
    out.push_back(inst);

    const bool negative_text = !operand.text.empty() && operand.text[0] == '-';
    const bool sign_extend =
        negative_text ||
        (desired_type == "i128" && operand.int_value < 0);
    inst = make_instruction(mir::Instruction::MI_MOV);
    inst.operands.push_back(reg(hi));
    inst.operands.push_back(imm(sign_extend ? -1 : 0));
    out.push_back(inst);
  }

  void emit_load_i128_value(const FunctionLayout & layout,
                            const lir::Operand & operand,
                            const string & desired_type,
                            X64Register lo,
                            X64Register hi,
                            vector<mir::Instruction> & out) const
  {
    switch(operand.kind) {
      case lir::Operand::OP_INTEGER:
        emit_load_i128_literal(operand, desired_type, lo, hi, out);
        return;
      case lir::Operand::OP_SLOT:
      case lir::Operand::OP_TEMP:
      case lir::Operand::OP_GLOBAL: {
        const string source_type = operand_type(layout, operand);
        if(is_i128_scalar_type(source_type)) {
          emit_load_address(layout, operand, XR_R11, out);
          emit_load_i128_from_address_register(XR_R11, lo, hi, out);
          return;
        }
        emit_load_value(layout, operand, source_type, lo, out);
        if(source_type == "ptr" || source_type == "i1" ||
           source_type == "u8" || source_type == "u16" ||
           source_type == "u32") {
          emit_zero_register(hi, out);
        } else {
          const size_t source_size = lir::type_size(lir::LowType{source_type});
          if(source_size < 8) {
            mir::Instruction sext = make_instruction(mir::Instruction::MI_SEXT);
            sext.byte_count = source_size;
            sext.operands.push_back(reg(lo));
            out.push_back(sext);
          }
          emit_sign_mask_from_register(lo, hi, out);
        }
        return;
      }
      default:
        break;
    }
    throw logic_error("invalid i128 operand kind for emit_load_i128_value");
  }

  void emit_store_i128_temp(const FunctionLayout & layout,
                            const string & dest,
                            X64Register lo,
                            X64Register hi,
                            vector<mir::Instruction> & out) const
  {
    if(temp_register_for(layout, dest) != nullptr) {
      throw logic_error("i128 temp unexpectedly allocated to a register " + dest);
    }
    map<string, string>::const_iterator found = layout.storage_type.find(dest);
    if(found == layout.storage_type.end()) {
      throw logic_error(unknown_storage_error(layout, dest));
    }
    if(!is_i128_scalar_type(found->second)) {
      throw logic_error("non-i128 storage passed to emit_store_i128_temp " + dest);
    }
    lir::Operand storage;
    storage.kind = lir::Operand::OP_TEMP;
    storage.text = dest;
    emit_load_address(layout, storage, XR_R11, out);
    emit_store_i128_to_address_register(XR_R11, lo, hi, out);
  }

  vector<lir::Parameter> call_signature_params(const lir::Instruction & inst) const
  {
    if(inst.has_call_signature) {
      return inst.call_params;
    }
    if(inst.first.kind != lir::Operand::OP_GLOBAL) {
      return vector<lir::Parameter>();
    }
    map<string, vector<lir::Parameter> >::const_iterator found =
        function_params_.find(inst.first.text);
    return found == function_params_.end() ? vector<lir::Parameter>() : found->second;
  }

  lir::FunctionBoundaryMetadata resolved_call_boundary(const lir::Instruction & inst) const
  {
    lir::FunctionBoundaryMetadata boundary;
    if(inst.kind != lir::Instruction::IK_CALL) {
      return boundary;
    }
    if(inst.first.kind == lir::Operand::OP_GLOBAL) {
      map<string, lir::FunctionBoundaryMetadata>::const_iterator found =
          function_boundaries_.find(inst.first.text);
      if(found != function_boundaries_.end()) {
        merge_boundary_metadata(boundary, found->second);
      }
    }
    if(inst.has_call_signature) {
      merge_boundary_metadata(boundary, inst.call_boundary);
    }
    return boundary;
  }

  bool function_needs_host_eh_enabled(const lir::Function & function) const
  {
    if(function_contains_eh_regions(function, function_roles_)) {
      return true;
    }
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const lir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.kind != lir::Instruction::IK_CALL) {
          continue;
        }
        const lir::FunctionBoundaryMetadata boundary = resolved_call_boundary(inst);
        if(boundary.unwind != lir::CUM_NO) {
          return true;
        }
      }
    }
    return false;
  }

  void emit_load_object_chunk(const FunctionLayout & layout,
                              const lir::Operand & operand,
                              const string & chunk_type,
                              size_t chunk_offset,
                              X64Register dst,
                              vector<mir::Instruction> & out) const
  {
    if(operand.kind == lir::Operand::OP_INTEGER) {
      if(chunk_offset != 0 && chunk_offset != 8) {
        throw logic_error("invalid integer ABI chunk offset");
      }
      const bool negative_text = !operand.text.empty() && operand.text[0] == '-';
      mir::Instruction inst = make_instruction(mir::Instruction::MI_MOV);
      inst.operands.push_back(reg(dst));
      inst.operands.push_back(
          imm(chunk_offset == 0 ? operand.int_value : (negative_text ? -1 : 0)));
      out.push_back(inst);
      return;
    }
    emit_load_address(layout, operand, XR_R11, out);
    mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
    inst.type = chunk_type;
    inst.operands.push_back(reg(dst));
    inst.operands.push_back(deref_offset(XR_R11, static_cast<long long>(chunk_offset)));
    out.push_back(inst);
  }

  void emit_load_object_return_chunks(const FunctionLayout & layout,
                                      const lir::Operand & operand,
                                      const string & object_type,
                                      vector<mir::Instruction> & out) const
  {
    const vector<string> chunk_types = scalar_abi_chunk_types(object_type);
    if(chunk_types.empty()) {
      throw logic_error("direct object return requires chunk types");
    }
    size_t chunk_offset = 0;
    for(size_t i = 0; i < chunk_types.size(); ++i) {
      emit_load_object_chunk(layout,
                             operand,
                             chunk_types[i],
                             chunk_offset,
                             integer_return_register(i),
                             out);
      chunk_offset += lir::type_size(lir::LowType{chunk_types[i]});
    }
  }

  void emit_store_object_return_temp(const FunctionLayout & layout,
                                     const string & dest,
                                     const string & object_type,
                                     vector<mir::Instruction> & out) const
  {
    const vector<string> chunk_types = scalar_abi_chunk_types(object_type);
    if(chunk_types.empty()) {
      throw logic_error("direct object call result requires chunk types");
    }
    lir::Operand storage;
    map<string, string>::const_iterator aliased =
        layout.aliased_object_return_slots.find(dest);
    if(aliased != layout.aliased_object_return_slots.end()) {
      storage.kind = lir::Operand::OP_SLOT;
      storage.text = aliased->second;
    } else {
      storage.kind = lir::Operand::OP_TEMP;
      storage.text = dest;
    }
    emit_load_address(layout, storage, XR_R11, out);
    size_t chunk_offset = 0;
    for(size_t i = 0; i < chunk_types.size(); ++i) {
      mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
      inst.type = chunk_types[i];
      inst.operands.push_back(deref_offset(XR_R11, static_cast<long long>(chunk_offset)));
      inst.operands.push_back(reg(integer_return_register(i)));
      out.push_back(inst);
      chunk_offset += lir::type_size(lir::LowType{chunk_types[i]});
    }
  }

  void emit_load_storage_address_value(const FunctionLayout & layout,
                                       const lir::Operand & operand,
                                       X64Register dst,
                                       vector<mir::Instruction> & out) const
  {
    if(operand.kind == lir::Operand::OP_SLOT || operand.kind == lir::Operand::OP_GLOBAL) {
      emit_load_address(layout, operand, dst, out);
      return;
    }
    if(operand.kind == lir::Operand::OP_TEMP) {
      const string source_type = operand_type(layout, operand);
      if(source_type == "ptr") {
        emit_load_value(layout, operand, "ptr", dst, out);
      } else {
        emit_load_address(layout, operand, dst, out);
      }
      return;
    }
    throw logic_error("invalid storage address operand " + operand.text);
  }

  bool try_emit_load_global_pointer_call_target(const FunctionLayout & layout,
                                                const lir::Operand & operand,
                                                X64Register dst,
                                                vector<mir::Instruction> & out) const
  {
    if(operand.kind != lir::Operand::OP_TEMP) {
      return false;
    }

    map<string, lir::Instruction>::const_iterator found =
        layout.temp_def_instruction.find(operand.text);
    if(found == layout.temp_def_instruction.end() ||
       found->second.kind != lir::Instruction::IK_ADDR ||
       found->second.first.kind != lir::Operand::OP_GLOBAL) {
      return false;
    }

    const string & global_name = found->second.first.text;
    map<string, string>::const_iterator global_type =
        scalar_global_types_.find(global_name);
    if(global_type == scalar_global_types_.end() || global_type->second != "ptr") {
      return false;
    }

    mir::Instruction inst = make_instruction(mir::Instruction::MI_LOAD);
    inst.type = "ptr";
    inst.operands.push_back(reg(dst));
    if(is_thread_local_global_name(global_name)) {
      emit_load_address(layout, found->second.first, XR_R11, out);
      inst.operands.push_back(deref(XR_R11));
    } else {
      inst.operands.push_back(global_ref(global_name));
    }
    out.push_back(inst);
    return true;
  }

  void emit_store_temp(const FunctionLayout & layout,
                       const string & dest,
                       X64Register src,
                       vector<mir::Instruction> & out) const
  {
    if(rematerialized_slot_address_def(layout, dest) != nullptr) {
      return;
    }
    if(const X64Register * assigned = temp_register_for(layout, dest)) {
      map<string, string>::const_iterator found = layout.storage_type.find(dest);
      if(found == layout.storage_type.end()) {
        throw logic_error(unknown_storage_error(layout, dest));
      }
      if(*assigned != src) {
        mir::Instruction move = make_instruction(mir::Instruction::MI_MOV);
        move.operands.push_back(reg(*assigned));
        move.operands.push_back(reg(src));
        out.push_back(move);
      }
      emit_normalize_integer_temp(found->second, *assigned, out);
      return;
    }
    map<string, string>::const_iterator found = layout.storage_type.find(dest);
    if(found == layout.storage_type.end()) {
      throw logic_error(unknown_storage_error(layout, dest));
    }
    if(is_i128_scalar_type(found->second)) {
      throw logic_error("i128 storage passed to scalar temp store " + dest);
    }
    mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
    inst.type = found->second;
    inst.operands.push_back(frame(layout, dest));
    inst.operands.push_back(reg(src));
    out.push_back(inst);
  }

  mir::Operand float_source_operand(const FunctionLayout & layout,
                                    const lir::Operand & operand) const
  {
    switch(operand.kind) {
      case lir::Operand::OP_SLOT:
        return frame(layout, operand.text);
      case lir::Operand::OP_TEMP:
        if(const XmmRegister * assigned = float_temp_register_for(layout, operand.text)) {
          return xmm(*assigned);
        }
        return frame(layout, operand.text);
      case lir::Operand::OP_GLOBAL:
        return global_ref(operand.text);
      case lir::Operand::OP_FLOAT:
      case lir::Operand::OP_INTEGER:
        return float_imm(operand);
      default:
        throw logic_error("invalid floating source operand");
    }
  }

mir::Operand integer_source_operand(const FunctionLayout & layout,
                                    const lir::Operand & operand) const
{
    switch(operand.kind) {
      case lir::Operand::OP_SLOT: {
        map<string, string>::const_iterator promoted =
            layout.promoted_param_slots.find(operand.text);
        if(promoted != layout.promoted_param_slots.end()) {
          lir::Operand forwarded;
          forwarded.kind = lir::Operand::OP_TEMP;
          forwarded.text = promoted->second;
          return integer_source_operand(layout, forwarded);
        }
        return frame(layout, operand.text);
      }
      case lir::Operand::OP_TEMP:
        if(const X64Register * assigned = temp_register_for(layout, operand.text)) {
          return reg(*assigned);
        }
        return frame(layout, operand.text);
      case lir::Operand::OP_GLOBAL:
        return global_ref(operand.text);
      case lir::Operand::OP_INTEGER:
        return imm(operand.int_value);
      default:
        throw logic_error("invalid integer source operand");
    }
  }

  mir::Operand integer_dest_operand(const FunctionLayout & layout,
                                    const string & dest) const
  {
    if(const X64Register * assigned = temp_register_for(layout, dest)) {
      return reg(*assigned);
    }
    return frame(layout, dest);
  }

  mir::Operand storage_operand(const FunctionLayout & layout,
                               const lir::Operand & operand) const
  {
    if(operand.kind == lir::Operand::OP_SLOT) {
      return frame(layout, operand.text);
    }
    if(operand.kind == lir::Operand::OP_TEMP) {
      if(const XmmRegister * assigned = float_temp_register_for(layout, operand.text)) {
        return xmm(*assigned);
      }
      return frame(layout, operand.text);
    }
    if(operand.kind == lir::Operand::OP_GLOBAL) {
      return global_ref(operand.text);
    }
    throw logic_error("invalid storage operand " + operand.text);
  }

  void emit_store_temp_float(const FunctionLayout & layout,
                             const string & dest,
                             const lir::Operand & src,
                             vector<mir::Instruction> & out) const
  {
    const string dest_type = layout.storage_type.find(dest)->second;
    const string source_type = operand_type(layout, src);
    mir::Operand dst;
    if(const XmmRegister * assigned = float_temp_register_for(layout, dest)) {
      dst = xmm(*assigned);
    } else {
      dst = frame(layout, dest);
    }
    emit_float_move_or_convert(dst,
                               dest_type,
                               source_type,
                               float_source_operand(layout, src),
                               out);
  }

  void emit_store_temp_float(const FunctionLayout & layout,
                             const string & dest,
                             const mir::Operand & src,
                             vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_FMOV);
    inst.type = layout.storage_type.find(dest)->second;
    if(const XmmRegister * assigned = float_temp_register_for(layout, dest)) {
      inst.operands.push_back(xmm(*assigned));
    } else {
      inst.operands.push_back(frame(layout, dest));
    }
    inst.operands.push_back(src);
    out.push_back(inst);
  }

  void emit_store_storage(const FunctionLayout & layout,
                          const lir::Operand & operand,
                          const string & type,
                          X64Register src,
                          vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_STORE);
    inst.type = type;
    if(operand.kind == lir::Operand::OP_SLOT || operand.kind == lir::Operand::OP_TEMP) {
      inst.operands.push_back(frame(layout, operand.text));
      inst.operands.push_back(reg(src));
    } else if(operand.kind == lir::Operand::OP_GLOBAL) {
      if(is_thread_local_global_name(operand.text)) {
        X64Register preserved_src = src;
        bool reload_from_spill = false;
        if(is_call_clobbered_register(src)) {
          X64Register spare = XR_RAX;
          if(!find_spare_callee_saved_register(layout, spare)) {
            if(!layout.has_preserve_spill) {
              throw logic_error("missing preserve spill slot for thread_local store");
            }
            mir::Instruction preserve = make_instruction(mir::Instruction::MI_STORE);
            preserve.type = "i64";
            preserve.operands.push_back(deref_offset(XR_RBP,
                                                     preserve_spill_slot_offset(layout, 0)));
            preserve.operands.push_back(reg(src));
            out.push_back(preserve);
            preserved_src = XR_RAX;
            reload_from_spill = true;
          } else {
            mir::Instruction preserve = make_instruction(mir::Instruction::MI_MOV);
            preserve.operands.push_back(reg(spare));
            preserve.operands.push_back(reg(src));
            out.push_back(preserve);
            preserved_src = spare;
          }
        }
        emit_load_address(layout, operand, XR_R11, out);
        if(reload_from_spill) {
          mir::Instruction reload = make_instruction(mir::Instruction::MI_LOAD);
          reload.type = "i64";
          reload.operands.push_back(reg(preserved_src));
          reload.operands.push_back(deref_offset(XR_RBP,
                                                 preserve_spill_slot_offset(layout, 0)));
          out.push_back(reload);
        }
        inst.operands.push_back(deref(XR_R11));
        inst.operands.push_back(reg(preserved_src));
      } else {
        inst.operands.push_back(global_ref(operand.text));
        inst.operands.push_back(reg(src));
      }
    } else {
      throw logic_error("invalid storage operand " + operand.text);
    }
    out.push_back(inst);
  }

  PreservedIntegerValue preserve_integer_value_if_needed(
      const FunctionLayout & layout,
      const lir::Operand & operand,
      X64Register original,
      const set<X64Register> & blocked,
      const set<X64Register> & reserved,
      size_t spill_index,
      vector<mir::Instruction> & out) const
  {
    PreservedIntegerValue result;
    if(operand.kind != lir::Operand::OP_TEMP) {
      result.reg = original;
      return result;
    }
    const X64Register * assigned = temp_register_for(layout, operand.text);
    if(assigned == nullptr || *assigned != original) {
      result.reg = original;
      return result;
    }
    const vector<X64Register> & caller_saved = call_setup_preserve_registers();
    for(size_t i = 0; i < caller_saved.size(); ++i) {
      const X64Register candidate = caller_saved[i];
      if(candidate == original ||
         blocked.count(candidate) != 0 ||
         reserved.count(candidate) != 0) {
        continue;
      }
      bool reserved = false;
      for(map<string, X64Register>::const_iterator it = layout.temp_register.begin();
          it != layout.temp_register.end();
          ++it) {
        if(it->second == candidate) {
          reserved = true;
          break;
        }
      }
      if(reserved) {
        continue;
      }
      mir::Instruction move = make_instruction(mir::Instruction::MI_MOV);
      move.operands.push_back(reg(candidate));
      move.operands.push_back(reg(original));
      out.push_back(move);
      result.reg = candidate;
      return result;
    }
    X64Register spare = XR_RAX;
    if(find_spare_callee_saved_register(layout, spare) &&
       blocked.count(spare) == 0 &&
       reserved.count(spare) == 0) {
      mir::Instruction move = make_instruction(mir::Instruction::MI_MOV);
      move.operands.push_back(reg(spare));
      move.operands.push_back(reg(original));
      out.push_back(move);
      result.reg = spare;
      return result;
    }
    if(!layout.has_preserve_spill) {
      throw logic_error("missing preserve spill slot for forwarded parameter");
    }
    mir::Instruction store = make_instruction(mir::Instruction::MI_STORE);
    store.type = "i64";
    store.operands.push_back(deref_offset(XR_RBP,
                                          preserve_spill_slot_offset(layout, spill_index)));
    store.operands.push_back(reg(original));
    out.push_back(store);
    result.spilled = true;
    result.spill_index = spill_index;
    return result;
  }

  bool can_preserve_value_without_spill(const FunctionLayout & layout,
                                        X64Register original,
                                        const set<X64Register> & blocked,
                                        const set<X64Register> & reserved) const
  {
    const vector<X64Register> & caller_saved = call_setup_preserve_registers();
    for(size_t i = 0; i < caller_saved.size(); ++i) {
      const X64Register candidate = caller_saved[i];
      if(candidate == original ||
         blocked.count(candidate) != 0 ||
         reserved.count(candidate) != 0) {
        continue;
      }
      bool reserved = false;
      for(map<string, X64Register>::const_iterator it = layout.temp_register.begin();
          it != layout.temp_register.end();
          ++it) {
        if(it->second == candidate) {
          reserved = true;
          break;
        }
      }
      if(!reserved) {
        return true;
      }
    }
    X64Register spare = XR_RAX;
    return find_spare_callee_saved_register(layout, spare) &&
           blocked.count(spare) == 0 &&
           reserved.count(spare) == 0;
  }

  size_t preserve_spill_slots_required(const lir::Function & function,
                                       const FunctionLayout & layout) const
  {
    size_t required = 0;
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const lir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.kind == lir::Instruction::IK_STORE &&
           inst.second.kind == lir::Operand::OP_GLOBAL &&
           is_thread_local_global_name(inst.second.text) &&
           !is_float_type(inst.type) &&
           !is_i128_scalar_type(inst.type.text)) {
          X64Register spare = XR_RAX;
          if(!find_spare_callee_saved_register(layout, spare)) {
            required = max(required, static_cast<size_t>(1));
          }
        }

        if(inst.kind == lir::Instruction::IK_COPYOBJ &&
           inst.first.kind == lir::Operand::OP_TEMP) {
          const X64Register * assigned = temp_register_for(layout, inst.first.text);
          if(assigned != nullptr && *assigned == XR_RDI) {
            set<X64Register> blocked;
            blocked.insert(XR_RDI);
            blocked.insert(XR_RSI);
            set<X64Register> reserved;
            if(!can_preserve_value_without_spill(layout, XR_RDI, blocked, reserved)) {
              required = max(required, static_cast<size_t>(1));
            }
          }
        }

        if(inst.kind != lir::Instruction::IK_CALL) {
          continue;
        }

        const vector<lir::Parameter> call_params = call_signature_params(inst);
        struct CallArgPiece
        {
          lir::Operand operand;
          string type;
          bool memory_object = false;
          bool direct_call_index = false;
          lir::Operand direct_call_index_base;
          long long direct_call_index_offset = 0;
          lir::ParamPassingMode passing = lir::PPM_DIRECT;
        };

        vector<CallArgPiece> pieces;
        for(size_t i = 0; i < inst.args.size(); ++i) {
          const string param_type =
              i < call_params.size() ? call_params[i].type.text : operand_type(layout, inst.args[i]);
          if(is_memory_class_object_abi_type(param_type)) {
            CallArgPiece piece;
            piece.operand = inst.args[i];
            piece.type = param_type;
            piece.memory_object = true;
            pieces.push_back(piece);
            continue;
          }
          const vector<string> chunk_types = scalar_abi_chunk_types(param_type);
          if(!chunk_types.empty()) {
            for(size_t chunk_index = 0; chunk_index < chunk_types.size(); ++chunk_index) {
              CallArgPiece piece;
              piece.operand = inst.args[i];
              piece.type = chunk_types[chunk_index];
              if(piece.operand.kind == lir::Operand::OP_TEMP &&
                 layout.direct_call_arg_index_temps.count(piece.operand.text) != 0) {
                piece.direct_call_index =
                    direct_call_arg_index_info(layout,
                                               piece.operand.text,
                                               piece.direct_call_index_base,
                                               piece.direct_call_index_offset);
              }
              piece.passing =
                  i < call_params.size() ? call_params[i].metadata.passing : lir::PPM_DIRECT;
              pieces.push_back(piece);
            }
            continue;
          }
          CallArgPiece piece;
          piece.operand = inst.args[i];
          piece.type = param_type;
          if(piece.operand.kind == lir::Operand::OP_TEMP &&
             layout.direct_call_arg_index_temps.count(piece.operand.text) != 0) {
            piece.direct_call_index =
                direct_call_arg_index_info(layout,
                                           piece.operand.text,
                                           piece.direct_call_index_base,
                                           piece.direct_call_index_offset);
          }
          piece.passing =
              i < call_params.size() ? call_params[i].metadata.passing : lir::PPM_DIRECT;
          pieces.push_back(piece);
        }

        vector<bool> arg_in_reg(pieces.size(), false);
        vector<size_t> arg_reg_index(pieces.size(), 0);
        size_t next_reg = 0;
        size_t next_xmm = 0;
        for(size_t i = 0; i < pieces.size(); ++i) {
          const string & arg_type = pieces[i].type;
          if(pieces[i].memory_object) {
            continue;
          } else if((arg_type == "f32" || arg_type == "f64") && next_xmm < 8) {
            ++next_xmm;
          } else if(!is_float_type(arg_type) && next_reg < 6) {
            arg_in_reg[i] = true;
            arg_reg_index[i] = next_reg++;
          }
        }

        set<X64Register> blocked_arg_regs;
        const bool direct_symbol_call =
            inst.first.kind == lir::Operand::OP_GLOBAL &&
            function_names_.count(inst.first.text) != 0;
        for(size_t i = 0; i < pieces.size(); ++i) {
          if(arg_in_reg[i]) {
            blocked_arg_regs.insert(arg_register(arg_reg_index[i]));
          }
        }
        if(!direct_symbol_call) {
          blocked_arg_regs.insert(XR_R10);
        }

        size_t spills_this_call = 0;
        set<X64Register> reserved_preserve_regs;
        for(size_t i = 0; i < pieces.size(); ++i) {
          const lir::Operand preserve_operand =
              pieces[i].direct_call_index ? pieces[i].direct_call_index_base :
                                            pieces[i].operand;
          if(preserve_operand.kind != lir::Operand::OP_TEMP) {
            continue;
          }
          const X64Register * assigned = temp_register_for(layout, preserve_operand.text);
          if(assigned == nullptr || blocked_arg_regs.count(*assigned) == 0) {
            continue;
          }
          const bool clobbered_before_materialization =
              call_source_reg_clobbered_before_call_piece(arg_in_reg,
                                                          arg_reg_index,
                                                          i,
                                                          *assigned,
                                                          direct_symbol_call,
                                                          XR_R10);
          bool needs_preserve = false;
          if(arg_in_reg[i]) {
            needs_preserve =
                *assigned != arg_register(arg_reg_index[i]) ||
                clobbered_before_materialization;
          } else {
            needs_preserve = clobbered_before_materialization;
          }
          if(!needs_preserve) {
            continue;
          }
          if(!can_preserve_value_without_spill(layout,
                                              *assigned,
                                              blocked_arg_regs,
                                              reserved_preserve_regs)) {
            ++spills_this_call;
          } else {
            const vector<X64Register> & caller_saved = call_setup_preserve_registers();
            bool reserved_candidate = false;
            for(size_t ri = 0; ri < caller_saved.size(); ++ri) {
              const X64Register candidate = caller_saved[ri];
              if(candidate == *assigned ||
                 blocked_arg_regs.count(candidate) != 0 ||
                 reserved_preserve_regs.count(candidate) != 0) {
                continue;
              }
              bool layout_reserved = false;
              for(map<string, X64Register>::const_iterator it = layout.temp_register.begin();
                  it != layout.temp_register.end();
                  ++it) {
                if(it->second == candidate) {
                  layout_reserved = true;
                  break;
                }
              }
              if(!layout_reserved) {
                reserved_preserve_regs.insert(candidate);
                reserved_candidate = true;
                break;
              }
            }
            if(!reserved_candidate) {
              X64Register spare = XR_RAX;
              if(find_spare_callee_saved_register(layout, spare) &&
                 blocked_arg_regs.count(spare) == 0 &&
                 reserved_preserve_regs.count(spare) == 0) {
                reserved_preserve_regs.insert(spare);
              }
            }
          }
        }
        required = max(required, spills_this_call);
      }
    }
    return required;
  }

  void ensure_preserve_spill_slot(const lir::Function & function,
                                  FunctionLayout & layout) const
  {
    const size_t spill_count =
        max(layout.forwarded_params.empty() ? static_cast<size_t>(0) : static_cast<size_t>(1),
            preserve_spill_slots_required(function, layout));
    if(layout.has_preserve_spill || spill_count == 0) {
      return;
    }
    layout.frame_bytes = (layout.frame_bytes + 7) & ~static_cast<size_t>(7);
    layout.preserve_spill_offset = layout.frame_bytes + 8;
    layout.preserve_spill_count = spill_count;
    layout.frame_bytes += 8 * spill_count;
    layout.has_preserve_spill = true;
  }

  void emit_store_storage_float(const FunctionLayout & layout,
                                const mir::Operand & dst,
                                const string & type,
                                const mir::Operand & src,
                                vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_FMOV);
    inst.type = type;
    inst.operands.push_back(dst);
    inst.operands.push_back(src);
    out.push_back(inst);
  }

  void emit_float_move_or_convert(const mir::Operand & dst,
                                  const string & dest_type,
                                  const string & source_type,
                                  const mir::Operand & src,
                                  vector<mir::Instruction> & out) const
  {
    mir::Instruction inst;
    if(is_float_type(source_type) &&
       source_type != dest_type &&
       float_exec_width_bytes(source_type) != float_exec_width_bytes(dest_type)) {
      inst = make_instruction(float_exec_width_bytes(source_type) <
                                      float_exec_width_bytes(dest_type) ?
                                  mir::Instruction::MI_FPEXT :
                                  mir::Instruction::MI_FPTRUNC);
      inst.type = dest_type;
      inst.byte_count = float_exec_width_bytes(source_type);
      inst.operands.push_back(dst);
      inst.operands.push_back(src);
      out.push_back(inst);
      return;
    }

    inst = make_instruction(mir::Instruction::MI_FMOV);
    inst.type = dest_type;
    inst.operands.push_back(dst);
    inst.operands.push_back(src);
    out.push_back(inst);
  }

  void emit_setcc_bool(X86Condition condition,
                       X64Register dst,
                       vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_SETCC);
    inst.condition = condition;
    inst.operands.push_back(reg(dst));
    out.push_back(inst);

    inst = make_instruction(mir::Instruction::MI_MOVZX);
    inst.operands.push_back(reg(dst));
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_setcc_byte(X86Condition condition,
                       X64Register dst,
                       vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_SETCC);
    inst.condition = condition;
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_zero_extend_setcc_byte(X64Register dst,
                                   vector<mir::Instruction> & out) const
  {
    mir::Instruction inst = make_instruction(mir::Instruction::MI_MOVZX);
    inst.operands.push_back(reg(dst));
    inst.operands.push_back(reg(dst));
    out.push_back(inst);
  }

  void emit_compare_i128(const FunctionLayout & layout,
                         const lir::Instruction & inst,
                         vector<mir::Instruction> & out) const
  {
    const bool unsigned_predicate =
        inst.op == "ult" || inst.op == "ugt" || inst.op == "ule" || inst.op == "uge";
    const bool is_unsigned = inst.type.text == "u128" || unsigned_predicate;
    const X86Condition high_lt = is_unsigned ? XC_B : XC_L;
    const X86Condition high_gt = is_unsigned ? XC_A : XC_G;
    const X86Condition low_lt = XC_B;
    const X86Condition low_gt = XC_A;
    const X86Condition low_le = XC_BE;
    const X86Condition low_ge = XC_AE;

    emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
    emit_load_i128_value(layout, inst.second, inst.type.text, XR_RCX, XR_RSI, out);

    mir::Instruction mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = "i64";
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(reg(XR_RSI));
    out.push_back(mi);

    if(inst.op == "eq" || inst.op == "ne") {
      emit_setcc_bool(inst.op == "eq" ? XC_E : XC_NE, XR_R10, out);
      mi = make_instruction(mir::Instruction::MI_CMP);
      mi.type = "i64";
      mi.operands.push_back(reg(XR_RAX));
      mi.operands.push_back(reg(XR_RCX));
      out.push_back(mi);
      emit_setcc_bool(inst.op == "eq" ? XC_E : XC_NE, XR_R11, out);
      mi = make_instruction(inst.op == "eq" ? mir::Instruction::MI_AND :
                                               mir::Instruction::MI_OR);
      mi.operands.push_back(reg(XR_R10));
      mi.operands.push_back(reg(XR_R11));
      out.push_back(mi);
      emit_store_temp(layout, inst.dest, XR_R10, out);
      return;
    }

    const bool greater_compare = inst.op == "gt" || inst.op == "ugt" ||
        inst.op == "ge" || inst.op == "uge";
    const X86Condition high_relation = greater_compare ? high_gt : high_lt;
    emit_setcc_byte(high_relation, XR_R10, out);
    emit_setcc_byte(XC_E, XR_R11, out);
    emit_zero_extend_setcc_byte(XR_R10, out);
    emit_zero_extend_setcc_byte(XR_R11, out);

    mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = "i64";
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(reg(XR_RCX));
    out.push_back(mi);

    X86Condition low_relation = low_lt;
    if(inst.op == "lt" || inst.op == "ult") {
      low_relation = low_lt;
    } else if(inst.op == "gt" || inst.op == "ugt") {
      low_relation = low_gt;
    } else if(inst.op == "le" || inst.op == "ule") {
      low_relation = low_le;
    } else if(inst.op == "ge" || inst.op == "uge") {
      low_relation = low_ge;
    } else {
      throw logic_error("unsupported i128 cmp predicate " + inst.op);
    }

    emit_setcc_bool(low_relation, XR_RAX, out);
    mi = make_instruction(mir::Instruction::MI_AND);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(reg(XR_R11));
    out.push_back(mi);
    mi = make_instruction(mir::Instruction::MI_OR);
    mi.operands.push_back(reg(XR_R10));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);
    emit_store_temp(layout, inst.dest, XR_R10, out);
  }

  bool direct_call_arg_index_info(const FunctionLayout & layout,
                                  const string & temp_name,
                                  lir::Operand & base,
                                  long long & scaled_offset) const
  {
    map<string, lir::Instruction>::const_iterator found =
        layout.temp_def_instruction.find(temp_name);
    if(found == layout.temp_def_instruction.end() ||
       found->second.kind != lir::Instruction::IK_INDEX ||
       found->second.second.kind != lir::Operand::OP_INTEGER) {
      return false;
    }

    const lir::Instruction & inst = found->second;
    const size_t scale = lir::type_size(lir::LowType{inst.op});
    if(__builtin_mul_overflow(inst.second.int_value,
                              static_cast<long long>(scale),
                              &scaled_offset)) {
      return false;
    }
    base = inst.first;
    return true;
  }

  void emit_adjusted_pointer_from_register(X64Register base,
                                           long long scaled_offset,
                                           X64Register dst,
                                           vector<mir::Instruction> & out) const
  {
    if(dst != base) {
      mir::Instruction move = make_instruction(mir::Instruction::MI_MOV);
      move.operands.push_back(reg(dst));
      move.operands.push_back(reg(base));
      out.push_back(move);
    }
    if(scaled_offset != 0) {
      mir::Instruction inst = make_instruction(mir::Instruction::MI_LEA);
      inst.operands.push_back(reg(dst));
      inst.operands.push_back(deref_offset(dst, scaled_offset));
      out.push_back(inst);
    }
  }

  void emit_direct_call_arg_index_temp(const FunctionLayout & layout,
                                       const string & temp_name,
                                       X64Register dst,
                                       vector<mir::Instruction> & out) const
  {
    lir::Operand base;
    long long scaled_offset = 0;
    if(!direct_call_arg_index_info(layout, temp_name, base, scaled_offset)) {
      throw logic_error("invalid direct call index temp " + temp_name);
    }

    emit_load_value(layout, base, "ptr", dst, out);
    emit_adjusted_pointer_from_register(dst, scaled_offset, dst, out);
  }

  void emit_call_i128_binary_helper(const FunctionLayout & layout,
                                    const string & helper,
                                    const lir::Instruction & inst,
                                    vector<mir::Instruction> & out) const
  {
    emit_load_i128_value(layout, inst.first, inst.type.text, XR_RDI, XR_RSI, out);
    emit_load_i128_value(layout, inst.second, inst.type.text, XR_RDX, XR_RCX, out);

    mir::Instruction mi = make_instruction(mir::Instruction::MI_CALL);
    mi.operands.push_back(symbol(helper));
    out.push_back(mi);

    emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
  }

  void emit_call_i128_shift_helper(const FunctionLayout & layout,
                                   const string & helper,
                                   const lir::Instruction & inst,
                                   vector<mir::Instruction> & out) const
  {
    emit_load_i128_value(layout, inst.first, inst.type.text, XR_RDI, XR_RSI, out);
    emit_load_value(layout, inst.second, "i32", XR_RDX, out);

    mir::Instruction mi = make_instruction(mir::Instruction::MI_CALL);
    mi.operands.push_back(symbol(helper));
    out.push_back(mi);

    emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
  }

  void emit_va_start(const FunctionLayout & layout,
                     const lir::Instruction & inst,
                     vector<mir::Instruction> & out) const
  {
    if(!layout.variadic || layout.va_reg_save_area_offset == 0) {
      throw logic_error("va_start requires variadic function layout");
    }

    emit_load_value(layout, inst.first, "ptr", XR_R11, out);

    mir::Instruction mi = make_instruction(mir::Instruction::MI_MOV);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(imm(layout.va_gp_offset));
    out.push_back(mi);
    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "i32";
    mi.operands.push_back(deref_offset(XR_R11, 0));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_MOV);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(imm(layout.va_fp_offset));
    out.push_back(mi);
    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "i32";
    mi.operands.push_back(deref_offset(XR_R11, 4));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LEA);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(deref_offset(XR_RBP, layout.va_overflow_stack_offset));
    out.push_back(mi);
    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "i64";
    mi.operands.push_back(deref_offset(XR_R11, 8));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LEA);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(deref_offset(
        XR_RBP,
        -static_cast<long long>(layout.va_reg_save_area_offset)));
    out.push_back(mi);
    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "i64";
    mi.operands.push_back(deref_offset(XR_R11, 16));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);
  }

  void emit_va_arg_gp(const FunctionLayout & layout,
                      const lir::Instruction & inst,
                      vector<mir::Instruction> & out) const
  {
    const string & type = inst.type.text;
    if(type == "void" || is_float_type(type) || is_i128_scalar_type(type) ||
       is_object_type(type) || lir::type_size(inst.type) > 8) {
      throw logic_error("unsupported __builtin_va_arg type " + type);
    }

    emit_load_value(layout, inst.first, "ptr", XR_R11, out);

    mir::Instruction mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = "u32";
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(deref_offset(XR_R11, 0));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = "ptr";
    mi.operands.push_back(reg(XR_RCX));
    mi.operands.push_back(deref_offset(XR_R11, 16));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_ADD);
    mi.operands.push_back(reg(XR_RCX));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = "ptr";
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(deref_offset(XR_R11, 8));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = "u32";
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(imm(40));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_SETCC);
    mi.condition = XC_BE;
    mi.operands.push_back(reg(XR_R10));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_MOVZX);
    mi.operands.push_back(reg(XR_R10));
    mi.operands.push_back(reg(XR_R10));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_NEG);
    mi.operands.push_back(reg(XR_R10));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_AND);
    mi.operands.push_back(reg(XR_RCX));
    mi.operands.push_back(reg(XR_R10));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_MOV);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(reg(XR_R10));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_NOT);
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_AND);
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_OR);
    mi.operands.push_back(reg(XR_RCX));
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = "u32";
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(deref_offset(XR_R11, 0));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_CMP);
    mi.type = "u32";
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(imm(40));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_SETCC);
    mi.condition = XC_BE;
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_MOVZX);
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    for(size_t i = 0; i < 3; ++i) {
      mi = make_instruction(mir::Instruction::MI_ADD);
      mi.operands.push_back(reg(XR_RDX));
      mi.operands.push_back(reg(XR_RDX));
      out.push_back(mi);
    }

    mi = make_instruction(mir::Instruction::MI_ADD);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "u32";
    mi.operands.push_back(deref_offset(XR_R11, 0));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_MOV);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(imm(8));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_SUB);
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = "ptr";
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(deref_offset(XR_R11, 8));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_ADD);
    mi.operands.push_back(reg(XR_RDX));
    mi.operands.push_back(reg(XR_RAX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_STORE);
    mi.type = "ptr";
    mi.operands.push_back(deref_offset(XR_R11, 8));
    mi.operands.push_back(reg(XR_RDX));
    out.push_back(mi);

    mi = make_instruction(mir::Instruction::MI_LOAD);
    mi.type = type;
    mi.operands.push_back(reg(XR_RAX));
    mi.operands.push_back(deref(XR_RCX));
    out.push_back(mi);

    emit_normalize_integer_temp(type, XR_RAX, out);
    emit_store_temp(layout, inst.dest, XR_RAX, out);
  }

  void emit_instruction(const string & function_name,
                        const FunctionLayout & layout,
                        const lir::Instruction & inst,
                        vector<mir::Instruction> & out) const
  {
    mir::Instruction mi;
    switch(inst.kind) {
      case lir::Instruction::IK_CONST:
        if(is_float_type(inst.type)) {
          emit_store_temp_float(layout, inst.dest, inst.first, out);
        } else if(is_i128_scalar_type(inst.type.text)) {
          emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
          emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
        } else {
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          mi = make_instruction(mir::Instruction::MI_MOV);
          mi.operands.push_back(reg(dst));
          mi.operands.push_back(imm(scalar_literal_bits(inst.first, inst.type.text)));
          out.push_back(mi);
          emit_store_temp(layout, inst.dest, dst, out);
        }
        return;

      case lir::Instruction::IK_COPY:
        if(is_float_type(inst.type)) {
          emit_store_temp_float(layout, inst.dest, inst.first, out);
        } else if(is_i128_scalar_type(inst.type.text)) {
          emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
          emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
        } else {
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          emit_load_value(layout, inst.first, inst.type.text, dst, out);
          emit_store_temp(layout, inst.dest, dst, out);
        }
        return;

      case lir::Instruction::IK_ADDR:
      {
        if(rematerialized_slot_address_def(layout, inst.dest) != nullptr) {
          return;
        }
        const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
        emit_load_address(layout, inst.first, dst, out);
        emit_store_temp(layout, inst.dest, dst, out);
        return;
      }

      case lir::Instruction::IK_LOAD:
        if(layout.elided_direct_branch_load_sources.count(inst.dest) != 0) {
          return;
        }
        if(inst.first.kind == lir::Operand::OP_SLOT &&
           layout.promoted_param_slots.count(inst.first.text) != 0 &&
           !is_float_type(inst.type) &&
           !is_i128_scalar_type(inst.type.text)) {
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          emit_load_value(layout, inst.first, inst.type.text, dst, out);
          emit_store_temp(layout, inst.dest, dst, out);
          return;
        }
        if(inst.first.kind == lir::Operand::OP_TEMP &&
           operand_type(layout, inst.first) == "ptr") {
          const X64Register * direct_base =
              direct_pointer_base_register(layout, inst.first);
          if(is_float_type(inst.type)) {
            if(direct_base != nullptr) {
              emit_store_temp_float(layout, inst.dest, deref(*direct_base), out);
            } else {
              emit_load_value(layout, inst.first, operand_type(layout, inst.first), XR_RCX, out);
              emit_store_temp_float(layout, inst.dest, deref(XR_RCX), out);
            }
          } else if(is_i128_scalar_type(inst.type.text)) {
            if(direct_base != nullptr) {
              emit_load_i128_from_address_register(*direct_base, XR_RAX, XR_RDX, out);
            } else {
              emit_load_value(layout, inst.first, operand_type(layout, inst.first), XR_RCX, out);
              emit_load_i128_from_address_register(XR_RCX, XR_RAX, XR_RDX, out);
            }
            emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
          } else {
            const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
            mi = make_instruction(mir::Instruction::MI_LOAD);
            mi.type = inst.type.text;
            mi.operands.push_back(reg(dst));
            if(direct_base != nullptr) {
              mi.operands.push_back(deref(*direct_base));
            } else {
              emit_load_value(layout, inst.first, operand_type(layout, inst.first), XR_RCX, out);
              mi.operands.push_back(deref(XR_RCX));
            }
            out.push_back(mi);
            emit_store_temp(layout, inst.dest, dst, out);
          }
        } else {
          if(is_float_type(inst.type)) {
            if(is_thread_local_global_operand(inst.first)) {
              emit_load_address(layout, inst.first, XR_R11, out);
              emit_store_temp_float(layout, inst.dest, deref(XR_R11), out);
            } else {
              emit_store_temp_float(layout, inst.dest, storage_operand(layout, inst.first), out);
            }
          } else if(is_i128_scalar_type(inst.type.text)) {
            emit_load_address(layout, inst.first, XR_R11, out);
            emit_load_i128_from_address_register(XR_R11, XR_RAX, XR_RDX, out);
            emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
          } else {
            const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
            mi = make_instruction(mir::Instruction::MI_LOAD);
            mi.type = inst.type.text;
            mi.operands.push_back(reg(dst));
            if(is_thread_local_global_operand(inst.first)) {
              emit_load_address(layout, inst.first, XR_R11, out);
              mi.operands.push_back(deref(XR_R11));
            } else if(inst.first.kind == lir::Operand::OP_GLOBAL) {
              mi.operands.push_back(global_ref(inst.first.text));
            } else {
              mi.operands.push_back(frame(layout, inst.first.text));
            }
            out.push_back(mi);
            emit_store_temp(layout, inst.dest, dst, out);
          }
        }
        return;

      case lir::Instruction::IK_ATOMIC_LOAD: {
        const long long order = atomic_order_value(inst.second);
        const X64Register * direct_base =
            direct_pointer_base_register(layout, inst.first);
        if(direct_base == nullptr) {
          emit_load_value(layout, inst.first, "ptr", XR_RCX, out);
        }
        mi = make_instruction(mir::Instruction::MI_LOAD);
        mi.type = inst.type.text;
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(deref(direct_base != nullptr ? *direct_base : XR_RCX));
        out.push_back(mi);
        (void)order;
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;
      }

      case lir::Instruction::IK_STORE:
        if(is_promoted_param_slot_store(layout, inst)) {
          return;
        }
        if(is_float_type(inst.type)) {
          const string source_type = operand_type(layout, inst.first);
          const mir::Operand src = float_source_operand(layout, inst.first);
          if(inst.second.kind == lir::Operand::OP_TEMP &&
             operand_type(layout, inst.second) == "ptr") {
            const X64Register * direct_base =
                direct_pointer_base_register(layout, inst.second);
            if(direct_base != nullptr) {
              emit_float_move_or_convert(deref(*direct_base),
                                         inst.type.text,
                                         source_type,
                                         src,
                                         out);
            } else {
              emit_load_value(layout, inst.second, operand_type(layout, inst.second), XR_RCX, out);
              emit_float_move_or_convert(deref(XR_RCX),
                                         inst.type.text,
                                         source_type,
                                         src,
                                         out);
            }
          } else {
            if(is_thread_local_global_operand(inst.second)) {
              emit_load_address(layout, inst.second, XR_R11, out);
              emit_float_move_or_convert(deref(XR_R11),
                                         inst.type.text,
                                         source_type,
                                         src,
                                         out);
            } else {
              emit_float_move_or_convert(storage_operand(layout, inst.second),
                                         inst.type.text,
                                         source_type,
                                         src,
                                         out);
            }
          }
        } else if(is_i128_scalar_type(inst.type.text)) {
          emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
          if(inst.second.kind == lir::Operand::OP_TEMP &&
             operand_type(layout, inst.second) == "ptr") {
            const X64Register * direct_base =
                direct_pointer_base_register(layout, inst.second);
            if(direct_base != nullptr) {
              emit_store_i128_to_address_register(*direct_base, XR_RAX, XR_RDX, out);
            } else {
              emit_load_value(layout, inst.second, "ptr", XR_RCX, out);
              emit_store_i128_to_address_register(XR_RCX, XR_RAX, XR_RDX, out);
            }
          } else {
            emit_load_address(layout, inst.second, XR_R11, out);
            emit_store_i128_to_address_register(XR_R11, XR_RAX, XR_RDX, out);
          }
        } else {
          emit_load_value(layout, inst.first, inst.type.text, XR_RAX, out);
          if(inst.second.kind == lir::Operand::OP_TEMP &&
             operand_type(layout, inst.second) == "ptr") {
            mi = make_instruction(mir::Instruction::MI_STORE);
            mi.type = inst.type.text;
            if(const X64Register * direct_base =
                   direct_pointer_base_register(layout, inst.second)) {
              mi.operands.push_back(deref(*direct_base));
            } else {
              emit_load_value(layout, inst.second, operand_type(layout, inst.second), XR_RCX, out);
              mi.operands.push_back(deref(XR_RCX));
            }
            mi.operands.push_back(reg(XR_RAX));
            out.push_back(mi);
          } else {
            emit_store_storage(layout, inst.second, inst.type.text, XR_RAX, out);
          }
        }
        return;

      case lir::Instruction::IK_ATOMIC_STORE: {
        const long long order = atomic_order_value(inst.third);
        const X64Register * direct_base =
            direct_pointer_base_register(layout, inst.second);
        if(direct_base == nullptr) {
          emit_load_value(layout, inst.second, "ptr", XR_RCX, out);
        }
        emit_load_value(layout, inst.first, inst.type.text, XR_RAX, out);
        if(is_seq_cst_order(order)) {
          mi = make_instruction(mir::Instruction::MI_XCHG);
          mi.type = inst.type.text;
          mi.operands.push_back(deref(direct_base != nullptr ? *direct_base : XR_RCX));
          mi.operands.push_back(reg(XR_RAX));
          out.push_back(mi);
        } else {
          mi = make_instruction(mir::Instruction::MI_STORE);
          mi.type = inst.type.text;
          mi.operands.push_back(deref(direct_base != nullptr ? *direct_base : XR_RCX));
          mi.operands.push_back(reg(XR_RAX));
          out.push_back(mi);
        }
        return;
      }

      case lir::Instruction::IK_ATOMIC_EXCHANGE: {
        const X64Register * direct_base =
            direct_pointer_base_register(layout, inst.first);
        if(direct_base == nullptr) {
          emit_load_value(layout, inst.first, "ptr", XR_RCX, out);
        }
        emit_load_value(layout, inst.second, inst.type.text, XR_RAX, out);
        mi = make_instruction(mir::Instruction::MI_XCHG);
        mi.type = inst.type.text;
        mi.operands.push_back(deref(direct_base != nullptr ? *direct_base : XR_RCX));
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;
      }

      case lir::Instruction::IK_ATOMIC_COMPARE_EXCHANGE: {
        if(inst.args.size() != 2) {
          throw lir::ParseError("atomic_compare_exchange requires two order operands");
        }
        emit_load_value(layout, inst.first, "ptr", XR_RCX, out);
        emit_load_value(layout, inst.second, "ptr", XR_RDX, out);
        mi = make_instruction(mir::Instruction::MI_LOAD);
        mi.type = inst.type.text;
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(deref(XR_RDX));
        out.push_back(mi);
        emit_load_value(layout, inst.third, inst.type.text, XR_RSI, out);
        mi = make_instruction(mir::Instruction::MI_LOCK_CMPXCHG);
        mi.type = inst.type.text;
        mi.operands.push_back(deref(XR_RCX));
        mi.operands.push_back(reg(XR_RSI));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_STORE);
        mi.type = inst.type.text;
        mi.operands.push_back(deref(XR_RDX));
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_SETCC);
        mi.condition = XC_E;
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_MOVZX);
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;
      }

      case lir::Instruction::IK_INDEX: {
        if(layout.direct_call_arg_index_temps.count(inst.dest) != 0) {
          return;
        }
        const size_t scale = lir::type_size(lir::LowType{inst.op});
        const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
        emit_load_value(layout, inst.first, "ptr", dst, out);
        if(inst.second.kind == lir::Operand::OP_INTEGER) {
          long long scaled_offset = 0;
          const bool overflow =
              __builtin_mul_overflow(inst.second.int_value,
                                     static_cast<long long>(scale),
                                     &scaled_offset);
          if(!overflow) {
            if(scaled_offset != 0) {
              mi = make_instruction(mir::Instruction::MI_LEA);
              mi.operands.push_back(reg(dst));
              mi.operands.push_back(deref_offset(dst, scaled_offset));
              out.push_back(mi);
            }
            emit_store_temp(layout, inst.dest, dst, out);
            return;
          }
        }
        const X64Register index_reg = alternate_integer_scratch(dst);
        emit_load_value(layout, inst.second, "i64", index_reg, out);
        if(scale != 1) {
          mi = make_instruction(mir::Instruction::MI_IMUL);
          mi.operands.push_back(reg(index_reg));
          mi.operands.push_back(imm(static_cast<long long>(scale)));
          out.push_back(mi);
        }
        mi = make_instruction(mir::Instruction::MI_ADD);
        mi.operands.push_back(reg(dst));
        mi.operands.push_back(reg(index_reg));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, dst, out);
        return;
      }

      case lir::Instruction::IK_UNARY:
        if(is_float_type(inst.type)) {
          if(inst.op == "neg") {
            mi = make_instruction(mir::Instruction::MI_FNEG);
            mi.type = inst.type.text;
            mi.operands.push_back(float_dest_operand(layout, inst.dest));
            mi.operands.push_back(float_source_operand(layout, inst.first));
            out.push_back(mi);
          } else if(inst.op == "not") {
            mi = make_instruction(mir::Instruction::MI_FEQ);
            mi.type = inst.type.text;
            mi.operands.push_back(reg(XR_RAX));
            mi.operands.push_back(float_source_operand(layout, inst.first));
            mi.operands.push_back(zero_float_imm(inst.type.text));
            out.push_back(mi);
            emit_store_temp(layout, inst.dest, XR_RAX, out);
          } else {
            throw logic_error("unsupported unary op " + inst.op);
          }
        } else {
          if(is_i128_scalar_type(inst.type.text)) {
            if(inst.op == "decay") {
              throw logic_error("unsupported unary op " + inst.op);
            }
            emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
            if(inst.op == "neg") {
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(XR_RCX));
              mi.operands.push_back(reg(XR_RAX));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_NEG);
              mi.operands.push_back(reg(XR_RAX));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_CMP);
              mi.type = "i64";
              mi.operands.push_back(reg(XR_RCX));
              mi.operands.push_back(imm(0));
              out.push_back(mi);
              emit_setcc_bool(XC_NE, XR_R10, out);

              mi = make_instruction(mir::Instruction::MI_NEG);
              mi.operands.push_back(reg(XR_RDX));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_SUB);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_R10));
              out.push_back(mi);
              emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              return;
            }
            if(inst.op == "bitnot") {
              mi = make_instruction(mir::Instruction::MI_NOT);
              mi.operands.push_back(reg(XR_RAX));
              out.push_back(mi);
              mi = make_instruction(mir::Instruction::MI_NOT);
              mi.operands.push_back(reg(XR_RDX));
              out.push_back(mi);
              emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              return;
            }
            if(inst.op == "not") {
              mi = make_instruction(mir::Instruction::MI_CMP);
              mi.type = "i64";
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(imm(0));
              out.push_back(mi);
              emit_setcc_bool(XC_E, XR_R10, out);

              mi = make_instruction(mir::Instruction::MI_CMP);
              mi.type = "i64";
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(imm(0));
              out.push_back(mi);
              emit_setcc_bool(XC_E, XR_R11, out);

              mi = make_instruction(mir::Instruction::MI_AND);
              mi.operands.push_back(reg(XR_R10));
              mi.operands.push_back(reg(XR_R11));
              out.push_back(mi);
              emit_store_temp(layout, inst.dest, XR_R10, out);
              return;
            }
            throw logic_error("unsupported unary op " + inst.op);
          }
          if(inst.op == "not" &&
             layout.direct_branch_temps.count(inst.dest) != 0) {
            return;
          }
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          emit_load_value(layout, inst.first, inst.type.text, dst, out);
          if(inst.op == "decay") {
            if(inst.type.text != "ptr") {
              throw logic_error("unsupported unary op " + inst.op);
            }
          } else if(inst.op == "neg") {
            mi = make_instruction(mir::Instruction::MI_NEG);
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          } else if(inst.op == "bswap") {
            if(inst.type.text == "i16" || inst.type.text == "u16") {
              const X64Register scratch = alternate_integer_scratch(dst);
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(scratch));
              mi.operands.push_back(reg(dst));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(XR_RCX));
              mi.operands.push_back(imm(8));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_SHL_CL);
              mi.operands.push_back(reg(dst));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_SHR_CL);
              mi.operands.push_back(reg(scratch));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_OR);
              mi.operands.push_back(reg(dst));
              mi.operands.push_back(reg(scratch));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(XR_RCX));
              mi.operands.push_back(imm(0xFFFF));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_AND);
              mi.operands.push_back(reg(dst));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);
            } else if(inst.type.text == "i32" || inst.type.text == "u32" ||
                      inst.type.text == "i64") {
              mi = make_instruction(mir::Instruction::MI_BSWAP);
              mi.type = inst.type.text;
              mi.operands.push_back(reg(dst));
              out.push_back(mi);
            } else {
              throw logic_error("unsupported bswap type " + inst.type.text);
            }
          } else if(inst.op == "bitnot") {
            mi = make_instruction(mir::Instruction::MI_NOT);
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          } else if(inst.op == "not") {
            mi = make_instruction(mir::Instruction::MI_CMP);
            mi.type = inst.type.text;
            mi.operands.push_back(reg(dst));
            mi.operands.push_back(imm(0));
            out.push_back(mi);
            mi = make_instruction(mir::Instruction::MI_SETCC);
            mi.condition = XC_E;
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
            mi = make_instruction(mir::Instruction::MI_MOVZX);
            mi.operands.push_back(reg(dst));
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          } else {
            throw logic_error("unsupported unary op " + inst.op);
          }
          emit_store_temp(layout, inst.dest, dst, out);
        }
        return;

      case lir::Instruction::IK_BINARY:
        if(is_float_type(inst.type)) {
          mi = make_instruction(float_binary_opcode(inst.op));
          mi.type = inst.type.text;
          mi.operands.push_back(float_dest_operand(layout, inst.dest));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          mi.operands.push_back(float_source_operand(layout, inst.second));
          out.push_back(mi);
        } else {
          if(is_i128_scalar_type(inst.type.text)) {
            if(inst.op == "mul") {
              emit_call_i128_binary_helper(layout, "cppgm_builtin_i128_mul", inst, out);
              return;
            }
            if(inst.op == "div") {
              emit_call_i128_binary_helper(layout, "cppgm_builtin_i128_div", inst, out);
              return;
            }
            if(inst.op == "udiv") {
              emit_call_i128_binary_helper(layout, "cppgm_builtin_u128_div", inst, out);
              return;
            }
            if(inst.op == "mod") {
              emit_call_i128_binary_helper(layout, "cppgm_builtin_i128_mod", inst, out);
              return;
            }
            if(inst.op == "umod") {
              emit_call_i128_binary_helper(layout, "cppgm_builtin_u128_mod", inst, out);
              return;
            }
            if(inst.op == "shl") {
              emit_call_i128_shift_helper(layout, "cppgm_builtin_i128_shl", inst, out);
              return;
            }
            if(inst.op == "shr") {
              emit_call_i128_shift_helper(layout, "cppgm_builtin_i128_sar", inst, out);
              return;
            }
            if(inst.op == "ushr") {
              emit_call_i128_shift_helper(layout, "cppgm_builtin_u128_shr", inst, out);
              return;
            }

            emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
            emit_load_i128_value(layout, inst.second, inst.type.text, XR_RCX, XR_RSI, out);
            if(inst.op == "add") {
              mi = make_instruction(mir::Instruction::MI_ADD);
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);

              mi = make_instruction(mir::Instruction::MI_CMP);
              mi.type = "i64";
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);
              emit_setcc_bool(XC_B, XR_R10, out);

              mi = make_instruction(mir::Instruction::MI_ADD);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_RSI));
              out.push_back(mi);
              mi = make_instruction(mir::Instruction::MI_ADD);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_R10));
              out.push_back(mi);
              emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              return;
            }
            if(inst.op == "sub") {
              mi = make_instruction(mir::Instruction::MI_CMP);
              mi.type = "i64";
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);
              emit_setcc_bool(XC_B, XR_R10, out);

              mi = make_instruction(mir::Instruction::MI_SUB);
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);
              mi = make_instruction(mir::Instruction::MI_SUB);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_RSI));
              out.push_back(mi);
              mi = make_instruction(mir::Instruction::MI_SUB);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_R10));
              out.push_back(mi);
              emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              return;
            }
            if(inst.op == "and" || inst.op == "or" || inst.op == "xor") {
              const mir::Instruction::Opcode opcode =
                  inst.op == "and" ? mir::Instruction::MI_AND :
                  inst.op == "or" ? mir::Instruction::MI_OR :
                                    mir::Instruction::MI_XOR;
              mi = make_instruction(opcode);
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(XR_RCX));
              out.push_back(mi);
              mi = make_instruction(opcode);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(reg(XR_RSI));
              out.push_back(mi);
              emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              return;
            }
            throw logic_error("unsupported i128 binary op " + inst.op);
          }
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          const X64Register rhs = alternate_integer_scratch(dst);
          emit_load_value(layout, inst.first, inst.type.text, dst, out);
          const X64Register * rhs_direct = direct_integer_temp_register(layout, inst.second);
          mi = make_instruction(integer_binary_opcode(inst.op));

          if(inst.op == "div" || inst.op == "mod" ||
             inst.op == "udiv" || inst.op == "umod") {
            mi = make_instruction(mir::Instruction::MI_MOV);
            mi.operands.push_back(reg(XR_RCX));
            if(rhs_direct != nullptr) {
              mi.operands.push_back(reg(*rhs_direct));
            } else {
              emit_load_value(layout, inst.second, inst.type.text, rhs, out);
              mi.operands.push_back(reg(rhs));
            }
            out.push_back(mi);
            emit_normalize_integer_temp(inst.type.text, XR_RCX, out);
            if(dst != XR_RAX) {
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(XR_RAX));
              mi.operands.push_back(reg(dst));
              out.push_back(mi);
            }
            emit_normalize_integer_temp(inst.type.text, XR_RAX, out);
            if(inst.op == "div" || inst.op == "mod") {
              mi = make_instruction(mir::Instruction::MI_CQO);
            } else {
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(XR_RDX));
              mi.operands.push_back(imm(0));
            }
            out.push_back(mi);
            mi = make_instruction((inst.op == "div" || inst.op == "mod") ?
                                      mir::Instruction::MI_IDIV :
                                      mir::Instruction::MI_DIV);
            mi.operands.push_back(reg(XR_RCX));
            out.push_back(mi);
            if(inst.op == "mod" || inst.op == "umod") {
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(dst));
              mi.operands.push_back(reg(XR_RDX));
              out.push_back(mi);
            } else if(dst != XR_RAX) {
              mi = make_instruction(mir::Instruction::MI_MOV);
              mi.operands.push_back(reg(dst));
              mi.operands.push_back(reg(XR_RAX));
              out.push_back(mi);
            }
          } else if(inst.op == "shl" || inst.op == "shr" || inst.op == "ushr") {
            emit_normalize_integer_temp(inst.type.text, dst, out);
            mi = make_instruction(mir::Instruction::MI_MOV);
            mi.operands.push_back(reg(XR_RCX));
            if(rhs_direct != nullptr) {
              mi.operands.push_back(reg(*rhs_direct));
            } else {
              emit_load_value(layout, inst.second, inst.type.text, rhs, out);
              mi.operands.push_back(reg(rhs));
            }
            out.push_back(mi);
            mi = make_instruction(inst.op == "shl" ?
                                      mir::Instruction::MI_SHL_CL :
                                      inst.op == "ushr" ?
                                          mir::Instruction::MI_SHR_CL :
                                          mir::Instruction::MI_SAR_CL);
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          } else {
            const bool rhs_immediate =
                inst.second.kind == lir::Operand::OP_INTEGER &&
                integer_binary_immediate_form_supported(inst.op,
                                                        inst.type.text,
                                                        inst.second.int_value);
            if(rhs_direct == nullptr &&
               !rhs_immediate) {
              emit_load_value(layout, inst.second, inst.type.text, rhs, out);
            }
            mi.operands.push_back(reg(dst));
            if(rhs_direct != nullptr) {
              mi.operands.push_back(reg(*rhs_direct));
            } else if(rhs_immediate) {
              mi.operands.push_back(imm(inst.second.int_value));
            } else {
              mi.operands.push_back(reg(rhs));
            }
            out.push_back(mi);
          }
          emit_store_temp(layout, inst.dest, dst, out);
        }
        return;

      case lir::Instruction::IK_CMP:
        if(layout.direct_branch_temps.count(inst.dest) != 0) {
          return;
        }
        if(is_float_type(inst.type)) {
          mi = make_instruction(float_compare_opcode(inst.op));
          mi.type = inst.type.text;
          mi.operands.push_back(reg(XR_RAX));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          mi.operands.push_back(float_source_operand(layout, inst.second));
          out.push_back(mi);
          emit_store_temp(layout, inst.dest, XR_RAX, out);
        } else {
          if(is_i128_scalar_type(inst.type.text)) {
            emit_compare_i128(layout, inst, out);
            return;
          }
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          const X64Register rhs = alternate_integer_scratch(dst);
          emit_load_value(layout, inst.first, inst.type.text, dst, out);
          mi = make_instruction(mir::Instruction::MI_CMP);
          mi.type = inst.type.text;
          mi.operands.push_back(reg(dst));
          if(const X64Register * rhs_direct = direct_integer_temp_register(layout, inst.second)) {
            mi.operands.push_back(reg(*rhs_direct));
          } else {
            emit_load_value(layout, inst.second, inst.type.text, rhs, out);
            mi.operands.push_back(reg(rhs));
          }
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_SETCC);
          mi.condition = integer_cmp_condition(inst.op);
          mi.operands.push_back(reg(dst));
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_MOVZX);
          mi.operands.push_back(reg(dst));
          mi.operands.push_back(reg(dst));
          out.push_back(mi);
          emit_store_temp(layout, inst.dest, dst, out);
        }
        return;

      case lir::Instruction::IK_CONVERT:
        if(inst.op == "sext" || inst.op == "zext" || inst.op == "trunc") {
          if(is_i128_scalar_type(inst.type.text) ||
             is_i128_scalar_type(inst.source_type.text)) {
            if(is_i128_scalar_type(inst.source_type.text)) {
              emit_load_i128_value(layout, inst.first, inst.source_type.text, XR_RAX, XR_RDX, out);
              if(is_i128_scalar_type(inst.type.text)) {
                emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
              } else {
                emit_store_temp(layout, inst.dest, XR_RAX, out);
              }
              return;
            }
            emit_load_value(layout, inst.first, inst.source_type.text, XR_RAX, out);
            if(inst.op == "zext") {
              emit_zero_register(XR_RDX, out);
            } else {
              const size_t source_size = lir::type_size(inst.source_type);
              if(source_size < 8) {
                mi = make_instruction(mir::Instruction::MI_SEXT);
                mi.byte_count = source_size;
                mi.operands.push_back(reg(XR_RAX));
                out.push_back(mi);
              }
              emit_sign_mask_from_register(XR_RAX, XR_RDX, out);
            }
            emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
            return;
          }
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          emit_load_value(layout, inst.first, inst.source_type.text, dst, out);
          if(inst.op == "sext") {
            mi = make_instruction(mir::Instruction::MI_SEXT);
            mi.byte_count = lir::type_size(inst.source_type);
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          } else if(inst.op == "zext") {
            mi = make_instruction(mir::Instruction::MI_ZEXT);
            mi.byte_count = lir::type_size(inst.source_type);
            mi.operands.push_back(reg(dst));
            out.push_back(mi);
          }
          emit_store_temp(layout, inst.dest, dst, out);
          return;
        }
        if(inst.op == "sitofp") {
          mi = make_instruction(mir::Instruction::MI_SITOFP);
          mi.type = inst.type.text;
          mi.byte_count = lir::type_size(inst.source_type);
          mi.operands.push_back(float_dest_operand(layout, inst.dest));
          mi.operands.push_back(integer_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        if(inst.op == "uitofp") {
          mi = make_instruction(mir::Instruction::MI_UITOFP);
          mi.type = inst.type.text;
          mi.byte_count = lir::type_size(inst.source_type);
          mi.operands.push_back(float_dest_operand(layout, inst.dest));
          mi.operands.push_back(integer_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        if(inst.op == "fptosi") {
          mi = make_instruction(mir::Instruction::MI_FPTOSI);
          mi.type = inst.source_type.text;
          mi.byte_count = lir::type_size(inst.type);
          mi.operands.push_back(integer_dest_operand(layout, inst.dest));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        if(inst.op == "fptoui") {
          mi = make_instruction(mir::Instruction::MI_FPTOUI);
          mi.type = inst.source_type.text;
          mi.byte_count = lir::type_size(inst.type);
          mi.operands.push_back(integer_dest_operand(layout, inst.dest));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        if(inst.op == "fpext") {
          mi = make_instruction(mir::Instruction::MI_FPEXT);
          mi.type = inst.type.text;
          mi.byte_count = float_exec_width_bytes(inst.source_type.text);
          mi.operands.push_back(float_dest_operand(layout, inst.dest));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        if(inst.op == "fptrunc") {
          mi = make_instruction(mir::Instruction::MI_FPTRUNC);
          mi.type = inst.type.text;
          mi.byte_count = float_exec_width_bytes(inst.source_type.text);
          mi.operands.push_back(float_dest_operand(layout, inst.dest));
          mi.operands.push_back(float_source_operand(layout, inst.first));
          out.push_back(mi);
          return;
        }
        throw logic_error("unsupported convert op " + inst.op);

      case lir::Instruction::IK_ATOMIC_ADD_FETCH: {
        emit_load_value(layout, inst.first, "ptr", XR_RCX, out);
        emit_load_value(layout, inst.second, inst.type.text, XR_RDX, out);
        mi = make_instruction(mir::Instruction::MI_MOV);
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(reg(XR_RDX));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_LOCK_XADD);
        mi.type = inst.type.text;
        mi.operands.push_back(deref(XR_RCX));
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_ADD);
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(reg(XR_RDX));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;
      }

      case lir::Instruction::IK_ATOMIC_THREAD_FENCE:
        if(is_seq_cst_order(atomic_order_value(inst.first))) {
          mi = make_instruction(mir::Instruction::MI_MFENCE);
          out.push_back(mi);
        }
        return;

      case lir::Instruction::IK_ATOMIC_SIGNAL_FENCE:
        return;

      case lir::Instruction::IK_VA_START:
        emit_va_start(layout, inst, out);
        return;

      case lir::Instruction::IK_VA_ARG:
        emit_va_arg_gp(layout, inst, out);
        return;

      case lir::Instruction::IK_STACK_ALLOC:
      {
        if(inst.type.text != "ptr") {
          throw logic_error("stack_alloc result must be ptr");
        }
        const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
        if(inst.first.kind == lir::Operand::OP_INTEGER) {
          const uint64_t size = static_cast<uint64_t>(inst.first.int_value);
          const uint64_t aligned = (size + 15u) & ~static_cast<uint64_t>(15u);
          if(aligned != 0) {
            mi = make_instruction(mir::Instruction::MI_SUB);
            mi.operands.push_back(reg(XR_RSP));
            mi.operands.push_back(imm(static_cast<long long>(aligned)));
            out.push_back(mi);
          }
        } else {
          emit_load_value(layout, inst.first, "i64", XR_RAX, out);
          mi = make_instruction(mir::Instruction::MI_ADD);
          mi.operands.push_back(reg(XR_RAX));
          mi.operands.push_back(imm(15));
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_MOV);
          mi.operands.push_back(reg(XR_R11));
          mi.operands.push_back(imm(-16));
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_AND);
          mi.operands.push_back(reg(XR_RAX));
          mi.operands.push_back(reg(XR_R11));
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_SUB);
          mi.operands.push_back(reg(XR_RSP));
          mi.operands.push_back(reg(XR_RAX));
          out.push_back(mi);
        }
        mi = make_instruction(mir::Instruction::MI_MOV);
        mi.operands.push_back(reg(dst));
        mi.operands.push_back(reg(XR_RSP));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, dst, out);
        return;
      }

      case lir::Instruction::IK_CALL:
      {
        struct CallArgPiece
        {
          lir::Operand operand;
          string type;
          size_t source_offset = 0;
          bool object_chunk = false;
          bool memory_object = false;
          bool direct_call_index = false;
          lir::Operand direct_call_index_base;
          long long direct_call_index_offset = 0;
          lir::ParamPassingMode passing = lir::PPM_DIRECT;
          bool preserved = false;
          bool preserved_spilled = false;
          X64Register preserved_reg = XR_RAX;
          size_t preserved_spill_index = 0;
        };

        const bool direct_symbol_call =
            inst.first.kind == lir::Operand::OP_GLOBAL &&
            function_names_.count(inst.first.text) != 0;
        const X64Register indirect_target_reg = XR_R10;
        const vector<lir::Parameter> call_params = call_signature_params(inst);
        vector<CallArgPiece> pieces;
        for(size_t i = 0; i < inst.args.size(); ++i) {
          const string param_type =
              i < call_params.size() ? call_params[i].type.text : operand_type(layout, inst.args[i]);
          if(is_memory_class_object_abi_type(param_type)) {
            CallArgPiece piece;
            piece.operand = inst.args[i];
            piece.type = param_type;
            piece.memory_object = true;
            pieces.push_back(piece);
            continue;
          }
          const vector<string> chunk_types = scalar_abi_chunk_types(param_type);
          if(!chunk_types.empty()) {
            size_t chunk_offset = 0;
            for(size_t chunk_index = 0; chunk_index < chunk_types.size(); ++chunk_index) {
              CallArgPiece piece;
              piece.operand = inst.args[i];
              piece.type = chunk_types[chunk_index];
              piece.source_offset = chunk_offset;
              piece.object_chunk = true;
              if(piece.operand.kind == lir::Operand::OP_TEMP &&
                 layout.direct_call_arg_index_temps.count(piece.operand.text) != 0) {
                piece.direct_call_index =
                    direct_call_arg_index_info(layout,
                                               piece.operand.text,
                                               piece.direct_call_index_base,
                                               piece.direct_call_index_offset);
              }
              piece.passing =
                  i < call_params.size() ? call_params[i].metadata.passing : lir::PPM_DIRECT;
              pieces.push_back(piece);
              chunk_offset += lir::type_size(lir::LowType{piece.type});
            }
            continue;
          }
          CallArgPiece piece;
          piece.operand = inst.args[i];
          piece.type = param_type;
          if(piece.operand.kind == lir::Operand::OP_TEMP &&
             layout.direct_call_arg_index_temps.count(piece.operand.text) != 0) {
            piece.direct_call_index =
                direct_call_arg_index_info(layout,
                                           piece.operand.text,
                                           piece.direct_call_index_base,
                                           piece.direct_call_index_offset);
          }
          piece.passing =
              i < call_params.size() ? call_params[i].metadata.passing : lir::PPM_DIRECT;
          pieces.push_back(piece);
        }
        vector<bool> arg_in_reg(pieces.size(), false);
        vector<bool> arg_in_xmm(pieces.size(), false);
        vector<size_t> arg_reg_index(pieces.size(), 0);
        vector<size_t> arg_xmm_index(pieces.size(), 0);
        vector<size_t> arg_stack_offset(pieces.size(), 0);
        size_t next_reg = 0;
        size_t next_xmm = 0;
        size_t stack_bytes = 0;
        for(size_t i = 0; i < pieces.size(); ++i) {
          const string & arg_type = pieces[i].type;
          if(pieces[i].memory_object) {
            const size_t alignment =
                min<size_t>(16, lir::type_alignment(lir::LowType{arg_type}));
            stack_bytes = align_up_size(stack_bytes, max<size_t>(8, alignment));
            arg_stack_offset[i] = stack_bytes;
            stack_bytes += align_up_size(lir::type_size(lir::LowType{arg_type}), 8);
          } else if((arg_type == "f32" || arg_type == "f64") && next_xmm < 8) {
            arg_in_xmm[i] = true;
            arg_xmm_index[i] = next_xmm++;
          } else if(!is_float_type(arg_type) && next_reg < 6) {
            arg_in_reg[i] = true;
            arg_reg_index[i] = next_reg++;
          } else {
            arg_stack_offset[i] = stack_bytes;
            stack_bytes += stack_arg_size(arg_type);
          }
        }
        vector<XmmCallArgMove> xmm_arg_moves;
        for(size_t i = 0; i < pieces.size(); ++i) {
          if(!arg_in_xmm[i]) {
            continue;
          }
          XmmCallArgMove move;
          move.type = pieces[i].type;
          move.dst = float_arg_register(arg_xmm_index[i]);
          move.src = float_source_operand(layout, pieces[i].operand);
          xmm_arg_moves.push_back(move);
        }
        set<X64Register> blocked_arg_regs;
        for(size_t i = 0; i < pieces.size(); ++i) {
          if(arg_in_reg[i]) {
            blocked_arg_regs.insert(arg_register(arg_reg_index[i]));
          }
        }
        if(!direct_symbol_call) {
          blocked_arg_regs.insert(indirect_target_reg);
        }
        set<X64Register> reserved_preserve_regs;
        size_t next_preserve_spill_index = 0;
        for(size_t i = 0; i < pieces.size(); ++i) {
          const lir::Operand preserve_operand =
              pieces[i].direct_call_index ? pieces[i].direct_call_index_base :
                                            pieces[i].operand;
          if(preserve_operand.kind != lir::Operand::OP_TEMP) {
            continue;
          }
          const X64Register * assigned =
              temp_register_for(layout, preserve_operand.text);
          if(assigned == nullptr || blocked_arg_regs.count(*assigned) == 0) {
            continue;
          }
          const bool clobbered_before_materialization =
              call_source_reg_clobbered_before_call_piece(arg_in_reg,
                                                          arg_reg_index,
                                                          i,
                                                          *assigned,
                                                          direct_symbol_call,
                                                          indirect_target_reg);
          bool needs_preserve = false;
          if(arg_in_reg[i]) {
            needs_preserve =
                *assigned != arg_register(arg_reg_index[i]) ||
                clobbered_before_materialization;
          } else {
            needs_preserve = clobbered_before_materialization;
          }
          if(!needs_preserve) {
            continue;
          }
          pieces[i].preserved = true;
          const PreservedIntegerValue preserved =
              preserve_integer_value_if_needed(layout,
                                               preserve_operand,
                                               *assigned,
                                               blocked_arg_regs,
                                               reserved_preserve_regs,
                                               next_preserve_spill_index,
                                               out);
          pieces[i].preserved_reg = preserved.reg;
          if(preserved.spilled) {
            pieces[i].preserved_spilled = true;
            pieces[i].preserved_spill_index = next_preserve_spill_index++;
          } else {
            reserved_preserve_regs.insert(preserved.reg);
          }
        }
        const bool needs_xmm_arg_spill =
            xmm_arg_register_moves_need_stack_spill(xmm_arg_moves);
        const size_t xmm_arg_spill_bytes = needs_xmm_arg_spill ? 16 : 0;
        const size_t stack_payload_bytes = stack_bytes + xmm_arg_spill_bytes;
        const size_t stack_pad =
            stack_payload_bytes == 0 || (stack_payload_bytes % 16) == 0
            ? 0
            : 16 - (stack_payload_bytes % 16);
        if(stack_payload_bytes + stack_pad != 0) {
          mi = make_instruction(mir::Instruction::MI_SUB);
          mi.operands.push_back(reg(XR_RSP));
          mi.operands.push_back(
              imm(static_cast<long long>(stack_payload_bytes + stack_pad)));
          out.push_back(mi);
        }
        if(!direct_symbol_call) {
          // Materialize the indirect target after preserving any call arguments
          // that were assigned to the dedicated target register.
          if(!try_emit_load_global_pointer_call_target(layout,
                                                       inst.first,
                                                       indirect_target_reg,
                                                       out)) {
            emit_load_value(layout, inst.first, "ptr", indirect_target_reg, out);
          }
        }
        for(size_t i = 0; i < pieces.size(); ++i) {
          if(!pieces[i].memory_object) {
            continue;
          }
          emit_load_storage_address_value(layout, pieces[i].operand, XR_RSI, out);
          mi = make_instruction(mir::Instruction::MI_LEA);
          mi.operands.push_back(reg(XR_RDI));
          mi.operands.push_back(
              deref_offset(XR_RSP, static_cast<long long>(arg_stack_offset[i])));
          out.push_back(mi);
          mi = make_instruction(mir::Instruction::MI_COPY_BYTES);
          mi.byte_count = lir::type_size(lir::LowType{pieces[i].type});
          mi.byte_alignment = lir::type_alignment(lir::LowType{pieces[i].type});
          mi.operands.push_back(reg(XR_RDI));
          mi.operands.push_back(reg(XR_RSI));
          out.push_back(mi);
        }
        for(size_t i = 0; i < pieces.size(); ++i) {
          const string & arg_type = pieces[i].type;
          if(pieces[i].memory_object) {
            continue;
          }
          const bool rematerialize_direct_call_index = pieces[i].direct_call_index;
          if(arg_in_reg[i]) {
            if(pieces[i].object_chunk) {
              emit_load_object_chunk(layout,
                                     pieces[i].operand,
                                     arg_type,
                                     pieces[i].source_offset,
                                     arg_register(arg_reg_index[i]),
                                     out);
            } else if(uses_storage_address_passing(pieces[i].passing)) {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                if(pieces[i].preserved_reg != arg_register(arg_reg_index[i])) {
                  mi = make_instruction(mir::Instruction::MI_MOV);
                  mi.operands.push_back(reg(arg_register(arg_reg_index[i])));
                  mi.operands.push_back(reg(pieces[i].preserved_reg));
                  out.push_back(mi);
                }
              } else {
                emit_load_storage_address_value(layout,
                                                pieces[i].operand,
                                                arg_register(arg_reg_index[i]),
                                                out);
              }
            } else if(rematerialize_direct_call_index) {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                emit_adjusted_pointer_from_register(
                    pieces[i].preserved_spilled ? XR_RAX : pieces[i].preserved_reg,
                    pieces[i].direct_call_index_offset,
                    arg_register(arg_reg_index[i]),
                    out);
              } else {
                emit_direct_call_arg_index_temp(layout,
                                                pieces[i].operand.text,
                                                arg_register(arg_reg_index[i]),
                                                out);
              }
            } else {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                if(pieces[i].preserved_reg != arg_register(arg_reg_index[i])) {
                  mi = make_instruction(mir::Instruction::MI_MOV);
                  mi.operands.push_back(reg(arg_register(arg_reg_index[i])));
                  mi.operands.push_back(reg(pieces[i].preserved_reg));
                  out.push_back(mi);
                }
              } else {
                emit_load_value(layout,
                                pieces[i].operand,
                                arg_type,
                                arg_register(arg_reg_index[i]),
                                out);
              }
            }
            continue;
          }
          if(arg_in_xmm[i]) {
            continue;
          }
          if(is_float_type(arg_type)) {
            emit_store_storage_float(layout,
                                     deref_offset(XR_RSP,
                                                  static_cast<long long>(arg_stack_offset[i])),
                                     arg_type,
                                     float_source_operand(layout, pieces[i].operand),
                                     out);
          } else {
            if(pieces[i].object_chunk) {
              emit_load_object_chunk(layout,
                                     pieces[i].operand,
                                     arg_type,
                                     pieces[i].source_offset,
                                     XR_R11,
                                     out);
            } else if(uses_storage_address_passing(pieces[i].passing)) {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                if(pieces[i].preserved_reg != XR_R11) {
                  mi = make_instruction(mir::Instruction::MI_MOV);
                  mi.operands.push_back(reg(XR_R11));
                  mi.operands.push_back(reg(pieces[i].preserved_reg));
                  out.push_back(mi);
                }
              } else {
                emit_load_storage_address_value(layout, pieces[i].operand, XR_R11, out);
              }
            } else if(rematerialize_direct_call_index) {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                emit_adjusted_pointer_from_register(
                    pieces[i].preserved_spilled ? XR_RAX : pieces[i].preserved_reg,
                    pieces[i].direct_call_index_offset,
                    XR_R11,
                    out);
              } else {
                emit_direct_call_arg_index_temp(layout,
                                                pieces[i].operand.text,
                                                XR_R11,
                                                out);
              }
            } else {
              if(pieces[i].preserved) {
                if(pieces[i].preserved_spilled) {
                  mi = make_instruction(mir::Instruction::MI_LOAD);
                  mi.type = "i64";
                  mi.operands.push_back(reg(XR_RAX));
                  mi.operands.push_back(deref_offset(
                      XR_RBP,
                      preserve_spill_slot_offset(layout,
                                                 pieces[i].preserved_spill_index)));
                  out.push_back(mi);
                }
                if(pieces[i].preserved_reg != XR_R11) {
                  mi = make_instruction(mir::Instruction::MI_MOV);
                  mi.operands.push_back(reg(XR_R11));
                  mi.operands.push_back(reg(pieces[i].preserved_reg));
                  out.push_back(mi);
                }
              } else {
                emit_load_value(layout, pieces[i].operand, arg_type, XR_R11, out);
              }
            }
            mi = make_instruction(mir::Instruction::MI_STORE);
            mi.type = arg_type;
            mi.operands.push_back(
                deref_offset(XR_RSP, static_cast<long long>(arg_stack_offset[i])));
            mi.operands.push_back(reg(XR_R11));
            out.push_back(mi);
          }
        }
        mir::Operand xmm_arg_spill_slot;
        const mir::Operand * xmm_arg_spill_slot_ptr = nullptr;
        if(needs_xmm_arg_spill) {
          xmm_arg_spill_slot = deref_offset(XR_RSP, static_cast<long long>(stack_bytes));
          xmm_arg_spill_slot_ptr = &xmm_arg_spill_slot;
        }
        emit_xmm_arg_register_moves(xmm_arg_moves, xmm_arg_spill_slot_ptr, out);
        const lir::FunctionBoundaryMetadata boundary = resolved_call_boundary(inst);
        if(boundary.arity == lir::CAM_VARIADIC) {
          mi = make_instruction(mir::Instruction::MI_MOV);
          mi.operands.push_back(reg(XR_RAX));
          mi.operands.push_back(imm(static_cast<long long>(next_xmm)));
          out.push_back(mi);
        }
        if(direct_symbol_call) {
          mi = make_instruction(mir::Instruction::MI_CALL);
          mi.call_unwind_no = boundary.unwind == lir::CUM_NO;
          mi.call_returns_noreturn = boundary.returns == lir::CRM_NORETURN;
          mi.call_variadic = boundary.arity == lir::CAM_VARIADIC;
          mi.operands.push_back(symbol(inst.first.text));
          out.push_back(mi);
        } else {
          mi = make_instruction(mir::Instruction::MI_CALL_INDIRECT);
          mi.call_unwind_no = boundary.unwind == lir::CUM_NO;
          mi.call_returns_noreturn = boundary.returns == lir::CRM_NORETURN;
          mi.call_variadic = boundary.arity == lir::CAM_VARIADIC;
          mi.operands.push_back(reg(indirect_target_reg));
          out.push_back(mi);
        }
        if(stack_payload_bytes + stack_pad != 0) {
          mi = make_instruction(mir::Instruction::MI_ADD);
          mi.operands.push_back(reg(XR_RSP));
          mi.operands.push_back(
              imm(static_cast<long long>(stack_payload_bytes + stack_pad)));
          out.push_back(mi);
        }
        const bool dead_call_result =
            !inst.call_returns_void &&
            layout.dead_call_result_temps.count(inst.dest) != 0;
        if(dead_call_result) {
          map<string, string>::const_iterator result_type =
              layout.storage_type.find(inst.dest);
          if(result_type != layout.storage_type.end() &&
             result_type->second == "f80") {
            out.push_back(make_instruction(mir::Instruction::MI_FPOP));
          }
        } else if(!inst.call_returns_void) {
          const string result_type = layout.storage_type.find(inst.dest)->second;
          if(!object_abi_chunk_types(result_type).empty()) {
            emit_store_object_return_temp(layout, inst.dest, result_type, out);
          } else if(result_type == "f32" || result_type == "f64") {
            emit_store_temp_float(layout, inst.dest, xmm(XMM_0), out);
          } else if(is_float_type(result_type)) {
            mi = make_instruction(mir::Instruction::MI_FSTP);
            mi.type = result_type;
            mi.operands.push_back(frame(layout, inst.dest));
            out.push_back(mi);
          } else if(is_i128_scalar_type(result_type)) {
            emit_store_i128_temp(layout, inst.dest, XR_RAX, XR_RDX, out);
          } else {
            emit_store_temp(layout, inst.dest, XR_RAX, out);
          }
        }
        return;
      }

      case lir::Instruction::IK_COPYOBJ: {
        const X64Register * src_assigned =
            inst.first.kind == lir::Operand::OP_TEMP
                ? temp_register_for(layout, inst.first.text)
                : nullptr;
        if(src_assigned != nullptr && *src_assigned == XR_RDI) {
          set<X64Register> blocked;
          set<X64Register> reserved;
          blocked.insert(XR_RDI);
          blocked.insert(XR_RSI);
          const PreservedIntegerValue preserved =
              preserve_integer_value_if_needed(layout,
                                               inst.first,
                                               XR_RDI,
                                               blocked,
                                               reserved,
                                               0,
                                               out);
          emit_load_storage_address_value(layout, inst.second, XR_RDI, out);
          if(preserved.spilled) {
            mi = make_instruction(mir::Instruction::MI_LOAD);
            mi.type = "i64";
            mi.operands.push_back(reg(XR_RSI));
            mi.operands.push_back(deref_offset(XR_RBP,
                                               preserve_spill_slot_offset(
                                                   layout,
                                                   preserved.spill_index)));
            out.push_back(mi);
          } else if(preserved.reg != XR_RSI) {
            mi = make_instruction(mir::Instruction::MI_MOV);
            mi.operands.push_back(reg(XR_RSI));
            mi.operands.push_back(reg(preserved.reg));
            out.push_back(mi);
          }
        } else {
          emit_load_storage_address_value(layout, inst.second, XR_RDI, out);
          emit_load_storage_address_value(layout, inst.first, XR_RSI, out);
        }
        mi = make_instruction(mir::Instruction::MI_COPY_BYTES);
        mi.byte_count = inst.byte_count;
        mi.byte_alignment = inst.byte_alignment;
        mi.operands.push_back(reg(XR_RDI));
        mi.operands.push_back(reg(XR_RSI));
        out.push_back(mi);
        return;
      }

      case lir::Instruction::IK_ZEROINIT:
        emit_load_storage_address_value(layout, inst.first, XR_RDI, out);
        mi = make_instruction(mir::Instruction::MI_ZERO_BYTES);
        mi.byte_count = inst.byte_count;
        mi.byte_alignment = inst.byte_alignment;
        mi.operands.push_back(reg(XR_RDI));
        out.push_back(mi);
        return;

      case lir::Instruction::IK_EH_TRY:
      case lir::Instruction::IK_EH_CLEANUP:
        mi = make_instruction(mir::Instruction::MI_EH_PUSH);
        mi.operands.push_back(symbol(block_symbol(function_name, inst.first.text)));
        out.push_back(mi);
        return;

      case lir::Instruction::IK_EH_CATCH:
      case lir::Instruction::IK_EH_CLEANUP_CLAUSE:
      case lir::Instruction::IK_EH_FILTER:
      case lir::Instruction::IK_EH_CATCH_ALL:
        return;

      case lir::Instruction::IK_EH_END:
        mi = make_instruction(mir::Instruction::MI_EH_POP);
        out.push_back(mi);
        return;

      case lir::Instruction::IK_THROW:
        emit_load_value(layout, inst.first, inst.type.text, XR_RAX, out);
        mi = make_instruction(mir::Instruction::MI_THROW);
        mi.type = inst.type.text;
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        return;

      case lir::Instruction::IK_EXCEPTION:
        mi = make_instruction(mir::Instruction::MI_LOAD_EXCEPTION);
        mi.type = inst.type.text;
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;

      case lir::Instruction::IK_EXCEPTION_SELECTOR:
        mi = make_instruction(mir::Instruction::MI_LOAD_EXCEPTION_SELECTOR);
        mi.type = inst.type.text;
        mi.operands.push_back(reg(XR_RAX));
        out.push_back(mi);
        emit_store_temp(layout, inst.dest, XR_RAX, out);
        return;

      case lir::Instruction::IK_RESUME:
        mi = make_instruction(mir::Instruction::MI_RESUME);
        out.push_back(mi);
        return;

      case lir::Instruction::IK_JUMP:
        mi = make_instruction(mir::Instruction::MI_JMP);
        mi.operands.push_back(label(inst.first.text));
        out.push_back(mi);
        return;

      case lir::Instruction::IK_BRANCH:
        if(emit_direct_branch(layout, inst, out)) {
          return;
        }
        emit_load_value(layout, inst.first, "i64", XR_RAX, out);
        mi = make_instruction(mir::Instruction::MI_CMP);
        mi.type = "i64";
        mi.operands.push_back(reg(XR_RAX));
        mi.operands.push_back(imm(0));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_JNE);
        mi.operands.push_back(label(inst.second.text));
        out.push_back(mi);
        mi = make_instruction(mir::Instruction::MI_JMP);
        mi.operands.push_back(label(inst.third.text));
        out.push_back(mi);
        return;

      case lir::Instruction::IK_SWITCH: {
        const string selector_type = operand_type(layout, inst.first);
        if(!is_integer_scalar_type(selector_type)) {
          throw lir::ParseError("switch requires integer selector");
        }
        emit_load_value(layout, inst.first, selector_type, XR_RAX, out);
        for(size_t i = 0; i + 1 < inst.args.size(); i += 2) {
          emit_load_value(layout, inst.args[i], selector_type, XR_RCX, out);
          mi = make_instruction(mir::Instruction::MI_CMP);
          mi.type = selector_type;
          mi.operands.push_back(reg(XR_RAX));
          mi.operands.push_back(reg(XR_RCX));
          out.push_back(mi);
          emit_jcc(XC_E, inst.args[i + 1].text, out);
        }
        mi = make_instruction(mir::Instruction::MI_JMP);
        mi.operands.push_back(label(inst.second.text));
        out.push_back(mi);
        return;
      }

      case lir::Instruction::IK_RETURN:
        if(inst.type.text != "void") {
          if(inst.type.text == "f32" || inst.type.text == "f64") {
            emit_float_move_or_convert(xmm(XMM_0),
                                       inst.type.text,
                                       operand_type(layout, inst.first),
                                       float_source_operand(layout, inst.first),
                                       out);
            mi = make_instruction(mir::Instruction::MI_RET);
            out.push_back(mi);
            return;
          }
          if(is_float_type(inst.type)) {
            mi = make_instruction(mir::Instruction::MI_FRET);
            const string source_type = operand_type(layout, inst.first);
            mi.type = is_float_type(source_type) ? source_type : inst.type.text;
            mi.operands.push_back(float_source_operand(layout, inst.first));
            out.push_back(mi);
            return;
          }
          if(is_i128_scalar_type(inst.type.text)) {
            emit_load_i128_value(layout, inst.first, inst.type.text, XR_RAX, XR_RDX, out);
            mi = make_instruction(mir::Instruction::MI_RET);
            out.push_back(mi);
            return;
          }
          if(!object_abi_chunk_types(inst.type.text).empty()) {
            emit_load_object_return_chunks(layout, inst.first, inst.type.text, out);
            mi = make_instruction(mir::Instruction::MI_RET);
            out.push_back(mi);
            return;
          }
          emit_load_value(layout, inst.first, inst.type.text, XR_RAX, out);
          mi = make_instruction(mir::Instruction::MI_RET);
          mi.operands.push_back(reg(XR_RAX));
          out.push_back(mi);
        } else {
          mi = make_instruction(mir::Instruction::MI_RET);
          out.push_back(mi);
        }
        return;
    }
  }

  void emit_function(const lir::Function & function)
  {
    struct RecentScalarSlotStore
    {
      bool valid = false;
      string slot;
      string type;
      X64Register reg = XR_RAX;
    };

    validate_function(function);
    FunctionLayout layout =
        build_layout(function,
                     host_eh_requested_ &&
                         function_needs_host_eh_enabled(function),
                     function_params_,
                     thread_local_globals_);
    ensure_preserve_spill_slot(function, layout);
    mir::Function out;
    out.name = function.name;
    out.return_type = function.return_type.text;
    out.debug_location.file = function.debug_location.file;
    out.debug_location.line = function.debug_location.line;
    out.debug_location.column = function.debug_location.column;
    out.scratch_bytes = layout.scratch_bytes;
    out.host_eh_enabled = layout.host_eh_enabled;
    out.host_eh_exception_offset = -static_cast<long long>(layout.host_eh_exception_offset);
    out.host_eh_selector_offset = -static_cast<long long>(layout.host_eh_selector_offset);
    if(out.host_eh_enabled) {
      out.host_eh_clauses = collect_host_eh_clauses(function);
    }
    set<X64Register> used_callee_saved;

    out.params = collect_param_bindings(function);
    for(size_t i = 0; i < layout.params.size(); ++i) {
      mir::FrameBinding binding;
      binding.kind = mir::FrameBinding::FB_PARAM_SLOT;
      binding.name = layout.params[i];
      binding.offset = slot_offset(layout, layout.params[i]);
      binding.type = layout.storage_type.find(layout.params[i])->second;
      out.frame_bindings.push_back(binding);
    }
    for(size_t i = 0; i < layout.slots.size(); ++i) {
      mir::FrameBinding binding;
      binding.kind = mir::FrameBinding::FB_SLOT;
      binding.name = layout.slots[i];
      binding.offset = slot_offset(layout, layout.slots[i]);
      binding.type = layout.storage_type.find(layout.slots[i])->second;
      out.frame_bindings.push_back(binding);
    }
    for(size_t i = 0; i < layout.temps.size(); ++i) {
      mir::FrameBinding binding;
      binding.kind = mir::FrameBinding::FB_TEMP;
      binding.name = layout.temps[i];
      binding.offset = slot_offset(layout, layout.temps[i]);
      binding.type = layout.storage_type.find(layout.temps[i])->second;
      out.frame_bindings.push_back(binding);
    }
    out.debug_variables = collect_debug_variables(function, layout);

    size_t source_position = 0;
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      mir::Block block;
      block.label = function.blocks[bi].label;
      set<string> preinitialized_param_slots;
      if(bi == 0) {
        emit_entry_param_setup(layout,
                               out.params,
                               block.instructions,
                               preinitialized_param_slots);
      }
      RecentScalarSlotStore recent_slot_store;
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const lir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(bi == 0 &&
           is_redundant_param_materialization(layout, inst, preinitialized_param_slots)) {
          ++source_position;
          continue;
        }
        if(is_redundant_aliased_object_return_materialization(layout, inst)) {
          ++source_position;
          continue;
        }
        ScopedMachineIRSourceInstruction scoped_source_instruction(inst, source_position);
        if(layout.elided_direct_branch_load_sources.count(inst.dest) != 0) {
          ++source_position;
          continue;
        }
        if(recent_slot_store.valid &&
           inst.kind == lir::Instruction::IK_LOAD &&
           !is_float_type(inst.type) &&
           !is_i128_scalar_type(inst.type.text) &&
           inst.first.kind == lir::Operand::OP_SLOT &&
           inst.first.text == recent_slot_store.slot &&
           inst.type.text == recent_slot_store.type) {
          const X64Register dst = temp_result_register(layout, inst.dest, XR_RAX);
          if(dst != recent_slot_store.reg) {
            mir::Instruction mi = make_instruction(mir::Instruction::MI_MOV);
            mi.operands.push_back(reg(dst));
            mi.operands.push_back(reg(recent_slot_store.reg));
            block.instructions.push_back(mi);
          }
          emit_store_temp(layout, inst.dest, dst, block.instructions);
          recent_slot_store.valid = false;
          ++source_position;
          continue;
        }

        emit_instruction(function.name, layout, inst, block.instructions);
        recent_slot_store.valid = false;
        if(!is_promoted_param_slot_store(layout, inst) &&
           inst.kind == lir::Instruction::IK_STORE &&
           !is_float_type(inst.type) &&
           inst.second.kind == lir::Operand::OP_SLOT) {
          recent_slot_store.valid = true;
          recent_slot_store.slot = inst.second.text;
          recent_slot_store.type = inst.type.text;
          recent_slot_store.reg = XR_RAX;
        }
        ++source_position;
      }
      out.blocks.push_back(block);
    }
    for(size_t bi = 0; bi < out.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < out.blocks[bi].instructions.size(); ++ii) {
        const mir::Instruction & inst = out.blocks[bi].instructions[ii];
        for(size_t oi = 0; oi < inst.operands.size(); ++oi) {
          const mir::Operand & operand = inst.operands[oi];
          if((operand.kind == mir::Operand::OP_REG ||
              operand.kind == mir::Operand::OP_DEREF) &&
             is_callee_saved_temp_register(operand.reg)) {
            used_callee_saved.insert(operand.reg);
          }
        }
      }
    }
    out.callee_saved_regs.assign(used_callee_saved.begin(), used_callee_saved.end());
    const size_t fixed_frame_bytes = align_up_size(layout.frame_bytes, 8);
    out.frame_bytes = fixed_frame_bytes;
    const size_t callee_saved_bytes = out.callee_saved_regs.size() * 8;
    out.stack_size = (fixed_frame_bytes + layout.scratch_bytes + callee_saved_bytes + 15) &
        ~static_cast<size_t>(15);
    machine_.functions.push_back(out);
  }
};

}  // namespace

mir_model::MirProgram build_lowir_machine_ir(const vector<string> & srcfiles,
                                             const string & output_target)
{
  return build_lowir_machine_ir(lowir::parse_lowir_program_files(srcfiles), output_target);
}

mir_model::MirProgram build_lowir_machine_ir(const lowir::LowirProgram & program,
                                             const string & output_target)
{
  return MachineIRBuilder(program, output_target, false).build();
}

mir_model::MirProgram build_lowir_machine_ir_object(const vector<string> & srcfiles,
                                                    const string & output_target,
                                                    bool enable_host_eh)
{
  return build_lowir_machine_ir_object(lowir::parse_lowir_program_files(srcfiles),
                                      output_target,
                                      enable_host_eh);
}

mir_model::MirProgram build_lowir_machine_ir_object(const lowir::LowirProgram & program,
                                                    const string & output_target,
                                                    bool enable_host_eh)
{
  return MachineIRBuilder(program, output_target, enable_host_eh).build_object();
}
