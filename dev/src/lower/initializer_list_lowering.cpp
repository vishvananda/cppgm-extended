#include "lower/function_lowering.h"

#include "sema/aggregates.h"
#include "sema/conversions.h"

// std::initializer_list objects (8.5.4p5-p6): a braced-init-list the
// analysis marked (sema::kInitializerList) materializes the array of its
// elements' copies, a temporary whose lifetime the object extends as a
// reference would (12.2p5): a local variable's (or a local aggregate's
// member's, or a temporary a local reference binds) until its scope ends,
// a static one's until the program ends, a constructor's member's until
// the constructor exits, an element's of another such array as long as
// that array; otherwise its full-expression's. It then stores the array's
// address and element count in the object's first two members (the
// builtin layout's __begin and __size, or the program's own class's). An
// initializer-list constructor is called with such an object for its first
// parameter.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

const syntax::BracedInitList* FunctionLowering::list_object(const syntax::Expr& expr)
{
	// `{...}` or `std::initializer_list<E>{...}` initializing the object
	// itself (8.5.4p3), not an initializer-list constructor's parameter nor
	// another class's converting constructor's argument.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const syntax::BracedInitList* list = e->kind == syntax::ExprKind::BracedInit
		? static_cast<const syntax::BracedInitList*>(e)
		: e->kind == syntax::ExprKind::FunctionalCast ? static_cast<const syntax::FunctionalCastExpr*>(e)->list()
													  : nullptr;
	if (list == nullptr || list->facts == nullptr || (list->facts->flags & sema::kInitializerList) == 0 ||
		e->facts == nullptr || (e->facts->conversion != nullptr && e->facts->conversion->constructor != nullptr))
		return nullptr;
	const sema::Entity* made = e->facts->entity;
	return made == nullptr || made->kind != sema::EntityKind::Function ? list : nullptr;
}

const syntax::BracedInitList* FunctionLowering::elided_list(const syntax::Initializer* init, const sema::Type* type)
{
	// `= std::initializer_list<E>{...}` or `(std::initializer_list<E>{...})`
	// initializing an object of that type without a constructor: the
	// prvalue initializes the object itself (12.8p31), its list's array
	// living as long (8.5.4p6).
	const syntax::Expr* source = init == nullptr                                            ? nullptr
		: init->kind == syntax::InitializerKind::Assign                                     ? init->expression
		: init->kind == syntax::InitializerKind::Parenthesized && init->arguments.size == 1 ? init->arguments[0]
																							: nullptr;
	if (source == nullptr || WithoutParentheses(source)->kind != syntax::ExprKind::FunctionalCast)
		return nullptr;
	const syntax::BracedInitList* list = list_object(*source);
	return list != nullptr && WithoutParentheses(source)->facts->type->unqualified == type->unqualified ? list
																										: nullptr;
}

const syntax::BracedInitList* FunctionLowering::list_variable(
	const sema::VariableEntity* variable, const syntax::Initializer* init)
{
	// The braced-init-list (or `= std::initializer_list<E>{...}`'s) that
	// initializes a std::initializer_list variable itself (8.5.4p5), else
	// null.
	if (init == nullptr || variable->constructor != nullptr || variable->type->is_reference())
		return nullptr;
	const syntax::BracedInitList* list = init->kind == syntax::InitializerKind::Braced ||
			(init->kind == syntax::InitializerKind::Assign && init->expression->kind == syntax::ExprKind::BracedInit)
		? static_cast<const syntax::BracedInitList*>(init->expression)
		: elided_list(init, variable->type);
	return list != nullptr && list->facts != nullptr && (list->facts->flags & sema::kInitializerList) != 0 ? list
																										   : nullptr;
}

bool FunctionLowering::destroyed_list_array(const syntax::Expr& expr)
{
	// A std::initializer_list object's array that is destroyed (its
	// elements', or a nested list's, 12.2p5).
	const syntax::BracedInitList* list = list_object(expr);
	if (list == nullptr)
		return false;
	if (!list->elements.empty() && destroyed_temporary(list->facts->target))
		return true;
	for (const syntax::Expr* item : list->elements)
	{
		if (destroyed_list_array(*item))
			return true;
	}
	return false;
}

void FunctionLowering::initializer_list_object(const syntax::BracedInitList& list, lowir::Value object,
	const sema::ClassEntity* entity, const sema::Entity* owner, bool persistent, uint32_t location)
{
	const sema::Type* array = list.facts->target;
	const sema::VariableEntity* variable = owner != nullptr && owner->kind == sema::EntityKind::Variable
		? static_cast<const sema::VariableEntity*>(owner)
		: nullptr;
	persistent = persistent ||
		(variable != nullptr &&
			(variable->is_static || variable->owner == nullptr || variable->owner->kind == sema::ScopeKind::Namespace ||
				(variable->owner->kind == sema::ScopeKind::Class && !variable->member)));
	const sema::DataMember* first = entity != nullptr ? entity->members : nullptr;
	const sema::DataMember* second = first != nullptr ? first->next : nullptr;
	if (second == nullptr)
		throw LoweringError(location, "a std::initializer_list without its array's address and size members");
	const uint64_t offsets[2] = { lowering_.member_offset(
									  static_cast<const sema::VariableEntity*>(first->variable), location),
		lowering_.member_offset(static_cast<const sema::VariableEntity*>(second->variable), location) };
	// An empty list's object refers to no array (8.5.4p3: value-initialized;
	// the course's form).
	const bool empty = list.elements.empty();
	const lowir::Value elements = empty ? lowir::Value()
										: backing_array(list, array, persistent, persistent ? nullptr : owner);
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	RValue values[2];
	values[0].value = elements;
	values[0].type = pointer;
	if (empty)
		values[0] = integer(pointer, 0);
	values[1] = integer(i64, array->bound);
	for (unsigned i = 0; i < 2; ++i)
	{
		Place member = Place();
		member.storage = offsets[i] == 0 ? object : index(i8, 0, object, integer(i64, offsets[i])).value;
		store(values[i].type, values[i], member);
	}
}

lowir::Value FunctionLowering::backing_array(
	const syntax::BracedInitList& list, const sema::Type* array, bool persistent, const sema::Entity* owner)
{
	// The array's storage (a global for a static object's), then each
	// element in order: a scalar's value stored at its address, a class
	// object initialized there, after the first in a region whose handler
	// destroys the ones before it (the course's form; past eight elements
	// one handler destroys them by the count made, kept in a slot; see
	// chained_elements for elements whose initialization makes temporaries).
	// An array of class objects to destroy is a temporary once all are made
	// (of the full-expression, or the owner's), destroyed last to first at
	// the addresses they were made at (past eight, by a loop; by their
	// indices when an element's destructor may throw, whose exception the
	// elements before it are destroyed by, 15.2p2), or destroyed at program
	// end.
	const sema::Type* element = array->element;
	const uint64_t size = lowering_.size_of(element, list.location);
	const bool destroyed = !persistent && destroyed_temporary(element);
	if (owner == nullptr &&
		(destroyed || (!persistent && eager_pending_ && guarded_branches_ == 0 && reopened_choices_ == 0)))
		temporary_region();
	Place storage = Place();
	storage.storage = temporary_storage(array, persistent, lowir::SlotKind::Initlist);
	const lowir::Value base = address_of(storage);
	open_deferred_region();
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const bool object = sema::ClassOf(element) != nullptr;
	const bool guarded = object && needs_destruction(element);
	// (A block-scope static's persistent array too: its initialization
	// is retried, 6.7p4; counts_elements excludes a namespace-scope one's.)
	if (guarded && (!persistent || static_local_) && explicit_regions_ == 0 && counts_elements(array, &list))
	{
		// So do elements of an aggregate class: their members made are
		// destroyed when a later one's initialization throws (15.2p2),
		// tracked in regions no element's explicit region can hold.
		if (sema::IsAggregateClass(sema::ClassOf(element)))
			return chained_elements(list, array, base, persistent, owner, storage);
		for (const syntax::Expr* item : list.elements)
		{
			if (creates_temporaries(item, nullptr, false, element))
				return chained_elements(list, array, base, persistent, owner, storage);
		}
	}
	if (guarded && list.elements.size > 8)
		return counted_elements(list, array, base, persistent, owner, storage);
	// The guarded elements' addresses (at most eight; a nested list's are
	// kept apart in element_values_ while an element is made).
	lowir::Value made[8];
	uint32_t made_count = 0;
	bool inside = false;
	size_t covered = SIZE_MAX;
	for (uint32_t i = 0; i < list.elements.size; ++i)
	{
		const syntax::Expr& item = *list.elements[i];
		if (!object)
		{
			// A braced element initializes the scalar from its one element,
			// or value-initializes it (8.5.4p3); a vector's lanes or a
			// complex value's two parts are given one by one (GNU).
			const syntax::Expr* source = &item;
			const syntax::BracedInitList* lanes = nullptr;
			if (item.kind == syntax::ExprKind::BracedInit)
			{
				const syntax::BracedInitList& inner = static_cast<const syntax::BracedInitList&>(item);
				if (sema::LaneList(element, inner))
					lanes = &inner;
				source = inner.elements.empty() ? nullptr : inner.elements[0];
			}
			const RValue value = lanes != nullptr ? vector_list(element, *lanes)
				: source != nullptr               ? fit(operand(*source, ConversionContext::ArrayElement), element)
												  : zero_value(element);
			Place target = Place();
			target.storage = i == 0 ? base : index(i8, ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size)).value;
			store(lowering_.storage_type(element), value, target);
			continue;
		}
		// The element regions inside the full-expression's (see
		// enclose_element_region); the element's address is computed in
		// the element's region when the full-expression had one (and in
		// each later one), else before it.
		const lowir::BlockId dispatch = guarded && i != 0 ? new_block(lowir::BlockKind::CallUnwindDispatch)
														  : lowir::kNoId;
		const lowir::BlockId next = dispatch != lowir::kNoId ? new_block(lowir::BlockKind::CallUnwindEnd)
															 : lowir::kNoId;
		const size_t start = cleanups_.size();
		if (dispatch != lowir::kNoId)
		{
			inside = inside || region_open_;
			enclose_element_region(covered);
		}
		const bool split = dispatch != lowir::kNoId && (inside || (!persistent && full_expression_));
		// The full-expression's region around the element's: its landing,
		// which the element's handler enters (its records popped, the
		// course's form), and it stays open across the element's end.
		const lowir::BlockId enclosing = split && region_open_ ? region_entry_ : lowir::kNoId;
		lowir::Value target = base;
		if (i != 0 && !split)
			target = index(i8, ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size)).value;
		if (dispatch != lowir::kNoId)
		{
			emit_marker(lowir::Opcode::EhTry, dispatch);
			++explicit_regions_;
		}
		if (split)
			target = index(i8, ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size)).value;
		// A nested std::initializer_list element's array lives as long as
		// this one (12.2p5).
		if (const syntax::BracedInitList* nested = list_object(item))
			initializer_list_object(*nested, target, sema::ClassOf(element), owner, persistent, item.location);
		else if (item.kind == syntax::ExprKind::BracedInit && sema::IsSubaggregate(element))
			materialize_extended(item, target, owner, persistent, item.location);
		else if (item.kind == syntax::ExprKind::BracedInit)
			materialize(item, target, item.location);
		else
			initialize_from(item, WithoutParentheses(&item)->facts->conversion, target, item.location);
		if (dispatch != lowir::kNoId)
		{
			--explicit_regions_;
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			const bool segment = region_segment_;
			region_segment_ = region_segment_ && enclosing == lowir::kNoId;
			jump(next);
			region_segment_ = segment;
			place(dispatch);
			// Inside a try or handler the enclosing region's landing is a
			// dispatch whose chain pops that region's record (see
			// eh_unwind_entry): the element's landing dispatches on the
			// innermost try's clauses as that one does and pops only its
			// own record; outside any, both records pop before the landing,
			// which resumes (the course's forms). Host objects: a landing
			// with catch clauses is entered from its pad only, so the
			// element's handler resumes into the enclosing one, the region
			// staying open.
			const bool contexts = enclosing != lowir::kNoId && !eh_contexts_.empty();
			const bool resumes = contexts && lowering_.host_objects();
			if (contexts && !resumes && innermost_try() != nullptr)
			{
				const EhContext* attempt = innermost_try();
				emit_try_clauses(*attempt->statement, attempt->chain_first, attempt->chain_count);
				emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
			}
			// The temporaries the element's initialization made, then the
			// elements made before it (15.2p1).
			for (size_t c = cleanups_.size(); c-- > start;)
				destroy_local(cleanups_[c], lowir::kNoId, true);
			for (uint32_t j = made_count; j-- > 0;)
				destroy(element, made[j], item.location);
			if (resumes)
			{
				region_segment_ = false;
				emit_resume();
				region_segment_ = segment;
			}
			else if (enclosing != lowir::kNoId)
			{
				emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
				if (!contexts)
					emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
				region_segment_ = false;
				jump(enclosing);
				// (A tracked landing's placeholder is retargeted with the
				// regions' markers.)
				note_region(builder_.last_instruction(builder_.insertion_block()));
				region_segment_ = segment;
			}
			else
				emit_resume();
			place(next);
		}
		if (guarded)
			made[made_count++] = target;
	}
	uint32_t saved = 0;
	if (destroyed && guarded && !destructor_may_throw(element))
	{
		saved = static_cast<uint32_t>(element_values_.size()) + 1;
		element_values_.insert(element_values_.end(), made, made + made_count);
	}
	finish_backing_array(list, base, array, persistent, owner, storage, saved);
	return base;
}

void FunctionLowering::finish_backing_array(const syntax::BracedInitList& list, lowir::Value base,
	const sema::Type* array, bool persistent, const sema::Entity* owner, const Place& storage, uint32_t saved)
{
	// Once all are made: destroyed at program end (a static object's), or
	// a temporary (its elements' addresses kept at `saved`, else destroyed
	// by their indices).
	if (persistent && needs_destruction(array->element) && sema::ClassOf(array->element) != nullptr)
	{
		if (static_temporary(storage.storage.index, array, list.location))
			;
		else if (static_local_ || lowering_.object_mode())
			register_destruction(storage.storage.index, array, list.location);
		else
			lowering_.finalize(storage.storage.index, array, list.location, lowir::kNoId);
	}
	else if (!persistent && destroyed_temporary(array->element))
	{
		if (temporary_alive(base, array, saved) && owner != nullptr)
			extend_temporary(owner);
		// Its region reopens at once (the course's form), here: not where
		// the segment began, before the elements were made.
		if (full_expression_ && !region_open_ && sema::ClassOf(array->element) != nullptr)
		{
			eager_pending_ = false;
			open_region(false, true);
		}
	}
	else if (!persistent)
		temporary_done();
}

void FunctionLowering::enclose_element_region(size_t& covered)
{
	// Before an element's region: with objects alive (or temporaries the
	// earlier elements made) the full-expression's region is open around
	// it, covering them all (reopened when the elements made more), so the
	// element's handler resumes into it once it destroyed the elements
	// made (15.2p1: they were constructed last; the reference compiler's
	// form); with none it ends before the first (the course's form).
	// Nothing opens or splits it inside the element's region.
	if (cleanups_.empty())
	{
		if (region_open_)
			close_region();
		return;
	}
	// (Outside a full-expression that may throw none opens: nothing then
	// unwinds through the objects alive, the course's form.)
	if (!full_expression_ && !region_open_)
		return;
	if (covered == SIZE_MAX && region_open_)
		covered = cleanups_.size();
	if (region_open_ && covered == cleanups_.size())
		return;
	close_region();
	open_region(false, true);
	covered = cleanups_.size();
}

lowir::Value FunctionLowering::counted_elements(const syntax::BracedInitList& list, const sema::Type* array,
	lowir::Value base, bool persistent, const sema::Entity* owner, const Place& storage)
{
	// More than eight class objects to destroy: the count made is kept in a
	// slot, each element after the first made in a region whose one handler
	// destroys that many, last first (the course's form, linear in the
	// elements).
	const sema::Type* element = array->element;
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const uint64_t size = lowering_.size_of(element, list.location);
	Place count = Place();
	count.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::InitlistConstructorIndex, i64));
	store(i64, integer(i64, 0), count);
	const lowir::BlockId cleanup = new_block(lowir::BlockKind::InitlistConstructorCleanup);
	size_t covered = SIZE_MAX;
	for (uint32_t i = 0; i < list.elements.size; ++i)
	{
		const syntax::Expr& item = *list.elements[i];
		lowir::Value target = base;
		if (i != 0)
		{
			enclose_element_region(covered);
			store(i64, integer(i64, i), count);
			target = index(i8, ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size)).value;
			emit_marker(lowir::Opcode::EhTry, cleanup);
			++explicit_regions_;
		}
		if (item.kind == syntax::ExprKind::BracedInit && sema::IsSubaggregate(element))
			materialize_extended(item, target, owner, persistent, item.location);
		else if (item.kind == syntax::ExprKind::BracedInit)
			materialize(item, target, item.location);
		else
			initialize_from(item, WithoutParentheses(&item)->facts->conversion, target, item.location);
		if (i != 0)
		{
			--explicit_regions_;
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		}
	}
	store(i64, integer(i64, list.elements.size), count);
	const lowir::BlockId end = new_block(lowir::BlockKind::InitlistConstructorEnd);
	// The full-expression's region around the elements' (see
	// backing_array): it stays open across the handler, which enters its
	// landing once it destroyed the elements made.
	const bool split = list.elements.size > 1 && (region_open_ || (!persistent && full_expression_));
	const lowir::BlockId enclosing = split && region_open_ ? region_entry_ : lowir::kNoId;
	const bool segment = region_segment_;
	region_segment_ = region_segment_ && enclosing == lowir::kNoId;
	jump(end);
	// (Nothing closes the region inside the handler.)
	region_segment_ = false;
	place(cleanup);
	const bool contexts = enclosing != lowir::kNoId && !eh_contexts_.empty();
	const bool resumes = contexts && lowering_.host_objects();
	if (contexts && !resumes && innermost_try() != nullptr)
	{
		const EhContext* attempt = innermost_try();
		emit_try_clauses(*attempt->statement, attempt->chain_first, attempt->chain_count);
		emit_marker(lowir::Opcode::EhCleanup, lowir::kNoId);
	}
	const lowir::BlockId test = new_block(lowir::BlockKind::InitlistConstructorCleanupCondition);
	jump(test);
	place(test);
	const RValue made = load(i64, count);
	const lowir::BlockId body = new_block(lowir::BlockKind::InitlistConstructorCleanupBody);
	const lowir::BlockId resume = new_block(lowir::BlockKind::InitlistConstructorResume);
	branch(compare(lowir::ComparePredicate::Ne, i64, made, integer(i64, 0)), body, resume);
	place(body);
	const RValue last = binary(lowir::BinaryOp::Sub, i64, made, integer(i64, 1));
	store(i64, last, count);
	RValue offset = last;
	if (size != 1)
		offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, size));
	destroy(element, index(i8, ir_model::IPK_ARRAY_ELEMENT, base, offset).value, list.location);
	jump(test);
	place(resume);
	if (enclosing != lowir::kNoId && !resumes)
	{
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		if (!contexts)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		jump(enclosing);
		note_region(builder_.last_instruction(builder_.insertion_block()));
	}
	else
		emit_resume();
	region_segment_ = segment;
	place(end);
	finish_backing_array(list, base, array, persistent, owner, storage, 0);
	return base;
}

lowir::Value FunctionLowering::chained_elements(const syntax::BracedInitList& list, const sema::Type* array,
	lowir::Value base, bool persistent, const sema::Entity* owner, const Place& storage)
{
	// Elements whose initialization makes temporaries (12.2p3) and may
	// throw: the array joins the alive objects with none made, under those
	// temporaries (push_counted, as a braced array's), so that a throw
	// destroys the temporaries constructed, then the elements made, last
	// first (15.2p1-p2: g++'s and clang's partial array). Once all are
	// made it is the temporary finish_backing_array registers (up to eight
	// elements at the addresses they were made at, as backing_array's).
	const sema::Type* element = array->element;
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const uint64_t size = lowering_.size_of(element, list.location);
	ElementCount count = ElementCount();
	const size_t counted = push_counted(array, storage.storage, count);
	const size_t saved_members = aggregate_members_;
	const bool saved_regions = member_regions_;
	aggregate_members_ = SIZE_MAX;
	lowir::Value made[8];
	for (uint32_t i = 0; i < list.elements.size; ++i)
	{
		const syntax::Expr& item = *list.elements[i];
		const size_t first = cleanups_.size();
		const lowir::Value target = i == 0 ? base
										   : index(i8, ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size)).value;
		if (const syntax::BracedInitList* nested = list_object(item))
			initializer_list_object(*nested, target, sema::ClassOf(element), owner, persistent, item.location);
		else if (item.kind == syntax::ExprKind::BracedInit && sema::IsSubaggregate(element))
			materialize_extended(item, target, owner, persistent, item.location);
		else if (item.kind == syntax::ExprKind::BracedInit)
			materialize(item, target, item.location);
		else
			initialize_from(item, WithoutParentheses(&item)->facts->conversion, target, item.location);
		element_made(count, first);
		if (i < 8)
			made[i] = target;
	}
	aggregate_members_ = saved_members;
	member_regions_ = saved_regions;
	end_counted(counted);
	uint32_t saved = 0;
	if (list.elements.size <= 8 && !destructor_may_throw(element))
	{
		saved = static_cast<uint32_t>(element_values_.size()) + 1;
		element_values_.insert(element_values_.end(), made, made + list.elements.size);
	}
	finish_backing_array(list, base, array, persistent, owner, storage, saved);
	return base;
}

void FunctionLowering::list_constructor_call(const sema::FunctionEntity* constructor, EntryVariant variant,
	lowir::Value object, const syntax::BracedInitList& list, uint32_t location)
{
	// 13.3.1.7p1's first phase: the constructor's std::initializer_list
	// parameter is copy-initialized from the object referring to the
	// list's array (a reference's temporary), passed as a class parameter
	// is; its other parameters take their default arguments.
	const sema::Type* type = constructor->type->canonical;
	const sema::Type* parameter = type->params[0];
	const bool throws = call_may_throw(constructor);
	begin_call(throws);
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	const sema::Type* list_type = parameter->is_reference() ? parameter->element : parameter;
	const sema::ClassEntity* entity = sema::ClassOf(list_type);
	Place temporary = Place();
	temporary.storage = parameter->is_reference()
		? temporary_storage(list_type, false, lowir::SlotKind::Arg)
		: lowir::Value::slot(generated_slot(lowir::SlotKind::Argobj, lowering_.storage_type(list_type)));
	const lowir::Value argument = address_of(temporary);
	initializer_list_object(list, argument, entity, nullptr, false, location);
	arguments_.push_back(
		parameter->is_reference() || !lowering_.passed_directly(entity) ? argument : temporary.storage);
	for (uint32_t i = 1; i < type->param_count; ++i)
	{
		if (constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr)
			throw LoweringError(location, "a constructor call without an argument for a parameter");
		push_default_argument(constructor, i, type->params[i], location);
	}
	lowering_.demand(constructor, variant);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(constructor, variant));
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, lowir::kNoId, throws);
	end_call(throws);
}

} // namespace lower
