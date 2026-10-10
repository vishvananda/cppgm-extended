#include "lower/program_lowering.h"

#include "lower/function_lowering.h"

#include <cstring>

#include "lowir/literals.h"
#include "sema/analyzer.h"
#include "sema/conversions.h"

// Polymorphic classes' run-time data (Itanium C++ ABI 2.5, 2.9.5), built
// from the analysis's virtual tables: a class's virtual table (the offset
// to the top, its type information, then a function address per slot,
// __cxa_pure_virtual for a pure one) and the type information objects
// (the runtime's class, single- or multiple-inheritance kind, the type's
// name, its bases). Each object is one global of its ABI name, declared
// on first reference and defined once per program: a table by the unit
// defining the class's key function, or where its address is stored when
// the class has none; the type information by the unit defining a table
// (or a derived class's type information) that refers to it.
namespace lower {

namespace {

const sema::ClassEntity* BaseClassOf(const sema::BaseClass* base)
{
	return static_cast<const sema::ClassEntity*>(base->base);
}

// Itanium 2.9.5p6: a single public non-virtual base at offset zero.
bool SingleInheritance(const sema::ClassEntity* entity)
{
	const sema::BaseClass* base = entity->bases;
	return base != nullptr && base->next == nullptr && !base->is_virtual && base->access == sema::Access::Public &&
		base->offset == 0;
}

// The base subobjects the flags' walk may visit; past it the repeat flag
// is set (it only disables the runtime's shortcuts).
const uint32_t kFlagWork = 1u << 16;

} // namespace

bool OnPrimaryChain(const sema::VtableGroup* group, uint32_t segment)
{
	// Its table pointer is the complete class's: it shares it up to
	// segment 0.
	while (segment != 0 && group->segments[segment].shared)
		segment = group->segments[segment].parent;
	return segment == 0;
}

uint32_t ProgramLowering::inheritance_flags(const sema::ClassEntity* entity)
{
	// Itanium 2.9.5p6's __vmi_class_type_info flags: 0x1 when a class is a
	// base subobject more than once without being one shared virtual base
	// (__non_diamond_repeat_mask), as GCC marks it: every path walked, a
	// shared virtual base's own bases once; 0x2 when a virtual base is
	// reached through several paths (__diamond_shaped_mask: libsupc++'s
	// searches stop early without it, so a cross cast to a virtual base
	// reached privately first succeeds where GCC's type information fails
	// it).
	std::vector<const sema::ClassEntity*>& stack = flag_stack_;
	PointerMap& marks = flag_marks_;
	marks.clear();
	uint32_t flags = 0, work = 0;
	stack.assign(1, entity);
	while (!stack.empty())
	{
		const sema::ClassEntity* cls = stack.back();
		stack.pop_back();
		for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
		{
			if (++work > kFlagWork)
				return 1;
			const sema::ClassEntity* next = BaseClassOf(base);
			const uint32_t found = marks.find(next);
			const uint32_t seen = found == PointerMap::kMissing ? 0 : found;
			if (base->is_virtual)
			{
				flags |= (seen & 1) != 0 ? 1 : 0;
				if ((seen & 2) != 0)
				{
					flags |= 2;
					continue;
				}
				marks.set(next, seen | 2);
			}
			else
			{
				flags |= seen != 0 ? 1 : 0;
				marks.set(next, seen | 1);
			}
			stack.push_back(next);
		}
	}
	return flags;
}

lowir::DataItem AddressItem(lowir::SymbolId symbol, int64_t addend)
{
	lowir::DataItem item = lowir::DataItem();
	item.kind = lowir::DataItemKind::Address;
	item.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	item.address = symbol;
	item.amount = addend;
	return item;
}

lowir::DataItem EntryItem(lowir::SymbolId target)
{
	return target != lowir::kNoId ? AddressItem(target, 0) : ScalarItem(lowir::TypeKind::Ptr, 0);
}

lowir::DataItem ScalarItem(lowir::TypeKind kind, uint64_t value)
{
	lowir::DataItem item = lowir::DataItem();
	item.kind = lowir::DataItemKind::Scalar;
	item.type = lowir::Type::scalar(kind);
	item.constant.kind = lowir::ValueKind::Integer;
	item.constant.float_kind = lowir::TypeKind::Void;
	item.constant.bits = lowir::integer_literal(static_cast<int64_t>(value));
	return item;
}

bool ProgramLowering::instantiated_elsewhere(const sema::ClassEntity* entity)
{
	// 14.7.2p10-p11: a class template specialization an explicit
	// instantiation declaration names, or a member class of one (p8), and
	// no definition in this unit: its explicit instantiation definition is
	// in another unit of the program, whose object holds the class's table
	// group, VTT and type information (Itanium 5.2.3, as GCC and Clang
	// treat an `extern template` class).
	for (const sema::ClassEntity* cls = entity; cls != nullptr && cls->templ != nullptr;
		cls = cls->owner != nullptr && cls->owner->kind == sema::ScopeKind::Class
			? static_cast<const sema::ClassEntity*>(cls->owner->entity)
			: nullptr)
	{
		if (cls->templ->explicit_instantiation)
			return false;
		if (cls->templ->extern_instantiation)
			return true;
	}
	return false;
}

bool ProgramLowering::emits_vtable(const sema::ClassEntity* entity) const
{
	// Itanium 5.2.3: with a key function, the unit defining it; without
	// one, each unit that needs the table; an explicitly instantiated
	// class's, its explicit instantiation definition's unit.
	const sema::FunctionEntity* key = entity->virtuals != nullptr ? entity->virtuals->key_function : nullptr;
	return (key == nullptr || key->defined) && !instantiated_elsewhere(entity);
}

std::string ProgramLowering::class_object(const sema::ClassEntity* entity, abi_mangle::AbiTargetKind kind)
{
	abi_mangle::AbiTarget target;
	target.kind = kind;
	target.node = abi_type(entity->type);
	std::string out;
	encoder_->encode(target, out);
	++counters_.mangled_names;
	return out;
}

lowir::SymbolId ProgramLowering::class_global(
	const sema::ClassEntity* entity, abi_mangle::AbiTargetKind kind, const char* prefix, bool internal_data)
{
	// The course's presentation names: `<qualified name>__vtable`,
	// `__rtti_<class-key>_<qualified name>`, `__typeinfo_name__<class-key>_
	// <qualified name>`; a local class's by its type's encoding. The run-time
	// data of a class of internal or no linkage (3.5p4, 9.8), or of a
	// specialization over one (3.5p8: its own template arguments count), is
	// the unit's own: another unit's data of the same spelling describes
	// another class.
	const std::string object = class_object(entity, kind);
	const bool internal = internal_data || entity_locality(entity) != Locality::External;
	std::string spelling = prefix;
	if (internal && entity->owner->kind != sema::ScopeKind::Namespace)
		spelling += "type_" + class_object(entity, abi_mangle::ABI_TARGET_TYPE);
	else if (kind == abi_mangle::ABI_TARGET_VTABLE)
		spelling = class_spelling(entity) + "__vtable";
	else
	{
		if (kind == abi_mangle::ABI_TARGET_TYPEINFO_NAME)
			spelling += "_";
		spelling += entity->key == TokenKind::KW_CLASS ? "class_"
			: entity->key == TokenKind::KW_UNION       ? "union_"
													   : "struct_";
		spelling += class_spelling(entity);
	}
	lowir::SymbolId symbol = lowir::kNoId;
	if (internal)
	{
		symbol = new_symbol(spelling);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, spelling);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved)
	{
		PendingDeclaration pending = PendingDeclaration();
		pending.symbol = symbol;
		pending.function = false;
		pending.typed = false;
		pending.metadata.binding = internal ? ir_model::SBM_INTERNAL : ir_model::SBM_STRONG;
		if (symbol_objects_[symbol] != 0)
			pending.metadata.object = symbol_objects_[symbol];
		set_pending(symbol, pending);
	}
	return symbol;
}

lowir::SymbolId ProgramLowering::vtable_symbol(const sema::ClassEntity* entity)
{
	const uint32_t found = vtables_.find(entity);
	if (found != PointerMap::kMissing)
		return found;
	const lowir::SymbolId symbol = class_global(entity, abi_mangle::ABI_TARGET_VTABLE, "__vtable_");
	vtables_.set(entity, symbol);
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved && emits_vtable(entity))
		define_vtable(entity);
	return symbol;
}

void ProgramLowering::define_vtable(const sema::ClassEntity* entity)
{
	// Itanium 2.5.2: the offset to the top (0: the primary table), the
	// type information, then the slots.
	const lowir::SymbolId symbol = vtables_.find(entity);
	// The referenced symbols first: defining them appends data of their own.
	std::vector<lowir::SymbolId> targets;
	const uint32_t size = entity->virtuals->vtable_size;
	targets.reserve(size + 1);
	targets.push_back(typeinfo_symbol(entity));
	// A key function defined inline after all puts the table in every
	// unit defining it (Itanium 5.2.3), as a class without one does.
	const sema::FunctionEntity* key = entity->virtuals->key_function;
	lowir::SymbolMetadata metadata;
	metadata.binding = entity_locality(entity) != Locality::External ? ir_model::SBM_INTERNAL
		: key != nullptr && !key->is_inline                          ? ir_model::SBM_STRONG
																	 : ir_model::SBM_WEAK;
	// The group gives the primary table's final overriders (a virtual
	// primary base's may lie off the chain).
	const sema::VirtualSlot* slots = analyzer_->virtual_slots(entity);
	const sema::VtableGroup* own_group = entity->virtuals->group;
	for (uint32_t i = 0; i < size; ++i)
	{
		if (own_group != nullptr)
			targets.push_back(table_entry(own_group->entries[i].function, own_group->entries[i].kind,
				entry_adjustment(TableContext{ entity, entity, 0, false }, 0, i), metadata.binding, slots[i].adjusted,
				entity));
		else
			targets.push_back(table_entry(slots[i].function, slots[i].kind, ThisAdjustment{ 0, 0, false },
				metadata.binding, slots[i].adjusted, entity));
	}
	// Object mode: the group is one object, its secondary tables after the
	// primary in the external layout (table_address), so its symbol names
	// the whole group as Itanium's does: a COMDAT copy keeps it together,
	// and another unit (an extern template's, a key function's) addresses
	// the group from the symbol alone. Their slots' symbols first.
	const bool one_object = object_mode_ && entity->virtuals->group != nullptr;
	const size_t primary_slots = targets.size();
	if (one_object)
		group_targets(entity, metadata.binding, targets);
	// A class with virtual bases has their offsets' rows first.
	const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
	table_rows(TableContext{ entity, entity, 0, false }, 0);
	data_item(ScalarItem(lowir::TypeKind::I64, 0));
	for (size_t i = 0; i < primary_slots; ++i)
		data_item(i == 0 ? AddressItem(targets[i], 0) : EntryItem(targets[i]));
	if (one_object)
	{
		group_items(entity, targets.data() + primary_slots);
		define_data(symbol, first, metadata);
		++counters_.vtables;
	}
	else
	{
		define_data(symbol, first, metadata);
		++counters_.vtables;
		if (entity->virtuals->group != nullptr)
			define_views(entity, metadata.binding);
	}
	if (VirtualBaseCount(entity) != 0)
		vtt_symbol(entity);
	// The course defines a table group's dynamic bases' tables with it
	// (where the unit may: no key function, or one defined here); a host
	// object those its code refers to (a base constructor's).
	for (const sema::BaseClass* base = entity->virtuals->group != nullptr && !host_objects_ ? entity->bases : nullptr;
		base != nullptr; base = base->next)
		class_table(BaseClassOf(base));
}

lowir::SymbolId ProgramLowering::table_entry(const sema::FunctionEntity* function, sema::SlotKind kind,
	ThisAdjustment adjustment, uint8_t binding, const sema::FunctionEntity* introducer,
	const sema::ClassEntity* through, bool strong)
{
	// A slot's address: its final overrider's entry, through a thunk
	// moving `this` from the table's subobject to the overrider's class's.
	// A deleted function's slot is never called (10.3p16 keeps its
	// overriders deleted); LowIR has no __cxa_deleted_virtual role, so it
	// takes the pure-virtual handler (Itanium 2.5.2's slot for it). A
	// covariant result needing adjustment from the overrider's class to the
	// one `introducer` returns (the table class's VirtualSlot::adjusted: the
	// function sharing the slot nearest it, whose result every caller
	// sharing the slot reads) goes through a covariant thunk, the result
	// reaching that class through the one subobject it designates (a class
	// repeated in the hierarchy is reached through it): `through`, the
	// table's class.
	if (adjustment.unused)
		return lowir::kNoId;
	if (function->pure || function->deleted)
		return pure_virtual_symbol(function);
	ResultAdjustment result = ResultAdjustment();
	const bool covariant = covariant_entry(function, kind, introducer, through, result);
	if (kind == sema::SlotKind::CompleteDestructor)
		demand_vtable_entries(function);
	else if (kind == sema::SlotKind::Function)
		demand(function);
	const EntryVariant variant = kind == sema::SlotKind::DeletingDestructor ? EntryVariant::Deleting
																			: EntryVariant::Complete;
	if (adjustment.offset != 0 || adjustment.vcall != 0 || covariant)
		return thunk_symbol(function, variant, adjustment, binding, covariant ? &result : nullptr, strong);
	return entry_symbol(function, variant);
}

bool ProgramLowering::covariant_entry(const sema::FunctionEntity* function, sema::SlotKind kind,
	const sema::FunctionEntity* introducer, const sema::ClassEntity* through, ResultAdjustment& result)
{
	// `introducer`: the function the slot's result is adjusted to
	// (VirtualSlot::adjusted, of the table's class).
	return kind == sema::SlotKind::Function && function != introducer &&
		result_adjustment(function, introducer, result, through);
}

ProgramLowering::ThisAdjustment ProgramLowering::entry_adjustment(
	const TableContext& context, uint32_t segment, uint32_t slot)
{
	// From the entry's subobject (slot_anchor: the table's, or a virtual
	// primary base on its chain where the context put it) to the
	// overrider's: the path crosses a virtual edge exactly when the
	// subobject lies in a virtual base the overrider does not, the last one
	// on the path, whose table holds a vcall offset per signature (a
	// virtual thunk, its fixed part to that base).
	const sema::VtableGroup* group = context.cls->virtuals->group;
	const sema::VtableSegment& part = group->segments[segment];
	const sema::VtableEntry& entry = group->entries[part.first_entry + slot];
	const int64_t overrider_at = context_at(context, entry.overrider);
	const sema::VirtualSlot* slots = analyzer_->virtual_slots(part.cls);
	const sema::FunctionEntity* declared = slots[slot].function;
	uint32_t place = segment, within = part.within;
	bool unused = false;
	if (const sema::ClassEntity* anchor = slot_anchor(context, segment, slot, unused))
		place = within = group_segment(context.cls, virtual_base_offset(context.cls, anchor), anchor);
	if (unused)
		return ThisAdjustment{ 0, 0, true };
	const int64_t level_at = context_at(context, place);
	ResultAdjustment result = ResultAdjustment();
	if (level_at == overrider_at && !(entry.function->pure || entry.function->deleted) &&
		!covariant_entry(entry.function, entry.kind, slots[slot].adjusted, part.cls, result))
		return ThisAdjustment{ 0, 0, false };
	if (within != 0 && group->segments[entry.overrider].within != within)
		return ThisAdjustment{ context_at(context, within) - level_at,
			itanium_vcall_offset(group->segments[within].cls, declared), false };
	return ThisAdjustment{ overrider_at - level_at, 0, false };
}

const sema::ClassEntity* ProgramLowering::slot_anchor(
	const TableContext& context, uint32_t segment, uint32_t slot, bool& unused)
{
	// g++'s first_defn (update_vtable_entry_for_fn): along the primary chain
	// of the table's class, the nearest class declaring an override of the
	// slot's function -- for a covariant entry, the nearest below the
	// overrider's class whose own entry is no thunk; the entry's subobject
	// is the last virtual primary base on the way there (null for none: the
	// table's own). Each class of the chain lies where its link puts it in
	// the context's complete object; a class whose virtual primary base
	// lies elsewhere has lost it (g++'s BINFO_LOST_PRIMARY_P), and an entry
	// whose walk passes such a link is unused.
	const sema::VtableGroup* group = context.cls->virtuals->group;
	const sema::VtableSegment& part = group->segments[segment];
	const sema::VtableEntry& entry = group->entries[part.first_entry + slot];
	unused = false;
	// (Local: own_entry_thunk asks again for chain classes.)
	std::vector<const sema::ClassEntity*> chain;
	for (const sema::ClassEntity* link = part.cls; link != nullptr;)
	{
		if (link->virtuals == nullptr || slot >= link->virtuals->vtable_size)
			break;
		chain.push_back(link);
		const sema::BaseClass* primary = link->virtuals->primary_base;
		link = primary != nullptr ? BaseClassOf(primary) : nullptr;
	}
	const size_t depth = chain.size();
	if (depth == 0)
		return nullptr;
	std::vector<uint8_t> lost(depth, 0);
	int64_t place = context_at(context, segment);
	for (size_t k = 1; k < depth; ++k)
	{
		if (!chain[k - 1]->virtuals->primary_base->is_virtual)
			continue;
		const int64_t at = static_cast<int64_t>(virtual_base_offset(context.complete, chain[k]));
		lost[k] = at != place ? 1 : 0;
		place = at;
	}
	size_t parent = depth - 1;
	for (size_t j = 0; j < depth; ++j)
	{
		const sema::FunctionEntity* function = analyzer_->virtual_slots(chain[j])[slot].function;
		if (function->owner == chain[j]->scope)
		{
			parent = j;
			break;
		}
		if (j + 1 < depth && lost[j + 1] != 0)
			unused = true;
	}
	ResultAdjustment result = ResultAdjustment();
	if (!(entry.function->pure || entry.function->deleted) && entry.kind == sema::SlotKind::Function &&
		entry.function != analyzer_->virtual_slots(part.cls)[slot].adjusted &&
		result_adjustment(entry.function, analyzer_->virtual_slots(part.cls)[slot].adjusted, result, part.cls))
	{
		if (entry.function->owner == chain[parent]->scope && parent + 1 < depth)
			++parent;
		for (; parent + 1 < depth && own_entry_thunk(chain[parent], slot); ++parent)
		{
			if (lost[parent + 1] != 0)
				unused = true;
		}
	}
	const sema::ClassEntity* anchor = nullptr;
	for (size_t k = 1; k <= parent; ++k)
	{
		if (chain[k - 1]->virtuals->primary_base->is_virtual)
			anchor = chain[k];
	}
	return anchor;
}

ProgramLowering::ThisAdjustment ProgramLowering::construction_adjustment(
	const TableContext& context, uint32_t segment, uint32_t slot)
{
	const sema::VtableGroup* group = context.cls->virtuals->group;
	const sema::VtableSegment& part = group->segments[segment];
	if (group->entries[part.first_entry + slot].kind != sema::SlotKind::Function)
		return ThisAdjustment{ 0, 0, true };
	return part.shared ? entry_adjustment(context, segment, slot)
					   : entry_adjustment(TableContext{ context.cls, context.cls, 0, false }, segment, slot);
}

bool ProgramLowering::own_entry_thunk(const sema::ClassEntity* cls, uint32_t slot)
{
	// Its own table's entry (the class complete) moves `this` or adjusts a
	// covariant result.
	const sema::VirtualSlot* slots = analyzer_->virtual_slots(cls);
	const sema::VtableGroup* group = cls->virtuals->group;
	const sema::FunctionEntity* function = group != nullptr ? group->entries[slot].function : slots[slot].function;
	const sema::SlotKind kind = group != nullptr ? group->entries[slot].kind : slots[slot].kind;
	if (function->pure || function->deleted)
		return false;
	ResultAdjustment result = ResultAdjustment();
	if (covariant_entry(function, kind, slots[slot].adjusted, cls, result))
		return true;
	if (group == nullptr)
		return false;
	const ThisAdjustment adjustment = entry_adjustment(TableContext{ cls, cls, 0, false }, 0, slot);
	return adjustment.offset != 0 || adjustment.vcall != 0;
}

void ProgramLowering::definition_thunks(const sema::FunctionEntity* function, uint8_t binding)
{
	// A virtual function's definition carries the thunks the unit's table
	// groups and construction tables have for it (Itanium 5.1.4.2's thunks,
	// as GCC emits them with the function for every class the unit
	// completes: a derived class's table another unit defines may name
	// them): its class's primary table's covariant slots, the groups'
	// adjusted ones; strong when the function is (another unit's copy of a
	// group, its key function's, references them without defining them),
	// else weak as the function. Strong definitions are lowered before any
	// thunk is emitted (emit_demanded follows the unit's declarations), so
	// a thunk a table made weak becomes strong.
	if (binding == ir_model::SBM_INTERNAL || function->pure || function->deleted)
		return;
	const sema::ClassEntity* cls = sema::ClassOf(function->owner->entity->type);
	if (cls == nullptr || cls->virtuals == nullptr)
		return;
	if (definition_thunk_classes_.find(cls) == PointerMap::kMissing)
	{
		if (definition_thunk_classes_.find(nullptr) == PointerMap::kMissing)
		{
			// Whether a class's construction contexts may give a table no
			// group has (one of its own or its bases' groups shares a virtual
			// base's table pointer, which a context may not): its bases come
			// first in completion order.
			definition_thunk_classes_.set(nullptr, 1);
			for (const sema::ClassEntity* grouped : analyzer_->grouped_classes())
			{
				bool sensitive = false;
				const sema::VtableGroup* group = grouped->virtuals->group;
				for (uint32_t s = 1; s < group->segment_count && !sensitive; ++s)
					sensitive = group->segments[s].shared && group->segments[s].within == s;
				for (const sema::BaseClass* base = grouped->bases; base != nullptr && !sensitive; base = base->next)
					sensitive = construction_sensitive_.find(base->base) != PointerMap::kMissing;
				for (uint32_t i = 0; i < VirtualBaseCount(grouped) && !sensitive; ++i)
					sensitive = construction_sensitive_.find(VirtualBases(grouped)[i].base->base) !=
						PointerMap::kMissing;
				if (sensitive)
					construction_sensitive_.set(grouped, 1);
			}
			for (const sema::ClassEntity* grouped : analyzer_->grouped_classes())
			{
				if (definition_thunk_classes_.find(grouped) == PointerMap::kMissing)
				{
					definition_thunk_classes_.set(grouped, 1);
					index_definition_thunks(grouped);
				}
			}
		}
		if (definition_thunk_classes_.find(cls) == PointerMap::kMissing)
		{
			definition_thunk_classes_.set(cls, 1);
			index_definition_thunks(cls);
		}
	}
	for (uint32_t i = definition_thunk_heads_.find(function); i != PointerMap::kMissing;)
	{
		const DefinitionThunk entry = definition_thunks_[i];
		table_entry(function, entry.kind, entry.adjustment, binding, entry.introducer, entry.through,
			binding == ir_model::SBM_STRONG);
		i = entry.next;
	}
}

void ProgramLowering::note_definition_thunk(const sema::FunctionEntity* function, sema::SlotKind kind,
	ThisAdjustment adjustment, const sema::FunctionEntity* introducer, const sema::ClassEntity* through)
{
	// By function, each once. (GCC also defines a covariant entry's
	// static-`this` variant beside a virtual-`this` one, as the target its
	// thunk code jumps to; no table names it, so it is not emitted here.)
	ResultAdjustment result = ResultAdjustment();
	const bool covariant = !function->pure && !function->deleted &&
		covariant_entry(function, kind, introducer, through, result);
	if (function->pure || function->deleted || (adjustment.offset == 0 && adjustment.vcall == 0 && !covariant))
		return;
	push_definition_thunk(function, kind, through, adjustment, introducer);
}

void ProgramLowering::push_definition_thunk(const sema::FunctionEntity* function, sema::SlotKind kind,
	const sema::ClassEntity* through, ThisAdjustment adjustment, const sema::FunctionEntity* introducer)
{
	// At the head of the function's list (its definition emits the most
	// recent first); a thunk noted before moves there.
	const DefinitionKey key = DefinitionKey{ function, introducer, adjustment.offset, adjustment.vcall, kind, through };
	const uint32_t head = definition_thunk_heads_.find(function);
	uint32_t index = static_cast<uint32_t>(definition_thunks_.size());
	if (const uint32_t* known = definition_thunk_seen_.find(key))
	{
		index = *known;
		if (index == head)
			return;
		DefinitionThunk& old = definition_thunks_[index];
		definition_thunks_[old.previous].next = old.next;
		if (old.next != PointerMap::kMissing)
			definition_thunks_[old.next].previous = old.previous;
	}
	else
	{
		definition_thunk_seen_.insert(key, index);
		definition_thunks_.push_back(DefinitionThunk{ kind, through, adjustment, introducer, 0, 0 });
	}
	definition_thunks_[index].next = head;
	definition_thunks_[index].previous = static_cast<uint32_t>(PointerMap::kMissing);
	if (head != PointerMap::kMissing)
		definition_thunks_[head].previous = index;
	definition_thunk_heads_.set(function, index);
}

void ProgramLowering::index_construction_thunks(
	const sema::ClassEntity* complete, const sema::ClassEntity* cls, uint64_t offset, bool top)
{
	// vtt_entries' walk: each sub-VTT's tables in their construction
	// context (a table of a virtual base sharing a pointer in the base's
	// own layout but not there takes thunks no other table does), below
	// the classes whose contexts may have one (construction_sensitive_).
	if (construction_sensitive_.find(cls) == PointerMap::kMissing)
		return;
	if (!top)
	{
		const sema::VtableGroup* group = cls->virtuals->group;
		const uint32_t order = vtt_pointer_order(cls);
		const TableContext context = TableContext{ complete, cls, offset, true };
		for (uint32_t k = 0; k <= vtt_order_store_[order]; ++k)
		{
			const uint32_t segment = construction_owner(
				complete, cls, offset, k == 0 ? 0 : vtt_order_store_[order + k]);
			const sema::VtableSegment& part = group->segments[segment];
			const sema::VirtualSlot* own = analyzer_->virtual_slots(part.cls);
			for (uint32_t slot = 0; slot < part.cls->virtuals->vtable_size; ++slot)
			{
				const sema::VtableEntry& entry = group->entries[part.first_entry + slot];
				note_definition_thunk(entry.function, entry.kind, construction_adjustment(context, segment, slot),
					own[slot].adjusted, cls->virtuals->group->segments[segment].cls);
			}
		}
	}
	for (const sema::BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (!base->is_virtual && VirtualBaseCount(BaseClassOf(base)) != 0)
			index_construction_thunks(complete, BaseClassOf(base), offset + base->offset, false);
	}
	for (uint32_t i = 0; top && i < VirtualBaseCount(complete); ++i)
	{
		const sema::BaseClass* base = VirtualBases(complete)[i].base;
		if (VirtualBaseCount(BaseClassOf(base)) != 0)
			index_construction_thunks(complete, BaseClassOf(base), base->offset, false);
	}
}

void ProgramLowering::index_definition_thunks(const sema::ClassEntity* cls)
{
	// The entries of the class's group that take a thunk (moving `this`, or
	// adjusting a covariant result), and its construction tables'.
	// An entry whose overrider reaches the slot's class through a virtual
	// primary base on the table class's chain (from the overrider's class
	// on it, or from the table's subobject when the overrider lies above it)
	// also has that base's virtual thunk, which a derived class's table
	// placing the base elsewhere names (as GCC defines it with the function,
	// from the inheritance path).
	const auto note = [&](const sema::FunctionEntity* function, sema::SlotKind kind, ThisAdjustment adjustment,
						  const sema::FunctionEntity* introducer, const sema::ClassEntity* table, uint32_t slot) {
		note_definition_thunk(function, kind, adjustment, introducer, table);
		if (function->pure || function->deleted)
			return;
		const sema::ClassEntity* owner = sema::ClassOf(function->owner->entity->type);
		const sema::ClassEntity* level = nullptr;
		bool below = true;
		for (const sema::ClassEntity* link = table; link != nullptr && link->virtuals != nullptr && below;)
		{
			below = link != owner;
			const sema::BaseClass* primary = link->virtuals->primary_base;
			link = primary != nullptr ? BaseClassOf(primary) : nullptr;
		}
		for (const sema::ClassEntity* link = table;
			link != nullptr && link->virtuals != nullptr && slot < link->virtuals->vtable_size;)
		{
			below = below || link == owner;
			const sema::BaseClass* primary = link->virtuals->primary_base;
			if (primary == nullptr)
				break;
			const sema::ClassEntity* next = BaseClassOf(primary);
			if (below && primary->is_virtual && slot < next->virtuals->vtable_size)
				level = next;
			link = next;
		}
		if (level != nullptr)
			note_definition_thunk(
				function, kind, ThisAdjustment{ 0, itanium_vcall_offset(level, function), false }, introducer, table);
	};
	if (VirtualBaseCount(cls) != 0)
		index_construction_thunks(cls, cls, 0, true);
	const sema::VirtualSlot* slots = analyzer_->virtual_slots(cls);
	const sema::VtableGroup* group = cls->virtuals->group;
	for (uint32_t i = 0; i < cls->virtuals->vtable_size; ++i)
	{
		if (group != nullptr)
			note(group->entries[i].function, group->entries[i].kind,
				entry_adjustment(TableContext{ cls, cls, 0, false }, 0, i), slots[i].adjusted, cls, i);
		else
			note(slots[i].function, slots[i].kind, ThisAdjustment{ 0, 0, false }, slots[i].adjusted, cls, i);
	}
	if (group == nullptr)
		return;
	for (uint32_t segment = 1; segment < group->segment_count; ++segment)
	{
		const sema::VtableSegment& part = group->segments[segment];
		if (part.shared)
			continue;
		const sema::VirtualSlot* own = analyzer_->virtual_slots(part.cls);
		for (uint32_t slot = 0; slot < part.cls->virtuals->vtable_size; ++slot)
		{
			const sema::VtableEntry& entry = group->entries[part.first_entry + slot];
			note(entry.function, entry.kind, entry_adjustment(TableContext{ cls, cls, 0, false }, segment, slot),
				own[slot].adjusted, cls->virtuals->group->segments[segment].cls, slot);
		}
	}
}

const uint64_t* ProgramLowering::segment_offsets(const sema::ClassEntity* entity)
{
	// Each segment's subobject lies at its parent's offset plus its base's
	// (the class laid out first).
	const uint32_t found = offset_tables_.find(entity);
	if (found != PointerMap::kMissing)
		return segment_offsets_.data() + found;
	size_of(entity->type, 0);
	const sema::VtableGroup* group = entity->virtuals->group;
	const uint32_t first = static_cast<uint32_t>(segment_offsets_.size());
	segment_offsets_.push_back(0);
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		// A virtual base's offset is its base-specifier's (its parent is the
		// segment whose table pointer it shares, if any).
		const sema::VtableSegment& segment = group->segments[i];
		segment_offsets_.push_back(
			(segment.within == i ? 0 : segment_offsets_[first + segment.parent]) + segment.base->offset);
	}
	offset_tables_.set(entity, first);
	return segment_offsets_.data() + first;
}

void ProgramLowering::view_targets(
	const sema::ClassEntity* entity, uint32_t segment, uint8_t binding, std::vector<lowir::SymbolId>& targets)
{
	// A secondary table's slots: their final overriders' entries, through
	// thunks from the table's subobject (appended to `targets`).
	const sema::VtableGroup* group = entity->virtuals->group;
	const sema::VtableSegment& part = group->segments[segment];
	const sema::VirtualSlot* own = analyzer_->virtual_slots(part.cls);
	for (uint32_t slot = 0; slot < part.cls->virtuals->vtable_size; ++slot)
	{
		const sema::VtableEntry& entry = group->entries[part.first_entry + slot];
		targets.push_back(table_entry(entry.function, entry.kind,
			entry_adjustment(TableContext{ entity, entity, 0, false }, segment, slot), binding, own[slot].adjusted,
			part.cls));
	}
}

const lowir::SymbolId* ProgramLowering::view_items(
	const sema::ClassEntity* entity, uint32_t segment, const lowir::SymbolId* targets)
{
	// One secondary table's items: its rows, the offset to the top, the
	// type information, then its slots (from `targets`; returns the rest).
	const sema::VtableGroup* group = entity->virtuals->group;
	const uint64_t* offsets = segment_offsets(entity);
	const sema::VtableSegment& part = group->segments[segment];
	table_rows(TableContext{ entity, entity, 0, false }, segment);
	data_item(ScalarItem(lowir::TypeKind::I64, static_cast<uint64_t>(-static_cast<int64_t>(offsets[segment]))));
	data_item(AddressItem(typeinfo_symbol(entity), 0));
	for (uint32_t slot = 0; slot < part.cls->virtuals->vtable_size; ++slot)
		data_item(EntryItem(*targets++));
	return targets;
}

void ProgramLowering::group_targets(
	const sema::ClassEntity* entity, uint8_t binding, std::vector<lowir::SymbolId>& targets)
{
	// Object mode: the slots of the secondary tables with a table pointer
	// of their own, in subobject order (the external layout of
	// table_address; a segment sharing its parent's pointer has none).
	const sema::VtableGroup* group = entity->virtuals->group;
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		if (!group->segments[i].shared)
			view_targets(entity, i, binding, targets);
	}
}

void ProgramLowering::group_items(const sema::ClassEntity* entity, const lowir::SymbolId* targets)
{
	const sema::VtableGroup* group = entity->virtuals->group;
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		if (!group->segments[i].shared)
			targets = view_items(entity, i, targets);
	}
}

void ProgramLowering::define_views(const sema::ClassEntity* entity, uint8_t binding)
{
	// The course's secondary tables: a global per polymorphic subobject
	// off the primary chain with a table pointer of its own
	// (`<class>____view__<base>__<offset>__vtable`), unit-internal: its
	// rows, the offset to the top, the type information, then the
	// subobject's class's slots. A subobject sharing its parent's table
	// pointer (a secondary subobject's primary base, a virtual primary
	// base) has no table of its own (table_address gives its parent's).
	const sema::VtableGroup* group = entity->virtuals->group;
	const uint64_t* offsets = segment_offsets(entity);
	const uint32_t first_symbol = static_cast<uint32_t>(view_symbols_.size());
	view_symbols_.push_back(vtables_.find(entity));
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		const sema::VtableSegment& segment = group->segments[i];
		if (segment.shared)
		{
			view_symbols_.push_back(lowir::kNoId);
			continue;
		}
		// Each class_spelling may number a new class: one per statement, the
		// last first (see FunctionLowering::binary).
		const std::string base = class_spelling(segment.cls);
		view_symbols_.push_back(
			new_symbol(class_spelling(entity) + "____view__" + base + "__" + std::to_string(offsets[i]) + "__vtable"));
	}
	view_tables_.set(entity, first_symbol);
	std::vector<lowir::SymbolId> targets;
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		if (group->segments[i].shared)
			continue;
		targets.clear();
		view_targets(entity, i, binding, targets);
		const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
		view_items(entity, i, targets.data());
		lowir::SymbolMetadata metadata;
		metadata.binding = ir_model::SBM_INTERNAL;
		define_data(view_symbols_[first_symbol + i], first, metadata);
		++counters_.vtables;
	}
}

ProgramLowering::TableAddress ProgramLowering::table_address(const sema::ClassEntity* entity, uint32_t segment)
{
	// A segment sharing its parent's table pointer is its parent's table.
	const sema::VtableGroup* group = entity->virtuals->group;
	while (segment != 0 && group->segments[segment].shared)
		segment = group->segments[segment].parent;
	const lowir::SymbolId table = vtable_symbol(entity);
	// Each table's address point follows its rows (its vcall rows, its
	// class's virtual base offsets), its offset to the top and its type
	// information.
	const auto rows = [&](uint32_t i) -> uint64_t { return row_count(group, i); };
	if (segment == 0)
		return TableAddress{ table, static_cast<int64_t>(kTableHeaderBytes + 8 * rows(0)) };
	const uint32_t views = view_tables_.find(entity);
	if (views != PointerMap::kMissing)
		return TableAddress{ view_symbols_[views + segment],
			static_cast<int64_t>(kTableHeaderBytes + 8 * rows(segment)) };
	// The external group: the primary table, then each segment with a
	// table pointer of its own, in subobject order (Itanium 2.5.2); each
	// segment's address point computed once per class (0 for a segment
	// sharing its parent's).
	const uint32_t known = external_groups_.find(entity);
	if (known != PointerMap::kMissing)
		return TableAddress{ table, static_cast<int64_t>(external_points_[known + segment]) };
	const uint32_t first = static_cast<uint32_t>(external_points_.size());
	uint64_t position = (2 + rows(0) + static_cast<uint64_t>(entity->virtuals->vtable_size)) * 8;
	external_points_.push_back(kTableHeaderBytes + 8 * rows(0));
	for (uint32_t i = 1; i < group->segment_count; ++i)
	{
		const uint64_t own = group->segments[i].shared ? 0 : rows(i);
		external_points_.push_back(group->segments[i].shared ? 0 : position + 8 * own + kTableHeaderBytes);
		if (!group->segments[i].shared)
			position += (2 + own + static_cast<uint64_t>(group->segments[i].cls->virtuals->vtable_size)) * 8;
	}
	external_groups_.set(entity, first);
	return TableAddress{ table, static_cast<int64_t>(external_points_[first + segment]) };
}

lowir::SymbolId ProgramLowering::thunk_symbol(const sema::FunctionEntity* function, EntryVariant variant,
	ThisAdjustment adjustment, uint8_t binding, const ResultAdjustment* result, bool strong)
{
	// `_<entry>__vtable_return_adjust__this_<neg|pos><bytes>__return_
	// <neg|pos><bytes>[__vcall_<n|p><bytes>][__virtual_<n|p><bytes>]` (the
	// course's spelling: a virtual thunk's vcall offset, a covariant
	// result's virtual base offset), the ABI's `_ZTh`, `_ZTv` for a virtual
	// `this` adjustment (`vcall`), or `_ZTc` for a covariant thunk. A thunk
	// is defined with its function (Itanium 5.1.4: another unit's table or
	// construction table may name it): by the unit defining the function's
	// entry, after its definitions (emit_demanded), strong when the
	// function's strong definition asks for it (`strong`, see
	// definition_thunks), else weak; a unit whose tables name a thunk to a
	// function defined elsewhere only declares it. A virtual base's offset
	// is named by its Itanium position in the table of the class returned
	// (itanium_vbase_offset_offset).
	const bool internal = binding == ir_model::SBM_INTERNAL;
	const ThunkKey key = ThunkKey{ function, adjustment.offset, adjustment.vcall, variant, internal,
		result != nullptr ? result->from : nullptr, result != nullptr ? result->via : nullptr,
		result != nullptr ? result->offset : 0 };
	if (const uint32_t* known = thunk_symbols_.find(key))
	{
		Thunk& thunk = thunks_[*known];
		if (strong && !internal)
		{
			thunk.strong = true;
			if (program_.symbols[thunk.symbol].kind == lowir::SymbolKind::FunctionDefinition)
				program_.symbols[thunk.symbol].metadata.binding = ir_model::SBM_STRONG;
		}
		return thunk.symbol;
	}
	abi_mangle::AbiTarget target;
	target.kind = abi_mangle::ABI_TARGET_THUNK;
	target.node = function_encoding(function, variant);
	target.this_adjustment.offset = adjustment.offset;
	target.this_adjustment.is_virtual = adjustment.vcall != 0;
	target.this_adjustment.vcall_offset = adjustment.vcall;
	int64_t vbase_offset = 0;
	if (result != nullptr)
	{
		target.has_result_adjustment = true;
		target.result_adjustment.offset = result->offset;
		target.result_adjustment.is_virtual = result->via != nullptr;
		if (result->via != nullptr)
			target.result_adjustment.vcall_offset = vbase_offset = itanium_vbase_offset_offset(
				result->from, result->via);
	}
	std::string object;
	encoder_->encode(target, object);
	++counters_.mangled_names;
	const lowir::SymbolId entry = entry_symbol(function, variant);
	const auto magnitude = [](int64_t value) {
		return std::to_string(static_cast<uint64_t>(value < 0 ? -value : value));
	};
	const int64_t returned = result != nullptr ? result->offset : 0;
	std::string spelling = std::string("_") + program_.spelling(program_.symbols[entry].name) +
		"__vtable_return_adjust__this_" + (adjustment.offset < 0 ? "neg" : "pos") + magnitude(adjustment.offset) +
		"__return_" + (returned < 0 ? "neg" : "pos") + magnitude(returned);
	if (adjustment.vcall != 0)
		spelling += std::string("__vcall_") + (adjustment.vcall < 0 ? "n" : "p") + magnitude(adjustment.vcall);
	if (result != nullptr && result->via != nullptr)
		spelling += std::string("__virtual_") + (vbase_offset < 0 ? "n" : "p") + magnitude(vbase_offset);
	lowir::SymbolId symbol = lowir::kNoId;
	if (internal)
	{
		symbol = new_symbol(spelling);
		symbol_objects_[symbol] = local_object(object, false);
	}
	else
		symbol = external_symbol(object, spelling);
	const lowir::SignatureId signature = variant == EntryVariant::Deleting ? deleting_call_signature()
																		   : argument_signature(function);
	const uint32_t this_index = variant == EntryVariant::Complete && indirect_result(function) ? 1 : 0;
	// Object mode: a jump thunk (lowir::JumpThunk, no body: the object
	// compiler writes its adjustment and jump) leaves the arguments where
	// the caller put them: a variadic function's (a covariant one would have
	// to return through it, which GCC refuses too: `generic thunk code
	// fails`), and in host objects every thunk without a result adjustment,
	// as GCC and Clang emit them (no frame of its own). Course objects keep
	// the course's thunk bodies.
	const bool variadic = function->type->canonical->variadic;
	if (object_mode_ && variadic && result != nullptr)
		throw LoweringError(0, "a covariant thunk of a variadic function");
	const bool jump = object_mode_ && (variadic || (host_objects_ && result == nullptr));
	thunk_symbols_.insert(key, static_cast<uint32_t>(thunks_.size()));
	thunks_.push_back(Thunk{ symbol, entry, this_index, adjustment, result != nullptr,
		result != nullptr ? *result : ResultAdjustment(), jump, signature, internal, strong && !internal, function,
		variant });
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
	{
		// Another unit of the program declared or defined it.
		thunks_.back().target = lowir::kNoId;
		return symbol;
	}
	PendingDeclaration pending = PendingDeclaration();
	pending.symbol = symbol;
	pending.function = true;
	pending.signature = signature;
	pending.metadata.binding = internal ? ir_model::SBM_INTERNAL : ir_model::SBM_STRONG;
	if (symbol_objects_[symbol] != 0)
		pending.metadata.object = symbol_objects_[symbol];
	set_pending(symbol, pending);
	return symbol;
}

size_t ProgramLowering::ThunkHash::operator()(const ThunkKey& key) const
{
	return (reinterpret_cast<uintptr_t>(key.function) * 0x9E3779B97F4A7C15ull) ^ static_cast<uint64_t>(key.adjustment) ^
		static_cast<uint64_t>(key.vcall) << 40 ^
		(static_cast<uint64_t>(key.variant) << 1 | (key.internal ? 1 : 0)) << 56 ^
		reinterpret_cast<uintptr_t>(key.result_via) * 31 ^ static_cast<uint64_t>(key.result_offset) << 20;
}

void ProgramLowering::emit_thunk(const Thunk& thunk)
{
	// Defined with its function's entry, else left to its declaration.
	if (thunk.target == lowir::kNoId || program_.symbols[thunk.target].kind != lowir::SymbolKind::FunctionDefinition ||
		program_.symbols[thunk.symbol].kind != lowir::SymbolKind::Unresolved)
		return;
	program_.declare_function(thunk.symbol, lowir::SymbolKind::FunctionDefinition, thunk.signature);
	lowir::SymbolMetadata& metadata = program_.symbols[thunk.symbol].metadata;
	metadata.binding = thunk.internal ? ir_model::SBM_INTERNAL
		: thunk.strong                ? ir_model::SBM_STRONG
									  : ir_model::SBM_WEAK;
	if (symbol_objects_[thunk.symbol] != 0)
		metadata.object = symbol_objects_[thunk.symbol];
	note_thunk_family(thunk);
	if (thunk.jump)
	{
		program_.jump_thunks.push_back(lowir::JumpThunk{
			thunk.symbol, thunk.target, thunk.this_index, thunk.adjustment.offset, thunk.adjustment.vcall });
		if (stream_ != nullptr && stream_->reads_jump_thunks())
		{
			stream_->append(program_, thunk.symbol);
			++counters_.functions;
			return;
		}
	}
	{
		FunctionLowering body(*this, thunk.symbol, nullptr);
		body.lower_thunk(thunk.symbol, thunk.target, thunk.this_index, thunk.adjustment.offset, thunk.adjustment.vcall,
			thunk.has_result ? &thunk.result : nullptr);
	}
	release_definition(thunk.symbol);
	++counters_.functions;
}

lowir::SymbolId ProgramLowering::typeinfo_symbol(const sema::ClassEntity* entity, bool derived)
{
	// A polymorphic class's type information goes with its table (Itanium
	// 2.9.5), or with a derived class's type information referring to it
	// (the course's), but not an explicitly instantiated class's that its
	// instantiation's unit defines; any other class's with each unit
	// referring to it.
	const uint32_t found = typeinfos_.find(entity);
	lowir::SymbolId symbol = found;
	if (found == PointerMap::kMissing)
	{
		symbol = class_global(entity, abi_mangle::ABI_TARGET_TYPEINFO, "__rtti_", local_type_information(entity));
		typeinfos_.set(entity, symbol);
	}
	if (program_.symbols[symbol].kind == lowir::SymbolKind::Unresolved &&
		(!entity->polymorphic || (derived && !instantiated_elsewhere(entity)) || emits_vtable(entity)))
		define_typeinfo(entity, symbol);
	return symbol;
}

lowir::SymbolId ProgramLowering::typeinfo_name(const sema::ClassEntity* entity)
{
	// Itanium 2.9.5p2: the type's mangled name (without _Z), null
	// terminated; in object mode `*` first for a class its unit alone
	// names (see private_type_name).
	const uint32_t found = typeinfo_names_.find(entity);
	if (found != PointerMap::kMissing)
		return found;
	const lowir::SymbolId symbol = class_global(entity, abi_mangle::ABI_TARGET_TYPEINFO_NAME, "__typeinfo_name_");
	typeinfo_names_.set(entity, symbol);
	if (program_.symbols[symbol].kind != lowir::SymbolKind::Unresolved)
		return symbol;
	const std::string name = (object_mode_ && private_entity_name(entity) ? "*" : "") +
		class_object(entity, abi_mangle::ABI_TARGET_TYPE);
	const uint32_t first = static_cast<uint32_t>(program_.data_items.size());
	for (const char c : name)
		data_item(ScalarItem(lowir::TypeKind::I8, static_cast<uint8_t>(c)));
	data_item(ScalarItem(lowir::TypeKind::I8, 0));
	lowir::SymbolMetadata metadata;
	metadata.binding = entity_locality(entity) != Locality::External ? ir_model::SBM_INTERNAL : ir_model::SBM_WEAK;
	define_data(symbol, first, metadata);
	return symbol;
}

void ProgramLowering::define_typeinfo(const sema::ClassEntity* entity, lowir::SymbolId symbol)
{
	// Itanium 2.9.5: the runtime class's vtable (past its offset and type
	// information) and the name; a single public non-virtual base at
	// offset zero by address (__si_class_type_info); other bases with
	// the hierarchy's flags, their count, then each base's type
	// information and its offset shifted past the public (2) and virtual
	// (1) flags. A complete class's bases' offsets are the layout's (made
	// first: a typeid may name the class before anything lays it out); an
	// incomplete one is described without bases.
	if (entity->complete)
		size_of(entity->type, 0);
	const bool single = SingleInheritance(entity);
	const lowir::SymbolId kind = rtti_vtable(entity->bases == nullptr ? TypeInfoKind::Class
			: single                                                  ? TypeInfoKind::SingleInheritance
																	  : TypeInfoKind::MultipleInheritance);
	const lowir::SymbolId name = typeinfo_name(entity);
	std::vector<lowir::SymbolId> bases;
	for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next)
		bases.push_back(typeinfo_symbol(BaseClassOf(base), true));
	const uint32_t first = data_item(AddressItem(kind, 16));
	data_item(AddressItem(name, 0));
	if (single)
		data_item(AddressItem(bases[0], 0));
	else if (entity->bases != nullptr)
	{
		data_item(ScalarItem(lowir::TypeKind::I32, inheritance_flags(entity)));
		data_item(ScalarItem(lowir::TypeKind::I32, bases.size()));
		size_t index = 0;
		for (const sema::BaseClass* base = entity->bases; base != nullptr; base = base->next, ++index)
		{
			data_item(AddressItem(bases[index], 0));
			// A virtual base's offset is its row's position in the table
			// (Itanium 2.9.5p6c).
			const uint64_t flags = (base->access == sema::Access::Public ? 2 : 0) | (base->is_virtual ? 1 : 0);
			const uint64_t offset = base->is_virtual
				? static_cast<uint64_t>(vbase_row_offset(entity, BaseClassOf(base)))
				: base->offset;
			data_item(ScalarItem(lowir::TypeKind::I64, (offset << 8) | flags));
		}
	}
	lowir::SymbolMetadata metadata;
	metadata.binding = local_type_information(entity) || entity_locality(entity) != Locality::External
		? ir_model::SBM_INTERNAL
		: ir_model::SBM_WEAK;
	define_data(symbol, first, metadata);
}

bool ProgramLowering::local_type_information(const sema::ClassEntity* entity)
{
	return host_objects_ && !entity->complete;
}

bool ProgramLowering::local_type_information(const sema::Type* type)
{
	// A pointer or pointer to member whose pointee (or member pointer's
	// class), through further pointers, is an incomplete class.
	if (!host_objects_)
		return false;
	for (type = type->canonical->unqualified;; type = type->element->unqualified)
	{
		if (type->kind == sema::TypeKind::MemberPointer)
		{
			const sema::ClassEntity* owner = sema::ClassOf(type->owner);
			if (owner != nullptr && !owner->complete)
				return true;
		}
		else if (type->kind == sema::TypeKind::Class)
			return !static_cast<const sema::ClassEntity*>(type->entity)->complete;
		else if (type->kind != sema::TypeKind::Pointer)
			return false;
	}
}

lowir::SymbolId ProgramLowering::pure_virtual_symbol(const sema::FunctionEntity* function)
{
	// The runtime's __cxa_pure_virtual fills a pure function's slots; its
	// declaration takes the boundary of the first slot it fills (the
	// course's form).
	if (pure_virtual_ != lowir::kNoId)
		return pure_virtual_;
	pure_virtual_ = new_symbol("__cxa_pure_virtual");
	const lowir::SignatureId signature = argument_signature(function);
	program_.signatures[signature].boundary.unwind = ir_model::CUM_NO;
	program_.signatures[signature].boundary.returns = ir_model::CRM_NORETURN;
	program_.declare_function(pure_virtual_, lowir::SymbolKind::FunctionDeclaration, signature);
	lowir::SymbolMetadata& metadata = program_.symbols[pure_virtual_].metadata;
	metadata.role = ir_model::SR_PURE_VIRTUAL;
	metadata.binding = ir_model::SBM_STRONG;
	++counters_.declarations;
	return pure_virtual_;
}

lowir::SignatureId ProgramLowering::virtual_call_signature(const sema::FunctionEntity* function)
{
	const uint32_t known = virtual_signatures_.find(function);
	if (known != PointerMap::kMissing)
		return known;
	const lowir::SignatureId signature = argument_signature(function);
	virtual_signatures_.set(function, signature);
	return signature;
}

lowir::SignatureId ProgramLowering::argument_signature(const sema::FunctionEntity* function)
{
	// A new copy of the function's boundary, its parameters named argN as
	// an indirect call's are.
	const lowir::SignatureId signature = function_signature(function, false);
	const lowir::Signature& boundary = program_.signatures[signature];
	for (uint32_t i = 0; i < boundary.parameter_count; ++i)
		program_.parameters[boundary.first_parameter + i].name = argument_name(i);
	return signature;
}

lowir::SignatureId ProgramLowering::deleting_call_signature()
{
	if (deleting_signature_ != lowir::kNoId)
		return deleting_signature_;
	lowir::Signature signature = lowir::Signature();
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	lowir::Parameter object = lowir::Parameter();
	object.name = argument_name(0);
	object.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
	program_.parameters.push_back(object);
	signature.parameter_count = 1;
	signature.result = lowir::Type::scalar(lowir::TypeKind::Void);
	deleting_signature_ = program_.add_signature(signature);
	return deleting_signature_;
}

uint32_t ProgramLowering::data_item(const lowir::DataItem& item)
{
	program_.data_items.push_back(item);
	return static_cast<uint32_t>(program_.data_items.size() - 1);
}

void ProgramLowering::define_data(lowir::SymbolId symbol, uint32_t first, lowir::SymbolMetadata metadata)
{
	// The items appended since `first`, read-only.
	lowir::Global global = lowir::Global();
	global.typed = false;
	global.init = lowir::GlobalInit::Data;
	global.first_item = first;
	global.item_count = static_cast<uint32_t>(program_.data_items.size() - first);
	program_.declare_global(symbol, lowir::SymbolKind::GlobalDefinition, global);
	metadata.storage = ir_model::GSM_READONLY;
	if (symbol_objects_[symbol] != 0)
		metadata.object = symbol_objects_[symbol];
	program_.symbols[symbol].metadata = metadata;
	++counters_.globals;
}

} // namespace lower
