#include "x86_assembler.h"

#include <stdexcept>

using namespace std;

namespace {

bool fits_int8(std::int32_t value)
{
  return value >= -128 && value <= 127;
}

}  // namespace

X86Assembler::X86Assembler(bool count_only)
  : count_only_(count_only),
    size_(0)
{
}

void X86Assembler::emit_mov_r32_imm32(X64Register dst, std::uint32_t imm)
{
  emit_rex(false, 0, 0, dst);
  emit_u8(static_cast<unsigned char>(0xB8 + (dst & 7)));
  emit_u32(imm);
}

void X86Assembler::emit_mov_r64_imm64(X64Register dst, std::uint64_t imm)
{
  emit_rex(true, 0, 0, dst);
  emit_u8(static_cast<unsigned char>(0xB8 + (dst & 7)));
  emit_u64(imm);
}

void X86Assembler::emit_mov_r32_r32(X64Register dst, X64Register src)
{
  emit_reg_reg(false, 0x89, dst, src);
}

void X86Assembler::emit_mov_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x89, dst, src);
}

void X86Assembler::emit_movsxd_r64_r32(X64Register dst, X64Register src)
{
  emit_rex(true, dst, 0, src);
  emit_u8(0x63);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_mov_r64_m64(X64Register dst, const X86Memory & src)
{
  emit_rex(true, dst, 0, src.base);
  emit_u8(0x8B);
  emit_modrm_mem(dst & 7, src);
}

void X86Assembler::emit_mov_r32_m32(X64Register dst, const X86Memory & src)
{
  emit_rex(false, dst, 0, src.base);
  emit_u8(0x8B);
  emit_modrm_mem(dst & 7, src);
}

void X86Assembler::emit_movzx_r64_m8(X64Register dst, const X86Memory & src)
{
  emit_rex(true, dst, 0, src.base);
  emit_u8(0x0F);
  emit_u8(0xB6);
  emit_modrm_mem(dst & 7, src);
}

void X86Assembler::emit_movzx_r64_m16(X64Register dst, const X86Memory & src)
{
  emit_rex(true, dst, 0, src.base);
  emit_u8(0x0F);
  emit_u8(0xB7);
  emit_modrm_mem(dst & 7, src);
}

void X86Assembler::emit_mov_m8_r64(const X86Memory & dst, X64Register src)
{
  emit_rex_byte(src, 0, dst.base);
  emit_u8(0x88);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_mov_m16_r64(const X86Memory & dst, X64Register src)
{
  emit_u8(0x66);
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x89);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_mov_m32_r64(const X86Memory & dst, X64Register src)
{
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x89);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_mov_m64_r64(const X86Memory & dst, X64Register src)
{
  emit_rex(true, src, 0, dst.base);
  emit_u8(0x89);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_movss_xmm_m32(XmmRegister dst, const X86Memory & src)
{
  emit_sse_mem(0xF3, 0x10, dst, src);
}

void X86Assembler::emit_movss_m32_xmm(const X86Memory & dst, XmmRegister src)
{
  emit_sse_mem(0xF3, 0x11, src, dst);
}

void X86Assembler::emit_movss_xmm_xmm(XmmRegister dst, XmmRegister src)
{
  emit_u8(0xF3);
  emit_u8(0x0F);
  emit_u8(0x10);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_movsd_xmm_m64(XmmRegister dst, const X86Memory & src)
{
  emit_sse_mem(0xF2, 0x10, dst, src);
}

void X86Assembler::emit_movsd_m64_xmm(const X86Memory & dst, XmmRegister src)
{
  emit_sse_mem(0xF2, 0x11, src, dst);
}

void X86Assembler::emit_movsd_xmm_xmm(XmmRegister dst, XmmRegister src)
{
  emit_u8(0xF2);
  emit_u8(0x0F);
  emit_u8(0x10);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_movd_xmm_r32(XmmRegister dst, X64Register src)
{
  emit_u8(0x66);
  emit_rex(false, dst, 0, src);
  emit_u8(0x0F);
  emit_u8(0x6E);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_movq_xmm_r64(XmmRegister dst, X64Register src)
{
  emit_u8(0x66);
  emit_rex(true, dst, 0, src);
  emit_u8(0x0F);
  emit_u8(0x6E);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_lock_xadd_m8_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex_byte(src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xC0);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_xadd_m16_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_u8(0x66);
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xC1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_xadd_m32_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xC1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_xadd_m64_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex(true, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xC1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_xchg_m8_r64(const X86Memory & dst, X64Register src)
{
  emit_rex_byte(src, 0, dst.base);
  emit_u8(0x86);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_xchg_m16_r64(const X86Memory & dst, X64Register src)
{
  emit_u8(0x66);
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x87);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_xchg_m32_r64(const X86Memory & dst, X64Register src)
{
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x87);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_xchg_m64_r64(const X86Memory & dst, X64Register src)
{
  emit_rex(true, src, 0, dst.base);
  emit_u8(0x87);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_cmpxchg_m8_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex_byte(src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xB0);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_cmpxchg_m16_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_u8(0x66);
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xB1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_cmpxchg_m32_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex(false, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xB1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_cmpxchg_m64_r64(const X86Memory & dst, X64Register src)
{
  emit_lock_prefix();
  emit_rex(true, src, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xB1);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_lock_cmpxchg16b_m128(const X86Memory & dst)
{
  emit_lock_prefix();
  emit_rex(true, 0, 0, dst.base);
  emit_u8(0x0F);
  emit_u8(0xC7);
  emit_modrm_mem(1, dst);
}

void X86Assembler::emit_lea_r64_m(X64Register dst, const X86Memory & src)
{
  emit_rex(true, dst, 0, src.base);
  emit_u8(0x8D);
  emit_modrm_mem(dst & 7, src);
}

std::size_t X86Assembler::emit_lea_r64_rip_rel32_placeholder(X64Register dst)
{
  emit_rex(true, dst, 0, XR_RAX);
  emit_u8(0x8D);
  emit_modrm(0, dst & 7, 5);
  const std::size_t imm_offset = offset();
  emit_u32(0);
  return imm_offset;
}

std::size_t X86Assembler::emit_mov_r64_rip_rel32_placeholder(X64Register dst)
{
  emit_rex(true, dst, 0, XR_RAX);
  emit_u8(0x8B);
  emit_modrm(0, dst & 7, 5);
  const std::size_t imm_offset = offset();
  emit_u32(0);
  return imm_offset;
}

void X86Assembler::emit_xor_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x31, dst, src);
}

void X86Assembler::emit_and_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x21, dst, src);
}

void X86Assembler::emit_or_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x09, dst, src);
}

void X86Assembler::emit_add_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x01, dst, src);
}

void X86Assembler::emit_sub_r64_r64(X64Register dst, X64Register src)
{
  emit_reg_reg(true, 0x29, dst, src);
}

void X86Assembler::emit_and_r64_imm32(X64Register dst, std::uint32_t imm)
{
  emit_binary_r64_imm32(4, dst, static_cast<std::int32_t>(imm));
}

void X86Assembler::emit_or_r64_imm32(X64Register dst, std::uint32_t imm)
{
  emit_binary_r64_imm32(1, dst, static_cast<std::int32_t>(imm));
}

void X86Assembler::emit_add_r64_imm32(X64Register dst, std::int32_t imm)
{
  emit_binary_r64_imm32(0, dst, imm);
}

void X86Assembler::emit_sub_r64_imm32(X64Register dst, std::int32_t imm)
{
  emit_binary_r64_imm32(5, dst, imm);
}

void X86Assembler::emit_cmp_r32_imm32(X64Register lhs, std::uint32_t imm)
{
  emit_rex(false, 7, 0, lhs);
  emit_u8(0x81);
  emit_modrm(3, 7, lhs & 7);
  emit_u32(imm);
}

void X86Assembler::emit_cmp_r64_imm32(X64Register lhs, std::int32_t imm)
{
  emit_binary_r64_imm32(7, lhs, imm);
}

void X86Assembler::emit_cmp_r32_r32(X64Register lhs, X64Register rhs)
{
  emit_reg_reg(false, 0x39, lhs, rhs);
}

void X86Assembler::emit_cmp_r64_r64(X64Register lhs, X64Register rhs)
{
  emit_reg_reg(true, 0x39, lhs, rhs);
}

void X86Assembler::emit_cmp_r32_m32(X64Register lhs, const X86Memory & rhs)
{
  emit_reg_rm(false, 0x3B, lhs, rhs);
}

void X86Assembler::emit_cmp_r64_m64(X64Register lhs, const X86Memory & rhs)
{
  emit_reg_rm(true, 0x3B, lhs, rhs);
}

void X86Assembler::emit_cmp_m8_imm8(const X86Memory & lhs, std::uint8_t imm)
{
  emit_rex_byte(7, 0, lhs.base);
  emit_u8(0x80);
  emit_modrm_mem(7, lhs);
  emit_u8(imm);
}

void X86Assembler::emit_cmp_m16_imm16(const X86Memory & lhs, std::uint16_t imm)
{
  emit_u8(0x66);
  emit_rex(false, 7, 0, lhs.base);
  emit_u8(0x81);
  emit_modrm_mem(7, lhs);
  emit_u16(imm);
}

void X86Assembler::emit_cmp_m32_imm32(const X86Memory & lhs, std::uint32_t imm)
{
  emit_rex(false, 7, 0, lhs.base);
  emit_u8(0x81);
  emit_modrm_mem(7, lhs);
  emit_u32(imm);
}

void X86Assembler::emit_cmp_m64_imm32(const X86Memory & lhs, std::int32_t imm)
{
  emit_rex(true, 7, 0, lhs.base);
  emit_u8(0x81);
  emit_modrm_mem(7, lhs);
  emit_u32(static_cast<std::uint32_t>(imm));
}

void X86Assembler::emit_cmp_m32_r32(const X86Memory & lhs, X64Register rhs)
{
  emit_rm_reg(false, 0x39, lhs, rhs);
}

void X86Assembler::emit_cmp_m64_r64(const X86Memory & lhs, X64Register rhs)
{
  emit_rm_reg(true, 0x39, lhs, rhs);
}

void X86Assembler::emit_test_r32_r32(X64Register lhs, X64Register rhs)
{
  emit_reg_reg(false, 0x85, lhs, rhs);
}

void X86Assembler::emit_test_r64_r64(X64Register lhs, X64Register rhs)
{
  emit_reg_reg(true, 0x85, lhs, rhs);
}

void X86Assembler::emit_neg_r64(X64Register reg)
{
  emit_unary_rm64(3, reg);
}

void X86Assembler::emit_not_r64(X64Register reg)
{
  emit_unary_rm64(2, reg);
}

void X86Assembler::emit_bswap_r32(X64Register reg)
{
  emit_rex(false, 0, 0, reg);
  emit_u8(0x0F);
  emit_u8(static_cast<unsigned char>(0xC8 + (reg & 7)));
}

void X86Assembler::emit_bswap_r64(X64Register reg)
{
  emit_rex(true, 0, 0, reg);
  emit_u8(0x0F);
  emit_u8(static_cast<unsigned char>(0xC8 + (reg & 7)));
}

void X86Assembler::emit_imul_r64_r64(X64Register dst, X64Register src)
{
  emit_rex(true, dst, 0, src);
  emit_u8(0x0F);
  emit_u8(0xAF);
  emit_modrm(3, dst & 7, src & 7);
}

void X86Assembler::emit_cqo()
{
  emit_u8(0x48);
  emit_u8(0x99);
}

void X86Assembler::emit_div_r64(X64Register src)
{
  emit_rex(true, 6, 0, src);
  emit_u8(0xF7);
  emit_modrm(3, 6, src & 7);
}

void X86Assembler::emit_idiv_r64(X64Register src)
{
  emit_rex(true, 7, 0, src);
  emit_u8(0xF7);
  emit_modrm(3, 7, src & 7);
}

void X86Assembler::emit_shl_r64_cl(X64Register reg)
{
  emit_shift_r64(4, reg, true);
}

void X86Assembler::emit_shr_r64_cl(X64Register reg)
{
  emit_shift_r64(5, reg, true);
}

void X86Assembler::emit_sar_r64_cl(X64Register reg)
{
  emit_shift_r64(7, reg, true);
}

void X86Assembler::emit_shl_r64_imm8(X64Register reg, unsigned char imm)
{
  emit_shift_r64_imm8(4, reg, imm);
}

void X86Assembler::emit_shr_r64_imm8(X64Register reg, unsigned char imm)
{
  emit_shift_r64_imm8(5, reg, imm);
}

void X86Assembler::emit_sar_r64_imm8(X64Register reg, unsigned char imm)
{
  emit_shift_r64_imm8(7, reg, imm);
}

void X86Assembler::emit_setcc_r8(X86Condition cond, X64Register reg)
{
  emit_rex_byte(0, 0, reg);
  emit_u8(0x0F);
  emit_u8(static_cast<unsigned char>(0x90 + cond));
  emit_modrm(3, 0, reg & 7);
}

void X86Assembler::emit_fld_m32(const X86Memory & src)
{
  emit_x87_mem(0xD9, 0, src);
}

void X86Assembler::emit_fld_m64(const X86Memory & src)
{
  emit_x87_mem(0xDD, 0, src);
}

void X86Assembler::emit_fld_m80(const X86Memory & src)
{
  emit_x87_mem(0xDB, 5, src);
}

void X86Assembler::emit_fild_m16(const X86Memory & src)
{
  emit_x87_mem(0xDF, 0, src);
}

void X86Assembler::emit_fild_m32(const X86Memory & src)
{
  emit_x87_mem(0xDB, 0, src);
}

void X86Assembler::emit_fild_m64(const X86Memory & src)
{
  emit_x87_mem(0xDF, 5, src);
}

void X86Assembler::emit_fstp_m32(const X86Memory & dst)
{
  emit_x87_mem(0xD9, 3, dst);
}

void X86Assembler::emit_fstp_m64(const X86Memory & dst)
{
  emit_x87_mem(0xDD, 3, dst);
}

void X86Assembler::emit_fstp_m80(const X86Memory & dst)
{
  emit_x87_mem(0xDB, 7, dst);
}

void X86Assembler::emit_fisttp_m64(const X86Memory & dst)
{
  emit_x87_mem(0xDD, 1, dst);
}

void X86Assembler::emit_fldz()
{
  emit_u8(0xD9);
  emit_u8(0xEE);
}

void X86Assembler::emit_fchs()
{
  emit_u8(0xD9);
  emit_u8(0xE0);
}

void X86Assembler::emit_faddp_st1()
{
  emit_u8(0xDE);
  emit_u8(0xC1);
}

void X86Assembler::emit_fsubp_st1()
{
  emit_u8(0xDE);
  emit_u8(0xE9);
}

void X86Assembler::emit_fmulp_st1()
{
  emit_u8(0xDE);
  emit_u8(0xC9);
}

void X86Assembler::emit_fdivp_st1()
{
  emit_u8(0xDE);
  emit_u8(0xF9);
}

void X86Assembler::emit_fucomip_st1()
{
  emit_u8(0xDF);
  emit_u8(0xE9);
}

void X86Assembler::emit_fstp_st0()
{
  emit_u8(0xDD);
  emit_u8(0xD8);
}

void X86Assembler::emit_syscall()
{
  emit_u8(0x0F);
  emit_u8(0x05);
}

void X86Assembler::emit_mfence()
{
  emit_u8(0x0F);
  emit_u8(0xAE);
  emit_u8(0xF0);
}

void X86Assembler::emit_cld()
{
  emit_u8(0xFC);
}

void X86Assembler::emit_rep_movsb()
{
  emit_u8(0xF3);
  emit_u8(0xA4);
}

void X86Assembler::emit_rep_stosb()
{
  emit_u8(0xF3);
  emit_u8(0xAA);
}

void X86Assembler::emit_ret()
{
  emit_u8(0xC3);
}

void X86Assembler::emit_ud2()
{
  emit_u8(0x0F);
  emit_u8(0x0B);
}

void X86Assembler::emit_call_r64(X64Register reg)
{
  emit_rex(false, 0, 0, reg);
  emit_u8(0xFF);
  emit_modrm(3, 2, reg & 7);
}

void X86Assembler::emit_call_m64(const X86Memory & mem)
{
  emit_rex(false, 0, 0, mem.base);
  emit_u8(0xFF);
  emit_modrm_mem(2, mem);
}

std::size_t X86Assembler::emit_call_rel32_placeholder()
{
  emit_u8(0xE8);
  std::size_t imm_offset = offset();
  emit_u32(0);
  return imm_offset;
}

void X86Assembler::emit_jmp_r64(X64Register reg)
{
  emit_rex(false, 0, 0, reg);
  emit_u8(0xFF);
  emit_modrm(3, 4, reg & 7);
}

std::size_t X86Assembler::emit_jmp_rel32_placeholder()
{
  emit_u8(0xE9);
  std::size_t imm_offset = offset();
  emit_u32(0);
  return imm_offset;
}

std::size_t X86Assembler::emit_jcc_rel32_placeholder(X86Condition cond)
{
  emit_u8(0x0F);
  emit_u8(static_cast<unsigned char>(0x80 + cond));
  std::size_t imm_offset = offset();
  emit_u32(0);
  return imm_offset;
}

std::size_t X86Assembler::size() const
{
  return size_;
}

std::size_t X86Assembler::offset() const
{
  return size_;
}

void X86Assembler::append(const std::vector<unsigned char> & bytes)
{
  size_ += bytes.size();
  if(!count_only_) {
    bytes_.insert(bytes_.end(), bytes.begin(), bytes.end());
  }
}

void X86Assembler::patch_u32(std::size_t patch_offset, std::uint32_t value)
{
  if(patch_offset + 4 > size_) {
    throw logic_error("x86 patch_u32 out of range");
  }
  if(count_only_) {
    return;
  }
  for(std::size_t i = 0; i < 4; ++i) {
    bytes_[patch_offset + i] =
        static_cast<unsigned char>((value >> (8 * i)) & 0xFF);
  }
}

void X86Assembler::patch_u64(std::size_t patch_offset, std::uint64_t value)
{
  if(patch_offset + 8 > size_) {
    throw logic_error("x86 patch_u64 out of range");
  }
  if(count_only_) {
    return;
  }
  for(std::size_t i = 0; i < 8; ++i) {
    bytes_[patch_offset + i] =
        static_cast<unsigned char>((value >> (8 * i)) & 0xFF);
  }
}

void X86Assembler::patch_rel32(std::size_t imm_offset, std::size_t target_offset)
{
  if(imm_offset + 4 > size_) {
    throw logic_error("x86 patch_rel32 out of range");
  }
  if(count_only_) {
    return;
  }
  std::int64_t disp = static_cast<std::int64_t>(target_offset)
                    - static_cast<std::int64_t>(imm_offset + 4);
  if(disp < INT32_MIN || disp > INT32_MAX) {
    throw logic_error("x86 rel32 target out of range");
  }
  patch_u32(imm_offset, static_cast<std::uint32_t>(static_cast<std::int32_t>(disp)));
}

const std::vector<unsigned char> & X86Assembler::bytes() const
{
  return bytes_;
}

void X86Assembler::emit_rex(bool w, int r, int x, int b)
{
  unsigned char rex = 0x40;
  if(w) {
    rex |= 0x08;
  }
  if((r & 8) != 0) {
    rex |= 0x04;
  }
  if((x & 8) != 0) {
    rex |= 0x02;
  }
  if((b & 8) != 0) {
    rex |= 0x01;
  }
  if(rex != 0x40) {
    emit_u8(rex);
  }
}

void X86Assembler::emit_rex_byte(int r, int x, int b)
{
  unsigned char rex = 0x40;
  if((r & 8) != 0) {
    rex |= 0x04;
  }
  if((x & 8) != 0) {
    rex |= 0x02;
  }
  if((b & 8) != 0) {
    rex |= 0x01;
  }
  emit_u8(rex);
}

void X86Assembler::emit_modrm(int mod, int reg, int rm)
{
  emit_u8(static_cast<unsigned char>(((mod & 3) << 6) |
                                     ((reg & 7) << 3) |
                                     (rm & 7)));
}

void X86Assembler::emit_sib(int scale, int index, int base)
{
  emit_u8(static_cast<unsigned char>(((scale & 3) << 6) |
                                     ((index & 7) << 3) |
                                     (base & 7)));
}

void X86Assembler::emit_reg_reg(bool w,
                                unsigned char opcode,
                                X64Register dst,
                                X64Register src)
{
  emit_rex(w, src, 0, dst);
  emit_u8(opcode);
  emit_modrm(3, src & 7, dst & 7);
}

void X86Assembler::emit_rm_reg(bool w,
                               unsigned char opcode,
                               const X86Memory & dst,
                               X64Register src)
{
  emit_rex(w, src, 0, dst.base);
  emit_u8(opcode);
  emit_modrm_mem(src & 7, dst);
}

void X86Assembler::emit_reg_rm(bool w,
                               unsigned char opcode,
                               X64Register dst,
                               const X86Memory & src)
{
  emit_rex(w, dst, 0, src.base);
  emit_u8(opcode);
  emit_modrm_mem(dst & 7, src);
}

void X86Assembler::emit_unary_rm64(unsigned char opcode_ext, X64Register reg)
{
  emit_rex(true, opcode_ext, 0, reg);
  emit_u8(0xF7);
  emit_modrm(3, opcode_ext & 7, reg & 7);
}

void X86Assembler::emit_binary_r64_imm32(unsigned char opcode_ext,
                                         X64Register reg,
                                         std::int32_t imm)
{
  emit_rex(true, opcode_ext, 0, reg);
  emit_u8(0x81);
  emit_modrm(3, opcode_ext & 7, reg & 7);
  emit_u32(static_cast<std::uint32_t>(imm));
}

void X86Assembler::emit_shift_r64(unsigned char opcode_ext,
                                  X64Register reg,
                                  bool use_cl)
{
  emit_rex(true, opcode_ext, 0, reg);
  emit_u8(use_cl ? 0xD3 : 0xD1);
  emit_modrm(3, opcode_ext & 7, reg & 7);
}

void X86Assembler::emit_shift_r64_imm8(unsigned char opcode_ext,
                                       X64Register reg,
                                       unsigned char imm)
{
  emit_rex(true, opcode_ext, 0, reg);
  emit_u8(0xC1);
  emit_modrm(3, opcode_ext & 7, reg & 7);
  emit_u8(imm);
}

void X86Assembler::emit_x87_mem(unsigned char opcode,
                                int reg_field,
                                const X86Memory & mem)
{
  emit_rex(false, 0, 0, mem.base);
  emit_u8(opcode);
  emit_modrm_mem(reg_field, mem);
}

void X86Assembler::emit_sse_mem(unsigned char prefix,
                                unsigned char opcode,
                                int reg_field,
                                const X86Memory & mem)
{
  emit_u8(prefix);
  emit_rex(false, reg_field, 0, mem.base);
  emit_u8(0x0F);
  emit_u8(opcode);
  emit_modrm_mem(reg_field, mem);
}

void X86Assembler::emit_lock_prefix()
{
  emit_u8(0xF0);
}

void X86Assembler::emit_modrm_mem(int reg_field, const X86Memory & mem)
{
  int base_low = mem.base & 7;
  bool need_sib = base_low == 4;
  int mod = 0;

  if(mem.disp == 0 && base_low != 5) {
    mod = 0;
  } else if(fits_int8(mem.disp)) {
    mod = 1;
  } else {
    mod = 2;
  }

  if(base_low == 5 && mod == 0) {
    mod = 1;
  }

  emit_modrm(mod, reg_field, need_sib ? 4 : base_low);
  if(need_sib) {
    emit_sib(0, 4, base_low);
  }

  if(mod == 1) {
    emit_u8(static_cast<unsigned char>(mem.disp));
  } else if(mod == 2) {
    emit_u32(static_cast<std::uint32_t>(mem.disp));
  }
}

void X86Assembler::emit_u8(unsigned char value)
{
  ++size_;
  if(!count_only_) {
    bytes_.push_back(value);
  }
}

void X86Assembler::emit_u16(std::uint16_t value)
{
  emit_u8(static_cast<unsigned char>(value & 0xFF));
  emit_u8(static_cast<unsigned char>((value >> 8) & 0xFF));
}

void X86Assembler::emit_u32(std::uint32_t value)
{
  for(std::size_t i = 0; i < 4; ++i) {
    emit_u8(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}

void X86Assembler::emit_u64(std::uint64_t value)
{
  for(std::size_t i = 0; i < 8; ++i) {
    emit_u8(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}
