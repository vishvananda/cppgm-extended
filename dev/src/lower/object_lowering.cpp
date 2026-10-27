#include "lower/function_lowering.h"

#include "sema/aggregates.h"
#include "sema/conversions.h"

// Class objects of the non-polymorphic object model (12.1, 12.4, 12.6,
// 6.6p2, 6.7p2): construction by the constructor the analysis selected for
// each initialization, destruction when an automatic object's scope ends
// (and on every jump out of it), and constructor and destructor bodies with
// their subobject initializations and destructions.
namespace lower {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// A user-provided special member (8.4.2p4).
bool UserProvided(const sema::FunctionEntity* function)
{
	return !function->implicit && !function->defaulted && !function->deleted;
}

// The braced-init-list of `{...}` or `= {...}`, else null.
const syntax::BracedInitList* BracedList(const syntax::Initializer& init)
{
	if (init.kind == syntax::InitializerKind::Braced ||
		(init.kind == syntax::InitializerKind::Assign && init.expression->kind == syntax::ExprKind::BracedInit))
		return static_cast<const syntax::BracedInitList*>(init.expression);
	return nullptr;
}

} // namespace

const sema::ClassEntity* FunctionLowering::object_class(const sema::Type* type)
{
	while (type->kind == sema::TypeKind::Array)
		type = type->element;
	return sema::ClassOf(type);
}

bool FunctionLowering::needs_destruction(const sema::Type* type)
{
	// 12.4p5: a class object (or array of them) whose destructor is not
	// trivial.
	if (type->is_reference())
		return false;
	const sema::ClassEntity* entity = object_class(type);
	if (entity == nullptr || entity->destructor == nullptr || entity->destructor->trivial)
		return false;
	// A union's implicit destructor destroys no variant member (12.4p8).
	return entity->key != TokenKind::KW_UNION || !entity->destructor->implicit;
}

// ------------------------------------------------------------ construction

void FunctionLowering::call_special(const sema::FunctionEntity* function, EntryVariant variant, lowir::Value object,
	const syntax::Slice<syntax::Expr*>& written, uint32_t location, bool elision)
{
	// A constructor or destructor call: the object's address, then the
	// arguments converted to the parameters. A trivial copy or move
	// constructor copies the source's bytes (12.8p16).
	const sema::ClassEntity* owner = function->owner != nullptr && function->owner->kind == sema::ScopeKind::Class
		? static_cast<const sema::ClassEntity*>(function->owner->entity)
		: nullptr;
	// A base subobject whose tail is not its own (see construct_base): a
	// storage copy writes its data only.
	const bool data_only = zero_data_only_;
	zero_data_only_ = false;
	if (written.size == 1 && elides_conversion(function, WithoutParentheses(written[0])->facts->conversion))
	{
		// A copy of a conversion function's class prvalue is that result.
		const syntax::Expr* e = WithoutParentheses(written[0]);
		call_conversion(*e, e->facts, e->facts->conversion->constructor, object);
		return;
	}
	// A local's direct-initialization calls an explicitly defaulted move
	// constructor even when trivial (the course's form).
	const bool local_direct = local_direct_init_;
	local_direct_init_ = false;
	const bool declared = declaring_;
	declaring_ = false;
	const bool allocation_copy = allocation_copy_;
	allocation_copy_ = false;
	const bool defaulted_move = local_direct && function->copy_kind == sema::CopyKind::MoveConstructor &&
		!function->implicit;
	if (written.size == 1 && copies_storage(function) && !defaulted_move &&
		!conversion_function(WithoutParentheses(written[0])->facts->conversion))
	{
		// A trivial copy of a prvalue of the class (a conditional's) is the
		// prvalue constructing the object itself.
		const syntax::Expr* e = WithoutParentheses(written[0]);
		if (e->facts->category == sema::ValueCategory::PRValue && sema::ClassOf(e->facts->type) == owner)
		{
			materialize(*e, object, location);
			return;
		}
	}
	// A new-expression's object copied from a glvalue calls the copy
	// constructor, trivial or not (the course's form).
	if (written.size == 1 && copies_storage(function) && !defaulted_move && !allocation_copy &&
		!conversion_function(WithoutParentheses(written[0])->facts->conversion) && !copy_called(function))
	{
		// An empty class copies nothing: a source without side effects is
		// not evaluated (the course's form).
		if (owner->empty && side_effect_free(written[0], false, 0))
			return;
		const lowir::Value source = bind_reference(
			*written[0], function->type->canonical->params[0], ConversionContext::Argument);
		if (!owner->empty && data_only)
			copy_span(source, object, owner->nvsize, lowering_.alignment_of(owner->type, location));
		else if (!owner->empty)
			copy_object(owner->type, source, object, location);
		return;
	}
	// An implicit destruction's unwinding is lowered by its caller's
	// explicit regions (destroy_cleanups, the destructor epilogue): the
	// alive objects' region would destroy the object again.
	const bool throws = !function->destructor && call_may_throw(function);
	declared_construction_ = declared;
	note_call_arguments(function, written);
	begin_call(throws);
	declared_construction_ = false;
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	const bool hidden = ProgramLowering::HiddenBaseParameters(function, variant);
	if (hidden && function->constructor && written.size != 0)
		hidden_base_vtt(location);
	if (function->constructor)
		arguments(written, function->type->canonical, function, location);
	if (hidden)
		hidden_base_arguments(base, location);
	lowering_.demand(function, variant);
	// The course defines a dynamic class's tables (where the unit may)
	// with each construction of an object of it, its constructor defined
	// here or not (a host object only the tables its code refers to).
	if (function->constructor && owner != nullptr && owner->dynamic && !lowering_.host_objects())
		lowering_.class_table(owner);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(function, variant));
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, lowir::kNoId, throws, elision);
	end_call(throws);
}

bool FunctionLowering::elidable_source(const syntax::Expr& expr, const sema::FunctionEntity* constructor)
{
	// 12.8p31: a temporary of the object's own class (cv aside), not a
	// derived class's the copy slices.
	const syntax::Expr* e = WithoutParentheses(&expr);
	return constructor != nullptr && !copies_storage(constructor) && e->kind == syntax::ExprKind::Conditional &&
		e->facts->category == sema::ValueCategory::PRValue && constructor->copy_kind != sema::CopyKind::None &&
		sema::ClassOf(e->facts->type) == sema::ClassOf(constructor->owner->entity->type);
}

bool FunctionLowering::copy_called(const sema::FunctionEntity* function)
{
	// A trivial copy or move constructor of an empty class whose destructor
	// is not trivial: a copy from a glvalue calls it all the same (the
	// course's form); a prvalue of the class still initializes the object.
	const sema::ClassEntity* owner = sema::ClassOf(function->owner->entity->type);
	return function->trivial && owner != nullptr && owner->empty && needs_destruction(owner->type) &&
		(function->copy_kind == sema::CopyKind::CopyConstructor ||
			function->copy_kind == sema::CopyKind::MoveConstructor);
}

bool FunctionLowering::copies_storage(const sema::FunctionEntity* function)
{
	// A trivial copy or move constructor: its call is the source's bytes
	// (12.8p16, the README's direct value transfer).
	if (function->copy_kind != sema::CopyKind::CopyConstructor &&
		function->copy_kind != sema::CopyKind::MoveConstructor)
		return false;
	if (function->trivial)
		return true;
	// So is a class template instance's one explicitly defaulted out of
	// the template (user-provided, 8.4.2p4, so passed as not trivial) whose
	// subobject copies are all trivial: the course performs it as the bytes
	// (as-if, 1.9p1).
	const sema::FunctionEntity* pattern = sema::PatternOf(function);
	if (!function->instantiated || pattern == nullptr || !pattern->synthesized || pattern->defaulted ||
		!lowering_.instantiate_definition(function))
		return false;
	for (uint32_t i = 0; i < function->init_count; ++i)
	{
		const sema::FunctionEntity* copy = function->inits[i].constructor;
		if (copy != nullptr && !copy->trivial)
			return false;
	}
	return true;
}

void FunctionLowering::call_constructor(const sema::FunctionEntity* constructor, lowir::Value object,
	const syntax::Expr& argument, const sema::Conversion* conversion, uint32_t location, bool elision)
{
	// A constructor called with one expression whose binding to its first
	// parameter is `conversion` (a converting constructor's user-defined
	// conversion, 13.3.3.1.2, or a copy or move of an object of its class);
	// its other parameters take their default arguments.
	const sema::Type* signature = constructor->type->canonical;
	const sema::ClassEntity* owner = sema::ClassOf(constructor->owner->entity->type);
	if (elides_conversion(constructor, conversion))
	{
		// A copy of a conversion function's class prvalue is that result.
		call_conversion(argument, WithoutParentheses(&argument)->facts, conversion->constructor, object);
		return;
	}
	// An explicit conversion function's result (13.3.1.4, CWG 899) is a
	// temporary the constructor is called with, even a trivial one.
	if (copies_storage(constructor) && !conversion_function(conversion))
	{
		const syntax::Expr* e = WithoutParentheses(&argument);
		if (e->facts->category == sema::ValueCategory::PRValue && sema::ClassOf(e->facts->type) == owner)
		{
			materialize(*e, object, location);
			return;
		}
		if (!copy_called(constructor))
		{
			const lowir::Value source = bind_converted(
				argument, conversion, signature->params[0], ConversionContext::Argument, false);
			if (!owner->empty)
				copy_object(owner->type, source, object, location);
			return;
		}
	}
	const bool throws = call_may_throw(constructor);
	begin_call(throws);
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	// `C(...)` takes its operand through the ellipsis (13.3.3.1.3).
	push_converted(argument, signature->param_count != 0 ? signature->params[0] : nullptr, conversion);
	for (uint32_t i = 1; i < signature->param_count; ++i)
	{
		if (constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr)
			throw LoweringError(location, "a constructor call without an argument for a parameter");
		push_default_argument(constructor, i, signature->params[i], location);
	}
	lowering_.demand(constructor, EntryVariant::Complete);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(constructor, EntryVariant::Complete));
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, lowir::kNoId, throws, elision);
	end_call(throws);
}

void FunctionLowering::construct_from(
	const sema::FunctionEntity* constructor, lowir::Value object, lowir::Value source, uint32_t location)
{
	// A constructor whose first parameter, a reference, binds the object at
	// `source` (its others take their default arguments); a trivial copy or
	// move is the source's bytes (12.8p16).
	const sema::Type* signature = constructor->type->canonical;
	const sema::ClassEntity* owner = sema::ClassOf(constructor->owner->entity->type);
	if (copies_storage(constructor) && !copy_called(constructor))
	{
		if (!owner->empty)
			copy_object(owner->type, source, object, location);
		return;
	}
	const bool throws = call_may_throw(constructor);
	begin_call(throws);
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	arguments_.push_back(source);
	for (uint32_t i = 1; i < signature->param_count; ++i)
	{
		if (constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr)
			throw LoweringError(location, "a constructor call without an argument for a parameter");
		push_default_argument(constructor, i, signature->params[i], location);
	}
	lowering_.demand(constructor, EntryVariant::Complete);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(constructor, EntryVariant::Complete));
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, lowir::kNoId, throws);
	end_call(throws);
}

bool FunctionLowering::elides_conversion(const sema::FunctionEntity* constructor, const sema::Conversion* conversion)
{
	// A copy or move constructor whose argument is a conversion function's
	// result of its class by value: the course constructs the object by the
	// conversion function (12.8p31).
	if (!conversion_function(conversion) || conversion->constructor->is_explicit ||
		(constructor->copy_kind != sema::CopyKind::CopyConstructor &&
			constructor->copy_kind != sema::CopyKind::MoveConstructor))
		return false;
	const sema::Type* returned = conversion->constructor->type->canonical->element;
	return !returned->is_reference() && returned->unqualified == constructor->owner->entity->type;
}

void FunctionLowering::initialize_from(
	const syntax::Expr& expr, const sema::Conversion* conversion, lowir::Value object, uint32_t location)
{
	if (conversion_function(conversion))
	{
		// A conversion function's prvalue of the class initializes the
		// object itself (12.8p31); another result (a reference's glvalue, a
		// derived class prvalue in a temporary) is the argument of the
		// constructor the analysis recorded as the second step (8.5p16).
		const sema::FunctionEntity* function = conversion->constructor;
		const sema::Conversion* after = conversion->after;
		const sema::FunctionEntity* constructor = after != nullptr ? after->constructor : nullptr;
		if (constructor == nullptr)
		{
			call_conversion(expr, WithoutParentheses(&expr)->facts, function, object);
			return;
		}
		const sema::Type* returned = function->type->canonical->element;
		const lowir::Value result = returned->is_reference()
			? call_conversion(expr, WithoutParentheses(&expr)->facts, function, lowir::Value()).value
			: conversion_temporary(expr, function, false);
		const sema::Type* parameter = constructor->type->canonical->params[0];
		// A derived class result's temporary is addressed again for its
		// base subobject (the course's form).
		const lowir::Value source = after->argument != nullptr &&
				after->argument->second == sema::ValueConversion::DerivedToBase
			? to_base(returned->is_reference() ? result : slot_address_again(result),
				  sema::ClassOf(returned->is_reference() ? returned->element : returned),
				  sema::ClassOf(parameter->element), location)
			: result;
		construct_from(constructor, object, source, location);
		return;
	}
	// A class object copy-initialized from an expression (8.5p16) as its
	// recorded conversion says: by the constructor it selected (a
	// converting, copy or move constructor), else as the class prvalue
	// that constructs the object in place (12.8p31).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (conversion != nullptr && conversion->constructor != nullptr)
	{
		// `{}` value-initializing an aggregate by its implicit default
		// constructor is its aggregate initialization (see
		// materialize_prvalue), by the helper where it takes the list.
		const sema::ClassEntity* entity = sema::ClassOf(conversion->constructor->owner->entity->type);
		uint32_t count = 0;
		if (e->kind == syntax::ExprKind::BracedInit &&
			static_cast<const syntax::BracedInitList*>(e)->elements.empty() && conversion->constructor->implicit &&
			conversion->constructor->constructor && sema::IsAggregateClass(entity) &&
			helper_shape(entity, static_cast<const syntax::BracedInitList*>(e), count))
		{
			uint32_t next = 0;
			aggregate_helper_call(
				entity, count, object, static_cast<const syntax::BracedInitList*>(e), next, location, 0);
			return;
		}
		call_constructor(conversion->constructor, object, expr, conversion->argument, location,
			elidable_source(expr, conversion->constructor));
		return;
	}
	if (e->facts->category != sema::ValueCategory::PRValue)
	{
		// A glvalue whose copy the analysis left to the object's bytes.
		const sema::ClassEntity* target = sema::ClassOf(
			conversion != nullptr && conversion->result != nullptr ? conversion->result : e->facts->type);
		const lowir::Value source = to_base(address(*e), sema::ClassOf(e->facts->type), target, location);
		if (!target->empty)
			copy_object(target->type, source, object, location);
		return;
	}
	materialize(*e, object, location);
}

void FunctionLowering::construct(const sema::Type* type, lowir::Value object, const syntax::Initializer* init,
	const sema::FunctionEntity* constructor, EntryVariant variant, uint32_t location)
{
	// 8.5p16 for a class object at `object` (its address already computed):
	// default-initialization runs a default constructor that is not
	// trivial; an initializer runs the constructor the analysis selected
	// with its arguments, value-initializes (`()` and `{}`), initializes an
	// aggregate member by member, or lets a class prvalue initialize the
	// object itself.
	const syntax::Slice<syntax::Expr*> none = syntax::Slice<syntax::Expr*>();
	if (init == nullptr)
	{
		if (constructor != nullptr && !constructor->trivial)
			call_special(constructor, variant, object, none, location);
		return;
	}
	if (const syntax::BracedInitList* list = BracedList(*init))
	{
		// 8.5.4p5: std::initializer_list's object, or an initializer-list
		// constructor's call with one.
		if (list->facts != nullptr && (list->facts->flags & sema::kInitializerList) != 0)
		{
			if (constructor == nullptr)
				initializer_list_object(*list, object, sema::ClassOf(list->facts->type), nullptr, false, location);
			else
				list_constructor_call(constructor, variant, object, *list, location);
			return;
		}
		if (constructor == nullptr && type->unqualified->kind == sema::TypeKind::Class)
		{
			const AddressPath path = AddressPath{ AddressPath::Root::Value, object, path_steps_.size() };
			aggregate_object(type, path, list, false, location);
		}
		else if (constructor == nullptr)
			initialize_object(type, object, *list, location);
		else if (list->elements.empty())
			value_initialize(type, object, constructor, variant, location);
		else
			call_special(constructor, variant, object, list->elements, location);
		return;
	}
	if (init->kind == syntax::InitializerKind::Parenthesized)
	{
		if (init->arguments.empty())
			value_initialize(type, object, constructor, variant, location);
		else if (constructor != nullptr)
			call_special(constructor, variant, object, init->arguments, location);
		else
			materialize(*init->arguments[0], object, location);
		return;
	}
	if (constructor == nullptr)
	{
		const sema::Conversion* conversion = WithoutParentheses(init->expression)->facts->conversion;
		if (conversion_function(conversion))
			initialize_from(*init->expression, conversion, object, location);
		else
			materialize(*init->expression, object, location);
		return;
	}
	syntax::Slice<syntax::Expr*> single;
	single.data = const_cast<syntax::Expr**>(&init->expression);
	single.size = 1;
	call_special(constructor, variant, object, single, location, elidable_source(*init->expression, constructor));
}

void FunctionLowering::closure_object(const syntax::LambdaExpr& lambda, lowir::Value object, uint32_t location)
{
	// Each member in order: the captured `this`, then each captured
	// entity's address, or its copy (5.1.2p21: a class object's by its
	// constructor, an array's elements in increasing subscript order, as
	// its bytes when they are copied so), as the enclosing body names it (a
	// capture of an enclosing lambda's through that closure). 15.2p2: a
	// member a later member's copy may throw after is destroyed by that
	// copy's unwinding, in a region of its own (the course's form for a
	// local aggregate's members); the closure owns them once complete.
	const sema::ClosureFacts* facts = lowering_.graph().closure_facts(sema::ClassOf(lambda.facts->type));
	if (facts == nullptr)
		return;
	const lowir::Type pointer = lowir::Type::scalar(lowir::TypeKind::Ptr);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const uint32_t first = facts->this_member != nullptr ? 1 : 0;
	const uint32_t count = facts->capture_count + first;
	// Whether a member's copy calls a constructor that may throw; the last
	// such member.
	const auto copy_throws = [this](const sema::Capture& capture) {
		const sema::FunctionEntity* constructor = capture.constructor;
		const sema::Type* type = capture.member->type;
		return constructor != nullptr && !type->is_reference() && call_may_throw(constructor) &&
			!(copies_storage(constructor) && (type->kind == sema::TypeKind::Array || !copy_called(constructor)));
	};
	uint32_t last_throwing = 0;
	for (uint32_t i = first; i < count; ++i)
	{
		if (copy_throws(facts->captures[i - first]))
			last_throwing = i;
	}
	const bool tracking = !static_initializer_;
	const size_t members = cleanups_.size();
	bool tracked = false, pending = false;
	for (uint32_t i = 0; i < count; ++i)
	{
		const bool is_this = i < first;
		const sema::Capture* capture = is_this ? nullptr : &facts->captures[i - first];
		const uint64_t offset = lowering_.member_offset(is_this ? facts->this_member : capture->member, location);
		Place member = Place();
		member.storage = offset == 0 ? object : index(i8, ir_model::IPK_FIELD, object, integer(i64, offset)).value;
		const sema::Type* type = is_this ? nullptr : capture->member->type;
		if (type != nullptr && !type->is_reference())
		{
			if (pending && copy_throws(*capture))
			{
				closure_member_region();
				pending = false;
			}
			const Place source = variable_place(capture->entity, capture->entity->type, location);
			if (capture->constructor != nullptr && type->kind != sema::TypeKind::Array)
				construct_from(capture->constructor, member.storage, address_of(source), location);
			else if (capture->constructor != nullptr && !copies_storage(capture->constructor))
				copy_captured_elements(type, capture->constructor, member.storage, address_of(source), location);
			else if (sema::ClassOf(type) != nullptr || type->kind == sema::TypeKind::Array)
				copy_object(type, address_of(source), member.storage, location);
			else
				store(lowering_.storage_type(type), load(lowering_.storage_type(type), source), member);
			if (tracking && i < last_throwing && needs_destruction(type))
			{
				cleanups_.push_back(Cleanup{ member.storage, type, nullptr, lowir::kNoId, lowir::kNoId, lowir::kNoId,
					false, false, lowir::kNoId, false, 0, 0, 0, true });
				tracked = pending = true;
			}
			continue;
		}
		RValue value;
		value.type = pointer;
		value.value = is_this ? object_pointer_as(sema::ClassOf(facts->this_member->type->element), location)
							  : address_of(variable_place(capture->entity, capture->entity->type, location));
		store(pointer, value, member);
	}
	if (!tracked)
		return;
	// The members leave the chain (temporaries the copies' default
	// arguments made stay until the full-expression ends, their chain
	// blocks made again).
	close_region();
	size_t kept = members;
	bool below = false;
	for (size_t i = members; i < cleanups_.size(); ++i)
	{
		if (cleanups_[i].member)
		{
			below = true;
			continue;
		}
		Cleanup cleanup = cleanups_[i];
		if (below)
		{
			cleanup.unwind = cleanup.shared_return = cleanup.conditional = lowir::kNoId;
			cleanup.eh_marked = cleanup.eh_unmarked = 0;
		}
		cleanups_[kept++] = cleanup;
	}
	cleanups_.resize(kept);
}

void FunctionLowering::closure_member_region()
{
	// The region a closure member's copy may throw in once members to
	// destroy were made: it opens here (not where the construction or the
	// segment began), its handler destroying them first.
	close_region();
	eager_pending_ = false;
	const bool construction = construction_;
	construction_ = false;
	open_region(false, true);
	construction_ = construction;
}

void FunctionLowering::copy_captured_elements(const sema::Type* type, const sema::FunctionEntity* constructor,
	lowir::Value destination, lowir::Value source, uint32_t location)
{
	// 5.1.2p21: an array member's elements (a multidimensional array's as
	// one sequence) copied from the captured array's in increasing
	// subscript order by the class's constructor; one that throws destroys
	// the elements before it, last first (15.2p2), as construct_elements
	// does: each element after the first in a region of its own (eight or
	// fewer), else a loop over an index slot whose handler destroys the
	// elements before the index.
	const sema::Type* element = type;
	uint64_t count = 1;
	while (element->kind == sema::TypeKind::Array)
	{
		count *= element->bound;
		element = element->element;
	}
	const uint64_t size = lowering_.size_of(element, location);
	// An element's copy whose default arguments make temporaries ends them
	// with the element (begin_element_temporaries); a default argument that
	// may throw guards the elements too.
	bool throws = false;
	const bool temporaries = element_temporaries(constructor, &throws);
	const bool guarded = needs_destruction(element) && throws;
	if (temporaries && throws && count <= 8)
		before_throwing_call();
	// The element regions sit inside the region destroying the objects
	// alive (see enclose_element_region), which stays open across them:
	// no region opens or closes inside an element's, whose handler resumes
	// into it.
	size_t covered = SIZE_MAX;
	if (count <= 8)
	{
		for (uint64_t i = 0; i < count; ++i)
		{
			if (i == 0 || !guarded)
			{
				const ElementScope scope = begin_element_temporaries(temporaries, true);
				// (Operands emitted last first: see FunctionLowering::binary.)
				const lowir::Value from = element_address(source, i, size);
				construct_from(constructor, element_address(destination, i, size), from, location);
				end_element_temporaries(scope);
				continue;
			}
			enclose_element_region(covered);
			const lowir::BlockId dispatch = new_block(lowir::BlockKind::CallUnwindDispatch);
			const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
			emit_marker(lowir::Opcode::EhCleanup, dispatch);
			++explicit_regions_;
			const ElementScope scope = begin_element_temporaries(temporaries);
			const lowir::Value from = element_address(source, i, size);
			construct_from(constructor, element_address(destination, i, size), from, location);
			end_element_temporaries(scope);
			--explicit_regions_;
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
			const bool segment = region_segment_;
			region_segment_ = segment && !region_open_;
			jump(next);
			place(dispatch);
			for (uint64_t j = i; j-- > 0;)
				destroy(element, element_address(destination, j, size), location);
			emit_resume();
			region_segment_ = segment;
			place(next);
		}
		return;
	}
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	Place index_slot = Place();
	index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::CopyArrayIndex, i64));
	store(i64, integer(i64, 0), index_slot);
	if (throws)
		before_throwing_call();
	// (The loop inside the region destroying the objects alive, open
	// across it, as the unrolled elements'.)
	const bool segment = region_segment_;
	if (guarded)
	{
		enclose_element_region(covered);
		region_segment_ = segment && !region_open_;
	}
	const lowir::BlockId condition = new_block(lowir::BlockKind::CopyArrayCond);
	const lowir::BlockId body = new_block(lowir::BlockKind::CopyArrayBody);
	const lowir::BlockId end = new_block(lowir::BlockKind::CopyArrayEnd);
	jump(condition);
	place(condition);
	const RValue current = load(i64, index_slot);
	branch(compare(lowir::ComparePredicate::Ult, i64, current, integer(i64, count)), body, end);
	place(body);
	lowir::BlockId cleanup = lowir::kNoId;
	if (guarded)
	{
		cleanup = new_block(lowir::BlockKind::CopyArrayCleanup);
		emit_marker(lowir::Opcode::EhCleanup, cleanup);
		++explicit_regions_;
	}
	const RValue offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, current, integer(i64, size)) : current;
	const ElementScope scope = begin_element_temporaries(temporaries);
	const lowir::Value from = index(i8, ir_model::IPK_ARRAY_ELEMENT, source, offset).value;
	construct_from(constructor, index(i8, ir_model::IPK_ARRAY_ELEMENT, destination, offset).value, from, location);
	end_element_temporaries(scope);
	if (guarded)
	{
		--explicit_regions_;
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	}
	store(i64, binary(lowir::BinaryOp::Add, i64, current, integer(i64, 1)), index_slot);
	jump(condition);
	if (guarded)
	{
		const lowir::BlockId cleanup_body = new_block(lowir::BlockKind::ConstructorArrayCleanupBody);
		const lowir::BlockId resume = new_block(lowir::BlockKind::ConstructorArrayResume);
		place(cleanup);
		const RValue constructed = load(i64, index_slot);
		branch(compare(lowir::ComparePredicate::Ne, i64, constructed, integer(i64, 0)), cleanup_body, resume);
		place(cleanup_body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, constructed, integer(i64, 1));
		store(i64, last, index_slot);
		const RValue last_offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, last, integer(i64, size)) : last;
		destroy(element, index(i8, ir_model::IPK_ARRAY_ELEMENT, destination, last_offset).value, location);
		jump(cleanup);
		place(resume);
		emit_resume();
	}
	region_segment_ = segment;
	place(end);
}

void FunctionLowering::value_initialize(const sema::Type* type, lowir::Value object,
	const sema::FunctionEntity* constructor, EntryVariant variant, uint32_t location)
{
	// 8.5p7: a user-provided default constructor runs; otherwise the object
	// is zero-initialized (one span), then an implicit default constructor
	// that is not trivial runs.
	if (constructor != nullptr && UserProvided(constructor))
	{
		call_special(constructor, variant, object, syntax::Slice<syntax::Expr*>(), location);
		return;
	}
	zero_object(type, object, location);
	if (constructor != nullptr && !constructor->trivial)
		call_special(constructor, variant, object, syntax::Slice<syntax::Expr*>(), location);
}

namespace {

// An aggregate whose value-initialization is its zero-initialization: its
// class members (arrays' elements too) are such aggregates (none has a
// constructor, 8.5p7; a C++11 aggregate has no member initializers).
bool ZeroedByValue(const sema::ClassEntity* entity, unsigned depth)
{
	if (depth > 64)
		return false;
	for (const sema::DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const sema::Type* type = member->variable->type;
		while (type->kind == sema::TypeKind::Array)
			type = type->element;
		const sema::ClassEntity* cls = type->is_reference() ? nullptr : sema::ClassOf(type);
		if (cls != nullptr && (!sema::IsAggregateClass(cls) || !ZeroedByValue(cls, depth + 1)))
			return false;
	}
	return true;
}

} // namespace

void FunctionLowering::zero_object(const sema::Type* type, lowir::Value object, uint32_t location)
{
	// The size first: it lays the class out. A complete empty object's
	// byte is padding and is zero-initialized too (8.5p6). An overlapping
	// empty subobject must not overwrite another subobject's bytes.
	uint64_t size = lowering_.size_of(type, location);
	const sema::ClassEntity* entity = type->kind == sema::TypeKind::Array ? nullptr : sema::ClassOf(type);
	uint64_t alignment = lowering_.alignment_of(type, location);
	const bool data_only = entity != nullptr && zero_data_only_;
	if (data_only)
	{
		size = entity->nvsize;
		alignment = sema::NonVirtualAlignment(entity);
	}
	zero_data_only_ = false;
	if (entity != nullptr && entity->empty && overlapping_empty_)
		return;
	zero_bytes(object, size, alignment);
	// Pointers to data members zero-initialize to their null value (-1,
	// Itanium 2.3; 8.5p5).
	null_data_member_pointers(type, object, 0, !data_only);
}

void FunctionLowering::null_data_member_pointers(
	const sema::Type* type, lowir::Value object, uint64_t offset, bool complete)
{
	// The null value stored in each pointer to data member of a zeroed
	// object of `type` at `object` + `offset`: a class's in its non-virtual
	// bases, its members (a union's first, 8.5p5) and, for a complete
	// object, its virtual bases; an array's elements in a loop past four, so
	// the code stays proportional to the type, not the object.
	if (!lowering_.holds_data_member_pointer(type))
		return;
	type = type->canonical;
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
	const auto at = [&](uint64_t bytes) {
		return bytes == 0 ? object : index(i8, ir_model::IPK_NONE, object, integer(i64, bytes)).value;
	};
	if (type->kind == sema::TypeKind::Array)
	{
		const uint64_t size = lowering_.size_of(type->element, 0);
		if (type->bound <= 4)
		{
			for (uint64_t i = 0; i < type->bound; ++i)
				null_data_member_pointers(type->element, object, offset + i * size, true);
			return;
		}
		RValue elements;
		elements.value = at(offset);
		elements.type = lowir::Type::scalar(lowir::TypeKind::Ptr);
		null_member_pointer_elements(elements, type->element, constant_value(i64, type->bound), 0);
		return;
	}
	if (type->unqualified->kind == sema::TypeKind::MemberPointer)
	{
		Place place = Place();
		place.storage = at(offset);
		store(i64, integer(i64, static_cast<uint64_t>(lowering_.null_data_member())), place);
		return;
	}
	const sema::ClassEntity* cls = sema::ClassOf(type);
	lowering_.size_of(type, 0);
	for (const sema::BaseClass* edge = cls->bases; edge != nullptr; edge = edge->next)
	{
		if (!edge->is_virtual)
			null_data_member_pointers(
				static_cast<const sema::ClassEntity*>(edge->base)->type, object, offset + edge->offset, false);
	}
	for (const sema::DataMember* member = cls->members; member != nullptr; member = member->next)
	{
		if (!member->bit_field && !member->variable->type->is_reference())
			null_data_member_pointers(member->variable->type, object, offset + member->offset / 8, true);
		if (cls->key == TokenKind::KW_UNION)
			break;
	}
	for (uint32_t i = 0; complete && i < sema::VirtualBaseCount(cls); ++i)
	{
		const sema::BaseClass* edge = sema::VirtualBases(cls)[i].base;
		null_data_member_pointers(
			static_cast<const sema::ClassEntity*>(edge->base)->type, object, offset + edge->offset, false);
	}
}

void FunctionLowering::zero_bytes(lowir::Value object, uint64_t size, uint64_t alignment)
{
	// Preliminary class-storage zeroing is one span, including union padding;
	// scalar initialization and subsequent volatile accesses remain separate.
	lowir::Instruction inst = lowir::make_instruction(
		lowir::Opcode::ZeroInit, lowir::Type::scalar(lowir::TypeKind::Void));
	inst.ops[0] = object;
	inst.immediate = static_cast<int64_t>(size);
	unsigned log2 = 0;
	while ((uint64_t(1) << log2) < alignment)
		++log2;
	inst.sub = static_cast<uint8_t>(log2);
	emit(inst);
}

void FunctionLowering::materialize(const syntax::Expr& expr, lowir::Value object, uint32_t location)
{
	// A class prvalue's own reference members' temporaries end with its
	// full-expression (not a local aggregate's, see local_object).
	const sema::Entity* saved = aggregate_extension_;
	aggregate_extension_ = nullptr;
	materialize_prvalue(expr, object, location);
	aggregate_extension_ = saved;
}

void FunctionLowering::materialize_extended(
	const syntax::Expr& expr, lowir::Value object, const sema::Entity* owner, bool persistent, uint32_t location)
{
	// A class prvalue whose object lives until `owner`'s scope ends (or the
	// program's, `persistent`): so do the temporaries its aggregate
	// reference members bind and its std::initializer_list members' arrays
	// (12.2p5, 8.5.4p6); none: as materialize.
	if (owner == nullptr && !persistent)
		return materialize(expr, object, location);
	const sema::Entity* saved = aggregate_extension_;
	const bool saved_static = static_object_;
	aggregate_extension_ = owner;
	static_object_ = owner == nullptr;
	materialize_prvalue(expr, object, location);
	aggregate_extension_ = saved;
	static_object_ = saved_static;
}

void FunctionLowering::materialize_prvalue(const syntax::Expr& expr, lowir::Value object, uint32_t location)
{
	// A class prvalue initializes the object it is materialized in (12.2,
	// 12.8p31): `T(args)`, `T{...}` and `T()` construct there.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const sema::ExprFacts* facts = e->facts;
	const sema::FunctionEntity* constructor = facts != nullptr && facts->entity != nullptr &&
			facts->entity->kind == sema::EntityKind::Function
		? static_cast<const sema::FunctionEntity*>(facts->entity)
		: nullptr;
	const sema::Type* type = facts->type;
	const syntax::BracedInitList* braced = e->kind == syntax::ExprKind::BracedInit
		? static_cast<const syntax::BracedInitList*>(e)
		: e->kind == syntax::ExprKind::FunctionalCast ? static_cast<const syntax::FunctionalCastExpr*>(e)->list()
													  : nullptr;
	const sema::ClassEntity* entity = sema::ClassOf(type);
	uint32_t count = 0;
	const bool helper = braced != nullptr &&
		(constructor == nullptr || (braced->elements.empty() && constructor->implicit)) && entity != nullptr &&
		sema::IsAggregateClass(entity) && helper_shape(entity, braced, count);
	if (helper)
	{
		// An aggregate temporary by its class's initialization helper (the
		// course's form for an object at a computed address); `{}`'s value-
		// initialization by the implicit default constructor is the same as
		// its aggregate initialization (8.5.4p3 as CWG 1467 orders it).
		uint32_t next = 0;
		aggregate_helper_call(entity, count, object, braced, next, location, type->cv);
		return;
	}
	// `T{}` of another aggregate whose class members are such aggregates
	// too: its value-initialization zeroes the whole object (the course's
	// form for a temporary or result with members, and for a declared
	// object); a member with a constructor is initialized by it (8.5.1p7).
	if (braced != nullptr && braced->elements.empty() && entity != nullptr && (!entity->empty || declaring_) &&
		sema::IsAggregateClass(entity) && (constructor == nullptr || constructor->trivial) && ZeroedByValue(entity, 0))
	{
		zero_object(type, object, location);
		return;
	}
	// 5.1.2p14: a closure object's members hold its captures (a
	// captureless lambda's has nothing to initialize).
	if (e->kind == syntax::ExprKind::Lambda)
	{
		closure_object(static_cast<const syntax::LambdaExpr&>(*e), object, location);
		return;
	}
	if (e->kind == syntax::ExprKind::FunctionalCast)
	{
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(*e);
		syntax::Initializer view = syntax::Initializer();
		if (cast.list() != nullptr)
		{
			view.kind = syntax::InitializerKind::Braced;
			view.expression = const_cast<syntax::BracedInitList*>(cast.list());
		}
		else
		{
			view.kind = syntax::InitializerKind::Parenthesized;
			view.arguments = cast.arguments;
		}
		view.location = cast.location;
		construct(type, object, &view, constructor, EntryVariant::Complete, location);
		return;
	}
	if (e->kind == syntax::ExprKind::BracedInit)
	{
		// A braced-init-list argument copy-list-initializes its parameter
		// (or a reference's temporary, 8.5.4p3).
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Assign;
		view.expression = const_cast<syntax::Expr*>(e);
		view.location = e->location;
		construct(type, object, &view, constructor, EntryVariant::Complete, location);
		return;
	}
	if (e->kind == syntax::ExprKind::Cast)
	{
		// 5.2.9p4, 5.4p4: `(T)e` and `static_cast<T>(e)` are `T t(e);`.
		syntax::Expr* operand = static_cast<const syntax::CastExpr*>(e)->operand;
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Parenthesized;
		view.arguments.data = &operand;
		view.arguments.size = 1;
		view.location = e->location;
		construct(type, object, &view, constructor, EntryVariant::Complete, location);
		return;
	}
	// An atomic builtin's class result (a __c11_atomic builtin's) is the
	// integer of its size: stored as the object's bytes.
	if (e->kind == syntax::ExprKind::Call && (facts->flags & sema::kIntrinsic) != 0)
	{
		const RValue bytes = intrinsic(static_cast<const syntax::CallExpr&>(*e));
		Place target = Place();
		target.storage = object;
		store(bytes.type, bytes, target);
		return;
	}
	// So are the class bytes read by __builtin_va_arg and __builtin_bit_cast.
	if (e->kind == syntax::ExprKind::BuiltinTrait &&
		(static_cast<const syntax::BuiltinTraitExpr*>(e)->trait == GnuTrait::VaArg ||
			static_cast<const syntax::BuiltinTraitExpr*>(e)->trait == GnuTrait::BitCast))
	{
		const RValue bytes = builtin_trait_value(static_cast<const syntax::BuiltinTraitExpr&>(*e));
		Place target = Place();
		target.storage = object;
		target.untyped = static_cast<const syntax::BuiltinTraitExpr*>(e)->trait == GnuTrait::BitCast;
		store(bytes.type, bytes, target);
		return;
	}
	if (e->kind == syntax::ExprKind::Call && static_cast<const syntax::CallExpr*>(e)->callee->facts == nullptr)
	{
		// `T(args)` whose type name the parser did not classify.
		syntax::Initializer view = syntax::Initializer();
		view.kind = syntax::InitializerKind::Parenthesized;
		view.arguments = static_cast<const syntax::CallExpr*>(e)->arguments;
		view.location = e->location;
		construct(type, object, &view, constructor, EntryVariant::Complete, location);
		return;
	}
	// 5.2.2p10, 12.2p1: a call's class result initializes the object.
	if (e->kind == syntax::ExprKind::Call)
	{
		call(static_cast<const syntax::CallExpr&>(*e), object);
		return;
	}
	if ((facts->flags & sema::kOperatorCall) != 0)
	{
		operator_call(*e, object);
		return;
	}
	// 2.14.8p1: so does a user-defined literal's literal operator call.
	if (e->kind == syntax::ExprKind::Literal && (facts->flags & sema::kUserLiteral) != 0)
	{
		literal(static_cast<const syntax::LiteralExpr&>(*e), object);
		return;
	}
	if (e->kind == syntax::ExprKind::Conditional)
	{
		// 5.16p6: the selected operand copy-initializes the object. Its
		// temporaries end with the full-expression, which may still copy
		// the object: guarded (5.16p1, 12.2p3).
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		const lowir::BlockId then_block = new_block(lowir::BlockKind::CondobjThen);
		const lowir::BlockId else_block = new_block(lowir::BlockKind::CondobjElse);
		const lowir::BlockId end = new_block(lowir::BlockKind::CondobjEnd);
		begin_guard_scope();
		branch(truth(*choice.condition), then_block, else_block);
		place(then_block);
		const uint32_t opened = regions_opened_;
		++guarded_branches_;
		conditional_operand(*choice.then_expr, object, location);
		--guarded_branches_;
		if (!terminated_)
			jump(end);
		place(else_block);
		// A tracked construction's region opens again at the second
		// operand's start, landing on the objects under the temporaries
		// (the course's form).
		const bool reopened = tracked_.active && tracked_.construction && full_expression_ && !region_open_ &&
			explicit_regions_ == 0 && regions_opened_ != opened;
		if (reopened)
		{
			const lowir::BlockId entry = eh_contexts_.empty() ? unwind_entry(tracked_.floor)
															  : region_landing(tracked_.floor);
			emit_marker(lowir::Opcode::EhTry, entry);
			region_open_ = true;
			region_depth_ = tracked_.floor;
			region_entry_ = entry;
			reopened_construction_ = true;
		}
		++guarded_branches_;
		conditional_operand(*choice.else_expr, object, location);
		--guarded_branches_;
		reopened_construction_ = false;
		if (!terminated_)
			jump(end);
		place(end);
		--guard_scopes_;
		return;
	}
	if (e->kind == syntax::ExprKind::Statement)
	{
		statement_expression(static_cast<const syntax::StatementExpr&>(*e), object);
		return;
	}
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA)
	{
		// 5.18p1: the right operand's prvalue after the left's effects.
		discard(*static_cast<const syntax::BinaryExpr*>(e)->left);
		materialize(*static_cast<const syntax::BinaryExpr*>(e)->right, object, location);
		return;
	}
	throw LoweringError(e->location, "a class prvalue outside the supported construction forms");
}

void FunctionLowering::conditional_operand(const syntax::Expr& operand_expr, lowir::Value object, uint32_t location)
{
	// A class conditional's selected operand initializing the object; a
	// throw-expression's initializes nothing (5.16p2).
	const syntax::Expr* e = WithoutParentheses(&operand_expr);
	if (e->kind == syntax::ExprKind::Throw)
	{
		discard(operand_expr);
		return;
	}
	// A std::initializer_list operand's object is made apart, its array
	// a temporary of the full-expression, then copied (the course's form).
	const syntax::BracedInitList* listed = list_object(operand_expr);
	const sema::Type* type = e->facts->type;
	if (listed != nullptr && sema::ClassOf(type) != nullptr && !listed->elements.empty())
	{
		if (tracked_.active && full_expression_ && !region_open_ && explicit_regions_ == 0)
			open_region(true, true);
		Place made = Place();
		made.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::Argobj, lowering_.storage_type(type)));
		const lowir::Value address = address_of(made);
		initializer_list_object(*listed, address, sema::ClassOf(type), nullptr, false, location);
		copy_object(type, made.storage, object, location);
		return;
	}
	initialize_from(operand_expr, e->facts->conversion, object, location);
}

lowir::Value FunctionLowering::temporary_object(const syntax::Expr& expr, bool persistent, lowir::SlotKind kind)
{
	// 12.2p1: a temporary the prvalue initializes, destroyed when its
	// full-expression ends (12.2p3); a region opens for its construction.
	// A comma's left operand is evaluated before the temporary its right
	// operand makes (5.18p1; the course's form).
	const syntax::Expr* e = WithoutParentheses(&expr);
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA &&
		(e->facts->flags & sema::kOperatorCall) == 0)
	{
		// (A reference extending the comma's temporary extends its right
		// operand's.)
		const syntax::Expr* right = static_cast<const syntax::BinaryExpr*>(e)->right;
		for (size_t i = 0; i < persistent_path_.size(); ++i)
		{
			if (persistent_path_[i] == e)
			{
				persistent_path_.push_back(WithoutParentheses(right));
				break;
			}
		}
		discard(*static_cast<const syntax::BinaryExpr*>(e)->left);
		return temporary_object(*right, persistent, kind);
	}
	const sema::Type* type = e->facts->type;
	// 12.2p5: one a reference binds persists: a static reference's in a
	// global, a local one's until the reference's scope ends.
	bool extended = false;
	for (size_t i = 0; i < persistent_path_.size() && !extended; ++i)
		extended = persistent_path_[i] == e;
	const bool scoped = extended && scope_extension_ != nullptr;
	persistent = persistent || (extended && !scoped);
	// One whose destruction does nothing opens a pending region all the
	// same (the course's form); a conditional's, made in its branches, does
	// not unless it is destroyed (its region then opens at the
	// full-expression's start, before the condition).
	const bool destroyed = !persistent && destroyed_temporary(type);
	const bool branched = e->kind == syntax::ExprKind::Conditional;
	const sema::Entity* made = e->facts != nullptr ? e->facts->entity : nullptr;
	const bool zeroed = made == nullptr ||
		(made->kind == sema::EntityKind::Function && static_cast<const sema::FunctionEntity*>(made)->trivial);
	// A destroyed temporary's region opening with nothing alive opens at
	// the declaration's start, before the declared object's address.
	if (destroyed && branched && eager_pending_ && !region_open_ && declared_start_block_ == segment_block_ &&
		cleanups_.size() <= region_floor())
		segment_after_ = declared_start_after_;
	// A temporary a local reference extends, made with nothing alive: no
	// region for its construction; one opens once it is alive, covering
	// the reference's initialization (the course's staging).
	const bool staged = scoped && destroyed && !region_open_ && chain_empty() && full_expression_;
	if (staged)
		;
	// A call's result a local reference extends (its destruction doing
	// nothing) with objects alive: the region opens at the segment's start,
	// before its address (the course's form).
	else if (scoped && !destroyed && e->kind == syntax::ExprKind::Call && eager_pending_ && !region_open_ &&
		!cleanups_.empty() && full_expression_ && !tracked_.active && explicit_regions_ == 0 && guarded_branches_ == 0)
		open_region(true);
	else if (branched
			? destroyed && eager_pending_ && !region_open_ && guarded_branches_ == 0 && reopened_choices_ == 0
			: destroyed || (!persistent && eager_pending_ && guarded_branches_ == 0 && reopened_choices_ == 0))
		temporary_region(zeroed && e->kind == syntax::ExprKind::FunctionalCast, branched);
	Place place = Place();
	place.storage = temporary_storage(type, persistent, kind);
	const lowir::Value object = address_of(place);
	if (!persistent && !branched)
		region_event();
	// A std::initializer_list object a reference extends (or a static
	// one's) extends its array alike (8.5.4p6).
	const syntax::BracedInitList* list = extended || persistent ? list_object(expr) : nullptr;
	if (list != nullptr)
		initializer_list_object(
			*list, object, sema::ClassOf(type), scoped ? scope_extension_ : nullptr, persistent, expr.location);
	else if (extended && sema::IsSubaggregate(type))
		materialize_extended(expr, object, scoped ? scope_extension_ : nullptr, persistent, expr.location);
	else
		materialize(expr, object, expr.location);
	if (destroyed)
	{
		if (temporary_alive(object, type) && scoped)
		{
			extend_temporary(scope_extension_);
			// (A conditional's whose subobject the reference designates.)
			const bool subobject = extension_root_ == nullptr || WithoutParentheses(extension_root_) != e;
			if ((staged || (branched && subobject)) && !region_open_ && full_expression_)
			{
				eager_pending_ = false;
				open_region(false, true);
			}
		}
	}
	else if (persistent)
		persistent_alive(place.storage, type);
	// (A std::initializer_list object a local reference extends: its
	// region stays open across the reference's initialization.)
	else if (list == nullptr)
		temporary_done((list_object(expr) != nullptr && persistent_path_.empty()) || scoped);
	return object;
}

lowir::Value FunctionLowering::array_temporary(
	const syntax::FunctionalCastExpr& expr, bool persistent, lowir::SlotKind kind)
{
	return array_temporary(expr, *expr.braced, persistent, kind);
}

lowir::Value FunctionLowering::array_temporary(
	const syntax::Expr& expr, const syntax::BracedInitList& list, bool persistent, lowir::SlotKind kind)
{
	// `A{...}` naming an array type, a prvalue (5.2.3p3) used as a glvalue,
	// or a braced-init-list argument of a reference to an array (8.5.4p3):
	// converted to a pointer (4.2p1), bound to a reference (8.5.3p5) or
	// discarded, it is a temporary array its list initializes (8.5.4p3,
	// 12.2p1), its elements in order (8.5.4p4), destroyed when its
	// full-expression ends (12.2p3) unless a reference extends it (12.2p5)
	// as temporary_object's class temporaries.
	bool extended = false;
	for (size_t i = 0; i < persistent_path_.size() && !extended; ++i)
		extended = persistent_path_[i] == &expr;
	const bool scoped = extended && scope_extension_ != nullptr;
	return array_temporary(
		expr, list, expr.facts->type, persistent || (extended && !scoped), scoped ? scope_extension_ : nullptr, kind);
}

lowir::Value FunctionLowering::array_temporary(const syntax::Expr& expr, const syntax::BracedInitList& list,
	const sema::Type* type, bool persistent, const sema::Entity* owner, lowir::SlotKind kind)
{
	// The temporary array of `type` the list initializes: in a global
	// (`persistent`), alive until `owner`'s scope ends, or until its
	// full-expression ends.
	const bool scoped = owner != nullptr;
	const bool destroyed = !persistent && destroyed_temporary(type);
	if (destroyed || (!persistent && eager_pending_ && guarded_branches_ == 0 && reopened_choices_ == 0))
		temporary_region();
	Place place = Place();
	place.storage = temporary_storage(type, persistent, kind);
	if (persistent)
	{
		aggregate_array(
			type, AddressPath{ AddressPath::Root::Storage, place.storage, path_steps_.size() }, &list, expr.location);
		persistent_alive(place.storage, type);
		return address_of(place);
	}
	// Its address taken once, for its initialization and its use.
	const lowir::Value object = address_of(place);
	region_event();
	Place array = Place();
	array.storage = object;
	aggregate_array_object(type, array, list, nullptr, expr.location);
	if (!destroyed)
	{
		temporary_done();
		return object;
	}
	if (temporary_alive(object, type) && scoped)
		extend_temporary(owner);
	return object;
}

// ------------------------------------------------------------- destruction

lowir::Value FunctionLowering::slot_address_again(lowir::Value address)
{
	// A slot's address taken again (an address another instruction took
	// otherwise as it is).
	if (!address.is(lowir::ValueKind::Temp))
		return address;
	const lowir::FunctionBody& body = builder_.body();
	const lowir::InstId def = body.temps[address.index].def;
	if (def == lowir::kNoId || body.instructions[def].op != lowir::Opcode::Addr ||
		!body.instructions[def].ops[0].is(lowir::ValueKind::Slot))
		return address;
	Place place = Place();
	place.storage = body.instructions[def].ops[0];
	return address_of(place);
}

lowir::Value FunctionLowering::element_address(lowir::Value array, uint64_t element, uint64_t size)
{
	// An array element's address: its byte offset, scaled from the index
	// by a multiplication unless elements are bytes.
	const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
	RValue offset = integer(i64, element);
	if (size != 1)
		offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, size));
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, array, offset).value;
}

void FunctionLowering::destroy(const sema::Type* type, lowir::Value object, uint32_t location, bool row)
{
	// 12.4p8: a class object's destructor; an array's elements in reverse
	// order of construction, a row's (an element of an enclosing array) at
	// their constant byte offsets in it.
	if (type->kind == sema::TypeKind::Array)
	{
		const sema::Type* element = type->element;
		const uint64_t size = lowering_.size_of(element, location);
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		for (uint64_t i = type->bound; i-- > 0;)
			destroy(element,
				row ? index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, object,
						  integer(i64, i * size))
						  .value
					: element_address(object, i, size),
				location, true);
		return;
	}
	const sema::ClassEntity* entity = sema::ClassOf(type);
	if (entity == nullptr || entity->destructor == nullptr || entity->destructor->trivial)
		return;
	call_special(entity->destructor, EntryVariant::Complete, object, syntax::Slice<syntax::Expr*>(), location);
}

void FunctionLowering::destroy_local(const Cleanup& cleanup, lowir::BlockId below, bool unwinding)
{
	// A static's initialization an exception leaves (Itanium 3.3.2).
	if (cleanup.aborts_guard)
	{
		guard_call(ProgramLowering::GuardCall::Abort, cleanup.storage.index);
		return;
	}
	// The object's address is taken again where its scope ends; a
	// destructor that may throw there runs in `below`'s region.
	const bool saved_destroying = destroying_;
	destroying_ = true;
	struct Restore
	{
		bool& flag;
		bool saved;
		~Restore() { flag = saved; }
	} restore{ destroying_, saved_destroying };
	// An automatic variable's destruction (not a temporary's, nor that of
	// one a reference or aggregate extends, whose owner is of another type)
	// demands a virtual destructor's entries together.
	const sema::ClassEntity* entity = object_class(cleanup.type);
	if (!cleanup.temporary && entity != nullptr && entity->destructor != nullptr && cleanup.variable != nullptr &&
		object_class(cleanup.variable->type) == entity)
		lowering_.demand_destruction(entity->destructor);
	Place place = Place();
	place.storage = cleanup.storage;
	lowir::BlockId done = lowir::kNoId;
	// 15.2p3: a destructor exiting by an exception during unwinding calls
	// std::terminate (15.5.1p1): such a destruction runs in a region of its
	// own whose handler does (a new-expression's storage is freed by a
	// deallocation function, which does not throw). The region encloses a
	// guarded temporary's test too.
	const bool terminates = unwinding && below == lowir::kNoId && cleanup.deallocator == nullptr &&
		destructor_may_throw(cleanup.type);
	const lowir::BlockId guard = terminates ? cleanup_terminate(lexical_placing_) : lowir::kNoId;
	const bool region = below != lowir::kNoId || terminates;
	if (region)
		emit_marker(lowir::Opcode::EhTry, terminates ? guard : below);
	// (A tracked full-expression's region stays open across the test.)
	const bool segment = region_segment_;
	if (cleanup.guarded)
	{
		// A guarded temporary only if it was made: its flag cleared first.
		const lowir::Type u8 = lowir::Type::scalar(lowir::TypeKind::U8);
		Place flag = Place();
		flag.storage = lowir::Value::slot(cleanup.flag);
		const lowir::BlockId destroy_block = new_block(lowir::BlockKind::ConditionalTemporaryDestroy);
		done = new_block(lowir::BlockKind::ConditionalTemporaryDone);
		region_segment_ = segment && !tracked_.active;
		branch(load(u8, flag), destroy_block, done);
		this->place(destroy_block);
		store(u8, integer(u8, 0), flag);
	}
	if (cleanup.deallocator != nullptr)
	{
		// A new-expression's storage (allocation): its deallocation
		// function, given a sized one's object size or a placement one's
		// further arguments again (5.3.4p20, 3.7.4.2p2).
		const RValue size = integer(lowir::Type::scalar(lowir::TypeKind::I64), lowering_.size_of(cleanup.type, 0));
		free_storage(cleanup.deallocator, cleanup.storage, size, 0, cleanup.saved_elements);
	}
	else if (cleanup.counted)
	{
		// An array being made (aggregate_array_object): the elements its
		// slot counts, last first.
		const sema::Type* element = cleanup.type;
		while (element->kind == sema::TypeKind::Array)
			element = element->element;
		const uint64_t size = lowering_.size_of(element, 0);
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place count = Place();
		count.storage = lowir::Value::slot(cleanup.flag);
		const lowir::BlockId test = new_block(lowir::BlockKind::ConstructionArrayCleanup);
		const lowir::BlockId body = new_block(lowir::BlockKind::ConstructionArrayCleanupBody);
		const lowir::BlockId end = new_block(lowir::BlockKind::ConstructionArrayCleanupEnd);
		jump(test);
		this->place(test);
		const RValue made = load(i64, count);
		branch(compare(lowir::ComparePredicate::Ne, i64, made, integer(i64, 0)), body, end);
		this->place(body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, made, integer(i64, 1));
		store(i64, last, count);
		const RValue offset = size != 1 ? binary(lowir::BinaryOp::Mul, i64, last, integer(i64, size)) : last;
		destroy(element,
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, address_of(place), offset)
				.value,
			0);
		jump(test);
		this->place(end);
	}
	else if (cleanup.saved_elements != 0 && cleanup.storage.is(lowir::ValueKind::Slot))
	{
		// A tracked list array's elements from its slot's address again.
		const lowir::Value base = address_of(place);
		const uint64_t size = lowering_.size_of(cleanup.type->element, 0);
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		for (uint64_t i = cleanup.type->bound; i-- > 0;)
			destroy(cleanup.type->element,
				index(
					lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, integer(i64, i * size))
					.value,
				0);
	}
	else if (cleanup.saved_elements != 0)
	{
		for (uint64_t i = cleanup.type->bound; i-- > 0;)
			destroy(cleanup.type->element, element_values_[cleanup.saved_elements - 1 + i], 0);
	}
	else
		destroy_at(cleanup.type, place, 0, !unwinding);
	if (region && done == lowir::kNoId)
		emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
	if (done != lowir::kNoId)
	{
		jump(done);
		this->place(done);
		if (region)
			emit_marker(lowir::Opcode::EhEnd, lowir::kNoId);
		region_segment_ = segment;
	}
}

void FunctionLowering::destroy_at(const sema::Type* type, const Place& place, uint32_t location, bool guarded)
{
	// A named object's destruction takes its address again for each
	// element of an array (beyond eight, once for a loop).
	if (type->kind == sema::TypeKind::Array)
	{
		if (guarded && destructor_may_throw(type))
		{
			uint64_t count = 1;
			const sema::Type* element = type;
			for (; element->kind == sema::TypeKind::Array; element = element->element)
				count *= element->bound;
			destroy_elements_guarded(element, ArrayBase{ place, nullptr }, count, location);
			return;
		}
		const sema::Type* element = type->element;
		if (element->kind != sema::TypeKind::Array && type->bound > 8)
		{
			destroy_array_loop(element, address_of(place), type->bound, location);
			return;
		}
		// A multidimensional array row by row, each row's elements from
		// its address.
		const uint64_t size = lowering_.size_of(element, location);
		for (uint64_t i = type->bound; i-- > 0;)
			destroy(element, element_address(address_of(place), i, size), location, true);
		return;
	}
	destroy(type, address_of(place), location);
}

void FunctionLowering::destroy_cleanups(size_t depth, bool returning, size_t contexts)
{
	// 6.6p2: leaving scopes destroys their automatic objects in reverse
	// order of construction, and leaves the exception-handling contexts
	// above the first `contexts` where they began (their normal exits). The
	// unwinding blocks a destructor's region made follow the destructions.
	// The lexical landings' blocks wait for the next placement (the
	// course's form); a handler's throwing end's are placed here.
	bool placed = false;
	size_t context = eh_contexts_.size();
	for (size_t i = cleanups_.size();; --i)
	{
		for (; context > contexts && eh_contexts_[context - 1].floor >= i; --context)
		{
			const size_t pending = chain_blocks_.size();
			exit_context(context - 1);
			placed = placed || chain_blocks_.size() != pending;
		}
		if (i == depth)
			break;
		if (!returning || !cleanups_[i - 1].result)
			destroy_local(cleanups_[i - 1], lexical_handler(i - 1, context), false);
	}
	if (!placed)
		return;
	const lowir::BlockId next = new_block(lowir::BlockKind::CallUnwindEnd);
	jump(next);
	place_chain_blocks();
	place(next);
}

void FunctionLowering::leave_scope(size_t depth)
{
	if (!terminated_)
		destroy_cleanups(depth);
	cleanups_.resize(depth);
}

// ------------------------------------------------------- local class objects

void FunctionLowering::local_object(const sema::VariableEntity* variable, const Place& place,
	const syntax::Initializer* initializer, uint32_t location, const sema::Type* object_type,
	const StaticInitialization* initialization)
{
	// A class object (or array of them) of automatic storage: constructed
	// where declared, destroyed when its scope ends.
	const sema::Type* type = object_type != nullptr ? object_type : variable->type;
	const syntax::BracedInitList* list = initializer != nullptr ? BracedList(*initializer) : nullptr;
	// `T x = T{...}` of an aggregate: the list initializes x itself
	// (12.8p31), member by member as `T x = {...}` (the course's form).
	const syntax::Expr* source = initializer != nullptr && initializer->kind == syntax::InitializerKind::Assign
		? WithoutParentheses(initializer->expression)
		: nullptr;
	// `T x = e` of an aggregate T by a conversion function whose result is
	// no indirect class result: the course plans a staging object for the
	// converted value beside the variable, though the result initializes
	// the variable itself (12.8p31).
	const sema::Conversion* staged = source != nullptr && source->facts != nullptr ? source->facts->conversion
																				   : nullptr;
	if (conversion_function(staged) && type->kind != sema::TypeKind::Array &&
		sema::IsAggregateClass(sema::ClassOf(type)) && !lowering_.indirect_result(staged->constructor))
		generated_slot(lowir::SlotKind::Argobj, lowering_.storage_type(type));
	const syntax::BracedInitList* elided = source != nullptr && source->kind == syntax::ExprKind::FunctionalCast &&
			source->facts->entity == nullptr && source->facts->type->unqualified == type->unqualified &&
			sema::IsAggregateClass(sema::ClassOf(type))
		? static_cast<const syntax::FunctionalCastExpr*>(source)->list()
		: nullptr;
	if (list == nullptr && elided != nullptr && !elided->elements.empty())
		list = elided;
	else
		elided = nullptr;
	// So does `std::initializer_list<E> x = std::initializer_list<E>{...}`'s.
	if (list == nullptr && variable->constructor == nullptr)
		list = elided_list(initializer, type);
	// An aggregate's initializer, or one that makes temporaries, is a
	// full-expression (a parenthesized list's arguments together, 1.9p10);
	// a constructor's call (or an array's) runs in a region when it may
	// throw.
	const syntax::Expr* single = initializer == nullptr || list != nullptr ? list
		: initializer->kind == syntax::InitializerKind::Parenthesized
		? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
		: initializer->expression;
	size_t first = cleanups_.size();
	bool region = false;
	// An object its constructor or a prvalue initializes: its address is
	// taken before its initializer's full-expression begins (the course's
	// form).
	const bool aggregate = list != nullptr && (variable->constructor == nullptr || elided != nullptr);
	const bool moved = source != nullptr && source->kind == syntax::ExprKind::Cast &&
		(source->facts->flags & sema::kReferenceCast) != 0 &&
		sema::ClassOf(source->facts->type) == sema::ClassOf(type) && variable->constructor != nullptr &&
		copies_storage(variable->constructor) && !sema::ClassOf(type)->empty;
	const bool constructed = type->kind != sema::TypeKind::Array && !aggregate && !moved;
	if (terminated_)
		this->place(new_block(lowir::BlockKind::Unreachable));
	const lowir::BlockId start_block = builder_.insertion_block();
	const lowir::InstId start_after = builder_.last_instruction(start_block);
	const lowir::Value object = constructed ? address_of(place) : lowir::Value::none();
	// The object's constructor is the full-expression's call (a declared
	// object's, 1.9p10). Its segment begins before the object's address
	// (where a region with nothing alive opens, the course's form).
	if (single != nullptr && (list != nullptr || creates_temporaries(single)))
	{
		const bool constructs = variable->constructor != nullptr && list == nullptr &&
			type->kind != sema::TypeKind::Array && !variable->constructor->trivial &&
			call_may_throw(variable->constructor);
		region = begin_full_expression(single, false, constructs);
		if (region && eager_pending_ && segment_block_ == start_block)
		{
			declared_start_block_ = start_block;
			declared_start_after_ = start_after;
		}
		// A tracked full-expression's flags are cleared before the object's
		// address.
		if (region && tracked_.active && tracked_.block == start_block && tracked_.first_reset == lowir::kNoId)
			tracked_.after = start_after;
	}
	else if (initializer != nullptr && initializer->kind == syntax::InitializerKind::Parenthesized)
	{
		bool temporaries = caller_destroys(variable->constructor);
		for (const syntax::Expr* argument : initializer->arguments)
			temporaries = temporaries || creates_temporaries(argument);
		region = temporaries && begin_initializer(initializer, variable->constructor);
	}
	else if (initializer != nullptr && caller_destroys(variable->constructor))
		region = begin_initializer(initializer, variable->constructor);
	if (!region)
		begin_construction(initializer);
	// 12.2p5: the temporaries its aggregate reference members are bound to
	// live as long as it (see bind_extended); a block-scope static's (or
	// thread_local's) in static storage, destroyed at exit after it (3.6.3,
	// as a namespace-scope static's and a static reference's are), not
	// with the block.
	const sema::Entity* saved_extension = aggregate_extension_;
	const bool saved_static_object = static_object_;
	aggregate_extension_ = list != nullptr && !static_local_ ? variable : nullptr;
	struct DeclaredStart
	{
		lowir::BlockId& block;
		~DeclaredStart() { block = lowir::kNoId; }
	} declared_start{ declared_start_block_ };
	if (static_local_)
		static_object_ = true;
	if (type->kind == sema::TypeKind::Array)
		construct_array(type, place, initializer, variable, location);
	else if (list != nullptr && list->facts != nullptr && (list->facts->flags & sema::kInitializerList) != 0 &&
		variable->constructor == nullptr)
		initializer_list_object(
			*list, address_of(place), sema::ClassOf(type), static_local_ ? nullptr : variable, static_local_, location);
	else if (aggregate)
	{
		// 8.5.1: a class aggregate member by member from its root.
		const AddressPath path = AddressPath{ AddressPath::Root::Storage, place.storage, path_steps_.size() };
		aggregate_object(type, path, list, true, location);
	}
	else if (moved)
	{
		// `T x = static_cast<T&&>(y)` by a trivial copy: the object's value
		// read and stored (the course's form).
		const lowir::Type storage = lowering_.storage_type(type);
		store(storage, load(storage, glvalue(*source)), place);
	}
	else
	{
		local_direct_init_ = initializer != nullptr && initializer->kind == syntax::InitializerKind::Parenthesized;
		declaring_ = true;
		construct(type, object, initializer, variable->constructor, EntryVariant::Complete, location);
		local_direct_init_ = false;
		declaring_ = false;
	}
	aggregate_extension_ = saved_extension;
	static_object_ = saved_static_object;
	// A single object whose destructor has no effect is not destroyed (the
	// destructor stays defined). The returned local constructed in the
	// result's destination is the caller's once returned; unwinding and
	// the other exits from its scope destroy it (15.2p1, 6.6p2), a
	// temporary's destructor throwing at the initializer's end too.
	const bool result = variable == returned_local_;
	const bool inert = !static_local_ && needs_destruction(type) && type->kind != sema::TypeKind::Array &&
		lowering_.destructor_does_nothing(object_class(type));
	const bool destroyed = !static_local_ && needs_destruction(type) && !inert;
	const Cleanup made = Cleanup{ place.storage, type, variable, lowir::kNoId, lowir::kNoId, lowir::kNoId, false };
	// An aggregate whose members' landings destroyed members is destroyed
	// from its completed address when its temporaries' destruction
	// unwinds (the course's form, whatever their destructors).
	Cleanup completed = made;
	completed_landing_ = aggregate && completion_address_.kind != lowir::ValueKind::None;
	if (completed_landing_)
		completed.storage = completion_address_;
	completion_address_ = lowir::Value::none();
	// A class conditional's temporary result, the object made from it, is
	// destroyed in a region landing on the objects under it (the course's
	// form).
	const syntax::Expr* chosen = single != nullptr ? WithoutParentheses(single) : nullptr;
	if (region && chosen != nullptr && chosen->kind == syntax::ExprKind::Conditional && cleanups_.size() > first &&
		!tracked_.active && !region_open_ && explicit_regions_ == 0 && full_expression_ &&
		chosen->facts->category == sema::ValueCategory::PRValue && sema::ClassOf(chosen->facts->type) != nullptr)
	{
		const lowir::BlockId entry = eh_contexts_.empty() ? unwind_entry(first) : region_landing(first);
		emit_marker(lowir::Opcode::EhTry, entry);
		region_open_ = true;
		eager_pending_ = false;
		region_depth_ = first;
		region_entry_ = entry;
		++regions_opened_;
		open_serial_ = regions_opened_;
	}
	if (initialization != nullptr)
		complete_static_initialization(*initialization, first);
	destroy_temporaries(first, true, destroyed ? &completed : nullptr);
	completed_landing_ = false;
	if (region)
		end_full_expression(region);
	else
		end_construction();
	if (static_local_)
		return;
	if (inert)
	{
		if (!result)
			lowering_.root_destructor(object_class(type)->destructor);
	}
	else if (destroyed)
	{
		cleanups_.push_back(made);
		cleanups_.back().result = result;
	}
}

void FunctionLowering::construct_array(const sema::Type* type, const Place& place, const syntax::Initializer* init,
	const sema::VariableEntity* variable, uint32_t location)
{
	const sema::FunctionEntity* constructor = variable->constructor;
	// An array of class objects: an aggregate initializer element by
	// element; otherwise each element default-initialized, in order.
	if (init != nullptr)
	{
		const syntax::BracedInitList* list = BracedList(*init);
		if (list == nullptr)
			throw LoweringError(location, "an array of class objects initialized from an expression");
		// A namespace-scope array's elements are addressed from its symbol
		// for each store, as its subscripts would be.
		if (place.storage.is(lowir::ValueKind::Symbol))
			aggregate_array(
				type, AddressPath{ AddressPath::Root::Storage, place.storage, path_steps_.size() }, list, location);
		else
			aggregate_array_object(type, place, *list, variable, location);
		return;
	}
	if (constructor == nullptr || constructor->trivial)
		return;
	// A multidimensional array's elements in order, as one sequence.
	uint64_t count = 1;
	const sema::Type* element = type;
	while (element->kind == sema::TypeKind::Array)
	{
		count *= element->bound;
		element = element->element;
	}
	construct_elements(element, ArrayBase{ place, nullptr }, count, constructor, location);
}

// ------------------------------------------------ namespace-scope objects

void FunctionLowering::construct_global(lowir::SymbolId global, const sema::VariableEntity* variable,
	const syntax::Initializer* initializer, uint32_t location)
{
	// 3.6.2: a class object (or array of them) of static storage duration
	// is constructed by the initialization function, in declaration order.
	Place place = Place();
	place.storage = lowir::Value::symbol(global);
	const sema::Type* type = variable->type;
	const syntax::BracedInitList* list = initializer != nullptr ? BracedList(*initializer) : nullptr;
	if (list == nullptr && variable->constructor == nullptr)
		list = elided_list(initializer, type);
	// Its aggregate reference members' temporaries persist with it
	// (12.2p5); a thread's own object keeps them in frames (see
	// temporary_storage). The initializer's other temporaries end with it
	// (1.9p10, 12.2p3).
	static_object_ = !variable->is_thread_local;
	const size_t first = cleanups_.size();
	begin_static_initializer();
	const ProgramLowering::TemporaryNames names = lowering_.begin_temporary_names(variable, global);
	const syntax::Expr* single = initializer == nullptr ? nullptr
		: list != nullptr                               ? list
		: initializer->kind == syntax::InitializerKind::Parenthesized
		? (initializer->arguments.size == 1 ? initializer->arguments[0] : nullptr)
		: initializer->expression;
	const syntax::Expr* saved = full_root(single);
	if (type->kind == sema::TypeKind::Array)
		construct_array(type, place, initializer, variable, location);
	else if (list != nullptr && list->facts != nullptr && (list->facts->flags & sema::kInitializerList) != 0 &&
		variable->constructor == nullptr)
		initializer_list_object(*list, address_of(place), sema::ClassOf(type), variable, false, location);
	else if (list != nullptr && variable->constructor == nullptr)
	{
		// The initialization function takes the object's address per
		// member only (the course's form).
		aggregate_object(
			type, AddressPath{ AddressPath::Root::Storage, place.storage, path_steps_.size() }, list, false, location);
	}
	else
		construct(type, address_of(place), initializer, variable->constructor, EntryVariant::Complete, location);
	choice_root_ = saved;
	lowering_.end_temporary_names(names);
	destroy_temporaries(first);
	static_initializer_ = false;
	static_object_ = false;
}

void FunctionLowering::finalize_global(const Finalizer& finalizer)
{
	// 3.6.3p1: destroyed in reverse order of construction, an array's
	// elements last to first (unrolled up to eight).
	Place storage = Place();
	storage.storage = lowir::Value::symbol(finalizer.global);
	const lowir::Value callee = lowir::Value::symbol(finalizer.destructor);
	lowir::BlockId skip = lowir::kNoId;
	if (finalizer.constructed != lowir::kNoId)
	{
		// A static reference's temporary, if it was made.
		const lowir::Type i8 = lowir::Type::scalar(lowir::TypeKind::I8);
		Place flag = Place();
		flag.storage = lowir::Value::symbol(finalizer.constructed);
		const lowir::BlockId destroy_block = new_block(lowir::BlockKind::RefDestroy);
		skip = new_block(lowir::BlockKind::RefDestroyDone);
		branch(load(i8, flag), destroy_block, skip);
		place(destroy_block);
	}
	else if (finalizer.guard != lowir::kNoId)
	{
		// The thread's flag, read through its address.
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		Place flag = Place();
		flag.storage = lowir::Value::symbol(finalizer.guard);
		Place reached = Place();
		reached.storage = address_of(flag);
		const lowir::BlockId destroy_block = new_block(lowir::BlockKind::LocalStaticDestroy);
		skip = new_block(lowir::BlockKind::LocalStaticFiniNext);
		branch(compare(lowir::ComparePredicate::Ne, i64, load(i64, reached), integer(i64, 0)), destroy_block, skip);
		place(destroy_block);
	}
	if (finalizer.count > 8)
	{
		// More than eight elements: a loop from the last down, the array's
		// address taken once (as for an automatic array).
		const lowir::Type i64 = lowir::Type::scalar(lowir::TypeKind::I64);
		const lowir::Value base = address_of(storage);
		Place index_slot = Place();
		index_slot.storage = lowir::Value::slot(generated_slot(lowir::SlotKind::DestructorArrayIndex, i64));
		store(i64, integer(i64, finalizer.count), index_slot);
		const lowir::BlockId condition = new_block(lowir::BlockKind::DestructorArrayCond);
		const lowir::BlockId body = new_block(lowir::BlockKind::DestructorArrayBody);
		const lowir::BlockId end = new_block(lowir::BlockKind::DestructorArrayEnd);
		jump(condition);
		place(condition);
		const RValue remaining = load(i64, index_slot);
		branch(compare(lowir::ComparePredicate::Ne, i64, remaining, integer(i64, 0)), body, end);
		place(body);
		const RValue last = binary(lowir::BinaryOp::Sub, i64, remaining, integer(i64, 1));
		store(i64, last, index_slot);
		RValue offset = last;
		if (finalizer.element_size != 1)
			offset = binary(lowir::BinaryOp::Mul, i64, offset, integer(i64, finalizer.element_size));
		lowir::Value object =
			index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base, offset).value;
		++lowering_.counters().instructions;
		builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), callee, &object, 1, lowir::kNoId, 0);
		jump(condition);
		place(end);
	}
	// (A reference's temporary array: its address taken once, each element
	// at its constant offset, the course's form.)
	const lowir::Value base = finalizer.reference_temporary && finalizer.array && finalizer.count <= 8
		? address_of(storage)
		: lowir::Value::none();
	for (uint64_t i = finalizer.count; finalizer.count <= 8 && i-- > 0;)
	{
		lowir::Value object = base.is(lowir::ValueKind::None) ? address_of(storage) : base;
		if (!base.is(lowir::ValueKind::None))
			object = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_ARRAY_ELEMENT, base,
				integer(lowir::Type::scalar(lowir::TypeKind::I64), i * finalizer.element_size))
						 .value;
		else if (finalizer.array)
			object = element_address(object, i, finalizer.element_size);
		++lowering_.counters().instructions;
		if (terminated_)
			place(new_block(lowir::BlockKind::Unreachable));
		builder_.call(lowir::Type::scalar(lowir::TypeKind::Void), callee, &object, 1, lowir::kNoId, 0);
	}
	if (skip != lowir::kNoId)
	{
		jump(skip);
		place(skip);
	}
}

// --------------------------------------------- constructor and destructor bodies

lowir::Value FunctionLowering::subobject_address(
	const sema::BaseClass* base, const sema::DataMember* member, uint32_t location)
{
	// A base subobject at its offset; a member at its byte offset.
	const lowir::Value object = this_pointer(location);
	if (base != nullptr)
		return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, object,
			integer(lowir::Type::scalar(lowir::TypeKind::I64), base->offset))
			.value;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
	const lowir::Value address = index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_FIELD, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), this_member_offset(variable, location)))
									 .value;
	if (union_member(variable))
		mark_untyped(address);
	return address;
}

uint64_t FunctionLowering::this_holder_offset(const sema::VariableEntity* member, uint32_t location)
{
	uint64_t anonymous = 0;
	lowering_.member_holder(member, sema::ClassOf(function_->owner->entity->type), anonymous, location);
	return anonymous;
}

uint64_t FunctionLowering::this_member_offset(const sema::VariableEntity* member, uint32_t location)
{
	return this_holder_offset(member, location) + lowering_.member_offset(member, location);
}

lowir::Value FunctionLowering::anonymous_object(
	lowir::Value object, const sema::ClassEntity*& from, const sema::ClassEntity* to, uint32_t location)
{
	// Both objects' offsets in the named class holding them.
	if (from == to || from == nullptr || to == nullptr || (!from->anonymous && !to->anonymous))
		return object;
	uint64_t from_offset = 0, to_offset = 0;
	if (lowering_.anonymous_home(from, from_offset, location) != lowering_.anonymous_home(to, to_offset, location))
		return object;
	from = to;
	if (from_offset == to_offset)
		return object;
	return index(lowir::Type::scalar(lowir::TypeKind::I8), ir_model::IPK_NONE, object,
		integer(lowir::Type::scalar(lowir::TypeKind::I64), to_offset - from_offset))
		.value;
}

lowir::Value FunctionLowering::object_pointer_as(const sema::ClassEntity* typed, uint32_t location)
{
	const sema::ClassEntity* from = object_class();
	return anonymous_object(object_pointer(location), from, typed, location);
}

FunctionLowering::OverlapScope::OverlapScope(bool& flag, const sema::DataMember* member) : flag_(flag), saved_(flag)
{
	const sema::ClassEntity* cls = member != nullptr && member->overlapping ? sema::ClassOf(member->variable->type)
																			: nullptr;
	flag_ = flag_ || (cls != nullptr && cls->empty);
}

void FunctionLowering::initialize_subobjects(uint32_t location)
{
	// 12.6.2p10: the direct base, then the members in declaration order,
	// each by its mem-initializer, brace-or-equal-initializer or
	// default-initialization.
	bit_unit_ = 0;
	bit_high_ = 0;
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	if (function_->delegation != nullptr)
	{
		// 12.6.2p6: the target constructor's entry of this entry's kind on
		// the object, a full-expression (p7); once it completes the object
		// is destroyed if the body throws (15.2p2).
		const sema::Type* type = function_->owner->entity->type;
		syntax::Initializer view = syntax::Initializer();
		const syntax::Initializer* written = mem_initializer_view(function_->delegation, view);
		const size_t first = cleanups_.size();
		const bool region = begin_initializer(written, function_->delegate);
		// A base-object entry passes its own VTT (and in the course's
		// convention virtual bases) on.
		if (base_entry_ && VirtualBaseCount(sema::ClassOf(type)) != 0)
			hidden_base_ = HiddenBase{ true, 0 };
		construct(type, this_pointer(location), written, function_->delegate,
			base_entry_ ? EntryVariant::Base : EntryVariant::Complete, location);
		hidden_base_.active = false;
		destroy_temporaries(first);
		end_full_expression(region);
		if (needs_destruction(type))
			subobject_constructed(Subobject{ nullptr, nullptr, -1 }, location);
		return;
	}
	bit_limit_ = subobject_bit_limit();
	// Until its tables are stored, `this`'s virtual bases are found without
	// them (see virtual_base_address).
	tables_pending_ = VirtualBaseCount(entity) != 0;
	// 12.6.2p10: the complete object's constructor initializes the virtual
	// bases first (a base-object constructor leaves them to it).
	if (!base_entry_)
	{
		const std::vector<uint32_t>& planned = virtual_base_inits(entity);
		for (const sema::BaseClass* shared : virtual_base_order(entity))
		{
			const uint32_t i = planned[lowering_.virtual_base_position(entity, shared->base)];
			if (i != PointerMap::kMissing)
				construct_base(function_->inits[i], i, location);
		}
	}
	// 12.7p4: once the bases are constructed the object is of this class
	// (its table stored before the members are initialized).
	bool table_stored = false;
	for (uint32_t i = 0; i < function_->init_count; ++i)
	{
		const sema::SubobjectInit& init = function_->inits[i];
		if (init.base != nullptr && init.base->is_virtual)
			continue;
		if (init.base == nullptr && !table_stored)
		{
			tables_stored(location);
			table_stored = true;
		}
		if (init.member == nullptr || !init.member->bit_field)
			bit_unit_ = 0;
		if (init.base != nullptr)
		{
			construct_base(init, i, location);
			continue;
		}
		const size_t first = cleanups_.size();
		syntax::Initializer member_view = syntax::Initializer();
		const bool region = begin_initializer(init.mem_initializer != nullptr
				? mem_initializer_view(init.mem_initializer, member_view)
				: init.default_initializer,
			init.constructor);
		// 12.2p5: the temporaries its aggregate reference members bind, and
		// its std::initializer_list objects' arrays (8.5.4p6), persist until
		// the constructor exits.
		const sema::Entity* saved_extension = aggregate_extension_;
		aggregate_extension_ = function_;
		// A brace-or-equal-initializer's source location builtins report
		// the constructor running it (its body; an implicit one has none).
		const uint32_t saved_site = source_site_;
		if (init.mem_initializer == nullptr && function_->body != nullptr && source_site_ == kNoSourceSite)
			source_site_ = location;
		const OverlapScope overlap(overlapping_empty_, init.member);
		if (initialize_member(init, location))
			bit_high_ = std::max<uint64_t>(bit_high_, member_end_bits(init.member, location));
		source_site_ = saved_site;
		aggregate_extension_ = saved_extension;
		destroy_temporaries(first);
		end_full_expression(region);
		// A union's members are variant members: never destroyed
		// implicitly (12.4p8, 15.2p2).
		const sema::DataMember* member = init.member;
		const sema::ClassEntity* owner = sema::ClassOf(function_->owner->entity->type);
		if (owner->key != TokenKind::KW_UNION && member->variable->kind == sema::EntityKind::Variable &&
			member->variable->name != nullptr && needs_destruction(member->variable->type))
			subobject_constructed(Subobject{ nullptr, member, -1 }, location);
	}
	if (!table_stored)
		tables_stored(location);
	bit_limit_ = UINT64_MAX;
}

void FunctionLowering::tables_stored(uint32_t location)
{
	tables_pending_ = false;
	if (vtt_param_.kind == lowir::ValueKind::None)
		this_values_.clear();
	store_vptr(location);
}

void FunctionLowering::construct_base(const sema::SubobjectInit& init, uint32_t index, uint32_t location)
{
	// A base subobject by its mem-initializer, its inherited constructor
	// or its default constructor, through its base-object entry (with the
	// base's part of the VTT when the base has virtual bases).
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	const sema::ClassEntity* base = static_cast<const sema::ClassEntity*>(init.base->base);
	if (init.forwards || init.mem_initializer != nullptr || (init.constructor != nullptr && !init.constructor->trivial))
	{
		// A base's construction writes within its non-virtual size (an
		// empty base at offset 0 after another lowers no mark).
		bit_high_ = std::max<uint64_t>(bit_high_, (init.base->offset + base->nvsize) * 8);
	}
	if (VirtualBaseCount(base) != 0)
		hidden_base_ = HiddenBase{ true,
			init.base->is_virtual ? lowering_.vtt_virtual_index(entity, base)
								  : lowering_.vtt_base_index(entity, init.base) };
	if (init.forwards)
	{
		forward_parameters(init.constructor, subobject_address(init.base, nullptr, location), location);
		hidden_base_.active = false;
		if (needs_destruction(base->type))
			subobject_constructed(Subobject{ init.base, nullptr, -1 }, location);
		return;
	}
	// An empty base overlaps the members: its value-initialization writes
	// no byte (only a constructor that is not trivial runs).
	const bool value_initialized = init.mem_initializer != nullptr && init.mem_initializer->arguments.empty() &&
		(init.mem_initializer->list == nullptr || init.mem_initializer->list->elements.empty());
	const bool empty_value = base->empty && value_initialized;
	const bool runs = empty_value
		? init.constructor != nullptr && !init.constructor->trivial
		: init.mem_initializer != nullptr || (init.constructor != nullptr && !init.constructor->trivial);
	if (runs)
	{
		// 12.6.2p7: each mem-initializer is a full-expression.
		syntax::Initializer view = syntax::Initializer();
		const syntax::Initializer* written = mem_initializer_view(init.mem_initializer, view);
		const size_t first = cleanups_.size();
		const bool region = begin_initializer(written, init.constructor);
		const bool saved = passing_empty_;
		passing_empty_ = empty_value;
		// Its zeroing or storage copy (a trivial copy or move constructor,
		// or a class prvalue initializing it) writes its data only when its
		// tail is not its own: such a prvalue initializes a temporary whose
		// data is copied (one to destroy initializes the base itself).
		const syntax::MemInitializer* given = init.mem_initializer;
		const syntax::Expr* single = given == nullptr                   ? nullptr
			: given->arguments.size == 1                                ? given->arguments[0]
			: given->list != nullptr && given->list->elements.size == 1 ? given->list->elements[0]
																		: nullptr;
		const sema::ExprFacts* facts = single != nullptr ? WithoutParentheses(single)->facts : nullptr;
		const bool prvalue = facts != nullptr && facts->category == sema::ValueCategory::PRValue &&
			sema::ClassOf(facts->type) == base;
		const bool stored = value_initialized ||
			(init.constructor != nullptr ? copies_storage(init.constructor) : prvalue);
		zero_data_only_ = stored && tail_occupied(index, location);
		const bool temporary = zero_data_only_ && prvalue && !base->empty && !needs_destruction(base->type);
		if (prvalue)
			zero_data_only_ = false;
		if (temporary)
		{
			// The base's address first, then the value made in the
			// temporary, its nvsize bytes copied (the course's order).
			const lowir::Value subobject = subobject_address(init.base, nullptr, location);
			Place slot = Place();
			slot.storage = lowir::Value::slot(
				generated_slot(lowir::SlotKind::Baseobj, lowering_.storage_type(base->type)));
			const lowir::Value made = address_of(slot);
			construct(base->type, made, written, init.constructor, EntryVariant::Complete, location);
			copy_span(made, subobject, base->nvsize, lowering_.alignment_of(base->type, location));
		}
		else
			construct(base->type, subobject_address(init.base, nullptr, location), written, init.constructor,
				EntryVariant::Base, location);
		zero_data_only_ = false;
		passing_empty_ = saved;
		destroy_temporaries(first);
		end_full_expression(region);
		// The course emits a user-provided base constructor's complete
		// entry with the base entry a polymorphic class uses, unless the
		// base is polymorphic with a non-virtual destructor, and with any
		// base entry a class with virtual bases uses. A host object defines
		// only the entries it calls (README PA31: demand-driven emission;
		// each entry is a COMDAT group of its own).
		const sema::FunctionEntity* constructor = init.constructor;
		if (!lowering_.host_objects() && constructor != nullptr && !constructor->implicit && !constructor->defaulted &&
			((entity->polymorphic &&
				 (!base->polymorphic || (base->destructor != nullptr && base->destructor->is_virtual))) ||
				VirtualBaseCount(entity) != 0))
			lowering_.demand(constructor, EntryVariant::Complete);
	}
	hidden_base_.active = false;
	// 15.2p2: a constructed subobject is destroyed if a later
	// initialization throws.
	if (needs_destruction(base->type))
		subobject_constructed(Subobject{ init.base, nullptr, -1 }, location);
}

const std::vector<const sema::BaseClass*>& FunctionLowering::virtual_base_order(const sema::ClassEntity* entity)
{
	// The class's virtual bases in construction order (see
	// ProgramLowering::virtual_base_order).
	std::vector<const sema::BaseClass*>& order = virtual_order_;
	const sema::BaseClass* const* known = lowering_.virtual_base_order(entity);
	order.assign(known, known + VirtualBaseCount(entity));
	return order;
}

const std::vector<uint32_t>& FunctionLowering::virtual_base_inits(const sema::ClassEntity* entity)
{
	// The first initialization of each virtual base in the plan (a direct
	// one's among the bases, an indirect one's after the members).
	std::vector<uint32_t>& inits = virtual_inits_;
	inits.assign(VirtualBaseCount(entity), static_cast<uint32_t>(PointerMap::kMissing));
	for (uint32_t i = 0; i < function_->init_count; ++i)
	{
		const sema::BaseClass* base = function_->inits[i].base;
		const uint32_t position = base != nullptr && base->is_virtual
			? lowering_.virtual_base_position(entity, base->base)
			: PointerMap::kMissing;
		if (position != PointerMap::kMissing && inits[position] == PointerMap::kMissing)
			inits[position] = i;
	}
	return inits;
}

void FunctionLowering::forward_parameters(
	const sema::FunctionEntity* constructor, lowir::Value object, uint32_t location)
{
	// 12.9p8: the inherited base constructor (its base entry) with this
	// constructor's parameters as its arguments (a class object by value
	// passed as its slot), then the default arguments of the parameters
	// this constructor omits (12.9p1).
	const size_t base = arguments_.size();
	arguments_.push_back(object);
	const sema::Type* type = function_->type->canonical;
	const sema::Type* inherited = constructor->type->canonical;
	for (uint32_t i = 0; i < type->param_count && i < parameter_slots_.size(); ++i)
	{
		const sema::Type* parameter = type->params[i];
		Place place = Place();
		place.storage = lowir::Value::slot(parameter_slots_[i]);
		if (!parameter->is_reference() && parameter->unqualified->kind == sema::TypeKind::Class)
		{
			arguments_.push_back(place.storage);
			continue;
		}
		const lowir::Type slot_type = parameter->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
																: lowering_.storage_type(parameter);
		arguments_.push_back(load(slot_type, place).value);
	}
	for (uint32_t i = type->param_count; i < inherited->param_count; ++i)
	{
		if (constructor->default_arguments == nullptr || constructor->default_arguments[i] == nullptr)
			throw LoweringError(location, "an inherited constructor without an argument for a parameter");
		push_default_argument(constructor, i, inherited->params[i], location);
	}
	if (ProgramLowering::HiddenBaseParameters(constructor, EntryVariant::Base))
		hidden_base_arguments(base, location);
	lowering_.demand(constructor, EntryVariant::Base);
	const lowir::Value callee = lowir::Value::symbol(lowering_.entry_symbol(constructor, EntryVariant::Base));
	emit_call(lowir::Type::scalar(lowir::TypeKind::Void), callee, base, lowir::kNoId, call_may_throw(constructor));
}

const syntax::Initializer* FunctionLowering::mem_initializer_view(
	const syntax::MemInitializer* written, syntax::Initializer& view)
{
	if (written == nullptr)
		return nullptr;
	view.kind = written->braced ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
	view.expression = written->list;
	view.arguments = written->arguments;
	view.location = written->location;
	return &view;
}

bool FunctionLowering::tail_occupied(uint32_t index, uint32_t location)
{
	// A base subobject's bytes past its data (its non-virtual size) are not
	// its own: a class with virtual bases (its sizeof places them, a base
	// subobject's are the complete object's, initialized on their own), a
	// non-virtual base whose tail padding passes what the constructor's
	// subobjects may write (see subobject_bit_limit), or one holding a
	// subobject initialized before it: a primary base is laid out first
	// (Itanium 2.4), a base declared before it after its data.
	const sema::BaseClass* base = function_->inits[index].base;
	const sema::ClassEntity* cls = static_cast<const sema::ClassEntity*>(base->base);
	if (VirtualBaseCount(cls) != 0)
		return true;
	const uint64_t begin = base->offset + cls->nvsize;
	const uint64_t end = base->offset + lowering_.size_of(base->base->type, location);
	if (!base->is_virtual && end * 8 > bit_limit_)
		return true;
	uint64_t from = 0, to = 0;
	for (uint32_t i = 0; i < index; ++i)
	{
		subobject_bytes(function_->inits[i], from, to, location);
		if (from < end && to > begin)
			return true;
	}
	return false;
}

uint64_t FunctionLowering::subobject_bit_limit()
{
	// The bit a constructor's or assignment's writes of its non-virtual
	// subobjects' bytes (a bit-field's whole unit, a base's tail padding)
	// may not pass. An assignment's object, and a base entry's that may be
	// called (a trivial constructor's never is), may be a base subobject
	// whose tail padding holds the derived object's virtual bases or a
	// base declared before its primary base (Itanium 2.4), initialized
	// before it (12.6.2p10, 12.8p15): its non-virtual size. A complete
	// object's virtual bases, initialized first, start at the lowest
	// non-empty one's offset.
	const sema::ClassEntity* entity = sema::ClassOf(function_->owner->entity->type);
	const bool assignment = function_->copy_kind == sema::CopyKind::CopyAssignment ||
		function_->copy_kind == sema::CopyKind::MoveAssignment;
	if (assignment || (base_entry_ && !function_->trivial))
		return entity->nvsize * 8;
	uint64_t limit = UINT64_MAX;
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const sema::BaseClass* base = VirtualBases(entity)[i].base;
		if (!static_cast<const sema::ClassEntity*>(base->base)->empty)
			limit = std::min(limit, base->offset * 8);
	}
	return limit;
}

bool FunctionLowering::initialize_member(const sema::SubobjectInit& init, uint32_t location)
{
	const sema::DataMember* member = init.member;
	if (init.inner != nullptr)
	{
		// An anonymous union's or struct's member object through its
		// members' plan (9.5p5), their bit-fields placed from its start.
		const uint64_t high = bit_high_;
		bit_unit_ = 0;
		bit_high_ = 0;
		bool wrote = false;
		for (uint32_t i = 0; i < init.inner_count; ++i)
		{
			if (init.inner[i].member == nullptr || !init.inner[i].member->bit_field)
				bit_unit_ = 0;
			const OverlapScope overlap(overlapping_empty_, init.inner[i].member);
			if (initialize_member(init.inner[i], location))
			{
				wrote = true;
				bit_high_ = member_end_bits(init.inner[i].member, location);
			}
		}
		bit_unit_ = 0;
		bit_high_ = high;
		return wrote;
	}
	if (member->variable->kind != sema::EntityKind::Variable || member->variable->name == nullptr)
		return false;
	const sema::VariableEntity* variable = static_cast<const sema::VariableEntity*>(member->variable);
	syntax::Initializer view = syntax::Initializer();
	const syntax::Initializer* written = init.mem_initializer != nullptr
		? mem_initializer_view(init.mem_initializer, view)
		: init.default_initializer;
	const sema::Type* type = variable->type;
	if (object_class(type) != nullptr && !type->is_reference())
	{
		if (written == nullptr && (init.constructor == nullptr || init.constructor->trivial))
			return false;
		if (type->kind == sema::TypeKind::Array)
		{
			const sema::Type* element = type;
			uint64_t count = 1;
			while (element->kind == sema::TypeKind::Array)
			{
				count *= element->bound;
				element = element->element;
			}
			if (written != nullptr && written->kind == syntax::InitializerKind::Parenthesized &&
				written->arguments.empty())
			{
				// 8.5p7: each element value-initialized: zeroed unless its
				// default constructor is user-provided, then constructed.
				if (init.constructor == nullptr || !UserProvided(init.constructor))
					zero_object(type, subobject_address(nullptr, member, location), location);
				if (init.constructor != nullptr && !init.constructor->trivial)
					construct_elements(element, ArrayBase{ Place(), member }, count, init.constructor, location);
				return true;
			}
			if (written != nullptr)
			{
				// 8.5.1: element by element (each addressed from `this`),
				// by the constructors the elements select.
				const syntax::BracedInitList* list = BracedList(*written);
				if (list == nullptr)
					throw LoweringError(location, "an array member initialized from an expression");
				const size_t first = path_steps_.size();
				path_steps_.push_back(PathStep{ PathStep::Kind::Field, this_member_offset(variable, location),
					lowir::Type(), 0, union_member(variable) });
				aggregate_array(type, AddressPath{ AddressPath::Root::This, lowir::Value(), first }, list, location);
				path_steps_.resize(first);
				return true;
			}
			construct_elements(element, ArrayBase{ Place(), member }, count, init.constructor, location);
			return true;
		}
		const syntax::BracedInitList* list = written != nullptr ? BracedList(*written) : nullptr;
		const syntax::BracedInitList* elided = init.constructor == nullptr ? elided_list(written, type) : nullptr;
		if (elided != nullptr ||
			(list != nullptr && init.constructor == nullptr && list->facts != nullptr &&
				(list->facts->flags & sema::kInitializerList) != 0))
		{
			if (elided != nullptr)
				list = elided;
			// A std::initializer_list member: its array persists until the
			// constructor exits (8.5.4p6, 12.2p5).
			initializer_list_object(*list, subobject_address(nullptr, member, location), sema::ClassOf(type),
				aggregate_extension_, false, location);
			return true;
		}
		if (list != nullptr && init.constructor == nullptr)
		{
			// An aggregate member, each store addressing it from `this`.
			const size_t first = path_steps_.size();
			path_steps_.push_back(PathStep{ PathStep::Kind::Field, this_member_offset(variable, location),
				lowir::Type(), 0, union_member(variable) });
			aggregate_object(
				type, AddressPath{ AddressPath::Root::This, lowir::Value(), first }, list, false, location);
			path_steps_.resize(first);
			return true;
		}
		construct(type, subobject_address(nullptr, member, location), written, init.constructor, EntryVariant::Complete,
			location);
		return true;
	}
	if (written == nullptr)
		return false;
	if (member->bit_field)
	{
		const syntax::Expr* single = nullptr;
		if (const syntax::BracedInitList* list = BracedList(*written))
			single = list->elements.empty() ? nullptr : list->elements[0];
		else if (written->kind == syntax::InitializerKind::Parenthesized)
			single = written->arguments.empty() ? nullptr : written->arguments[0];
		else
			single = written->expression;
		// A bit-field of an anonymous member object: its unit from the
		// object's start.
		const uint64_t anonymous = this_holder_offset(variable, location);
		const size_t first = path_steps_.size();
		if (anonymous != 0)
			path_steps_.push_back(PathStep{ PathStep::Kind::Field, anonymous, lowir::Type(), 0 });
		initialize_bit_field(
			member, variable, AddressPath{ AddressPath::Root::This, lowir::Value(), first }, single, true, location);
		path_steps_.resize(first);
		return true;
	}
	initialize_scalar_member(variable, member, *written, location);
	return true;
}

void FunctionLowering::initialize_scalar_member(const sema::VariableEntity* variable, const sema::DataMember* member,
	const syntax::Initializer& written, uint32_t location)
{
	// A non-class member from its initializer: the value (or a reference's
	// referent) first, then the member's address (the assignment order).
	const sema::Type* type = variable->type;
	if (type->kind == sema::TypeKind::Array)
	{
		const syntax::BracedInitList* list = BracedList(written);
		if (list != nullptr)
		{
			// Each element addressed again from `this`.
			const size_t first = path_steps_.size();
			path_steps_.push_back(PathStep{ PathStep::Kind::Field, this_member_offset(variable, location),
				lowir::Type(), 0, union_member(variable) });
			aggregate_array(type, AddressPath{ AddressPath::Root::This, lowir::Value(), first }, list, location);
			path_steps_.resize(first);
			return;
		}
		if (written.kind == syntax::InitializerKind::Parenthesized && written.arguments.empty())
		{
			// 8.5p7: each element value-initialized, as `{}` would.
			const size_t first = path_steps_.size();
			path_steps_.push_back(PathStep{ PathStep::Kind::Field, this_member_offset(variable, location),
				lowir::Type(), 0, union_member(variable) });
			aggregate_array(type, AddressPath{ AddressPath::Root::This, lowir::Value(), first }, nullptr, location);
			path_steps_.resize(first);
			return;
		}
		throw LoweringError(location, "an array member initialized from an expression");
	}
	const lowir::Type storage = type->is_reference() ? lowir::Type::scalar(lowir::TypeKind::Ptr)
													 : lowering_.storage_type(type);
	RValue value;
	const syntax::Expr* single = nullptr;
	// A reference's list binds its one element, or is the temporary the
	// list initializes (8.5.4p3, see reference_initializer).
	if (const syntax::BracedInitList* list = BracedList(written))
		single = type->is_reference()     ? reference_initializer(list)
			: sema::LaneList(type, *list) ? list
			: list->elements.empty()      ? nullptr
										  : list->elements[0];
	else if (written.kind == syntax::InitializerKind::Parenthesized)
		single = written.arguments.empty() ? nullptr : written.arguments[0];
	else
		single = written.expression;
	if (type->is_reference())
	{
		if (single == nullptr)
			throw LoweringError(location, "a reference member without an initializer");
		value.value = bind_reference(*single, type, ConversionContext::Initialization);
		value.type = storage;
	}
	else if (single == nullptr)
		value = zero_value(type);
	else
		value = fit(operand(*single, ConversionContext::Initialization), type);
	Place place = Place();
	place.storage = subobject_address(nullptr, member, location);
	qualify(place, type);
	store(storage, value, place);
}

} // namespace lower
