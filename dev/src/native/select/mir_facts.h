#pragma once

#include <cstdint>
#include <vector>

#include "native/mir/mir.h"

// Facts about single MIR instructions that the passes after selection share.
namespace native {

// Whether the flags insts[i] sets are dead: a later instruction of the
// block sets them, or the block jumps away, before any reads them (the
// selector never keeps flags across blocks).
bool flags_dead_after(const std::vector<Inst>& insts, uint32_t i, uint32_t end);

bool plain_gpr(const Operand& op);

bool narrow_kind(lowir::TypeKind type);

bool wide_kind(lowir::TypeKind type);

// A plain instruction: the emitter encodes it without implicit registers
// (no scratch, call or fixed register), so it writes only its destination.
bool simple_inst(const Inst& inst);

// The GPR a simple instruction writes, or kNoReg.
Reg written_reg(const Inst& inst);

} // namespace native
