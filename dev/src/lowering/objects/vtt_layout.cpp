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

// The class whose vtable `entity` shares (its non-virtual primary base).
EntityId PrimaryBase(const std::vector<ClassPolymorphismFacts>& facts,
	EntityId entity)
{
	if (entity >= facts.size() || facts[entity].primary_ancestors.empty())
		return kNoEntity;
	return facts[entity].primary_ancestors[0];
}

}

bool VttLayoutBuilder::InPrimaryChain(EntityId owner, EntityId base) const
{
	if (owner >= facts_.size()) return false;
	const std::vector<EntityId>& chain = facts_[owner].primary_ancestors;
	return std::find(chain.begin(), chain.end(), base) != chain.end();
}

void VttLayoutBuilder::WalkSecondaryPointers(EntityId root, EntityId current,
	std::uint64_t offset, bool morally_virtual, std::vector<EntityId>* visited,
	std::vector<std::uint32_t>* views, std::vector<std::uint32_t>* shared)
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
		std::uint64_t at = offset + edge.offset;
		bool nonvirtual_primary = false;
		if (edge.virtual_base)
		{
			if (std::find(visited->begin(), visited->end(), edge.entity) !=
				visited->end()) continue;
			visited->push_back(edge.entity);
			if (!program_.FindVirtualBase(root, edge.entity, &at))
				ThrowLoweringInternal("VTT walk has no virtual base offset");
			moral = true;
		}
		else nonvirtual_primary = edge.entity == primary;
		if (!nonvirtual_primary && (base.virtual_base_count != 0 || moral))
		{
			// The table the subobject's vptr holds: its own view's, or the
			// view (or primary table) whose primary chain it lies in.
			const std::vector<PolymorphicViewFact>& candidates =
				facts_[root].views;
			std::size_t view = 0;
			while (view < candidates.size() &&
				!(candidates[view].stores_vptr && candidates[view].offset == at &&
				  (candidates[view].entity == edge.entity ||
				   InPrimaryChain(candidates[view].entity, edge.entity))))
				++view;
			std::uint32_t ordinal = kNoVttIndex;
			if (view == candidates.size())
			{
				if (!InPrimaryChain(root, edge.entity) ||
					!program_.FindVirtualBase(root, edge.entity, 0, &ordinal))
					ThrowLoweringInternal("VTT virtual pointer has no vtable view");
				view = kVttPrimaryTable;
			}
			views->push_back(static_cast<std::uint32_t>(view));
			shared->push_back(ordinal);
		}
		WalkSecondaryPointers(root, edge.entity, at, moral, visited, views, shared);
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
	layout.view_index.assign(entity < facts_.size() ?
		facts_[entity].views.size() : 0, kNoVttIndex);
	std::vector<EntityId> visited;
	WalkSecondaryPointers(entity, entity, 0, false, &visited,
		&layout.secondary_views, &layout.secondary_shared);
	for (std::size_t i = 0; i < layout.secondary_views.size(); ++i)
	{
		if (layout.secondary_views[i] != kVttPrimaryTable)
			layout.view_index[layout.secondary_views[i]] = next;
		++next;
	}
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
