// Fact definitions and type specifications, read into canonical graph nodes.

#include "abi/facts/abi_fact_case.h"

#include <cstring>

namespace abi_facts {

using namespace abi_mangle;

namespace {

// A compact prefix (ptr: ... array:) of a compact type word, applied after
// the word's type is read.
struct PendingModifier
{
	AbiFactTypePrefix modifier;
	AbiArrayBound bound_kind;
	long long bound_value;
	AbiNode bound_expression;
};

AbiNode apply_modifier(AbiGraph& graph, const PendingModifier& pending, AbiNode type)
{
	switch (pending.modifier)
	{
	case ABI_FACT_PREFIX_PTR:
		return graph.pointer(type);
	case ABI_FACT_PREFIX_REF:
		return graph.lvalue_reference(type);
	case ABI_FACT_PREFIX_RREF:
		return graph.rvalue_reference(type);
	case ABI_FACT_PREFIX_CONST:
		return graph.qualified(type, ABI_QUALIFIER_CONST);
	case ABI_FACT_PREFIX_VOLATILE:
		return graph.qualified(type, ABI_QUALIFIER_VOLATILE);
	case ABI_FACT_PREFIX_PACK:
		return graph.pack_expansion(type);
	default:
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
	case ABI_FACT_DEFINITION_TYPE:
		return type_spec(definition.line, words, count);
	case ABI_FACT_DEFINITION_ARGUMENT:
		return read_argument_definition(definition.line, words, count);
	case ABI_FACT_DEFINITION_EXPRESSION:
		return read_expression_definition(definition.line, words, count);
	case ABI_FACT_DEFINITION_CONTEXT:
		return read_context_definition(definition.line, words, count);
	case ABI_FACT_DEFINITION_ENTITY:
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
	struct Wrapper
	{
		AbiFactTypeForm form;
		const FactWord* operand; // vendor's qualifier, builtin-transform's name
	};
	std::vector<Wrapper> wrappers;
	const FactWordClass* form = &classify_fact_word(
		count != 0 ? words[0].text : nullptr, count != 0 ? words[0].length : 0);
	while (count >= 2)
	{
		const AbiFactTypeForm wrapper = static_cast<AbiFactTypeForm>(form->type_form);
		std::uint32_t width = 0;
		if (wrapper <= ABI_FACT_TYPE_VOLATILE) // ptr ref rref pack const volatile
			width = 1;
		else if ((wrapper == ABI_FACT_TYPE_VENDOR || wrapper == ABI_FACT_TYPE_BUILTIN_TRANSFORM) && count >= 3)
			width = 2;
		else
			break;
		wrappers.push_back(Wrapper{ wrapper, words + 1 });
		words += width;
		count -= width;
		form = &words[0].classify();
	}
	AbiNode type = type_form(line, words, count, *form);
	for (std::size_t i = wrappers.size(); i > 0; --i)
	{
		const Wrapper& wrapper = wrappers[i - 1];
		switch (wrapper.form)
		{
		case ABI_FACT_TYPE_PTR:
			type = graph_.pointer(type);
			break;
		case ABI_FACT_TYPE_REF:
			type = graph_.lvalue_reference(type);
			break;
		case ABI_FACT_TYPE_RREF:
			type = graph_.rvalue_reference(type);
			break;
		case ABI_FACT_TYPE_PACK:
			type = graph_.pack_expansion(type);
			break;
		case ABI_FACT_TYPE_CONST:
			type = graph_.qualified(type, ABI_QUALIFIER_CONST);
			break;
		case ABI_FACT_TYPE_VOLATILE:
			type = graph_.qualified(type, ABI_QUALIFIER_VOLATILE);
			break;
		case ABI_FACT_TYPE_VENDOR:
			type = graph_.vendor_qualified(type, string(*wrapper.operand));
			break;
		default:
			type = graph_.vendor_type(string(*wrapper.operand), &type, 1);
			break;
		}
	}
	return type;
}

AbiNode CaseReader::type_form(std::uint32_t line, const FactWord* words, std::uint32_t count, const FactWordClass& form)
{
	if (count == 0)
		fail(line, "missing type");
	// In a type specification a type-form word is always the form, so alone
	// it is an incomplete form rather than a bare name (`named:ref` names a
	// class called `ref`).
	if (count == 1)
	{
		if (form.type_form != kNotFactWord)
			fail(line, "incomplete type form '" + words[0].str() + "'");
		return compact_type(line, words[0].text, words[0].length, &form);
	}
	const FactWord* operands = words + 1;
	const std::uint32_t n = count - 1;
	switch (static_cast<AbiFactTypeForm>(form.type_form))
	{
	case ABI_FACT_TYPE_NAMED:
	case ABI_FACT_TYPE_NAME:
		if (n == 1)
			return named_type(line, operands[0].text, operands[0].length);
		break;
	case ABI_FACT_TYPE_TEMPLATE:
		if (n >= 2)
		{
			const AbiNode templ = qualified_name(line, operands[0].text, operands[0].length);
			argument_list(operands + 1, n - 1, scratch_);
			return graph_.template_id(templ, scratch_.data(), scratch_.size());
		}
		break;
	case ABI_FACT_TYPE_STD_TEMPLATE:
		if (n >= 4)
		{
			const AbiStandardSubstitutionKind kind = abi_standard_substitution_kind(
				operands[0].text, operands[0].length);
			if (kind == ABI_STANDARD_SUBSTITUTION_NONE)
				fail(line, "unknown standard substitution '" + operands[0].str() + "'");
			const AbiNode abbreviation = graph_.std_abbreviation(kind);
			argument_list(operands + 3, n - 3, scratch_);
			if (flag_value(line, operands[1]))
				return abbreviation;
			return graph_.template_id(abbreviation, scratch_.data(), scratch_.size());
		}
		break;
	case ABI_FACT_TYPE_TEMPLATE_PARAM:
	case ABI_FACT_TYPE_TEMPLATE_PARAM_SUBST:
		if (n == 1)
			return graph_.template_param(
				index_value(line, operands[0]), form.type_form == ABI_FACT_TYPE_TEMPLATE_PARAM_SUBST);
		break;
	case ABI_FACT_TYPE_TEMPLATE_PARAM_TEMPLATE:
		if (n >= 2)
		{
			// The template-template parameter, a <template-prefix>, is itself
			// a substitution candidate (5.1.10: <template-template-param>).
			const AbiNode param = graph_.template_param(index_value(line, operands[0]), true);
			argument_list(operands + 1, n - 1, scratch_);
			return graph_.template_id(param, scratch_.data(), scratch_.size());
		}
		break;
	case ABI_FACT_TYPE_MEMBER:
		if (n == 2)
			return graph_.name(compact_type(line, operands[0].text, operands[0].length), string(operands[1]));
		break;
	case ABI_FACT_TYPE_MEMBER_TEMPLATE:
		if (n >= 3)
		{
			const AbiNode owner = compact_type(line, operands[0].text, operands[0].length);
			const AbiNode templ = graph_.name(owner, string(operands[1]));
			argument_list(operands + 2, n - 2, scratch_);
			return graph_.template_id(templ, scratch_.data(), scratch_.size());
		}
		break;
	case ABI_FACT_TYPE_DECLTYPE:
	case ABI_FACT_TYPE_DECLTYPE_ID:
		// decltype of an unparenthesized id-expression or member access is
		// Dt, of another expression DT (5.1.5.8).
		if (n == 1)
			return graph_.decltype_of(reference(operands[0], ABI_FACT_DEFINITION_EXPRESSION, "expression"),
				form.type_form == ABI_FACT_TYPE_DECLTYPE_ID);
		break;
	case ABI_FACT_TYPE_LAMBDA_CLOSURE:
		if (n == 2)
			return closure_entity(line, operands[0], operands[1], nullptr, 0);
		break;
	case ABI_FACT_TYPE_LOCAL_TYPE:
		if (n == 3)
			return local_entity(line, operands[0], string(operands[1]), operands[2]);
		break;
	case ABI_FACT_TYPE_NAMESPACE_LAMBDA:
		return graph_.name(namespace_path(line, operands + 1, n - 1), string(operands[0]));
	case ABI_FACT_TYPE_TAGGED:
		if (n >= 2)
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
		break;
	case ABI_FACT_TYPE_FUNCTION_TYPE:
	case ABI_FACT_TYPE_FUNCTION_TYPE_VARIADIC:
	{
		const AbiNode result = compact_type(line, operands[0].text, operands[0].length);
		std::vector<AbiNode> params;
		for (std::uint32_t i = 1; i < n; ++i)
			params.push_back(compact_type(line, operands[i].text, operands[i].length));
		drop_void_parameter(graph_, params);
		return graph_.function_type(
			result, params.data(), params.size(), form.type_form == ABI_FACT_TYPE_FUNCTION_TYPE_VARIADIC, 0);
	}
	case ABI_FACT_TYPE_MEMBER_POINTER:
		if (n == 2)
		{
			// Operands are read in order, each by its own statement: a call's
			// arguments are evaluated in an unspecified order, and the first
			// bad operand is the one reported.
			const AbiNode owner = compact_type(line, operands[0].text, operands[0].length);
			return graph_.member_pointer(owner, compact_type(line, operands[1].text, operands[1].length));
		}
		break;
	default:
		break;
	}
	fail(line, "unknown or malformed type form '" + words[0].str() + "'");
}

// A compact type word: modifier prefixes (`ptr:`, `const:`, `array:N:`,
// ...) applied to a builtin word, a type definition, `named:<name>`,
// `memberptr:<owner>:<member>` or a bare qualified name.
AbiNode CaseReader::compact_type(
	std::uint32_t line, const char* text, std::uint32_t length, const FactWordClass* word_class)
{
	PendingModifier modifiers[64];
	std::uint32_t modifier_count = 0;
	std::vector<PendingModifier> overflow;
	// The modifier prefixes (ptr: ... array:), then a compact head (named:,
	// memberptr:) or the type's word.
	std::uint8_t head = kNotFactWord;
	std::uint32_t head_length = 0;
	for (;;)
	{
		const std::uint8_t prefix = length != 0 ? kFactVocabulary.prefix_at(text, length) : kNotFactWord;
		if (prefix == kNotFactWord)
			break;
		const std::uint32_t prefix_length = kFactVocabulary.prefix_length(prefix);
		if (prefix > ABI_FACT_PREFIX_ARRAY)
		{
			head = prefix;
			head_length = prefix_length;
			break;
		}
		text += prefix_length;
		length -= prefix_length;
		PendingModifier pending{ static_cast<AbiFactTypePrefix>(prefix), ABI_ARRAY_BOUND_NONE, 0, ABI_NONE };
		if (prefix == ABI_FACT_PREFIX_ARRAY)
		{
			const char* bound_end = static_cast<const char*>(std::memchr(text, ':', length));
			if (bound_end == nullptr || bound_end == text)
				fail(line, "missing array bound");
			const FactWord bound{ text, static_cast<std::uint32_t>(bound_end - text) };
			if (bound.text[0] >= '0' && bound.text[0] <= '9')
			{
				pending.bound_kind = ABI_ARRAY_BOUND_VALUE;
				pending.bound_value = integer_value(line, bound);
			}
			else
			{
				pending.bound_kind = ABI_ARRAY_BOUND_EXPRESSION;
				pending.bound_expression = reference(bound, ABI_FACT_DEFINITION_EXPRESSION, "array bound expression");
			}
			length -= bound.length + 1;
			text = bound_end + 1;
		}
		if (modifier_count < 64)
			modifiers[modifier_count++] = pending;
		else
			overflow.push_back(pending);
	}
	if (length == 0)
		fail(line, "missing ABI type after prefix modifier");
	AbiNode type;
	if (head == ABI_FACT_PREFIX_NAMED)
		type = named_type(line, text + head_length, length - head_length);
	else if (head == ABI_FACT_PREFIX_MEMBERPTR)
		type = compact_member_pointer(line, text + head_length, length - head_length);
	else
	{
		const FactWord word{ text, length };
		if (is_defined_as(word, ABI_FACT_DEFINITION_TYPE))
			type = reference(word, ABI_FACT_DEFINITION_TYPE, "type");
		else
		{
			const AbiBuiltinTypeKind builtin =
				(word_class != nullptr && modifier_count == 0 ? *word_class : word.classify()).builtin;
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
	const AbiNode owner = is_defined_as(owner_word, ABI_FACT_DEFINITION_TYPE)
		? reference(owner_word, ABI_FACT_DEFINITION_TYPE, "type")
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
		resolved.push_back(reference(words[i], ABI_FACT_DEFINITION_ARGUMENT, "template argument"));
	args.swap(resolved);
}

AbiNode CaseReader::read_argument_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const FactWord& form = words[0];
	const FactWord* operands = words + 1;
	const std::uint32_t n = count - 1;
	const std::uint8_t argument_form = form.classify().argument_form;
	switch (static_cast<AbiFactArgumentForm>(argument_form))
	{
	case ABI_FACT_ARGUMENT_TYPE:
		return type_spec(line, operands, n);
	case ABI_FACT_ARGUMENT_VALUE:
		if (n == 2)
			return typed_literal(line, compact_type(line, operands[0].text, operands[0].length), operands[1]);
		break;
	case ABI_FACT_ARGUMENT_DEPENDENT_VALUE:
		if (n == 3)
		{
			// (Operands in order: see the member-pointer type form.)
			const AbiNode type = compact_type(line, operands[0].text, operands[0].length);
			return graph_.typed_argument(
				type, typed_literal(line, compact_type(line, operands[1].text, operands[1].length), operands[2]));
		}
		break;
	case ABI_FACT_ARGUMENT_EXPRESSION:
		if (n == 1)
			return graph_.expression_argument(reference(operands[0], ABI_FACT_DEFINITION_EXPRESSION, "expression"));
		break;
	case ABI_FACT_ARGUMENT_TEMPLATE_ENTITY:
		if (n == 1)
			return named_type(line, operands[0].text, operands[0].length);
		break;
	case ABI_FACT_ARGUMENT_MEMBER_TEMPLATE_ENTITY:
		if (n == 3)
			return graph_.name(compact_type(line, operands[0].text, operands[0].length), string(operands[1]));
		break;
	case ABI_FACT_ARGUMENT_TEMPLATE_PARAM_TEMPLATE:
		if (n == 1)
			return graph_.template_param(index_value(line, operands[0]), false);
		break;
	case ABI_FACT_ARGUMENT_ENTITY_ADDRESS:
	case ABI_FACT_ARGUMENT_ENTITY_REFERENCE:
		if (n == 1)
			return graph_.entity_argument(reference(operands[0], ABI_FACT_DEFINITION_ENTITY, "entity"),
				argument_form == ABI_FACT_ARGUMENT_ENTITY_ADDRESS);
		break;
	case ABI_FACT_ARGUMENT_MEMBER_EXTERNAL_ADDRESS:
		return read_member_address(line, operands, n);
	case ABI_FACT_ARGUMENT_PACK:
		argument_list(operands, n, scratch_);
		return graph_.argument_pack(scratch_.data(), scratch_.size());
	default:
		break;
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
	AbiExpressionShape shape = ABI_EXPRESSION_SHAPE_UNARY;
	std::uint32_t operand_count = 0;
	const FactWord* first_operand = operands + 1;
	std::uint16_t code = 0;
	switch (static_cast<AbiFactExpressionForm>(form.classify().expression_form))
	{
	case ABI_FACT_EXPRESSION_TEMPLATE_PARAM:
		if (n != 1)
			break;
		return graph_.template_param(index_value(line, operands[0]), false);
	case ABI_FACT_EXPRESSION_FUNCTION_PARAM:
		if (n != 1)
			break;
		return graph_.function_param(index_value(line, operands[0]));
	case ABI_FACT_EXPRESSION_LITERAL:
		if (n != 1)
			break;
		return graph_.literal(graph_.builtin(ABI_BUILTIN_TYPE_INT), integer_value(line, operands[0]));
	case ABI_FACT_EXPRESSION_SIZEOF_TYPE:
		if (n != 1)
			break;
		return graph_.sizeof_type(compact_type(line, operands[0].text, operands[0].length));
	case ABI_FACT_EXPRESSION_PACK:
	{
		if (n != 1)
			break;
		const AbiNode operand = reference(operands[0], ABI_FACT_DEFINITION_EXPRESSION, "expression");
		return graph_.operation(abi_pack_operation_code('s', 'p'), &operand, 1);
	}
	case ABI_FACT_EXPRESSION_ENTITY_REFERENCE:
		if (n != 1)
			break;
		return graph_.entity_reference(reference(operands[0], ABI_FACT_DEFINITION_ENTITY, "entity"));
	case ABI_FACT_EXPRESSION_MEMBER:
	{
		if (n != 3)
			break;
		// (Operands in order: see the member-pointer type form.)
		const AbiNode scope = compact_type(line, operands[0].text, operands[0].length);
		const bool flag = flag_value(line, operands[1]);
		return graph_.scoped_member(scope, string(operands[2]), flag);
	}
	case ABI_FACT_EXPRESSION_SOURCE_NAME:
		// An unqualified dependent name (a call's callee): its <source-name>.
		if (n != 1)
			break;
		return graph_.unresolved_name(string(operands[0]), nullptr, 0);
	case ABI_FACT_EXPRESSION_TEMPLATE_ID:
		if (n < 1)
			break;
		argument_list(operands + 1, n - 1, scratch_);
		return graph_.unresolved_name(string(operands[0]), scratch_.data(), scratch_.size());
	case ABI_FACT_EXPRESSION_TYPE_TRAIT:
	{
		if (n < 2)
			break;
		std::vector<AbiNode> types;
		for (std::uint32_t i = 1; i < n; ++i)
			types.push_back(compact_type(line, operands[i].text, operands[i].length));
		return graph_.vendor_expression(string(operands[0]), types.data(), types.size());
	}
	case ABI_FACT_EXPRESSION_CAST:
	{
		if (n != 3)
			break;
		const std::uint16_t cast = abi_expression_operation_code(
			operands[0].text, operands[0].length, ABI_EXPRESSION_SHAPE_CAST);
		if (cast == 0)
			fail(line, "unknown cast operation '" + operands[0].str() + "'");
		// (Operands in order: see the member-pointer type form.)
		const AbiNode type = compact_type(line, operands[1].text, operands[1].length);
		return graph_.cast(cast, type, reference(operands[2], ABI_FACT_DEFINITION_EXPRESSION, "expression"));
	}
	case ABI_FACT_EXPRESSION_OBJECT_MEMBER:
	{
		if (n < 3)
			break;
		const std::uint16_t access = abi_expression_operation_code(
			operands[0].text, operands[0].length, ABI_EXPRESSION_SHAPE_MEMBER_ACCESS);
		if (access == 0)
			fail(line, "unknown member access operation '" + operands[0].str() + "'");
		const AbiNode object = reference(operands[1], ABI_FACT_DEFINITION_EXPRESSION, "expression");
		argument_list(operands + 3, n - 3, scratch_);
		return graph_.member_access(access, object, string(operands[2]), scratch_.data(), scratch_.size());
	}
	case ABI_FACT_EXPRESSION_UNARY:
		if (n != 2)
			break;
		operand_count = 1;
		break;
	case ABI_FACT_EXPRESSION_BINARY:
		if (n != 3)
			break;
		shape = ABI_EXPRESSION_SHAPE_BINARY;
		operand_count = 2;
		break;
	case ABI_FACT_EXPRESSION_CONDITIONAL:
		if (n != 3)
			break;
		code = abi_pack_operation_code('q', 'u');
		operand_count = 3;
		first_operand = operands;
		break;
	case ABI_FACT_EXPRESSION_CALL:
	{
		if (n < 1)
			break;
		std::vector<AbiNode> items;
		for (std::uint32_t i = 0; i < n; ++i)
			items.push_back(reference(operands[i], ABI_FACT_DEFINITION_EXPRESSION, "expression"));
		return graph_.call(items.data(), items.size());
	}
	default:
		break;
	}
	if (operand_count == 0)
		fail(line, "unknown or malformed dependent expression form '" + form.str() + "'");
	if (code == 0)
	{
		code = abi_expression_operation_code(operands[0].text, operands[0].length, shape);
		if (code == 0)
			fail(line, "unknown expression operation '" + operands[0].str() + "'");
	}
	AbiNode items[3];
	for (std::uint32_t i = 0; i < operand_count; ++i)
		items[i] = reference(first_operand[i], ABI_FACT_DEFINITION_EXPRESSION, "expression");
	return graph_.operation(code, items, operand_count);
}

AbiNode CaseReader::read_context_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const std::uint8_t context_form = words[0].classify().context_form;
	if (context_form == ABI_FACT_CONTEXT_RAW)
	{
		if (count != 2)
			fail(line, "raw context takes one fragment");
		return graph_.raw_function_scope(string(words[1]));
	}
	if (context_form == ABI_FACT_CONTEXT_FUNCTION && count >= 2)
		return graph_.function_scope(function_entity(line, static_cast<std::uint32_t>(words + 1 - line_words(line))));
	fail(line, "unknown context form '" + words[0].str() + "'");
}

AbiNode CaseReader::read_entity_definition(std::uint32_t line, const FactWord* words, std::uint32_t count)
{
	const FactWord& form = words[0];
	const std::uint8_t entity_form = form.classify().entity_form;
	if ((entity_form == ABI_FACT_ENTITY_VARIABLE || entity_form == ABI_FACT_ENTITY_INTERNAL_VARIABLE) && count == 2)
	{
		const AbiNode name = named_type(line, words[1].text, words[1].length);
		if (entity_form == ABI_FACT_ENTITY_VARIABLE)
			return name;
		const AbiNodeData data = graph_.node(name);
		return graph_.name(data.a, data.b, graph_.list(name), data.list_size, data.c, true);
	}
	if (entity_form == ABI_FACT_ENTITY_SYMBOL && count == 2)
		return graph_.external_symbol(string(words[1]));
	if (entity_form == ABI_FACT_ENTITY_FUNCTION && count >= 2)
		return function_entity(line, static_cast<std::uint32_t>(words + 1 - line_words(line)));
	fail(line, "unknown or malformed entity form '" + form.str() + "'");
}

// A local entity directly in a function: the discriminator word counts
// earlier entities of the same name (0 for the first, 1 for `_0`, ...).
AbiNode CaseReader::local_entity(
	std::uint32_t line, const FactWord& context, AbiNode unqualified, const FactWord& discriminator)
{
	const AbiNode scope = reference(context, ABI_FACT_DEFINITION_CONTEXT, "context");
	return graph_.name(scope, unqualified, nullptr, 0, index_value(line, discriminator));
}

// A closure type in a function: the number word is the Itanium closure
// number (`-` when omitted, for the first closure with its signature).
AbiNode CaseReader::closure_entity(
	std::uint32_t line, const FactWord& context, const FactWord& number, const FactWord* signature, std::uint32_t count)
{
	const AbiNode scope = reference(context, ABI_FACT_DEFINITION_CONTEXT, "context");
	const std::uint32_t closure_number = number.is(ABI_FACT_WORD_PLACEHOLDER) ? 0 : index_value(line, number) + 1;
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
		scope = i == 0 && names[i].is(ABI_FACT_WORD_STD) ? graph_.std_scope() : graph_.name(scope, string(names[i]));
	}
	return scope;
}

} // namespace abi_facts
