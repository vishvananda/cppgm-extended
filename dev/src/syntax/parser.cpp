#include "syntax/parser.h"

#include "lex/identifier_table.h"

namespace syntax {

Parser::Parser(ParseTokenStream& tokens, IdentifierTable& identifiers, Arena& arena, bool hosted)
	: ParserState(tokens, identifiers, arena, hosted)
{}

TranslationUnit* Parser::parse_translation_unit()
{
	char marker;
	stack_base_ = reinterpret_cast<uintptr_t>(&marker);
	ListFrame<Decl*> list(decls_);
	if (!parse_declaration_seq(list, false, DeclContext::Namespace))
		return nullptr;
	TranslationUnit* unit = make<TranslationUnit>();
	unit->declarations = list.take(arena_);
	return unit;
}

bool Parser::parse_declaration_seq(ListFrame<Decl*>& list, bool until_brace, DeclContext context)
{
	for (;;)
	{
		// Outside speculation no earlier position is revisited.
		if (tentative_depth_ == 0)
		{
			failed_template_lists_.clear();
			angle_fences_.clear();
		}
		if (until_brace && at(TokenKind::OP_RBRACE))
			return true;
		if (tok().is_end())
			return until_brace ? fail() : true;
		Decl* decl = parse_declaration(context);
		if (decl == nullptr)
			return false;
		list.push(decl);
	}
}

// ------------------------------------------------------ member function bodies

bool Parser::at_function_body_start(bool allow_ctor_initializer)
{
	return at(TokenKind::OP_LBRACE) || at(TokenKind::KW_TRY) || (allow_ctor_initializer && at(TokenKind::OP_COLON));
}

bool Parser::skip_function_body()
{
	const bool try_block = accept(TokenKind::KW_TRY);
	if (accept(TokenKind::OP_COLON))
	{
		// mem-initializers: a `{` right after a name or template-id starts a
		// braced initializer; after `)`, `}` or `...` it starts the body.
		bool after_name = false;
		for (;;)
		{
			if (tok().is_end())
				return fail();
			if (at(TokenKind::OP_LBRACE) && !after_name)
				break;
			if (at(TokenKind::OP_LPAREN) || at(TokenKind::OP_LSQUARE) || at(TokenKind::OP_LBRACE))
			{
				if (!skip_balanced())
					return false;
				after_name = false;
				continue;
			}
			after_name = at_identifier() || at(TokenKind::OP_GT);
			next();
		}
	}
	if (!at(TokenKind::OP_LBRACE) || !skip_balanced())
		return fail();
	if (!try_block)
		return true;
	if (!at(TokenKind::KW_CATCH))
		return fail();
	while (accept(TokenKind::KW_CATCH))
	{
		if (!at(TokenKind::OP_LPAREN) || !skip_balanced() || !at(TokenKind::OP_LBRACE) || !skip_balanced())
			return fail();
	}
	return true;
}

bool Parser::defer_function_body(FunctionBody& body, bool allow_ctor_initializer, Scope* scope)
{
	const uint32_t begin = tokens_.position();
	tokens_.retain();
	const bool skipped = skip_function_body();
	TokenRun run{ nullptr, 0, 0 };
	if (skipped)
		run = tokens_.capture(begin, tokens_.position(), runs_);
	tokens_.release();
	if (!skipped)
		return false;
	deferred_tokens_ += run.size;
	body.ctor_initializer = nullptr;
	body.body = nullptr;
	body.try_block = nullptr;
	deferred_.push_back(DeferredBody{ &body, run, scope, allow_ctor_initializer });
	return true;
}

bool Parser::parse_deferred_bodies(size_t first)
{
	// The outermost class is complete: each captured body is parsed once, in
	// its function's scope (bodies of local classes queue and finish at
	// their own class's end).
	for (size_t i = first; i < deferred_.size(); ++i)
	{
		const DeferredBody entry = deferred_[i];
		++deferred_count_;
		tokens_.push_run(entry.tokens);
		Scope* saved = scopes_.enter(entry.scope);
		const bool saved_angle = angle_;
		angle_ = false;
		bool parsed = parse_function_body(*entry.target, entry.allow_ctor_initializer);
		if (parsed && !tok().is_end())
			parsed = fail();
		angle_ = saved_angle;
		scopes_.set_current(saved);
		tokens_.pop_run();
		if (!parsed)
			return false;
	}
	deferred_.resize(first);
	return true;
}

void Parser::parse_deferred_lambdas(size_t first)
{
	// The outermost speculation has committed, so the lambda bodies it
	// skipped belong to the tree: each is parsed once, in source order, in
	// its scope and with only the names declared before it visible (later
	// name changes of the committed region are detached meanwhile). A body
	// that does not parse is a syntax error of the translation unit: every
	// alternative contained the same lambda.
	const size_t last = lambdas_.size();
	const uint32_t resume = tokens_.position();
	Scope* const scope = scopes_.current();
	Scope* const function_scope = function_scope_;
	Scope* const parameter_scope = parameter_scope_;
	Scope* const type_id_scope = type_id_scope_;
	const uint8_t type_id_record = type_id_record_;
	const bool angle = angle_;
	const bool pending_template = pending_template_;
	const bool definite_type = definite_type_;
	const bool special_member_clause = special_member_clause_;
	const uint32_t base = lambdas_[first].log;
	std::vector<NameScopes::Change> later;
	scopes_.detach(base, later);
	size_t reattached = 0;
	// A body another reading of the statement already has is shared; the
	// statements of the bodies parsed here share their own.
	// (Both are in source order: the search resumes after the last match.)
	std::vector<std::pair<uint32_t, CompoundStmt*>>* const shared = shared_bodies_;
	shared_bodies_ = nullptr;
	size_t next_shared = 0;
	for (size_t i = first; i < last; ++i)
	{
		const DeferredLambda deferred = lambdas_[i];
		scopes_.reattach(later, reattached, deferred.log - base);
		reattached = deferred.log - base;
		size_t match = next_shared;
		while (shared != nullptr && match < shared->size() && (*shared)[match].first < deferred.begin)
			++match;
		if (shared != nullptr && match < shared->size() && (*shared)[match].first == deferred.begin)
		{
			deferred.lambda->body = (*shared)[match].second;
			next_shared = match + 1;
			continue;
		}
		SpeculationBoundary boundary(*this);
		++deferred_lambdas_;
		tokens_.rewind(deferred.begin);
		scopes_.set_current(deferred.scope);
		angle_ = false;
		pending_template_ = false;
		definite_type_ = deferred.definite_type;
		special_member_clause_ = deferred.special_member_clause;
		CompoundStmt* body = parse_compound_statement(true);
		if (body == nullptr || tokens_.position() != deferred.end)
			throw ParseError(error_location_, "syntax error");
		deferred.lambda->body = body;
		if (shared != nullptr)
			shared->push_back(std::make_pair(deferred.begin, body));
	}
	shared_bodies_ = shared;
	scopes_.reattach(later, reattached, later.size());
	lambdas_.resize(first);
	tokens_.rewind(resume);
	scopes_.set_current(scope);
	function_scope_ = function_scope;
	parameter_scope_ = parameter_scope;
	type_id_scope_ = type_id_scope;
	type_id_record_ = type_id_record;
	angle_ = angle;
	pending_template_ = pending_template;
	definite_type_ = definite_type;
	special_member_clause_ = special_member_clause;
}

bool Parser::parse_function_body(FunctionBody& body, bool allow_ctor_initializer)
{
	body.ctor_initializer = nullptr;
	body.try_block = nullptr;
	// The body's first call of a reserved builtin name no registry holds
	// (an enclosing body's is restored however the parse ends).
	struct ReservedCall
	{
		const Identifier*& current;
		const Identifier* saved;
		FunctionBody& body;
		~ReservedCall()
		{
			body.reserved_call = current;
			current = saved;
		}
	} reserved{ reserved_call_, reserved_call_, body };
	reserved_call_ = nullptr;
	const bool try_block = accept(TokenKind::KW_TRY);
	if (allow_ctor_initializer && at(TokenKind::OP_COLON))
	{
		body.ctor_initializer = parse_ctor_initializer();
		if (body.ctor_initializer == nullptr)
			return false;
	}
	const bool saved_template = pending_template_;
	pending_template_ = false;
	body.body = parse_compound_statement(true);
	pending_template_ = saved_template;
	if (body.body == nullptr)
		return false;
	if (!try_block)
		return true;
	ListFrame<Handler> handlers(handlers_);
	if (!parse_handlers(handlers))
		return false;
	body.try_block = make<TryStmt>(StmtKind::Try, body.body->location);
	body.try_block->body = body.body;
	body.try_block->handlers = handlers.take(arena_);
	return true;
}

bool Parser::parse_handlers(ListFrame<Handler>& handlers)
{
	if (!at(TokenKind::KW_CATCH))
		return fail();
	while (at(TokenKind::KW_CATCH))
	{
		Handler handler;
		if (!parse_handler(handler))
			return false;
		handlers.push(handler);
	}
	return true;
}

CtorInitializer* Parser::parse_ctor_initializer()
{
	CtorInitializer* initializer = make<CtorInitializer>();
	initializer->location = location();
	if (!expect(TokenKind::OP_COLON))
		return nullptr;
	ListFrame<MemInitializer> members(mem_inits_);
	do
	{
		MemInitializer member = MemInitializer();
		member.location = location();
		const uint32_t mentions = pack_mentions_;
		NameInfo info;
		if (at(TokenKind::KW_DECLTYPE))
		{
			NameComponent component = NameComponent();
			component.kind = NameComponentKind::Decltype;
			component.location = location();
			next();
			component.decltype_operand = parse_decltype_operand();
			if (component.decltype_operand == nullptr)
				return nullptr;
			member.name = make<Name>();
			member.name->global = false;
			member.name->typename_keyword = false;
			member.name->location = component.location;
			member.name->components = Slice<NameComponent>{ arena_.copy_array(&component, 1), 1 };
		}
		else
		{
			member.name = parse_name(NameMode::Type, info);
		}
		if (member.name == nullptr)
			return nullptr;
		if (at(TokenKind::OP_LBRACE))
		{
			member.braced = true;
			member.list = parse_braced_init_list();
			if (member.list == nullptr)
				return nullptr;
		}
		else
		{
			if (!expect(TokenKind::OP_LPAREN))
				return nullptr;
			ListFrame<Expr*> arguments(exprs_);
			if (!parse_expression_list(arguments, TokenKind::OP_RPAREN))
				return nullptr;
			member.arguments = arguments.take(arena_);
		}
		member.pack = accept(TokenKind::OP_DOTS);
		if (member.pack)
			pack_mentions_ = mentions;
		members.push(member);
	} while (accept(TokenKind::OP_COMMA));
	initializer->initializers = members.take(arena_);
	return initializer;
}

} // namespace syntax
