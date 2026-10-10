#include "lower/program_lowering.h"

#include "lower/function_lowering.h"
#include "sema/analyzer.h"
#include "sema/conversions.h"

// Virtual bases (10.1p4, Itanium C++ ABI 2.4-2.6): a class with one has a
// table pointer, its table a row per virtual base giving that base's
// offset from the subobject (the first base's row nearest the address
// point, a primary base's rows first); a conversion through a virtual
// base reads that row (a complete object's virtual base is at its offset
// there). Base-object constructors and destructors take the class's VTT
// (its tables for the construction under way: the complete object's, or
// a more derived class's construction tables) after `this`, and no
// parameter carries a virtual base's address; the complete-object entries
// construct the virtual bases and pass the bases their part of the
// class's VTT (Itanium 2.6: the primary table, the sub-VTTs of the direct
// non-virtual bases with virtual bases, the secondary virtual pointers in
// inheritance graph order, then the virtual bases' VTTs).
namespace lower {

namespace {

const sema::ClassEntity* BaseClassOf(const sema::BaseClass* base)
{
	return static_cast<const sema::ClassEntity*>(base->base);
}

// A segment whose table pointer a base-object entry takes from its VTT
// (index `i` in its group): one with a pointer of its own, or a virtual
// base sharing another's (a virtual primary base: Itanium 2.6.2 lists
// every virtual base, its table where it shares it).
bool VttSegment(const sema::VtableSegment& segment, uint32_t i)
{
	return (!segment.shared || segment.within == i) && (VirtualBaseCount(segment.cls) != 0 || segment.within != 0);
}

} // namespace

uint32_t ProgramLowering::vbase_rows(const sema::ClassEntity* cls)
{
	// The key of the class's virtual bases' places: its list (the primary
	// base's virtual bases, then the class's others in inheritance graph
	// order), each virtual base's place entered with it. A class with its
	// primary base's virtual bases in the same order (as a class deriving
	// from one class with virtual bases, adding none) has that base's list
	// and places.
	const uint32_t found = vbase_rows_.find(cls);
	if (found != PointerMap::kMissing)
		return found;
	const sema::BaseClass* primary = cls->virtuals != nullptr ? cls->virtuals->primary_base : nullptr;
	const sema::ClassEntity* inherited = primary != nullptr && VirtualBaseCount(BaseClassOf(primary)) != 0
		? BaseClassOf(primary)
		: nullptr;
	const uint32_t inherited_first = inherited != nullptr ? vbase_rows(inherited) : 0;
	const uint32_t inherited_count = inherited != nullptr ? VirtualBaseCount(inherited) : 0;
	const uint32_t count = VirtualBaseCount(cls);
	bool same = inherited_count == count;
	for (uint32_t i = 0; same && i < count; ++i)
		same = VirtualBases(cls)[i].base->base == VirtualBases(inherited)[i].base->base;
	if (same)
	{
		vbase_rows_.set(cls, inherited_first);
		return inherited_first;
	}
	const uint32_t first = static_cast<uint32_t>(vbase_row_store_.size());
	vbase_row_store_.resize(first + count);
	for (uint32_t k = 0; k < inherited_count; ++k)
		vbase_row_store_[first + k] = vbase_row_store_[inherited_first + k];
	uint32_t next = inherited_count;
	for (uint32_t i = 0; i < count; ++i)
	{
		const sema::ClassEntity* base = BaseClassOf(VirtualBases(cls)[i].base);
		const VirtualBasePlace* listed = inherited != nullptr
			? virtual_base_places_.find(PlaceKey{ inherited_first, base })
			: nullptr;
		const uint32_t row = listed != nullptr ? listed->row : next++;
		vbase_row_store_[first + row] = base;
		virtual_base_places_.insert(PlaceKey{ first, base }, VirtualBasePlace{ i, row, PointerMap::kMissing });
	}
	vbase_rows_.set(cls, first);
	return first;
}

ProgramLowering::VirtualBasePlace ProgramLowering::virtual_base_place(
	const sema::ClassEntity* cls, const sema::Entity* base)
{
	const VirtualBasePlace* place = VirtualBaseCount(cls) != 0
		? virtual_base_places_.find(PlaceKey{ vbase_rows(cls), base })
		: nullptr;
	return place != nullptr ? *place
							: VirtualBasePlace{ PointerMap::kMissing, PointerMap::kMissing, PointerMap::kMissing };
}

uint32_t ProgramLowering::virtual_base_position(const sema::ClassEntity* cls, const sema::Entity* base)
{
	return virtual_base_place(cls, base).position;
}

size_t ProgramLowering::PlaceHash::operator()(const PlaceKey& key) const
{
	return (static_cast<uint64_t>(key.rows) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.base);
}

size_t ProgramLowering::ClassPairHash::operator()(const ClassPair& key) const
{
	return (reinterpret_cast<uintptr_t>(key.cls) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.base);
}

const sema::BaseClass* const* ProgramLowering::virtual_base_order(const sema::ClassEntity* cls)
{
	// 12.6.2p10's order of a class's virtual bases (depth first, left to
	// right; a virtual base's own virtual bases before it, as GCC and Clang
	// construct them), as the class's base-specifiers for them: each direct
	// base's order, then that base when it is virtual, each class once.
	// Once per class, its bases' orders first (an explicit stack).
	const uint32_t found = virtual_orders_.find(cls);
	if (found != PointerMap::kMissing)
		return virtual_order_store_.data() + found;
	std::vector<const sema::ClassEntity*>& pending = order_stack_;
	pending.assign(1, cls);
	while (!pending.empty())
	{
		const sema::ClassEntity* top = pending.back();
		const size_t waiting = pending.size();
		for (const sema::BaseClass* base = top->bases; base != nullptr; base = base->next)
		{
			const sema::ClassEntity* next = BaseClassOf(base);
			if (VirtualBaseCount(next) != 0 && virtual_orders_.find(next) == PointerMap::kMissing)
				pending.push_back(next);
		}
		if (pending.size() != waiting)
			continue;
		pending.pop_back();
		if (virtual_orders_.find(top) != PointerMap::kMissing)
			continue;
		// Each virtual base's position in the class's list, and whether it
		// is ordered yet.
		PointerMap& position = order_positions_;
		position.clear();
		const uint32_t count = VirtualBaseCount(top);
		for (uint32_t i = 0; i < count; ++i)
			position.set(VirtualBases(top)[i].base->base, i);
		order_listed_.assign(count, 0);
		const uint32_t first = static_cast<uint32_t>(virtual_order_store_.size());
		const auto place = [&](const sema::Entity* shared) {
			const uint32_t at = position.find(shared);
			if (order_listed_[at] != 0)
				return;
			order_listed_[at] = 1;
			virtual_order_store_.push_back(VirtualBases(top)[at].base);
		};
		for (const sema::BaseClass* base = top->bases; base != nullptr; base = base->next)
		{
			const sema::ClassEntity* next = BaseClassOf(base);
			const uint32_t inherited = VirtualBaseCount(next) != 0 ? virtual_orders_.find(next) : 0;
			for (uint32_t i = 0; i < VirtualBaseCount(next); ++i)
				place(virtual_order_store_[inherited + i]->base);
			if (base->is_virtual)
				place(next);
		}
		virtual_orders_.set(top, first);
	}
	return virtual_order_store_.data() + virtual_orders_.find(cls);
}

int64_t ProgramLowering::vbase_row_offset(const sema::ClassEntity* cls, const sema::ClassEntity* base)
{
	return itanium_vbase_offset_offset(cls, base);
}

uint64_t ProgramLowering::virtual_base_offset(const sema::ClassEntity* complete, const sema::ClassEntity* base)
{
	// Its offset in a complete object (laid out first).
	size_of(complete->type, 0);
	const uint32_t position = virtual_base_position(complete, base);
	if (position == PointerMap::kMissing)
		throw LoweringError(0, "a virtual base the class does not have");
	return VirtualBases(complete)[position].base->offset;
}

ProgramLowering::BaseRoute ProgramLowering::base_route(
	const sema::ClassEntity* derived, const sema::ClassEntity* base, uint32_t location)
{
	// Through non-virtual bases when they reach it (the analysis made the
	// subobject unique), else through the first virtual base of `derived`
	// (in inheritance graph order) that it is, or lies in (through
	// non-virtual bases). A class with virtual bases keeps its routes.
	BaseRoute route = BaseRoute{ nullptr, 0 };
	if (VirtualBaseCount(derived) == 0)
	{
		if (base_path_offset(derived, base, route.offset, location))
			return route;
		throw LoweringError(location, "a conversion to a class that is not a base");
	}
	const ClassPair key = ClassPair{ derived, base };
	if (const BaseRoute* known = base_routes_.find(key))
		return *known;
	if (!base_path_offset(derived, base, route.offset, location))
	{
		const uint32_t position = virtual_base_position(derived, base);
		const uint32_t holder = virtual_holder(derived, base);
		if (holder != PointerMap::kMissing && holder < position)
		{
			route.via = BaseClassOf(VirtualBases(derived)[holder].base);
			if (!base_path_offset(route.via, base, route.offset, location))
				throw LoweringError(location, "a base its virtual base's subobject does not hold");
		}
		else if (position != PointerMap::kMissing)
			route = BaseRoute{ BaseClassOf(VirtualBases(derived)[position].base), 0 };
		else
			throw LoweringError(location, "a conversion to a class that is not a base");
	}
	base_routes_.insert(key, route);
	return route;
}

uint32_t ProgramLowering::virtual_holder(const sema::ClassEntity* derived, const sema::ClassEntity* base)
{
	// The position of the first of the class's virtual bases below which
	// `base` lies through non-virtual bases (kMissing for none). At the
	// class's first query each class below its virtual bases is entered
	// with the first one's position, keyed by the index of the class's
	// rows (as the places): a virtual base below an earlier one holds only
	// classes that one holds, and so does a class entered before.
	const uint32_t rows = vbase_rows(derived);
	if (virtual_holders_.find(PlaceKey{ rows, nullptr }) == nullptr)
	{
		virtual_holders_.insert(PlaceKey{ rows, nullptr }, static_cast<uint32_t>(PointerMap::kMissing));
		std::vector<const sema::ClassEntity*>& stack = holder_walk_;
		for (uint32_t i = 0; i < VirtualBaseCount(derived); ++i)
		{
			const sema::ClassEntity* shared = BaseClassOf(VirtualBases(derived)[i].base);
			if (virtual_holders_.find(PlaceKey{ rows, shared }) != nullptr)
				continue;
			stack.assign(1, shared);
			while (!stack.empty())
			{
				const sema::ClassEntity* cls = stack.back();
				stack.pop_back();
				for (const sema::BaseClass* step = cls->bases; step != nullptr; step = step->next)
				{
					const sema::ClassEntity* next = BaseClassOf(step);
					if (step->is_virtual || virtual_holders_.find(PlaceKey{ rows, next }) != nullptr)
						continue;
					virtual_holders_.insert(PlaceKey{ rows, next }, i);
					stack.push_back(next);
				}
			}
		}
	}
	const uint32_t* holder = virtual_holders_.find(PlaceKey{ rows, base });
	return holder != nullptr ? *holder : static_cast<uint32_t>(PointerMap::kMissing);
}

uint32_t ProgramLowering::vtt_size(const sema::ClassEntity* cls)
{
	// A class's VTT as a base's (its sub-VTT): its primary table, its
	// direct non-virtual bases' sub-VTTs, its VTT segments' pointers.
	const uint32_t found = vtt_sizes_.find(cls);
	if (found != PointerMap::kMissing)
		return found;
	uint32_t size = 1;
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (!base->is_virtual && VirtualBaseCount(BaseClassOf(base)) != 0)
			size += vtt_size(BaseClassOf(base));
	}
	const sema::VtableGroup* group = cls->virtuals != nullptr ? cls->virtuals->group : nullptr;
	for (uint32_t i = 1; group != nullptr && i < group->segment_count; ++i)
		size += VttSegment(group->segments[i], i) ? 1 : 0;
	vtt_sizes_.set(cls, size);
	return size;
}

uint32_t ProgramLowering::vtt_positions(const sema::ClassEntity* cls)
{
	// After the primary table, the direct non-virtual bases' sub-VTTs (each
	// base-specifier's position is the one past those before it), then a
	// pointer per VTT segment, in vtt_pointer_order's order.
	const uint32_t found = vtt_segments_.find(cls);
	if (found != PointerMap::kMissing)
		return found;
	uint32_t index = 1;
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		vtt_base_positions_.set(base, index);
		if (!base->is_virtual && VirtualBaseCount(BaseClassOf(base)) != 0)
			index += vtt_size(BaseClassOf(base));
	}
	const uint32_t first = static_cast<uint32_t>(vtt_segment_store_.size());
	vtt_segment_store_.push_back(index);
	const sema::VtableGroup* group = cls->virtuals != nullptr ? cls->virtuals->group : nullptr;
	if (group != nullptr)
	{
		vtt_segment_store_.resize(first + group->segment_count, index);
		const uint32_t order = vtt_pointer_order(cls);
		for (uint32_t k = 0; k < vtt_order_store_[order]; ++k)
			vtt_segment_store_[first + vtt_order_store_[order + 1 + k]] = index++;
	}
	vtt_segments_.set(cls, first);
	return first;
}

uint32_t ProgramLowering::vtt_pointer_order(const sema::ClassEntity* cls)
{
	// The segments whose table pointers follow the bases' sub-VTTs in a
	// sub-VTT of `cls` (Itanium 2.6.2's secondary virtual pointers,
	// inheritance graph preorder: a virtual base where the walk first meets
	// it): the index of their count in vtt_order_store_, the segments after
	// it. Each base
	// subobject the walk lists is found by its offset and class.
	const uint32_t found = vtt_orders_.find(cls);
	if (found != PointerMap::kMissing)
		return found;
	const sema::VtableGroup* group = cls->virtuals->group;
	const uint64_t* offsets = segment_offsets(cls);
	vtt_subobjects_.clear(group->segment_count);
	uint32_t expected = 0;
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		vtt_subobjects_.insert(SubobjectKey{ offsets[i], group->segments[i].cls }, i);
		expected += VttSegment(group->segments[i], i) ? 1 : 0;
	}
	const uint32_t first = static_cast<uint32_t>(vtt_order_store_.size());
	vtt_order_store_.push_back(0);
	vtt_walked_.clear();
	vtt_pointer_walk(cls, cls, 0, false);
	vtt_order_store_[first] = static_cast<uint32_t>(vtt_order_store_.size() - first - 1);
	if (vtt_order_store_[first] != expected)
		throw LoweringError(0, "a VTT whose virtual pointers the inheritance graph does not list");
	vtt_orders_.set(cls, first);
	return first;
}

void ProgramLowering::vtt_pointer_walk(
	const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, bool morally_virtual)
{
	// Clang's VTTBuilder::LayoutSecondaryVirtualPointers: each dynamic base
	// in declaration order (a virtual base once), listed unless it is its
	// class's non-virtual primary base when it has virtual bases or lies
	// in a virtual base, then the bases below it (a virtual primary base
	// is listed: its table is the one it shares).
	if (VirtualBaseCount(cls) == 0 && !morally_virtual)
		return;
	const sema::BaseClass* primary = cls->virtuals != nullptr ? cls->virtuals->primary_base : nullptr;
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		const sema::ClassEntity* next = BaseClassOf(base);
		if (!next->dynamic)
			continue;
		bool moral = morally_virtual;
		uint64_t at = offset + base->offset;
		if (base->is_virtual)
		{
			if (vtt_walked_.find(next) != PointerMap::kMissing)
				continue;
			vtt_walked_.set(next, 1);
			at = virtual_base_offset(complete, next);
			moral = true;
		}
		if ((base != primary || base->is_virtual) && (VirtualBaseCount(next) != 0 || moral))
		{
			const uint32_t* segment = vtt_subobjects_.find(SubobjectKey{ at, next });
			if (segment == nullptr || !VttSegment(complete->virtuals->group->segments[*segment], *segment))
				throw LoweringError(0, "a VTT's virtual pointer for a subobject without a table segment");
			vtt_order_store_.push_back(*segment);
		}
		vtt_pointer_walk(complete, next, at, moral);
	}
}

size_t ProgramLowering::SubobjectHash::operator()(const SubobjectKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.cls) * 0x9E3779B97F4A7C15ull) ^ key.offset;
}

uint32_t ProgramLowering::vtt_base_index(const sema::ClassEntity* cls, const sema::BaseClass* direct)
{
	const uint32_t first = vtt_positions(cls);
	return direct != nullptr ? vtt_base_positions_.find(direct) : vtt_segment_store_[first];
}

uint32_t ProgramLowering::vtt_segment_index(const sema::ClassEntity* cls, uint32_t segment)
{
	return vtt_segment_store_[vtt_positions(cls) + segment];
}

uint32_t ProgramLowering::vtt_virtual_index(const sema::ClassEntity* cls, const sema::ClassEntity* base)
{
	// The virtual bases' VTTs follow the complete class's sub-VTT, in
	// inheritance graph order (those with virtual bases have one); their
	// positions past it are entered in the places of all the class's
	// virtual bases at its first query.
	const VirtualBasePlace place = virtual_base_place(cls, base);
	if (place.position == PointerMap::kMissing)
		throw LoweringError(0, "a virtual base the class does not have");
	if (place.vtt != PointerMap::kMissing)
		return vtt_size(cls) + place.vtt;
	const uint32_t rows = vbase_rows(cls);
	uint32_t index = 0;
	for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
	{
		const sema::ClassEntity* shared = BaseClassOf(VirtualBases(cls)[i].base);
		virtual_base_places_.find(PlaceKey{ rows, shared })->vtt = index;
		if (VirtualBaseCount(shared) != 0)
			index += vtt_size(shared);
	}
	return vtt_size(cls) + virtual_base_places_.find(PlaceKey{ rows, base })->vtt;
}

lowir::SymbolId ProgramLowering::vtt_symbol(const sema::ClassEntity* entity)
{
	// `<class>____vtt` (the course's spelling, `_ZTT`), with the class's
	// tables: defined where they are, weak.
	const uint32_t found = vtts_.find(entity);
	if (found != PointerMap::kMissing)
		return found;
	vtable_symbol(entity);
	const std::string object = class_object(entity, abi_mangle::ABI_TARGET_VTT);
	const bool internal = entity_locality(entity) != Locality::External;
	const std::string spelling = class_spelling(entity) + "____vtt";
	lowir::SymbolId symbol = lowir::kNoId;
	if (internal)
	{
		symbol = new_symbol(spelling);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, spelling);
	vtts_.set(entity, symbol);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	if (!emits_vtable(entity))
	{
		PendingDeclaration pending = PendingDeclaration();
		pending.symbol = symbol;
		pending.function = false;
		pending.typed = false;
		pending.metadata.binding = internal ? ir_model::SBM_INTERNAL : ir_model::SBM_STRONG;
		if (symbol_objects_[symbol] != 0)
			pending.metadata.object = symbol_objects_[symbol];
		set_pending(symbol, pending);
		return symbol;
	}
	std::vector<TableAddress> entries;
	vtt_entries(entity, entity, 0, true, entries);
	const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
	for (const TableAddress& entry : entries)
		data_item(AddressItem(entry.symbol, entry.addend));
	lowir::SymbolMetadata metadata;
	metadata.binding = internal ? ir_model::SBM_INTERNAL : ir_model::SBM_WEAK;
	metadata.flags |= lowir::SF_OBJECT_ROOT;
	define_data(symbol, first, metadata);
	return symbol;
}

void ProgramLowering::vtt_entries(const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset,
	bool top, std::vector<TableAddress>& out)
{
	// `cls` at `offset` in `complete`: its sub-VTT (construction tables
	// unless it is the complete class), then, at the top, the virtual
	// bases' VTTs.
	out.push_back(top ? table_address(complete, 0) : construction_address(complete, cls, offset, 0));
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (!base->is_virtual && VirtualBaseCount(BaseClassOf(base)) != 0)
			vtt_entries(complete, BaseClassOf(base), offset + base->offset, false, out);
	}
	if (cls->virtuals->group != nullptr)
	{
		const uint32_t order = vtt_pointer_order(cls);
		for (uint32_t k = 0; k < vtt_order_store_[order]; ++k)
		{
			const uint32_t i = vtt_order_store_[order + 1 + k];
			out.push_back(top ? table_address(complete, i) : construction_address(complete, cls, offset, i));
		}
	}
	for (uint32_t i = 0; top && i < VirtualBaseCount(complete); ++i)
	{
		const sema::BaseClass* base = VirtualBases(complete)[i].base;
		if (VirtualBaseCount(BaseClassOf(base)) != 0)
			vtt_entries(complete, BaseClassOf(base), base->offset, false, out);
	}
}

uint64_t ProgramLowering::construction_segment_offset(
	const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment)
{
	// A segment of `cls`'s group, `cls` at `offset` in `complete`: its
	// non-virtual subobjects where `cls` is, those within its virtual bases
	// where `complete` put those bases.
	const sema::VtableGroup* group = cls->virtuals->group;
	const uint64_t* offsets = segment_offsets(cls);
	const uint32_t within = group->segments[segment].within;
	if (within == 0)
		return offset + offsets[segment];
	return virtual_base_offset(complete, group->segments[within].cls) + offsets[segment] - offsets[within];
}

ProgramLowering::TableAddress ProgramLowering::construction_address(
	const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment)
{
	// Itanium's construction vtable group (2.6.4), `_ZTC <complete>
	// <offset> _ <cls>`: `cls`'s tables as `cls` at `offset` in `complete`
	// sees them under construction (`cls`'s final overriders and type
	// information, `complete`'s virtual base offsets and vcall offsets),
	// its primary table then each segment with a table pointer of its own
	// there that the VTT names (one with virtual bases, or within a virtual
	// base: 2.6.4 leaves out the others), in subobject order. A virtual
	// base sharing its parent's table pointer in `cls`'s layout shares it
	// here only where `complete` put it at its parent's offset (Clang's
	// DeterminePrimaryVirtualBases); else it has a table of its own. The
	// group is unit-internal: only this unit's VTT names it. Object mode
	// makes it one object (as GCC and Clang lay it out); the LowIR text
	// gives each table a global of its own,
	// `<complete>____construction__<cls>__<offset>__s<segment>__vtable`.
	// A segment's address is its table's address point.
	const sema::VtableGroup* group = cls->virtuals->group;
	segment = construction_owner(complete, cls, offset, segment);
	const ConstructionKey identity = ConstructionKey{ complete, cls, offset };
	if (const uint32_t* known = construction_tables_.find(identity))
		return TableAddress{ construction_symbols_[*known + segment], construction_points_[*known + segment] };
	const uint32_t slots = static_cast<uint32_t>(construction_symbols_.size());
	construction_symbols_.resize(slots + group->segment_count, lowir::kNoId);
	construction_points_.resize(slots + group->segment_count, 0);
	construction_tables_.insert(identity, slots);
	abi_mangle::AbiTarget target;
	target.kind = abi_mangle::ABI_TARGET_CONSTRUCTION_VTABLE;
	target.node = abi_type(complete->type);
	target.base = abi_type(cls->type);
	target.offset = static_cast<long long>(offset);
	std::string object;
	encoder_->encode(target, object);
	++counters_.mangled_names;
	const sema::FunctionEntity* key = complete->virtuals->key_function;
	const uint8_t binding = entity_locality(complete) != Locality::External ? ir_model::SBM_INTERNAL
		: key != nullptr && !key->is_inline                                 ? ir_model::SBM_STRONG
																			: ir_model::SBM_WEAK;
	const TableContext context = TableContext{ complete, cls, offset, true };
	// The tables' entries first (defining them may append data).
	std::vector<uint32_t> owners;
	std::vector<lowir::SymbolId> targets;
	for (uint32_t s = 0; s < group->segment_count; ++s)
	{
		if (s != 0 && (construction_owner(complete, cls, offset, s) != s || !VttSegment(group->segments[s], s)))
			continue;
		owners.push_back(s);
		const sema::VtableSegment& own = group->segments[s];
		const sema::VirtualSlot* slots_of = analyzer_->virtual_slots(own.cls);
		for (uint32_t slot = 0; slot < own.cls->virtuals->vtable_size; ++slot)
		{
			const sema::VtableEntry& entry = group->entries[own.first_entry + slot];
			targets.push_back(table_entry(entry.function, entry.kind, construction_adjustment(context, s, slot),
				binding, slots_of[slot].adjusted, own.cls));
		}
	}
	const lowir::SymbolId information = typeinfo_symbol(cls);
	// (One class_spelling per statement, the last first: see
	// FunctionLowering::binary.)
	const std::string base = class_spelling(cls);
	const std::string prefix = class_spelling(complete) + "____construction__" + base + "__" + std::to_string(offset);
	lowir::SymbolMetadata metadata;
	metadata.binding = ir_model::SBM_INTERNAL;
	uint32_t first = static_cast<uint32_t>(program_.data_items.size());
	int64_t position = 0;
	size_t next = 0;
	for (const uint32_t s : owners)
	{
		const uint32_t rows = row_count(group, s);
		const uint32_t size = group->segments[s].cls->virtuals->vtable_size;
		table_rows(context, s);
		data_item(ScalarItem(lowir::TypeKind::I64, offset - construction_segment_offset(complete, cls, offset, s)));
		data_item(AddressItem(information, 0));
		for (uint32_t slot = 0; slot < size; ++slot)
			data_item(EntryItem(targets[next++]));
		construction_points_[slots + s] = position + static_cast<int64_t>(kTableHeaderBytes + 8 * rows);
		++counters_.vtables;
		if (object_mode_)
		{
			position += 8 * static_cast<int64_t>(rows + 2 + size);
			continue;
		}
		// Each table a global of its own, the group's first carrying its
		// ABI name.
		const lowir::SymbolId symbol = new_symbol(prefix + "__s" + std::to_string(s) + "__vtable");
		if (s == 0)
			symbol_objects_[symbol] = local_object(object, false);
		construction_symbols_[slots + s] = symbol;
		define_data(symbol, first, metadata);
		first = static_cast<uint32_t>(program_.data_items.size());
	}
	if (object_mode_)
	{
		const lowir::SymbolId symbol = new_symbol(prefix + "__vtable");
		symbol_objects_[symbol] = local_object(object, false);
		define_data(symbol, first, metadata);
		for (const uint32_t s : owners)
			construction_symbols_[slots + s] = symbol;
	}
	for (uint32_t s = 0; s < group->segment_count; ++s)
	{
		const uint32_t owner = construction_owner(complete, cls, offset, s);
		construction_symbols_[slots + s] = construction_symbols_[slots + owner];
		construction_points_[slots + s] = construction_points_[slots + owner];
	}
	return TableAddress{ construction_symbols_[slots + segment], construction_points_[slots + segment] };
}

uint32_t ProgramLowering::construction_owner(
	const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, uint32_t segment)
{
	// Each context's owners once (context_owners_).
	const ConstructionKey identity = ConstructionKey{ complete, cls, offset };
	if (const uint32_t* known = context_owners_.find(identity))
		return context_owner_store_[*known + segment];
	const sema::VtableGroup* group = cls->virtuals->group;
	const uint32_t count = group->segment_count;
	const uint32_t first = static_cast<uint32_t>(context_owner_store_.size());
	context_owners_.insert(identity, first);
	// Where the context puts each segment; the segments by their class's
	// virtual primary base; the virtual primary bases of `cls`'s chain.
	std::vector<uint64_t> at(count);
	for (uint32_t k = 0; k < count; ++k)
		at[k] = construction_segment_offset(complete, cls, offset, k);
	PointerMap& heads = owner_heads_;
	heads.clear();
	std::vector<uint32_t> next(count, PointerMap::kMissing);
	for (uint32_t k = count; k-- > 1;)
	{
		const sema::BaseClass* primary = group->segments[k].cls->virtuals->primary_base;
		if (primary == nullptr || !primary->is_virtual)
			continue;
		next[k] = heads.find(primary->base);
		heads.set(primary->base, k);
	}
	PointerMap& chained = owner_chain_;
	chained.clear();
	for (const sema::ClassEntity* link = cls; link != nullptr;)
	{
		const sema::BaseClass* primary = link->virtuals->primary_base;
		if (primary != nullptr && primary->is_virtual)
			chained.set(primary->base, 1);
		link = primary != nullptr && !primary->is_virtual ? BaseClassOf(primary) : nullptr;
	}
	// A segment's table pointer: its parent's along a non-virtual chain; a
	// virtual base's, that of a subobject whose class has it as its primary
	// base where the context puts it at that subobject (Clang's
	// DeterminePrimaryVirtualBases), else its own.
	std::vector<uint32_t> parent(count, PointerMap::kMissing);
	for (uint32_t s = 1; s < count; ++s)
	{
		const sema::VtableSegment& part = group->segments[s];
		if (part.within != s)
		{
			if (part.shared)
				parent[s] = part.parent;
			continue;
		}
		for (uint32_t k = heads.find(part.cls); k != PointerMap::kMissing && parent[s] == PointerMap::kMissing;
			k = next[k])
		{
			if (k != s && at[k] == at[s])
				parent[s] = k;
		}
		if (parent[s] == PointerMap::kMissing && at[s] == at[0] && chained.find(part.cls) != PointerMap::kMissing)
			parent[s] = 0;
	}
	context_owner_store_.resize(first + count, PointerMap::kMissing);
	for (uint32_t s = 0; s < count; ++s)
	{
		uint32_t root = s;
		for (uint32_t steps = 0; parent[root] != PointerMap::kMissing && steps < count; ++steps)
			root = parent[root];
		context_owner_store_[first + s] = root;
	}
	return context_owner_store_[first + segment];
}

size_t ProgramLowering::ConstructionHash::operator()(const ConstructionKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.complete) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.cls) ^
		(key.offset << 20);
}

uint32_t ProgramLowering::row_count(const sema::VtableGroup* group, uint32_t segment)
{
	return group != nullptr ? itanium_table_rows(group, segment) : 0;
}

void ProgramLowering::table_rows(const TableContext& context, uint32_t segment)
{
	// The Itanium rows (2.5.2, as Clang's VCallAndVBaseOffsetBuilder lays
	// them out), the last first.
	const sema::VtableGroup* group = context.cls->virtuals->group;
	if (group == nullptr)
		return;
	const uint32_t count = itanium_table_rows(group, segment);
	const int64_t* values = itanium_row_values(context, segment);
	for (uint32_t i = count; i-- > 0;)
		data_item(ScalarItem(lowir::TypeKind::I64, static_cast<uint64_t>(values[i])));
}

// ------------------------------------------------------------------ bodies

lowir::Value FunctionLowering::virtual_base_address(
	lowir::Value object, const sema::ClassEntity* cls, const sema::ClassEntity* base)
{
	// The row for the base in the object's table: its offset from the
	// object (Itanium 2.5.2). `this` in a base-object entry taking a VTT
	// reads the row from the construction table the VTT names, as its own
	// table pointer may still hold a base's (Itanium 2.6); the VTT never
	// changes, so one load of that table and of each row serves a block. A
	// complete-object constructor's `this` before its tables are stored (a
	// mem-initializer's operand, 12.7p3) has the offset in the complete
	// object.
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const bool own_this = (tables_pending_ || vtt_param_.kind != lowir::ValueKind::None) &&
		cls == sema::ClassOf(function_->owner->entity->type) && is_this_value(object);
	if (own_this && vtt_param_.kind != lowir::ValueKind::None)
		return index(i8, ir_model::IPK_NONE, object, construction_row(cls, base)).value;
	if (own_this)
		return index(i8, ir_model::IPK_NONE, object, integer(i64, lowering_.virtual_base_offset(cls, base))).value;
	Place vptr = Place();
	vptr.storage = object;
	Place row = Place();
	row.storage = index(i8, ir_model::IPK_NONE, load(pointer, vptr).value,
		integer(i64, static_cast<uint64_t>(lowering_.vbase_row_offset(cls, base))))
					  .value;
	return index(i8, ir_model::IPK_NONE, object, load(i64, row)).value;
}

bool FunctionLowering::is_this_value(lowir::Value object) const
{
	for (size_t i = this_values_.size(); i-- > 0;)
	{
		if (this_values_[i].kind == object.kind && this_values_[i].index == object.index)
			return true;
	}
	return false;
}

RValue FunctionLowering::construction_table()
{
	// VTT[0]: the table of the construction under way, loaded once per
	// block.
	const lowir::BlockId block = builder_.insertion_block();
	if (vtt_table_block_ != block || terminated_)
	{
		Place entry = Place();
		entry.storage = vtt_param_;
		vtt_table_ = load(lowir::Type::scalar(lowir::TypeKind::Ptr), entry);
		vtt_table_block_ = builder_.insertion_block();
		vtt_rows_.clear();
	}
	return vtt_table_;
}

RValue FunctionLowering::construction_row(const sema::ClassEntity* cls, const sema::ClassEntity* base)
{
	const RValue table = construction_table();
	for (const std::pair<const sema::ClassEntity*, RValue>& known : vtt_rows_)
	{
		if (known.first == base)
			return known.second;
	}
	Place row = Place();
	row.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, table.value,
		integer(
			lowir::Type::scalar(lowir::TypeKind::I64), static_cast<uint64_t>(lowering_.vbase_row_offset(cls, base))))
					  .value;
	const RValue offset = load(lowir::Type::scalar(lowir::TypeKind::I64), row);
	vtt_rows_.push_back(std::make_pair(base, offset));
	return offset;
}

void FunctionLowering::hidden_base_arguments(size_t base, uint32_t location)
{
	// A base-object entry of a class with virtual bases called for a base
	// subobject: the caller's VTT part for it after the object.
	if (!hidden_base_.active)
		throw LoweringError(location, "a base-object constructor or destructor called outside a construction");
	hidden_base_.active = false;
	if (!hidden_base_.vtt_pushed)
		arguments_.insert(arguments_.begin() + static_cast<std::ptrdiff_t>(base + 1), hidden_vtt_value());
	hidden_base_.vtt_pushed = false;
}

lowir::Value FunctionLowering::hidden_vtt_value()
{
	// The caller's VTT part for the base subobject: its own VTT's in a
	// base-object entry, else the complete class's.
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	if (base_entry_)
		return hidden_base_.vtt_index == 0
			? vtt_param_
			: index(i8, ir_model::IPK_NONE, vtt_param_, integer(i64, 8 * static_cast<uint64_t>(hidden_base_.vtt_index)))
				  .value;
	const sema::ClassEntity* current = sema::ClassOf(function_->owner->entity->type);
	return index(i8, ir_model::IPK_NONE, address_of_symbol(lowering_.vtt_symbol(current)),
		integer(i64, 8 * static_cast<uint64_t>(hidden_base_.vtt_index)))
		.value;
}

void FunctionLowering::hidden_base_vtt(uint32_t location)
{
	if (!hidden_base_.active)
		throw LoweringError(location, "a base-object constructor or destructor called outside a construction");
	arguments_.push_back(hidden_vtt_value());
	hidden_base_.vtt_pushed = true;
}

lowir::Value FunctionLowering::virtual_base_object(
	lowir::Value object, const sema::ClassEntity* cls, const sema::ClassEntity* base, uint32_t location)
{
	// The virtual base subobject itself (a class may also be a non-virtual
	// base, which a conversion by class would find first), through the
	// object's table (none: `this`).
	if (object.kind == lowir::ValueKind::None)
		object = this_pointer(location);
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, virtual_base_address(object, cls, base),
		integer(lowir::Type::scalar(lowir::TypeKind::I64), 0))
		.value;
}

RValue FunctionLowering::virtual_base_pointer(
	const RValue& value, const sema::ClassEntity* from, const sema::ClassEntity* to, bool nonnull, uint32_t location)
{
	// Through a virtual base, the object's table is read: a null pointer
	// is tested first (it converts to null, 4.10p3).
	RValue adjusted;
	adjusted.type = value.type;
	if (nonnull)
	{
		adjusted.value = to_base(value.value, from, to, location);
		return adjusted;
	}
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	Place result = Place();
	result.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Basecast, pointer));
	const lowir::BlockId null_block = new_block(lowir::BlockKind::BasecastNull);
	const lowir::BlockId adjust = new_block(lowir::BlockKind::BasecastAdjust);
	const lowir::BlockId end = new_block(lowir::BlockKind::BasecastEnd);
	branch(compare(lowir::ComparePredicate::Eq, pointer, value, integer(pointer, 0)), null_block, adjust);
	place(null_block);
	store(pointer, integer(pointer, 0), result);
	jump(end);
	place(adjust);
	adjusted.value = to_base(value.value, from, to, location);
	store(pointer, adjusted, result);
	jump(end);
	place(end);
	return load(pointer, result);
}

} // namespace lower
