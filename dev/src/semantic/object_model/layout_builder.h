#pragma once

#include "semantic/model/graph.h"
#include "semantic/model/program.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cppgm
{
namespace semantic
{

typedef std::vector<std::vector<ClassLayoutMember> > ClassLayoutMemberTable;

// The running sizes of a class being laid out (Itanium C++ ABI 2.4): its
// size so far, its data size (the size without tail padding, where the next
// non-empty component starts), and its alignment.
struct ClassLayoutCursor
{
	std::uint64_t size, data_size, alignment, natural_alignment;
	// The next free bit while byte data_size - 1 is only partly filled by a
	// bit-field declared in the class itself (2.4 II.1a).
	std::uint64_t bit_end;
	bool open_bit_byte;

	ClassLayoutCursor()
		: size(0), data_size(0), alignment(1), natural_alignment(1),
		  bit_end(0), open_bit_byte(false) {}

	void Place(std::uint64_t end, bool data);
};

// The class type of an object type, through arrays; `count` receives the
// number of elements (1 for a non-array).
EntityId LayoutElementClass(const Program& program, TypeId type,
	std::uint64_t* count);

// Whether a class just laid out, or one of its subobjects, is an empty class.
bool ClassHasEmptySubobject(const Program& program,
	const ClassLayoutMemberTable& table, EntityId entity);

// A completed class's virtual base ordinals in construction order.
std::vector<std::uint32_t> VirtualBaseConstructionOrder(const Program& program,
	EntityId entity);

// The indirect primary bases of `entity` (Itanium 2.4 I.a): the virtual
// primary bases of the classes below it, each class visited once.
void CollectIndirectPrimaryBases(const Program& program,
	const std::vector<ClassPolymorphismFacts>& facts, EntityId entity,
	std::vector<EntityId>* visited, std::vector<EntityId>* indirect);

// The empty class subobjects a class layout has placed (2.4 II.2-3): two
// subobjects of one type never share an offset, and only empty ones can
// meet.  A non-empty component lies below the data size, where only a later
// empty component tried at offset zero can reach, so of its subobjects only
// those below the record limit (the size of the largest such empty
// component) are recorded; an empty component is recorded whole.
class EmptySubobjectMap
{
public:
	EmptySubobjectMap();

	void Begin(const Program* program, const ClassLayoutMemberTable* members,
		std::uint64_t record_limit);
	bool Conflict(EntityId root, std::uint64_t offset, bool complete);
	bool MemberConflict(TypeId type, std::uint64_t offset);
	void Record(EntityId root, std::uint64_t offset, bool complete,
		bool whole);
	void RecordMember(TypeId type, std::uint64_t offset);
	std::size_t Visits() const { return visits_; }

private:
	struct Item
	{
		EntityId entity;
		std::uint64_t offset;
		bool complete;

		Item(EntityId entity_value = kNoEntity, std::uint64_t offset_value = 0,
			bool complete_value = false)
			: entity(entity_value), offset(offset_value),
			  complete(complete_value) {}
	};

	bool Find(EntityId entity, std::uint64_t offset, bool insert);
	bool Walk(EntityId root, std::uint64_t offset, bool complete, int mode);
	void PushMembers(const Item& item, std::uint64_t bound);

	const Program* program_;
	const ClassLayoutMemberTable* members_;
	std::vector<Item> slots_, work_;
	std::size_t count_, visits_;
	std::uint64_t max_offset_, record_limit_;
};

// The bit-fields of the class being laid out, in declaration order with
// their bit positions, given storage units once the class's size is known.
// A run of adjacent bit-fields (ended by any other member or a zero-width
// bit-field) takes the naturally aligned objects of the fields' declared
// types that hold them (SysV); those nest, so the largest ones partition the
// run.  A unit reaching outside the bytes the run may touch -- another
// component, or tail padding a derived class may reuse -- is narrowed to the
// narrowest 1-, 2-, 4- or 8-byte integer that holds its fields there.
class BitFieldLayout
{
public:
	void Clear();
	void Add(BindingId binding, std::uint64_t bit, std::uint64_t value_width,
		std::uint64_t storage_bytes, bool packed);
	void BeginRun(std::uint64_t first_byte);
	void EndRun(std::uint64_t limit);
	void Assign(Program* program, std::uint64_t open_limit);

private:
	struct Field
	{
		BindingId binding;
		std::uint64_t bit, value_width, storage_bytes;
		std::uint64_t unit_begin, unit_bytes;
		bool packed, shared;
	};
	struct Run
	{
		std::size_t first, last;
		std::uint64_t lo, hi;
		bool open;
	};

	void AssignRun(const Run& run);
	void AssignSegment(const Run& run, std::size_t first, std::size_t last,
		std::uint64_t begin, std::uint64_t bytes);
	void AssignFields(const Run& run, std::size_t first, std::size_t last,
		std::uint64_t lo, std::uint64_t hi);

	std::vector<Field> fields_;
	std::vector<Run> runs_;
};

}
}
