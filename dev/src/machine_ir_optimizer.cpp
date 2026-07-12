#include "machine_ir_optimizer.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "machine_ir.h"
#include "optimization_level.h"

using namespace std;

namespace {

namespace mir = machine_ir;
namespace mir_public = mir_model;

struct LiveState
{
  set<X64Register> regs;
  set<XmmRegister> xmms;
};

struct BlockControlFlow
{
  bool has_fallthrough = false;
  string fallthrough_label;
  bool has_explicit_target = false;
  string explicit_target_label;
  bool unconditional_jump = false;
  bool conditional_jump = false;
};

size_t align_up_size(size_t value, size_t alignment)
{
  return (value + alignment - 1) & ~(alignment - 1);
}

const X64Register kCallerSavedRegs[] = {
  XR_RAX, XR_RCX, XR_RDX, XR_RSI, XR_RDI, XR_R8, XR_R9, XR_R10, XR_R11
};

const XmmRegister kCallerSavedXmms[] = {
  XMM_0, XMM_1, XMM_2, XMM_3, XMM_4, XMM_5, XMM_6, XMM_7
};

const X64Register kIntegerArgRegs[] = {
  XR_RDI, XR_RSI, XR_RDX, XR_RCX, XR_R8, XR_R9
};

const XmmRegister kFloatArgRegs[] = {
  XMM_0, XMM_1, XMM_2, XMM_3, XMM_4, XMM_5, XMM_6, XMM_7
};

X64Register resolve_reg_alias(X64Register reg,
                              const map<X64Register, X64Register> & aliases)
{
  set<X64Register> seen;
  while(seen.insert(reg).second) {
    const map<X64Register, X64Register>::const_iterator found = aliases.find(reg);
    if(found == aliases.end()) {
      break;
    }
    reg = found->second;
  }
  return reg;
}

XmmRegister resolve_xmm_alias(XmmRegister xmm,
                              const map<XmmRegister, XmmRegister> & aliases)
{
  set<XmmRegister> seen;
  while(seen.insert(xmm).second) {
    const map<XmmRegister, XmmRegister>::const_iterator found = aliases.find(xmm);
    if(found == aliases.end()) {
      break;
    }
    xmm = found->second;
  }
  return xmm;
}

void kill_reg_alias(X64Register reg, map<X64Register, X64Register> & aliases)
{
  aliases.erase(reg);
  for(map<X64Register, X64Register>::iterator it = aliases.begin();
      it != aliases.end();) {
    if(it->second == reg) {
      aliases.erase(it++);
    } else {
      ++it;
    }
  }
}

void kill_xmm_alias(XmmRegister xmm, map<XmmRegister, XmmRegister> & aliases)
{
  aliases.erase(xmm);
  for(map<XmmRegister, XmmRegister>::iterator it = aliases.begin();
      it != aliases.end();) {
    if(it->second == xmm) {
      aliases.erase(it++);
    } else {
      ++it;
    }
  }
}

void kill_reg_value(X64Register reg, map<X64Register, mir::Operand> & values)
{
  values.erase(reg);
}

bool fits_signed_imm32(long long value)
{
  return value >= (-2147483647LL - 1) && value <= 2147483647LL;
}

bool can_rematerialize_read_operand(const mir::Instruction & inst,
                                    size_t operand_index,
                                    const mir::Operand & value)
{
  switch(inst.opcode) {
    case mir::Instruction::MI_MOV:
      return operand_index == 1 &&
             (value.kind == mir::Operand::OP_IMM ||
              value.kind == mir::Operand::OP_SYMBOL);

    case mir::Instruction::MI_ADD:
    case mir::Instruction::MI_SUB:
      return operand_index == 1 &&
             value.kind == mir::Operand::OP_IMM &&
             fits_signed_imm32(value.imm);

    case mir::Instruction::MI_IMUL:
      return operand_index == 1 && value.kind == mir::Operand::OP_IMM;

    case mir::Instruction::MI_CMP:
      return operand_index == 1 &&
             value.kind == mir::Operand::OP_IMM &&
             value.imm == 0;

    default:
      return false;
  }
}

mir::Operand frame_operand(long long offset)
{
  mir::Operand out;
  out.kind = mir::Operand::OP_FRAME;
  out.offset = offset;
  return out;
}

void note_read_operand(const mir::Operand & operand, LiveState & state)
{
  switch(operand.kind) {
    case mir::Operand::OP_REG:
      state.regs.insert(operand.reg);
      return;
    case mir::Operand::OP_XMM:
      state.xmms.insert(operand.xmm);
      return;
    case mir::Operand::OP_DEREF:
      state.regs.insert(operand.reg);
      return;
    default:
      return;
  }
}

void note_write_operand(const mir::Operand & operand, LiveState & state)
{
  switch(operand.kind) {
    case mir::Operand::OP_REG:
      state.regs.insert(operand.reg);
      return;
    case mir::Operand::OP_XMM:
      state.xmms.insert(operand.xmm);
      return;
    default:
      return;
  }
}

void note_address_operand(const mir::Operand & operand, LiveState & state)
{
  if(operand.kind == mir::Operand::OP_DEREF) {
    state.regs.insert(operand.reg);
  }
}

void note_caller_saved_defs(LiveState & defs)
{
  for(size_t i = 0; i < sizeof(kCallerSavedRegs) / sizeof(kCallerSavedRegs[0]); ++i) {
    defs.regs.insert(kCallerSavedRegs[i]);
  }
  for(size_t i = 0; i < sizeof(kCallerSavedXmms) / sizeof(kCallerSavedXmms[0]); ++i) {
    defs.xmms.insert(kCallerSavedXmms[i]);
  }
}

void note_call_argument_uses(LiveState & uses)
{
  for(size_t i = 0; i < sizeof(kIntegerArgRegs) / sizeof(kIntegerArgRegs[0]); ++i) {
    uses.regs.insert(kIntegerArgRegs[i]);
  }
  for(size_t i = 0; i < sizeof(kFloatArgRegs) / sizeof(kFloatArgRegs[0]); ++i) {
    uses.xmms.insert(kFloatArgRegs[i]);
  }
}

bool is_xmm_return_type(const string & type)
{
  return type == "f32" || type == "f64";
}

bool is_multi_reg_return_type(const string & type)
{
  return type == "i128" || type == "u128" ||
         (type.size() >= 4 && type.compare(0, 4, "obj<") == 0);
}

void collect_instruction_effects(const mir::Instruction & inst,
                                 LiveState & uses,
                                 LiveState & defs)
{
  switch(inst.opcode) {
    case mir::Instruction::MI_MOV:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_LOAD:
      note_write_operand(inst.operands[0], defs);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_STORE:
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_MFENCE:
      return;

    case mir::Instruction::MI_LOCK_XADD:
    case mir::Instruction::MI_XCHG:
    case mir::Instruction::MI_LOCK_CMPXCHG:
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      note_write_operand(inst.operands[1], defs);
      defs.regs.insert(XR_RAX);
      return;

    case mir::Instruction::MI_LEA:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_FMOV:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      if(inst.type == "f80") {
        // Native f80 stores use these registers to address globals and clear
        // the six ABI padding bytes after the x87 payload.
        defs.regs.insert(XR_RAX);
        defs.regs.insert(XR_R11);
      }
      return;

    case mir::Instruction::MI_FNEG:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_FADD:
    case mir::Instruction::MI_FSUB:
    case mir::Instruction::MI_FMUL:
    case mir::Instruction::MI_FDIV:
    case mir::Instruction::MI_FEQ:
    case mir::Instruction::MI_FNE:
    case mir::Instruction::MI_FLT:
    case mir::Instruction::MI_FGT:
    case mir::Instruction::MI_FLE:
    case mir::Instruction::MI_FGE:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      note_read_operand(inst.operands[2], uses);
      return;

    case mir::Instruction::MI_FCMP:
      note_read_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_FSTP:
      note_address_operand(inst.operands[0], uses);
      return;

    case mir::Instruction::MI_SITOFP:
    case mir::Instruction::MI_UITOFP:
    case mir::Instruction::MI_FPTOSI:
    case mir::Instruction::MI_FPTOUI:
    case mir::Instruction::MI_FPEXT:
    case mir::Instruction::MI_FPTRUNC:
      note_write_operand(inst.operands[0], defs);
      note_address_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_ADD:
    case mir::Instruction::MI_SUB:
    case mir::Instruction::MI_IMUL:
    case mir::Instruction::MI_AND:
    case mir::Instruction::MI_OR:
    case mir::Instruction::MI_XOR:
      note_read_operand(inst.operands[0], uses);
      note_write_operand(inst.operands[0], defs);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_NEG:
    case mir::Instruction::MI_NOT:
    case mir::Instruction::MI_BSWAP:
    case mir::Instruction::MI_SEXT:
    case mir::Instruction::MI_ZEXT:
      note_read_operand(inst.operands[0], uses);
      note_write_operand(inst.operands[0], defs);
      return;

    case mir::Instruction::MI_CMP:
    case mir::Instruction::MI_TEST:
      note_read_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_JCC:
    case mir::Instruction::MI_JNE:
    case mir::Instruction::MI_JMP:
      return;

    case mir::Instruction::MI_SETCC:
      note_write_operand(inst.operands[0], defs);
      return;

    case mir::Instruction::MI_MOVZX:
      note_write_operand(inst.operands[0], defs);
      note_read_operand(inst.operands[1], uses);
      return;

    case mir::Instruction::MI_CQO:
      uses.regs.insert(XR_RAX);
      defs.regs.insert(XR_RDX);
      return;

    case mir::Instruction::MI_IDIV:
    case mir::Instruction::MI_DIV:
      uses.regs.insert(XR_RAX);
      uses.regs.insert(XR_RDX);
      note_read_operand(inst.operands[0], uses);
      defs.regs.insert(XR_RAX);
      defs.regs.insert(XR_RDX);
      return;

    case mir::Instruction::MI_SHL_CL:
    case mir::Instruction::MI_SHR_CL:
    case mir::Instruction::MI_SAR_CL:
      note_read_operand(inst.operands[0], uses);
      note_write_operand(inst.operands[0], defs);
      uses.regs.insert(XR_RCX);
      return;

    case mir::Instruction::MI_TLS_ADDR:
      note_write_operand(inst.operands[0], defs);
      note_read_operand(inst.operands[1], uses);
      note_caller_saved_defs(defs);
      return;

    case mir::Instruction::MI_CALL:
      if(inst.call_variadic) {
        uses.regs.insert(XR_RAX);
      }
      note_call_argument_uses(uses);
      note_caller_saved_defs(defs);
      return;

    case mir::Instruction::MI_CALL_INDIRECT:
      note_read_operand(inst.operands[0], uses);
      if(inst.call_variadic) {
        uses.regs.insert(XR_RAX);
      }
      note_call_argument_uses(uses);
      note_caller_saved_defs(defs);
      return;

    case mir::Instruction::MI_COPY_BYTES:
      note_read_operand(inst.operands[0], uses);
      note_read_operand(inst.operands[1], uses);
      defs.regs.insert(XR_RCX);
      if(inst.byte_count > 16) {
        defs.regs.insert(XR_RDI);
        defs.regs.insert(XR_RSI);
      }
      return;

    case mir::Instruction::MI_ZERO_BYTES:
      note_read_operand(inst.operands[0], uses);
      defs.regs.insert(XR_RAX);
      defs.regs.insert(XR_RCX);
      defs.regs.insert(XR_RDI);
      return;

    case mir::Instruction::MI_EH_PUSH:
    case mir::Instruction::MI_EH_POP:
      return;

    case mir::Instruction::MI_LOAD_EXCEPTION:
    case mir::Instruction::MI_LOAD_EXCEPTION_SELECTOR:
      note_write_operand(inst.operands[0], defs);
      return;

    case mir::Instruction::MI_THROW:
      note_read_operand(inst.operands[0], uses);
      return;

    case mir::Instruction::MI_RESUME:
      return;

    case mir::Instruction::MI_JMP_INDIRECT:
      note_read_operand(inst.operands[0], uses);
      return;

    case mir::Instruction::MI_FRET:
      note_read_operand(inst.operands[0], uses);
      return;

    case mir::Instruction::MI_RET:
      if(!inst.operands.empty()) {
        note_read_operand(inst.operands[0], uses);
      }
      return;

    case mir::Instruction::MI_EXIT:
      return;
  }
}

void note_implicit_return_uses(const mir::Function & function,
                               const mir::Instruction & inst,
                               LiveState & uses)
{
  if(inst.opcode != mir::Instruction::MI_RET || !inst.operands.empty()) {
    return;
  }

  if(function.return_type == "void" || function.return_type == "f80") {
    return;
  }

  if(is_xmm_return_type(function.return_type)) {
    uses.xmms.insert(XMM_0);
    return;
  }

  uses.regs.insert(XR_RAX);
  if(is_multi_reg_return_type(function.return_type)) {
    uses.regs.insert(XR_RDX);
  }
}

void collect_instruction_effects(const mir::Function & function,
                                 const mir::Instruction & inst,
                                 LiveState & uses,
                                 LiveState & defs)
{
  collect_instruction_effects(inst, uses, defs);
  note_implicit_return_uses(function, inst, uses);
}

void rewrite_read_operand(mir::Operand & operand,
                          const map<X64Register, X64Register> & reg_aliases,
                          const map<XmmRegister, XmmRegister> & xmm_aliases,
                          bool & changed)
{
  switch(operand.kind) {
    case mir::Operand::OP_REG: {
      const X64Register resolved = resolve_reg_alias(operand.reg, reg_aliases);
      if(resolved != operand.reg) {
        operand.reg = resolved;
        changed = true;
      }
      return;
    }
    case mir::Operand::OP_XMM: {
      const XmmRegister resolved = resolve_xmm_alias(operand.xmm, xmm_aliases);
      if(resolved != operand.xmm) {
        operand.xmm = resolved;
        changed = true;
      }
      return;
    }
    case mir::Operand::OP_DEREF: {
      const X64Register resolved = resolve_reg_alias(operand.reg, reg_aliases);
      if(resolved != operand.reg) {
        operand.reg = resolved;
        changed = true;
      }
      return;
    }
    default:
      return;
  }
}

void rewrite_instruction_reads(mir::Instruction & inst,
                               const map<X64Register, X64Register> & reg_aliases,
                               const map<XmmRegister, XmmRegister> & xmm_aliases,
                               bool & changed)
{
  switch(inst.opcode) {
    case mir::Instruction::MI_MOV:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_LOAD:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_STORE:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_LOCK_XADD:
    case mir::Instruction::MI_XCHG:
    case mir::Instruction::MI_LOCK_CMPXCHG:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_LEA:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_FMOV:
    case mir::Instruction::MI_FNEG:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_FADD:
    case mir::Instruction::MI_FSUB:
    case mir::Instruction::MI_FMUL:
    case mir::Instruction::MI_FDIV:
    case mir::Instruction::MI_FEQ:
    case mir::Instruction::MI_FNE:
    case mir::Instruction::MI_FLT:
    case mir::Instruction::MI_FGT:
    case mir::Instruction::MI_FLE:
    case mir::Instruction::MI_FGE:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[2], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_FCMP:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_FSTP:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_SITOFP:
    case mir::Instruction::MI_UITOFP:
    case mir::Instruction::MI_FPTOSI:
    case mir::Instruction::MI_FPTOUI:
    case mir::Instruction::MI_FPEXT:
    case mir::Instruction::MI_FPTRUNC:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_ADD:
    case mir::Instruction::MI_SUB:
    case mir::Instruction::MI_IMUL:
    case mir::Instruction::MI_AND:
    case mir::Instruction::MI_OR:
    case mir::Instruction::MI_XOR:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_CMP:
    case mir::Instruction::MI_TEST:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_MOVZX:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_IDIV:
    case mir::Instruction::MI_DIV:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_TLS_ADDR:
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_CALL_INDIRECT:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_COPY_BYTES:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      rewrite_read_operand(inst.operands[1], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_ZERO_BYTES:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_LOAD_EXCEPTION:
    case mir::Instruction::MI_LOAD_EXCEPTION_SELECTOR:
    case mir::Instruction::MI_SETCC:
    case mir::Instruction::MI_CQO:
    case mir::Instruction::MI_EH_PUSH:
    case mir::Instruction::MI_EH_POP:
    case mir::Instruction::MI_RESUME:
    case mir::Instruction::MI_JMP:
    case mir::Instruction::MI_JCC:
    case mir::Instruction::MI_JNE:
    case mir::Instruction::MI_MFENCE:
    case mir::Instruction::MI_NEG:
    case mir::Instruction::MI_NOT:
    case mir::Instruction::MI_BSWAP:
    case mir::Instruction::MI_SEXT:
    case mir::Instruction::MI_ZEXT:
    case mir::Instruction::MI_SHL_CL:
    case mir::Instruction::MI_SHR_CL:
    case mir::Instruction::MI_SAR_CL:
    case mir::Instruction::MI_CALL:
    case mir::Instruction::MI_EXIT:
      return;

    case mir::Instruction::MI_THROW:
    case mir::Instruction::MI_JMP_INDIRECT:
    case mir::Instruction::MI_FRET:
      rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      return;

    case mir::Instruction::MI_RET:
      if(!inst.operands.empty()) {
        rewrite_read_operand(inst.operands[0], reg_aliases, xmm_aliases, changed);
      }
      return;
  }
}

void rematerialize_instruction_reads(mir::Instruction & inst,
                                     const map<X64Register, mir::Operand> & reg_values,
                                     bool & changed)
{
  for(size_t i = 0; i < inst.operands.size(); ++i) {
    mir::Operand & operand = inst.operands[i];
    if(operand.kind != mir::Operand::OP_REG) {
      continue;
    }
    const map<X64Register, mir::Operand>::const_iterator found = reg_values.find(operand.reg);
    if(found == reg_values.end() ||
       !can_rematerialize_read_operand(inst, i, found->second)) {
      continue;
    }
    operand = found->second;
    changed = true;
  }
}

bool fold_frame_address_operand(mir::Operand & operand,
                                const map<X64Register, long long> & frame_addresses)
{
  if(operand.kind != mir::Operand::OP_DEREF) {
    return false;
  }
  const map<X64Register, long long>::const_iterator found = frame_addresses.find(operand.reg);
  if(found == frame_addresses.end()) {
    return false;
  }
  operand = frame_operand(found->second + operand.offset);
  return true;
}

void kill_instruction_defs(const mir::Instruction & inst,
                           map<X64Register, X64Register> & reg_aliases,
                           map<XmmRegister, XmmRegister> & xmm_aliases,
                           map<X64Register, mir::Operand> & reg_values)
{
  LiveState uses;
  LiveState defs;
  collect_instruction_effects(inst, uses, defs);
  for(set<X64Register>::const_iterator it = defs.regs.begin();
      it != defs.regs.end(); ++it) {
    kill_reg_alias(*it, reg_aliases);
    kill_reg_value(*it, reg_values);
  }
  for(set<XmmRegister>::const_iterator it = defs.xmms.begin();
      it != defs.xmms.end(); ++it) {
    kill_xmm_alias(*it, xmm_aliases);
  }
}

bool fold_block_local_frame_addresses(mir::Block & block)
{
  bool changed = false;
  map<X64Register, long long> frame_addresses;
  vector<mir::Instruction> rewritten;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    mir::Instruction inst = block.instructions[i];
    for(size_t oi = 0; oi < inst.operands.size(); ++oi) {
      changed = fold_frame_address_operand(inst.operands[oi], frame_addresses) || changed;
    }

    if(inst.opcode == mir::Instruction::MI_MOV &&
       inst.operands.size() == 2 &&
       inst.operands[0].kind == mir::Operand::OP_REG &&
       inst.operands[1].kind == mir::Operand::OP_REG) {
      const map<X64Register, long long>::const_iterator found =
          frame_addresses.find(inst.operands[1].reg);
      if(found != frame_addresses.end()) {
        inst.opcode = mir::Instruction::MI_LEA;
        inst.operands[1] = frame_operand(found->second);
        changed = true;
      }
    }

    LiveState uses;
    LiveState defs;
    collect_instruction_effects(inst, uses, defs);
    for(set<X64Register>::const_iterator it = defs.regs.begin();
        it != defs.regs.end(); ++it) {
      frame_addresses.erase(*it);
    }

    if(inst.opcode == mir::Instruction::MI_LEA &&
       inst.operands.size() == 2 &&
       inst.operands[0].kind == mir::Operand::OP_REG &&
       inst.operands[1].kind == mir::Operand::OP_FRAME) {
      frame_addresses[inst.operands[0].reg] = inst.operands[1].offset;
    }

    rewritten.push_back(inst);
  }

  if(changed) {
    block.instructions.swap(rewritten);
  }
  return changed;
}

bool propagate_block_local_copies(mir::Block & block)
{
  bool changed = false;
  map<X64Register, X64Register> reg_aliases;
  map<XmmRegister, XmmRegister> xmm_aliases;
  map<X64Register, mir::Operand> reg_values;
  vector<mir::Instruction> rewritten;
  for(size_t i = 0; i < block.instructions.size(); ++i) {
    mir::Instruction inst = block.instructions[i];
    rewrite_instruction_reads(inst, reg_aliases, xmm_aliases, changed);
    rematerialize_instruction_reads(inst, reg_values, changed);

    if(inst.opcode == mir::Instruction::MI_MOV &&
       inst.operands.size() == 2 &&
       inst.operands[0].kind == mir::Operand::OP_REG &&
       inst.operands[1].kind == mir::Operand::OP_REG) {
      const X64Register dst = inst.operands[0].reg;
      const X64Register src = inst.operands[1].reg;
      if(dst == src) {
        changed = true;
        continue;
      }
      kill_reg_alias(dst, reg_aliases);
      kill_reg_value(dst, reg_values);
      reg_aliases[dst] = src;
      rewritten.push_back(inst);
      continue;
    }

    if(inst.opcode == mir::Instruction::MI_MOV &&
       inst.operands.size() == 2 &&
       inst.operands[0].kind == mir::Operand::OP_REG &&
       (inst.operands[1].kind == mir::Operand::OP_IMM ||
        inst.operands[1].kind == mir::Operand::OP_SYMBOL)) {
      const X64Register dst = inst.operands[0].reg;
      kill_reg_alias(dst, reg_aliases);
      reg_values[dst] = inst.operands[1];
      rewritten.push_back(inst);
      continue;
    }

    if(inst.opcode == mir::Instruction::MI_FMOV &&
       inst.operands.size() == 2 &&
       inst.operands[0].kind == mir::Operand::OP_XMM &&
       inst.operands[1].kind == mir::Operand::OP_XMM) {
      const XmmRegister dst = inst.operands[0].xmm;
      const XmmRegister src = inst.operands[1].xmm;
      if(dst == src) {
        changed = true;
        continue;
      }
      kill_xmm_alias(dst, xmm_aliases);
      xmm_aliases[dst] = src;
      rewritten.push_back(inst);
      continue;
    }

    kill_instruction_defs(inst, reg_aliases, xmm_aliases, reg_values);
    rewritten.push_back(inst);
  }

  if(changed) {
    block.instructions.swap(rewritten);
  }
  return changed;
}

void subtract_live_state(LiveState & dst, const LiveState & remove)
{
  for(set<X64Register>::const_iterator it = remove.regs.begin();
      it != remove.regs.end(); ++it) {
    dst.regs.erase(*it);
  }
  for(set<XmmRegister>::const_iterator it = remove.xmms.begin();
      it != remove.xmms.end(); ++it) {
    dst.xmms.erase(*it);
  }
}

void union_live_state(LiveState & dst, const LiveState & src)
{
  dst.regs.insert(src.regs.begin(), src.regs.end());
  dst.xmms.insert(src.xmms.begin(), src.xmms.end());
}

bool live_state_equal(const LiveState & lhs, const LiveState & rhs)
{
  return lhs.regs == rhs.regs && lhs.xmms == rhs.xmms;
}

bool instruction_has_terminal_control_flow(const mir::Instruction & inst)
{
  return inst.opcode == mir::Instruction::MI_RET ||
         inst.opcode == mir::Instruction::MI_FRET ||
         inst.opcode == mir::Instruction::MI_EXIT ||
         inst.opcode == mir::Instruction::MI_THROW ||
         inst.opcode == mir::Instruction::MI_RESUME ||
         inst.opcode == mir::Instruction::MI_JMP ||
         inst.opcode == mir::Instruction::MI_JMP_INDIRECT;
}

bool invert_condition(X86Condition cond, X86Condition & out)
{
  switch(cond) {
    case XC_O:  out = XC_NO; return true;
    case XC_NO: out = XC_O;  return true;
    case XC_B:  out = XC_AE; return true;
    case XC_AE: out = XC_B;  return true;
    case XC_E:  out = XC_NE; return true;
    case XC_NE: out = XC_E;  return true;
    case XC_BE: out = XC_A;  return true;
    case XC_A:  out = XC_BE; return true;
    case XC_S:  out = XC_NS; return true;
    case XC_NS: out = XC_S;  return true;
    case XC_P:  out = XC_NP; return true;
    case XC_NP: out = XC_P;  return true;
    case XC_L:  out = XC_GE; return true;
    case XC_GE: out = XC_L;  return true;
    case XC_LE: out = XC_G;  return true;
    case XC_G:  out = XC_LE; return true;
  }
  return false;
}

bool is_conditional_branch(const mir::Instruction & inst)
{
  return inst.opcode == mir::Instruction::MI_JCC ||
         inst.opcode == mir::Instruction::MI_JNE;
}

bool uses_only_zero_flag(const mir::Instruction & inst)
{
  if(inst.opcode == mir::Instruction::MI_JNE) {
    return true;
  }
  if((inst.opcode == mir::Instruction::MI_JCC ||
      inst.opcode == mir::Instruction::MI_SETCC) &&
     (inst.condition == XC_E || inst.condition == XC_NE)) {
    return true;
  }
  return false;
}

bool branch_target_label(const mir::Instruction & inst, string & out_label)
{
  if(!is_conditional_branch(inst) ||
     inst.operands.size() != 1 ||
     inst.operands[0].kind != mir::Operand::OP_LABEL) {
    return false;
  }
  out_label = inst.operands[0].text;
  return true;
}

bool invert_conditional_branch(mir::Instruction & inst)
{
  if(inst.opcode == mir::Instruction::MI_JNE) {
    inst.opcode = mir::Instruction::MI_JCC;
    inst.condition = XC_E;
    return true;
  }
  if(inst.opcode != mir::Instruction::MI_JCC) {
    return false;
  }
  X86Condition inverted = XC_E;
  if(!invert_condition(inst.condition, inverted)) {
    return false;
  }
  inst.condition = inverted;
  return true;
}

BlockControlFlow analyze_block_control_flow(const mir::Function & function, size_t block_index)
{
  BlockControlFlow out;
  const mir::Block & block = function.blocks[block_index];
  if(block.instructions.empty()) {
    if(block_index + 1 < function.blocks.size()) {
      out.has_fallthrough = true;
      out.fallthrough_label = function.blocks[block_index + 1].label;
    }
    return out;
  }

  const mir::Instruction & tail = block.instructions.back();
  if(tail.opcode == mir::Instruction::MI_JMP &&
     tail.operands.size() == 1 &&
     tail.operands[0].kind == mir::Operand::OP_LABEL) {
    out.unconditional_jump = true;
    out.has_explicit_target = true;
    out.explicit_target_label = tail.operands[0].text;
    return out;
  }
  if(is_conditional_branch(tail) &&
     tail.operands.size() == 1 &&
     tail.operands[0].kind == mir::Operand::OP_LABEL) {
    out.conditional_jump = true;
    out.has_explicit_target = true;
    out.explicit_target_label = tail.operands[0].text;
    if(block_index + 1 < function.blocks.size()) {
      out.has_fallthrough = true;
      out.fallthrough_label = function.blocks[block_index + 1].label;
    }
    return out;
  }
  if(!instruction_has_terminal_control_flow(tail) &&
     block_index + 1 < function.blocks.size()) {
    out.has_fallthrough = true;
    out.fallthrough_label = function.blocks[block_index + 1].label;
  }
  return out;
}

vector<LiveState> compute_block_live_out(const mir::Function & function)
{
  map<string, size_t> block_index;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    block_index[function.blocks[i].label] = i;
  }

  auto append_label_successor = [&](vector<size_t> & out, const string & label) {
    const map<string, size_t>::const_iterator found = block_index.find(label);
    if(found == block_index.end()) {
      return;
    }
    if(find(out.begin(), out.end(), found->second) == out.end()) {
      out.push_back(found->second);
    }
  };

  vector<vector<size_t> > successors(function.blocks.size());
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    const mir::Block & block = function.blocks[i];
    if(block.instructions.empty()) {
      if(i + 1 < function.blocks.size()) {
        successors[i].push_back(i + 1);
      }
      continue;
    }

    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const mir::Instruction & inst = block.instructions[ii];
      if((inst.opcode == mir::Instruction::MI_JCC ||
          inst.opcode == mir::Instruction::MI_JNE ||
          inst.opcode == mir::Instruction::MI_JMP) &&
         inst.operands.size() == 1 &&
         inst.operands[0].kind == mir::Operand::OP_LABEL) {
        append_label_successor(successors[i], inst.operands[0].text);
      }
    }

    const mir::Instruction & tail = block.instructions.back();
    if(tail.opcode == mir::Instruction::MI_JCC ||
       tail.opcode == mir::Instruction::MI_JNE ||
       (!instruction_has_terminal_control_flow(tail) && i + 1 < function.blocks.size())) {
      if(find(successors[i].begin(), successors[i].end(), i + 1) == successors[i].end()) {
        successors[i].push_back(i + 1);
      }
    }
  }

  vector<LiveState> block_use(function.blocks.size());
  vector<LiveState> block_def(function.blocks.size());
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    for(size_t j = 0; j < function.blocks[i].instructions.size(); ++j) {
      LiveState uses;
      LiveState defs;
      collect_instruction_effects(function, function.blocks[i].instructions[j], uses, defs);
      for(set<X64Register>::const_iterator it = uses.regs.begin();
          it != uses.regs.end(); ++it) {
        if(block_def[i].regs.count(*it) == 0) {
          block_use[i].regs.insert(*it);
        }
      }
      for(set<XmmRegister>::const_iterator it = uses.xmms.begin();
          it != uses.xmms.end(); ++it) {
        if(block_def[i].xmms.count(*it) == 0) {
          block_use[i].xmms.insert(*it);
        }
      }
      union_live_state(block_def[i], defs);
    }
  }

  vector<LiveState> live_in(function.blocks.size());
  vector<LiveState> live_out(function.blocks.size());
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t i = function.blocks.size(); i-- > 0;) {
      LiveState new_out;
      for(size_t si = 0; si < successors[i].size(); ++si) {
        union_live_state(new_out, live_in[successors[i][si]]);
      }

      LiveState new_in = block_use[i];
      LiveState remainder = new_out;
      subtract_live_state(remainder, block_def[i]);
      union_live_state(new_in, remainder);

      if(!live_state_equal(new_out, live_out[i]) ||
         !live_state_equal(new_in, live_in[i])) {
        live_out[i] = new_out;
        live_in[i] = new_in;
        changed = true;
      }
    }
  }

  return live_out;
}

bool is_trivially_dead_move(const mir::Instruction & inst)
{
  if(inst.opcode == mir::Instruction::MI_MOV) {
    if(inst.operands.size() != 2) {
      return false;
    }
    return inst.operands[0].kind == mir::Operand::OP_REG &&
           inst.operands[1].kind != mir::Operand::OP_DEREF;
  }
  if(inst.opcode == mir::Instruction::MI_FMOV) {
    if(inst.operands.size() != 2) {
      return false;
    }
    return inst.operands[0].kind == mir::Operand::OP_XMM &&
           inst.operands[1].kind != mir::Operand::OP_DEREF;
  }
  if(inst.opcode == mir::Instruction::MI_LEA) {
    return inst.operands.size() == 2 &&
           inst.operands[0].kind == mir::Operand::OP_REG;
  }
  return false;
}

bool move_destination_is_live(const mir::Instruction & inst, const LiveState & live)
{
  if(inst.opcode == mir::Instruction::MI_MOV &&
     inst.operands[0].kind == mir::Operand::OP_REG) {
    return live.regs.count(inst.operands[0].reg) != 0;
  }
  if(inst.opcode == mir::Instruction::MI_LEA &&
     inst.operands[0].kind == mir::Operand::OP_REG) {
    return live.regs.count(inst.operands[0].reg) != 0;
  }
  if(inst.opcode == mir::Instruction::MI_FMOV &&
     inst.operands[0].kind == mir::Operand::OP_XMM) {
    return live.xmms.count(inst.operands[0].xmm) != 0;
  }
  return true;
}

bool compatible_artifact_metadata(const mir::Instruction & first,
                                  const mir::Instruction & second)
{
  if(first.has_source_position &&
     second.has_source_position &&
     first.source_position != second.source_position) {
    return false;
  }
  if(first.debug_location.present() &&
     second.debug_location.present() &&
     (first.debug_location.file != second.debug_location.file ||
      first.debug_location.line != second.debug_location.line ||
      first.debug_location.column != second.debug_location.column)) {
    return false;
  }
  return true;
}

bool function_has_real_debug_info(const mir::Function & function)
{
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      if(function.blocks[bi].instructions[ii].debug_location.present()) {
        return true;
      }
    }
  }
  return false;
}

bool combine_metadata_is_compatible(const mir::Function & function,
                                    const mir::Instruction & first,
                                    const mir::Instruction & second)
{
  if(!function_has_real_debug_info(function)) {
    return true;
  }

  return compatible_artifact_metadata(first, second);
}

void merge_artifact_metadata(mir::Instruction & target,
                             const mir::Instruction & first,
                             const mir::Instruction & second)
{
  if(second.has_source_position) {
    target.has_source_position = true;
    target.source_position = second.source_position;
  } else {
    target.has_source_position = first.has_source_position;
    target.source_position = first.source_position;
  }

  if(second.debug_location.present()) {
    target.debug_location = second.debug_location;
  } else {
    target.debug_location = first.debug_location;
  }
}

bool operand_uses_reg(const mir::Operand & operand, X64Register reg)
{
  if(operand.kind == mir::Operand::OP_REG ||
     operand.kind == mir::Operand::OP_DEREF) {
    return operand.reg == reg;
  }
  return false;
}

bool is_droppable_self_move(const mir::Function & function,
                            const mir::Instruction & inst)
{
  return inst.opcode == mir::Instruction::MI_MOV &&
         inst.operands.size() == 2 &&
         inst.operands[0].kind == mir::Operand::OP_REG &&
         inst.operands[1].kind == mir::Operand::OP_REG &&
         inst.operands[0].reg == inst.operands[1].reg &&
         !inst.debug_location.present() &&
         (!inst.has_source_position || !function_has_real_debug_info(function));
}

bool debug_variable_depends_on_reg_after_instruction(const mir::Function & function,
                                                     X64Register reg,
                                                     const mir::Instruction & inst)
{
  if(!function_has_real_debug_info(function) ||
     function.debug_variables.empty()) {
    return false;
  }

  const bool has_cutoff = inst.has_source_position;
  const size_t cutoff = inst.source_position;
  for(size_t vi = 0; vi < function.debug_variables.size(); ++vi) {
    const mir::DebugVariable & variable = function.debug_variables[vi];
    for(size_t ri = 0; ri < variable.ranges.size(); ++ri) {
      const mir::DebugVariable::Range & range = variable.ranges[ri];
      if(range.location != mir::DebugVariable::Range::LK_REG ||
         range.reg != reg) {
        continue;
      }
      if(!has_cutoff || range.end_source_position > cutoff) {
        return true;
      }
    }
  }

  return false;
}

bool instruction_reads_reg(const mir::Function & function,
                           const mir::Instruction & inst,
                           X64Register reg)
{
  LiveState uses;
  LiveState defs;
  collect_instruction_effects(function, inst, uses, defs);
  return uses.regs.count(reg) != 0;
}

bool instruction_writes_reg(const mir::Instruction & inst, X64Register reg)
{
  LiveState uses;
  LiveState defs;
  collect_instruction_effects(inst, uses, defs);
  return defs.regs.count(reg) != 0;
}

bool combine_block_trivial_mov_artifacts(mir::Function & function)
{
  const vector<LiveState> live_out = compute_block_live_out(function);
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    mir::Block & block = function.blocks[bi];
    if(block.instructions.size() < 2) {
      continue;
    }

    vector<LiveState> live_after(block.instructions.size());
    LiveState live = live_out[bi];
    for(size_t ii = block.instructions.size(); ii-- > 0;) {
      live_after[ii] = live;
      LiveState uses;
      LiveState defs;
      collect_instruction_effects(function, block.instructions[ii], uses, defs);
      subtract_live_state(live, defs);
      union_live_state(live, uses);
    }

    vector<mir::Instruction> rewritten;
    bool block_changed = false;
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      bool rewrote_nonadjacent_alias_move = false;
      if(block.instructions[ii].opcode == mir::Instruction::MI_MOV &&
         block.instructions[ii].operands.size() == 2 &&
         block.instructions[ii].operands[0].kind == mir::Operand::OP_REG &&
         block.instructions[ii].operands[1].kind == mir::Operand::OP_REG) {
        const mir::Instruction & first = block.instructions[ii];
        for(size_t jj = ii + 1; jj < block.instructions.size(); ++jj) {
          const mir::Instruction & candidate = block.instructions[jj];
          if(instruction_writes_reg(candidate, first.operands[1].reg)) {
            break;
          }

          const bool touches_alias =
              instruction_reads_reg(function, candidate, first.operands[0].reg) ||
              instruction_writes_reg(candidate, first.operands[0].reg);
          if(!touches_alias) {
            continue;
          }

          if(candidate.opcode == mir::Instruction::MI_MOV &&
             candidate.operands.size() == 2 &&
             candidate.operands[0].kind == mir::Operand::OP_REG &&
             candidate.operands[1].kind == mir::Operand::OP_REG &&
             candidate.operands[1].reg == first.operands[0].reg &&
             combine_metadata_is_compatible(function, first, candidate)) {
            const bool alias_dead_after_candidate =
                live_after[jj].regs.count(first.operands[0].reg) == 0 &&
                !debug_variable_depends_on_reg_after_instruction(function,
                                                                 first.operands[0].reg,
                                                                 candidate);
            if(!alias_dead_after_candidate) {
              rewritten.push_back(first);
            }
            for(size_t kk = ii + 1; kk < jj; ++kk) {
              rewritten.push_back(block.instructions[kk]);
            }
            mir::Instruction bypassed = candidate;
            bypassed.operands[1] = first.operands[1];
            if(!is_droppable_self_move(function, bypassed)) {
              merge_artifact_metadata(bypassed, first, candidate);
              rewritten.push_back(bypassed);
            }
            ii = jj;
            changed = true;
            block_changed = true;
            rewrote_nonadjacent_alias_move = true;
          }
          break;
        }
        if(rewrote_nonadjacent_alias_move) {
          continue;
        }
      }

      if(ii + 2 < block.instructions.size()) {
        const mir::Instruction & first = block.instructions[ii];
        const mir::Instruction & second = block.instructions[ii + 1];
        const mir::Instruction & third = block.instructions[ii + 2];

        if(first.opcode == mir::Instruction::MI_MOV &&
           first.operands.size() == 2 &&
           first.operands[0].kind == mir::Operand::OP_REG &&
           first.operands[1].kind == mir::Operand::OP_REG &&
           second.opcode == mir::Instruction::MI_STORE &&
           second.operands.size() == 2 &&
           second.operands[1].kind == mir::Operand::OP_REG &&
           second.operands[1].reg == first.operands[0].reg &&
           !operand_uses_reg(second.operands[0], first.operands[0].reg) &&
           third.opcode == mir::Instruction::MI_MOV &&
           third.operands.size() == 2 &&
           third.operands[0].kind == mir::Operand::OP_REG &&
           third.operands[1].kind == mir::Operand::OP_REG &&
           third.operands[1].reg == first.operands[0].reg &&
           live_after[ii + 2].regs.count(first.operands[0].reg) == 0 &&
           !debug_variable_depends_on_reg_after_instruction(function,
                                                            first.operands[0].reg,
                                                            third) &&
           combine_metadata_is_compatible(function, first, second)) {
          mir::Instruction combined_store = second;
          combined_store.operands[1] = first.operands[1];
          merge_artifact_metadata(combined_store, first, second);
          rewritten.push_back(combined_store);

          mir::Instruction rewritten_move = third;
          rewritten_move.operands[1] = first.operands[1];
          if(!is_droppable_self_move(function, rewritten_move)) {
            rewritten.push_back(rewritten_move);
          }

          ii += 2;
          changed = true;
          block_changed = true;
          continue;
        }
      }

      if(ii + 1 < block.instructions.size()) {
        const mir::Instruction & first = block.instructions[ii];
        const mir::Instruction & second = block.instructions[ii + 1];

        if(first.opcode == mir::Instruction::MI_LOAD &&
           first.operands.size() == 2 &&
           first.operands[0].kind == mir::Operand::OP_REG &&
           second.opcode == mir::Instruction::MI_MOV &&
           second.operands.size() == 2 &&
           second.operands[0].kind == mir::Operand::OP_REG &&
           second.operands[1].kind == mir::Operand::OP_REG &&
           second.operands[1].reg == first.operands[0].reg &&
           live_after[ii + 1].regs.count(first.operands[0].reg) == 0 &&
           !debug_variable_depends_on_reg_after_instruction(function,
                                                            first.operands[0].reg,
                                                            second) &&
           combine_metadata_is_compatible(function, first, second)) {
          mir::Instruction combined = first;
          combined.operands[0] = second.operands[0];
          merge_artifact_metadata(combined, first, second);
          rewritten.push_back(combined);
          ++ii;
          changed = true;
          block_changed = true;
          continue;
        }

        if(first.opcode == mir::Instruction::MI_MOV &&
           first.operands.size() == 2 &&
           first.operands[0].kind == mir::Operand::OP_REG &&
           first.operands[1].kind == mir::Operand::OP_REG &&
           second.opcode == mir::Instruction::MI_MOV &&
           second.operands.size() == 2 &&
           second.operands[0].kind == mir::Operand::OP_REG &&
           second.operands[1].kind == mir::Operand::OP_REG &&
           second.operands[1].reg == first.operands[0].reg &&
           combine_metadata_is_compatible(function, first, second)) {
          mir::Instruction bypassed = second;
          bypassed.operands[1] = first.operands[1];

          if(live_after[ii + 1].regs.count(first.operands[0].reg) == 0 &&
             !debug_variable_depends_on_reg_after_instruction(function,
                                                              first.operands[0].reg,
                                                              second)) {
            if(!is_droppable_self_move(function, bypassed)) {
              merge_artifact_metadata(bypassed, first, second);
              rewritten.push_back(bypassed);
            }
          } else {
            rewritten.push_back(first);
            if(!is_droppable_self_move(function, bypassed)) {
              merge_artifact_metadata(bypassed, first, second);
              rewritten.push_back(bypassed);
            }
          }

          ++ii;
          changed = true;
          block_changed = true;
          continue;
        }

        if(first.opcode == mir::Instruction::MI_MOV &&
           first.operands.size() == 2 &&
           first.operands[0].kind == mir::Operand::OP_REG &&
           first.operands[1].kind == mir::Operand::OP_REG &&
           second.opcode == mir::Instruction::MI_MOV &&
           second.operands.size() == 2 &&
           second.operands[0].kind == mir::Operand::OP_REG &&
           second.operands[1].kind == mir::Operand::OP_REG &&
           second.operands[1].reg == first.operands[0].reg &&
           live_after[ii + 1].regs.count(first.operands[0].reg) == 0 &&
           !debug_variable_depends_on_reg_after_instruction(function,
                                                            first.operands[0].reg,
                                                            second) &&
           combine_metadata_is_compatible(function, first, second)) {
          mir::Instruction combined = second;
          combined.operands[1] = first.operands[1];
          if(!is_droppable_self_move(function, combined)) {
            merge_artifact_metadata(combined, first, second);
            rewritten.push_back(combined);
          }
          ++ii;
          changed = true;
          block_changed = true;
          continue;
        }

        if(first.opcode == mir::Instruction::MI_MOV &&
           first.operands.size() == 2 &&
           first.operands[0].kind == mir::Operand::OP_REG &&
           first.operands[1].kind == mir::Operand::OP_REG &&
           second.opcode == mir::Instruction::MI_STORE &&
           second.operands.size() == 2 &&
           second.operands[1].kind == mir::Operand::OP_REG &&
           second.operands[1].reg == first.operands[0].reg &&
           live_after[ii + 1].regs.count(first.operands[0].reg) == 0 &&
           !debug_variable_depends_on_reg_after_instruction(function,
                                                            first.operands[0].reg,
                                                            second) &&
           combine_metadata_is_compatible(function, first, second)) {
          mir::Instruction combined = second;
          combined.operands[1] = first.operands[1];
          merge_artifact_metadata(combined, first, second);
          rewritten.push_back(combined);
          ++ii;
          changed = true;
          block_changed = true;
          continue;
        }
      }
      if(is_droppable_self_move(function, block.instructions[ii])) {
        changed = true;
        block_changed = true;
        continue;
      }
      rewritten.push_back(block.instructions[ii]);
    }

    if(block_changed) {
      block.instructions.swap(rewritten);
    }
  }
  return changed;
}

bool remove_dead_copy_moves(mir::Function & function)
{
  const vector<LiveState> live_out = compute_block_live_out(function);
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    LiveState live = live_out[bi];
    vector<mir::Instruction> kept;
    bool block_changed = false;
    for(size_t ii = function.blocks[bi].instructions.size(); ii-- > 0;) {
      const mir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(is_trivially_dead_move(inst) && !move_destination_is_live(inst, live)) {
        changed = true;
        block_changed = true;
        continue;
      }

      LiveState uses;
      LiveState defs;
      collect_instruction_effects(function, inst, uses, defs);
      subtract_live_state(live, defs);
      union_live_state(live, uses);
      kept.push_back(inst);
    }
    reverse(kept.begin(), kept.end());
    if(block_changed) {
      function.blocks[bi].instructions.swap(kept);
    }
  }
  return changed;
}

void optimize_local_copy_cleanup(mir::Function & function)
{
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      changed = propagate_block_local_copies(function.blocks[i]) || changed;
    }
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      changed = fold_block_local_frame_addresses(function.blocks[i]) || changed;
    }
    changed = remove_dead_copy_moves(function) || changed;
  }
}

void remove_fallthrough_jumps(mir::Function & function)
{
  for(size_t i = 0; i + 1 < function.blocks.size(); ++i) {
    mir::Block & block = function.blocks[i];
    if(block.instructions.empty()) {
      continue;
    }
    const mir::Instruction & tail = block.instructions.back();
    if(tail.opcode != mir::Instruction::MI_JMP ||
       tail.operands.size() != 1 ||
       tail.operands[0].kind != mir::Operand::OP_LABEL ||
       tail.operands[0].text != function.blocks[i + 1].label) {
      continue;
    }
    block.instructions.pop_back();
  }
}

bool rewrite_zero_compares_to_tests(mir::Function & function)
{
  bool changed = false;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    mir::Block & block = function.blocks[bi];
    for(size_t ii = 0; ii + 1 < block.instructions.size(); ++ii) {
      mir::Instruction & inst = block.instructions[ii];
      if(inst.opcode != mir::Instruction::MI_CMP ||
         inst.operands.size() != 2 ||
         inst.operands[0].kind != mir::Operand::OP_REG ||
         inst.operands[1].kind != mir::Operand::OP_IMM ||
         inst.operands[1].imm != 0 ||
         !uses_only_zero_flag(block.instructions[ii + 1])) {
        continue;
      }
      inst.opcode = mir::Instruction::MI_TEST;
      inst.operands[1] = inst.operands[0];
      changed = true;
    }
  }
  return changed;
}

void simplify_conditional_jump_fallthroughs(mir::Function & function)
{
  for(size_t i = 0; i + 1 < function.blocks.size(); ++i) {
    mir::Block & block = function.blocks[i];
    if(block.instructions.size() < 2) {
      continue;
    }
    const string & fallthrough_label = function.blocks[i + 1].label;
    const mir::Instruction & tail_jump = block.instructions.back();
    if(tail_jump.opcode != mir::Instruction::MI_JMP ||
       tail_jump.operands.size() != 1 ||
       tail_jump.operands[0].kind != mir::Operand::OP_LABEL) {
      continue;
    }

    mir::Instruction & cond_branch = block.instructions[block.instructions.size() - 2];
    string cond_target;
    if(!branch_target_label(cond_branch, cond_target)) {
      continue;
    }

    const string jump_target = tail_jump.operands[0].text;
    if(jump_target == fallthrough_label) {
      block.instructions.pop_back();
      continue;
    }
    if(cond_target != fallthrough_label) {
      continue;
    }
    if(!invert_conditional_branch(cond_branch)) {
      continue;
    }
    cond_branch.operands[0].text = jump_target;
    block.instructions.pop_back();
  }
}

bool improve_block_layout(mir::Function & function)
{
  if(function.blocks.size() < 2) {
    return false;
  }

  map<string, size_t> block_index;
  map<string, BlockControlFlow> original_flow;
  for(size_t i = 0; i < function.blocks.size(); ++i) {
    block_index[function.blocks[i].label] = i;
    original_flow[function.blocks[i].label] = analyze_block_control_flow(function, i);
  }

  vector<size_t> order;
  vector<bool> visited(function.blocks.size(), false);
  for(size_t start = 0; start < function.blocks.size(); ++start) {
    size_t current = start;
    while(current < function.blocks.size() && !visited[current]) {
      visited[current] = true;
      order.push_back(current);
      const BlockControlFlow & flow = original_flow.find(function.blocks[current].label)->second;
      size_t next = function.blocks.size();
      if(flow.unconditional_jump) {
        const map<string, size_t>::const_iterator found =
            block_index.find(flow.explicit_target_label);
        if(found != block_index.end() && !visited[found->second]) {
          next = found->second;
        }
      } else if(flow.has_fallthrough) {
        const map<string, size_t>::const_iterator found =
            block_index.find(flow.fallthrough_label);
        if(found != block_index.end() && !visited[found->second]) {
          next = found->second;
        }
      }
      current = next;
    }
  }

  bool changed = false;
  for(size_t i = 0; i < order.size(); ++i) {
    if(order[i] != i) {
      changed = true;
      break;
    }
  }
  if(!changed) {
    return false;
  }

  vector<mir::Block> reordered;
  reordered.reserve(function.blocks.size());
  for(size_t i = 0; i < order.size(); ++i) {
    reordered.push_back(function.blocks[order[i]]);
  }
  function.blocks.swap(reordered);

  for(size_t i = 0; i < function.blocks.size(); ++i) {
    mir::Block & block = function.blocks[i];
    const BlockControlFlow & flow = original_flow.find(block.label)->second;
    const string next_label = i + 1 < function.blocks.size()
        ? function.blocks[i + 1].label
        : string();
    if(block.instructions.empty()) {
      if(flow.has_fallthrough && flow.fallthrough_label != next_label) {
        mir::Instruction jump;
        jump.opcode = mir::Instruction::MI_JMP;
        jump.operands.push_back(mir::Operand());
        jump.operands.back().kind = mir::Operand::OP_LABEL;
        jump.operands.back().text = flow.fallthrough_label;
        block.instructions.push_back(jump);
      }
      continue;
    }

    mir::Instruction & tail = block.instructions.back();
    if(flow.unconditional_jump) {
      if(next_label == flow.explicit_target_label &&
         tail.opcode == mir::Instruction::MI_JMP) {
        block.instructions.pop_back();
      }
      continue;
    }

    if(flow.conditional_jump && is_conditional_branch(tail)) {
      if(next_label == flow.fallthrough_label) {
        continue;
      }
      if(next_label == flow.explicit_target_label &&
         flow.has_fallthrough &&
         invert_conditional_branch(tail)) {
        tail.operands[0].text = flow.fallthrough_label;
        continue;
      }
      if(flow.has_fallthrough) {
        mir::Instruction jump;
        jump.opcode = mir::Instruction::MI_JMP;
        jump.operands.push_back(mir::Operand());
        jump.operands.back().kind = mir::Operand::OP_LABEL;
        jump.operands.back().text = flow.fallthrough_label;
        block.instructions.push_back(jump);
      }
      continue;
    }

    if(flow.has_fallthrough &&
       !instruction_has_terminal_control_flow(tail) &&
       next_label != flow.fallthrough_label) {
      mir::Instruction jump;
      jump.opcode = mir::Instruction::MI_JMP;
      jump.operands.push_back(mir::Operand());
      jump.operands.back().kind = mir::Operand::OP_LABEL;
      jump.operands.back().text = flow.fallthrough_label;
      block.instructions.push_back(jump);
    }
  }

  return true;
}

size_t fixed_frame_bytes(const mir::Function & function)
{
  size_t out = 0;
  for(size_t i = 0; i < function.frame_bindings.size(); ++i) {
    const long long offset = function.frame_bindings[i].offset;
    out = max(out,
              static_cast<size_t>(offset < 0 ? -offset : offset));
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const mir::Instruction & inst = function.blocks[bi].instructions[ii];
      for(size_t oi = 0; oi < inst.operands.size(); ++oi) {
        const mir::Operand & operand = inst.operands[oi];
        if(operand.kind == mir::Operand::OP_DEREF &&
           operand.reg == XR_RBP &&
           operand.offset < 0) {
          out = max(out, static_cast<size_t>(-operand.offset));
        }
      }
    }
  }
  if(function.host_eh_enabled) {
    const long long offsets[] = {
      function.host_eh_exception_offset,
      function.host_eh_selector_offset
    };
    for(size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
      out = max(out,
                static_cast<size_t>(offsets[i] < 0 ? -offsets[i] : offsets[i]));
    }
  }
  if(!function.callee_saved_regs.empty()) {
    out = max(out, function.frame_bytes);
  }
  return align_up_size(out, 8);
}

bool minimize_callee_saved_preservation(mir::Function & function)
{
  if(function.callee_saved_regs.empty()) {
    return false;
  }

  set<X64Register> used;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const mir::Instruction & inst = function.blocks[bi].instructions[ii];
      for(size_t oi = 0; oi < inst.operands.size(); ++oi) {
        const mir::Operand & operand = inst.operands[oi];
        if((operand.kind == mir::Operand::OP_REG ||
            operand.kind == mir::Operand::OP_DEREF) &&
           find(function.callee_saved_regs.begin(),
                function.callee_saved_regs.end(),
                operand.reg) != function.callee_saved_regs.end()) {
          used.insert(operand.reg);
        }
      }
    }
  }

  vector<X64Register> kept;
  for(size_t i = 0; i < function.callee_saved_regs.size(); ++i) {
    if(used.count(function.callee_saved_regs[i]) != 0) {
      kept.push_back(function.callee_saved_regs[i]);
    }
  }
  if(kept == function.callee_saved_regs) {
    return false;
  }
  function.callee_saved_regs.swap(kept);
  function.stack_size = align_up_size(fixed_frame_bytes(function) +
                                      function.scratch_bytes +
                                      function.callee_saved_regs.size() * 8,
                                      16);
  return true;
}

}  // namespace

mir_public::MirProgram optimize_machine_ir_program(const mir_public::MirProgram & program,
                                                   int optimization_level)
{
  mir_public::MirProgram out = program;
  optimization_level = normalize_optimization_level(optimization_level);
  for(size_t i = 0; i < out.functions.size(); ++i) {
    while(combine_block_trivial_mov_artifacts(out.functions[i])) {
    }
    if(!function_has_real_debug_info(out.functions[i])) {
      remove_dead_copy_moves(out.functions[i]);
    }
    minimize_callee_saved_preservation(out.functions[i]);
  }
  if(optimization_level <= 0) {
    return out;
  }
  for(size_t i = 0; i < out.functions.size(); ++i) {
    optimize_local_copy_cleanup(out.functions[i]);
    rewrite_zero_compares_to_tests(out.functions[i]);
    simplify_conditional_jump_fallthroughs(out.functions[i]);
    remove_fallthrough_jumps(out.functions[i]);
    if(optimization_level >= 2) {
      if(improve_block_layout(out.functions[i])) {
        simplify_conditional_jump_fallthroughs(out.functions[i]);
        remove_fallthrough_jumps(out.functions[i]);
      }
    }
  }
  return out;
}
