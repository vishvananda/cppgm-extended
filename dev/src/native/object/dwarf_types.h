#pragma once

#include "native/object/dwarf_bytes.h"
#include "native/object/dwarf_lines.h"

namespace native {

// Optional source types/local locations in the existing compilation unit.
// DIE references are resolved from typed ids after entries have been emitted.
class DwarfTypes
{
public:
	DwarfTypes(object::ObjectFile& object, const lowir::Program& program, uint32_t information, uint32_t strings,
		uint32_t strings_symbol);
	void abbreviations(DwarfBytes& out);
	void locals(DwarfBytes& out, lowir::SymbolId function, const std::vector<DwarfLines::Variable>& variables,
		uint32_t first, uint32_t count);
	void entries(DwarfBytes& out);
	void finish(DwarfBytes& out);

private:
	void entry(DwarfBytes& out, uint32_t id);
	void children(DwarfBytes& out, uint32_t parent);
	void name(DwarfBytes& out, lowir::NameId name);
	void reference(DwarfBytes& out, uint32_t type);
	void attributes(DwarfBytes& out, const lowir::DebugType& type, uint32_t id, uint32_t mask);
	object::ObjectFile& object_;
	const lowir::Program& program_;
	const lowir::DebugInfo& debug_;
	uint32_t information_, strings_, strings_symbol_;
	std::vector<uint64_t> specifications_;
	std::vector<uint32_t> codes_, first_child_, next_child_, offsets_;
	struct Reference
	{
		uint32_t offset;
		uint32_t type;
	};
	std::vector<Reference> references_;
};

} // namespace native
