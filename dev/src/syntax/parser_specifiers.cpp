#include <cstring>

#include "syntax/parser.h"

#include "lex/identifier_table.h"

namespace syntax {

namespace {

// One of GCC's 128-bit and extended floating type names.
bool ExtendedType(const Identifier* const (&names)[10], const Identifier* identifier)
{
	for (const Identifier* name : names)
	{
		if (name == identifier)
			return true;
	}
	return false;
}

bool IsSimpleTypeKeyword(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::KW_AUTO:
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
		return true;
	default:
		return false;
	}
}

// Storage-class, function and other non-type decl-specifier keywords, and
// the cv-qualifiers.
bool IsKeywordSpecifier(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::KW_TYPEDEF:
	case TokenKind::KW_FRIEND:
	case TokenKind::KW_STATIC:
	case TokenKind::KW_EXTERN:
	case TokenKind::KW_INLINE:
	case TokenKind::KW_VIRTUAL:
	case TokenKind::KW_EXPLICIT:
	case TokenKind::KW_CONSTEXPR:
	case TokenKind::KW_THREAD_LOCAL:
	case TokenKind::KW_MUTABLE:
	case TokenKind::KW_REGISTER:
	case TokenKind::KW_CONST:
	case TokenKind::KW_VOLATILE:
		return true;
	default:
		return false;
	}
}

bool IsClassKey(TokenKind kind)
{
	return kind == TokenKind::KW_CLASS || kind == TokenKind::KW_STRUCT || kind == TokenKind::KW_UNION;
}

// After the simple type keyword `kind`: the type specifiers are still only
// `signed` or `unsigned`.
bool SignOnly(bool has_type, bool sign_only, TokenKind kind)
{
	return (!has_type || sign_only) && (kind == TokenKind::KW_SIGNED || kind == TokenKind::KW_UNSIGNED);
}

DeclSpecifier KeywordSpecifier(TokenKind kind, uint32_t location)
{
	DeclSpecifier specifier = DeclSpecifier();
	specifier.kind = DeclSpecKind::Keyword;
	specifier.keyword = kind;
	specifier.location = location;
	return specifier;
}

} // namespace

bool Parser::parse_decl_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info, bool definite)
{
	info = SpecifierInfo();
	const bool saved_definite = definite_type_;
	definite_type_ = definite;
	const bool parsed = parse_decl_specifiers(list, info);
	definite_type_ = saved_definite;
	return parsed;
}

bool Parser::gnu_attribute(const char* spelling, Attribute& attribute, bool& kept)
{
	// One GNU attribute after its name of the kinds later stages use that
	// stand alone or take an expression (kept); false when no such kind
	// (`kept` stays false) or its operand does not parse.
	static const struct
	{
		const char* spelling;
		const char* reserved;
		AttributeKind kind;
	} kSimple[] = { { "noinline", "__noinline__", AttributeKind::NoInline },
		{ "packed", "__packed__", AttributeKind::Packed }, { "weak", "__weak__", AttributeKind::Weak },
		{ "pure", "__pure__", AttributeKind::Pure }, { "const", "__const__", AttributeKind::Const },
		{ "using_if_exists", "__using_if_exists__", AttributeKind::UsingIfExists },
		{ "exclude_from_explicit_instantiation", "__exclude_from_explicit_instantiation__",
			AttributeKind::ExcludeFromExplicitInstantiation },
		{ "always_inline", "__always_inline__", AttributeKind::AlwaysInline },
		{ "gnu_inline", "__gnu_inline__", AttributeKind::GnuInline }, { "used", "__used__", AttributeKind::Used },
		{ "noreturn", "__noreturn__", AttributeKind::NoReturn } };
	for (const auto& simple : kSimple)
	{
		if (std::strcmp(spelling, simple.spelling) == 0 || std::strcmp(spelling, simple.reserved) == 0)
		{
			attribute.kind = simple.kind;
			kept = true;
			return true;
		}
	}
	if (std::strcmp(spelling, "vector_size") == 0 || std::strcmp(spelling, "__vector_size__") == 0 ||
		std::strcmp(spelling, "ext_vector_type") == 0 || std::strcmp(spelling, "__ext_vector_type__") == 0)
	{
		// The vector's size, in bytes or (Clang's form) lanes.
		attribute.kind = std::strstr(spelling, "ext_") != nullptr ? AttributeKind::ExtVectorType
																  : AttributeKind::VectorSize;
		if (!expect(TokenKind::OP_LPAREN))
			return false;
		attribute.expression = parse_assignment_expression();
		kept = true;
		return attribute.expression != nullptr && expect(TokenKind::OP_RPAREN);
	}
	if (std::strcmp(spelling, "aligned") == 0 || std::strcmp(spelling, "__aligned__") == 0)
	{
		// aligned(N), or aligned alone (no expression: the target's
		// biggest alignment).
		attribute.kind = AttributeKind::Alignas;
		attribute.gnu = true;
		kept = true;
		if (!accept(TokenKind::OP_LPAREN))
			return true;
		attribute.expression = parse_assignment_expression();
		return attribute.expression != nullptr && expect(TokenKind::OP_RPAREN);
	}
	return false;
}

bool Parser::collect_attribute_specifiers(Attribute*& list)
{
	// (collect_attributes calls this at the first attribute.)
	// attribute-specifier-seq (7.6.1), alignment-specifiers (7.6.2) and GNU
	// __attribute__((...)): alignas and the GNU kinds later stages use
	// (aligned(N), noinline, packed, weak, section, abi_tag, pure, const,
	// vector_size ...) are kept, appended to `list`, as are their gnu::
	// forms (GCC honors them as the GNU forms); the others are skipped.
	Attribute** tail = &list;
	while (*tail != nullptr)
		tail = &(*tail)->next;
	// section ( string-literal ): one literal (adjacent pieces are one
	// token after phase 6), checked by the analysis.
	const auto section_operand = [&](Attribute& attribute) {
		attribute.kind = AttributeKind::Section;
		if (!expect(TokenKind::OP_LPAREN) || !tok().is_literal() || (tok().flags & kStringLiteral) == 0)
			return fail();
		attribute.expression = parse_primary_expression();
		return attribute.expression != nullptr && expect(TokenKind::OP_RPAREN);
	};
	const auto keep = [&](const Attribute& attribute) {
		Attribute* node = make<Attribute>();
		*node = attribute;
		*tail = node;
		tail = &node->next;
	};
	// abi_tag ( string-literal , ... ): one node per literal (GCC and
	// Clang reject any other argument, and an empty list); abi_tag alone
	// one node without a literal (an inline namespace's name is its tag).
	const auto abi_tag_operands = [&](Attribute attribute) {
		attribute.kind = AttributeKind::AbiTag;
		if (!at(TokenKind::OP_LPAREN))
		{
			keep(attribute);
			return true;
		}
		next();
		do
		{
			if (!tok().is_literal() || (tok().flags & kStringLiteral) == 0)
				return fail();
			attribute.location = location();
			attribute.expression = parse_primary_expression();
			if (attribute.expression == nullptr)
				return false;
			keep(attribute);
			attribute.continued = true;
		} while (accept(TokenKind::OP_COMMA));
		return expect(TokenKind::OP_RPAREN);
	};
	while (at_attribute())
	{
		const uint32_t start = location();
		if (at(TokenKind::OP_LSQUARE))
		{
			next();
			next();
			while (!at(TokenKind::OP_RSQUARE))
			{
				// attribute-token attribute-argument-clause? (7.6.1p1); a
				// gnu:: one of the GNU kinds above (aligned, section ...).
				Attribute attribute = Attribute();
				attribute.location = location();
				attribute.standard = true;
				const char* space = at_identifier() && at(TokenKind::OP_COLON2, 1) ? tok().identifier->spelling
																				   : nullptr;
				const bool gnu = space != nullptr &&
					(std::strcmp(space, "gnu") == 0 || std::strcmp(space, "__gnu__") == 0);
				const char* spelling = gnu && at_identifier(2) ? tok(2).identifier->spelling
					: gnu && at(TokenKind::KW_CONST, 2)        ? "const"
															   : "";
				bool skipped = false;
				if (std::strcmp(spelling, "section") == 0 || std::strcmp(spelling, "__section__") == 0)
				{
					next(), next(), next();
					if (!section_operand(attribute))
						return false;
					keep(attribute);
				}
				else if (std::strcmp(spelling, "abi_tag") == 0 || std::strcmp(spelling, "__abi_tag__") == 0)
				{
					next(), next(), next();
					if (!abi_tag_operands(attribute))
						return false;
				}
				else if (*spelling != '\0')
				{
					// Another gnu:: attribute: the GNU kinds as their
					// __attribute__ forms (aligned, packed, vector_size ...).
					next(), next(), next();
					bool kept = false;
					if (gnu_attribute(spelling, attribute, kept))
						keep(attribute);
					else if (kept)
						return false;
					else
						skipped = true;
				}
				else if (space == nullptr && at_identifier() &&
					(std::strcmp(tok().identifier->spelling, "no_unique_address") == 0 ||
						std::strcmp(tok().identifier->spelling, "__no_unique_address__") == 0))
				{
					next();
					attribute.kind = AttributeKind::NoUniqueAddress;
					keep(attribute);
				}
				else if (space == nullptr && at_identifier() &&
					(std::strcmp(tok().identifier->spelling, "noreturn") == 0 ||
						std::strcmp(tok().identifier->spelling, "__noreturn__") == 0))
				{
					// 7.6.3p1: no attribute-argument-clause shall be present.
					next();
					if (at(TokenKind::OP_LPAREN))
						return fail();
					attribute.kind = AttributeKind::NoReturn;
					keep(attribute);
				}
				else
					skipped = true;
				if (skipped)
				{
					while (!at(TokenKind::OP_COMMA) && !at(TokenKind::OP_RSQUARE))
					{
						if (at(TokenKind::OP_LPAREN) || at(TokenKind::OP_LSQUARE) || at(TokenKind::OP_LBRACE))
						{
							if (!skip_balanced())
								return false;
						}
						else if (tok().is_end() || at(TokenKind::OP_RPAREN) || at(TokenKind::OP_RBRACE))
							return fail();
						else
							next();
					}
				}
				if (!accept(TokenKind::OP_COMMA))
					break;
			}
			if (!expect(TokenKind::OP_RSQUARE) || !expect(TokenKind::OP_RSQUARE))
				return false;
			continue;
		}
		if (at(TokenKind::KW_ALIGNAS))
		{
			next();
			if (!expect(TokenKind::OP_LPAREN))
				return false;
			Attribute* attribute = make<Attribute>();
			*attribute = Attribute();
			attribute->kind = AttributeKind::Alignas;
			attribute->location = start;
			if (!parse_alignment_operand(*attribute) || !expect(TokenKind::OP_RPAREN))
				return false;
			*tail = attribute;
			tail = &attribute->next;
			continue;
		}
		next();
		if (!expect(TokenKind::OP_LPAREN) || !expect(TokenKind::OP_LPAREN))
			return false;
		while (!at(TokenKind::OP_RPAREN))
		{
			const bool named = at_identifier();
			// `const` is a keyword (GNU's const attribute).
			const char* spelling = named ? tok().identifier->spelling : at(TokenKind::KW_CONST) ? "const" : "";
			const uint32_t where = location();
			next();
			Attribute attribute = Attribute();
			attribute.location = where;
			bool kept = false;
			if (!gnu_attribute(spelling, attribute, kept))
			{
				// Its operand did not parse; else a kind of the parsers here,
				// or one skipped.
				if (kept)
					return false;
				if (std::strcmp(spelling, "section") == 0 || std::strcmp(spelling, "__section__") == 0)
				{
					if (!section_operand(attribute))
						return false;
					kept = true;
				}
				else if (std::strcmp(spelling, "abi_tag") == 0 || std::strcmp(spelling, "__abi_tag__") == 0)
				{
					if (!abi_tag_operands(attribute))
						return false;
				}
				else if (at(TokenKind::OP_LPAREN) && !skip_balanced())
					return false;
			}
			if (kept)
				keep(attribute);
			if (!accept(TokenKind::OP_COMMA))
				break;
		}
		if (!expect(TokenKind::OP_RPAREN) || !expect(TokenKind::OP_RPAREN))
			return false;
	}
	return true;
}

bool Parser::parse_alignment_operand(Attribute& attribute)
{
	// 7.6.2p1: alignas(type-id) or alignas(assignment-expression), a
	// type-id where one parses up to the closing parenthesis.
	if (name_starts_here())
	{
		const uint32_t start = location();
		NameInfo info;
		Name* name = parse_name(NameMode::Expression, info);
		if (name == nullptr)
			return false;
		if (is_type(name, info))
		{
			Tentative tentative(*this);
			TypeId* type = parse_type_id_from_name(name, info, start);
			if (type != nullptr && at(TokenKind::OP_RPAREN))
			{
				tentative.commit();
				attribute.type = type;
				return true;
			}
			if (abandoning_lists_)
				return false;
		}
		Expr* primary = primary_from_name(name, info, start);
		attribute.expression = primary == nullptr ? nullptr : continue_expression(primary);
		return attribute.expression != nullptr;
	}
	if (at_type_specifier_start())
	{
		Tentative tentative(*this);
		TypeId* type = parse_type_id(DeclaratorMode::Abstract, false);
		if (type != nullptr && at(TokenKind::OP_RPAREN))
		{
			tentative.commit();
			attribute.type = type;
			return true;
		}
		if (abandoning_lists_)
			return false;
	}
	attribute.expression = parse_assignment_expression();
	return attribute.expression != nullptr;
}

bool Parser::parse_decl_specifiers(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	for (;;)
	{
		// Those after a decl-specifier appertain to the type (7.1p1).
		Attribute** added = &pending_attributes_;
		while (*added != nullptr)
			added = &(*added)->next;
		if (!collect_attributes(pending_attributes_))
			return false;
		for (Attribute* attribute = *added; attribute != nullptr && list.size() != 0; attribute = attribute->next)
			attribute->on_type = true;
		vector_specifiers(*added, list);
		const ParseToken token = tok();
		if (token.kind == ParseTokenKind::Simple)
		{
			const TokenKind kind = token.simple;
			if (IsKeywordSpecifier(kind) || IsSimpleTypeKeyword(kind))
			{
				info.has_typedef = info.has_typedef || kind == TokenKind::KW_TYPEDEF;
				info.has_friend = info.has_friend || kind == TokenKind::KW_FRIEND;
				if (IsSimpleTypeKeyword(kind))
				{
					info.sign_only = SignOnly(info.has_type, info.sign_only, kind);
					info.complex_only = false;
				}
				info.has_type = info.has_type || IsSimpleTypeKeyword(kind);
				DeclSpecifier specifier = KeywordSpecifier(kind, token.location);
				next();
				if (kind == TokenKind::KW_EXPLICIT && at(TokenKind::OP_LPAREN))
				{
					specifier.operand = parse_decltype_operand();
					if (specifier.operand == nullptr)
						return false;
				}
				list.push(specifier);
				continue;
			}
			if (IsClassKey(kind) || kind == TokenKind::KW_ENUM)
			{
				if (info.has_type)
					break;
				if (!parse_class_or_enum_specifier(list, info))
					return false;
				continue;
			}
			if (kind == TokenKind::KW_TYPENAME || kind == TokenKind::KW_DECLTYPE || kind == TokenKind::OP_COLON2)
			{
				if (info.has_type)
					break;
				if (!parse_named_type_specifier(list, info))
				{
					if (kind == TokenKind::KW_TYPENAME)
						return fail();
					break;
				}
				continue;
			}
			break;
		}
		if (token.is_identifier() && !info.has_type && at_typeof())
		{
			if (!parse_typeof_specifier(list, info))
				return false;
			continue;
		}
		GnuTrait trait;
		if (token.is_identifier() && at_trait(trait) && IsGnuTypeTransform(trait) &&
			(!info.has_type || (info.sign_only && trait == GnuTrait::BitInt)))
		{
			if (!parse_transform_specifier(list, info, trait))
				return false;
			continue;
		}
		if (!token.is_identifier() ||
			(info.has_type && !(info.sign_only && token.identifier == extended_types_[0]) &&
				!(info.complex_only && ExtendedType(extended_types_, token.identifier)) &&
				gnu_names_.kind(token.identifier) != GnuNames::kComplex) ||
			!parse_named_type_specifier(list, info))
			break;
	}
	if (abandoning_lists_)
		return false;
	info.only_int = list.size() == 1 && list.back().kind == DeclSpecKind::Keyword &&
		list.back().keyword == TokenKind::KW_INT;
	return true;
}

void Parser::vector_specifiers(Attribute*& attributes, ListFrame<DeclSpecifier>& list)
{
	// A vector attribute among a decl-specifier-seq's makes the type a
	// vector: a specifier of its own, wherever the specifiers go (type-ids
	// too), taken out of the chain.
	Attribute** link = &attributes;
	while (*link != nullptr)
	{
		Attribute* attribute = *link;
		if (attribute->kind != AttributeKind::VectorSize && attribute->kind != AttributeKind::ExtVectorType)
		{
			link = &attribute->next;
			continue;
		}
		DeclSpecifier vector = DeclSpecifier();
		vector.kind = DeclSpecKind::Vector;
		vector.keyword = attribute->kind == AttributeKind::VectorSize ? TokenKind::KW_SIZEOF : TokenKind::OP_LSQUARE;
		vector.location = attribute->location;
		vector.operand = attribute->expression;
		list.push(vector);
		*link = attribute->next;
	}
}

void Parser::leading_vector_specifiers(const Attribute* attributes, ListFrame<DeclSpecifier>& list)
{
	// A vector attribute before a declaration's decl-specifiers makes their
	// type a vector too (GCC and Clang): its specifier, the attribute left
	// in the declaration's chain (which every reading of the declaration
	// starts from; the analysis skips vector attributes there).
	for (const Attribute* attribute = attributes; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->kind != AttributeKind::VectorSize && attribute->kind != AttributeKind::ExtVectorType)
			continue;
		DeclSpecifier vector = DeclSpecifier();
		vector.kind = DeclSpecKind::Vector;
		vector.keyword = attribute->kind == AttributeKind::VectorSize ? TokenKind::KW_SIZEOF : TokenKind::OP_LSQUARE;
		vector.location = attribute->location;
		vector.operand = attribute->expression;
		list.push(vector);
	}
}

bool Parser::parse_type_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	info = SpecifierInfo();
	return continue_type_specifier_seq(list, info);
}

bool Parser::continue_type_specifier_seq(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	for (;;)
	{
		// The attributes are dropped but for a vector's.
		Attribute* attributes = nullptr;
		if (!collect_attributes(attributes))
			return false;
		vector_specifiers(attributes, list);
		const ParseToken token = tok();
		if (token.kind == ParseTokenKind::Simple)
		{
			const TokenKind kind = token.simple;
			if (kind == TokenKind::KW_CONST || kind == TokenKind::KW_VOLATILE || IsSimpleTypeKeyword(kind))
			{
				if (IsSimpleTypeKeyword(kind))
				{
					info.sign_only = SignOnly(info.has_type, info.sign_only, kind);
					info.complex_only = false;
				}
				info.has_type = info.has_type || IsSimpleTypeKeyword(kind);
				list.push(KeywordSpecifier(kind, token.location));
				next();
				continue;
			}
			if (IsClassKey(kind) || kind == TokenKind::KW_ENUM)
			{
				if (info.has_type)
					break;
				if (!parse_class_or_enum_specifier(list, info))
					return false;
				continue;
			}
			if (kind == TokenKind::KW_TYPENAME || kind == TokenKind::KW_DECLTYPE || kind == TokenKind::OP_COLON2)
			{
				if (info.has_type || !parse_named_type_specifier(list, info))
					break;
				continue;
			}
			break;
		}
		if (token.is_identifier() && !info.has_type && at_typeof())
		{
			if (!parse_typeof_specifier(list, info))
				return false;
			continue;
		}
		GnuTrait trait;
		if (token.is_identifier() && at_trait(trait) && IsGnuTypeTransform(trait) &&
			(!info.has_type || (info.sign_only && trait == GnuTrait::BitInt)))
		{
			if (!parse_transform_specifier(list, info, trait))
				return false;
			continue;
		}
		if (!token.is_identifier() ||
			(info.has_type && !(info.sign_only && token.identifier == extended_types_[0]) &&
				!(info.complex_only && ExtendedType(extended_types_, token.identifier)) &&
				gnu_names_.kind(token.identifier) != GnuNames::kComplex) ||
			!parse_named_type_specifier(list, info))
			break;
	}
	if (abandoning_lists_)
		return false;
	if (!info.has_type)
		return fail();
	return true;
}

bool Parser::parse_transform_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info, GnuTrait trait)
{
	DeclSpecifier specifier = DeclSpecifier();
	specifier.kind = DeclSpecKind::Decltype;
	specifier.location = location();
	specifier.operand = parse_builtin_trait(trait);
	if (specifier.operand == nullptr)
		return false;
	list.push(specifier);
	info.has_type = true;
	info.sign_only = false;
	return true;
}

bool Parser::parse_typeof_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	// The decltype-specifier of a TypeofOperand.
	DeclSpecifier specifier = DeclSpecifier();
	specifier.kind = DeclSpecKind::Decltype;
	specifier.location = location();
	specifier.operand = parse_typeof_operand();
	if (specifier.operand == nullptr)
		return false;
	list.push(specifier);
	info.has_type = true;
	info.sign_only = false;
	return true;
}

bool Parser::parse_named_type_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	Tentative tentative(*this);
	DeclSpecifier specifier = DeclSpecifier();
	specifier.location = location();
	const bool complex = !info.has_type && tok().is_identifier() &&
		gnu_names_.kind(tok().identifier) == GnuNames::kComplex;
	const bool typename_keyword = accept(TokenKind::KW_TYPENAME);
	NameInfo name_info;
	const uint32_t parameters = template_parameter_mentions_;
	Name* name = parse_name(NameMode::Type, name_info);
	if (name == nullptr)
		return false;
	if (!name->global && name->components.size == 1 && name->last().kind == NameComponentKind::Decltype)
	{
		specifier.kind = DeclSpecKind::Decltype;
		specifier.operand = name->last().decltype_operand;
	}
	else
	{
		name->typename_keyword = typename_keyword;
		bool type = is_type(name, name_info);
		// An undeclared name directly followed by an identifier can only be a
		// type (the identifier is the declarator-id). Where a type is
		// expected, an undeclared name is one too, but not a dependent
		// qualified name (its prefix naming a template parameter, as in
		// `wrap<T>::value`): without `typename` that names no type (14.6p2).
		// GNU's undeclared `__real__` and `__imag__` are operators (`(int(
		// __real__ z))` is a functional cast's parenthesized operand).
		const bool dependent = name_info.prefix != PrefixState::Unqualified &&
			template_parameter_mentions_ != parameters;
		if (!type && name_info.category == NameCategory::None &&
			((definite_type_ && !dependent) ||
				(at_identifier() && !at_identifier(final_) && !at_identifier(override_))))
		{
			const uint8_t gnu = !name->global && name->components.size == 1 &&
					name->last().kind == NameComponentKind::Identifier
				? gnu_names_.kind(name->last().identifier)
				: 0;
			type = gnu != GnuNames::kRealPart && gnu != GnuNames::kImagPart;
		}
		// A namespace-scope declaration has no other reading (no statement
		// is read again there): a member some specializations of its
		// template declare as a type is one.
		if (!type && name_info.category == NameCategory::Value && name_info.prefix == PrefixState::Known &&
			scopes_.has_category_conflicts() && declaration_scope()->kind == ScopeKind::Namespace &&
			name->last().kind == NameComponentKind::Identifier)
		{
			const NameBinding found = scopes_.lookup_in(
				name_info.prefix_scope, name->last().identifier, false, name_info.prefix_specialization);
			type = found.conflicted &&
				(found.alternatives & (CategoryBit(NameCategory::Type) | CategoryBit(NameCategory::TypeTemplate))) != 0;
		}
		if (!type)
			return false;
		specifier.kind = DeclSpecKind::Name;
		specifier.name = name;
		info.type_scope = name_info.scope;
		info.type_template_member = name_info.template_member;
		info.type_name = name;
		info.type_name_info = name_info;
		info.type_mentions = template_parameter_mentions_ != parameters;
	}
	tentative.commit();
	list.push(specifier);
	info.has_type = true;
	info.sign_only = false;
	info.complex_only = complex;
	return true;
}

bool Parser::parse_class_or_enum_specifier(ListFrame<DeclSpecifier>& list, SpecifierInfo& info)
{
	DeclSpecifier specifier = DeclSpecifier();
	specifier.location = location();
	info.has_type = true;
	if (at(TokenKind::KW_ENUM))
	{
		specifier.kind = DeclSpecKind::Enum;
		specifier.enum_specifier = parse_enum_specifier();
		if (specifier.enum_specifier == nullptr)
			return false;
		info.defines_type = true;
		list.push(specifier);
		return true;
	}
	const TokenKind key = tok().simple;
	const uint32_t start = location();
	const uint64_t first_token = tokens_.absolute_position();
	const bool is_template = pending_template_;
	pending_template_ = false;
	next();
	Attribute* class_attributes = nullptr;
	if (!collect_attributes(class_attributes))
		return false;
	Name* name = nullptr;
	NameInfo name_info = NameInfo{ NameCategory::None, PrefixState::Unqualified, false, false, false, false, false,
		false, false, 0, nullptr, nullptr, nullptr };
	if (at_identifier() || at(TokenKind::OP_COLON2))
	{
		const bool saved_declared = naming_declared_;
		naming_declared_ = true;
		name = parse_name(NameMode::Type, name_info);
		naming_declared_ = saved_declared;
		if (name == nullptr)
			return false;
	}
	bool final = false;
	if (at_identifier(final_) && (at(TokenKind::OP_LBRACE, 1) || at(TokenKind::OP_COLON, 1)))
	{
		final = true;
		next();
	}
	// A class-key and `:` start a base-clause, the class named or not
	// (9p1: the class-head-name is optional).
	if (at(TokenKind::OP_LBRACE) || at(TokenKind::OP_COLON))
	{
		pending_template_ = is_template;
		specifier.kind = DeclSpecKind::Class;
		specifier.class_specifier = parse_class_body(
			key, name, final, start, name_info.prefix == PrefixState::Known ? name_info.prefix_scope : nullptr);
		if (specifier.class_specifier == nullptr)
			return false;
		// GNU attributes just past the closing brace are the class's too
		// (GCC's type attributes).
		if (tok().is_identifier() && gnu_names_.kind(tok().identifier) == GnuNames::kAttribute &&
			!collect_attributes(class_attributes))
			return false;
		specifier.class_specifier->first_token = first_token;
		specifier.class_specifier->attributes = class_attributes;
		specifier.class_specifier->pack = tokens_.pack_at(first_token);
		info.defines_type = true;
		list.push(specifier);
		return true;
	}
	if (name == nullptr)
		return fail();
	// An elaborated-type-specifier's ABI tags and GNU alignment are the
	// class's when the declaration declares it alone (see
	// Analyzer::simple_declaration; GCC's type attributes), and a standard
	// noreturn is kept for the analysis to reject (7.6.3p1); its other
	// attributes are dropped. They go before the declaration's in source
	// order (the first abi_tag attribute's arguments are the tags, see
	// Analyzer::collect_abi_tags).
	Attribute* tags = nullptr;
	Attribute** tail = &tags;
	for (Attribute* attribute = class_attributes; attribute != nullptr; attribute = attribute->next)
	{
		if (attribute->kind != AttributeKind::AbiTag &&
			!(attribute->kind == AttributeKind::Alignas && attribute->gnu) &&
			!(attribute->kind == AttributeKind::NoReturn && attribute->standard))
			continue;
		Attribute* node = make<Attribute>();
		*node = *attribute;
		node->on_class = true;
		*tail = node;
		tail = &node->next;
	}
	*tail = pending_attributes_;
	pending_attributes_ = tags;
	specifier.kind = DeclSpecKind::Elaborated;
	specifier.keyword = key;
	specifier.name = name;
	info.defines_type = true;
	info.type_scope = name_info.scope;
	if (!name->qualified() && !name->last().has_arguments && name->last().kind == NameComponentKind::Identifier)
	{
		const Identifier* identifier = name->last().identifier;
		if (!is_template && list.size() == 0 && at(TokenKind::OP_SEMICOLON))
		{
			// `class-key identifier ;` declares the class in this scope
			// whatever an enclosing scope declares (3.3.2p6, 7.1.6.3p2);
			// a same-scope object or function keeps hiding it (3.3.10p2).
			if (scopes_.own(declaration_scope(), identifier).category == NameCategory::None)
				scopes_.declare(declaration_scope(), identifier,
					NameBinding{ NameCategory::Type, false, class_scope_ahead(identifier) });
			list.push(specifier);
			return true;
		}
		// A template's declaration declares it in this scope, a friend's
		// names an enclosing scope's class or declares none here (its
		// definition elsewhere has a scope of its own).
		const NameBinding existing = is_template && !info.has_friend ? scopes_.own(declaration_scope(), identifier)
																	 : scopes_.lookup(identifier, true);
		Scope* declared = existing.scope;
		if (declared == nullptr && !info.has_friend)
			declared = class_scope_ahead(identifier);
		if (is_template)
			scopes_.declare(
				declaration_scope(), identifier, NameBinding{ NameCategory::TypeTemplate, false, declared });
		else if (existing.category == NameCategory::None && !info.has_friend)
		{
			// A value found here hides the class it names (a lookup before
			// `::` finds no other value): `struct stat st;` refers to that
			// class and leaves `stat` the function (3.4.4p2, 9.1p2).
			// `typedef struct X Y;` names the class it declares, in the
			// smallest enclosing namespace or block scope (3.3.2p7: `struct
			// X* p;` in a class declares ::X); a friend's in none that
			// lookup searches before its declaration there (7.3.1.2p3).
			Scope* where = declaration_scope();
			while (where->parent != nullptr && where->kind != ScopeKind::Namespace && where->kind != ScopeKind::Block)
				where = where->parent;
			scopes_.declare(where, identifier, NameBinding{ NameCategory::Type, false, declared });
			info.type_scope = declared;
		}
	}
	list.push(specifier);
	return true;
}

ClassSpecifier* Parser::parse_class_body(TokenKind key, Name* name, bool final, uint32_t start, Scope* member_of)
{
	ClassSpecifier* specifier = make<ClassSpecifier>();
	*specifier = ClassSpecifier();
	specifier->key = key;
	specifier->name = name;
	specifier->final = final;
	specifier->location = start;
	const bool is_template = pending_template_;
	pending_template_ = false;

	const Identifier* class_identifier = nullptr;
	bool specialization = false;
	if (name != nullptr && name->last().kind == NameComponentKind::Identifier)
	{
		class_identifier = name->last().identifier;
		specialization = name->last().has_arguments;
	}
	// `struct A::B { ... }` defines the B declared in A (9.7p3): a later
	// `A::B::` reaches these members. A template-id's prefix scope is its
	// primary template's, which shares its specializations' members
	// (link_specialization): `struct P<int>::Q { ... }` defines a Q a
	// specialization declared, taken as the earliest one not defined yet
	// (which specialization a template-id selects is semantic).
	const auto forward = [this, class_identifier](const NameBinding& binding) {
		return (binding.category == NameCategory::Type || binding.category == NameCategory::TypeTemplate) &&
			(binding.scope == nullptr || defined_later(binding, class_identifier));
	};
	const bool qualified_member = class_identifier != nullptr && name->qualified() && member_of != nullptr &&
		!specialization;
	Scope* home = member_of;
	NameBinding existing;
	if (qualified_member)
	{
		existing = scopes_.own(member_of, class_identifier);
		for (const ScopeEdge* edge = forward(existing) ? nullptr : member_of->edges; edge != nullptr; edge = edge->next)
		{
			const NameBinding candidate = edge->target->specialization ? scopes_.own(edge->target, class_identifier)
																	   : NameBinding();
			if (forward(candidate))
			{
				home = edge->target;
				existing = candidate;
			}
		}
		// `struct N::X { ... }` defines the X an inline namespace of N
		// declared (7.3.1p8), through nested inline namespaces too.
		for (Scope* space = member_of; space->kind == ScopeKind::Namespace && !forward(existing);)
		{
			Scope* inner = nullptr;
			for (const ScopeEdge* edge = space->edges; edge != nullptr && inner == nullptr; edge = edge->next)
			{
				if (edge->at == nullptr && edge->target->kind == ScopeKind::Namespace &&
					edge->target->name != nullptr &&
					scopes_.own(edge->target, class_identifier).category != NameCategory::None)
					inner = edge->target;
			}
			if (inner == nullptr)
				break;
			space = inner;
			const NameBinding candidate = scopes_.own(space, class_identifier);
			if (forward(candidate))
			{
				home = space;
				existing = candidate;
			}
		}
	}
	else if (class_identifier != nullptr && !name->qualified() && !specialization)
		existing = scopes_.own(declaration_scope(), class_identifier);
	Scope* enclosing = scopes_.current();
	Scope* scope = nullptr;
	if (defined_later(existing, class_identifier))
	{
		scope = existing.scope;
		scopes_.reopen(scope);
		if (is_template && scope->edges != nullptr)
			scopes_.note_late_primary(class_identifier);
	}
	else
		scope = scopes_.push(ScopeKind::Class, class_identifier);
	// A class defined out of its enclosing class (`struct A::B { ... }`)
	// sees A's members (9.7p3).
	scope->member_of = member_of;
	scopes_.set_current(enclosing);
	if (class_identifier != nullptr && !name->qualified())
	{
		if (specialization)
			link_specialization(class_identifier, scope);
		else
			declare_class(class_identifier, is_template, scope);
	}
	else if (qualified_member && forward(existing) && existing.scope != scope)
		scopes_.declare(home, class_identifier, NameBinding{ existing.category, false, scope });
	if (class_identifier != nullptr)
	{
		// The injected-class-name (9p2).
		scopes_.declare(scope, class_identifier,
			NameBinding{
				is_template || specialization ? NameCategory::TypeTemplate : NameCategory::Type, false, scope });
	}

	if (at(TokenKind::OP_COLON))
	{
		specifier->has_base_clause = true;
		ListFrame<BaseSpecifier> bases(bases_);
		if (!parse_base_clause(bases, scope, is_template && !specialization && class_identifier != nullptr))
			return nullptr;
		specifier->bases = bases.take(arena_);
	}
	if (!expect(TokenKind::OP_LBRACE))
		return nullptr;

	scopes_.enter(scope);
	class_names_.push_back(class_identifier);
	const size_t first_deferred = deferred_.size();
	const Arena::Mark runs = runs_.mark();
	SpeculationBoundary boundary(*this);
	ListFrame<Decl*> members(decls_);
	Attribute* enclosing_attributes = pending_attributes_;
	const bool parsed = parse_declaration_seq(members, true, DeclContext::Class) && expect(TokenKind::OP_RBRACE);
	pending_attributes_ = enclosing_attributes;
	specifier->end_token = tokens_.absolute_position();
	class_names_.pop_back();
	scopes_.set_current(enclosing);
	if (!parsed)
		return nullptr;
	scopes_.complete_class(scope, is_template || specialization);
	specifier->members = members.take(arena_);
	if (class_names_.empty())
	{
		// The bodies are parsed into the tree; their tokens are dead.
		if (!parse_deferred_bodies(first_deferred))
			return nullptr;
		telemetry::note_peak(peak_run_bytes_, runs_.bytes_reserved());
		runs_.rewind(runs);
	}
	return specifier;
}

void Parser::link_specialization(const Identifier* template_name, Scope* scope)
{
	// Members of a partial or explicit specialization are found through the
	// primary template's scope (a parsing approximation: which
	// specialization a template-id selects is a semantic question). A
	// primary that is only declared gets a forwarding scope for the edges.
	const NameBinding primary = scopes_.lookup(template_name, true);
	if (primary.category != NameCategory::TypeTemplate)
		return;
	Scope* target = primary.scope;
	if (target == nullptr)
	{
		target = class_scope_ahead(template_name);
		scopes_.declare(declaration_scope(), template_name, NameBinding{ NameCategory::TypeTemplate, false, target });
	}
	// The specialization's scope was just made: no edge reaches it yet.
	scopes_.add_specialization(target, scope);
}

Scope* Parser::class_scope_ahead(const Identifier* name)
{
	Scope* current = scopes_.current();
	Scope* scope = scopes_.push(ScopeKind::Class, name);
	scopes_.set_current(current);
	return scope;
}

bool Parser::defined_later(const NameBinding& binding, const Identifier* name) const
{
	// Its definition declares the injected-class-name (undone with it).
	// A class an object or function of its scope hides keeps its scope
	// (Merge): its definition reopens it too.
	Scope* scope = binding.scope;
	return scope != nullptr && name != nullptr && scope->kind == ScopeKind::Class && scope->name == name &&
		!scope->specialization &&
		(binding.category == NameCategory::Type || binding.category == NameCategory::TypeTemplate ||
			binding.category == NameCategory::Value) &&
		scopes_.own(scope, name).category == NameCategory::None;
}

bool ParserState::scope_pending(Scope* scope) const
{
	return scope->kind == ScopeKind::Class && scope->name != nullptr && !scope->specialization &&
		scope->edges == nullptr && scopes_.own(scope, scope->name).category == NameCategory::None;
}

void Parser::declare_class(const Identifier* name, bool is_template, Scope* scope)
{
	// A class defined after its specializations, when another scope than
	// the one they were linked to holds its members, keeps theirs reachable
	// through an edge.
	Scope* where = declaration_scope();
	const NameBinding existing = scopes_.own(where, name);
	if (existing.scope != nullptr && existing.scope != scope &&
		(existing.category == NameCategory::TypeTemplate || existing.category == NameCategory::Type))
		scopes_.add_edge(scope, existing.scope);
	scopes_.declare(
		where, name, NameBinding{ is_template ? NameCategory::TypeTemplate : NameCategory::Type, false, scope });
}

bool Parser::parse_base_clause(ListFrame<BaseSpecifier>& bases, Scope* class_scope, bool primary_template)
{
	// Parsed in the enclosing scope; the class scope gets an edge to each
	// base whose scope is known, for lookups in the class (10.2); a base
	// naming a template parameter is searched only in a specialization's
	// members: unqualified lookup does not search a dependent base
	// (14.6.2p3).
	if (!expect(TokenKind::OP_COLON))
		return false;
	PointerMap reached;
	do
	{
		BaseSpecifier base = BaseSpecifier();
		base.location = location();
		if (!skip_attributes())
			return false;
		if (accept(TokenKind::KW_VIRTUAL))
		{
			base.is_virtual = true;
			base.virtual_first = true;
		}
		if (at(TokenKind::KW_PUBLIC) || at(TokenKind::KW_PROTECTED) || at(TokenKind::KW_PRIVATE))
		{
			base.has_access = true;
			base.access = tok().simple;
			next();
		}
		if (!base.is_virtual && accept(TokenKind::KW_VIRTUAL))
			base.is_virtual = true;
		NameInfo info;
		const uint32_t mentions = pack_mentions_;
		const uint32_t parameters = template_parameter_mentions_;
		base.name = parse_name(NameMode::Type, info);
		if (base.name == nullptr)
			return false;
		base.pack = accept(TokenKind::OP_DOTS);
		if (base.pack)
			pack_mentions_ = mentions;
		if (info.scope != nullptr)
		{
			scopes_.add_base_edge(class_scope, info.scope, template_parameter_mentions_ != parameters, reached);
			if (class_scope->specialization)
				scopes_.note_specialization_base(class_scope);
		}
		else if (primary_template && template_parameter_mentions_ != parameters && !base.name->global &&
			base.name->components.size == 1 && base.name->last().kind == NameComponentKind::Identifier &&
			!base.name->last().has_arguments)
			record_parameter_base(class_scope, base.name->last().identifier, base.pack);
		bases.push(base);
	} while (accept(TokenKind::OP_COMMA));
	return true;
}

EnumSpecifier* Parser::parse_enum_specifier()
{
	EnumSpecifier* specifier = make<EnumSpecifier>();
	*specifier = EnumSpecifier();
	specifier->location = location();
	if (!expect(TokenKind::KW_ENUM))
		return nullptr;
	if (at(TokenKind::KW_CLASS) || at(TokenKind::KW_STRUCT))
	{
		specifier->scoped = true;
		specifier->key = tok().simple;
		next();
	}
	// Its abi_tag attributes (GCC mangles them) and a standard noreturn,
	// which the analysis rejects (7.6.3p1); the others are ignored.
	Attribute* attributes = nullptr;
	if (!collect_attributes(attributes))
		return nullptr;
	Attribute** link = &specifier->attributes;
	for (Attribute* attribute = attributes; attribute != nullptr;)
	{
		Attribute* const following = attribute->next;
		if (attribute->kind == AttributeKind::AbiTag ||
			(attribute->kind == AttributeKind::NoReturn && attribute->standard))
		{
			attribute->next = nullptr;
			*link = attribute;
			link = &attribute->next;
		}
		attribute = following;
	}
	const Identifier* simple_name = nullptr;
	if (at_identifier() || at(TokenKind::OP_COLON2))
	{
		NameInfo info;
		const bool saved_declared = naming_declared_;
		naming_declared_ = true;
		specifier->name = parse_name(NameMode::Type, info);
		naming_declared_ = saved_declared;
		if (specifier->name == nullptr)
			return nullptr;
		if (!specifier->name->qualified() && !specifier->name->last().has_arguments)
			simple_name = specifier->name->last().identifier;
	}
	if (accept(TokenKind::OP_COLON))
	{
		TypeId* underlying = make<TypeId>();
		underlying->location = location();
		underlying->declarator = nullptr;
		SpecifierInfo info;
		ListFrame<DeclSpecifier> specifiers(specs_);
		if (!parse_type_specifier_seq(specifiers, info))
			return nullptr;
		underlying->specifiers = specifiers.take(arena_);
		specifier->underlying = underlying;
	}
	Scope* enclosing = scopes_.current();
	if (!at(TokenKind::OP_LBRACE))
	{
		if (specifier->name == nullptr)
		{
			fail();
			return nullptr;
		}
		if (simple_name != nullptr)
		{
			const NameBinding existing = scopes_.lookup(simple_name, true);
			if (existing.category != NameCategory::Type)
				scopes_.declare(declaration_scope(), simple_name, NameBinding{ NameCategory::Type, false, nullptr });
		}
		return specifier;
	}
	specifier->has_body = true;
	Scope* scope = scopes_.push(ScopeKind::Enum, simple_name);
	scopes_.set_current(enclosing);
	Scope* members = declaration_scope();
	if (simple_name != nullptr)
		scopes_.declare(members, simple_name, NameBinding{ NameCategory::Type, false, scope });
	next();
	ListFrame<Enumerator> enumerators(enumerators_);
	while (!at(TokenKind::OP_RBRACE))
	{
		Enumerator enumerator = Enumerator();
		enumerator.location = location();
		if (!at_identifier())
		{
			fail();
			return nullptr;
		}
		enumerator.name = tok().identifier;
		next();
		if (!skip_attributes())
			return nullptr;
		if (accept(TokenKind::OP_ASS))
		{
			enumerator.value = parse_assignment_expression();
			if (enumerator.value == nullptr)
				return nullptr;
		}
		scopes_.declare(scope, enumerator.name, NameBinding{ NameCategory::Value, false, nullptr });
		if (!specifier->scoped)
			scopes_.declare(members, enumerator.name, NameBinding{ NameCategory::Value, false, nullptr });
		enumerators.push(enumerator);
		if (!accept(TokenKind::OP_COMMA))
			break;
		specifier->trailing_comma = at(TokenKind::OP_RBRACE);
	}
	if (!expect(TokenKind::OP_RBRACE))
		return nullptr;
	specifier->enumerators = enumerators.take(arena_);
	return specifier;
}

} // namespace syntax
