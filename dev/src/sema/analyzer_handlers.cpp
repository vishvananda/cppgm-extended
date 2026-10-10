#include "sema/analyzer.h"

// Throwing and handling exceptions (clause 15): a throw-expression
// copy-initializes the exception object from its operand, and a handler's
// exception-declaration declares the variable its exception object
// initializes. The facts the lowering reads: the operand's conversion to
// the exception object's type (a class object's constructor, none when
// its prvalue initializes the object itself), a ThrowExpr's facts'
// `target` the exception object's type, and each handler's caught type
// and variable (syntax::ExceptionDeclaration).
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

} // namespace

const Type* Analyzer::throw_operand(const syntax::Expr& operand, Scope* scope, uint32_t location)
{
	// 15.1p3: the exception object's type is the operand's static type,
	// top-level cv-qualifiers removed, an array or function adjusted to a
	// pointer; it is copy-initialized from the operand, a non-volatile
	// automatic object's name (not a function's or handler's parameter)
	// whose scope ends inside the innermost try-block taken first as an
	// rvalue (12.8p32). p5: a class object's destructor is potentially
	// invoked (so accessible and not deleted), and the class complete and
	// not abstract (15.1p4 with 10.4p3).
	ExprFacts* facts = typed(operand, scope);
	if (facts == nullptr)
		return nullptr;
	check_resolved(operand);
	if (facts->type == nullptr || (facts->flags & (kOverloadSet | kBoundMember)) != 0)
		error(location, "a throw operand that is not a value");
	const Type* type = facts->type;
	if (IsVoid(type))
		error(location, "a throw operand of type void (15.1p3)");
	if (type->kind == TypeKind::Array)
		type = types_.pointer(type->element);
	else if (type->kind == TypeKind::Function)
		type = types_.pointer(type);
	type = type->unqualified;
	ClassEntity* cls = type->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->entity) : nullptr;
	if (cls == nullptr)
	{
		if (type->kind == TypeKind::Pointer && !IsVoid(type->element->unqualified) &&
			type->element->kind != TypeKind::Function && !complete_pointee(type, location))
			error(location, "a throw operand pointing to an incomplete type (15.1p3)");
		convert(operand, type, false);
		return type;
	}
	if (!require_complete(cls) || cls->scope == nullptr)
		error(location, "a throw operand of incomplete class type (15.1p3)");
	if (!check_abstract_object(type, location))
		return nullptr;
	check_destruction(type, scope, location);
	expression_scope_ = scope;
	const syntax::Expr* inner = WithoutParentheses(&operand);
	const Entity* named = inner->kind == syntax::ExprKind::Id ? inner->facts->entity : nullptr;
	const Scope* try_parent = body_ != nullptr ? body_->try_parent : nullptr;
	const bool automatic = named != nullptr && named->kind == EntityKind::Variable && body_ != nullptr &&
		body_->function != nullptr && names_automatic_object(operand) &&
		catch_parameters_.find(named) == PointerMap::kMissing;
	const bool as_rvalue = automatic &&
		(try_parent == nullptr || (named->owner != try_parent && encloses(try_parent, named->owner))) &&
		moves_from(cls, operand);
	ExprFacts* object = facts_of(*inner);
	const ValueCategory category = object->category;
	if (as_rvalue)
		object->category = ValueCategory::XValue;
	record_copy(cls, type, operand, location, true);
	object->category = category;
	return type;
}

ExprFacts* Analyzer::throw_expression(const syntax::ThrowExpr& expr, Scope* scope)
{
	// 15.1p2: a prvalue of type void; `throw` alone rethrows (15.1p8).
	const Type* thrown = expr.operand != nullptr ? throw_operand(*expr.operand, scope, expr.location) : nullptr;
	if (expr.operand != nullptr && thrown == nullptr)
		return nullptr;
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Void), ValueCategory::PRValue);
	facts->target = thrown;
	++potentially_throwing_;
	return facts;
}

void Analyzer::try_statement(const syntax::TryStmt& stmt, Scope* scope)
{
	// 15.3: the compound-statement, then each handler's exception-declaration
	// in a block of its own with the handler's statements.
	// No jump enters them (15p2: their blocks are marked, and the switch
	// statements enclosing them counted).
	const Scope* saved = body_ != nullptr ? body_->try_parent : nullptr;
	if (body_ != nullptr)
	{
		body_->try_parent = scope;
		++body_->handled;
	}
	compound(*stmt.body, scope, false, true);
	if (body_ != nullptr)
		body_->try_parent = saved;
	handlers(stmt.handlers, scope);
	if (body_ != nullptr)
		--body_->handled;
}

void Analyzer::exception_declaration(
	const syntax::ExceptionDeclaration& exception, const DeclaratorInfo& info, Scope* block, uint32_t location)
{
	// 15.3p1: no incomplete type (a pointer or reference to one only when it
	// is cv void), abstract class or rvalue reference; p2's adjustment of an
	// array or function type to a pointer. The variable is initialized from
	// the exception object (15.3p16): a class object by copy-initialization
	// from an lvalue of its type, its destructor potentially invoked.
	const Type* type = info.type;
	if (type->kind == TypeKind::RValueReference)
		error(location, "a handler of rvalue reference type (15.3p1)");
	if (type->kind == TypeKind::Array)
		type = types_.pointer(type->element);
	else if (type->kind == TypeKind::Function)
		type = types_.pointer(type);
	const Type* object = type->is_reference() ? type->element : type;
	const Type* pointee = object->unqualified->kind == TypeKind::Pointer ? object->unqualified->element : nullptr;
	if (pointee != nullptr && !IsVoid(pointee->unqualified) && pointee->kind != TypeKind::Function &&
		ClassOf(pointee) != nullptr && !require_complete(ClassOf(pointee)))
		error(location, "a handler of pointer to incomplete type (15.3p1)");
	ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(object));
	if (cls != nullptr && !require_complete(cls))
		error(location, "a handler of incomplete class type (15.3p1)");
	if (cls != nullptr && !type->is_reference() && !check_abstract_object(object, location))
		return;
	exception.type = type;
	// An unnamed class object is copy-initialized too (15.3p16), into a
	// variable no name finds: the copy 12.8p31 would let it elide is kept,
	// as the host compilers keep it, with its destruction.
	if (info.name == nullptr && (cls == nullptr || type->is_reference() || !semantic()))
		return;
	const Identifier* name = info.name != nullptr ? info.name->last().identifier : nullptr;
	VariableEntity* variable = new_entity<VariableEntity>(EntityKind::Variable, name, block, type);
	variable->defined = true;
	Binding* binding = new_binding(BindingKind::Variable, name, variable, type);
	if (name != nullptr)
	{
		print_in(block, binding);
		declare_in(block, binding);
	}
	exception.declared = binding;
	catch_parameters_.set(variable, 1);
	if (cls != nullptr && !type->is_reference() && semantic())
	{
		bool deleted = false;
		variable->constructor = subobject_copy(cls, CopyKind::CopyConstructor, object->cv, 0, cls, deleted);
		if (variable->constructor == nullptr || deleted)
			error(location, "a handler's class object without an accessible copy constructor (15.3p16)");
		check_special_access(variable->constructor, cls, block, location);
		odr_use(variable->constructor, true);
		check_destruction(object, block, location);
	}
}

} // namespace sema
