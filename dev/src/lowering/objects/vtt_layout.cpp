#include "lowering/objects/vtt_layout.h"
#include "lowering/support/errors.h"

#include <algorithm>

namespace cppgm
{
namespace lowering
{

using namespace semantic;

namespace
{

// The class whose vtable `entity` shares (its primary base).
EntityId PrimaryBase(const std::vector<ClassPolymorphismFacts>& facts,
	EntityId entity)
{
	if (entity >= facts.size() || facts[entity].primary_ancestors.empty())
		return kNoEntity;
	return facts[entity].primary_ancestors[0];
}

}

// Clang's VTTBuilder::LayoutSecondaryVirtualPointers.
void VttLayoutBuilder::WalkSecondaryPointers(EntityId current, VttSubobject at,
	bool morally_virtual, std::vector<EntityId>* visited,
	std::vector<VttSubobject>* secondary)
{
	const EntityRecord& owner = program_.entities[current];
	if (owner.virtual_base_count == 0 && !morally_virtual) return;
	const EntityId primary = PrimaryBase(facts_, current);
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program_.DirectBase(current, i);
		const EntityRecord& base = program_.entities[edge.entity];
		if (!base.dynamic_class) continue;
		bool moral = morally_virtual;
		VttSubobject next(edge.entity, at.anchor, at.relative + edge.offset);
		bool nonvirtual_primary = false;
		if (edge.virtual_base)
		{
			if (std::find(visited->begin(), visited->end(), edge.entity) !=
				visited->end()) continue;
			visited->push_back(edge.entity);
			next.anchor = edge.entity;
			next.relative = 0;
			moral = true;
		}
		else nonvirtual_primary = edge.entity == primary;
		if (!nonvirtual_primary && (base.virtual_base_count != 0 || moral))
			secondary->push_back(next);
		WalkSecondaryPointers(edge.entity, next, moral, visited, secondary);
	}
}

bool BaseEntryTakesVtt(const Program& program,
	const std::vector<ClassPolymorphismFacts>& facts, BindingId binding)
{
	if (binding == kNoBinding || binding >= program.bindings.size()) return false;
	const BindingRecord& function = program.bindings[binding];
	if (!((function.constructor && function.constructor_base_entry) ||
		  (function.destructor && function.destructor_base_entry)) ||
		function.member_owner == kNoEntity ||
		function.member_owner >= facts.size() ||
		program.entities[function.member_owner].virtual_base_count == 0)
		return false;
	return facts[function.member_owner].HasObjectTable();
}

std::uint32_t SubVttIndex(const Program& program, const ClassVttLayout& layout,
	EntityId owner, EntityId base, bool complete, bool virtual_base)
{
	std::uint32_t ordinal = 0;
	if (virtual_base)
		return complete && program.FindVirtualBase(owner, base, 0, &ordinal) ?
			layout.virtual_index[ordinal] : kNoVttIndex;
	for (std::size_t i = 0; i < layout.base_index.size(); ++i)
		if (program.DirectBase(owner, i).entity == base &&
			!program.DirectBase(owner, i).virtual_base)
			return layout.base_index[i];
	return kNoVttIndex;
}

bool FindVirtualBasePathAnchor(const Program& program, EntityId owner,
	EntityId target, EntityId* anchor, std::uint64_t* relative_offset)
{
	if (program.FindVirtualBase(owner, target))
	{
		*anchor = target;
		*relative_offset = 0;
		return true;
	}
	std::vector<std::uint32_t> path;
	if (!program.QueryBasePath(owner, target, 0, 0, 0, 0, &path)) return false;
	EntityId current = owner;
	bool found = false;
	for (std::size_t i = 0; i < path.size(); ++i)
	{
		const DirectBaseEdge& edge = program.DirectBase(current, path[i]);
		current = edge.entity;
		if (edge.virtual_base)
		{
			*anchor = current;
			*relative_offset = 0;
			found = true;
		}
		else if (found) *relative_offset += edge.offset;
	}
	return found;
}

const ClassVttLayout& VttLayoutBuilder::Layout(EntityId entity)
{
	if (layouts_.size() <= entity) layouts_.resize(entity + 1);
	if (layouts_[entity].computed) return layouts_[entity];
	ClassVttLayout layout;
	const EntityRecord& owner = program_.entities[entity];
	layout.base_index.assign(owner.direct_base_count, kNoVttIndex);
	std::uint32_t next = 1;
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program_.DirectBase(entity, i);
		if (edge.virtual_base ||
			program_.entities[edge.entity].virtual_base_count == 0) continue;
		layout.base_index[i] = next;
		next += Layout(edge.entity).size;
	}
	std::vector<EntityId> visited;
	WalkSecondaryPointers(entity, VttSubobject(entity), false, &visited,
		&layout.secondary);
	next += static_cast<std::uint32_t>(layout.secondary.size());
	layout.size = next;
	layout.virtual_index.assign(owner.virtual_base_count, kNoVttIndex);
	for (std::size_t i = 0; i < owner.virtual_base_count; ++i)
	{
		const EntityId base = program_.VirtualBase(entity, i).entity;
		if (program_.entities[base].virtual_base_count == 0) continue;
		layout.virtual_index[i] = next;
		next += Layout(base).size;
	}
	layout.complete_size = next;
	layout.computed = true;
	layouts_[entity] = layout;
	return layouts_[entity];
}

}
}
