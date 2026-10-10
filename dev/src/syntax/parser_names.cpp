#include "syntax/parser.h"

#include "lex/identifier_table.h"

#include <algorithm>
#include <cctype>

namespace syntax {

namespace {

// The operators an operator-function-id may name (13.5p1), besides new,
// delete, () and [] which take two tokens.
bool IsOverloadableOperator(TokenKind kind)
{
	switch (kind)
	{
	case TokenKind::OP_PLUS:
	case TokenKind::OP_MINUS:
	case TokenKind::OP_STAR:
	case TokenKind::OP_DIV:
	case TokenKind::OP_MOD:
	case TokenKind::OP_XOR:
	case TokenKind::OP_AMP:
	case TokenKind::OP_BOR:
	case TokenKind::OP_COMPL:
	case TokenKind::OP_LNOT:
	case TokenKind::OP_ASS:
	case TokenKind::OP_LT:
	case TokenKind::OP_GT:
	case TokenKind::OP_PLUSASS:
	case TokenKind::OP_MINUSASS:
	case TokenKind::OP_STARASS:
	case TokenKind::OP_DIVASS:
	case TokenKind::OP_MODASS:
	case TokenKind::OP_XORASS:
	case TokenKind::OP_BANDASS:
	case TokenKind::OP_BORASS:
	case TokenKind::OP_LSHIFT:
	case TokenKind::OP_RSHIFTASS:
	case TokenKind::OP_LSHIFTASS:
	case TokenKind::OP_EQ:
	case TokenKind::OP_NE:
	case TokenKind::OP_LE:
	case TokenKind::OP_GE:
	case TokenKind::OP_LAND:
	case TokenKind::OP_LOR:
	case TokenKind::OP_INC:
	case TokenKind::OP_DEC:
	case TokenKind::OP_COMMA:
	case TokenKind::OP_ARROWSTAR:
	case TokenKind::OP_ARROW:
		return true;
	default:
		return false;
	}
}

bool IsTemplateCategory(NameCategory category)
{
	return category == NameCategory::TypeTemplate || category == NameCategory::FunctionTemplate;
}

} // namespace

bool Parser::names_type_hint(const Identifier* identifier) const
{
	// parsing.md's lexical fallback for names no declaration classifies: an
	// identifier containing C, Y or E can stand for a type, and T marks a
	// potential template or type name.
	for (uint32_t i = 0; i < identifier->length; ++i)
	{
		const char c = identifier->spelling[i];
		if (c == 'C' || c == 'Y' || c == 'E' || c == 'T')
			return true;
	}
	return false;
}

bool ParserState::current_instantiation(const Scope* templ) const
{
	// The class template whose definition encloses the name, or whose
	// member an out-of-class definition defines (14.6.2.1p1, 3.4.1p8).
	for (const Scope* scope = scopes_.current(); scope != nullptr; scope = scope->parent)
	{
		if (scope == templ)
			return true;
		for (const Scope* cls = scope->member_of; cls != nullptr && cls->kind == ScopeKind::Class;
			cls = cls->member_of != nullptr ? cls->member_of : cls->parent)
		{
			if (cls == templ)
				return true;
		}
	}
	return false;
}

void Parser::note_name_conflict(uint32_t conflicts, uint32_t component)
{
	if (conflicts_.uses != conflicts && conflicts_.name_component < 0)
	{
		conflicts_.name_component = static_cast<int32_t>(component);
		conflicts_.name_category = conflicts_.last;
		conflicts_.name_alternatives = conflicts_.last_alternatives;
	}
}

NameBinding Parser::resolve_component(const Identifier* identifier, const NameInfo& info, bool prefix_only)
{
	switch (info.prefix)
	{
	case PrefixState::Unqualified:
		return scopes_.lookup(identifier, prefix_only);
	case PrefixState::Known:
	{
		NameBinding binding = scopes_.lookup_in(info.prefix_scope, identifier, prefix_only, info.prefix_specialization);
		// A specialization's member its template's scope lacks may be its
		// argument's, through a base naming a template parameter.
		if (binding.category == NameCategory::None && info.prefix_scope->parameter_bases && info.prefix_specialization)
			binding = parameter_base_lookup(info.prefix_scope,
				Slice<TemplateArgument>{
					const_cast<TemplateArgument*>(info.prefix_arguments), info.prefix_argument_count },
				identifier, prefix_only, 0);
		// Through a template-id prefix that depends on template parameters
		// (not the current instantiation), a member the specializations
		// declare with several categories names neither a type nor a
		// template without `typename` or `template` (14.6p2, 14.2p4): a
		// value (either keyword decides the name elsewhere).
		if (info.prefix_dependent && binding.category != NameCategory::None && scopes_.has_conflicts() &&
			scopes_.member_categories_differ(info.prefix_scope, identifier) &&
			!current_instantiation(info.prefix_scope))
			return NameBinding{ NameCategory::Value, false, nullptr };
		if (binding.conflicted || (info.prefix_approximate && !prefix_only))
			return guessed_component(binding);
		return binding;
	}
	case PrefixState::Dependent:
		// Outside every template definition a member of a type whose class
		// is unknown: a value as a dependent name would be, or a type.
		if (info.prefix_uncertain && !prefix_only)
			return guessed_component(NameBinding{ NameCategory::None, false, nullptr });
		break;
	case PrefixState::Unknown:
		break;
	}
	return NameBinding{ NameCategory::None, false, nullptr };
}

NameBinding Parser::guessed_component(NameBinding binding)
{
	// A member whose category the template's specializations disagree on,
	// or one the parser can only guess (in a class that is a guess,
	// NameBinding::approximate, or one unknown): the found one (none, which
	// the context then decides, when the class is unknown or does not
	// declare it), or in another reading of a statement the category that
	// reading takes where the name has it, a type or a value (see
	// parse_declaration_or_expression_statement).
	if (!binding.conflicted)
	{
		if (!alternatives_active_)
			return binding;
		if (binding.category == NameCategory::None)
			binding.alternatives = CategoryBit(NameCategory::Type) | CategoryBit(NameCategory::Value);
		else if (binding.category == NameCategory::Value || binding.category == NameCategory::Type)
			binding.alternatives = CategoryBit(
				binding.category == NameCategory::Value ? NameCategory::Type : NameCategory::Value);
		else
			return binding;
		binding.conflicted = true;
	}
	++conflicts_.uses;
	const uint8_t all = static_cast<uint8_t>(binding.alternatives | CategoryBit(binding.category));
	conflicts_.seen = static_cast<uint8_t>(conflicts_.seen | binding.alternatives);
	if (conflicts_.take != NameCategory::None && (binding.alternatives & CategoryBit(conflicts_.take)) != 0)
		binding = NameBinding{ conflicts_.take, false, nullptr };
	conflicts_.last = binding.category;
	conflicts_.last_alternatives = static_cast<uint8_t>(all & ~CategoryBit(binding.category));
	return binding;
}

NameBinding ParserState::parameter_base_lookup(const Scope* templ, const Slice<TemplateArgument>& arguments,
	const Identifier* identifier, bool prefix_only, uint32_t depth)
{
	// 14.7.1, 10.2: the specialization's bases are the substituted ones, so
	// a base naming a template parameter is the class its argument names
	// (an argument that is a lone class name; a pack's expansion is every
	// argument from its position on). Searched after the template's own
	// members and other bases, as the semantic lookup's order allows when
	// the name is found once.
	const std::vector<uint32_t>* bases = nullptr;
	for (const auto& entry : parameter_bases_)
	{
		if (entry.first == templ)
			bases = &entry.second;
	}
	if (bases == nullptr || depth > 8)
		return NameBinding();
	for (const uint32_t base : *bases)
	{
		const uint32_t index = base & ~kPackBase;
		const uint32_t end = (base & kPackBase) != 0 ? arguments.size : index + 1;
		for (uint32_t i = index; i < end && i < arguments.size; ++i)
		{
			// A lone class name (its arguments name no template parameter:
			// the specialization's do not).
			const TemplateArgument& argument = arguments[i];
			Scope* const named = argument_class(argument);
			if (named == nullptr)
				continue;
			const Name& name = *argument.type->specifiers[0].name;
			const NameComponent& last = name.last();
			// A typedef of such a specialization keeps its arguments.
			const uint8_t record = !last.has_arguments && named->parameter_bases ? specialization_record(name) : 0;
			const bool specialization = last.has_arguments || record != 0;
			NameBinding binding = scopes_.lookup_in(named, identifier, prefix_only, specialization);
			if (binding.category == NameCategory::None && specialization && named->parameter_bases)
				binding = parameter_base_lookup(named,
					last.has_arguments ? last.arguments : specialization_records_[record - 1u], identifier, prefix_only,
					depth + 1);
			if (binding.category != NameCategory::None)
				return binding;
		}
	}
	return NameBinding();
}

Scope* ParserState::argument_class(const TemplateArgument& argument)
{
	// The class a template argument that is a lone class name (a type-id
	// of one name specifier, without a declarator or pack expansion) names,
	// its name looked up again by its components (the argument keeps no
	// scope); null for another argument or an unknown class.
	if (argument.type == nullptr || argument.pack || argument.type->declarator != nullptr ||
		argument.type->specifiers.size != 1 || argument.type->specifiers[0].kind != DeclSpecKind::Name)
		return nullptr;
	const Name& name = *argument.type->specifiers[0].name;
	Scope* scope = name.global ? scopes_.global() : nullptr;
	for (uint32_t i = 0; i < name.components.size; ++i)
	{
		const NameComponent& component = name.components[i];
		if (component.kind != NameComponentKind::Identifier)
			return nullptr;
		const bool prefix = i + 1 < name.components.size;
		const NameBinding binding = scope == nullptr ? scopes_.lookup(component.identifier, prefix)
													 : scopes_.lookup_in(scope, component.identifier, prefix, false);
		if (binding.scope == nullptr || scope_pending(binding.scope))
			return nullptr;
		scope = binding.scope;
	}
	return scope != nullptr && scope->kind == ScopeKind::Class ? scope : nullptr;
}

uint8_t ParserState::typedef_record(const Name* name, Scope* scope, NameCategory category, bool mentions)
{
	if (name == nullptr || scope == nullptr || !scope->parameter_bases ||
		(name->last().has_arguments && (category != NameCategory::TypeTemplate || mentions)))
		return 0;
	return specialization_record(*name);
}

uint8_t ParserState::specialization_record(const Name& name)
{
	const NameComponent& last = name.last();
	if (last.has_arguments)
	{
		if (specialization_records_.size() >= 0xFF)
			return 0;
		specialization_records_.push_back(last.arguments);
		return static_cast<uint8_t>(specialization_records_.size());
	}
	if (name.qualified() || last.kind != NameComponentKind::Identifier)
		return 0;
	return scopes_.lookup(last.identifier, false).specialization_record();
}

void ParserState::record_parameter_base(Scope* class_scope, const Identifier* name, bool pack)
{
	if (template_head_ == nullptr)
		return;
	const Slice<TemplateParameter*>& parameters = template_head_->parameters;
	for (uint32_t i = 0; i < parameters.size; ++i)
	{
		const TemplateParameter* parameter = parameters[i];
		if (parameter->name != name || parameter->kind != TemplateParameterKind::Type)
			continue;
		if (parameter->pack != pack)
			return;
		const uint32_t base = pack ? (i | kPackBase) : i;
		std::vector<uint32_t>* bases = nullptr;
		for (auto& entry : parameter_bases_)
		{
			if (entry.first == class_scope)
				bases = &entry.second;
		}
		if (bases == nullptr)
		{
			parameter_bases_.push_back(std::make_pair(class_scope, std::vector<uint32_t>()));
			bases = &parameter_bases_.back().second;
		}
		if (std::find(bases->begin(), bases->end(), base) == bases->end())
			bases->push_back(base);
		class_scope->parameter_bases = true;
		return;
	}
}

void Parser::advance_prefix(const NameComponent& component, NameInfo& info, bool dependent_arguments, bool declared)
{
	// A component followed by `::` becomes the scope of the next one: a
	// known class, enumeration or namespace; dependent when it is a type
	// without a known scope (a template parameter); otherwise unknown.
	if (component.kind != NameComponentKind::Identifier)
	{
		info.prefix_uncertain = info.prefix == PrefixState::Dependent ? info.prefix_uncertain
																	  : !in_template_definition();
		info.prefix = PrefixState::Dependent;
		info.prefix_scope = nullptr;
		info.prefix_specialization = false;
		info.prefix_approximate = false;
		return;
	}
	const NameBinding binding = resolve_component(component.identifier, info, true);
	if (binding.pack)
		++pack_mentions_;
	if (binding.parameter)
		++template_parameter_mentions_;
	// Whether the component's type is dependent or its class a guess (see
	// prefix_guesses): only through a template-id or a guess.
	uint8_t guesses = 0;
	if (binding.scope != nullptr &&
		(binding.flags != 0 ||
			(info.prefix == PrefixState::Known &&
				(info.prefix_dependent || info.prefix_approximate || info.prefix_specialization))))
		guesses = prefix_guesses(component, info, binding, declared);
	const bool dependent = (guesses & NameBinding::kDependent) != 0;
	const bool was_dependent = info.prefix == PrefixState::Dependent;
	info.prefix_specialization = false;
	info.prefix_dependent = false;
	info.prefix_approximate = false;
	if (binding.scope != nullptr && !scope_pending(binding.scope) && !dependent)
	{
		info.prefix = PrefixState::Known;
		info.prefix_scope = binding.scope;
		// A typedef of a specialization is one too (its template's
		// arguments kept when bases name its parameters).
		const uint8_t record = component.has_arguments ? 0 : binding.specialization_record();
		info.prefix_specialization = component.has_arguments
			? !dependent_arguments && binding.category == NameCategory::TypeTemplate
			: record != 0;
		info.prefix_dependent = component.has_arguments && dependent_arguments &&
			binding.category == NameCategory::TypeTemplate;
		info.prefix_approximate = (guesses & NameBinding::kApproximate) != 0;
		if (info.prefix_specialization && binding.scope->parameter_bases)
		{
			const Slice<TemplateArgument> arguments = component.has_arguments ? component.arguments
																			  : specialization_records_[record - 1u];
			info.prefix_arguments = arguments.data;
			info.prefix_argument_count = arguments.size;
		}
	}
	else if (binding.category == NameCategory::Type || binding.category == NameCategory::TypeTemplate ||
		was_dependent || dependent)
	{
		if (!was_dependent)
			info.prefix_uncertain = !in_template_definition();
		info.prefix = PrefixState::Dependent;
		info.prefix_scope = nullptr;
	}
	else
	{
		info.prefix = PrefixState::Unknown;
		info.prefix_scope = nullptr;
	}
}

uint8_t Parser::prefix_guesses(
	const NameComponent& component, const NameInfo& info, const NameBinding& binding, bool declared)
{
	// A member of a template-id depending on template parameters (not the
	// current instantiation), or a typedef-name of a dependent type its
	// template finds: a dependent type, whose members are dependent names
	// (14.6.2.1p7) though its primary's class is known (kDependent). A
	// member's class is a guess where a specialization declares the member
	// too, or through a guess (kApproximate).
	const bool known = info.prefix == PrefixState::Known;
	bool dependent = false;
	if (!declared && known && info.prefix_dependent)
		dependent = !current_instantiation(info.prefix_scope) || binding.dependent();
	else if (!declared && binding.dependent())
		dependent = info.prefix == PrefixState::Unqualified || (known && current_instantiation(info.prefix_scope));
	if (dependent)
		return NameBinding::kDependent;
	const bool approximate = binding.approximate() ||
		(known &&
			(info.prefix_approximate ||
				(info.prefix_specialization && scopes_.has_category_conflicts() &&
					scopes_.member_specialized(info.prefix_scope, component.identifier))));
	return approximate ? NameBinding::kApproximate : 0;
}

Parser::TemplateIdChoice Parser::template_id_follows(
	const Identifier* identifier, const NameInfo& info, NameMode mode, bool template_keyword)
{
	if (template_keyword)
		return TemplateIdChoice::Yes;
	// After `.` or `->` the name is looked up in the object's class
	// (3.4.5p1), which the parser does not know: a member name followed by
	// a template argument list and a call is taken as a member template --
	// when some declaration made the name a template at all. A name no
	// declaration so far made one (`x.imm < INT32_MIN || x.imm > (max)`)
	// compares, whatever follows its `>`.
	if (mode == NameMode::Member && info.prefix == PrefixState::Unqualified)
		return scopes_.ever_template(identifier) ? TemplateIdChoice::MaybeCall : TemplateIdChoice::No;
	const NameBinding binding = resolve_component(identifier, info, false);
	if (IsTemplateCategory(binding.category))
		return TemplateIdChoice::Yes;
	if (binding.category != NameCategory::None)
		return TemplateIdChoice::No;
	// A dependent qualified name is not a template without `template`
	// (14.2p4); an undeclared one is tried as a template-id.
	if (info.prefix == PrefixState::Dependent)
		return TemplateIdChoice::No;
	return TemplateIdChoice::Maybe;
}

Name* Parser::parse_name(NameMode mode, NameInfo& info)
{
	// The name's component whose category specializations disagree on
	// (nested names keep their own; a unit without such names has none).
	if (!scopes_.has_conflicts() && !alternatives_active_)
		return parse_name_components(mode, info);
	const int32_t outer_component = conflicts_.name_component;
	const NameCategory outer_category = conflicts_.name_category;
	const uint8_t outer_alternatives = conflicts_.name_alternatives;
	conflicts_.name_component = -1;
	Name* name = parse_name_components(mode, info);
	if (name != nullptr && conflicts_.name_component >= 0 && conflicts_.statement.name == nullptr)
		conflicts_.statement = StatementConflict{ name, static_cast<uint32_t>(conflicts_.name_component),
			conflicts_.name_category, conflicts_.name_alternatives };
	conflicts_.name_component = outer_component;
	conflicts_.name_category = outer_category;
	conflicts_.name_alternatives = outer_alternatives;
	return name;
}

Name* Parser::parse_name_components(NameMode mode, NameInfo& info)
{
	info = NameInfo{ NameCategory::None, PrefixState::Unqualified, false, false, false, false, false, false, false, 0,
		nullptr, nullptr, nullptr };
	Name* name = make<Name>();
	name->global = false;
	name->typename_keyword = false;
	name->location = location();
	if (at(TokenKind::OP_COLON2))
	{
		next();
		name->global = true;
		info.prefix = PrefixState::Known;
		info.prefix_scope = scopes_.global();
	}
	ListFrame<NameComponent> components(components_);
	bool more = true;
	while (more)
	{
		if (!parse_name_component(mode, components, info, more))
			return nullptr;
	}
	name->components = components.take(arena_);
	return name;
}

bool Parser::parse_name_component(NameMode mode, ListFrame<NameComponent>& components, NameInfo& info, bool& more)
{
	more = false;
	NameComponent component = NameComponent();
	component.location = location();
	if (at(TokenKind::KW_TEMPLATE) && (info.prefix != PrefixState::Unqualified || mode == NameMode::Member))
	{
		component.template_keyword = true;
		next();
	}
	// GNU `__typeof__ (` and `__underlying_type (` begin a name as decltype
	// does (a declaration statement, sizeof's type-id, a functional cast).
	const bool gnu_decltype = components.size() == 0 && info.prefix == PrefixState::Unqualified &&
		(mode == NameMode::Expression || mode == NameMode::Type) && at(TokenKind::OP_LPAREN, 1) && at_gnu_decltype();
	if (at_identifier() && !gnu_decltype)
	{
		component.kind = NameComponentKind::Identifier;
		component.identifier = tok().identifier;
		next();
		const uint32_t mentions = template_parameter_mentions_;
		const uint32_t conflicts = conflicts_.uses;
		if (at(TokenKind::OP_LT))
		{
			const TemplateIdChoice choice = template_id_follows(
				component.identifier, info, mode, component.template_keyword);
			if (choice == TemplateIdChoice::Yes)
			{
				if (!parse_template_arguments(component.arguments))
					return false;
				component.has_arguments = true;
			}
			else if (choice == TemplateIdChoice::Maybe || choice == TemplateIdChoice::MaybeCall)
			{
				// Tried once per position (see try_outermost_template_arguments).
				const bool call_follows = choice == TemplateIdChoice::MaybeCall;
				const TemplateListKey key = template_list_key();
				if (!abandoning_lists_ && !template_list_failed_before(key) && closing_angle_ahead())
				{
					if (open_lists_.size() == open_lists_base_)
					{
						component.has_arguments = try_outermost_template_arguments(
							component.arguments, call_follows, key);
					}
					else
					{
						open_lists_.push_back(key);
						Tentative tentative(*this);
						if (!parse_template_arguments(component.arguments) || abandoning_lists_ ||
							(call_follows &&
								((!at(TokenKind::OP_LPAREN) && !at(TokenKind::OP_COLON2)) ||
									!member_template_arguments_plausible(component.arguments))))
						{
							abandon_template_lists();
							return false;
						}
						tentative.commit();
						open_lists_.pop_back();
						component.has_arguments = true;
					}
				}
			}
		}
		// `X::` continues the name unless `*` follows (a pointer to member).
		const bool continues = at(TokenKind::OP_COLON2) &&
			(at_identifier(1) || at(TokenKind::KW_TEMPLATE, 1) || at(TokenKind::OP_COMPL, 1) ||
				at(TokenKind::KW_OPERATOR, 1));
		if (continues)
		{
			advance_prefix(component, info, template_parameter_mentions_ != mentions,
				mode == NameMode::Declarator || naming_declared_);
			next();
			note_name_conflict(conflicts, components.size());
			components.push(component);
			more = true;
			return true;
		}
		const NameBinding binding = resolve_component(component.identifier, info, false);
		info.category = binding.category;
		info.scope = binding.scope;
		info.template_member = info.prefix == PrefixState::Known &&
			(info.prefix_specialization || info.prefix_dependent);
		info.approximate = binding.approximate();
		info.dependent = binding.dependent() && info.prefix == PrefixState::Unqualified;
		if (binding.pack && mode != NameMode::Member)
			++pack_mentions_;
		if (binding.parameter && mode != NameMode::Member)
			++template_parameter_mentions_;
		note_name_conflict(conflicts, components.size());
		components.push(component);
		return true;
	}
	if (at(TokenKind::OP_COMPL) && mode != NameMode::Type)
	{
		next();
		component.kind = NameComponentKind::Destructor;
		if (at(TokenKind::KW_DECLTYPE))
		{
			next();
			component.destructor_decltype = true;
			component.decltype_operand = parse_decltype_operand();
			if (component.decltype_operand == nullptr)
				return false;
		}
		else
		{
			if (!at_identifier())
				return fail();
			component.identifier = tok().identifier;
			next();
			// A destructor name is only ever called (5.2.4p1, 12.4p12):
			// `<` after it begins the type-name's template argument list
			// (`p->names.~vector<std::string>()`), whose template 3.4.5p3
			// finds in the object's class when the context has none.
			if (at(TokenKind::OP_LT))
			{
				if (!parse_template_arguments(component.arguments))
					return false;
				component.has_arguments = true;
			}
		}
		components.push(component);
		return true;
	}
	if (at(TokenKind::KW_OPERATOR) && mode != NameMode::Type)
	{
		// A qualified conversion-function-id's type is looked up in the
		// class first (3.4.3.1p1, 3.4.3p3: `X::operator T()`).
		Scope* member_of = info.prefix == PrefixState::Known && info.prefix_scope->kind == ScopeKind::Class
			? info.prefix_scope
			: nullptr;
		if (!parse_operator_name(component, member_of))
			return false;
		if (at(TokenKind::OP_LT) && component.kind != NameComponentKind::Conversion)
		{
			Tentative tentative(*this);
			if (parse_template_arguments(component.arguments))
			{
				tentative.commit();
				component.has_arguments = true;
			}
			else if (abandoning_lists_)
				return false;
		}
		info.category = NameCategory::Value;
		components.push(component);
		return true;
	}
	if (gnu_decltype ||
		(at(TokenKind::KW_DECLTYPE) && components.size() == 0 && info.prefix == PrefixState::Unqualified))
	{
		component.kind = NameComponentKind::Decltype;
		if (gnu_decltype)
		{
			component.decltype_operand = parse_gnu_decltype_operand();
		}
		else
		{
			next();
			component.decltype_operand = parse_decltype_operand();
		}
		if (component.decltype_operand == nullptr)
			return false;
		components.push(component);
		if (at(TokenKind::OP_COLON2) &&
			(at_identifier(1) || at(TokenKind::KW_TEMPLATE, 1) || at(TokenKind::OP_COMPL, 1)))
		{
			next();
			info.prefix = PrefixState::Dependent;
			more = true;
			return true;
		}
		// A lone decltype-specifier: callers take it as a type.
		info.category = NameCategory::Type;
		return true;
	}
	return fail();
}

bool Parser::parse_operator_name(NameComponent& component, Scope* member_of)
{
	if (!expect(TokenKind::KW_OPERATOR))
		return false;
	component.kind = NameComponentKind::Operator;
	const ParseToken token = tok();
	if (token.is(TokenKind::KW_NEW) || token.is(TokenKind::KW_DELETE))
	{
		component.op = token.simple;
		next();
		if (at(TokenKind::OP_LSQUARE) && at(TokenKind::OP_RSQUARE, 1))
		{
			component.array_form = true;
			next();
			next();
		}
		return true;
	}
	if ((token.is(TokenKind::OP_LPAREN) && at(TokenKind::OP_RPAREN, 1)) ||
		(token.is(TokenKind::OP_LSQUARE) && at(TokenKind::OP_RSQUARE, 1)))
	{
		component.op = token.simple;
		next();
		next();
		return true;
	}
	if (at_shift_right() && (tok(1).flags & kSecondShiftHalf) != 0)
	{
		component.op = TokenKind::OP_RSHIFT;
		next();
		next();
		return true;
	}
	// A ud-suffix split from `""` (see lex/post_tokenizer.cpp) is an
	// identifier even when it spells a keyword: `operator""if` is
	// `operator user-defined-string-literal` (CWG 1473, C++14's
	// [over.literal]p1; <complex>'s `if` literals).
	const bool keyword_suffix = tok(1).kind == ParseTokenKind::Simple && tok(1).location == token.location + 2 &&
		tok(1).spelling.length != 0 &&
		(std::isalpha(static_cast<unsigned char>(tok(1).spelling.text[0])) || tok(1).spelling.text[0] == '_');
	if (token.is_literal() && (token.flags & kStringLiteral) != 0 && token.spelling.length == 2 &&
		(at_identifier(1) || keyword_suffix))
	{
		// operator "" identifier (13.5.8).
		component.kind = NameComponentKind::LiteralOperator;
		next();
		component.identifier = tok().is_identifier() ? tok().identifier
													 : identifiers_.intern(tok().spelling.text, tok().spelling.length);
		next();
		return true;
	}
	if (token.kind == ParseTokenKind::Simple && IsOverloadableOperator(token.simple))
	{
		component.op = token.simple;
		component.alternative = IsAlternativeSpelling(token.spelling.text, token.spelling.length);
		next();
		return true;
	}
	component.kind = NameComponentKind::Conversion;
	Scope* outer = member_of != nullptr ? scopes_.enter(member_lookup_scope(member_of)) : nullptr;
	component.conversion = parse_type_id(DeclaratorMode::Conversion);
	if (outer != nullptr)
		scopes_.set_current(outer);
	return component.conversion != nullptr;
}

bool Parser::parse_template_arguments(Slice<TemplateArgument>& arguments)
{
	NestingGuard nesting(*this);
	if (!expect(TokenKind::OP_LT))
		return false;
	const bool saved_angle = angle_;
	angle_ = true;
	ListFrame<TemplateArgument> list(template_args_);
	bool parsed = true;
	if (!at_close_angle())
	{
		do
		{
			if (!parse_template_argument(list))
			{
				parsed = false;
				break;
			}
		} while (accept(TokenKind::OP_COMMA));
	}
	angle_ = saved_angle;
	if (!parsed || !at_close_angle())
		return fail();
	next();
	arguments = list.take(arena_);
	return true;
}

bool Parser::template_list_failed_before(const TemplateListKey& key) const
{
	return std::binary_search(failed_template_lists_.begin(), failed_template_lists_.end(), key);
}

void Parser::remember_failed_template_lists(const TemplateListKey* keys, size_t count)
{
	// `keys` ascend (nested lists open at increasing positions).
	const size_t size = failed_template_lists_.size();
	failed_template_lists_.insert(failed_template_lists_.end(), keys, keys + count);
	std::inplace_merge(
		failed_template_lists_.begin(), failed_template_lists_.begin() + size, failed_template_lists_.end());
}

void Parser::abandon_template_lists()
{
	remember_failed_template_lists(open_lists_.data() + open_lists_base_, open_lists_.size() - open_lists_base_);
	open_lists_.resize(open_lists_base_);
	if (abandoning_lists_)
		return;
	abandoning_lists_ = true;
	abandoned_error_position_ = error_position_;
	abandoned_error_location_ = error_location_;
}

bool Parser::try_outermost_template_arguments(
	Slice<TemplateArgument>& arguments, bool call_follows, const TemplateListKey& key)
{
	// An undeclared name's `<` is tried as a template argument list once. A
	// list that fails inside another's arguments fails every enclosing
	// speculative list with it (abandon_template_lists): read as a value,
	// the inner name leaves the same tokens to the enclosing argument, which
	// then cannot end at `,` or `>` either. Only this outermost list falls
	// back to the value reading, once, and each failed position is
	// remembered, so the enclosing lists are not re-parsed level by level.
	open_lists_.push_back(key);
	bool parsed = false;
	{
		Tentative tentative(*this);
		parsed = parse_template_arguments(arguments) && !abandoning_lists_ &&
			(!call_follows ||
				((at(TokenKind::OP_LPAREN) || at(TokenKind::OP_COLON2)) &&
					member_template_arguments_plausible(arguments)));
		if (parsed)
			tentative.commit();
	}
	if (abandoning_lists_)
	{
		// A nested list's failure returned here: nothing the abandoned
		// lists' parse did after it is kept, its later failures included.
		abandoning_lists_ = false;
		error_position_ = abandoned_error_position_;
		error_location_ = abandoned_error_location_;
	}
	open_lists_.resize(open_lists_base_);
	if (!parsed)
	{
		arguments = Slice<TemplateArgument>{ nullptr, 0 };
		remember_failed_template_lists(&key, 1);
	}
	return parsed;
}

bool Parser::member_template_arguments_plausible(const Slice<TemplateArgument>& arguments)
{
	// `x.count < a || x.count > (b)` where another `count` is a template:
	// read as a template-id the one argument is `a || x.count`, a bare
	// logical expression no member template call spells (its `>` would end
	// the list); the two comparisons are the reading.
	for (const TemplateArgument& argument : arguments)
	{
		const Expr* expression = argument.expression;
		if (expression == nullptr || expression->kind != ExprKind::Binary)
			continue;
		const TokenKind op = static_cast<const BinaryExpr*>(expression)->op;
		if (op == TokenKind::OP_LOR || op == TokenKind::OP_LAND)
			return false;
	}
	return true;
}

bool Parser::parse_template_argument(ListFrame<TemplateArgument>& arguments)
{
	NestingGuard nesting(*this);
	const uint32_t mentions = pack_mentions_;
	// 14.3p2: an argument that can be a type-id is one. A GNU trait's name
	// before `(` is the trait (an expression), whatever a visible
	// declaration of the name (libstdc++'s std::__is_invocable) is.
	TemplateArgument argument = TemplateArgument();
	GnuTrait trait;
	const bool trait_expression = at_trait(trait) && !IsGnuTypeTransform(trait);
	if (!trait_expression && name_starts_here())
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
			if (type != nullptr && (at(TokenKind::OP_COMMA) || at_close_angle() || at(TokenKind::OP_DOTS)))
			{
				tentative.commit();
				argument.type = type;
			}
			if (abandoning_lists_)
				return false;
		}
		if (argument.type == nullptr)
		{
			Expr* primary = primary_from_name(name, info, start);
			argument.expression = primary == nullptr ? nullptr : continue_assignment_expression(primary);
			if (argument.expression == nullptr)
				return false;
		}
	}
	else if (!trait_expression && at_type_specifier_start())
	{
		Tentative tentative(*this);
		TypeId* type = parse_type_id(DeclaratorMode::Abstract, false);
		if (type != nullptr && (at(TokenKind::OP_COMMA) || at_close_angle() || at(TokenKind::OP_DOTS)))
		{
			tentative.commit();
			argument.type = type;
		}
		if (abandoning_lists_)
			return false;
	}
	if (argument.type == nullptr && argument.expression == nullptr)
	{
		argument.expression = parse_assignment_expression();
		if (argument.expression == nullptr)
			return false;
	}
	argument.pack = accept(TokenKind::OP_DOTS);
	if (argument.pack ||
		(argument.type != nullptr && argument.type->declarator != nullptr &&
			argument.type->declarator->pack != PackPosition::None))
		pack_mentions_ = mentions;
	arguments.push(argument);
	return true;
}

void Parser::note_type_name(const SpecifierInfo& info, NameBinding& binding, bool alias_template)
{
	const Name* name = info.type_name;
	if (name == nullptr)
		return;
	const NameInfo& name_info = info.type_name_info;
	if (binding.scope != nullptr && name->last().kind == NameComponentKind::Identifier)
	{
		const bool known = name_info.prefix == PrefixState::Known;
		const bool approximate = name_info.approximate ||
			(known &&
				(name_info.prefix_approximate ||
					(name_info.prefix_specialization && scopes_.has_category_conflicts() &&
						scopes_.member_specialized(name_info.prefix_scope, name->last().identifier))));
		// An alias template's own parameters are each template-id's
		// arguments.
		bool dependent = name_info.dependent;
		if (!dependent && info.type_mentions && known && name_info.prefix_dependent)
			dependent = !current_instantiation(name_info.prefix_scope);
		else if (!dependent && info.type_mentions && name->last().has_arguments &&
			name_info.category == NameCategory::TypeTemplate)
			dependent = !current_instantiation(name_info.scope);
		binding.flags = static_cast<uint8_t>((approximate ? NameBinding::kApproximate : 0) |
			(dependent && !alias_template ? NameBinding::kDependent : 0));
		if (!approximate)
			return;
	}
	else if (binding.scope != nullptr)
		return;
	// A name of a type whose class is unknown or a guess: the names
	// through it may need another reading.
	scopes_.note_uncertain();
}

bool Parser::is_type(const Name* name, const NameInfo& info)
{
	if (name->typename_keyword)
		return true;
	const NameComponent& last = name->last();
	if (last.kind == NameComponentKind::Decltype)
		return true;
	if (last.kind != NameComponentKind::Identifier)
		return false;
	switch (info.category)
	{
	case NameCategory::Type:
	case NameCategory::TypeTemplate:
		return true;
	case NameCategory::Value:
	case NameCategory::FunctionTemplate:
	case NameCategory::Namespace:
		return false;
	case NameCategory::None:
		break;
	}
	// 14.6p2: a dependent qualified name is a value unless `typename`.
	if (info.prefix == PrefixState::Dependent)
		return false;
	return names_type_hint(last.identifier);
}

bool Parser::at_type_specifier_start(uint32_t ahead)
{
	const ParseToken token = tok(ahead);
	if (token.is_identifier())
		return true;
	if (token.kind != ParseTokenKind::Simple)
		return false;
	switch (token.simple)
	{
	case TokenKind::OP_COLON2:
	case TokenKind::KW_CONST:
	case TokenKind::KW_VOLATILE:
	case TokenKind::KW_TYPENAME:
	case TokenKind::KW_DECLTYPE:
	case TokenKind::KW_CLASS:
	case TokenKind::KW_STRUCT:
	case TokenKind::KW_UNION:
	case TokenKind::KW_ENUM:
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

Expr* Parser::parse_decltype_operand()
{
	if (!expect(TokenKind::OP_LPAREN))
		return nullptr;
	const bool saved_angle = angle_;
	angle_ = false;
	Expr* operand = parse_expression();
	angle_ = saved_angle;
	if (operand == nullptr || !expect(TokenKind::OP_RPAREN))
		return nullptr;
	return operand;
}

} // namespace syntax
