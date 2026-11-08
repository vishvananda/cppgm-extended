#pragma once

#include <vector>

#include "lowir/assembly.h"
#include "lowir/program.h"
#include "native/mir/registers.h"

namespace native {

// Assembly-only keys add vector16..31 and mask0..7 to the ordinary GPR/XMM units.
// They never become ordinary MIR registers or widen its register sets.
inline bool assembly_vector_register(mir_model::Reg reg)
{
	return reg >= 16 && reg < 48;
}
inline bool assembly_mask_register(mir_model::Reg reg)
{
	return reg >= 48 && reg < 56;
}
inline mir_model::Reg assembly_register_key(const asm_model::Register& reg)
{
	using asm_model::RegisterBank;
	if (reg.bank == RegisterBank::Gpr && reg.number < 16)
		return reg.number;
	if (reg.bank == RegisterBank::Vector && reg.number < 32)
		return 16 + reg.number;
	if (reg.bank == RegisterBank::Mask && reg.number < 8)
		return 48 + reg.number;
	return mir_model::kNoReg;
}
inline uint64_t assembly_register_bit(mir_model::Reg reg)
{
	return uint64_t(1) << reg;
}
inline uint32_t ordinary_assembly_mask(mir_model::Reg reg)
{
	return reg < 32 ? mir_model::reg_bit(reg) : 0;
}

struct AssemblyBinding
{
	std::vector<mir_model::Reg> registers;
	std::vector<uint8_t> locations;
	std::vector<lowir::Literal> constants;
	std::vector<asm_model::SymbolReference> symbols;
	std::vector<uint32_t> sources; // constraint operand -> evaluated input operand
};

AssemblyBinding assign_assembly_operands(const lowir::AssemblyModule& module, const lowir::AssemblyTemplate& definition,
	uint64_t clobbers, const lowir::FunctionBody& body, const std::vector<lowir::Value>& arguments);

} // namespace native
