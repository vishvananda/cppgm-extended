#include "sema/analyzer.h"

#include <cmath>
#include <cstring>
#include <string>
#include <utility>

#include "lex/identifier_table.h"
#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/aggregates.h"
#include "sema/string_literals.h"

// Clause 5 typing for the PA7 slice (semantics mode): each analyzed
// expression gets its type, value category and what it designates; the
// conversions applied to operands are recorded on the operands.
namespace sema {

namespace {

// A class template specialization (not a compound type built from one).
bool SpecializationType(const Type* type)
{
	const Type* bare = type->unqualified;
	return bare->kind == TypeKind::Class && static_cast<const ClassEntity*>(bare->entity)->templ != nullptr &&
		static_cast<const ClassEntity*>(bare->entity)->templ->role == TemplateRole::Specialization;
}

// 13.3.1.2p1: only an operand of class or enumeration type (or one whose
// type is not yet known) may make an operator a call of an operator
// function; built-in operators skip the candidate search.
bool MayOverload(const syntax::Expr* operand)
{
	const ExprFacts* facts = operand->facts;
	if (facts == nullptr || facts->type == nullptr)
		return true;
	const TypeKind kind = facts->type->unqualified->kind;
	return kind == TypeKind::Class || kind == TypeKind::Enum;
}

bool IsNullptrType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT;
}

bool IsBoolType(const Type* type)
{
	type = type->unqualified;
	return type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Bool;
}

bool IsPointer(const Type* type)
{
	return type->unqualified->kind == TypeKind::Pointer;
}

bool IsMemberPointer(const Type* type)
{
	return type->unqualified->kind == TypeKind::MemberPointer;
}

bool IsComparison(TokenKind op)
{
	return op == TokenKind::OP_EQ || op == TokenKind::OP_NE || op == TokenKind::OP_LT || op == TokenKind::OP_GT ||
		op == TokenKind::OP_LE || op == TokenKind::OP_GE;
}

// The binary operator of a compound assignment (5.17p7).
TokenKind CompoundOperator(TokenKind op)
{
	switch (op)
	{
	case TokenKind::OP_PLUSASS:
		return TokenKind::OP_PLUS;
	case TokenKind::OP_MINUSASS:
		return TokenKind::OP_MINUS;
	case TokenKind::OP_STARASS:
		return TokenKind::OP_STAR;
	case TokenKind::OP_DIVASS:
		return TokenKind::OP_DIV;
	case TokenKind::OP_MODASS:
		return TokenKind::OP_MOD;
	case TokenKind::OP_XORASS:
		return TokenKind::OP_XOR;
	case TokenKind::OP_BANDASS:
		return TokenKind::OP_AMP;
	case TokenKind::OP_BORASS:
		return TokenKind::OP_BOR;
	case TokenKind::OP_LSHIFTASS:
		return TokenKind::OP_LSHIFT;
	case TokenKind::OP_RSHIFTASS:
		return TokenKind::OP_RSHIFT;
	default:
		return op;
	}
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

size_t HashConversion(const Conversion& c)
{
	size_t hash = static_cast<size_t>(c.form) | static_cast<size_t>(c.first) << 4 | static_cast<size_t>(c.second) << 8 |
		static_cast<size_t>(c.rank) << 12 | static_cast<size_t>(c.qualification) << 16 |
		static_cast<size_t>(c.initialization) << 17 | static_cast<size_t>(c.rvalue_reference) << 18 |
		static_cast<size_t>(c.temporary) << 19 | static_cast<size_t>(c.function_lvalue) << 20 |
		static_cast<size_t>(c.initializer_list) << 21;
	hash ^= reinterpret_cast<uintptr_t>(c.target) * 0x9E3779B97F4A7C15ull;
	hash ^= reinterpret_cast<uintptr_t>(c.result) * 0xC2B2AE3D27D4EB4Full;
	hash ^= reinterpret_cast<uintptr_t>(c.constructor) * 0x165667B19E3779F9ull;
	hash ^= reinterpret_cast<uintptr_t>(c.argument) * 0x27D4EB2F165667C5ull;
	hash ^= reinterpret_cast<uintptr_t>(c.after) * 0x94D049BB133111EBull;
	return hash ^ (hash >> 29);
}

bool SameConversion(const Conversion& a, const Conversion& b)
{
	return a.form == b.form && a.first == b.first && a.second == b.second && a.qualification == b.qualification &&
		a.rank == b.rank && a.initialization == b.initialization && a.rvalue_reference == b.rvalue_reference &&
		a.temporary == b.temporary && a.function_lvalue == b.function_lvalue &&
		a.initializer_list == b.initializer_list && a.target == b.target && a.result == b.result &&
		a.constructor == b.constructor && a.argument == b.argument && a.after == b.after;
}

// The object expression `this` of E1->E2, or `*this` of E1.E2.
bool NamesThis(const syntax::MemberExpr& expr)
{
	const syntax::Expr* base = WithoutParentheses(expr.base);
	if (expr.op == TokenKind::OP_DOT)
	{
		if (base->kind != syntax::ExprKind::Unary ||
			static_cast<const syntax::UnaryExpr*>(base)->op != TokenKind::OP_STAR)
			return false;
		base = WithoutParentheses(static_cast<const syntax::UnaryExpr*>(base)->operand);
	}
	return base->kind == syntax::ExprKind::KeywordLiteral &&
		static_cast<const syntax::KeywordLiteralExpr*>(base)->keyword == TokenKind::KW_THIS;
}

// A parameter object's type (8.3.5p5): arrays and functions adjusted to
// pointers, the declared cv-qualifiers kept.
const Type* ParameterObjectType(TypeTable& types, const Type* type)
{
	if (type->kind == TypeKind::Array)
		return types.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types.pointer(type);
	return type;
}

} // namespace

ExprFacts* Analyzer::new_facts(const syntax::Expr& expr, const Type* type, ValueCategory category)
{
	ExprFacts* facts = static_cast<ExprFacts*>(arena_.allocate(sizeof(ExprFacts), alignof(ExprFacts)));
	std::memset(static_cast<void*>(facts), 0, sizeof(ExprFacts));
	// Expression types are canonical (function parameters adjusted,
	// 8.3.5p5), so type identity is pointer identity.
	facts->type = type != nullptr ? type->canonical : nullptr;
	facts->category = category;
	expr.facts = facts;
	// An object of a class template instance: its definition may matter
	// (not a decltype operand's call, 5.2.2p11).
	if (type != nullptr && type->unqualified->kind == TypeKind::Class &&
		static_cast<const ClassEntity*>(type->unqualified->entity)->templ != nullptr && &expr != decltype_call_)
		require_complete(static_cast<const ClassEntity*>(type->unqualified->entity));
	++graph_.expressions;
	return facts;
}

bool Analyzer::check_not_deleted(const FunctionEntity* function, uint32_t location)
{
	// 8.4.3p2; in a substitution its failure (14.8.2p8), recorded while the
	// analysis goes on to the boundary.
	if (function->deleted)
		return substitution_error(location, "use of a deleted function");
	return true;
}

void Analyzer::odr_use(FunctionEntity* function, bool call)
{
	// 3.2p2: a function named by a potentially evaluated expression is
	// odr-used. The edge goes to the definition being analyzed or, for code
	// a unit always emits, to the unit's roots; each list names a function
	// once (the function's mark holds the list's epoch and its node).
	if (function == nullptr)
		return;
	if (unevaluated_ != 0)
	{
		if (deferring())
			defer_use(
				DeferredUse{ DeferredUse::Kind::Function, call, nullptr, nullptr, function, nullptr, nullptr, 0 });
		return;
	}
	// Its exception-specification is needed now (15.4p13).
	if (function->noexcept_pending)
		function_nothrow(function);
	// An instantiated function's definition is then required (14.7.1p3).
	if (function->instantiated && !function->definition_demanded && semantic())
	{
		function->definition_demanded = true;
		demanded_definitions_.push_back(function);
	}
	// A constructor a static variable's initializer names (not in a body
	// its evaluation instantiates) may run uncalled when the variable is
	// constant-initialized: noted for the variable (see SemanticGraph).
	if (initializing_ != nullptr && function->constructor && !function->trivial &&
		(body_ == nullptr || body_->function == nullptr))
		graph_.note_initializer_constructor(initializing_, function);
	if (function->demand_mark == current_epoch_)
	{
		function->demand_node->call = function->demand_node->call || call;
		return;
	}
	function->demand_mark = current_epoch_;
	DemandList* node = static_cast<DemandList*>(arena_.allocate(sizeof(DemandList), alignof(DemandList)));
	*node = DemandList{ function, call, nullptr };
	function->demand_node = node;
	++graph_.demands;
	DemandList*& head = demanding_ != nullptr ? demanding_->demands : graph_.root_demands;
	DemandList*& tail = demanding_ != nullptr ? demanding_->last_demand : graph_.last_root_demand;
	if (tail == nullptr)
		head = node;
	else
		tail->next = node;
	tail = node;
}

// ------------------------------------------------------------ dispatch

ExprFacts* Analyzer::typed(const syntax::Expr& expr, Scope* scope)
{
	// An expression whose analysis named something dependent is marked
	// (see NameLookup::dependent_uses): in an instantiation it may be
	// type-dependent in its template, which decides the context of the
	// argument-dependent lookup of an operator applied to it (14.6p10,
	// 14.6.4.2).
	const uint64_t uses = names_.dependent_uses();
	ExprFacts* facts = typed_form(expr, scope);
	switch (expr.kind)
	{
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::Delete:
	case syntax::ExprKind::Throw:
		// Never type-dependent (14.6.2.2p4).
		names_.set_dependent_uses(uses);
		break;
	case syntax::ExprKind::BuiltinTrait:
		if (static_cast<const syntax::BuiltinTraitExpr&>(expr).trait < GnuTrait::UnderlyingType)
			names_.set_dependent_uses(uses);
		break;
	default:
		break;
	}
	if (facts != nullptr && names_.dependent_uses() != uses)
		facts->flags |= kDependentUse;
	return facts;
}

ExprFacts* Analyzer::typed_form(const syntax::Expr& expr, Scope* scope)
{
	check_stack(expr.location);
	expression_scope_ = scope;
	if (&expr == pretyped_[0] || &expr == pretyped_[1] ||
		(expr.facts != nullptr && (expr.facts->flags & kImplicit) != 0))
		return typed_already(expr);
	switch (expr.kind)
	{
	case syntax::ExprKind::Id:
		return id_expression(static_cast<const syntax::IdExpr&>(expr), scope);
	case syntax::ExprKind::Literal:
		return literal(static_cast<const syntax::LiteralExpr&>(expr));
	case syntax::ExprKind::KeywordLiteral:
		return keyword_literal(static_cast<const syntax::KeywordLiteralExpr&>(expr), scope);
	case syntax::ExprKind::Paren:
	{
		// Parentheses change nothing but decltype's reading (7.1.6.2p4):
		// they share their operand's facts.
		ExprFacts* inner = typed(*static_cast<const syntax::ParenExpr&>(expr).inner, scope);
		expr.facts = inner;
		return inner;
	}
	case syntax::ExprKind::Binary:
		return binary_chain(static_cast<const syntax::BinaryExpr&>(expr), scope);
	case syntax::ExprKind::Assign:
		return assignment(static_cast<const syntax::BinaryExpr&>(expr), scope);
	case syntax::ExprKind::Conditional:
		return conditional(static_cast<const syntax::ConditionalExpr&>(expr), scope);
	case syntax::ExprKind::Unary:
		return unary(static_cast<const syntax::UnaryExpr&>(expr), scope);
	case syntax::ExprKind::Postfix:
		return increment(static_cast<const syntax::UnaryExpr&>(expr), scope);
	case syntax::ExprKind::Call:
		return call(static_cast<const syntax::CallExpr&>(expr), scope);
	case syntax::ExprKind::FunctionalCast:
		return functional_cast(static_cast<const syntax::FunctionalCastExpr&>(expr), scope);
	case syntax::ExprKind::Subscript:
		return subscript(static_cast<const syntax::SubscriptExpr&>(expr), scope);
	case syntax::ExprKind::Member:
		return member_access(static_cast<const syntax::MemberExpr&>(expr), scope);
	case syntax::ExprKind::Cast:
		return cast_expression(static_cast<const syntax::CastExpr&>(expr), scope);
	case syntax::ExprKind::Sizeof:
		return size_of(static_cast<const syntax::SizeofExpr&>(expr), scope);
	case syntax::ExprKind::SizeofPack:
	{
		// 5.3.3p5: a pack's number of elements, a constant of type
		// std::size_t (a size query, as sizeof's value is).
		uint64_t count = 0;
		if (!pack_size(static_cast<const syntax::SizeofPackExpr&>(expr).pack, scope, count))
			error(expr.location, "sizeof... of a name that is no parameter pack");
		ExprFacts* facts = new_facts(
			expr, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
		facts->value = count;
		return facts;
	}
	case syntax::ExprKind::Lambda:
		// A closure type in a template is dependent (a local class).
		names_.note_dependent_use();
		return lambda_expression(static_cast<const syntax::LambdaExpr&>(expr), scope);
	case syntax::ExprKind::New:
		return allocation(static_cast<const syntax::NewExpr&>(expr), scope);
	case syntax::ExprKind::Delete:
		return deletion(static_cast<const syntax::DeleteExpr&>(expr), scope);
	case syntax::ExprKind::BracedInit:
		error(expr.location, "a braced-init-list is not an expression here");
	case syntax::ExprKind::Throw:
		return throw_expression(static_cast<const syntax::ThrowExpr&>(expr), scope);
	case syntax::ExprKind::Statement:
		return statement_expression(static_cast<const syntax::StatementExpr&>(expr), scope);
	case syntax::ExprKind::BuiltinTrait:
		return builtin_trait(static_cast<const syntax::BuiltinTraitExpr&>(expr), scope);
	case syntax::ExprKind::TypeTrait:
	{
		const syntax::TypeTraitExpr& trait = static_cast<const syntax::TypeTraitExpr&>(expr);
		if (trait.keyword == TokenKind::KW_ALIGNOF)
		{
			// Of a type-id, or (GNU) of an expression's type, the operand not
			// evaluated.
			const Type* type = nullptr;
			uint32_t aliased = 0;
			if (trait.type != nullptr)
			{
				type = type_id(*trait.type, scope);
				aliased = type_id_alignment_;
			}
			else
			{
				++unevaluated_;
				const ExprFacts* operand = typed_value(*trait.operand, scope);
				--unevaluated_;
				type = operand != nullptr ? operand->type : nullptr;
				aliased = operand != nullptr ? alias_alignment_of(*trait.operand, false) : 0;
			}
			if (type == nullptr)
				return nullptr;
			// 5.3.6p1: of a complete object type, an array of one or a
			// reference to one (in a substitution, its failure otherwise).
			const Type* object = type->is_reference() ? type->element : type;
			while (object->kind == TypeKind::Array)
				object = object->element;
			uint64_t size = 0, alignment = 0;
			if (object->kind == TypeKind::Function || IsVoid(object) ||
				!type_size(object, size, alignment, expr.location))
			{
				substitution_error(
					expr.location, "alignof of an incomplete or function type", ClassOf(object) != nullptr);
				return nullptr;
			}
			// A std::size_t constant (5.3.6p2).
			ExprFacts* facts = new_facts(
				expr, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
			facts->value = aliased != 0 ? aliased : alignment;
			if (trait.type != nullptr && SpecializationType(type_id(*trait.type, scope)))
				facts->flags |= kSpecializationQuery;
			return facts;
		}
		if (trait.keyword == TokenKind::KW_NOEXCEPT && trait.operand != nullptr)
			return noexcept_expression(trait, scope);
		if (trait.keyword == TokenKind::KW_TYPEID)
			return typeid_expression(trait, scope);
		error(expr.location, "unsupported type trait");
	}
	case syntax::ExprKind::Fold:
		// Instantiation expands it (syntax/ast_clone.cpp).
		error(expr.location, "a fold-expression without a pack to expand");
	default:
		error(expr.location, "unsupported expression");
	}
}

ExprFacts* Analyzer::typed_value(const syntax::Expr& expr, Scope* scope)
{
	// An operand that must denote a value: not an unresolved overload set
	// (13.4p1 needs a target) nor a member function without its call.
	ExprFacts* facts = typed(expr, scope);
	if (facts == nullptr)
		return nullptr;
	check_resolved(expr);
	return facts;
}

void Analyzer::check_resolved(const syntax::Expr& expr)
{
	const ExprFacts* facts = expr.facts;
	if ((facts->flags & kOverloadSet) != 0)
	{
		if (resolve_lone_function(expr))
			return;
		error(expr.location, "overloaded function name without a target type");
	}
	if ((facts->flags & kBoundMember) != 0)
		error(expr.location, "a member function must be called");
}

bool Analyzer::resolve_lone_function(const syntax::Expr& expr)
{
	// A lone function in the set resolves without a target (a template-id
	// whose trailing packs a target would deduce names the specialization
	// with them empty, 14.8.1p3); a lone function template does not: its
	// specialization is deduced from the target's type (13.4p1-p2,
	// 14.8.2.2).
	const ExprFacts* facts = expr.facts;
	if ((facts->flags & kOverloadSet) == 0 || facts->functions == nullptr || facts->functions->next != nullptr ||
		static_cast<const FunctionEntity*>(facts->functions->binding->entity)->templated)
		return false;
	resolve_overloaded(expr, static_cast<FunctionEntity*>(facts->functions->binding->entity));
	return true;
}

// ------------------------------------------------------------- names

ExprFacts* Analyzer::id_expression(const syntax::IdExpr& expr, Scope* scope)
{
	const syntax::Name& name = *expr.name;
	const syntax::NameComponent& last = name.last();
	// An operator-function-id or literal-operator-id names its function
	// (13.5p4, 13.5.8).
	const bool operator_id = last.kind == syntax::NameComponentKind::Operator ||
		last.kind == syntax::NameComponentKind::LiteralOperator;
	if ((last.kind == syntax::NameComponentKind::Identifier || operator_id) && last.has_arguments)
	{
		// A template-id naming function template specializations, its
		// template-name an identifier or an operator-function-id (14.2p1:
		// `&operator+<int>`).
		LookupResult result = LookupResult();
		NamePrefix prefix = NamePrefix{ nullptr, nullptr, nullptr };
		if (operator_id)
			result = operator_function_lookup(name, scope);
		else if (!name.qualified())
			result = unqualified(scope, last.identifier, LookupFilter::Ordinary);
		else
			result = lookup_in(prefix = name_prefix(name, scope), last.identifier, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		// A variable template's specialization (see variable_template_binding),
		// a member one named through its class with the template's access
		// (11.2p5, 14.5.2p2).
		if (result.found() && variable_template_of(result.binding) != nullptr)
		{
			if (semantic() && template_depth_ == 0 && prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class)
				check_access(result.binding, static_cast<const ClassEntity*>(prefix.scope->entity), nullptr, scope,
					name.location);
			Binding* specialization = variable_template_binding(result.binding, last, scope);
			if (failed())
				return nullptr;
			return binding_expression(expr, &name, LookupResult{ specialization, nullptr, false, false }, scope);
		}
		if (!result.found() || result.binding->kind != BindingKind::Function)
			error(name.location, "template-id does not name a function template");
		const FunctionList* set = explicit_specializations(last, result.functions, scope);
		if (set == nullptr)
			return nullptr;
		ExprFacts* facts = new_facts(expr, nullptr, ValueCategory::LValue);
		facts->entity = set->binding->entity;
		facts->functions = set;
		if (set->next != nullptr || static_cast<const FunctionEntity*>(set->binding->entity)->templated ||
			set->binding->type->kind == TypeKind::Pack)
			facts->flags |= kOverloadSet;
		else
		{
			if (!callee_name_ && !require_deduced(static_cast<FunctionEntity*>(set->binding->entity), expr.location))
				return nullptr;
			facts->type = set->binding->entity->type;
			check_not_deleted(static_cast<const FunctionEntity*>(set->binding->entity), expr.location);
			odr_use(static_cast<FunctionEntity*>(set->binding->entity));
		}
		facts->type = facts->type != nullptr ? facts->type->canonical : nullptr;
		return facts;
	}
	if (last.kind == syntax::NameComponentKind::Conversion && !last.has_arguments)
		return conversion_name(expr, scope);
	const bool operator_name = operator_id && !last.has_arguments;
	if (last.kind != syntax::NameComponentKind::Identifier && !operator_name)
		error(name.location, "template and special member names are not supported here");
	LookupResult result = LookupResult();
	if (operator_name)
	{
		// 13.5p4: an operator function named explicitly; one a substituted
		// qualifier lacks (an enumeration's `T::operator new`) is the
		// substitution's failure (14.8.2p8).
		result = operator_function_lookup(name, scope);
		if (failed())
			return nullptr;
		if (!result.found() && name.qualified())
		{
			substitution_error(name.location, "undeclared name");
			return nullptr;
		}
		if (!result.found())
			error(name.location, "undeclared name");
		if (result.ambiguous)
			error(name.location, "ambiguous name");
	}
	else if (!name.qualified())
	{
		result = unqualified(scope, last.identifier, LookupFilter::Ordinary);
		if (!result.found())
			error(name.location, "undeclared name");
		if (result.ambiguous)
			error(name.location, "ambiguous name");
	}
	else
		result = lookup_name(name, scope, LookupFilter::Ordinary);
	if (failed())
		return nullptr;
	// A variable template's name alone names no variable (14.2, 14.5).
	if (variable_template_of(result.binding) != nullptr)
		error(name.location, "a variable template named without template arguments");
	return binding_expression(expr, &name, result, scope);
}

ExprFacts* Analyzer::conversion_name(const syntax::IdExpr& expr, Scope* scope)
{
	// An id-expression `operator T` or `N::operator T` (12.3.2): the
	// conversion functions conversion_lookup names in the innermost class
	// enclosing the point (3.4.1p8: member functions are found in their
	// class's scope, its bases' included) or in the class N names
	// (3.4.3.1p1), as any function name of that lookup: in a member
	// function of their class or a derived one a call through `(*this)`
	// (9.3.1p3), its subobject checked through the naming class (10.2,
	// 11.2p6; binding_expression).
	const syntax::Name& name = *expr.name;
	last_prefix_ = NamePrefix{ nullptr, nullptr, nullptr };
	const Scope* home = nullptr;
	if (name.qualified())
	{
		last_prefix_ = name_prefix(name, scope);
		if (failed())
			return nullptr;
		home = last_prefix_.scope;
	}
	else
	{
		for (home = scope; home != nullptr && home->kind != ScopeKind::Class; home = home->parent)
		{
		}
	}
	if (home == nullptr || home->kind != ScopeKind::Class || home->entity == nullptr)
	{
		substitution_error(name.location, "a conversion-function-id that names no class member");
		return nullptr;
	}
	ClassEntity* cls = static_cast<ClassEntity*>(home->entity);
	if (cls->templ != nullptr || !cls->complete)
		require_complete(cls);
	const Type* target = failed() ? nullptr : type_id(*name.last().conversion, scope);
	FunctionList* functions = target != nullptr ? conversion_lookup(cls, target->canonical, name.location) : nullptr;
	if (functions == nullptr)
		return nullptr;
	return binding_expression(expr, &name, LookupResult{ functions->binding, functions, false, false }, scope);
}

bool Analyzer::implicit_object(const ClassEntity* home)
{
	// 9.3.1p3: a non-static member of `home` named alone in a member
	// function of its class or a derived class is `(*this).m`: whether the
	// body being analyzed is one. Anonymous classes stand for their
	// enclosing one.
	const ClassEntity* current = body_ != nullptr && body_->this_type != nullptr ? ClassOf(body_->this_type->element)
																				 : nullptr;
	while (home->anonymous && home->owner != nullptr && home->owner->kind == ScopeKind::Class)
		home = static_cast<const ClassEntity*>(home->owner->entity);
	while (current != nullptr && current->anonymous && current->owner != nullptr &&
		current->owner->kind == ScopeKind::Class)
		current = static_cast<const ClassEntity*>(current->owner->entity);
	return current != nullptr && (home == current || conversions_.is_base_of(home, current));
}

ExprFacts* Analyzer::binding_expression(
	const syntax::Expr& expr, const syntax::Name* name, const LookupResult& result, Scope* scope)
{
	(void)scope;
	const Binding* binding = result.binding;
	switch (binding->kind)
	{
	case BindingKind::Variable:
	{
		VariableEntity* variable = static_cast<VariableEntity*>(binding->entity);
		const Scope* owner = variable->owner;
		if (owner != nullptr && owner->kind == ScopeKind::Class && owner->entity != nullptr)
		{
			ClassEntity* home = static_cast<ClassEntity*>(owner->entity);
			// A member of an anonymous union (or struct) member names a
			// member of the class holding its unnamed object (9.5p5).
			while (
				home->anonymous && home->storage != nullptr && home->storage->entity->owner->kind == ScopeKind::Class)
				home = static_cast<ClassEntity*>(home->storage->entity->owner->entity);
			if (home->anonymous && home->storage != nullptr)
			{
				// A member of an anonymous union at namespace or block scope:
				// an access to the union's unnamed object (9.5p5).
				const Type* type = variable->type->is_reference() ? variable->type->element : variable->type;
				ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
				facts->flags |= kAnonymousMember;
				facts->entity = variable;
				return facts;
			}
			if (last_prefix_.scope == nullptr || name == nullptr || !name->qualified())
			{
				// Found in an anonymous class's own scope (from its members'
				// default member initializers), it is named there (11.2p5).
				const ClassEntity* declaring = binding->declared_in != nullptr &&
						binding->declared_in->kind == ScopeKind::Class
					? static_cast<const ClassEntity*>(binding->declared_in->entity)
					: nullptr;
				const ClassEntity* naming = naming_class(
					declaring != nullptr && declaring->anonymous ? declaring : home, scope);
				check_access(binding, naming, variable->member ? naming : nullptr, scope, expr.location);
			}
			if (variable->member)
			{
				// 9.3.1p3: `(*this).m` in a member function of its class (or
				// a derived class); of another class (an enclosing one), only
				// in an unevaluated operand (5.1.1p13).
				if (!implicit_object(home))
				{
					evaluated_error(expr.location, "non-static data member used without an object");
					ExprFacts* facts = new_facts(expr, variable->type, ValueCategory::LValue);
					facts->entity = variable;
					return facts;
				}
				if (body_->lambda != nullptr)
					capture_this(expr.location);
				ExprFacts* object = static_cast<ExprFacts*>(arena_.allocate(sizeof(ExprFacts), alignof(ExprFacts)));
				std::memset(static_cast<void*>(object), 0, sizeof(ExprFacts));
				object->type = body_->this_type->element;
				object->category = ValueCategory::LValue;
				const ClassEntity* current = ClassOf(object->type);
				const ClassEntity* naming = name != nullptr && name->qualified() && last_prefix_.scope != nullptr &&
						last_prefix_.scope->kind == ScopeKind::Class
					? static_cast<const ClassEntity*>(last_prefix_.scope->entity)
					: current;
				if (current != nullptr && naming != nullptr &&
					!member_subobject(expr, variable, current, naming, expr.location))
					return nullptr;
				return member_of_object(expr, object, binding, nullptr, true);
			}
		}
		// A variable template's specialization declared with an omitted
		// bound has the bound its definition's initializer gives (3.9p6,
		// 8.3.4p3): instantiated when named, as a static member's is (see
		// static_constant).
		if (variable->templ != nullptr && variable->templ->arguments != nullptr &&
			variable->type->kind == TypeKind::Array && variable->type->bound == 0 && variable->type->owner == nullptr &&
			template_depth_ == 0 && semantic())
			instantiate_variable(variable);
		const Type* type = variable->type;
		if (type->is_reference())
			type = type->element;
		ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
		facts->entity = variable;
		// A structured binding of a bit-field member designates the
		// bit-field (C++17's 11.5p4).
		const BindingEntity* alias = BindingAlias(variable);
		if (alias != nullptr && alias->member != nullptr && alias->member->bit_field)
			facts->flags |= kBitField;
		if (owner != nullptr && owner->kind == ScopeKind::Class)
			static_constant(facts, variable);
		else if (variable->templ != nullptr && variable->templ->arguments != nullptr && semantic() &&
			template_depth_ == 0)
		{
			// A variable template's specialization with a constant value is
			// read as that value (the course's form: no odr-use, nothing
			// emitted).
			Constant value;
			if (IsIntegralOrUnscopedEnum(type) && variable_constant(variable, value))
			{
				facts->flags |= kStaticConstant;
				facts->value = value.bits;
			}
		}
		else if (owner != nullptr && (owner->kind == ScopeKind::Block || owner->kind == ScopeKind::Function))
			local_name(expr, facts, variable, scope);
		return facts;
	}
	case BindingKind::Parameter:
	{
		// A substituted prototype scope's parameter whose type failed (see
		// substituted_scope).
		if (binding->type == nullptr)
		{
			substitution_error(expr.location, "a parameter whose type's substitution failed");
			return nullptr;
		}
		const Type* type = ParameterObjectType(types_, binding->type);
		if (type->is_reference())
			type = type->element;
		ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
		facts->entity = binding->entity;
		local_name(expr, facts, binding->entity, scope);
		return facts;
	}
	case BindingKind::Function:
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(binding->entity);
		// A function template's name is a set too: its context selects the
		// specialization (13.4, 14.8.2).
		const bool several = (result.functions != nullptr && result.functions->next != nullptr) || function->templated;
		// The naming class of a member function set (its call checks access).
		naming_class_ = nullptr;
		if (function->owner != nullptr && function->owner->kind == ScopeKind::Class)
		{
			const ClassEntity* home = static_cast<const ClassEntity*>(function->owner->entity);
			naming_class_ = name != nullptr && name->qualified() && last_prefix_.scope != nullptr &&
					last_prefix_.scope->kind == ScopeKind::Class
				? static_cast<const ClassEntity*>(last_prefix_.scope->entity)
				: naming_class(home, scope);
		}
		// 9.3.1p3: member functions named (qualified or not) in a member
		// function of their class or a derived class are called through
		// `(*this)`; a set with a non-static member resolves with it.
		bool member = false;
		for (const FunctionList* node = result.functions; node != nullptr && !member; node = node->next)
		{
			const FunctionEntity* candidate = static_cast<const FunctionEntity*>(node->binding->entity);
			member = candidate->member && !candidate->is_static;
		}
		if (result.functions == nullptr)
			member = function->member && !function->is_static;
		if (member && body_ != nullptr && body_->this_type != nullptr)
		{
			const ClassEntity* current = ClassOf(body_->this_type->element);
			const ClassEntity* owner = function->owner->entity != nullptr &&
					function->owner->entity->kind == EntityKind::Class
				? static_cast<const ClassEntity*>(function->owner->entity)
				: nullptr;
			member = owner != nullptr && (owner == current || conversions_.is_base_of(owner, current));
		}
		else
			member = false;
		if (member)
		{
			ExprFacts* object = static_cast<ExprFacts*>(arena_.allocate(sizeof(ExprFacts), alignof(ExprFacts)));
			std::memset(static_cast<void*>(object), 0, sizeof(ExprFacts));
			object->type = body_->this_type->element;
			object->category = ValueCategory::LValue;
			// 10.2, 11.2p6: a non-static member function's subobject (its
			// naming class's path is the call's, call_naming_class).
			const ClassEntity* current = ClassOf(object->type);
			const ClassEntity* naming = naming_class_ != nullptr ? naming_class_ : current;
			if (!function->is_static && current != nullptr &&
				!member_subobject(expr, function, current, naming, expr.location))
				return nullptr;
			return member_of_object(expr, object, binding, result.functions, true);
		}
		// C++14's 7.1.6.4p11-p12: a function named but not called (its call
		// checks it) has its return type deduced.
		if (!several && !callee_name_ && !require_deduced(function, expr.location))
			return nullptr;
		// A builtin the call analysis expands (sema::Intrinsic) is no
		// function to designate: it is only called (Clang: "builtin
		// functions must be directly called"; it has no definition).
		if (!several && !callee_name_ && function->builtin == BuiltinFunction::Intrinsic)
			return substitution_error(
					   expr.location, "a builtin expanded where it is called named other than in a call"),
				   nullptr;
		ExprFacts* facts = new_facts(expr, several ? nullptr : function->type->canonical, ValueCategory::LValue);
		facts->entity = function;
		facts->functions = result.functions;
		if (several)
			facts->flags |= kOverloadSet;
		// A call's callee is checked and used as the function its
		// resolution selects (argument-dependent lookup may add others).
		else if (!callee_name_)
		{
			check_not_deleted(function, expr.location);
			odr_use(function);
		}
		return facts;
	}
	case BindingKind::Enumerator:
	{
		// A member enumerator named unqualified is access-checked here (a
		// qualified name by its lookup).
		if (name == nullptr || !name->qualified())
			check_unqualified_access(binding, scope, expr.location);
		const EnumeratorEntity* enumerator = static_cast<const EnumeratorEntity*>(binding->entity);
		if (enumerator->argument != nullptr)
			return argument_name(expr, enumerator);
		ExprFacts* facts = new_facts(expr, enumerator->value.type, ValueCategory::PRValue);
		facts->entity = enumerator;
		facts->flags |= kFolded;
		facts->value = enumerator->value.bits;
		return facts;
	}
	case BindingKind::Type:
	case BindingKind::TypeAlias:
		// In a substitution, a member that is a type where a value is
		// required is its failure (14.8.2p8).
		substitution_error(expr.location, "type name used as a value");
		return nullptr;
	default:
		error(expr.location, "namespace name used as a value");
	}
}

const ClassEntity* Analyzer::naming_class(const ClassEntity* home, const Scope* scope)
{
	// 11.2p5: an unqualified member name is named in the innermost class
	// enclosing the point that has it as a member (its class or one derived
	// from it); without one, its own class.
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		if (s->kind != ScopeKind::Class || s->entity == nullptr)
			continue;
		const ClassEntity* cls = static_cast<const ClassEntity*>(s->entity);
		if (cls == home || conversions_.is_base_of(home, cls))
			return cls;
	}
	return home;
}

void Analyzer::static_constant(ExprFacts* facts, const VariableEntity* variable)
{
	// 9.4.2p3: a const static data member of integral or enumeration type
	// initialized by a constant expression; reading it is not an odr-use.
	// An instance's member may take its initializer from its template's
	// definition.
	if (variable->constant == ConstantState::Unknown && variable->templ != nullptr &&
		IsIntegralOrUnscopedEnum(variable->type) && (variable->type->cv & kConst) != 0)
		instantiate_variable(variable);
	// One declared with an omitted bound has the type its template's
	// definition completes (3.9p6), instantiated when named (as Clang
	// instantiates it when its type must be complete).
	else if (variable->templ != nullptr && variable->type->kind == TypeKind::Array && variable->type->bound == 0 &&
		variable->type->owner == nullptr && template_depth_ == 0 && instantiate_variable(variable))
		facts->type = variable->type;
	// Named where evaluated, an instance's member's definition is emitted
	// even when its value is read as a constant (the course's form; not in
	// a template argument, a constant expression, 3.2p2).
	if (template_arguments_ == 0 && variable->templ != nullptr)
		demand_variable(variable);
	if (variable->constant != ConstantState::Value || variable->type->is_reference())
		return;
	facts->flags |= kStaticConstant;
	facts->value = variable->value.bits;
}

ExprFacts* Analyzer::this_expression(uint32_t location)
{
	// 9.3.2p1: a prvalue pointer to the member function's class, qualified
	// as the function is.
	if (body_ == nullptr || body_->this_type == nullptr)
		error(location, "`this` outside a non-static member function");
	if (body_->lambda != nullptr)
		capture_this(location);
	ExprFacts* facts = static_cast<ExprFacts*>(arena_.allocate(sizeof(ExprFacts), alignof(ExprFacts)));
	std::memset(static_cast<void*>(facts), 0, sizeof(ExprFacts));
	facts->type = body_->this_type;
	facts->category = ValueCategory::PRValue;
	return facts;
}

// ------------------------------------------------------- conversions

const Type* Analyzer::decayed(ExprFacts* facts)
{
	// 5p9 via 4.1-4.3: the prvalue an operand of a built-in operator reads.
	const Type* type = facts->type;
	if (type->kind == TypeKind::Array)
		return types_.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types_.pointer(type);
	return type->unqualified;
}

const Type* Analyzer::promoted(const Type* type)
{
	return IsIntegralOrUnscopedEnum(type) ? PromotedType(types_, type) : type->unqualified;
}

const Type* Analyzer::field_promoted(const ExprFacts* facts, const Type* type)
{
	// 4.5p3: an integral bit-field operand promotes to int when int
	// represents all its values, else to unsigned int when that does, else
	// not; one of enumeration type promotes as its type does.
	if ((facts->flags & kBitField) == 0 || facts->entity == nullptr || facts->entity->kind != EntityKind::Variable)
		return type;
	const DataMember* member = static_cast<const VariableEntity*>(facts->entity)->data_member;
	IntegralKind kind;
	if (member == nullptr || !member->bit_field || type->unqualified->kind != TypeKind::Fundamental ||
		!IntegralKindOf(type->unqualified, kind))
		return type;
	const uint64_t width = member->width < kind.bits ? member->width : kind.bits;
	if (width < 32 || (width == 32 && kind.is_signed))
		return types_.fundamental(FundamentalType::Int);
	return width == 32 ? types_.fundamental(FundamentalType::UnsignedInt) : type;
}

const Type* Analyzer::usual_arithmetic(const Type* a, const Type* b)
{
	// 5p10: the floating operand of greater conversion rank, at an equal
	// rank an extended type (its greater subrank, C++23 7.4p1.4 as GCC has
	// it in every mode). A GNU complex operand makes the result the complex
	// type of its parts' and the other operand's (C99 6.3.1.8p1's corresponding
	// real type).
	if (IsComplex(a) || IsComplex(b))
		return types_.complex(
			usual_arithmetic(IsComplex(a) ? a->unqualified->element : a, IsComplex(b) ? b->unqualified->element : b));
	const unsigned rank_a = IsFloating(a) ? FloatingRank(a->unqualified->fundamental) : 0;
	const unsigned rank_b = IsFloating(b) ? FloatingRank(b->unqualified->fundamental) : 0;
	if (rank_a != 0 || rank_b != 0)
	{
		if (rank_a != rank_b)
			return rank_a > rank_b ? a->unqualified : b->unqualified;
		const FundamentalType kind = a->unqualified->fundamental;
		const bool standard = kind == FundamentalType::Float || kind == FundamentalType::Double ||
			kind == FundamentalType::LongDouble;
		return standard ? b->unqualified : a->unqualified;
	}
	return CommonType(types_, promoted(a), promoted(b));
}

bool Analyzer::modifiable_lvalue(const ExprFacts* facts) const
{
	// 3.10p10.
	if (facts->category != ValueCategory::LValue || facts->type == nullptr)
		return false;
	const Type* type = facts->type;
	if (type->kind == TypeKind::Array || type->kind == TypeKind::Function)
		return false;
	if ((type->cv & kConst) != 0)
		return false;
	return !IsVoid(type);
}

const Conversion* Analyzer::record_conversion(ExprFacts* facts, const Conversion& conversion)
{
	++graph_.conversions;
	facts->conversion = intern_conversion(conversion);
	return facts->conversion;
}

const Conversion* Analyzer::intern_conversion(const Conversion& conversion)
{
	// Identical conversions share one record (an open-addressed table
	// owned by the unit's analysis; records live in the graph's arena).
	if ((conversion_count_ + 1) * 2 > conversion_slots_.size())
	{
		std::vector<const Conversion*> old;
		old.swap(conversion_slots_);
		conversion_slots_.assign(old.empty() ? 256 : old.size() * 2, nullptr);
		const size_t mask = conversion_slots_.size() - 1;
		for (const Conversion* entry : old)
		{
			if (entry == nullptr)
				continue;
			size_t index = HashConversion(*entry) & mask;
			while (conversion_slots_[index] != nullptr)
				index = (index + 1) & mask;
			conversion_slots_[index] = entry;
		}
	}
	const size_t mask = conversion_slots_.size() - 1;
	size_t index = HashConversion(conversion) & mask;
	while (conversion_slots_[index] != nullptr)
	{
		if (SameConversion(*conversion_slots_[index], conversion))
			return conversion_slots_[index];
		index = (index + 1) & mask;
	}
	Conversion* stored = static_cast<Conversion*>(arena_.allocate(sizeof(Conversion), alignof(Conversion)));
	*stored = conversion;
	conversion_slots_[index] = stored;
	++conversion_count_;
	return stored;
}

const Conversion* Analyzer::convert(const syntax::Expr& expr, const Type* target, bool direct, bool operand)
{
	ExprFacts* facts = facts_of(expr);
	if ((facts->flags & kOverloadSet) != 0)
	{
		FunctionEntity* function = select_overloaded(facts, target);
		if (function == nullptr)
			error(expr.location, "no overloaded function matches the target type");
		resolve_overloaded(expr, function);
	}
	if ((facts->flags & kBoundMember) != 0)
		error(expr.location, "a member function must be called");
	Conversion conversion = Conversion();
	conversions_.set_arithmetic_operand(operand);
	const bool converts = !IsVoid(facts->type) && conversions_.implicit(operand_of(facts), target, direct, conversion);
	conversions_.set_arithmetic_operand(false);
	if (!converts && (IsVoid(facts->type) || operand || !user_conversion(facts, target, conversion, direct)))
	{
		substitution_error(expr.location, "invalid implicit conversion");
		return nullptr;
	}
	conversion.initialization = !operand;
	// 8.5.3p5: only a const (non-volatile) lvalue reference binds a
	// bit-field, to a temporary copy of its value.
	if (conversion.form == ConversionForm::Reference && (facts->flags & kBitField) != 0 && !conversion.temporary &&
		!conversion.rvalue_reference && conversion.constructor == nullptr)
	{
		substitution_error(expr.location, "a non-const reference bound to a bit-field");
		return nullptr;
	}
	if (conversion.constructor != nullptr)
		apply_user_conversion(facts, conversion, expr.location);
	check_conversion_access(facts, conversion, expr.location);
	check_binding_temporary(facts, conversion, expr.location);
	odr_use_constants(expr, &conversion);
	return record_conversion(facts, conversion);
}

void Analyzer::convert_to_bool(const syntax::Expr& expr)
{
	// 4p3: `bool t(e);` (direct-initialization, so std::nullptr_t too;
	// never a scoped enumeration; a class object by a conversion function,
	// explicit ones included, 12.3.2p2).
	const Type* boolean = types_.fundamental(FundamentalType::Bool);
	convert_to_scalar(expr, boolean, true);
	convert(expr, boolean, true, true);
}

FunctionEntity* Analyzer::select_overloaded(const ExprFacts* facts, const Type* target, bool explicit_cast)
{
	// 13.4p1: the function whose type is the target's function type; a
	// non-template is preferred (13.4p4). Member functions match a
	// pointer to member of their class or a class derived from it (the
	// class is ignored in the match; 4.11p2 converts the result), and as
	// a static_cast's operand of a class it is derived from (13.4p1's
	// explicit conversion target, converted by 5.2.9p12).
	if (target->is_reference())
		target = target->element;
	target = target->unqualified;
	const Type* wanted = nullptr;
	const Type* owner = nullptr;
	if (target->kind == TypeKind::Pointer && target->element->kind == TypeKind::Function)
		wanted = target->element;
	else if (target->kind == TypeKind::Function)
		wanted = target;
	else if (target->kind == TypeKind::MemberPointer && target->element->kind == TypeKind::Function)
	{
		wanted = target->element;
		owner = target->owner;
	}
	if (wanted == nullptr)
		return nullptr;
	FunctionEntity* found = nullptr;
	bool found_template = false;
	for (const FunctionList* node = facts->functions; node != nullptr; node = node->next)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
		const bool nonstatic_member = function->member && !function->is_static;
		if (nonstatic_member != (owner != nullptr))
			continue;
		if ((owner != nullptr && function->owner->entity->type != owner &&
				!conversions_.is_base_of(ClassOf(function->owner->entity->type), ClassOf(owner)) &&
				(!explicit_cast || !conversions_.is_base_of(ClassOf(owner), ClassOf(function->owner->entity->type)))) ||
			IgnoredByOverloadResolution(function))
			continue;
		if (function->templated || (node->binding->type != nullptr && node->binding->type->kind == TypeKind::Pack))
		{
			found_template = true;
			continue;
		}
		if (function->type->canonical != wanted->canonical)
			continue;
		if (found != nullptr && found != function)
			return nullptr;
		found = function;
	}
	if (found != nullptr || !found_template)
		return found;
	// 13.4p2: a template's specialization deduced from the target type;
	// 13.4p4: of several, the one whose template is more specialized than
	// each other's (14.5.6.2, by the function types, 14.8.2.4p3).
	const size_t base = selected_specializations_.size();
	for (const FunctionList* node = facts->functions; node != nullptr; node = node->next)
	{
		FunctionEntity* primary = static_cast<FunctionEntity*>(node->binding->entity);
		if (!primary->templated && primary->primary != nullptr && node->binding->type != nullptr &&
			node->binding->type->kind == TypeKind::Pack)
			primary = primary->primary;
		if (!primary->templated || (primary->member && !primary->is_static) != (owner != nullptr))
			continue;
		// A template-id's template with its explicit arguments (see
		// explicit_specializations).
		const Type* given = node->binding->type != nullptr && node->binding->type->kind == TypeKind::Pack
			? node->binding->type
			: nullptr;
		FunctionEntity* function = deduce_for_target(primary, wanted->canonical, given);
		if (function == nullptr || function->type->canonical != wanted->canonical)
			continue;
		bool known = false;
		for (size_t i = base; i < selected_specializations_.size() && !known; ++i)
			known = selected_specializations_[i] == function;
		if (!known)
			selected_specializations_.push_back(function);
	}
	const uint32_t count = wanted->param_count;
	const Ordering form = Ordering::FunctionType;
	for (size_t i = base; i < selected_specializations_.size(); ++i)
	{
		FunctionEntity* function = selected_specializations_[i];
		if (found == nullptr ||
			(at_least_as_specialized(function->primary, found->primary, count, form) &&
				!at_least_as_specialized(found->primary, function->primary, count, form)))
			found = function;
	}
	for (size_t i = base; i < selected_specializations_.size() && found != nullptr; ++i)
	{
		FunctionEntity* other = selected_specializations_[i];
		if (other != found &&
			(!at_least_as_specialized(found->primary, other->primary, count, form) ||
				at_least_as_specialized(other->primary, found->primary, count, form)))
			found = nullptr;
	}
	selected_specializations_.resize(base);
	return found;
}

void Analyzer::resolve_overloaded(const syntax::Expr& expr, FunctionEntity* function)
{
	// Rewrites an overloaded name (or its address) to the selected function.
	check_not_deleted(function, expr.location);
	const syntax::Expr* node = &expr;
	ExprFacts* facts = facts_of(expr);
	const bool address = WithoutParentheses(node)->kind == syntax::ExprKind::Unary;
	if (address)
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(WithoutParentheses(node));
		ExprFacts* inner = facts_of(*unary->operand);
		inner->type = function->type->canonical;
		inner->entity = function;
		inner->flags &= static_cast<uint32_t>(~kOverloadSet);
		const bool member = function->member && !function->is_static;
		facts->type = member ? types_.member_pointer(function->owner->entity->type, function->type->canonical)
							 : types_.pointer(function->type->canonical);
		// 11.2p5, 11.4p1: the selected member's access through `&C::f`'s
		// naming class (which stands for the object).
		const uint32_t named = member_address_index_.find(unary);
		if (named != PointerMap::kMissing)
		{
			const MemberAddress& address = member_addresses_[named];
			check_function_access(function, address.functions != nullptr ? address.functions : facts->functions,
				address.naming, address.naming, address.scope, expr.location);
		}
	}
	else
		facts->type = function->type->canonical;
	facts->entity = function;
	facts->flags &= static_cast<uint32_t>(~kOverloadSet);
	odr_use(function);
}

// ---------------------------------------------------------- operators

ExprFacts* Analyzer::binary_chain(const syntax::BinaryExpr& expr, Scope* scope)
{
	// Left-nested chains (as long as the source makes them) are walked
	// iteratively: the left spine first, then each operator upward.
	const size_t base = binary_spine_.size();
	const syntax::Expr* node = &expr;
	while (node->kind == syntax::ExprKind::Binary)
	{
		binary_spine_.push_back(static_cast<const syntax::BinaryExpr*>(node));
		node = binary_spine_.back()->left;
	}
	const uint64_t uses = names_.dependent_uses();
	ExprFacts* result = typed(*node, scope);
	for (size_t i = binary_spine_.size(); result != nullptr && i-- > base;)
	{
		const syntax::BinaryExpr* current = binary_spine_[i];
		result = combine(*current, scope);
		// Each operator's expression is marked as typed() marks.
		if (result != nullptr && names_.dependent_uses() != uses)
			result->flags |= kDependentUse;
	}
	binary_spine_.resize(base);
	return result;
}

ExprFacts* Analyzer::combine(const syntax::BinaryExpr& expr, Scope* scope)
{
	ExprFacts* left = facts_of(*expr.left);
	if (expr.op == TokenKind::OP_DOTSTAR)
	{
		ExprFacts* right = typed_value(*expr.right, scope);
		return right != nullptr ? member_pointer_access(expr, left, right) : nullptr;
	}
	// 13.3.1.2: an operand of class or enumeration type may make the
	// expression a call of an operator function.
	if (typed(*expr.right, scope) == nullptr)
		return nullptr;
	if (ExprFacts* overloaded = MayOverload(expr.left) || MayOverload(expr.right) ? binary_operator(expr, scope)
																				  : nullptr)
		return overloaded;
	if (failed())
		return nullptr;
	// A class operand may now stand for its conversion function's result.
	left = facts_of(*expr.left);
	switch (expr.op)
	{
	case TokenKind::OP_COMMA:
	{
		// 5.18p1: the left operand is a discarded-value expression (a
		// class prvalue a temporary, 12.2p1).
		check_resolved(*expr.left);
		check_resolved(*expr.right);
		check_temporary_destruction(left, expr.left->location);
		ExprFacts* right = facts_of(*expr.right);
		ExprFacts* facts = new_facts(expr, right->type, right->category);
		facts->flags = right->flags & kBitField;
		return facts;
	}
	case TokenKind::OP_LAND:
	case TokenKind::OP_LOR:
		// 5.14, 5.15.
		check_resolved(*expr.left);
		convert_to_bool(*expr.left);
		check_resolved(*expr.right);
		convert_to_bool(*expr.right);
		return new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
	case TokenKind::OP_ARROWSTAR:
		check_resolved(*expr.right);
		return member_pointer_access(expr, left, facts_of(*expr.right));
	default:
		break;
	}
	check_resolved(*expr.left);
	check_resolved(*expr.right);
	ExprFacts* right = facts_of(*expr.right);
	if (IsComparison(expr.op))
		return comparison(expr, left, right);
	return arithmetic(expr, left, right);
}

ExprFacts* Analyzer::arithmetic(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right)
{
	const TokenKind op = expr.op;
	const Type* lt = decayed(left);
	const Type* rt = decayed(right);
	if (IsVoid(lt) || IsVoid(rt))
	{
		substitution_error(expr.location, "operand of type void");
		return nullptr;
	}
	if (op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS)
	{
		// 5.7: pointer arithmetic on pointers to complete object types.
		const bool lp = IsObjectPointer(lt), rp = IsObjectPointer(rt);
		if (lp || rp)
		{
			if (op == TokenKind::OP_MINUS && lp && rp)
			{
				if (lt->element->unqualified != rt->element->unqualified || IsVoid(lt->element))
				{
					substitution_error(expr.location, "subtraction of pointers to different types");
					return nullptr;
				}
				convert_operand(*expr.left, lt);
				convert_operand(*expr.right, rt);
				return new_facts(expr, types_.fundamental(FundamentalType::LongInt), ValueCategory::PRValue);
			}
			const Type* pointer = lp ? lt : rt;
			const syntax::Expr* index = lp ? expr.right : expr.left;
			const Type* index_type = lp ? rt : lt;
			if ((rp && op == TokenKind::OP_MINUS) || (lp && rp) || !IsIntegralOrUnscopedEnum(index_type))
			{
				substitution_error(expr.location, "invalid operands to pointer arithmetic");
				return nullptr;
			}
			if (!complete_pointee(pointer, expr.location))
				return nullptr;
			convert_operand(lp ? *expr.left : *expr.right, pointer);
			convert_operand(*index, promoted(index_type));
			return new_facts(expr, pointer, ValueCategory::PRValue);
		}
	}
	const bool integral_only = op == TokenKind::OP_MOD || op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT ||
		op == TokenKind::OP_AMP || op == TokenKind::OP_BOR || op == TokenKind::OP_XOR;
	if (lt->unqualified->kind == TypeKind::Vector || rt->unqualified->kind == TypeKind::Vector)
		return vector_arithmetic(expr, lt, rt, integral_only);
	if (!integral_only && (IsComplex(lt) || IsComplex(rt)))
		return complex_arithmetic(expr, lt, rt);
	const bool lok = integral_only ? IsIntegralOrUnscopedEnum(lt) : (IsArithmetic(lt) || IsIntegralOrUnscopedEnum(lt));
	const bool rok = integral_only ? IsIntegralOrUnscopedEnum(rt) : (IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt));
	if (!lok || !rok)
	{
		substitution_error(expr.location, "invalid operands to a binary operator");
		return nullptr;
	}
	if (op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT)
	{
		// 5.8p1: each operand is promoted; the result has the left's type.
		const Type* type = promoted(field_promoted(left, lt));
		convert_operand(*expr.left, type);
		convert_operand(*expr.right, promoted(field_promoted(right, rt)));
		return new_facts(expr, type, ValueCategory::PRValue);
	}
	const Type* type = usual_arithmetic(field_promoted(left, lt), field_promoted(right, rt));
	convert_operand(*expr.left, type);
	convert_operand(*expr.right, type);
	return new_facts(expr, type, ValueCategory::PRValue);
}

ExprFacts* Analyzer::vector_arithmetic(
	const syntax::BinaryExpr& expr, const Type* lt, const Type* rt, bool integral_only)
{
	// GNU and Clang vectors of one type, or a vector and a scalar converted
	// to its lane type (as if in every lane), lane by lane in the lane type,
	// unpromoted; %, the bitwise operators and the shifts on integral lanes
	// only (Clang's own <mmintrin.h> adds __m64 lanes so).
	const bool left_vector = lt->unqualified->kind == TypeKind::Vector;
	const Type* vector = left_vector ? lt->unqualified : rt->unqualified;
	const Type* other = left_vector ? rt->unqualified : lt->unqualified;
	const Type* lane = vector->element->unqualified;
	if (vector->owner != nullptr ||
		(other != vector &&
			(other->kind == TypeKind::Vector || !(IsArithmetic(other) || IsIntegralOrUnscopedEnum(other)))))
	{
		substitution_error(expr.location, "arithmetic on vectors of different types");
		return nullptr;
	}
	if ((integral_only && !IsIntegralType(lane)) ||
		(lane->kind == TypeKind::Fundamental && lane->fundamental == FundamentalType::Bool))
	{
		substitution_error(expr.location, "invalid operands to a binary operator");
		return nullptr;
	}
	if (other != vector && !scalar_splats(left_vector ? *expr.right : *expr.left, other, lane))
	{
		substitution_error(
			expr.location, "a scalar operand that does not convert to the vector's lanes without truncation");
		return nullptr;
	}
	convert_operand(*expr.left, left_vector || other == vector ? vector : lane);
	convert_operand(*expr.right, !left_vector || other == vector ? vector : lane);
	return new_facts(expr, vector, ValueCategory::PRValue);
}

ExprFacts* Analyzer::complex_arithmetic(const syntax::BinaryExpr& expr, const Type* lt, const Type* rt)
{
	// GNU complex + - * / (C99 6.5.5, 6.5.6): the complex type of the
	// usual arithmetic conversions; a real operand converts to its part
	// type and stays real (C99 G.5.1: x * (u + iv) is xu + ixv, as GCC
	// computes it).
	const bool lok = IsComplex(lt) || IsArithmetic(lt) || IsIntegralOrUnscopedEnum(lt);
	const bool rok = IsComplex(rt) || IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt);
	if (!lok || !rok)
	{
		substitution_error(expr.location, "invalid operands to a binary operator");
		return nullptr;
	}
	const Type* type = usual_arithmetic(lt, rt);
	convert_operand(*expr.left, IsComplex(lt) ? type : type->element);
	convert_operand(*expr.right, IsComplex(rt) ? type : type->element);
	return new_facts(expr, type, ValueCategory::PRValue);
}

bool Analyzer::complete_pointee(const Type* pointer, uint32_t location)
{
	// 5.7p1, 5.2.6p1: pointer arithmetic needs a pointer to a completely-
	// defined object type (not void, an array of unknown bound or an
	// incomplete class, whose completion may come later: not memoized).
	const Type* element = pointer->unqualified->element;
	if (IsVoid(element) || (element->kind == TypeKind::Array && element->bound == 0 && element->owner == nullptr))
		return substitution_error(location, "arithmetic on a pointer to an incomplete type");
	const ClassEntity* cls = ClassOf(element);
	if (cls != nullptr && !require_complete(cls))
		return substitution_error(location, "arithmetic on a pointer to an incomplete class", true);
	return true;
}

namespace {

// Clang's integer conversion rank (bool, the char types, short, int,
// long, long long, __int128; char16_t, char32_t and wchar_t as their
// underlying types), and getIntegerTypeOrder over it: which of two integer
// types is the greater (0 for the same rank and signedness).
unsigned IntegerRank(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::Bool:
		return 1;
	case FundamentalType::Char:
	case FundamentalType::SignedChar:
	case FundamentalType::UnsignedChar:
		return 2;
	case FundamentalType::ShortInt:
	case FundamentalType::UnsignedShortInt:
	case FundamentalType::Char16T:
		return 3;
	case FundamentalType::Int:
	case FundamentalType::UnsignedInt:
	case FundamentalType::Char32T:
	case FundamentalType::WcharT:
		return 4;
	case FundamentalType::LongInt:
	case FundamentalType::UnsignedLongInt:
		return 5;
	case FundamentalType::LongLongInt:
	case FundamentalType::UnsignedLongLongInt:
		return 6;
	default:
		return 7;
	}
}

int IntegerOrder(const Type* a, const IntegralKind& ak, const Type* b, const IntegralKind& bk)
{
	const unsigned ar = IntegerRank(a->fundamental), br = IntegerRank(b->fundamental);
	if (ak.is_signed == bk.is_signed)
		return ar == br ? 0 : ar > br ? 1 : -1;
	if (!ak.is_signed)
		return ar >= br ? 1 : -1;
	return br >= ar ? -1 : 1;
}

// A floating type's rank (__bf16 < _Float16 < float < double < long
// double < binary128) and its significand's precision in bits.
unsigned FloatingRank(FundamentalType type, unsigned& precision)
{
	switch (type)
	{
	case FundamentalType::BFloat16:
		precision = 8;
		return 1;
	case FundamentalType::Float16:
		precision = 11;
		return 2;
	case FundamentalType::Float:
	case FundamentalType::Float32:
		precision = 24;
		return 3;
	case FundamentalType::Double:
	case FundamentalType::Float64:
	case FundamentalType::Float32x:
		precision = 53;
		return 4;
	case FundamentalType::LongDouble:
	case FundamentalType::Float64x:
		precision = 64;
		return 5;
	default:
		precision = 113;
		return 6;
	}
}

// Whether `value` is exact in a floating type of `precision` bits (its
// range too: binary16's largest finite value is 65504, float's and
// double's those of the host types).
bool ExactIn(long double value, unsigned precision)
{
	if (value != value || value == 0 || precision >= 64)
		return true;
	if (precision == 24)
		return static_cast<long double>(static_cast<float>(value)) == value;
	if (precision == 53)
		return static_cast<long double>(static_cast<double>(value)) == value;
	// binary16: 11 significant bits at or above 2^-14, multiples of 2^-24
	// below; at most 65504.
	if (std::fabs(value) > 65504.0L)
		return false;
	int exponent = 0;
	std::frexp(value, &exponent);
	const int quantum = exponent - 1 < -14 ? -24 : exponent - 1 - 10;
	const long double scaled = std::ldexp(value, -quantum);
	return scaled == std::floor(scaled);
}

} // namespace

bool Analyzer::scalar_splats(const syntax::Expr& scalar, const Type* type, const Type* lane)
{
	// A scalar operand beside a GNU vector is converted to the lane type
	// only when no value it may have changes (GCC's scalar_to_vector,
	// Clang's tryGCCVectorConvertAndSplat; where they differ, Clang's
	// rule): an integer of a greater conversion rank only as a constant
	// that fits the lanes, a floating one into integer lanes of its size,
	// into floating lanes a constant exact in them or a type of no greater
	// rank (integers: of no more bits than their precision); never an
	// enumeration.
	type = type->unqualified;
	lane = lane->unqualified;
	if (!semantic() || type->kind != TypeKind::Fundamental || lane->kind != TypeKind::Fundamental)
		return type->kind == TypeKind::Fundamental || type->kind != TypeKind::Enum;
	IntegralKind lk, sk;
	const bool lane_integral = IntegralKindOf(lane, lk);
	const bool scalar_integral = IntegralKindOf(type, sk);
	unsigned lane_precision = 0, scalar_precision = 0;
	if (lane_integral && scalar_integral)
	{
		const int order = IntegerOrder(lane, lk, type, sk);
		if (order == 0)
			return true;
		Constant value;
		if (!constant_value(scalar, nullptr, value, false))
			return order > 0;
		// Its significant bits (a negative signed value's with its sign).
		unsigned bits = 0;
		if (sk.is_signed && static_cast<int64_t>(value.bits) < 0)
		{
			const uint64_t magnitude = ~value.bits;
			bits = 1;
			for (uint64_t v = magnitude; v != 0; v >>= 1)
				++bits;
		}
		else
			for (uint64_t v = value.bits; v != 0; v >>= 1)
				++bits;
		if (order < 0 && lk.bits < bits)
			return false;
		return !(sk.is_signed != lk.is_signed && bits > lk.bits);
	}
	if (lane_integral)
		return FundamentalTypeSize(lane->fundamental) == FundamentalTypeSize(type->fundamental);
	const unsigned lane_rank = FloatingRank(lane->fundamental, lane_precision);
	if (scalar_integral)
	{
		Constant value;
		if (!constant_value(scalar, nullptr, value, false))
			return sk.bits <= lane_precision;
		const long double exact = sk.is_signed ? static_cast<long double>(static_cast<int64_t>(value.bits))
											   : static_cast<long double>(value.bits);
		return ExactIn(exact, lane_precision);
	}
	const unsigned scalar_rank = FloatingRank(type->fundamental, scalar_precision);
	long double value = 0;
	if (!floating_constant(scalar, nullptr, value))
		return scalar_rank <= lane_rank;
	return ExactIn(value, lane_precision);
}

ExprFacts* Analyzer::comparison(const syntax::BinaryExpr& expr, ExprFacts* left, ExprFacts* right)
{
	// 5.9, 5.10.
	const Type* boolean = types_.fundamental(FundamentalType::Bool);
	const Type* lt = decayed(left);
	const Type* rt = decayed(right);
	const bool equality = expr.op == TokenKind::OP_EQ || expr.op == TokenKind::OP_NE;
	if ((IsComplex(lt) && (IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt) || IsComplex(rt))) ||
		(IsComplex(rt) && (IsArithmetic(lt) || IsIntegralOrUnscopedEnum(lt))))
	{
		// GNU complex values are equal when both parts are (C99 6.5.9p4),
		// in the complex type of the usual arithmetic conversions; they
		// have no order.
		if (!equality)
		{
			substitution_error(expr.location, "an ordering comparison of complex values");
			return nullptr;
		}
		const Type* type = usual_arithmetic(lt, rt);
		convert_operand(*expr.left, type);
		convert_operand(*expr.right, type);
		return new_facts(expr, boolean, ValueCategory::PRValue);
	}
	if ((IsArithmetic(lt) || IsIntegralOrUnscopedEnum(lt)) && (IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt)))
	{
		const Type* type = usual_arithmetic(field_promoted(left, lt), field_promoted(right, rt));
		convert_operand(*expr.left, type);
		convert_operand(*expr.right, type);
		return new_facts(expr, boolean, ValueCategory::PRValue);
	}
	if (lt->unqualified->kind == TypeKind::Vector || rt->unqualified->kind == TypeKind::Vector)
	{
		// GNU and Clang vectors of one type, or a vector and a scalar
		// converted to its lane type (as if in every lane), lane by lane: a
		// vector of the signed integer type of the lanes' size (a bool
		// vector's bool), each lane -1 (true) or 0.
		const bool left_vector = lt->unqualified->kind == TypeKind::Vector;
		const Type* vector = left_vector ? lt->unqualified : rt->unqualified;
		const Type* other = left_vector ? rt->unqualified : lt->unqualified;
		if (vector->owner != nullptr ||
			(other != vector &&
				(other->kind == TypeKind::Vector || !(IsArithmetic(other) || IsIntegralOrUnscopedEnum(other)))))
		{
			substitution_error(expr.location, "comparison of vectors of different types");
			return nullptr;
		}
		if (other != vector && !scalar_splats(left_vector ? *expr.right : *expr.left, other, vector->element))
		{
			substitution_error(
				expr.location, "a scalar operand that does not convert to the vector's lanes without truncation");
			return nullptr;
		}
		convert_operand(*expr.left, left_vector ? vector : (other == vector ? vector : vector->element->unqualified));
		convert_operand(*expr.right, !left_vector ? vector : (other == vector ? vector : vector->element->unqualified));
		const FundamentalType lane = vector->element->unqualified->fundamental;
		const unsigned size = FundamentalTypeSize(lane);
		const FundamentalType truth = lane == FundamentalType::Bool ? FundamentalType::Bool
			: size == 1                                             ? FundamentalType::SignedChar
			: size == 2                                             ? FundamentalType::ShortInt
			: size == 4                                             ? FundamentalType::Int
			: size == 8                                             ? FundamentalType::LongInt
																	: FundamentalType::Int128;
		return new_facts(expr, types_.vector(types_.fundamental(truth), vector->bound), ValueCategory::PRValue);
	}
	if (IsScopedEnumeration(lt) || IsScopedEnumeration(rt))
	{
		if (lt != rt)
		{
			substitution_error(expr.location, "comparison of a scoped enumeration with another type");
			return nullptr;
		}
		convert_operand(*expr.left, lt);
		convert_operand(*expr.right, rt);
		return new_facts(expr, boolean, ValueCategory::PRValue);
	}
	// Otherwise pointers, pointers to members or null pointer constants
	// (5.9p2, 5.10p1): two objects of a class without an operator function
	// are none.
	const Type* composite = composite_pointer(left, right, true);
	if (composite == nullptr || !(IsPointer(composite) || IsMemberPointer(composite) || IsNullptrType(composite)) ||
		(!equality && (IsMemberPointer(composite) || IsNullptrType(composite))))
	{
		substitution_error(expr.location, "invalid operands to a comparison");
		return nullptr;
	}
	convert_operand(*expr.left, composite);
	convert_operand(*expr.right, composite);
	return new_facts(expr, boolean, ValueCategory::PRValue);
}

ExprFacts* Analyzer::assignment(const syntax::BinaryExpr& expr, Scope* scope)
{
	// 5.17.
	ExprFacts* left = typed_value(*expr.left, scope);
	if (left == nullptr)
		return nullptr;
	if (expr.right->kind == syntax::ExprKind::BracedInit)
	{
		// 5.17p9: `x = {v}` of a scalar is `x = T{v}`, `x = {}` is `x = T{}`;
		// Overloaded assignments take the list as their right argument.
		if (left->type != nullptr &&
			(left->type->unqualified->kind == TypeKind::Class ||
				(expr.op != TokenKind::OP_ASS && left->type->unqualified->kind == TypeKind::Enum)))
		{
			braced_argument(static_cast<const syntax::BracedInitList&>(*expr.right), scope);
			if (ExprFacts* overloaded = failed() ? nullptr : binary_operator(expr, scope))
				return overloaded;
			if (failed())
				return nullptr;
			substitution_error(expr.location, "no viable assignment operator for the braced-init-list");
			return nullptr;
		}
		if (expr.op != TokenKind::OP_ASS || !(IsScalar(left->type) || IsComplex(left->type)))
		{
			substitution_error(expr.right->location, "assignment from a braced-init-list is not supported");
			return nullptr;
		}
		if (!modifiable_lvalue(left))
		{
			substitution_error(expr.location, "assignment to something that is not a modifiable lvalue");
			return nullptr;
		}
		list_initialize(static_cast<const syntax::BracedInitList&>(*expr.right), left->type->unqualified, scope);
		if (failed())
			return nullptr;
		ExprFacts* facts = new_facts(expr, left->type, ValueCategory::LValue);
		facts->flags = left->flags & kBitField;
		return facts;
	}
	if (typed(*expr.right, scope) == nullptr)
		return nullptr;
	if (ExprFacts* overloaded = MayOverload(expr.left) || MayOverload(expr.right) ? binary_operator(expr, scope)
																				  : nullptr)
		return overloaded;
	if (failed())
		return nullptr;
	// A class operand of a compound assignment may now stand for its
	// conversion function's result (13.3.1.2p7).
	left = facts_of(*expr.left);
	// 13.5.3: a class object is assigned by an operator= member (its
	// implicit copy or move assignment operator at least, 12.8p18-p20).
	// An invalid assignment in a substitution is its failure (14.8.2p8).
	if (expr.op == TokenKind::OP_ASS && left->type->unqualified->kind == TypeKind::Class)
	{
		substitution_error(expr.location, "no viable assignment operator for the class object");
		return nullptr;
	}
	if (!modifiable_lvalue(left))
	{
		substitution_error(expr.location, "assignment to something that is not a modifiable lvalue");
		return nullptr;
	}
	const Type* type = left->type;
	if (expr.op == TokenKind::OP_ASS)
	{
		if (convert(*expr.right, type->unqualified, false) == nullptr)
			return nullptr;
		ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
		facts->flags = left->flags & kBitField;
		return facts;
	}
	check_resolved(*expr.right);
	ExprFacts* right = facts_of(*expr.right);
	const TokenKind op = CompoundOperator(expr.op);
	const Type* rt = decayed(right);
	const Type* lt = type->unqualified;
	const Type* operation = lt;
	if ((op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS) && IsObjectPointer(lt))
	{
		// 5.17p7: E1 += E2 with E1 a pointer to a complete object type.
		if (!IsIntegralOrUnscopedEnum(rt))
		{
			substitution_error(expr.location, "invalid operands to a compound assignment");
			return nullptr;
		}
		if (!complete_pointee(lt, expr.location))
			return nullptr;
		convert_operand(*expr.right, promoted(rt));
	}
	else if (op == TokenKind::OP_PLUS && IsBoolType(lt) && IsObjectPointer(rt))
	{
		// E1 = E1 + E2 (5.17p7): a bool plus a pointer to a complete object
		// type, the sum converted to bool (4.12).
		if (!complete_pointee(rt, expr.location))
			return nullptr;
		convert_operand(*expr.right, rt);
		operation = rt;
	}
	else if (IsComplex(lt) &&
		(op == TokenKind::OP_PLUS || op == TokenKind::OP_MINUS || op == TokenKind::OP_STAR || op == TokenKind::OP_DIV))
	{
		// GNU: a complex E1 combined in the usual arithmetic conversions'
		// complex type (a real E2 kept real, see complex_arithmetic), the
		// result converted back to E1's type.
		if (!(IsComplex(rt) || IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt)))
		{
			substitution_error(expr.location, "invalid operands to a compound assignment");
			return nullptr;
		}
		const Type* common = usual_arithmetic(lt, rt);
		convert_operand(*expr.right, IsComplex(rt) ? common : common->element);
		operation = common;
	}
	else if (lt->kind == TypeKind::Vector)
	{
		// GNU: a vector E1 combined lane by lane with a vector of its type or
		// a scalar converted to its lanes (see vector_arithmetic).
		const bool integral_only = op == TokenKind::OP_MOD || op == TokenKind::OP_LSHIFT ||
			op == TokenKind::OP_RSHIFT || op == TokenKind::OP_AMP || op == TokenKind::OP_BOR || op == TokenKind::OP_XOR;
		const Type* lane = lt->element->unqualified;
		const Type* other = rt->unqualified;
		if (lt->owner != nullptr || (integral_only && !IsIntegralType(lane)) ||
			(lane->kind == TypeKind::Fundamental && lane->fundamental == FundamentalType::Bool) ||
			(other != lt &&
				(other->kind == TypeKind::Vector || !(IsArithmetic(other) || IsIntegralOrUnscopedEnum(other)) ||
					!scalar_splats(*expr.right, other, lane))))
		{
			substitution_error(expr.location, "invalid operands to a compound assignment");
			return nullptr;
		}
		convert_operand(*expr.right, other == lt ? lt : lane);
	}
	else
	{
		const bool integral_only = op == TokenKind::OP_MOD || op == TokenKind::OP_LSHIFT ||
			op == TokenKind::OP_RSHIFT || op == TokenKind::OP_AMP || op == TokenKind::OP_BOR || op == TokenKind::OP_XOR;
		const bool lok = integral_only ? IsIntegralType(lt) : IsArithmetic(lt);
		const bool rok = integral_only ? IsIntegralOrUnscopedEnum(rt)
									   : (IsArithmetic(rt) || IsIntegralOrUnscopedEnum(rt));
		if (!lok || !rok)
		{
			substitution_error(expr.location, "invalid operands to a compound assignment");
			return nullptr;
		}
		const bool shift = op == TokenKind::OP_LSHIFT || op == TokenKind::OP_RSHIFT;
		const Type* promoted_left = field_promoted(left, lt);
		const Type* common = shift ? promoted(rt) : usual_arithmetic(promoted_left, field_promoted(right, rt));
		convert_operand(*expr.right, common);
		operation = shift ? promoted(promoted_left) : common;
	}
	ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
	facts->flags = left->flags & kBitField;
	facts->target = operation;
	return facts;
}

ExprFacts* Analyzer::unary(const syntax::UnaryExpr& expr, Scope* scope)
{
	switch (expr.op)
	{
	case TokenKind::OP_AMP:
		return address_of(expr, scope);
	case TokenKind::OP_INC:
	case TokenKind::OP_DEC:
		return increment(expr, scope);
	default:
		break;
	}
	ExprFacts* operand = typed(*expr.operand, scope);
	if (operand == nullptr)
		return nullptr;
	if (expr.op == TokenKind::OP_STAR && (operand->flags & kOverloadSet) != 0)
	{
		// `*f` of an overloaded name: resolved by its use (a call).
		ExprFacts* facts = new_facts(expr, nullptr, ValueCategory::LValue);
		facts->flags = kOverloadSet;
		facts->functions = operand->functions;
		facts->entity = operand->entity;
		return facts;
	}
	check_resolved(*expr.operand);
	if (ExprFacts* overloaded = MayOverload(expr.operand) ? unary_operator(expr, false, scope) : nullptr)
		return overloaded;
	if (failed())
		return nullptr;
	operand = facts_of(*expr.operand);
	const Type* type = decayed(operand);
	switch (expr.op)
	{
	case TokenKind::OP_STAR:
	{
		// 5.3.1p1: indirection through a pointer to an object or function
		// type yields an lvalue.
		if (!IsPointer(type) || IsVoid(type->element))
		{
			substitution_error(expr.location, "indirection requires a pointer to an object or function type");
			return nullptr;
		}
		convert_operand(*expr.operand, type);
		return new_facts(expr, type->element, ValueCategory::LValue);
	}
	case TokenKind::OP_LNOT:
		convert_to_bool(*expr.operand);
		return new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
	case TokenKind::OP_PLUS:
		if (IsPointer(type))
		{
			convert_operand(*expr.operand, type);
			return new_facts(expr, type, ValueCategory::PRValue);
		}
		// fall through
	case TokenKind::OP_MINUS:
		if (!IsArithmetic(type) && !IsIntegralOrUnscopedEnum(type) && !IsComplex(type) &&
			!(type->unqualified->kind == TypeKind::Vector && type->unqualified->owner == nullptr))
		{
			substitution_error(expr.location, "unary + or - requires an arithmetic operand");
			return nullptr;
		}
		break;
	case TokenKind::OP_COMPL:
		// GNU: `~` of a complex value is its conjugate; of a vector of
		// integral lanes, each lane's complement.
		if (!IsIntegralOrUnscopedEnum(type) && !IsComplex(type) &&
			!(type->unqualified->kind == TypeKind::Vector && type->unqualified->owner == nullptr &&
				IsIntegralType(type->unqualified->element)))
		{
			substitution_error(expr.location, "~ requires an integral operand");
			return nullptr;
		}
		break;
	default:
		error(expr.location, "unsupported unary operator");
	}
	// A complex operand keeps its type: GCC and Clang promote no integral
	// parts here (-, + and ~ act on each part in the part type, wrapping;
	// the binary operators' usual arithmetic conversions promote them); a
	// vector its lanes'.
	const Type* result = IsComplex(type) || type->unqualified->kind == TypeKind::Vector
		? type->unqualified
		: promoted(field_promoted(operand, type));
	convert_operand(*expr.operand, result);
	return new_facts(expr, result, ValueCategory::PRValue);
}

ExprFacts* Analyzer::address_of(const syntax::UnaryExpr& expr, Scope* scope)
{
	// 5.3.1p3: `&C::m` of a qualified non-static member is a pointer to
	// member (not when parenthesized, p4); otherwise the operand is an
	// lvalue (or a function) and the result points to it.
	const syntax::Expr* operand = expr.operand;
	// A qualified template-id names function template specializations (a
	// member template's pointer to member is outside the stage).
	if (operand->kind == syntax::ExprKind::Id && static_cast<const syntax::IdExpr*>(operand)->name->qualified() &&
		!static_cast<const syntax::IdExpr*>(operand)->name->last().has_arguments)
	{
		const syntax::Name& name = *static_cast<const syntax::IdExpr*>(operand)->name;
		const syntax::NameComponent& last = name.last();
		const bool conversion = last.kind == syntax::NameComponentKind::Conversion;
		if (last.kind != syntax::NameComponentKind::Identifier && last.kind != syntax::NameComponentKind::Operator &&
			last.kind != syntax::NameComponentKind::LiteralOperator && !conversion)
			error(name.location, "template and special member names are not supported here");
		const NamePrefix prefix = name_prefix(name, scope);
		if (failed())
			return nullptr;
		if (prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class)
		{
			LookupResult result = LookupResult();
			if (conversion)
			{
				// `&C::operator T` (12.3.2, 3.4.3.1p1): the class's
				// conversion functions to T (conversion_lookup).
				ClassEntity* cls = static_cast<ClassEntity*>(prefix.scope->entity);
				require_complete(cls);
				const Type* target = failed() ? nullptr : type_id(*last.conversion, scope);
				result.functions = target != nullptr ? conversion_lookup(cls, target->canonical, name.location)
													 : nullptr;
				if (result.functions == nullptr)
					return nullptr;
				result.binding = result.functions->binding;
			}
			else
			{
				implicit_members_for(prefix.scope, name_key(last));
				result = names_.qualified(prefix.scope, name_key(last), LookupFilter::Ordinary);
			}
			if (!result.found())
			{
				// A member its substituted class lacks (14.8.2p8); a class
				// being defined may declare it later (not memoized).
				substitution_error(name.location, "undeclared member",
					!static_cast<const ClassEntity*>(prefix.scope->entity)->complete);
				return nullptr;
			}
			if (result.ambiguous)
			{
				substitution_error(name.location, "ambiguous member");
				return nullptr;
			}
			const Binding* binding = result.binding;
			const Type* owner = prefix.entity->type;
			// 11.4p1: a protected member's pointer to member names a class
			// derived from the one its access is through (the naming class
			// stands for the object).
			const ClassEntity* naming = ClassOf(owner);
			if (binding->kind == BindingKind::Variable && static_cast<VariableEntity*>(binding->entity)->member)
			{
				const VariableEntity* member = static_cast<const VariableEntity*>(binding->entity);
				if (member->type->is_reference())
					error(name.location, "pointer to member of reference type");
				check_access(binding, naming, naming, scope, name.location);
				if (failed())
					return nullptr;
				ExprFacts* inner = new_facts(*operand, member->type, ValueCategory::LValue);
				inner->entity = member;
				return new_facts(
					expr, types_.member_pointer(MemberClassOf(member)->type, member->type), ValueCategory::PRValue);
			}
			if (binding->kind == BindingKind::Function)
			{
				FunctionEntity* function = static_cast<FunctionEntity*>(binding->entity);
				const bool several = (result.functions != nullptr && result.functions->next != nullptr) ||
					function->templated;
				if (!several)
					check_not_deleted(function, name.location);
				if (!several && !require_deduced(function, name.location))
					return nullptr;
				if (!several && (!function->member || function->is_static))
				{
					check_access(binding, naming, naming, scope, name.location);
					if (failed())
						return nullptr;
					ExprFacts* inner = new_facts(*operand, function->type->canonical, ValueCategory::LValue);
					inner->entity = function;
					odr_use(function);
					return new_facts(expr, types_.pointer(function->type->canonical), ValueCategory::PRValue);
				}
				if (!several)
				{
					check_access(binding, naming, naming, scope, name.location);
					if (failed())
						return nullptr;
				}
				ExprFacts* inner = new_facts(
					*operand, several ? nullptr : function->type->canonical, ValueCategory::LValue);
				inner->entity = function;
				inner->functions = result.functions;
				ExprFacts* facts = new_facts(expr, nullptr, ValueCategory::PRValue);
				facts->entity = function;
				facts->functions = result.functions;
				if (several)
				{
					inner->flags |= kOverloadSet;
					facts->flags |= kOverloadSet;
					member_address_index_.set(&expr, static_cast<uint32_t>(member_addresses_.size()));
					member_addresses_.push_back(MemberAddress{ naming, scope, nullptr });
					return facts;
				}
				facts->type = types_.member_pointer(function->owner->entity->type, function->type->canonical);
				(void)owner;
				odr_use(function);
				return facts;
			}
		}
	}
	// `&C::f<A>`: C is the naming class of the specialization the
	// arguments (or the target, 13.4) select, whose access its template's
	// declaration gives (11.2p5, 11.4p1, 14.5.2p2).
	const ClassEntity* template_naming = nullptr;
	const FunctionList* template_functions = nullptr;
	if (operand->kind == syntax::ExprKind::Id && static_cast<const syntax::IdExpr*>(operand)->name->qualified() &&
		static_cast<const syntax::IdExpr*>(operand)->name->last().has_arguments)
	{
		const syntax::Name& name = *static_cast<const syntax::IdExpr*>(operand)->name;
		const NamePrefix prefix = name_prefix(name, scope);
		if (failed())
			return nullptr;
		if (prefix.scope != nullptr && prefix.scope->kind == ScopeKind::Class && prefix.entity != nullptr)
		{
			template_naming = ClassOf(prefix.entity->type);
			template_functions =
				names_.qualified(prefix.scope, name_key(name.last()), LookupFilter::Ordinary).functions;
		}
	}
	ExprFacts* inner = typed(*operand, scope);
	if (inner == nullptr)
		return nullptr;
	if ((inner->flags & kOverloadSet) != 0)
	{
		ExprFacts* facts = new_facts(expr, nullptr, ValueCategory::PRValue);
		facts->flags = kOverloadSet;
		facts->functions = inner->functions;
		facts->entity = inner->entity;
		if (template_naming != nullptr)
		{
			member_address_index_.set(&expr, static_cast<uint32_t>(member_addresses_.size()));
			member_addresses_.push_back(MemberAddress{ template_naming, scope, template_functions });
		}
		return facts;
	}
	if (template_naming != nullptr && inner->entity != nullptr && inner->entity->kind == EntityKind::Function)
	{
		check_function_access(static_cast<const FunctionEntity*>(inner->entity), template_functions, template_naming,
			template_naming, scope, expr.location);
		if (failed())
			return nullptr;
	}
	// `&C::f<A>` naming a non-static member function template's
	// specialization is a pointer to member too (5.3.1p3).
	if (operand->kind == syntax::ExprKind::Id && static_cast<const syntax::IdExpr*>(operand)->name->qualified() &&
		inner->entity != nullptr && inner->entity->kind == EntityKind::Function &&
		static_cast<const FunctionEntity*>(inner->entity)->member &&
		!static_cast<const FunctionEntity*>(inner->entity)->is_static)
	{
		const FunctionEntity* function = static_cast<const FunctionEntity*>(inner->entity);
		ExprFacts* facts = new_facts(expr,
			types_.member_pointer(function->owner->entity->type, function->type->canonical), ValueCategory::PRValue);
		facts->entity = inner->entity;
		return facts;
	}
	check_resolved(*operand);
	if (ExprFacts* overloaded = MayOverload(expr.operand) ? unary_operator(expr, false, scope) : nullptr)
		return overloaded;
	if (failed())
		return nullptr;
	if (inner->category != ValueCategory::LValue || (inner->flags & kBitField) != 0)
	{
		substitution_error(expr.location,
			inner->category != ValueCategory::LValue ? "taking the address of an rvalue"
													 : "taking the address of a bit-field");
		return nullptr;
	}
	odr_use_constants(*operand, nullptr);
	return new_facts(expr, types_.pointer(inner->type), ValueCategory::PRValue);
}

ExprFacts* Analyzer::increment(const syntax::UnaryExpr& expr, Scope* scope)
{
	// 5.3.2 (prefix: an lvalue) and 5.2.6 (postfix: a prvalue of the
	// operand's cv-unqualified type).
	ExprFacts* operand = typed_value(*expr.operand, scope);
	if (operand == nullptr)
		return nullptr;
	if (ExprFacts* overloaded =
			MayOverload(expr.operand) ? unary_operator(expr, expr.kind == syntax::ExprKind::Postfix, scope) : nullptr)
		return overloaded;
	if (failed())
		return nullptr;
	// A class operand may now stand for its conversion function's result.
	operand = facts_of(*expr.operand);
	if (!modifiable_lvalue(operand))
	{
		substitution_error(expr.location, "increment or decrement of something that is not a modifiable lvalue");
		return nullptr;
	}
	const Type* type = operand->type;
	const bool decrement = expr.op == TokenKind::OP_DEC;
	// GNU: a complex operand's real part is stepped.
	if (IsBoolType(type) ? decrement : !(IsArithmetic(type) || IsObjectPointer(type) || IsComplex(type)))
	{
		substitution_error(expr.location, "invalid operand of increment or decrement");
		return nullptr;
	}
	if (IsObjectPointer(type) && !complete_pointee(type->unqualified, expr.location))
		return nullptr;
	if (expr.kind == syntax::ExprKind::Postfix)
		return new_facts(expr, type->unqualified, ValueCategory::PRValue);
	ExprFacts* facts = new_facts(expr, type, ValueCategory::LValue);
	facts->flags = operand->flags & kBitField;
	return facts;
}

ExprFacts* Analyzer::subscript(const syntax::SubscriptExpr& expr, Scope* scope)
{
	// 5.2.1: E1[E2] is *((E1)+(E2)), one operand a pointer to an object
	// type (arrays decay), the other of integral or unscoped enumeration
	// type; an lvalue.
	if (expr.index->kind == syntax::ExprKind::BracedInit)
	{
		// 13.5.5p1: x[{...}] of a class object x is x.operator[]({...}), the
		// list converting as an argument (13.3.3.1.5); a built-in
		// subscript's operand is an expression (5.2.1p1).
		ExprFacts* object = typed_value(*expr.base, scope);
		if (object == nullptr)
			return nullptr;
		if (object->type == nullptr || object->type->unqualified->kind != TypeKind::Class)
		{
			substitution_error(expr.index->location, "a braced-init-list subscript of no class object");
			return nullptr;
		}
		braced_argument(static_cast<const syntax::BracedInitList&>(*expr.index), scope);
		syntax::Expr* pair[2] = { expr.base, expr.index };
		syntax::Slice<syntax::Expr*> operands;
		operands.data = pair;
		operands.size = 2;
		if (ExprFacts* overloaded = failed() ? nullptr
											 : overloaded_operator(expr, TokenKind::OP_LSQUARE, operands, false, scope))
			return overloaded;
		if (!failed())
			substitution_error(expr.location, "no viable subscript operator for the braced-init-list");
		return nullptr;
	}
	ExprFacts* base = typed_value(*expr.base, scope);
	ExprFacts* index = base != nullptr ? typed_value(*expr.index, scope) : nullptr;
	if (index == nullptr)
		return nullptr;
	{
		syntax::Expr* pair[2] = { expr.base, expr.index };
		syntax::Slice<syntax::Expr*> operands;
		operands.data = pair;
		operands.size = 2;
		if (ExprFacts* overloaded = overloaded_operator(expr, TokenKind::OP_LSQUARE, operands, false, scope))
			return overloaded;
		if (failed())
			return nullptr;
	}
	base = facts_of(*expr.base);
	index = facts_of(*expr.index);
	const Type* bt = decayed(base);
	const Type* it = decayed(index);
	if (base->type->unqualified->kind == TypeKind::Vector && base->type->unqualified->owner == nullptr &&
		IsIntegralOrUnscopedEnum(it))
	{
		// A GNU vector's lane (GCC, Clang): of a vector lvalue an lvalue,
		// else (and a bool vector's, a bit) a prvalue.
		convert_operand(*expr.index, promoted(it));
		const Type* lane = types_.qualified(base->type->unqualified->element, base->type->cv);
		const bool bits = lane->unqualified->fundamental == FundamentalType::Bool;
		return new_facts(expr, bits ? lane->unqualified : lane,
			base->category == ValueCategory::LValue && !bits ? ValueCategory::LValue : ValueCategory::PRValue);
	}
	bool commuted = false;
	if (!IsObjectPointer(bt) && IsObjectPointer(it))
	{
		std::swap(bt, it);
		commuted = true;
	}
	if (!IsObjectPointer(bt) || IsVoid(bt->element) || !IsIntegralOrUnscopedEnum(it))
	{
		substitution_error(expr.location, "invalid subscript operands");
		return nullptr;
	}
	convert_operand(commuted ? *expr.index : *expr.base, bt);
	convert_operand(commuted ? *expr.base : *expr.index, promoted(it));
	ExprFacts* facts = new_facts(expr, bt->element, ValueCategory::LValue);
	if (commuted)
		facts->flags |= kCommuted;
	return facts;
}

// ------------------------------------------------------------- members

ExprFacts* Analyzer::member_access(const syntax::MemberExpr& expr, Scope* scope)
{
	// 5.2.5: E1.E2 with E1 of complete class type, E1->E2 as (*E1).E2;
	// E2 is looked up in the class (3.4.5), a qualified E2 in the class
	// its nested-name-specifier names there.
	ExprFacts* object = typed_value(*expr.base, scope);
	if (object == nullptr)
		return nullptr;
	// A member of a class prvalue (or its operator->) is a temporary's
	// (12.2p1).
	check_temporary_destruction(object, expr.location);
	if (expr.op == TokenKind::OP_ARROW)
	{
		// 13.5.6: while the operand is a class object, it is replaced by
		// the call of its operator-> (a class with an operator-> that yields
		// itself would never end: bounded).
		for (uint32_t depth = 0; object->type->unqualified->kind == TypeKind::Class; ++depth)
		{
			if (depth == 64)
				error(expr.location, "operator-> chain too long");
			object = arrow_operator(expr, object, scope);
			if (object == nullptr)
				return nullptr;
		}
	}
	const Type* type = object->type;
	if (expr.op == TokenKind::OP_ARROW)
	{
		const Type* pointer = decayed(object);
		if (!IsPointer(pointer))
		{
			substitution_error(expr.location, "-> requires a pointer to a class");
			return nullptr;
		}
		convert_operand(*expr.base, pointer);
		type = pointer->element;
	}
	const syntax::Name& name = *expr.member;
	const syntax::NameComponent& last = name.last();
	if (last.kind == syntax::NameComponentKind::Destructor)
		return destructor_access(expr, type, scope);
	const ClassEntity* cls = ClassOf(type);
	if (cls == nullptr)
	{
		substitution_error(expr.location, "member access on a non-class");
		return nullptr;
	}
	// 5.1.1p3: outside a member function body (in its trailing return
	// type), *this need not be complete: the members its class declared
	// so far are visible; so are they to any member access appearing in
	// the class's definition (CWG 1397's wording, as the hosts accept).
	if ((!require_complete(cls) || cls->scope == nullptr) &&
		(cls->scope == nullptr || (!NamesThis(expr) && !encloses(cls->scope, scope))))
	{
		substitution_error(expr.location, "member access on an incomplete class", true);
		return nullptr;
	}
	if (!cls->complete)
		++incomplete_classes_seen_;
	// E2 may be an operator-function-id (13.5p4: `a.operator+(b)`) or a
	// conversion-function-id, qualified or not, naming the conversion
	// functions for its type (12.3.2, 3.4.5p7).
	if (last.kind == syntax::NameComponentKind::Conversion && !last.has_arguments)
		return conversion_member(expr, object, cls, type, scope);
	if (last.kind != syntax::NameComponentKind::Identifier && last.kind != syntax::NameComponentKind::Operator)
		error(name.location, "operator, template and special member names are not supported here");
	const ClassEntity* qualifier = name.qualified() ? member_qualifier(name, cls, scope) : cls;
	if (qualifier == nullptr)
		return nullptr;
	Scope* search = qualifier->scope;
	implicit_members_for(search, name_key(last));
	LookupResult result = names_.qualified(search, name_key(last), LookupFilter::Ordinary);
	if (!result.found())
	{
		substitution_error(name.location, "no member of that name");
		return nullptr;
	}
	if (result.ambiguous)
	{
		substitution_error(name.location, "ambiguous member name");
		return nullptr;
	}
	// A template-id names the member function templates of its name (their
	// call takes its arguments as explicit ones, 14.8.1).
	if (last.has_arguments)
	{
		result.functions = member_templates(result, name.location);
		result.binding = result.functions->binding;
	}
	// 11.2p5: named in the class searched; a member function's access is
	// its call's (after overload resolution).
	const ClassEntity* naming = static_cast<const ClassEntity*>(search->entity);
	naming_class_ = naming;
	// 11.2p6, 5.2.5p5: a non-static member's subobject.
	const Entity* named = result.binding->entity;
	const bool nonstatic = named != nullptr &&
		((named->kind == EntityKind::Variable && static_cast<const VariableEntity*>(named)->member) ||
			(named->kind == EntityKind::Function && static_cast<const FunctionEntity*>(named)->member &&
				!static_cast<const FunctionEntity*>(named)->is_static));
	if (nonstatic && !member_subobject(expr, named, cls, naming, name.location))
		return nullptr;
	if (result.binding->kind != BindingKind::Function)
		check_access(result.binding, naming, cls, scope, name.location);
	ExprFacts object_view = *object;
	object_view.type = type;
	if (expr.op == TokenKind::OP_ARROW)
		object_view.category = ValueCategory::LValue;
	return member_of_object(expr, &object_view, result.binding, result.functions, false);
}

FunctionList* Analyzer::member_templates(const LookupResult& result, uint32_t location)
{
	// The function templates among a member name's functions.
	FunctionList* head = nullptr;
	FunctionList* tail = nullptr;
	for (const FunctionList* node = result.binding->kind == BindingKind::Function ? result.functions : nullptr;
		node != nullptr; node = node->next)
	{
		if (!static_cast<const FunctionEntity*>(node->binding->entity)->templated)
			continue;
		FunctionList* item = static_cast<FunctionList*>(arena_.allocate(sizeof(FunctionList), alignof(FunctionList)));
		*item = FunctionList{ node->binding, nullptr };
		if (tail == nullptr)
			head = item;
		else
			tail->next = item;
		tail = item;
	}
	if (head == nullptr)
		error(location, "a template-id names no member function template");
	return head;
}

ExprFacts* Analyzer::conversion_member(
	const syntax::MemberExpr& expr, ExprFacts* object, const ClassEntity* cls, const Type* type, Scope* scope)
{
	// `E1.operator T`, `E1.N::operator T` (N found as 3.4.5p4 says): the
	// conversion functions conversion_lookup names in the object's class or
	// N, as a bound member function the call resolves, named in that class
	// (11.2p5). Its subobject, and the functions' class's in it, are unique
	// (5.2.5p5, 10.2): checked here when the candidates share their class
	// (a using-declared one beside the class's own may differ).
	const syntax::Name& name = *expr.member;
	const ClassEntity* naming = name.qualified() ? member_qualifier(name, cls, scope) : cls;
	if (naming == nullptr)
		return nullptr;
	const Type* target = type_id(*name.last().conversion, scope);
	if (target == nullptr)
		return nullptr;
	FunctionList* functions = conversion_lookup(naming, target->canonical, name.location);
	if (functions == nullptr)
		return nullptr;
	const Entity* first = functions->binding->entity;
	bool shared = true;
	for (const FunctionList* node = functions->next; node != nullptr && shared; node = node->next)
		shared = node->binding->entity->owner == first->owner;
	if (shared && !member_subobject(expr, first, cls, naming, name.location))
		return nullptr;
	naming_class_ = naming;
	ExprFacts object_view = *object;
	object_view.type = type;
	if (expr.op == TokenKind::OP_ARROW)
		object_view.category = ValueCategory::LValue;
	return member_of_object(expr, &object_view, functions->binding, functions, false);
}

ExprFacts* Analyzer::arrow_operator(const syntax::MemberExpr& expr, ExprFacts* object, Scope* scope)
{
	// 13.5.6: x->m for a class object x is (x.operator->())->m, with the
	// member operator-> overload resolution selects for x (13.3.1.2p3: no
	// non-member or built-in candidates). The call is built as syntax, in
	// one arena record, and becomes the access's operand: the one rewrite
	// of the tree, so later phases lower the call as any member call and
	// the access through its result.
	struct ArrowCall
	{
		syntax::NameComponent component;
		syntax::Name name;
		syntax::MemberExpr callee;
		syntax::CallExpr call;
	};
	// In a substitution, an operand without one is its failure (14.8.2p8).
	const ClassEntity* cls = ClassOf(object->type);
	if (!require_complete(cls) || cls->scope == nullptr)
	{
		substitution_error(expr.location, "-> on an object of incomplete class type", true);
		return nullptr;
	}
	const LookupResult found = names_.qualified(cls->scope, operator_key(TokenKind::OP_ARROW), LookupFilter::Ordinary);
	if (!found.found() || found.binding->kind != BindingKind::Function || found.ambiguous)
	{
		// 10.2p7: members of different bases are an invalid set.
		substitution_error(expr.location,
			found.ambiguous ? "ambiguous member name"
							: "-> requires a pointer or an object of a class with operator->");
		return nullptr;
	}
	ArrowCall* built = static_cast<ArrowCall*>(arena_.allocate(sizeof(ArrowCall), alignof(ArrowCall)));
	built->component = syntax::NameComponent();
	built->component.kind = syntax::NameComponentKind::Operator;
	built->component.op = TokenKind::OP_ARROW;
	built->component.location = expr.location;
	built->name = syntax::Name();
	built->name.components = syntax::Slice<syntax::NameComponent>{ &built->component, 1 };
	built->name.location = expr.location;
	built->callee = syntax::MemberExpr();
	built->callee.kind = syntax::ExprKind::Member;
	built->callee.location = expr.location;
	built->callee.op = TokenKind::OP_DOT;
	built->callee.base = expr.base;
	built->callee.member = &built->name;
	built->call = syntax::CallExpr();
	built->call.kind = syntax::ExprKind::Call;
	built->call.location = expr.location;
	built->call.callee = &built->callee;
	built->call.arguments = syntax::Slice<syntax::Expr*>{ nullptr, 0 };
	ExprFacts view = *object;
	member_of_object(built->callee, &view, found.binding, found.functions, false);
	expression_scope_ = scope;
	adl_functions_.clear();
	ExprFacts* result = resolved_call(built->call, found.functions, &view, false, cls);
	const_cast<syntax::MemberExpr&>(expr).base = &built->call;
	return result;
}

const ClassEntity* Analyzer::member_qualifier(const syntax::Name& name, const ClassEntity* cls, Scope* scope)
{
	// 3.4.5p4: the nested-name-specifier of a qualified E2: its first name
	// is looked up in the object's class, then in the context of the
	// expression (after a leading ::, in the global namespace); each later
	// one in what the names before it denote (3.4.3). A simple-template-id
	// among them (`a.A<int>::f`, `this->base<T>::f`) names a class
	// template specialization whose arguments are looked up in the
	// expression's context (3.4.5p6). It names the object's class or a
	// base of it (5.2.5p5).
	NamePrefix named = name.global ? NamePrefix{ graph_.global, graph_.global->entity }
								   : NamePrefix{ nullptr, nullptr };
	for (uint32_t i = 0; i + 1 < name.components.size; ++i)
	{
		const syntax::NameComponent& component = name.components[i];
		if (component.kind != syntax::NameComponentKind::Identifier)
			error(name.location, "unsupported qualified member name");
		LookupResult prefix = LookupResult();
		if (named.entity != nullptr)
			prefix = lookup_in(named, component.identifier, LookupFilter::Prefix);
		else
		{
			// A member of the object's class is named there (11.2p5: a
			// base's injected-class-name through a protected or private
			// base is not accessible outside, 11.1p5).
			prefix = names_.qualified(cls->scope, component.identifier, LookupFilter::Prefix);
			if (prefix.found() && !prefix.ambiguous)
				check_access(prefix.binding, cls, nullptr, scope, name.location);
			else if (!prefix.found())
				prefix = names_.unqualified(scope, component.identifier, LookupFilter::Prefix);
			if (failed())
				return nullptr;
		}
		// In a substitution, a name that is missing, names no class or
		// names no base is its failure (14.8.2p8): null.
		if (!prefix.found())
		{
			substitution_error(name.location, "undeclared name in a member access");
			return nullptr;
		}
		const Binding* binding = prefix.binding;
		if (component.has_arguments)
		{
			binding = template_id_binding(binding, component, scope);
			if (failed())
				return nullptr;
			if (binding == nullptr)
				error(component.location, "a template-id in a nested-name-specifier that names no class template");
		}
		named = scope_of_binding(binding, name.location);
		if (failed())
			return nullptr;
	}
	const ClassEntity* base = named.entity != nullptr && named.entity->kind == EntityKind::Class
		? static_cast<const ClassEntity*>(named.entity)
		: nullptr;
	if (base == nullptr || (base != cls && !conversions_.is_base_of(base, cls)))
	{
		substitution_error(name.location, "qualified member name does not name a base class");
		return nullptr;
	}
	return base;
}

ExprFacts* Analyzer::destructor_access(const syntax::MemberExpr& expr, const Type* object, Scope* scope)
{
	// 12.4p12-p13: E.~T() or E->~T() calls the destructor of a class
	// object; 5.2.4: for a scalar object it is a pseudo-destructor call,
	// which only evaluates E. T names the object's type (looked up in the
	// class of the object and in the context, 3.4.5p3). E.C::~T() calls the
	// destructor of C, the object's class or a base (its subobject's; no
	// virtual call, 12.4p13), T looked up where C was (3.4.3p6: C's scope,
	// where its injected-class-name is, then the context).
	const syntax::NameComponent& last = expr.member->last();
	const bool qualified = expr.member->qualified();
	if (last.destructor_decltype)
		error(expr.location, "a destructor name outside the supported forms");
	const ClassEntity* cls = ClassOf(object);
	const bool complete = cls != nullptr && require_complete(cls) && cls->scope != nullptr;
	if (qualified && !complete)
		error(expr.location, "a qualified destructor name for an object of no complete class type");
	if (qualified && (cls = member_qualifier(*expr.member, cls, scope)) == nullptr)
		return nullptr;
	LookupResult found = LookupResult();
	if (complete)
		found = names_.qualified(cls->scope, last.identifier, LookupFilter::Elaborated);
	if (!found.found())
		found = names_.unqualified(scope, last.identifier, LookupFilter::Elaborated);
	if (!found.found() || (found.binding->kind != BindingKind::Type && found.binding->kind != BindingKind::TypeAlias))
		error(expr.location, "the destructor name does not name a type");
	const Binding* named = found.binding;
	if (last.has_arguments)
	{
		// `p->~vector<std::string>()`: the template-id names the class
		// template specialization (14.2) of the template the name found --
		// its injected-class-name in the object's class (14.6.1p1) or the
		// template itself in the context.
		named = template_id_binding(found.binding, last, scope);
		if (failed())
			return nullptr;
		if (named == nullptr)
			error(expr.location, "the destructor name's template-id names no class template");
	}
	const Type* target = qualified ? cls->type : object->unqualified;
	if (!qualified && !last.has_arguments && binding_type(named, expr.location)->unqualified != target)
	{
		// 3.4.5p3: either lookup may name the object's type. A member
		// typedef need not agree with a type parameter in the context.
		const LookupResult context = names_.unqualified(scope, last.identifier, LookupFilter::Elaborated);
		if (context.found() &&
			(context.binding->kind == BindingKind::Type || context.binding->kind == BindingKind::TypeAlias))
			named = context.binding;
	}
	if (binding_type(named, expr.location)->unqualified != target)
	{
		substitution_error(expr.location, "the destructor name does not name the object's type");
		return nullptr;
	}
	if (cls == nullptr)
	{
		ExprFacts* facts = new_facts(expr,
			types_.function(types_.fundamental(FundamentalType::Void), nullptr, 0, false, 0, RefQualifier::None),
			ValueCategory::PRValue);
		facts->flags |= kBoundMember | kPseudoDestructor;
		return facts;
	}
	FunctionEntity* destructor = cls->destructor;
	if (destructor == nullptr)
		error(expr.location, "a class without a destructor");
	check_not_deleted(destructor, expr.location);
	check_special_access(destructor, cls, scope, expr.location, ClassOf(object));
	ExprFacts* facts = new_facts(expr, destructor->type->canonical, ValueCategory::PRValue);
	facts->entity = destructor;
	facts->flags |= kBoundMember;
	return facts;
}

ExprFacts* Analyzer::member_of_object(const syntax::Expr& expr, ExprFacts* object, const Binding* member,
	const FunctionList* functions, bool implicit_this)
{
	const uint16_t implicit = implicit_this ? kImplicitThis : 0;
	switch (member->kind)
	{
	case BindingKind::Variable:
	{
		VariableEntity* variable = static_cast<VariableEntity*>(member->entity);
		const Type* type = variable->type;
		if (!variable->member)
		{
			// A static data member.
			ExprFacts* facts = new_facts(expr, type->is_reference() ? type->element : type, ValueCategory::LValue);
			facts->entity = variable;
			facts->flags |= implicit;
			static_constant(facts, variable);
			return facts;
		}
		// 5.2.5p4: an lvalue object gives an lvalue, otherwise an xvalue;
		// the object's cv-qualifiers join the member's (mutable aside).
		ValueCategory category = object->category == ValueCategory::LValue ? ValueCategory::LValue
																		   : ValueCategory::XValue;
		if (type->is_reference())
		{
			type = type->element;
			category = ValueCategory::LValue;
		}
		else
			type = types_.qualified(
				type, static_cast<uint8_t>(object->type->cv & (variable->is_mutable ? ~kConst : 0xff)));
		ExprFacts* facts = new_facts(expr, type, category);
		facts->entity = variable;
		facts->flags |= implicit | (variable->bit_field ? kBitField : 0);
		return facts;
	}
	case BindingKind::Function:
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(member->entity);
		const bool several = (functions != nullptr && functions->next != nullptr) || function->templated;
		// 5.2.5p4: a static member function so named is an lvalue.
		ExprFacts* facts = new_facts(expr, several ? nullptr : function->type->canonical,
			function->is_static && !several ? ValueCategory::LValue : ValueCategory::PRValue);
		facts->entity = function;
		facts->functions = functions;
		facts->flags |= implicit | kBoundMember;
		if (several)
			facts->flags |= kOverloadSet;
		return facts;
	}
	case BindingKind::Enumerator:
	{
		const EnumeratorEntity* enumerator = static_cast<const EnumeratorEntity*>(member->entity);
		ExprFacts* facts = new_facts(expr, enumerator->value.type, ValueCategory::PRValue);
		facts->entity = enumerator;
		facts->flags |= kFolded;
		facts->value = enumerator->value.bits;
		return facts;
	}
	default:
		substitution_error(expr.location, "member name does not denote a data member or member function");
		return nullptr;
	}
}

// --------------------------------------------------------------- casts

ExprFacts* Analyzer::cast_expression(const syntax::CastExpr& expr, Scope* scope)
{
	const Type* target = type_id(*expr.type, scope);
	if (target != nullptr && expr.cast == TokenKind::KW_DYNAMIC_CAST && semantic())
		return dynamic_cast_expression(expr, target, scope);
	ExprFacts* facts = target != nullptr ? explicit_conversion(expr, target, expr.operand, expr.cast, scope) : nullptr;
	if (facts == nullptr)
		return nullptr;
	if (expr.cast == TokenKind::KW_DYNAMIC_CAST && semantic())
	{
		// 5.2.7p3, p5: to the operand's own class or a base, a dynamic_cast
		// is the static conversion; any other needs the run-time check
		// (5.2.7p8, RTTI), outside PA13.
		const Type* from = facts_of(*expr.operand)->type;
		const ClassEntity* source = nullptr;
		const ClassEntity* goal = nullptr;
		if (target->kind == TypeKind::Pointer)
		{
			const Type* pointer = from->kind == TypeKind::Array ? nullptr : from->unqualified;
			source = pointer != nullptr && pointer->kind == TypeKind::Pointer ? ClassOf(pointer->element) : nullptr;
			goal = ClassOf(target->element);
		}
		else if (target->is_reference())
		{
			source = ClassOf(from);
			goal = ClassOf(target->element);
		}
		if (source == nullptr || goal == nullptr || (source != goal && !conversions_.is_base_of(goal, source)))
		{
			// p6: its operand then points to or is an object of a
			// polymorphic class (in a substitution, its failure).
			if (source == nullptr || !source->polymorphic)
			{
				substitution_error(expr.location, "a dynamic_cast of an operand of no polymorphic class");
				return nullptr;
			}
			error(expr.location, "a dynamic_cast needing a run-time check is not supported here");
		}
	}
	return facts;
}

ExprFacts* Analyzer::functional_cast(const syntax::FunctionalCastExpr& expr, Scope* scope)
{
	// 5.2.3: T(e) is the cast (T)e; T() and T{} value-initialize.
	Specifiers specifiers;
	this->specifiers(expr.type, scope, specifiers, false);
	if (failed())
		return nullptr;
	if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
		error(expr.location, "functional cast without a type");
	const Type* target = specifiers.type;
	if (target->unqualified->kind == TypeKind::Class)
	{
		// 5.2.3: `T(args)` direct-initializes a prvalue of the class, `T{...}`
		// list-initializes one, `T()` and `T{}` value-initialize it.
		syntax::Initializer view = syntax::Initializer();
		view.kind = expr.braced != nullptr ? syntax::InitializerKind::Braced : syntax::InitializerKind::Parenthesized;
		view.expression = expr.braced;
		view.arguments = expr.arguments;
		view.location = expr.location;
		return class_prvalue(expr, target, view, scope);
	}
	// An array type named by a typedef-name: `A{...}` is a prvalue array
	// the list initializes (8.5.4p3, 5.2.3p3); a GNU vector's list its
	// lanes.
	if (expr.braced != nullptr && (target->unqualified->kind == TypeKind::Vector || PartList(target, *expr.braced)))
	{
		list_initialize(*expr.braced, target, scope);
		return failed() ? nullptr : new_facts(expr, target->unqualified, ValueCategory::PRValue);
	}
	if (expr.braced != nullptr && target->kind == TypeKind::Array)
	{
		const ExprFacts* list = braced_list(*expr.braced, target, scope);
		return failed() ? nullptr : new_facts(expr, list->type, ValueCategory::PRValue);
	}
	if (expr.braced != nullptr)
	{
		// 8.5.4p3: one element without narrowing (p7).
		if (expr.braced->elements.size > 1)
		{
			substitution_error(expr.location, "list-initialization of a scalar from several values");
			return nullptr;
		}
		if (expr.braced->elements.size == 1)
		{
			const syntax::Expr& element = *expr.braced->elements[0];
			if (typed_value(element, scope) == nullptr)
				return nullptr;
			const Conversion* conversion = convert(element, target, true);
			if (conversion == nullptr || !check_narrowing(element, conversion, target, scope))
				return nullptr;
			return new_facts(expr, target->unqualified, ValueCategory::PRValue);
		}
	}
	else if (expr.arguments.size == 1)
		return explicit_conversion(expr, target, expr.arguments[0], TokenKind::OP_LPAREN, scope);
	else if (expr.arguments.size > 1)
	{
		substitution_error(expr.location, "functional cast of a non-class type from several values");
		return nullptr;
	}
	// 8.5p7: a value-initialized scalar is zero-initialized; a class
	// object's default constructor runs.
	if (target->unqualified->kind == TypeKind::Class)
	{
		ExprFacts* facts = new_facts(expr, target->unqualified, ValueCategory::PRValue);
		FunctionEntity* constructor = implicit_constructor(static_cast<ClassEntity*>(target->unqualified->entity));
		if (constructor == nullptr)
			error(expr.location, "no default constructor for the class");
		check_not_deleted(constructor, expr.location);
		facts->entity = constructor;
		odr_use(constructor);
		return facts;
	}
	// 5.2.3p2: void() is a void prvalue (void{} too, as CWG 2351 resolved).
	if (IsVoid(target))
		return new_facts(expr, target->unqualified, ValueCategory::PRValue);
	// A GNU complex or vector value: zero parts or lanes (no folded value).
	if (IsComplex(target) || target->unqualified->kind == TypeKind::Vector)
		return new_facts(expr, target->unqualified, ValueCategory::PRValue);
	if (!IsScalar(target))
		error(expr.location, "value-initialization of this type is not supported");
	ExprFacts* facts = new_facts(expr, target->unqualified, ValueCategory::PRValue);
	facts->flags |= kFolded;
	facts->value = 0;
	return facts;
}

ExprFacts* Analyzer::explicit_conversion(
	const syntax::Expr& expr, const Type* target, const syntax::Expr* operand, TokenKind cast, Scope* scope)
{
	// static_cast (5.2.9), the C-style and functional casts (5.4, which
	// also allow what const_cast and reinterpret_cast do), and the other
	// named casts for the procedural slice. In a substitution a cast none
	// of them allows is its failure (14.8.2p8).
	ExprFacts* source = typed(*operand, scope);
	if (source == nullptr)
		return nullptr;
	const bool c_style = cast == TokenKind::OP_LPAREN;
	const bool reinterpret = c_style || cast == TokenKind::KW_REINTERPET_CAST;
	const bool constness = c_style || cast == TokenKind::KW_CONST_CAST;
	if ((source->flags & kOverloadSet) != 0)
	{
		FunctionEntity* function = select_overloaded(
			source, target, cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST);
		if (function == nullptr)
			error(expr.location, "no overloaded function matches the cast's type");
		resolve_overloaded(*operand, function);
	}
	check_resolved(*operand);
	if (target->is_reference())
	{
		// A glvalue of the referenced type: lvalue for an lvalue reference
		// (or a function), xvalue for an rvalue reference to an object. The
		// C style tries each cast in turn (5.4p4): const_cast between
		// similar types (5.2.11p4); static_cast between related classes
		// (5.2.9p2-p3, with const_cast's freedom in the C style) or as the
		// binding `T t(e);` makes, perhaps to a temporary (p4), recorded;
		// reinterpret_cast of the glvalue, not casting away constness
		// (5.2.10p11, 5.2.11p9). An lvalue reference binds an rvalue only
		// as `T t(e);` does (5.2.9p4); the other forms cast lvalues.
		const Type* referenced = target->element;
		const bool rvalue = target->kind == TypeKind::RValueReference;
		const bool glvalue_cast = rvalue || source->category == ValueCategory::LValue;
		const Type* from_pointer = types_.pointer(source->type);
		const Type* to_pointer = types_.pointer(referenced);
		const bool keeps_const = !ConversionRules::casts_away_constness(from_pointer, to_pointer);
		const bool static_kind = cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST;
		bool allowed = glvalue_cast && constness && ConversionRules::const_castable(from_pointer, to_pointer);
		uint32_t hierarchy = 0;
		Conversion conversion = Conversion();
		bool converted = false;
		if (!allowed && static_kind)
		{
			const bool related = conversions_.reference_related(referenced, source->type) ||
				conversions_.reference_related(source->type, referenced);
			allowed = glvalue_cast && related && (keeps_const || constness);
			const ClassEntity* from_class = allowed ? ClassOf(source->type) : nullptr;
			const ClassEntity* to_class = allowed ? ClassOf(referenced) : nullptr;
			if (from_class != nullptr && to_class != nullptr && from_class != to_class)
				hierarchy = conversions_.is_base_of(to_class, from_class) ? kBaseCast : kDerivedCast;
			// An unambiguous base, accessible but in the C style (5.4p4).
			expression_scope_ = scope;
			if ((hierarchy == kDerivedCast && !downcast_base(from_class, to_class, !c_style, expr.location)) ||
				(hierarchy == kBaseCast && !upcast_base(from_class, to_class, !c_style, expr.location)))
				return nullptr;
			if (!allowed &&
				(conversions_.implicit(operand_of(source), target, true, conversion) ||
					user_conversion(source, target, conversion, true)))
			{
				expression_scope_ = scope;
				if (conversion.constructor != nullptr)
					apply_user_conversion(source, conversion, expr.location);
				check_conversion_access(source, conversion, expr.location, !c_style);
				if (failed())
					return nullptr;
				record_conversion(source, conversion);
				allowed = true;
				converted = true;
			}
		}
		if (!allowed && reinterpret)
			allowed = glvalue_cast && (keeps_const || constness);
		if (!allowed)
		{
			substitution_error(expr.location,
				glvalue_cast ? "invalid cast to a reference type" : "cast of an rvalue to an lvalue reference");
			return nullptr;
		}
		odr_use_constants(*operand, converted ? &conversion : nullptr);
		const ValueCategory category = !rvalue || referenced->kind == TypeKind::Function ? ValueCategory::LValue
																						 : ValueCategory::XValue;
		ExprFacts* facts = new_facts(expr, referenced, category);
		facts->flags |= kReferenceCast | hierarchy;
		facts->entity = source->entity;
		facts->target = target;
		return facts;
	}
	// 5.2.9p6: a cast to void discards the operand (a class prvalue a
	// temporary, 12.2p1).
	if (IsVoid(target))
	{
		check_temporary_destruction(source, expr.location);
		return new_facts(expr, target->unqualified, ValueCategory::PRValue);
	}
	const Type* goal = target->unqualified;
	if (goal->kind == TypeKind::Class && cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST)
	{
		// 5.2.9p4, 5.4p4: `T t(e);`, a class prvalue direct-initialized by
		// the constructor the operand selects (explicit ones included).
		ClassEntity* entity = static_cast<ClassEntity*>(goal->entity);
		if (!require_complete(entity) || entity->scope == nullptr)
			error(expr.location, "a cast to an incomplete class type");
		if (!check_abstract_object(goal, expr.location))
			return nullptr;
		expression_scope_ = scope;
		FunctionEntity* constructor = nullptr;
		if (same_class_source(entity, *operand))
			constructor = copy_initialization(entity, *operand, ConstructionKind::Direct, expr.location);
		else
		{
			syntax::Expr* argument = const_cast<syntax::Expr*>(operand);
			syntax::Slice<syntax::Expr*> arguments;
			arguments.data = &argument;
			arguments.size = 1;
			constructor = select_constructor(entity, arguments, ConstructionKind::Direct, expr.location);
		}
		if (failed())
			return nullptr;
		// A class prvalue keeps the target's cv-qualifiers (3.10p4).
		ExprFacts* facts = new_facts(expr, target, ValueCategory::PRValue);
		facts->entity = constructor;
		return facts;
	}
	const Type* from = source->type->kind == TypeKind::Class ? source->type : decayed(source);
	if (IsMemberPointer(goal) && cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST &&
		(!c_style || static_member_pointer_cast(source, from, goal)))
		return member_pointer_cast(expr, source, from, goal, c_style);
	Conversion conversion = Conversion();
	if (cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST &&
		conversions_.implicit(operand_of(source), goal, true, conversion))
	{
		// 5.4p4: a C-style cast may convert to an inaccessible base (not
		// to an ambiguous one).
		expression_scope_ = scope;
		check_conversion_access(source, conversion, expr.location, !c_style);
		record_conversion(source, conversion);
		return new_facts(expr, goal, ValueCategory::PRValue);
	}
	// 5.2.9p4: a class operand by a conversion function (explicit ones
	// too: `T t(e);` is direct-initialization).
	if (cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST && ClassOf(source->type) != nullptr &&
		user_conversion(source, goal, conversion, true))
	{
		expression_scope_ = scope;
		apply_user_conversion(source, conversion, expr.location);
		record_conversion(source, conversion);
		return new_facts(expr, goal, ValueCategory::PRValue);
	}
	bool allowed = false;
	uint32_t hierarchy = 0;
	if (cast != TokenKind::KW_CONST_CAST && cast != TokenKind::KW_REINTERPET_CAST)
	{
		// 5.2.9p9-p13: the inverses of standard conversions.
		// Floating values too since CWG 1094 (in N3485).
		if (goal->kind == TypeKind::Enum && (IsIntegralType(from) || from->kind == TypeKind::Enum || IsFloating(from)))
			allowed = true;
		else if (IsScopedEnumeration(from) && (IsIntegralType(goal) || IsFloating(goal)))
			allowed = true;
		else if (IsPointer(from) && IsPointer(goal))
		{
			const Type* pf = from->element;
			const Type* pg = goal->element;
			const bool cv_ok = constness || !ConversionRules::casts_away_constness(from, goal);
			if (IsVoid(pf) && pg->kind != TypeKind::Function && cv_ok)
				allowed = true;
			else if (ClassOf(pf) != nullptr && ClassOf(pg) != nullptr &&
				conversions_.is_base_of(ClassOf(pf), ClassOf(pg)) && cv_ok)
			{
				expression_scope_ = scope;
				if (!downcast_base(ClassOf(pf), ClassOf(pg), !c_style, expr.location))
					return nullptr;
				allowed = true;
				hierarchy = kDerivedCast;
			}
			// 5.4p4: a C-style cast to a base casting away constness is the
			// derived-to-base conversion (to an unambiguous base, of any
			// access), then a const_cast.
			else if (constness && ClassOf(pf) != nullptr && ClassOf(pg) != nullptr &&
				conversions_.is_base_of(ClassOf(pg), ClassOf(pf)))
			{
				if (!upcast_base(ClassOf(pf), ClassOf(pg), false, expr.location))
					return nullptr;
				allowed = true;
				hierarchy = kBaseCast;
			}
		}
	}
	if (!allowed && constness && ConversionRules::const_castable(from, goal))
		allowed = true;
	if (!allowed && reinterpret)
	{
		// 5.2.10: pointers to pointers and member pointers alike (not
		// casting away constness but in the C style, p2; a pointer to data
		// member only to another, a pointer to member function only to
		// another, p10), pointers to and from integers wide enough.
		// A pointer converts to an integral type large enough for it,
		// std::nullptr_t to any integral type (5.2.10p4).
		IntegralKind width = IntegralKind{ 0, false };
		IntegralKindOf(goal, width);
		if ((IsPointer(from) && IsPointer(goal)) ||
			(IsMemberPointer(from) && IsMemberPointer(goal) &&
				(from->element->kind == TypeKind::Function) == (goal->element->kind == TypeKind::Function)))
			allowed = constness || !ConversionRules::casts_away_constness(from, goal);
		else if (IsPointer(from) && IsIntegralType(goal) && width.bits >= 64)
			allowed = true;
		else if (from->kind == TypeKind::Fundamental && from->fundamental == FundamentalType::NullptrT &&
			IsIntegralType(goal))
			allowed = true;
		else if (IsIntegralOrUnscopedEnum(from) && IsPointer(goal))
			allowed = true;
		else if (from == goal)
			allowed = true;
		else
			allowed = vector_reinterpretation(from, goal);
	}
	if (!allowed)
	{
		substitution_error(expr.location, "invalid explicit conversion");
		return nullptr;
	}
	Conversion carrier = Conversion();
	carrier.target = goal;
	carrier.result = goal;
	carrier.rank = ConversionRank::Conversion;
	record_conversion(source, carrier);
	ExprFacts* facts = new_facts(expr, goal, ValueCategory::PRValue);
	facts->flags |= hierarchy;
	return facts;
}

ExprFacts* Analyzer::size_of(const syntax::SizeofExpr& expr, Scope* scope)
{
	// 5.3.3: of a type or of an unevaluated operand's type; a size_t
	// prvalue (unsigned long under the SysV ABI).
	// A parenthesized name the parse could not tell from a type-id (a
	// member type of an instantiated class, or one a complete-class context
	// declares later) is one when it names a type (8.2p2: the type-id
	// reading wins).
	const syntax::Name* named = expr.type == nullptr && expr.parenthesized && expr.operand->kind == syntax::ExprKind::Id
		? static_cast<const syntax::IdExpr*>(expr.operand)->name
		: nullptr;
	const Type* type = expr.type != nullptr ? type_id(*expr.type, scope) : nullptr;
	if (failed())
		return nullptr;
	if (named != nullptr && named->last().kind == syntax::NameComponentKind::Identifier)
	{
		const LookupResult found = lookup_name(*named, scope, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		if (found.binding->kind == BindingKind::Type || found.binding->kind == BindingKind::TypeAlias)
			type = binding_type(found.binding, named->location);
	}
	// So is a type named with an empty argument list, `T()`: the type-id of
	// a function returning T, whose size is ill-formed (5.3.3p1).
	if (type == nullptr && expr.parenthesized && empty_functional_cast(*expr.operand, scope))
	{
		substitution_error(expr.location, "sizeof of a function type");
		return nullptr;
	}
	if (type == nullptr)
	{
		++unevaluated_;
		ExprFacts* operand = typed(*expr.operand, scope);
		--unevaluated_;
		if (operand == nullptr)
			return nullptr;
		if (resolve_lone_function(*expr.operand))
			operand = facts_of(*expr.operand);
		if ((operand->flags & (kOverloadSet | kBoundMember)) != 0 || operand->type->kind == TypeKind::Function)
		{
			substitution_error(expr.location, "sizeof of a function");
			return nullptr;
		}
		if ((operand->flags & kBitField) != 0)
		{
			substitution_error(expr.location, "sizeof of a bit-field");
			return nullptr;
		}
		// A class prvalue operand is a temporary, unevaluated (12.2p1); a
		// type transform's or typeof's result names a type and makes none.
		if (WithoutParentheses(expr.operand)->kind != syntax::ExprKind::BuiltinTrait)
			check_temporary_destruction(operand, expr.location);
		type = operand->type;
	}
	if (type->is_reference())
		type = type->element;
	// 5.3.3p1: nor of an incomplete type, an array of unknown bound among
	// them (3.9p5).
	uint64_t size = 0, alignment = 0;
	if (type->kind == TypeKind::Function || IsVoid(type) ||
		(type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr && template_depth_ == 0) ||
		!type_size(type, size, alignment, expr.location))
	{
		substitution_error(expr.location, "sizeof of an incomplete or function type", ClassOf(type) != nullptr);
		return nullptr;
	}
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::UnsignedLongInt), ValueCategory::PRValue);
	facts->value = size;
	if (SpecializationType(type))
		facts->flags |= kSpecializationQuery;
	return facts;
}

bool Analyzer::empty_functional_cast(const syntax::Expr& expr, Scope* scope)
{
	// Whether an expression could be a type-id `T()` (8.2p2): a
	// function-style cast of no arguments, or a call of no arguments whose
	// callee names a type (the parse cannot tell a named type there).
	if (expr.kind == syntax::ExprKind::FunctionalCast)
	{
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(expr);
		return cast.arguments.size == 0 && cast.braced == nullptr;
	}
	if (expr.kind != syntax::ExprKind::Call || static_cast<const syntax::CallExpr&>(expr).arguments.size != 0)
		return false;
	const syntax::Expr* callee = static_cast<const syntax::CallExpr&>(expr).callee;
	if (callee->kind != syntax::ExprKind::Id)
		return false;
	const syntax::Name& name = *static_cast<const syntax::IdExpr*>(callee)->name;
	if (name.last().kind != syntax::NameComponentKind::Identifier)
		return false;
	const LookupResult found = name.qualified()
		? lookup_name(name, scope, LookupFilter::Ordinary)
		: names_.unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
	return !failed() && found.found() && !found.ambiguous &&
		(found.binding->kind == BindingKind::Type || found.binding->kind == BindingKind::TypeAlias);
}

ExprFacts* Analyzer::braced_list(const syntax::BracedInitList& list, const Type* target, Scope* scope)
{
	// 8.5.1, 8.5.4 for arrays of the slice: each element copy-initializes
	// the next array element; the list stands for the array object.
	// Elements consumed by brace elision as aggregate_element does; an
	// omitted bound is the number of elements initialized (8.5.1p4).
	++empty_list_epoch_;
	uint32_t next = 0;
	uint64_t count = 0;
	for (; target->bound == 0 ? next < list.elements.size : count < target->bound; ++count)
		aggregate_element(list, next, target->element, scope);
	if (next < list.elements.size)
		error(list.location, "too many initializers for the array");
	if (target->bound == 0 && count != 0)
		target = types_.array(target->element, count);
	return new_facts(list, target, ValueCategory::LValue);
}

} // namespace sema
