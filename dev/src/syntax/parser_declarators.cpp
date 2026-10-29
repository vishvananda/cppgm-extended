#include "syntax/parser.h"

#include <cstring>

#include "lex/identifier_table.h"

namespace syntax {

namespace {

bool IsNullability(const char* spelling)
{
	return std::strcmp(spelling, "_Nonnull") == 0 || std::strcmp(spelling, "_Nullable") == 0 ||
		std::strcmp(spelling, "_Null_unspecified") == 0 || std::strcmp(spelling, "_Nullable_result") == 0;
}

} // namespace

bool Parser::declarator_is_empty(const Declarator* declarator)
{
	return declarator->pointers.empty() && declarator->pack == PackPosition::None && declarator->name == nullptr &&
		declarator->nested == nullptr && declarator->suffixes.empty();
}

const Name* Parser::declarator_name(const Declarator* declarator)
{
	for (const Declarator* d = declarator; d != nullptr; d = d->nested)
	{
		if (d->name != nullptr)
			return d->name;
	}
	return nullptr;
}

namespace {

// The first derivation applied to a declarator's name (8.3p1): 1 for a
// function suffix, 0 for another derivation, -1 when this level adds none.
int FirstDerivation(const Declarator* declarator)
{
	if (declarator->nested != nullptr)
	{
		const int nested = FirstDerivation(declarator->nested);
		if (nested != -1)
			return nested;
	}
	if (!declarator->suffixes.empty())
		return declarator->suffixes[0].kind == DeclaratorSuffixKind::Function ? 1 : 0;
	if (!declarator->pointers.empty())
		return 0;
	return -1;
}

// A declarator whose name is written directly or only in parentheses.
bool NamesDirectly(const Declarator* declarator)
{
	if (declarator->name != nullptr)
		return true;
	const Declarator* nested = declarator->nested;
	return nested != nullptr && nested->pointers.empty() && nested->suffixes.empty() && NamesDirectly(nested);
}

} // namespace

bool Parser::is_function_declarator(const Declarator* declarator)
{
	return declarator_name(declarator) != nullptr && FirstDerivation(declarator) == 1;
}

bool Parser::member_pointer_ahead(uint32_t ahead)
{
	// nested-name-specifier `*`: `::`? (identifier template-args? `::`)+ `*`.
	uint32_t i = ahead;
	if (at(TokenKind::OP_COLON2, i))
		++i;
	bool any = false;
	for (;;)
	{
		if (at(TokenKind::KW_TEMPLATE, i))
			++i;
		if (!at_identifier(i))
			return false;
		++i;
		if (at(TokenKind::OP_LT, i))
		{
			// Bracketed expressions isolate relational and shift operators
			// from the surrounding template argument list's angle brackets.
			int depth = 0;
			closers_.clear();
			for (;; ++i)
			{
				const ParseToken token = tok(i);
				if (token.is_end() || token.is(TokenKind::OP_SEMICOLON))
					return false;
				if (token.is(TokenKind::OP_LPAREN))
					closers_.push_back(TokenKind::OP_RPAREN);
				else if (token.is(TokenKind::OP_LSQUARE))
					closers_.push_back(TokenKind::OP_RSQUARE);
				else if (token.is(TokenKind::OP_LBRACE))
					closers_.push_back(TokenKind::OP_RBRACE);
				else if (token.is(TokenKind::OP_RPAREN) || token.is(TokenKind::OP_RSQUARE) ||
					token.is(TokenKind::OP_RBRACE))
				{
					if (closers_.empty() || !token.is(closers_.back()))
						return false;
					closers_.pop_back();
				}
				else if (closers_.empty())
				{
					if (token.is(TokenKind::OP_LT))
						++depth;
					else if (token.is(TokenKind::OP_GT))
						--depth;
					if (depth == 0)
					{
						++i;
						break;
					}
				}
			}
		}
		if (!at(TokenKind::OP_COLON2, i))
			return false;
		++i;
		any = true;
		if (at(TokenKind::OP_STAR, i))
			return any;
	}
}

bool Parser::ptr_operator_ahead(uint32_t ahead)
{
	if (at(TokenKind::OP_STAR, ahead) || at(TokenKind::OP_AMP, ahead) || at(TokenKind::OP_LAND, ahead) ||
		(hosted_ && at(TokenKind::OP_XOR, ahead)))
		return true;
	if (at_identifier(ahead) || at(TokenKind::OP_COLON2, ahead))
		return member_pointer_ahead(ahead);
	return false;
}

bool Parser::parse_ptr_operator(ListFrame<PtrOperator>& pointers)
{
	PtrOperator pointer = PtrOperator();
	// `^` is Clang's block pointer (hosted compilations).
	if (at(TokenKind::OP_STAR) || at(TokenKind::OP_AMP) || at(TokenKind::OP_LAND) || at(TokenKind::OP_XOR))
	{
		pointer.kind = tok().simple;
		pointer.alternative = IsAlternativeSpelling(tok().spelling.text, tok().spelling.length);
		next();
	}
	else
	{
		NameInfo info;
		pointer.kind = TokenKind::OP_COLON2;
		pointer.class_name = parse_name(NameMode::Type, info);
		if (pointer.class_name == nullptr || !expect(TokenKind::OP_COLON2) || !expect(TokenKind::OP_STAR))
			return false;
	}
	if (pointer.kind == TokenKind::OP_STAR || pointer.kind == TokenKind::OP_COLON2 || pointer.kind == TokenKind::OP_XOR)
	{
		ListFrame<TokenKind> cv(cvs_);
		for (;;)
		{
			if (!skip_attributes())
				return false;
			// Clang's nullability qualifiers (_Nonnull, _Nullable,
			// _Null_unspecified, _Nullable_result) promise nothing here.
			if (at_identifier() && tok().identifier->length >= 8 && tok().identifier->spelling[0] == '_' &&
				tok().identifier->spelling[1] == 'N' && IsNullability(tok().identifier->spelling))
			{
				next();
				continue;
			}
			if (!at(TokenKind::KW_CONST) && !at(TokenKind::KW_VOLATILE))
				break;
			cv.push(tok().simple);
			next();
		}
		pointer.cv = cv.take(arena_);
	}
	else if (!skip_attributes())
	{
		return false;
	}
	pointers.push(pointer);
	return true;
}

bool Parser::nested_declarator_ahead(DeclaratorMode mode)
{
	// At `(` where a declarator-id could be: a nested declarator, or (when
	// the declarator may be abstract) a parameter clause.
	if (mode == DeclaratorMode::Named)
		return true;
	// A parameter clause cannot start with `(`, `[` or a ptr-operator.
	if (at(TokenKind::OP_STAR, 1) || at(TokenKind::OP_AMP, 1) || at(TokenKind::OP_LAND, 1) || member_pointer_ahead(1) ||
		(hosted_ && at(TokenKind::OP_XOR, 1)) || (at(TokenKind::OP_LSQUARE, 1) && !at(TokenKind::OP_LSQUARE, 2)) ||
		at(TokenKind::OP_LPAREN, 1))
		return true;
	if (mode == DeclaratorMode::Abstract)
		return false;
	// A parameter's declarator-id is unqualified (8.3p1): a parenthesized
	// qualified name (not a member pointer's class, above) is a type name
	// starting a parameter clause (8.2p7: `void f(int (n::B));` takes a
	// pointer to function).
	if (mode == DeclaratorMode::Parameter && at_identifier(1) && at(TokenKind::OP_COLON2, 2))
		return false;
	if (at_identifier(1))
	{
		// A parenthesized name that is not a type is a declarator-id (8.2p7
		// makes a type name start a parameter clause instead).
		const NameBinding binding = scopes_.lookup(tok(1).identifier, false);
		if (binding.category == NameCategory::None)
			return !names_type_hint(tok(1).identifier);
		return binding.category != NameCategory::Type && binding.category != NameCategory::TypeTemplate;
	}
	return false;
}

Declarator* Parser::parse_declarator(DeclaratorMode mode)
{
	Declarator* declarator = make<Declarator>();
	*declarator = Declarator();
	declarator->abstract = mode != DeclaratorMode::Named && mode != DeclaratorMode::Parameter;
	declarator->location = location();
	if (!parse_declarator_into(mode, declarator))
		return nullptr;
	return declarator;
}

bool Parser::parse_declarator_into(DeclaratorMode mode, Declarator* declarator)
{
	NestingGuard nesting(*this);
	{
		ListFrame<PtrOperator> pointers(pointers_);
		while (ptr_operator_ahead(0))
		{
			if (!parse_ptr_operator(pointers))
				return false;
		}
		declarator->pointers = pointers.take(arena_);
	}
	if (mode == DeclaratorMode::Conversion)
		return true;
	if (mode == DeclaratorMode::New)
		return parse_declarator_suffixes(mode, declarator, false, nullptr);
	// At the end of a parameter-declaration-clause, `...` expands this
	// parameter only if its type names an unexpanded template parameter
	// pack; otherwise it is the clause's ellipsis (8.3.5p14).
	if (at(TokenKind::OP_DOTS) &&
		(mode != DeclaratorMode::Parameter || !at(TokenKind::OP_RPAREN, 1) || pack_mentions_ > parameter_mentions_))
	{
		declarator->pack = PackPosition::BeforeName;
		next();
	}
	Scope* member_of = nullptr;
	if (at(TokenKind::OP_LPAREN) && nested_declarator_ahead(mode))
	{
		next();
		Declarator* nested = make<Declarator>();
		*nested = Declarator();
		nested->abstract = declarator->abstract;
		nested->location = location();
		if (!parse_declarator_into(mode, nested) || !expect(TokenKind::OP_RPAREN))
			return false;
		declarator->nested = nested;
		// The suffixes after `)` follow the nested declarator-id too.
		member_of = declarator_member_of_;
	}
	else if (mode != DeclaratorMode::Abstract &&
		(at_identifier() || at(TokenKind::OP_COLON2) || at(TokenKind::KW_OPERATOR) || at(TokenKind::OP_COMPL)))
	{
		NameInfo info;
		declarator->name = parse_name(NameMode::Declarator, info);
		if (declarator->name == nullptr)
			return false;
		if (declarator->name->qualified() && info.prefix == PrefixState::Known &&
			!(friend_declarator_ && mode == DeclaratorMode::Named && info.prefix_scope->kind == ScopeKind::Namespace))
			member_of = info.prefix_scope;
		if (!collect_attributes(declarator->attributes))
			return false;
		// A pack's `...` precedes the declarator-id (8p4); one after it is
		// the clause's ellipsis (8.3.5p4: `, ...` and `...` are synonymous).
	}
	else if (mode == DeclaratorMode::Named)
	{
		return fail();
	}
	else if (mode == DeclaratorMode::Parameter && at(TokenKind::OP_LPAREN) &&
		(at(TokenKind::OP_RPAREN, 1) || at(TokenKind::OP_DOTS, 1)))
	{
		// `()` and `(...)` make the parameter's declarator abstract.
		return fail();
	}
	const bool parsed = parse_declarator_suffixes(mode, declarator, NamesDirectly(declarator), member_of);
	declarator_member_of_ = member_of;
	return parsed;
}

Scope* Parser::member_lookup_scope(Scope* member_of)
{
	Scope* outer = scopes_.current();
	Scope* scope = scopes_.push(ScopeKind::Block, nullptr);
	scope->member_of = member_of;
	scopes_.set_current(outer);
	return scope;
}

bool Parser::parse_declarator_suffixes(DeclaratorMode mode, Declarator* declarator, bool named, Scope* member_of)
{
	ListFrame<DeclaratorSuffix> suffixes(suffixes_);
	// Array bounds after a qualified declarator-id see its class's or
	// namespace's names (3.4.3p3).
	Scope* bound_scope = nullptr;
	for (;;)
	{
		if (at(TokenKind::OP_LSQUARE) && !at(TokenKind::OP_LSQUARE, 1))
		{
			DeclaratorSuffix suffix = DeclaratorSuffix();
			suffix.kind = DeclaratorSuffixKind::Array;
			next();
			if (!at(TokenKind::OP_RSQUARE))
			{
				const bool saved_angle = angle_;
				angle_ = false;
				Scope* outer = nullptr;
				if (member_of != nullptr)
				{
					if (bound_scope == nullptr)
						bound_scope = member_lookup_scope(member_of);
					outer = scopes_.enter(bound_scope);
				}
				suffix.bound = parse_expression();
				if (outer != nullptr)
					scopes_.set_current(outer);
				angle_ = saved_angle;
				if (suffix.bound == nullptr)
					return false;
			}
			// Attributes after `]` appertain to the array type (8.3.4p1);
			// GCC applies GNU ones to the declared entity.
			Attribute** added = &declarator->attributes;
			while (*added != nullptr)
				added = &(*added)->next;
			if (!expect(TokenKind::OP_RSQUARE) || !collect_attributes(declarator->attributes))
				return false;
			for (Attribute* attribute = *added; attribute != nullptr; attribute = attribute->next)
				attribute->on_type = true;
			suffixes.push(suffix);
			continue;
		}
		if (!at(TokenKind::OP_LPAREN) || mode == DeclaratorMode::New || mode == DeclaratorMode::Conversion)
			break;
		DeclaratorSuffix suffix = DeclaratorSuffix();
		suffix.kind = DeclaratorSuffixKind::Function;
		{
			// A parenthesized list after a declarator is a parameter clause if
			// it can be one (8.2p1); otherwise it is left for an initializer.
			// Only a named declarator's clause competes with an initializer;
			// an operator or special member name decides for the clause.
			const bool definite = mode != DeclaratorMode::Named || special_member_clause_ ||
				(declarator->name != nullptr && declarator->name->last().kind != NameComponentKind::Identifier);
			Tentative tentative(*this);
			suffix.parameters = parse_parameter_clause(suffixes.size() == 0 ? member_of : nullptr, definite);
			if (suffix.parameters == nullptr && abandoning_lists_)
				return false;
			if (suffix.parameters == nullptr)
				break;
			tentative.commit();
		}
		if (named && suffixes.size() == 0)
			function_scope_ = parameter_scope_;
		ListFrame<FunctionQualifier> qualifiers(qualifiers_);
		// After a qualified declarator-id, its exception-specification and
		// trailing return type see the class's or namespace's names, as its
		// parameters do (3.4.3p3).
		Scope* outer = member_of != nullptr && suffixes.size() == 0 ? scopes_.enter(parameter_scope_) : nullptr;
		const bool qualified = parse_function_qualifiers(
			qualifiers, mode == DeclaratorMode::Named ? &declarator->attributes : nullptr);
		if (outer != nullptr)
			scopes_.set_current(outer);
		if (!qualified)
			return false;
		suffix.qualifiers = qualifiers.take(arena_);
		suffixes.push(suffix);
	}
	declarator->suffixes = suffixes.take(arena_);
	// A GNU asm label names the declared entity's symbol (an attribute of
	// its own kind: declarators carry no field for it); GNU attributes may
	// follow it.
	if (mode == DeclaratorMode::Named && at(TokenKind::KW_ASM))
	{
		const uint32_t start = location();
		next();
		if (!expect(TokenKind::OP_LPAREN))
			return false;
		if (!tok().is_literal() || (tok().flags & kStringLiteral) == 0)
			return fail();
		Attribute* label = make<Attribute>();
		*label = Attribute();
		label->kind = AttributeKind::AsmLabel;
		label->location = start;
		label->expression = parse_primary_expression();
		Attribute** tail = &declarator->attributes;
		while (*tail != nullptr)
			tail = &(*tail)->next;
		*tail = label;
		if (!expect(TokenKind::OP_RPAREN) || !collect_attributes(declarator->attributes))
			return false;
	}
	return true;
}

ParameterClause* Parser::parse_parameter_clause(Scope* member_of, bool definite)
{
	NestingGuard nesting(*this);
	ParameterClause* clause = make<ParameterClause>();
	*clause = ParameterClause();
	clause->location = location();
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	Scope* outer = scopes_.current();
	Scope* scope = scopes_.push(ScopeKind::Function, nullptr);
	scope->member_of = member_of;
	const bool saved_angle = angle_;
	const bool saved_template = pending_template_;
	angle_ = false;
	pending_template_ = false;
	ListFrame<ParameterDeclaration*> parameters(params_);
	bool parsed = true;
	while (!at(TokenKind::OP_RPAREN))
	{
		if (accept(TokenKind::OP_DOTS))
		{
			clause->ellipsis = true;
			break;
		}
		// After one parameter-declaration the list is a parameter clause, so
		// an undeclared name in a later one is a type.
		ParameterDeclaration* parameter = parse_parameter_declaration(definite);
		if (parameter == nullptr)
		{
			parsed = false;
			break;
		}
		definite = true;
		parameters.push(parameter);
		if (accept(TokenKind::OP_COMMA))
			continue;
		if (accept(TokenKind::OP_DOTS))
		{
			clause->ellipsis = true;
			clause->ellipsis_without_comma = true;
		}
		break;
	}
	angle_ = saved_angle;
	pending_template_ = saved_template;
	scopes_.set_current(outer);
	if (!parsed || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	clause->parameters = parameters.take(arena_);
	parameter_scope_ = scope;
	return clause;
}

ParameterDeclaration* Parser::parse_parameter_declaration(bool definite)
{
	ParameterDeclaration* parameter = make<ParameterDeclaration>();
	*parameter = ParameterDeclaration();
	parameter->location = location();
	// Its leading attributes are dropped but for a vector's.
	Attribute* leading = nullptr;
	if (!collect_attributes(leading))
		return nullptr;
	const uint32_t enclosing_mentions = parameter_mentions_;
	parameter_mentions_ = pack_mentions_;
	{
		ListFrame<DeclSpecifier> specifiers(specs_);
		SpecifierInfo info;
		leading_type_attributes(leading, specifiers);
		if (!parse_decl_specifier_seq(specifiers, info, definite))
			return nullptr;
		if (specifiers.size() == 0)
		{
			fail();
			return nullptr;
		}
		parameter->specifiers = specifiers.take(arena_);
	}
	Declarator* declarator = nullptr;
	{
		Tentative tentative(*this);
		declarator = parse_declarator(DeclaratorMode::Parameter);
		if (declarator != nullptr)
			tentative.commit();
	}
	if (declarator == nullptr)
	{
		if (abandoning_lists_)
			return nullptr;
		declarator = parse_declarator(DeclaratorMode::Abstract);
		if (declarator == nullptr)
			return nullptr;
	}
	if (!declarator_is_empty(declarator))
		parameter->declarator = declarator;
	if (declarator->pack != PackPosition::None)
		pack_mentions_ = parameter_mentions_;
	parameter_mentions_ = enclosing_mentions;
	if (!skip_attributes())
		return nullptr;
	if (accept(TokenKind::OP_ASS))
	{
		Initializer* initializer = make<Initializer>();
		*initializer = Initializer();
		initializer->kind = InitializerKind::Assign;
		initializer->location = location();
		initializer->expression = parse_initializer_clause();
		if (initializer->expression == nullptr)
			return nullptr;
		parameter->default_argument = initializer;
	}
	const Name* name = declarator_name(parameter->declarator);
	if (name != nullptr && !name->qualified() && name->last().kind == NameComponentKind::Identifier)
		scopes_.declare(scopes_.current(), name->last().identifier, NameBinding{ NameCategory::Value, false, nullptr });
	return parameter;
}

bool Parser::parse_throw_types(Slice<TypeId*>& out)
{
	// ( type-id-list? ) of a dynamic-exception-specification (15.4p1),
	// after `throw`.
	if (!expect(TokenKind::OP_LPAREN))
		return false;
	ListFrame<TypeId*> types(type_ids_);
	if (!at(TokenKind::OP_RPAREN))
	{
		do
		{
			// A pack expansion `T...` is the type-id's pack declarator.
			const uint32_t mentions = pack_mentions_;
			TypeId* type = parse_type_id();
			if (type == nullptr)
				return false;
			if (type->declarator != nullptr && type->declarator->pack != PackPosition::None)
				pack_mentions_ = mentions;
			types.push(type);
		} while (accept(TokenKind::OP_COMMA));
	}
	if (!expect(TokenKind::OP_RPAREN))
		return false;
	out = types.take(arena_);
	return true;
}

bool Parser::parse_function_qualifiers(ListFrame<FunctionQualifier>& qualifiers, Attribute** attributes)
{
	for (;;)
	{
		// A named declarator's GNU attributes among them appertain to the
		// declared entity (GCC); an attribute-specifier there to the
		// function type (8.3.5p1), marked so.
		if (attributes != nullptr)
		{
			Attribute** added = attributes;
			while (*added != nullptr)
				added = &(*added)->next;
			if (!collect_attributes(*attributes))
				return false;
			for (Attribute* attribute = *added; attribute != nullptr; attribute = attribute->next)
				attribute->on_type = attribute->on_type || attribute->standard;
		}
		else if (!skip_attributes())
			return false;
		FunctionQualifier qualifier = FunctionQualifier();
		qualifier.location = location();
		if (at(TokenKind::KW_CONST) || at(TokenKind::KW_VOLATILE))
		{
			qualifier.kind = FunctionQualifierKind::Cv;
			qualifier.token = tok().simple;
			next();
		}
		else if (at(TokenKind::OP_AMP) || at(TokenKind::OP_LAND))
		{
			qualifier.kind = FunctionQualifierKind::Ref;
			qualifier.token = tok().simple;
			next();
		}
		else if (accept(TokenKind::KW_NOEXCEPT))
		{
			qualifier.kind = FunctionQualifierKind::Noexcept;
			if (at(TokenKind::OP_LPAREN))
			{
				next();
				qualifier.has_operand = true;
				const bool saved_angle = angle_;
				angle_ = false;
				qualifier.noexcept_operand = parse_expression();
				angle_ = saved_angle;
				if (qualifier.noexcept_operand == nullptr || !expect(TokenKind::OP_RPAREN))
					return false;
			}
		}
		else if (accept(TokenKind::KW_THROW))
		{
			qualifier.kind = FunctionQualifierKind::Throw;
			if (!parse_throw_types(qualifier.throw_types))
				return false;
		}
		else if (accept(TokenKind::OP_ARROW))
		{
			qualifier.kind = FunctionQualifierKind::TrailingReturn;
			qualifier.trailing = parse_type_id();
			if (qualifier.trailing == nullptr)
				return false;
		}
		else if (at_identifier(final_) || at_identifier(override_))
		{
			qualifier.kind = FunctionQualifierKind::Virt;
			qualifier.virt = tok().identifier;
			next();
		}
		else
		{
			return true;
		}
		qualifiers.push(qualifier);
	}
}

TypeId* Parser::parse_type_id(DeclaratorMode mode, bool definite, SpecifierInfo* specified)
{
	TypeId* type = make<TypeId>();
	type->location = location();
	type->declarator = nullptr;
	SpecifierInfo info;
	{
		ListFrame<DeclSpecifier> specifiers(specs_);
		const bool saved_definite = definite_type_;
		definite_type_ = definite;
		const bool parsed = parse_type_specifier_seq(specifiers, info);
		definite_type_ = saved_definite;
		if (!parsed)
			return nullptr;
		type->specifiers = specifiers.take(arena_);
	}
	if (specified != nullptr)
		*specified = info;
	return finish_type_id(type, info, mode);
}

TypeId* Parser::parse_type_id_from_name(Name* name, const NameInfo& name_info, uint32_t start)
{
	TypeId* type = make<TypeId>();
	type->location = start;
	type->declarator = nullptr;
	SpecifierInfo info = SpecifierInfo();
	{
		ListFrame<DeclSpecifier> specifiers(specs_);
		DeclSpecifier specifier = DeclSpecifier();
		specifier.location = start;
		if (!name->global && name->components.size == 1 && name->last().kind == NameComponentKind::Decltype)
		{
			specifier.kind = DeclSpecKind::Decltype;
			specifier.operand = name->last().decltype_operand;
		}
		else
		{
			specifier.kind = DeclSpecKind::Name;
			specifier.name = name;
			info.type_scope = name_info.scope;
			info.type_template_member = name_info.template_member;
		}
		specifiers.push(specifier);
		info.has_type = true;
		const bool saved_definite = definite_type_;
		definite_type_ = false;
		const bool parsed = continue_type_specifier_seq(specifiers, info);
		definite_type_ = saved_definite;
		if (!parsed)
			return nullptr;
		type->specifiers = specifiers.take(arena_);
	}
	return finish_type_id(type, info, DeclaratorMode::Abstract);
}

TypeId* Parser::finish_type_id(TypeId* type, const SpecifierInfo& info, DeclaratorMode mode)
{
	Declarator* declarator = make<Declarator>();
	*declarator = Declarator();
	declarator->abstract = true;
	declarator->location = location();
	if (!parse_declarator_into(mode, declarator))
		return nullptr;
	if (!declarator_is_empty(declarator))
		type->declarator = declarator;
	type_id_scope_ = type->declarator == nullptr ? info.type_scope : nullptr;
	type_id_record_ = type->declarator == nullptr && info.type_scope != nullptr && info.type_scope->parameter_bases
		? typedef_record(info.type_name, info.type_name_info.scope, info.type_name_info.category, info.type_mentions)
		: 0;
	return type;
}

} // namespace syntax
