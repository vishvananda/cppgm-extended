#include "sema/analyzer.h"

#include <algorithm>
#include <cstring>

#include "lex/identifier_table.h"
#include "lex/number_literal.h"
#include "lex/text_literal.h"
#include "sema/pretty_function.h"

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

} // namespace

bool Analyzer::function_body(FunctionEntity* function, const Type* type, const syntax::ParameterClause* parameters,
	const syntax::FunctionBody& body, Scope* print, Scope* parent)
{
	// A body is not part of an enclosing class's member-specification: a
	// local class in it completes on its own. False when the result it
	// deduces (a lambda's) is undeducible (see BodyContext::undeduced).
	// Claim this body's lambda frame before parameter/type completion can
	// reenter another function body. Other bodies do not consume it.
	LambdaFrame* lambda = pending_lambda_ != nullptr &&
			(pending_lambda_->facts->call == function ||
				(pending_lambda_->sealed && function->primary == pending_lambda_->facts->call))
		? pending_lambda_
		: nullptr;
	if (lambda != nullptr)
		pending_lambda_ = nullptr;
	const uint32_t saved_depth = class_depth_;
	class_depth_ = 0;
	Scope* scope = new_scope(ScopeKind::Function, parent, print, function, function->label);
	function->scope = scope;
	function->body = &body;
	if (parameters != nullptr)
	{
		// The parameters of the definition, with their declared types; an
		// instantiation's parameter is dependent if its pattern's type is
		// (see Binding::dependent), any local function's in an
		// instantiation (a lambda's, a local class member's) is taken as
		// dependent.
		const Type* pattern = nullptr;
		bool all_dependent = false;
		if (function->instantiated)
		{
			const FunctionEntity* source = function;
			for (int depth = 0; depth < 64; ++depth)
			{
				const FunctionEntity* next = source->primary != nullptr && source->primary != source
					? source->primary
					: PatternOf(source);
				if (next == nullptr || next == source)
					break;
				source = next;
			}
			// Parameter adjustment removes an array bound's dependence when
			// the declared array parameter becomes a pointer (8.3.5p5).
			pattern = source != function ? source->type->canonical : nullptr;
			all_dependent = pattern == nullptr || pattern->param_count != type->param_count;
			for (uint32_t i = 0; i < type->param_count && !all_dependent; ++i)
				all_dependent = pattern->params[i]->kind == TypeKind::Expansion;
		}
		else
			all_dependent = names_.limit() != NameLookup::kNoLimit;
		for (uint32_t i = 0; i < type->param_count; ++i)
		{
			const syntax::Name* id = DeclaratorId(parameters->parameters[i]->declarator);
			const Identifier* name = id != nullptr ? id->last().identifier : nullptr;
			Entity* parameter = new_entity<Entity>(EntityKind::Parameter, name, scope, type->params[i]);
			Binding* binding = new_binding(BindingKind::Parameter, name, parameter, type->params[i]);
			binding->dependent = all_dependent || (pattern != nullptr && dependent_type(pattern->params[i]));
			print_in(scope, binding);
			if (name == nullptr)
				continue;
			if (semantic())
				note_nothrow_target(parameter, type->params[i], parameters->parameters[i]->declarator, scope);
			if (names_.find(scope, name) != nullptr)
				error(id->location, "duplicate parameter name");
			declare_in(scope, binding);
		}
	}
	// Semantics mode: the body's context, and where its odr-uses go (an
	// inline member's own list; other definitions are always emitted).
	BodyContext context = BodyContext();
	BodyContext* saved_body = body_;
	FunctionEntity* saved_demanding = demanding_;
	const uint32_t saved_epoch = current_epoch_;
	if (semantic() && template_depth_ == 0)
	{
		require_definition_types(type, body.body != nullptr ? body.body->location : 0);
		context.function = function;
		context.result = type->element;
		context.enclosing = saved_body;
		context.first_label = labels_.size();
		context.first_goto = gotos_.size();
		context.deduce_result = function == deducing_call_ || holds_auto(type->element);
		context.placeholder = function != deducing_call_ && holds_auto(type->element) ? type->element : nullptr;
		// A lambda's body names no closure object (5.1.2p7: `this` is the
		// enclosing member function's, usable when the lambda may capture
		// it, p10: under a capture-default or written in its capture).
		// A templated lambda's call operator specialization's (its template
		// the closure's call) has its closure's captures; `this` the captured
		// one's type.
		context.lambda = lambda;
		if (context.lambda != nullptr && context.lambda->sealed)
		{
			if (context.lambda->facts->this_member != nullptr)
				context.this_type = context.lambda->facts->this_member->type;
		}
		else if (context.lambda != nullptr)
		{
			if (saved_body != nullptr && (context.lambda->expr->has_default || context.lambda->this_captured))
				context.this_type = saved_body->this_type;
		}
		else if (function->member && !function->is_static && function->owner->entity != nullptr &&
			graph_.closure_call(static_cast<const ClassEntity*>(function->owner->entity)) != function)
			context.this_type = types_.pointer(types_.qualified(function->owner->entity->type, type->function_cv));
		body_ = &context;
		if (function->inline_member)
		{
			demanding_ = function;
			current_epoch_ = ++demand_epoch_;
		}
		else
		{
			demanding_ = nullptr;
			current_epoch_ = root_epoch_;
		}
	}
	else if (semantic())
	{
		context.function = function;
		context.enclosing = saved_body;
		context.first_label = labels_.size();
		context.first_goto = gotos_.size();
		context.pattern = true;
		// A member template's nondependent local initializers may be typed
		// in its definition. They have the same implicit object type as its
		// instantiated body; a closure's this is handled by its captures.
		if (function->member && !function->is_static && function->owner->entity != nullptr &&
			graph_.closure_call(static_cast<const ClassEntity*>(function->owner->entity)) != function)
			context.this_type = types_.pointer(types_.qualified(function->owner->entity->type, type->function_cv));
		body_ = &context;
	}
	const uint32_t body_location = body.body != nullptr ? body.body->location : 0;
	if (semantic())
		check_constexpr_function(function, type, body_location);
	// 7.1.5p3-p4: a constexpr function's or constructor's function-body is
	// a compound-statement, not a function-try-block.
	if (function->is_constexpr && body.try_block)
		error(body_location, "a constexpr function with a function-try-block");
	// An instance's member template, or a member (or a friend defined in
	// the class) of its member class template, shares its class template's
	// body, whose names that template's definition bound (14.6p10): its
	// specializations analyze their copies; nothing is walked here (the
	// instance's parameter clause has its class parameter packs expanded,
	// which the shared body's names do not match).
	const Scope* home = function->owner->kind == ScopeKind::Class ? function->owner : print;
	const bool shared = template_depth_ != 0 && (function->templated || templated_class(home)) && WithinInstance(home);
	if (!shared && function->constructor && body_ == &context && !context.pattern && function->owner->entity != nullptr)
	{
		// 12.6.2: the constructor's subobject initializations, with its
		// mem-initializers typed where the parameters are visible.
		ClassEntity* cls = static_cast<ClassEntity*>(function->owner->entity);
		bool planned = true;
		if (deferring_reserved_ != 0 && body.ctor_initializer != nullptr)
			planned = deferrable_plan(function, cls, *body.ctor_initializer, scope, body_location);
		else
			constructor_plan(function, cls, body.ctor_initializer, scope, body_location);
		if (planned)
		{
			check_constexpr_constructor(function, body_location);
			if (function->delegate == nullptr)
				check_subobject_destruction(cls, scope, body_location);
		}
	}
	else if (!shared && function->name == destructor_key_ && function->member && body_ == &context &&
		!context.pattern && function->owner->entity != nullptr)
		check_subobject_destruction(static_cast<ClassEntity*>(function->owner->entity), scope, body_location);
	else if (!shared && body.ctor_initializer != nullptr)
	{
		for (const syntax::MemInitializer& initializer : body.ctor_initializer->initializers)
		{
			for (const syntax::Expr* argument : initializer.arguments)
				expression(argument, scope);
			if (initializer.list != nullptr)
				expression_walk(initializer.list, scope);
		}
	}
	if (body.body != nullptr && !shared)
	{
		compound(*body.body, scope, true);
		// A hosted wrapper's reserved builtin call in a context with no
		// full-expression of its own above (see define_function): the rest
		// of the compound statement was skipped (see deferrable_statement)
		// and is deferred with its emission.
		if (deferring_reserved_ != 0 && deferred_reserved_call())
		{
			consume_deferred_call();
			body_ = &context;
			context.undeduced = true;
			function->unanalyzed_body = true;
		}
		if (body.try_block)
		{
			context.constructor_handlers = function->constructor;
			handlers(body.try_block->handlers, scope);
			context.constructor_handlers = false;
		}
	}
	if (body_ == &context)
		check_gotos(context);
	// A placeholder return type no return statement deduced is void
	// (C++14's 7.1.6.4p9: as `return;`). A deferred return statement makes
	// it undeducible: the placeholder stays, its uses errors (p11).
	if (body_ == &context && context.undeduced && context.placeholder != nullptr)
	{
		if (context.deduced)
			deduce_function_result(function, context.placeholder);
	}
	else if (body_ == &context && context.placeholder != nullptr && !context.deduced && body.body != nullptr && !shared)
		deduced_result(nullptr, body_location);
	const bool deducible = !context.undeduced || !context.deduce_result;
	body_ = saved_body;
	demanding_ = saved_demanding;
	current_epoch_ = saved_epoch;
	class_depth_ = saved_depth;
	return deducible;
}

void Analyzer::compound(const syntax::CompoundStmt& body, Scope* scope, bool guarded, bool handles)
{
	Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
	block->guarded = guarded;
	block->handles = handles;
	block->selection_init = body.selection_init;
	// The block's variables leave scope at its end (6.7p3 counts those a
	// later label could bypass).
	SwitchContext* context = semantic() && body_ != nullptr ? body_->switch_statement : nullptr;
	const uint32_t initialized = context != nullptr ? context->initialized : 0;
	for (const syntax::Stmt* item : body.items)
		statement(*item, block);
	if (context != nullptr)
		context->initialized = initialized;
}

void Analyzer::substatement(const syntax::Stmt& stmt, Scope* scope)
{
	// 6.4p1, 6.5p2: a substatement is a block scope of its own. A
	// selection statement with an init-statement is one statement: its
	// init-statement's block is no outermost block of this substatement
	// (its names may hide the enclosing construct's).
	if (stmt.kind == syntax::StmtKind::Compound)
	{
		compound(static_cast<const syntax::CompoundStmt&>(stmt), scope, !stmt.selection_init);
		return;
	}
	Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
	block->guarded = true;
	statement(stmt, block);
}

bool Analyzer::deferrable_expression(const syntax::Expr* expr, Scope* scope)
{
	// A selection or iteration statement's own full-expression (its
	// condition, a for's init-statement and increment, a do's condition)
	// calling a reserved builtin a hosted wrapper's body lacks (see
	// deferrable_statement): only that full-expression is deferred, its
	// names checked; the substatements are typed (their return statements
	// deduce). False then.
	if (deferring_reserved_ == 0 || body_ == nullptr || body_->pattern || expr == nullptr)
	{
		expression(expr, scope);
		return true;
	}
	// One pending from an earlier part of the statement: the statement is
	// deferred whole (see deferrable_statement); nothing more is typed.
	if (deferred_reserved_call())
		return false;
	BodyContext* const body = body_;
	const uint32_t unevaluated = unevaluated_;
	expression(expr, scope);
	if (!deferred_reserved_call())
		return true;
	consume_deferred_call();
	body_ = body;
	unevaluated_ = unevaluated;
	body->function->unanalyzed_body = true;
	body->pattern = true;
	expression(expr, scope);
	body->pattern = false;
	return false;
}

bool Analyzer::condition(const syntax::Condition& condition, Scope* scope)
{
	// False when the condition's expression is deferred (a declaration's
	// initializer is deferred on its own, its variable typed).
	if (condition.declaration == nullptr)
		return deferrable_expression(condition.expression, scope);
	// A deferral pending from an earlier part of the statement (see
	// deferrable_expression).
	if (deferring_reserved_ != 0 && deferred_reserved_call())
		return false;
	const syntax::ConditionDeclaration& declaration = *condition.declaration;
	Specifiers specifiers;
	this->specifiers(declaration.specifiers, scope, specifiers, false);
	if (deferred_reserved_call())
		return false;
	if (specifiers.type == nullptr)
		error(declaration.declarator->location, "condition declaration without a type");
	syntax::InitDeclarator init;
	init.declarator = declaration.declarator;
	init.initializer = declaration.initializer;
	const Type* saved_auto = auto_deduced_;
	auto_deduced_ = nullptr;
	declare_declarator(specifiers, init, scope, declaration.declarator->location);
	auto_deduced_ = saved_auto;
	declaration.declared = init.declared;
	// Its auto type's initializer deferred (see declare_declarator): the
	// statement is deferred whole.
	return !deferred_reserved_call();
}

void Analyzer::handlers(const syntax::Slice<syntax::Handler>& list, Scope* scope)
{
	// 15.3p5: a `...` handler is its try block's last.
	bool catch_all = false;
	for (const syntax::Handler& handler : list)
	{
		if (catch_all)
			error(handler.body->location, "a handler after a `...` handler (15.3p5)");
		catch_all = handler.exception.ellipsis;
		Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
		block->handles = true;
		const syntax::ExceptionDeclaration& exception = handler.exception;
		if (!exception.ellipsis)
		{
			Specifiers specifiers;
			this->specifiers(exception.specifiers, block, specifiers, false);
			// A deferred reserved builtin call (see deferrable_statement).
			if (deferred_reserved_call())
				return;
			if (specifiers.type == nullptr || specifiers.type == auto_placeholder_)
				error(handler.body->location, "exception declaration without a type");
			const DeclaratorInfo info = declarator(
				exception.declarator, specifiers.type, block, handler.body->location);
			if (semantic() && template_depth_ == 0 && body_ != nullptr && !body_->pattern)
				exception_declaration(exception, info, block, handler.body->location);
			else if (info.name != nullptr)
			{
				const Identifier* name = info.name->last().identifier;
				Entity* variable = new_entity<VariableEntity>(EntityKind::Variable, name, block, info.type);
				Binding* binding = new_binding(BindingKind::Variable, name, variable, info.type);
				print_in(block, binding);
				declare_in(block, binding);
				// Dependent only if its type is (14.6.2.2p3).
				binding->dependent = dependent_type(info.type);
			}
		}
		compound(*handler.body, block, true);
	}
}

void Analyzer::statement(const syntax::Stmt& stmt, Scope* scope)
{
	check_stack(stmt.location);
	// Semantics mode checks the statement rules too (inside a template
	// nothing is typed until instantiation). A selection or iteration
	// statement's condition and substatement variables end with it.
	BodyContext* body = semantic() ? body_ : nullptr;
	SwitchContext* context = body != nullptr ? body->switch_statement : nullptr;
	const uint32_t initialized = context != nullptr ? context->initialized : 0;
	if (deferring_reserved_ != 0 && body != nullptr)
	{
		// A deferral no statement consumed (see function_body) skips the
		// rest of the body.
		if (deferred_reserved_call())
			return;
		if (!body->pattern)
			deferrable_statement(stmt, scope, body);
		else
			scoped_statement(stmt, scope, body);
	}
	else
		scoped_statement(stmt, scope, body);
	if (context != nullptr && syntax::ChosenReading(stmt).kind != syntax::StmtKind::Declaration &&
		stmt.kind != syntax::StmtKind::Case && stmt.kind != syntax::StmtKind::Default &&
		stmt.kind != syntax::StmtKind::Labeled)
		context->initialized = initialized;
}

void Analyzer::deferrable_statement(const syntax::Stmt& stmt, Scope* scope, BodyContext* body)
{
	// A hosted wrapper's statement (see define_function) calling a reserved
	// builtin this compiler lacks: analyzed again as a template's is, its
	// names checked (a return statement in it leaves a deduced result
	// undeducible). A declaration's initializer is caught on its own
	// (typed_initializer), the variable declared, as are a selection or
	// iteration statement's own full-expressions (deferrable_expression),
	// its substatements typed; a condition's scope is the statement's, so
	// the first attempt's declarations are not seen again. The function (a
	// lambda's call operator in the wrapper too) is never emitted.
	const uint32_t unevaluated = unevaluated_;
	scoped_statement(stmt, scope, body);
	if (!deferred_reserved_call())
		return;
	consume_deferred_call();
	unevaluated_ = unevaluated;
	body->function->unanalyzed_body = true;
	body->pattern = true;
	scoped_statement(stmt, scope, body);
	// Analyzed again, a declaration may defer once more (an auto variable's
	// lambda whose deferred return statement leaves its result
	// undeducible): the body's remaining statements are skipped then, and
	// its pattern stays (see function_body).
	if (!deferred_reserved_call())
		body->pattern = false;
}

bool Analyzer::deferrable_plan(FunctionEntity* constructor, ClassEntity* entity, const syntax::CtorInitializer& list,
	Scope* scope, uint32_t location)
{
	// A hosted wrapper's constructor (see define_function) whose
	// mem-initializer calls a reserved builtin this compiler lacks: the
	// ctor-initializer's names are checked as a template's are, and the
	// plan is left unfinished (the constructor is never emitted; a constant
	// evaluation fails at the untyped mem-initializer, which precedes the
	// unplanned subobjects). False then.
	BodyContext* const body = body_;
	Scope* const saved_scope = expression_scope_;
	const ClassEntity* const saved_object = base_object_;
	const syntax::Initializer* const saved_initializing = initializing_;
	const uint32_t unevaluated = unevaluated_;
	constructor_plan(constructor, entity, &list, scope, location);
	if (!deferred_reserved_call())
		return true;
	consume_deferred_call();
	body_ = body;
	expression_scope_ = saved_scope;
	base_object_ = saved_object;
	initializing_ = saved_initializing;
	unevaluated_ = unevaluated;
	body_->pattern = true;
	for (const syntax::MemInitializer& initializer : list.initializers)
	{
		for (const syntax::Expr* argument : initializer.arguments)
			expression(argument, scope);
		if (initializer.list != nullptr)
			expression_walk(initializer.list, scope);
	}
	body_->pattern = false;
	return false;
}

void Analyzer::scoped_statement(const syntax::Stmt& stmt, Scope* scope, BodyContext* body)
{
	switch (stmt.kind)
	{
	case syntax::StmtKind::Compound:
		return compound(static_cast<const syntax::CompoundStmt&>(stmt), scope, false);
	case syntax::StmtKind::Expression:
		return expression(static_cast<const syntax::ExpressionStmt&>(stmt).expression, scope);
	case syntax::StmtKind::Asm:
		return asm_statement(static_cast<const syntax::AsmStmt&>(stmt), scope);
	case syntax::StmtKind::Declaration:
		return declaration(*static_cast<const syntax::DeclarationStmt&>(stmt).declaration, scope);
	case syntax::StmtKind::If:
	{
		const syntax::IfStmt& if_stmt = static_cast<const syntax::IfStmt&>(stmt);
		Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
		// After an init-statement the condition may not redeclare its names
		// (see check_parameter_redeclaration).
		block->guarded = scope->selection_init;
		const bool typed = condition(if_stmt.condition, block);
		if (body != nullptr && !body->pattern && typed)
		{
			condition_to_bool(if_stmt.condition);
			// `if constexpr`: a constant condition selects the branch analyzed
			// (instantiated); the other is discarded.
			if (if_stmt.is_constexpr)
			{
				if (if_stmt.condition.declaration != nullptr)
					error(stmt.location, "`if constexpr` with a condition declaration");
				Constant value;
				converted_constant(*if_stmt.condition.expression, value, true);
				if_stmt.constexpr_value = value.bits != 0 ? 2 : 1;
			}
		}
		if (const syntax::Stmt* branch = syntax::LiveThen(if_stmt))
			substatement(*branch, block);
		if (const syntax::Stmt* branch = syntax::LiveElse(if_stmt))
			substatement(*branch, block);
		return;
	}
	case syntax::StmtKind::Switch:
	case syntax::StmtKind::While:
	{
		const syntax::ConditionalLoopStmt& loop = static_cast<const syntax::ConditionalLoopStmt&>(stmt);
		Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
		block->guarded = scope->selection_init;
		const bool typed = condition(loop.condition, block);
		if (body == nullptr)
		{
			substatement(*loop.body, block);
			return;
		}
		if (stmt.kind == syntax::StmtKind::While)
		{
			if (!body->pattern && typed)
				condition_to_bool(loop.condition);
			++body->loops;
			substatement(*loop.body, block);
			--body->loops;
			return;
		}
		SwitchContext context = SwitchContext();
		// (Its case labels are a template's when the condition is deferred.)
		context.type = body->pattern || !typed ? nullptr : switch_condition(loop.condition);
		context.first_value = case_values_.size();
		context.handled = body->handled;
		SwitchContext* saved = body->switch_statement;
		body->switch_statement = &context;
		substatement(*loop.body, block);
		body->switch_statement = saved;
		// 6.4.2p2: no two case values equal after conversion (checked once
		// all are known, in O(n log n)); nested switches' values were
		// pushed and popped above this one's.
		const auto first = case_values_.begin() + static_cast<std::ptrdiff_t>(context.first_value);
		std::sort(first, case_values_.end());
		if (std::adjacent_find(first, case_values_.end()) != case_values_.end())
			error(stmt.location, "duplicate case value");
		case_values_.resize(context.first_value);
		return;
	}
	case syntax::StmtKind::Do:
	{
		const syntax::DoStmt& loop = static_cast<const syntax::DoStmt&>(stmt);
		Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
		if (body != nullptr)
			++body->loops;
		substatement(*loop.body, block);
		if (body != nullptr)
			--body->loops;
		if (deferrable_expression(loop.condition, block) && body != nullptr && !body->pattern)
			convert_to_bool(*loop.condition);
		return;
	}
	case syntax::StmtKind::For:
	{
		const syntax::ForStmt& loop = static_cast<const syntax::ForStmt&>(stmt);
		Scope* block = new_scope(ScopeKind::Block, scope, scope, nullptr, nullptr);
		if (loop.init_declaration != nullptr)
			declaration(*loop.init_declaration, block);
		else
			deferrable_expression(loop.init_expression, block);
		if (loop.condition != nullptr && condition(*loop.condition, block) && body != nullptr && !body->pattern)
			condition_to_bool(*loop.condition);
		deferrable_expression(loop.iteration, block);
		if (body != nullptr)
			++body->loops;
		substatement(*loop.body, block);
		if (body != nullptr)
			--body->loops;
		return;
	}
	case syntax::StmtKind::RangeFor:
		return range_for(static_cast<const syntax::RangeForStmt&>(stmt), scope);
	case syntax::StmtKind::Case:
	{
		const syntax::CaseStmt& case_stmt = static_cast<const syntax::CaseStmt&>(stmt);
		if (body != nullptr && !body->pattern &&
			(body->switch_statement == nullptr || body->switch_statement->type != nullptr))
			case_label(case_stmt, scope);
		else
		{
			// A template's case label: where it may appear, its value later
			// (a deferred switch condition's: never converted).
			if (body != nullptr && body->switch_statement == nullptr)
				error(stmt.location, "case label outside a switch statement");
			if (body != nullptr && body->switch_statement->initialized != 0)
				error(stmt.location, "case label bypasses a variable's initialization");
			if (body != nullptr && body->switch_statement->handled != body->handled)
				error(stmt.location,
					"case label inside a try block, handler (15p2) or statement expression the switch encloses");
			expression(case_stmt.value, scope);
		}
		return statement(*case_stmt.body, scope);
	}
	case syntax::StmtKind::Default:
		if (body != nullptr)
		{
			// 6.4.2p2-p3: one default label, inside a switch.
			if (body->switch_statement == nullptr)
				error(stmt.location, "default label outside a switch statement");
			if (body->switch_statement->has_default)
				error(stmt.location, "duplicate default label");
			if (body->switch_statement->initialized != 0)
				error(stmt.location, "default label bypasses a variable's initialization");
			if (body->switch_statement->handled != body->handled)
				error(stmt.location,
					"default label inside a try block, handler (15p2) or statement expression the switch encloses");
			body->switch_statement->has_default = true;
		}
		return statement(*static_cast<const syntax::LabeledStmt&>(stmt).body, scope);
	case syntax::StmtKind::Labeled:
		if (body != nullptr)
			label_statement(static_cast<const syntax::LabeledStmt&>(stmt), scope);
		return statement(*static_cast<const syntax::LabeledStmt&>(stmt).body, scope);
	case syntax::StmtKind::Return:
		if (body != nullptr && body->constructor_handlers)
			error(stmt.location, "a return statement in a handler of a constructor's function-try-block (15.3p14)");
		if (body != nullptr && !body->pattern)
			return return_statement(static_cast<const syntax::ValueStmt&>(stmt), scope);
		// (A deduced result a deferred one leaves undeducible.)
		if (body != nullptr && body->deduce_result)
			body->undeduced = true;
		return expression(static_cast<const syntax::ValueStmt&>(stmt).value, scope);
	case syntax::StmtKind::Throw:
	{
		const syntax::Expr* operand = static_cast<const syntax::ValueStmt&>(stmt).value;
		if (body == nullptr || body->pattern || operand == nullptr)
			return expression(operand, scope);
		throw_operand(*operand, scope, stmt.location);
		if (deferred_reserved_call())
			return;
		++potentially_throwing_;
		return;
	}
	case syntax::StmtKind::Try:
		return try_statement(static_cast<const syntax::TryStmt&>(stmt), scope);
	case syntax::StmtKind::Ambiguous:
	{
		const syntax::AmbiguousStmt& ambiguous = static_cast<const syntax::AmbiguousStmt&>(stmt);
		ambiguous.chosen = ambiguous_reading(ambiguous, scope);
		return scoped_statement(ambiguous.reading(), scope, body);
	}
	case syntax::StmtKind::Break:
		// 6.6.1p1, 6.6.2p1.
		if (body != nullptr && body->loops == 0 && body->switch_statement == nullptr)
			error(stmt.location, "break outside a loop or switch statement");
		return;
	case syntax::StmtKind::Continue:
		if (body != nullptr && body->loops == 0)
			error(stmt.location, "continue outside a loop");
		return;
	case syntax::StmtKind::Goto:
		if (body != nullptr)
			gotos_.push_back(PendingGoto{
				static_cast<const syntax::GotoStmt&>(stmt).label, stmt.location, scope, scope->initialized });
		return;
	}
}

uint8_t Analyzer::ambiguous_reading(const syntax::AmbiguousStmt& stmt, Scope* scope)
{
	// The name's component is a member of the specialization its prefix
	// selects (14.5.5.1): the reading that took it with its category there
	// (a template, a type, or a value; the first when none did, which then
	// fails). A prefix that depends on template parameters (the parser took
	// it for a known one) names neither a type nor a template (14.6p2,
	// 14.2p4): the reading that took it for a value, in the template
	// definition and each of its instantiations.
	syntax::ReadingCategory category = syntax::ReadingCategory::Value;
	if (!stmt.dependent)
	{
		syntax::Name prefix = *stmt.name;
		prefix.components.size = stmt.component + 1;
		const syntax::NameComponent& member = prefix.last();
		if (prefix.components.size < 2 || member.kind != syntax::NameComponentKind::Identifier)
			return 0;
		const NamePrefix found = name_prefix(prefix, scope);
		if (failed() || (found.dependent == nullptr && found.scope == nullptr))
			return 0;
		if (found.dependent != nullptr)
			stmt.dependent = true;
		else
		{
			const LookupResult result = lookup_in(found, member.identifier, LookupFilter::Ordinary);
			if (!result.found())
				return 0;
			bool is_template = IsClassTemplate(result.binding);
			for (const FunctionList* node = result.binding->kind == BindingKind::Function ? result.functions : nullptr;
				node != nullptr && !is_template; node = node->next)
				is_template = static_cast<const FunctionEntity*>(node->binding->entity)->templated;
			category = is_template ? syntax::ReadingCategory::Template
				: result.binding->kind == BindingKind::Type || result.binding->kind == BindingKind::TypeAlias
				? syntax::ReadingCategory::Type
				: syntax::ReadingCategory::Value;
		}
	}
	for (uint8_t i = 0; i < stmt.count; ++i)
	{
		if (stmt.categories[i] == category)
			return i;
	}
	return 0;
}

void Analyzer::label_statement(const syntax::LabeledStmt& stmt, Scope* scope)
{
	// 6.1p1: a label's scope is its function (checked with the gotos).
	labels_.push_back(PendingLabel{ stmt.label, stmt.location, scope, scope->initialized });
}

bool Analyzer::bypasses_initialization(const PendingGoto& jump, const PendingLabel& label)
{
	// 6.7p3: the jump bypasses a declaration when one in scope at the label
	// is not in scope at the goto. A block's declarations before a point
	// are the point's own count in its block, or the count a nested block
	// on its chain recorded at entry. Blocks shared by both chains compare
	// their counts; a block only the label is in counts from zero. 15p2: no
	// block only the label is in is a try block's or a handler's.
	goto_chain_.clear();
	label_chain_.clear();
	for (const Scope* s = jump.scope; s != nullptr && s->kind == ScopeKind::Block; s = s->parent)
		goto_chain_.push_back(s);
	for (const Scope* s = label.scope; s != nullptr && s->kind == ScopeKind::Block; s = s->parent)
		label_chain_.push_back(s);
	size_t g = goto_chain_.size(), l = label_chain_.size();
	while (g > 0 && l > 0 && goto_chain_[g - 1] == label_chain_[l - 1])
	{
		--g;
		--l;
	}
	// label_chain_[l] (if any) is the innermost block both are in.
	for (size_t i = 0; i < l; ++i)
	{
		if (label_chain_[i]->handles)
			error(jump.location, "goto into a try block, handler (15p2) or statement expression");
	}
	for (size_t i = label_chain_.size(); i-- > 0;)
	{
		const uint32_t at_label = i == 0 ? label.initialized : label_chain_[i - 1]->initialized_at_entry;
		uint32_t at_goto = 0;
		if (i >= l)
		{
			const size_t shared = g + (i - l);
			at_goto = shared == 0 ? jump.initialized : goto_chain_[shared - 1]->initialized_at_entry;
		}
		if (at_label > at_goto)
			return true;
	}
	return false;
}

void Analyzer::check_gotos(const BodyContext& body)
{
	// 6.1p1: a name labels one statement of the function; 6.6.4p1: each
	// goto names one of them. The function's labels are sorted by name
	// (interned identity, then position) once, so each goto is a binary
	// search; the duplicate reported is the earliest in the source.
	const auto first = labels_.begin() + static_cast<std::ptrdiff_t>(body.first_label);
	const auto by_name = [](const PendingLabel& a, const PendingLabel& b) { return a.label->id < b.label->id; };
	std::sort(first, labels_.end(), [](const PendingLabel& a, const PendingLabel& b) {
		return a.label->id != b.label->id ? a.label->id < b.label->id : a.location < b.location;
	});
	const PendingLabel* duplicate = nullptr;
	for (auto label = first; label != labels_.end(); ++label)
	{
		if (label != first && (label - 1)->label == label->label &&
			(duplicate == nullptr || label->location < duplicate->location))
			duplicate = &*label;
	}
	if (duplicate != nullptr)
		error(duplicate->location, "duplicate label");
	for (size_t g = body.first_goto; g < gotos_.size(); ++g)
	{
		const PendingLabel key{ gotos_[g].label, 0, nullptr, 0 };
		const auto found = std::lower_bound(first, labels_.end(), key, by_name);
		if (found == labels_.end() || found->label != gotos_[g].label)
			error(gotos_[g].location, "goto names no label of the function");
		if (bypasses_initialization(gotos_[g], *found))
			error(gotos_[g].location, "goto bypasses a variable's initialization");
	}
	labels_.resize(body.first_label);
	gotos_.resize(body.first_goto);
}

void Analyzer::condition_to_bool(const syntax::Condition& condition)
{
	// 6.4p4: the condition's value contextually converted to bool.
	if (condition.declaration == nullptr)
	{
		convert_to_bool(*condition.expression);
		return;
	}
	const Binding* declared = condition.declaration->declared;
	const Type* type = declared->entity->type;
	Conversion conversion = Conversion();
	const Operand operand = Operand{ type->is_reference() ? type->element : type, ValueCategory::LValue, 0 };
	const Type* boolean = types_.fundamental(FundamentalType::Bool);
	if (!conversions_.implicit(operand, boolean, true, conversion))
	{
		// A class object by a conversion function (explicit ones too: the
		// conversion is direct-initialization, 4p3).
		ExprFacts facts = ExprFacts();
		facts.type = operand.type;
		facts.category = ValueCategory::LValue;
		if (!user_conversion(&facts, boolean, conversion, true))
			error(condition.declaration->declarator->location, "condition is not contextually convertible to bool");
		apply_user_conversion(&facts, conversion, condition.declaration->declarator->location);
	}
	condition.declaration->conversion = intern_conversion(conversion);
}

namespace {

bool IsIntegralOrEnumeration(const Type* type)
{
	return IsIntegralType(type) || type->kind == TypeKind::Enum;
}

} // namespace

const Type* Analyzer::switch_condition(const syntax::Condition& condition)
{
	// 6.4.2p2: an integral or enumeration condition, promoted.
	const Type* type = nullptr;
	const FunctionEntity* converter = nullptr;
	if (condition.declaration == nullptr)
	{
		// A class condition by its single conversion function to an
		// integral or enumeration type.
		const ClassEntity* cls = ClassOf(facts_of(*condition.expression)->type);
		if (cls != nullptr && (converter = unique_conversion(cls, IsIntegralOrEnumeration)) != nullptr)
			convert_class_operand(*condition.expression, converter);
		type = decayed(facts_of(*condition.expression));
	}
	else
	{
		type = condition.declaration->declared->entity->type;
		if (type->is_reference())
			type = type->element;
		type = type->unqualified;
		const ClassEntity* cls = ClassOf(type);
		if (cls != nullptr && (converter = unique_conversion(cls, IsIntegralOrEnumeration)) != nullptr)
		{
			ValueCategory category;
			type = result_category(converter->type->canonical->element, category)->unqualified;
		}
	}
	if (!IsIntegralType(type) && type->kind != TypeKind::Enum)
		error(condition.declaration != nullptr ? condition.declaration->declarator->location
											   : condition.expression->location,
			"switch condition is not of integral or enumeration type");
	const Type* result = promoted(type);
	if (condition.declaration == nullptr)
		convert_operand(*condition.expression, result);
	else if (converter != nullptr)
	{
		// The declared object's conversion function, then the promotion.
		ExprFacts facts = ExprFacts();
		facts.type = condition.declaration->declared->entity->type->is_reference()
			? condition.declaration->declared->entity->type->element
			: condition.declaration->declared->entity->type;
		facts.category = ValueCategory::LValue;
		Conversion conversion = Conversion();
		if (!user_conversion(&facts, result, conversion, false))
			error(
				condition.declaration->declarator->location, "switch condition is not of integral or enumeration type");
		apply_user_conversion(&facts, conversion, condition.declaration->declarator->location);
		condition.declaration->conversion = intern_conversion(conversion);
	}
	else
	{
		Conversion conversion = Conversion();
		const Operand operand = Operand{ type, ValueCategory::LValue, 0 };
		if (conversions_.implicit(operand, result, false, conversion))
			condition.declaration->conversion = intern_conversion(conversion);
	}
	return result;
}

void Analyzer::case_label(const syntax::CaseStmt& stmt, Scope* scope)
{
	// 6.4.2p2: a converted constant expression of the promoted condition
	// type, distinct among the switch's labels.
	SwitchContext* context = body_->switch_statement;
	if (context == nullptr)
		error(stmt.location, "case label outside a switch statement");
	if (context->initialized != 0)
		error(stmt.location, "case label bypasses a variable's initialization");
	if (context->handled != body_->handled)
		error(
			stmt.location, "case label inside a try block, handler (15p2) or statement expression the switch encloses");
	typed_value(*stmt.value, scope);
	// A deferred reserved builtin call (see deferrable_statement).
	if (deferred_reserved_call())
		return;
	// A class operand converts by the conversion function the promoted
	// type's copy-initialization selects (5.19p3).
	convert_to_scalar(*stmt.value, context->type, false);
	Constant value;
	constant_value(*stmt.value, scope, value, true);
	convert_operand(*stmt.value, context->type);
	IntegralKind kind;
	IntegralKindOf(context->type, kind);
	// 5.19p3: the conversion of a converted constant expression is not
	// narrowing (8.5.4p7): the value is representable in the promoted type.
	if (value.type->kind != TypeKind::Enum && !Representable(value, kind))
		error(stmt.value->location, "case value narrowed by conversion to the switch condition's type");
	else if (!HeldAsWideConstant(value, kind))
		error(stmt.value->location, "a 128-bit case value beyond the 64-bit constants evaluated");
	stmt.converted = ConvertIntegral(value.bits, kind);
	case_values_.push_back(stmt.converted);
}

void Analyzer::return_statement(const syntax::ValueStmt& stmt, Scope* scope)
{
	// 6.6.3: a value copy-initializes the result; a void function returns
	// none (or a void expression), a non-void one needs one.
	const Type* result = body_->result;
	bool void_result = result == nullptr || IsVoid(result) || body_->function == nullptr ||
		body_->function->constructor || body_->function->name == destructor_key_;
	if (stmt.value == nullptr)
	{
		if (body_->deduce_result)
			result = deduced_result(nullptr, stmt.location);
		void_result = void_result || IsVoid(result);
		if (!void_result)
			error(stmt.location, "return without a value in a function returning a value");
		return;
	}
	if (stmt.value->kind == syntax::ExprKind::BracedInit)
	{
		if (body_->deduce_result)
			error(stmt.value->location, "a braced-init-list deduces no return type (7.1.6.4p7)");
		// 6.6.3p2: the braced-init-list copy-list-initializes the result.
		if (void_result || result->kind == TypeKind::Array)
			error(stmt.value->location, "returning a braced-init-list for this result");
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*stmt.value);
		if (result->unqualified->kind == TypeKind::Class)
			list_argument(list, result, scope, stmt.value->location);
		else
			list_initialize(list, result, scope);
		return;
	}
	ExprFacts* facts = typed(*stmt.value, scope);
	if (facts == nullptr)
		return;
	if (body_->deduce_result)
	{
		if (!check_resolved(*stmt.value))
			return;
		result = deduced_result(facts, stmt.location);
		void_result = IsVoid(result);
	}
	if ((void_result || result->unqualified->kind == TypeKind::Class) && !check_resolved(*stmt.value))
		return;
	if (void_result)
	{
		if (!IsVoid(facts->type))
			error(stmt.location, "return with a value in a function returning void");
		return;
	}
	if (result->unqualified->kind == TypeKind::Class)
	{
		// 6.6.3p2, 8.5p16: the result is copy-initialized, from an
		// expression of its class (or a derived one) by the constructor it
		// selects, else by a converting constructor.
		ClassEntity* cls = static_cast<ClassEntity*>(result->unqualified->entity);
		expression_scope_ = scope;
		if (!same_class_source(cls, *stmt.value))
		{
			// CWG 1579: an eligible local can initialize a different result
			// class through its converting constructor. Keep the selected
			// typed conversion; successful rvalue selection is not repeated.
			if (!names_automatic_object(*stmt.value) || !move_return_conversion(*stmt.value, result))
				convert(*stmt.value, result, false);
			return;
		}
		// 12.8p32: a non-volatile automatic object (or parameter) named
		// alone is first treated as an rvalue, kept when that selects a
		// constructor whose first parameter is an rvalue reference to the
		// object's type.
		ExprFacts* inner = facts_of(*WithoutParentheses(stmt.value));
		const bool as_rvalue = names_automatic_object(*stmt.value) && moves_from(cls, *stmt.value);
		const ValueCategory category = inner->category;
		if (as_rvalue)
			inner->category = ValueCategory::XValue;
		record_copy(cls, result, *stmt.value, stmt.location, true);
		inner->category = category;
		return;
	}
	convert(*stmt.value, result, false);
}

bool Analyzer::move_return_conversion(const syntax::Expr& source, const Type* result)
{
	ExprFacts* facts = facts_of(source);
	ExprFacts view = *facts;
	view.category = ValueCategory::XValue;
	Conversion conversion = Conversion();
	if (!user_conversion(&view, result, conversion, false))
		return false;
	const FunctionEntity* selected = conversion.constructor;
	if (selected == nullptr || !selected->constructor || selected->type->canonical->param_count == 0)
		return false;
	const Type* parameter = selected->type->canonical->params[0];
	if (parameter->kind != TypeKind::RValueReference || parameter->element->unqualified != facts->type->unqualified)
		return false;
	const ValueCategory category = facts->category;
	facts->category = ValueCategory::XValue;
	finish_conversion(source, conversion, false);
	facts->category = category;
	return true;
}

bool Analyzer::names_automatic_object(const syntax::Expr& expr) const
{
	// An id-expression naming a non-volatile object of automatic storage
	// duration: a parameter or a block-scope variable that is neither
	// static, extern nor thread_local (12.8p31-p32), the function's own (a
	// lambda's, not the enclosing function's it captured; CWG 1579).
	const syntax::Expr* e = WithoutParentheses(&expr);
	const Entity* entity = e->kind == syntax::ExprKind::Id && e->facts != nullptr ? e->facts->entity : nullptr;
	if (entity == nullptr || entity->type == nullptr || entity->type->is_reference() ||
		(entity->type->cv & kVolatile) != 0)
		return false;
	if (!encloses(body_->function->scope, entity->owner))
		return false;
	if (entity->kind == EntityKind::Parameter)
		return true;
	if (entity->kind != EntityKind::Variable || entity->owner == nullptr || entity->owner->kind != ScopeKind::Block)
		return false;
	// A structured binding names no object of its own (C++17's 11.5; GCC
	// and Clang copy from it).
	const VariableEntity* variable = static_cast<const VariableEntity*>(entity);
	return !variable->is_static && !variable->is_extern && !variable->is_thread_local && !variable->structured_binding;
}

bool Analyzer::moves_from(ClassEntity* entity, const syntax::Expr& source)
{
	// Whether overload resolution with the object as an rvalue selects a
	// constructor whose first parameter is an rvalue reference to its type.
	ExprFacts view = *WithoutParentheses(&source)->facts;
	view.category = ValueCategory::XValue;
	syntax::Expr probe = syntax::Expr();
	probe.kind = syntax::ExprKind::Id;
	probe.facts = &view;
	syntax::Expr* pointer = &probe;
	syntax::Slice<syntax::Expr*> arguments;
	arguments.data = &pointer;
	arguments.size = 1;
	std::vector<FunctionEntity*>& set = overload_set_;
	set.clear();
	constructor_candidates(entity, arguments, ConstructionKind::Copy, set);
	Candidate chosen = Candidate();
	if (set.empty() || resolve(arguments, set, nullptr, chosen) != Resolution::Selected)
		return false;
	const Type* parameter = chosen.function->type->canonical->params[0];
	return parameter->kind == TypeKind::RValueReference && parameter->element->unqualified == view.type->unqualified;
}

// ------------------------------------------------------------ expressions

void Analyzer::initializer_names(const syntax::Initializer* initializer, Scope* scope, bool constructs_class)
{
	if (initializer == nullptr)
		return;
	// A class object's initialization selects constructors, a later
	// stage's semantics: its operands are only name-checked here.
	const bool saved = suppress_implicit_;
	suppress_implicit_ = suppress_implicit_ || constructs_class;
	switch (initializer->kind)
	{
	case syntax::InitializerKind::Assign:
	case syntax::InitializerKind::Braced:
		expression(initializer->expression, scope);
		break;
	case syntax::InitializerKind::Parenthesized:
		for (const syntax::Expr* argument : initializer->arguments)
			expression(argument, scope);
		break;
	default:
		break;
	}
	suppress_implicit_ = saved;
}

const Binding* Analyzer::check_id_expression(
	const syntax::Name& name, Scope* scope, bool allow_undeclared, bool allow_type)
{
	// Names in expressions must be declared (3.4.1); an unqualified callee
	// may still be found by argument-dependent lookup (3.4.2), which needs
	// expression types. Dependent names wait for instantiation; a template
	// definition's other names are checked where they appear (14.6p8,
	// 14.6.3). A type name is a value only as a functional cast's type or
	// where an operand may be a type-id (sizeof, alignof, typeid). A
	// template-id's name is found as a template where it appears, even
	// when its arguments depend on a template parameter (14.2p2, 14.6.2p1:
	// only an unqualified one's function lookup waits), and its arguments'
	// names are checked (14.3.1: types).
	const syntax::NameComponent& last = name.last();
	if (last.kind != syntax::NameComponentKind::Identifier)
		return nullptr;
	LookupResult result = LookupResult();
	if (!name.qualified() && !last.has_arguments)
	{
		result = unqualified(scope, last.identifier, LookupFilter::Ordinary);
		if (result.ambiguous)
			error(name.location, "ambiguous name");
		if (!result.found())
		{
			if (!allow_undeclared)
				error(name.location, "undeclared name");
			return nullptr;
		}
	}
	else if (!name.qualified() && allow_undeclared &&
		!unqualified(scope, last.identifier, LookupFilter::Ordinary).found())
	{
		// An undeclared template-id callee names the templates argument-
		// dependent lookup finds (see template_callee).
		template_argument_names(last, scope);
		return nullptr;
	}
	else
	{
		result = lookup_name(name, scope, LookupFilter::Ordinary);
		if (failed())
			return nullptr;
		// 14.6p2: a name its dependent qualifier makes dependent is assumed
		// to name no type; an instantiation where it names one is
		// ill-formed (14.6p6), which the instantiations' calls check.
		const bool dependent = last_prefix_.dependent != nullptr;
		if (dependent)
			name.dependent_qualifier = true;
		if (last.has_arguments &&
			(dependent || result.binding->kind == BindingKind::Function || IsDependentMember(result.binding)))
			template_argument_names(last, scope);
		if (dependent)
			return nullptr;
	}
	if (IsDependentMember(result.binding))
		return nullptr;
	const BindingKind kind = result.binding->kind;
	if (kind == BindingKind::Namespace || kind == BindingKind::NamespaceAlias)
		error(name.location, "namespace name used as a value");
	if ((kind == BindingKind::Type || kind == BindingKind::TypeAlias) && !allow_type)
		error(name.location, "type name used as a value");
	return result.binding;
}

void Analyzer::call_names(const syntax::CallExpr& call, Scope* scope, bool unevaluated)
{
	for (uint32_t i = call.arguments.size; i-- > 0;)
		expression_work_.push_back(PendingExpr{ call.arguments[i], unevaluated });
	if (call.callee->kind != syntax::ExprKind::Id)
	{
		expression_work_.push_back(PendingExpr{ call.callee, unevaluated });
		return;
	}
	// `T(args)` whose type the parser did not know is a functional cast.
	const syntax::Name& name = *static_cast<const syntax::IdExpr*>(call.callee)->name;
	if (deferring_reserved_ != 0 && DeferredBuiltinName(name) &&
		!unqualified(scope, name.last().identifier, LookupFilter::Ordinary).found())
		return;
	const Binding* callee = check_id_expression(name, scope, !call.arguments.empty(), true);
	// An operator-function-id callee (`operator+(t, t)`) is looked up with
	// the operator's own rules where it is instantiated.
	if (callee == nullptr && template_depth_ != 0 && !name.qualified() &&
		name.last().kind == syntax::NameComponentKind::Identifier && !call.arguments.empty())
		check_unbound_callee(call, name, scope);
	if (callee != nullptr && (callee->kind == BindingKind::Type || callee->kind == BindingKind::TypeAlias) &&
		call.arguments.empty() && !unevaluated)
		value_initialized_class(binding_type(callee, call.location));
}

void Analyzer::default_argument_names(FunctionEntity* function, const syntax::ParameterClause* clause, Scope* scope)
{
	// A templated function's default arguments are instantiated where a
	// call uses them (14.7.1p12), but their non-dependent names are bound
	// where they appear (14.6p10, 8.3.6p5): in a member-specification (a
	// member's, a friend's) in the complete class (9.2p2, with the member
	// bodies), elsewhere now.
	bool any = false;
	for (const syntax::ParameterDeclaration* parameter : clause->parameters)
		any = any || parameter->default_argument != nullptr;
	if (!any)
		return;
	if (class_depth_ != 0)
		deferred_.push_back(
			DeferredBody{ function, function->type->canonical, clause, nullptr, nullptr, scope, nullptr });
	else
		type_default_arguments(function, clause, scope);
}

void Analyzer::template_argument_names(const syntax::NameComponent& component, Scope* scope)
{
	// A function template-id's arguments (a class template-id's are its
	// lookup's): types, their names checked where they appear.
	TypeList arguments;
	template_arguments(component, scope, arguments);
}

namespace {

// A literal without a ud-suffix: of a fundamental type or an array of one,
// with no associated namespace or class (3.4.2p2).
bool PlainLiteral(const syntax::LiteralExpr& literal)
{
	const TokenSpelling& spelling = literal.spelling;
	TextLiteralParts parts;
	if (SplitTextLiteral(spelling.text, spelling.length, parts))
		return parts.suffix_begin == spelling.length;
	const NumberLiteralKind kind = ClassifyNumber(spelling.text, spelling.length).kind;
	return kind == NumberLiteralKind::Integer || kind == NumberLiteralKind::Floating;
}

// A class or enumeration declared in a block (or nested in one such), or
// a compound type of one: in a template definition, a type of the
// template's own that its instantiations make anew, as dependent as the
// body declaring it.
bool LocalType(const Type* type)
{
	while (type->kind == TypeKind::Pointer || type->kind == TypeKind::LValueReference ||
		type->kind == TypeKind::RValueReference || type->kind == TypeKind::Array)
		type = type->element;
	type = type->unqualified;
	if (type->kind != TypeKind::Class && type->kind != TypeKind::Enum)
		return false;
	for (const Scope* scope = type->entity->owner; scope != nullptr && scope->kind != ScopeKind::Namespace;
		scope = scope->parent)
	{
		if (scope->kind == ScopeKind::Block || scope->kind == ScopeKind::Function)
			return true;
	}
	return false;
}

} // namespace

const Type* Analyzer::known_argument_type(const syntax::Expr& expr, Scope* scope, uint32_t depth)
{
	// The type of a call argument in a template definition when names alone
	// show that it depends on no template parameter (14.6.2.2), without
	// typing the expression; null otherwise (maybe type-dependent). Only
	// whether the type has associated namespaces and classes matters, so a
	// fundamental type stands for every other: literals, sizeof, alignof
	// and noexcept (never type-dependent, 14.6.2.2p4), and arithmetic on
	// such operands, which no operator function can take (13.5p6); deep
	// operator nests and the template's local types are left unknown.
	if (depth > 32)
		return nullptr;
	const syntax::Expr* e = WithoutParentheses(&expr);
	const Type* fundamental = types_.fundamental(FundamentalType::Int);
	switch (e->kind)
	{
	case syntax::ExprKind::Literal:
		return PlainLiteral(static_cast<const syntax::LiteralExpr&>(*e)) ? fundamental : nullptr;
	case syntax::ExprKind::KeywordLiteral:
		return static_cast<const syntax::KeywordLiteralExpr&>(*e).keyword != TokenKind::KW_THIS ? fundamental : nullptr;
	case syntax::ExprKind::Sizeof:
	case syntax::ExprKind::SizeofPack:
		return fundamental;
	case syntax::ExprKind::BuiltinTrait:
		return static_cast<const syntax::BuiltinTraitExpr&>(*e).trait < GnuTrait::UnderlyingType ? fundamental
																								 : nullptr;
	case syntax::ExprKind::TypeTrait:
		return static_cast<const syntax::TypeTraitExpr&>(*e).keyword != TokenKind::KW_TYPEID ? fundamental : nullptr;
	case syntax::ExprKind::Unary:
	{
		const syntax::UnaryExpr& unary = static_cast<const syntax::UnaryExpr&>(*e);
		if (unary.op == TokenKind::OP_AMP || unary.op == TokenKind::OP_STAR)
			return nullptr;
		const Type* operand = known_argument_type(*unary.operand, scope, depth + 1);
		return operand != nullptr && IsArithmetic(operand->unqualified) ? fundamental : nullptr;
	}
	case syntax::ExprKind::Binary:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(*e);
		const Type* left = known_argument_type(*binary.left, scope, depth + 1);
		const Type* right = left != nullptr && IsArithmetic(left->unqualified)
			? known_argument_type(*binary.right, scope, depth + 1)
			: nullptr;
		return right != nullptr && IsArithmetic(right->unqualified) ? fundamental : nullptr;
	}
	case syntax::ExprKind::FunctionalCast:
	{
		Specifiers specifiers;
		this->specifiers(static_cast<const syntax::FunctionalCastExpr&>(*e).type, scope, specifiers, false);
		return specifiers.type != nullptr && !dependent_type(specifiers.type) && !LocalType(specifiers.type)
			? specifiers.type
			: nullptr;
	}
	case syntax::ExprKind::Call:
	{
		// `X(args)` naming a type the parser did not classify.
		const syntax::Expr* callee = static_cast<const syntax::CallExpr&>(*e).callee;
		if (callee->kind != syntax::ExprKind::Id)
			return nullptr;
		const syntax::Name& name = *static_cast<const syntax::IdExpr*>(callee)->name;
		if (name.qualified() || name.last().kind != syntax::NameComponentKind::Identifier || name.last().has_arguments)
			return nullptr;
		const LookupResult result = unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
		if (!result.found() || result.ambiguous ||
			(result.binding->kind != BindingKind::Type && result.binding->kind != BindingKind::TypeAlias))
			return nullptr;
		const Type* type = result.binding->kind == BindingKind::Type ? result.binding->entity->type
																	 : result.binding->type;
		return dependent_type(type) || LocalType(type) ? nullptr : type;
	}
	case syntax::ExprKind::Id:
	{
		const syntax::Name& name = *static_cast<const syntax::IdExpr&>(*e).name;
		if (name.qualified() || name.last().kind != syntax::NameComponentKind::Identifier || name.last().has_arguments)
			return nullptr;
		const LookupResult result = unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
		if (!result.found() || result.ambiguous || result.binding->entity == nullptr)
			return nullptr;
		const Type* type = nullptr;
		switch (result.binding->kind)
		{
		case BindingKind::Variable:
		case BindingKind::Enumerator:
			type = result.binding->entity->type;
			break;
		case BindingKind::Parameter:
			type = result.binding->type;
			break;
		default:
			return nullptr;
		}
		if (type == nullptr || dependent_type(type) || LocalType(type))
			return nullptr;
		return type->is_reference() ? type->element : type;
	}
	default:
		return nullptr;
	}
}

void Analyzer::check_unbound_callee(const syntax::CallExpr& call, const syntax::Name& name, Scope* scope)
{
	// 14.6.2p1, 14.6p9-p10: an unqualified callee that ordinary lookup does
	// not find is a dependent name only if an argument is type-dependent;
	// with arguments that depend on no template parameter it is bound
	// where it appears, by argument-dependent lookup from their types
	// (3.4.2), which must find a function. Arguments whose types a name
	// alone does not show leave it for the instantiations, as does a name
	// lookup finds as a dependent member (a using-declaration's) and a
	// name reserved to the implementation (17.6.4.3.2p1: its intrinsics,
	// which calls resolve undeclared).
	const uint32_t count = call.arguments.size;
	static const uint32_t kProbes = 8;
	const char* spelling = name.last().identifier->spelling;
	if (spelling[0] == '_' && (spelling[1] == '_' || (spelling[1] >= 'A' && spelling[1] <= 'Z')))
		return;
	// The parser's unspellable callees (the coroutine operations).
	if (spelling[0] == ' ')
		return;
	if (count > kProbes || unqualified(scope, name.last().identifier, LookupFilter::Ordinary).found())
		return;
	const Type* types[kProbes];
	bool associates = false;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (call.arguments[i]->kind == syntax::ExprKind::BracedInit ||
			(types[i] = known_argument_type(*call.arguments[i], scope)) == nullptr)
			return;
		associates = associates || !IsArithmetic(types[i]->unqualified);
	}
	if (associates)
	{
		// The associated namespaces' functions of the name, as a call with
		// arguments of these types would find them.
		syntax::Expr probes[kProbes];
		ExprFacts facts[kProbes];
		syntax::Expr* pointers[kProbes];
		for (uint32_t i = 0; i < count; ++i)
		{
			facts[i] = ExprFacts();
			facts[i].type = types[i];
			probes[i] = syntax::Expr();
			probes[i].facts = &facts[i];
			pointers[i] = &probes[i];
		}
		const size_t found = adl_functions_.size();
		argument_dependent(syntax::Slice<syntax::Expr*>{ pointers, count }, name.last().identifier);
		const bool bound = adl_functions_.size() != found;
		adl_functions_.resize(found);
		if (bound)
			return;
	}
	error(name.location, "undeclared name");
}

LookupResult Analyzer::unqualified(Scope* scope, const Identifier* name, LookupFilter filter)
{
	// operator= is found in the innermost enclosing class (every class has
	// one, 12.8p18).
	if (name == assign_key_)
	{
		Scope* found = scope;
		while (found != nullptr && found->kind != ScopeKind::Class)
			found = found->parent;
		implicit_members_for(found, name);
	}
	LookupResult result = names_.unqualified(scope, name, filter);
	const bool predefined = name == func_ || name == function_name_ || name == pretty_function_;
	if (!result.found() && filter == LookupFilter::Ordinary && !predefined)
		return builtin_lookup(name, result);
	if (!predefined || filter != LookupFilter::Ordinary)
		return result;
	// 8.4.1p8: the function-local predefined variable __func__, as if
	// `static const char __func__[] = "function-name";` (GCC's unqualified
	// name) began the body, without a line of its own; materialized on its
	// first reference. It is the innermost function's: an enclosing
	// function's is hidden in a lambda's or a local class member's body
	// (never captured).
	Scope* function = scope;
	while (function != nullptr && (function->kind != ScopeKind::Function || function->entity == nullptr))
		function = function->parent;
	if (result.found() &&
		(result.binding->entity == nullptr || result.binding->entity->kind != EntityKind::Variable ||
			!static_cast<const VariableEntity*>(result.binding->entity)->predefined_name ||
			result.binding->entity->owner == function))
		return result;
	if (function == nullptr || function->entity->kind != EntityKind::Function)
		return result;
	const FunctionEntity* owner = static_cast<const FunctionEntity*>(function->entity);
	const std::string name_text = name == pretty_function_ ? PrettyFunctionName(owner, graph_)
														   : PredefinedFunctionName(owner);
	const size_t length = name_text.size();
	const char* text = copy_label(name_text.data(), length);
	const Type* type = types_.array(types_.qualified(types_.fundamental(FundamentalType::Char), kConst), length + 1);
	PredefinedNameEntity* variable = new_entity<PredefinedNameEntity>(EntityKind::Variable, name, function, type);
	variable->predefined_name = true;
	variable->text = text;
	Binding* binding = new_binding(BindingKind::Variable, name, variable, type);
	binding->hidden = true;
	declare_in(function, binding);
	return names_.unqualified(scope, name, filter);
}

void Analyzer::value_initialized_class(const Type* type)
{
	// T() and T{} (5.2.3p2) and new T / new T() (5.3.4p15) create an
	// object of class type with its default constructor.
	if (type == nullptr || suppress_implicit_ || template_depth_ != 0)
		return;
	type = type->unqualified;
	if (type->kind == TypeKind::Class)
		define_implicit_default_constructor(static_cast<ClassEntity*>(type->entity));
}

void Analyzer::expression(const syntax::Expr* expr, Scope* scope)
{
	if (expr == nullptr)
		return;
	// (A hosted wrapper's statement analyzed again names only, see
	// deferrable_statement, is a pattern.)
	if (semantic() && template_depth_ == 0 && body_ != nullptr && !body_->pattern)
	{
		// A discarded or converted class prvalue (an expression statement,
		// a for-statement's, a condition's operand of a conversion
		// function) is a temporary (12.2p1).
		check_temporary_destruction(typed_value(*expr, scope), expr->location);
		return;
	}
	expression_walk(expr, scope);
}

void Analyzer::expression_walk(const syntax::Expr* expr, Scope* scope, bool unevaluated)
{
	if (expr == nullptr)
		return;
	if (template_depth_ != 0 && semantic())
		validate_fixed_expression(*expr, scope);
	const size_t base = expression_work_.size();
	expression_work_.push_back(PendingExpr{ expr, unevaluated });
	while (expression_work_.size() > base)
	{
		const PendingExpr pending = expression_work_.back();
		expression_work_.pop_back();
		expression_names(*pending.expr, scope, pending.unevaluated);
	}
}

bool Analyzer::dependent_object(const syntax::Expr& object, bool arrow, Scope* scope)
{
	// Whether an object expression that is a variable's or parameter's name
	// has a dependent class type (through the pointer for `->`) that is not
	// the current instantiation (a template's, partial specialization's or
	// member class's own type in its definition, 14.6.2.1p1).
	if (object.kind != syntax::ExprKind::Id)
		return false;
	const syntax::Name& name = *static_cast<const syntax::IdExpr&>(object).name;
	if (name.qualified() || name.last().has_arguments || name.last().kind != syntax::NameComponentKind::Identifier)
		return false;
	const LookupResult found = names_.unqualified(scope, name.last().identifier, LookupFilter::Ordinary);
	if (!found.found() || found.ambiguous ||
		(found.binding->kind != BindingKind::Variable && found.binding->kind != BindingKind::Parameter) ||
		found.binding->type == nullptr)
		return false;
	const Type* type = found.binding->type;
	if (type->is_reference())
		type = type->element;
	if (arrow)
	{
		if (type->unqualified->kind != TypeKind::Pointer)
			return false;
		type = type->unqualified->element;
	}
	type = type->unqualified;
	if (!dependent_type(type))
		return false;
	if (type->kind == TypeKind::Class)
	{
		const ClassTemplateFacts* facts = static_cast<const ClassEntity*>(type->entity)->templ;
		return facts == nullptr || facts->role == TemplateRole::Specialization;
	}
	return type->kind == TypeKind::TemplateParameter;
}

void Analyzer::expression_names(const syntax::Expr& expr, Scope* scope, bool unevaluated)
{
	// One node; operands go on the work list, so left-nested chains of any
	// length take constant stack.
	std::vector<PendingExpr>& work = expression_work_;
	switch (expr.kind)
	{
	case syntax::ExprKind::Id:
	{
		const Binding* binding = check_id_expression(
			*static_cast<const syntax::IdExpr&>(expr).name, scope, false, unevaluated);
		if (capture_scan_ != nullptr && binding != nullptr && !unevaluated)
			scan_capture(binding, expr.location);
		return;
	}
	case syntax::ExprKind::Paren:
		work.push_back(PendingExpr{ static_cast<const syntax::ParenExpr&>(expr).inner, unevaluated });
		return;
	case syntax::ExprKind::Binary:
	case syntax::ExprKind::Assign:
	{
		const syntax::BinaryExpr& binary = static_cast<const syntax::BinaryExpr&>(expr);
		work.push_back(PendingExpr{ binary.right, unevaluated });
		work.push_back(PendingExpr{ binary.left, unevaluated });
		return;
	}
	case syntax::ExprKind::Conditional:
	{
		const syntax::ConditionalExpr& conditional = static_cast<const syntax::ConditionalExpr&>(expr);
		work.push_back(PendingExpr{ conditional.else_expr, unevaluated });
		work.push_back(PendingExpr{ conditional.then_expr, unevaluated });
		work.push_back(PendingExpr{ conditional.condition, unevaluated });
		return;
	}
	case syntax::ExprKind::Unary:
	case syntax::ExprKind::Postfix:
		work.push_back(PendingExpr{ static_cast<const syntax::UnaryExpr&>(expr).operand, unevaluated });
		return;
	case syntax::ExprKind::Call:
		return call_names(static_cast<const syntax::CallExpr&>(expr), scope, unevaluated);
	case syntax::ExprKind::FunctionalCast:
	{
		const syntax::FunctionalCastExpr& cast = static_cast<const syntax::FunctionalCastExpr&>(expr);
		Specifiers specifiers;
		this->specifiers(cast.type, scope, specifiers, false);
		const bool empty = cast.braced != nullptr ? cast.braced->elements.empty() : cast.arguments.empty();
		if (empty && !unevaluated)
			value_initialized_class(specifiers.type);
		// The parser fills only the written form's operands.
		if (cast.braced != nullptr)
			work.push_back(PendingExpr{ cast.braced, unevaluated });
		else
		{
			for (const syntax::Expr* argument : cast.arguments)
				work.push_back(PendingExpr{ argument, unevaluated });
		}
		return;
	}
	case syntax::ExprKind::Subscript:
	{
		const syntax::SubscriptExpr& subscript = static_cast<const syntax::SubscriptExpr&>(expr);
		work.push_back(PendingExpr{ subscript.index, unevaluated });
		work.push_back(PendingExpr{ subscript.base, unevaluated });
		return;
	}
	case syntax::ExprKind::Member:
	{
		// The member name needs the object's class: expression typing. A
		// member template-id of an object of a dependent type other than
		// the current instantiation needs `template` (14.2p4): checked for
		// an object named by a variable or parameter. A destructor's
		// `~P<T>` is none: its type-name is looked up in the context of
		// the whole postfix-expression too (3.4.5p3).
		const syntax::MemberExpr& member = static_cast<const syntax::MemberExpr&>(expr);
		const syntax::NameComponent& last = member.member->last();
		if (last.has_arguments && !last.template_keyword && !member.member->qualified() &&
			last.kind != syntax::NameComponentKind::Destructor &&
			dependent_object(*member.base, member.op == TokenKind::OP_ARROW, scope))
			error(member.member->location, "a member template-id of a dependent object needs template");
		work.push_back(PendingExpr{ member.base, unevaluated });
		return;
	}
	case syntax::ExprKind::Cast:
	{
		// A template definition's type-ids name types as its declarations'
		// do (14.6p2, p10), dependent ones kept as such.
		const syntax::CastExpr& cast = static_cast<const syntax::CastExpr&>(expr);
		type_id(*cast.type, scope);
		work.push_back(PendingExpr{ cast.operand, unevaluated });
		return;
	}
	case syntax::ExprKind::Sizeof:
	{
		const syntax::SizeofExpr& size = static_cast<const syntax::SizeofExpr&>(expr);
		if (size.type != nullptr)
			type_id(*size.type, scope);
		// 5.3.3p1: `sizeof unary-expression`: a name there is a value.
		if (size.operand != nullptr && !size.parenthesized && size.operand->kind == syntax::ExprKind::Id)
			check_id_expression(*static_cast<const syntax::IdExpr*>(size.operand)->name, scope, false, false);
		else if (size.operand != nullptr)
			work.push_back(PendingExpr{ size.operand, true });
		return;
	}
	case syntax::ExprKind::TypeTrait:
	{
		const syntax::TypeTraitExpr& trait = static_cast<const syntax::TypeTraitExpr&>(expr);
		if (trait.type != nullptr)
			type_id(*trait.type, scope);
		if (trait.operand != nullptr)
			work.push_back(PendingExpr{ trait.operand, true });
		return;
	}
	case syntax::ExprKind::BuiltinTrait:
		for (const syntax::TypeId* type : static_cast<const syntax::BuiltinTraitExpr&>(expr).types)
			type_id(*type, scope);
		if (static_cast<const syntax::BuiltinTraitExpr&>(expr).operand != nullptr)
			work.push_back(PendingExpr{ static_cast<const syntax::BuiltinTraitExpr&>(expr).operand,
				static_cast<const syntax::BuiltinTraitExpr&>(expr).trait == GnuTrait::TypeofOperand });
		return;
	case syntax::ExprKind::New:
		return new_expression(static_cast<const syntax::NewExpr&>(expr), scope, unevaluated);
	case syntax::ExprKind::Delete:
		work.push_back(PendingExpr{ static_cast<const syntax::DeleteExpr&>(expr).operand, unevaluated });
		return;
	case syntax::ExprKind::BracedInit:
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(expr);
		for (uint32_t i = list.elements.size; i-- > 0;)
			work.push_back(PendingExpr{ list.elements[i], unevaluated });
		return;
	}
	case syntax::ExprKind::PackExpansion:
		work.push_back(PendingExpr{ static_cast<const syntax::PackExpansionExpr&>(expr).pattern, unevaluated });
		return;
	case syntax::ExprKind::Fold:
	{
		const syntax::FoldExpr& fold = static_cast<const syntax::FoldExpr&>(expr);
		work.push_back(PendingExpr{ fold.pattern, unevaluated });
		if (fold.init != nullptr)
			work.push_back(PendingExpr{ fold.init, unevaluated });
		return;
	}
	case syntax::ExprKind::SizeofPack:
	{
		// 5.3.3p5: the operand of sizeof... names a parameter pack (a
		// template's, or a function parameter pack, whose type is an
		// expansion), checked where it appears.
		const Identifier* pack = static_cast<const syntax::SizeofPackExpr&>(expr).pack;
		const LookupResult found = unqualified(scope, pack, LookupFilter::Ordinary);
		const Binding* binding = found.binding;
		const bool template_pack = binding != nullptr && binding->entity != nullptr &&
			binding->entity->kind == EntityKind::TemplateParameter &&
			static_cast<const TemplateParameterEntity*>(binding->entity)->pack;
		const bool function_pack = binding != nullptr && binding->kind == BindingKind::Parameter &&
			binding->type != nullptr && binding->type->kind == TypeKind::Expansion;
		uint64_t count = 0;
		if (!template_pack && !function_pack && !pack_size(pack, scope, count))
			error(expr.location, "sizeof... of a name that is no parameter pack");
		return;
	}
	case syntax::ExprKind::Throw:
		// `throw` alone (a rethrow) has no operand.
		if (static_cast<const syntax::ThrowExpr&>(expr).operand != nullptr)
			work.push_back(PendingExpr{ static_cast<const syntax::ThrowExpr&>(expr).operand, unevaluated });
		return;
	case syntax::ExprKind::Statement:
		statement_block(static_cast<const syntax::StatementExpr&>(expr), scope);
		return;
	case syntax::ExprKind::Lambda:
		// A lambda's body is outside PA6's statement slice, but a templated
		// lambda's walk sees the names it uses (scan_nested_lambda).
		if (capture_scan_ != nullptr && !unevaluated)
			scan_nested_lambda(static_cast<const syntax::LambdaExpr&>(expr), scope);
		return;
	default:
		// Literals, `this` (which a templated lambda's capture-default
		// captures, see scan_capture) and sizeof....
		if (capture_scan_ != nullptr && expr.kind == syntax::ExprKind::KeywordLiteral &&
			static_cast<const syntax::KeywordLiteralExpr&>(expr).keyword == TokenKind::KW_THIS &&
			capture_scan_->expr->has_default)
			capture_scan_->this_captured = true;
		return;
	}
}

void Analyzer::statement_block(const syntax::StatementExpr& expr, Scope* scope)
{
	// GNU: the block is part of its function's body (none outside one: a
	// namespace-scope or static data member's initializer, a default
	// argument or default member initializer has no function); jumping
	// into it is not allowed (a goto naming one of its labels from
	// outside, a case or default label in it of a switch outside): its
	// block is marked as a try block's, and the switch statements outside
	// count it. The types mode keeps no body context: it only walks the
	// block's names.
	if (!semantic())
	{
		compound(*expr.body, scope, false, true);
		return;
	}
	if (body_ == nullptr || body_->function == nullptr)
		error(expr.location, "a statement expression outside a function body");
	++body_->handled;
	compound(*expr.body, scope, false, true);
	--body_->handled;
}

ExprFacts* Analyzer::statement_expression(const syntax::StatementExpr& expr, Scope* scope)
{
	// GNU: a block of the enclosing function's body; the value of its last
	// statement when that is an expression statement, labeled or not
	// (decayed, cv-unqualified; a class object copy-initialized as a
	// returned one is), else void.
	statement_block(expr, scope);
	// A deferral its statements left pending (see deferrable_statement):
	// the full-expression holding the block is deferred.
	if (deferred_reserved_call())
		return nullptr;
	const syntax::ExpressionStmt* last = syntax::StatementExprValue(expr);
	const syntax::Expr* value = last != nullptr ? last->expression : nullptr;
	const ExprFacts* facts = value != nullptr ? value->facts : nullptr;
	const Type* void_type = types_.fundamental(FundamentalType::Void);
	if (facts == nullptr || facts->type == nullptr || IsVoid(facts->type))
		return new_facts(expr, void_type, ValueCategory::PRValue);
	const Type* type = facts->type->unqualified;
	if (type->kind == TypeKind::Array)
		type = types_.pointer(type->element);
	else if (type->kind == TypeKind::Function)
		type = types_.pointer(type);
	if (type->kind == TypeKind::Class)
	{
		ClassEntity* cls = static_cast<ClassEntity*>(type->entity);
		expression_scope_ = scope;
		if (!same_class_source(cls, *value))
			convert(*value, type, false);
		else
			record_copy(cls, type, *value, expr.location, true);
	}
	else
		convert(*value, type, false);
	return new_facts(expr, type, ValueCategory::PRValue);
}

void Analyzer::new_expression(const syntax::NewExpr& expr, Scope* scope, bool unevaluated)
{
	// The parser fills the placement and initializer operands of the forms
	// written.
	if (expr.has_placement)
	{
		for (const syntax::Expr* argument : expr.placement)
			expression_work_.push_back(PendingExpr{ argument, unevaluated });
	}
	// 5.3.4p6: the first bound of a new-type-id is an ordinary expression.
	syntax::TypeId type_syntax = *expr.type;
	syntax::Declarator declarator = syntax::Declarator();
	if (type_syntax.declarator != nullptr && !type_syntax.declarator->suffixes.empty() &&
		type_syntax.declarator->suffixes[0].kind == syntax::DeclaratorSuffixKind::Array &&
		type_syntax.declarator->suffixes[0].bound != nullptr)
	{
		expression_work_.push_back(PendingExpr{ type_syntax.declarator->suffixes[0].bound, unevaluated });
		declarator = *type_syntax.declarator;
		declarator.suffixes.data += 1;
		declarator.suffixes.size -= 1;
		type_syntax.declarator = &declarator;
	}
	// 5.3.4p2: an auto type is the initializer's (see new_auto_type).
	const Type* type = TypeIdHoldsAuto(type_syntax) ? nullptr : type_id(type_syntax, scope);
	const syntax::Initializer* initializer = expr.initializer;
	const bool empty = initializer == nullptr ||
		(initializer->kind == syntax::InitializerKind::Parenthesized && initializer->arguments.empty());
	if (empty && !unevaluated && type != nullptr)
		value_initialized_class(type);
	if (initializer == nullptr)
		return;
	if (initializer->kind == syntax::InitializerKind::Parenthesized)
	{
		for (const syntax::Expr* argument : initializer->arguments)
			expression_work_.push_back(PendingExpr{ argument, unevaluated });
	}
	else if (initializer->kind == syntax::InitializerKind::Braced ||
		initializer->kind == syntax::InitializerKind::Assign)
		expression_work_.push_back(PendingExpr{ initializer->expression, unevaluated });
}

} // namespace sema
