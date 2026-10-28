#pragma once

#include <vector>

#include "lowir/program.h"
#include "native/object/object_file.h"
#include "native/x86/code_location.h"

namespace native {

struct Function;

// Source rows retained after a streamed function's MIR and LowIR are
// released. No instance or row pool is created for a compilation without
// source locations.
class DwarfLines
{
public:
	void append(lowir::SymbolId symbol, uint32_t size, lowir::DebugLocation declaration,
		const std::vector<CodeLocation>& locations, const native::Function& machine);
	void write(
		object::ObjectFile& object, const lowir::Program& program, const std::vector<uint32_t>& object_symbols) const;

	struct Function
	{
		lowir::SymbolId symbol;
		uint32_t size;
		lowir::DebugLocation declaration;
		uint32_t first;
		uint32_t count;
		uint32_t first_variable;
		uint32_t variable_count;
	};

	struct Variable
	{
		lowir::SlotId slot;
		int32_t offset;
		bool aligned;
	};

private:
	std::vector<Variable> variables_;
	std::vector<Function> functions_;
	std::vector<CodeLocation> locations_;
};

} // namespace native
