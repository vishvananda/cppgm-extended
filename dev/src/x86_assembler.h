#pragma once

#include <cstdint>
#include <vector>

#include "x86_register_model.h"

struct X86Memory
{
  X86Memory(X64Register base_reg, std::int32_t displacement = 0)
      : base(base_reg), disp(displacement)
  {
  }

  X64Register base;
  std::int32_t disp;
};

class X86Assembler
{
public:
  explicit X86Assembler(bool count_only = false);

  void emit_mov_r32_imm32(X64Register dst, std::uint32_t imm);
  void emit_mov_r64_imm64(X64Register dst, std::uint64_t imm);
  void emit_mov_r32_r32(X64Register dst, X64Register src);
  void emit_mov_r64_r64(X64Register dst, X64Register src);
  void emit_movsxd_r64_r32(X64Register dst, X64Register src);
  void emit_mov_r64_m64(X64Register dst, const X86Memory & src);
  void emit_mov_r32_m32(X64Register dst, const X86Memory & src);
  void emit_movzx_r64_m8(X64Register dst, const X86Memory & src);
  void emit_movzx_r64_m16(X64Register dst, const X86Memory & src);
  void emit_mov_m8_r64(const X86Memory & dst, X64Register src);
  void emit_mov_m16_r64(const X86Memory & dst, X64Register src);
  void emit_mov_m32_r64(const X86Memory & dst, X64Register src);
  void emit_mov_m64_r64(const X86Memory & dst, X64Register src);
  void emit_movss_xmm_m32(XmmRegister dst, const X86Memory & src);
  void emit_movss_m32_xmm(const X86Memory & dst, XmmRegister src);
  void emit_movss_xmm_xmm(XmmRegister dst, XmmRegister src);
  void emit_movsd_xmm_m64(XmmRegister dst, const X86Memory & src);
  void emit_movsd_m64_xmm(const X86Memory & dst, XmmRegister src);
  void emit_movsd_xmm_xmm(XmmRegister dst, XmmRegister src);
  void emit_movd_xmm_r32(XmmRegister dst, X64Register src);
  void emit_movq_xmm_r64(XmmRegister dst, X64Register src);
  void emit_lock_xadd_m8_r64(const X86Memory & dst, X64Register src);
  void emit_lock_xadd_m16_r64(const X86Memory & dst, X64Register src);
  void emit_lock_xadd_m32_r64(const X86Memory & dst, X64Register src);
  void emit_lock_xadd_m64_r64(const X86Memory & dst, X64Register src);
  void emit_xchg_m8_r64(const X86Memory & dst, X64Register src);
  void emit_xchg_m16_r64(const X86Memory & dst, X64Register src);
  void emit_xchg_m32_r64(const X86Memory & dst, X64Register src);
  void emit_xchg_m64_r64(const X86Memory & dst, X64Register src);
  void emit_lock_cmpxchg_m8_r64(const X86Memory & dst, X64Register src);
  void emit_lock_cmpxchg_m16_r64(const X86Memory & dst, X64Register src);
  void emit_lock_cmpxchg_m32_r64(const X86Memory & dst, X64Register src);
  void emit_lock_cmpxchg_m64_r64(const X86Memory & dst, X64Register src);
  void emit_lock_cmpxchg16b_m128(const X86Memory & dst);
  void emit_lea_r64_m(X64Register dst, const X86Memory & src);
  std::size_t emit_lea_r64_rip_rel32_placeholder(X64Register dst);
  std::size_t emit_mov_r64_rip_rel32_placeholder(X64Register dst);

  void emit_xor_r64_r64(X64Register dst, X64Register src);
  void emit_and_r64_r64(X64Register dst, X64Register src);
  void emit_or_r64_r64(X64Register dst, X64Register src);
  void emit_add_r64_r64(X64Register dst, X64Register src);
  void emit_sub_r64_r64(X64Register dst, X64Register src);
  void emit_and_r64_imm32(X64Register dst, std::uint32_t imm);
  void emit_add_r64_imm32(X64Register dst, std::int32_t imm);
  void emit_sub_r64_imm32(X64Register dst, std::int32_t imm);
  void emit_cmp_r32_imm32(X64Register lhs, std::uint32_t imm);
  void emit_cmp_r64_imm32(X64Register lhs, std::int32_t imm);
  void emit_cmp_r32_r32(X64Register lhs, X64Register rhs);
  void emit_cmp_r64_r64(X64Register lhs, X64Register rhs);
  void emit_cmp_r32_m32(X64Register lhs, const X86Memory & rhs);
  void emit_cmp_r64_m64(X64Register lhs, const X86Memory & rhs);
  void emit_cmp_m8_imm8(const X86Memory & lhs, std::uint8_t imm);
  void emit_cmp_m16_imm16(const X86Memory & lhs, std::uint16_t imm);
  void emit_cmp_m32_imm32(const X86Memory & lhs, std::uint32_t imm);
  void emit_cmp_m64_imm32(const X86Memory & lhs, std::int32_t imm);
  void emit_cmp_m32_r32(const X86Memory & lhs, X64Register rhs);
  void emit_cmp_m64_r64(const X86Memory & lhs, X64Register rhs);
  void emit_test_r32_r32(X64Register lhs, X64Register rhs);
  void emit_test_r64_r64(X64Register lhs, X64Register rhs);
  void emit_neg_r64(X64Register reg);
  void emit_not_r64(X64Register reg);
  void emit_bswap_r32(X64Register reg);
  void emit_bswap_r64(X64Register reg);
  void emit_imul_r64_r64(X64Register dst, X64Register src);
  void emit_cqo();
  void emit_div_r64(X64Register src);
  void emit_idiv_r64(X64Register src);
  void emit_shl_r64_cl(X64Register reg);
  void emit_shr_r64_cl(X64Register reg);
  void emit_sar_r64_cl(X64Register reg);
  void emit_shl_r64_imm8(X64Register reg, unsigned char imm);
  void emit_shr_r64_imm8(X64Register reg, unsigned char imm);
  void emit_sar_r64_imm8(X64Register reg, unsigned char imm);
  void emit_setcc_r8(X86Condition cond, X64Register reg);

  void emit_fld_m32(const X86Memory & src);
  void emit_fld_m64(const X86Memory & src);
  void emit_fld_m80(const X86Memory & src);
  void emit_fild_m64(const X86Memory & src);
  void emit_fstp_m32(const X86Memory & dst);
  void emit_fstp_m64(const X86Memory & dst);
  void emit_fstp_m80(const X86Memory & dst);
  void emit_fisttp_m64(const X86Memory & dst);
  void emit_fchs();
  void emit_faddp_st1();
  void emit_fsubp_st1();
  void emit_fmulp_st1();
  void emit_fdivp_st1();
  void emit_fucomip_st1();
  void emit_fstp_st0();

  void emit_syscall();
  void emit_mfence();
  void emit_cld();
  void emit_rep_movsb();
  void emit_rep_stosb();
  void emit_ret();
  void emit_ud2();
  void emit_call_r64(X64Register reg);
  void emit_call_m64(const X86Memory & mem);
  std::size_t emit_call_rel32_placeholder();
  void emit_jmp_r64(X64Register reg);
  std::size_t emit_jmp_rel32_placeholder();
  std::size_t emit_jcc_rel32_placeholder(X86Condition cond);

  std::size_t size() const;
  std::size_t offset() const;
  void append(const std::vector<unsigned char> & bytes);
  void patch_u32(std::size_t offset, std::uint32_t value);
  void patch_rel32(std::size_t imm_offset, std::size_t target_offset);
  const std::vector<unsigned char> & bytes() const;

private:
  void emit_rex(bool w, int r, int x, int b);
  void emit_rex_byte(int r, int x, int b);
  void emit_modrm(int mod, int reg, int rm);
  void emit_sib(int scale, int index, int base);
  void emit_reg_reg(bool w, unsigned char opcode, X64Register dst, X64Register src);
  void emit_rm_reg(bool w,
                   unsigned char opcode,
                   const X86Memory & dst,
                   X64Register src);
  void emit_reg_rm(bool w,
                   unsigned char opcode,
                   X64Register dst,
                   const X86Memory & src);
  void emit_unary_rm64(unsigned char opcode_ext, X64Register reg);
  void emit_binary_r64_imm32(unsigned char opcode_ext,
                             X64Register reg,
                             std::int32_t imm);
  void emit_shift_r64(unsigned char opcode_ext, X64Register reg, bool use_cl);
  void emit_shift_r64_imm8(unsigned char opcode_ext,
                           X64Register reg,
                           unsigned char imm);
  void emit_x87_mem(unsigned char opcode, int reg_field, const X86Memory & mem);
  void emit_sse_mem(unsigned char prefix,
                    unsigned char opcode,
                    int reg_field,
                    const X86Memory & mem);
  void emit_lock_prefix();
  void emit_modrm_mem(int reg_field, const X86Memory & mem);
  void emit_u8(unsigned char value);
  void emit_u16(std::uint16_t value);
  void emit_u32(std::uint32_t value);
  void emit_u64(std::uint64_t value);

  bool count_only_;
  std::size_t size_;
  std::vector<unsigned char> bytes_;
};
