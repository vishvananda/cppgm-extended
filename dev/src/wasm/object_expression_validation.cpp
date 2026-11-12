#include "wasm/object_reader.h"

#include "wasm/validation.h"

namespace wasm {

void validate_constant_function(const Function& function, ValueType type, uint32_t globals, const Module& module)
{
	for (const Instruction& inst : function.instructions)
		if (inst.op == Op::GlobalGet)
		{
			const uint64_t index = function.operands[inst.operands.first];
			if (index >= globals || module.mutable_globals[index])
				throw WasmError("Wasm constant expression reads an absent, forward or mutable global");
		}
	validate_function(function, Signature{ {}, { type } }, module);
}

} // namespace wasm
