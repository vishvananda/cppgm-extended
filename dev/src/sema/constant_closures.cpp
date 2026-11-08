#include "sema/constant_interpreter.h"

#include "sema/analyzer.h"

// C++17 closure objects in constant evaluation: initialize the members the
// normal lambda analysis recorded, and bind captured names to those members
// while invoking the call operator. No source names are reparsed or inferred.
namespace sema {

bool ConstantInterpreter::closure_object(const syntax::LambdaExpr& expr, const ConstPlace& target)
{
	const ClassEntity* cls = ClassOf(expr.facts->type);
	const ClosureFacts* closure = analyzer_.graph_.closure_facts(cls);
	if (!analyzer_.cxx17_ || closure == nullptr || !analyzer_.literal_type(expr.facts->type))
		return fail(expr.location, "a lambda of nonliteral type or outside C++17 constant evaluation");
	if (closure->this_member != nullptr)
	{
		ConstPlace self, member;
		if (!this_place(self, expr.location) || !member_slot(target, closure->this_member, member, expr.location) ||
			!write(member, PointerTo(self, closure->this_member->type), expr.location))
			return false;
	}
	for (uint32_t i = 0; i < closure->capture_count; ++i)
	{
		ConstPlace member;
		if (!member_slot(target, closure->captures[i].member, member, expr.location) ||
			!closure_capture(closure->captures[i], member, expr.location))
			return false;
	}
	return true;
}

bool ConstantInterpreter::closure_capture(const Capture& capture, const ConstPlace& target, uint32_t location)
{
	const Type* type = capture.member->type;
	if (capture.initializer != nullptr)
		return construct(type, target, capture.initializer, capture.constructor, location);
	ConstPlace source;
	if (!entity_place(capture.entity, capture.entity->type, source, location))
		return false;
	if (type->is_reference())
		return write(target, PointerTo(source, type), location);
	return copy_closure_capture(type, capture.constructor, target, source, location);
}

bool ConstantInterpreter::copy_closure_capture(const Type* type, const FunctionEntity* constructor,
	const ConstPlace& target, const ConstPlace& source, uint32_t location)
{
	if (type->kind == TypeKind::Array && constructor != nullptr && !constructor->trivial)
	{
		// Element constructors address class subobjects directly. Establish
		// the array's indeterminate shape before initializing those members.
		if (!write(target, skeleton(type), location))
			return false;
		const uint64_t stride = size_of(type->element, location);
		for (uint64_t i = 0; i < type->bound; ++i)
		{
			ConstPlace to = target, from = source;
			to.type = from.type = type->element;
			to.offset += static_cast<int64_t>(i * stride);
			from.offset += static_cast<int64_t>(i * stride);
			if (!copy_closure_capture(type->element, constructor, to, from, location))
				return false;
		}
		return true;
	}
	if (type->kind != TypeKind::Array && constructor != nullptr)
	{
		Argument argument = Argument();
		argument.bound = true;
		argument.place = source;
		return run_constructor(constructor, target, &argument, 1, location);
	}
	ConstValue value;
	return read(source, value, location) && write(target, value, location);
}

bool ConstantInterpreter::bind_closure_captures(
	const FunctionEntity* function, const ConstPlace& object, uint32_t location)
{
	if (function->owner == nullptr || function->owner->kind != ScopeKind::Class)
		return true;
	const ClosureFacts* closure = analyzer_.graph_.closure_facts(
		static_cast<const ClassEntity*>(function->owner->entity));
	if (closure == nullptr || (closure->call != function && closure->call != function->primary))
		return true;
	for (uint32_t i = 0; i < closure->capture_count; ++i)
	{
		const Capture& capture = closure->captures[i];
		Local local = Local();
		local.entity = capture.entity;
		if (!member_place(object, capture.member, local.place, location))
			return false;
		locals_.push_back(local);
	}
	return true;
}

} // namespace sema
