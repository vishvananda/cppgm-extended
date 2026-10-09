#include "semantic/model/local_contexts.h"

namespace cppgm
{
namespace semantic
{
namespace
{

bool HasTemplateSpecializationOwner(const Program& program, EntityId owner)
{
	for (std::size_t depth = 0; owner != kNoEntity &&
		owner < program.entities.size() && depth < program.entities.size();
		++depth)
	{
		if (program.entities[owner].template_argument_begin != kNoBinding)
			return true;
		owner = program.entities[owner].enclosing_class;
	}
	return false;
}

// A numbered closure of a data member's initializer or of an in-class
// default argument is named in that member or function, and other units
// name it wherever they name the member.
BindingId MemberClosureAnchor(const Program& program, const EntityRecord& closure,
	bool* candidate)
{
	*candidate = true;
	return program.bindings[closure.lambda_member_context].canonical;
}

}

BindingId LocalContextFunction(const Program& program, BindingId function)
{
	function = program.bindings[function].canonical;
	const BindingRecord& binding = program.bindings[function];
	if (!binding.lambda_invocation ||
		binding.lambda_invocation_owner >= program.entities.size())
		return function;
	const BindingId call =
		program.entities[binding.lambda_invocation_owner].lambda_call_operator;
	return call == kNoBinding || call >= program.bindings.size() ? function :
		program.bindings[call].canonical;
}

bool IsInlineOrSpecialization(const Program& program, BindingId function)
{
	const BindingRecord& binding =
		program.bindings[program.bindings[function].canonical];
	return binding.inline_function || binding.weak_odr ||
		binding.template_argument_count != 0 ||
		binding.function_template_specialization ||
		HasTemplateSpecializationOwner(program, binding.member_owner);
}

bool ClosureHasManglingNumber(const Program& program, EntityId closure)
{
	if (closure == kNoEntity || closure >= program.entities.size()) return false;
	const EntityRecord& record = program.entities[closure];
	// A namespace function's default argument's closure is a unit-local
	// `$_<n>`, and so is a non-template class's static data member
	// initializer's, in its class.
	if (record.lambda_member_context != kNoBinding)
	{
		const BindingRecord& member = program.bindings[record.lambda_member_context];
		if (record.lambda_default_argument != 0) return member.member_owner != kNoEntity;
		return member.storage_class != STORAGE_CLASS_STATIC ||
			member.variable_template_specialization ||
			HasTemplateSpecializationOwner(program, member.member_owner);
	}
	const BindingId context = record.local_context;
	return context != kNoBinding && context < program.bindings.size() &&
		IsInlineOrSpecialization(program, LocalContextFunction(program, context));
}

BindingId LocalVisibilityAnchor(const Program& program, BindingId function,
	bool* candidate)
{
	*candidate = false;
	for (std::size_t depth = 0; depth < program.bindings.size(); ++depth)
	{
		function = LocalContextFunction(program, function);
		const EntityId owner = program.bindings[function].member_owner;
		if (owner == kNoEntity || owner >= program.entities.size())
			break;
		const EntityRecord& record = program.entities[owner];
		if (record.lambda_closure)
		{
			if (!ClosureHasManglingNumber(program, owner)) return function;
			if (record.lambda_member_context != kNoBinding)
				return MemberClosureAnchor(program, record, candidate);
		}
		if (record.local_context == kNoBinding ||
			record.local_context >= program.bindings.size())
			break;
		function = record.local_context;
	}
	*candidate = IsInlineOrSpecialization(program, function);
	return function;
}

BindingId EntityVisibilityAnchor(const Program& program, EntityId entity,
	bool* candidate)
{
	*candidate = false;
	if (entity == kNoEntity || entity >= program.entities.size()) return kNoBinding;
	const EntityRecord& record = program.entities[entity];
	if (record.lambda_closure)
	{
		if (!ClosureHasManglingNumber(program, entity)) return kNoBinding;
		if (record.lambda_member_context != kNoBinding)
			return MemberClosureAnchor(program, record, candidate);
	}
	if (record.local_context == kNoBinding ||
		record.local_context >= program.bindings.size())
		return kNoBinding;
	return LocalVisibilityAnchor(program, record.local_context, candidate);
}

}
}
