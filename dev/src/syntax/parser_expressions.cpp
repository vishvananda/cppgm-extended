#include "syntax/parser.h"

#include <cstring>

#include "lex/gnu_builtins.h"
#include "lex/identifier_table.h"

namespace syntax {

namespace {

// A call's callee that is a reserved `__builtin_` name no registry holds: a
// GNU inline wrapper of a builtin this compiler lacks may be left
// unanalyzed while unused (see Analyzer::define_function).
const Identifier* ReservedBuiltinCallee(const Name& name)
{
	if (name.components.size != 1 || name.components[0].kind != NameComponentKind::Identifier)
		return nullptr;
	const Identifier* identifier = name.components[0].identifier;
	if (std::strncmp(identifier->spelling, "__builtin_", 10) == 0 &&
		!IsProvidedBuiltin(identifier->spelling, static_cast<uint32_t>(std::strlen(identifier->spelling))))
		return identifier;
	return nullptr;
}

const int kLogicalOrPrecedence = 3;

bool IsSimpleTypeKeyword(TokenKind kind)
{
	switch (kind)
	{
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
	case TokenKind::KW_AUTO:
		return true;
	default:
		return false;
	}
}

// A type-id whose declarator is a function declarator on a named type, as
// in `C()`: in sizeof and typeid such an operand is taken as a functional
// cast expression (a function type would be ill-formed there, 5.3.3p1).
bool IsNamedFunctionType(const TypeId* type)
{
	const Declarator* declarator = type->declarator;
	if (declarator == nullptr || !declarator->pointers.empty() || declarator->nested != nullptr ||
		declarator->suffixes.empty() || declarator->suffixes[0].kind != DeclaratorSuffixKind::Function)
		return false;
	for (const DeclSpecifier& specifier : type->specifiers)
	{
		if (specifier.kind == DeclSpecKind::Name)
			return true;
	}
	return false;
}

} // namespace

Expr* Parser::parse_expression()
{
	return finish_expression(parse_assignment_expression());
}

Expr* Parser::finish_expression(Expr* left)
{
	// Comma operators after the first assignment-expression (a comma fold
	// leaves its `, ...` to its parentheses).
	while (left != nullptr && at(TokenKind::OP_COMMA) && !at(TokenKind::OP_DOTS, 1))
	{
		BinaryExpr* comma = make<BinaryExpr>(ExprKind::Binary, left->location);
		comma->op = TokenKind::OP_COMMA;
		comma->alternative = false;
		next();
		comma->left = left;
		comma->right = parse_assignment_expression();
		if (comma->right == nullptr)
			return nullptr;
		left = comma;
	}
	return left;
}

Expr* Parser::parse_initializer_clause()
{
	if (at(TokenKind::OP_LBRACE))
		return parse_braced_init_list();
	return parse_assignment_expression();
}

Expr* Parser::parse_assignment_expression()
{
	if (at(TokenKind::KW_THROW))
	{
		ThrowExpr* expr = make<ThrowExpr>(ExprKind::Throw, location());
		next();
		expr->operand = nullptr;
		if (at_cast_operand_start())
		{
			expr->operand = parse_assignment_expression();
			if (expr->operand == nullptr)
				return nullptr;
		}
		return expr;
	}
	if (at_coroutine_keyword(coroutine_keywords_[1]))
	{
		const uint32_t start = location();
		next();
		Expr* operand = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_assignment_expression();
		return operand == nullptr ? nullptr : coroutine_operation(coroutine_keywords_[1], operand, start);
	}
	return finish_assignment_expression(parse_binary(kLogicalOrPrecedence));
}

Expr* Parser::continue_assignment_expression(Expr* primary)
{
	Expr* postfix = parse_postfix_suffixes(primary);
	if (postfix == nullptr)
		return nullptr;
	return finish_assignment_expression(parse_binary_rest(postfix, kLogicalOrPrecedence));
}

Expr* Parser::continue_expression(Expr* primary)
{
	return finish_expression(continue_assignment_expression(primary));
}

Expr* Parser::finish_assignment_expression(Expr* left)
{
	// A logical-or-expression, then `?:` or an assignment operator.
	if (left == nullptr)
		return nullptr;
	if (at(TokenKind::OP_QMARK))
	{
		ConditionalExpr* expr = make<ConditionalExpr>(ExprKind::Conditional, left->location);
		next();
		expr->condition = left;
		expr->then_expr = parse_expression();
		if (expr->then_expr == nullptr || !expect(TokenKind::OP_COLON))
			return nullptr;
		expr->else_expr = parse_assignment_expression();
		return expr->else_expr == nullptr ? nullptr : expr;
	}
	if (tok().kind == ParseTokenKind::Simple && IsAssignmentOperator(tok().simple) && !at(TokenKind::OP_DOTS, 1))
	{
		BinaryExpr* expr = make<BinaryExpr>(ExprKind::Assign, left->location);
		expr->op = tok().simple;
		expr->alternative = IsAlternativeSpelling(tok().spelling.text, tok().spelling.length);
		next();
		expr->left = left;
		expr->right = parse_initializer_clause();
		return expr->right == nullptr ? nullptr : expr;
	}
	return left;
}

int Parser::binary_precedence(TokenKind& op, uint32_t& width)
{
	const ParseToken token = tok();
	width = 1;
	if (token.kind != ParseTokenKind::Simple)
		return 0;
	op = token.simple;
	switch (token.simple)
	{
	case TokenKind::OP_DOTSTAR:
	case TokenKind::OP_ARROWSTAR:
		return 13;
	case TokenKind::OP_STAR:
	case TokenKind::OP_DIV:
	case TokenKind::OP_MOD:
		return 12;
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
		return 11;
	case TokenKind::OP_LSHIFT:
		return 10;
	case TokenKind::OP_GT:
		// In a template argument list the first non-nested `>` (or `>>`)
		// closes the list (14.2p3).
		if (angle_)
			return 0;
		if ((token.flags & kFirstShiftHalf) != 0 && (tok(1).flags & kSecondShiftHalf) != 0)
		{
			op = TokenKind::OP_RSHIFT;
			width = 2;
			return 10;
		}
		return 9;
	case TokenKind::OP_LT:
	case TokenKind::OP_LE:
	case TokenKind::OP_GE:
		return 9;
	case TokenKind::OP_EQ:
	case TokenKind::OP_NE:
		return 8;
	case TokenKind::OP_AMP:
		return 7;
	case TokenKind::OP_XOR:
		return 6;
	case TokenKind::OP_BOR:
		return 5;
	case TokenKind::OP_LAND:
		return 4;
	case TokenKind::OP_LOR:
		return 3;
	default:
		return 0;
	}
}

Expr* Parser::parse_binary(int min_precedence)
{
	Expr* left = parse_cast_expression();
	if (left == nullptr)
		return nullptr;
	return parse_binary_rest(left, min_precedence);
}

Expr* Parser::parse_binary_rest(Expr* left, int min_precedence)
{
	for (;;)
	{
		TokenKind op = TokenKind::OP_COMMA;
		uint32_t width = 1;
		const int precedence = binary_precedence(op, width);
		// A fold-expression's operator (its `...` follows) ends the operand.
		if (precedence == 0 || precedence < min_precedence || at(TokenKind::OP_DOTS, width))
			return left;
		BinaryExpr* expr = make<BinaryExpr>(ExprKind::Binary, left->location);
		expr->op = op;
		expr->alternative = IsAlternativeSpelling(tok().spelling.text, tok().spelling.length);
		for (uint32_t i = 0; i < width; ++i)
			next();
		expr->left = left;
		expr->right = parse_binary(precedence + 1);
		if (expr->right == nullptr)
			return nullptr;
		left = expr;
	}
}

bool Parser::at_cast_operand_start()
{
	const ParseToken token = tok();
	if (token.is_identifier() || token.is_literal())
		return true;
	if (token.kind != ParseTokenKind::Simple)
		return false;
	switch (token.simple)
	{
	case TokenKind::OP_LPAREN:
	case TokenKind::OP_COLON2:
	case TokenKind::OP_COMPL:
	case TokenKind::OP_LNOT:
	case TokenKind::OP_MINUS:
	case TokenKind::OP_PLUS:
	case TokenKind::OP_STAR:
	case TokenKind::OP_AMP:
	case TokenKind::OP_INC:
	case TokenKind::OP_DEC:
	case TokenKind::OP_LSQUARE:
	case TokenKind::KW_TRUE:
	case TokenKind::KW_FALSE:
	case TokenKind::KW_NULLPTR:
	case TokenKind::KW_THIS:
	case TokenKind::KW_SIZEOF:
	case TokenKind::KW_ALIGNOF:
	case TokenKind::KW_NOEXCEPT:
	case TokenKind::KW_NEW:
	case TokenKind::KW_DELETE:
	case TokenKind::KW_TYPEID:
	case TokenKind::KW_STATIC_CAST:
	case TokenKind::KW_DYNAMIC_CAST:
	case TokenKind::KW_REINTERPET_CAST:
	case TokenKind::KW_CONST_CAST:
	case TokenKind::KW_TYPENAME:
	case TokenKind::KW_DECLTYPE:
	case TokenKind::KW_OPERATOR:
		return true;
	default:
		return IsSimpleTypeKeyword(token.simple);
	}
}

Expr* Parser::parse_cast_expression()
{
	NestingGuard nesting(*this);
	if (at(TokenKind::OP_LPAREN))
		return parse_parenthesized_or_cast();
	return parse_unary_expression();
}

Expr* Parser::parenthesized_from_name(uint32_t start)
{
	// At `(` name: a C-style cast when the name starts a type-id followed by
	// `)` and an operand, else a parenthesized expression that starts with
	// the name; the name is parsed once either way.
	next();
	const bool saved_angle = angle_;
	angle_ = false;
	const uint32_t name_start = location();
	NameInfo info;
	Name* name = parse_name(NameMode::Expression, info);
	if (name == nullptr)
		return nullptr;
	if (is_type(name, info))
	{
		Tentative tentative(*this);
		TypeId* type = parse_type_id_from_name(name, info, name_start);
		if (type != nullptr && at(TokenKind::OP_RPAREN) && at_compound_literal(type))
		{
			tentative.commit();
			angle_ = saved_angle;
			return compound_literal(type, start);
		}
		if (type != nullptr && accept(TokenKind::OP_RPAREN) && at_cast_operand_start())
		{
			tentative.commit();
			angle_ = saved_angle;
			CastExpr* cast = make<CastExpr>(ExprKind::Cast, start);
			cast->cast = TokenKind::OP_LPAREN;
			cast->type = type;
			cast->operand = parse_cast_expression();
			return cast->operand == nullptr ? nullptr : cast;
		}
	}
	if (abandoning_lists_)
		return nullptr;
	Expr* primary = primary_from_name(name, info, name_start);
	ParenExpr* paren = make<ParenExpr>(ExprKind::Paren, start);
	paren->inner = primary == nullptr ? nullptr : continue_expression(primary);
	if (paren->inner != nullptr && at_fold_operator())
	{
		Expr* fold = parse_fold_rest(start, paren->inner);
		angle_ = saved_angle;
		return fold;
	}
	angle_ = saved_angle;
	if (paren->inner == nullptr || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	return paren;
}

bool Parser::at_fold_operator()
{
	// A fold-operator (C++17's 5.1.6) with `...` after it.
	if (tok().kind != ParseTokenKind::Simple)
		return false;
	TokenKind op = TokenKind::OP_COMMA;
	uint32_t width = 1;
	const bool saved_angle = angle_;
	angle_ = false;
	const bool binary = binary_precedence(op, width) != 0;
	angle_ = saved_angle;
	return (binary || at(TokenKind::OP_COMMA) || IsAssignmentOperator(tok().simple)) && at(TokenKind::OP_DOTS, width);
}

Expr* Parser::parse_fold_rest(uint32_t start, Expr* left)
{
	// At `op ...` after `( left`: a right fold of the pattern `left`, or,
	// with `op operand` after the `...`, a binary fold of `left` and
	// `operand`. Both operands are cast-expressions (C++17's
	// [expr.prim.fold]): not a binary, assignment, conditional or throw
	// expression unparenthesized.
	if (left->kind == ExprKind::Binary || left->kind == ExprKind::Assign || left->kind == ExprKind::Conditional ||
		left->kind == ExprKind::Throw)
	{
		fail();
		return nullptr;
	}
	FoldExpr* fold = make<FoldExpr>(ExprKind::Fold, start);
	TokenKind op = TokenKind::OP_COMMA;
	uint32_t width = 1;
	if (binary_precedence(op, width) == 0)
		op = tok().simple;
	fold->op = op;
	fold->pattern = left;
	fold->init = nullptr;
	fold->right = true;
	for (uint32_t i = 0; i < width + 1; ++i)
		next();
	if (!at(TokenKind::OP_RPAREN))
	{
		TokenKind second = TokenKind::OP_COMMA;
		uint32_t second_width = 1;
		if (binary_precedence(second, second_width) == 0)
			second = tok().simple;
		if (second != op)
		{
			fail();
			return nullptr;
		}
		for (uint32_t i = 0; i < second_width; ++i)
			next();
		Expr* operand = parse_cast_expression();
		if (operand == nullptr)
			return nullptr;
		// `( E op ... op E2 )`: which operand is the pattern -- the one
		// naming an unexpanded pack -- is known where it is expanded.
		fold->init = operand;
	}
	return expect(TokenKind::OP_RPAREN) ? fold : nullptr;
}

Expr* Parser::parse_left_fold(uint32_t start)
{
	// `( ... op pattern )`.
	FoldExpr* fold = make<FoldExpr>(ExprKind::Fold, start);
	next();
	next();
	TokenKind op = TokenKind::OP_COMMA;
	uint32_t width = 1;
	if (binary_precedence(op, width) == 0)
		op = tok().simple;
	fold->op = op;
	fold->init = nullptr;
	fold->right = false;
	for (uint32_t i = 0; i < width; ++i)
		next();
	fold->pattern = parse_cast_expression();
	if (fold->pattern == nullptr || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	return fold;
}

bool Parser::at_compound_literal(const TypeId* type)
{
	// At `)` of `( type-id )`, `{` after it: a GNU compound literal of a
	// type-id that is a simple-type-specifier alone (hosted compilations).
	const Declarator* declarator = type->declarator;
	return hosted_ && at(TokenKind::OP_LBRACE, 1) &&
		(declarator == nullptr ||
			(declarator->pointers.empty() && declarator->nested == nullptr && declarator->suffixes.empty()));
}

Expr* Parser::compound_literal(const TypeId* type, uint32_t start)
{
	// `( T ) { list }`: GCC's C++ compound literal, a temporary as `T{list}`
	// (5.2.3p3) is.
	next();
	FunctionalCastExpr* literal = make<FunctionalCastExpr>(ExprKind::FunctionalCast, start);
	literal->type = type->specifiers;
	literal->arguments = Slice<Expr*>{ nullptr, 0 };
	literal->braced = parse_braced_init_list();
	return literal->braced == nullptr ? nullptr : literal;
}

Expr* Parser::parse_parenthesized_or_cast()
{
	// `(` type-id `)` cast-expression (5.4), when the parenthesized tokens
	// are a type-id and an operand follows; otherwise a parenthesized
	// expression.
	const uint32_t start = location();
	if (at_name_identifier(1) || (at(TokenKind::OP_COLON2, 1) && at_identifier(2)))
	{
		Expr* expr = parenthesized_from_name(start);
		if (expr == nullptr || expr->kind == ExprKind::Cast)
			return expr;
		return parse_postfix_suffixes(expr);
	}
	if (at_type_specifier_start(1))
	{
		Tentative tentative(*this);
		next();
		const bool saved_angle = angle_;
		angle_ = false;
		TypeId* type = parse_type_id(DeclaratorMode::Abstract, false);
		angle_ = saved_angle;
		if (type != nullptr && at(TokenKind::OP_RPAREN) && at_compound_literal(type))
		{
			tentative.commit();
			return parse_postfix_suffixes(compound_literal(type, start));
		}
		if (type != nullptr && accept(TokenKind::OP_RPAREN) && at_cast_operand_start())
		{
			tentative.commit();
			CastExpr* cast = make<CastExpr>(ExprKind::Cast, start);
			cast->cast = TokenKind::OP_LPAREN;
			cast->type = type;
			cast->operand = parse_cast_expression();
			return cast->operand == nullptr ? nullptr : cast;
		}
		if (abandoning_lists_)
			return nullptr;
	}
	return parse_unary_expression();
}

Expr* Parser::parse_unary_expression()
{
	const ParseToken token = tok();
	if (at_coroutine_keyword(coroutine_keywords_[0]))
	{
		next();
		Expr* operand = parse_cast_expression();
		return operand == nullptr ? nullptr : coroutine_operation(coroutine_keywords_[0], operand, token.location);
	}
	const uint8_t part = token.is_identifier() ? gnu_names_.kind(token.identifier) : 0;
	if (part == GnuNames::kRealPart || part == GnuNames::kImagPart)
	{
		// GNU `__real__ cast-expression` and `__imag__ cast-expression`.
		BuiltinTraitExpr* expr = make<BuiltinTraitExpr>(ExprKind::BuiltinTrait, token.location);
		expr->trait = part == GnuNames::kRealPart ? GnuTrait::RealPart : GnuTrait::ImagPart;
		expr->types = Slice<TypeId*>{ nullptr, 0 };
		expr->type = nullptr;
		next();
		expr->operand = parse_cast_expression();
		return expr->operand == nullptr ? nullptr : expr;
	}
	if (token.kind != ParseTokenKind::Simple)
		return parse_postfix_expression();
	switch (token.simple)
	{
	case TokenKind::OP_INC:
	case TokenKind::OP_DEC:
	case TokenKind::OP_STAR:
	case TokenKind::OP_AMP:
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
	case TokenKind::OP_LNOT:
	case TokenKind::OP_COMPL:
	{
		UnaryExpr* expr = make<UnaryExpr>(ExprKind::Unary, token.location);
		expr->op = token.simple;
		expr->alternative = IsAlternativeSpelling(token.spelling.text, token.spelling.length);
		next();
		expr->operand = parse_cast_expression();
		return expr->operand == nullptr ? nullptr : expr;
	}
	case TokenKind::KW_SIZEOF:
		return parse_sizeof();
	case TokenKind::KW_ALIGNOF:
	case TokenKind::KW_NOEXCEPT:
		return parse_type_trait(token.simple);
	case TokenKind::KW_NEW:
		return parse_new();
	case TokenKind::KW_DELETE:
		return parse_delete();
	case TokenKind::OP_COLON2:
		if (at(TokenKind::KW_NEW, 1))
			return parse_new();
		if (at(TokenKind::KW_DELETE, 1))
			return parse_delete();
		return parse_postfix_expression();
	default:
		return parse_postfix_expression();
	}
}

Expr* Parser::parse_sizeof()
{
	SizeofExpr* expr = make<SizeofExpr>(ExprKind::Sizeof, location());
	next();
	if (accept(TokenKind::OP_DOTS))
	{
		SizeofPackExpr* pack = make<SizeofPackExpr>(ExprKind::SizeofPack, expr->location);
		if (!expect(TokenKind::OP_LPAREN) || !at_identifier())
		{
			fail();
			return nullptr;
		}
		pack->pack = tok().identifier;
		next();
		return expect(TokenKind::OP_RPAREN) ? pack : nullptr;
	}
	return parse_sizeof_operand(expr->type, expr->operand, expr->parenthesized) ? expr : nullptr;
}

bool Parser::parse_sizeof_operand(TypeId*& type, Expr*& operand, bool& parenthesized, bool function_types)
{
	type = nullptr;
	operand = nullptr;
	parenthesized = false;
	// A name, or a GNU decltype-like specifier, read once (a GNU trait is
	// an expression).
	if (at(TokenKind::OP_LPAREN) &&
		(at_name_identifier(1) || at_gnu_decltype(1) || (at(TokenKind::OP_COLON2, 1) && at_identifier(2))))
	{
		const uint32_t start = location();
		next();
		const bool saved_angle = angle_;
		angle_ = false;
		const uint32_t name_start = location();
		NameInfo info;
		Name* name = parse_name(NameMode::Expression, info);
		if (name == nullptr)
			return false;
		if (is_type(name, info))
		{
			Tentative tentative(*this);
			TypeId* named = parse_type_id_from_name(name, info, name_start);
			if (named != nullptr && at(TokenKind::OP_RPAREN) && (function_types || !IsNamedFunctionType(named)))
			{
				next();
				tentative.commit();
				angle_ = saved_angle;
				type = named;
				return true;
			}
			if (abandoning_lists_)
				return false;
		}
		Expr* primary = primary_from_name(name, info, name_start);
		ParenExpr* paren = make<ParenExpr>(ExprKind::Paren, start);
		paren->inner = primary == nullptr ? nullptr : continue_expression(primary);
		angle_ = saved_angle;
		if (paren->inner == nullptr || !expect(TokenKind::OP_RPAREN))
			return false;
		operand = parse_postfix_suffixes(paren);
		if (operand == nullptr)
			return false;
		if (operand == paren)
		{
			operand = paren->inner;
			parenthesized = true;
		}
		return true;
	}
	if (at(TokenKind::OP_LPAREN))
	{
		if (at_type_specifier_start(1))
		{
			Tentative tentative(*this);
			next();
			const bool saved_angle = angle_;
			angle_ = false;
			TypeId* parsed = parse_type_id(DeclaratorMode::Abstract, false);
			angle_ = saved_angle;
			if (parsed != nullptr && at(TokenKind::OP_RPAREN) && !IsNamedFunctionType(parsed))
			{
				next();
				tentative.commit();
				type = parsed;
				return true;
			}
			if (abandoning_lists_)
				return false;
		}
	}
	operand = parse_unary_expression();
	if (operand == nullptr)
		return false;
	if (operand->kind == ExprKind::Paren)
	{
		// `sizeof ( expression )` keeps the expression without the parens.
		operand = static_cast<ParenExpr*>(operand)->inner;
		parenthesized = true;
	}
	return true;
}

Expr* Parser::parse_typeof_operand()
{
	BuiltinTraitExpr* expr = make<BuiltinTraitExpr>(ExprKind::BuiltinTrait, location());
	expr->trait = GnuTrait::TypeofOperand;
	expr->types = Slice<TypeId*>{ nullptr, 0 };
	expr->type = nullptr;
	next();
	TypeId* type = nullptr;
	bool parenthesized = false;
	if (!parse_sizeof_operand(type, expr->operand, parenthesized, true))
		return nullptr;
	if (type != nullptr)
		expr->types = Slice<TypeId*>{ arena_.copy_array(&type, 1), 1 };
	return expr;
}

Expr* Parser::parse_gnu_decltype_operand()
{
	const uint8_t name = gnu_names_.kind(tok().identifier);
	if (name == GnuNames::kTypeof)
		return parse_typeof_operand();
	return parse_builtin_trait(static_cast<GnuTrait>(name - 1));
}

Expr* Parser::parse_type_trait(TokenKind keyword)
{
	TypeTraitExpr* expr = make<TypeTraitExpr>(ExprKind::TypeTrait, location());
	expr->keyword = keyword;
	expr->type = nullptr;
	expr->operand = nullptr;
	next();
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = false;
	if (keyword == TokenKind::KW_ALIGNOF && !name_starts_here())
	{
		// A type-id, or GNU's `__alignof__ ( expression )`.
		Tentative tentative(*this);
		expr->type = parse_type_id(DeclaratorMode::Abstract, false);
		if (expr->type != nullptr && at(TokenKind::OP_RPAREN))
			tentative.commit();
		else
			expr->type = nullptr;
		if (abandoning_lists_)
			return nullptr;
	}
	else if ((keyword == TokenKind::KW_TYPEID || keyword == TokenKind::KW_ALIGNOF) && name_starts_here())
	{
		const uint32_t start = location();
		NameInfo info;
		Name* name = parse_name(NameMode::Expression, info);
		if (name == nullptr)
			return nullptr;
		if (is_type(name, info))
		{
			Tentative tentative(*this);
			TypeId* type = parse_type_id_from_name(name, info, start);
			if (type != nullptr && at(TokenKind::OP_RPAREN) && !IsNamedFunctionType(type))
			{
				tentative.commit();
				expr->type = type;
			}
			if (abandoning_lists_)
				return nullptr;
		}
		if (expr->type == nullptr)
		{
			Expr* primary = primary_from_name(name, info, start);
			expr->operand = primary == nullptr ? nullptr : continue_expression(primary);
			if (expr->operand == nullptr)
				return nullptr;
		}
	}
	else if (keyword == TokenKind::KW_TYPEID && at_type_specifier_start())
	{
		Tentative tentative(*this);
		TypeId* type = parse_type_id(DeclaratorMode::Abstract, false);
		if (type != nullptr && at(TokenKind::OP_RPAREN) && !IsNamedFunctionType(type))
		{
			tentative.commit();
			expr->type = type;
		}
		if (abandoning_lists_)
			return nullptr;
	}
	if (expr->type == nullptr && expr->operand == nullptr)
		expr->operand = parse_expression();
	angle_ = saved_angle;
	if ((expr->type == nullptr && expr->operand == nullptr) || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	return expr;
}

Expr* Parser::parse_builtin_trait(GnuTrait trait)
{
	BuiltinTraitExpr* expr = make<BuiltinTraitExpr>(ExprKind::BuiltinTrait, location());
	expr->trait = trait;
	expr->types = Slice<TypeId*>{ nullptr, 0 };
	expr->type = nullptr;
	expr->operand = nullptr;
	next();
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = false;
	if (trait == GnuTrait::IntegerPack || trait == GnuTrait::BitInt)
	{
		expr->operand = parse_assignment_expression();
		angle_ = saved_angle;
		if (expr->operand == nullptr || !expect(TokenKind::OP_RPAREN))
			return nullptr;
		return expr;
	}
	if (trait == GnuTrait::Offsetof)
	{
		// The type-id, then the member-designator: an identifier, then
		// `. identifier` and `[ expression ]` parts, as member accesses and
		// subscripts of the type's object (OffsetofBase).
		TypeId* type = parse_type_id();
		if (type == nullptr || !expect(TokenKind::OP_COMMA))
			return nullptr;
		expr->types = Slice<TypeId*>{ arena_.copy_array(&type, 1), 1 };
		BuiltinTraitExpr* object = make<BuiltinTraitExpr>(ExprKind::BuiltinTrait, location());
		object->trait = GnuTrait::OffsetofBase;
		object->types = expr->types;
		object->type = nullptr;
		object->operand = nullptr;
		Expr* designator = object;
		bool member = true;
		for (;;)
		{
			if (member)
			{
				if (!at_identifier())
				{
					fail();
					return nullptr;
				}
				MemberExpr* access = make<MemberExpr>(ExprKind::Member, location());
				access->op = TokenKind::OP_DOT;
				access->base = designator;
				NameInfo info;
				access->member = parse_name(NameMode::Member, info);
				if (access->member == nullptr)
					return nullptr;
				designator = access;
			}
			else
			{
				SubscriptExpr* subscript = make<SubscriptExpr>(ExprKind::Subscript, location());
				subscript->base = designator;
				next();
				subscript->index = parse_expression();
				if (subscript->index == nullptr || !expect(TokenKind::OP_RSQUARE))
					return nullptr;
				designator = subscript;
			}
			if (accept(TokenKind::OP_DOT))
				member = true;
			else if (at(TokenKind::OP_LSQUARE))
				member = false;
			else
				break;
		}
		angle_ = saved_angle;
		if (!expect(TokenKind::OP_RPAREN))
			return nullptr;
		expr->operand = designator;
		return expr;
	}
	if (trait == GnuTrait::BitCast)
	{
		// The type-id, then the operand.
		TypeId* type = parse_type_id();
		if (type == nullptr || !expect(TokenKind::OP_COMMA))
			return nullptr;
		expr->operand = parse_assignment_expression();
		angle_ = saved_angle;
		if (expr->operand == nullptr || !expect(TokenKind::OP_RPAREN))
			return nullptr;
		expr->types = Slice<TypeId*>{ arena_.copy_array(&type, 1), 1 };
		return expr;
	}
	if (trait == GnuTrait::VaArg || trait == GnuTrait::ConvertVector)
	{
		// The list (the vector), then the argument's (the result's) type-id.
		expr->operand = parse_assignment_expression();
		if (expr->operand == nullptr || !expect(TokenKind::OP_COMMA))
			return nullptr;
		TypeId* type = parse_type_id();
		angle_ = saved_angle;
		if (type == nullptr || !expect(TokenKind::OP_RPAREN))
			return nullptr;
		expr->types = Slice<TypeId*>{ arena_.copy_array(&type, 1), 1 };
		return expr;
	}
	ListFrame<TypeId*> types(type_ids_);
	do
	{
		// A pack expansion `T...` is the type-id's pack declarator.
		const uint32_t mentions = pack_mentions_;
		TypeId* type = parse_type_id();
		if (type == nullptr)
			return nullptr;
		if (type->declarator != nullptr && type->declarator->pack != PackPosition::None)
			pack_mentions_ = mentions;
		types.push(type);
	} while (accept(TokenKind::OP_COMMA));
	angle_ = saved_angle;
	const uint32_t arity = GnuTraitArity(trait);
	if ((arity != 0 && types.size() != arity) || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	expr->types = types.take(arena_);
	return expr;
}

Expr* Parser::parse_new()
{
	NewExpr* expr = make<NewExpr>(ExprKind::New, location());
	expr->global = accept(TokenKind::OP_COLON2);
	expr->has_placement = false;
	// Read without has_placement by its consumers: empty unless given.
	expr->placement = Slice<Expr*>();
	expr->parenthesized_type = false;
	expr->type = nullptr;
	expr->initializer = nullptr;
	if (!expect(TokenKind::KW_NEW))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = false;
	if (at(TokenKind::OP_LPAREN))
	{
		// `new ( type-id )` when the parentheses hold a type-id not followed
		// by another type; otherwise a new-placement.
		{
			Tentative tentative(*this);
			next();
			TypeId* type = parse_type_id(DeclaratorMode::Abstract, false);
			if (type != nullptr && accept(TokenKind::OP_RPAREN) && !at_type_specifier_start())
			{
				tentative.commit();
				expr->type = type;
				expr->parenthesized_type = true;
			}
		}
		if (abandoning_lists_)
			return nullptr;
		if (expr->type == nullptr)
		{
			next();
			expr->has_placement = true;
			ListFrame<Expr*> placement(exprs_);
			if (!parse_expression_list(placement, TokenKind::OP_RPAREN))
				return nullptr;
			expr->placement = placement.take(arena_);
			if (accept(TokenKind::OP_LPAREN))
			{
				expr->type = parse_type_id();
				expr->parenthesized_type = true;
				if (expr->type == nullptr || !expect(TokenKind::OP_RPAREN))
					return nullptr;
			}
		}
	}
	if (expr->type == nullptr)
	{
		expr->type = parse_type_id(DeclaratorMode::New);
		if (expr->type == nullptr)
			return nullptr;
	}
	if (at(TokenKind::OP_LPAREN) || at(TokenKind::OP_LBRACE))
	{
		expr->initializer = parse_initializer(true);
		if (expr->initializer == nullptr)
			return nullptr;
	}
	angle_ = saved_angle;
	return expr;
}

Expr* Parser::parse_delete()
{
	DeleteExpr* expr = make<DeleteExpr>(ExprKind::Delete, location());
	expr->global = accept(TokenKind::OP_COLON2);
	if (!expect(TokenKind::KW_DELETE))
		return nullptr;
	expr->array = false;
	if (at(TokenKind::OP_LSQUARE) && at(TokenKind::OP_RSQUARE, 1))
	{
		expr->array = true;
		next();
		next();
	}
	expr->operand = parse_cast_expression();
	return expr->operand == nullptr ? nullptr : expr;
}

Expr* Parser::parse_postfix_expression()
{
	Expr* primary = parse_primary_expression();
	if (primary == nullptr)
		return nullptr;
	return parse_postfix_suffixes(primary);
}

Expr* Parser::parse_postfix_suffixes(Expr* expr)
{
	for (;;)
	{
		const ParseToken token = tok();
		if (token.is(TokenKind::OP_LPAREN))
		{
			if (hosted_ && reserved_call_ == nullptr && expr->kind == ExprKind::Id)
				reserved_call_ = ReservedBuiltinCallee(*static_cast<const IdExpr*>(expr)->name);
			CallExpr* call = make<CallExpr>(ExprKind::Call, expr->location);
			call->callee = expr;
			next();
			ListFrame<Expr*> arguments(exprs_);
			if (!parse_expression_list(arguments, TokenKind::OP_RPAREN))
				return nullptr;
			call->arguments = arguments.take(arena_);
			expr = call;
		}
		else if (token.is(TokenKind::OP_LSQUARE))
		{
			SubscriptExpr* subscript = make<SubscriptExpr>(ExprKind::Subscript, expr->location);
			subscript->base = expr;
			next();
			const bool saved_angle = angle_;
			angle_ = false;
			subscript->index = at(TokenKind::OP_LBRACE) ? parse_braced_init_list() : parse_expression();
			angle_ = saved_angle;
			if (subscript->index == nullptr || !expect(TokenKind::OP_RSQUARE))
				return nullptr;
			expr = subscript;
		}
		else if (token.is(TokenKind::OP_DOT) || token.is(TokenKind::OP_ARROW))
		{
			MemberExpr* member = make<MemberExpr>(ExprKind::Member, expr->location);
			member->op = token.simple;
			member->base = expr;
			next();
			NameInfo info;
			member->member = parse_name(NameMode::Member, info);
			if (member->member == nullptr)
				return nullptr;
			expr = member;
		}
		else if (token.is(TokenKind::OP_INC) || token.is(TokenKind::OP_DEC))
		{
			UnaryExpr* postfix = make<UnaryExpr>(ExprKind::Postfix, expr->location);
			postfix->op = token.simple;
			postfix->alternative = false;
			postfix->operand = expr;
			next();
			expr = postfix;
		}
		else
		{
			return expr;
		}
	}
}

Expr* Parser::parse_primary_expression()
{
	const ParseToken token = tok();
	const uint32_t start = token.location;
	if (token.is_literal())
	{
		LiteralExpr* literal = make<LiteralExpr>(ExprKind::Literal, start);
		literal->spelling = token.spelling;
		literal->token_form = false;
		next();
		return literal;
	}
	if (token.is_identifier())
	{
		GnuTrait trait;
		if (at_trait(trait) && !IsGnuTypeTransform(trait))
			return parse_builtin_trait(trait);
		return parse_id_expression_or_cast();
	}
	if (token.kind != ParseTokenKind::Simple)
	{
		fail();
		return nullptr;
	}
	switch (token.simple)
	{
	case TokenKind::KW_TRUE:
	case TokenKind::KW_FALSE:
	case TokenKind::KW_NULLPTR:
	case TokenKind::KW_THIS:
	{
		KeywordLiteralExpr* literal = make<KeywordLiteralExpr>(ExprKind::KeywordLiteral, start);
		literal->keyword = token.simple;
		next();
		return literal;
	}
	case TokenKind::OP_LPAREN:
	{
		if (at(TokenKind::OP_LBRACE, 1))
		{
			// GNU statement expression.
			StatementExpr* statement = make<StatementExpr>(ExprKind::Statement, start);
			next();
			const bool saved_angle = angle_;
			angle_ = false;
			statement->body = parse_compound_statement(true);
			angle_ = saved_angle;
			if (statement->body == nullptr || !expect(TokenKind::OP_RPAREN))
				return nullptr;
			return statement;
		}
		const bool saved_angle = angle_;
		angle_ = false;
		if (at(TokenKind::OP_DOTS, 1))
		{
			Expr* fold = parse_left_fold(start);
			angle_ = saved_angle;
			return fold;
		}
		ParenExpr* paren = make<ParenExpr>(ExprKind::Paren, start);
		next();
		paren->inner = parse_expression();
		if (paren->inner != nullptr && at_fold_operator())
		{
			Expr* fold = parse_fold_rest(start, paren->inner);
			angle_ = saved_angle;
			return fold;
		}
		angle_ = saved_angle;
		if (paren->inner == nullptr || !expect(TokenKind::OP_RPAREN))
			return nullptr;
		return paren;
	}
	case TokenKind::OP_LSQUARE:
		return parse_lambda();
	case TokenKind::OP_LBRACE:
		return parse_braced_init_list();
	case TokenKind::KW_STATIC_CAST:
	case TokenKind::KW_DYNAMIC_CAST:
	case TokenKind::KW_REINTERPET_CAST:
	case TokenKind::KW_CONST_CAST:
		return parse_keyword_cast();
	case TokenKind::KW_TYPEID:
		return parse_type_trait(TokenKind::KW_TYPEID);
	case TokenKind::OP_COLON2:
	case TokenKind::KW_OPERATOR:
	case TokenKind::KW_DECLTYPE:
	case TokenKind::KW_TYPENAME:
		return parse_id_expression_or_cast();
	default:
		if (IsSimpleTypeKeyword(token.simple))
			return parse_keyword_functional_cast();
		fail();
		return nullptr;
	}
}

Expr* Parser::parse_id_expression_or_cast()
{
	const uint32_t start = location();
	const bool typename_keyword = accept(TokenKind::KW_TYPENAME);
	NameInfo info;
	Name* name = parse_name(NameMode::Expression, info);
	if (name == nullptr)
		return nullptr;
	name->typename_keyword = typename_keyword;
	return primary_from_name(name, info, start);
}

Expr* Parser::primary_from_name(Name* name, NameInfo& info, uint32_t start)
{
	const bool typename_keyword = name->typename_keyword;
	const bool lone_decltype = !name->global && name->components.size == 1 &&
		name->last().kind == NameComponentKind::Decltype;
	// A function-style cast of a type the parser knows (5.2.3); other calls
	// stay calls until semantic analysis.
	const bool known_type = typename_keyword || lone_decltype || info.category == NameCategory::Type ||
		info.category == NameCategory::TypeTemplate;
	if ((at(TokenKind::OP_LPAREN) && known_type) || (at(TokenKind::OP_LBRACE) && (known_type || is_type(name, info))))
	{
		FunctionalCastExpr* cast = make<FunctionalCastExpr>(ExprKind::FunctionalCast, start);
		DeclSpecifier* type = make<DeclSpecifier>();
		*type = DeclSpecifier();
		type->location = start;
		if (lone_decltype)
		{
			type->kind = DeclSpecKind::Decltype;
			type->operand = name->last().decltype_operand;
		}
		else
		{
			type->kind = DeclSpecKind::Name;
			type->name = name;
		}
		cast->type = Slice<DeclSpecifier>{ type, 1 };
		cast->arguments = Slice<Expr*>{ nullptr, 0 };
		cast->braced = nullptr;
		if (at(TokenKind::OP_LBRACE))
		{
			cast->braced = parse_braced_init_list();
			return cast->braced == nullptr ? nullptr : cast;
		}
		next();
		ListFrame<Expr*> arguments(exprs_);
		if (!parse_expression_list(arguments, TokenKind::OP_RPAREN))
			return nullptr;
		cast->arguments = arguments.take(arena_);
		return cast;
	}
	if (lone_decltype || typename_keyword)
	{
		fail();
		return nullptr;
	}
	IdExpr* id = make<IdExpr>(ExprKind::Id, start);
	id->name = name;
	return id;
}

Expr* Parser::parse_keyword_functional_cast()
{
	FunctionalCastExpr* cast = make<FunctionalCastExpr>(ExprKind::FunctionalCast, location());
	cast->arguments = Slice<Expr*>{ nullptr, 0 };
	cast->braced = nullptr;
	{
		ListFrame<DeclSpecifier> keywords(specs_);
		while (tok().kind == ParseTokenKind::Simple && IsSimpleTypeKeyword(tok().simple))
		{
			DeclSpecifier keyword = DeclSpecifier();
			keyword.kind = DeclSpecKind::Keyword;
			keyword.keyword = tok().simple;
			keyword.location = location();
			keywords.push(keyword);
			next();
		}
		cast->type = keywords.take(arena_);
	}
	if (at(TokenKind::OP_LBRACE))
	{
		cast->braced = parse_braced_init_list();
		return cast->braced == nullptr ? nullptr : cast;
	}
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	ListFrame<Expr*> arguments(exprs_);
	if (!parse_expression_list(arguments, TokenKind::OP_RPAREN))
		return nullptr;
	cast->arguments = arguments.take(arena_);
	return cast;
}

Expr* Parser::parse_keyword_cast()
{
	CastExpr* cast = make<CastExpr>(ExprKind::Cast, location());
	cast->cast = tok().simple;
	next();
	if (!expect(TokenKind::OP_LT))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = true;
	cast->type = parse_type_id();
	angle_ = false;
	bool parsed = cast->type != nullptr && at_close_angle();
	if (parsed)
	{
		next();
		parsed = expect(TokenKind::OP_LPAREN);
	}
	if (parsed)
	{
		cast->operand = parse_expression();
		parsed = cast->operand != nullptr && expect(TokenKind::OP_RPAREN);
	}
	angle_ = saved_angle;
	if (!parsed)
	{
		fail();
		return nullptr;
	}
	return cast;
}

Expr* Parser::parse_lambda()
{
	LambdaExpr* lambda = make<LambdaExpr>(ExprKind::Lambda, location());
	*lambda = LambdaExpr();
	lambda->kind = ExprKind::Lambda;
	lambda->location = location();
	next();
	if ((at(TokenKind::OP_AMP) && (at(TokenKind::OP_COMMA, 1) || at(TokenKind::OP_RSQUARE, 1))) ||
		at(TokenKind::OP_ASS))
	{
		lambda->has_default = true;
		lambda->capture_default = tok().simple;
		next();
	}
	ListFrame<LambdaCapture> captures(captures_);
	bool first = !lambda->has_default;
	while (!at(TokenKind::OP_RSQUARE))
	{
		if (!first && !expect(TokenKind::OP_COMMA))
			return nullptr;
		first = false;
		LambdaCapture capture = LambdaCapture();
		if (accept(TokenKind::KW_THIS))
		{
			capture.kind = TokenKind::KW_THIS;
		}
		else
		{
			capture.by_reference = accept(TokenKind::OP_AMP);
			capture.kind = capture.by_reference ? TokenKind::OP_AMP : TokenKind::OP_ASS;
			if (!at_identifier())
			{
				fail();
				return nullptr;
			}
			capture.name = tok().identifier;
			next();
		}
		capture.pack = accept(TokenKind::OP_DOTS);
		captures.push(capture);
	}
	next();
	lambda->captures = captures.take(arena_);
	Scope* saved = scopes_.current();
	Scope* body_scope = saved;
	const bool saved_template = pending_template_;
	pending_template_ = false;
	if (hosted_ && at(TokenKind::OP_LT))
	{
		// `[captures] < template-parameter-list >` (C++20's templated
		// lambda): its parameters' names are seen by the rest.
		body_scope = scopes_.push(ScopeKind::Template, nullptr);
		lambda->template_parameters = parse_template_parameter_clause();
		if (lambda->template_parameters == nullptr)
		{
			scopes_.set_current(saved);
			return nullptr;
		}
	}
	lambda->clause_token = static_cast<uint32_t>(tokens_.unit_position());
	if (at(TokenKind::OP_LPAREN))
	{
		lambda->has_declarator = true;
		lambda->parameters = parse_parameter_clause(nullptr, true);
		if (lambda->parameters == nullptr)
			return nullptr;
		body_scope = parameter_scope_;
		// GNU attributes may come before the specifiers.
		if (!collect_attributes(lambda->attributes))
			return nullptr;
		lambda->is_mutable = accept(TokenKind::KW_MUTABLE);
		if (accept(TokenKind::KW_NOEXCEPT))
		{
			lambda->has_noexcept = true;
			if (accept(TokenKind::OP_LPAREN))
			{
				lambda->noexcept_operand = parse_expression();
				if (lambda->noexcept_operand == nullptr || !expect(TokenKind::OP_RPAREN))
					return nullptr;
			}
		}
		// A dynamic-exception-specification (5.1.2p1: an
		// exception-specification of either form).
		else if (accept(TokenKind::KW_THROW))
		{
			lambda->has_throw = true;
			if (!parse_throw_types(lambda->throw_types))
				return nullptr;
		}
		if (!collect_attributes(lambda->attributes))
			return nullptr;
		if (accept(TokenKind::OP_ARROW))
		{
			lambda->trailing = parse_type_id();
			if (lambda->trailing == nullptr)
				return nullptr;
		}
	}
	pending_template_ = saved_template;
	lambda->body_token = static_cast<uint32_t>(tokens_.unit_position());
	if (!lambda->has_declarator)
		lambda->clause_token = lambda->body_token;
	if (speculation_ > 0)
	{
		// While speculating, the body is skipped: it is the same lambda body
		// in every alternative, so it is parsed once, after the commit.
		DeferredLambda deferred{ lambda, tokens_.position(), 0, body_scope, scopes_.log_mark(), definite_type_,
			special_member_clause_ };
		if (!at(TokenKind::OP_LBRACE) || !skip_balanced())
		{
			fail();
			return nullptr;
		}
		deferred.end = tokens_.position();
		lambdas_.push_back(deferred);
		lambda->body = nullptr;
		return lambda;
	}
	scopes_.enter(body_scope);
	const bool saved_angle = angle_;
	angle_ = false;
	pending_template_ = false;
	lambda->body = parse_compound_statement(true);
	angle_ = saved_angle;
	pending_template_ = saved_template;
	scopes_.set_current(saved);
	return lambda->body == nullptr ? nullptr : lambda;
}

BracedInitList* Parser::parse_braced_init_list()
{
	NestingGuard nesting(*this);
	BracedInitList* list = make<BracedInitList>(ExprKind::BracedInit, location());
	list->trailing_comma = false;
	if (!expect(TokenKind::OP_LBRACE))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = false;
	ListFrame<Expr*> elements(exprs_);
	std::vector<const Identifier*> designators;
	bool parsed = true;
	while (!at(TokenKind::OP_RBRACE))
	{
		const uint32_t mentions = pack_mentions_;
		// A designator `.member =` or `.member {` (C99's and C++20's
		// designated initializers; hosted compilations).
		const Identifier* designator = nullptr;
		if (hosted_ && at(TokenKind::OP_DOT) && at_identifier(1) &&
			(at(TokenKind::OP_ASS, 2) || at(TokenKind::OP_LBRACE, 2)))
		{
			designator = tok(1).identifier;
			next();
			next();
			accept(TokenKind::OP_ASS);
			designators.resize(elements.size(), nullptr);
			designators.push_back(designator);
		}
		else if (!designators.empty())
			designators.push_back(nullptr);
		Expr* element = designator != nullptr && at(TokenKind::OP_LBRACE) ? parse_braced_init_list()
																		  : parse_initializer_clause();
		if (element == nullptr)
		{
			parsed = false;
			break;
		}
		if (at(TokenKind::OP_DOTS))
		{
			PackExpansionExpr* pack = make<PackExpansionExpr>(ExprKind::PackExpansion, element->location);
			pack->pattern = element;
			next();
			element = pack;
			pack_mentions_ = mentions;
		}
		elements.push(element);
		if (!accept(TokenKind::OP_COMMA))
			break;
		if (at(TokenKind::OP_RBRACE))
			list->trailing_comma = true;
	}
	angle_ = saved_angle;
	if (!parsed || !expect(TokenKind::OP_RBRACE))
		return nullptr;
	list->elements = elements.take(arena_);
	if (!designators.empty())
		list->designators = arena_.copy_array(designators.data(), designators.size());
	return list;
}

bool Parser::parse_expression_list(ListFrame<Expr*>& list, TokenKind close)
{
	const bool saved_angle = angle_;
	angle_ = false;
	bool parsed = true;
	while (!at(close))
	{
		const uint32_t mentions = pack_mentions_;
		Expr* element = parse_initializer_clause();
		if (element == nullptr)
		{
			parsed = false;
			break;
		}
		if (at(TokenKind::OP_DOTS))
		{
			PackExpansionExpr* pack = make<PackExpansionExpr>(ExprKind::PackExpansion, element->location);
			pack->pattern = element;
			next();
			element = pack;
			pack_mentions_ = mentions;
		}
		list.push(element);
		if (!accept(TokenKind::OP_COMMA))
			break;
	}
	angle_ = saved_angle;
	return parsed && expect(close);
}

} // namespace syntax
