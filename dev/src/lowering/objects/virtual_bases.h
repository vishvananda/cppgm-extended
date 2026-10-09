#ifndef CPPGM_LOWERING_OBJECTS_VIRTUAL_BASES_H
#define CPPGM_LOWERING_OBJECTS_VIRTUAL_BASES_H

#include "semantic/model/program.h"
#include "semantic/model/graph.h"
#include "lowering/support/errors.h"
#include "lowering/support/sequences.h"
#include "lowering/presentation/local_names.h"
#include "lowering/ir/model.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace lowering::ir;
using namespace lowering::support;

template <class Derived>
class VirtualBaseLowering
{
protected:
	bool IncludesConstructionVtt(BindingId binding) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		return BaseEntryTakesVtt(derived.program_,
			derived.graph_.class_polymorphism, binding);
	}

	// A base-object constructor or destructor of a class with virtual bases
	// takes the VTT after `this` (Itanium 2.6); there are no other hidden
	// parameters: virtual bases are found through the vtable.
	void AppendConstructionVttParameter(std::uint32_t node,
		std::vector<Parameter>* parameters) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		if (!IncludesConstructionVtt(derived.arena_.nodes[node].binding)) return;
		Parameter vtt;
		vtt.name = InternLocalName(derived.output_, "__vtt");
		vtt.type = LowPtr();
		parameters->insert(parameters->begin() + 1, vtt);
	}

	void PrepareConstructionVtt(std::uint32_t node)
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		current_construction_vtt_ =
			IncludesConstructionVtt(derived.arena_.nodes[node].binding) ?
			ParameterId(1) : ParameterId(kNoLowId);
		construction_table_block_ = BlockId(kNoLowId);
	}

	// The table VTT[0] names, which a base-object constructor or destructor
	// reads its own virtual bases' offsets from: the VTT does not change, so
	// one load serves the rest of the block.
	Operand CurrentConstructionTable()
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (construction_table_block_ != derived.current_block_)
		{
			construction_table_ =
				derived.LoadStorage(CurrentConstructionVtt(), LowPtr());
			construction_table_block_ = derived.current_block_;
			construction_rows_.clear();
		}
		return construction_table_;
	}

	// The variable an expression designates through single-operand nodes.
	BindingId BoundaryBindingForExpression(std::uint32_t node) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		for (std::size_t depth = 0;
			depth <= derived.arena_.nodes.size(); ++depth)
		{
			const DumpNode& record = derived.arena_.nodes[node];
			if (record.kind == DUMP_ID_EXPRESSION &&
				record.binding != kNoBinding)
				return record.binding;
			const NodeChildren children = derived.Children(node);
			if (children.size() != 1) break;
			node = children[0];
		}
		return kNoBinding;
	}

	// FindVirtualBasePathAnchor (vtt_layout.h) for the program lowered.
	bool VirtualBasePathAnchor(EntityId owner, EntityId target,
		EntityId* anchor, std::uint64_t* relative_offset) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		return FindVirtualBasePathAnchor(derived.program_, owner, target, anchor,
			relative_offset);
	}

	bool HasCurrentConstructionVtt() const
	{
		return current_construction_vtt_ != kNoLowId;
	}

	Operand CurrentConstructionVtt() const
	{
		if (!HasCurrentConstructionVtt())
			ThrowLoweringInternal("construction VTT parameter is unavailable");
		return Operand(current_construction_vtt_, LowPtr());
	}

	std::uint32_t ConstructionVttIndex(EntityId owner, EntityId base,
		bool complete, bool virtual_base) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		if (owner >= derived.polymorphism_.class_vtt_layouts.size() ||
			!derived.polymorphism_.class_vtt_layouts[owner].computed)
			ThrowLoweringInternal("constructor's class has no VTT layout");
		return SubVttIndex(derived.program_,
			derived.polymorphism_.class_vtt_layouts[owner], owner, base, complete,
			virtual_base);
	}

	Operand ConstructionVttArgument(EntityId complete, EntityId base,
		bool virtual_base = false)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const EntityId owner = complete == kNoEntity ?
			derived.current_member_owner_ : complete;
		if (owner == kNoEntity)
			ThrowLoweringInternal("construction VTT has no owning class");
		if (complete == kNoEntity)
		{
			if (!HasCurrentConstructionVtt())
				ThrowLoweringInternal("nested base constructor has no VTT");
			if (base == owner) return CurrentConstructionVtt();
		}
		if (complete != kNoEntity &&
			(complete >= derived.polymorphism_.class_vtt_symbols.size() ||
			 derived.polymorphism_.class_vtt_symbols[complete] == kNoLowId))
			ThrowLoweringInternal("complete constructor has no VTT symbol");
		const std::uint32_t index =
			ConstructionVttIndex(owner, base, complete != kNoEntity, virtual_base);
		if (index == kNoVttIndex)
			ThrowLoweringInternal("base constructor has no construction VTT slice");
		const Operand offset(static_cast<std::int64_t>(index) * 8, LowI64());
		if (complete == kNoEntity)
			return derived.IndexAddress(LowI8(), CurrentConstructionVtt(),
				offset, false);
		const SymbolId symbol =
			derived.polymorphism_.class_vtt_symbols[complete];
		derived.output_.symbols[symbol].referenced = true;
		const Operand table = derived.Temp(LowPtr());
		Instruction address(Instruction::ADDR);
		address.dest = table.id;
		address.first = Operand(Operand::GLOBAL, symbol, LowPtr());
		derived.Emit(address);
		return derived.IndexAddress(LowI8(), table, offset, false);
	}

	// The address of `owner`'s virtual base through the object's vbase-offset
	// row.  The object under construction or destruction in a base-object
	// constructor or destructor reads its construction vtable from the VTT,
	// as its own vptr may still hold a base's table (Itanium 2.6).
	Operand RuntimeVirtualBaseAddress(const Operand& view, EntityId owner,
		std::size_t virtual_base_ordinal, bool current_object = false)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if ((view.kind == Operand::INTEGER && view.integer_value == 0) ||
			view.kind == Operand::NULL_POINTER)
			return Operand(0, LowPtr());
		const bool construction = current_object && HasCurrentConstructionVtt() &&
			owner == derived.current_member_owner_;
		const Operand vtable = construction ? CurrentConstructionTable() :
			derived.LoadStorage(view, LowPtr());
		// Row 0 is nearest the address point (Itanium 2.5.2).
		if (owner >= derived.graph_.class_polymorphism.size() ||
			virtual_base_ordinal >= derived.graph_.class_polymorphism[
				owner].virtual_base_rows.size())
			ThrowLoweringInternal("virtual base has no vtable row");
		for (std::size_t i = 0; construction && i < construction_rows_.size(); ++i)
			if (construction_rows_[i].first == virtual_base_ordinal)
				return derived.IndexAddress(LowI8(), view,
					construction_rows_[i].second, false);
		const std::int64_t row = -24 - 8 * static_cast<std::int64_t>(
			derived.graph_.class_polymorphism[owner].virtual_base_rows[
				virtual_base_ordinal]);
		const Operand entry = derived.IndexAddress(
			LowI8(), vtable, Operand(row, LowI64()), false);
		const Operand offset = derived.LoadStorage(entry, LowI64());
		if (construction)
			construction_rows_.push_back(std::make_pair(virtual_base_ordinal, offset));
		return derived.IndexAddress(LowI8(), view, offset, false);
	}

	Operand ProjectNullableVirtualBaseAddress(const Operand& view,
		std::uint64_t offset, EntityId owner = kNoEntity,
		std::size_t virtual_base_ordinal = 0)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if ((view.kind == Operand::INTEGER && view.integer_value == 0) ||
			view.kind == Operand::NULL_POINTER) return Operand(0, LowPtr());
		Slot slot;
		slot.name = InternLocalName(derived.output_,
			derived.GeneratedSlotName("basecast"));
		slot.type = LowPtr();
		const Operand result(
			static_cast<SlotId>(derived.function_->slots.size()), LowPtr());
		derived.function_->slots.push_back(slot);
		const BlockId null_block = derived.AddBlock(
			derived.NewLabel("basecast_null"));
		const BlockId adjust_block = derived.AddBlock(
			derived.NewLabel("basecast_adjust"));
		const BlockId end_block = derived.AddBlock(
			derived.NewLabel("basecast_end"));
		const Operand is_null = derived.Temp(LowI64());
		Instruction compare(Instruction::CMP);
		compare.dest = is_null.id;
		compare.op = LOW_OP_EQ;
		compare.type = LowPtr();
		compare.first = view;
		compare.second = Operand(0, LowPtr());
		derived.Emit(compare);
		derived.EmitBranch(is_null, null_block, adjust_block);
		derived.SelectBlock(null_block);
		Instruction store(Instruction::STORE);
		store.type = LowPtr();
		store.first = Operand(0, LowPtr());
		store.second = result;
		derived.Emit(store);
		derived.EmitJump(end_block);
		derived.SelectBlock(adjust_block);
		store.first = derived.ProjectBaseSubobjectOffset(owner == kNoEntity ?
			view : RuntimeVirtualBaseAddress(view, owner, virtual_base_ordinal),
			offset);
		derived.Emit(store);
		derived.EmitJump(end_block);
		derived.SelectBlock(end_block);
		return derived.LoadStorage(result, LowPtr());
	}

	bool RuntimeVirtualBaseAddressForExpression(std::uint32_t expression,
		const Operand& view, EntityId target, Operand* address)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const DumpNode& source = derived.arena_.nodes[expression];
		// A named complete object has a fixed layout. Reference and pointer
		// bindings still need the table of the actual containing object.
		if (source.kind == DUMP_TEMPORARY_OBJECT ||
			(source.kind == DUMP_ID_EXPRESSION && source.binding != kNoBinding &&
			 derived.program_.types.Get(derived.program_.types.RemoveTopCv(
				derived.program_.bindings[source.binding].type)).kind == TYPE_NAMED))
			return false;
		const EntityId owner = derived.BaseEntityForType(
			derived.arena_.nodes[expression].type);
		if (owner == kNoEntity ||
			owner >= derived.graph_.class_polymorphism.size() ||
			!derived.graph_.class_polymorphism[owner].HasObjectTable()) return false;
		EntityId anchor = kNoEntity;
		std::uint64_t relative_offset = 0;
		if (!VirtualBasePathAnchor(
			owner, target, &anchor, &relative_offset)) return false;
		const EntityRecord& record = derived.program_.entities[owner];
		for (std::size_t ordinal = 0;
			ordinal < record.virtual_base_count; ++ordinal)
		{
			if (derived.program_.VirtualBase(owner, ordinal).entity != anchor)
				continue;
			const bool nullable = derived.program_.types.Get(
				derived.program_.types.RemoveTopCv(source.type)).kind == TYPE_POINTER &&
				!(source.kind == DUMP_ID_EXPRESSION &&
				  source.binding != kNoBinding &&
				  source.binding == derived.current_this_binding_) &&
				!(source.kind == DUMP_UNARY_EXPRESSION && source.OperationIs(OP_AMP));
			if (nullable)
			{
				*address = ProjectNullableVirtualBaseAddress(
					view, relative_offset, owner, ordinal);
				return true;
			}
			*address = derived.ProjectBaseSubobjectOffset(
				RuntimeVirtualBaseAddress(view, owner, ordinal,
					derived.current_this_binding_ != kNoBinding &&
					BoundaryBindingForExpression(expression) ==
						derived.current_this_binding_), relative_offset);
			return true;
		}
		return false;
	}

	void ResetConstructionVtt()
	{
		current_construction_vtt_ = kNoLowId;
		construction_table_block_ = BlockId(kNoLowId);
	}

	ParameterId current_construction_vtt_;
	BlockId construction_table_block_;
	Operand construction_table_;
	std::vector<std::pair<std::size_t, Operand> > construction_rows_;
};

}
}

#endif
