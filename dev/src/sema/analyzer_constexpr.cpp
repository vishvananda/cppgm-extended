#include "sema/analyzer.h"

#include "sema/aggregates.h"
#include "sema/constant_interpreter.h"

// The declaration rules of constexpr (7.1.5, 3.9p10): literal types, and
// what a constexpr function, constructor or variable requires of its
// types and initializations. A template's declaration is checked where its
// types do not depend on its parameters; an instantiated specialization is
// not checked again (the PA16 boundary: 7.1.5p6 makes a specialization that
// fails the rules merely not constexpr).
namespace sema {

namespace {

bool UserProvidedMember(const FunctionEntity* function)
{
	return !function->implicit && !function->defaulted && !function->deleted;
}

// Whether a constructor's plan initializes the subobject: a mem-initializer
// or brace-or-equal-initializer for it, or for a member of an anonymous
// member object (which initializes that object, 9.5p5, 12.6.2p8).
bool PlanInitializes(const SubobjectInit& init)
{
	if (init.mem_initializer != nullptr || init.default_initializer != nullptr)
		return true;
	for (uint32_t i = 0; i < init.inner_count; ++i)
	{
		if (PlanInitializes(init.inner[i]))
			return true;
	}
	return false;
}

// An array's element type (3.9.3p5: its cv-qualifiers), or the type.
const Type* ObjectType(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	return type;
}

bool ImplicitThis(const ExprFacts* facts)
{
	return facts != nullptr &&
		((facts->flags & kImplicitThis) != 0 ||
			((facts->flags & kUserConverted) != 0 && facts->original != nullptr &&
				(facts->original->flags & kImplicitThis) != 0));
}

// Whether a member's brace-or-equal-initializer may use the object it
// initializes (`this`, or a member named alone, 9.3.1p3) in a potentially
// evaluated operand; forms not examined are taken as using it.
bool UsesObject(const syntax::Expr& expr, uint32_t depth)
{
	const syntax::Expr* e = &expr;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	if (depth > 64 || ImplicitThis(e->facts))
		return true;
	switch (e->kind)
	{
	case syntax::ExprKind::Id:
	case syntax::ExprKind::Literal:
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::BuiltinTrait:
		return false;
	case syntax::ExprKind::KeywordLiteral:
		return static_cast<const syntax::KeywordLiteralExpr*>(e)->keyword == TokenKind::KW_THIS;
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		return UsesObject(*static_cast<const syntax::BinaryExpr*>(e)->left, depth + 1) ||
			UsesObject(*static_cast<const syntax::BinaryExpr*>(e)->right, depth + 1);
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return UsesObject(*static_cast<const syntax::UnaryExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(e);
		return UsesObject(*choice->condition, depth + 1) || UsesObject(*choice->then_expr, depth + 1) ||
			UsesObject(*choice->else_expr, depth + 1);
	}
	case syntax::ExprKind::Subscript:
		return UsesObject(*static_cast<const syntax::SubscriptExpr*>(e)->base, depth + 1) ||
			UsesObject(*static_cast<const syntax::SubscriptExpr*>(e)->index, depth + 1);
	case syntax::ExprKind::Member:
		return UsesObject(*static_cast<const syntax::MemberExpr*>(e)->base, depth + 1);
	case syntax::ExprKind::Cast:
		return UsesObject(*static_cast<const syntax::CastExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(e);
		bool uses = UsesObject(*call->callee, depth + 1);
		for (uint32_t i = 0; !uses && i < call->arguments.size; ++i)
			uses = UsesObject(*call->arguments[i], depth + 1);
		return uses;
	}
	case syntax::ExprKind::FunctionalCast:
	case syntax::ExprKind::BracedInit:
	{
		const syntax::Slice<syntax::Expr*>* operands = nullptr;
		if (e->kind == syntax::ExprKind::BracedInit)
			operands = &static_cast<const syntax::BracedInitList*>(e)->elements;
		else
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			operands = cast->braced != nullptr ? &cast->braced->elements : &cast->arguments;
		}
		bool uses = false;
		for (uint32_t i = 0; !uses && i < operands->size; ++i)
			uses = UsesObject(*(*operands)[i], depth + 1);
		return uses;
	}
	default:
		return true;
	}
}

} // namespace

bool Analyzer::literal_type(const Type* type)
{
	// 3.9p10: a scalar, a reference (C++11: to a literal type; as C++14 and
	// the implementations have it, any), an array of a literal type, or a
	// class with a trivial destructor that is an aggregate or has a
	// constexpr constructor other than a copy or move constructor, whose
	// bases and non-static data members are of non-volatile literal types.
	// void is none (C++11). A type that depends on template parameters is
	// taken as one.
	if (type->is_reference())
		return true;
	while (type->kind == TypeKind::Array)
		type = type->element;
	if (template_depth_ != 0 && dependent_type(type))
		return true;
	if (IsVoid(type))
		return false;
	if (IsScalar(type))
		return true;
	ClassEntity* cls = type->unqualified->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->unqualified->entity)
																  : nullptr;
	if (cls == nullptr)
		return true;
	if (cls->literal_state == 1 || cls->literal_state == 3)
		return true;
	if (cls->literal_state == 2)
		return false;
	if (!require_complete(cls) || cls->scope == nullptr)
		return false;
	cls->literal_state = 3;
	bool literal = cls->destructor == nullptr || cls->destructor->trivial;
	for (const BaseClass* base = cls->bases; literal && base != nullptr; base = base->next)
		literal = !base->is_virtual && literal_type(base->base->type);
	for (const DataMember* member = cls->members; literal && member != nullptr; member = member->next)
	{
		// Of non-volatile literal types (a volatile member's reads are no
		// constant expressions, 5.19p2).
		const Type* type = member->variable->type;
		literal = member->variable->kind != EntityKind::Variable ||
			(literal_type(type) && (type->is_reference() || (ObjectType(type)->cv & kVolatile) == 0));
	}
	if (literal && !IsAggregateClass(cls))
	{
		// A constexpr constructor (or constructor template) other than a
		// copy or move constructor; an implicit default constructor is one
		// when it initializes every member (12.1p6).
		bool has_constexpr = false;
		const NameEntry* entry = names_.find(cls->scope, constructor_key_);
		for (const FunctionList* node = entry != nullptr ? entry->functions : nullptr;
			node != nullptr && !has_constexpr; node = node->next)
		{
			// An inheriting constructor is one as 12.9p6 makes it.
			const FunctionEntity* constructor = static_cast<const FunctionEntity*>(node->binding->entity);
			has_constexpr = constructor->copy_kind == CopyKind::None &&
				(constructor->is_constexpr ||
					(constructor->inherited != nullptr && constexpr_constructor(constructor, 0)));
		}
		if (!has_constexpr && !cls->user_declared_constructor)
			has_constexpr = constexpr_constructor(queried_default_constructor(cls), 0);
		literal = has_constexpr;
	}
	cls->literal_state = literal ? 1 : 2;
	return literal;
}

bool Analyzer::constexpr_constructor(const FunctionEntity* constructor, uint32_t depth)
{
	// A user-provided constructor is constexpr as declared; an implicit or
	// defaulted one when the user-written one would satisfy 7.1.5p4: a
	// default constructor as 12.1p6, an inheriting one as 12.9p6, a copy
	// or move constructor as 12.8p13 say.
	if (constructor == nullptr || constructor->deleted)
		return false;
	if (UserProvidedMember(constructor))
		return constructor->is_constexpr;
	if (constructor->inherited != nullptr)
		return constexpr_inheriting(constructor, depth);
	return constructor->copy_kind != CopyKind::None ? constexpr_copy(constructor, depth)
													: constexpr_default(constructor, depth);
}

// One outermost question of constexpr_default/copy/assignment: answers
// recorded while it lasts (Analyzer::constexpr_members_), so a tree of
// classes asks each member function once, not once per path; the next
// query starts afresh (a class completed meanwhile may change an answer).
struct ConstexprQuery
{
	Analyzer& analyzer;
	explicit ConstexprQuery(Analyzer& owner) : analyzer(owner) { ++analyzer.constexpr_queries_; }
	~ConstexprQuery()
	{
		if (--analyzer.constexpr_queries_ == 0)
			analyzer.constexpr_members_.clear();
	}
};

bool Analyzer::remember_constexpr(const FunctionEntity* function, bool result)
{
	constexpr_members_.set(function, result ? 1 : 0);
	return result;
}

bool Analyzer::constexpr_default(const FunctionEntity* constructor, uint32_t depth)
{
	const ConstexprQuery query(*this);
	const uint32_t known = constexpr_members_.find(constructor);
	return known != PointerMap::kMissing ? known != 0
										 : remember_constexpr(constructor, constexpr_default_plan(constructor, depth));
}

bool Analyzer::constexpr_inheriting(const FunctionEntity* constructor, uint32_t depth)
{
	const ConstexprQuery query(*this);
	const uint32_t known = constexpr_members_.find(constructor);
	return known != PointerMap::kMissing
		? known != 0
		: remember_constexpr(constructor, constexpr_inheriting_plan(constructor, depth));
}

bool Analyzer::constexpr_copy(const FunctionEntity* constructor, uint32_t depth)
{
	const ConstexprQuery query(*this);
	const uint32_t known = constexpr_members_.find(constructor);
	return known != PointerMap::kMissing ? known != 0
										 : remember_constexpr(constructor, constexpr_copy_plan(constructor, depth));
}

bool Analyzer::constexpr_assignment(const FunctionEntity* assignment, uint32_t depth)
{
	const ConstexprQuery query(*this);
	const uint32_t known = constexpr_members_.find(assignment);
	return known != PointerMap::kMissing ? known != 0
										 : remember_constexpr(assignment, constexpr_assignment_plan(assignment, depth));
}

bool Analyzer::constexpr_default_plan(const FunctionEntity* constructor, uint32_t depth)
{
	// 12.1p6: the implicit definition of a default constructor (its plan)
	// satisfies 7.1.5p4: no virtual base; each non-variant member and base
	// initialized, by constexpr constructors, and a union's (or anonymous
	// union's) one member. Subobjects nest no deeper than instantiations.
	if (constructor->deleted || depth > kInstantiationDepthLimit)
		return false;
	const ClassEntity* cls = static_cast<const ClassEntity*>(constructor->owner->entity);
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (base->is_virtual)
			return false;
	}
	const bool is_union = cls->key == TokenKind::KW_UNION;
	uint32_t initialized = 0;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		if (!constexpr_default_subobject(constructor->inits[i], is_union, initialized, depth))
			return false;
	}
	return !is_union || cls->members == nullptr || initialized == 1;
}

bool Analyzer::constexpr_default_subobject(
	const SubobjectInit& init, bool variant, uint32_t& initialized, uint32_t depth)
{
	// One subobject of constexpr_default's plan (a brace-or-equal-
	// initializer is taken as the constant initialization p4 requires).
	if (init.inner != nullptr)
	{
		// An anonymous member object that is another variant member than
		// the initialized one is not initialized (as a named one, 12.6.2p8;
		// 9.5p8: no member of it has a brace-or-equal-initializer).
		if (variant && !MemberInitialized(init.member))
			return true;
		// The members of a variant anonymous struct are variant members too
		// (they share the union's storage; GCC and Clang read it so).
		const ClassEntity* anonymous = ObjectClassOf(init.member->variable->type);
		const bool is_union = anonymous != nullptr && anonymous->key == TokenKind::KW_UNION;
		uint32_t inner_initialized = 0;
		for (uint32_t i = 0; i < init.inner_count; ++i)
		{
			if (!constexpr_default_subobject(init.inner[i], is_union || variant, inner_initialized, depth))
				return false;
		}
		if (is_union && init.inner_count != 0 && inner_initialized != 1)
			return false;
		initialized += inner_initialized != 0;
		return true;
	}
	if (init.base == nullptr &&
		(init.member == nullptr || init.member->variable->kind != EntityKind::Variable ||
			init.member->variable->name == nullptr))
		return true;
	if (init.default_initializer != nullptr)
	{
		++initialized;
		return true;
	}
	if (variant)
		return true;
	// Default-initialized: a class object by its default constructor; a
	// scalar or reference is left uninitialized.
	const Type* type = init.base != nullptr ? init.base->base->type : init.member->variable->type;
	if (ObjectClassOf(type) == nullptr || type->is_reference())
		return false;
	++initialized;
	return constexpr_constructor(init.constructor, depth + 1);
}

bool Analyzer::constexpr_inheriting_plan(const FunctionEntity* constructor, uint32_t depth)
{
	// 12.9p6: an inheriting constructor is constexpr (a constructor
	// characteristic, 12.9p2) when its implicit definition, the user-written
	// constructor of 12.9p8, would satisfy 7.1.5p4: the inherited
	// constructor, which initializes its base, is constexpr (as its
	// default-initialized subobjects' are, as for 12.1p6), and the
	// brace-or-equal-initializers of its members are constant expressions.
	// Its plan is made before they are typed (see typed_inheriting_plans).
	if (constructor->deleted || depth > kInstantiationDepthLimit)
		return false;
	return constexpr_default_plan(constructor, depth) &&
		constant_member_initializers(constructor->inits, constructor->init_count, depth);
}

bool Analyzer::constant_member_initializers(const SubobjectInit* inits, uint32_t count, uint32_t depth)
{
	// 7.1.5p4 for the members of a plan that their brace-or-equal-
	// initializers initialize (an anonymous member object's within it): the
	// initialization's constructor is constexpr, and an initializer that
	// cannot depend on the object being constructed (see UsesObject)
	// evaluates as a constant expression. One not yet typed, or using the
	// object, is left to the evaluation that runs it.
	for (uint32_t i = 0; i < count; ++i)
	{
		const SubobjectInit& init = inits[i];
		if (init.inner != nullptr)
		{
			if (!constant_member_initializers(init.inner, init.inner_count, depth))
				return false;
			continue;
		}
		if (init.default_initializer == nullptr || init.member->variable->kind != EntityKind::Variable)
			continue;
		if (init.constructor != nullptr && !init.constructor->trivial &&
			!constexpr_constructor(init.constructor, depth + 1))
			return false;
		const VariableEntity* member = static_cast<const VariableEntity*>(init.member->variable);
		const syntax::Initializer* initializer = init.default_initializer;
		if (!member->initializer_typed || initializer->expression == nullptr || UsesObject(*initializer->expression, 0))
			continue;
		ConstantInterpreter interpreter(*this);
		const ConstValue* value = nullptr;
		if (!interpreter.initialize(member, initializer, value, true))
			return false;
	}
	return true;
}

bool Analyzer::constexpr_copy_plan(const FunctionEntity* constructor, uint32_t depth)
{
	// 12.8p13: the implicit definition of a copy or move constructor
	// satisfies 7.1.5p4: no virtual base, and each subobject copied by a
	// constexpr constructor (a scalar or reference as its value; a trivial
	// copy copies each subobject trivially, 12.8p12).
	if (constructor->deleted || depth > kInstantiationDepthLimit)
		return false;
	if (constructor->trivial)
		return true;
	const ClassEntity* cls = static_cast<const ClassEntity*>(constructor->owner->entity);
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (base->is_virtual)
			return false;
	}
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		const FunctionEntity* selected = constructor->inits[i].constructor;
		if (selected != nullptr && !constexpr_constructor(selected, depth + 1))
			return false;
	}
	return true;
}

bool Analyzer::constexpr_assignment_plan(const FunctionEntity* assignment, uint32_t depth)
{
	// C++14 12.8p26 (-std=c++14; C++11's assignment operators are never
	// constexpr): the implicit definition of a copy or move assignment
	// operator of a literal class is constexpr when the one selected for
	// each direct base and class member is (a user-provided one if declared
	// constexpr, an implicit one by this rule; scalars are assigned as
	// such).
	if (!cxx14_ || assignment->deleted || depth > kInstantiationDepthLimit)
		return false;
	const ClassEntity* cls = static_cast<const ClassEntity*>(assignment->owner->entity);
	if (!literal_type(cls->type))
		return false;
	for (uint32_t i = 0; i < assignment->init_count; ++i)
	{
		const FunctionEntity* selected = assignment->inits[i].constructor;
		if (selected == nullptr)
			continue;
		const bool implicit = selected->implicit || (selected->defaulted && !selected->is_constexpr);
		if (implicit ? !(selected->trivial || constexpr_assignment(selected, depth + 1)) : !selected->is_constexpr)
			return false;
	}
	return true;
}

const ClassEntity* Analyzer::ObjectClassOf(const Type* type)
{
	while (type->kind == TypeKind::Array)
		type = type->element;
	return ClassOf(type);
}

void Analyzer::check_constexpr_function(const FunctionEntity* function, const Type* type, uint32_t location)
{
	// 7.1.5p3: a constexpr function's return type and parameter types are
	// literal types (not void); p8: a non-static member function's class is
	// a literal type (a templated class's is known in its instances). p6: a
	// function template's or a class template's member's specialization
	// that fails them is merely no constexpr function, so neither a
	// template nor a member of a templated class or of an instance is
	// diagnosed.
	if (!function->is_constexpr || function->instantiated || function->deleted || function->templated ||
		(function->owner != nullptr && function->owner->kind == ScopeKind::Class &&
			(templated_class(function->owner) || WithinInstance(function->owner))))
		return;
	const Type* canonical = type->canonical;
	if (!function->constructor)
	{
		// void is a literal type under -std=c++14 (N3652, see set_cxx14).
		if (!literal_type(canonical->element) && !(cxx14_ && IsVoid(canonical->element)))
			error(location, "a constexpr function's return type is not a literal type");
		if (function->member && !function->is_static && function->owner != nullptr &&
			function->owner->kind == ScopeKind::Class && !templated_class(function->owner) &&
			!literal_type(function->owner->entity->type))
			error(location, "a constexpr member function of a class that is not a literal type");
	}
	for (uint32_t i = 0; i < canonical->param_count; ++i)
	{
		if (!literal_type(canonical->params[i]))
			error(location, "a constexpr function's parameter type is not a literal type");
	}
}

void Analyzer::check_constexpr_constructor(const FunctionEntity* constructor, uint32_t location)
{
	// 7.1.5p4: no virtual bases; every base and non-static data member
	// initialized (a non-empty union's one member), each by a constexpr
	// constructor where one runs (a delegating constructor's target is the
	// one that initializes them).
	if (!constructor->is_constexpr || constructor->instantiated || constructor->deleted || constructor->implicit ||
		constructor->defaulted)
		return;
	if (constructor->delegation != nullptr)
	{
		if (constructor->delegate != nullptr && !constexpr_constructor(constructor->delegate, 0))
			error(location, "a constexpr constructor delegates to a constructor that is not constexpr");
		return;
	}
	const ClassEntity* cls = static_cast<const ClassEntity*>(constructor->owner->entity);
	for (const BaseClass* base = cls->bases; base != nullptr; base = base->next)
	{
		if (base->is_virtual)
			error(location, "a constexpr constructor of a class with a virtual base");
	}
	const bool is_union = cls->key == TokenKind::KW_UNION;
	uint32_t initialized = 0;
	for (uint32_t i = 0; i < constructor->init_count; ++i)
	{
		// A variant member another initializes is not checked (an anonymous
		// struct's members are its own, initialized only with it).
		const SubobjectInit& init = constructor->inits[i];
		const bool initializes = PlanInitializes(init);
		if (is_union && initializes)
			++initialized;
		if (!is_union || initializes)
			check_constexpr_subobject(init, is_union, location);
	}
	// (Several are rejected with the plan, 12.6.2p8.)
	if (is_union && initialized == 0 && cls->members != nullptr)
		error(location, "a constexpr constructor of a union initializes no member");
}

void Analyzer::check_constexpr_subobject(const SubobjectInit& init, bool variant, uint32_t location)
{
	// The constructor initializing the subobject is constexpr (an implicit
	// or defaulted one as 12.1p6 and 12.8p13 make it); value-initialization
	// by a trivial default constructor zero-initializes, running none
	// (8.5p7).
	const syntax::MemInitializer* written = init.mem_initializer;
	const bool value_initialized = written != nullptr && written->arguments.size == 0 &&
		(written->list == nullptr || written->list->elements.empty());
	if (init.constructor != nullptr && !init.constructor->deleted &&
		!(value_initialized && init.constructor->trivial) && !constexpr_constructor(init.constructor, 0))
		error(location, "a constexpr constructor initializes a subobject by a constructor that is not constexpr");
	if (init.base != nullptr || init.member == nullptr)
		return;
	if (init.mem_initializer == nullptr && init.default_initializer != nullptr)
		check_constexpr_member_initializer(init, location);
	if (init.inner != nullptr)
	{
		// A variant anonymous member the constructor does not initialize is
		// not checked; a variant anonymous struct's members are variant
		// members too (see constexpr_default_subobject).
		if (variant && !PlanInitializes(init))
			return;
		const ClassEntity* anonymous = ObjectClassOf(init.member->variable->type);
		const bool is_union = anonymous != nullptr && anonymous->key == TokenKind::KW_UNION;
		uint32_t initialized = 0;
		for (uint32_t i = 0; i < init.inner_count; ++i)
		{
			const SubobjectInit& inner = init.inner[i];
			const bool initializes = PlanInitializes(inner);
			if (is_union && initializes)
				++initialized;
			if (!is_union || initializes)
				check_constexpr_subobject(inner, is_union || variant, location);
		}
		if (is_union && initialized == 0 && init.inner_count != 0)
			error(location, "a constexpr constructor initializes no member of an anonymous union");
		return;
	}
	const Entity* member = init.member->variable;
	if (member->kind != EntityKind::Variable || member->name == nullptr || variant)
		return;
	const Type* type = member->type;
	if (init.mem_initializer != nullptr || init.default_initializer != nullptr)
		return;
	// Default-initialized: a class member by its (constexpr) default
	// constructor; a scalar one is left uninitialized.
	if (ObjectClassOf(type) == nullptr || type->is_reference())
		error(location, "a constexpr constructor leaves a non-static data member uninitialized");
}

void Analyzer::check_defaulted_constexpr(FunctionEntity* function, uint32_t location)
{
	// 8.4.2p2: an explicitly-defaulted function is declared constexpr only
	// if its implicit declaration would be: a default constructor as 12.1p6
	// and a copy or move constructor as 12.8p13 make it; an assignment
	// operator never is in C++11 (7.1.5p3: a C++11 constexpr body is one
	// return statement), under -std=c++14 as its 12.8p26 makes it; a
	// destructor never is.
	if (!function->is_constexpr || function->deleted)
		return;
	bool implicit_constexpr = false;
	if (function->constructor && function->copy_kind != CopyKind::None)
		implicit_constexpr = constexpr_copy(function, 0);
	else if (!function->constructor && function->copy_kind != CopyKind::None)
		implicit_constexpr = constexpr_assignment(function, 0);
	else if (function->constructor)
	{
		if (function->inits == nullptr)
			implicit_default_facts(static_cast<ClassEntity*>(function->owner->entity), function);
		implicit_constexpr = constexpr_default(function, 0);
	}
	if (!implicit_constexpr)
		error(location, "an explicitly defaulted function declared constexpr whose implicit declaration would not be");
}

void Analyzer::check_defaulted_constexprs()
{
	// Those defaulted on their first declaration, once every class of the
	// unit is complete (their plans use the members' brace-or-equal-
	// initializers, typed in the complete class, 9.2p2).
	for (const DefaultedConstexpr& pending : defaulted_constexprs_)
		check_defaulted_constexpr(pending.function, pending.location);
	defaulted_constexprs_.clear();
}

void Analyzer::check_constexpr_member_initializer(const SubobjectInit& init, uint32_t location)
{
	// 7.1.5p4: the brace-or-equal-initializer of a member no mem-initializer
	// names is a constant expression. Decided where it cannot depend on the
	// object being constructed: a scalar member's `= expression` that does
	// not use `this`, which the constant interpreter fails to evaluate.
	const syntax::Initializer* initializer = init.default_initializer;
	const Type* type = init.member->variable->type;
	if (initializer->kind != syntax::InitializerKind::Assign ||
		initializer->expression->kind == syntax::ExprKind::BracedInit || type->is_reference() || !IsScalar(type) ||
		UsesObject(*initializer->expression, 0))
		return;
	ConstantInterpreter interpreter(*this);
	ConstValue value;
	if (!interpreter.evaluate(*initializer->expression, true, value))
		error(interpreter.failure() != nullptr ? interpreter.failure_location() : location,
			"a constexpr constructor leaves a member to a brace-or-equal-initializer that is not a constant "
			"expression");
}

void Analyzer::integral_constant_operand(const syntax::Expr& expr, Scope* scope, const Type* target)
{
	// The operand of an integral constant expression typed as a constant
	// expression's. 5.19p6: one of literal class type is contextually
	// implicitly converted (4p5) to an integral or unscoped enumeration type
	// by its one such non-explicit conversion function; a converted
	// constant expression of type `target` (5.19p3: an array bound, 8.3.4p1;
	// an enumerator of a fixed underlying type, 7.2p5) converts by the
	// conversion function overload resolution selects (13.3.1.5). The
	// operand then stands for the call, which the evaluation requires to be
	// of a constexpr function (5.19p2).
	constant_operand(expr, scope);
	const ExprFacts* facts = expr.facts;
	if (facts == nullptr || facts->type == nullptr || facts->type->unqualified->kind != TypeKind::Class)
		return;
	BodyContext* saved_body = body_;
	BodyContext context = BodyContext();
	if (body_ == nullptr)
		body_ = &context;
	Scope* saved_scope = expression_scope_;
	expression_scope_ = scope;
	if (target != nullptr)
		convert_to_scalar(expr, target, false);
	else if (const FunctionEntity* converter = unique_conversion(ClassOf(facts->type), IsIntegralOrUnscopedEnum))
		convert_class_operand(expr, converter);
	body_ = saved_body;
	expression_scope_ = saved_scope;
}

void Analyzer::check_constexpr_variable(const VariableEntity* variable, uint32_t location)
{
	// 7.1.5p9: a constexpr object's type is a literal type.
	if (variable->is_constexpr && !literal_type(variable->type))
		error(location, "a constexpr variable of a type that is not a literal type");
}

} // namespace sema
