#include "semantic/analysis/analyzer.h"
#include "semantic/object_model/vtable_groups.h"
#include "support/exceptions.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <utility>

namespace cppgm
{
namespace semantic
{

namespace
{

std::uint64_t AlignClassBase(std::uint64_t value, std::uint64_t alignment)
{
	if (alignment <= 1) return value;
	const std::uint64_t remainder = value % alignment;
	return remainder == 0 ? value : value + alignment - remainder;
}

struct VirtualSlotLocation
{
	std::uint32_t view, slot;
	BindingId next;

	VirtualSlotLocation(std::uint32_t view_value, std::uint32_t slot_value,
		BindingId next_value)
		: view(view_value), slot(slot_value), next(next_value) {}
};

std::vector<VirtualSlotFact>& SlotsForView(
	ClassPolymorphismFacts* facts, std::size_t view)
{
	return view == 0 ? facts->slots : facts->views[view - 1].slots;
}

const std::vector<VirtualSlotFact>& SlotsForView(
	const ClassPolymorphismFacts& facts, std::size_t view)
{
	return view == 0 ? facts.slots : facts.views[view - 1].slots;
}

std::uint32_t VirtualBaseRow(const std::vector<ClassPolymorphismFacts>& facts,
	EntityId entity, std::size_t ordinal)
{
	if (entity >= facts.size() || ordinal >= facts[entity].virtual_base_rows.size())
		ThrowInternalCompilerError("virtual base has no vtable row");
	return facts[entity].virtual_base_rows[ordinal];
}

EntityId VirtualResultEntity(const Program& program, BindingId binding)
{
	if (binding == kNoBinding || binding >= program.bindings.size())
		return kNoEntity;
	const TypeRecord& function = program.types.Get(
		program.bindings[binding].type);
	if (function.kind != TYPE_FUNCTION) return kNoEntity;
	const TypeRecord& result = program.types.Get(function.child);
	if (result.kind != TYPE_POINTER &&
		result.kind != TYPE_LVALUE_REFERENCE &&
		result.kind != TYPE_RVALUE_REFERENCE) return kNoEntity;
	TypeId target = program.types.RemoveTopCv(result.child);
	const TypeRecord& named = program.types.Get(target);
	return named.kind == TYPE_NAMED ? named.entity : kNoEntity;
}

// Whether converting an overrider's result to the result of the function it
// overrides moves the pointer: a path through a virtual base, or a nonzero
// offset (Itanium 2.5.2's return adjustment).  `known` is false while the
// overrider's result class is still being laid out.
bool CovariantResultAdjusts(const Program& program, BindingId function,
	BindingId overridden, bool* known)
{
	*known = true;
	const EntityId result = VirtualResultEntity(program, function);
	const EntityId base = VirtualResultEntity(program, overridden);
	if (result == kNoEntity || base == kNoEntity || result == base) return false;
	if (program.HasVirtualBasePath(result, base)) return true;
	if (!program.entities[result].layout_complete)
	{
		*known = false;
		return true;
	}
	std::uint64_t offset = 0;
	return program.QueryBasePath(result, base, 0, 0, &offset) && offset != 0;
}

// The physical slot of each virtual function root and final overrider in a
// class's tables (a destructor takes two).  An overrider keeps its first
// place whose entry returns its own result unadjusted, the primary table
// coming first, unless it roots an entry itself: a call dispatches through
// that place (g++'s DECL_VINDEX), never through a covariant thunk's, which
// a lost virtual primary base leaves null.  Only if it has none does it
// keep its first place.
void IndexVirtualSlots(const Program& program,
	const ClassPolymorphismFacts& facts, std::vector<std::uint32_t>* slot_of)
{
	std::vector<std::pair<BindingId, std::uint32_t> > adjusted_places;
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		const std::vector<VirtualSlotFact>& slots = SlotsForView(facts, view);
		std::uint32_t physical_slot = 0;
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
		{
			const BindingId root = program.bindings[slots[slot].root].canonical;
			const BindingId function =
				program.bindings[slots[slot].function].canonical;
			(*slot_of)[root] = physical_slot;
			bool known = true;
			if ((*slot_of)[function] == kNoDumpEdge)
			{
				if (!CovariantResultAdjusts(program, function, root, &known))
					(*slot_of)[function] = physical_slot;
				else
					adjusted_places.push_back(std::make_pair(function, physical_slot));
			}
			const std::uint32_t width = program.bindings[root].destructor ? 2 : 1;
			if (physical_slot > std::numeric_limits<std::uint32_t>::max() - width)
				ThrowSemanticResourceLimit("too many virtual slots");
			physical_slot += width;
		}
	}
	for (std::size_t i = 0; i < adjusted_places.size(); ++i)
		if ((*slot_of)[adjusted_places[i].first] == kNoDumpEdge)
			(*slot_of)[adjusted_places[i].first] = adjusted_places[i].second;
}

// Two paths reaching one subobject bring its table's slots: each takes the
// final overrider that dominates (one whose class derives from the other's),
// none when neither does.
// Whether the subobject of `inner`'s final overrider is a base subobject of
// `outer`'s: one in a virtual base of outer's class, or one in outer's own
// component at the place a base of that class takes in it.
bool OverriderWithin(const Program& program, const VirtualSlotFact& inner,
	const VirtualSlotFact& outer)
{
	const EntityId inner_class = program.bindings[inner.function].member_owner;
	const EntityId outer_class = program.bindings[outer.function].member_owner;
	if (inner_class == kNoEntity || outer_class == kNoEntity ||
		inner_class == outer_class || !program.IsBaseOf(inner_class, outer_class))
		return false;
	if (inner.overrider_anchor != kNoEntity &&
		inner.overrider_anchor != outer.overrider_anchor)
		return program.FindVirtualBase(outer_class, inner.overrider_anchor);
	if (inner.overrider_anchor != outer.overrider_anchor ||
		inner.overrider_base != outer.overrider_base ||
		inner.overrider_offset < outer.overrider_offset) return false;
	const std::uint64_t distance = static_cast<std::uint64_t>(
		inner.overrider_offset - outer.overrider_offset);
	std::uint64_t offset = 0;
	bool ambiguous = false;
	if (!program.QueryBasePath(outer_class, inner_class, 0, 0, &offset, &ambiguous))
		return false;
	if (!ambiguous) return distance == offset;
	return distance < LayoutNonvirtualSize(program.entities[outer_class]);
}

void MergeSharedSlots(const Program& program,
	std::vector<VirtualSlotFact>* retained,
	const std::vector<VirtualSlotFact>& incoming, std::size_t count)
{
	if (retained->size() < count || incoming.size() < count)
		ThrowInternalCompilerError("shared virtual views have incompatible slots");
	for (std::size_t i = 0; i < count; ++i)
	{
		VirtualSlotFact& selected = (*retained)[i];
		const VirtualSlotFact& candidate = incoming[i];
		if (program.bindings[selected.root].canonical !=
			program.bindings[candidate.root].canonical)
			ThrowInternalCompilerError("shared virtual slot roots do not match");
		if (selected.function == kNoBinding)
		{
			if (candidate.function != kNoBinding &&
				program.bindings[candidate.function].final_virtual)
				ThrowSemanticError(
					"virtual function overrides final function");
			continue;
		}
		if (candidate.function == kNoBinding)
		{
			selected = candidate;
			continue;
		}
		const BindingId selected_function =
			program.bindings[selected.function].canonical;
		const BindingId candidate_function =
			program.bindings[candidate.function].canonical;
		// One function in two subobjects is two final overriders
		// ([class.virtual]/2): neither subobject derives from the other.
		if (selected_function == candidate_function)
		{
			if (selected.overrider_anchor != candidate.overrider_anchor ||
				selected.overrider_base != candidate.overrider_base ||
				selected.overrider_offset != candidate.overrider_offset)
				selected.function = kNoBinding;
			continue;
		}
		// An overrider dominates one whose subobject it contains; a class
		// derived from the other's is not enough ([class.virtual]/2).
		const bool selected_is_base = OverriderWithin(program, selected, candidate);
		const bool candidate_is_base = OverriderWithin(program, candidate, selected);
		if (selected_is_base && !candidate_is_base)
			selected = candidate;
		else if (!candidate_is_base || selected_is_base)
		{
			if (program.bindings[selected_function].final_virtual ||
				program.bindings[candidate_function].final_virtual)
				ThrowSemanticError(
					"virtual function overrides final function");
			selected.function = kNoBinding;
		}
	}
}

// Slots copied from a direct base keep their final overriders' subobjects:
// one in the base's non-virtual part now lies in that base (placed once the
// class is laid out), or in the base itself when it is virtual.
void InheritOverriders(std::vector<VirtualSlotFact>* slots,
	const DirectBaseEdge& edge, std::uint32_t ordinal)
{
	for (std::size_t i = 0; i < slots->size(); ++i)
	{
		VirtualSlotFact& slot = (*slots)[i];
		if (slot.overrider_anchor != kNoEntity) continue;
		if (edge.virtual_base) slot.overrider_anchor = edge.entity;
		else slot.overrider_base = ordinal;
	}
}

// Places a slot's final overrider in the laid-out class: one in a direct
// base is now relative to the class, one in a virtual base stays relative
// to that base.
void ResolveOverrider(const Program& program, EntityId entity,
	VirtualSlotFact* slot)
{
	if (slot->overrider_base == kNoDumpEdge) return;
	slot->overrider_offset += static_cast<std::int64_t>(
		program.DirectBase(entity, slot->overrider_base).offset);
	slot->overrider_base = kNoDumpEdge;
}

// A class sharing a table through a virtual primary base link is the same
// subobject as the virtual base view of that base, which another path may
// have brought with a dominating final overrider: the table's slots and the
// view's agree on the merged overriders.
void SyncSharedTables(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, EntityId entity)
{
	ClassPolymorphismFacts& facts = all[entity];
	for (std::size_t owner = 0; owner <= facts.views.size(); ++owner)
	{
		const EntityId head = owner == 0 ? entity : facts.views[owner - 1].entity;
		EntityId anchor = owner == 0 ? kNoEntity :
			facts.views[owner - 1].virtual_anchor;
		std::uint64_t anchor_offset = owner == 0 ? 0 :
			facts.views[owner - 1].virtual_anchor_offset;
		EntityId current = head;
		const std::vector<EntityId> chain = all[head].primary_ancestors;
		for (std::size_t link = 0; link < chain.size(); ++link)
		{
			if (all[current].primary_virtual)
			{
				anchor = chain[link];
				anchor_offset = 0;
			}
			current = chain[link];
			if (anchor == kNoEntity) continue;
			for (std::size_t other = 0; other < facts.views.size(); ++other)
			{
				PolymorphicViewFact& shared = facts.views[other];
				if (other + 1 == owner || shared.entity != current ||
					shared.virtual_anchor != anchor ||
					shared.virtual_anchor_offset != anchor_offset) continue;
				std::vector<VirtualSlotFact>& slots = SlotsForView(&facts, owner);
				const std::size_t count = all[current].slots.size();
				MergeSharedSlots(program, &slots, shared.slots, count);
				std::copy(slots.begin(), slots.begin() +
					static_cast<std::ptrdiff_t>(count), shared.slots.begin());
			}
		}
	}
}

// A primary table entry added for an overrider whose result is its own
// class, before that class was laid out, stays only if converting the class
// to the overridden function's result moves the pointer.
void DropUnadjustedCovariantEntries(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, EntityId entity,
	const std::function<FunctionSignatureKey(BindingId)>& key_of,
	std::vector<std::uint32_t>* slot_of)
{
	ClassPolymorphismFacts& facts = all[entity];
	if (facts.primary_ancestors.empty()) return;
	const std::vector<VirtualSlotFact>& inherited =
		all[facts.primary_ancestors[0]].slots;
	bool dropped = false;
	for (std::size_t slot = inherited.size(); slot < facts.slots.size();)
	{
		const BindingId member = facts.slots[slot].root;
		BindingId overridden = kNoBinding;
		if (member == facts.slots[slot].function &&
			VirtualResultEntity(program, member) == entity)
			for (std::size_t i = 0; i < inherited.size(); ++i)
				if (key_of(inherited[i].root) == key_of(member))
				{
					overridden = inherited[i].function;
					break;
				}
		bool known = true;
		if (overridden != kNoBinding &&
			!CovariantResultAdjusts(program, member, overridden, &known))
		{
			facts.slots.erase(facts.slots.begin() + static_cast<std::ptrdiff_t>(slot));
			(*slot_of)[program.bindings[member].canonical] = kNoDumpEdge;
			dropped = true;
			continue;
		}
		++slot;
	}
	if (dropped) IndexVirtualSlots(program, facts, slot_of);
}

}

void Analyzer::AppendPolymorphicView(ClassPolymorphismFacts* facts,
	const PolymorphicViewFact& view)
{
	if (view.virtual_anchor == kNoEntity)
	{
		facts->views.push_back(view);
		return;
	}
	++polymorphic_virtual_view_lookups_;
	for (std::size_t i = 0; i < facts->views.size(); ++i)
	{
		PolymorphicViewFact& retained = facts->views[i];
		if (retained.virtual_anchor != view.virtual_anchor ||
			retained.entity != view.entity ||
			retained.virtual_anchor_offset != view.virtual_anchor_offset)
			continue;
		if (retained.slots.size() != view.slots.size())
			ThrowInternalCompilerError("shared virtual views have incompatible slots");
		MergeSharedSlots(*program_, &retained.slots, view.slots, view.slots.size());
		retained.virtual_base = retained.virtual_base || view.virtual_base;
		++polymorphic_virtual_view_merges_;
		return;
	}
	facts->views.push_back(view);
}

void Analyzer::PublishVirtualBaseStats()
{
	stats_->virtual_base_layout_edge_visits = virtual_base_layout_edge_visits_;
	stats_->virtual_base_layout_facts = virtual_base_layout_facts_;
	stats_->virtual_base_layout_lookups = program_->virtual_base_layout_lookups;
	stats_->virtual_base_layout_probes = program_->virtual_base_layout_probes;
	stats_->direct_base_validation_visits =
		program_->direct_base_validation_visits;
	stats_->polymorphic_virtual_view_lookups =
		polymorphic_virtual_view_lookups_;
	stats_->polymorphic_virtual_view_merges =
		polymorphic_virtual_view_merges_;
}

std::size_t Analyzer::PreferredClassLayoutBaseOrdinal(
	EntityId entity) const
{
	const EntityRecord& owner = program_->entities[entity];
	for (std::size_t ordinal = 0; ordinal < owner.direct_base_count; ++ordinal)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, ordinal);
		if (!edge.virtual_base && edge.entity == owner.direct_base)
			return ordinal;
	}
	return owner.direct_base_count;
}

const EntityRecord* Analyzer::InitializeClassBaseLayout(
	EntityId entity, std::size_t packing_alignment, ClassLayoutCursor* cursor)
{
	EntityRecord& owner = program_->entities[entity];
	if (!owner.has_user_declared_destructor)
	{
		owner.destructible = true;
		owner.trivial_destructor = true;
	}
	const std::size_t preferred_ordinal = PreferredClassLayoutBaseOrdinal(entity);
	const bool dynamic_primary = preferred_ordinal != owner.direct_base_count &&
		program_->entities[program_->DirectBase(
			entity, preferred_ordinal).entity].dynamic_class;
	// Itanium C++ ABI 2.4 I-II: a dynamic class with no primary base starts
	// with its virtual table pointer; a nearly empty virtual primary base is
	// placed first, at offset zero, sharing it.
	const bool virtual_primary = entity < class_polymorphism_.size() &&
		class_polymorphism_[entity].primary_virtual;
	if (owner.dynamic_class && !dynamic_primary)
	{
		const std::uint64_t pointer_alignment = virtual_primary ?
			LayoutNonvirtualAlignment(program_->entities[
				class_polymorphism_[entity].primary_ancestors[0]]) : 8;
		// The virtual primary base's empty subobjects lie at offset zero.
		if (virtual_primary)
			empty_subobjects_.Record(class_polymorphism_[entity].primary_ancestors[0],
				0, false, false);
		cursor->Place(8, true);
		cursor->natural_alignment = std::max<std::uint64_t>(
			cursor->natural_alignment, pointer_alignment);
		cursor->alignment = std::max<std::uint64_t>(cursor->alignment,
			packing_alignment == 0 ? pointer_alignment :
			std::min<std::uint64_t>(pointer_alignment, packing_alignment));
	}
	if (owner.direct_base_count == 0) return 0;
	owner.direct_base = kNoEntity;
	const EntityRecord* primary = 0;
	for (std::size_t layout_index = 0; layout_index < owner.direct_base_count;
		++layout_index)
	{
		const std::size_t base_index = preferred_ordinal == owner.direct_base_count ?
			layout_index : layout_index == 0 ? preferred_ordinal :
			layout_index <= preferred_ordinal ? layout_index - 1 : layout_index;
		DirectBaseEdge& edge = program_->MutableDirectBase(entity, base_index);
		const EntityRecord* base = &program_->entities[edge.entity];
		if (!base->layout_complete)
			ThrowSemanticError("direct base layout is incomplete");
		if (!edge.virtual_base && !primary)
		{
			primary = base; owner.direct_base = edge.entity;
		}
		// 2.4 II.2-3: the primary base first, then the other non-virtual
		// bases in declaration order, each at the data size rounded up to its
		// non-virtual alignment (an empty one at offset zero first), moved on
		// by that alignment while one of its empty subobjects would share its
		// type and offset with one placed before.  A non-empty base claims
		// only its non-virtual size, so its tail padding is reused.
		std::uint64_t offset = 0;
		if (!edge.virtual_base)
		{
			const std::uint64_t base_alignment = LayoutNonvirtualAlignment(*base);
			const std::uint64_t effective = packing_alignment == 0 ?
				base_alignment : std::min<std::uint64_t>(base_alignment,
					packing_alignment);
			const bool empty = base->empty_class;
			if (!empty || empty_subobjects_.Conflict(edge.entity, 0, false))
			{
				offset = AlignClassBase(cursor->data_size, effective);
				while (empty_subobjects_.Conflict(edge.entity, offset, false))
					offset += effective;
			}
			const std::uint64_t extent = empty ? base->object_size :
				LayoutNonvirtualSize(*base);
			if (offset > std::numeric_limits<std::uint64_t>::max() - extent)
				ThrowSemanticResourceLimit("class layout is too large");
			empty_subobjects_.Record(edge.entity, offset, false, empty);
			cursor->Place(offset + extent, !empty);
			cursor->natural_alignment = std::max(cursor->natural_alignment,
				base_alignment);
			cursor->alignment = std::max(cursor->alignment, effective);
		}
		edge.offset = offset;
		owner.has_nonzero_base_subobject_offset =
			owner.has_nonzero_base_subobject_offset || offset != 0 ||
			base->has_nonzero_base_subobject_offset;
		if (primary == base) owner.direct_base_offset = offset;
		if (!base->destructible) owner.destructible = false;
		if (!base->trivial_destructor) owner.trivial_destructor = false;
		const BindingId destructor = DestructorForType(base->type);
		if (destructor == kNoBinding || !CanAccessMember(destructor, edge.entity))
			owner.destructible = false;
	}
	return primary;
}

std::size_t Analyzer::InitializeClassPolymorphismBases(EntityId entity)
{
	EntityRecord& owner = program_->entities[entity];
	std::size_t primary = owner.direct_base_count;
	for (std::size_t ordinal = 0; ordinal < owner.direct_base_count; ++ordinal)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, ordinal);
		if (edge.entity >= class_polymorphism_.size() ||
			!class_polymorphism_[edge.entity].complete)
			ThrowInternalCompilerError(
				"base polymorphism facts are incomplete: " +
				program_->names.Get(owner.identity_name) + " derives from " +
				program_->names.Get(program_->entities[edge.entity].identity_name));
		if (!edge.virtual_base && primary == owner.direct_base_count &&
			program_->entities[edge.entity].dynamic_class)
			primary = ordinal;
		owner.dynamic_class = owner.dynamic_class || edge.virtual_base ||
			program_->entities[edge.entity].dynamic_class;
		owner.polymorphic_class = owner.polymorphic_class ||
			program_->entities[edge.entity].polymorphic_class;
	}
	return primary;
}

namespace
{

// A view inherited through a direct base lies in a virtual base when it does
// in the base, or in the base itself when that base is virtual.
void InheritPolymorphicViewAnchor(const Program& program,
	const DirectBaseEdge& edge, const PolymorphicViewFact& inherited,
	PolymorphicViewFact* view)
{
	view->virtual_anchor = kNoEntity;
	view->virtual_anchor_offset = 0;
	if (inherited.virtual_anchor_ordinal != kNoDumpEdge)
	{
		view->virtual_anchor = program.VirtualBase(
			edge.entity, inherited.virtual_anchor_ordinal).entity;
		view->virtual_anchor_offset = inherited.virtual_anchor_offset;
	}
	else if (edge.virtual_base)
	{
		view->virtual_anchor = edge.entity;
		view->virtual_anchor_offset = inherited.offset;
	}
}

// The vcall offsets a virtual base's table adds (clang's AddVCallOffsets):
// its non-virtual primary chain first, then the virtual functions it
// declares, then its other non-virtual bases, one per signature.
void AppendVcallEntries(const Program& program,
	const std::vector<ClassPolymorphismFacts>& facts,
	const std::vector<std::vector<BindingId> >& functions,
	const std::function<FunctionSignatureKey(BindingId)>& key_of,
	EntityId cls, std::vector<FunctionSignatureKey>* present,
	std::vector<VtablePrefixEntry>* out)
{
	const ClassPolymorphismFacts& owner = facts[cls];
	const EntityId primary = owner.primary_ancestors.empty() ||
		owner.primary_virtual ? kNoEntity : owner.primary_ancestors[0];
	if (primary != kNoEntity)
		AppendVcallEntries(program, facts, functions, key_of, primary, present, out);
	for (std::size_t i = 0; cls < functions.size() && i < functions[cls].size(); ++i)
	{
		const BindingId function = program.bindings[functions[cls][i]].canonical;
		const BindingRecord& binding = program.bindings[function];
		if (!binding.virtual_function || binding.static_member_function) continue;
		const FunctionSignatureKey key = key_of(function);
		if (std::find(present->begin(), present->end(), key) != present->end())
			continue;
		present->push_back(key);
		out->push_back(VtablePrefixEntry(kNoEntity, function));
	}
	const EntityRecord& record = program.entities[cls];
	for (std::size_t i = 0; i < record.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program.DirectBase(cls, i);
		if (!edge.virtual_base && edge.entity != primary &&
			program.entities[edge.entity].dynamic_class)
			AppendVcallEntries(program, facts, functions, key_of, edge.entity,
				present, out);
	}
}

// The values of a table's prefix entries: a virtual base's offset from the
// table's subobject; vcall entries are filled once the slots are adjusted.
void FillVtablePrefixValues(const Program& program, EntityId entity,
	const std::vector<VtablePrefixEntry>& entries, std::uint64_t table_offset,
	std::vector<std::int64_t>* values)
{
	values->assign(entries.size(), 0);
	for (std::size_t i = 0; i < entries.size(); ++i)
	{
		if (entries[i].virtual_base == kNoEntity) continue;
		std::uint64_t offset = 0;
		if (!program.FindVirtualBase(entity, entries[i].virtual_base, &offset))
			ThrowInternalCompilerError("vtable prefix virtual base has no offset");
		(*values)[i] = static_cast<std::int64_t>(offset) -
			static_cast<std::int64_t>(table_offset);
	}
}

// A table's vcall offsets follow the final overriders of the table's
// slots and of the views in the virtual bases sharing it.
void FillVtableVcallValues(const VtablePlacement& placement,
	std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId entity)
{
	ClassPolymorphismFacts& facts = all[entity];
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		const EntityId owner = view == 0 ? entity : facts.views[view - 1].entity;
		std::vector<VtablePrefixEntry> entries = all[owner].prefix;
		if (view != 0 && facts.views[view - 1].virtual_base)
			entries.insert(entries.end(),
				all[owner].virtual_prefix.begin(),
				all[owner].virtual_prefix.end());
		std::vector<std::int64_t>& values = view == 0 ?
			facts.virtual_base_offsets : facts.views[view - 1].virtual_base_offsets;
		FillVcallRows(placement, key_of, facts, entries, SlotsForView(facts, view),
			TableVirtualAnchors(all, owner,
				view == 0 ? kNoEntity : facts.views[view - 1].virtual_anchor),
			view == 0 ? 0 : static_cast<std::int64_t>(facts.views[view - 1].offset),
			&values);
	}
}

// The tables of a class's vtable group follow its primary table in clang's
// order -- the non-virtual bases' secondary tables depth first, then the
// virtual bases' in inheritance graph order (Itanium 2.5.2) -- and the views
// sharing a table come after them.
void OrderPolymorphicViews(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, EntityId entity)
{
	ClassPolymorphismFacts& facts = all[entity];
	const std::vector<ConstructionTableFact> order =
		VtableGroupOrder(program, all, entity);
	std::vector<std::pair<std::size_t, std::size_t> > ranks;
	for (std::size_t view = 0; view < facts.views.size(); ++view)
	{
		const PolymorphicViewFact& current = facts.views[view];
		const std::uint64_t relative = current.virtual_anchor == kNoEntity ?
			current.offset : current.virtual_anchor_offset;
		std::size_t rank = order.size();
		for (std::size_t table = 0; current.stores_vptr && table < order.size(); ++table)
			if (order[table].entity == current.entity &&
				order[table].anchor == current.virtual_anchor &&
				order[table].relative == relative)
			{
				rank = table;
				break;
			}
		ranks.push_back(std::make_pair(rank, view));
	}
	std::stable_sort(ranks.begin(), ranks.end());
	std::vector<PolymorphicViewFact> views;
	views.reserve(facts.views.size());
	for (std::size_t i = 0; i < ranks.size(); ++i)
		views.push_back(facts.views[ranks[i].second]);
	facts.views.swap(views);
}

// A view whose subobject shares a table -- the class's own primary table at
// offset zero, or another view's at its offset, being in that table's
// primary chain -- stores no vptr of its own and has no table (2.5.2).
void MarkSharedPolymorphicViews(std::vector<ClassPolymorphismFacts>& all,
	EntityId entity)
{
	ClassPolymorphismFacts& facts = all[entity];
	for (std::size_t view = 0; view < facts.views.size(); ++view)
	{
		PolymorphicViewFact& current = facts.views[view];
		bool shared = current.offset == 0 &&
			std::find(facts.primary_ancestors.begin(), facts.primary_ancestors.end(),
				current.entity) != facts.primary_ancestors.end();
		for (std::size_t other = 0; !shared && other < facts.views.size(); ++other)
		{
			if (other == view || facts.views[other].offset != current.offset) continue;
			const std::vector<EntityId>& chain =
				all[facts.views[other].entity].primary_ancestors;
			shared = std::find(chain.begin(), chain.end(), current.entity) !=
				chain.end();
		}
		current.stores_vptr = !shared;
	}
}

// Itanium 2.2: a dynamic class whose non-virtual part is its vptr alone.
bool NearlyEmptyClass(const EntityRecord& record)
{
	return record.dynamic_class && record.layout_complete &&
		LayoutNonvirtualSize(record) == 8;
}


}

// Itanium 2.4 I.b: with no non-virtual dynamic base, the primary base is the
// first nearly empty virtual base in inheritance graph order that is no
// other base's primary base, else the first nearly empty one.
EntityId Analyzer::SelectVirtualPrimaryBase(EntityId entity)
{
	std::vector<VirtualBaseLayout> layouts;
	CollectVirtualBaseLayouts(entity, &layouts);
	std::vector<EntityId> visited, indirect;
	CollectIndirectPrimaryBases(*program_, class_polymorphism_, entity,
		&visited, &indirect);
	EntityId first = kNoEntity;
	for (std::size_t i = 0; i < layouts.size(); ++i)
	{
		if (!NearlyEmptyClass(program_->entities[layouts[i].entity])) continue;
		if (first == kNoEntity) first = layouts[i].entity;
		if (std::find(indirect.begin(), indirect.end(), layouts[i].entity) ==
			indirect.end())
			return layouts[i].entity;
	}
	return first;
}

// The class shares its primary base's vtable: the base's slots and views,
// a nearly empty virtual primary base's too (Itanium 2.4 I.b).
void Analyzer::InheritPrimaryPolymorphism(EntityId entity,
	const DirectBaseEdge& edge, std::uint32_t ordinal)
{
	ClassPolymorphismFacts& facts = class_polymorphism_[entity];
	const ClassPolymorphismFacts& inherited = class_polymorphism_[edge.entity];
	facts.primary_virtual = edge.virtual_base;
	facts.slots = inherited.slots;
	InheritOverriders(&facts.slots, edge, ordinal);
	facts.primary_ancestors.push_back(edge.entity);
	facts.primary_ancestors.insert(facts.primary_ancestors.end(),
		inherited.primary_ancestors.begin(), inherited.primary_ancestors.end());
	for (std::size_t i = 0; i < inherited.views.size(); ++i)
	{
		PolymorphicViewFact view = inherited.views[i];
		view.direct_base_ordinal = ordinal;
		view.relative_offset = inherited.views[i].offset;
		InheritPolymorphicViewAnchor(*program_, edge, inherited.views[i], &view);
		InheritOverriders(&view.slots, edge, ordinal);
		AppendPolymorphicView(&facts, view);
	}
}

void Analyzer::CompleteClassPolymorphism(EntityId entity)
{
	if (class_polymorphism_.size() <= entity)
		class_polymorphism_.resize(static_cast<std::size_t>(entity) + 1);
	if (class_polymorphism_[entity].complete) return;
	// A complete base whose facts were never asked for (a lambda closure
	// libc++ makes an empty base of its compressed pair) gets them first;
	// only a base still being defined is an error below.  This runs before
	// the facts reference is taken: the recursion may grow the table.
	for (std::size_t ordinal = 0;
		ordinal < program_->entities[entity].direct_base_count; ++ordinal)
	{
		const EntityId base = program_->DirectBase(entity, ordinal).entity;
		if ((base >= class_polymorphism_.size() ||
			 !class_polymorphism_[base].complete) &&
			program_->entities[base].complete)
			CompleteClassPolymorphism(base);
	}
	if (class_polymorphism_.size() <= entity)
		class_polymorphism_.resize(static_cast<std::size_t>(entity) + 1);
	ClassPolymorphismFacts& facts = class_polymorphism_[entity];
	if (facts.complete) return;
	EntityRecord& owner = program_->entities[entity];
	const std::size_t primary = InitializeClassPolymorphismBases(entity);
	const EntityId virtual_primary = primary == owner.direct_base_count ?
		SelectVirtualPrimaryBase(entity) : kNoEntity;
	if (virtual_primary != kNoEntity)
	{
		std::uint32_t reached_through = 0;
		while (reached_through + 1 < owner.direct_base_count &&
			program_->DirectBase(entity, reached_through).entity != virtual_primary &&
			!program_->IsBaseOf(virtual_primary,
				program_->DirectBase(entity, reached_through).entity))
			++reached_through;
		InheritPrimaryPolymorphism(entity,
			DirectBaseEdge(virtual_primary, ACCESS_PUBLIC, true), reached_through);
	}
	if (primary != owner.direct_base_count)
	{
		owner.direct_base = program_->DirectBase(entity, primary).entity;
		InheritPrimaryPolymorphism(entity, program_->DirectBase(entity, primary),
			static_cast<std::uint32_t>(primary));
	}
	for (std::size_t ordinal = 0; ordinal < owner.direct_base_count; ++ordinal)
	{
		if (ordinal == primary) continue;
		const DirectBaseEdge& edge = program_->DirectBase(entity, ordinal);
		const ClassPolymorphismFacts& inherited =
			class_polymorphism_[edge.entity];
		if (!inherited.HasObjectTable()) continue;
		// A secondary dynamic base owns a physical address point even
		// when it has no virtual function slots of its own.
		// Keep that physical view distinct from the inherited virtual view.
		{
			PolymorphicViewFact view(edge.entity,
				static_cast<std::uint32_t>(ordinal));
			view.slots = inherited.slots;
			InheritOverriders(&view.slots, edge, static_cast<std::uint32_t>(ordinal));
			if (edge.virtual_base) view.virtual_anchor = edge.entity;
			AppendPolymorphicView(&facts, view);
		}
		if (!inherited.primary_ancestors.empty())
		{
			// Each primary ancestor shares the base's table at the base's
			// address, or at its own place once the chain reaches a virtual
			// primary base.
			EntityId anchor = edge.virtual_base ? edge.entity : kNoEntity;
			EntityId chain = edge.entity;
			for (std::size_t i = 0; i < inherited.primary_ancestors.size(); ++i)
			{
				const EntityId ancestor = inherited.primary_ancestors[i];
				if (class_polymorphism_[chain].primary_virtual) anchor = ancestor;
				chain = ancestor;
				PolymorphicViewFact alias(ancestor,
					static_cast<std::uint32_t>(ordinal), 0, false);
				// The ancestor's slots are the shared table's first ones.
				alias.slots.assign(inherited.slots.begin(), inherited.slots.begin() +
					static_cast<std::ptrdiff_t>(std::min(inherited.slots.size(),
						class_polymorphism_[ancestor].slots.size())));
				InheritOverriders(&alias.slots, edge, alias.direct_base_ordinal);
				alias.virtual_anchor = anchor;
				AppendPolymorphicView(&facts, alias);
			}
		}
		for (std::size_t i = 0; i < inherited.views.size(); ++i)
		{
			PolymorphicViewFact view = inherited.views[i];
			view.direct_base_ordinal = static_cast<std::uint32_t>(ordinal);
			view.relative_offset = inherited.views[i].offset;
			InheritPolymorphicViewAnchor(*program_, edge, inherited.views[i], &view);
			InheritOverriders(&view.slots, edge, static_cast<std::uint32_t>(ordinal));
			AppendPolymorphicView(&facts, view);
		}
	}
	SyncSharedTables(*program_, class_polymorphism_, entity);

	bool inherits_virtual_destructor = false;
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		const std::vector<VirtualSlotFact>& slots = SlotsForView(facts, view);
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
			if (program_->bindings[slots[slot].root].destructor)
				inherits_virtual_destructor = true;
	}
	if (inherits_virtual_destructor &&
		(entity >= entity_destructor_by_entity_.size() ||
		 entity_destructor_by_entity_[entity] == kNoBinding))
	{
		const BindingId destructor = EnsureImplicitDestructor(entity);
		if (entity_member_functions_.size() <= entity)
			entity_member_functions_.resize(static_cast<std::size_t>(entity) + 1);
		std::vector<BindingId>& functions = entity_member_functions_[entity];
		if (std::find(functions.begin(), functions.end(), destructor) ==
			functions.end()) functions.push_back(destructor);
	}

	FunctionSignatureTable slot_index;
	std::vector<VirtualSlotLocation> locations;
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		const std::vector<VirtualSlotFact>& slots = SlotsForView(facts, view);
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
		{
			if (locations.size() >= kNoBinding)
				ThrowSemanticResourceLimit("too many virtual slot locations");
			const FunctionSignatureKey key = VirtualSignatureKey(slots[slot].root);
			const BindingId next = slot_index.Find(key);
			locations.push_back(VirtualSlotLocation(
				static_cast<std::uint32_t>(view),
				static_cast<std::uint32_t>(slot), next));
			slot_index.Insert(key,
				static_cast<BindingId>(locations.size() - 1));
		}
	}
	if (entity_member_functions_.size() <= entity)
		entity_member_functions_.resize(static_cast<std::size_t>(entity) + 1);
	const std::vector<BindingId>& members = entity_member_functions_[entity];
	for (std::size_t member_index = 0; member_index < members.size(); ++member_index)
	{
		const BindingId member =
			program_->bindings[members[member_index]].canonical;
		BindingRecord& binding = program_->bindings[member];
		if (binding.static_member_function || binding.constructor) continue;
		++virtual_signature_lookups_;
		BindingId location = slot_index.Find(VirtualSignatureKey(member));
		const bool matched = location != kNoBinding;
		// The function this one overrides in the primary base, if any.
		BindingId primary_overridden = kNoBinding;
		while (location != kNoBinding)
		{
			VirtualSlotLocation& indexed = locations[location];
			std::vector<VirtualSlotFact>& slots =
				SlotsForView(&facts, indexed.view);
			VirtualSlotFact& slot = slots[indexed.slot];
			if (indexed.view == 0 && primary_overridden == kNoBinding)
				primary_overridden = slot.function != kNoBinding ?
					slot.function : slot.root;
			if (!VirtualSignatureMatches(member, slot.root))
				ThrowSemanticError("invalid covariant virtual return type");
			if (slot.function != kNoBinding &&
				program_->bindings[slot.function].final_virtual)
				ThrowSemanticError("virtual function overrides final function");
			if (slot.function != kNoBinding)
				RecordVirtualExceptionOverride(entity, member, slot.function);
			else
			{
				// Shared virtual views can merge distinct final overriders.
				// The new declaration must satisfy each incoming restriction.
				for (std::size_t ordinal = 0;
					ordinal < owner.direct_base_count; ++ordinal)
				{
					const ClassPolymorphismFacts& inherited = class_polymorphism_[
						program_->DirectBase(entity, ordinal).entity];
					for (std::size_t view = 0; view <= inherited.views.size(); ++view)
					{
						const std::vector<VirtualSlotFact>& incoming =
							SlotsForView(inherited, view);
						for (std::size_t i = 0; i < incoming.size(); ++i)
							if (incoming[i].root == slot.root)
								RecordVirtualExceptionOverride(
									entity, member, incoming[i].function);
					}
				}
			}
			slot.function = member;
			slot.overrider_anchor = kNoEntity;
			slot.overrider_base = kNoDumpEdge;
			slot.overrider_offset = 0;
			location = indexed.next;
			++virtual_overrides_;
		}
		if (matched)
		{
			// Itanium 2.5.2: the primary table has an entry for each virtual
			// function the class declares unless it overrides one of the
			// primary base's without a return adjustment.  While the result
			// is this class, its offset is checked once it is laid out.
			binding.virtual_function = true;
			bool known = true;
			if (primary_overridden == kNoBinding || CovariantResultAdjusts(
				*program_, member, primary_overridden, &known))
				facts.slots.push_back(VirtualSlotFact(member, member));
		}
		else
		{
			if (binding.override_specifier)
				ThrowSemanticError("override has no matching base virtual");
			if (binding.pure_virtual && !binding.virtual_function)
				ThrowSemanticError("pure function is not virtual");
			if (binding.final_virtual && !binding.virtual_function)
				ThrowSemanticError("final function is not virtual");
			if (binding.virtual_function)
				facts.slots.push_back(VirtualSlotFact(member, member));
		}
	}

	owner.polymorphic_class = owner.polymorphic_class || !facts.slots.empty();
	owner.dynamic_class = owner.dynamic_class || owner.polymorphic_class;
	if (owner.polymorphic_class) ++polymorphic_classes_;
	owner.abstract_class = false;
	if (virtual_slot_by_binding_.size() < program_->bindings.size())
		virtual_slot_by_binding_.resize(program_->bindings.size(), kNoDumpEdge);
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		const std::vector<VirtualSlotFact>& slots = SlotsForView(facts, view);
		virtual_slots_ += slots.size();
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
		{
			if (slots[slot].function == kNoBinding)
				ThrowSemanticError("no unique final overrider");
			const BindingId function =
				program_->bindings[slots[slot].function].canonical;
			if (program_->bindings[function].pure_virtual) owner.abstract_class = true;
			if (program_->bindings[function].destructor)
				owner.trivial_destructor = false;
		}
	}
	IndexVirtualSlots(*program_, facts, &virtual_slot_by_binding_);
	facts.complete = true;
}

// A view lies where its virtual anchor is placed in this class, else at its
// direct base's offset.
void Analyzer::AnchorPolymorphicView(EntityId entity,
	const DirectBaseEdge& edge, PolymorphicViewFact* view)
{
	view->virtual_anchor_ordinal = kNoDumpEdge;
	std::uint64_t anchor_offset = 0;
	std::uint32_t anchor_ordinal = 0;
	if (view->virtual_anchor == kNoEntity || !program_->FindVirtualBase(
		entity, view->virtual_anchor, &anchor_offset, &anchor_ordinal))
	{
		view->virtual_anchor = kNoEntity;
		view->virtual_anchor_offset = 0;
		view->offset = edge.offset + view->relative_offset;
		return;
	}
	view->offset = anchor_offset + view->virtual_anchor_offset;
	view->virtual_anchor_ordinal = anchor_ordinal;
}

// Itanium C++ ABI 2.5.2 (clang's VCallAndVBaseOffsetBuilder): a class's
// vtable prefix is its primary base's (the base's vcall entries too when it
// is a virtual primary), then its other virtual bases' vbase offsets in
// inheritance graph order; a virtual base's table adds a vcall offset for
// each virtual function signature declared in it, its non-virtual primary
// chain first and its other non-virtual bases after, not already present.
void Analyzer::AssignVtablePrefixes(EntityId entity)
{
	ClassPolymorphismFacts& facts = class_polymorphism_[entity];
	const EntityRecord& owner = program_->entities[entity];
	facts.prefix.clear();
	facts.virtual_prefix.clear();
	if (!facts.primary_ancestors.empty())
	{
		const ClassPolymorphismFacts& primary =
			class_polymorphism_[facts.primary_ancestors[0]];
		facts.prefix = primary.prefix;
		if (facts.primary_virtual)
			facts.prefix.insert(facts.prefix.end(),
				primary.virtual_prefix.begin(), primary.virtual_prefix.end());
	}
	facts.virtual_base_rows.assign(owner.virtual_base_count, kNoDumpEdge);
	for (std::size_t entry = 0; entry < facts.prefix.size(); ++entry)
	{
		std::uint32_t ordinal = 0;
		if (facts.prefix[entry].virtual_base != kNoEntity &&
			program_->FindVirtualBase(entity, facts.prefix[entry].virtual_base,
				0, &ordinal))
			facts.virtual_base_rows[ordinal] = static_cast<std::uint32_t>(entry);
	}
	for (std::size_t base = 0; base < owner.virtual_base_count; ++base)
		if (facts.virtual_base_rows[base] == kNoDumpEdge)
		{
			facts.virtual_base_rows[base] =
				static_cast<std::uint32_t>(facts.prefix.size());
			facts.prefix.push_back(VtablePrefixEntry(
				program_->VirtualBase(entity, base).entity));
		}
	std::vector<FunctionSignatureKey> present;
	for (std::size_t entry = 0; entry < facts.prefix.size(); ++entry)
		if (facts.prefix[entry].function != kNoBinding)
			present.push_back(VirtualSignatureKey(facts.prefix[entry].function));
	const std::function<FunctionSignatureKey(BindingId)> key_of =
		[this](BindingId binding) { return VirtualSignatureKey(binding); };
	AppendVcallEntries(*program_, class_polymorphism_, entity_member_functions_,
		key_of, entity, &present, &facts.virtual_prefix);
}

void Analyzer::FinalizeClassPolymorphismViews(EntityId entity)
{
	if (entity >= class_polymorphism_.size()) return;
	DropUnadjustedCovariantEntries(*program_, class_polymorphism_, entity,
		[this](BindingId binding) { return VirtualSignatureKey(binding); },
		&virtual_slot_by_binding_);
	ClassPolymorphismFacts& facts = class_polymorphism_[entity];
	AssignVtablePrefixes(entity);
	FillVtablePrefixValues(*program_, entity, facts.prefix, 0,
		&facts.virtual_base_offsets);
	for (std::size_t view = 0; view < facts.views.size(); ++view)
	{
		PolymorphicViewFact& current = facts.views[view];
		const bool inherited_virtual_view = current.virtual_base;
		if (current.direct_base_ordinal >=
			program_->entities[entity].direct_base_count)
			ThrowInternalCompilerError("polymorphic view has no direct-base owner");
		const DirectBaseEdge& edge = program_->DirectBase(
			entity, current.direct_base_ordinal);
		AnchorPolymorphicView(entity, edge, &current);
		current.virtual_base = false;
		current.virtual_base_ordinal = kNoDumpEdge;
		std::uint64_t virtual_offset = 0;
		std::uint32_t virtual_ordinal = 0;
		if (program_->FindVirtualBase(entity, current.entity,
			&virtual_offset, &virtual_ordinal) &&
			(inherited_virtual_view || virtual_offset == current.offset))
		{
			current.virtual_base = true;
			current.virtual_base_ordinal = virtual_ordinal;
			current.offset = virtual_offset;
			current.virtual_anchor = current.entity;
			current.virtual_anchor_ordinal = virtual_ordinal;
			current.virtual_anchor_offset = 0;
		}
		std::vector<VtablePrefixEntry> entries =
			class_polymorphism_[current.entity].prefix;
		if (current.virtual_base)
			entries.insert(entries.end(),
				class_polymorphism_[current.entity].virtual_prefix.begin(),
				class_polymorphism_[current.entity].virtual_prefix.end());
		FillVtablePrefixValues(*program_, entity, entries, current.offset,
			&current.virtual_base_offsets);
		current.address_point = 8 * (current.virtual_base_offsets.size() + 2);
	}
	MarkSharedPolymorphicViews(class_polymorphism_, entity);
	OrderPolymorphicViews(*program_, class_polymorphism_, entity);
	facts.address_point = 8 * (facts.virtual_base_offsets.size() + 2);
	const SignatureKeyOf key_of =
		[this](BindingId binding) { return VirtualSignatureKey(binding); };
	const VtablePlacement placement(*program_, entity, 0);
	for (std::size_t view = 0; view <= facts.views.size(); ++view)
	{
		std::vector<VirtualSlotFact>& slots = SlotsForView(&facts, view);
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
			ResolveOverrider(*program_, entity, &slots[slot]);
		PlaceTableSlots(placement, class_polymorphism_, key_of,
			view == 0 ? entity : facts.views[view - 1].entity,
			view == 0 ? kNoEntity : facts.views[view - 1].virtual_anchor,
			view == 0 ? 0 : facts.views[view - 1].offset, &slots);
		for (std::size_t slot = 0; slot < slots.size(); ++slot)
		{
			slots[slot].return_adjustment = 0;
			slots[slot].return_vtable_offset = 0;
			slots[slot].return_adjustment_virtual = false;

			const EntityId result = VirtualResultEntity(
				*program_, slots[slot].function);
			const EntityId root_result = VirtualResultEntity(
				*program_, slots[slot].root);
			if (result == kNoEntity || root_result == kNoEntity ||
				result == root_result) continue;
			std::uint64_t result_offset = 0;
			if (program_->HasVirtualBasePath(result, root_result))
			{
				// Itanium 5.1.4: the result moves to the virtual base its
				// overridden result lies in by that base's vbase offset,
				// then by the fixed offset inside it.
				std::size_t virtual_ordinal = 0;
				const std::size_t count = program_->entities[result].virtual_base_count;
				while (virtual_ordinal < count)
				{
					const EntityId base =
						program_->VirtualBase(result, virtual_ordinal).entity;
					result_offset = 0;
					if (base == root_result || (!program_->HasVirtualBasePath(
						base, root_result) && program_->QueryBasePath(
							base, root_result, 0, 0, &result_offset))) break;
					++virtual_ordinal;
				}
				if (result >= class_polymorphism_.size() || virtual_ordinal == count)
					ThrowInternalCompilerError(
						"covariant result has no finalized virtual-base slot");
				slots[slot].return_adjustment_virtual = true;
				slots[slot].return_adjustment = static_cast<std::int64_t>(result_offset);
				slots[slot].return_vtable_offset = -24 - 8 *
					static_cast<std::int64_t>(VirtualBaseRow(
						class_polymorphism_, result, virtual_ordinal));
			}
			else
			{
				if (!program_->QueryBasePath(
					result, root_result, 0, 0, &result_offset))
					ThrowInternalCompilerError(
						"covariant result has no finalized base path");
				slots[slot].return_adjustment =
					static_cast<std::int64_t>(result_offset);
			}
		}
	}
	FillVtableVcallValues(placement, class_polymorphism_, key_of, entity);
	BuildConstructionGroups(*program_, class_polymorphism_, key_of, entity);
}

}
}
