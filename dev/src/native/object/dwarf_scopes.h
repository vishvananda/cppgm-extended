#pragma once

#include "native/object/dwarf_types.h"
#include "support/flat_map.h"

namespace native {

// Per-function lexical PC ranges from typed scope transitions. Work is
// proportional to this function's scopes, source rows and emitted ranges.
class DwarfScopes
{
public:
	DwarfScopes(const lowir::Program& program, const DwarfLines::Function& function,
		const std::vector<DwarfLines::Variable>& variables, const std::vector<CodeLocation>& locations);
	void write(DwarfBytes& out, DwarfTypes& types, object::ObjectFile& object, uint32_t information, uint32_t ranges,
		uint32_t ranges_symbol, uint32_t function_symbol);

private:
	struct Hash
	{
		size_t operator()(uint32_t id) const { return id; }
	};
	struct Range
	{
		uint32_t begin, end;
	};
	struct Scope
	{
		uint32_t parent = 0, depth = 0, first_child = 0, next_child = 0, first_local = lowir::kNoId, start = 0;
		bool needed = false;
		std::vector<Range> ranges;
	};
	void collect(const lowir::DebugInfo& debug, const lowir::DebugFunction& function);
	void intervals(const DwarfLines::Function& function, const std::vector<CodeLocation>& locations);
	void transition(uint32_t from, uint32_t to, uint32_t pc);
	void entry(uint32_t id, DwarfBytes& out, DwarfTypes& types, object::ObjectFile& object, uint32_t information,
		uint32_t ranges, uint32_t ranges_symbol, uint32_t function_symbol);
	const std::vector<lowir::DebugLocal>& locals_;
	FlatMap<uint32_t, uint32_t, Hash> index_;
	std::vector<Scope> scopes_;
	std::vector<uint32_t> next_local_, entering_;
	std::vector<const DwarfLines::Variable*> variables_;
};
} // namespace native
