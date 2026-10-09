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

std::size_t AlignVirtualBase(std::size_t value, std::size_t alignment)
{
	if (alignment <= 1) return value;
	const std::size_t remainder = value % alignment;
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
			MergeSysvQuarters(&quarters, offset,
				offset + program.SizeOf(member.type), 1);
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

void Analyzer::FinalizeClassVirtualBaseLayout(EntityId entity,
	std::size_t packing_alignment, std::size_t* size,
	std::size_t* alignment, std::size_t* natural_alignment, bool* empty_class)
{
	EntityRecord& owner = program_->entities[entity];
	std::vector<VirtualBaseLayout> layouts;
	CollectVirtualBaseLayouts(entity, &layouts);
	const std::size_t own_nonvirtual_alignment = *alignment;
	for (std::size_t i = 0; i < layouts.size(); ++i)
	{
		const EntityRecord& base = program_->entities[layouts[i].entity];
		const std::size_t base_alignment =
			static_cast<std::size_t>(base.object_alignment);
		const std::size_t effective = packing_alignment == 0 ? base_alignment :
			std::min(base_alignment, packing_alignment);
		*natural_alignment = std::max(*natural_alignment, base_alignment);
		*alignment = std::max(*alignment, effective);
	}
	if (owner.requested_alignment != 0 &&
		owner.requested_alignment < *natural_alignment)
		ThrowSemanticError(
			"requested alignment is weaker than the natural class alignment");
	*alignment = std::max(*alignment,
		static_cast<std::size_t>(owner.requested_alignment));
	if (*size == 0) *size = 1;
	owner.nonvirtual_size = *size;
	owner.nonvirtual_alignment = std::max(own_nonvirtual_alignment,
		static_cast<std::size_t>(owner.requested_alignment));
	for (std::size_t i = 0; i < layouts.size(); ++i)
	{
		const EntityRecord& base = program_->entities[layouts[i].entity];
		const std::size_t base_alignment = static_cast<std::size_t>(
			base.nonvirtual_alignment == 0 ? base.object_alignment :
			base.nonvirtual_alignment);
		const std::size_t effective = packing_alignment == 0 ? base_alignment :
			std::min(base_alignment, packing_alignment);
		*size = AlignVirtualBase(*size, effective);
		layouts[i].offset = *size;
		const std::size_t base_size = static_cast<std::size_t>(
			base.nonvirtual_size == 0 ? base.object_size : base.nonvirtual_size);
		if (*size > std::numeric_limits<std::size_t>::max() - base_size)
			ThrowSemanticResourceLimit("class layout is too large");
		*size += base_size;
	}
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
	*size = AlignVirtualBase(*size, *alignment);
	owner.object_size = *size;
	owner.object_alignment = *alignment;
	owner.natural_alignment = *natural_alignment;
	owner.empty_class = *empty_class;
	owner.layout_complete = true;
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
	for (std::size_t virtual_ordinal = 0;
		virtual_ordinal < owner.virtual_base_count; ++virtual_ordinal)
	{
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
