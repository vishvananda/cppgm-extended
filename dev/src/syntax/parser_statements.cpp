#include "syntax/parser.h"

#include "lex/identifier_table.h"

#include <algorithm>
#include <vector>

namespace syntax {

CompoundStmt* Parser::parse_compound_statement(bool new_scope)
{
	CompoundStmt* stmt = make<CompoundStmt>(StmtKind::Compound, location());
	if (!expect(TokenKind::OP_LBRACE))
		return nullptr;
	// A body in a statement read again (a lambda's, a local class member's)
	// is read as written: its statements have readings of their own.
	const ConflictScope conflicts(conflicts_);
	Scope* saved = scopes_.current();
	if (new_scope)
		scopes_.push(ScopeKind::Block, nullptr);
	ListFrame<Stmt*> items(stmts_);
	bool parsed = true;
	while (!at(TokenKind::OP_RBRACE))
	{
		if (tentative_depth_ == 0)
		{
			failed_template_lists_.clear();
			angle_fences_.clear();
		}
		if (tok().is_end())
		{
			parsed = fail();
			break;
		}
		Stmt* item = parse_statement();
		if (item == nullptr)
		{
			parsed = false;
			break;
		}
		items.push(item);
	}
	scopes_.set_current(saved);
	if (!parsed || !expect(TokenKind::OP_RBRACE))
		return nullptr;
	stmt->items = items.take(arena_);
	return stmt;
}

namespace {

// `keyword ( N <` or `keyword ( N ::` with N a template whose
// specializations may disagree on a member's category: the head may begin
// with a declaration whose type is such a member (an expression's
// operand's category does not decide its parse).
bool HeadNamesSpecialized(const ParseToken& name, const ParseToken& next, const NameScopes& scopes)
{
	return name.is_identifier() && (next.is(TokenKind::OP_LT) || next.is(TokenKind::OP_COLON2)) &&
		scopes.member_conflicts(name.identifier);
}

} // namespace

Stmt* Parser::parse_statement()
{
	NestingGuard nesting(*this);
	// A declaration statement's leading attributes are its declaration's
	// (7p1); other statements drop theirs.
	Attribute* attributes = nullptr;
	if (!collect_attributes(attributes))
		return nullptr;
	const uint32_t start = location();
	const ParseToken token = tok();
	if (token.is_identifier() && at(TokenKind::OP_COLON, 1))
	{
		LabeledStmt* label = make<LabeledStmt>(StmtKind::Labeled, start);
		label->label = token.identifier;
		next();
		next();
		label->body = parse_statement();
		return label->body == nullptr ? nullptr : label;
	}
	if (at_coroutine_keyword(coroutine_keywords_[2]))
	{
		// co_return expression-or-braced-init-list? ; (C++20 9.5.4).
		next();
		Expr* operand = nullptr;
		if (!at(TokenKind::OP_SEMICOLON) &&
			(operand = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_expression()) == nullptr)
			return nullptr;
		if (!expect(TokenKind::OP_SEMICOLON))
			return nullptr;
		ExpressionStmt* stmt = make<ExpressionStmt>(StmtKind::Expression, start);
		stmt->expression = coroutine_operation(coroutine_keywords_[2], operand, start);
		return stmt;
	}
	// `co_await e;`, `co_yield e;` and GNU's `__real__ e ...;` are no
	// declarations.
	const uint8_t part = token.is_identifier() ? gnu_names_.kind(token.identifier) : 0;
	if (at_coroutine_keyword(coroutine_keywords_[0]) || at_coroutine_keyword(coroutine_keywords_[1]) ||
		part == GnuNames::kRealPart || part == GnuNames::kImagPart)
		return parse_expression_statement();
	if (token.kind != ParseTokenKind::Simple)
		return parse_declaration_or_expression_statement(attributes);
	switch (token.simple)
	{
	case TokenKind::OP_LBRACE:
		return parse_compound_statement(true);
	// A condition's or for-range-declaration's names, like a declaration
	// statement's, may need the statement read each way.
	case TokenKind::KW_IF:
		if (scopes_.any_member_conflicts() && HeadNamesSpecialized(tok(2), tok(3), scopes_))
			return read_statement(&Parser::parse_control_reading, nullptr);
		return parse_if();
	case TokenKind::KW_SWITCH:
		if (scopes_.any_member_conflicts() && HeadNamesSpecialized(tok(2), tok(3), scopes_))
			return read_statement(&Parser::parse_control_reading, nullptr);
		return parse_loop_or_switch(StmtKind::Switch);
	case TokenKind::KW_WHILE:
		if (scopes_.any_member_conflicts() && HeadNamesSpecialized(tok(2), tok(3), scopes_))
			return read_statement(&Parser::parse_control_reading, nullptr);
		return parse_loop_or_switch(StmtKind::While);
	case TokenKind::KW_DO:
		return parse_do();
	case TokenKind::KW_FOR:
		if (scopes_.any_member_conflicts() && HeadNamesSpecialized(tok(2), tok(3), scopes_))
			return read_statement(&Parser::parse_control_reading, nullptr);
		return parse_for();
	case TokenKind::KW_TRY:
		return parse_try();
	case TokenKind::KW_BREAK:
	case TokenKind::KW_CONTINUE:
	{
		Stmt* jump = make<Stmt>(token.simple == TokenKind::KW_BREAK ? StmtKind::Break : StmtKind::Continue, start);
		next();
		return expect(TokenKind::OP_SEMICOLON) ? jump : nullptr;
	}
	case TokenKind::KW_GOTO:
	{
		GotoStmt* jump = make<GotoStmt>(StmtKind::Goto, start);
		next();
		if (!at_identifier())
		{
			fail();
			return nullptr;
		}
		jump->label = tok().identifier;
		next();
		return expect(TokenKind::OP_SEMICOLON) ? jump : nullptr;
	}
	case TokenKind::KW_RETURN:
	case TokenKind::KW_THROW:
		return read_statement(&Parser::parse_value_statement, nullptr);
	case TokenKind::KW_ASM:
		return parse_asm();
	case TokenKind::KW_CASE:
	{
		CaseStmt* label = make<CaseStmt>(StmtKind::Case, start);
		next();
		label->value = parse_expression();
		if (label->value == nullptr || !expect(TokenKind::OP_COLON))
			return nullptr;
		label->body = parse_statement();
		return label->body == nullptr ? nullptr : label;
	}
	case TokenKind::KW_DEFAULT:
	{
		LabeledStmt* label = make<LabeledStmt>(StmtKind::Default, start);
		label->label = nullptr;
		next();
		if (!expect(TokenKind::OP_COLON))
			return nullptr;
		label->body = parse_statement();
		return label->body == nullptr ? nullptr : label;
	}
	default:
		return parse_declaration_or_expression_statement(attributes);
	}
}

Stmt* Parser::parse_asm()
{
	// asm volatile? inline? ( string-literal [: outputs [: inputs
	// [: clobbers]]] ) ; -- `::` stands for two colons.
	AsmStmt* stmt = make<AsmStmt>(StmtKind::Asm, location());
	stmt->is_volatile = false;
	next();
	for (;;)
	{
		if (accept(TokenKind::KW_VOLATILE))
			stmt->is_volatile = true;
		else if (!accept(TokenKind::KW_INLINE))
			break;
	}
	if (!expect(TokenKind::OP_LPAREN) || !tok().is_literal() || (tok().flags & kStringLiteral) == 0)
	{
		fail();
		return nullptr;
	}
	stmt->text = tok().spelling;
	next();
	// Rare: collected locally rather than on the scratch stacks.
	std::vector<AsmOperand> outputs, inputs;
	std::vector<TokenSpelling> clobbers;
	uint32_t section = 0;
	uint32_t pending_colons = 0;
	for (;;)
	{
		if (pending_colons == 0)
		{
			if (accept(TokenKind::OP_COLON))
				pending_colons = 1;
			else if (accept(TokenKind::OP_COLON2))
				pending_colons = 2;
			else
				break;
		}
		--pending_colons;
		if (++section > 3)
		{
			fail();
			return nullptr;
		}
		if (pending_colons != 0 || at(TokenKind::OP_COLON) || at(TokenKind::OP_COLON2) || at(TokenKind::OP_RPAREN))
			continue;
		do
		{
			if (section == 3)
			{
				if (!tok().is_literal() || (tok().flags & kStringLiteral) == 0)
				{
					fail();
					return nullptr;
				}
				clobbers.push_back(tok().spelling);
				next();
				continue;
			}
			AsmOperand operand = AsmOperand();
			operand.location = location();
			if (accept(TokenKind::OP_LSQUARE))
			{
				if (!at_identifier())
				{
					fail();
					return nullptr;
				}
				operand.name = tok().identifier;
				next();
				if (!expect(TokenKind::OP_RSQUARE))
					return nullptr;
			}
			if (!tok().is_literal() || (tok().flags & kStringLiteral) == 0)
			{
				fail();
				return nullptr;
			}
			operand.constraint = tok().spelling;
			next();
			if (!expect(TokenKind::OP_LPAREN))
				return nullptr;
			operand.expression = parse_expression();
			if (operand.expression == nullptr || !expect(TokenKind::OP_RPAREN))
				return nullptr;
			(section == 1 ? outputs : inputs).push_back(operand);
		} while (accept(TokenKind::OP_COMMA));
	}
	if (!expect(TokenKind::OP_RPAREN) || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	stmt->outputs = Slice<AsmOperand>{
		outputs.empty() ? nullptr : arena_.copy_array(outputs.data(), static_cast<uint32_t>(outputs.size())),
		static_cast<uint32_t>(outputs.size())
	};
	stmt->inputs = Slice<AsmOperand>{
		inputs.empty() ? nullptr : arena_.copy_array(inputs.data(), static_cast<uint32_t>(inputs.size())),
		static_cast<uint32_t>(inputs.size())
	};
	stmt->clobbers = Slice<TokenSpelling>{
		clobbers.empty() ? nullptr : arena_.copy_array(clobbers.data(), static_cast<uint32_t>(clobbers.size())),
		static_cast<uint32_t>(clobbers.size())
	};
	return stmt;
}

Stmt* Parser::parse_value_statement(Attribute*)
{
	const uint32_t start = location();
	const bool is_return = at(TokenKind::KW_RETURN);
	ValueStmt* jump = make<ValueStmt>(is_return ? StmtKind::Return : StmtKind::Throw, start);
	jump->value = nullptr;
	next();
	if (!at(TokenKind::OP_SEMICOLON))
	{
		if (is_return)
			jump->value = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_expression();
		else
			jump->value = parse_assignment_expression();
		if (jump->value == nullptr)
			return nullptr;
	}
	return expect(TokenKind::OP_SEMICOLON) ? jump : nullptr;
}

bool Parser::statement_starts_declaration(bool& tentative)
{
	// Declarations win when both parses are possible (6.8): keywords that
	// only begin declarations commit to one; a type followed by `(` might be
	// a functional cast, so that declaration is tentative.
	tentative = false;
	const ParseToken token = tok();
	if (token.kind == ParseTokenKind::Simple)
	{
		switch (token.simple)
		{
		case TokenKind::KW_TYPEDEF:
		case TokenKind::KW_STATIC:
		case TokenKind::KW_EXTERN:
		case TokenKind::KW_REGISTER:
		case TokenKind::KW_THREAD_LOCAL:
		case TokenKind::KW_CONSTEXPR:
		case TokenKind::KW_MUTABLE:
		case TokenKind::KW_INLINE:
		case TokenKind::KW_VIRTUAL:
		case TokenKind::KW_EXPLICIT:
		case TokenKind::KW_FRIEND:
		case TokenKind::KW_CLASS:
		case TokenKind::KW_STRUCT:
		case TokenKind::KW_UNION:
		case TokenKind::KW_ENUM:
		case TokenKind::KW_NAMESPACE:
		case TokenKind::KW_USING:
		case TokenKind::KW_STATIC_ASSERT:
		case TokenKind::KW_TEMPLATE:
		case TokenKind::KW_CONST:
		case TokenKind::KW_VOLATILE:
		case TokenKind::KW_AUTO:
			return true;
		case TokenKind::KW_BOOL:
		case TokenKind::KW_CHAR:
		case TokenKind::KW_CHAR16_T:
		case TokenKind::KW_CHAR32_T:
		case TokenKind::KW_DOUBLE:
		case TokenKind::KW_FLOAT:
		case TokenKind::KW_INT:
		case TokenKind::KW_LONG:
		case TokenKind::KW_SHORT:
		case TokenKind::KW_SIGNED:
		case TokenKind::KW_UNSIGNED:
		case TokenKind::KW_VOID:
		case TokenKind::KW_WCHAR_T:
			if (at(TokenKind::OP_LBRACE, 1))
				return false;
			tentative = at(TokenKind::OP_LPAREN, 1);
			return true;
		case TokenKind::KW_TYPENAME:
		case TokenKind::KW_DECLTYPE:
			tentative = true;
			return true;
		case TokenKind::OP_COLON2:
			break;
		default:
			return false;
		}
	}
	else if (!token.is_identifier())
	{
		return false;
	}
	else if (at_gnu_decltype())
	{
		// GNU `__typeof__ (` and `__underlying_type (`, as decltype.
		tentative = true;
		return true;
	}
	Tentative probe(*this);
	NameInfo info;
	Name* name = parse_name(NameMode::Type, info);
	if (name == nullptr)
		return false;
	bool type = is_type(name, info);
	if (!type && info.category == NameCategory::None && at_identifier() && !at_identifier(final_) &&
		!at_identifier(override_))
		type = true;
	if (!type || at(TokenKind::OP_LBRACE))
		return false;
	tentative = at(TokenKind::OP_LPAREN);
	return true;
}

Stmt* Parser::parse_declaration_or_expression_statement(Attribute* attributes)
{
	return read_statement(&Parser::parse_statement_reading, attributes);
}

Stmt* Parser::read_statement(StatementReader reader, Attribute* attributes)
{
	// A statement (with no substatement, or a selection or iteration
	// statement whose head may declare such a name's type, see
	// control_head_qualified; its substatements' statements are read in
	// their own right) whose lookups met names the specializations of their
	// templates give different categories is read again once per other
	// category those names have (the ones that have it
	// take it, see resolve_component). A reading is kept when it parses, takes
	// the tokens the first kept one took, and took the first such name its
	// reading records with a category no kept one did; several are kept for
	// semantic analysis to choose by that name (AmbiguousStmt). The tokens
	// stay retained for the other readings.
	if (!reads_alternatives())
		return (this->*reader)(attributes);
	const bool outer_active = alternatives_active_;
	alternatives_active_ = true;
	const uint32_t start_location = location();
	tokens_.retain();
	const uint32_t start = tokens_.position();
	const ConflictScope outer(conflicts_);
	std::vector<std::pair<uint32_t, CompoundStmt*>> shared;
	std::vector<std::pair<uint32_t, CompoundStmt*>>* const outer_shared = shared_bodies_;
	shared_bodies_ = &shared;
	Stmt* readings[AmbiguousStmt::kMaxReadings];
	ReadingCategory categories[AmbiguousStmt::kMaxReadings];
	uint32_t count = 0;
	uint32_t end = start;
	StatementConflict kept = StatementConflict{ nullptr, 0, NameCategory::None, 0 };
	uint8_t seen = 0;
	for (unsigned take = 0; take <= static_cast<unsigned>(NameCategory::Namespace); ++take)
	{
		// The first reading takes each name's found category.
		if (take != 0 && (seen & CategoryBit(static_cast<NameCategory>(take))) == 0)
			continue;
		tokens_.rewind(start);
		conflicts_.take = static_cast<NameCategory>(take);
		conflicts_.statement.name = nullptr;
		conflicts_.seen = 0;
		Tentative attempt(*this);
		Stmt* reading = (this->*reader)(attributes);
		const StatementConflict conflict = conflicts_.statement;
		const ReadingCategory category = ReadingOf(conflict.category);
		if (take == 0)
			seen = conflicts_.seen;
		bool repeated = false;
		for (uint32_t i = 0; i < count; ++i)
			repeated = repeated || categories[i] == category;
		if (reading == nullptr ||
			(count != 0 && (tokens_.position() != end || kept.name == nullptr || conflict.name == nullptr || repeated)))
			continue;
		conflicts_.take = NameCategory::None;
		attempt.commit();
		if (count == 0)
			kept = conflict;
		readings[count] = reading;
		categories[count++] = category;
		end = tokens_.position();
	}
	shared_bodies_ = outer_shared;
	alternatives_active_ = outer_active;
	tokens_.rewind(end);
	tokens_.release();
	if (count <= 1)
		return count != 0 ? readings[0] : nullptr;
	AmbiguousStmt* ambiguous = make<AmbiguousStmt>(StmtKind::Ambiguous, start_location);
	for (uint32_t i = 0; i < count; ++i)
	{
		ambiguous->readings[i] = readings[i];
		ambiguous->categories[i] = categories[i];
	}
	ambiguous->count = static_cast<uint8_t>(count);
	ambiguous->name = kept.name;
	ambiguous->component = kept.component;
	return ambiguous;
}

Stmt* Parser::parse_control_reading(Attribute*)
{
	// A selection or iteration statement whose head is read each way (see
	// parse_statement).
	switch (tok().simple)
	{
	case TokenKind::KW_IF:
		return parse_if();
	case TokenKind::KW_SWITCH:
		return parse_loop_or_switch(StmtKind::Switch);
	case TokenKind::KW_WHILE:
		return parse_loop_or_switch(StmtKind::While);
	default:
		return parse_for();
	}
}

Stmt* Parser::parse_statement_reading(Attribute* attributes)
{
	const uint32_t start = location();
	bool tentative = false;
	if (statement_starts_declaration(tentative))
	{
		if (!tentative)
		{
			DeclarationStmt* stmt = make<DeclarationStmt>(StmtKind::Declaration, start);
			stmt->declaration = parse_declaration(DeclContext::Block, attributes);
			return stmt->declaration == nullptr ? nullptr : stmt;
		}
		Tentative attempt(*this);
		DeclarationStmt* stmt = make<DeclarationStmt>(StmtKind::Declaration, start);
		stmt->declaration = parse_declaration(DeclContext::Block, attributes);
		if (stmt->declaration != nullptr)
		{
			attempt.commit();
			return stmt;
		}
	}
	return parse_expression_statement();
}

Stmt* Parser::parse_expression_statement()
{
	ExpressionStmt* stmt = make<ExpressionStmt>(StmtKind::Expression, location());
	stmt->expression = nullptr;
	if (accept(TokenKind::OP_SEMICOLON))
		return stmt;
	stmt->expression = parse_expression();
	if (stmt->expression == nullptr || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	return stmt;
}

bool Parser::parse_condition(Condition& condition)
{
	condition.expression = nullptr;
	condition.declaration = nullptr;
	// An attribute-specifier-seq begins a condition declaration (6.4p1;
	// the attributes are skipped).
	Attribute* attributes = nullptr;
	const bool attributed = at_attribute();
	if (attributed && !collect_attributes(attributes))
		return false;
	bool tentative = false;
	if (attributed || statement_starts_declaration(tentative))
	{
		// A condition declaration needs its initializer (6.4p1).
		Tentative attempt(*this);
		ListFrame<DeclSpecifier> specifiers(specs_);
		SpecifierInfo info;
		if (parse_decl_specifier_seq(specifiers, info) && info.has_type)
		{
			Declarator* declarator = parse_declarator(DeclaratorMode::Named);
			if (declarator != nullptr && (at(TokenKind::OP_ASS) || at(TokenKind::OP_LBRACE)))
			{
				declare_declarator(declarator, info);
				Initializer* initializer = parse_initializer(false);
				if (initializer != nullptr)
				{
					ConditionDeclaration* declaration = make<ConditionDeclaration>();
					declaration->specifiers = specifiers.take(arena_);
					declaration->declarator = declarator;
					declaration->initializer = initializer;
					condition.declaration = declaration;
					attempt.commit();
					return true;
				}
			}
		}
	}
	condition.expression = parse_expression();
	return condition.expression != nullptr;
}

namespace {

// The tokens a selection statement's scan for an init-statement's `;`
// looks ahead (see Parser::parse_selection_head).
const uint32_t kSelectionScan = 1024;

} // namespace

bool Parser::parse_selection_head(Stmt*& init, Condition& condition)
{
	// C++17's `if ( init-statement condition )` and switch alike (an
	// extension here, as hosted headers use it): an expression-statement, a
	// simple-declaration or (C++23) an alias-declaration, recognized by a
	// `;` before the closing parenthesis. The statement is then as if
	// `{ init-statement if ( condition ) ... }`. The scan for the `;` looks
	// at most kSelectionScan tokens ahead, which the token window holds
	// until they are parsed (a lambda's body in a condition would otherwise
	// be held whole). Past that a declaration is tried as the
	// init-statement (it is one when a `;` ends it), and the tokens are
	// otherwise parsed as the condition and, if a `;` follows, taken as the
	// init-statement (an expression).
	init = nullptr;
	int depth = 0;
	bool found = false;
	bool stopped = false;
	for (uint32_t ahead = 0;; ++ahead)
	{
		const ParseToken token = tok(ahead);
		if (token.is_end() || (depth == 0 && (token.is(TokenKind::OP_RPAREN) || token.is(TokenKind::OP_RBRACE))))
			break;
		if (depth == 0 && token.is(TokenKind::OP_SEMICOLON))
		{
			found = true;
			break;
		}
		if (ahead == kSelectionScan)
		{
			stopped = true;
			break;
		}
		if (token.is(TokenKind::OP_LPAREN) || token.is(TokenKind::OP_LSQUARE) || token.is(TokenKind::OP_LBRACE))
			++depth;
		else if (token.is(TokenKind::OP_RPAREN) || token.is(TokenKind::OP_RSQUARE) || token.is(TokenKind::OP_RBRACE))
			--depth;
	}
	bool declaration = false;
	const uint64_t position = tokens_.absolute_position();
	const auto failed = std::lower_bound(failed_selection_inits_.begin(), failed_selection_inits_.end(), position);
	const bool tried = failed != failed_selection_inits_.end() && *failed == position;
	if (!found && stopped && !tried &&
		(at_attribute() || at(TokenKind::KW_USING) || statement_starts_declaration(declaration)))
	{
		{
			Tentative attempt(*this);
			const uint32_t start = location();
			Attribute* attributes = nullptr;
			if (!at_attribute() || collect_attributes(attributes))
			{
				DeclarationStmt* statement = make<DeclarationStmt>(StmtKind::Declaration, start);
				statement->declaration = parse_declaration(DeclContext::Block, attributes);
				if (statement->declaration != nullptr)
				{
					attempt.commit();
					init = statement;
					return parse_condition(condition);
				}
			}
		}
		failed_selection_inits_.insert(
			std::lower_bound(failed_selection_inits_.begin(), failed_selection_inits_.end(), position), position);
	}
	if (!found)
	{
		const uint32_t start = location();
		if (!parse_condition(condition))
			return false;
		if (!stopped || !at(TokenKind::OP_SEMICOLON))
			return true;
		if (condition.expression != nullptr)
		{
			ExpressionStmt* statement = make<ExpressionStmt>(StmtKind::Expression, start);
			statement->expression = condition.expression;
			init = statement;
		}
		else
		{
			SimpleDecl* decl = make<SimpleDecl>(DeclKind::Simple, start);
			decl->specifiers = condition.declaration->specifiers;
			InitDeclarator* declarators = arena_.allocate_array<InitDeclarator>(1);
			declarators[0].declarator = condition.declaration->declarator;
			declarators[0].initializer = condition.declaration->initializer;
			decl->declarators = Slice<InitDeclarator>{ declarators, 1 };
			DeclarationStmt* statement = make<DeclarationStmt>(StmtKind::Declaration, start);
			statement->declaration = decl;
			init = statement;
		}
		next();
		return parse_condition(condition);
	}
	if (at(TokenKind::KW_USING))
	{
		const uint32_t start = location();
		Decl* alias = parse_using();
		if (alias == nullptr || alias->kind != DeclKind::Alias)
			return false;
		DeclarationStmt* declaration = make<DeclarationStmt>(StmtKind::Declaration, start);
		declaration->declaration = alias;
		init = declaration;
		return parse_condition(condition);
	}
	Attribute* attributes = nullptr;
	if (at_attribute() && !collect_attributes(attributes))
		return false;
	init = at(TokenKind::OP_SEMICOLON) ? parse_expression_statement()
									   : parse_declaration_or_expression_statement(attributes);
	return init != nullptr && parse_condition(condition);
}

bool Parser::coroutine_operator(const Identifier* keyword)
{
	// C++20's co_await, co_yield and co_return are identifiers in C++11;
	// hosted headers' coroutine templates use them as operators, which they
	// are here where no declaration of the name is visible and an operand
	// (or co_return's `;`) follows (at_coroutine_keyword has checked that
	// the parse is hosted and the token is the keyword).
	if (scopes_.lookup(keyword, false).category != NameCategory::None)
		return false;
	const ParseToken following = tok(1);
	return !following.is(TokenKind::OP_RPAREN) && !following.is(TokenKind::OP_COMMA) &&
		!following.is(TokenKind::OP_ASS) && !following.is(TokenKind::OP_RSQUARE) &&
		(keyword == coroutine_keywords_[2] || !following.is(TokenKind::OP_SEMICOLON));
}

Expr* Parser::coroutine_operation(const Identifier* keyword, Expr* operand, uint32_t location)
{
	// A call of an unspellable name (` co_await` and so on), which only an
	// uninstantiated template's definition may hold: coroutines (C++20) are
	// outside PA29, so the analysis rejects any other.
	const std::string spelling = std::string(" ") + keyword->spelling;
	IdExpr* callee = make<IdExpr>(ExprKind::Id, location);
	callee->name = make_simple_name(
		identifiers_.intern(spelling.data(), static_cast<uint32_t>(spelling.size())), location);
	CallExpr* call = make<CallExpr>(ExprKind::Call, location);
	call->callee = callee;
	Expr** arguments = operand != nullptr ? arena_.allocate_array<Expr*>(1) : nullptr;
	if (operand != nullptr)
		arguments[0] = operand;
	call->arguments = Slice<Expr*>{ arguments, operand != nullptr ? 1u : 0u };
	return call;
}

Stmt* Parser::with_selection_init(Stmt* init, Stmt* stmt, uint32_t start)
{
	if (init == nullptr || stmt == nullptr)
		return stmt;
	CompoundStmt* block = make<CompoundStmt>(StmtKind::Compound, start);
	block->selection_init = true;
	Stmt** items = arena_.allocate_array<Stmt*>(2);
	items[0] = init;
	items[1] = stmt;
	block->items = Slice<Stmt*>{ items, 2 };
	return block;
}

Stmt* Parser::parse_if()
{
	const uint32_t start = location();
	IfStmt* stmt = make<IfStmt>(StmtKind::If, start);
	stmt->else_branch = nullptr;
	next();
	stmt->is_constexpr = accept(TokenKind::KW_CONSTEXPR);
	Scope* saved = scopes_.current();
	scopes_.push(ScopeKind::Block, nullptr);
	Stmt* init = nullptr;
	bool parsed = expect(TokenKind::OP_LPAREN) && parse_selection_head(init, stmt->condition) &&
		expect(TokenKind::OP_RPAREN);
	if (parsed)
	{
		stmt->then_branch = parse_statement();
		parsed = stmt->then_branch != nullptr;
	}
	if (parsed && accept(TokenKind::KW_ELSE))
	{
		stmt->else_branch = parse_statement();
		parsed = stmt->else_branch != nullptr;
	}
	scopes_.set_current(saved);
	return parsed ? with_selection_init(init, stmt, start) : nullptr;
}

Stmt* Parser::parse_loop_or_switch(StmtKind kind)
{
	const uint32_t start = location();
	ConditionalLoopStmt* stmt = make<ConditionalLoopStmt>(kind, start);
	next();
	Scope* saved = scopes_.current();
	scopes_.push(ScopeKind::Block, nullptr);
	Stmt* init = nullptr;
	bool parsed = expect(TokenKind::OP_LPAREN) &&
		(kind == StmtKind::Switch ? parse_selection_head(init, stmt->condition) : parse_condition(stmt->condition)) &&
		expect(TokenKind::OP_RPAREN);
	if (parsed)
	{
		stmt->body = parse_statement();
		parsed = stmt->body != nullptr;
	}
	scopes_.set_current(saved);
	return parsed ? with_selection_init(init, stmt, start) : nullptr;
}

Stmt* Parser::parse_do()
{
	DoStmt* stmt = make<DoStmt>(StmtKind::Do, location());
	next();
	stmt->body = parse_statement();
	if (stmt->body == nullptr || !expect(TokenKind::KW_WHILE) || !expect(TokenKind::OP_LPAREN))
		return nullptr;
	stmt->condition = parse_expression();
	if (stmt->condition == nullptr || !expect(TokenKind::OP_RPAREN) || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	return stmt;
}

Stmt* Parser::parse_for()
{
	const uint32_t start = location();
	next();
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	Scope* saved = scopes_.current();
	scopes_.push(ScopeKind::Block, nullptr);
	Stmt* result = nullptr;
	// A for-range-declaration or the for-init-statement's declaration may
	// begin with an attribute-specifier-seq (6.5.4p1, 6.5p1).
	Attribute* attributes = nullptr;
	if (at_attribute() && !collect_attributes(attributes))
	{
		scopes_.set_current(saved);
		return nullptr;
	}
	bool tentative = false;
	if (statement_starts_declaration(tentative))
	{
		// for ( for-range-declaration : for-range-initializer ) (6.5.4).
		Tentative attempt(*this);
		ListFrame<DeclSpecifier> specifiers(specs_);
		SpecifierInfo info;
		const bool specified = parse_decl_specifier_seq(specifiers, info) && info.has_type;
		if (specified && at_structured_binding())
		{
			// for ( specifiers ref? [ names ] : range ): the object e is the
			// for-range-declaration's, and the names are declared first in
			// the body (C++17's 11.5), by a declaration naming e without its
			// initializer.
			TokenKind ref;
			const Identifier* hidden = nullptr;
			std::vector<Declarator*> names;
			if (parse_binding_names(ref, hidden, names) && accept(TokenKind::OP_COLON))
			{
				attempt.commit();
				RangeForStmt* stmt = make<RangeForStmt>(StmtKind::RangeFor, start);
				stmt->specifiers = specifiers.take(arena_);
				stmt->declarator = binding_declarator(hidden, ref, start);
				InitDeclarator object = InitDeclarator();
				object.declarator = binding_declarator(hidden, ref, start);
				object.initializer = nullptr;
				SimpleDecl* bindings = make<SimpleDecl>(DeclKind::Simple, start);
				bindings->specifiers = stmt->specifiers;
				bindings->attributes = nullptr;
				bindings->declarators = binding_declarators(names, info, object);
				bindings->decomposition = true;
				stmt->range = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_expression();
				Stmt* body = stmt->range != nullptr && expect(TokenKind::OP_RPAREN) ? parse_statement() : nullptr;
				if (body != nullptr)
				{
					DeclarationStmt* declaration = make<DeclarationStmt>(StmtKind::Declaration, start);
					declaration->declaration = bindings;
					CompoundStmt* block = make<CompoundStmt>(StmtKind::Compound, start);
					Stmt** items = arena_.allocate_array<Stmt*>(2);
					items[0] = declaration;
					items[1] = body;
					block->items = Slice<Stmt*>{ items, 2 };
					stmt->body = block;
					result = stmt;
				}
				scopes_.set_current(saved);
				return result;
			}
		}
		else if (specified)
		{
			Declarator* declarator = parse_declarator(DeclaratorMode::Named);
			if (declarator != nullptr && accept(TokenKind::OP_COLON))
			{
				attempt.commit();
				RangeForStmt* stmt = make<RangeForStmt>(StmtKind::RangeFor, start);
				stmt->specifiers = specifiers.take(arena_);
				stmt->declarator = declarator;
				declare_declarator(declarator, info);
				stmt->range = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_expression();
				if (stmt->range != nullptr && expect(TokenKind::OP_RPAREN))
				{
					stmt->body = parse_statement();
					if (stmt->body != nullptr)
						result = stmt;
				}
				scopes_.set_current(saved);
				return result;
			}
		}
	}
	ForStmt* stmt = make<ForStmt>(StmtKind::For, start);
	stmt->init_declaration = nullptr;
	stmt->init_expression = nullptr;
	stmt->condition = nullptr;
	stmt->iteration = nullptr;
	bool parsed = true;
	// A for-init-statement with several readings (see
	// parse_declaration_or_expression_statement) precedes the loop in a
	// block, as 6.5.3p1's equivalent has it.
	Stmt* readings = nullptr;
	if (!accept(TokenKind::OP_SEMICOLON))
	{
		Stmt* init = parse_declaration_or_expression_statement(attributes);
		if (init == nullptr)
			parsed = false;
		else if (init->kind == StmtKind::Declaration)
			stmt->init_declaration = static_cast<DeclarationStmt*>(init)->declaration;
		else if (init->kind == StmtKind::Ambiguous)
			readings = init;
		else
			stmt->init_expression = static_cast<ExpressionStmt*>(init)->expression;
	}
	if (parsed && !at(TokenKind::OP_SEMICOLON))
	{
		stmt->condition = make<Condition>();
		parsed = parse_condition(*stmt->condition);
	}
	parsed = parsed && expect(TokenKind::OP_SEMICOLON);
	if (parsed && !at(TokenKind::OP_RPAREN))
	{
		stmt->iteration = parse_expression();
		parsed = stmt->iteration != nullptr;
	}
	parsed = parsed && expect(TokenKind::OP_RPAREN);
	if (parsed)
	{
		stmt->body = parse_statement();
		parsed = stmt->body != nullptr;
	}
	scopes_.set_current(saved);
	if (!parsed)
		return nullptr;
	if (readings == nullptr)
		return stmt;
	CompoundStmt* block = make<CompoundStmt>(StmtKind::Compound, start);
	Stmt** items = arena_.allocate_array<Stmt*>(2);
	items[0] = readings;
	items[1] = stmt;
	block->items = Slice<Stmt*>{ items, 2 };
	return block;
}

Stmt* Parser::parse_try()
{
	TryStmt* stmt = make<TryStmt>(StmtKind::Try, location());
	next();
	stmt->body = parse_compound_statement(true);
	if (stmt->body == nullptr)
		return nullptr;
	ListFrame<Handler> handlers(handlers_);
	if (!parse_handlers(handlers))
		return nullptr;
	stmt->handlers = handlers.take(arena_);
	return stmt;
}

bool Parser::parse_handler(Handler& handler)
{
	handler = Handler();
	if (!expect(TokenKind::KW_CATCH) || !expect(TokenKind::OP_LPAREN))
		return false;
	Scope* saved = scopes_.current();
	scopes_.push(ScopeKind::Block, nullptr);
	bool parsed = true;
	if (accept(TokenKind::OP_DOTS))
	{
		handler.exception.ellipsis = true;
	}
	else
	{
		ListFrame<DeclSpecifier> specifiers(specs_);
		SpecifierInfo info;
		parsed = parse_decl_specifier_seq(specifiers, info) && info.has_type;
		if (parsed)
		{
			handler.exception.specifiers = specifiers.take(arena_);
			Declarator* declarator = nullptr;
			{
				Tentative attempt(*this);
				declarator = parse_declarator(DeclaratorMode::Parameter);
				if (declarator != nullptr)
					attempt.commit();
			}
			if (declarator == nullptr)
				declarator = parse_declarator(DeclaratorMode::Abstract);
			parsed = declarator != nullptr;
			if (parsed && !declarator_is_empty(declarator))
			{
				handler.exception.declarator = declarator;
				declare_declarator(declarator, info);
			}
		}
		else
		{
			fail();
		}
	}
	parsed = parsed && expect(TokenKind::OP_RPAREN);
	if (parsed)
	{
		handler.body = parse_compound_statement(false);
		parsed = handler.body != nullptr;
	}
	scopes_.set_current(saved);
	return parsed;
}

} // namespace syntax
