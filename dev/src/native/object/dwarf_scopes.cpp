#include "native/object/dwarf_scopes.h"
#include "native/object/dwarf_format.h"

namespace native {

DwarfScopes::DwarfScopes(const lowir::Program& program, const DwarfLines::Function& function,
	const std::vector<DwarfLines::Variable>& variables, const std::vector<CodeLocation>& locations)
	: locals_(program.debug->functions[function.symbol].locals), scopes_(1), next_local_(locals_.size(), lowir::kNoId)
{
	collect(*program.debug, program.debug->functions[function.symbol]);
	uint32_t maximum = 0;
	for (const lowir::DebugLocal& local : locals_)
		maximum = std::max(maximum, local.slot + 1);
	variables_.resize(maximum, nullptr);
	for (uint32_t i = 0; i < function.variable_count; ++i)
	{
		const DwarfLines::Variable& variable = variables[function.first_variable + i];
		if (variable.slot < maximum)
			variables_[variable.slot] = &variable;
	}
	intervals(function, locations);
}

void DwarfScopes::collect(const lowir::DebugInfo& debug, const lowir::DebugFunction& function)
{
	for (uint32_t id = function.first_scope; id != 0; id = debug.scopes[id].next)
	{
		const uint32_t* found = index_.find(debug.scopes[id].parent);
		const uint32_t parent = found != nullptr ? *found : 0;
		const uint32_t index = static_cast<uint32_t>(scopes_.size());
		scopes_.push_back(Scope());
		scopes_.back().parent = parent;
		scopes_.back().depth = scopes_[parent].depth + 1;
		index_.insert(id, index);
	}
	for (const lowir::DebugLocal& local : locals_)
	{
		const uint32_t* found = index_.find(local.scope);
		scopes_[found != nullptr ? *found : 0].needed = true;
	}
	for (uint32_t i = static_cast<uint32_t>(scopes_.size()); --i != 0;)
		if (scopes_[i].needed)
			scopes_[scopes_[i].parent].needed = true;
	// Keep needed ancestors. Empty sibling scopes resolve to their nearest
	// needed ancestor when their source rows are encountered.
	for (uint32_t i = static_cast<uint32_t>(scopes_.size()); --i != 0;)
	{
		if (!scopes_[i].needed)
			continue;
		Scope& parent = scopes_[scopes_[i].parent];
		scopes_[i].next_child = parent.first_child;
		parent.first_child = i;
	}
	for (uint32_t i = static_cast<uint32_t>(locals_.size()); i-- != 0;)
	{
		const uint32_t* found = index_.find(locals_[i].scope);
		Scope& scope = scopes_[found != nullptr ? *found : 0];
		next_local_[i] = scope.first_local;
		scope.first_local = i;
	}
}

void DwarfScopes::transition(uint32_t from, uint32_t to, uint32_t pc)
{
	entering_.clear();
	while (from != to)
	{
		if (scopes_[from].depth >= scopes_[to].depth)
		{
			Scope& scope = scopes_[from];
			if (scope.start < pc)
				scope.ranges.push_back(Range{ scope.start, pc });
			from = scope.parent;
		}
		else
		{
			entering_.push_back(to);
			to = scopes_[to].parent;
		}
	}
	for (uint32_t id : entering_)
		scopes_[id].start = pc;
}

void DwarfScopes::intervals(const DwarfLines::Function& function, const std::vector<CodeLocation>& locations)
{
	// Cache the representative once per owned scope, never walk its ancestors
	// once per machine row (deep blocks and macros can share coordinates).
	std::vector<uint32_t> representative(scopes_.size(), 0);
	for (uint32_t i = 1; i < scopes_.size(); ++i)
		representative[i] = scopes_[i].needed ? i : representative[scopes_[i].parent];
	uint32_t current = 0;
	for (uint32_t i = 0; i < function.count; ++i)
	{
		const CodeLocation& row = locations[function.first + i];
		const uint32_t* found = index_.find(row.location.scope);
		const uint32_t next = found != nullptr ? representative[*found] : 0;
		if (current != next)
		{
			transition(current, next, row.offset);
			current = next;
		}
	}
	transition(current, 0, function.size);
}

void DwarfScopes::write(DwarfBytes& out, DwarfTypes& types, object::ObjectFile& object, uint32_t information,
	uint32_t ranges, uint32_t ranges_symbol, uint32_t function_symbol)
{
	entry(0, out, types, object, information, ranges, ranges_symbol, function_symbol);
}

void DwarfScopes::entry(uint32_t id, DwarfBytes& out, DwarfTypes& types, object::ObjectFile& object,
	uint32_t information, uint32_t ranges, uint32_t ranges_symbol, uint32_t function_symbol)
{
	const Scope& scope = scopes_[id];
	if (id != 0)
	{
		out.uleb(8);
		object::Section& info = object.sections[information];
		const uint32_t offset = static_cast<uint32_t>(object.sections[ranges].bytes.size());
		info.relocations.push_back(object::Relocation{ out.size(), object::R_X86_64_32, ranges_symbol, offset });
		out.fixed(0, 4);
		object::Section& section = object.sections[ranges];
		DwarfBytes bytes(section.bytes);
		for (const Range& range : scope.ranges)
			for (uint32_t pc : { range.begin, range.end })
			{
				section.relocations.push_back(
					object::Relocation{ bytes.size(), object::R_X86_64_64, function_symbol, pc });
				bytes.fixed(0, 8);
			}
		bytes.fixed(0, 8);
		bytes.fixed(0, 8);
	}
	for (uint32_t local = scope.first_local; local != lowir::kNoId; local = next_local_[local])
		types.local(out, locals_[local], variables_[locals_[local].slot]);
	for (uint32_t child = scope.first_child; child != 0; child = scopes_[child].next_child)
		entry(child, out, types, object, information, ranges, ranges_symbol, function_symbol);
	if (id != 0)
		out.uleb(0);
}
} // namespace native
