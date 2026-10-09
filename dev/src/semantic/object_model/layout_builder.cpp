#include "semantic/object_model/layout_builder.h"
#include "support/exceptions.h"

#include <algorithm>
#include <limits>

namespace cppgm
{
namespace semantic
{

namespace
{

enum EmptySubobjectWalkMode
{
	EMPTY_WALK_CONFLICT,
	EMPTY_WALK_RECORD,
	EMPTY_WALK_RECORD_REACHABLE
};

std::size_t EmptySubobjectHash(EntityId entity, std::uint64_t offset)
{
	std::uint64_t key = (static_cast<std::uint64_t>(entity) << 32) ^
		(offset * 0x9e3779b97f4a7c15ull);
	key ^= key >> 29;
	key *= 0xbf58476d1ce4e5b9ull;
	key ^= key >> 32;
	return static_cast<std::size_t>(key);
}

std::uint64_t CeilBytes(std::uint64_t bits)
{
	return bits / 8 + (bits % 8 != 0 ? 1 : 0);
}

// The narrowest 1-, 2-, 4- or 8-byte window in [lo, hi) holding bytes
// [first, last): naturally aligned if one is, else starting at `first`,
// else ending at `hi`.
bool NarrowStorageWindow(std::uint64_t first, std::uint64_t last,
	std::uint64_t lo, std::uint64_t hi, std::uint64_t* begin,
	std::uint64_t* bytes)
{
	for (std::uint64_t width = 1; width <= 8; width *= 2)
	{
		if (width < last - first || width > hi - lo) continue;
		const std::uint64_t aligned = first / width * width;
		if (aligned >= lo && aligned + width <= hi && aligned + width >= last)
			*begin = aligned;
		else if (first + width <= hi) *begin = first;
		else if (hi - width >= lo && hi - width <= first) *begin = hi - width;
		else continue;
		*bytes = width;
		return true;
	}
	return false;
}

}

void ClassLayoutCursor::Place(std::uint64_t end, bool data)
{
	size = std::max(size, end);
	if (!data) return;
	data_size = end;
	open_bit_byte = false;
}

EntityId LayoutElementClass(const Program& program, TypeId type,
	std::uint64_t* count)
{
	*count = 1;
	const TypeRecord* record = &program.types.Get(type);
	while (record->kind == TYPE_ARRAY || record->kind == TYPE_QUALIFIED)
	{
		if (record->kind == TYPE_ARRAY)
			*count = record->zero_length_array ? 0 : *count * record->bound;
		type = record->child;
		record = &program.types.Get(type);
	}
	if (record->kind != TYPE_NAMED || record->entity >= program.entities.size())
		return kNoEntity;
	return IsClassNamedFlavor(program.entities[record->entity].flavor) ?
		record->entity : kNoEntity;
}

bool ClassHasEmptySubobject(const Program& program,
	const ClassLayoutMemberTable& table, EntityId entity)
{
	const EntityRecord& owner = program.entities[entity];
	if (owner.empty_class) return true;
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
		if (program.entities[
			program.DirectBase(entity, i).entity].has_empty_subobject)
			return true;
	if (entity >= table.size()) return false;
	const std::vector<ClassLayoutMember>& members = table[entity];
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		std::uint64_t count = 0;
		const EntityId element = members[i].bit_field ? kNoEntity :
			LayoutElementClass(program, members[i].type, &count);
		if (element != kNoEntity && count != 0 &&
			program.entities[element].has_empty_subobject) return true;
	}
	return false;
}

// The indirect primary bases of `entity` (2.4 I.a): the virtual primary
// bases of the classes below it.
void CollectIndirectPrimaryBases(const Program& program,
	const std::vector<ClassPolymorphismFacts>& facts, EntityId entity,
	std::vector<EntityId>* visited, std::vector<EntityId>* indirect)
{
	const EntityRecord& owner = program.entities[entity];
	for (std::size_t i = 0; i < owner.direct_base_count; ++i)
	{
		const EntityId base = program.DirectBase(entity, i).entity;
		if (program.entities[base].virtual_base_count == 0 ||
			std::find(visited->begin(), visited->end(), base) != visited->end())
			continue;
		visited->push_back(base);
		if (base < facts.size() && facts[base].primary_virtual)
			indirect->push_back(facts[base].primary_ancestors[0]);
		CollectIndirectPrimaryBases(program, facts, base, visited, indirect);
	}
}

EmptySubobjectMap::EmptySubobjectMap()
	: program_(0), members_(0), count_(0), visits_(0), max_offset_(0),
	  record_limit_(0)
{
}

void EmptySubobjectMap::Begin(const Program* program,
	const ClassLayoutMemberTable* members, std::uint64_t record_limit)
{
	program_ = program;
	members_ = members;
	if (count_ != 0) std::fill(slots_.begin(), slots_.end(), Item());
	count_ = 0;
	max_offset_ = 0;
	record_limit_ = record_limit;
}

bool EmptySubobjectMap::Find(EntityId entity, std::uint64_t offset,
	bool insert)
{
	if (insert && (count_ + 1) * 2 > slots_.size())
	{
		std::vector<Item> previous(std::max<std::size_t>(64, slots_.size() * 2));
		previous.swap(slots_);
		count_ = 0;
		for (std::size_t i = 0; i < previous.size(); ++i)
			if (previous[i].entity != kNoEntity)
				Find(previous[i].entity, previous[i].offset, true);
	}
	if (slots_.empty()) return false;
	const std::size_t mask = slots_.size() - 1;
	for (std::size_t index = EmptySubobjectHash(entity, offset) & mask;;
		index = (index + 1) & mask)
	{
		Item& slot = slots_[index];
		if (slot.entity == entity && slot.offset == offset) return true;
		if (slot.entity != kNoEntity) continue;
		if (insert)
		{
			slot = Item(entity, offset);
			++count_;
		}
		return false;
	}
}

void EmptySubobjectMap::PushMembers(const Item& item, std::uint64_t bound)
{
	if (item.entity >= members_->size()) return;
	const std::vector<ClassLayoutMember>& members = (*members_)[item.entity];
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		const ClassLayoutMember& member = members[i];
		if (member.bit_field || member.binding == kNoBinding) continue;
		std::uint64_t count = 0;
		const EntityId element = LayoutElementClass(*program_, member.type, &count);
		if (element == kNoEntity || !program_->entities[element].has_empty_subobject)
			continue;
		const std::uint64_t start = item.offset + program_->BindingLayout(
			program_->bindings[member.binding]).member_offset;
		const std::uint64_t size = program_->entities[element].object_size;
		for (std::uint64_t index = 0; index < count; ++index)
		{
			const std::uint64_t at = start + index * size;
			if (at > bound) break;
			work_.push_back(Item(element, at, true));
		}
	}
}

bool EmptySubobjectMap::Walk(EntityId root, std::uint64_t offset,
	bool complete, int mode)
{
	if (mode == EMPTY_WALK_CONFLICT && count_ == 0) return false;
	const std::uint64_t bound = mode == EMPTY_WALK_CONFLICT ? max_offset_ :
		mode == EMPTY_WALK_RECORD ? std::numeric_limits<std::uint64_t>::max() :
		(record_limit_ == 0 ? 0 : record_limit_ - 1);
	if (mode == EMPTY_WALK_RECORD_REACHABLE && record_limit_ == 0) return false;
	const std::size_t first = work_.size();
	work_.push_back(Item(root, offset, complete));
	while (work_.size() > first)
	{
		const Item item = work_.back();
		work_.pop_back();
		++visits_;
		const EntityRecord& record = program_->entities[item.entity];
		if (!record.has_empty_subobject || item.offset > bound) continue;
		if (record.empty_class)
		{
			if (mode != EMPTY_WALK_CONFLICT)
			{
				(void)Find(item.entity, item.offset, true);
				max_offset_ = std::max(max_offset_, item.offset);
			}
			else if (Find(item.entity, item.offset, false))
			{
				work_.resize(first);
				return true;
			}
		}
		for (std::size_t i = 0; i < record.direct_base_count; ++i)
		{
			const DirectBaseEdge& edge = program_->DirectBase(item.entity, i);
			if (!edge.virtual_base)
				work_.push_back(Item(edge.entity, item.offset + edge.offset, false));
		}
		for (std::size_t i = 0; item.complete && i < record.virtual_base_count; ++i)
		{
			const VirtualBaseLayout& base = program_->VirtualBase(item.entity, i);
			work_.push_back(Item(base.entity, item.offset + base.offset, false));
		}
		PushMembers(item, bound);
	}
	return false;
}

bool EmptySubobjectMap::Conflict(EntityId root, std::uint64_t offset,
	bool complete)
{
	return Walk(root, offset, complete, EMPTY_WALK_CONFLICT);
}

bool EmptySubobjectMap::MemberConflict(TypeId type, std::uint64_t offset)
{
	std::uint64_t count = 0;
	const EntityId element = LayoutElementClass(*program_, type, &count);
	if (element == kNoEntity || count_ == 0) return false;
	const std::uint64_t size = program_->entities[element].object_size;
	for (std::uint64_t index = 0; index < count; ++index)
	{
		const std::uint64_t at = offset + index * size;
		if (at > max_offset_) break;
		if (Walk(element, at, true, EMPTY_WALK_CONFLICT)) return true;
	}
	return false;
}

void EmptySubobjectMap::Record(EntityId root, std::uint64_t offset,
	bool complete, bool whole)
{
	(void)Walk(root, offset, complete,
		whole ? EMPTY_WALK_RECORD : EMPTY_WALK_RECORD_REACHABLE);
}

void EmptySubobjectMap::RecordMember(TypeId type, std::uint64_t offset)
{
	std::uint64_t count = 0;
	const EntityId element = LayoutElementClass(*program_, type, &count);
	if (element == kNoEntity) return;
	const std::uint64_t size = program_->entities[element].object_size;
	for (std::uint64_t index = 0; index < count; ++index)
	{
		const std::uint64_t at = offset + index * size;
		if (at >= record_limit_) break;
		Walk(element, at, true, EMPTY_WALK_RECORD_REACHABLE);
	}
}

void BitFieldLayout::Clear()
{
	fields_.clear();
	runs_.clear();
}

void BitFieldLayout::BeginRun(std::uint64_t first_byte)
{
	if (!runs_.empty() && runs_.back().open) return;
	Run run;
	run.first = run.last = fields_.size();
	run.lo = first_byte;
	run.hi = 0;
	run.open = true;
	runs_.push_back(run);
}

void BitFieldLayout::Add(BindingId binding, std::uint64_t bit,
	std::uint64_t value_width, std::uint64_t storage_bytes, bool packed)
{
	if (runs_.empty() || !runs_.back().open)
		ThrowInternalCompilerError("bit-field has no open run");
	Field field;
	field.binding = binding;
	field.bit = bit;
	field.value_width = value_width;
	field.storage_bytes = storage_bytes;
	field.unit_begin = field.unit_bytes = field.split_bytes = 0;
	field.packed = packed;
	field.shared = false;
	fields_.push_back(field);
	runs_.back().last = fields_.size();
}

void BitFieldLayout::EndRun(std::uint64_t limit)
{
	if (runs_.empty() || !runs_.back().open) return;
	runs_.back().hi = limit;
	runs_.back().open = false;
}

void BitFieldLayout::AssignFields(const Run& run, std::size_t first,
	std::size_t last, std::uint64_t lo, std::uint64_t hi)
{
	for (std::size_t i = first; i < last; ++i)
	{
		Field& field = fields_[i];
		const std::uint64_t begin = field.bit / 8;
		const std::uint64_t end = CeilBytes(field.bit + field.value_width);
		if (!NarrowStorageWindow(begin, end, lo, hi,
			&field.unit_begin, &field.unit_bytes))
		{
			// A packed field no access within the class holds is read and
			// written piecewise as the bytes it spans (as g++ does), within a
			// 64-bit unit, or a 128-bit one past 8 bytes.
			if (field.packed && end - begin <= 16)
			{
				field.unit_begin = begin;
				field.unit_bytes = end - begin <= 8 ? 8 : 16;
				field.split_bytes = end - begin;
				field.shared = true;
				continue;
			}
			if (field.packed)
				ThrowSemanticError("packed bit-field has no storage unit in its class");
			field.unit_bytes = field.storage_bytes;
			field.unit_begin = begin / field.unit_bytes * field.unit_bytes;
			field.shared = true;
		}
	}
	(void)run;
	for (std::size_t i = first; i + 1 < last; ++i)
	{
		Field& left = fields_[i];
		Field& right = fields_[i + 1];
		const bool same = left.unit_begin == right.unit_begin &&
			left.unit_bytes == right.unit_bytes;
		if (!same && right.unit_begin < left.unit_begin + left.unit_bytes)
			left.shared = right.shared = true;
	}
}

void BitFieldLayout::AssignSegment(const Run& run, std::size_t first,
	std::size_t last, std::uint64_t begin, std::uint64_t bytes)
{
	std::uint64_t unit_begin = begin;
	std::uint64_t unit_bytes = bytes;
	if (begin < run.lo || begin + bytes > run.hi)
	{
		const std::uint64_t lo = std::max(run.lo, begin);
		const std::uint64_t hi = std::min(run.hi, begin + bytes);
		std::uint64_t data_first = std::numeric_limits<std::uint64_t>::max();
		std::uint64_t data_last = 0;
		for (std::size_t i = first; i < last; ++i)
		{
			data_first = std::min(data_first, fields_[i].bit / 8);
			data_last = std::max(data_last,
				CeilBytes(fields_[i].bit + fields_[i].value_width));
		}
		if (!NarrowStorageWindow(data_first, data_last, lo, hi,
			&unit_begin, &unit_bytes))
		{
			AssignFields(run, first, last, lo, hi);
			return;
		}
	}
	for (std::size_t i = first; i < last; ++i)
	{
		fields_[i].unit_begin = unit_begin;
		fields_[i].unit_bytes = unit_bytes;
	}
}

void BitFieldLayout::AssignRun(const Run& run)
{
	if (run.first == run.last) return;
	if (fields_[run.first].packed)
	{
		AssignFields(run, run.first, run.last, run.lo, run.hi);
		return;
	}
	struct Segment { std::size_t first, last; std::uint64_t begin, bytes; };
	std::vector<Segment> segments;
	for (std::size_t i = run.first; i < run.last; ++i)
	{
		const std::uint64_t bytes = fields_[i].storage_bytes;
		const std::uint64_t begin = fields_[i].bit / 8 / bytes * bytes;
		std::size_t first = i;
		while (!segments.empty() && begin <= segments.back().begin &&
			segments.back().begin + segments.back().bytes <= begin + bytes)
		{
			first = segments.back().first;
			segments.pop_back();
		}
		if (!segments.empty() && segments.back().begin <= begin &&
			begin + bytes <= segments.back().begin + segments.back().bytes)
		{
			segments.back().last = i + 1;
			continue;
		}
		const Segment segment = { first, i + 1, begin, bytes };
		segments.push_back(segment);
	}
	for (std::size_t i = 0; i < segments.size(); ++i)
		AssignSegment(run, segments[i].first, segments[i].last,
			segments[i].begin, segments[i].bytes);
}

void BitFieldLayout::Assign(Program* program, std::uint64_t open_limit)
{
	for (std::size_t i = 0; i < runs_.size(); ++i)
	{
		if (runs_[i].open)
		{
			runs_[i].hi = open_limit;
			runs_[i].open = false;
		}
		AssignRun(runs_[i]);
	}
	for (std::size_t i = 0; i < fields_.size(); ++i)
	{
		const Field& field = fields_[i];
		if (field.binding == kNoBinding) continue;
		BindingLayoutFact& layout = program->MutableBindingLayout(
			program->bindings[field.binding]);
		layout.member_offset = field.unit_begin;
		layout.bit_offset = static_cast<std::uint32_t>(
			field.bit - 8 * field.unit_begin);
		layout.bit_storage_bits = static_cast<std::uint32_t>(8 * field.unit_bytes);
		layout.bit_storage_shared = field.shared;
		layout.bit_storage_split = static_cast<std::uint32_t>(field.split_bytes);
	}
	Clear();
}

}
}
