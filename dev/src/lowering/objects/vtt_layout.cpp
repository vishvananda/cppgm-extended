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
