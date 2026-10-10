#include "lower/program_lowering.h"

#include <algorithm>

#include "sema/analyzer.h"

// The Itanium C++ ABI's table facts: Itanium 2.2's nearly empty classes,
// 2.4's primary bases (a nearly empty virtual one included) and the
// rows of a class's tables (2.5.2, as Clang's VCallAndVBaseOffsetBuilder
// lays out the components): its vbase offsets, read by conversions
// through virtual bases and spelled by covariant thunks' names, and a
// virtual base's vcall offsets, read by virtual thunks. Each fact is
// computed once per class and unit; the walks over the inheritance graph
// visit each class once (a class met again adds no virtual base its first
// visit did not).
namespace lower {

namespace {

const sema::ClassEntity* BaseClassOf(const sema::BaseClass* base)
{
	return static_cast<const sema::ClassEntity*>(base->base);
}

} // namespace

const sema::ClassEntity* ProgramLowering::itanium_primary_base(const sema::ClassEntity* cls, bool& is_virtual)
{
	// The analysis's (sema/primary_bases.cpp): layouts are built from the
	// same choice.
	return analyzer_->itanium_primary(cls, is_virtual);
}

const ProgramLowering::ItaniumRows& ProgramLowering::itanium_rows(const sema::ClassEntity* cls)
{
	// A class's rows (its table's as a non-virtual base) once, built from
	// its primary base's (its full rows when the primary base is virtual):
	// those, then its virtual bases not among them in inheritance graph
	// order (Clang's AddVBaseOffsets meets them in that order). A class
	// without virtual bases has none. The chain is walked down to a class
	// built, then built upward (an explicit list).
	const uint32_t found = itanium_row_lists_.find(cls);
	if (found != PointerMap::kMissing)
		return itanium_row_heads_[found];
	std::vector<std::pair<const sema::ClassEntity*, bool>>& chain = itanium_chain_;
	chain.clear();
	bool needs_full = false;
	for (const sema::ClassEntity* link = cls; link != nullptr;)
	{
		const uint32_t known = itanium_row_lists_.find(link);
		if (known != PointerMap::kMissing && (!needs_full || itanium_row_heads_[known].full != PointerMap::kMissing))
			break;
		chain.push_back(std::make_pair(link, needs_full));
		if (known != PointerMap::kMissing || sema::VirtualBaseCount(link) == 0)
			break;
		link = itanium_primary_base(link, needs_full);
	}
	for (size_t i = chain.size(); i-- > 0;)
	{
		const std::pair<const sema::ClassEntity*, bool> step = itanium_chain_[i];
		itanium_build_rows(step.first);
		if (step.second)
			itanium_build_full(step.first);
	}
	return itanium_row_heads_[itanium_row_lists_.find(cls)];
}

void ProgramLowering::itanium_build_rows(const sema::ClassEntity* cls)
{
	if (itanium_row_lists_.find(cls) != PointerMap::kMissing)
		return;
	const uint32_t first = static_cast<uint32_t>(itanium_row_store_.size());
	if (sema::VirtualBaseCount(cls) != 0)
	{
		bool primary_virtual = false;
		const sema::ClassEntity* primary = itanium_primary_base(cls, primary_virtual);
		if (primary != nullptr && (primary_virtual || sema::VirtualBaseCount(primary) != 0))
		{
			const ItaniumRows base = itanium_row_heads_[itanium_row_lists_.find(primary)];
			const uint32_t from = primary_virtual ? base.full : base.first;
			const uint32_t length = primary_virtual ? base.count : base.prefix;
			for (uint32_t j = 0; j < length; ++j)
			{
				const ItaniumRow row = itanium_row_store_[from + j];
				if (row.vbase != nullptr)
					itanium_vbase_positions_.insert(ClassPair{ cls, row.vbase }, j);
				itanium_row_store_.push_back(row);
			}
		}
		for (uint32_t i = 0; i < sema::VirtualBaseCount(cls); ++i)
		{
			const sema::ClassEntity* shared = BaseClassOf(sema::VirtualBases(cls)[i].base);
			if (itanium_vbase_positions_.find(ClassPair{ cls, shared }) != nullptr)
				continue;
			itanium_vbase_positions_.insert(
				ClassPair{ cls, shared }, static_cast<uint32_t>(itanium_row_store_.size()) - first);
			itanium_row_store_.push_back(ItaniumRow{ shared, nullptr, nullptr, nullptr, 0 });
		}
	}
	itanium_row_lists_.set(cls, static_cast<uint32_t>(itanium_row_heads_.size()));
	itanium_row_heads_.push_back(ItaniumRows{ first, static_cast<uint32_t>(itanium_row_store_.size()) - first,
		static_cast<uint32_t>(PointerMap::kMissing), 0 });
}

void ProgramLowering::itanium_build_full(const sema::ClassEntity* cls)
{
	// The class's rows as a virtual base's: its rows, then its own vcall
	// offsets (a signature met in them keeps its row).
	const uint32_t index = itanium_row_lists_.find(cls);
	const ItaniumRows rows = itanium_row_heads_[index];
	if (rows.full != PointerMap::kMissing)
		return;
	const uint32_t full = static_cast<uint32_t>(itanium_row_store_.size());
	itanium_signatures_.clear(64);
	for (uint32_t j = 0; j < rows.prefix; ++j)
	{
		const ItaniumRow row = itanium_row_store_[rows.first + j];
		if (row.function != nullptr)
		{
			const SignatureKey key = row.function->destructor
				? SignatureKey{ nullptr, nullptr }
				: SignatureKey{ row.function->name, row.function->signature };
			itanium_signatures_.insert(key, 1);
			itanium_vcall_positions_.insert(VcallKey{ cls, key.name, key.signature }, j);
		}
		itanium_row_store_.push_back(row);
	}
	itanium_row_base_ = full;
	itanium_vcall_walk(cls, cls, cls, 0);
	itanium_row_heads_[index].full = full;
	itanium_row_heads_[index].count = static_cast<uint32_t>(itanium_row_store_.size()) - full;
}

const ProgramLowering::ItaniumRows& ProgramLowering::itanium_full_rows(const sema::ClassEntity* cls)
{
	itanium_rows(cls);
	itanium_build_full(cls);
	return itanium_row_heads_[itanium_row_lists_.find(cls)];
}

size_t ProgramLowering::SignatureHash::operator()(const SignatureKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.name) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.signature);
}

void ProgramLowering::itanium_vcall_walk(
	const sema::ClassEntity* owner, const sema::ClassEntity* level, const sema::ClassEntity* holder, uint64_t offset)
{
	// Clang's AddVCallOffsets for `holder` at `offset` in the level: its
	// non-virtual primary base first, its own virtual functions in table
	// order (its new slots follow declaration order), then its other
	// non-virtual dynamic bases; a signature met before in the list keeps
	// its row.
	bool primary_virtual = false;
	const sema::ClassEntity* primary = itanium_primary_base(holder, primary_virtual);
	if (primary != nullptr && !primary_virtual)
		itanium_vcall_walk(owner, level, primary, offset);
	const sema::VirtualSlot* slots = holder->virtuals != nullptr ? analyzer_->virtual_slots(holder) : nullptr;
	const uint32_t size = slots != nullptr ? holder->virtuals->vtable_size : 0;
	for (uint32_t i = 0; i < size; ++i)
	{
		const sema::FunctionEntity* function = slots[i].function;
		if (function->owner != holder->scope || slots[i].kind == sema::SlotKind::DeletingDestructor)
			continue;
		const SignatureKey key = function->destructor ? SignatureKey{ nullptr, nullptr }
													  : SignatureKey{ function->name, function->signature };
		if (itanium_signatures_.find(key) != nullptr)
			continue;
		itanium_signatures_.insert(key, 1);
		itanium_vcall_positions_.insert(VcallKey{ owner, key.name, key.signature },
			static_cast<uint32_t>(itanium_row_store_.size()) - itanium_row_base_);
		itanium_row_store_.push_back(ItaniumRow{ nullptr, function, holder, level, offset });
	}
	for (const sema::BaseClass* edge = holder->bases; edge != nullptr; edge = edge->next)
	{
		if (!edge->is_virtual && BaseClassOf(edge) != primary && BaseClassOf(edge)->dynamic)
			itanium_vcall_walk(owner, level, BaseClassOf(edge), offset + edge->offset);
	}
}

int64_t ProgramLowering::itanium_vbase_offset_offset(const sema::ClassEntity* cls, const sema::ClassEntity* base)
{
	// The position of `base`'s vbase offset in `cls`'s tables, from the
	// address point.
	itanium_rows(cls);
	const uint32_t* index = itanium_vbase_positions_.find(ClassPair{ cls, base });
	if (index == nullptr)
		throw LoweringError(0, "a virtual base the class does not have");
	return -8 * (3 + static_cast<int64_t>(*index));
}

int64_t ProgramLowering::itanium_vcall_offset(const sema::ClassEntity* vbase, const sema::FunctionEntity* function)
{
	// Its position in the base's table, from the address point.
	itanium_full_rows(vbase);
	const uint32_t* index = itanium_vcall_positions_.find(function->destructor
			? VcallKey{ vbase, nullptr, nullptr }
			: VcallKey{ vbase, function->name, function->signature });
	if (index == nullptr)
		throw LoweringError(0, "a virtual base without a vcall offset for a function its table overrides");
	return -8 * (3 + static_cast<int64_t>(*index));
}

int64_t ProgramLowering::context_at(const TableContext& context, uint32_t segment)
{
	return static_cast<int64_t>(context.construction
			? construction_segment_offset(context.complete, context.cls, context.offset, segment)
			: segment_offsets(context.cls)[segment]);
}

uint32_t ProgramLowering::itanium_table_rows(const sema::VtableGroup* group, uint32_t segment)
{
	// A virtual base's table has its own vcall offsets too.
	const sema::VtableSegment& own = group->segments[segment];
	return segment != 0 && own.within == segment ? itanium_full_rows(own.cls).count : itanium_rows(own.cls).prefix;
}

const int64_t* ProgramLowering::itanium_row_values(const TableContext& context, uint32_t segment)
{
	// A vbase offset: its base's offset from the table's subobject. A vcall
	// offset: the offset of its function's final overrider's subobject (the
	// group's entry for the function's slot in the segment of the subobject
	// declaring it, found below the level where the owner's layout put it)
	// from the table's subobject (Clang's: all relative to the table's).
	const sema::VtableGroup* group = context.cls->virtuals->group;
	const sema::ClassEntity* holder = group->segments[segment].cls;
	const uint32_t count = itanium_table_rows(group, segment);
	const ItaniumRows rows = itanium_rows(holder);
	const uint32_t from = segment != 0 && group->segments[segment].within == segment ? rows.full : rows.first;
	const uint64_t* own = segment_offsets(context.cls);
	const int64_t table_at = context_at(context, segment);
	itanium_row_values_.clear();
	for (uint32_t j = 0; j < count; ++j)
	{
		const ItaniumRow row = itanium_row_store_[from + j];
		if (row.vbase != nullptr)
		{
			itanium_row_values_.push_back(
				static_cast<int64_t>(virtual_base_offset(context.complete, row.vbase)) - table_at);
			continue;
		}
		const uint32_t level = row.level == holder
			? segment
			: group_segment(context.cls, virtual_base_offset(context.cls, row.level), row.level);
		const uint32_t k = group_segment(context.cls, own[level] + row.offset, row.holder);
		const sema::VtableEntry& slot = group->entries[group->segments[k].first_entry + row.function->vtable_slot];
		itanium_row_values_.push_back(context_at(context, slot.overrider) - table_at);
	}
	return itanium_row_values_.data();
}

uint32_t ProgramLowering::group_segment(const sema::ClassEntity* owner, uint64_t offset, const sema::ClassEntity* cls)
{
	// The owner's segments entered at its first query.
	if (group_segments_.find(SegmentKey{ owner, 0, nullptr }) == nullptr)
	{
		group_segments_.insert(SegmentKey{ owner, 0, nullptr }, 0);
		const sema::VtableGroup* group = owner->virtuals->group;
		const uint64_t* offsets = segment_offsets(owner);
		for (uint32_t i = 1; i < group->segment_count; ++i)
		{
			const SegmentKey key = SegmentKey{ owner, offsets[i], group->segments[i].cls };
			if (group_segments_.find(key) == nullptr)
				group_segments_.insert(key, i);
		}
	}
	const uint32_t* found = group_segments_.find(SegmentKey{ owner, offset, cls });
	if (found == nullptr)
		throw LoweringError(0, "a virtual base's subobject without a table segment");
	return *found;
}

size_t ProgramLowering::VcallHash::operator()(const VcallKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.cls) * 0x9E3779B97F4A7C15ull) ^ reinterpret_cast<uintptr_t>(key.name) * 31 ^
		reinterpret_cast<uintptr_t>(key.signature);
}

size_t ProgramLowering::SegmentHash::operator()(const SegmentKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.owner) * 0x9E3779B97F4A7C15ull) ^
		reinterpret_cast<uintptr_t>(key.cls) * 31 ^ key.offset;
}

} // namespace lower
