#include "semantic/object_model/vtable_groups.h"
#include "support/exceptions.h"

#include <algorithm>

namespace cppgm
{
namespace semantic
{

std::uint64_t VtablePlacement::VirtualBaseOffset(EntityId base) const
{
	std::uint64_t offset = 0;
	if (!program->FindVirtualBase(layout, base, &offset))
		ThrowInternalCompilerError("vtable virtual base has no offset");
	return offset;
}

std::int64_t OverriderPosition(const VtablePlacement& placement,
	const VirtualSlotFact& slot)
{
	const std::uint64_t base = slot.overrider_anchor != kNoEntity ?
		placement.VirtualBaseOffset(slot.overrider_anchor) : placement.offset;
	return static_cast<std::int64_t>(base) + slot.overrider_offset;
}

std::vector<EntityId> TableVirtualAnchors(
	const std::vector<ClassPolymorphismFacts>& all, EntityId table_class,
	EntityId table_anchor)
{
	std::vector<EntityId> anchors;
	if (table_anchor != kNoEntity) anchors.push_back(table_anchor);
	EntityId current = table_class;
	const std::vector<EntityId>& chain = all[table_class].primary_ancestors;
	for (std::size_t link = 0; link < chain.size(); ++link)
	{
		if (all[current].primary_virtual) anchors.push_back(chain[link]);
		current = chain[link];
	}
	return anchors;
}

namespace
{

EntityId SlotResultEntity(const Program& program, BindingId binding)
{
	const TypeRecord& function = program.types.Get(program.bindings[binding].type);
	if (function.kind != TYPE_FUNCTION) return kNoEntity;
	const TypeRecord& result = program.types.Get(function.child);
	if (result.kind != TYPE_POINTER && result.kind != TYPE_LVALUE_REFERENCE &&
		result.kind != TYPE_RVALUE_REFERENCE) return kNoEntity;
	const TypeRecord& named = program.types.Get(program.types.RemoveTopCv(result.child));
	return named.kind == TYPE_NAMED ? named.entity : kNoEntity;
}

// Whether an entry is a covariant thunk: its overrider's result converts to
// the slot's result by a virtual base or a nonzero offset.
bool SlotReturnAdjusts(const Program& program, const VirtualSlotFact& slot)
{
	if (slot.function == kNoBinding || slot.root == kNoBinding) return false;
	const EntityId result = SlotResultEntity(program, slot.function);
	const EntityId base = SlotResultEntity(program, slot.root);
	if (result == kNoEntity || base == kNoEntity || result == base) return false;
	if (program.HasVirtualBasePath(result, base)) return true;
	std::uint64_t offset = 0;
	return program.QueryBasePath(result, base, 0, 0, &offset) && offset != 0;
}

// The subobject a slot's entry is laid out for (g++'s first_defn in
// update_vtable_entry_for_fn): the nearest class in the table's primary
// chain declaring an override of the slot's function -- for a covariant
// entry, the nearest below the overrider's class whose own entry there is
// no thunk -- at the table's offset unless a virtual primary base link lies
// on the way, at whose own place it then is; past a link to a virtual
// primary base that lies elsewhere the entry is unused.
void SlotDeclaration(const VtablePlacement& placement,
	const std::vector<ClassPolymorphismFacts>& all, EntityId table_class,
	std::size_t slot, const VirtualSlotFact& entry, EntityId* anchor,
	std::uint64_t* offset, bool* unused)
{
	const Program& program = *placement.program;
	const std::uint64_t table_offset = *offset;
	*unused = false;
	std::vector<EntityId> chain(1, table_class);
	chain.insert(chain.end(), all[table_class].primary_ancestors.begin(),
		all[table_class].primary_ancestors.end());
	std::size_t depth = 0;
	while (depth < chain.size() && slot < all[chain[depth]].slots.size()) ++depth;
	// A link to a virtual primary base that lies elsewhere is lost.
	std::vector<std::uint8_t> lost_link(chain.size(), 0);
	for (std::size_t link = 1; link < depth; ++link)
		lost_link[link] = all[chain[link - 1]].primary_virtual &&
			placement.VirtualBaseOffset(chain[link]) != table_offset;
	std::size_t parent = depth == 0 ? 0 : depth - 1;
	for (std::size_t j = 0; j < depth; ++j)
	{
		const BindingId function = all[chain[j]].slots[slot].function;
		if (function != kNoBinding &&
			program.bindings[function].member_owner == chain[j])
		{
			parent = j;
			break;
		}
		if (j + 1 < depth && lost_link[j + 1]) *unused = true;
	}
	if (SlotReturnAdjusts(program, entry))
	{
		if (program.bindings[entry.function].member_owner == chain[parent] &&
			parent + 1 < depth) ++parent;
		while (parent + 1 < depth)
		{
			const VirtualSlotFact& own = all[chain[parent]].slots[slot];
			if (own.this_adjustment == 0 && !own.return_adjustment_virtual &&
				own.return_adjustment == 0) break;
			if (lost_link[parent + 1]) *unused = true;
			++parent;
		}
	}
	for (std::size_t link = 1; link <= parent; ++link)
		if (all[chain[link - 1]].primary_virtual)
		{
			*anchor = chain[link];
			*offset = placement.VirtualBaseOffset(chain[link]);
		}
}

// The place of a signature's vcall offset in a virtual base's table.
std::int64_t VcallEntryOffset(const ClassPolymorphismFacts& base,
	const SignatureKeyOf& key_of, BindingId root)
{
	const FunctionSignatureKey key = key_of(root);
	for (std::size_t i = 0; i < base.prefix.size() + base.virtual_prefix.size(); ++i)
	{
		const VtablePrefixEntry& entry = i < base.prefix.size() ? base.prefix[i] :
			base.virtual_prefix[i - base.prefix.size()];
		if (entry.function != kNoBinding && key_of(entry.function) == key)
			return -24 - 8 * static_cast<std::int64_t>(i);
	}
	ThrowInternalCompilerError("virtual thunk has no vcall offset");
}

}

void PlaceTableSlots(const VtablePlacement& placement,
	const std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId table_class, EntityId table_anchor, std::uint64_t table_offset,
	std::vector<VirtualSlotFact>* slots)
{
	for (std::size_t i = 0; i < slots->size(); ++i)
	{
		VirtualSlotFact& slot = (*slots)[i];
		EntityId anchor = table_anchor;
		std::uint64_t offset = table_offset;
		SlotDeclaration(placement, all, table_class, i, slot, &anchor, &offset,
			&slot.unused);
		slot.this_adjustment = OverriderPosition(placement, slot) -
			static_cast<std::int64_t>(offset);
		slot.virtual_call_offset = 0;
		slot.virtual_this_offset = 0;
		// An overrider outside the virtual base the entry's subobject lies
		// in is reached through that base: a virtual thunk moves there and
		// adds the vcall offset its table holds.
		if (anchor == kNoEntity || slot.overrider_anchor == anchor) continue;
		slot.virtual_this_offset = static_cast<std::int64_t>(
			placement.VirtualBaseOffset(anchor)) - static_cast<std::int64_t>(offset);
		slot.virtual_call_offset = VcallEntryOffset(all[anchor], key_of, slot.root);
	}
}

void FillVcallRows(const VtablePlacement& placement,
	const SignatureKeyOf& key_of, const ClassPolymorphismFacts& placed,
	const std::vector<VtablePrefixEntry>& entries,
	const std::vector<VirtualSlotFact>& slots,
	const std::vector<EntityId>& anchors, std::int64_t table_offset,
	std::vector<std::int64_t>* rows)
{
	for (std::size_t i = 0; i < entries.size() && i < rows->size(); ++i)
	{
		if (entries[i].function == kNoBinding) continue;
		const FunctionSignatureKey key = key_of(entries[i].function);
		const VirtualSlotFact* found = 0;
		for (std::size_t slot = 0; !found && slot < slots.size(); ++slot)
			if (key_of(slots[slot].root) == key) found = &slots[slot];
		for (std::size_t view = 0; !found && view < placed.views.size(); ++view)
		{
			if (std::find(anchors.begin(), anchors.end(),
				placed.views[view].virtual_anchor) == anchors.end()) continue;
			const std::vector<VirtualSlotFact>& candidates = placed.views[view].slots;
			for (std::size_t slot = 0; !found && slot < candidates.size(); ++slot)
				if (key_of(candidates[slot].root) == key) found = &candidates[slot];
		}
		if (found)
			(*rows)[i] = OverriderPosition(placement, *found) - table_offset;
	}
}

namespace
{

// Lays out a vtable group the way clang's ItaniumVTableBuilder does: the
// most derived class's table and its secondary tables, then those of its
// virtual bases in inheritance graph order.  A construction group has the
// base being constructed as its most derived class and the complete class as
// its layout class; a virtual primary base that does not share its class's
// place there gets its own table.  A class's own group is only ordered here.
class ConstructionGroupBuilder
{
public:
	ConstructionGroupBuilder(const Program& program,
		const std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
		EntityId complete, EntityId base, std::uint64_t offset,
		bool construction = true)
		: program_(program), all_(all), key_of_(key_of),
		  placement_(program, complete, offset), base_(base),
		  group_(base, offset), construction_(construction) {}

	// The base's own table has no vcall offsets even when it is a virtual
	// base of the complete class (g++; clang adds them).
	ConstructionGroupFact Build()
	{
		LayoutTable(base_, kNoEntity, 0, placement_.offset, false, false);
		std::vector<EntityId> visited;
		DeterminePrimaryVirtualBases(base_, placement_.offset, &visited);
		visited.clear();
		LayoutVirtualBases(base_, &visited);
		return group_;
	}

private:
	static bool SameSubobject(EntityId anchor, std::uint64_t relative,
		const PolymorphicViewFact& view)
	{
		return view.virtual_anchor == anchor &&
			(anchor == kNoEntity ? view.offset : view.virtual_anchor_offset) == relative;
	}

	// The base's slots for a subobject: its own, a view's, or the first ones
	// of the table whose primary chain holds the subobject; `shared` when the
	// subobject has no table of its own in the base's vtable group.
	std::vector<VirtualSlotFact> SourceSlots(EntityId entity, EntityId anchor,
		std::uint64_t relative, bool* shared) const
	{
		*shared = false;
		const ClassPolymorphismFacts& placed = all_[base_];
		if (entity == base_ && anchor == kNoEntity && relative == 0)
			return placed.slots;
		for (std::size_t view = 0; view < placed.views.size(); ++view)
			if (placed.views[view].entity == entity &&
				SameSubobject(anchor, relative, placed.views[view]))
			{
				*shared = !placed.views[view].stores_vptr;
				return placed.views[view].slots;
			}
		*shared = true;
		for (std::size_t owner = 0; owner <= placed.views.size(); ++owner)
		{
			EntityId current = owner == 0 ? base_ : placed.views[owner - 1].entity;
			EntityId chain_anchor = owner == 0 ? kNoEntity :
				placed.views[owner - 1].virtual_anchor;
			std::uint64_t chain_relative = owner == 0 ? 0 :
				chain_anchor == kNoEntity ? placed.views[owner - 1].offset :
				placed.views[owner - 1].virtual_anchor_offset;
			const std::vector<VirtualSlotFact>& slots = owner == 0 ?
				placed.slots : placed.views[owner - 1].slots;
			const std::vector<EntityId>& chain = all_[current].primary_ancestors;
			for (std::size_t link = 0; link < chain.size(); ++link)
			{
				if (all_[current].primary_virtual)
				{
					chain_anchor = chain[link];
					chain_relative = 0;
				}
				current = chain[link];
				if (current != entity || chain_anchor != anchor ||
					chain_relative != relative) continue;
				const std::size_t count =
					std::min(slots.size(), all_[entity].slots.size());
				return std::vector<VirtualSlotFact>(slots.begin(),
					slots.begin() + static_cast<std::ptrdiff_t>(count));
			}
		}
		ThrowInternalCompilerError("construction vtable subobject has no slots");
	}

	void LayoutTable(EntityId entity, EntityId anchor, std::uint64_t relative,
		std::uint64_t offset, bool morally_virtual, bool virtual_in_layout)
	{
		ConstructionTableFact table;
		table.entity = entity;
		table.anchor = anchor;
		table.relative = relative;
		table.offset = offset;
		if (!construction_)
		{
			group_.tables.push_back(table);
			LayoutSecondaryTables(entity, anchor, relative, offset, morally_virtual);
			return;
		}
		std::vector<VtablePrefixEntry> entries = all_[entity].prefix;
		if (virtual_in_layout)
			entries.insert(entries.end(), all_[entity].virtual_prefix.begin(),
				all_[entity].virtual_prefix.end());
		table.rows.assign(entries.size(), 0);
		for (std::size_t i = 0; i < entries.size(); ++i)
			if (entries[i].virtual_base != kNoEntity)
				table.rows[i] = static_cast<std::int64_t>(
					placement_.VirtualBaseOffset(entries[i].virtual_base)) -
					static_cast<std::int64_t>(offset);
		// The entries are the base's own (g++ copies them from the base's
		// tables, unused ones included): their thunks do not depend on the
		// complete class's layout, only the vcall offsets below do.  A
		// virtual primary base sharing a table in the base's own group gets
		// entries for its own subobject here.
		bool shared = false;
		table.slots = SourceSlots(entity, anchor, relative, &shared);
		if (shared)
			PlaceTableSlots(placement_, all_, key_of_, entity, anchor, offset,
				&table.slots);
		// g++ names no destructor in a construction table: no object is
		// destroyed through one.
		for (std::size_t slot = 0; slot < table.slots.size(); ++slot)
			if (program_.bindings[table.slots[slot].root].destructor)
				table.slots[slot].unused = true;
		FillVcallRows(placement_, key_of_, all_[base_], entries, table.slots,
			TableVirtualAnchors(all_, entity, anchor),
			static_cast<std::int64_t>(offset), &table.rows);
		table.address_point = 8 * (entries.size() + 2);
		EntityId current = entity;
		const std::vector<EntityId>& chain = all_[entity].primary_ancestors;
		for (std::size_t link = 0; link < chain.size(); ++link)
		{
			if (all_[current].primary_virtual &&
				placement_.VirtualBaseOffset(chain[link]) != offset) break;
			table.sharing.push_back(chain[link]);
			current = chain[link];
		}
		group_.tables.push_back(table);
		LayoutSecondaryTables(entity, anchor, relative, offset, morally_virtual);
	}

	// Itanium 2.6.4: a non-virtual base that is not morally virtual and has
	// no virtual bases needs no construction table.
	void LayoutSecondaryTables(EntityId entity, EntityId anchor,
		std::uint64_t relative, std::uint64_t offset, bool morally_virtual)
	{
		const ClassPolymorphismFacts& facts = all_[entity];
		const EntityId primary = facts.primary_ancestors.empty() ||
			facts.primary_virtual ? kNoEntity : facts.primary_ancestors[0];
		const EntityRecord& record = program_.entities[entity];
		for (std::size_t i = 0; i < record.direct_base_count; ++i)
		{
			const DirectBaseEdge& edge = program_.DirectBase(entity, i);
			const EntityRecord& base = program_.entities[edge.entity];
			if (edge.virtual_base || !base.dynamic_class) continue;
			if (construction_ && !morally_virtual && base.virtual_base_count == 0)
				continue;
			if (edge.entity == primary)
				LayoutSecondaryTables(edge.entity, anchor, relative + edge.offset,
					offset + edge.offset, morally_virtual);
			else
				LayoutTable(edge.entity, anchor, relative + edge.offset,
					offset + edge.offset, morally_virtual, false);
		}
	}

	void DeterminePrimaryVirtualBases(EntityId entity, std::uint64_t offset,
		std::vector<EntityId>* visited)
	{
		const ClassPolymorphismFacts& facts = all_[entity];
		if (facts.primary_virtual &&
			placement_.VirtualBaseOffset(facts.primary_ancestors[0]) == offset)
			primary_virtual_bases_.push_back(facts.primary_ancestors[0]);
		const EntityRecord& record = program_.entities[entity];
		for (std::size_t i = 0; i < record.direct_base_count; ++i)
		{
			const DirectBaseEdge& edge = program_.DirectBase(entity, i);
			std::uint64_t base_offset = offset + edge.offset;
			if (edge.virtual_base)
			{
				if (std::find(visited->begin(), visited->end(), edge.entity) !=
					visited->end()) continue;
				visited->push_back(edge.entity);
				base_offset = placement_.VirtualBaseOffset(edge.entity);
			}
			DeterminePrimaryVirtualBases(edge.entity, base_offset, visited);
		}
	}

	void LayoutVirtualBases(EntityId entity, std::vector<EntityId>* visited)
	{
		const EntityRecord& record = program_.entities[entity];
		for (std::size_t i = 0; i < record.direct_base_count; ++i)
		{
			const DirectBaseEdge& edge = program_.DirectBase(entity, i);
			if (edge.virtual_base && program_.entities[edge.entity].dynamic_class &&
				std::find(primary_virtual_bases_.begin(), primary_virtual_bases_.end(),
					edge.entity) == primary_virtual_bases_.end() &&
				std::find(visited->begin(), visited->end(), edge.entity) ==
					visited->end())
			{
				visited->push_back(edge.entity);
				LayoutTable(edge.entity, edge.entity, 0,
					placement_.VirtualBaseOffset(edge.entity), true, true);
			}
			if (program_.entities[edge.entity].virtual_base_count != 0)
				LayoutVirtualBases(edge.entity, visited);
		}
	}

	const Program& program_;
	const std::vector<ClassPolymorphismFacts>& all_;
	const SignatureKeyOf& key_of_;
	VtablePlacement placement_;
	EntityId base_;
	ConstructionGroupFact group_;
	bool construction_;
	std::vector<EntityId> primary_virtual_bases_;
};

bool ConstructionBase(const Program& program,
	const std::vector<ClassPolymorphismFacts>& all, EntityId base)
{
	return base < all.size() && program.entities[base].virtual_base_count != 0 &&
		all[base].HasObjectTable();
}

void AddNonvirtualConstructionGroups(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId complete, EntityId owner, std::uint64_t offset)
{
	const EntityRecord& record = program.entities[owner];
	for (std::size_t i = 0; i < record.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program.DirectBase(owner, i);
		if (edge.virtual_base || !ConstructionBase(program, all, edge.entity))
			continue;
		ConstructionGroupBuilder builder(program, all, key_of, complete,
			edge.entity, offset + edge.offset);
		all[complete].construction_groups.push_back(builder.Build());
		AddNonvirtualConstructionGroups(program, all, key_of, complete,
			edge.entity, offset + edge.offset);
	}
}

}

std::vector<ConstructionTableFact> VtableGroupOrder(const Program& program,
	const std::vector<ClassPolymorphismFacts>& all, EntityId entity)
{
	const SignatureKeyOf no_key;
	ConstructionGroupBuilder builder(program, all, no_key, entity, entity, 0, false);
	return builder.Build().tables;
}

void BuildConstructionGroups(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId complete)
{
	all[complete].construction_groups.clear();
	const EntityRecord& record = program.entities[complete];
	if (record.virtual_base_count == 0) return;
	AddNonvirtualConstructionGroups(program, all, key_of, complete, complete, 0);
	for (std::size_t i = 0; i < record.virtual_base_count; ++i)
	{
		const VirtualBaseLayout& layout = program.VirtualBase(complete, i);
		if (!ConstructionBase(program, all, layout.entity)) continue;
		ConstructionGroupBuilder builder(program, all, key_of, complete,
			layout.entity, layout.offset);
		all[complete].construction_groups.push_back(builder.Build());
		AddNonvirtualConstructionGroups(program, all, key_of, complete,
			layout.entity, layout.offset);
	}
}

}
}
