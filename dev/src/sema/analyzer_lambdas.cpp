#include "sema/analyzer.h"

#include <cstdio>

// Lambda expressions (5.1.2; PA20's subset: captureless lambdas and
// captures by reference of automatic variables and of `this`): a lambda's
// closure type is an unnamed class of the smallest scope that contains it
// (p3), complete where the lambda appears, with a public inline function
// call operator (p5) whose body is analyzed there too, its names found in
// the lambda's context (p7). The body's odr-uses of the enclosing
// functions' automatic variables and of `this` capture them (p11-p13),
// each capture a member of the closure; the expression is a prvalue of
// the closure type, its members initialized with the captured entities.
namespace sema {

namespace {

// Whether a name used in `scope` for an automatic variable or parameter of
// the block or function scope `owner` is used in a body nested in owner's
// function: a local class's (across its class scope) or a lambda's (across
// its function scope, parented at the enclosing block).
bool InNestedBody(const Scope* scope, const Scope* owner)
{
	bool nested = false;
	for (const Scope* s = scope; s != nullptr; s = s->parent)
	{
		if (s == owner)
			return nested;
		nested = nested || s->kind == ScopeKind::Class || s->kind == ScopeKind::Function;
	}
	return false;
}

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

} // namespace

ExprFacts* Analyzer::lambda_expression(const syntax::LambdaExpr& expr, Scope* scope)
{
	// p2: no lambda-expression appears in an unevaluated operand (sizeof's,
	// decltype's, noexcept's; an auto variable's initializer is none, see
	// deduced_type).
	if (unevaluated_ != 0)
		error(expr.location, "a lambda-expression in an unevaluated operand");
	if (template_depth_ != 0)
		error(expr.location, "a lambda in a template definition is analyzed in its instantiations");
	// One closure type per lambda-expression (p3): an expression typed
	// again (an auto variable's initializer, deduced first) keeps its own.
	const ClassEntity* typed_before = expr.facts != nullptr ? ClassOf(expr.facts->type) : nullptr;
	if (typed_before != nullptr && graph_.closure_call(typed_before) != nullptr)
		return new_facts(expr, typed_before->type, ValueCategory::PRValue);
	Scope* where = declaration_scope(scope);
	ClassEntity* closure = new_class(TokenKind::KW_CLASS, nullptr, where);
	// Its presentation name (a unit-unique label, as an unnamed class's).
	char buffer[40];
	const int length = std::snprintf(buffer, sizeof buffer, "__lambda%u", ++graph_.local_types);
	closure->label = copy_label(buffer, static_cast<size_t>(length));
	closure->local_label = closure->label;
	closure->scope = new_scope(ScopeKind::Class, where, where, closure, closure->label);
	closure->scope->dumped = false;
	ClosureFacts* facts = static_cast<ClosureFacts*>(arena_.allocate(sizeof(ClosureFacts), alignof(ClosureFacts)));
	*facts = ClosureFacts();
	facts->enclosing = body_ != nullptr ? body_->function : nullptr;
	facts->clause_token = expr.clause_token;
	facts->body_token = expr.body_token;
	if (expr.template_parameters != nullptr)
		return templated_lambda(closure, facts, expr, scope);
	facts->call = closure_call(closure, expr, scope);
	if (facts->call == nullptr)
		return nullptr;
	graph_.closure_index.set(closure, static_cast<uint32_t>(graph_.closures.size()));
	graph_.closures.push_back(facts);
	FunctionEntity* call = facts->call;
	// Complete: a class that is no aggregate (p3), empty until it captures,
	// with its implicit special members, its default constructor and copy
	// assignment operator deleted (p19; see implicit_constructor and
	// declare_implicit_copy_members, which make them on demand).
	closure->complete = true;
	closure->empty = true;
	closure->not_aggregate = true;
	if (semantic())
		number_closure(closure, facts);
	// The captures the lambda-capture writes, in the enclosing context.
	LambdaFrame frame;
	frame.closure = closure;
	frame.facts = facts;
	frame.expr = &expr;
	frame.this_captured = false;
	explicit_captures(frame, scope);
	// The body, analyzed where the lambda appears: the expression's state
	// around it is set aside (the body is no unevaluated operand, p2, and
	// no part of a substitution). An instantiation's copy whose parameter
	// clause expanded function parameter packs binds their names to the
	// packs of its parameters (sizeof... counts them).
	syntax::FunctionBody* body = static_cast<syntax::FunctionBody*>(
		arena_.allocate(sizeof(syntax::FunctionBody), alignof(syntax::FunctionBody)));
	*body = syntax::FunctionBody();
	body->body = expr.body;
	Scope* parent = scope;
	if (expr.pattern_parameters != nullptr)
		parent = function_pack_scope(
			scope, *expr.pattern_parameters, expr.parameter_packs.data, expr.parameter_packs.size, call->type);
	Scope* saved_scope = expression_scope_;
	const ClassEntity* saved_naming = naming_class_;
	const NamePrefix saved_prefix = last_prefix_;
	const uint32_t saved_unevaluated = unevaluated_;
	const uint32_t saved_deferring = deferring_;
	const uint32_t saved_throwing = potentially_throwing_;
	const uint32_t saved_soft = soft_substitution_;
	const bool saved_callee = callee_name_;
	const FunctionEntity* saved_deducing = deducing_call_;
	LambdaFrame* saved_lambda = pending_lambda_;
	unevaluated_ = 0;
	deferring_ = 0;
	soft_substitution_ = 0;
	callee_name_ = false;
	deducing_call_ = expr.trailing == nullptr && expr.body != nullptr ? call : nullptr;
	pending_lambda_ = &frame;
	++lambda_bodies_;
	const bool deducible = function_body(call, call->type, expr.parameters, *body, closure->scope, parent);
	--lambda_bodies_;
	pending_lambda_ = saved_lambda;
	deducing_call_ = saved_deducing;
	expression_scope_ = saved_scope;
	naming_class_ = saved_naming;
	last_prefix_ = saved_prefix;
	unevaluated_ = saved_unevaluated;
	deferring_ = saved_deferring;
	potentially_throwing_ = saved_throwing;
	soft_substitution_ = saved_soft;
	callee_name_ = saved_callee;
	finish_captures(frame);
	// Its special members, once its members are known (a capture by copy's
	// class makes them non-trivial).
	if (semantic())
	{
		declare_destructor(closure, expr.location);
		classify_copy_members(closure);
		virtual_facts(closure, expr.location);
	}
	// The capture records are dead once no lambda body is being analyzed.
	if (lambda_bodies_ == 0)
		captured_.clear();
	// p6: a lambda without a lambda-capture converts to a pointer to
	// function (its conversion function declared where first needed).
	if (!expr.has_default && expr.captures.empty() && semantic())
	{
		frame.facts->convertible = true;
		closure->has_conversion_functions = true;
	}
	// A return statement a hosted wrapper's deferral left untyped (see
	// Analyzer::deferrable_statement): the call's result is undeducible, so
	// the full-expression holding the lambda is deferred too.
	if (!deducible && deferring_reserved_ != 0)
	{
		defer_reserved_call(expr.location);
		return nullptr;
	}
	return new_facts(expr, closure->type, ValueCategory::PRValue);
}

ExprFacts* Analyzer::templated_lambda(
	ClassEntity* closure, ClosureFacts* facts, const syntax::LambdaExpr& expr, Scope* scope)
{
	// C++20's templated lambda (hosted compilations): the closure as a
	// lambda's (p3, p19), its call operator a member function template
	// (templated_closure_call) whose specializations' bodies are analyzed
	// where calls instantiate them, with the captures fixed here.
	graph_.closure_index.set(closure, static_cast<uint32_t>(graph_.closures.size()));
	graph_.closures.push_back(facts);
	facts->lambda = &expr;
	closure->empty = true;
	closure->not_aggregate = true;
	LambdaFrame frame;
	frame.closure = closure;
	frame.facts = facts;
	frame.expr = &expr;
	frame.this_captured = false;
	explicit_captures(frame, scope);
	facts->call = templated_closure_call(closure, expr, scope, frame);
	closure->complete = true;
	if (semantic())
		number_closure(closure, facts);
	finish_captures(frame);
	if (semantic())
	{
		declare_destructor(closure, expr.location);
		classify_copy_members(closure);
		virtual_facts(closure, expr.location);
	}
	if (lambda_bodies_ == 0)
		captured_.clear();
	return new_facts(expr, closure->type, ValueCategory::PRValue);
}

FunctionEntity* Analyzer::templated_closure_call(
	ClassEntity* closure, const syntax::LambdaExpr& expr, Scope* scope, LambdaFrame& frame)
{
	// `template<parameters> auto operator()(parameters) const` (not const
	// when mutable) with the lambda's exception-specification and trailing
	// return type, its body the lambda's: declared as a member template of
	// the closure is (public, inline). Its definition is walked now as a
	// template definition is; the enclosing automatic variables and `this`
	// its body names are captured then under a capture-default (C++14's
	// 5.1.2p12 for a generic lambda, as named in the body), before the
	// closure's members are laid out.
	struct Built
	{
		syntax::TemplateDecl templ;
		syntax::FunctionDecl function;
		syntax::Declarator declarator;
		syntax::Name name;
		syntax::NameComponent component;
		syntax::DeclaratorSuffix suffix;
		syntax::FunctionQualifier qualifiers[4];
		syntax::DeclSpecifier specifier;
		syntax::ParameterClause none;
		syntax::Decl* list[1];
	};
	Built* built = static_cast<Built*>(arena_.allocate(sizeof(Built), alignof(Built)));
	new (built) Built();
	built->component.kind = syntax::NameComponentKind::Operator;
	built->component.op = TokenKind::OP_LPAREN;
	built->component.location = expr.location;
	built->name.components = syntax::Slice<syntax::NameComponent>{ &built->component, 1 };
	built->name.location = expr.location;
	uint32_t count = 0;
	syntax::FunctionQualifier* qualifiers = built->qualifiers;
	if (!expr.is_mutable)
	{
		qualifiers[count].kind = syntax::FunctionQualifierKind::Cv;
		qualifiers[count++].token = TokenKind::KW_CONST;
	}
	if (expr.has_noexcept)
	{
		qualifiers[count].kind = syntax::FunctionQualifierKind::Noexcept;
		qualifiers[count].location = expr.location;
		qualifiers[count].has_operand = expr.noexcept_operand != nullptr;
		qualifiers[count++].noexcept_operand = expr.noexcept_operand;
	}
	if (expr.has_throw)
	{
		qualifiers[count].kind = syntax::FunctionQualifierKind::Throw;
		qualifiers[count].location = expr.location;
		qualifiers[count++].throw_types = expr.throw_types;
	}
	if (expr.trailing != nullptr)
	{
		qualifiers[count].kind = syntax::FunctionQualifierKind::TrailingReturn;
		qualifiers[count++].trailing = expr.trailing;
	}
	built->suffix.kind = syntax::DeclaratorSuffixKind::Function;
	built->suffix.parameters = expr.parameters != nullptr ? expr.parameters : &built->none;
	built->suffix.qualifiers = syntax::Slice<syntax::FunctionQualifier>{ qualifiers, count };
	built->declarator.location = expr.location;
	built->declarator.name = &built->name;
	built->declarator.suffixes = syntax::Slice<syntax::DeclaratorSuffix>{ &built->suffix, 1 };
	built->specifier.kind = syntax::DeclSpecKind::Keyword;
	built->specifier.keyword = TokenKind::KW_AUTO;
	built->specifier.location = expr.location;
	built->function.kind = syntax::DeclKind::Function;
	built->function.location = expr.location;
	built->function.specifiers = syntax::Slice<syntax::DeclSpecifier>{ &built->specifier, 1 };
	built->function.declarator = &built->declarator;
	built->function.body.body = expr.body;
	built->templ.kind = syntax::DeclKind::Template;
	built->templ.location = expr.location;
	built->templ.parameters = expr.template_parameters;
	built->templ.declaration = &built->function;
	built->list[0] = &built->templ;
	const TokenKind saved_access = current_access_;
	Scope* saved_member_scope = member_scope_;
	LambdaFrame* saved_scan = capture_scan_;
	const size_t first_deferred = deferred_.size();
	current_access_ = TokenKind::KW_PUBLIC;
	member_scope_ = closure->scope;
	++class_depth_;
	declarations(syntax::Slice<syntax::Decl*>{ built->list, 1 }, closure->scope);
	--class_depth_;
	current_access_ = saved_access;
	member_scope_ = saved_member_scope;
	capture_scan_ = &frame;
	finish_class_nesting(first_deferred);
	capture_scan_ = saved_scan;
	const LookupResult found = names_.qualified(
		closure->scope, operator_key(TokenKind::OP_LPAREN), LookupFilter::Ordinary);
	if (!found.found() || found.binding->kind != BindingKind::Function)
		error(expr.location, "a templated lambda without its call operator");
	return static_cast<FunctionEntity*>(found.binding->entity);
}

void Analyzer::scan_capture(const Binding* binding, uint32_t location)
{
	// A name a templated lambda's body uses (outside unevaluated operands):
	// an automatic variable of an enclosing function, captured by the
	// capture-default (a constant too: whether the specializations odr-use
	// it is not known here; C++20's 7.5.5.2p8 after P0588R1 captures what
	// the body potentially references, as g++ and Clang capture it); a
	// non-static member, whose implicit `this` the default captures.
	LambdaFrame& frame = *capture_scan_;
	const Entity* entity = binding->entity;
	if (entity == nullptr || !frame.expr->has_default || entity->owner == nullptr)
		return;
	const bool member = (entity->kind == EntityKind::Variable && static_cast<const VariableEntity*>(entity)->member) ||
		(entity->kind == EntityKind::Function && static_cast<const FunctionEntity*>(entity)->member &&
			!static_cast<const FunctionEntity*>(entity)->is_static);
	if (member)
	{
		if (body_ != nullptr && body_->enclosing != nullptr && body_->enclosing->this_type != nullptr)
			frame.this_captured = true;
		return;
	}
	const bool automatic = binding->kind == BindingKind::Parameter ||
		(binding->kind == BindingKind::Variable && entity->kind == EntityKind::Variable &&
			!static_cast<const VariableEntity*>(entity)->is_static &&
			!static_cast<const VariableEntity*>(entity)->is_extern &&
			!static_cast<const VariableEntity*>(entity)->is_thread_local &&
			(entity->owner->kind == ScopeKind::Block || entity->owner->kind == ScopeKind::Function));
	if (!automatic || encloses(frame.closure->scope, entity->owner))
		return;
	add_capture(frame, entity, false, frame.expr->capture_default == TokenKind::OP_ASS, location);
}

void Analyzer::scan_nested_lambda(const syntax::LambdaExpr& lambda, Scope* scope)
{
	// A lambda in a templated lambda's body: the names its captures and
	// body use are the templated lambda's to capture too (p13). Its body is
	// walked as the template definition's is, its parameters declared (as
	// dependent names), a templated one's template parameters too.
	for (const syntax::LambdaCapture& capture : lambda.captures)
	{
		if (capture.kind == TokenKind::KW_THIS)
		{
			if (capture_scan_->expr->has_default)
				capture_scan_->this_captured = true;
			continue;
		}
		const LookupResult found = unqualified(scope, capture.name, LookupFilter::Ordinary);
		if (found.found())
			scan_capture(found.binding, lambda.location);
	}
	if (lambda.body == nullptr)
		return;
	if (lambda.template_parameters != nullptr)
	{
		Scope* parameters = new_scope(ScopeKind::TemplateParameters, scope, scope, nullptr, nullptr);
		parameters->dumped = false;
		std::vector<TemplateParameterEntity*> entities;
		template_parameters(*lambda.template_parameters, parameters, true, &entities, -1);
		scope = parameters;
	}
	Scope* inner = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
	inner->dumped = false;
	const Type* placeholder = types_.fundamental(FundamentalType::Int);
	for (uint32_t i = 0; lambda.parameters != nullptr && i < lambda.parameters->parameters.size; ++i)
	{
		const syntax::Name* id = DeclaratorId(lambda.parameters->parameters[i]->declarator);
		if (id == nullptr || id->last().kind != syntax::NameComponentKind::Identifier)
			continue;
		const Identifier* name = id->last().identifier;
		Entity* parameter = new_entity<Entity>(EntityKind::Parameter, name, inner, placeholder);
		Binding* binding = new_binding(BindingKind::Parameter, name, parameter, placeholder);
		binding->dependent = true;
		binding->hidden = true;
		declare_in(inner, binding);
	}
	statement(*lambda.body, inner);
}

bool Analyzer::sealed_frame(const ClassEntity* closure, LambdaFrame& frame)
{
	// A templated lambda's call operator specialization (its closure's
	// facts): the frame of its body, its captures those of the closure.
	const ClosureFacts* facts = closure != nullptr ? graph_.closure_facts(closure) : nullptr;
	if (facts == nullptr || facts->lambda == nullptr)
		return false;
	frame.closure = const_cast<ClassEntity*>(closure);
	frame.facts = const_cast<ClosureFacts*>(facts);
	frame.expr = facts->lambda;
	frame.this_captured = facts->this_member != nullptr;
	frame.sealed = true;
	for (uint32_t i = 0; i < facts->capture_count; ++i)
	{
		const Capture& capture = facts->captures[i];
		frame.captures.push_back(capture.entity);
		if (captured_.find(EntityPair{ closure, capture.entity }) == nullptr)
			captured_.insert(EntityPair{ closure, capture.entity }, !capture.member->type->is_reference());
	}
	return true;
}

void Analyzer::explicit_captures(LambdaFrame& frame, Scope* scope)
{
	// p8: each capture once, and none repeating the capture-default's kind
	// (`this` after `=` included, C++11); p10: an identifier names an
	// automatic variable of the reaching scope, captured (by each enclosing
	// lambda too, p13) where the lambda appears, by reference or by copy.
	const syntax::LambdaExpr& expr = *frame.expr;
	for (const syntax::LambdaCapture& capture : expr.captures)
	{
		if (capture.kind == TokenKind::KW_THIS)
		{
			if (frame.this_captured || (expr.has_default && expr.capture_default == TokenKind::OP_ASS))
				error(expr.location, "`this` captured twice or after a copy capture-default (5.1.2p8)");
			if (body_ == nullptr || body_->this_type == nullptr)
				error(expr.location, "`this` captured outside a non-static member function (5.1.2p8)");
			capture_this(expr.location);
			frame.this_captured = true;
			continue;
		}
		if (capture.pack)
			error(expr.location, "a capture pack expansion that names no function parameter pack (14.5.3p5)");
		if (capture.by_reference && expr.has_default && expr.capture_default == TokenKind::OP_AMP)
			error(expr.location, "a reference capture after a reference capture-default (5.1.2p8)");
		if (!capture.by_reference && expr.has_default && expr.capture_default == TokenKind::OP_ASS)
			error(expr.location, "a copy capture after a copy capture-default (5.1.2p8)");
		const LookupResult found = unqualified(scope, capture.name, LookupFilter::Ordinary);
		const Entity* entity = found.found() ? found.binding->entity : nullptr;
		const bool automatic = entity != nullptr && found.binding->kind == BindingKind::Parameter
			? true
			: entity != nullptr && found.binding->kind == BindingKind::Variable &&
				!static_cast<const VariableEntity*>(entity)->is_static &&
				!static_cast<const VariableEntity*>(entity)->is_extern &&
				!static_cast<const VariableEntity*>(entity)->is_thread_local &&
				!static_cast<const VariableEntity*>(entity)->member && entity->owner != nullptr &&
				(entity->owner->kind == ScopeKind::Block || entity->owner->kind == ScopeKind::Function);
		if (!automatic || body_ == nullptr)
			error(expr.location, "a capture names no automatic variable of the reaching scope (5.1.2p10)");
		if (!capture_variable(entity, expr.location))
			error(expr.location, "a capture of a variable a local class cannot use (5.1.2p10)");
		add_capture(frame, entity, true, !capture.by_reference, expr.location);
	}
}

void Analyzer::add_capture(LambdaFrame& frame, const Entity* entity, bool written, bool by_copy, uint32_t location)
{
	// The records map the closure's captures to whether each is by copy.
	const EntityPair key = EntityPair{ frame.closure, entity };
	if (captured_.find(key) != nullptr)
	{
		if (written)
			error(location, "an entity captured twice (5.1.2p8)");
		return;
	}
	captured_.insert(key, by_copy);
	frame.captures.push_back(entity);
}

void Analyzer::local_name(const syntax::Expr& expr, ExprFacts* facts, const Entity* entity, Scope* scope)
{
	// A block-scope variable or a parameter named potentially evaluated: in
	// no default argument (8.3.6p7, a constant's, a static or extern one's
	// included). An automatic one of an enclosing function: odr-used by no
	// local class's declarations (9.8p1), captured by a lambda's body that
	// odr-uses it (5.1.2p11-p13); a constant's name is read as its value
	// (3.2p2: no odr-use, no capture).
	if (unevaluated_ != 0)
	{
		if (deferring())
			defer_use(DeferredUse{ DeferredUse::Kind::Local, false, &expr, facts, entity, scope, nullptr, 0 });
		return;
	}
	if (scope == default_argument_scope_ && scope != nullptr)
		error(expr.location, "a local variable or parameter in a default argument (8.3.6p7)");
	const VariableEntity* variable = entity->kind == EntityKind::Variable ? static_cast<const VariableEntity*>(entity)
																		  : nullptr;
	if (!semantic() || template_depth_ != 0 || !InNestedBody(scope, entity->owner))
		return;
	if (variable != nullptr && variable->is_static)
	{
		// An enclosing function's static (6.7p4: one object, not captured):
		// each body between the use and that function's names it.
		for (BodyContext* context = body_; context != nullptr && context->function != nullptr &&
			!(context->function->scope != nullptr && encloses(context->function->scope, entity->owner));
			context = context->enclosing)
			context->function->names_enclosing_statics = true;
		return;
	}
	if (variable != nullptr && (variable->is_extern || variable->is_thread_local))
		return;
	Constant value;
	if (variable != nullptr && (facts->type->cv & kConst) != 0 && IsIntegralOrUnscopedEnum(facts->type) &&
		constant_value(expr, scope, value, false))
	{
		facts->flags |= kStaticConstant;
		facts->value = value.bits;
	}
	else if (!capture_variable(entity, expr.location))
		error(expr.location, "a local class odr-uses an automatic variable of an enclosing function (9.8p1)");
	else
	{
		// p18: a name of an entity captured by copy designates the member of
		// the innermost lambda that copied it, const in a call operator that
		// is not mutable (p5); lambdas inside capture that member.
		for (BodyContext* context = body_; context != nullptr && context->lambda != nullptr;
			context = context->enclosing)
		{
			const bool* by_copy = captured_.find(EntityPair{ context->lambda->closure, entity });
			if (by_copy == nullptr || !*by_copy)
				continue;
			if (!context->lambda->expr->is_mutable && facts->type->kind != TypeKind::Function)
				facts->type = types_.qualified(facts->type, static_cast<uint8_t>(facts->type->cv | kConst));
			break;
		}
	}
}

bool Analyzer::capture_variable(const Entity* entity, uint32_t location)
{
	// p11-p13: an automatic variable of an enclosing function odr-used in
	// a lambda's body is captured by each lambda between the use and the
	// variable's function, implicitly under a reference capture-default,
	// the outermost first; without one it must be captured explicitly.
	// False when a local class's member function is in between, or when
	// the use is in no body of the variable's function or of a lambda in it
	// (a local class's default argument or member initializer, 9.8p1; a
	// default argument, 8.3.6p7).
	SmallList<LambdaFrame*, 8> crossed;
	BodyContext* context = body_;
	bool sealed = false;
	for (; context != nullptr && context->function != nullptr; context = context->enclosing)
	{
		if (context->function->scope != nullptr && encloses(context->function->scope, entity->owner))
			break;
		if (context->lambda == nullptr)
			return false;
		// A templated lambda's specialization: the closure's captures (no
		// more), where the variable's function is not being analyzed.
		if (context->lambda->sealed)
		{
			if (captured_.find(EntityPair{ context->lambda->closure, entity }) == nullptr)
				error(location, "a templated lambda's body names an automatic variable its closure did not capture");
			sealed = true;
			break;
		}
		crossed.push_back(context->lambda);
	}
	if (!sealed && (context == nullptr || context->function == nullptr))
		return false;
	for (uint32_t i = crossed.size(); i-- > 0;)
	{
		LambdaFrame& frame = *crossed[i];
		if (captured_.find(EntityPair{ frame.closure, entity }) != nullptr)
			continue;
		if (!frame.expr->has_default)
			error(location, "a lambda odr-uses an automatic variable it does not capture (5.1.2p11)");
		add_capture(frame, entity, false, frame.expr->capture_default == TokenKind::OP_ASS, location);
	}
	return true;
}

void Analyzer::odr_use_constant_operand(const syntax::Expr& expr)
{
	// 3.2p3: a constant of an enclosing function that a nested body names
	// (read as its value, local_name) is odr-used when it is a potential
	// result of a glvalue no lvalue-to-rvalue conversion reads: the operand
	// of a built-in `&`, of a cast to a reference type, or what a reference
	// binds directly (see odr_use_constants). A lambda's body captures it
	// then (5.1.2p17's example); a local class's cannot (9.8p1).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const ExprFacts* facts = e->facts;
	if (facts == nullptr || facts->category == ValueCategory::PRValue || (facts->flags & kOperatorCall) != 0)
		return;
	if (e->kind == syntax::ExprKind::Conditional)
	{
		const syntax::ConditionalExpr& conditional = static_cast<const syntax::ConditionalExpr&>(*e);
		if (conditional.then_expr != nullptr)
			odr_use_constant_operand(*conditional.then_expr);
		odr_use_constant_operand(*conditional.else_expr);
		return;
	}
	if (e->kind == syntax::ExprKind::Binary && static_cast<const syntax::BinaryExpr*>(e)->op == TokenKind::OP_COMMA)
	{
		odr_use_constant_operand(*static_cast<const syntax::BinaryExpr*>(e)->right);
		return;
	}
	const Entity* entity = facts->entity;
	if (e->kind != syntax::ExprKind::Id || (facts->flags & kStaticConstant) == 0 || entity == nullptr ||
		entity->kind != EntityKind::Variable || entity->owner == nullptr ||
		(entity->owner->kind != ScopeKind::Block && entity->owner->kind != ScopeKind::Function))
		return;
	if (!capture_variable(entity, e->location))
		error(e->location, "a local class odr-uses an automatic variable of an enclosing function (9.8p1)");
}

const Type* Analyzer::captured_type(const Entity* entity, const Type* type)
{
	// p18: decltype((x)) in a lambda's body, x an automatic variable of an
	// enclosing function, is typed as the access to the member x's odr-use
	// would make: the innermost lambda's that captures it by copy (once
	// captured, else by its `=` capture-default; one capturing it by
	// reference refers to the enclosing lambda's, p16), of x's type (a
	// reference to an object's referenced type, p14), const in a call
	// operator that is not mutable (p5). `type` (x's declared type) when no
	// lambda would copy it.
	if (entity->owner == nullptr ||
		(entity->owner->kind != ScopeKind::Block && entity->owner->kind != ScopeKind::Function))
		return type;
	if (entity->kind == EntityKind::Variable)
	{
		const VariableEntity* variable = static_cast<const VariableEntity*>(entity);
		if (variable->is_static || variable->is_extern || variable->is_thread_local || variable->member)
			return type;
	}
	for (BodyContext* context = body_; context != nullptr && context->lambda != nullptr; context = context->enclosing)
	{
		if (context->function->scope != nullptr && encloses(context->function->scope, entity->owner))
			return type;
		const LambdaFrame& frame = *context->lambda;
		const bool* by_copy = captured_.find(EntityPair{ frame.closure, entity });
		if (by_copy == nullptr && !frame.expr->has_default)
			return type;
		if (by_copy != nullptr ? !*by_copy : frame.expr->capture_default != TokenKind::OP_ASS)
			continue;
		const Type* member = type->is_reference() ? type->element : type;
		if (member->kind == TypeKind::Function)
			return type;
		return frame.expr->is_mutable ? member : types_.qualified(member, static_cast<uint8_t>(member->cv | kConst));
	}
	return type;
}

void Analyzer::capture_this(uint32_t location)
{
	// p10-p13: `this` odr-used in a lambda's body (named, or through an
	// implicit member access) is captured by each lambda between the use
	// and the member function; one that captured it had its enclosing
	// lambdas capture it first. Not in an unevaluated operand.
	if (unevaluated_ != 0)
	{
		if (deferring())
			defer_use(
				DeferredUse{ DeferredUse::Kind::This, false, nullptr, nullptr, nullptr, nullptr, nullptr, location });
		return;
	}
	for (BodyContext* context = body_; context != nullptr && context->lambda != nullptr; context = context->enclosing)
	{
		LambdaFrame& frame = *context->lambda;
		if (frame.this_captured)
			return;
		if (!frame.expr->has_default || frame.sealed)
			error(location, "a lambda uses `this` without capturing it (5.1.2p10)");
		frame.this_captured = true;
	}
}

void Analyzer::finish_captures(LambdaFrame& frame)
{
	// p14-p15 in the course's form: a pointer member holding a captured
	// `this` first, then a member per captured entity, in the order
	// captured: a reference bound to it, or (by copy) an object of its type,
	// a reference to an object's referenced type (a reference to a function
	// stays one), a class object's constructor selected as its direct-
	// initialization from the entity's lvalue (p21).
	ClosureFacts* facts = frame.facts;
	ClassEntity* closure = frame.closure;
	if (frame.this_captured)
	{
		VariableEntity* member = new_entity<VariableEntity>(
			EntityKind::Variable, nullptr, closure->scope, body_->this_type);
		member->member = true;
		add_data_member(closure, member, 0, false);
		facts->this_member = member;
	}
	const uint32_t count = static_cast<uint32_t>(frame.captures.size());
	Capture* captures = count != 0 ? static_cast<Capture*>(arena_.allocate(sizeof(Capture) * count, alignof(Capture)))
								   : nullptr;
	for (uint32_t i = 0; i < count; ++i)
	{
		const Entity* entity = frame.captures[i];
		const Type* type = entity->type->is_reference() ? entity->type->element : entity->type;
		const bool* by_copy = captured_.find(EntityPair{ closure, entity });
		const bool copy = by_copy != nullptr && *by_copy && type->kind != TypeKind::Function;
		VariableEntity* member = new_entity<VariableEntity>(
			EntityKind::Variable, nullptr, closure->scope, copy ? type : types_.reference(type, false));
		member->member = true;
		add_data_member(closure, member, 0, false);
		captures[i] = Capture{ entity, member,
			copy ? capture_constructor(closure, type, frame.expr->location) : nullptr };
	}
	facts->captures = captures;
	facts->capture_count = count;
	if (count != 0 || frame.this_captured)
	{
		closure->empty = false;
		closure->non_pod = true;
	}
}

const FunctionEntity* Analyzer::capture_constructor(ClassEntity* closure, const Type* type, uint32_t location)
{
	// p21: a class object captured by copy (an array's elements) is
	// direct-initialized from the entity's lvalue, in the context of the
	// lambda-expression; null for other types.
	const Type* element = type;
	while (element->kind == TypeKind::Array)
		element = element->element;
	ClassEntity* cls = const_cast<ClassEntity*>(ClassOf(element));
	if (cls == nullptr)
		return nullptr;
	if (!require_complete(cls))
		error(location, "a capture by copy of an object of incomplete class type");
	bool deleted = false;
	FunctionEntity* constructor = subobject_copy(cls, CopyKind::CopyConstructor, element->cv, 0, closure, deleted);
	if (constructor == nullptr || deleted)
		error(location, "a capture by copy of a class object without an accessible copy constructor (5.1.2p21)");
	odr_use(constructor, true);
	if (!constructor->nothrow)
		++potentially_throwing_;
	return constructor;
}

void Analyzer::closure_conversion(const ClassEntity* entity, bool conversion_function)
{
	// p6: the closure's public non-virtual non-explicit const conversion
	// function to pointer to function with the function call operator's
	// parameter and return types, returning the address of a function with
	// the operator's effect: the operator's body (captureless, it uses no
	// object) without an implicit object parameter. Declared on the first
	// need of a captureless closure's (its call operator complete then);
	// a direct call needs the function alone.
	const uint32_t index = graph_.closure_index.find(entity);
	if (index == PointerMap::kMissing)
		return;
	ClosureFacts* facts = graph_.closures[index];
	if (!facts->convertible || facts->conversion != nullptr || (facts->invoker != nullptr && !conversion_function))
		return;
	ClassEntity* closure = const_cast<ClassEntity*>(entity);
	const FunctionEntity* call = facts->call;
	const Type* operator_type = call->type->canonical;
	const Type* function_type = types_.function(operator_type->element, operator_type->params,
		operator_type->param_count, operator_type->variadic, 0, RefQualifier::None);
	if (facts->invoker == nullptr)
	{
		// Its name is Clang's (`__invoke`, a reserved identifier no lookup
		// finds: it is declared in no scope), the symbol's; its label the
		// course's (`_FUN`, GCC's), its LowIR spelling.
		if (invoker_key_ == nullptr)
			invoker_key_ = special_key("__invoke");
		FunctionEntity* invoker = new_entity<FunctionEntity>(
			EntityKind::Function, invoker_key_, closure->scope, function_type);
		invoker->label = "_FUN";
		invoker->signature = signature_of(function_type);
		invoker->member = true;
		invoker->is_static = true;
		invoker->is_inline = true;
		invoker->inline_member = true;
		invoker->defined = true;
		invoker->body = call->body;
		invoker->scope = call->scope;
		invoker->parameters = call->parameters;
		invoker->exception_spec = call->exception_spec;
		invoker->nothrow = call->nothrow;
		// It runs the operator's body itself (the course's form), so it
		// has its dynamic-exception-specification too: a call through the
		// function pointer is guarded as the operator's (15.5.2).
		if (const SemanticGraph::ThrowSpecification* thrown = graph_.throw_specification(call))
			graph_.set_throw_specification(invoker, *thrown);
		facts->invoker = invoker;
	}
	if (!conversion_function)
		return;
	const Type* conversion_type = types_.function(
		types_.pointer(function_type), nullptr, 0, false, kConst, RefQualifier::None);
	FunctionEntity* conversion = new_entity<FunctionEntity>(
		EntityKind::Function, conversion_key_, closure->scope, conversion_type);
	conversion->label = "operator <function pointer>";
	conversion->signature = signature_of(conversion_type);
	conversion->member = true;
	conversion->conversion = true;
	conversion->is_inline = true;
	conversion->inline_member = true;
	conversion->nothrow = true;
	Binding* binding = new_binding(BindingKind::Function, conversion_key_, conversion, conversion_type);
	binding->declared_in = closure->scope;
	binding->access = Access::Public;
	names_.declare(closure->scope, binding);
	facts->conversion = conversion;
}

FunctionEntity* Analyzer::closure_call(ClassEntity* closure, const syntax::LambdaExpr& expr, Scope* scope)
{
	// 5.1.2p5: `R operator()(parameters) const` (not const when mutable),
	// R the trailing return type, else (p4) the type the body's one
	// return statement returns, else void; its exception-specification
	// the lambda's.
	syntax::ParameterClause none = syntax::ParameterClause();
	syntax::FunctionQualifier qualifiers[3];
	uint32_t count = 0;
	if (!expr.is_mutable)
	{
		qualifiers[count] = syntax::FunctionQualifier();
		qualifiers[count].kind = syntax::FunctionQualifierKind::Cv;
		qualifiers[count++].token = TokenKind::KW_CONST;
	}
	if (expr.has_throw)
	{
		qualifiers[count] = syntax::FunctionQualifier();
		qualifiers[count].kind = syntax::FunctionQualifierKind::Throw;
		qualifiers[count].location = expr.location;
		qualifiers[count++].throw_types = expr.throw_types;
	}
	if (expr.trailing != nullptr)
	{
		qualifiers[count] = syntax::FunctionQualifier();
		qualifiers[count].kind = syntax::FunctionQualifierKind::TrailingReturn;
		qualifiers[count++].trailing = expr.trailing;
	}
	syntax::DeclaratorSuffix suffix = syntax::DeclaratorSuffix();
	suffix.kind = syntax::DeclaratorSuffixKind::Function;
	suffix.parameters = expr.parameters != nullptr ? expr.parameters : &none;
	suffix.qualifiers = syntax::Slice<syntax::FunctionQualifier>{ qualifiers, count };
	const PendingPrototype saved_prototype = pending_prototype_;
	const Type* type = apply_suffix(suffix,
		expr.trailing != nullptr ? auto_placeholder_ : types_.fundamental(FundamentalType::Void), scope, expr.location,
		closure);
	pending_prototype_ = saved_prototype;
	// A deferred reserved builtin call in its declarator (see
	// deferrable_statement): no call operator.
	if (deferred_reserved_call())
		return nullptr;
	// 10.4p3: no parameter or return type is an abstract class (a deduced
	// one: see deduced_result).
	if (semantic() && template_depth_ == 0)
	{
		const Type* canonical = type->canonical;
		check_abstract_object(canonical->element, expr.location);
		for (uint32_t i = 0; i < canonical->param_count; ++i)
			check_abstract_object(canonical->params[i], expr.location);
	}
	FunctionEntity* call = new_entity<FunctionEntity>(
		EntityKind::Function, operator_key(TokenKind::OP_LPAREN), closure->scope, type);
	call->label = "operator()";
	call->signature = signature_of(type);
	call->member = true;
	call->is_operator = true;
	call->operator_token = TokenKind::OP_LPAREN;
	call->is_inline = true;
	call->inline_member = true;
	call->defined = true;
	call->parameters = expr.parameters;
	// A dynamic-exception-specification is the call operator's as a
	// function's (15.4; throw() allows no exception, 15.4p12).
	call->exception_spec = expr.has_noexcept || expr.has_throw;
	call->nothrow = (expr.has_noexcept &&
						(expr.noexcept_operand == nullptr || explicit_value(*expr.noexcept_operand, scope))) ||
		(expr.has_throw && expr.throw_types.empty());
	if (expr.has_throw)
		record_throw_specification(call, &suffix, scope);
	// GNU noinline, used, pure and const on the lambda-declarator are the
	// call operator's, as GCC reads them (const over pure, see
	// effect_attributes).
	for (const syntax::Attribute* attribute = expr.attributes; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->kind == syntax::AttributeKind::NoInline)
			call->no_inline = true;
		if (attribute->kind == syntax::AttributeKind::Used)
			call->used = true;
		const FunctionEffect effect = attribute->kind == syntax::AttributeKind::Const ? FunctionEffect::ReadNone
			: attribute->kind == syntax::AttributeKind::Pure                          ? FunctionEffect::ReadOnly
																					  : FunctionEffect::Default;
		if (effect > call->effects)
			call->effects = effect;
	}
	// 5.1.2p5, 8.3.6: the parameters' default arguments, which name no
	// local variable (8.3.6p7).
	if (semantic() && expr.parameters != nullptr)
	{
		const Scope* saved_defaults = default_argument_scope_;
		default_argument_scope_ = scope;
		record_default_arguments(call, expr.parameters, scope);
		default_argument_scope_ = saved_defaults;
	}
	Binding* binding = new_binding(BindingKind::Function, call->name, call, type);
	binding->declared_in = closure->scope;
	binding->access = Access::Public;
	names_.declare(closure->scope, binding);
	return call;
}

void Analyzer::number_closure(ClassEntity* closure, ClosureFacts* facts)
{
	// Itanium 5.1.8: a local closure type is numbered among its function's
	// closure types of the same parameter types, in lexical order. One of a
	// class's member-specification (a default member initializer's or a
	// member function's default argument's, typed with the complete class
	// before any definition outside it) is numbered among its class's, and
	// named in the class's scope alike in every unit (the course's form of
	// 5.1.8's data member and default argument contexts); a class template
	// instance's members are typed on demand, in no fixed order. Another
	// outside a function body (a namespace-scope initializer's) is numbered
	// among its unit's and is the unit's own.
	// Objects name a closure of a member's default member initializer or
	// default argument (an instance's member's too: each is typed whole),
	// or of a class template instance's static data member's initializer,
	// in 5.1.8's context, numbered there (ProgramLowering::
	// closure_context_scope).
	// Itanium's lambda-sig counts a variadic lambda's ellipsis as one of its
	// types (z): its closure is numbered apart from those of its parameter
	// types without one (facts->closure_number, the number objects name it
	// by). The course numbers both kinds in one sequence (the class's
	// local_discriminator, its LowIR names: PA20).
	if (body_ == nullptr)
		return;
	const Type* call = graph_.closure_call(closure)->type->canonical;
	const Scope* owner = closure->owner;
	const bool member_specification = body_->function == nullptr && initializing_ == nullptr &&
		owner->kind == ScopeKind::Class && owner->entity != nullptr;
	// The lambda-sig's key, and the one apart of a variadic lambda's (its
	// types and void, which no parameter-type-list holds).
	SmallList<const Type*, 8> key;
	if (member_specification && static_cast<const ClassEntity*>(owner->entity)->templ == nullptr)
		key.push_back(owner->entity->type);
	for (uint32_t i = 0; i < call->param_count; ++i)
		key.push_back(call->params[i]);
	const Type* signature = types_.pack(key.data(), key.size());
	key.push_back(types_.fundamental(FundamentalType::Void));
	const Type* variadic = types_.pack(key.data(), key.size());
	const bool member_initializer = body_->function == nullptr && initializing_ != nullptr &&
		owner->kind == ScopeKind::Class && body_->closure_context != nullptr;
	if ((member_specification || member_initializer) && body_->closure_context != nullptr)
	{
		const ClosureContextKey probe = ClosureContextKey{ body_->closure_context, body_->default_argument,
			call->variadic ? variadic : types_.pack(call->params, call->param_count) };
		uint32_t* count = closure_context_numbers_.find(probe);
		const uint32_t earlier = count == nullptr ? 0 : (*count)++;
		if (count == nullptr)
			closure_context_numbers_.insert(probe, 1);
		facts->context = body_->closure_context;
		facts->default_argument = body_->default_argument;
		facts->context_number = static_cast<uint16_t>(earlier < 0xffff ? earlier : 0xffff);
	}
	facts->member_context = member_specification && static_cast<const ClassEntity*>(owner->entity)->templ == nullptr;
	if (call->variadic)
	{
		closure->local_discriminator = local_number(signature);
		facts->closure_number = local_number(variadic);
		return;
	}
	// One without an ellipsis: the course's number less the earlier
	// variadic closures' of its parameter types.
	closure->local_discriminator = local_number(signature);
	const uint32_t* found = local_numbers_.find(LocalNumberKey{ body_->function, variadic });
	const uint32_t earlier_variadic = found != nullptr ? *found : 0;
	facts->closure_number = static_cast<uint16_t>(
		closure->local_discriminator > earlier_variadic ? closure->local_discriminator - earlier_variadic : 0);
}

const Type* Analyzer::return_deduction(ExprFacts* facts, uint32_t location)
{
	// The result a return statement's operand deduces. A lambda's (5.1.2p4
	// as CWG 1048 revised it, the auto rules GCC and Clang apply in C++11):
	// its type after the array-to-pointer and function-to-pointer
	// conversions, cv-unqualified, a class type's too. A function's
	// placeholder return type (C++14's 7.1.6.4p7): auto deduced from the
	// operand as a variable's declared with that type would be (7.1.6.4p6).
	FunctionEntity* function = body_->function;
	if (graph_.closure_call(static_cast<const ClassEntity*>(function->owner->entity)) == function)
	{
		const Type* result = decayed(facts);
		return IsVoid(result) ? types_.fundamental(FundamentalType::Void) : result->canonical;
	}
	const Type* declared = body_->placeholder;
	TypeList deduced;
	deduced.push_back(nullptr);
	if (IsVoid(facts->type))
	{
		// 7.1.6.4p7 (C++14): a void operand deduces void for a plain auto.
		if (declared->unqualified != auto_placeholder_)
			error(location, "a void return deduces no reference or pointer to auto");
		return types_.fundamental(FundamentalType::Void);
	}
	if (!deduce_argument(declared, facts->type, facts->category, 0, deduced) || deduced[0] == nullptr)
		error(location, "the return statement deduces no return type (7.1.6.4p7)");
	return replaced_auto(declared, deduced[0], 0)->canonical;
}

const Type* Analyzer::deduced_result(ExprFacts* facts, uint32_t location)
{
	// The first return statement deduces the result (a lambda's function
	// call operator's or the function's type takes it); each later one
	// deduces the same (C++14's 7.1.6.4p9, CWG 975 for lambdas). A return
	// without an operand (facts null) deduces void.
	const Type* result = facts != nullptr ? return_deduction(facts, location)
		: body_->placeholder == nullptr || body_->placeholder->unqualified == auto_placeholder_
		? types_.fundamental(FundamentalType::Void)
		: nullptr;
	if (result == nullptr)
		error(location, "a return without a value deduces no reference or pointer to auto");
	// 10.4p3: an abstract class is no return type.
	if (semantic() && template_depth_ == 0)
		check_abstract_object(result, location);
	if (body_->deduced)
	{
		if (result->canonical != body_->result->canonical)
			error(location, "return statements deduce different return types (7.1.6.4p9)");
		return body_->result;
	}
	deduce_function_result(body_->function, result);
	body_->result = result;
	body_->deduced = true;
	return result;
}

void Analyzer::deduce_function_result(FunctionEntity* function, const Type* result)
{
	const Type* old = function->type;
	function->type = types_.function(result, old->params, old->param_count, old->variadic, old->function_cv, old->ref);
	function->signature = signature_of(function->type);
}

bool Analyzer::require_deduced(FunctionEntity* function, uint32_t location)
{
	// A function whose placeholder return type is undeduced where its type
	// is needed: a template specialization's definition is instantiated
	// (C++14's 7.1.6.4p12), else the program is ill-formed (p11).
	if (!holds_auto(function->type->canonical->element))
		return true;
	if (function->instantiated && instantiate_definition(function) && !holds_auto(function->type->canonical->element))
		return true;
	return substitution_error(location, "a function whose return type is not deduced yet (7.1.6.4p11)");
}

} // namespace sema
