#include "machine_ir.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace machine_ir {

namespace {

const char * x86_condition_text(X86Condition cond)
{
  static const char * kCondText[] = {
    "o", "no", "b", "ae", "e", "ne", "be", "a",
    "s", "ns", "p", "np", "l", "ge", "le", "g"
  };
  return kCondText[cond];
}

bool is_float_type(const string & type)
{
  return type == "f32" || type == "f64" || type == "f80";
}

string int_width_type_text(size_t width_bytes)
{
  switch(width_bytes) {
    case 1:
      return "i8";
    case 2:
      return "i16";
    case 4:
      return "i32";
    case 8:
      return "i64";
    case 16:
      return "i128";
  }
  return "?";
}

string float_width_type_text(size_t width_bytes)
{
  switch(width_bytes) {
    case 4:
      return "f32";
    case 8:
      return "f64";
    case 10:
      return "f80";
  }
  return "?";
}

string float_text(long double value, const string & literal_text)
{
  if(!literal_text.empty()) {
    return literal_text;
  }
  ostringstream out;
  out << setprecision(20) << static_cast<double>(value);
  return out.str();
}

void dump_storage_span(ostream & out, const Instruction & inst)
{
  out << inst.byte_count;
  if(inst.byte_alignment != 0) {
    out << "x" << inst.byte_alignment;
  }
}

string operand_text(const Operand & operand)
{
  switch(operand.kind) {
    case Operand::OP_REG:
      return register_text(operand.reg);
    case Operand::OP_XMM:
      switch(operand.xmm) {
        case XMM_0: return "xmm0";
        case XMM_1: return "xmm1";
        case XMM_2: return "xmm2";
        case XMM_3: return "xmm3";
        case XMM_4: return "xmm4";
        case XMM_5: return "xmm5";
        case XMM_6: return "xmm6";
        case XMM_7: return "xmm7";
      }
      return "xmm?";
    case Operand::OP_IMM: {
      ostringstream out;
      out << operand.imm;
      return out.str();
    }
    case Operand::OP_FLOAT_IMM:
      return float_text(operand.float_imm, operand.text);
    case Operand::OP_SYMBOL:
    case Operand::OP_LABEL:
      return operand.text;
    case Operand::OP_FRAME: {
      ostringstream out;
      out << "[rbp";
      if(operand.offset < 0) {
        out << operand.offset;
      } else if(operand.offset > 0) {
        out << "+" << operand.offset;
      }
      out << "]";
      return out.str();
    }
    case Operand::OP_GLOBAL:
      return operand.text;
    case Operand::OP_DEREF: {
      ostringstream out;
      out << "[" << register_text(operand.reg);
      if(operand.offset < 0) {
        out << operand.offset;
      } else if(operand.offset > 0) {
        out << "+" << operand.offset;
      }
      out << "]";
      return out.str();
    }
  }
  return "?";
}

void dump_instruction(ostringstream & out, const Instruction & inst)
{
  out << "    ";
  switch(inst.opcode) {
    case Instruction::MI_MOV:
      out << "mov " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_LOAD:
      out << "load." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_STORE:
      out << "store." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_MFENCE:
      out << "mfence";
      break;
    case Instruction::MI_LOCK_XADD:
      out << "lock_xadd." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_XCHG:
      out << "xchg." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_LOCK_CMPXCHG:
      out << "lock_cmpxchg." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_LEA:
      out << "lea " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FMOV:
      out << "fmov." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FNEG:
      out << "fneg." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FADD:
      out << "fadd." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FSUB:
      out << "fsub." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FMUL:
      out << "fmul." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FDIV:
      out << "fdiv." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FEQ:
      out << "feq." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FNE:
      out << "fne." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FLT:
      out << "flt." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FGT:
      out << "fgt." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FLE:
      out << "fle." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FGE:
      out << "fge." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]) << ", "
          << operand_text(inst.operands[2]);
      break;
    case Instruction::MI_FCMP:
      out << "fcmp." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FSTP:
      out << "fstp." << inst.type << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_SITOFP:
      out << "sitofp." << int_width_type_text(inst.byte_count)
          << "." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_UITOFP:
      out << "uitofp." << int_width_type_text(inst.byte_count)
          << "." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FPTOSI:
      out << "fptosi." << inst.type
          << "." << int_width_type_text(inst.byte_count) << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FPTOUI:
      out << "fptoui." << inst.type
          << "." << int_width_type_text(inst.byte_count) << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FPEXT:
      out << "fpext." << float_width_type_text(inst.byte_count)
          << "." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_FPTRUNC:
      out << "fptrunc." << float_width_type_text(inst.byte_count)
          << "." << inst.type << " "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_ADD:
      out << "add " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_SUB:
      out << "sub " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_IMUL:
      out << "imul " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_AND:
      out << "and " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_OR:
      out << "or " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_XOR:
      out << "xor " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_NEG:
      out << "neg " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_NOT:
      out << "not " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_BSWAP:
      out << "bswap " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_CMP:
      out << "cmp";
      if(!inst.type.empty()) {
        out << "." << inst.type;
      }
      out << " " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_TEST:
      out << "test";
      if(!inst.type.empty()) {
        out << "." << inst.type;
      }
      out << " " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_JCC:
      out << "j" << x86_condition_text(inst.condition) << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_SETCC:
      out << "set" << x86_condition_text(inst.condition) << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_MOVZX:
      out << "movzx " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_SEXT:
      out << "sext." << int_width_type_text(inst.byte_count) << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_ZEXT:
      out << "zext." << int_width_type_text(inst.byte_count) << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_CQO:
      out << "cqo";
      break;
    case Instruction::MI_IDIV:
      out << "idiv " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_DIV:
      out << "div " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_SHL_CL:
      out << "shl " << operand_text(inst.operands[0]) << ", cl";
      break;
    case Instruction::MI_SHR_CL:
      out << "shr " << operand_text(inst.operands[0]) << ", cl";
      break;
    case Instruction::MI_SAR_CL:
      out << "sar " << operand_text(inst.operands[0]) << ", cl";
      break;
    case Instruction::MI_TLS_ADDR:
      out << "tls_addr " << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_CALL:
      out << "call " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_CALL_INDIRECT:
      out << "call *" << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_COPY_BYTES:
      out << "copy_bytes ";
      dump_storage_span(out, inst);
      out << ", "
          << operand_text(inst.operands[0]) << ", "
          << operand_text(inst.operands[1]);
      break;
    case Instruction::MI_ZERO_BYTES:
      out << "zero_bytes ";
      dump_storage_span(out, inst);
      out << ", "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_EH_PUSH:
      out << "eh_push " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_EH_POP:
      out << "eh_pop";
      break;
    case Instruction::MI_LOAD_EXCEPTION:
      out << "load_exception." << inst.type << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_LOAD_EXCEPTION_SELECTOR:
      out << "load_exception_selector." << inst.type << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_THROW:
      out << "throw " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_RESUME:
      out << "resume";
      break;
    case Instruction::MI_JMP:
      out << "jmp " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_JMP_INDIRECT:
      out << "jmp *" << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_JNE:
      out << "jne " << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_FRET:
      out << "fret." << inst.type << " "
          << operand_text(inst.operands[0]);
      break;
    case Instruction::MI_RET:
      if(inst.operands.empty()) {
        out << "ret";
      } else {
        out << "ret " << operand_text(inst.operands[0]);
      }
      break;
    case Instruction::MI_EXIT:
      out << "exit";
      break;
  }
  if(inst.debug_location.present()) {
    out << " !dbg(" << inst.debug_location.file
        << ", " << inst.debug_location.line
        << ", " << inst.debug_location.column << ")";
  }
  out << "\n";
}

}  // namespace

const char * register_text(X64Register reg)
{
  switch(reg) {
    case XR_RAX: return "rax";
    case XR_RCX: return "rcx";
    case XR_RDX: return "rdx";
    case XR_RBX: return "rbx";
    case XR_RSP: return "rsp";
    case XR_RBP: return "rbp";
    case XR_RSI: return "rsi";
    case XR_RDI: return "rdi";
    case XR_R8: return "r8";
    case XR_R9: return "r9";
    case XR_R10: return "r10";
    case XR_R11: return "r11";
    case XR_R12: return "r12";
    case XR_R13: return "r13";
    case XR_R14: return "r14";
    case XR_R15: return "r15";
  }
  return "rx";
}

string dump_program(const Program & program)
{
  ostringstream out;
  out << "machine_ir x86_64 " << program.target << "\n";

  if(!program.startup.empty()) {
    out << "\nstartup\n";
    for(size_t i = 0; i < program.startup.size(); ++i) {
      dump_instruction(out, program.startup[i]);
    }
  }

  for(size_t i = 0; i < program.globals.size(); ++i) {
    const GlobalDefinition & global = program.globals[i];
    out << "\n";
    out << "global " << global.name;
    if(global.readonly) {
      out << " readonly";
    }
    if(global.thread_local_storage) {
      out << " thread_local";
    }
    out << "\n";
    if(global.storage_kind == GlobalDefinition::GS_SCALAR) {
      out << "  storage scalar " << global.type << "\n";
      out << "  init ";
      switch(global.init_kind) {
        case GlobalDefinition::GI_ZERO:
          out << "zero";
          break;
        case GlobalDefinition::GI_INTEGER:
          out << global.type << " " << global.int_value;
          break;
        case GlobalDefinition::GI_FLOAT:
          out << global.type << " "
              << float_text(global.float_value, global.literal_text);
          break;
        case GlobalDefinition::GI_ADDR:
          out << "addr " << global.symbol;
          if(global.addr_addend != 0) {
            out << (global.addr_addend > 0 ? " + " : " - ")
                << (global.addr_addend > 0 ? global.addr_addend : -global.addr_addend);
          }
          break;
      }
      out << "\n";
      continue;
    }

    out << "  storage data\n";
    for(size_t j = 0; j < global.data_items.size(); ++j) {
      const GlobalDefinition::DataItem & item = global.data_items[j];
      out << "  item ";
      switch(item.kind) {
        case GlobalDefinition::DataItem::ITEM_INTEGER:
          out << item.type << " " << item.int_value;
          break;
        case GlobalDefinition::DataItem::ITEM_FLOAT:
          out << item.type << " "
              << float_text(item.float_value, item.literal_text);
          break;
        case GlobalDefinition::DataItem::ITEM_ADDR:
          out << "ptr addr " << item.symbol;
          if(item.addr_addend != 0) {
            out << (item.addr_addend > 0 ? " + " : " - ")
                << (item.addr_addend > 0 ? item.addr_addend : -item.addr_addend);
          }
          break;
        case GlobalDefinition::DataItem::ITEM_ZERO:
          out << "zero " << item.zero_bytes;
          break;
      }
      out << "\n";
    }
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    const Function & function = program.functions[i];
    out << "\n";
    out << "function " << function.name << "\n";
    out << "  abi\n";
    for(size_t j = 0; j < function.params.size(); ++j) {
      out << "    param " << function.params[j].name << " -> ";
      if(function.params[j].location == ParamBinding::PL_REG) {
        out << register_text(function.params[j].reg);
      } else if(function.params[j].location == ParamBinding::PL_XMM) {
        switch(function.params[j].xmm) {
          case XMM_0: out << "xmm0"; break;
          case XMM_1: out << "xmm1"; break;
          case XMM_2: out << "xmm2"; break;
          case XMM_3: out << "xmm3"; break;
          case XMM_4: out << "xmm4"; break;
          case XMM_5: out << "xmm5"; break;
          case XMM_6: out << "xmm6"; break;
          case XMM_7: out << "xmm7"; break;
        }
      } else {
        out << "[rbp";
        if(function.params[j].stack_offset > 0) {
          out << "+" << function.params[j].stack_offset;
        } else if(function.params[j].stack_offset < 0) {
          out << function.params[j].stack_offset;
        }
        out << "]";
      }
      out << " : " << function.params[j].type << "\n";
    }
    out << "    return " << function.return_type << " -> "
         << (function.return_type == "void"
                ? "void"
                : function.return_type == "f80" ? "st0"
                : is_float_type(function.return_type) ? "xmm0" : "rax")
         << "\n";
    out << "  frame\n";
    out << "    stack_size " << function.stack_size << "\n";
    out << "    scratch_bytes " << function.scratch_bytes << "\n";
    if(function.host_eh_enabled) {
      out << "    host_eh_exception -> [rbp";
      if(function.host_eh_exception_offset < 0) {
        out << function.host_eh_exception_offset;
      } else if(function.host_eh_exception_offset > 0) {
        out << "+" << function.host_eh_exception_offset;
      }
      out << "]\n";
      out << "    host_eh_selector -> [rbp";
      if(function.host_eh_selector_offset < 0) {
        out << function.host_eh_selector_offset;
      } else if(function.host_eh_selector_offset > 0) {
        out << "+" << function.host_eh_selector_offset;
      }
      out << "]\n";
    }
    if(!function.callee_saved_regs.empty()) {
      out << "    preserve";
      for(size_t j = 0; j < function.callee_saved_regs.size(); ++j) {
        out << " " << register_text(function.callee_saved_regs[j]);
      }
      out << "\n";
    }
    for(size_t j = 0; j < function.frame_bindings.size(); ++j) {
      const FrameBinding & binding = function.frame_bindings[j];
      const char * prefix = binding.kind == FrameBinding::FB_PARAM_SLOT
          ? "param-slot"
          : binding.kind == FrameBinding::FB_SLOT
              ? "slot"
              : "temp";
      out << "    " << prefix << " " << binding.name << " -> [rbp";
      if(binding.offset < 0) {
        out << binding.offset;
      } else if(binding.offset > 0) {
        out << "+" << binding.offset;
      }
      out << "] : " << binding.type << "\n";
    }
    for(size_t j = 0; j < function.blocks.size(); ++j) {
      out << "\n";
      out << "  block " << function.blocks[j].label << "\n";
      for(size_t k = 0; k < function.blocks[j].instructions.size(); ++k) {
        dump_instruction(out, function.blocks[j].instructions[k]);
      }
    }
  }

  return out.str();
}

}  // namespace machine_ir
