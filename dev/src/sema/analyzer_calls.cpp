#include "sema/analyzer.h"

#include <cstring>

#include "lex/identifier_table.h"

// Function calls (5.2.2) and overload resolution (13.3) for the procedural
// slice: candidates are the functions a name's lookup found; each viable
// candidate's implicit conversion sequences live in one scratch vector, so
// a resolution allocates nothing per candidate and a failure is a result,
// not a string.
namespace sema {

bool DeferredBuiltinName(const syntax::Name& name)
{
	if (name.global || name.components.size != 1 || name.last().kind != syntax::NameComponentKind::Identifier ||
		name.last().has_arguments)
		return false;
	return std::strncmp(name.last().identifier->spelling, "__builtin_", 10) == 0;
}

namespace {

// A static member function named through an object (5.2.5p4): an lvalue
// designating the function, not a bound member.
bool IsStaticMemberFunction(const ExprFacts* facts)
{
	return (facts->flags & kOverloadSet) == 0 && facts->entity != nullptr &&
		facts->entity->kind == EntityKind::Function && static_cast<const FunctionEntity*>(facts->entity)->is_static;
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// 5.2.2p1, 10.3p15: a member function named by a qualified-id is called
// itself, not its final overrider.
bool QualifiedCallee(const syntax::Expr& callee)
{
	const syntax::Expr* e = WithoutParentheses(&callee);
	if (e->kind == syntax::ExprKind::Member)
		return static_cast<const syntax::MemberExpr*>(e)->member->qualified();
	return e->kind == syntax::ExprKind::Id && static_cast<const syntax::IdExpr*>(e)->name->qualified();
}

// The id-expression a callee designates a function through: the name
// itself, or under parentheses and unary `*` and `&` (4.3, 5.3.1), each
// level designating the same function (`&C::f` of a non-static member is
// a member pointer, which is not callable); `levels` counts the operators.
const syntax::IdExpr* DesignatingName(const syntax::Expr* callee, unsigned& levels)
{
	levels = 0;
	for (callee = WithoutParentheses(callee); callee->kind == syntax::ExprKind::Unary; ++levels)
	{
		const syntax::UnaryExpr* unary = static_cast<const syntax::UnaryExpr*>(callee);
		if (unary->op != TokenKind::OP_AMP && unary->op != TokenKind::OP_STAR)
			return nullptr;
		callee = WithoutParentheses(unary->operand);
	}
	return callee->kind == syntax::ExprKind::Id ? static_cast<const syntax::IdExpr*>(callee) : nullptr;
}

} // namespace

const Type* Analyzer::result_category(const Type* result, ValueCategory& category)
{
	// 5.2.2p10: an lvalue reference (or an rvalue reference to a function)
	// gives an lvalue, an rvalue reference to an object an xvalue, any other
	// type a prvalue.
	if (result->kind == TypeKind::LValueReference ||
		(result->kind == TypeKind::RValueReference && result->element->kind == TypeKind::Function))
	{
		category = ValueCategory::LValue;
		return result->element;
	}
	if (result->kind == TypeKind::RValueReference)
	{
		category = ValueCategory::XValue;
		return result->element;
	}
	category = ValueCategory::PRValue;
	return result->unqualified->kind == TypeKind::Class ? result : result->unqualified;
}

ExprFacts* Analyzer::template_callee(const syntax::IdExpr& named, const syntax::Expr* callee, Scope* scope,
	TypeList& explicit_arguments, bool& undeclared)
{
	// The function templates a template-id callee names, as an overload set
	// the call resolves; null when the name is no function's. An
	// unparenthesized identifier that lookup does not find names the
	// function templates argument-dependent lookup finds (`undeclared`,
	// its arguments typed without a template's head): ill-formed in C++11
	// (14.8.1p8's note), C++20's rule (P0846) as the course reference and
	// Clang accept it.
	const syntax::Name& name = *named.name;
	const LookupResult result = name.last().kind == syntax::NameComponentKind::Operator ||
			name.last().kind == syntax::NameComponentKind::LiteralOperator
		? operator_function_lookup(name, scope)
		: name.qualified() ? lookup_name(name, scope, LookupFilter::Ordinary)
						   : unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
	undeclared = false;
	if (!failed() && !result.found() && callee == &named && !name.qualified() &&
		name.last().kind == syntax::NameComponentKind::Identifier)
	{
		undeclared = template_arguments(name.last(), scope, explicit_arguments, nullptr);
		return nullptr;
	}
	if (failed() || !result.found() || result.binding->kind != BindingKind::Function)
		return nullptr;
	if (result.ambiguous)
		error(named.location, "ambiguous name");
	// The arguments' own lookups leave this name's prefix as it was (its
	// naming class, see call).
	const NamePrefix prefix = last_prefix_;
	if (!template_arguments(name.last(), scope, explicit_arguments, SoleTemplateHead(result.functions)))
		return nullptr;
	last_prefix_ = prefix;
	FunctionList* head = nullptr;
	FunctionList* tail = nullptr;
	for (const FunctionList* node = result.functions; node != nullptr; node = node->next)
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
		error(named.location, "template-id does not name a function template");
	ExprFacts* facts = new_facts(named, nullptr, ValueCategory::LValue);
	facts->entity = head->binding->entity;
	facts->functions = head;
	facts->flags |= kOverloadSet;
	for (const syntax::Expr* node = callee; node != &named; node = static_cast<const syntax::ParenExpr*>(node)->inner)
		node->facts = facts;
	return facts;
}

ExprFacts* Analyzer::template_id_call(
	const syntax::CallExpr& expr, ExprFacts* callee, const TypeList& explicit_arguments, const ClassEntity* naming)
{
	// The call of the function templates a template-id callee names (see
	// template_callee). 9.3.1p3: member function templates named in a
	// member function of their class or a derived class are called through
	// `(*this)`.
	explicit_arguments_ = &explicit_arguments;
	bool member = false;
	for (const FunctionList* node = callee->functions; node != nullptr && !member; node = node->next)
	{
		const FunctionEntity* candidate = static_cast<const FunctionEntity*>(node->binding->entity);
		member = candidate->member && !candidate->is_static;
	}
	const ClassEntity* current = member && body_ != nullptr && body_->this_type != nullptr
		? ClassOf(body_->this_type->element)
		: nullptr;
	const Scope* home = callee->entity->owner;
	const ClassEntity* owner = home != nullptr && home->kind == ScopeKind::Class
		? static_cast<const ClassEntity*>(home->entity)
		: nullptr;
	if (current != nullptr && owner != nullptr && (owner == current || conversions_.is_base_of(owner, current)))
	{
		ExprFacts view;
		std::memset(static_cast<void*>(&view), 0, sizeof view);
		view.type = body_->this_type->element;
		view.category = ValueCategory::LValue;
		callee->flags |= kBoundMember | kImplicitThis;
		return resolved_call(expr, callee->functions, &view, true, naming);
	}
	return resolved_call(expr, callee->functions, nullptr, false, naming);
}

bool Analyzer::member_template_arguments(
	const syntax::MemberExpr& access, const ExprFacts* callee, Scope* scope, TypeList& explicit_arguments)
{
	// A member function template-id's explicit arguments, typed once the
	// member is found: its one template's parameters give a value argument
	// its type (a reference parameter's object, a pointer parameter's
	// overload set, 14.3.2p1, p5). The arguments' own lookups leave the
	// member's naming as it was.
	const NamePrefix prefix = last_prefix_;
	const ClassEntity* naming = naming_class_;
	template_arguments(access.member->last(), scope, explicit_arguments,
		callee != nullptr ? SoleTemplateHead(callee->functions) : nullptr);
	last_prefix_ = prefix;
	naming_class_ = naming;
	return !failed();
}

ExprFacts* Analyzer::call(const syntax::CallExpr& expr, Scope* scope)
{
	unsigned levels = 0;
	const syntax::IdExpr* named = DesignatingName(expr.callee, levels);
	const bool bare = named != nullptr && WithoutParentheses(expr.callee) == named;
	// An identifier, or an operator-function-id naming the operator
	// function as a function (13.5p4: `operator+(a, b)`; a literal
	// operator's too, `operator "" _x(1ULL)`, 13.5.8).
	const syntax::NameComponentKind kind = named != nullptr ? named->name->last().kind
															: syntax::NameComponentKind::Decltype;
	const bool operator_name = kind == syntax::NameComponentKind::Operator ||
		kind == syntax::NameComponentKind::LiteralOperator;
	const bool identifier = named != nullptr && (kind == syntax::NameComponentKind::Identifier || operator_name) &&
		!named->name->last().has_arguments;
	// 3.4.2p1: an unqualified, unparenthesized callee name also finds
	// functions in the arguments' associated namespaces, unless ordinary
	// lookup found something other than namespace-scope functions (p3).
	const bool unqualified_name = expr.callee->kind == syntax::ExprKind::Id && identifier && !named->name->qualified();
	bool dependent = false;
	bool undeclared = false;
	ExprFacts* callee = nullptr;
	// A template-id naming function templates (14.8.1), an operator
	// function's too (14.2p1): its arguments are the leading template
	// arguments, the rest deduced from the call.
	TypeList explicit_arguments;
	const uint64_t callee_uses = names_.dependent_uses();
	const bool template_call = bare && named != nullptr &&
		(kind == syntax::NameComponentKind::Identifier || operator_name) && named->name->last().has_arguments &&
		(callee = template_callee(*named, expr.callee, scope, explicit_arguments, undeclared)) != nullptr;
	// A member function template named by a template-id through an object
	// (`o.f<A>(args)`, `p->template f<A>(args)`, `o.operator()<A>()`,
	// 14.2p1): its explicit arguments, once the member is found.
	const syntax::Expr* access = WithoutParentheses(expr.callee);
	const syntax::NameComponentKind member_kind = access->kind == syntax::ExprKind::Member
		? static_cast<const syntax::MemberExpr*>(access)->member->last().kind
		: syntax::NameComponentKind::Decltype;
	const bool member_template_call = access->kind == syntax::ExprKind::Member &&
		static_cast<const syntax::MemberExpr*>(access)->member->last().has_arguments &&
		(member_kind == syntax::NameComponentKind::Identifier || member_kind == syntax::NameComponentKind::Operator);
	if (failed())
		return nullptr;
	if (template_call)
	{
		// 3.4.2p3: no argument-dependent lookup once a class member is found.
		const FunctionEntity* first = static_cast<const FunctionEntity*>(callee->entity);
		dependent = !named->name->qualified() && !first->member;
		// 11.2p5: a qualified name's naming class is its qualifier's class.
		naming_class_ = !first->member ? nullptr
			: named->name->qualified() && last_prefix_.scope != nullptr && last_prefix_.scope->kind == ScopeKind::Class
			? static_cast<const ClassEntity*>(last_prefix_.scope->entity)
			: naming_class(static_cast<const ClassEntity*>(first->owner->entity), scope);
	}
	else if (bare && identifier)
	{
		// The callee name, parenthesized or not, is looked up once: a type
		// makes a functional cast, a function name a call that argument-
		// dependent lookup may extend.
		const Identifier* name = name_key(named->name->last());
		if (name->spelling[0] == ' ' && unqualified_name)
			error(expr.location, "a coroutine operation (C++20 coroutines are not supported)");
		LookupResult result = operator_name ? operator_function_lookup(*named->name, scope)
			: named->name->qualified()      ? lookup_name(*named->name, scope, LookupFilter::Ordinary)
											: unqualified(scope, name, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		// A builtin function of an intrinsic or query form (lex/gnu_builtins.h;
		// lookup declares one when first named, as `::__builtin_x` too).
		if (result.found() && result.binding->kind == BindingKind::Function &&
			static_cast<const FunctionEntity*>(result.binding->entity)->builtin == BuiltinFunction::Intrinsic)
			return builtin_operation(expr, static_cast<const FunctionEntity*>(result.binding->entity), scope);
		if (!result.found())
		{
			if (!unqualified_name)
			{
				substitution_error(named->location, "undeclared function name");
				return nullptr;
			}
			// A hosted wrapper's call of a reserved builtin this compiler
			// lacks (see define_function): neither it nor its operands are
			// typed (their names are checked where its full-expression
			// began).
			if (deferring_reserved_ != 0 && soft_substitution_ == 0 && DeferredBuiltinName(*named->name))
			{
				defer_reserved_call(named->location);
				return nullptr;
			}
			undeclared = true;
		}
		else
		{
			if (result.ambiguous)
				error(named->location, "ambiguous name");
			const Binding* binding = result.binding;
			if (binding->kind == BindingKind::Type || binding->kind == BindingKind::TypeAlias)
			{
				// `T(args)` naming a type the parser did not classify is a
				// functional cast (5.2.3); not a name the template definition
				// qualified dependently without typename (14.6p2, p6).
				if (named->name->dependent_qualifier)
					error(named->location, "a dependent qualified name names a type only with typename");
				const Type* target = binding_type(binding, named->location);
				if (target->unqualified->kind == TypeKind::Class)
				{
					syntax::Initializer view = syntax::Initializer();
					view.kind = syntax::InitializerKind::Parenthesized;
					view.arguments = expr.arguments;
					view.location = expr.location;
					return class_prvalue(expr, target, view, scope);
				}
				if (expr.arguments.size == 1)
					return explicit_conversion(expr, target, expr.arguments[0], TokenKind::OP_LPAREN, scope);
				error(expr.location, "functional cast with several arguments is not supported");
			}
			if (unqualified_name && binding->kind == BindingKind::Function)
			{
				const FunctionEntity* function = static_cast<const FunctionEntity*>(binding->entity);
				dependent = !function->member && function->owner->kind != ScopeKind::Block;
			}
			naming_class_ = nullptr;
			callee_name_ = binding->kind == BindingKind::Function;
			callee = binding_expression(*named, named->name, result, scope);
			callee_name_ = false;
			for (const syntax::Expr* node = expr.callee; node != named;
				node = static_cast<const syntax::ParenExpr*>(node)->inner)
				node->facts = callee;
		}
	}
	else if (!undeclared)
		callee = typed(*expr.callee, scope);
	if (failed())
		return nullptr;
	const ClassEntity* naming = naming_class_;
	if (member_template_call &&
		!member_template_arguments(*static_cast<const syntax::MemberExpr*>(access), callee, scope, explicit_arguments))
		return nullptr;
	// 14.6p10, 14.6.2p1: a call none of whose arguments is type-dependent
	// in its template is bound where the template defines it, its
	// argument-dependent lookup too (conservatively decided, see
	// NameLookup::dependent_uses); a dependent one's also sees the
	// instantiation's context (14.6.4.2).
	const uint64_t uses = names_.dependent_uses();
	for (const syntax::Expr* argument : expr.arguments)
	{
		if (argument->kind == syntax::ExprKind::BracedInit)
			braced_argument(static_cast<const syntax::BracedInitList&>(*argument), scope);
		else
			typed(*argument, scope);
		if (failed())
			return nullptr;
	}
	// An unqualified template-id callee with template arguments that
	// depend on template parameters is a dependent name too (C++20
	// [temp.dep.general]p2.3, which GCC and Clang apply in C++11 mode:
	// `any_cast<T>(&a)` finds an overload declared after the template).
	const bool bound = names_.dependent_uses() == (template_call ? callee_uses : uses);
	expression_scope_ = scope;
	adl_functions_.clear();
	// 13.3.1.1.2: a class object called is the object of its operator().
	if (callee != nullptr && callee->type != nullptr && (callee->flags & (kOverloadSet | kBoundMember)) == 0 &&
		callee->type->kind != TypeKind::Function && ClassOf(callee->type) != nullptr)
		return call_operator(expr, callee);
	if (dependent || undeclared)
		argument_dependent(expr.arguments, name_key(named->name->last()), bound);
	if (template_call)
		return template_id_call(expr, callee, explicit_arguments, naming);
	if (undeclared)
	{
		if (adl_functions_.empty())
		{
			substitution_error(named->location, "undeclared function name");
			return nullptr;
		}
		// An undeclared template-id's explicit arguments (see template_callee).
		explicit_arguments_ = named->name->last().has_arguments ? &explicit_arguments : nullptr;
		return resolved_call(expr, nullptr, nullptr, false, nullptr);
	}
	if ((callee->flags & kBoundMember) != 0 &&
		((callee->flags & kPseudoDestructor) != 0 ||
			(callee->entity != nullptr && static_cast<const FunctionEntity*>(callee->entity)->destructor)))
	{
		// 12.4p12, 5.2.4: a destructor call (for any cv-qualified object,
		// 12.4p2) or a pseudo-destructor call; no arguments, void.
		if (!expr.arguments.empty())
			error(expr.location, "a destructor call with arguments");
		ExprFacts* facts = new_facts(expr, types_.fundamental(FundamentalType::Void), ValueCategory::PRValue);
		facts->flags |= callee->flags & kPseudoDestructor;
		if ((callee->flags & kPseudoDestructor) == 0)
		{
			facts->entity = callee->entity;
			facts->flags |= kDirectCall;
			if (static_cast<const FunctionEntity*>(callee->entity)->is_virtual && !QualifiedCallee(*expr.callee))
				facts->flags |= kVirtualCall;
			odr_use(static_cast<FunctionEntity*>(const_cast<Entity*>(callee->entity)), true);
		}
		return facts;
	}
	if ((callee->flags & kBoundMember) != 0)
	{
		// A member function named through an object: the object is the
		// member access's operand (or `*this`).
		ExprFacts* object = nullptr;
		ExprFacts view;
		if ((callee->flags & kImplicitThis) != 0)
		{
			std::memset(static_cast<void*>(&view), 0, sizeof view);
			view.type = body_->this_type->element;
			view.category = ValueCategory::LValue;
			object = &view;
		}
		else if (access->kind == syntax::ExprKind::Member)
		{
			const syntax::MemberExpr* member = static_cast<const syntax::MemberExpr*>(access);
			view = *member->base->facts;
			if (member->op == TokenKind::OP_ARROW)
			{
				view.type = view.type->kind == TypeKind::Array ? view.type->element : view.type->unqualified->element;
				view.category = ValueCategory::LValue;
			}
			object = &view;
		}
		else if (access->kind == syntax::ExprKind::Binary)
			return member_pointer_call(expr, callee, static_cast<const syntax::BinaryExpr&>(*access));
		else
			error(expr.location, "a call of a bound member function without its object");
		if (member_template_call)
			explicit_arguments_ = &explicit_arguments;
		return resolved_call(expr, callee->functions, object, (callee->flags & kImplicitThis) != 0, naming);
	}
	// A callee designating a function by name calls it directly. An
	// overloaded name is resolved by the call when parenthesized (13.3.1.1.1)
	// or under one `&` or `*` (as GCC and Clang do); deeper, it needs a
	// target type (13.4), which the indirect call reports missing.
	const ExprFacts* designated = named != nullptr ? named->facts : nullptr;
	if (designated != nullptr && designated->entity != nullptr && designated->entity->kind == EntityKind::Function &&
		(designated->flags & kBoundMember) == 0 && (levels <= 1 || (designated->flags & kOverloadSet) == 0))
		return resolved_call(expr, designated->functions, nullptr, false, naming);
	return indirect_call(expr, callee);
}

void Analyzer::associate(Scope* scope)
{
	// Each associated namespace once per lookup (its mark).
	NamespaceEntity* space = static_cast<NamespaceEntity*>(scope->entity);
	if (space->association_mark == association_epoch_)
		return;
	space->association_mark = association_epoch_;
	adl_namespaces_.push_back(scope);
}

void Analyzer::associate_enclosing(const Entity* entity)
{
	// The innermost enclosing namespace of a class or enumeration.
	Scope* scope = entity->owner;
	while (scope != nullptr && scope->kind != ScopeKind::Namespace)
		scope = scope->parent;
	if (scope != nullptr)
		associate(scope);
}

void Analyzer::associated(const Type* type, uint32_t depth)
{
	// 3.4.2p2: the associated namespaces and classes of a type.
	if (depth > 64)
		return;
	type = type->unqualified;
	switch (type->kind)
	{
	case TypeKind::Pointer:
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
	case TypeKind::Array:
		return associated(type->element, depth + 1);
	case TypeKind::MemberPointer:
		associated(type->owner, depth + 1);
		return associated(type->element, depth + 1);
	case TypeKind::Function:
		for (uint32_t i = 0; i < type->param_count; ++i)
			associated(type->params[i], depth + 1);
		return associated(type->element, depth + 1);
	case TypeKind::Pack:
		// An argument pack's elements are template arguments too.
		for (uint32_t i = 0; i < type->param_count; ++i)
			associated(type->params[i], depth + 1);
		return;
	case TypeKind::Template:
	{
		// A template template-argument: the namespace of which its template
		// is a member, or the class of a member template.
		const Scope* home = type->entity->owner;
		if (home != nullptr && home->kind == ScopeKind::Class && home->entity != nullptr)
			adl_enclosing_.push_back(static_cast<ClassEntity*>(home->entity));
		else
			associate_enclosing(type->entity);
		return;
	}
	case TypeKind::Enum:
	case TypeKind::Class:
		break;
	default:
		return;
	}
	// The class or enumeration and the class it is a member of (marked
	// once all arguments are walked: its bases are not associated), and a
	// class's direct and indirect bases, each walked once per lookup (its
	// mark); their namespaces.
	Entity* entity = type->entity;
	const Scope* owner = entity->owner;
	if (owner != nullptr && owner->kind == ScopeKind::Class && owner->entity != nullptr)
		adl_enclosing_.push_back(static_cast<ClassEntity*>(owner->entity));
	if (entity->kind != EntityKind::Class)
	{
		associate_enclosing(entity);
		return;
	}
	// A class template specialization: its definition (for its bases), and
	// what its type template arguments are associated with, walked once per
	// lookup (its argument mark), so the arguments a DAG of specializations
	// shares (P<Q, Q> of P<Q', Q'> ...) do not multiply the walk.
	const ClassEntity* specialization = static_cast<const ClassEntity*>(entity);
	const uint32_t epoch = association_epoch_;
	require_complete(specialization);
	if (association_epoch_ != epoch)
		renew_association();
	ClassTemplateFacts* facts = specialization->templ;
	if (facts != nullptr && facts->role == TemplateRole::Specialization && facts->argument_mark != association_epoch_)
	{
		facts->argument_mark = association_epoch_;
		for (uint32_t i = 0; i < facts->argument_count; ++i)
			associated(facts->arguments[i], depth + 1);
	}
	std::vector<ClassEntity*>& pending = adl_pending_;
	pending.clear();
	pending.push_back(static_cast<ClassEntity*>(entity));
	while (!pending.empty())
	{
		ClassEntity* cls = pending.back();
		pending.pop_back();
		if (cls->association_mark == association_epoch_)
			continue;
		cls->association_mark = association_epoch_;
		adl_classes_.push_back(cls);
		associate_enclosing(cls);
		for (const BaseClass* edge = cls->bases; edge != nullptr; edge = edge->next)
			pending.push_back(static_cast<ClassEntity*>(edge->base));
	}
}

void Analyzer::renew_association()
{
	// A class instantiated for this lookup made lookups of its own (each
	// with a mark of its own, its scratch swapped): this lookup's
	// namespaces and classes take a new mark, so those the instantiation
	// marked are associated here too (a vector<basic_string<char, traits,
	// A>>'s iterator compared before the string type is instantiated).
	++association_epoch_;
	for (Scope* scope : adl_namespaces_)
		static_cast<NamespaceEntity*>(scope->entity)->association_mark = association_epoch_;
	for (ClassEntity* cls : adl_classes_)
		cls->association_mark = association_epoch_;
}

void Analyzer::argument_dependent(const syntax::Slice<syntax::Expr*>& arguments, const Identifier* name, bool bound)
{
	// The functions of that name in the associated namespaces (with their
	// inline members and, for an inline one, the enclosing namespace),
	// using-directives ignored, and the friends the associated classes
	// declared that no namespace-scope declaration made visible (3.4.2p4:
	// visible in their namespace, the class's innermost enclosing one,
	// which is associated too). The namespaces' sets hold only visible
	// functions (see NameLookup::declare); the friends are indexed by
	// class, so a call costs its associated entities' own functions.
	// `bound`: a call bound in a template's definition (14.6p10) sees the
	// sets and friends as the lookup limit does; a dependent one's also
	// sees the instantiation's context (14.6.4.2), the whole unit here. A
	// friend also visible is found twice; the candidate sets take each
	// function once.
	adl_namespaces_.clear();
	adl_enclosing_.clear();
	adl_classes_.clear();
	++association_epoch_;
	for (const syntax::Expr* argument : arguments)
	{
		const ExprFacts* facts = argument->facts;
		if (facts->type != nullptr && (facts->flags & kOverloadSet) == 0)
			associated(facts->type, 0);
	}
	for (ClassEntity* cls : adl_enclosing_)
	{
		if (cls->association_mark == association_epoch_)
			continue;
		cls->association_mark = association_epoch_;
		adl_classes_.push_back(cls);
	}
	for (size_t i = 0; i < adl_namespaces_.size(); ++i)
	{
		Scope* scope = adl_namespaces_[i];
		const NamespaceEntity* entity = static_cast<const NamespaceEntity*>(scope->entity);
		for (const NamespaceList* member = entity->inline_members; member != nullptr; member = member->next)
			associate(member->scope);
		if (entity->is_inline && scope->parent != nullptr)
			associate(scope->parent);
		const NameEntry* found = names_.find(scope, name);
		if (found == nullptr)
			continue;
		for (const FunctionList* node = bound ? names_.functions(*found) : found->functions; node != nullptr;
			node = node->next)
			adl_functions_.push_back(static_cast<FunctionEntity*>(node->binding->entity));
	}
	// A class template instance's friends are declared where it is
	// instantiated, a point before the definition that needs it (14.6.4.1p4):
	// all of them are in the definition's context.
	const uint32_t limit = bound ? names_.limit() : NameLookup::kNoLimit;
	for (const ClassEntity* cls : adl_classes_)
	{
		const FunctionList* node = cls->scope != nullptr ? names_.friends(cls->scope, name) : nullptr;
		const bool instance = cls->templ != nullptr &&
			(cls->templ->role == TemplateRole::Specialization || cls->templ->role == TemplateRole::MemberInstance);
		while (node != nullptr && !instance && node->binding->ordinal > limit)
			node = node->next;
		for (; node != nullptr; node = node->next)
			adl_functions_.push_back(static_cast<FunctionEntity*>(node->binding->entity));
	}
}

ExprFacts* Analyzer::indirect_call(const syntax::CallExpr& expr, ExprFacts* callee)
{
	// A call through a function lvalue, a reference to a function or a
	// pointer to a function (5.2.2p1): the arguments convert to the
	// parameters of its type.
	check_resolved(*expr.callee);
	const Type* function = nullptr;
	if (callee->type->kind == TypeKind::Function)
		function = callee->type;
	else
	{
		const Type* pointer = decayed(callee);
		if ((pointer->kind == TypeKind::Pointer || pointer->kind == TypeKind::BlockPointer) &&
			pointer->element->kind == TypeKind::Function)
		{
			function = pointer->element;
			convert_operand(*expr.callee, pointer);
		}
	}
	// 14.8.2p8: in a substitution, an invalid call is its failure.
	if (function == nullptr)
	{
		substitution_error(expr.location, "called object is not a function or pointer to function");
		return nullptr;
	}
	const uint32_t count = expr.arguments.size;
	if (count < function->param_count || (count > function->param_count && !function->variadic))
	{
		substitution_error(expr.location, "wrong number of arguments in an indirect call");
		return nullptr;
	}
	convert_arguments(expr.arguments, function, nullptr, 0);
	ValueCategory category;
	const Type* type = result_category(function->element, category);
	return new_facts(expr, type, category);
}

ExprFacts* Analyzer::member_pointer_call(
	const syntax::CallExpr& expr, const ExprFacts* callee, const syntax::BinaryExpr& access)
{
	// 5.5p6: `(E1.*E2)(args)` calls the member function E2 points to on
	// E1 (`*E1` for ->*); the object must suit the function's cv- and
	// ref-qualifiers (13.3.1p4-p5), the arguments convert to its
	// parameters.
	const Type* function = callee->type->canonical;
	ExprFacts* left = facts_of(*access.left);
	const Type* object = access.op == TokenKind::OP_ARROWSTAR ? decayed(left)->element : left->type;
	const bool lvalue = access.op == TokenKind::OP_ARROWSTAR || left->category == ValueCategory::LValue;
	const char* failure = nullptr;
	if ((object->cv & ~function->function_cv & (kConst | kVolatile)) != 0)
		failure = "a call through a pointer to member function drops the object's cv-qualifiers";
	else if ((function->ref == RefQualifier::LValue && !lvalue) || (function->ref == RefQualifier::RValue && lvalue))
		failure = "the object's value category does not suit the member function's ref-qualifier";
	const uint32_t count = expr.arguments.size;
	if (failure == nullptr && (count < function->param_count || (count > function->param_count && !function->variadic)))
		failure = "wrong number of arguments in a call through a pointer to member function";
	if (failure != nullptr)
	{
		substitution_error(expr.location, failure);
		return nullptr;
	}
	convert_arguments(expr.arguments, function, nullptr, 0);
	ValueCategory category;
	const Type* type = result_category(function->element, category);
	return new_facts(expr, type, category);
}

ExprFacts* Analyzer::call_operator(const syntax::CallExpr& expr, ExprFacts* callee)
{
	check_resolved(*expr.callee);
	const ClassEntity* cls = ClassOf(callee->type);
	// (A substitution's failure, 14.8.2p8: __is_invocable of an incomplete
	// class object answers false, as GCC's does.)
	if (!require_complete(cls) || cls->scope == nullptr)
		return substitution_error(expr.location, "call of an object of incomplete class type"), nullptr;
	// A captureless lambda-expression called directly is lowered as a call
	// of the function its conversion yields (the course's form).
	const ClosureFacts* closure = graph_.closure_facts(cls);
	if (WithoutParentheses(expr.callee)->kind == syntax::ExprKind::Lambda && closure != nullptr &&
		closure->lambda == nullptr)
		closure_conversion(cls, false);
	const LookupResult members = names_.qualified(
		cls->scope, operator_key(TokenKind::OP_LPAREN), LookupFilter::Ordinary);
	// 10.2p7: members of different bases are an invalid set (13.3.1.1.2p1).
	if (members.ambiguous)
	{
		substitution_error(expr.location, "ambiguous member name");
		return nullptr;
	}
	const bool has_members = members.found() && members.binding->kind == BindingKind::Function;
	ExprFacts view = *callee;
	if (has_members)
	{
		// The members' resolution first; failing that, the surrogates.
		std::vector<FunctionEntity*>& set = overload_set_;
		set.clear();
		for (const FunctionList* node = members.functions; node != nullptr; node = node->next)
			set.push_back(static_cast<FunctionEntity*>(node->binding->entity));
		Candidate chosen = Candidate();
		if (resolve(expr.arguments, set, &view, chosen) == Resolution::Selected || !surrogate_call(expr, cls))
		{
			ExprFacts* facts = resolved_call(expr, members.functions, &view, false, cls, true);
			if (facts != nullptr)
				facts->flags |= kOperatorCall;
			return facts;
		}
		return indirect_call(expr, facts_of(*expr.callee));
	}
	if (!surrogate_call(expr, cls))
	{
		substitution_error(expr.location, "called object is not a function or pointer to function");
		return nullptr;
	}
	return indirect_call(expr, facts_of(*expr.callee));
}

bool Analyzer::surrogate_call(const syntax::CallExpr& expr, const ClassEntity* cls)
{
	// 13.3.1.1.2: a non-explicit conversion function to a pointer (or
	// reference) to a function whose parameters take the arguments; the
	// callee becomes its result (then called indirectly). The best such by
	// the arguments' conversions; none or several: false.
	std::vector<FunctionEntity*> functions;
	conversion_functions(cls, functions);
	const FunctionEntity* best = nullptr;
	std::vector<Conversion> best_sequences, sequences;
	bool ambiguous = false;
	for (const FunctionEntity* function : functions)
	{
		const Type* result = function->type->canonical->element;
		if (result->is_reference())
			result = result->element;
		if (result->unqualified->kind == TypeKind::Pointer)
			result = result->unqualified->element;
		if (function->is_explicit || result->kind != TypeKind::Function)
			continue;
		if (expr.arguments.size != result->param_count &&
			!(result->variadic && expr.arguments.size > result->param_count))
			continue;
		sequences.assign(expr.arguments.size, Conversion());
		bool viable = true;
		for (uint32_t i = 0; viable && i < result->param_count; ++i)
			viable = implicit_sequence(*expr.arguments[i], result->params[i], sequences[i]);
		if (!viable)
			continue;
		int order = best == nullptr ? 1 : 0;
		for (uint32_t i = 0; best != nullptr && i < result->param_count; ++i)
		{
			const int step = conversions_.compare(sequences[i], best_sequences[i]);
			order = step != 0 && (order == 0 || order == step) ? step : order == -step && step != 0 ? 2 : order;
		}
		if (order == 1)
		{
			best = function;
			best_sequences = sequences;
			ambiguous = false;
		}
		else if (order != -1)
			ambiguous = true;
	}
	if (best == nullptr || ambiguous)
		return false;
	convert_class_operand(*expr.callee, best);
	return true;
}

ExprFacts* Analyzer::resolved_call(const syntax::CallExpr& expr, const FunctionList* functions, ExprFacts* object,
	bool implicit_this, const ClassEntity* naming, bool operator_object)
{
	// The candidate functions, each once (a function can reach the set
	// through several declarations), in the analysis's scratch.
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	const uint32_t mark = ++candidate_epoch_;
	for (const FunctionList* node = functions; node != nullptr; node = node->next)
	{
		FunctionEntity* function = static_cast<FunctionEntity*>(node->binding->entity);
		if (node->binding->invisible || function->candidate_mark == mark)
			continue;
		function->candidate_mark = mark;
		set.push_back(function);
	}
	for (FunctionEntity* function : adl_functions_)
	{
		if (function->candidate_mark == mark)
			continue;
		function->candidate_mark = mark;
		set.push_back(function);
	}
	adl_functions_.clear();
	if (set.empty())
	{
		// A name designating a function with no lookup's set: a reference
		// template parameter's (14.3.2p1), under `&` or `*` too (4.3,
		// 5.3.1p1: `*F` is the function again).
		unsigned levels = 0;
		const syntax::IdExpr* named = DesignatingName(expr.callee, levels);
		const ExprFacts* callee = named != nullptr && named->facts != nullptr ? named->facts : expr.callee->facts;
		if (callee->entity != nullptr && callee->entity->kind == EntityKind::Function)
			set.push_back(static_cast<FunctionEntity*>(const_cast<Entity*>(callee->entity)));
	}
	const Candidate chosen = best_viable(expr.arguments, expr.location, set, object, true);
	FunctionEntity* selected = chosen.function;
	if (selected == nullptr)
		return nullptr;
	check_not_deleted(selected, expr.location);
	if (naming != nullptr)
		check_function_access(selected, functions, naming, object != nullptr ? ClassOf(object->type) : nullptr,
			expression_scope_, expr.location);
	// 10.2, 5.2.5p5, 11.2p6: the selected non-static member is of an
	// unambiguous base subobject of the naming class, whatever member the
	// name's lookup found first (a using-declaration brings a base's
	// member beside the class's own, 7.3.3p16; an operator() or operator->
	// is a base's member).
	const syntax::Expr* access = WithoutParentheses(expr.callee);
	const ClassEntity* object_class = object != nullptr ? ClassOf(object->type) : nullptr;
	if (object_class != nullptr && naming != nullptr && selected->member && !selected->is_static &&
		!member_subobject(
			access->kind == syntax::ExprKind::Member ? *access : expr, selected, object_class, naming, expr.location))
		return nullptr;
	// 9.3.1p1, 5.1.1p13: a non-static member function needs an object.
	if (object == nullptr && selected->member && !selected->is_static)
	{
		// A call needs it in an unevaluated operand too. In an immediate
		// substitution context this invalid expression rejects the candidate.
		substitution_error(expr.location, "non-static member function called without an object");
		return nullptr;
	}
	if (!require_deduced(selected, expr.location))
		return nullptr;
	// 5.1.2p10: a lambda's implicit member call captures `this`.
	if (implicit_this && selected->member && !selected->is_static && body_ != nullptr && body_->lambda != nullptr)
		capture_this(expr.location);
	const Type* type = selected->type->canonical;
	// The selected candidate's conversion sequences are the arguments';
	// the implicit object argument's conversion to the function's class
	// (13.3.1p4) is the member callee's (a callee is never converted
	// otherwise).
	const bool object_slot = object != nullptr;
	if (object_slot && selected->member && !selected->is_static)
	{
		Conversion conversion = Conversion();
		object_sequence(object, selected, false, conversion);
		record_conversion(facts_of(*expr.callee), conversion);
	}
	convert_arguments(expr.arguments, type, &chosen, object_slot ? 1 : 0);
	for (uint32_t i = expr.arguments.size; selected->default_arguments != nullptr && i < type->param_count; ++i)
		instantiate_default_argument(selected, i);
	odr_use(selected, true);
	// The callee designates the selected function now (found by
	// argument-dependent lookup alone, it has no facts yet).
	if (expr.callee->facts == nullptr)
	{
		ExprFacts* found = new_facts(*expr.callee, type, ValueCategory::LValue);
		found->entity = selected;
	}
	// Each level of a designation through `*` and `&` is the function
	// lvalue, or a pointer to it under `&` (parentheses share facts).
	const ExprFacts* callee = expr.callee->facts;
	if (!operator_object && ((callee->flags & kOverloadSet) != 0 || callee->entity != selected))
	{
		for (const syntax::Expr* node = WithoutParentheses(expr.callee);; node = WithoutParentheses(node))
		{
			const bool unary = node->kind == syntax::ExprKind::Unary;
			ExprFacts* level = facts_of(*node);
			level->type = unary && static_cast<const syntax::UnaryExpr*>(node)->op == TokenKind::OP_AMP
				? types_.pointer(type)
				: type;
			level->entity = selected;
			level->flags &= static_cast<uint32_t>(~kOverloadSet);
			if (!unary)
				break;
			node = static_cast<const syntax::UnaryExpr*>(node)->operand;
		}
	}
	ValueCategory category;
	const Type* result = result_category(type->element, category);
	ExprFacts* facts = new_facts(expr, result, category);
	facts->entity = selected;
	facts->flags |= kDirectCall;
	// A class object's operator() is its member's call, never qualified.
	if (selected->is_virtual && (operator_object || !QualifiedCallee(*expr.callee)))
		facts->flags |= kVirtualCall;
	if (implicit_this)
		facts->flags |= kImplicitThis;
	// A qualified call through `(*this)` whose naming class is neither the
	// object's class nor the member's: the object converts to the naming
	// class first (the course's presentation of 11.2p5's path).
	const ClassEntity* current = implicit_this && object != nullptr ? ClassOf(object->type) : nullptr;
	const ClassEntity* home = selected->member ? static_cast<const ClassEntity*>(selected->owner->entity) : nullptr;
	if (current != nullptr && naming != nullptr && home != nullptr && naming != current && naming != home &&
		QualifiedCallee(*expr.callee))
	{
		naming_index_.set(&expr, static_cast<uint32_t>(namings_.size()));
		namings_.push_back(naming);
	}
	return facts;
}

bool Analyzer::implicit_sequence(const syntax::Expr& argument, const Type* parameter, Conversion& out)
{
	if (argument.kind == syntax::ExprKind::BracedInit)
		return list_sequence(static_cast<const syntax::BracedInitList&>(argument), parameter, out);
	const ExprFacts* facts = argument.facts;
	if ((facts->flags & kBoundMember) != 0 && !IsStaticMemberFunction(facts))
		return false;
	if ((facts->flags & kOverloadSet) != 0)
	{
		// 13.3.3.1p? via 13.4: the argument resolves to the function the
		// parameter's type selects, an exact match.
		if (select_overloaded(facts, parameter) == nullptr)
			return false;
		out = Conversion();
		out.target = parameter;
		out.result = parameter;
		out.rank = ConversionRank::Exact;
		out.form = parameter->is_reference() ? ConversionForm::Reference : ConversionForm::Standard;
		out.function_lvalue = parameter->is_reference();
		out.rvalue_reference = parameter->kind == TypeKind::RValueReference;
		out.first = parameter->is_reference() ? LvalueTransform::None : LvalueTransform::FunctionToPointer;
		return true;
	}
	if (IsVoid(facts->type))
		return false;
	if (conversions_.implicit(operand_of(facts), parameter, false, out))
		return true;
	if (standard_only_)
		return false;
	// 13.3.1.4p1, 13.3.1.6p1 (CWG 899): a reference to the class a single
	// argument direct-initializes is also bound by explicit conversion
	// functions.
	const bool direct = direct_class_ != nullptr && parameter->is_reference() &&
		parameter->element->unqualified == direct_class_->type;
	return user_conversion(facts, parameter, out, direct);
}

bool Analyzer::object_sequence(const ExprFacts* object, const FunctionEntity* function, bool ranking, Conversion& out)
{
	// 13.3.1p4-p5: the implicit object parameter is `cv X&` for a member
	// of X; the object binds to it even as an rvalue. The candidates of one
	// member lookup are all members of the naming class (a function a
	// using-declaration introduces counts as one, p4), so for ranking their
	// object conversions differ only in cv-qualification, which X = the
	// object's own class keeps; the call converts to the function's class.
	// 13.3.1p4-p5: `cv X&&` for a member declared with `&&`, else `cv X&`;
	// only a member without a ref-qualifier binds an rvalue object to a
	// non-const lvalue reference.
	const Type* owner = ranking ? object->type->unqualified : function->owner->entity->type;
	const RefQualifier ref = function->type->canonical->ref;
	const Type* parameter = types_.reference(
		types_.qualified(owner, function->type->function_cv), ref == RefQualifier::RValue);
	Operand operand = operand_of(object);
	if (ref == RefQualifier::None)
		operand.category = ValueCategory::LValue;
	if (!conversions_.implicit(operand, parameter, false, out))
		return false;
	out.implicit_object = ref == RefQualifier::None;
	if (ref == RefQualifier::None)
		out.rvalue_reference = false;
	return true;
}

int Analyzer::compare_candidates(const Candidate& a, const Candidate& b, uint32_t count, bool object)
{
	// 13.3.3p1: no conversion sequence worse and at least one better. The
	// implicit object parameter of a static member neither helps nor
	// hurts (13.3.1p4).
	bool better = false, worse = false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (i == 0 && object &&
			(a.function->is_static || b.function->is_static || !a.function->member || !b.function->member))
			continue;
		const int order = conversions_.compare(sequences_[a.first + i], sequences_[b.first + i]);
		better = better || order > 0;
		worse = worse || order < 0;
	}
	if (better && !worse)
		return 1;
	if (worse && !better)
		return -1;
	// 13.3.3p1: otherwise a non-template beats a template specialization,
	// and a specialization of a more specialized template another's
	// (14.5.6.2, over the parameters the call has arguments for; without
	// an object argument, an operator's member candidate is ordered with
	// its object parameter inserted, p3).
	if (!better && !worse && a.from_template != b.from_template)
		return a.from_template ? -1 : 1;
	if (!better && !worse && a.from_template && a.function->primary != nullptr && b.function->primary != nullptr &&
		a.function->primary != b.function->primary)
	{
		const uint32_t arguments = count - (object ? 1 : 0);
		const Ordering form = object ? Ordering::Call : Ordering::CallInserting;
		const bool forward = at_least_as_specialized(a.function->primary, b.function->primary, arguments, form);
		const bool backward = at_least_as_specialized(b.function->primary, a.function->primary, arguments, form);
		if (forward != backward)
			return forward ? 1 : -1;
		// CWG 1395 (as GCC and Clang apply it to C++11): of two templates
		// each at least as specialized, one whose trailing function
		// parameter pack takes no argument is less specialized than one
		// without a trailing pack or a parameter corresponding to it.
		if (forward && backward)
		{
			const int references = reference_ordering(a.function->primary, b.function->primary, arguments, form);
			if (references != 0)
				return references;
			if (trailing_pack_unused(b.function->primary, a.function->primary, arguments))
				return 1;
			if (trailing_pack_unused(a.function->primary, b.function->primary, arguments))
				return -1;
		}
	}
	return 0;
}

Candidate Analyzer::best_viable(const syntax::Slice<syntax::Expr*>& call_arguments, uint32_t location,
	std::vector<FunctionEntity*>& set, ExprFacts* object, bool substitution)
{
	// `substitution`: a failure may be a substitution's (see
	// soft_substitution_), a candidate of no function then.
	Candidate chosen = Candidate();
	switch (resolve(call_arguments, set, object, chosen))
	{
	case Resolution::NoViable:
		if (!substitution)
			error(location, "no viable function for the call");
		substitution_error(location, "no viable function for the call");
		return Candidate();
	case Resolution::Ambiguous:
		if (!substitution)
			error(location, "ambiguous call to overloaded function");
		substitution_error(location, "ambiguous call to overloaded function");
		return Candidate();
	case Resolution::Selected:
		break;
	}
	return chosen;
}

void Analyzer::note_added_default_arguments(const FunctionEntity* function, const syntax::ParameterClause& clause)
{
	// A later declaration's default arguments (8.3.6p4) exist from its
	// place in declaration order.
	const uint32_t count = function->type->canonical->param_count;
	if (clause.parameters.size != count)
		return;
	uint32_t first = added_defaults_.find(function);
	for (uint32_t i = 0; i < count; ++i)
	{
		if (clause.parameters[i]->default_argument == nullptr)
			continue;
		if (first == PointerMap::kMissing)
		{
			first = static_cast<uint32_t>(added_default_ordinals_.size());
			added_defaults_.set(function, first);
			added_default_ordinals_.resize(added_default_ordinals_.size() + count, 0);
		}
		added_default_ordinals_[first + i] = definition_point();
	}
}

bool Analyzer::default_argument_visible(const FunctionEntity* function, uint32_t index) const
{
	// 14.6p10, 14.6.4.2: a call in a template's instantiation uses the
	// default arguments of the declarations in its definition's context.
	if (added_default_ordinals_.empty() || names_.limit() == NameLookup::kNoLimit)
		return true;
	const uint32_t first = added_defaults_.find(function);
	return first == PointerMap::kMissing || added_default_ordinals_[first + index] <= names_.limit();
}

Resolution Analyzer::resolve(const syntax::Slice<syntax::Expr*>& call_arguments, std::vector<FunctionEntity*>& set,
	ExprFacts* object, Candidate& chosen)
{
	// 13.3.2-13.3.3: the viable candidates, then the one better than every
	// other; a failure is a result, not an error.
	const uint32_t arguments = call_arguments.size;
	const bool with_object = object != nullptr;
	const uint32_t slots = arguments + (with_object ? 1 : 0);
	// A template-id call's explicit arguments are this resolution's alone.
	const TypeList* explicit_arguments = explicit_arguments_;
	explicit_arguments_ = nullptr;
	const bool converting = converting_only_;
	converting_only_ = false;
	candidates_.clear();
	sequences_.clear();
	++graph_.resolutions;
	graph_.candidates += set.size();
	// A viable non-template candidate whose every conversion is the
	// identity (a reference binding directly, of the argument's own type and
	// value category) is better than every template specialization
	// (13.3.3p1), so the templates are not deduced: as GCC and Clang
	// resolve calls (P3606), the deductions a perfect match makes moot
	// instantiate nothing (a recursion through a trailing return type that
	// such a match ends, say). The templates follow the other candidates.
	bool templates = false;
	bool perfect = false;
	for (FunctionEntity* candidate : set)
		templates = templates || candidate->templated;
	for (unsigned pass = 0; pass < (templates ? 2u : 1u) && !perfect; ++pass)
		for (FunctionEntity* candidate : set)
		{
			FunctionEntity* function = candidate;
			if (templates && candidate->templated != (pass == 1))
				continue;
			if (IgnoredByOverloadResolution(candidate))
				continue;
			if (explicit_arguments != nullptr && !candidate->templated)
				continue;
			// 13.3.2p2: enough arguments (or default arguments for the rest),
			// and not more unless the function is variadic; a template's
			// specializations have its parameters, defaults and ellipsis, so
			// it is filtered before any deduction or substitution.
			// A template's trailing function parameter pack takes any number of
			// arguments (14.8.2.1p1); one that does not end the list takes as
			// many as its explicit template arguments give it, none otherwise
			// (14.8.1p3, 14.8.2.5p5): the specialization's parameters are
			// counted.
			const Type* declared = candidate->type->canonical;
			uint32_t parameters = declared->param_count;
			const bool pack = candidate->templated && parameters != 0 &&
				declared->params[parameters - 1]->kind == TypeKind::Expansion;
			if (pack)
				--parameters;
			bool inner_pack = false;
			for (uint32_t i = 0; candidate->templated && i < parameters; ++i)
				inner_pack = inner_pack || declared->params[i]->kind == TypeKind::Expansion;
			bool viable = arguments <= parameters || declared->variadic || pack || inner_pack;
			for (uint32_t i = arguments; viable && !inner_pack && i < parameters; ++i)
				viable = candidate->default_arguments != nullptr && candidate->default_arguments[i] != nullptr &&
					default_argument_visible(candidate, i);
			if (!viable)
				continue;
			if (candidate->templated)
			{
				// 13.3.1.4p1: copy-initialization's constructors are converting
				// ones, so not a specialization explicit(condition) makes
				// explicit (see constructor_candidates).
				function = deduce_for_call(candidate, call_arguments, explicit_arguments);
				if (function == nullptr || (converting && function->is_explicit))
					continue;
			}
			const Type* type = function->type->canonical;
			parameters = type->param_count;
			if (arguments > parameters && !type->variadic)
				continue;
			for (uint32_t i = arguments; viable && inner_pack && i < parameters; ++i)
				viable = function->default_arguments != nullptr && function->default_arguments[i] != nullptr;
			if (!viable)
				continue;
			const uint32_t first = static_cast<uint32_t>(sequences_.size());
			sequences_.resize(first + slots);
			if (with_object)
			{
				Conversion& slot = sequences_[first];
				slot = Conversion();
				if (function->member && !function->is_static)
					viable = object_sequence(object, function, true, slot);
			}
			for (uint32_t i = 0; viable && i < arguments; ++i)
			{
				Conversion& slot = sequences_[first + (with_object ? 1 : 0) + i];
				if (i < parameters)
					viable = implicit_sequence(*call_arguments[i], type->params[i], slot);
				else
				{
					slot = Conversion();
					slot.form = ConversionForm::Ellipsis;
					slot.rank = ConversionRank::Ellipsis;
					const ExprFacts* facts = call_arguments[i]->facts;
					viable = facts->type != nullptr && (facts->flags & (kOverloadSet | kBoundMember)) == 0 &&
						!IsVoid(facts->type);
				}
			}
			if (!viable)
			{
				sequences_.resize(first);
				continue;
			}
			candidates_.push_back(Candidate{ function, first, candidate->templated });
			if (templates && !candidate->templated)
			{
				bool all = !with_object || PerfectSequence(sequences_[first], object);
				for (uint32_t i = 0; all && i < arguments; ++i)
					all = PerfectSequence(sequences_[first + (with_object ? 1 : 0) + i], call_arguments[i]->facts);
				perfect = all;
			}
		}
	if (candidates_.empty())
		return Resolution::NoViable;
	size_t best = 0;
	for (size_t i = 1; i < candidates_.size(); ++i)
	{
		if (compare_candidates(candidates_[i], candidates_[best], slots, with_object) > 0)
			best = i;
	}
	for (size_t i = 0; i < candidates_.size(); ++i)
	{
		if (i != best && compare_candidates(candidates_[best], candidates_[i], slots, with_object) <= 0)
			return Resolution::Ambiguous;
	}
	chosen = candidates_[best];
	return Resolution::Selected;
}

bool Analyzer::PerfectSequence(const Conversion& conversion, const ExprFacts* argument)
{
	// The identity conversion (13.3.3.1.1; an lvalue's value read, a class
	// argument of the parameter's class, 13.3.3.1p6), or a reference
	// binding directly to an argument of the referenced type, cv-qualifiers
	// included, whose value category the reference's kind takes (no rule
	// of 13.3.3.2p3 prefers another binding).
	if (argument == nullptr || argument->type == nullptr || (argument->flags & kOverloadSet) != 0 ||
		conversion.rank != ConversionRank::Exact || conversion.second != ValueConversion::None ||
		conversion.qualification || conversion.constructor != nullptr || conversion.initializer_list)
		return false;
	if (conversion.form == ConversionForm::Standard)
		return conversion.first == LvalueTransform::None || conversion.first == LvalueTransform::LvalueToRvalue;
	if (conversion.form != ConversionForm::Reference || conversion.temporary || conversion.function_lvalue ||
		conversion.first != LvalueTransform::None || conversion.result == nullptr ||
		conversion.result->unqualified != argument->type->unqualified || conversion.result->cv != argument->type->cv)
		return false;
	return conversion.rvalue_reference == (argument->category != ValueCategory::LValue);
}

void Analyzer::convert_arguments(
	const syntax::Slice<syntax::Expr*>& arguments, const Type* function, const Candidate* chosen, uint32_t offset)
{
	// The chosen candidate's sequences, taken out of the scratch first: a
	// class parameter's copy-initialization resolves its constructor.
	const size_t first = argument_scratch_.size();
	for (uint32_t i = 0; chosen != nullptr && i < arguments.size && i < function->param_count; ++i)
		argument_scratch_.push_back(sequences_[chosen->first + offset + i]);
	for (uint32_t i = 0; i < arguments.size; ++i)
	{
		const syntax::Expr& argument = *arguments[i];
		if (i < function->param_count)
		{
			// A resolved call records its candidate's sequence; an
			// overloaded function argument is resolved to its parameter's
			// target first (13.4).
			const Type* parameter = function->params[i];
			// 5.2.2p4: a class parameter is destroyed in the caller's
			// context (12.4p11: its destructor not deleted, accessible);
			// 10.4p3: it is no abstract class (checked at the call, not
			// where its function type is formed: P0929R2, as both hosts).
			if (!parameter->is_reference() && parameter->unqualified->kind == TypeKind::Class)
			{
				check_destruction(parameter, expression_scope_, argument.location);
				check_abstract_object(parameter, argument.location);
			}
			if (argument.kind == syntax::ExprKind::BracedInit)
			{
				list_argument(static_cast<const syntax::BracedInitList&>(argument), parameter, expression_scope_,
					argument.location);
				continue;
			}
			if (chosen != nullptr && (argument.facts->flags & kOverloadSet) == 0)
			{
				Conversion conversion = argument_scratch_[first + i];
				conversion.initialization = true;
				check_conversion_access(argument.facts, conversion, argument.location);
				check_binding_temporary(argument.facts, conversion, argument.location);
				if (conversion.constructor != nullptr)
					apply_user_conversion(argument.facts, conversion, argument.location);
				odr_use_constants(argument, &conversion);
				record_conversion(facts_of(argument), conversion);
			}
			else
				convert(argument, parameter, false);
			// 8.5p16: a class parameter from an expression of its class (or
			// a derived one) is initialized by the constructor it selects.
			ClassEntity* cls = parameter->is_reference() ? nullptr : const_cast<ClassEntity*>(ClassOf(parameter));
			if (cls != nullptr && same_class_source(cls, argument))
				record_copy(cls, parameter, argument, argument.location);
			continue;
		}
		// 5.2.2p7: an argument matched by the ellipsis gets the lvalue
		// transformations, float to double, the integral promotions and
		// std::nullptr_t to void*.
		if (argument.kind == syntax::ExprKind::BracedInit)
			error(argument.location, "a braced-init-list argument matched by the ellipsis");
		check_resolved(argument);
		ExprFacts* facts = facts_of(argument);
		if (IsVoid(facts->type))
			error(argument.location, "argument of type void");
		const Type* type = facts->type->unqualified->kind == TypeKind::Class ? facts->type : decayed(facts);
		if (type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::Float)
			type = types_.fundamental(FundamentalType::Double);
		else if (type->kind == TypeKind::Fundamental && type->fundamental == FundamentalType::NullptrT)
			type = types_.pointer(types_.fundamental(FundamentalType::Void));
		else if (IsIntegralOrUnscopedEnum(type))
			type = promoted(type);
		Conversion conversion = Conversion();
		if (!conversions_.implicit(operand_of(facts), type, false, conversion))
		{
			conversion = Conversion();
			conversion.target = type;
			conversion.result = type;
		}
		conversion.form = ConversionForm::Ellipsis;
		conversion.rank = ConversionRank::Ellipsis;
		record_conversion(facts, conversion);
	}
	argument_scratch_.resize(first);
}

} // namespace sema
