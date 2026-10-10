#include "sema/analyzer.h"

#include "sema/constant_interpreter.h"

// Non-type template arguments of pointer, reference and pointer to member
// type (14.3.2):
// the argument's form, the object or function it designates, its conversion
// to its parameter's type (14.3.2p5), and the name of a parameter an
// instantiation binds to one. An argument is an interned TypeKind::Value
// (see sema/semantic_types.h), so specializations compare such arguments by
// pointer as they compare integral values.
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

bool IsNullptrType(const Type* type)
{
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

// 14.3.2p1: an object of static storage duration with external or internal
// linkage (a namespace-scope variable, a static data member, a block-scope
// extern declaration), or a function that is no non-static member.
bool LinkedEntity(const Entity* entity)
{
	if (entity->kind == EntityKind::Function)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(entity);
		return !function->member || function->is_static;
	}
	const VariableEntity* variable = static_cast<const VariableEntity*>(entity);
	if (variable->member || variable->is_thread_local || variable->owner == nullptr)
		return false;
	const ScopeKind owner = variable->owner->kind;
	return owner == ScopeKind::Namespace || owner == ScopeKind::Class || variable->is_extern;
}

} // namespace

bool AddressParameterType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Pointer || type->kind == TypeKind::LValueReference ||
		type->kind == TypeKind::MemberPointer || IsNullptrType(type);
}

bool AddressArgument(const Type* argument)
{
	return argument->kind == TypeKind::Value && AddressParameterType(argument->element);
}

bool Analyzer::NamesLinkedObject(const syntax::Expr& expr)
{
	// An id-expression (parenthesized or not) naming an object of integral
	// or enumeration type that a reference parameter's argument may
	// designate (14.3.2p1).
	const syntax::Expr* named = WithoutParentheses(&expr);
	const Entity* entity = named->kind == syntax::ExprKind::Id && named->facts != nullptr ? named->facts->entity
																						  : nullptr;
	if (entity == nullptr || entity->kind != EntityKind::Variable || entity->type->is_reference() ||
		!LinkedEntity(entity))
		return false;
	const Type* type = entity->type->unqualified;
	return IsIntegralType(type) || type->kind == TypeKind::Enum;
}

ConstValue ArgumentPointer(const Type* argument)
{
	// The null pointer, or the address of the argument's object (an array's
	// first element when it decayed, 4.2) or function, at offset zero.
	ConstValue value = ConstValue();
	value.kind = ValueKind::Pointer;
	value.type = argument->element;
	const Entity* entity = argument->entity;
	if (value.type->kind == TypeKind::MemberPointer)
	{
		value = NullMemberPointer(argument->element);
		value.address.entity = entity;
		return value;
	}
	value.root = entity == nullptr             ? RootKind::Null
		: entity->kind == EntityKind::Function ? RootKind::Function
											   : RootKind::Variable;
	value.address.entity = entity;
	return value;
}

const Type* Analyzer::address_argument(const syntax::Expr& expr, const Type* target)
{
	// 14.3.2p1: the argument of a pointer or reference parameter (typed as a
	// constant expression's operand) is `&` id-expression or an
	// id-expression naming an object or a function (a set of overloaded
	// functions resolved by the parameter's type `target`, 13.4), a
	// parameter an instantiation bound to one, or a constant expression
	// evaluating to a null pointer. Its own value, which converted_value
	// converts to the parameter's type (an integral one is left for it to
	// reject); null for a substitution's failure.
	ExprFacts* facts = facts_of(expr);
	if ((facts->flags & kOverloadSet) != 0)
	{
		FunctionEntity* function = target != nullptr ? select_overloaded(facts, target) : nullptr;
		if (function == nullptr)
		{
			substitution_error(expr.location, "no overloaded function matches the template parameter's type");
			return nullptr;
		}
		resolve_overloaded(expr, function);
	}
	const syntax::Expr* e = WithoutParentheses(&expr);
	const bool address = e->kind == syntax::ExprKind::Unary &&
		static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_AMP && (e->facts->flags & kOperatorCall) == 0;
	const syntax::Expr* named = address ? WithoutParentheses(static_cast<const syntax::UnaryExpr*>(e)->operand) : e;
	const Entity* entity = named->kind == syntax::ExprKind::Id && named->facts != nullptr ? named->facts->entity
																						  : nullptr;
	if (!address && entity != nullptr && entity->kind == EntityKind::Enumerator &&
		static_cast<const EnumeratorEntity*>(entity)->argument != nullptr)
		return static_cast<const EnumeratorEntity*>(entity)->argument;
	// An object of pointer or std::nullptr_t type named without `&` is no
	// pointer parameter's designation (only a reference parameter's) but a
	// constant expression, valid when it evaluates to a null pointer value.
	const Type* named_type = entity != nullptr && entity->kind == EntityKind::Variable ? named->facts->type->unqualified
																					   : nullptr;
	if (!address && named_type != nullptr &&
		(named_type->kind == TypeKind::Pointer || IsNullptrType(named_type) ||
			named_type->kind == TypeKind::MemberPointer) &&
		(target == nullptr || !target->is_reference()))
		entity = nullptr;
	// A pointer to member expressed as `&X::m` (5.3.1p3), which designates
	// the member.
	if (address && e->facts->type != nullptr && e->facts->type->unqualified->kind == TypeKind::MemberPointer)
	{
		if (entity == nullptr || (entity->kind != EntityKind::Variable && entity->kind != EntityKind::Function))
		{
			substitution_error(e->location, "a pointer to member template argument designates no member");
			return nullptr;
		}
		note_argument_demand(entity);
		return types_.address(e->facts->type->unqualified->canonical, const_cast<Entity*>(entity));
	}
	if (entity != nullptr && (entity->kind == EntityKind::Variable || entity->kind == EntityKind::Function))
	{
		const char* failure = nullptr;
		if (!LinkedEntity(entity))
			failure = "a template argument designates no object of static storage duration with linkage";
		else if (!address && entity->type->is_reference())
			failure = "a reference named as a template argument (14.3.2p1 names an object)";
		if (failure != nullptr)
		{
			substitution_error(named->location, failure);
			return nullptr;
		}
		Entity* designated = const_cast<Entity*>(entity);
		// 3.2p2: the object a pointer or reference parameter's argument
		// designates is odr-used (no lvalue-to-rvalue conversion reads it;
		// an integral one named for an unknown parameter may yet be read):
		// an instance's static data member is then defined (14.7.1p2), its
		// initializer run.
		const VariableEntity* variable = entity->kind == EntityKind::Variable
			? static_cast<const VariableEntity*>(entity)
			: nullptr;
		const bool readable = !address && target == nullptr && variable != nullptr &&
			(IsIntegralType(variable->type->unqualified) || variable->type->unqualified->kind == TypeKind::Enum);
		if (variable != nullptr && !readable && variable->templ != nullptr)
			demand_variable(variable);
		note_argument_demand(entity);
		if (address)
			return types_.address(e->facts->type->unqualified->canonical, designated);
		return types_.address(types_.reference(named->facts->type, false)->canonical, designated);
	}
	if (address)
	{
		substitution_error(
			e->location, "a template argument's address of something that is no named object or function");
		return nullptr;
	}
	ConstantInterpreter interpreter(*this);
	ConstValue value;
	if (!interpreter.evaluate(expr, false, value))
	{
		substitution_error(interpreter.failure() != nullptr ? interpreter.failure_location() : expr.location,
			interpreter.failure() != nullptr ? interpreter.failure()
											 : "a template argument that is not a constant expression");
		return nullptr;
	}
	if (value.kind == ValueKind::Integer)
		return types_.value(value.type->unqualified, value.bits);
	if (value.kind == ValueKind::Pointer && value.root == RootKind::Null)
		return types_.address(value.type->unqualified->canonical, nullptr);
	if (value.kind == ValueKind::MemberPointer && value.address.entity == nullptr)
		return types_.address(value.type->unqualified->canonical, nullptr);
	substitution_error(expr.location, "a template argument that designates no named object or function");
	return nullptr;
}

void Analyzer::note_argument_demand(const Entity* entity)
{
	// 3.2p2: a function a template argument designates is odr-used where
	// the argument appears (in a template's instantiation, not its
	// definition, 14.7.1); no lowered code names it, so it is listed by the
	// definition whose body names it (none outside a body) for the
	// lowering to demand where that is emitted.
	if (entity == nullptr || entity->kind != EntityKind::Function || unevaluated_ != 0 || template_depth_ != 0)
		return;
	// Each (context, function) is listed once, found by one probe.
	const FunctionEntity* function = static_cast<const FunctionEntity*>(entity);
	const FunctionEntity* context = body_ != nullptr ? body_->function : nullptr;
	const EntityPair key = EntityPair{ context, function };
	if (argument_demanded_.find(key) != nullptr)
		return;
	argument_demanded_.insert(key, true);
	const uint32_t head = argument_demand_heads_.find(context);
	argument_demands_.push_back(ArgumentDemand{ function, head == PointerMap::kMissing ? 0 : head });
	argument_demand_heads_.set(context, static_cast<uint32_t>(argument_demands_.size()));
}

uint32_t Analyzer::argument_demand_head(const FunctionEntity* context) const
{
	const uint32_t head = argument_demand_heads_.find(context);
	return head == PointerMap::kMissing ? 0 : head;
}

const Type* Analyzer::converted_address(const Type* target, const Type* argument, bool fatal, uint32_t location)
{
	// 14.3.2p5 for a parameter of pointer, reference or std::nullptr_t type
	// `target` (unqualified): an object pointer takes qualification
	// conversions (4.4) and the array-to-pointer conversion (4.2), a
	// function pointer the function-to-pointer conversion (4.3), either
	// the null pointer conversion of a std::nullptr_t value (4.10); a
	// reference binds directly to an lvalue whose type it may qualify
	// further. No integer converts (not even a null pointer constant), nor
	// does such an argument to an integral or enumeration type (5.19p3).
	target = target->canonical;
	const Type* source = argument->element;
	Entity* entity = argument->entity;
	const char* failure = nullptr;
	if (!AddressArgument(argument))
		failure = "an integral template argument given for a pointer or reference parameter";
	else if (!AddressParameterType(target))
		failure = "a pointer or reference template argument given for an integral or enumeration parameter";
	else if (IsNullptrType(target))
	{
		if (entity != nullptr || !IsNullptrType(source))
			failure = "a std::nullptr_t template parameter's argument is not nullptr";
	}
	else if (target->kind == TypeKind::MemberPointer)
	{
		// A pointer to member function's argument takes no conversion but
		// std::nullptr_t's null member pointer conversion; a pointer to data
		// member's, qualification conversions too (4.4, 4.11).
		const bool same = source == target ||
			(target->element->kind != TypeKind::Function && source->kind == TypeKind::MemberPointer &&
				ConversionRules::qualification_convertible(source, target));
		if (!(entity == nullptr && IsNullptrType(source)) && !same)
			failure = "a template argument does not convert to its pointer to member parameter's type";
	}
	else if (target->kind == TypeKind::LValueReference)
	{
		const Type* referred = target->element;
		if (entity == nullptr || !source->is_reference())
			failure = "a reference template parameter's argument names no object or function";
		else if (source->element->unqualified != referred->unqualified || (source->element->cv & ~referred->cv) != 0)
			failure = "a reference template parameter's argument of another type";
	}
	else if (entity == nullptr)
	{
		if (!IsNullptrType(source) && source != target &&
			!(source->kind == TypeKind::Pointer && ConversionRules::qualification_convertible(source, target)))
			failure = "a null pointer template argument of another pointer type";
	}
	else
	{
		const Type* pointer = source;
		if (source->is_reference())
		{
			const Type* referred = source->element;
			pointer = referred->kind == TypeKind::Array ? types_.pointer(referred->element)
				: referred->kind == TypeKind::Function  ? types_.pointer(referred)
														: nullptr;
		}
		if (pointer == nullptr)
			failure = "a pointer template parameter's argument is an object without `&`";
		else if (pointer->canonical != target &&
			(target->element->kind == TypeKind::Function ||
				!ConversionRules::qualification_convertible(pointer->canonical, target)))
			failure = "a template argument does not convert to its pointer parameter's type";
	}
	if (failure != nullptr)
	{
		if (fatal)
			error(location, failure);
		return nullptr;
	}
	return types_.address(target, entity);
}

ExprFacts* Analyzer::argument_name(const syntax::Expr& expr, const EnumeratorEntity* constant)
{
	// A pointer parameter's name in an instantiation is a prvalue of its
	// argument (14.1p6); a reference parameter's designates the object or
	// function its argument names (an lvalue of the parameter's referred
	// type, which the reference binds directly, 14.3.2p5).
	const Type* argument = constant->argument;
	const Type* type = argument->element;
	if (!type->is_reference())
	{
		ExprFacts* facts = new_facts(expr, type, ValueCategory::PRValue);
		facts->entity = constant;
		if (argument->entity == nullptr)
			facts->flags |= kFolded;
		return facts;
	}
	ExprFacts* facts = new_facts(expr, type->element, ValueCategory::LValue);
	facts->entity = argument->entity;
	if (argument->entity->kind == EntityKind::Function && !callee_name_)
		odr_use(static_cast<FunctionEntity*>(argument->entity));
	return facts;
}

} // namespace sema
