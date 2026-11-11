#include "lower/function_lowering.h"

#include "sema/conversions.h"

// Polymorphic objects in bodies (10.3, 12.4, 12.7): a polymorphic class's
// constructors and destructors point the object's virtual table pointer
// (offset 0 of every polymorphic subobject, Itanium 2.4) at the class's
// table, past its offset to the top and type information (Itanium 2.5.2);
// a virtual call loads the final overrider's address from the object's
// table at the slot the analysis assigned; a delete-expression of an
// object with a virtual destructor calls its dynamic type's deleting
// entry, which destroys and deallocates it.
namespace lower {

void FunctionLowering::store_vptr(uint32_t location)
{
	// Each table pointer of the object: the primary one, then (a class
	// with a table group) each other subobject's with a pointer of its own
	// (from a VTT, each virtual base's); a subobject within a virtual base
	// found through that base. A base-object entry of a class with virtual
	// bases takes the tables of the construction under way from its VTT
	// (VTT[0] once per block, the rows from the table it names: see
	// virtual_base_address).
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	if (!entity->dynamic)
		return;
	lowering_.vtable_symbol(entity);
	const sema::VtableGroup* group = entity->virtuals->group;
	const uint64_t* offsets = group != nullptr ? lowering_.segment_offsets(entity) : nullptr;
	const uint32_t count = group != nullptr ? group->segment_count : 1;
	const bool from_vtt = base_entry_ && VirtualBaseCount(entity) != 0;
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = AbiWordType(program_.target);
	const lowir::Type pointer = program_.pointer_type();
	// A segment's subobject: within a virtual base, at its offset in that
	// base found through the base's row (in a base-object entry the
	// construction table's, the base itself at its own offset), else at its
	// offset from `this`.
	const auto subobject = [&](uint32_t i) {
		Place object = Place();
		const uint32_t within = i != 0 ? group->segments[i].within : 0;
		if (within != 0)
		{
			const lowir::Value address = virtual_base_address(
				this_pointer(location), entity, group->segments[within].cls);
			object.storage = from_vtt && offsets[i] == offsets[within]
				? address
				: index(i8, ir_model::IPK_NONE, address, integer(i64, offsets[i] - offsets[within])).value;
		}
		else
		{
			object.storage = this_pointer(location);
			if (i != 0)
				object.storage = index(i8, ir_model::IPK_NONE, object.storage, integer(i64, offsets[i])).value;
		}
		return object;
	};
	// The primary table pointer (in a base-object entry VTT[0]), then the
	// group's other pointers but a base-object entry's secondary virtual
	// pointers, which follow from its VTT in its order (Itanium 2.6.2:
	// each where the complete object placed its subobject, a virtual
	// primary base lying elsewhere included, as GCC stores them).
	const uint32_t order = from_vtt && group != nullptr ? lowering_.vtt_pointer_order(entity) : 0;
	const uint32_t secondary = from_vtt && group != nullptr ? lowering_.vtt_order_store_[order] : 0;
	// (`this` first, then the table: an order of evaluation of its own.)
	const Place primary = subobject(0);
	if (from_vtt)
	{
		const RValue table = construction_table();
		store(pointer, table, primary);
	}
	else
		store_table(primary, entity, 0);
	for (uint32_t i = 1; i < count; ++i)
	{
		if (group->segments[i].shared)
			continue;
		bool listed = false;
		for (uint32_t k = 0; k < secondary && !listed; ++k)
			listed = lowering_.vtt_order_store_[order + 1 + k] == i;
		if (!listed)
			store_table(subobject(i), entity, i);
	}
	for (uint32_t k = 0; k < secondary; ++k)
	{
		const uint32_t i = lowering_.vtt_order_store_[order + 1 + k];
		const Place object = subobject(i);
		const uint32_t slot = lowering_.vtt_segment_index(entity, i);
		Place entry = Place();
		entry.storage = index(i8, ir_model::IPK_NONE, vtt_param_,
			integer(i64, target_model::cxx_abi(program_.target).word_bytes * static_cast<uint64_t>(slot)))
							.value;
		store(pointer, load(pointer, entry), object);
	}
}

void FunctionLowering::store_table(const Place& object, const sema::ClassEntity* entity, uint32_t segment)
{
	const lowir::Type pointer = program_.pointer_type();
	const ProgramLowering::TableAddress table_address = lowering_.table_address(entity, segment);
	Place table = Place();
	table.storage = lowir::Value::symbol(table_address.symbol);
	RValue address;
	address.value = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, address_of(table),
		integer(AbiWordType(program_.target), static_cast<uint64_t>(table_address.addend)))
						.value;
	address.type = pointer;
	store(pointer, address, object);
}

lowir::Value FunctionLowering::virtual_callee(lowir::Value object, uint32_t slot)
{
	const lowir::Type pointer = program_.pointer_type();
	Place vptr = Place();
	vptr.storage = object;
	Place entry = Place();
	entry.storage = load(pointer, vptr).value;
	if (slot != 0)
		entry.storage = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, entry.storage,
			integer(AbiWordType(program_.target),
				static_cast<uint64_t>(slot) * target_model::cxx_abi(program_.target).word_bytes))
							.value;
	return load(pointer, entry).value;
}

lowir::Value FunctionLowering::member_callee(
	const sema::FunctionEntity* function, lowir::Value object, bool dispatch, lowir::SignatureId& signature)
{
	// 5.2.2p1: a virtual function not named by a qualified-id is its final
	// overrider, loaded from the object's table with the function's own
	// boundary. Only table entries or direct calls demand a definition.
	if (!dispatch)
	{
		signature = lowir::kNoId;
		const sema::FunctionEntity* pattern = sema::PatternOf(function);
		if (function->always_inline || (function->primary != nullptr && function->primary->always_inline) ||
			(pattern != nullptr && pattern->always_inline))
			return lowir::Value::symbol(lowering_.inline_copy(function));
		lowering_.demand(function);
		return lowir::Value::symbol(lowering_.function_symbol(function));
	}
	signature = lowering_.virtual_call_signature(function);
	const lowir::Value callee = virtual_callee(object, function->vtable_slot);
	if (lowering_.optimizing() && !function->has_overrider && !function->pure && function->defined &&
		function->body != nullptr && !function->destructor && !function->always_inline && !function->unanalyzed_body &&
		!function->excluded_instantiation && !function->type->canonical->variadic)
	{
		lowering_.demand(function);
		speculated_callee_ = callee;
		speculated_target_ = lowir::Value::symbol(lowering_.function_symbol(function));
	}
	return callee;
}

lowir::TempId FunctionLowering::speculative_call(
	lowir::Type result, lowir::Value callee, lowir::Value target, size_t base, lowir::SignatureId signature)
{
	const lowir::Type pointer = program_.pointer_type();
	const bool value = result.kind() != lowir::TypeKind::Void;
	Place merged = Place();
	if (value)
		merged.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Call, result));
	const lowir::BlockId direct = new_block(lowir::BlockKind::Plain);
	const lowir::BlockId other = new_block(lowir::BlockKind::Plain);
	const lowir::BlockId done = new_block(lowir::BlockKind::Plain);
	RValue have, want;
	have.value = callee;
	have.type = want.type = pointer;
	want.value = target;
	// (The call's region stays open across the branch: it guards both calls.)
	const auto finish = [this](const lowir::Instruction& inst) {
		emit(inst);
		terminated_ = true;
	};
	lowir::Instruction test = lowir::make_instruction(
		lowir::Opcode::Branch, lowir::Type::scalar(lowir::TypeKind::Void));
	test.ops[0] = compare(lowir::ComparePredicate::Eq, pointer, have, want).value;
	test.ops[1] = lowir::Value::block(direct);
	test.ops[2] = lowir::Value::block(other);
	finish(test);
	const uint32_t count = static_cast<uint32_t>(arguments_.size() - base);
	for (unsigned arm = 0; arm < 2; ++arm)
	{
		place(arm == 0 ? direct : other);
		const lowir::TempId temp = builder_.call(result, arm == 0 ? target : callee, arguments_.data() + base, count,
			arm == 0 ? lowir::kNoId : signature, 0, 0);
		if (value)
		{
			RValue returned;
			returned.value = lowir::Value::temp(temp);
			returned.type = result;
			store(result, returned, merged);
		}
		lowir::Instruction leave = lowir::make_instruction(
			lowir::Opcode::Jump, lowir::Type::scalar(lowir::TypeKind::Void));
		leave.ops[0] = lowir::Value::block(done);
		finish(leave);
	}
	place(done);
	return value ? load(result, merged).value.index : lowir::kNoId;
}

bool FunctionLowering::deleting_inlines_body(uint32_t location)
{
	// The course's deleting entry is the complete destructor's own code
	// when that stores the table and destroys non-virtual bases only (an
	// empty body, no member to destroy); otherwise it calls the complete
	// entry.
	const syntax::FunctionBody* body = function_->body;
	if (body != nullptr && !function_->defaulted && (body->try_block || !body->body->items.empty()))
		return false;
	if (VirtualBaseCount(sema::ClassOf(function_->owner->entity->type)) != 0)
		return false;
	destructor_regions(location);
	for (const Subobject& subobject : subobjects_)
	{
		if (subobject.base == nullptr)
			return false;
	}
	return true;
}

void FunctionLowering::call_complete_destructor(uint32_t location)
{
	// The complete entry the class's table demands with the deleting one
	// (no base entry of its own for that).
	const size_t base = arguments_.size();
	arguments_.push_back(this_pointer(location));
	lowering_.demand_vtable_entries(function_);
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void),
		lowir::Value::symbol(lowering_.entry_symbol(function_, EntryVariant::Complete)), base, lowir::kNoId, false);
}

void FunctionLowering::deallocate_object(uint32_t location)
{
	// 12.4p12, 12.5p4: the deallocation function found where the virtual
	// destructor is defined, a sized one given the object's size.
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	RValue size = integer(AbiWordType(program_.target), lowering_.size_of(entity->type, location));
	free_storage(entity->virtuals->deleting_deallocator, this_pointer(location), size, location, 0,
		lowering_.alignment_of(entity->type, location));
}

void FunctionLowering::virtual_deletion(const sema::ClassEntity* entity, const RValue& pointer)
{
	// The deleting entry follows the complete one in the table (which the
	// course emits where such a delete-expression is).
	// A throwing destructor leaves by the enclosing objects' region (15.2p1).
	const sema::FunctionEntity* destructor = entity->destructor;
	const bool throws = call_may_throw(destructor);
	lowering_.vtable_symbol(entity);
	begin_call(throws);
	const size_t base = arguments_.size();
	arguments_.push_back(pointer.value);
	const lowir::Value callee = virtual_callee(pointer.value, destructor->vtable_slot + 1);
	lowir::SignatureId signature = lowering_.virtual_call_signature(destructor);
	if (program_.signatures[signature].boundary.this_return)
	{
		lowir::Signature deleting = program_.signatures[signature];
		deleting.boundary.this_return = false;
		signature = program_.add_signature(deleting);
	}
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, signature, throws);
	end_call(throws);
}

} // namespace lower
