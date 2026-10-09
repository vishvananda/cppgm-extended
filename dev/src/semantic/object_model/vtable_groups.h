#pragma once

#include "semantic/analysis/index_tables.h"
#include "semantic/model/graph.h"
#include "semantic/model/program.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace cppgm
{
namespace semantic
{

typedef std::function<FunctionSignatureKey(BindingId)> SignatureKeyOf;

// Where a class's tables lie: in the class `layout` (the complete object),
// with the class whose final overriders they hold at `offset`.  A class's
// own vtable group has itself as its layout; a construction vtable group has
// the complete class whose base it is being constructed in.
struct VtablePlacement
{
	const Program* program;
	EntityId layout;
	std::uint64_t offset;

	VtablePlacement(const Program& program_value, EntityId layout_value,
		std::uint64_t offset_value)
		: program(&program_value), layout(layout_value), offset(offset_value) {}

	std::uint64_t VirtualBaseOffset(EntityId base) const;
};

// A final overrider's offset in the layout class.
std::int64_t OverriderPosition(const VtablePlacement& placement,
	const VirtualSlotFact& slot);

// The virtual bases sharing a table through its class's primary chain (and
// the virtual base the table's subobject lies in).
std::vector<EntityId> TableVirtualAnchors(
	const std::vector<ClassPolymorphismFacts>& all, EntityId table_class,
	EntityId table_anchor);

// Gives the slots of the table for a subobject of class `table_class` (in
// the virtual base `table_anchor` of the placed class, or none) at
// `table_offset` the `this` adjustments of their entries (Itanium 5.1.4):
// a fixed one, or a virtual thunk's move to a virtual base and the place of
// the vcall offset it reads; the entries of a virtual primary base lost to
// another subobject are marked unused (2.5.2).
void PlaceTableSlots(const VtablePlacement& placement,
	const std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId table_class, EntityId table_anchor, std::uint64_t table_offset,
	std::vector<VirtualSlotFact>* slots);

// A table's vcall offsets (clang's AddVCallOffsets): for each vcall entry,
// the distance from the table's subobject to the final overrider of its
// signature, which the table's slots or the placed class's views lying in
// one of `anchors` hold.
void FillVcallRows(const VtablePlacement& placement,
	const SignatureKeyOf& key_of, const ClassPolymorphismFacts& placed,
	const std::vector<VtablePrefixEntry>& entries,
	const std::vector<VirtualSlotFact>& slots,
	const std::vector<EntityId>& anchors, std::int64_t table_offset,
	std::vector<std::int64_t>* rows);

// The subobjects whose tables a class's vtable group holds, in group order
// (Itanium 2.5.2, clang's ItaniumVTableBuilder): only each table's class,
// virtual base, relative and complete offsets are set.
std::vector<ConstructionTableFact> VtableGroupOrder(const Program& program,
	const std::vector<ClassPolymorphismFacts>& all, EntityId entity);

// The construction vtable groups a complete class's VTT names (Itanium
// 2.6.2-2.6.4, clang's construction ItaniumVTableBuilder): one for each base
// with virtual bases at each place it has in the class.
void BuildConstructionGroups(const Program& program,
	std::vector<ClassPolymorphismFacts>& all, const SignatureKeyOf& key_of,
	EntityId complete);

}
}
