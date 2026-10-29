#include "syntax/parser.h"

#include <string>
#include <vector>

#include "lex/identifier_table.h"

namespace syntax {

namespace {

bool IsMemberFunctionSpecifier(TokenKind kind)
{
	return kind == TokenKind::KW_INLINE || kind == TokenKind::KW_VIRTUAL || kind == TokenKind::KW_EXPLICIT ||
		kind == TokenKind::KW_CONSTEXPR || kind == TokenKind::KW_FRIEND || kind == TokenKind::KW_STATIC;
}

bool IsAccessSpecifier(TokenKind kind)
{
	return kind == TokenKind::KW_PUBLIC || kind == TokenKind::KW_PROTECTED || kind == TokenKind::KW_PRIVATE;
}

} // namespace

Decl* Parser::parse_declaration(DeclContext context, Attribute* leading)
{
	NestingGuard nesting(*this);
	const uint32_t start = location();
	// The declaration's leading attributes, which every reading of it
	// starts from (a nested declaration has its own).
	struct LeadingAttributes
	{
		Attribute*& slot;
		Attribute* saved;
		LeadingAttributes(Attribute*& s, Attribute* first) : slot(s), saved(s) { slot = first; }
		~LeadingAttributes() { slot = saved; }
	} restore(leading_attributes_, leading);
	if (at_attribute())
	{
		if (!collect_attributes(leading_attributes_))
			return nullptr;
		if (accept(TokenKind::OP_SEMICOLON))
			return make<Decl>(DeclKind::Empty, start);
	}
	const ParseToken token = tok();
	if (token.kind == ParseTokenKind::Simple)
	{
		switch (token.simple)
		{
		case TokenKind::KW_ASM:
			if (context == DeclContext::Class)
			{
				fail();
				return nullptr;
			}
			return parse_asm_declaration();
		case TokenKind::OP_SEMICOLON:
			next();
			return make<Decl>(DeclKind::Empty, start);
		case TokenKind::KW_NAMESPACE:
			return parse_namespace();
		case TokenKind::KW_INLINE:
			if (at(TokenKind::KW_NAMESPACE, 1))
				return parse_namespace();
			break;
		case TokenKind::KW_USING:
			return parse_using();
		case TokenKind::KW_TEMPLATE:
			if (at(TokenKind::OP_LT, 1))
				return parse_template_declaration(context);
			return parse_explicit_instantiation(context);
		case TokenKind::KW_EXTERN:
			if (tok(1).is_literal() && (tok(1).flags & kStringLiteral) != 0)
				return parse_linkage_specification(context);
			if (at(TokenKind::KW_TEMPLATE, 1))
				return parse_explicit_instantiation(context);
			break;
		case TokenKind::KW_STATIC_ASSERT:
			return parse_static_assert();
		default:
			if (context == DeclContext::Class && IsAccessSpecifier(token.simple) && at(TokenKind::OP_COLON, 1))
			{
				AccessDecl* access = make<AccessDecl>(DeclKind::Access, start);
				access->access = token.simple;
				next();
				next();
				return access;
			}
			break;
		}
	}
	if (context != DeclContext::Block && at_deduction_guide())
		return parse_deduction_guide(start);
	Decl* special = nullptr;
	if (context != DeclContext::Block && try_special_member(context, special))
		return special;
	return parse_simple_or_function(context);
}

bool Parser::at_structured_binding()
{
	// C++17's structured binding declaration: after the decl-specifiers, a
	// ref-qualifier? then `[` (no attribute's `[[`).
	const uint32_t ahead = at(TokenKind::OP_AMP) || at(TokenKind::OP_LAND) ? 1 : 0;
	return at(TokenKind::OP_LSQUARE, ahead) && !at(TokenKind::OP_LSQUARE, ahead + 1);
}

Declarator* Parser::binding_declarator(const Identifier* name, TokenKind ref, uint32_t location)
{
	// `ref name`: the object e's (its ref-qualifier, or none: OP_COMMA) or
	// a binding's name.
	Declarator* declarator = make<Declarator>();
	*declarator = Declarator();
	declarator->location = location;
	if (ref != TokenKind::OP_COMMA)
	{
		PtrOperator* pointer = make<PtrOperator>();
		*pointer = PtrOperator();
		pointer->kind = ref;
		declarator->pointers = Slice<PtrOperator>{ pointer, 1 };
	}
	declarator->name = make_simple_name(name, location);
	return declarator;
}

bool Parser::parse_binding_names(TokenKind& ref, const Identifier*& hidden, std::vector<Declarator*>& names)
{
	// ref-qualifier? [ identifier-list ]: the names, and the name of the
	// object e they bind to (C++17's 11.5p1: unique, so unspellable): the
	// list as Itanium 5.1.4 shows the declaration, `[a, b]` (no other
	// declaration of one scope has it while the names are unique).
	ref = TokenKind::OP_COMMA;
	if (at(TokenKind::OP_AMP) || at(TokenKind::OP_LAND))
	{
		ref = tok().simple;
		next();
	}
	next();
	std::string spelling = "[";
	do
	{
		if (!at_identifier())
			return fail();
		const Identifier* name = tok().identifier;
		names.push_back(binding_declarator(name, TokenKind::OP_COMMA, location()));
		if (spelling.size() > 1)
			spelling += ", ";
		spelling.append(name->spelling, name->length);
		next();
	} while (accept(TokenKind::OP_COMMA));
	if (!expect(TokenKind::OP_RSQUARE))
		return false;
	spelling += ']';
	hidden = identifiers_.intern(spelling.data(), static_cast<uint32_t>(spelling.size()));
	return true;
}

Slice<InitDeclarator> Parser::binding_declarators(
	const std::vector<Declarator*>& names, const SpecifierInfo& info, const InitDeclarator& object)
{
	// The object e first, then each name alone: the analysis binds it to
	// its element of e (sema/analyzer_bindings.cpp).
	const uint32_t count = static_cast<uint32_t>(names.size() + 1);
	InitDeclarator* declarators = arena_.allocate_array<InitDeclarator>(count);
	declarators[0] = object;
	for (uint32_t i = 1; i < count; ++i)
	{
		declare_declarator(names[i - 1], info);
		declarators[i] = InitDeclarator();
		declarators[i].declarator = names[i - 1];
		declarators[i].initializer = nullptr;
	}
	return Slice<InitDeclarator>{ declarators, count };
}

Decl* Parser::parse_structured_binding(Slice<DeclSpecifier> specifiers, const SpecifierInfo& info, uint32_t start)
{
	// `specifiers ref? [ names ] initializer ;` (C++17's 11.5, an extension
	// here): one simple-declaration of the object e, `specifiers ref e
	// initializer`, and the names.
	TokenKind ref;
	const Identifier* hidden = nullptr;
	std::vector<Declarator*> names;
	if (!parse_binding_names(ref, hidden, names))
		return nullptr;
	if (!at(TokenKind::OP_ASS) && !at(TokenKind::OP_LBRACE) && !at(TokenKind::OP_LPAREN))
	{
		fail();
		return nullptr;
	}
	InitDeclarator object = InitDeclarator();
	object.declarator = binding_declarator(hidden, ref, start);
	object.initializer = parse_initializer(true);
	if (object.initializer == nullptr || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	SimpleDecl* decl = make<SimpleDecl>(DeclKind::Simple, start);
	decl->specifiers = specifiers;
	decl->attributes = pending_attributes_;
	pending_attributes_ = nullptr;
	decl->declarators = binding_declarators(names, info, object);
	decl->decomposition = true;
	return decl;
}

bool Parser::at_deduction_guide()
{
	// C++17's deduction-guide: `explicit`? template-name ( parameter-
	// declaration-clause ) -> simple-template-id ;
	const uint32_t ahead = at(TokenKind::KW_EXPLICIT) ? 1 : 0;
	if (!at_identifier(ahead) || !at(TokenKind::OP_LPAREN, ahead + 1) ||
		scopes_.lookup(tok(ahead).identifier, false).category != NameCategory::TypeTemplate)
		return false;
	uint32_t depth = 0;
	for (uint32_t i = ahead + 1;; ++i)
	{
		const ParseToken token = tok(i);
		if (token.is_end())
			return false;
		if (token.is(TokenKind::OP_LPAREN))
			++depth;
		else if (token.is(TokenKind::OP_RPAREN) && --depth == 0)
			return at(TokenKind::OP_ARROW, i + 1);
	}
}

Decl* Parser::parse_deduction_guide(uint32_t start)
{
	// A guide only class template argument deduction (C++17) reads, which
	// C++11 code never performs: parsed and dropped (hosted headers declare
	// them in later-standard sections their C++11 mode still reads).
	accept(TokenKind::KW_EXPLICIT);
	next();
	if (!skip_balanced() || !expect(TokenKind::OP_ARROW) || parse_type_id() == nullptr ||
		!expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	return make<Decl>(DeclKind::Empty, start);
}

Decl* Parser::parse_namespace()
{
	const uint32_t start = location();
	const bool is_inline = accept(TokenKind::KW_INLINE);
	if (!expect(TokenKind::KW_NAMESPACE))
		return nullptr;
	if (at_identifier() && at(TokenKind::OP_ASS, 1))
	{
		NamespaceAliasDecl* alias = make<NamespaceAliasDecl>(DeclKind::NamespaceAlias, start);
		alias->name = tok().identifier;
		next();
		next();
		NameInfo info;
		alias->target = parse_name(NameMode::Type, info);
		if (alias->target == nullptr || !expect(TokenKind::OP_SEMICOLON))
			return nullptr;
		scopes_.declare(scopes_.current(), alias->name, NameBinding{ NameCategory::Namespace, false, info.scope });
		return alias;
	}
	// Attributes before or (GNU ones) after the name: abi_tag is kept.
	Attribute* attributes = nullptr;
	if (!collect_attributes(attributes))
		return nullptr;
	NamespaceDecl* ns = make<NamespaceDecl>(DeclKind::Namespace, start);
	ns->is_inline = is_inline;
	ns->name = nullptr;
	if (at_identifier())
	{
		ns->name = tok().identifier;
		next();
	}
	if (!collect_attributes(attributes) || !expect(TokenKind::OP_LBRACE))
		return nullptr;
	ns->attributes = attributes;
	// A reopened namespace continues its scope; an unnamed or inline one is
	// nominated by its enclosing namespace (7.3.1.1, 7.3.1p8).
	Scope* enclosing = scopes_.current();
	const Identifier* key = ns->name != nullptr ? ns->name : unnamed_namespace_;
	const NameBinding existing = scopes_.own(enclosing, key);
	Scope* scope = existing.category == NameCategory::Namespace ? existing.scope : nullptr;
	if (scope == nullptr)
	{
		scope = scopes_.push(ScopeKind::Namespace, ns->name);
		scopes_.set_current(enclosing);
		scopes_.declare(enclosing, key, NameBinding{ NameCategory::Namespace, false, scope });
	}
	if (ns->name == nullptr || is_inline)
		scopes_.add_edge(enclosing, scope);
	scopes_.enter(scope);
	ListFrame<Decl*> declarations(decls_);
	const bool parsed = parse_declaration_seq(declarations, true, DeclContext::Namespace) &&
		expect(TokenKind::OP_RBRACE);
	scopes_.set_current(enclosing);
	if (!parsed)
		return nullptr;
	ns->declarations = declarations.take(arena_);
	return ns;
}

Decl* Parser::parse_using()
{
	const uint32_t start = location();
	const bool is_template = pending_template_;
	pending_template_ = false;
	if (!expect(TokenKind::KW_USING))
		return nullptr;
	if (accept(TokenKind::KW_NAMESPACE))
	{
		UsingDecl* directive = make<UsingDecl>(DeclKind::UsingDirective, start);
		directive->typename_keyword = false;
		directive->if_exists = false;
		NameInfo info;
		directive->target = parse_name(NameMode::Type, info);
		if (directive->target == nullptr || !expect(TokenKind::OP_SEMICOLON))
			return nullptr;
		if (info.category == NameCategory::Namespace && info.scope != nullptr)
			scopes_.add_using_directive(scopes_.current(), info.scope);
		return directive;
	}
	if (at_identifier() && (at(TokenKind::OP_ASS, 1) || at_attribute(1)))
	{
		AliasDecl* alias = make<AliasDecl>(DeclKind::Alias, start);
		alias->name = tok().identifier;
		next();
		Attribute* attributes = nullptr;
		if (!collect_attributes(attributes) || !expect(TokenKind::OP_ASS))
			return nullptr;
		alias->attributes = attributes;
		SpecifierInfo type_info = SpecifierInfo();
		alias->type = parse_type_id(DeclaratorMode::Abstract, true, &type_info);
		if (alias->type == nullptr || !expect(TokenKind::OP_SEMICOLON))
			return nullptr;
		// A vector attribute of the alias makes its type a vector (GCC and
		// Clang), as among a typedef's specifiers.
		if (attributes != nullptr)
		{
			ListFrame<DeclSpecifier> specifiers(specs_);
			for (const DeclSpecifier& specifier : alias->type->specifiers)
				specifiers.push(specifier);
			const size_t written = specifiers.size();
			type_attributes(attributes, specifiers);
			if (specifiers.size() != written)
			{
				alias->type->specifiers = specifiers.take(arena_);
				for (uint32_t i = static_cast<uint32_t>(written); i < alias->type->specifiers.size; ++i)
					if (alias->type->specifiers[i].kind == DeclSpecKind::Mode)
						alias->type->specifiers[i].keyword = TokenKind::KW_USING;
			}
		}
		// An alias template of a member a template-id reaches (`typename
		// traits<T>::type`) has no known class: a specialization, declared
		// before or after it, may make the member another class for its
		// arguments. One of a class or template-id names its type's members
		// (`template<class T> using A = B<T>;`), as another alias does.
		NameBinding binding{ is_template ? NameCategory::TypeTemplate : NameCategory::Type, false,
			is_template && type_info.type_template_member ? nullptr : type_id_scope_ };
		note_type_name(type_info, binding, is_template);
		if (!is_template)
			binding.alternatives = type_id_record_;
		scopes_.declare(declaration_scope(), alias->name, binding);
		return alias;
	}
	UsingDecl* declaration = make<UsingDecl>(DeclKind::UsingDeclaration, start);
	declaration->typename_keyword = accept(TokenKind::KW_TYPENAME);
	declaration->if_exists = false;
	NameInfo info;
	declaration->target = parse_name(NameMode::Declarator, info);
	// GNU attributes after the name (Clang's using_if_exists).
	Attribute* attributes = nullptr;
	if (declaration->target == nullptr || !collect_attributes(attributes) || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	for (const Attribute* attribute = attributes; attribute != nullptr; attribute = attribute->next)
		declaration->if_exists = declaration->if_exists || attribute->kind == AttributeKind::UsingIfExists;
	const NameComponent& last = declaration->target->last();
	if (last.kind == NameComponentKind::Identifier)
	{
		NameBinding binding{ info.category, false, info.scope };
		if (info.specialization && binding.scope != nullptr)
			binding.flags |= NameBinding::kSpecialization;
		if (declaration->typename_keyword && binding.category == NameCategory::None)
			binding.category = NameCategory::Type;
		if (binding.category != NameCategory::None)
			scopes_.declare(scopes_.current(), last.identifier, binding);
	}
	return declaration;
}

Decl* Parser::parse_static_assert()
{
	StaticAssertDecl* assertion = make<StaticAssertDecl>(DeclKind::StaticAssert, location());
	assertion->has_message = false;
	assertion->user_defined_message = false;
	if (!expect(TokenKind::KW_STATIC_ASSERT) || !expect(TokenKind::OP_LPAREN))
		return nullptr;
	assertion->condition = parse_assignment_expression();
	if (assertion->condition == nullptr)
		return nullptr;
	if (accept(TokenKind::OP_COMMA))
	{
		if (!tok().is_literal() || (tok().flags & kStringLiteral) == 0)
		{
			fail();
			return nullptr;
		}
		assertion->has_message = true;
		assertion->user_defined_message = tok().kind == ParseTokenKind::UserDefinedLiteral;
		assertion->message = tok().spelling;
		next();
	}
	if (!expect(TokenKind::OP_RPAREN) || !expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	return assertion;
}

Decl* Parser::parse_linkage_specification(DeclContext context)
{
	LinkageDecl* linkage = make<LinkageDecl>(DeclKind::Linkage, location());
	next();
	// The string-literal's contents, without its quotes.
	const TokenSpelling literal = tok().spelling;
	linkage->language = TokenSpelling{ literal.text + 1, literal.length >= 2 ? literal.length - 2 : 0 };
	next();
	ListFrame<Decl*> declarations(decls_);
	linkage->braced = accept(TokenKind::OP_LBRACE);
	if (linkage->braced)
	{
		if (!parse_declaration_seq(declarations, true, context) || !expect(TokenKind::OP_RBRACE))
			return nullptr;
	}
	else
	{
		Decl* decl = parse_declaration(context);
		if (decl == nullptr)
			return nullptr;
		declarations.push(decl);
	}
	linkage->declarations = declarations.take(arena_);
	return linkage;
}

Decl* Parser::parse_template_declaration(DeclContext context)
{
	TemplateDecl* decl = make<TemplateDecl>(DeclKind::Template, location());
	if (!expect(TokenKind::KW_TEMPLATE))
		return nullptr;
	Scope* enclosing = scopes_.current();
	scopes_.push(ScopeKind::Template, nullptr);
	decl->parameters = parse_template_parameter_clause();
	if (decl->parameters == nullptr)
		return nullptr;
	// An explicit specialization (`template<>`) declares no new template.
	pending_template_ = !decl->parameters->parameters.empty();
	const TemplateParameterList* const enclosing_head = template_head_;
	template_head_ = decl->parameters;
	decl->declaration = parse_declaration(context);
	template_head_ = enclosing_head;
	pending_template_ = false;
	scopes_.set_current(enclosing);
	if (decl->declaration == nullptr)
		return nullptr;
	return decl;
}

Decl* Parser::parse_explicit_instantiation(DeclContext context)
{
	ExplicitInstantiationDecl* decl = make<ExplicitInstantiationDecl>(DeclKind::ExplicitInstantiation, location());
	decl->is_extern = accept(TokenKind::KW_EXTERN);
	if (!expect(TokenKind::KW_TEMPLATE))
		return nullptr;
	decl->declaration = parse_declaration(context);
	if (decl->declaration == nullptr)
		return nullptr;
	return decl;
}

TemplateParameterList* Parser::parse_template_parameter_clause()
{
	TemplateParameterList* list = make<TemplateParameterList>();
	list->location = location();
	if (!expect(TokenKind::OP_LT))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = true;
	ListFrame<TemplateParameter*> parameters(template_params_);
	bool parsed = true;
	if (!at_close_angle())
	{
		do
		{
			TemplateParameter* parameter = parse_template_parameter();
			if (parameter == nullptr)
			{
				parsed = false;
				break;
			}
			parameters.push(parameter);
		} while (accept(TokenKind::OP_COMMA));
	}
	angle_ = saved_angle;
	if (!parsed || !at_close_angle())
	{
		fail();
		return nullptr;
	}
	next();
	list->parameters = parameters.take(arena_);
	return list;
}

TemplateParameter* Parser::parse_template_parameter()
{
	TemplateParameter* parameter = make<TemplateParameter>();
	*parameter = TemplateParameter();
	parameter->location = location();
	if (at(TokenKind::KW_TEMPLATE))
	{
		// A template template-parameter; its own parameters are scoped.
		parameter->kind = TemplateParameterKind::Template;
		next();
		Scope* enclosing = scopes_.current();
		scopes_.push(ScopeKind::Template, nullptr);
		parameter->parameters = parse_template_parameter_clause();
		scopes_.set_current(enclosing);
		if (parameter->parameters == nullptr)
			return nullptr;
		if (!at(TokenKind::KW_CLASS) && !at(TokenKind::KW_TYPENAME))
		{
			fail();
			return nullptr;
		}
		parameter->key = tok().simple;
		next();
		parameter->pack = accept(TokenKind::OP_DOTS);
		if (at_identifier())
		{
			parameter->name = tok().identifier;
			next();
		}
		if (accept(TokenKind::OP_ASS))
		{
			parameter->default_type = parse_type_id();
			if (parameter->default_type == nullptr)
				return nullptr;
		}
		if (parameter->name != nullptr)
			scopes_.declare(scopes_.current(), parameter->name,
				NameBinding{ NameCategory::TypeTemplate, parameter->pack, nullptr, true });
		return parameter;
	}
	const bool type_key = at(TokenKind::KW_CLASS) || at(TokenKind::KW_TYPENAME);
	const bool type_parameter = type_key &&
		(at(TokenKind::OP_DOTS, 1) || at(TokenKind::OP_COMMA, 1) || at(TokenKind::OP_GT, 1) ||
			at(TokenKind::OP_ASS, 1) ||
			(at_identifier(1) && (at(TokenKind::OP_COMMA, 2) || at(TokenKind::OP_GT, 2) || at(TokenKind::OP_ASS, 2))));
	if (type_parameter)
	{
		parameter->kind = TemplateParameterKind::Type;
		parameter->key = tok().simple;
		next();
		parameter->pack = accept(TokenKind::OP_DOTS);
		if (at_identifier())
		{
			parameter->name = tok().identifier;
			next();
		}
		if (accept(TokenKind::OP_ASS))
		{
			parameter->default_type = parse_type_id();
			if (parameter->default_type == nullptr)
				return nullptr;
		}
		// A parameter's kind replaces what its name meant before.
		if (parameter->name != nullptr)
			scopes_.declare(
				scopes_.current(), parameter->name, NameBinding{ NameCategory::Type, parameter->pack, nullptr, true });
		return parameter;
	}
	parameter->kind = TemplateParameterKind::NonType;
	SpecifierInfo info;
	{
		ListFrame<DeclSpecifier> specifiers(specs_);
		if (!parse_decl_specifier_seq(specifiers, info))
			return nullptr;
		if (specifiers.size() == 0)
		{
			fail();
			return nullptr;
		}
		parameter->specifiers = specifiers.take(arena_);
	}
	parameter->pack = accept(TokenKind::OP_DOTS);
	Declarator* declarator = parse_declarator(DeclaratorMode::Parameter);
	if (declarator == nullptr)
		return nullptr;
	if (!declarator_is_empty(declarator))
		parameter->declarator = declarator;
	if (accept(TokenKind::OP_ASS))
	{
		parameter->default_value = parse_assignment_expression();
		if (parameter->default_value == nullptr)
			return nullptr;
		// The course reference dumps the lone-literal default of an unnamed
		// `int` parameter with its token kind.
		const Name* name = declarator_name(parameter->declarator);
		if (name == nullptr && info.only_int && parameter->default_value->kind == ExprKind::Literal)
			static_cast<LiteralExpr*>(parameter->default_value)->token_form = true;
	}
	const Name* name = declarator_name(parameter->declarator);
	if (name != nullptr && !name->qualified() && name->last().kind == NameComponentKind::Identifier)
		scopes_.declare(scopes_.current(), name->last().identifier,
			NameBinding{ NameCategory::Value, parameter->pack, nullptr, true });
	return parameter;
}

// ------------------------------------------------------------ special members

bool Parser::special_member_ahead(DeclContext context, Name*& name, bool parenthesized)
{
	// A constructor, destructor or conversion function declared without a
	// type: `C(`, `~C(`, `operator T(` in a class; `X::X(`, `X::~X(`,
	// `X::operator T(` anywhere.
	name = nullptr;
	const auto follows_name = [&](uint32_t ahead) {
		return at(TokenKind::OP_LPAREN, ahead) || (parenthesized && at(TokenKind::OP_RPAREN, ahead));
	};
	if (context == DeclContext::Class)
	{
		const Identifier* class_name = class_names_.empty() ? nullptr : class_names_.back();
		if (at_identifier() && class_name != nullptr && tok().identifier == class_name && follows_name(1))
		{
			name = make_simple_name(class_name, location());
			next();
			return true;
		}
		if ((at(TokenKind::OP_COMPL) && at_identifier(1)) || at(TokenKind::KW_OPERATOR))
		{
			NameInfo info;
			name = parse_name(NameMode::Declarator, info);
			if (name == nullptr || !follows_name(0))
				return false;
			const NameComponentKind kind = name->last().kind;
			return kind == NameComponentKind::Destructor || kind == NameComponentKind::Conversion;
		}
	}
	if (!at_identifier() && !at(TokenKind::OP_COLON2))
		return false;
	NameInfo info;
	name = parse_name(NameMode::Declarator, info);
	if (name == nullptr || !name->qualified() || name->components.size < 2 || !follows_name(0))
		return false;
	const NameComponent& last = name->last();
	if (last.kind == NameComponentKind::Destructor || last.kind == NameComponentKind::Conversion)
		return true;
	const NameComponent& owner = name->components[name->components.size - 2];
	return last.kind == NameComponentKind::Identifier && !last.has_arguments &&
		owner.kind == NameComponentKind::Identifier && owner.identifier == last.identifier;
}

bool Parser::try_special_member(DeclContext context, Decl*& decl)
{
	decl = nullptr;
	Tentative tentative(*this);
	const uint32_t start = location();
	ListFrame<DeclSpecifier> specifiers(specs_);
	pending_attributes_ = leading_attributes_;
	for (;;)
	{
		if (!collect_attributes(pending_attributes_))
			return false;
		if (tok().kind != ParseTokenKind::Simple || !IsMemberFunctionSpecifier(tok().simple))
			break;
		DeclSpecifier specifier = DeclSpecifier();
		specifier.kind = DeclSpecKind::Keyword;
		specifier.keyword = tok().simple;
		specifier.location = location();
		next();
		if (specifier.keyword == TokenKind::KW_EXPLICIT && at(TokenKind::OP_LPAREN))
		{
			// C++20 explicit(constant-expression), which the course accepts.
			specifier.operand = parse_decltype_operand();
			if (specifier.operand == nullptr)
				return false;
		}
		specifiers.push(specifier);
	}
	Name* name = nullptr;
	Declarator* declarator = nullptr;
	if (at(TokenKind::OP_LPAREN))
	{
		Scope* member_of = nullptr;
		Attribute* attributes = pending_attributes_;
		pending_attributes_ = nullptr;
		declarator = parse_parenthesized_special_member(context, name, member_of);
		pending_attributes_ = attributes;
		if (declarator == nullptr)
			return false;
	}
	else if (!special_member_ahead(context, name))
	{
		return false;
	}
	decl = parse_special_member(context, specifiers.take(arena_), name, declarator, start, tentative);
	if (decl == nullptr)
		return tentative.committed();
	tentative.commit();
	return true;
}

Declarator* Parser::parse_parenthesized_special_member(DeclContext context, Name*& name, Scope*& member_of)
{
	NestingGuard nesting(*this);
	const uint32_t start = location();
	next();
	Declarator* nested = nullptr;
	if (at(TokenKind::OP_LPAREN))
		nested = parse_parenthesized_special_member(context, name, member_of);
	else if (special_member_ahead(context, name, true))
		nested = parse_special_member_declarator(name, member_of);
	if (nested == nullptr || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	Declarator* declarator = make<Declarator>();
	*declarator = Declarator();
	declarator->location = start;
	declarator->nested = nested;
	special_member_clause_ = true;
	const bool parsed = parse_declarator_suffixes(DeclaratorMode::Named, declarator, true, member_of);
	special_member_clause_ = false;
	return parsed ? declarator : nullptr;
}

Declarator* Parser::parse_special_member_declarator(Name* name, Scope*& member_of)
{
	pending_template_ = false;
	Declarator* declarator = make<Declarator>();
	*declarator = Declarator();
	declarator->name = name;
	declarator->location = name->location;
	member_of = nullptr;
	if (name->qualified())
	{
		// The class the member belongs to, for lookups in its body.
		Name prefix = *name;
		prefix.components.size -= 1;
		NameInfo info = NameInfo{ NameCategory::None, PrefixState::Unqualified, false, false, false, false, false,
			false, false, false, 0, nullptr, nullptr, nullptr };
		if (prefix.global)
		{
			info.prefix = PrefixState::Known;
			info.prefix_scope = scopes_.global();
		}
		for (const NameComponent& component : prefix.components)
			advance_prefix(component, info, true, true);
		if (info.prefix == PrefixState::Known)
			member_of = info.prefix_scope;
	}
	function_scope_ = nullptr;
	special_member_clause_ = true;
	const bool suffixes = parse_declarator_suffixes(DeclaratorMode::Named, declarator, true, member_of);
	special_member_clause_ = false;
	return suffixes ? declarator : nullptr;
}

Decl* Parser::parse_special_member(DeclContext context, Slice<DeclSpecifier> specifiers, Name* name,
	Declarator* declarator, uint32_t start, Tentative& tentative)
{
	SpecialMemberDecl* decl = make<SpecialMemberDecl>(DeclKind::SpecialMember, start);
	decl->definition = false;
	decl->specifiers = specifiers;
	decl->attributes = pending_attributes_;
	pending_attributes_ = nullptr;
	decl->name = name;
	decl->initializer = nullptr;
	decl->body = FunctionBody();
	if (declarator == nullptr)
	{
		Scope* member_of = nullptr;
		declarator = parse_special_member_declarator(name, member_of);
	}
	if (declarator == nullptr)
		return nullptr;
	if (!is_function_declarator(declarator))
	{
		fail();
		return nullptr;
	}
	decl->declarator = declarator;
	Scope* scope = function_scope_;
	if (at(TokenKind::OP_ASS))
	{
		decl->initializer = parse_initializer(false);
		if (decl->initializer == nullptr || !expect(TokenKind::OP_SEMICOLON))
			return nullptr;
		return decl;
	}
	if (accept(TokenKind::OP_SEMICOLON))
		return decl;
	if (!at_function_body_start(true))
	{
		fail();
		return nullptr;
	}
	decl->definition = true;
	// With a body ahead no other reading of the declaration remains, so the
	// checkpoint closes before the body: its scope stays the declarator.
	tentative.commit();
	if (context == DeclContext::Class)
	{
		if (!defer_function_body(decl->body, true, scope))
			return nullptr;
		return decl;
	}
	Scope* saved = scopes_.enter(scope);
	const bool parsed = parse_function_body(decl->body, true);
	scopes_.set_current(saved);
	return parsed ? decl : nullptr;
}

// --------------------------------------------------------- simple declarations

Decl* Parser::parse_simple_or_function(DeclContext context)
{
	const uint32_t start = location();
	const bool is_template = pending_template_;
	SpecifierInfo info;
	ListFrame<DeclSpecifier> list(specs_);
	pending_attributes_ = leading_attributes_;
	leading_type_attributes(leading_attributes_, list);
	if (!parse_decl_specifier_seq(list, info))
		return nullptr;
	if (list.size() == 0)
	{
		fail();
		return nullptr;
	}
	Slice<DeclSpecifier> specifiers = list.take(arena_);
	pending_template_ = is_template && !info.defines_type;
	Decl* decl = finish_simple_declaration(context, specifiers, info, start);
	pending_template_ = false;
	return decl;
}

void Parser::declare_declarator(const Declarator* declarator, const SpecifierInfo& info)
{
	const Name* name = declarator_name(declarator);
	if (name == nullptr || name->qualified() || name->last().kind != NameComponentKind::Identifier ||
		name->last().has_arguments)
		return;
	NameBinding binding{ NameCategory::Value, false, nullptr };
	if (info.has_typedef)
	{
		binding.category = NameCategory::Type;
		if (declarator->pointers.empty() && declarator->suffixes.empty() && declarator->nested == nullptr)
		{
			binding.scope = info.type_scope;
			note_type_name(info, binding, false);
			if (info.type_scope != nullptr && info.type_scope->parameter_bases)
				binding.alternatives = typedef_record(
					info.type_name, info.type_name_info.scope, info.type_name_info.category, info.type_mentions);
		}
	}
	else if (pending_template_)
	{
		// A function template or a variable template: its template-id is a
		// value.
		binding.category = NameCategory::FunctionTemplate;
	}
	scopes_.declare(declaration_scope(), name->last().identifier, binding);
}

Decl* Parser::finish_simple_declaration(
	DeclContext context, Slice<DeclSpecifier> specifiers, const SpecifierInfo& info, uint32_t start)
{
	if (accept(TokenKind::OP_SEMICOLON))
	{
		// A lone class or enumeration specifier declares just the type (the
		// course dump keeps a member enumeration's simple-declaration).
		const bool member_enum = context == DeclContext::Class && specifiers[0].kind == DeclSpecKind::Enum;
		// An elaborated-type-specifier with ABI tags (Attribute::on_class)
		// stays a simple-declaration, which keeps its attributes.
		bool class_tags = false;
		for (const Attribute* attribute = pending_attributes_; attribute != nullptr && !class_tags;
			attribute = attribute->next)
			class_tags = attribute->on_class;
		if (specifiers.size == 1 && specifiers[0].kind != DeclSpecKind::Keyword &&
			specifiers[0].kind != DeclSpecKind::Name && specifiers[0].kind != DeclSpecKind::Decltype && !member_enum &&
			!class_tags)
		{
			if (specifiers[0].kind == DeclSpecKind::Enum)
			{
				EnumDecl* decl = make<EnumDecl>(DeclKind::Enum, start);
				decl->enum_specifier = specifiers[0].enum_specifier;
				return decl;
			}
			ClassDecl* decl = make<ClassDecl>(DeclKind::Class, start);
			decl->specifier = specifiers[0];
			return decl;
		}
		SimpleDecl* decl = make<SimpleDecl>(DeclKind::Simple, start);
		decl->specifiers = specifiers;
		decl->declarators = Slice<InitDeclarator>{ nullptr, 0 };
		decl->attributes = pending_attributes_;
		pending_attributes_ = nullptr;
		return decl;
	}
	if (context == DeclContext::Class && at(TokenKind::OP_COLON))
		return parse_bit_field_rest(specifiers, nullptr, start);

	if (context != DeclContext::Class && at_structured_binding())
		return parse_structured_binding(specifiers, info, start);
	function_scope_ = nullptr;
	Attribute* attributes = pending_attributes_;
	pending_attributes_ = nullptr;
	uint32_t first_token = static_cast<uint32_t>(tokens_.unit_position());
	const bool saved_friend = friend_declarator_;
	friend_declarator_ = info.has_friend;
	Declarator* declarator = parse_declarator(DeclaratorMode::Named);
	friend_declarator_ = saved_friend;
	if (declarator == nullptr)
		return nullptr;
	if (is_function_declarator(declarator) && at_function_body_start(false))
	{
		FunctionDecl* function = make<FunctionDecl>(DeclKind::Function, start);
		function->specifiers = specifiers;
		function->attributes = attributes;
		function->declarator = declarator;
		function->body = FunctionBody();
		Scope* scope = function_scope_;
		declare_declarator(declarator, info);
		pending_template_ = false;
		if (context == DeclContext::Class)
		{
			if (!defer_function_body(function->body, false, scope))
				return nullptr;
			return function;
		}
		Scope* saved = scopes_.enter(scope);
		const bool parsed = parse_function_body(function->body, false);
		scopes_.set_current(saved);
		return parsed ? function : nullptr;
	}
	if (context == DeclContext::Class && at(TokenKind::OP_COLON))
	{
		pending_attributes_ = attributes;
		return parse_bit_field_rest(specifiers, declarator, start);
	}

	SimpleDecl* decl = make<SimpleDecl>(DeclKind::Simple, start);
	decl->specifiers = specifiers;
	decl->attributes = attributes;
	ListFrame<InitDeclarator> declarators(init_declarators_);
	for (;;)
	{
		Scope* const member_of = declarator_member_of_;
		declare_declarator(declarator, info);
		InitDeclarator init = InitDeclarator();
		init.declarator = declarator;
		init.first_token = first_token;
		if (at(TokenKind::OP_ASS) || at(TokenKind::OP_LBRACE) || at(TokenKind::OP_LPAREN))
		{
			const bool saved_template = pending_template_;
			pending_template_ = false;
			// A static data member's initializer is in the scope of its
			// class (9.4.2p2), a namespace member's in its namespace's
			// (3.4.3p3).
			Scope* outer = member_of != nullptr ? scopes_.enter(member_lookup_scope(member_of)) : nullptr;
			init.initializer = parse_initializer(true);
			if (outer != nullptr)
				scopes_.set_current(outer);
			pending_template_ = saved_template;
			if (init.initializer == nullptr)
				return nullptr;
		}
		init.end_token = static_cast<uint32_t>(tokens_.unit_position());
		declarators.push(init);
		if (!accept(TokenKind::OP_COMMA))
			break;
		first_token = static_cast<uint32_t>(tokens_.unit_position());
		friend_declarator_ = info.has_friend;
		declarator = parse_declarator(DeclaratorMode::Named);
		friend_declarator_ = saved_friend;
		if (declarator == nullptr)
			return nullptr;
	}
	if (!expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	decl->declarators = declarators.take(arena_);
	return decl;
}

Decl* Parser::parse_bit_field_rest(Slice<DeclSpecifier> specifiers, Declarator* first, uint32_t start)
{
	BitFieldDecl* decl = make<BitFieldDecl>(DeclKind::BitField, start);
	decl->specifiers = specifiers;
	decl->attributes = pending_attributes_;
	pending_attributes_ = nullptr;
	ListFrame<BitFieldDeclarator> members(bit_fields_);
	Declarator* declarator = first;
	for (;;)
	{
		BitFieldDeclarator member = BitFieldDeclarator();
		member.declarator = declarator;
		if (declarator != nullptr)
		{
			const Name* name = declarator_name(declarator);
			if (name != nullptr && !name->qualified() && name->last().kind == NameComponentKind::Identifier)
				scopes_.declare(
					scopes_.current(), name->last().identifier, NameBinding{ NameCategory::Value, false, nullptr });
		}
		if (!expect(TokenKind::OP_COLON))
			return nullptr;
		member.width = parse_assignment_expression();
		if (member.width == nullptr)
			return nullptr;
		members.push(member);
		if (!accept(TokenKind::OP_COMMA))
			break;
		declarator = nullptr;
		if (!at(TokenKind::OP_COLON))
		{
			declarator = parse_declarator(DeclaratorMode::Named);
			if (declarator == nullptr)
				return nullptr;
		}
	}
	if (!expect(TokenKind::OP_SEMICOLON))
		return nullptr;
	decl->declarators = members.take(arena_);
	return decl;
}

Initializer* Parser::parse_initializer(bool allow_parenthesized)
{
	Initializer* initializer = make<Initializer>();
	*initializer = Initializer();
	initializer->location = location();
	const bool saved_angle = angle_;
	angle_ = false;
	Initializer* result = initializer;
	if (accept(TokenKind::OP_ASS))
	{
		if ((at(TokenKind::KW_DEFAULT) || at(TokenKind::KW_DELETE)) &&
			(at(TokenKind::OP_SEMICOLON, 1) || at(TokenKind::OP_COMMA, 1)))
		{
			initializer->kind = at(TokenKind::KW_DEFAULT) ? InitializerKind::Default : InitializerKind::Delete;
			next();
		}
		else
		{
			initializer->kind = InitializerKind::Assign;
			initializer->expression = parse_initializer_clause();
			if (initializer->expression == nullptr)
				result = nullptr;
		}
	}
	else if (at(TokenKind::OP_LBRACE))
	{
		initializer->kind = InitializerKind::Braced;
		initializer->expression = parse_braced_init_list();
		if (initializer->expression == nullptr)
			result = nullptr;
	}
	else if (allow_parenthesized && accept(TokenKind::OP_LPAREN))
	{
		initializer->kind = InitializerKind::Parenthesized;
		ListFrame<Expr*> arguments(exprs_);
		if (parse_expression_list(arguments, TokenKind::OP_RPAREN))
			initializer->arguments = arguments.take(arena_);
		else
			result = nullptr;
	}
	else
	{
		fail();
		result = nullptr;
	}
	angle_ = saved_angle;
	return result;
}

} // namespace syntax
