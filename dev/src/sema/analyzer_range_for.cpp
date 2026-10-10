#include "sema/analyzer.h"

#include <new>

#include "sema/statement_facts.h"

// Range-based for statements (6.5.4). In semantics mode the statement is
// analyzed as the equivalent 6.5.4p1 defines it by (sema::RangeForFacts):
// its implicit variables are declared as written ones are (their lookup
// keys are no identifiers a program can spell), and its implicit
// expressions are syntax nodes typed once as written ones are, whose
// operands naming those variables carry their facts (kImplicit). Each
// initializer of the equivalent (range-init, a braced-init-list's
// elements, begin-expr and end-expr, `*__begin`) is typed once too: its
// facts are marked kImplicit, so the declaration's auto deduction and
// initialization read them as they are (recording its conversion).
namespace sema {

namespace {

const syntax::Name* DeclaratorId(const syntax::Declarator* declarator)
{
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name != nullptr)
			return declarator->name;
	}
	return nullptr;
}

const syntax::Expr* WithoutParentheses(const syntax::Expr* expr)
{
	while (expr->kind == syntax::ExprKind::Paren)
		expr = static_cast<const syntax::ParenExpr*>(expr)->inner;
	return expr;
}

// The identifiers of the equivalent: the member and function names looked
// up, the implicit variables' lookup keys, and their printed names.
enum RangeForIdentifier : unsigned
{
	kBeginName,
	kEndName,
	kRangeKey,
	kBeginKey,
	kEndKey,
	kIndexKey,
	kRangeLabel,
	kBeginLabel,
	kEndLabel,
	kIndexLabel
};

const char* const kRangeForSpellings[] = { "begin", "end", "<range>", "<begin>", "<end>", "<index>", "__range",
	"__begin", "__end", "__idx" };

template<typename T>
void located(T& node, uint32_t location)
{
	node.location = location;
}

void located(syntax::InitDeclarator&, uint32_t)
{}

// A lambda's call operator (its body's `this` is the enclosing function's).
bool ClosureBody(const SemanticGraph& graph, const FunctionEntity* function)
{
	return function != nullptr && function->member && function->owner->entity != nullptr &&
		graph.closure_call(static_cast<const ClassEntity*>(function->owner->entity)) == function;
}

} // namespace

void Analyzer::range_for(const syntax::RangeForStmt& loop, Scope* scope)
{
	// 6.5.4p2: the for-range-declaration's decl-specifiers are
	// type-specifiers or constexpr (no storage class, function specifier,
	// typedef or friend).
	for (const syntax::DeclSpecifier& specifier : loop.specifiers)
	{
		if (specifier.kind != syntax::DeclSpecKind::Keyword)
			continue;
		switch (specifier.keyword)
		{
		case TokenKind::KW_TYPEDEF:
		case TokenKind::KW_FRIEND:
		case TokenKind::KW_EXTERN:
		case TokenKind::KW_STATIC:
		case TokenKind::KW_REGISTER:
		case TokenKind::KW_THREAD_LOCAL:
		case TokenKind::KW_MUTABLE:
		case TokenKind::KW_INLINE:
		case TokenKind::KW_VIRTUAL:
		case TokenKind::KW_EXPLICIT:
			error(specifier.location, "a for-range-declaration's decl-specifier is no type-specifier or constexpr");
		default:
			break;
		}
	}
	// 6.5.4: the loop's names are the statement's (3.3.3p4: its declaration
	// is not redeclared in the controlled statement's outermost block).
	Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
	BodyContext* body = semantic() ? body_ : nullptr;
	if (body != nullptr && !body->pattern && deferring_reserved_ != 0)
	{
		// A hosted wrapper's range-init calling a reserved builtin this
		// compiler lacks (see deferrable_statement) is deferred alone when
		// the loop variable's type is declared: its names checked, the
		// variable declared, the statement typed (its return statements
		// deduce). With `auto` (the range's element type) the statement is
		// deferred whole.
		const uint32_t unevaluated = unevaluated_;
		range_for_equivalent(loop, block);
		if (deferred_reserved_call())
		{
			const uint32_t deferred_at = failure_.location;
			consume_deferred_call();
			body_ = body;
			unevaluated_ = unevaluated;
			Specifiers declared;
			this->specifiers(loop.specifiers, scope, declared, false);
			if (declared.type == nullptr || holds_auto(declared.type))
			{
				// Deferred whole by the enclosing statement.
				defer_reserved_call(deferred_at);
				return;
			}
			body->function->unanalyzed_body = true;
			block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
			body->pattern = true;
			range_for_names(loop, block);
			body->pattern = false;
		}
	}
	else if (body != nullptr && !body->pattern)
		range_for_equivalent(loop, block);
	else
		range_for_names(loop, block);
	// A deferral pending (see deferrable_statement): the statement is
	// deferred whole.
	if (deferred_reserved_call())
		return;
	if (body != nullptr)
		++body->loops;
	substatement(*loop.body, block);
	if (body != nullptr)
		--body->loops;
}

void Analyzer::range_for_names(const syntax::RangeForStmt& loop, Scope* block)
{
	// Types mode and template definitions: the range's names, and the loop
	// variable with its declared type; `auto` is deduced from `*__begin`,
	// an lvalue of an array range's element type or of a braced-init-list
	// range's const E (std::initializer_list<E>'s, 18.9p1, see
	// range_for_equivalent). In a template
	// definition, another range's element type (or a braced-init-list's
	// dependent element's) is known in its instantiations: the variable
	// is typed by the range's (or that element's) own dependent
	// placeholder until then.
	// Whether the range names anything dependent (see dependent_uses: a
	// template parameter in a value-dependent operand too).
	const uint64_t uses = names_.dependent_uses();
	expression(loop.range, block);
	const bool names_dependent = names_.dependent_uses() != uses;
	Specifiers specifiers;
	this->specifiers(loop.specifiers, block, specifiers, false);
	if (specifiers.type == nullptr)
		error(loop.location, "range-based for declaration without a type");
	const Type* type = specifiers.type;
	if (type == auto_placeholder_)
	{
		const Type* element = nullptr;
		const Type* dependent = nullptr;
		// The category of the element the variable is initialized by.
		ValueCategory element_category = ValueCategory::LValue;
		if (loop.range->kind == syntax::ExprKind::BracedInit)
		{
			const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*loop.range);
			if (list.elements.empty())
				error(loop.location, "a range-based for over an empty braced-init-list deduces no element type");
			for (const syntax::Expr* item : list.elements)
			{
				// A braced element deduces nothing (14.8.2.1p1).
				if (item->kind == syntax::ExprKind::BracedInit)
					continue;
				// Its decltype type: an lvalue's a reference, a named variable
				// an lvalue (as deduced_type reads an initializer here).
				const Type* item_type = decltype_type(*item, block);
				// A deferred reserved builtin call (see range_for).
				if (item_type == nullptr && deferred_reserved_call())
					return;
				const ValueCategory category = item_type->kind == TypeKind::LValueReference ||
						item->kind == syntax::ExprKind::Id
					? ValueCategory::LValue
					: ValueCategory::PRValue;
				if (item_type->is_reference())
					item_type = item_type->element;
				if (template_depth_ != 0 && dependent_type(item_type))
				{
					dependent = dependent != nullptr ? dependent : item_type;
					continue;
				}
				TypeList deduced;
				deduced.push_back(nullptr);
				if (!deduce_argument(auto_placeholder_, item_type, category, 0, deduced) || deduced[0] == nullptr ||
					(element != nullptr && deduced[0]->canonical != element->canonical))
					error(item->location, "a braced-init-list range's elements deduce different types (7.1.6.4p6)");
				element = deduced[0];
			}
			if (element == nullptr && dependent == nullptr)
				error(loop.location, "a braced-init-list range without an element that deduces its type (7.1.6.4p6)");
			// The range is a std::initializer_list<E> (see
			// range_for_equivalent), which must be declared (8.5.4p2): checked
			// here unless an element is dependent (an instantiation checks its
			// own, as g++ checks a template's).
			if (dependent == nullptr)
				declared_initializer_list(loop.location);
			if (element != nullptr)
				element = types_.qualified(element, kConst);
		}
		else
		{
			const Type* range = decltype_type(*loop.range, block);
			// A deferred reserved builtin call (see range_for).
			if (range == nullptr && deferred_reserved_call())
				return;
			if (range->is_reference())
				range = range->element;
			if (range->kind == TypeKind::Array)
				element = range->element;
			else if (template_depth_ == 0)
				error(loop.location, "unsupported range-based for deduction");
			else if (semantic() && body_ != nullptr)
				element = known_range_element(loop, block, names_dependent, element_category);
			if (element == nullptr && template_depth_ != 0)
				dependent = dependent_decltype(*loop.range, block);
		}
		if (dependent != nullptr)
			type = dependent;
		else
		{
			// 7.1.6.4p6: the declared type with auto replaced by U deduced
			// from the element lvalue.
			auto_declarator_ = true;
			const Type* declared =
				declarator(loop.declarator, auto_placeholder_, block, loop.location, &specifiers).type;
			auto_declarator_ = false;
			TypeList deduced;
			deduced.push_back(nullptr);
			if (!holds_auto(declared) ||
				!deduce_argument(
					replaced_auto(declared, auto_placeholder_, specifiers.cv), element, element_category, 0, deduced) ||
				deduced[0] == nullptr)
				error(loop.location, "auto deduction fails for the range's element (7.1.6.4p6)");
			type = replaced_auto(declared, deduced[0], specifiers.cv);
		}
	}
	else
		type = declarator(loop.declarator, type, block, loop.location).type;
	const syntax::Name* id = DeclaratorId(loop.declarator);
	if (id == nullptr)
		error(loop.location, "range-based for declaration without a name");
	Entity* variable = new_entity<VariableEntity>(EntityKind::Variable, id->last().identifier, block, type);
	Binding* binding = new_binding(BindingKind::Variable, id->last().identifier, variable, type);
	print_in(block, binding);
	declare_in(block, binding);
	// Naming it names something dependent only if its type is dependent
	// (14.6.2.2p3; a dependent range's placeholder is).
	binding->dependent = dependent_type(type);
}

const Type* Analyzer::known_range_element(
	const syntax::RangeForStmt& loop, Scope* block, bool names_dependent, ValueCategory& category)
{
	// A template definition's range that is not type-dependent (14.6.2.2):
	// the variable's type is known in the definition, `*__begin`'s
	// (6.5.4p1), typed as written there, unevaluated (instantiations analyze
	// their own copy of the statement), `this` the member function's (a
	// lambda's body's, its enclosing one's); not for a list's builtin form
	// (null: the dependent form). Typed as a non-template expression, it
	// names nothing dependent: a value-dependent range (say f(sizeof(T)))
	// keeps the dependent form, as an auto variable's initializer does.
	// Or a data member of the current instantiation (14.6.2.1p1) named
	// alone or as `this->m` in a member function of its class (or of a
	// class it is a base of): not type-dependent when its declared type is
	// not (14.6.2.2p3, p5), though the analysis takes every name in a
	// templated class's member as dependent; typed from its declared type,
	// a non-static one's with the function's cv (5.1.1p13, 5.2.5p4).
	const BodyContext* owner = body_;
	while (owner != nullptr && ClosureBody(graph_, owner->function))
		owner = owner->enclosing;
	const FunctionEntity* function = owner != nullptr ? owner->function : nullptr;
	const VariableEntity* member = nullptr;
	if (names_dependent || type_dependent(loop.range, block))
	{
		if (function == nullptr || !function->member || function->owner == nullptr ||
			function->owner->kind != ScopeKind::Class || function->owner->entity == nullptr)
			return nullptr;
		const ClassEntity* cls = static_cast<const ClassEntity*>(function->owner->entity);
		const syntax::Expr* inner = WithoutParentheses(loop.range);
		const syntax::Name* name = nullptr;
		bool arrow = false;
		if (inner->kind == syntax::ExprKind::Id)
			name = static_cast<const syntax::IdExpr*>(inner)->name;
		else if (inner->kind == syntax::ExprKind::Member)
		{
			const syntax::MemberExpr* access = static_cast<const syntax::MemberExpr*>(inner);
			const syntax::Expr* object = WithoutParentheses(access->base);
			if (access->op != TokenKind::OP_ARROW || object->kind != syntax::ExprKind::KeywordLiteral ||
				static_cast<const syntax::KeywordLiteralExpr*>(object)->keyword != TokenKind::KW_THIS)
				return nullptr;
			name = access->member;
			arrow = true;
		}
		if (name == nullptr || name->qualified() || name->last().kind != syntax::NameComponentKind::Identifier ||
			name->last().has_arguments)
			return nullptr;
		const uint64_t saved = names_.dependent_uses();
		const LookupResult found = arrow ? names_.qualified(cls->scope, name->last().identifier, LookupFilter::Ordinary)
										 : names_.unqualified(block, name->last().identifier, LookupFilter::Ordinary);
		names_.set_dependent_uses(saved);
		if (!found.found() || found.ambiguous || found.binding->kind != BindingKind::Variable ||
			found.binding->entity == nullptr)
			return nullptr;
		member = static_cast<const VariableEntity*>(found.binding->entity);
		const Scope* home = member->owner;
		if (home == nullptr || home->kind != ScopeKind::Class || home->entity == nullptr ||
			dependent_type(member->type))
			return nullptr;
		const ClassEntity* holder = static_cast<const ClassEntity*>(home->entity);
		if (member->member &&
			(function->is_static || member->bit_field || (holder != cls && !conversions_.is_base_of(holder, cls))))
			return nullptr;
	}
	struct Known
	{
		Analyzer& analyzer;
		uint32_t depth;
		const Type* this_type;
		Known(Analyzer& a, const FunctionEntity* function)
			: analyzer(a), depth(a.template_depth_), this_type(a.body_->this_type)
		{
			if (this_type == nullptr && function != nullptr && function->member && !function->is_static &&
				function->owner->entity != nullptr)
				analyzer.body_->this_type = analyzer.types_.pointer(
					analyzer.types_.qualified(function->owner->entity->type, function->type->function_cv));
			analyzer.template_depth_ = 0;
			++analyzer.unevaluated_;
		}
		~Known()
		{
			analyzer.body_->this_type = this_type;
			analyzer.template_depth_ = depth;
			--analyzer.unevaluated_;
		}
	} known(*this, function);
	syntax::Expr* range_node = loop.range;
	ExprFacts* typed_range = nullptr;
	if (member != nullptr)
	{
		const uint8_t cv = member->member && !member->is_mutable ? function->type->function_cv : 0;
		const Type* member_type = member->type->is_reference() ? member->type->element
															   : types_.qualified(member->type, cv);
		syntax::IdExpr* use = implicit_node<syntax::IdExpr>(loop.location);
		use->kind = syntax::ExprKind::Id;
		use->name = implicit_name(member->name, loop.location);
		typed_range = new_facts(*use, member_type, ValueCategory::LValue);
		typed_range->entity = member;
		range_node = use;
	}
	else
		typed_range = typed_value(*loop.range, block);
	const ClassEntity* cls = typed_range != nullptr ? ClassOf(typed_range->type) : nullptr;
	if (typed_range != nullptr && typed_range->type->kind == TypeKind::Array)
		return typed_range->type->element;
	if (cls == nullptr || builtin_list_class(cls) || !require_complete(cls))
		return nullptr;
	typed_range->flags |= kImplicit;
	const Identifier* begin_name = range_for_identifier(kBeginName);
	implicit_members_for(cls->scope, begin_name);
	const bool members = names_.qualified(cls->scope, begin_name, LookupFilter::Ordinary).found() ||
		names_.qualified(cls->scope, range_for_identifier(kEndName), LookupFilter::Ordinary).found();
	syntax::CallExpr* call = implicit_node<syntax::CallExpr>(loop.location);
	call->kind = syntax::ExprKind::Call;
	if (members)
	{
		syntax::MemberExpr* access = implicit_node<syntax::MemberExpr>(loop.location);
		access->kind = syntax::ExprKind::Member;
		access->op = TokenKind::OP_DOT;
		access->base = range_node;
		access->member = implicit_name(begin_name, loop.location);
		call->callee = access;
	}
	else
	{
		syntax::Expr** argument = static_cast<syntax::Expr**>(
			arena_.allocate(sizeof(syntax::Expr*), alignof(syntax::Expr*)));
		*argument = range_node;
		syntax::IdExpr* callee = implicit_node<syntax::IdExpr>(loop.location);
		callee->kind = syntax::ExprKind::Id;
		callee->name = implicit_name(begin_name, loop.location);
		call->callee = callee;
		call->arguments.data = argument;
		call->arguments.size = 1;
		argument_dependent_call(*call, begin_name, true, block);
	}
	syntax::UnaryExpr* dereference = implicit_node<syntax::UnaryExpr>(loop.location);
	dereference->kind = syntax::ExprKind::Unary;
	dereference->op = TokenKind::OP_STAR;
	dereference->operand = call;
	const ExprFacts* item = typed_value(*dereference, block);
	if (item == nullptr)
		return nullptr;
	category = item->category;
	return item->type;
}

template<typename T>
T* Analyzer::implicit_node(uint32_t location)
{
	T* node = new (arena_.allocate(sizeof(T), alignof(T))) T();
	located(*node, location);
	return node;
}

const Identifier* Analyzer::range_for_identifier(unsigned which)
{
	if (range_for_identifiers_[which] == nullptr)
		range_for_identifiers_[which] = special_key(kRangeForSpellings[which]);
	return range_for_identifiers_[which];
}

syntax::Name* Analyzer::implicit_name(const Identifier* identifier, uint32_t location)
{
	syntax::NameComponent* component = implicit_node<syntax::NameComponent>(location);
	component->kind = syntax::NameComponentKind::Identifier;
	component->identifier = identifier;
	syntax::Name* name = implicit_node<syntax::Name>(location);
	name->components.data = component;
	name->components.size = 1;
	return name;
}

syntax::InitDeclarator* Analyzer::implicit_variable(
	unsigned key, const Type* declared, syntax::Expr* init, Scope* block, uint32_t location)
{
	// `declared name = init;` in the statement's block; `auto` (or a
	// reference to it) is deduced from `init` as a written declaration's.
	syntax::Initializer* initializer = implicit_node<syntax::Initializer>(location);
	initializer->kind = syntax::InitializerKind::Assign;
	initializer->expression = init;
	Specifiers specifiers = Specifiers();
	specifiers.type = declared;
	specifiers.location = location;
	specifiers.uses = names_.dependent_uses();
	DeclaratorInfo info = DeclaratorInfo();
	info.type = declared;
	info.name = implicit_name(range_for_identifier(key), location);
	info.location = location;
	if (holds_auto(declared))
	{
		const Type* saved_auto = auto_deduced_;
		auto_deduced_ = nullptr;
		info.type = deduced_type(specifiers, declared, initializer, block, location);
		auto_deduced_ = saved_auto;
		// A deferred reserved builtin call in `init` (see range_for): no
		// variable.
		if (deferred_reserved_call())
			return nullptr;
	}
	Binding* binding = declare_variable(specifiers, info, initializer, block, location);
	// Its lookup key names no written variable; it prints (and its storage
	// is named) as the variable 6.5.4p1 shows.
	binding->entity->name = range_for_identifier(key + (kRangeLabel - kRangeKey));
	syntax::InitDeclarator* variable = implicit_node<syntax::InitDeclarator>(location);
	variable->initializer = initializer;
	variable->declared = binding;
	return variable;
}

syntax::Expr* Analyzer::implicit_use(const syntax::InitDeclarator& variable, uint32_t location, bool dependent)
{
	// An lvalue naming one of the implicit variables (5.1.1p8); a use of
	// one whose type came from a range that named something dependent is
	// a dependent use (see kDependentUse: in an instantiation, operators
	// applied to it and the loop variable deduced from it are looked up
	// and marked as the written equivalent's would be).
	const Entity* entity = variable.declared->entity;
	const Type* type = entity->type->is_reference() ? entity->type->element : entity->type;
	syntax::IdExpr* use = implicit_node<syntax::IdExpr>(location);
	use->kind = syntax::ExprKind::Id;
	use->name = implicit_name(entity->name, location);
	ExprFacts* facts = new_facts(*use, type, ValueCategory::LValue);
	facts->entity = entity;
	facts->flags = kImplicit | (dependent ? kDependentUse : 0);
	return use;
}

syntax::Expr* Analyzer::implicit_copy(const syntax::Expr& named)
{
	// Another use of the variable a range's id-expression names.
	const syntax::IdExpr& id = static_cast<const syntax::IdExpr&>(*WithoutParentheses(&named));
	syntax::IdExpr* use = implicit_node<syntax::IdExpr>(id.location);
	use->kind = syntax::ExprKind::Id;
	use->name = id.name;
	ExprFacts* facts = new_facts(*use, id.facts->type, id.facts->category);
	*facts = *id.facts;
	facts->conversion = nullptr;
	facts->flags |= kImplicit;
	return use;
}

syntax::Expr* Analyzer::implicit_constant(const Type* type, uint64_t value, uint32_t location)
{
	syntax::IdExpr* constant = implicit_node<syntax::IdExpr>(location);
	constant->kind = syntax::ExprKind::Id;
	constant->name = implicit_name(range_for_identifier(kIndexLabel), location);
	ExprFacts* facts = new_facts(*constant, type, ValueCategory::PRValue);
	facts->flags = kImplicit | kFolded;
	facts->value = value;
	return constant;
}

ExprFacts* Analyzer::typed_already(const syntax::Expr& expr)
{
	// An expression typed before it is typed again (an implicit node, or a
	// call's operand typed with its callee): its facts as they are. An
	// implicit dependent use is noted again, as typing it again would
	// (see Analyzer::typed).
	if ((expr.facts->flags & (kImplicit | kDependentUse)) == (kImplicit | kDependentUse))
		names_.note_dependent_use();
	return const_cast<ExprFacts*>(expr.facts);
}

ExprFacts* Analyzer::argument_dependent_call(syntax::CallExpr& call, const Identifier* name, bool bound, Scope* scope)
{
	// `name(arguments)` whose functions argument-dependent lookup alone
	// finds (3.4.2); typed once (kImplicit).
	for (const syntax::Expr* argument : call.arguments)
		typed_value(*argument, scope);
	expression_scope_ = scope;
	adl_functions_.clear();
	argument_dependent(call.arguments, name, bound);
	if (adl_functions_.empty())
		error(call.location, "no begin or end function for the range (6.5.4p1: argument-dependent lookup)");
	explicit_arguments_ = nullptr;
	ExprFacts* facts = resolved_call(call, nullptr, nullptr, false, nullptr);
	if (facts == nullptr)
		error(call.location, "no viable begin or end function for the range");
	facts->flags |= kImplicit;
	return facts;
}

void Analyzer::range_for_equivalent(const syntax::RangeForStmt& loop, Scope* block)
{
	// 6.5.4p1 in the statement's block: __range, then __begin and __end
	// (the index), then the for-range-declaration; a lambda in range-init
	// is the range's (an unevaluated operand is none of them).
	const uint32_t location = loop.location;
	RangeForFacts* facts = new (arena_.allocate(sizeof(RangeForFacts), alignof(RangeForFacts))) RangeForFacts();
	syntax::Expr* range = loop.range;
	const uint64_t uses = names_.dependent_uses();
	const syntax::Expr* named = nullptr;
	const Type* range_type = nullptr;
	if (range->kind == syntax::ExprKind::BracedInit)
	{
		// `auto&& __range = braced-init-list;`: auto deduces
		// std::initializer_list<E> from the elements (7.1.6.4p6), an
		// implicit use of the class template, ill-formed unless it is
		// declared (8.5.4p2); the range is that class (18.9: its begin and
		// end members, or the builtin layout of a template declared
		// without a definition).
		facts->range = implicit_variable(kRangeKey, types_.reference(auto_placeholder_, true), range, block, location);
		if (facts->range == nullptr)
			return;
		range_type = facts->range->declared->entity->type->element;
	}
	else
	{
		ExprFacts* typed_range = typed_value(*range, block);
		if (typed_range == nullptr)
			return;
		typed_range->flags |= kImplicit;
		const syntax::Expr* inner = WithoutParentheses(range);
		const Entity* entity = typed_range->entity;
		if (inner->kind == syntax::ExprKind::Id && entity != nullptr &&
			(entity->kind == EntityKind::Variable || entity->kind == EntityKind::Parameter) &&
			(typed_range->flags & (kImplicitThis | kAnonymousMember)) == 0 &&
			!(entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(entity)->member))
		{
			named = range;
			range_type = typed_range->type;
		}
		else
		{
			facts->range = implicit_variable(
				kRangeKey, types_.reference(auto_placeholder_, true), range, block, location);
			if (facts->range == nullptr)
				return;
			range_type = facts->range->declared->entity->type->element;
		}
	}
	const bool bound = names_.dependent_uses() == uses;
	// Each use of __range: a copy of the variable's name, or __range.
	struct RangeUse
	{
		Analyzer* analyzer;
		const syntax::Expr* named;
		const syntax::InitDeclarator* range;
		uint32_t location;
		bool dependent;
		syntax::Expr* operator()() const
		{
			return named != nullptr ? analyzer->implicit_copy(*named)
									: analyzer->implicit_use(*range, location, dependent);
		}
	};
	const RangeUse range_use = RangeUse{ this, named, facts->range, location, !bound };
	syntax::Expr* element = nullptr;
	if (builtin_list_class(ClassOf(range_type)))
	{
		// std::initializer_list's builtin layout, the course's form: an int
		// index from 0 while less than the element count, the element
		// `__range.__begin[__idx]` (18.9.1: begin() is its array's first
		// element, size() its count).
		const Type* index = types_.fundamental(FundamentalType::Int);
		syntax::MemberExpr* members[2];
		for (unsigned i = 0; i < 2; ++i)
		{
			members[i] = implicit_node<syntax::MemberExpr>(location);
			members[i]->kind = syntax::ExprKind::Member;
			members[i]->op = TokenKind::OP_DOT;
			members[i]->base = range_use();
			members[i]->member = implicit_name(builtin_list_key(i), location);
		}
		facts->begin = implicit_variable(kIndexKey, index, implicit_constant(index, 0, location), block, location);
		syntax::BinaryExpr* condition = implicit_node<syntax::BinaryExpr>(location);
		condition->kind = syntax::ExprKind::Binary;
		condition->op = TokenKind::OP_LT;
		condition->left = implicit_use(*facts->begin, location, false);
		condition->right = members[1];
		facts->condition = condition;
		syntax::UnaryExpr* increment = implicit_node<syntax::UnaryExpr>(location);
		increment->kind = syntax::ExprKind::Unary;
		increment->op = TokenKind::OP_INC;
		increment->operand = implicit_use(*facts->begin, location, false);
		facts->increment = increment;
		syntax::SubscriptExpr* subscript = implicit_node<syntax::SubscriptExpr>(location);
		subscript->kind = syntax::ExprKind::Subscript;
		subscript->base = members[0];
		subscript->index = implicit_use(*facts->begin, location, false);
		element = subscript;
	}
	else if (range_type->kind == TypeKind::Array)
	{
		// An array of known bound (6.5.4p1: of unknown size or incomplete
		// element type, the program is ill-formed): the index is an int
		// when every position fits one, else a long.
		if (range_type->bound == 0)
			error(location, "a range-based for over an array of unknown bound");
		const Type* index = types_.fundamental(
			range_type->bound <= 0x7fffffffu ? FundamentalType::Int : FundamentalType::LongInt);
		facts->begin = implicit_variable(kIndexKey, index, implicit_constant(index, 0, location), block, location);
		syntax::BinaryExpr* condition = implicit_node<syntax::BinaryExpr>(location);
		condition->kind = syntax::ExprKind::Binary;
		condition->op = TokenKind::OP_LT;
		condition->left = implicit_use(*facts->begin, location, false);
		condition->right = implicit_constant(index, range_type->bound, location);
		facts->condition = condition;
		syntax::UnaryExpr* increment = implicit_node<syntax::UnaryExpr>(location);
		increment->kind = syntax::ExprKind::Unary;
		increment->op = TokenKind::OP_INC;
		increment->operand = implicit_use(*facts->begin, location, false);
		facts->increment = increment;
		syntax::SubscriptExpr* subscript = implicit_node<syntax::SubscriptExpr>(location);
		subscript->kind = syntax::ExprKind::Subscript;
		subscript->base = range_use();
		subscript->index = implicit_use(*facts->begin, location, false);
		element = subscript;
	}
	else
	{
		const ClassEntity* cls = ClassOf(range_type);
		if (cls == nullptr)
			error(location, "a range-based for over a range that is no array or class");
		if (!require_complete(cls))
			error(location, "a range-based for over an incomplete class");
		// Its members begin and end (class member access lookup, 3.4.5),
		// if either is found; else begin(__range) and end(__range) by
		// argument-dependent lookup alone.
		const Identifier* begin_name = range_for_identifier(kBeginName);
		const Identifier* end_name = range_for_identifier(kEndName);
		implicit_members_for(cls->scope, begin_name);
		const bool members = names_.qualified(cls->scope, begin_name, LookupFilter::Ordinary).found() ||
			names_.qualified(cls->scope, end_name, LookupFilter::Ordinary).found();
		syntax::Expr* ends[2];
		for (unsigned i = 0; i < 2; ++i)
		{
			const Identifier* name = i == 0 ? begin_name : end_name;
			syntax::CallExpr* call = implicit_node<syntax::CallExpr>(location);
			call->kind = syntax::ExprKind::Call;
			if (members)
			{
				syntax::MemberExpr* access = implicit_node<syntax::MemberExpr>(location);
				access->kind = syntax::ExprKind::Member;
				access->op = TokenKind::OP_DOT;
				access->base = range_use();
				access->member = implicit_name(name, location);
				call->callee = access;
				ExprFacts* typed_call = typed_value(*call, block);
				if (typed_call == nullptr)
					return;
				typed_call->flags |= kImplicit;
			}
			else
			{
				syntax::IdExpr* callee = implicit_node<syntax::IdExpr>(location);
				callee->kind = syntax::ExprKind::Id;
				callee->name = implicit_name(name, location);
				syntax::Expr** argument = static_cast<syntax::Expr**>(
					arena_.allocate(sizeof(syntax::Expr*), alignof(syntax::Expr*)));
				*argument = range_use();
				call->callee = callee;
				call->arguments.data = argument;
				call->arguments.size = 1;
				argument_dependent_call(*call, name, bound, block);
			}
			ends[i] = call;
		}
		// `auto __begin = begin-expr, __end = end-expr;`: one declaration,
		// so both deduce one type (7.1.6.4p7).
		facts->begin = implicit_variable(kBeginKey, auto_placeholder_, ends[0], block, location);
		facts->end = implicit_variable(kEndKey, auto_placeholder_, ends[1], block, location);
		if (facts->begin->declared->entity->type->canonical != facts->end->declared->entity->type->canonical)
			error(location, "a range's begin and end deduce different types (7.1.6.4p7)");
		syntax::BinaryExpr* condition = implicit_node<syntax::BinaryExpr>(location);
		condition->kind = syntax::ExprKind::Binary;
		condition->op = TokenKind::OP_NE;
		condition->left = implicit_use(*facts->begin, location, !bound);
		condition->right = implicit_use(*facts->end, location, !bound);
		facts->condition = condition;
		syntax::UnaryExpr* increment = implicit_node<syntax::UnaryExpr>(location);
		increment->kind = syntax::ExprKind::Unary;
		increment->op = TokenKind::OP_INC;
		increment->operand = implicit_use(*facts->begin, location, !bound);
		facts->increment = increment;
		syntax::UnaryExpr* dereference = implicit_node<syntax::UnaryExpr>(location);
		dereference->kind = syntax::ExprKind::Unary;
		dereference->op = TokenKind::OP_STAR;
		dereference->operand = implicit_use(*facts->begin, location, !bound);
		element = dereference;
	}
	if (typed_value(*facts->condition, block) == nullptr || typed_value(*facts->increment, block) == nullptr)
		return;
	convert_to_bool(*facts->condition);
	ExprFacts* typed_element = typed_value(*element, block);
	if (typed_element == nullptr)
		return;
	typed_element->flags |= kImplicit;
	// `for-range-declaration = *__begin;`
	Specifiers specifiers;
	this->specifiers(loop.specifiers, block, specifiers, false);
	if (specifiers.type == nullptr)
		error(location, "range-based for declaration without a type");
	syntax::Initializer* initializer = implicit_node<syntax::Initializer>(location);
	initializer->kind = syntax::InitializerKind::Assign;
	initializer->expression = element;
	facts->declaration.declarator = loop.declarator;
	facts->declaration.initializer = initializer;
	const Type* saved_auto = auto_deduced_;
	auto_deduced_ = nullptr;
	declare_declarator(specifiers, facts->declaration, block, location);
	auto_deduced_ = saved_auto;
	if (facts->declaration.declared == nullptr || facts->declaration.declared->kind != BindingKind::Variable)
		error(location, "a for-range-declaration that declares no variable");
	loop.facts = facts;
}

} // namespace sema
