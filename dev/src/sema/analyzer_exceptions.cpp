#include "sema/analyzer.h"

#include <algorithm>
#include <functional>

#include "sema/template_packs.h"

// Exception facts of expressions: the noexcept operator (5.3.7)
// over a typed operand, and noexcept-specifications whose constant
// expression is evaluated on first need (15.4p1; DR 1330: an instance's is
// instantiated when needed, and a member's is a complete-class context,
// 9.2p2).
namespace sema {

namespace {

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// A function template's own template parameters and its function
// parameters of dependent types, by name.
struct OwnNames
{
	static const uint32_t kMaximum = 32;
	const Identifier* names[kMaximum];
	uint32_t count;

	bool has(const Identifier* name) const
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			if (names[i] == name)
				return true;
		}
		return false;
	}
};

bool ExprNamesOwn(const syntax::Expr* expr, const OwnNames& own, uint32_t depth);

bool NameNamesOwn(const syntax::Name& name, const OwnNames& own, uint32_t depth)
{
	// An unqualified identifier among them, or one in a template argument
	// or a decltype-specifier of any component.
	for (uint32_t i = 0; i < name.components.size; ++i)
	{
		const syntax::NameComponent& component = name.components[i];
		if (component.kind == syntax::NameComponentKind::Identifier && i == 0 && !name.global &&
			own.has(component.identifier))
			return true;
		if (component.kind == syntax::NameComponentKind::Decltype &&
			ExprNamesOwn(component.decltype_operand, own, depth + 1))
			return true;
		for (const syntax::TemplateArgument& argument : component.arguments)
		{
			if (argument.expression != nullptr && ExprNamesOwn(argument.expression, own, depth + 1))
				return true;
			if (argument.type != nullptr)
			{
				for (const syntax::DeclSpecifier& specifier : argument.type->specifiers)
				{
					if ((specifier.kind == syntax::DeclSpecKind::Name &&
							NameNamesOwn(*specifier.name, own, depth + 1)) ||
						(specifier.kind == syntax::DeclSpecKind::Decltype &&
							ExprNamesOwn(specifier.operand, own, depth + 1)))
						return true;
				}
			}
		}
	}
	return false;
}

bool TypeNamesOwn(const syntax::TypeId* type, const OwnNames& own, uint32_t depth)
{
	if (type == nullptr)
		return false;
	for (const syntax::DeclSpecifier& specifier : type->specifiers)
	{
		if ((specifier.kind == syntax::DeclSpecKind::Name && NameNamesOwn(*specifier.name, own, depth + 1)) ||
			(specifier.kind == syntax::DeclSpecKind::Decltype && ExprNamesOwn(specifier.operand, own, depth + 1)))
			return true;
	}
	return false;
}

bool ExprNamesOwn(const syntax::Expr* expr, const OwnNames& own, uint32_t depth)
{
	// Whether an expression names one of `own` where a name means the
	// parameter (so it depends on the template, 14.6.2.2p3, 14.6.2.3p2);
	// false for forms not examined.
	if (expr == nullptr || depth > 64)
		return false;
	switch (expr->kind)
	{
	case syntax::ExprKind::Id:
		return NameNamesOwn(*static_cast<const syntax::IdExpr*>(expr)->name, own, depth);
	case syntax::ExprKind::Paren:
		return ExprNamesOwn(static_cast<const syntax::ParenExpr*>(expr)->inner, own, depth + 1);
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		return ExprNamesOwn(static_cast<const syntax::BinaryExpr*>(expr)->left, own, depth + 1) ||
			ExprNamesOwn(static_cast<const syntax::BinaryExpr*>(expr)->right, own, depth + 1);
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return ExprNamesOwn(static_cast<const syntax::UnaryExpr*>(expr)->operand, own, depth + 1);
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr* choice = static_cast<const syntax::ConditionalExpr*>(expr);
		return ExprNamesOwn(choice->condition, own, depth + 1) || ExprNamesOwn(choice->then_expr, own, depth + 1) ||
			ExprNamesOwn(choice->else_expr, own, depth + 1);
	}
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr* call = static_cast<const syntax::CallExpr*>(expr);
		if (ExprNamesOwn(call->callee, own, depth + 1))
			return true;
		for (const syntax::Expr* argument : call->arguments)
		{
			if (ExprNamesOwn(argument, own, depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::Member:
		return ExprNamesOwn(static_cast<const syntax::MemberExpr*>(expr)->base, own, depth + 1);
	case syntax::ExprKind::Subscript:
		return ExprNamesOwn(static_cast<const syntax::SubscriptExpr*>(expr)->base, own, depth + 1) ||
			ExprNamesOwn(static_cast<const syntax::SubscriptExpr*>(expr)->index, own, depth + 1);
	case syntax::ExprKind::Cast:
		return TypeNamesOwn(static_cast<const syntax::CastExpr*>(expr)->type, own, depth) ||
			ExprNamesOwn(static_cast<const syntax::CastExpr*>(expr)->operand, own, depth + 1);
	case syntax::ExprKind::Sizeof:
		return TypeNamesOwn(static_cast<const syntax::SizeofExpr*>(expr)->type, own, depth) ||
			ExprNamesOwn(static_cast<const syntax::SizeofExpr*>(expr)->operand, own, depth + 1);
	case syntax::ExprKind::SizeofPack:
		return own.has(static_cast<const syntax::SizeofPackExpr*>(expr)->pack);
	case syntax::ExprKind::TypeTrait:
		return TypeNamesOwn(static_cast<const syntax::TypeTraitExpr*>(expr)->type, own, depth) ||
			ExprNamesOwn(static_cast<const syntax::TypeTraitExpr*>(expr)->operand, own, depth + 1);
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(expr);
		for (const syntax::DeclSpecifier& specifier : cast->type)
		{
			if (specifier.kind == syntax::DeclSpecKind::Name && NameNamesOwn(*specifier.name, own, depth + 1))
				return true;
		}
		for (const syntax::Expr* argument : cast->arguments)
		{
			if (ExprNamesOwn(argument, own, depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::PackExpansion:
		return ExprNamesOwn(static_cast<const syntax::PackExpansionExpr*>(expr)->pattern, own, depth + 1);
	case syntax::ExprKind::BuiltinTrait:
		for (const syntax::TypeId* type : static_cast<const syntax::BuiltinTraitExpr*>(expr)->types)
		{
			if (TypeNamesOwn(type, own, depth))
				return true;
		}
		return false;
	default:
		return false;
	}
}

// The noexcept-specification's constant expression when it is not the
// literal `true` (15.4p1), else null.
const syntax::Expr* NoexceptOperand(const syntax::DeclaratorSuffix* suffix)
{
	if (suffix == nullptr)
		return nullptr;
	for (const syntax::FunctionQualifier& qualifier : suffix->qualifiers)
	{
		if (qualifier.kind != syntax::FunctionQualifierKind::Noexcept || !qualifier.has_operand)
			continue;
		const syntax::Expr* operand = WithoutParentheses(qualifier.noexcept_operand);
		if (operand->kind == syntax::ExprKind::KeywordLiteral &&
			static_cast<const syntax::KeywordLiteralExpr*>(operand)->keyword == TokenKind::KW_TRUE)
			return nullptr;
		return qualifier.noexcept_operand;
	}
	return nullptr;
}

} // namespace

bool Analyzer::NonThrowingSpecification(const syntax::DeclaratorSuffix* suffix)
{
	// 15.4p12: `throw()`, `noexcept` and `noexcept(true)` do not allow any
	// exception (another noexcept operand is evaluated when needed, see
	// function_nothrow).
	if (suffix == nullptr)
		return false;
	for (const syntax::FunctionQualifier& qualifier : suffix->qualifiers)
	{
		if (qualifier.kind == syntax::FunctionQualifierKind::Throw && qualifier.throw_types.empty())
			return true;
		if (qualifier.kind != syntax::FunctionQualifierKind::Noexcept)
			continue;
		const syntax::Expr* operand = qualifier.has_operand ? WithoutParentheses(qualifier.noexcept_operand) : nullptr;
		if (operand == nullptr ||
			(operand->kind == syntax::ExprKind::KeywordLiteral &&
				static_cast<const syntax::KeywordLiteralExpr*>(operand)->keyword == TokenKind::KW_TRUE))
			return true;
	}
	return false;
}

bool Analyzer::HasExceptionSpecification(const syntax::DeclaratorSuffix* suffix)
{
	if (suffix == nullptr)
		return false;
	for (const syntax::FunctionQualifier& qualifier : suffix->qualifiers)
	{
		if (qualifier.kind == syntax::FunctionQualifierKind::Throw ||
			qualifier.kind == syntax::FunctionQualifierKind::Noexcept)
			return true;
	}
	return false;
}

bool Analyzer::DeclaratorSpecifiesExceptions(const syntax::Declarator* declarator)
{
	// A function declarator of the declarator itself (not of a parameter)
	// with an exception-specification.
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		for (const syntax::DeclaratorSuffix& suffix : declarator->suffixes)
		{
			if (suffix.kind == syntax::DeclaratorSuffixKind::Function && HasExceptionSpecification(&suffix))
				return true;
		}
	}
	return false;
}

void Analyzer::record_throw_specification(
	FunctionEntity* function, const syntax::DeclaratorSuffix* suffix, Scope* scope)
{
	// A first declaration's dynamic-exception-specification (see
	// resolve_throw_specification); a function template's in its own terms.
	// A member's declared in its class names its types in a complete-class
	// context (9.2p2 as DR 1330 extends it), once the outermost class is
	// complete (see finish_class_exceptions).
	if (suffix == nullptr || (template_depth_ != 0 && !function->templated))
		return;
	for (const syntax::FunctionQualifier& qualifier : suffix->qualifiers)
	{
		if (qualifier.kind != syntax::FunctionQualifierKind::Throw)
			continue;
		if (class_depth_ != 0 && function->member && function->owner == member_scope_ && !function->templated &&
			qualifier.throw_types.size != 0)
			pending_throws_.push_back(PendingThrow{ function, &qualifier, scope });
		else
			resolve_throw_specification(function, qualifier, scope);
		return;
	}
}

const Type* Analyzer::AdjustedThrowType(TypeTable& types, const Type* type)
{
	if (type->kind == TypeKind::Array)
		return types.pointer(type->element);
	if (type->kind == TypeKind::Function)
		return types.pointer(type);
	return type->is_reference() ? type : type->unqualified;
}

void Analyzer::note_nothrow_target(const Entity* entity, const Type* type, const syntax::Declarator* declarator)
{
	// 15.4p2, 15.4p12: a pointer, reference or pointer to member to a
	// function type (or a parameter of function type) whose declarator's
	// exception-specification is `throw()`, `noexcept` or `noexcept(true)`:
	// a call through it does not throw (5.3.7p3). Only the declarators of
	// such types are walked.
	if (type->is_reference() || type->kind == TypeKind::Pointer || type->kind == TypeKind::MemberPointer)
		type = type->element;
	if (type->kind != TypeKind::Function)
		return;
	const syntax::DeclaratorSuffix* function = nullptr;
	uint32_t derivations = 0;
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		derivations += declarator->pointers.size;
		for (uint32_t i = declarator->suffixes.size; i-- > 0;)
		{
			const syntax::DeclaratorSuffix& suffix = declarator->suffixes[i];
			function = suffix.kind == syntax::DeclaratorSuffixKind::Function ? &suffix : nullptr;
			derivations = 0;
		}
	}
	if (function != nullptr && derivations <= 1 && NonThrowingSpecification(function))
		nothrow_targets_.set(entity, 1);
}

void Analyzer::check_nothrow_initializer(const syntax::Initializer& initializer)
{
	// 15.4p5: such an entity's initializer allows no exception it does not:
	// a function (or its address) that is not non-throwing, or another
	// pointer, reference or pointer to member to a function without such a
	// specification, is rejected.
	const syntax::Expr* source = initializer.expression;
	if (initializer.kind == syntax::InitializerKind::Parenthesized && initializer.arguments.size == 1)
		source = initializer.arguments[0];
	if (source != nullptr && source->kind == syntax::ExprKind::BracedInit &&
		static_cast<const syntax::BracedInitList*>(source)->elements.size == 1)
		source = static_cast<const syntax::BracedInitList*>(source)->elements[0];
	if (source == nullptr || source->kind == syntax::ExprKind::BracedInit)
		return;
	source = WithoutParentheses(source);
	if (source->kind == syntax::ExprKind::Unary &&
		static_cast<const syntax::UnaryExpr*>(source)->op == TokenKind::OP_AMP)
		source = WithoutParentheses(static_cast<const syntax::UnaryExpr*>(source)->operand);
	const Entity* entity = source->facts != nullptr ? source->facts->entity : nullptr;
	if (entity == nullptr)
		return;
	bool allows = false;
	if (entity->kind == EntityKind::Function)
		allows = !function_nothrow(static_cast<const FunctionEntity*>(entity));
	else if (entity->kind == EntityKind::Variable || entity->kind == EntityKind::Parameter)
	{
		const Type* type = entity->type;
		if (type->is_reference() || type->kind == TypeKind::Pointer || type->kind == TypeKind::MemberPointer)
			type = type->element;
		allows = type->kind == TypeKind::Function && nothrow_targets_.find(entity) == PointerMap::kMissing;
	}
	if (allows)
		error(initializer.location,
			"a function pointer or reference that allows no exceptions initialized with one that does");
}

bool Analyzer::nothrow_target(const syntax::Expr& callee)
{
	// A call's callee designating a function through such an entity
	// (`fp()`, `(*fp)()`, `s.fp()`, `(obj.*pm)()`).
	const syntax::Expr* e = WithoutParentheses(&callee);
	if (e->kind == syntax::ExprKind::Unary && static_cast<const syntax::UnaryExpr*>(e)->op == TokenKind::OP_STAR)
		e = WithoutParentheses(static_cast<const syntax::UnaryExpr*>(e)->operand);
	if (e->kind == syntax::ExprKind::Binary &&
		(static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_DOTSTAR ||
			static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_ARROWSTAR))
		e = WithoutParentheses(static_cast<const syntax::BinaryExpr*>(e)->right);
	return e->facts != nullptr && e->facts->entity != nullptr &&
		nothrow_targets_.find(e->facts->entity) != PointerMap::kMissing;
}

void Analyzer::pending_noexcept(FunctionEntity* function, const syntax::DeclaratorSuffix* suffix,
	const syntax::ParameterClause* parameters, Scope* scope)
{
	// A declaration's noexcept(constant-expression) other than `true`,
	// evaluated when first needed (template definitions wait for their
	// instances), its declarator's parameters in scope (3.3.4p1).
	const syntax::Expr* operand = NoexceptOperand(suffix);
	if (operand == nullptr || (template_depth_ != 0 && !function->templated))
		return;
	// A function template's is its specializations', substituted (see
	// function_nothrow).
	if (parameters != nullptr && !function->templated)
		scope = prototype_scope(*parameters, function->type->params, function->type->param_count, scope);
	noexcept_index_.set(function, static_cast<uint32_t>(noexcepts_.size()));
	// A class template instance's member waits for its need (14.7.1p1 as
	// DR 1330 extends it); others are evaluated by the end of the unit.
	const bool unit = !function->templated && class_instantiations_.empty();
	noexcepts_.push_back(PendingNoexcept{ operand, scope, parameters, function, unit, definition_point() });
	function->noexcept_pending = true;
	function->nothrow = false;
}

void Analyzer::check_pending_exceptions()
{
	// At the end of the unit, every class complete: 15.4p5 for overriders
	// whose noexcept-specification waited for its class (9.2p2), then
	// 15.4p1 for each noexcept-specification not yet needed, whose
	// expression is a constant expression whether or not the function is
	// used (a class template instance member's is instantiated only when
	// needed, 14.7.1p1 as DR 1330 extends it).
	for (const PendingOverride& pending : pending_overrides_)
	{
		if (!function_nothrow(pending.function))
			error(pending.location, "an overrider allows exceptions its overridden function does not");
	}
	pending_overrides_.clear();
	check_exception_overrides(0, true);
	for (size_t i = 0; i < noexcepts_.size(); ++i)
	{
		if (noexcepts_[i].unit && noexcepts_[i].function->noexcept_pending)
			function_nothrow(noexcepts_[i].function);
		else if (!noexcepts_[i].unit && noexcepts_[i].function->templated)
			check_fixed_template_noexcept(i);
	}
}

void Analyzer::check_fixed_template_noexcept(size_t index)
{
	// 14.6p8, p10: a function template's noexcept-specification whose
	// expression names nothing dependent is the same in every
	// specialization: it is checked where it appears (a constant
	// expression, 15.4p1), used or not. A member template of a templated
	// class waits for the class's instances.
	const PendingNoexcept pending = noexcepts_[index];
	FunctionEntity* function = pending.function;
	Scope* scope = pending.scope;
	if (TemplatedClassScope(scope))
		return;
	// One naming its own template parameters, or function parameters of
	// dependent types, depends on the template: no scope or key is made
	// for it (most library specifications are of this form).
	OwnNames own;
	own.count = 0;
	for (const Scope* outer = scope; outer != nullptr; outer = outer->parent)
	{
		if (outer->kind != ScopeKind::TemplateParameters)
			continue;
		for (const Binding* binding = outer->bindings; binding != nullptr && own.count < OwnNames::kMaximum;
			binding = binding->next)
			own.names[own.count++] = binding->name;
	}
	if (pending.parameters != nullptr && pending.parameters->parameters.size == function->type->param_count)
	{
		for (uint32_t i = 0; i < function->type->param_count && own.count < OwnNames::kMaximum; ++i)
		{
			const syntax::Declarator* declarator = pending.parameters->parameters[i]->declarator;
			while (declarator != nullptr && declarator->name == nullptr)
				declarator = declarator->nested;
			if (declarator != nullptr && !declarator->name->qualified() &&
				declarator->name->last().kind == syntax::NameComponentKind::Identifier &&
				dependent_type(function->type->params[i]))
				own.names[own.count++] = declarator->name->last().identifier;
		}
	}
	if (ExprNamesOwn(pending.operand, own, 0))
		return;
	if (pending.parameters != nullptr)
	{
		if (pending.parameters->parameters.size != function->type->param_count)
			return;
		scope = prototype_scope(*pending.parameters, function->type->params, function->type->param_count, scope);
	}
	// Keyed as in its template's definition (a pack expansion in it names
	// the template's packs), evaluated as a non-template's.
	const uint32_t limit = names_.limit();
	const uint32_t depth = template_depth_;
	names_.set_limit(pending.ordinal != 0 ? pending.ordinal : NameLookup::kNoLimit);
	template_depth_ = depth != 0 ? depth : 1;
	const bool fixed = fixed_expression(*pending.operand, scope) && !failed();
	template_depth_ = depth;
	if (fixed)
		noexcept_value(function, pending.operand, scope);
	names_.set_limit(limit);
}

void Analyzer::check_redeclared_exceptions(
	const FunctionEntity* function, const DeclaratorInfo& info, Scope* scope, uint32_t location)
{
	// 15.4p3-p4: the values of the declarations' exception-specifications
	// agree (an explicit specialization's with its template's for its
	// arguments): a noexcept(constant-expression) other than `true` is
	// evaluated (15.4p1; DR 1330: the comparison needs it) where its names
	// are looked up (3.3.4p1), unless it may depend on template parameters
	// or need its class complete (9.2p2). A destructor's declaration
	// without one has the implicit one (12.4p3), known where its class is
	// not a template; a deallocation function's is noexcept(true)
	// (15.4p15). Under -fno-exceptions an allocation or deallocation
	// function (a replaceable global one, its nothrow forms, a class's) may
	// be redeclared with another specification (GCC and Clang accept
	// `void* operator new(std::size_t) noexcept` there): it is not compared,
	// the first declaration's staying in force. (The hosts compare no
	// redeclaration under -fno-exceptions; other functions' are compared
	// as with exceptions, a known difference from them.)
	const syntax::Expr* operand = NoexceptOperand(info.function_suffix);
	if (!exceptions_ && function->is_operator &&
		(function->operator_token == TokenKind::KW_NEW || function->operator_token == TokenKind::KW_DELETE))
		return;
	if (operand != nullptr && function->templated && template_depth_ != 0)
	{
		// A function template's: two noexcept(constant-expression)s are
		// compatible when their expressions are equivalent (15.4p3,
		// 14.5.6.1p5), decided where their structure is known (see
		// decltype_words), with the declarators' parameters in scope.
		const uint32_t index = noexcept_index_.find(function);
		if (index == PointerMap::kMissing)
			return;
		const PendingNoexcept& first = noexcepts_[index];
		const Type* type = function->type;
		Scope* earlier = first.parameters != nullptr
			? prototype_scope(*first.parameters, type->params, type->param_count, first.scope)
			: first.scope;
		if (info.parameters != nullptr)
			scope = prototype_scope(*info.parameters, type->params, type->param_count, scope);
		std::vector<uint64_t>& words = decltype_words_;
		const size_t base = words.size();
		bool known = decltype_words(first.operand, earlier);
		const size_t middle = words.size();
		known = known && decltype_words(operand, scope);
		const bool equivalent = words.size() - middle == middle - base &&
			std::equal(words.begin() + static_cast<std::ptrdiff_t>(base),
				words.begin() + static_cast<std::ptrdiff_t>(middle),
				words.begin() + static_cast<std::ptrdiff_t>(middle));
		words.resize(base);
		if (known && !equivalent)
			error(location, "redeclaration with an incompatible exception-specification");
		return;
	}
	if ((operand != nullptr || function->noexcept_pending) && (template_depth_ != 0 || class_depth_ != 0))
		return;
	const bool implicit = function->destructor && !HasExceptionSpecification(info.function_suffix);
	if (implicit && template_depth_ != 0)
		return;
	// 15.4p15: a deallocation function's declaration without one is
	// noexcept(true), as its first declaration's is (see new_function).
	const bool deallocation = function->is_operator && function->operator_token == TokenKind::KW_DELETE &&
		!HasExceptionSpecification(info.function_suffix);
	bool redeclared = implicit ? implicit_destructor_nothrow(static_cast<const ClassEntity*>(function->owner->entity))
							   : deallocation || NonThrowingSpecification(info.function_suffix);
	if (operand != nullptr)
	{
		if (info.parameters != nullptr)
			scope = prototype_scope(*info.parameters, function->type->params, function->type->param_count, scope);
		redeclared = noexcept_value(function, operand, scope);
	}
	// Clang's rule for a C library's non-throwing declarations (C library
	// implementations may add them): a redeclaration without an
	// exception-specification of a C-linkage function a system header
	// declared non-throwing first takes that specification (clang's
	// mm_malloc.h redeclares glibc's posix_memalign so).
	if (!redeclared && function->system_nothrow && !HasExceptionSpecification(info.function_suffix))
		return;
	if (function_nothrow(function) != redeclared)
		error(location, "redeclaration with an incompatible exception-specification");
	// Otherwise both allow exceptions: dynamic-exception-specifications
	// are compatible when they have the same set of adjusted types (one
	// allowing all exceptions, none or noexcept(false), with none);
	// checked outside templates for a function that is no template's
	// specialization (whose first declaration recorded its own).
	if (redeclared || implicit || template_depth_ != 0 || function->templated || function->primary != nullptr)
		return;
	const SemanticGraph::ThrowSpecification* first = graph_.throw_specification(function);
	const syntax::FunctionQualifier* written = nullptr;
	for (uint32_t i = 0; info.function_suffix != nullptr && i < info.function_suffix->qualifiers.size; ++i)
	{
		if (info.function_suffix->qualifiers[i].kind == syntax::FunctionQualifierKind::Throw)
			written = &info.function_suffix->qualifiers[i];
	}
	if (first == nullptr && written == nullptr)
		return;
	TypeList types;
	for (uint32_t i = 0; written != nullptr && i < written->throw_types.size; ++i)
	{
		const Type* type = type_id(*written->throw_types[i], scope);
		if (type == nullptr)
			return;
		check_complete_throw_type(AdjustedThrowType(types_, type), written->throw_types[i]->location);
		types.push_back(AdjustedThrowType(types_, type)->canonical);
	}
	// operator new's C++03 declaration, `throw(std::bad_alloc)`, beside
	// C++11's without one (18.6.1.1): the hosts take it as compatible.
	const ClassEntity* thrown = types.size() == 1 ? ClassOf(types[0]) : nullptr;
	if (first == nullptr &&
		(function->allocation == AllocationRole::New || function->allocation == AllocationRole::NewArray) &&
		thrown != nullptr && thrown->name == bad_alloc_key_ && in_std(thrown))
		return;
	if ((first != nullptr) != (written != nullptr))
		error(location, "redeclaration with an incompatible exception-specification");
	// The two sets of canonical types, each sorted and unique (by
	// identity: only their equality is observed).
	TypeList earlier;
	for (uint32_t i = 0; i < first->count; ++i)
		earlier.push_back(first->types[i]->canonical);
	const auto as_set = [](TypeList& list) {
		std::sort(list.begin(), list.end(), std::less<const Type*>());
		return static_cast<size_t>(std::unique(list.begin(), list.end()) - list.begin());
	};
	const size_t count = as_set(types);
	if (count != as_set(earlier) || !std::equal(types.begin(), types.begin() + count, earlier.begin()))
		error(location, "redeclaration with an incompatible exception-specification");
}

bool Analyzer::function_nothrow(const FunctionEntity* entity)
{
	// 15.4p12: the noexcept-specification's value, the expression
	// contextually converted to bool (15.4p1) and evaluated once.
	FunctionEntity* function = const_cast<FunctionEntity*>(entity);
	if (!function->noexcept_pending || function->templated)
		return function->nothrow;
	function->noexcept_pending = false;
	// An inheriting constructor's, from its implicit definition's plan (see
	// planned_inheriting_constructor).
	if (function->inherited != nullptr && function->constructor)
	{
		function->nothrow = plan_nothrow(function);
		return function->nothrow;
	}
	const FunctionEntity* primary = function->primary;
	uint32_t index = noexcept_index_.find(function);
	if (index == PointerMap::kMissing && primary != nullptr)
		index = noexcept_index_.find(primary);
	if (index == PointerMap::kMissing)
		return function->nothrow;
	const syntax::Expr* operand = noexcepts_[index].operand;
	Scope* scope = noexcepts_[index].scope;
	if (primary != nullptr && primary->template_parameters != nullptr && HeadOf(primary) != nullptr)
	{
		// A function template specialization's: the template's expression
		// copied in the frame of its arguments (14.8.2p7, DR 1330), over the
		// template parameters of the declaration it belongs to (a later
		// definition's may be named otherwise).
		Scope* head = scope != nullptr && scope->kind == ScopeKind::TemplateParameters ? scope
																					   : primary->template_parameters;
		scope = instantiation_frame(head, ArgumentsOf(function), HeadOf(primary)->count, head->parent);
		FrameExpander expander(*this, scope);
		syntax::SyntaxCloner cloner(arena_, true, expander.get());
		// Its parameters with their substituted types, a function parameter
		// pack's elements each its own (copied first, so the operand's
		// expansions of the pack expand, as a definition's body's do).
		const syntax::ParameterClause* parameters = noexcepts_[index].parameters;
		const syntax::ParameterClause* copy = parameters != nullptr && expander.get() != nullptr
			? cloner.parameter_clause(parameters)
			: parameters;
		operand = cloner.expr(operand);
		if (copy != nullptr && copy->parameters.size == function->type->param_count)
		{
			Scope* packs = function_pack_scope(
				scope, *parameters, cloner.parameter_packs().data(), cloner.parameter_packs().size(), function->type);
			scope = prototype_scope(*copy, function->type->params, function->type->param_count, packs);
		}
	}
	// Evaluated when first needed, its names looked up where it is
	// declared (a template's in its definition's context, 14.6p10,
	// 14.6.4.2, as the instance member's class template's).
	// A non-template's (one the end of the unit evaluates) sees no later
	// declaration, not even as an instantiation would (see
	// global_allocation_functions).
	const uint32_t limit = names_.limit();
	const bool lexical = lexical_limit_;
	names_.set_limit(noexcepts_[index].ordinal != 0 ? noexcepts_[index].ordinal : NameLookup::kNoLimit);
	lexical_limit_ = noexcepts_[index].unit;
	function->nothrow = noexcept_value(function, operand, scope);
	names_.set_limit(limit);
	lexical_limit_ = lexical;
	return function->nothrow;
}

bool Analyzer::noexcept_value(const FunctionEntity* function, const syntax::Expr* operand, Scope* scope)
{
	// A noexcept-specification's constant expression of `function`'s
	// declaration, typed in `scope` and contextually converted to bool
	// (15.4p1). Instantiated on need, it is no part of a substitution under
	// way: an invalid expression in it is an error (14.8.2p7's note).
	BodyContext* saved_body = body_;
	FunctionEntity* saved_demanding = demanding_;
	const uint32_t saved_epoch = current_epoch_;
	const uint32_t saved_unevaluated = unevaluated_;
	const uint32_t saved_deferring = deferring_;
	const uint32_t saved_soft = soft_substitution_;
	Scope* saved_expression = expression_scope_;
	BodyContext context = BodyContext();
	// A non-static member function's `this` (5.1.1p3), qualified by its
	// cv-qualifiers.
	if (function->member && !function->is_static && !function->constructor && function->owner != nullptr &&
		function->owner->kind == ScopeKind::Class)
		context.this_type = types_.pointer(types_.qualified(function->owner->entity->type, function->type->cv));
	else if (function->constructor && function->owner != nullptr && function->owner->kind == ScopeKind::Class)
		context.this_type = types_.pointer(function->owner->entity->type);
	body_ = &context;
	demanding_ = nullptr;
	current_epoch_ = root_epoch_;
	unevaluated_ = 0;
	deferring_ = 0;
	soft_substitution_ = 0;
	typed_value(*operand, scope);
	convert_to_bool(*operand);
	Constant value;
	converted_constant(*operand, value, true);
	body_ = saved_body;
	demanding_ = saved_demanding;
	current_epoch_ = saved_epoch;
	unevaluated_ = saved_unevaluated;
	deferring_ = saved_deferring;
	soft_substitution_ = saved_soft;
	expression_scope_ = saved_expression;
	return value.bits != 0;
}

ExprFacts* Analyzer::noexcept_expression(const syntax::TypeTraitExpr& expr, Scope* scope)
{
	// 5.3.7: a bool constant, false when the unevaluated operand could
	// throw.
	++unevaluated_;
	const ExprFacts* operand = typed(*expr.operand, scope);
	--unevaluated_;
	if (operand == nullptr)
		return nullptr;
	ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Bool), ValueCategory::PRValue);
	facts->flags |= kFolded;
	facts->value = may_throw(*expr.operand, 0) ? 0u : 1u;
	return facts;
}

bool Analyzer::function_may_throw(const FunctionEntity* function)
{
	// A trivial special member runs no code, but a declared
	// exception-specification still gives its calls' (5.3.7p3, 15.4p12:
	// `~T() noexcept(false) = default;`).
	return function != nullptr && function->builtin == BuiltinFunction::None &&
		(!function->trivial || function->exception_spec) && !function_nothrow(function);
}

bool Analyzer::conversion_may_throw(const Conversion* conversion, uint32_t depth)
{
	// A user-defined conversion's function, its operand's conversion and a
	// conversion function's second step (a copy by a constructor).
	for (; conversion != nullptr && depth < 64; ++depth)
	{
		if (function_may_throw(conversion->constructor))
			return true;
		if (conversion->after != nullptr && conversion_may_throw(conversion->after, depth + 1))
			return true;
		conversion = conversion->argument;
	}
	return false;
}

bool Analyzer::temporary_may_throw(const ExprFacts* facts)
{
	// A class temporary is destroyed at the end of the full-expression
	// (12.2p3): its destructor counts.
	if (facts->category != ValueCategory::PRValue || facts->type == nullptr || facts->type->is_reference())
		return false;
	const ClassEntity* cls = ClassOf(facts->type);
	return cls != nullptr && cls->destructor != nullptr && function_may_throw(cls->destructor);
}

bool Analyzer::may_throw(const syntax::Expr& expr, uint32_t depth)
{
	// 5.3.7p3: a potentially-evaluated call of a function without a
	// non-throwing exception-specification (a constructor, conversion,
	// operator or allocation function's, a temporary's destructor), or a
	// throw-expression; unevaluated operands (sizeof, decltype, noexcept)
	// do not count.
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || depth > 256)
		return false;
	check_stack(e->location);
	if (conversion_may_throw(facts->conversion, 0))
		return true;
	if ((facts->flags & kUserConverted) != 0)
		return function_may_throw(static_cast<const FunctionEntity*>(facts->entity)) || temporary_may_throw(facts) ||
			(facts->original != nullptr && facts->original->category == ValueCategory::PRValue &&
				temporary_may_throw(facts->original));
	if ((facts->flags & (kFolded | kStaticConstant)) != 0 && e->kind != syntax::ExprKind::Member)
		return false;
	if ((facts->flags & (kOperatorCall | kUserLiteral)) != 0 && e->kind != syntax::ExprKind::Call &&
		(function_may_throw(static_cast<const FunctionEntity*>(facts->entity)) || temporary_may_throw(facts)))
		return true;
	switch (e->kind)
	{
	case syntax::ExprKind::Throw:
	// A statement expression's statements are not walked here.
	case syntax::ExprKind::Statement:
		return true;
	case syntax::ExprKind::BuiltinTrait:
	{
		// A builtin evaluating its operand (a complex part, a vector
		// conversion, a bit cast) throws as it does.
		const syntax::BuiltinTraitExpr& trait = static_cast<const syntax::BuiltinTraitExpr&>(*e);
		return (trait.trait == GnuTrait::RealPart || trait.trait == GnuTrait::ImagPart ||
				   trait.trait == GnuTrait::ConvertVector || trait.trait == GnuTrait::BitCast) &&
			trait.operand != nullptr && may_throw(*trait.operand, depth + 1);
	}
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
	case syntax::ExprKind::TypeTrait:
	case syntax::ExprKind::Lambda:
		return false;
	case syntax::ExprKind::Call:
	{
		const syntax::CallExpr& call = static_cast<const syntax::CallExpr&>(*e);
		if ((facts->flags & (kIntrinsic | kPseudoDestructor)) != 0)
			return may_throw(*call.callee, depth + 1);
		const FunctionEntity* function = (facts->flags & kDirectCall) != 0
			? static_cast<const FunctionEntity*>(facts->entity)
			: nullptr;
		if ((facts->flags & kDirectCall) == 0 && call.callee->facts != nullptr && !nothrow_target(*call.callee))
			return true;
		if (function_may_throw(function) || temporary_may_throw(facts) || may_throw(*call.callee, depth + 1))
			return true;
		for (const syntax::Expr* argument : call.arguments)
		{
			if (may_throw(*argument, depth + 1))
				return true;
		}
		const Type* type = function != nullptr ? function->type->canonical : nullptr;
		for (uint32_t i = call.arguments.size; type != nullptr && i < type->param_count; ++i)
		{
			const syntax::Expr* argument = function->default_arguments != nullptr ? function->default_arguments[i]
																				  : nullptr;
			if (argument != nullptr && may_throw(*argument, depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::FunctionalCast:
	case syntax::ExprKind::BracedInit:
	case syntax::ExprKind::Cast:
	{
		// A class prvalue's constructor and destructor; its operands.
		const FunctionEntity* constructor = facts->entity != nullptr && facts->entity->kind == EntityKind::Function
			? static_cast<const FunctionEntity*>(facts->entity)
			: nullptr;
		if (ClassOf(facts->type) != nullptr && facts->category == ValueCategory::PRValue &&
			(function_may_throw(constructor) || temporary_may_throw(facts)))
			return true;
		if (e->kind == syntax::ExprKind::Cast)
			return may_throw(*static_cast<const syntax::CastExpr*>(e)->operand, depth + 1);
		const syntax::Slice<syntax::Expr*>* operands = nullptr;
		if (e->kind == syntax::ExprKind::BracedInit)
			operands = &static_cast<const syntax::BracedInitList*>(e)->elements;
		else
		{
			const syntax::FunctionalCastExpr* cast = static_cast<const syntax::FunctionalCastExpr*>(e);
			operands = cast->braced != nullptr ? &cast->braced->elements : &cast->arguments;
		}
		for (const syntax::Expr* operand : *operands)
		{
			if (may_throw(*operand, depth + 1))
				return true;
		}
		// The constructor's default arguments (their temporaries'
		// destructors among them, 12.2p3).
		const Type* type = constructor != nullptr ? constructor->type->canonical : nullptr;
		for (uint32_t i = operands->size; type != nullptr && i < type->param_count; ++i)
		{
			const syntax::Expr* argument = constructor->default_arguments != nullptr ? constructor->default_arguments[i]
																					 : nullptr;
			if (argument != nullptr && may_throw(*argument, depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::New:
	{
		const syntax::NewExpr& allocation = static_cast<const syntax::NewExpr&>(*e);
		if (function_may_throw(static_cast<const FunctionEntity*>(facts->entity)) ||
			function_may_throw(allocation.constructor))
			return true;
		for (const syntax::Expr* argument : allocation.placement)
		{
			if (may_throw(*argument, depth + 1))
				return true;
		}
		const syntax::Initializer* init = allocation.initializer;
		if (init != nullptr && init->expression != nullptr && may_throw(*init->expression, depth + 1))
			return true;
		for (uint32_t i = 0; init != nullptr && i < init->arguments.size; ++i)
		{
			if (may_throw(*init->arguments[i], depth + 1))
				return true;
		}
		return false;
	}
	case syntax::ExprKind::Delete:
	{
		const syntax::DeleteExpr& deletion = static_cast<const syntax::DeleteExpr&>(*e);
		const ExprFacts* operand = WithoutParentheses(deletion.operand)->facts;
		const Type* pointer = operand != nullptr && operand->type != nullptr ? operand->type->unqualified : nullptr;
		const ClassEntity* cls = pointer != nullptr && pointer->kind == TypeKind::Pointer ? ClassOf(pointer->element)
																						  : nullptr;
		return function_may_throw(static_cast<const FunctionEntity*>(facts->entity)) ||
			(cls != nullptr && cls->destructor != nullptr && function_may_throw(cls->destructor)) ||
			may_throw(*deletion.operand, depth + 1);
	}
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
		return may_throw(*static_cast<const syntax::BinaryExpr*>(e)->left, depth + 1) ||
			may_throw(*static_cast<const syntax::BinaryExpr*>(e)->right, depth + 1);
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		return may_throw(*static_cast<const syntax::UnaryExpr*>(e)->operand, depth + 1);
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr& choice = static_cast<const syntax::ConditionalExpr&>(*e);
		return may_throw(*choice.condition, depth + 1) || may_throw(*choice.then_expr, depth + 1) ||
			may_throw(*choice.else_expr, depth + 1);
	}
	case syntax::ExprKind::Subscript:
		return may_throw(*static_cast<const syntax::SubscriptExpr*>(e)->base, depth + 1) ||
			may_throw(*static_cast<const syntax::SubscriptExpr*>(e)->index, depth + 1);
	case syntax::ExprKind::Member:
		return may_throw(*static_cast<const syntax::MemberExpr*>(e)->base, depth + 1);
	default:
		return false;
	}
}

} // namespace sema
