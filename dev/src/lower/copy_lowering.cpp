#include "lower/function_lowering.h"

#include "sema/conversions.h"

// The synthesized bodies of defaulted copy and move constructors and
// assignment operators (12.8p15, p28): each base and member copied, moved
// or assigned from the source object's by the function the analysis
// selected for it (its plan), in declaration order. The course's forms
// (the README's value-semantics conventions): a leading run of subobjects
// whose selected functions are trivial is one storage copy up to the first
// that is not; a class whose subobjects are all trivial is copied whole
// unless it has bases or bit-fields; otherwise field by field (empty
// subobjects skipped), adjacent bit-fields of one allocation unit
// transferred once.
namespace lower {

namespace {

bool IsAssignment(sema::CopyKind kind)
{
	return kind == sema::CopyKind::CopyAssignment || kind == sema::CopyKind::MoveAssignment;
}

// The class of a member of class type or array of it, else null.
const sema::ClassEntity* MemberClass(const sema::Type* type)
{
	if (type->is_reference())
		return nullptr;
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	return sema::ClassOf(type);
}

// A member of an empty class type (not an array of one): an assignment
// ends its leading storage copy there and names the member's assignment
// operator without calling it (the course's form, as the reference
// presents it).
bool EmptyClassMember(const sema::SubobjectInit& init)
{
	if (init.member == nullptr)
		return false;
	const sema::Type* type = init.member->variable->type;
	const sema::ClassEntity* cls = type->is_reference() || type->kind == sema::TypeKind::Array ? nullptr
																							   : sema::ClassOf(type);
	return cls != nullptr && cls->empty;
}

} // namespace

bool FunctionLowering::trivial_subobject(const sema::SubobjectInit& init)
{
	// Copied as storage: a scalar member, or a subobject whose selected
	// function is trivial (a reference member is rebound by itself, the
	// course's form).
	const sema::Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
	if (type->is_reference())
		return false;
	if (MemberClass(type) == nullptr)
		return true;
	return init.constructor == nullptr || init.constructor->trivial;
}

uint64_t FunctionLowering::subobject_offset(const sema::SubobjectInit& init, uint32_t location)
{
	if (init.base != nullptr)
		return init.base->offset;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(init.member->variable);
	return init.member->bit_field ? lowering_.member_bit_offset(variable, location) / 8
								  : lowering_.member_offset(variable, location);
}

lowir::Value FunctionLowering::source_object()
{
	// The source object's address: the parameter's slot read again.
	Place slot = Place();
	slot.storage = lowir::Value::slot(parameter_slots_[0]);
	return load(lowir::Type::scalar(lowir::TypeKind::Ptr), slot).value;
}

lowir::Value FunctionLowering::subobject_of(lowir::Value object, const sema::SubobjectInit& init, uint32_t location)
{
	// A base at its offset (no projection), a member at its field offset.
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	if (init.base != nullptr)
		return index(i8, ir_model::IPK_NONE, object, integer(i64, init.base->offset)).value;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(init.member->variable);
	const lowir::Value address =
		index(i8, ir_model::IPK_FIELD, object, integer(i64, lowering_.member_offset(variable, location))).value;
	if (union_member(variable))
		mark_untyped(address);
	return address;
}

void FunctionLowering::copy_members(uint32_t location)
{
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	const bool assignment = IsAssignment(function_->copy_kind);
	const uint32_t count = function_->init_count;
	uint32_t first = 0;
	while (first < count && trivial_subobject(function_->inits[first]) &&
		!(assignment && EmptyClassMember(function_->inits[first])))
		++first;
	// A dynamic object's table pointer is its own class's, never the
	// source's (whose dynamic type may be derived): members one by one.
	if (entity->dynamic)
		first = 0;
	bool field_wise = entity->bases != nullptr || entity->dynamic;
	for (const sema::DataMember* member = entity->members; member != nullptr && !field_wise; member = member->next)
		field_wise = member->bit_field && member->variable->name != nullptr;
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	bit_limit_ = subobject_bit_limit();
	// An assignment never stores the table pointer (the object keeps its
	// dynamic type).
	const bool table_stored = assignment;
	// 15.2p2: a constructor's copied subobjects are destroyed, last first,
	// when a later one's copy throws: each destructible one before the
	// last copy that may throw opens a cleanup region (the mem-initializer
	// form, see initialize_subobjects).
	const auto copy_throws = [this](const sema::SubobjectInit& init) {
		// A class (or class array) subobject's copy by a constructor that is
		// not trivial, which may throw or whose default arguments may.
		const sema::FunctionEntity* constructor = init.constructor;
		const sema::Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
		if (MemberClass(type) == nullptr || constructor == nullptr || constructor->trivial)
			return false;
		bool throws = call_may_throw(constructor);
		const sema::Type* signature = constructor->type->canonical;
		for (uint32_t k = 1; k < signature->param_count && !throws && constructor->default_arguments != nullptr; ++k)
			throws = constructor->default_arguments[k] != nullptr &&
				may_throw(lowering_.default_argument(constructor, k));
		return throws;
	};
	uint32_t guarded = 0;
	for (uint32_t i = count; i-- > 0 && !assignment;)
	{
		if (copy_throws(function_->inits[i]))
		{
			guarded = i;
			break;
		}
	}
	// A synthesized constructor's copied subobjects are destroyed through
	// plain regions' chains (the course's form; see construction_made).
	construction_chain_ = !assignment;
	construction_actions_.clear();
	construction_resume_ = lowir::kNoId;
	const auto copied = [this, entity, location](const sema::SubobjectInit& init) {
		// A union's members are variant members (12.4p8), an anonymous
		// union's too; a closure's unnamed members (5.1.2p14) are not.
		const sema::Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
		const sema::ClassEntity* unnamed = init.base == nullptr && init.member->variable->name == nullptr
			? MemberClass(type)
			: nullptr;
		if (entity->key == TokenKind::KW_UNION || (unnamed != nullptr && unnamed->key == TokenKind::KW_UNION) ||
			!needs_destruction(type))
			return;
		if (!construction_chain_)
		{
			subobject_constructed(Subobject{ init.base, init.base != nullptr ? nullptr : init.member, -1 }, location);
			return;
		}
		construction_made(type,
			copied_address_.kind != lowir::ValueKind::None ? copied_address_
														   : subobject_of(this_pointer(location), init, location),
			init.base);
		copied_address_ = lowir::Value::none();
	};
	if (first == count && !field_wise)
	{
		// All trivial: the whole object's storage (an empty class has
		// none). An assignment or a base entry copies only the class's
		// data: `*this` may be a base subobject whose tail padding holds
		// members or virtual bases of the derived object (3.9p2 excludes
		// base subobjects from byte copies; 12.8p28 assigns the subobjects
		// only; see subobject_bit_limit).
		if (!entity->empty)
		{
			const lowir::Value destination = this_pointer(location);
			if (bit_limit_ < entity->size * 8)
				copy_span(source_object(), destination, entity->nvsize, lowering_.alignment_of(entity->type, location));
			else
				copy_object(entity->type, source_object(), destination, location);
		}
	}
	else
	{
		uint64_t prefix = first < count && !entity->dynamic ? subobject_offset(function_->inits[first], location) : 0;
		if (first == count)
			first = 0;
		// 12.8p15: the complete object's constructor copies the virtual
		// bases first (an assignment through its direct bases, 12.8p28).
		// Each is destroyed when a copy after it may throw (a later virtual
		// base's or any other subobject's), not only one after it in the
		// plan's order.
		if (!assignment && !base_entry_)
		{
			const std::vector<uint32_t>& planned = virtual_base_inits(entity);
			std::vector<uint32_t>& order = virtual_copies_;
			order.clear();
			for (const sema::BaseClass* shared : virtual_base_order(entity))
			{
				const uint32_t i = planned[lowering_.virtual_base_position(entity, shared->base)];
				if (i != PointerMap::kMissing)
					order.push_back(i);
			}
			bool later = false;
			for (uint32_t i = 0; i < count && !later; ++i)
				later = !(function_->inits[i].base != nullptr && function_->inits[i].base->is_virtual) &&
					copy_throws(function_->inits[i]);
			std::vector<uint8_t>& tracked = virtual_tracked_;
			tracked.assign(order.size(), 0);
			for (size_t k = order.size(); k-- > 0;)
			{
				tracked[k] = later || order[k] < guarded;
				later = later || copy_throws(function_->inits[order[k]]);
			}
			for (size_t k = 0; k < order.size(); ++k)
			{
				copied_address_ = lowir::Value::none();
				copy_subobject(function_->inits[order[k]], false, location);
				if (tracked[k])
					copied(function_->inits[order[k]]);
			}
		}
		if (prefix != 0)
		{
			// The leading trivial subobjects' storage, in the class's
			// alignment.
			const lowir::Value destination = this_pointer(location);
			const lowir::Value source = source_object();
			copy_span(source, destination, prefix, lowering_.alignment_of(entity->type, location));
			copied_address_ = lowir::Value::none();
			for (uint32_t i = 0; i < first && i < guarded; ++i)
				copied(function_->inits[i]);
		}
		else
			first = 0;
		for (uint32_t i = first; i < count; ++i)
		{
			const sema::SubobjectInit& init = function_->inits[i];
			if (!assignment && init.base != nullptr && init.base->is_virtual)
				continue;
			if (init.member != nullptr && init.member->bit_field)
			{
				if (init.member->variable->name != nullptr)
					i = copy_bit_fields(i, location);
				continue;
			}
			copied_address_ = lowir::Value::none();
			copy_subobject(init, assignment, location);
			if (i < guarded)
				copied(init);
		}
	}
	construction_chain_ = false;
	construction_actions_.clear();
	// The tables after the members (the course's form: no member's copy
	// can observe them).
	if (!table_stored)
		store_vptr(location);
	bit_limit_ = UINT64_MAX;
	if (assignment)
	{
		// 12.8p28: `*this`.
		lowir::Instruction inst = lowir::make_instruction(lowir::Opcode::Return, pointer);
		inst.ops[0] = this_pointer(location);
		terminate(inst);
	}
}

void FunctionLowering::copy_span(lowir::Value source, lowir::Value destination, uint64_t size, uint64_t alignment)
{
	lowir::Instruction inst = lowir::make_instruction(
		lowir::Opcode::CopyObject, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = source;
	inst.ops[1] = destination;
	inst.immediate = static_cast<int64_t>(size);
	unsigned log2 = 0;
	while ((uint64_t(1) << log2) < alignment)
		++log2;
	inst.sub = static_cast<uint8_t>(log2);
	emit(inst);
}

void FunctionLowering::copy_subobject(const sema::SubobjectInit& init, bool assignment, uint32_t location)
{
	// One base or member: a class subobject by its selected function (a
	// constructor's trivial one as storage), an array element by element
	// (a scalar array as storage), a scalar or reference by its value.
	const sema::Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
	const sema::ClassEntity* cls = MemberClass(type);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const OverlapScope overlap(overlapping_empty_, init.base != nullptr ? nullptr : init.member);
	if (cls == nullptr && type->kind != sema::TypeKind::Array)
	{
		const bool reference = type->is_reference();
		const lowir::Type storage = reference ? pointer : lowering_.storage_type(type);
		Place target = Place();
		Place from = Place();
		RValue value;
		if (assignment || reference)
		{
			// The value first (5.17p1; a reference's binding, 8.5.3).
			from.storage = subobject_of(source_object(), init, location);
			value = load(storage, from);
			target.storage = subobject_of(this_pointer(location), init, location);
		}
		else
		{
			target.storage = subobject_of(this_pointer(location), init, location);
			from.storage = subobject_of(source_object(), init, location);
			value = load(storage, from);
		}
		store(storage, value, target);
		return;
	}
	if (cls == nullptr || (type->kind == sema::TypeKind::Array && !assignment && trivial_subobject(init)))
	{
		// A scalar array, or a constructor's array of trivially copied
		// elements: its storage (an assignment reads the source first).
		// (Its address taken again once copied, see copied.)
		lowir::Value destination, source;
		if (assignment)
		{
			source = subobject_of(source_object(), init, location);
			destination = subobject_of(this_pointer(location), init, location);
		}
		else
		{
			destination = subobject_of(this_pointer(location), init, location);
			source = subobject_of(source_object(), init, location);
		}
		copy_object(type, source, destination, location);
		return;
	}
	if (type->kind == sema::TypeKind::Array)
	{
		copy_elements(init, type, location);
		return;
	}
	// An empty subobject has no storage to copy (a trivial constructor or
	// assignment operator is not called for it; an empty member's
	// assignment operator is still emitted, see EmptyClassMember).
	if (cls->empty && (init.constructor == nullptr || init.constructor->trivial))
	{
		if (assignment && init.constructor != nullptr && EmptyClassMember(init))
			lowering_.demand(init.constructor);
		// (A synthesized constructor's destructible one: its addresses,
		// the course's form.)
		if (construction_chain_ && needs_destruction(type))
		{
			copied_address_ = subobject_of(this_pointer(location), init, location);
			subobject_of(source_object(), init, location);
		}
		return;
	}
	// A virtual base of the source (and of an assigned object, which may
	// be a base subobject) through its table's row.
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	const bool shared = init.base != nullptr && init.base->is_virtual;
	const lowir::Value destination = shared && assignment ? virtual_base_address(this_pointer(location), entity, cls)
														  : subobject_of(this_pointer(location), init, location);
	const lowir::Value source = shared ? virtual_base_address(source_object(), entity, cls)
									   : subobject_of(source_object(), init, location);
	{
		if (!assignment && (init.constructor == nullptr || init.constructor->trivial))
		{
			// A non-virtual base whose tail padding passes what the copy may
			// write: its data (see subobject_bit_limit).
			if (!shared && init.base != nullptr &&
				(init.base->offset + lowering_.size_of(type, location)) * 8 > bit_limit_)
				copy_span(source, destination, cls->nvsize, lowering_.alignment_of(type, location));
			else if (!cls->empty)
				copy_object(type, source, destination, location);
			copied_address_ = destination;
			return;
		}
		if (!assignment && init.base != nullptr && VirtualBaseCount(cls) != 0)
			hidden_base_ = HiddenBase{ true,
				shared ? lowering_.vtt_virtual_index(entity, cls) : lowering_.vtt_base_index(entity, init.base) };
		copy_call(init, destination, source, location);
		copied_address_ = destination;
		hidden_base_.active = false;
	}
}

void FunctionLowering::copy_call(
	const sema::SubobjectInit& init, lowir::Value destination, lowir::Value source, uint32_t location)
{
	// The selected constructor (a base's by its base entry) or assignment
	// operator, with the subobject and the source's.
	const sema::FunctionEntity* function = init.constructor;
	if (function == nullptr)
		throw LoweringError(location, "a class subobject copy without its selected function");
	const EntryVariant variant = function->constructor && init.base != nullptr ? EntryVariant::Base
																			   : EntryVariant::Complete;
	const sema::Type* parameter = function->type->canonical->params[0];
	if (caller_destroyed(parameter))
	{
		// An operator= taking its class by value: the copy is the caller's
		// temporary, destroyed after the call (Itanium 3.1.2.3), a
		// full-expression of its own.
		const bool region = !full_expression_ && begin_segment();
		const size_t first = cleanups_.size();
		temporary_region();
		source = copied_parameter(init.parameter_copy, parameter, source, location);
		temporary_alive(source, parameter);
		call_copy(function, variant, destination, source, call_may_throw(function), location);
		destroy_temporaries(first);
		end_full_expression(region);
		return;
	}
	if (!parameter->is_reference())
		source = copied_parameter(init.parameter_copy, parameter, source, location);
	call_copy(function, variant, destination, source, call_may_throw(function), location);
}

lowir::Value FunctionLowering::copied_parameter(
	const sema::FunctionEntity* constructor, const sema::Type* parameter, lowir::Value source, uint32_t location)
{
	// An operator= taking its class by value (12.8p28): a temporary
	// initialized from the source's subobject by the plan's constructor
	// (8.5p15, 5.2.2p4), passed as its bytes or its address.
	if (constructor == nullptr)
		throw LoweringError(location, "a by-value assignment parameter without its constructor");
	const sema::ClassEntity* cls = sema::ClassOf(parameter);
	const lowir::SlotId slot = generated_slot(lowir::SlotKind::Argobj, lowering_.storage_type(parameter));
	Place temporary = Place();
	temporary.storage = lowir::Value::slot(slot);
	const lowir::Value object = address_of(temporary);
	if (!copies_storage(constructor) || copy_called(constructor))
		call_copy(constructor, EntryVariant::Complete, object, source, call_may_throw(constructor), location);
	else if (!cls->empty)
		copy_object(cls->type, source, object, location);
	return lowering_.passed_directly(cls) ? lowir::Value::slot(slot) : object;
}

void FunctionLowering::call_copy(const sema::FunctionEntity* function, EntryVariant variant, lowir::Value destination,
	lowir::Value source, bool throws, uint32_t location)
{
	// A copy or move constructor or assignment operator called with the
	// object and the source; a constructor's other parameters take their
	// default arguments (8.3.6, 12.8p2-p3), a full-expression with the
	// call whose temporaries end with it.
	const sema::Type* type = function->type->canonical;
	bool temporaries = !cleanups_.empty(), defaults_throw = false;
	for (uint32_t i = 1; i < type->param_count; ++i)
	{
		if (function->default_arguments == nullptr || function->default_arguments[i] == nullptr)
			throw LoweringError(location, "a copy without an argument for a parameter");
		const syntax::Expr* argument = lowering_.default_argument(function, i);
		temporaries = temporaries || creates_temporaries(argument);
		defaults_throw = defaults_throw || may_throw(argument);
	}
	// A synthesized constructor's copy that may throw (or whose default
	// arguments may) with subobjects made runs in a plain region landing on
	// their chain, opened before its default arguments: their full-
	// expression's regions nest inside it, their handlers resuming into it.
	// (Their temporaries in explicit regions, as an array element's, see
	// begin_element_temporaries: the regions with nothing to destroy land
	// on the function's shared resume, installed outside.)
	const bool chained = construction_chain_ && (throws || defaults_throw) && !construction_actions_.empty();
	const bool defaults = chained && type->param_count > 1;
	if (defaults)
		emit_marker(lowir::Opcode::EhTry, construction_handler());
	const ElementScope scope = begin_element_temporaries(defaults && element_temporaries(function));
	const bool region = type->param_count > 1 && !full_expression_ && temporaries && (throws || defaults_throw) &&
		begin_segment();
	const size_t first = cleanups_.size();
	const size_t base = arguments_.size();
	arguments_.push_back(destination);
	arguments_.push_back(source);
	for (uint32_t i = 1; i < type->param_count; ++i)
		push_default_argument(function, i, type->params[i], location);
	if (ProgramLowering::HiddenBaseParameters(function, variant))
		hidden_base_arguments(base, location);
	lowering_.demand(function, variant);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(function, variant));
	const lowir::Type result = function->constructor ? lowir::Type::scalar(lowir::TypeKind::Void)
													 : lowir::Type::scalar(lowir::TypeKind::Ptr);
	if (chained && !defaults)
		emit_marker(lowir::Opcode::EhTry, construction_handler());
	emit_call(result, callee, base, lowir::kNoId, throws);
	// (An array element's temporaries end with its scope, after their
	// regions: end_element_temporaries.)
	if (cleanups_.size() > first && element_scopes_ == 0)
		destroy_temporaries(first);
	end_full_expression(region);
	end_element_temporaries(scope);
	if (chained)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
		jump(next);
		place_construction_handlers();
		place(next);
	}
}

void FunctionLowering::copy_elements(const sema::SubobjectInit& init, const sema::Type* type, uint32_t location)
{
	// An array member element by element from the arrays' addresses
	// (unrolled up to eight, the first element's destination before the
	// source array's address, else a loop over an index slot).
	const sema::Type* element = type;
	uint64_t count = 1;
	while (element->kind == sema::TypeKind::Array)
	{
		count *= element->bound;
		element = element->element;
	}
	const uint64_t size = lowering_.size_of(element, location);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	sema::SubobjectInit each = init;
	// 15.2p2: a constructor's elements copied are destroyed, last first,
	// when a later one's copy throws (construct_elements' form: each after
	// the first in a region whose handler destroys the ones before it;
	// past eight, one handler destroys those before the index).
	const sema::FunctionEntity* constructor = init.constructor;
	bool guarded = !IsAssignment(function_->copy_kind) && needs_destruction(element) && constructor != nullptr &&
		!constructor->trivial && call_may_throw(constructor);
	const sema::Type* signature = constructor != nullptr ? constructor->type->canonical : nullptr;
	for (uint32_t k = 1; signature != nullptr && k < signature->param_count && !guarded && needs_destruction(element) &&
		!IsAssignment(function_->copy_kind) && constructor->default_arguments != nullptr;
		++k)
		guarded = constructor->default_arguments[k] != nullptr && may_throw(lowering_.default_argument(constructor, k));
	const ArrayBase where = ArrayBase{ Place(), init.member };
	const lowir::Value destination = subobject_of(this_pointer(location), init, location);
	// An element's copy whose default arguments make temporaries ends them
	// with the element, in explicit regions (see begin_element_temporaries):
	// the alive objects' chains are not made under each element's region.
	const bool temporaries = !IsAssignment(function_->copy_kind) && element_temporaries(constructor);
	if (construction_chain_ && count > 8)
	{
		copy_elements_loop(
			init, element, count, destination, subobject_of(source_object(), init, location), guarded, location);
		copied_address_ = destination;
		return;
	}
	if (construction_chain_)
	{
		// Each element made joins the chain (destroyed by the next one's
		// landing); the whole array replaces them once made.
		const size_t mark = construction_actions_.size();
		lowir::Value source = lowir::Value();
		for (uint64_t i = 0; i < count; ++i)
		{
			const lowir::Value to = element_address(destination, i, size);
			if (i == 0)
				source = subobject_of(source_object(), init, location);
			const lowir::Value from = element_address(source, i, size);
			copy_call(each, to, from, location);
			if (guarded && i + 1 < count)
				construction_made(element, to);
		}
		construction_actions_.resize(mark);
		copied_address_ = destination;
		return;
	}
	if (count <= 8)
	{
		lowir::Value source = lowir::Value();
		for (uint64_t i = 0; i < count; ++i)
		{
			const lowir::BlockId dispatch = guarded && i != 0 ? new_block(lowir::BlockKind::CallUnwindDispatch)
															  : lowir::kNoId;
			if (dispatch != lowir::kNoId)
				emit_marker(lowir::Opcode::EhTry, dispatch);
			const lowir::Value to = element_address(destination, i, size);
			if (i == 0)
				source = subobject_of(source_object(), init, location);
			const lowir::Value from = element_address(source, i, size);
			const ElementScope scope = begin_element_temporaries(temporaries, true);
			copy_call(each, to, from, location);
			end_element_temporaries(scope);
			if (dispatch == lowir::kNoId)
				continue;
			const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			jump(next);
			place(dispatch);
			destroy_elements(element, where, i, location);
			emit_resume();
			place(next);
		}
		return;
	}
	const lowir::Value source = subobject_of(source_object(), init, location);
	Place index_slot = Place();
	const bool assignment = IsAssignment(function_->copy_kind);
	index_slot.storage = lowir::Value::slot(
		generated_slot(assignment ? lowir::SlotKind::AssignArrayIndex : lowir::SlotKind::CopyArrayIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	const lowir::BlockId condition = new_block(
		assignment ? lowir::BlockKind::AssignArrayCond : lowir::BlockKind::CopyArrayCond);
	const lowir::BlockId body = new_block(
		assignment ? lowir::BlockKind::AssignArrayBody : lowir::BlockKind::CopyArrayBody);
	const lowir::BlockId end = new_block(
		assignment ? lowir::BlockKind::AssignArrayEnd : lowir::BlockKind::CopyArrayEnd);
	jump(condition);
	place(condition);
	const RValue current = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, integer(i64, count)), body, end);
	place(body);
	const lowir::BlockId cleanup = guarded ? new_block(lowir::BlockKind::CopyArrayCleanup) : lowir::kNoId;
	if (guarded)
		emit_marker(lowir::Opcode::EhTry, cleanup);
	const RValue to_offset = binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size));
	const lowir::Value to = index(i8, ir_model::IPK_ARRAY_ELEMENT, destination, to_offset).value;
	const RValue from_offset = binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size));
	const lowir::Value from = index(i8, ir_model::IPK_ARRAY_ELEMENT, source, from_offset).value;
	const ElementScope scope = begin_element_temporaries(temporaries);
	copy_call(each, to, from, location);
	end_element_temporaries(scope);
	if (guarded)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), index_slot);
	jump(condition);
	if (guarded)
	{
		// The elements before the index, last first.
		const lowir::BlockId cleanup_body = new_block(lowir::BlockKind::ConstructorArrayCleanupBody);
		const lowir::BlockId resume = new_block(lowir::BlockKind::ConstructorArrayResume);
		place(cleanup);
		const RValue made = load(i64, index_slot);
		branch(compare(lowir::ComparePredicate::Ne, i64, made, integer(i64, 0)), cleanup_body, resume);
		place(cleanup_body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, made, integer(i64, 1));
		store(i64, last, index_slot);
		const RValue offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, last, integer(i64, size)) : last;
		destroy(element, index(i8, ir_model::IPK_ARRAY_ELEMENT, array_base(where, location), offset).value, location);
		jump(cleanup);
		place(resume);
		emit_resume();
	}
	place(end);
}

void FunctionLowering::copy_elements_loop(const sema::SubobjectInit& init, const sema::Type* element, uint64_t count,
	lowir::Value destination, lowir::Value source, bool guarded, uint32_t location)
{
	// A synthesized constructor's array of more than eight elements: a
	// loop over an index slot inside the made subobjects' region; each
	// copy's landing destroys the elements before the index, last first,
	// then (its records popped) enters their chain (the course's form).
	const uint64_t size = lowering_.size_of(element, location);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::CopyArrayIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	const lowir::BlockId condition = new_block(lowir::BlockKind::CopyArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::CopyArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::CopyArrayEnd);
	const lowir::BlockId cleanup = guarded ? new_block(lowir::BlockKind::CopyArrayCleanup) : lowir::kNoId;
	const bool chained = !construction_actions_.empty();
	const lowir::BlockId outer = chained ? construction_handler() : lowir::kNoId;
	if (chained)
		emit_marker(lowir::Opcode::EhTry, outer);
	jump(condition);
	place(condition);
	const RValue current = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, integer(i64, count)), body, end);
	place(body);
	const lowir::Value to = index(
		i8, ir_model::IPK_ARRAY_ELEMENT, destination, binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size)))
								.value;
	const lowir::Value from = index(
		i8, ir_model::IPK_ARRAY_ELEMENT, source, binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size)))
								  .value;
	if (guarded)
		emit_marker(lowir::Opcode::EhTry, cleanup);
	construction_chain_ = false;
	sema::SubobjectInit each = init;
	copy_call(each, to, from, location);
	construction_chain_ = true;
	if (guarded)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), index_slot);
	jump(condition);
	if (guarded)
	{
		const lowir::BlockId test = new_block(lowir::BlockKind::ConstructionArrayCleanup);
		const lowir::BlockId cleanup_body = new_block(lowir::BlockKind::ConstructionArrayCleanupBody);
		const lowir::BlockId done = new_block(lowir::BlockKind::ConstructionArrayCleanupEnd);
		place(cleanup);
		jump(test);
		place(test);
		const RValue made = load(i64, index_slot);
		branch(compare(lowir::ComparePredicate::Ne, i64, made, integer(i64, 0)), cleanup_body, done);
		place(cleanup_body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, made, integer(i64, 1));
		store(i64, last, index_slot);
		const lowir::BlockId guard = open_unwind_guard(destructor_may_throw(element));
		destroy(element,
			index(i8, ir_model::IPK_ARRAY_ELEMENT, destination,
				binary(lowir::BinaryOp::Mul, i64, last, integer(i64, size)))
				.value,
			location);
		close_unwind_guard(guard);
		jump(test);
		place_unwind_guard(guard);
		place(done);
		if (chained)
		{
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			jump(outer);
		}
		else
			emit_resume();
	}
	place(end);
	if (!chained)
		return;
	emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
	jump(next);
	place_construction_handlers();
	place(next);
}

void FunctionLowering::construction_made(const sema::Type* type, lowir::Value address, const sema::BaseClass* base)
{
	construction_actions_.push_back(ConstructionAction{ address, type, lowir::kNoId, base });
}

lowir::BlockId FunctionLowering::construction_handler()
{
	// The innermost action's handler, the missing ones made bottom-up (the
	// resume block first).
	if (construction_resume_ == lowir::kNoId)
	{
		construction_resume_ = new_block(lowir::BlockKind::CleanupResume);
		construction_pending_.push_back(SIZE_MAX);
	}
	size_t first = construction_actions_.size();
	while (first > 0 && construction_actions_[first - 1].handler == lowir::kNoId)
		--first;
	for (size_t i = first; i < construction_actions_.size(); ++i)
	{
		construction_actions_[i].handler = new_block(lowir::BlockKind::ConstructionCleanup);
		construction_pending_.push_back(i);
	}
	return construction_actions_.empty() ? construction_resume_ : construction_actions_.back().handler;
}

void FunctionLowering::place_construction_handlers()
{
	// Each destroys its object (an array's elements last first: at their
	// constant offsets, beyond eight in a loop), then continues below.
	std::vector<size_t> pending;
	pending.swap(construction_pending_);
	for (const size_t i : pending)
	{
		if (i == SIZE_MAX)
		{
			place(construction_resume_);
			emit_resume();
			continue;
		}
		const ConstructionAction action = construction_actions_[i];
		place(action.handler);
		const lowir::BlockId guard = open_unwind_guard(destructor_may_throw(action.type));
		if (action.type->kind == sema::TypeKind::Array)
		{
			uint64_t total = 1;
			const sema::Type* element = action.type;
			for (; element->kind == sema::TypeKind::Array; element = element->element)
				total *= element->bound;
			if (total > 8)
				destroy_array_loop(element, action.address, total, 0);
			else
			{
				const uint64_t size = lowering_.size_of(element, 0);
				for (uint64_t k = total; k-- > 0;)
					destroy(element,
						index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, action.address,
							integer(lowir::Type::scalar(lowir::TypeKind::I64), k * size))
							.value,
						0);
			}
		}
		else if (action.base != nullptr &&
			VirtualBaseCount(static_cast<const sema::ClassEntity*>(action.base->base)) != 0)
		{
			// 12.4p8: a base subobject's destruction leaves the virtual
			// bases it shares to the complete object's (15.2p2: each is
			// destroyed once).
			destroy_subobject(Subobject{ action.base, nullptr, -1, false }, 0, false);
		}
		else
			destroy(action.type, action.address, 0);
		close_unwind_guard(guard);
		jump(i > 0 ? construction_actions_[i - 1].handler : construction_resume_);
		place_unwind_guard(guard);
	}
}

bool FunctionLowering::unit_transferable(uint32_t first, uint32_t last, const BitField& field, uint32_t location)
{
	// A unit is copied whole only when that copies nothing else: it is not
	// shared with another unit or subobject (sema::DataMember); no field
	// of the run is volatile; no other subobject sharing the unit's bytes
	// is volatile or copied by a function (whose result the unit's bytes
	// would replace); the unit ends within what the copy may write (an
	// assignment's or a base entry's class's data, where a base
	// subobject's tail padding may hold the derived object's members or
	// virtual bases; a complete object's virtual bases, copied first: see
	// subobject_bit_limit). Members lie in declaration order, so the scans
	// over them stop at the first one outside the unit; bases need not (an
	// empty base at offset 0, a primary base laid out first, Itanium 2.4):
	// each is checked.
	const uint64_t begin = field.unit_offset;
	const uint64_t end = begin + lowir::integer_bits(field.unit.kind()) / 8;
	if (field.shared || end * 8 > bit_limit_)
		return false;
	for (uint32_t i = first; i <= last; ++i)
	{
		if ((function_->inits[i].member->variable->type->cv & sema::kVolatile) != 0)
			return false;
	}
	uint64_t from = 0, to = 0;
	for (uint32_t i = first; i-- > 0 && function_->inits[i].base == nullptr;)
	{
		subobject_bytes(function_->inits[i], from, to, location);
		if (to <= begin)
			break;
		if (!copied_as_bytes(function_->inits[i]))
			return false;
	}
	for (uint32_t i = 0; i < first && function_->inits[i].base != nullptr; ++i)
	{
		subobject_bytes(function_->inits[i], from, to, location);
		if (from < end && to > begin && !copied_as_bytes(function_->inits[i]))
			return false;
	}
	for (uint32_t i = last + 1; i < function_->init_count; ++i)
	{
		subobject_bytes(function_->inits[i], from, to, location);
		if (from >= end)
			break;
		if (from < to && !copied_as_bytes(function_->inits[i]))
			return false;
	}
	return true;
}

bool FunctionLowering::copied_as_bytes(const sema::SubobjectInit& init)
{
	const sema::Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
	if (!type->is_reference())
	{
		const sema::Type* object = type;
		while (object->kind == sema::TypeKind::Array)
			object = object->element;
		if ((object->cv & sema::kVolatile) != 0)
			return false;
	}
	return trivial_subobject(init);
}

void FunctionLowering::subobject_bytes(const sema::SubobjectInit& init, uint64_t& from, uint64_t& to, uint32_t location)
{
	// The bytes a base's data or a member occupies.
	if (init.base != nullptr)
	{
		from = init.base->offset;
		to = from + sema::ClassOf(init.base->base->type)->nvsize;
		return;
	}
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(init.member->variable);
	if (init.member->bit_field)
	{
		const uint64_t bit = lowering_.member_bit_offset(variable, location);
		from = bit / 8;
		to = (bit + init.member->width + 7) / 8;
		return;
	}
	from = lowering_.member_offset(variable, location);
	to = from + lowering_.size_of(variable->type, location);
}

uint32_t FunctionLowering::copy_bit_fields(uint32_t first, uint32_t location)
{
	// The allocation unit of the bit-field at `first`, once, for the
	// named bit-fields after it in the same unit (the README's rule: a
	// zero-width bit-field or another unit ends the run); returns the
	// plan index of the run's last field.
	const sema::SubobjectInit& init = function_->inits[first];
	const BitField field = bit_field(static_cast<const sema::VariableEntity*>(init.member->variable), location);
	uint32_t last = first;
	for (uint32_t i = first + 1; i < function_->init_count; ++i)
	{
		const sema::DataMember* member = function_->inits[i].member;
		if (member == nullptr || !member->bit_field || member->width == 0)
			break;
		if (member->variable->name == nullptr)
		{
			last = i;
			continue;
		}
		const BitField next = bit_field(static_cast<const sema::VariableEntity*>(member->variable), location);
		if (next.unit_offset != field.unit_offset || next.unit.kind() != field.unit.kind())
			break;
		last = i;
	}
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	if (!unit_transferable(first, last, field, location))
	{
		// Field by field: each value read and merged into the object's
		// unit, as its assignment would (the README's value semantics).
		for (uint32_t i = first; i <= last; ++i)
		{
			const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(
				function_->inits[i].member->variable);
			if (variable->name == nullptr)
				continue;
			const BitField each = bit_field(variable, location);
			const bool is_volatile = (variable->type->cv & sema::kVolatile) != 0;
			Place from = Place();
			from.storage = index(i8, ir_model::IPK_FIELD, source_object(), integer(i64, each.unit_offset)).value;
			from.is_volatile = is_volatile;
			const RValue value = read_bit_field(each, from, false);
			Place target = Place();
			target.storage =
				index(i8, ir_model::IPK_FIELD, this_pointer(location), integer(i64, each.unit_offset)).value;
			target.is_volatile = is_volatile;
			write_bit_field(each, target, to_unit(each, value, variable->type->unqualified), false);
		}
		return last;
	}
	// The unit as the unsigned integer of its width.
	const unsigned unit_bits = lowir::integer_bits(field.unit.kind());
	const lowir::Type unit = lowir::Type::scalar(unit_bits == 8 ? lowir::TypeKind::U8
			: unit_bits == 16                                   ? lowir::TypeKind::U16
			: unit_bits == 32                                   ? lowir::TypeKind::U32
																: field.unit.kind());
	Place from = Place();
	from.storage = index(i8, ir_model::IPK_FIELD, source_object(), integer(i64, field.unit_offset)).value;
	from.untyped = true;
	const RValue value = load(unit, from);
	Place target = Place();
	target.storage = index(i8, ir_model::IPK_FIELD, this_pointer(location), integer(i64, field.unit_offset)).value;
	target.untyped = true;
	store(unit, value, target);
	return last;
}

} // namespace lower
