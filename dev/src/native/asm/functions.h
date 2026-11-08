#pragma once

#include "lowir/assembly.h"
#include "asm/module_expand.h"
#include "native/object/object_file.h"

namespace native {
class ModuleAssembler;
class AssemblyEnvironment;

// Raw functions share the module assembler's labels, fixups and CFI rules.
void emit_assembly_function(ModuleAssembler& assembler, const lowir::Program& program,
	const lowir::AssemblyFunction& function, const std::vector<uint32_t>& symbols, AssemblyEnvironment& environment);

} // namespace native
