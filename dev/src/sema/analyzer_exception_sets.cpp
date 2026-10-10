#include "sema/analyzer.h"

// Dynamic-exception-specifications as sets of types (15.4): their adjusted
// types, named in a member's complete-class context, and the exceptions an
// overrider may allow (15.4p5), each type allowed when a handler of a type
// of the overridden function's set would match it (15.4p8, 15.3p3).
namespace sema {

void Analyzer::resolve_throw_specification(
	FunctionEntity* function, const syntax::FunctionQualifier& qualifier, Scope* scope)
{
	// The types adjusted (15.4p2: cv T as T, an array as a pointer to its
	// element, a function as a pointer to it), none an rvalue reference; a
	// pack expansion kept in a function template (its specializations
	// substitute them, see substitute_throw_specification); an expansion of
	// a pack a frame binds (a class template instance's member) is the
	// pack's elements.
	TypeList list;
	for (uint32_t i = 0; i < qualifier.throw_types.size; ++i)
	{
		const Type* type = type_id(*qualifier.throw_types[i], scope);
		if (type == nullptr || (type->kind == TypeKind::Expansion && !function->templated))
			return;
		if (type->kind == TypeKind::Pack)
		{
			for (uint32_t k = 0; k < type->param_count; ++k)
				list.push_back(AdjustedThrowType(types_, type->params[k]));
			continue;
		}
		if (type->kind == TypeKind::RValueReference)
			error(qualifier.throw_types[i]->location, "an rvalue reference type in an exception-specification");
		if (type->kind != TypeKind::Expansion && !function->templated)
			check_complete_throw_type(AdjustedThrowType(types_, type), qualifier.throw_types[i]->location);
		list.push_back(type->kind == TypeKind::Expansion ? type : AdjustedThrowType(types_, type));
	}
	const Type** types = arena_.allocate_array<const Type*>(list.size());
	std::copy(list.begin(), list.end(), types);
	graph_.set_throw_specification(
		function, SemanticGraph::ThrowSpecification{ types, static_cast<uint32_t>(list.size()) });
}

void Analyzer::check_complete_throw_type(const Type* type, uint32_t location)
{
	// 15.4p2: an adjusted type names no incomplete type, nor a pointer or
	// reference to one other than cv void.
	const Type* object = type->is_reference() ? type->element : type;
	const bool indirect = type->is_reference() || object->unqualified->kind == TypeKind::Pointer;
	if (!type->is_reference() && object->unqualified->kind == TypeKind::Pointer)
		object = object->unqualified->element;
	if (IsVoid(object->unqualified))
	{
		if (!indirect)
			error(location, "void in an exception-specification (15.4p2)");
		return;
	}
	const ClassEntity* cls = ClassOf(object);
	if (cls != nullptr && !require_complete(cls))
		error(location, "an incomplete class type in an exception-specification (15.4p2)");
}

void Analyzer::finish_class_exceptions(size_t first_throw, size_t first_override)
{
	// The outermost class is complete: its members' specifications name
	// their types, then its overriders compare their sets.
	for (size_t i = first_throw; i < pending_throws_.size(); ++i)
		resolve_throw_specification(
			pending_throws_[i].function, *pending_throws_[i].qualifier, pending_throws_[i].scope);
	pending_throws_.resize(first_throw);
	check_exception_overrides(first_override, false);
}

void Analyzer::note_exception_override(const FunctionEntity* function, const FunctionEntity* found, uint32_t location)
{
	// An overridden function whose specification may allow a finite set
	// (a destructor's implicit one included), compared at once unless a
	// class is still being defined.
	if (!found->exception_spec && !found->destructor)
		return;
	exception_overrides_.push_back(ExceptionOverride{ function, found, location });
	if (class_depth_ == 0 && pending_throws_.empty())
		check_exception_overrides(exception_overrides_.size() - 1, false);
}

void Analyzer::check_exception_overrides(size_t first, bool final)
{
	// 15.4p5: an overrider allows only exceptions its overridden function
	// allows; one allowing none is the non-throwing rule's (see overrides).
	// A noexcept-specification not yet evaluated waits for the end of the
	// unit (`final`).
	size_t kept = first;
	for (size_t i = first; i < exception_overrides_.size(); ++i)
	{
		const ExceptionOverride pending = exception_overrides_[i];
		if (!final && (pending.function->noexcept_pending || pending.found->noexcept_pending))
		{
			exception_overrides_[kept++] = pending;
			continue;
		}
		TypeList allowed, thrown;
		if (allowed_exceptions(pending.found, allowed) || allowed.empty())
			continue;
		bool allows = !allowed_exceptions(pending.function, thrown);
		for (size_t t = 0; allows && t < thrown.size(); ++t)
		{
			bool matched = false;
			for (size_t a = 0; !matched && a < allowed.size(); ++a)
				matched = handler_allows(allowed[a], thrown[t]);
			allows = matched;
		}
		if (!allows)
			error(pending.location, "an overrider allows exceptions its overridden function does not");
	}
	exception_overrides_.resize(kept);
}

bool Analyzer::allowed_exceptions(const FunctionEntity* function, TypeList& out)
{
	// The exceptions a function allows: true for all, else `out` gains its
	// set (none: non-throwing). A destructor without an exception-
	// specification has the implicit one (12.4p3, 15.4p14): the union of
	// what its bases' and members' destructors allow.
	if (function_nothrow(function))
		return false;
	if (const SemanticGraph::ThrowSpecification* thrown = graph_.throw_specification(function))
	{
		for (uint32_t i = 0; i < thrown->count; ++i)
			out.push_back(thrown->types[i]);
		return false;
	}
	if (!function->destructor || function->exception_spec || function->owner == nullptr ||
		function->owner->kind != ScopeKind::Class)
		return true;
	const ClassEntity* entity = static_cast<const ClassEntity*>(function->owner->entity);
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const FunctionEntity* inherited = static_cast<const ClassEntity*>(base->base)->destructor;
		if (inherited != nullptr && allowed_exceptions(inherited, out))
			return true;
	}
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const FunctionEntity* inherited =
			static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base)->destructor;
		if (!VirtualBases(entity)[i].direct && inherited != nullptr && allowed_exceptions(inherited, out))
			return true;
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		const Type* type = member->variable->type;
		const ClassEntity* cls = type->is_reference() ? nullptr : ObjectClassOf(type);
		if (cls != nullptr && cls->destructor != nullptr && allowed_exceptions(cls->destructor, out))
			return true;
	}
	return false;
}

bool Analyzer::public_base(const ClassEntity* base, const ClassEntity* derived)
{
	// 15.3p3: an unambiguous public base class (reached through public
	// base-specifiers).
	for (const BaseClass* node = derived->bases; node != nullptr; node = node->next)
	{
		const ClassEntity* cls = static_cast<const ClassEntity*>(node->base);
		if (node->access == Access::Public && (cls == base || public_base(base, cls)))
			return (base_path_facts(base, derived, 0) & kBasePathCount) <= 1;
	}
	return false;
}

bool Analyzer::handler_allows(const Type* handler, const Type* thrown)
{
	// 15.3p3: a handler of type cv T or cv T& matches an exception of type
	// E (an allowed type's referent, unqualified) that is T, or of which T
	// is an unambiguous public base class; a pointer or pointer to member
	// handler, or a const reference to one (DR 388), matches a pointer E
	// converting to it by a standard pointer conversion to an accessible
	// base or void and qualification conversions, and std::nullptr_t.
	const Type* referred = handler->is_reference() ? handler->element : handler;
	const Type* target = referred->canonical->unqualified;
	const Type* exception = (thrown->is_reference() ? thrown->element : thrown)->canonical->unqualified;
	if (target == exception)
		return true;
	const ClassEntity* to = ClassOf(target);
	const ClassEntity* from = ClassOf(exception);
	if (to != nullptr && from != nullptr)
		return conversions_.is_base_of(to, from) && public_base(to, from);
	const bool pointer = target->kind == TypeKind::Pointer || target->kind == TypeKind::MemberPointer;
	if (!pointer || (handler->is_reference() && (referred->cv & (kConst | kVolatile)) != kConst))
		return false;
	if (exception->kind == TypeKind::Fundamental && exception->fundamental == FundamentalType::NullptrT)
		return true;
	if (exception->kind != target->kind)
		return false;
	if (ConversionRules::qualification_convertible(exception, target))
		return true;
	if (target->kind != TypeKind::Pointer || (exception->element->cv & ~target->element->cv) != 0)
		return false;
	const Type* pointee = target->element->unqualified;
	if (pointee->kind == TypeKind::Fundamental && pointee->fundamental == FundamentalType::Void)
		return exception->element->kind != TypeKind::Function;
	to = ClassOf(pointee);
	from = ClassOf(exception->element);
	return to != nullptr && from != nullptr && conversions_.is_base_of(to, from) && public_base(to, from);
}

} // namespace sema
