#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "native_format.h"
#include "x86_assembler.h"

namespace cy86_internal {

const int kNativeTemp0Disp = -16;
const int kNativeTemp1Disp = -32;
const int kNativeTemp2Disp = -48;

ProgramOutputTarget parse_output_target(const string & output_target)
{
  if(output_target.empty() ||
     output_target == "native" ||
     output_target == "host" ||
     output_target == "local") {
    return POT_NATIVE;
  }
  if(output_target == "linux") {
    return POT_LINUX;
  }
  if(output_target == "macos" ||
     output_target == "osx" ||
     output_target == "darwin") {
    return POT_MACOS;
  }
  throw logic_error("unknown output target: " + output_target);
}

struct NativeLayout
{
  vector<uint64_t> statement_offsets;
  LabelMap label_vaddrs;
  uint64_t image_base;
  uint64_t entry_offset;
  uint64_t payload_size;

  NativeLayout()
    : image_base(0),
      entry_offset(0),
      payload_size(0)
  {
  }
};

NativeTarget native_target_for_output(ProgramOutputTarget target)
{
  if(target == POT_LINUX) {
    return NT_LINUX;
  }
  if(target == POT_MACOS) {
    return NT_MACOS;
  }
#if defined(__APPLE__) && defined(__MACH__)
  return NT_MACOS;
#else
  return NT_LINUX;
#endif
}

uint64_t native_payload_offset()
{
  return 0x1000ULL;
}

uint64_t native_image_base(NativeTarget target)
{
  return native_format::hooks_for_target(target).base_vaddr + native_payload_offset();
}

X64Register native_full_register(const string & reg)
{
  if(reg == "sp") {
    return XR_RSP;
  }
  if(reg == "bp") {
    return XR_RBP;
  }
  if(reg.empty()) {
    throw logic_error("invalid CY86 register");
  }
  switch(reg[0]) {
  case 'x':
    return XR_R12;
  case 'y':
    return XR_R13;
  case 'z':
    return XR_R14;
  case 't':
    return XR_R15;
  default:
    throw logic_error("invalid CY86 register: " + reg);
  }
}

uint64_t evaluate_native_constant_expr(const AddressExpr & expr,
                                       const LabelMap & labels)
{
  uint64_t value = 0;
  if(expr.base_kind == EB_LITERAL) {
    value = convert_literal_to_uint64(expr.literal);
  } else if(expr.base_kind == EB_LABEL) {
    LabelMap::const_iterator it = labels.find(expr.name);
    if(it == labels.end()) {
      throw logic_error("unknown native label: " + expr.name);
    }
    value = it->second;
  } else {
    throw logic_error("native constant expression may not use register base");
  }

  if(expr.has_offset) {
    uint64_t offset = convert_literal_to_uint64(expr.offset);
    value = expr.subtract_offset ? value - offset : value + offset;
  }
  return value;
}

bool try_native_direct_memory(const AddressExpr & expr, X86Memory * memory)
{
  if(expr.base_kind != EB_REGISTER) {
    return false;
  }

  int64_t disp = 0;
  if(expr.has_offset) {
    uint64_t raw = convert_literal_to_uint64(expr.offset);
    if(expr.subtract_offset) {
      if(raw > 0x80000000ULL) {
        return false;
      }
      disp = -static_cast<int64_t>(raw);
    } else {
      if(raw > 0x7FFFFFFFULL) {
        return false;
      }
      disp = static_cast<int64_t>(raw);
    }
  }

  if(disp < INT32_MIN || disp > INT32_MAX) {
    return false;
  }

  *memory = X86Memory(native_full_register(expr.name),
                      static_cast<int32_t>(disp));
  return true;
}

void emit_native_sign_extend(X86Assembler & out,
                             X64Register reg,
                             size_t width_bytes)
{
  if(width_bytes >= 8) {
    return;
  }
  unsigned shift = static_cast<unsigned>(64 - width_bytes * 8);
  out.emit_shl_r64_imm8(reg, static_cast<unsigned char>(shift));
  out.emit_sar_r64_imm8(reg, static_cast<unsigned char>(shift));
}

void emit_native_mask_width(X86Assembler & out,
                            X64Register reg,
                            size_t width_bytes)
{
  if(width_bytes >= 8) {
    return;
  }
  if(width_bytes == 1) {
    out.emit_and_r64_imm32(reg, 0xFF);
  } else if(width_bytes == 2) {
    out.emit_and_r64_imm32(reg, 0xFFFF);
  } else if(width_bytes == 4) {
    out.emit_and_r64_imm32(reg, 0xFFFFFFFFU);
  }
}

void emit_native_load_address_expr(X86Assembler & out,
                                   const AddressExpr & expr,
                                   X64Register dst,
                                   X64Register tmp,
                                   const LabelMap & labels)
{
  if(expr.base_kind == EB_REGISTER) {
    X64Register full = native_full_register(expr.name);
    if(dst != full) {
      out.emit_mov_r64_r64(dst, full);
    }
  } else {
    out.emit_mov_r64_imm64(dst, evaluate_native_constant_expr(expr, labels));
    return;
  }

  if(expr.has_offset) {
    uint64_t offset = convert_literal_to_uint64(expr.offset);
    if((!expr.subtract_offset && offset <= 0x7FFFFFFFULL) ||
       (expr.subtract_offset && offset <= 0x80000000ULL)) {
      int32_t disp = expr.subtract_offset
          ? -static_cast<int32_t>(offset)
          : static_cast<int32_t>(offset);
      if(disp >= 0) {
        out.emit_add_r64_imm32(dst, disp);
      } else {
        out.emit_sub_r64_imm32(dst, -disp);
      }
    } else {
      out.emit_mov_r64_imm64(tmp, offset);
      if(expr.subtract_offset) {
        out.emit_sub_r64_r64(dst, tmp);
      } else {
        out.emit_add_r64_r64(dst, tmp);
      }
    }
  }
}

uint64_t native_immediate_value(const AddressExpr & expr,
                                size_t width_bytes,
                                const LabelMap & labels)
{
  if(expr.base_kind == EB_LITERAL && !expr.has_offset) {
    return decode_uint64(convert_literal_to_width(expr.literal, width_bytes),
                         width_bytes);
  }
  return evaluate_native_constant_expr(expr, labels) &
         mask_for_width(width_bytes);
}

void emit_native_load_uint_operand(X86Assembler & out,
                                   const Operand & operand,
                                   size_t width_bytes,
                                   X64Register dst,
                                   X64Register tmp,
                                   const LabelMap & labels)
{
  if(operand.kind == OPERAND_REGISTER) {
    X64Register full = native_full_register(operand.reg);
    if(dst != full) {
      out.emit_mov_r64_r64(dst, full);
    }
    emit_native_mask_width(out, dst, width_bytes);
    return;
  }

  if(operand.kind == OPERAND_IMMEDIATE) {
    out.emit_mov_r64_imm64(dst,
                           native_immediate_value(operand.expr,
                                                  width_bytes,
                                                  labels));
    return;
  }

  X86Memory memory(XR_RAX, 0);
  if(try_native_direct_memory(operand.expr, &memory)) {
    if(width_bytes == 1) {
      out.emit_movzx_r64_m8(dst, memory);
    } else if(width_bytes == 2) {
      out.emit_movzx_r64_m16(dst, memory);
    } else if(width_bytes == 4) {
      out.emit_mov_r32_m32(dst, memory);
    } else {
      out.emit_mov_r64_m64(dst, memory);
    }
    return;
  }

  emit_native_load_address_expr(out, operand.expr, tmp, dst, labels);
  memory = X86Memory(tmp, 0);
  if(width_bytes == 1) {
    out.emit_movzx_r64_m8(dst, memory);
  } else if(width_bytes == 2) {
    out.emit_movzx_r64_m16(dst, memory);
  } else if(width_bytes == 4) {
    out.emit_mov_r32_m32(dst, memory);
  } else {
    out.emit_mov_r64_m64(dst, memory);
  }
}

void emit_native_load_signed_operand(X86Assembler & out,
                                     const Operand & operand,
                                     size_t width_bytes,
                                     X64Register dst,
                                     X64Register tmp,
                                     const LabelMap & labels)
{
  emit_native_load_uint_operand(out, operand, width_bytes, dst, tmp, labels);
  emit_native_sign_extend(out, dst, width_bytes);
}

void emit_native_store_uint_operand(X86Assembler & out,
                                    const Operand & operand,
                                    size_t width_bytes,
                                    X64Register src,
                                    X64Register tmp1,
                                    X64Register tmp2,
                                    const LabelMap & labels)
{
  if(operand.kind == OPERAND_IMMEDIATE) {
    throw logic_error("native backend cannot write immediate operand");
  }

  if(operand.kind == OPERAND_REGISTER) {
    X64Register full = native_full_register(operand.reg);
    if(width_bytes == 8) {
      if(full != src) {
        out.emit_mov_r64_r64(full, src);
      }
      return;
    }
    if(width_bytes == 4) {
      if(full != src) {
        out.emit_mov_r64_r64(full, src);
      }
      out.emit_and_r64_imm32(full, 0xFFFFFFFFU);
      return;
    }

    out.emit_mov_r64_r64(tmp1, full);
    out.emit_and_r64_imm32(tmp1, width_bytes == 1 ? 0xFFFFFF00U
                                                  : 0xFFFF0000U);
    if(tmp2 != src) {
      out.emit_mov_r64_r64(tmp2, src);
    }
    emit_native_mask_width(out, tmp2, width_bytes);
    out.emit_or_r64_r64(tmp1, tmp2);
    out.emit_mov_r64_r64(full, tmp1);
    return;
  }

  X86Memory memory(XR_RAX, 0);
  if(!try_native_direct_memory(operand.expr, &memory)) {
    emit_native_load_address_expr(out, operand.expr, tmp1, tmp2, labels);
    memory = X86Memory(tmp1, 0);
  }

  if(width_bytes == 1) {
    out.emit_mov_m8_r64(memory, src);
  } else if(width_bytes == 2) {
    out.emit_mov_m16_r64(memory, src);
  } else if(width_bytes == 4) {
    out.emit_mov_m32_r64(memory, src);
  } else {
    out.emit_mov_m64_r64(memory, src);
  }
}

vector<unsigned char> native_immediate_bytes(const Operand & operand,
                                             size_t width_bytes,
                                             const LabelMap & labels)
{
  if(operand.kind != OPERAND_IMMEDIATE) {
    throw logic_error("native immediate bytes require immediate operand");
  }

  if(operand.expr.base_kind == EB_LITERAL &&
     !operand.expr.has_offset) {
    return convert_literal_to_width(operand.expr.literal, width_bytes);
  }

  if(width_bytes > 8) {
    throw logic_error("native float80 immediate requires literal operand");
  }

  return encode_uint64(evaluate_native_constant_expr(operand.expr, labels),
                       width_bytes);
}

void emit_native_write_bytes_to_memory(X86Assembler & out,
                                       const X86Memory & memory,
                                       const vector<unsigned char> & bytes,
                                       X64Register tmp)
{
  size_t offset = 0;
  while(offset < bytes.size()) {
    size_t remaining = bytes.size() - offset;
    size_t chunk = remaining >= 8 ? 8
                  : remaining >= 4 ? 4
                  : remaining >= 2 ? 2
                                   : 1;
    uint64_t value = 0;
    for(size_t i = 0; i < chunk; ++i) {
      value |= uint64_t(bytes[offset + i]) << (8 * i);
    }

    out.emit_mov_r64_imm64(tmp, value);
    X86Memory chunk_memory(memory.base,
                           memory.disp + static_cast<int32_t>(offset));
    if(chunk == 8) {
      out.emit_mov_m64_r64(chunk_memory, tmp);
    } else if(chunk == 4) {
      out.emit_mov_m32_r64(chunk_memory, tmp);
    } else if(chunk == 2) {
      out.emit_mov_m16_r64(chunk_memory, tmp);
    } else {
      out.emit_mov_m8_r64(chunk_memory, tmp);
    }

    offset += chunk;
  }
}

X86Memory resolve_native_memory_operand(X86Assembler & out,
                                        const Operand & operand,
                                        X64Register addr_tmp,
                                        X64Register scratch_tmp,
                                        const LabelMap & labels)
{
  if(operand.kind != OPERAND_MEMORY) {
    throw logic_error("native memory operand required");
  }

  X86Memory memory(XR_RAX, 0);
  if(try_native_direct_memory(operand.expr, &memory)) {
    return memory;
  }

  emit_native_load_address_expr(out, operand.expr, addr_tmp, scratch_tmp, labels);
  return X86Memory(addr_tmp, 0);
}

void emit_native_fld_operand(X86Assembler & out,
                             const Operand & operand,
                             size_t width_bytes,
                             int temp_disp,
                             X64Register value_tmp,
                             X64Register addr_tmp,
                             const LabelMap & labels)
{
  X86Memory memory(XR_RSP, temp_disp);

  if(operand.kind == OPERAND_MEMORY) {
    memory = resolve_native_memory_operand(out, operand, addr_tmp, value_tmp, labels);
  } else if(operand.kind == OPERAND_REGISTER) {
    if(width_bytes > 8) {
      throw logic_error("native backend cannot load float80 from register operand");
    }
    emit_native_load_uint_operand(out, operand, width_bytes, value_tmp, addr_tmp, labels);
    if(width_bytes == 4) {
      out.emit_mov_m32_r64(memory, value_tmp);
    } else {
      out.emit_mov_m64_r64(memory, value_tmp);
    }
  } else {
    emit_native_write_bytes_to_memory(out,
                                      memory,
                                      native_immediate_bytes(operand, width_bytes, labels),
                                      value_tmp);
  }

  if(width_bytes == 4) {
    out.emit_fld_m32(memory);
  } else if(width_bytes == 8) {
    out.emit_fld_m64(memory);
  } else {
    out.emit_fld_m80(memory);
  }
}

void emit_native_fstp_operand(X86Assembler & out,
                              const Operand & operand,
                              size_t width_bytes,
                              int temp_disp,
                              X64Register value_tmp,
                              X64Register addr_tmp,
                              X64Register scratch_tmp,
                              const LabelMap & labels)
{
  if(operand.kind == OPERAND_IMMEDIATE) {
    throw logic_error("native backend cannot write immediate operand");
  }

  if(operand.kind == OPERAND_MEMORY) {
    X86Memory memory = resolve_native_memory_operand(out,
                                                     operand,
                                                     addr_tmp,
                                                     scratch_tmp,
                                                     labels);
    if(width_bytes == 4) {
      out.emit_fstp_m32(memory);
    } else if(width_bytes == 8) {
      out.emit_fstp_m64(memory);
    } else {
      out.emit_fstp_m80(memory);
    }
    return;
  }

  if(width_bytes > 8) {
    throw logic_error("native backend cannot store float80 into register operand");
  }

  X86Memory memory(XR_RSP, temp_disp);
  if(width_bytes == 4) {
    out.emit_fstp_m32(memory);
    out.emit_mov_r32_m32(value_tmp, memory);
  } else {
    out.emit_fstp_m64(memory);
    out.emit_mov_r64_m64(value_tmp, memory);
  }

  emit_native_store_uint_operand(out,
                                 operand,
                                 width_bytes,
                                 value_tmp,
                                 addr_tmp,
                                 scratch_tmp,
                                 labels);
}

void emit_native_fld_f32_constant_bits(X86Assembler & out,
                                       uint32_t bits,
                                       int temp_disp,
                                       X64Register tmp)
{
  X86Memory memory(XR_RSP, temp_disp);
  out.emit_mov_r64_imm64(tmp, bits);
  out.emit_mov_m32_r64(memory, tmp);
  out.emit_fld_m32(memory);
}

uint64_t map_native_syscall_number(NativeTarget target, uint64_t linux_number)
{
  if(target == NT_LINUX) {
    return linux_number;
  }

  switch(linux_number) {
  case 0:
    return 0x2000003ULL;
  case 1:
    return 0x2000004ULL;
  case 9:
    return 0x20000C5ULL;
  case 60:
    return 0x2000001ULL;
  default:
    throw logic_error("unsupported CY86 syscall for macOS target");
  }
}

uint64_t map_native_mmap_flags(NativeTarget target, uint64_t linux_flags)
{
  if(target == NT_LINUX) {
    return linux_flags;
  }

  // Keep the CY86 source ABI Linux-shaped and translate the small mmap flag
  // subset used by the PA9 library helpers to the host kernel values.
  uint64_t native_flags = 0;
  if(linux_flags & 0x01ULL) {
    native_flags |= 0x01ULL;   // MAP_SHARED
  }
  if(linux_flags & 0x02ULL) {
    native_flags |= 0x02ULL;   // MAP_PRIVATE
  }
  if(linux_flags & 0x20ULL) {
    native_flags |= 0x1000ULL; // MAP_ANON on macOS
  }
  return native_flags;
}

vector<unsigned char> build_native_startup(bool has_entry,
                                           uint64_t entry_vaddr,
                                           NativeTarget target)
{
  X86Assembler out;
  out.emit_xor_r64_r64(XR_R12, XR_R12);
  out.emit_xor_r64_r64(XR_R13, XR_R13);
  out.emit_xor_r64_r64(XR_R14, XR_R14);
  out.emit_xor_r64_r64(XR_R15, XR_R15);
  out.emit_mov_r64_r64(XR_RBP, XR_RSP);

  if(!has_entry) {
    out.emit_mov_r64_imm64(XR_RAX, map_native_syscall_number(target, 60));
    out.emit_mov_r64_imm64(XR_RDI, 0);
    out.emit_syscall();
    out.emit_ud2();
    return out.bytes();
  }

  out.emit_mov_r64_imm64(XR_RAX, entry_vaddr);
  out.emit_jmp_r64(XR_RAX);
  return out.bytes();
}

vector<unsigned char> build_native_data_bytes(const Statement & statement,
                                              const LabelMap & labels)
{
  if(statement.kind == SK_LITERAL_DATA) {
    return evaluated_literal_bytes(statement.literal);
  }

  int width_bits = 0;
  parse_data_width(statement.opcode, &width_bits);
  size_t width_bytes = opcode_width_bytes(width_bits);

  if(statement.operands[0].kind == OPERAND_IMMEDIATE &&
     statement.operands[0].expr.base_kind == EB_LITERAL &&
     !statement.operands[0].expr.has_offset) {
    return convert_literal_to_width(statement.operands[0].expr.literal, width_bytes);
  }

  if(statement.operands[0].kind == OPERAND_IMMEDIATE) {
    uint64_t value = evaluate_native_constant_expr(statement.operands[0].expr,
                                                   labels);
    return encode_uint64(value, width_bytes);
  }

  throw logic_error("native data opcode requires immediate operand");
}

void emit_native_opcode(X86Assembler & out,
                        const Statement & statement,
                        const LabelMap & labels,
                        NativeTarget target)
{
  size_t width_bytes = opcode_width_bytes(statement.width_bits);

  switch(statement.exec_kind) {
  case EK_JUMP:
    emit_native_load_uint_operand(out, statement.operands[0], 8,
                                  XR_RAX, XR_R11, labels);
    out.emit_jmp_r64(XR_RAX);
    return;

  case EK_JUMPIF: {
    emit_native_load_uint_operand(out, statement.operands[0], 1,
                                  XR_RAX, XR_R11, labels);
    out.emit_test_r64_r64(XR_RAX, XR_RAX);
    size_t skip = out.emit_jcc_rel32_placeholder(XC_E);
    emit_native_load_uint_operand(out, statement.operands[1], 8,
                                  XR_RAX, XR_R11, labels);
    out.emit_jmp_r64(XR_RAX);
    out.patch_rel32(skip, out.offset());
    return;
  }

  case EK_CALL:
    emit_native_load_uint_operand(out, statement.operands[0], 8,
                                  XR_RAX, XR_R11, labels);
    out.emit_call_r64(XR_RAX);
    return;

  case EK_RET:
    out.emit_ret();
    return;

  case EK_SYSCALL: {
    static const X64Register kArgRegs[] = {
      XR_RDI, XR_RSI, XR_RDX, XR_R10, XR_R8, XR_R9
    };
    uint64_t linux_syscall_number = 0;

    if(target == NT_MACOS) {
      if(statement.operands[1].kind != OPERAND_IMMEDIATE) {
        throw logic_error("macOS CY86 syscalls currently require immediate syscall numbers");
      }
      linux_syscall_number = native_immediate_value(statement.operands[1].expr,
                                                    8,
                                                    labels);
      out.emit_mov_r64_imm64(XR_RAX,
                             map_native_syscall_number(
                                 target,
                                 linux_syscall_number));
    } else {
      emit_native_load_uint_operand(out, statement.operands[1], 8,
                                    XR_RAX, XR_R11, labels);
    }

    for(size_t i = 0; i < static_cast<size_t>(statement.syscall_arity); ++i) {
      if(target == NT_MACOS &&
         linux_syscall_number == 9 &&
         i == 3 &&
         statement.operands[i + 2].kind == OPERAND_IMMEDIATE) {
        out.emit_mov_r64_imm64(kArgRegs[i],
                               map_native_mmap_flags(
                                   target,
                                   native_immediate_value(statement.operands[i + 2].expr,
                                                          8,
                                                          labels)));
      } else {
        emit_native_load_uint_operand(out, statement.operands[i + 2], 8,
                                      kArgRegs[i], XR_R11, labels);
      }
    }

    out.emit_syscall();
    if(target == NT_MACOS) {
      size_t ok = out.emit_jcc_rel32_placeholder(XC_AE);
      out.emit_neg_r64(XR_RAX);
      out.patch_rel32(ok, out.offset());
    }
    emit_native_store_uint_operand(out, statement.operands[0], 8,
                                   XR_RAX, XR_R10, XR_R11, labels);
    return;
  }

  case EK_MOVE:
    if(width_bytes == 10) {
      throw logic_error("native float80 moves not implemented yet");
    }
    emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                  XR_RAX, XR_R11, labels);
    emit_native_store_uint_operand(out, statement.operands[0], width_bytes,
                                   XR_RAX, XR_R10, XR_R11, labels);
    return;

  case EK_NOT:
    emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                  XR_RAX, XR_R11, labels);
    out.emit_not_r64(XR_RAX);
    emit_native_store_uint_operand(out, statement.operands[0], width_bytes,
                                   XR_RAX, XR_R10, XR_R11, labels);
    return;

  case EK_BSWAP:
    emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                  XR_RAX, XR_R11, labels);
    if(width_bytes == 2) {
      out.emit_bswap_r32(XR_RAX);
      out.emit_shr_r64_imm8(XR_RAX, 16);
    } else if(width_bytes == 4) {
      out.emit_bswap_r32(XR_RAX);
    } else if(width_bytes == 8) {
      out.emit_bswap_r64(XR_RAX);
    } else {
      throw logic_error("native bswap unsupported width");
    }
    emit_native_store_uint_operand(out, statement.operands[0], width_bytes,
                                   XR_RAX, XR_R10, XR_R11, labels);
    return;

  case EK_AND:
  case EK_OR:
  case EK_XOR:
  case EK_IADD:
  case EK_ISUB:
  case EK_SMUL:
  case EK_UMUL:
  case EK_SDIV:
  case EK_UDIV:
  case EK_SMOD:
  case EK_UMOD:
  case EK_LSHIFT:
  case EK_SRSHIFT:
  case EK_URSHIFT: {
    size_t rhs_width = (statement.exec_kind == EK_LSHIFT ||
                        statement.exec_kind == EK_SRSHIFT ||
                        statement.exec_kind == EK_URSHIFT) ? 1
                                                            : width_bytes;
    emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                  XR_RAX, XR_R11, labels);
    emit_native_load_uint_operand(out, statement.operands[2], rhs_width,
                                  XR_RBX, XR_R11, labels);

    if(statement.exec_kind == EK_AND) {
      out.emit_and_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_OR) {
      out.emit_or_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_XOR) {
      out.emit_xor_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_IADD) {
      out.emit_add_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_ISUB) {
      out.emit_sub_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_SMUL ||
              statement.exec_kind == EK_UMUL) {
      out.emit_imul_r64_r64(XR_RAX, XR_RBX);
    } else if(statement.exec_kind == EK_SDIV ||
              statement.exec_kind == EK_SMOD) {
      emit_native_sign_extend(out, XR_RAX, width_bytes);
      emit_native_sign_extend(out, XR_RBX, width_bytes);
      out.emit_cqo();
      out.emit_idiv_r64(XR_RBX);
      if(statement.exec_kind == EK_SMOD) {
        out.emit_mov_r64_r64(XR_RAX, XR_RDX);
      }
    } else if(statement.exec_kind == EK_UDIV ||
              statement.exec_kind == EK_UMOD) {
      out.emit_xor_r64_r64(XR_RDX, XR_RDX);
      out.emit_div_r64(XR_RBX);
      if(statement.exec_kind == EK_UMOD) {
        out.emit_mov_r64_r64(XR_RAX, XR_RDX);
      }
    } else {
      out.emit_mov_r64_r64(XR_RCX, XR_RBX);
      if(statement.exec_kind == EK_SRSHIFT) {
        emit_native_sign_extend(out, XR_RAX, width_bytes);
        out.emit_sar_r64_cl(XR_RAX);
      } else if(statement.exec_kind == EK_URSHIFT) {
        out.emit_shr_r64_cl(XR_RAX);
      } else {
        out.emit_shl_r64_cl(XR_RAX);
      }
    }

    emit_native_store_uint_operand(out, statement.operands[0], width_bytes,
                                   XR_RAX, XR_R10, XR_R11, labels);
    return;
  }

  case EK_IEQ:
  case EK_INE:
  case EK_SLT:
  case EK_ULT:
  case EK_SGT:
  case EK_UGT:
  case EK_SLE:
  case EK_ULE:
  case EK_SGE:
  case EK_UGE: {
    if(statement.exec_kind == EK_SLT ||
       statement.exec_kind == EK_SGT ||
       statement.exec_kind == EK_SLE ||
       statement.exec_kind == EK_SGE) {
      emit_native_load_signed_operand(out, statement.operands[1], width_bytes,
                                      XR_RAX, XR_R11, labels);
      emit_native_load_signed_operand(out, statement.operands[2], width_bytes,
                                      XR_RBX, XR_R11, labels);
    } else {
      emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                    XR_RAX, XR_R11, labels);
      emit_native_load_uint_operand(out, statement.operands[2], width_bytes,
                                    XR_RBX, XR_R11, labels);
    }

    out.emit_cmp_r64_r64(XR_RAX, XR_RBX);
    X86Condition cond = XC_E;
    switch(statement.exec_kind) {
    case EK_IEQ: cond = XC_E; break;
    case EK_INE: cond = XC_NE; break;
    case EK_SLT: cond = XC_L; break;
    case EK_ULT: cond = XC_B; break;
    case EK_SGT: cond = XC_G; break;
    case EK_UGT: cond = XC_A; break;
    case EK_SLE: cond = XC_LE; break;
    case EK_ULE: cond = XC_BE; break;
    case EK_SGE: cond = XC_GE; break;
    case EK_UGE: cond = XC_AE; break;
    default: break;
    }

    out.emit_setcc_r8(cond, XR_RCX);
    emit_native_store_uint_operand(out, statement.operands[0], 1,
                                   XR_RCX, XR_R10, XR_R11, labels);
    return;
  }

  case EK_FADD:
  case EK_FSUB:
  case EK_FMUL:
  case EK_FDIV:
    emit_native_fld_operand(out, statement.operands[1], width_bytes,
                            kNativeTemp0Disp, XR_RAX, XR_R11, labels);
    emit_native_fld_operand(out, statement.operands[2], width_bytes,
                            kNativeTemp1Disp, XR_RAX, XR_R11, labels);
    if(statement.exec_kind == EK_FADD) {
      out.emit_faddp_st1();
    } else if(statement.exec_kind == EK_FSUB) {
      out.emit_fsubp_st1();
    } else if(statement.exec_kind == EK_FMUL) {
      out.emit_fmulp_st1();
    } else {
      out.emit_fdivp_st1();
    }
    emit_native_fstp_operand(out, statement.operands[0], width_bytes,
                             kNativeTemp2Disp, XR_RAX, XR_R10, XR_R11, labels);
    return;

  case EK_FEQ:
  case EK_FNE:
  case EK_FLT:
  case EK_FGT:
  case EK_FLE:
  case EK_FGE:
    emit_native_fld_operand(out, statement.operands[1], width_bytes,
                            kNativeTemp0Disp, XR_RAX, XR_R11, labels);
    emit_native_fld_operand(out, statement.operands[2], width_bytes,
                            kNativeTemp1Disp, XR_RAX, XR_R11, labels);
    out.emit_fucomip_st1();
    out.emit_fstp_st0();

    if(statement.exec_kind == EK_FEQ) {
      out.emit_setcc_r8(XC_E, XR_RCX);
      out.emit_setcc_r8(XC_NP, XR_RDX);
      out.emit_and_r64_r64(XR_RCX, XR_RDX);
    } else if(statement.exec_kind == EK_FNE) {
      out.emit_setcc_r8(XC_NE, XR_RCX);
      out.emit_setcc_r8(XC_P, XR_RDX);
      out.emit_or_r64_r64(XR_RCX, XR_RDX);
    } else if(statement.exec_kind == EK_FLT) {
      out.emit_setcc_r8(XC_A, XR_RCX);
    } else if(statement.exec_kind == EK_FGT) {
      out.emit_setcc_r8(XC_B, XR_RCX);
      out.emit_setcc_r8(XC_NP, XR_RDX);
      out.emit_and_r64_r64(XR_RCX, XR_RDX);
    } else if(statement.exec_kind == EK_FLE) {
      out.emit_setcc_r8(XC_AE, XR_RCX);
    } else {
      out.emit_setcc_r8(XC_BE, XR_RCX);
      out.emit_setcc_r8(XC_NP, XR_RDX);
      out.emit_and_r64_r64(XR_RCX, XR_RDX);
    }

    emit_native_store_uint_operand(out, statement.operands[0], 1,
                                   XR_RCX, XR_R10, XR_R11, labels);
    return;

  case EK_CONV_TO80:
    if(statement.category == 's') {
      emit_native_load_signed_operand(out, statement.operands[1], width_bytes,
                                      XR_RAX, XR_R11, labels);
      out.emit_mov_m64_r64(X86Memory(XR_RSP, kNativeTemp0Disp), XR_RAX);
      out.emit_fild_m64(X86Memory(XR_RSP, kNativeTemp0Disp));
    } else if(statement.category == 'u') {
      emit_native_load_uint_operand(out, statement.operands[1], width_bytes,
                                    XR_RAX, XR_R11, labels);
      out.emit_mov_m64_r64(X86Memory(XR_RSP, kNativeTemp0Disp), XR_RAX);
      out.emit_fild_m64(X86Memory(XR_RSP, kNativeTemp0Disp));
      if(width_bytes == 8) {
        out.emit_test_r64_r64(XR_RAX, XR_RAX);
        size_t skip_adjust = out.emit_jcc_rel32_placeholder(XC_NS);
        emit_native_fld_f32_constant_bits(out, 0x5F800000U,
                                          kNativeTemp1Disp, XR_RDX);
        out.emit_faddp_st1();
        out.patch_rel32(skip_adjust, out.offset());
      }
    } else {
      emit_native_fld_operand(out, statement.operands[1], width_bytes,
                              kNativeTemp0Disp, XR_RAX, XR_R11, labels);
    }

    emit_native_fstp_operand(out, statement.operands[0], 10,
                             kNativeTemp2Disp, XR_RAX, XR_R10, XR_R11, labels);
    return;

  case EK_CONV_FROM80:
    emit_native_fld_operand(out, statement.operands[1], 10,
                            kNativeTemp0Disp, XR_RAX, XR_R11, labels);
    if(statement.category == 's' || statement.category == 'u') {
      if(statement.category == 'u' && width_bytes == 8) {
        emit_native_fld_f32_constant_bits(out, 0x5F000000U,
                                          kNativeTemp1Disp, XR_RDX);
        out.emit_fucomip_st1();
        size_t direct_path = out.emit_jcc_rel32_placeholder(XC_A);

        emit_native_fld_f32_constant_bits(out, 0x5F000000U,
                                          kNativeTemp1Disp, XR_RDX);
        out.emit_fsubp_st1();
        out.emit_fisttp_m64(X86Memory(XR_RSP, kNativeTemp2Disp));
        out.emit_mov_r64_m64(XR_RAX, X86Memory(XR_RSP, kNativeTemp2Disp));
        out.emit_mov_r64_imm64(XR_RDX, 0x8000000000000000ULL);
        out.emit_or_r64_r64(XR_RAX, XR_RDX);
        size_t done_path = out.emit_jmp_rel32_placeholder();

        out.patch_rel32(direct_path, out.offset());
        out.emit_fisttp_m64(X86Memory(XR_RSP, kNativeTemp2Disp));
        out.emit_mov_r64_m64(XR_RAX, X86Memory(XR_RSP, kNativeTemp2Disp));
        out.patch_rel32(done_path, out.offset());
      } else {
        out.emit_fisttp_m64(X86Memory(XR_RSP, kNativeTemp2Disp));
        out.emit_mov_r64_m64(XR_RAX, X86Memory(XR_RSP, kNativeTemp2Disp));
      }

      emit_native_store_uint_operand(out, statement.operands[0], width_bytes,
                                     XR_RAX, XR_R10, XR_R11, labels);
    } else {
      emit_native_fstp_operand(out, statement.operands[0], width_bytes,
                               kNativeTemp2Disp, XR_RAX, XR_R10, XR_R11, labels);
    }
    return;

  case EK_NONE:
    break;
  }

  throw logic_error("unimplemented native opcode: " + statement.opcode);
}

vector<unsigned char> build_native_opcode_bytes(const Statement & statement,
                                                const LabelMap & labels,
                                                NativeTarget target)
{
  X86Assembler out;
  emit_native_opcode(out, statement, labels, target);
  return out.bytes();
}

size_t measure_native_opcode_size(const Statement & statement,
                                  const LabelMap & labels,
                                  NativeTarget target)
{
  X86Assembler out(true);
  emit_native_opcode(out, statement, labels, target);
  return out.size();
}

NativeLayout layout_native_program(const Program & program, NativeTarget target)
{
  NativeLayout layout;
  layout.statement_offsets.resize(program.statements.size(), 0);
  layout.image_base = native_image_base(target);

  LabelMap zero_labels;
  zero_labels.reserve(program.labels.size());
  layout.label_vaddrs.reserve(program.labels.size());
  for(size_t i = 0; i < program.statements.size(); ++i) {
    for(size_t j = 0; j < program.statements[i].labels.size(); ++j) {
      zero_labels[program.statements[i].labels[j]] = 0;
    }
  }

  vector<unsigned char> startup = build_native_startup(program.has_entry, 0, target);
  uint64_t offset = startup.size();

  for(size_t i = 0; i < program.statements.size(); ++i) {
    const Statement & statement = program.statements[i];
    offset = align_up(offset, statement.alignment);
    layout.statement_offsets[i] = offset;
    for(size_t j = 0; j < statement.labels.size(); ++j) {
      layout.label_vaddrs[statement.labels[j]] = layout.image_base + offset;
    }

    if(statement.kind == SK_OPCODE) {
      offset += measure_native_opcode_size(statement, zero_labels, target);
    } else {
      offset += statement.size;
    }
  }

  layout.payload_size = offset;
  if(program.has_entry) {
    LabelMap::const_iterator start = layout.label_vaddrs.find("start");
    layout.entry_offset = start == layout.label_vaddrs.end()
        ? layout.statement_offsets.front()
        : start->second - layout.image_base;
  }
  return layout;
}

vector<unsigned char> build_native_payload(const Program & program,
                                           const NativeLayout & layout,
                                           NativeTarget target)
{
  vector<unsigned char> payload(layout.payload_size, 0);

  vector<unsigned char> startup = build_native_startup(program.has_entry,
                                                       layout.image_base + layout.entry_offset,
                                                       target);
  copy(startup.begin(), startup.end(), payload.begin());

  for(size_t i = 0; i < program.statements.size(); ++i) {
    const Statement & statement = program.statements[i];
    vector<unsigned char> bytes = statement.kind == SK_OPCODE
        ? build_native_opcode_bytes(statement, layout.label_vaddrs, target)
        : build_native_data_bytes(statement, layout.label_vaddrs);
    copy(bytes.begin(),
         bytes.end(),
         payload.begin() + layout.statement_offsets[i]);
  }

  return payload;
}

void write_native_program(const Program & program,
                          const string & outfile,
                          NativeTarget target)
{
  NativeLayout layout = layout_native_program(program, target);
  vector<unsigned char> payload = build_native_payload(program, layout, target);
  const native_format::Hooks & format_hooks = native_format::hooks_for_target(target);
  format_hooks.write_x86_64_executable(outfile,
                                       payload,
                                       0,
                                       format_hooks.base_vaddr);
}

}  // namespace cy86_internal
