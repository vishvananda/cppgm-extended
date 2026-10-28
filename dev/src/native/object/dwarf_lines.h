#pragma once

#include <vector>

#include "lowir/program.h"
#include "native/object/object_file.h"
#include "native/x86/code_location.h"

namespace native {

// Source rows retained after a streamed function's MIR and LowIR are
// released. No instance or row pool is created for a compilation without
// source locations.
class DwarfLines
{
public:
	void append(lowir::SymbolId symbol, uint32_t size, lowir::DebugLocation declaration,
		const std::vector<CodeLocation>& locations);
	void write(
		object::ObjectFile& object, const lowir::Program& program, const std::vector<uint32_t>& object_symbols) const;

	struct Function
	{
		lowir::SymbolId symbol;
		uint32_t size;
		lowir::DebugLocation declaration;
		uint32_t first;
		uint32_t count;
	};

private:
	std::vector<Function> functions_;
	std::vector<CodeLocation> locations_;
};

} // namespace native
