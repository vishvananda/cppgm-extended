#pragma once

#include <vector>

#include "lowir/assembly.h"
#include "lowir/program.h"
#include "native/mir/registers.h"

namespace native {

// Assembly-only keys extend the ordinary GPR/XMM units with vector16..31.
// They never become ordinary MIR registers or widen its register sets.
inline bool assembly_vector_register(mir_model::Reg reg)
{
	return reg >= 16 && reg < 48;
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
