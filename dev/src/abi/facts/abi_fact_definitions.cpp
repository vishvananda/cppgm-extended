// Fact definitions and type specifications, read into canonical graph nodes.

#include "abi/facts/abi_fact_case.h"

#include <cstring>

namespace abi_facts {

using namespace abi_mangle;

namespace {

bool starts_with(const char* text, std::uint32_t length, const char* prefix)
{
	const std::size_t prefix_length = std::strlen(prefix);
	return length >= prefix_length && std::memcmp(text, prefix, prefix_length) == 0;
}

enum Modifier : std::uint8_t
{
	MODIFIER_POINTER,
	MODIFIER_LVALUE_REFERENCE,
	MODIFIER_RVALUE_REFERENCE,
	MODIFIER_CONST,
	MODIFIER_VOLATILE,
	MODIFIER_PACK,
	MODIFIER_ARRAY
};

struct ModifierPrefix
{
	const char* prefix;
	Modifier modifier;
};

const ModifierPrefix kModifierPrefixes[] = {
	{ "ptr:", MODIFIER_POINTER },
	{ "ref:", MODIFIER_LVALUE_REFERENCE },
	{ "rref:", MODIFIER_RVALUE_REFERENCE },
	{ "const:", MODIFIER_CONST },
	{ "volatile:", MODIFIER_VOLATILE },
	{ "pack:", MODIFIER_PACK },
	{ "array:", MODIFIER_ARRAY },
};

struct FormWord
{
	const char* text;
	std::uint32_t length;
};

#define ABI_TYPE_FORM(text) { text, sizeof text - 1 }

// The words that begin a multiword type form. In a type specification such
// a word is always the form, so alone it is an incomplete form rather than
// a bare name (`named:ref` names a class called `ref`).
const FormWord kTypeForms[] = {
	ABI_TYPE_FORM("ptr"),
	ABI_TYPE_FORM("ref"),
	ABI_TYPE_FORM("rref"),
	ABI_TYPE_FORM("pack"),
	ABI_TYPE_FORM("const"),
	ABI_TYPE_FORM("volatile"),
	ABI_TYPE_FORM("vendor"),
	ABI_TYPE_FORM("builtin-transform"),
	ABI_TYPE_FORM("named"),
	ABI_TYPE_FORM("name"),
	ABI_TYPE_FORM("template"),
	ABI_TYPE_FORM("std-template"),
	ABI_TYPE_FORM("template-param"),
	ABI_TYPE_FORM("template-param-subst"),
	ABI_TYPE_FORM("template-param-template"),
	ABI_TYPE_FORM("member"),
	ABI_TYPE_FORM("member-template"),
	ABI_TYPE_FORM("decltype"),
	ABI_TYPE_FORM("decltype-id"),
	ABI_TYPE_FORM("lambda-closure"),
	ABI_TYPE_FORM("local-type"),
	ABI_TYPE_FORM("namespace-lambda"),
	ABI_TYPE_FORM("tagged"),
	ABI_TYPE_FORM("function-type"),
	ABI_TYPE_FORM("function-type-variadic"),
	ABI_TYPE_FORM("member-pointer"),
};

#undef ABI_TYPE_FORM

bool is_type_form_word(const FactWord& word)
{
	for (const FormWord& form : kTypeForms)
	{
		if (form.length == word.length && std::memcmp(form.text, word.text, word.length) == 0)
			return true;
	}
	return false;
}

struct PendingModifier
{
	Modifier modifier;
	AbiArrayBound bound_kind;
	long long bound_value;
	AbiNode bound_expression;
};

AbiNode apply_modifier(AbiGraph& graph, const PendingModifier& pending, AbiNode type)
{
	switch (pending.modifier)
	{
	case MODIFIER_POINTER:
		return graph.pointer(type);
	case MODIFIER_LVALUE_REFERENCE:
		return graph.lvalue_reference(type);
	case MODIFIER_RVALUE_REFERENCE:
		return graph.rvalue_reference(type);
	case MODIFIER_CONST:
		return graph.qualified(type, ABI_QUALIFIER_CONST);
	case MODIFIER_VOLATILE:
		return graph.qualified(type, ABI_QUALIFIER_VOLATILE);
	case MODIFIER_PACK:
		return graph.pack_expansion(type);
	case MODIFIER_ARRAY:
		break;
	}
	return graph.array(type, pending.bound_kind, pending.bound_value, pending.bound_expression);
}

} // namespace

void drop_void_parameter(const AbiGraph& graph, std::vector<AbiNode>& params)
{
	if (params.size() == 1 && graph.kind(params[0]) == ABI_NODE_BUILTIN &&
		graph.node(params[0]).code == ABI_BUILTIN_TYPE_VOID)
		params.clear();
}

bool is_generated_lambda_name(const FactWord& word)
{
	if (word.length < 3 || word.text[0] != '$' || word.text[1] != '_')
		return false;
	for (std::uint32_t i = 2; i < word.length; ++i)
	{
		if (word.text[i] < '0' || word.text[i] > '9')
			return false;
	}
	return true;
}

AbiNode CaseReader::read_definition(const Definition& definition)
{
	const FactWord* words = line_words(definition.line) + 2;
	const std::uint32_t count = line_size(definition.line) - 2;
	switch (definition.kind)
	{
	case DEFINITION_TYPE:
		return type_spec(definition.line, words, count);
	case DEFINITION_ARGUMENT:
		return read_argument_definition(definition.line, words, count);
	case DEFINITION_EXPRESSION:
		return read_expression_definition(definition.line, words, count);
	case DEFINITION_CONTEXT:
		return read_context_definition(definition.line, words, count);
	case DEFINITION_ENTITY:
		return read_entity_definition(definition.line, words, count);
	}
	fail(definition.line, "invalid definition");
}

// A type specification: one compact word, or a form word and its operands.
// The wrapper forms (`ptr`, `ref`, `rref`, `pack`, `const`, `volatile`,
// `vendor <qualifier>`, `builtin-transform <name>`) take the rest of the
// line as their operand; they are peeled in one pass and applied innermost
// first.
AbiNode CaseReader::type_spec(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	std::vector<const FactWord*> wrappers;
	while (count >= 2)
	{
		const FactWord& form = words[0];
		std::uint32_t width = 0;
		if (form.is("ptr") || form.is("ref") || form.is("rref") || form.is("pack") || form.is("const") ||
			form.is("volatile"))
			width = 1;
		else if ((form.is("vendor") || form.is("builtin-transform")) && count >= 3)
			width = 2;
		else
			break;
		wrappers.push_back(words);
		words += width;
		count -= width;
	}
	AbiNode type = type_form(line, words, count);
	for (std::size_t i = wrappers.size(); i > 0; --i)
	{
		const FactWord& form = wrappers[i - 1][0];
		if (form.is("ptr"))
			type = graph_.pointer(type);
		else if (form.is("ref"))
			type = graph_.lvalue_reference(type);
		else if (form.is("rref"))
			type = graph_.rvalue_reference(type);
		else if (form.is("pack"))
			type = graph_.pack_expansion(type);
		else if (form.is("const") || form.is("volatile"))
			type = graph_.qualified(type, form.is("const") ? ABI_QUALIFIER_CONST : ABI_QUALIFIER_VOLATILE);
		else if (form.is("vendor"))
		{
			type = graph_.vendor_qualified(type, string(wrappers[i - 1][1]));
		}
		else
			type = graph_.vendor_type(string(wrappers[i - 1][1]), &type, 1);
	}
	return type;
}

AbiNode CaseReader::type_form(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	if (count == 0)
		fail(line, "missing type");
	if (count == 1)
	{
		if (is_type_form_word(words[0]))
			fail(line, "incomplete type form '" + words[0].str() + "'");
		return compact_type(line, words[0].text, words[0].length);
	}
	const FactWord& form = words[0];
	const FactWord* operands = words + 1;
	const std::uint32_t n = count - 1;
	if ((form.is("named") || form.is("name")) && n == 1)
		return named_type(line, operands[0].text, operands[0].length);
	if (form.is("template") && n >= 2)
	{
		const AbiNode templ = qualified_name(line, operands[0].text, operands[0].length);
		argument_list(operands + 1, n - 1, scratch_);
		return graph_.template_id(templ, scratch_.data(), scratch_.size());
	}
	if (form.is("std-template") && n >= 4)
	{
		const AbiStandardSubstitutionKind kind = abi_standard_substitution_kind(operands[0].text, operands[0].length);
		if (kind == ABI_STANDARD_SUBSTITUTION_NONE)
			fail(line, "unknown standard substitution '" + operands[0].str() + "'");
		const AbiNode abbreviation = graph_.std_abbreviation(kind);
		argument_list(operands + 3, n - 3, scratch_);
		if (flag_value(line, operands[1]))
			return abbreviation;
		return graph_.template_id(abbreviation, scratch_.data(), scratch_.size());
	}
	if ((form.is("template-param") || form.is("template-param-subst")) && n == 1)
		return graph_.template_param(index_value(line, operands[0]), form.is("template-param-subst"));
	if (form.is("template-param-template") && n >= 2)
	{
		// The template-template parameter, a <template-prefix>, is itself a
		// substitution candidate (5.1.10: <template-template-param>).
		const AbiNode param = graph_.template_param(index_value(line, operands[0]), true);
		argument_list(operands + 1, n - 1, scratch_);
		return graph_.template_id(param, scratch_.data(), scratch_.size());
	}
	if (form.is("member") && n == 2)
		return graph_.name(compact_type(line, operands[0].text, operands[0].length), string(operands[1]));
	if (form.is("member-template") && n >= 3)
	{
		const AbiNode owner = compact_type(line, operands[0].text, operands[0].length);
		const AbiNode templ = graph_.name(owner, string(operands[1]));
		argument_list(operands + 2, n - 2, scratch_);
		return graph_.template_id(templ, scratch_.data(), scratch_.size());
	}
	// decltype of an unparenthesized id-expression or member access is Dt,
	// of another expression DT (5.1.5.8).
	if ((form.is("decltype") || form.is("decltype-id")) && n == 1)
		return graph_.decltype_of(reference(operands[0], DEFINITION_EXPRESSION, "expression"), form.is("decltype-id"));
	if (form.is("lambda-closure") && n == 2)
		return closure_entity(line, operands[0], operands[1], nullptr, 0);
	if (form.is("local-type") && n == 3)
		return local_entity(line, operands[0], string(operands[1]), operands[2]);
	if (form.is("namespace-lambda"))
		return graph_.name(namespace_path(line, operands + 1, n - 1), string(operands[0]));
	if (form.is("tagged") && n >= 2)
	{
		const AbiNode type = compact_type(line, operands[0].text, operands[0].length);
		const AbiNode named = graph_.kind(type) == ABI_NODE_TEMPLATE_ID ? graph_.node(type).a : type;
		if (graph_.kind(named) != ABI_NODE_NAME && graph_.kind(named) != ABI_NODE_STD_ABBREVIATION)
			fail(line, "tagged type needs a named or class-template type");
		std::vector<AbiNode> tags;
		for (std::uint32_t i = 1; i < n; ++i)
			tags.push_back(string(operands[i]));
		return graph_.with_tags(type, tags.data(), tags.size());
	}
	if ((form.is("function-type") || form.is("function-type-variadic")) && n >= 1)
	{
		const AbiNode result = compact_type(line, operands[0].text, operands[0].length);
		std::vector<AbiNode> params;
		for (std::uint32_t i = 1; i < n; ++i)
			params.push_back(compact_type(line, operands[i].text, operands[i].length));
		drop_void_parameter(graph_, params);
		return graph_.function_type(result, params.data(), params.size(), form.is("function-type-variadic"), 0);
	}
	if (form.is("member-pointer") && n == 2)
	{
		// Operands are read in order, each by its own statement: a call's
		// arguments are evaluated in an unspecified order, and the first
		// bad operand is the one reported.
		const AbiNode owner = compact_type(line, operands[0].text, operands[0].length);
		return graph_.member_pointer(owner, compact_type(line, operands[1].text, operands[1].length));
	}
	fail(line, "unknown or malformed type form '" + form.str() + "'");
}

// A compact type word: modifier prefixes (`ptr:`, `const:`, `array:N:`,
// ...) applied to a builtin word, a type definition, `named:<name>`,
// `memberptr:<owner>:<member>` or a bare qualified name.
AbiNode CaseReader::compact_type(std::uint32_t line, const char* text, std::uint32_t length)
{
	PendingModifier modifiers[64];
	std::uint32_t modifier_count = 0;
	std::vector<PendingModifier> overflow;
	for (;;)
	{
		const ModifierPrefix* matched = nullptr;
		for (const ModifierPrefix& candidate : kModifierPrefixes)
		{
			if (starts_with(text, length, candidate.prefix))
			{
				matched = &candidate;
				break;
			}
		}
		if (matched == nullptr)
			break;
		const std::uint32_t prefix_length = static_cast<std::uint32_t>(std::strlen(matched->prefix));
		text += prefix_length;
		length -= prefix_length;
		PendingModifier pending{ matched->modifier, ABI_ARRAY_BOUND_NONE, 0, ABI_NONE };
		if (matched->modifier == MODIFIER_ARRAY)
		{
			const char* colon = static_cast<const char*>(std::memchr(text, ':', length));
			if (colon == nullptr || colon == text)
				fail(line, "missing array bound");
			const FactWord bound{ text, static_cast<std::uint32_t>(colon - text) };
			if (bound.text[0] >= '0' && bound.text[0] <= '9')
			{
				pending.bound_kind = ABI_ARRAY_BOUND_VALUE;
				pending.bound_value = integer_value(line, bound);
			}
			else
			{
				pending.bound_kind = ABI_ARRAY_BOUND_EXPRESSION;
				pending.bound_expression = reference(bound, DEFINITION_EXPRESSION, "array bound expression");
			}
			length -= bound.length + 1;
			text = colon + 1;
		}
		if (modifier_count < 64)
			modifiers[modifier_count++] = pending;
		else
			overflow.push_back(pending);
	}
	if (length == 0)
		fail(line, "missing ABI type after prefix modifier");
	AbiNode type;
	if (starts_with(text, length, "named:"))
		type = named_type(line, text + 6, length - 6);
	else if (starts_with(text, length, "memberptr:"))
		type = compact_member_pointer(line, text + 10, length - 10);
	else
	{
		const FactWord word{ text, length };
		if (is_defined_as(word, DEFINITION_TYPE))
			type = reference(word, DEFINITION_TYPE, "type");
		else
		{
			const AbiBuiltinTypeKind builtin = abi_builtin_type_kind(text, length);
			type = builtin != ABI_BUILTIN_TYPE_NONE ? graph_.builtin(builtin) : named_type(line, text, length);
		}
	}
	for (std::size_t i = overflow.size(); i > 0; --i)
		type = apply_modifier(graph_, overflow[i - 1], type);
	for (std::uint32_t i = modifier_count; i > 0; --i)
		type = apply_modifier(graph_, modifiers[i - 1], type);
	return type;
}

// `memberptr:<owner>:<member>`: the owner is a qualified name or type
// definition ending at the first single colon; `::` never delimits.
AbiNode CaseReader::compact_member_pointer(std::uint32_t line, const char* text, std::uint32_t length)
{
	std::uint32_t split = 0;
	while (split < length)
	{
		if (text[split] == ':')
		{
			if (split + 1 < length && text[split + 1] == ':')
			{
				split += 2;
				continue;
			}
			break;
		}
		++split;
	}
	if (split == 0 || split >= length - 1)
		fail(line, "invalid compact member-pointer type");
	const FactWord owner_word{ text, split };
	const AbiNode owner = is_defined_as(owner_word, DEFINITION_TYPE) ? reference(owner_word, DEFINITION_TYPE, "type")
																	 : named_type(line, text, split);
	return graph_.member_pointer(owner, compact_type(line, text + split + 1, length - split - 1));
}

AbiNode CaseReader::named_type(std::uint32_t line, const char* text, std::uint32_t length)
{
	const AbiNode name = qualified_name(line, text, length);
	if (graph_.kind(name) != ABI_NODE_NAME)
		fail(line, "empty named ABI type");
	return name;
}

// Resolving an argument can read further definitions, so the list is built
// locally and handed over complete.
void CaseReader::argument_list(const FactWord* words, std::uint32_t count, std::vector<AbiNode>& args)
{
	std::vector<AbiNode> resolved;
	resolved.reserve(count);
	for (std::uint32_t i = 0; i < count; ++i)
		resolved.push_back(reference(words[i], DEFINITION_ARGUMENT, "template argument"));
	args.swap(resolved);
}

AbiNode CaseReader::read_argument_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const FactWord& form = words[0];
	const FactWord* operands = words + 1;
	const std::uint32_t n = count - 1;
	if (form.is("type"))
		return type_spec(line, operands, n);
	if (form.is("value") && n == 2)
		return typed_literal(line, compact_type(line, operands[0].text, operands[0].length), operands[1]);
	if (form.is("dependent-value") && n == 3)
	{
		// (Operands in order: see the member-pointer type form.)
		const AbiNode type = compact_type(line, operands[0].text, operands[0].length);
		return graph_.typed_argument(
			type, typed_literal(line, compact_type(line, operands[1].text, operands[1].length), operands[2]));
	}
	if (form.is("expression") && n == 1)
		return graph_.expression_argument(reference(operands[0], DEFINITION_EXPRESSION, "expression"));
	if (form.is("template-entity") && n == 1)
		return named_type(line, operands[0].text, operands[0].length);
	if (form.is("member-template-entity") && n == 3)
		return graph_.name(compact_type(line, operands[0].text, operands[0].length), string(operands[1]));
	if (form.is("template-param-template") && n == 1)
		return graph_.template_param(index_value(line, operands[0]), false);
	if ((form.is("entity-address") || form.is("entity-reference")) && n == 1)
		return graph_.entity_argument(reference(operands[0], DEFINITION_ENTITY, "entity"), form.is("entity-address"));
	if (form.is("member-external-address"))
		return read_member_address(line, operands, n);
	if (form.is("pack"))
	{
		argument_list(operands, n, scratch_);
		return graph_.argument_pack(scratch_.data(), scratch_.size());
	}
	fail(line, "unknown or malformed template argument form '" + form.str() + "'");
}

// `member-external-address <symbol> <owner> <name> <function> <const>
// <volatile> <lvalue-ref> <rvalue-ref> <variadic> <param>...`: the address
// of a member, encoded from its typed owner and callable shape. The symbol
// spelling is the member's external name for readers; it is not reparsed.
AbiNode CaseReader::read_member_address(std::uint32_t line, const FactWord* operands, std::uint32_t n)
{
	if (n < 9 || operands[0].length == 0)
		fail(line, "member external address has invalid operands");
	const AbiNode owner = compact_type(line, operands[1].text, operands[1].length);
	const AbiNode member = graph_.name(owner, string(operands[2]));
	if (!flag_value(line, operands[3]))
	{
		if (n != 9)
			fail(line, "member data address has parameter types");
		return graph_.entity_argument(member, true);
	}
	unsigned qualifiers = 0;
	if (flag_value(line, operands[4]))
		qualifiers |= ABI_QUALIFIER_CONST;
	if (flag_value(line, operands[5]))
		qualifiers |= ABI_QUALIFIER_VOLATILE;
	if (flag_value(line, operands[6]))
		qualifiers |= ABI_QUALIFIER_LVALUE_REF;
	if (flag_value(line, operands[7]))
		qualifiers |= ABI_QUALIFIER_RVALUE_REF;
	if ((qualifiers & ABI_QUALIFIER_LVALUE_REF) != 0 && (qualifiers & ABI_QUALIFIER_RVALUE_REF) != 0)
		fail(line, "member external entity has conflicting ref qualifiers");
	const bool variadic = flag_value(line, operands[8]);
	std::vector<AbiNode> params;
	for (std::uint32_t i = 9; i < n; ++i)
		params.push_back(compact_type(line, operands[i].text, operands[i].length));
	drop_void_parameter(graph_, params);
	const AbiNode encoding = graph_.function_encoding(
		member, ABI_NONE, params.data(), params.size(), variadic, qualifiers);
	return graph_.entity_argument(encoding, true);
}

// A typed integral value (README "value" facts): signed values keep their
// sign; a negative value of an unsigned builtin denotes its modulo bit
// pattern at the type's width.
AbiNode CaseReader::typed_literal(std::uint32_t line, AbiNode type, const FactWord& word)
{
	long long value = integer_value(line, word);
	const AbiNodeData& data = graph_.node(type);
	if (data.kind != ABI_NODE_BUILTIN)
		return graph_.literal(type, value);
	const AbiBuiltinTypeKind builtin = static_cast<AbiBuiltinTypeKind>(data.code);
	const unsigned bits = abi_builtin_type_integral_bits(builtin);
	if (bits == 0)
		fail(line, "value needs an integral type");
	const bool negative_input = word.text[0] == '-';
	if (builtin == ABI_BUILTIN_TYPE_BOOL && value != 0 && value != 1)
		fail(line, "bool ABI value must be 0 or 1");
	if (abi_builtin_type_is_unsigned(builtin))
	{
		if (bits < 64)
		{
			const unsigned long long mask = (1ull << bits) - 1;
			if (!negative_input && static_cast<unsigned long long>(value) > mask)
				fail(line, "unsigned ABI value out of range '" + word.str() + "'");
			value = static_cast<long long>(static_cast<unsigned long long>(value) & mask);
		}
		return graph_.literal(type, value);
	}
	if (!negative_input && value < 0)
		fail(line, "signed ABI value out of range '" + word.str() + "'");
	if (bits < 64)
	{
		const long long limit = 1ll << (bits - 1);
		if (value < -limit || value >= limit)
			fail(line, "signed ABI value out of range '" + word.str() + "'");
	}
	return graph_.literal(type, value);
}

AbiNode CaseReader::read_expression_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const FactWord& form = words[0];
	const FactWord* operands = words + 1;
	const std::uint32_t n = count - 1;
	if (form.is("template-param") && n == 1)
		return graph_.template_param(index_value(line, operands[0]), false);
	if (form.is("function-param") && n == 1)
		return graph_.function_param(index_value(line, operands[0]));
	if (form.is("literal") && n == 1)
		return graph_.literal(graph_.builtin(ABI_BUILTIN_TYPE_INT), integer_value(line, operands[0]));
	if (form.is("sizeof-type") && n == 1)
		return graph_.sizeof_type(compact_type(line, operands[0].text, operands[0].length));
	if (form.is("pack") && n == 1)
	{
		const AbiNode operand = reference(operands[0], DEFINITION_EXPRESSION, "expression");
		return graph_.operation(abi_pack_operation_code('s', 'p'), &operand, 1);
	}
	if (form.is("entity-reference") && n == 1)
		return graph_.entity_reference(reference(operands[0], DEFINITION_ENTITY, "entity"));
	if (form.is("member") && n == 3)
	{
		// (Operands in order: see the member-pointer type form.)
		const AbiNode scope = compact_type(line, operands[0].text, operands[0].length);
		const bool flag = flag_value(line, operands[1]);
		return graph_.scoped_member(scope, string(operands[2]), flag);
	}
	// An unqualified dependent name (a call's callee): its <source-name>.
	if (form.is("source-name") && n == 1)
		return graph_.unresolved_name(string(operands[0]), nullptr, 0);
	if (form.is("template-id") && n >= 1)
	{
		argument_list(operands + 1, n - 1, scratch_);
		return graph_.unresolved_name(string(operands[0]), scratch_.data(), scratch_.size());
	}
	if (form.is("type-trait") && n >= 2)
	{
		std::vector<AbiNode> types;
		for (std::uint32_t i = 1; i < n; ++i)
			types.push_back(compact_type(line, operands[i].text, operands[i].length));
		return graph_.vendor_expression(string(operands[0]), types.data(), types.size());
	}
	if (form.is("cast") && n == 3)
	{
		const std::uint16_t code = abi_expression_operation_code(
			operands[0].text, operands[0].length, ABI_EXPRESSION_SHAPE_CAST);
		if (code == 0)
			fail(line, "unknown cast operation '" + operands[0].str() + "'");
		// (Operands in order: see the member-pointer type form.)
		const AbiNode type = compact_type(line, operands[1].text, operands[1].length);
		return graph_.cast(code, type, reference(operands[2], DEFINITION_EXPRESSION, "expression"));
	}
	if (form.is("object-member") && n >= 3)
	{
		const std::uint16_t code = abi_expression_operation_code(
			operands[0].text, operands[0].length, ABI_EXPRESSION_SHAPE_MEMBER_ACCESS);
		if (code == 0)
			fail(line, "unknown member access operation '" + operands[0].str() + "'");
		const AbiNode object = reference(operands[1], DEFINITION_EXPRESSION, "expression");
		argument_list(operands + 3, n - 3, scratch_);
		return graph_.member_access(code, object, string(operands[2]), scratch_.data(), scratch_.size());
	}
	AbiExpressionShape shape = ABI_EXPRESSION_SHAPE_UNARY;
	std::uint32_t operand_count = 0;
	const FactWord* first_operand = operands + 1;
	std::uint16_t code = 0;
	if (form.is("unary") && n == 2)
	{
		operand_count = 1;
	}
	else if (form.is("binary") && n == 3)
	{
		shape = ABI_EXPRESSION_SHAPE_BINARY;
		operand_count = 2;
	}
	else if (form.is("conditional") && n == 3)
	{
		code = abi_pack_operation_code('q', 'u');
		operand_count = 3;
		first_operand = operands;
	}
	else if (form.is("call") && n >= 1)
	{
		std::vector<AbiNode> items;
		for (std::uint32_t i = 0; i < n; ++i)
			items.push_back(reference(operands[i], DEFINITION_EXPRESSION, "expression"));
		return graph_.call(items.data(), items.size());
	}
	else
		fail(line, "unknown or malformed dependent expression form '" + form.str() + "'");
	if (code == 0)
	{
		code = abi_expression_operation_code(operands[0].text, operands[0].length, shape);
		if (code == 0)
			fail(line, "unknown expression operation '" + operands[0].str() + "'");
	}
	AbiNode items[3];
	for (std::uint32_t i = 0; i < operand_count; ++i)
		items[i] = reference(first_operand[i], DEFINITION_EXPRESSION, "expression");
	return graph_.operation(code, items, operand_count);
}

AbiNode CaseReader::read_context_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	if (words[0].is("raw"))
	{
		if (count != 2)
			fail(line, "raw context takes one fragment");
		return graph_.raw_function_scope(string(words[1]));
	}
	if (words[0].is("function") && count >= 2)
		return graph_.function_scope(function_entity(line, static_cast<std::uint32_t>(words + 1 - line_words(line))));
	fail(line, "unknown context form '" + words[0].str() + "'");
}

AbiNode CaseReader::read_entity_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const FactWord& form = words[0];
	if ((form.is("variable") || form.is("internal-variable")) && count == 2)
	{
		const AbiNode name = named_type(line, words[1].text, words[1].length);
		if (form.is("variable"))
			return name;
		const AbiNodeData data = graph_.node(name);
		return graph_.name(data.a, data.b, graph_.list(name), data.list_size, data.c, true);
	}
	if (form.is("symbol") && count == 2)
		return graph_.external_symbol(string(words[1]));
	if (form.is("function") && count >= 2)
		return function_entity(line, static_cast<std::uint32_t>(words + 1 - line_words(line)));
	fail(line, "unknown or malformed entity form '" + form.str() + "'");
}

// A local entity directly in a function: the discriminator word counts
// earlier entities of the same name (0 for the first, 1 for `_0`, ...).
AbiNode CaseReader::local_entity(
	std::uint32_t line, const FactWord& context, AbiNode unqualified, const FactWord& discriminator)
{
	const AbiNode scope = reference(context, DEFINITION_CONTEXT, "context");
	return graph_.name(scope, unqualified, nullptr, 0, index_value(line, discriminator));
}

// A closure type in a function: the number word is the Itanium closure
// number (`-` when omitted, for the first closure with its signature).
AbiNode CaseReader::closure_entity(
	std::uint32_t line, const FactWord& context, const FactWord& number, const FactWord* signature, std::uint32_t count)
{
	const AbiNode scope = reference(context, DEFINITION_CONTEXT, "context");
	const std::uint32_t closure_number = number.is("-") ? 0 : index_value(line, number) + 1;
	std::vector<AbiNode> types;
	for (std::uint32_t i = 0; i < count; ++i)
		types.push_back(compact_type(line, signature[i].text, signature[i].length));
	drop_void_parameter(graph_, types);
	return graph_.name(scope, graph_.closure_name(types.data(), types.size(), closure_number));
}

AbiNode CaseReader::namespace_path(std::uint32_t line, const FactWord* names, std::uint32_t count)
{
	AbiNode scope = ABI_NONE;
	for (std::uint32_t i = 0; i < count; ++i)
	{
		if (std::memchr(names[i].text, ':', names[i].length) != nullptr)
			fail(line, "stray ':' in ABI namespace qualifier '" + names[i].str() + "'");
		scope = i == 0 && names[i].is("std") ? graph_.std_scope() : graph_.name(scope, string(names[i]));
	}
	return scope;
}

} // namespace abi_facts
