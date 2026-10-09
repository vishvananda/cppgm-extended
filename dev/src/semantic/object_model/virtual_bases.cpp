#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <algorithm>
#include <limits>

namespace cppgm
{
namespace semantic
{

namespace
{

std::uint64_t AlignVirtualBase(std::uint64_t value, std::uint64_t alignment)
{
	if (alignment <= 1) return value;
	const std::uint64_t remainder = value % alignment;
	return remainder == 0 ? value : value + alignment - remainder;
}

}

void Analyzer::CollectVirtualBaseLayouts(EntityId entity,
	std::vector<VirtualBaseLayout>* layouts)
{
	layouts->clear();
	if (virtual_base_layout_entity_marks_.size() < program_->entities.size())
	{
		virtual_base_layout_entity_marks_.resize(program_->entities.size(), 0);
		virtual_base_layout_fact_marks_.resize(program_->entities.size(), 0);
	}
	if (virtual_base_layout_generation_ ==
		std::numeric_limits<std::uint32_t>::max())
	{
		std::fill(virtual_base_layout_entity_marks_.begin(),
			virtual_base_layout_entity_marks_.end(), 0);
		std::fill(virtual_base_layout_fact_marks_.begin(),
			virtual_base_layout_fact_marks_.end(), 0);
		virtual_base_layout_generation_ = 0;
	}
	const std::uint32_t generation = ++virtual_base_layout_generation_;
	virtual_base_layout_scratch_.clear();
	virtual_base_layout_entity_marks_[entity] = generation;
	virtual_base_layout_scratch_.push_back(
		std::make_pair(entity, static_cast<std::uint32_t>(0)));
	while (!virtual_base_layout_scratch_.empty())
	{
		std::pair<EntityId, std::uint32_t>& frame =
			virtual_base_layout_scratch_.back();
		const EntityRecord& current = program_->entities[frame.first];
		if (frame.second == current.direct_base_count)
		{
			virtual_base_layout_scratch_.pop_back();
			continue;
		}
		const DirectBaseEdge& edge = program_->DirectBase(
			frame.first, frame.second++);
		++virtual_base_layout_edge_visits_;
		if (edge.virtual_base &&
			virtual_base_layout_fact_marks_[edge.entity] != generation)
		{
			virtual_base_layout_fact_marks_[edge.entity] = generation;
			layouts->push_back(VirtualBaseLayout(edge.entity));
			++virtual_base_layout_facts_;
		}
		if (virtual_base_layout_entity_marks_[edge.entity] == generation)
			continue;
		virtual_base_layout_entity_marks_[edge.entity] = generation;
		virtual_base_layout_scratch_.push_back(
			std::make_pair(edge.entity, static_cast<std::uint32_t>(0)));
	}
}

namespace
{

// Merges a class (1 INTEGER, 2 SSE) into the four-byte quarters that bytes
// [first, last) of a 16-byte object touch; INTEGER wins (psABI 3.2.3).
void MergeSysvQuarters(std::uint8_t* quarters, std::uint64_t first,
	std::uint64_t last, std::uint8_t sysv_class)
{
	for (std::uint64_t quarter = first / 4; quarter < 4 && quarter * 4 < last;
		++quarter)
	{
		const unsigned shift = static_cast<unsigned>(quarter * 2);
		const std::uint8_t current = (*quarters >> shift) & 3;
		const std::uint8_t merged = current == 1 ? 1 :
			current == 0 ? sysv_class : (sysv_class == 1 ? 1 : current);
		*quarters = static_cast<std::uint8_t>(
			(*quarters & ~(3u << shift)) | (merged << shift));
	}
}

// The SysV classes of one member at an offset within a class (psABI 3.2.3).
void ClassifySysvMember(const Program& program, TypeId type,
	std::uint64_t offset, std::uint8_t* quarters, bool* memory, bool* unmodeled)
{
	type = program.types.RemoveTopCv(type);
	const TypeRecord& record = program.types.Get(type);
	const std::uint64_t size = program.SizeOf(type);
	if (size == 0) return;
	if (offset + size > 16 || offset % program.AlignOf(type) != 0)
	{
		*memory = true;
		return;
	}
	std::uint8_t sysv_class = 1;
	if (record.kind == TYPE_FUNDAMENTAL)
	{
		switch (record.fundamental)
		{
		case FUND_FLOAT: case FUND_DOUBLE: case FUND_FLOAT16:
		case FUND_FLOAT32: case FUND_FLOAT32X: case FUND_FLOAT64:
			sysv_class = 2;
			break;
		case FUND_LONG_DOUBLE: case FUND_FLOAT64X: case FUND_STDFLOAT128:
		case FUND_FLOAT128:
			*unmodeled = true;
			return;
		default:
			break;
		}
	}
	else if (record.kind == TYPE_COMPLEX || record.kind == TYPE_ARRAY)
	{
		const std::uint64_t element = program.SizeOf(record.child);
		for (std::uint64_t at = 0; element != 0 && at < size; at += element)
			ClassifySysvMember(program, record.child, offset + at, quarters,
				memory, unmodeled);
		return;
	}
	else if (record.kind == TYPE_VECTOR)
	{
		if (size > 8) *unmodeled = true;
		sysv_class = 2;
	}
	else if (record.kind == TYPE_NAMED && record.entity < program.entities.size() &&
		IsClassNamedFlavor(program.entities[record.entity].flavor))
	{
		const EntityRecord& member = program.entities[record.entity];
		if (!member.layout_complete || member.sysv_memory)
		{
			*memory = true;
			return;
		}
		if (member.sysv_unmodeled) *unmodeled = true;
		for (unsigned quarter = 0; quarter < 4; ++quarter)
		{
			const std::uint8_t inner = (member.sysv_quarters >> (quarter * 2)) & 3;
			if (inner != 0)
				MergeSysvQuarters(quarters, offset + quarter * 4,
					offset + quarter * 4 + 4, inner);
		}
		return;
	}
	MergeSysvQuarters(quarters, offset, offset + size, sysv_class);
}

// The SysV classes of a completed class layout (EntityRecord::sysv_quarters).
void ClassifySysvClassLayout(Program& program, EntityId entity,
	const std::vector<ClassLayoutMember>* members)
{
	EntityRecord& owner = program.entities[entity];
	std::uint8_t quarters = 0;
	bool memory = owner.object_size > 16 || owner.dynamic_class ||
		owner.virtual_base_count != 0;
	bool unmodeled = false;
	for (std::size_t i = 0; !memory && i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& base = program.DirectBase(entity, i);
		ClassifySysvMember(program, program.entities[base.entity].type,
			base.offset, &quarters, &memory, &unmodeled);
	}
	for (std::size_t i = 0; !memory && members && i < members->size(); ++i)
	{
		const ClassLayoutMember& member = (*members)[i];
		if (member.binding == kNoBinding ||
			!program.bindings[member.binding].non_static_data_member)
			continue;
		const std::uint64_t offset = program.BindingLayout(
			program.bindings[member.binding]).member_offset;
		// A bit-field's storage unit holds integers.
		if (member.bit_field)
			MergeSysvQuarters(&quarters, offset, offset + program.BindingLayout(
				program.bindings[member.binding]).bit_storage_bits / 8, 1);
		else
			ClassifySysvMember(program, member.type, offset, &quarters,
				&memory, &unmodeled);
	}
	EntityRecord& classified = program.entities[entity];
	classified.sysv_memory = memory;
	classified.sysv_unmodeled = !memory && unmodeled;
	classified.sysv_quarters = memory || unmodeled ? 0 : quarters;
}

// The null data member pointers of a completed class's non-virtual part
// (EntityRecord::null_member_pointer_*): its non-virtual bases and data
// members, only the first member of a union (zero-initialization, 8.5p5).
void RecordNullMemberPointerRuns(Program& program, EntityId entity,
	const std::vector<ClassLayoutMember>* members)
{
	std::vector<NullMemberPointerRun> runs;
	const EntityRecord& owner = program.entities[entity];
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& base = program.DirectBase(entity, i);
		if (base.virtual_base) continue;
		const EntityRecord& part = program.entities[base.entity];
		for (std::uint32_t r = 0; r < part.null_member_pointer_count; ++r)
		{
			NullMemberPointerRun run = program.null_member_pointer_runs[
				part.null_member_pointer_begin + r];
			run.offset += base.offset;
			runs.push_back(run);
		}
	}
	for (std::size_t i = 0; members && i < members->size(); ++i)
	{
		const ClassLayoutMember& member = (*members)[i];
		if (member.binding == kNoBinding || member.bit_field ||
			!program.bindings[member.binding].non_static_data_member)
			continue;
		program.AppendNullMemberPointerRuns(member.type, program.BindingLayout(
			program.bindings[member.binding]).member_offset, &runs);
		if (owner.flavor == NAMED_UNION) break;
	}
	if (program.null_member_pointer_runs.size() + runs.size() >
		std::numeric_limits<std::uint32_t>::max())
		ThrowSemanticResourceLimit("too many null member pointer facts");
	EntityRecord& recorded = program.entities[entity];
	recorded.null_member_pointer_begin =
		static_cast<std::uint32_t>(program.null_member_pointer_runs.size());
	recorded.null_member_pointer_count = static_cast<std::uint32_t>(runs.size());
	program.null_member_pointer_runs.insert(
		program.null_member_pointer_runs.end(), runs.begin(), runs.end());
}

}

namespace
{

// A base subobject placed at `offset` claims the virtual primary base of its
// class, and of its non-virtual bases, when no subobject placed before did
// (Itanium 2.4: an indirect primary base lies with the base it is primary
// for).  `self` marks the class's own virtual primary base.
void ClaimPrimaryVirtualBases(const Program& program,
	const std::vector<ClassPolymorphismFacts>& facts, EntityId base,
	std::uint64_t offset, bool self, const std::vector<EntityId>& indirect,
	std::vector<std::pair<EntityId, std::uint64_t> >* claimed)
{
	if (self) claimed->push_back(std::make_pair(base, offset));
	if (base < facts.size() && facts[base].primary_virtual)
	{
		const EntityId primary = facts[base].primary_ancestors[0];
		bool taken = false;
		for (std::size_t i = 0; i < claimed->size(); ++i)
			taken = taken || (*claimed)[i].first == primary;
		if (!taken && std::find(indirect.begin(), indirect.end(), primary) !=
			indirect.end())
			ClaimPrimaryVirtualBases(program, facts, primary, offset, true,
				indirect, claimed);
	}
	const EntityRecord& record = program.entities[base];
	for (std::size_t i = 0; i < record.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program.DirectBase(base, i);
		if (!edge.virtual_base)
			ClaimPrimaryVirtualBases(program, facts, edge.entity,
				offset + edge.offset, false, indirect, claimed);
	}
}

// Each direct base's virtual bases in its construction order, then the base
// itself when it is virtual.
void RankVirtualBaseConstruction(const Program& program, EntityId entity,
	std::vector<VirtualBaseLayout>* layouts)
{
	std::vector<EntityId> order;
	order.reserve(layouts->size());
	const EntityRecord& owner = program.entities[entity];
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program.DirectBase(entity, i);
		const EntityRecord& base = program.entities[edge.entity];
		std::vector<EntityId> inherited(base.virtual_base_count, kNoEntity);
		for (std::size_t k = 0; k < base.virtual_base_count; ++k)
		{
			const VirtualBaseLayout& layout = program.VirtualBase(edge.entity, k);
			inherited[layout.construction_rank] = layout.entity;
		}
		if (edge.virtual_base) inherited.push_back(edge.entity);
		for (std::size_t k = 0; k < inherited.size(); ++k)
			if (std::find(order.begin(), order.end(), inherited[k]) == order.end())
				order.push_back(inherited[k]);
	}
	for (std::size_t i = 0; i < layouts->size(); ++i)
		(*layouts)[i].construction_rank = static_cast<std::uint32_t>(
			std::find(order.begin(), order.end(), (*layouts)[i].entity) -
			order.begin());
}

}

std::vector<std::uint32_t> VirtualBaseConstructionOrder(
	const Program& program, EntityId entity)
{
	const EntityRecord& owner = program.entities[entity];
	std::vector<std::uint32_t> order(owner.virtual_base_count, 0);
	for (std::size_t i = 0; i < owner.virtual_base_count; ++i)
		order[program.VirtualBase(entity, i).construction_rank] =
			static_cast<std::uint32_t>(i);
	return order;
}

void Analyzer::FinalizeClassVirtualBaseLayout(EntityId entity,
	std::size_t packing_alignment, ClassLayoutCursor* cursor, bool* empty_class)
{
	EntityRecord& owner = program_->entities[entity];
	std::vector<VirtualBaseLayout> layouts;
	CollectVirtualBaseLayouts(entity, &layouts);
	const std::uint64_t own_nonvirtual_alignment = cursor->alignment;
	const std::uint64_t member_data_size = cursor->data_size;
	for (std::size_t i = 0; i < layouts.size(); ++i)
		cursor->natural_alignment = std::max(cursor->natural_alignment,
			program_->entities[layouts[i].entity].object_alignment);
	if (owner.requested_alignment != 0 &&
		owner.requested_alignment < cursor->natural_alignment)
		ThrowSemanticError(
			"requested alignment is weaker than the natural class alignment");
	if (cursor->size == 0) cursor->size = 1;
	owner.nonvirtual_size = cursor->size;
	owner.nonvirtual_alignment = std::max(own_nonvirtual_alignment,
		owner.requested_alignment);
	// Itanium C++ ABI 2.4 III: the virtual bases in inheritance graph order,
	// placed as non-virtual bases are, from the data size -- except the
	// class's own virtual primary base (at offset zero) and the indirect
	// primary bases, which lie with the subobject claiming them.
	std::vector<EntityId> visited, indirect;
	CollectIndirectPrimaryBases(*program_, class_polymorphism_, entity,
		&visited, &indirect);
	std::vector<std::pair<EntityId, std::uint64_t> > claimed;
	if (entity < class_polymorphism_.size() &&
		class_polymorphism_[entity].primary_virtual)
		ClaimPrimaryVirtualBases(*program_, class_polymorphism_,
			class_polymorphism_[entity].primary_ancestors[0], 0, true,
			indirect, &claimed);
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, i);
		if (!edge.virtual_base)
			ClaimPrimaryVirtualBases(*program_, class_polymorphism_, edge.entity,
				edge.offset, false, indirect, &claimed);
	}
	for (std::size_t pass = 0; pass != 2; ++pass)
	for (std::size_t i = 0; i < layouts.size(); ++i)
	{
		const EntityId candidate = layouts[i].entity;
		std::size_t claim = 0;
		while (claim < claimed.size() && claimed[claim].first != candidate) ++claim;
		const bool is_indirect = std::find(indirect.begin(), indirect.end(),
			candidate) != indirect.end();
		if (claim != claimed.size())
		{
			layouts[i].offset = claimed[claim].second;
			continue;
		}
		// An indirect primary base no placed subobject claimed is laid out
		// after the others.
		if (is_indirect != (pass != 0)) continue;
		const EntityRecord& base = program_->entities[candidate];
		const std::uint64_t base_alignment = LayoutNonvirtualAlignment(base);
		const std::uint64_t effective = packing_alignment == 0 ? base_alignment :
			std::min<std::uint64_t>(base_alignment, packing_alignment);
		std::uint64_t offset = 0;
		const bool empty = base.empty_class;
		if (!empty || empty_subobjects_.Conflict(layouts[i].entity, 0, false))
		{
			offset = AlignVirtualBase(cursor->data_size, effective);
			while (empty_subobjects_.Conflict(layouts[i].entity, offset, false))
				offset += effective;
		}
		const std::uint64_t extent = empty ? base.object_size :
			LayoutNonvirtualSize(base);
		if (offset > std::numeric_limits<std::uint64_t>::max() - extent)
			ThrowSemanticResourceLimit("class layout is too large");
		empty_subobjects_.Record(layouts[i].entity, offset, false, empty);
		cursor->Place(offset + extent, !empty);
		cursor->alignment = std::max(cursor->alignment, effective);
		layouts[i].offset = offset;
		claimed.push_back(std::make_pair(candidate, offset));
		ClaimPrimaryVirtualBases(*program_, class_polymorphism_, candidate,
			offset, false, indirect, &claimed);
	}
	cursor->alignment = std::max(cursor->alignment, owner.requested_alignment);
	RankVirtualBaseConstruction(*program_, entity, &layouts);
	program_->SetVirtualBaseLayouts(entity, layouts);
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		DirectBaseEdge& edge = program_->MutableDirectBase(entity, i);
		if (!edge.virtual_base) continue;
		std::uint64_t offset = 0;
		if (!program_->FindVirtualBase(entity, edge.entity, &offset))
			ThrowInternalCompilerError("direct virtual base has no layout fact");
		edge.offset = offset;
	}
	if (!layouts.empty())
	{
		*empty_class = false;
		owner.trivial_default_constructor = false;
	}
	owner.object_size = AlignVirtualBase(cursor->size, cursor->alignment);
	owner.object_alignment = cursor->alignment;
	owner.natural_alignment = cursor->natural_alignment;
	owner.empty_class = *empty_class;
	owner.data_size = cursor->data_size;
	if (owner.pod_for_layout)
		owner.nonvirtual_size = owner.data_size = owner.object_size;
	owner.has_empty_subobject = ClassHasEmptySubobject(
		*program_, entity_layout_members_, entity);
	owner.layout_complete = true;
	bit_field_layout_.Assign(program_,
		owner.pod_for_layout ? owner.object_size : member_data_size);
	if (dependent_shape_completion_depth_ == 0)
	{
		const std::vector<ClassLayoutMember>* members =
			entity < entity_layout_members_.size() ? &entity_layout_members_[entity] : 0;
		ClassifySysvClassLayout(*program_, entity, members);
		RecordNullMemberPointerRuns(*program_, entity, members);
	}
}

void Analyzer::AddVirtualBaseInitializationActions(EntityId entity,
	ScopeId function_scope, const std::vector<NodeId>& initializers,
	const std::vector<ScopeId>& initializer_scopes,
	const std::vector<std::uint8_t>& initializer_expanded, std::uint32_t body)
{
	const EntityRecord& owner = program_->entities[entity];
	std::vector<std::uint32_t> direct_ordinals(owner.virtual_base_count,
		owner.direct_base_count);
	for (std::size_t candidate = 0;
		candidate < owner.direct_base_count; ++candidate)
	{
		const DirectBaseEdge& edge = program_->DirectBase(entity, candidate);
		if (!edge.virtual_base) continue;
		std::uint32_t virtual_ordinal = 0;
		if (!program_->FindVirtualBase(
			entity, edge.entity, 0, &virtual_ordinal))
			ThrowInternalCompilerError("direct virtual base has no layout fact");
		direct_ordinals[virtual_ordinal] = static_cast<std::uint32_t>(candidate);
	}
	const std::vector<std::uint32_t> order =
		VirtualBaseConstructionOrder(*program_, entity);
	for (std::size_t rank = 0; rank < order.size(); ++rank)
	{
		const std::size_t virtual_ordinal = order[rank];
		const VirtualBaseLayout& layout =
			program_->VirtualBase(entity, virtual_ordinal);
		const std::size_t direct_ordinal = direct_ordinals[virtual_ordinal];
		if (direct_ordinal != owner.direct_base_count)
			AddBaseInitializationAction(entity, direct_ordinal,
				initializers[direct_ordinal], initializer_scopes[direct_ordinal],
				body, initializer_expanded[direct_ordinal] != 0);
		else
		{
			const std::size_t initializer_ordinal =
				owner.direct_base_count + virtual_ordinal;
			AddBaseInitializationActionAt(entity, layout.entity,
				layout.offset, initializers[initializer_ordinal],
				initializer_scopes[initializer_ordinal], body,
				initializer_expanded[initializer_ordinal] != 0);
		}
	}
}

}
}
