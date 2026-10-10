// The Itanium C++ ABI encoder (5.1). Productions follow the grammar of
// doc/itanium-mangling.txt; substitution candidates are recorded left to
// right, components before the structures that contain them (5.1.10), and
// compared by canonical graph identity.

#include "abi/itanium/abi_encoder.h"

#include "abi/itanium/abi_encoding_error.h"
#include "support/stack_budget.h"

namespace abi_mangle {

namespace {

[[noreturn]] void fail(const char* message)
{
	throw EncodingError(message);
}

} // namespace

AbiEncoder::AbiEncoder(const AbiGraph& graph, AbiMangleStats* stats)
	: graph_(graph), stats_(stats), out_(nullptr), stack_base_(0), stack_budget_(stack_budget_bytes())
{}

void AbiEncoder::put_number(unsigned long long value)
{
	char digits[24];
	int count = 0;
	do
	{
		digits[count++] = static_cast<char>('0' + value % 10);
		value /= 10;
	} while (value != 0);
	while (count > 0)
		out_->push_back(digits[--count]);
}

void AbiEncoder::put_signed(long long value)
{
	if (value < 0)
	{
		put('n');
		put_number(0ull - static_cast<unsigned long long>(value));
	}
	else
		put_number(static_cast<unsigned long long>(value));
}

void AbiEncoder::put_source_name(AbiNode string)
{
	const AbiText text = graph_.text(string);
	put_number(text.length);
	out_->append(text.text, text.length);
}

// <abi-tags> ::= <abi-tag>+, <abi-tag> ::= B <source-name>: the node's
// sorted tag list (5.1.2).
void AbiEncoder::put_tags(AbiNode tagged)
{
	// An inline namespace's tags are not encoded.
	if (graph_.kind(tagged) == ABI_NODE_NAME && (graph_.node(tagged).flags & ABI_FLAG_HIDDEN_TAGS) != 0)
		return;
	const AbiNode* tags = graph_.list(tagged);
	for (std::uint32_t i = 0; i < graph_.list_size(tagged); ++i)
	{
		put('B');
		put_source_name(tags[i]);
	}
}

// <discriminator> ::= _ <number> (number < 10) | __ <number> _ (5.1.7).
void AbiEncoder::put_discriminator(std::uint32_t discriminator_plus_one)
{
	if (discriminator_plus_one == 0)
		return;
	const std::uint32_t number = discriminator_plus_one - 1;
	put('_');
	if (number < 10)
		put_number(number);
	else
	{
		put('_');
		put_number(number);
		put('_');
	}
}

bool AbiEncoder::lookup(AbiNode node, std::uint32_t* index) const
{
	if (node >= slots_.size() || slots_[node] == 0)
		return false;
	const std::uint32_t candidate = slots_[node] - 1;
	if (candidate >= entries_.size() || entries_[candidate] != node)
		return false;
	*index = candidate;
	return true;
}

// <substitution> ::= S_ | S <seq-id> _ with a base-36 <seq-id> (5.1.2).
bool AbiEncoder::substitute(AbiNode node)
{
	if (stats_ != nullptr)
		++stats_->substitution_lookups;
	std::uint32_t index;
	if (!lookup(node, &index))
		return false;
	if (stats_ != nullptr)
		++stats_->substitution_hits;
	put('S');
	if (index != 0)
	{
		std::uint32_t number = index - 1;
		char digits[8];
		int count = 0;
		do
		{
			const std::uint32_t digit = number % 36;
			digits[count++] = static_cast<char>(digit < 10 ? '0' + digit : 'A' + digit - 10);
			number /= 36;
		} while (number != 0);
		while (count > 0)
			put(digits[--count]);
	}
	put('_');
	return true;
}

// No entity is added to the dictionary twice (5.1.10).
void AbiEncoder::record(AbiNode node)
{
	std::uint32_t existing;
	if (lookup(node, &existing))
		return;
	if (node >= slots_.size())
		slots_.resize(graph_.size(), 0);
	entries_.push_back(node);
	slots_[node] = static_cast<std::uint32_t>(entries_.size());
	if (stats_ != nullptr)
		++stats_->substitution_entries;
}

// Member function qualifiers are part of a <nested-name> only (5.1.5); a
// name without one cannot carry them.
void AbiEncoder::check_no_qualifiers(unsigned qualifiers) const
{
	if (qualifiers != 0)
		fail("function qualifiers need a member function name");
}

void AbiEncoder::check_stack() const
{
	char marker;
	if (stack_bytes_used(stack_base_, &marker) > stack_budget_)
		fail("ABI name nested too deeply");
}

// Types built by prefix codes (P, R, O, Dp, qualifiers, A, M) form chains
// that are walked iteratively: each link's code is emitted on the way in
// and the links are recorded innermost first on the way out, so chain
// length costs no stack.
void AbiEncoder::encode_type(AbiNode type)
{
	check_stack();
	const std::size_t chain_base = chain_.size();
	for (;;)
	{
		const AbiNodeData& data = graph_.node(type);
		if (data.kind == ABI_NODE_BUILTIN &&
			!abi_builtin_type_is_substitutable(static_cast<AbiBuiltinTypeKind>(data.code)))
		{
			put(abi_builtin_type_code(static_cast<AbiBuiltinTypeKind>(data.code)));
			break;
		}
		if (data.kind == ABI_NODE_STD_ABBREVIATION && data.list_size == 0)
		{
			encode_abbreviation(type);
			break;
		}
		if (substitute(type))
			break;
		AbiNode next = data.a;
		switch (data.kind)
		{
		case ABI_NODE_POINTER:
			put('P');
			break;
		case ABI_NODE_LVALUE_REFERENCE:
			put('R');
			break;
		case ABI_NODE_RVALUE_REFERENCE:
			put('O');
			break;
		case ABI_NODE_PACK_EXPANSION:
			put("Dp");
			break;
		case ABI_NODE_COMPLEX:
			put('C');
			break;
		case ABI_NODE_QUALIFIED:
			encode_qualifiers(data.code);
			break;
		case ABI_NODE_VENDOR_QUALIFIED:
			put('U');
			put_source_name(data.b);
			break;
		case ABI_NODE_ARRAY:
			put(data.flags == ABI_ARRAY_BOUND_VECTOR ? "Dv" : "A");
			if (data.flags == ABI_ARRAY_BOUND_VALUE || data.flags == ABI_ARRAY_BOUND_VECTOR)
				put_number((static_cast<unsigned long long>(data.c) << 32) | data.b);
			else if (data.flags == ABI_ARRAY_BOUND_EXPRESSION)
				encode_expression(data.b);
			put('_');
			break;
		case ABI_NODE_MEMBER_POINTER:
			put('M');
			encode_type(data.a);
			next = data.b;
			break;
		default:
			next = ABI_NONE;
			break;
		}
		if (next == ABI_NONE)
		{
			encode_type_node(type);
			break;
		}
		chain_.push_back(type);
		type = next;
	}
	while (chain_.size() > chain_base)
	{
		record(chain_.back());
		chain_.pop_back();
	}
}

// A type that is not a prefix-code chain link, after its substitution
// check.
void AbiEncoder::encode_type_node(AbiNode type)
{
	const AbiNodeData& data = graph_.node(type);
	switch (data.kind)
	{
	case ABI_NODE_BUILTIN:
		put(abi_builtin_type_code(static_cast<AbiBuiltinTypeKind>(data.code)));
		break;
	case ABI_NODE_FUNCTION_TYPE:
		encode_function_type(type);
		break;
	case ABI_NODE_TEMPLATE_PARAM:
		encode_prefix(type);
		return;
	case ABI_NODE_DECLTYPE:
		put((data.flags & ABI_FLAG_ID_EXPRESSION) != 0 ? "Dt" : "DT");
		encode_expression(data.a);
		put('E');
		break;
	case ABI_NODE_VENDOR_TYPE:
		put('u');
		put_source_name(data.a);
		if (data.list_size != 0)
			encode_template_args(type);
		break;
	case ABI_NODE_NAME:
	case ABI_NODE_TEMPLATE_ID:
		encode_name(type, 0);
		break;
	case ABI_NODE_STD_ABBREVIATION:
		encode_abbreviation(type);
		break;
	default:
		fail("ABI node is not a type");
	}
	record(type);
}

// <CV-qualifiers> ::= [r] [V] [K]: K nearest the base (5.1.5.1).
void AbiEncoder::encode_qualifiers(unsigned qualifiers)
{
	if ((qualifiers & ABI_QUALIFIER_RESTRICT) != 0)
		put('r');
	if ((qualifiers & ABI_QUALIFIER_VOLATILE) != 0)
		put('V');
	if ((qualifiers & ABI_QUALIFIER_CONST) != 0)
		put('K');
}

// <function-type> ::= [<CV-qualifiers>] F <bare-function-type> [<ref-qualifier>] E
void AbiEncoder::encode_function_type(AbiNode type)
{
	const AbiNodeData& data = graph_.node(type);
	encode_qualifiers(data.code);
	put('F');
	if (data.a != ABI_NONE)
		encode_type(data.a);
	for (std::uint32_t i = 0; i < data.list_size; ++i)
		encode_type(graph_.list(type)[i]);
	if (data.list_size == 0 && (data.flags & ABI_FLAG_VARIADIC) == 0)
		put('v');
	if ((data.flags & ABI_FLAG_VARIADIC) != 0)
		put('z');
	if ((data.code & ABI_QUALIFIER_LVALUE_REF) != 0)
		put('R');
	else if ((data.code & ABI_QUALIFIER_RVALUE_REF) != 0)
		put('O');
	put('E');
}

// <name> for entities and class types: <unscoped-name>, <unscoped-template-
// name> <template-args>, <nested-name> or <local-name> (5.1.5, 5.1.7).
// Neither the name of a function nor a class type as a whole is recorded
// here; the caller records a type.
void AbiEncoder::encode_name(AbiNode name, unsigned qualifiers)
{
	const AbiNodeData& data = graph_.node(name);
	if (data.kind == ABI_NODE_STD_ABBREVIATION)
	{
		encode_abbreviation(name);
		return;
	}
	const bool is_template = data.kind == ABI_NODE_TEMPLATE_ID;
	const AbiNode final_name = is_template ? data.a : name;
	const AbiNodeKind final_kind = graph_.kind(final_name);
	if (final_kind != ABI_NODE_NAME)
	{
		// A template-template parameter or a standard abbreviation applied to
		// template arguments.
		if (!is_template)
			fail("ABI name has no final name component");
		encode_prefix(final_name);
		encode_template_args(name);
		return;
	}
	const AbiNode parent = graph_.node(final_name).a;
	const AbiNodeKind parent_kind = graph_.kind(parent);
	if (parent == ABI_NONE || parent_kind == ABI_NODE_STD)
	{
		// <unscoped-name>, or <unscoped-template-name> (a candidate) and its
		// arguments.
		check_no_qualifiers(qualifiers);
		if (is_template)
		{
			encode_prefix(final_name);
			encode_template_args(name);
			return;
		}
		if (parent_kind == ABI_NODE_STD)
			put("St");
		encode_unqualified(final_name);
		return;
	}
	if (parent_kind == ABI_NODE_FUNCTION_SCOPE)
	{
		encode_local_name(name, final_name, qualifiers);
		return;
	}
	AbiNode root = parent;
	AbiNode local = final_name;
	while (graph_.kind(root) == ABI_NODE_NAME || graph_.kind(root) == ABI_NODE_TEMPLATE_ID)
	{
		const AbiNode component = graph_.kind(root) == ABI_NODE_TEMPLATE_ID ? graph_.node(root).a : root;
		if (graph_.kind(component) != ABI_NODE_NAME)
			break;
		local = component;
		root = graph_.node(component).a;
	}
	if (graph_.kind(root) == ABI_NODE_FUNCTION_SCOPE && !chain_has_known_prefix(name))
	{
		encode_local_name(name, local, qualifiers);
		return;
	}
	encode_nested_name(name, qualifiers);
}

// <nested-name> ::= N [<CV-qualifiers>] [<ref-qualifier>] <prefix> <unqualified-name> E
//               ::= N [<CV-qualifiers>] [<ref-qualifier>] <template-prefix> <template-args> E
// A name's `a` is its prefix: the parent scope, or the template name.
void AbiEncoder::encode_nested_name(AbiNode name, unsigned qualifiers)
{
	put('N');
	encode_qualifiers(qualifiers);
	if ((qualifiers & ABI_QUALIFIER_LVALUE_REF) != 0)
		put('R');
	else if ((qualifiers & ABI_QUALIFIER_RVALUE_REF) != 0)
		put('O');
	encode_prefix(graph_.node(name).a);
	if (graph_.kind(name) == ABI_NODE_TEMPLATE_ID)
		encode_template_args(name);
	else
		encode_unqualified(name);
	put('E');
}

// <local-name> ::= Z <function encoding> E <entity name> [<discriminator>].
// `local` is the entity declared directly in the function; the rest of the
// name is mangled as if the function were the global scope, and the
// discriminator of `local` follows the whole entity name (5.1.7).
void AbiEncoder::encode_local_name(AbiNode name, AbiNode local, unsigned qualifiers)
{
	const AbiNodeData& local_data = graph_.node(local);
	encode_context(local_data.a);
	const AbiNodeData& data = graph_.node(name);
	const bool is_template = data.kind == ABI_NODE_TEMPLATE_ID;
	const AbiNode final_name = is_template ? data.a : name;
	if (final_name == local)
	{
		check_no_qualifiers(qualifiers);
		if (is_template)
		{
			encode_prefix(final_name);
			encode_template_args(name);
		}
		else
			encode_unqualified(local);
	}
	else
		encode_nested_name(name, qualifiers);
	put_discriminator(local_data.c);
}

// Whether a prefix of a name rooted in a function scope is already in the
// dictionary; its substitution then stands for the local scope within an
// ordinary nested name.
bool AbiEncoder::chain_has_known_prefix(AbiNode name) const
{
	AbiNode prefix = graph_.node(name).a;
	while (prefix != ABI_NONE)
	{
		const AbiNodeKind kind = graph_.kind(prefix);
		if (kind != ABI_NODE_NAME && kind != ABI_NODE_TEMPLATE_ID)
			return false;
		std::uint32_t index;
		if (lookup(prefix, &index))
			return true;
		prefix = graph_.node(prefix).a;
	}
	return false;
}

// <prefix> and <template-prefix>: every component is a candidate except
// ::std and the standard abbreviations (5.1.10). The chain of name and
// template-id prefixes is walked iteratively from the largest prefix down
// to the first one already in the dictionary (or the root), then emitted
// and recorded innermost first, so name length costs no stack.
void AbiEncoder::encode_prefix(AbiNode scope)
{
	check_stack();
	const std::size_t chain_base = chain_.size();
	AbiNode node = scope;
	for (;;)
	{
		const AbiNodeKind kind = graph_.kind(node);
		std::uint32_t index;
		if ((kind != ABI_NODE_NAME && kind != ABI_NODE_TEMPLATE_ID) || lookup(node, &index))
			break;
		chain_.push_back(node);
		node = graph_.node(node).a;
		if (kind == ABI_NODE_NAME && (node == ABI_NONE || graph_.kind(node) == ABI_NODE_FUNCTION_SCOPE))
		{
			node = ABI_NONE;
			break;
		}
	}
	if (node != ABI_NONE)
		encode_prefix_root(node);
	while (chain_.size() > chain_base)
	{
		const AbiNode link = chain_.back();
		chain_.pop_back();
		if (graph_.kind(link) == ABI_NODE_NAME)
			encode_unqualified(link);
		else
			encode_template_args(link);
		record(link);
	}
}

// The start of a prefix: a component already in the dictionary, ::std, a
// standard abbreviation, a template parameter, a decltype or a vendor type.
void AbiEncoder::encode_prefix_root(AbiNode scope)
{
	const AbiNodeData& data = graph_.node(scope);
	switch (data.kind)
	{
	case ABI_NODE_NAME:
	case ABI_NODE_TEMPLATE_ID:
		substitute(scope);
		return;
	case ABI_NODE_STD:
		put("St");
		return;
	case ABI_NODE_STD_ABBREVIATION:
		if (data.list_size == 0)
			encode_abbreviation(scope);
		else if (!substitute(scope))
		{
			encode_abbreviation(scope);
			record(scope);
		}
		return;
	case ABI_NODE_TEMPLATE_PARAM:
		if (substitute(scope))
			return;
		encode_template_param(data);
		if ((data.flags & ABI_FLAG_SUBSTITUTABLE) != 0)
			record(scope);
		return;
	case ABI_NODE_DECLTYPE:
	case ABI_NODE_VENDOR_TYPE:
		// <prefix> ::= <decltype>; vendor builtin transforms such as
		// __remove_const<T> scope dependent members the same way.
		encode_type(scope);
		return;
	default:
		fail("invalid ABI name prefix");
	}
}

void AbiEncoder::encode_unqualified(AbiNode name)
{
	// An internal name's `L`.
	const AbiNodeData& data = graph_.node(name);
	if ((data.flags & ABI_FLAG_INTERNAL) != 0)
		put('L');
	const AbiNodeData& unqualified = graph_.node(data.b);
	switch (unqualified.kind)
	{
	case ABI_NODE_STRING:
		put_source_name(data.b);
		break;
	case ABI_NODE_OPERATOR_NAME:
		put(abi_terminal_code(static_cast<AbiTerminalKind>(unqualified.code)));
		break;
	case ABI_NODE_CTOR_DTOR_NAME:
		if (unqualified.a != ABI_NONE)
		{
			// An inheriting constructor: CI1 or CI2, then its base (5.1.4.3):
			// the class's name, its prefixes and template name candidates but
			// not the class type as a whole (as Clang mangles it).
			put(unqualified.code == ABI_TERMINAL_CONSTRUCTOR_BASE ? "CI2" : "CI1");
			if (graph_.kind(unqualified.a) == ABI_NODE_NAME || graph_.kind(unqualified.a) == ABI_NODE_TEMPLATE_ID)
				encode_name(unqualified.a, 0);
			else
				encode_type(unqualified.a);
			break;
		}
		put(abi_terminal_code(static_cast<AbiTerminalKind>(unqualified.code)));
		break;
	case ABI_NODE_CONVERSION_NAME:
		put("cv");
		encode_type(unqualified.a);
		break;
	case ABI_NODE_LITERAL_OPERATOR_NAME:
		put("li");
		put_source_name(unqualified.a);
		break;
	case ABI_NODE_CLOSURE_NAME:
		put("Ul");
		for (std::uint32_t i = 0; i < unqualified.list_size; ++i)
			encode_type(graph_.list(data.b)[i]);
		// <lambda-sig> ::= <parameter type>+, the ellipsis's z among them
		// (5.1.8; `[](...)` is Ulz E, as GCC and Clang name it).
		if ((unqualified.flags & ABI_FLAG_VARIADIC) != 0)
			put('z');
		else if (unqualified.list_size == 0)
			put('v');
		put('E');
		if (unqualified.a != 0)
			put_number(unqualified.a - 1);
		put('_');
		break;
	case ABI_NODE_UNNAMED_TYPE_NAME:
		put("Ut");
		if (unqualified.a != 0)
			put_number(unqualified.a - 1);
		put('_');
		break;
	case ABI_NODE_DECOMPOSITION_NAME:
		// A structured binding declaration's object: DC <source-name>+ E
		// (5.1.4), its names in order.
		put("DC");
		for (std::uint32_t i = 0; i < unqualified.list_size; ++i)
			put_source_name(graph_.list(data.b)[i]);
		put('E');
		break;
	default:
		fail("invalid ABI unqualified name");
	}
	if ((data.flags & ABI_FLAG_DATA_MEMBER) != 0)
		put('M');
	put_tags(name);
}

// A standard abbreviation (5.1.10). Its ABI tags follow it, and the tagged
// abbreviation is then a substitutable component (5.1.2); the caller
// records it.
void AbiEncoder::encode_abbreviation(AbiNode abbreviation)
{
	put(abi_standard_substitution_code(static_cast<AbiStandardSubstitutionKind>(graph_.node(abbreviation).code)));
	put_tags(abbreviation);
}

void AbiEncoder::encode_template_args(AbiNode template_id)
{
	const AbiNodeData& data = graph_.node(template_id);
	put('I');
	for (std::uint32_t i = 0; i < data.list_size; ++i)
		encode_template_arg(graph_.list(template_id)[i]);
	put('E');
}

// <template-arg> (5.1.5.10).
void AbiEncoder::encode_template_arg(AbiNode arg)
{
	const AbiNodeData& data = graph_.node(arg);
	switch (data.kind)
	{
	case ABI_NODE_LITERAL:
		encode_literal(arg);
		return;
	case ABI_NODE_DECLARED_ARGUMENT:
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_template_param_decl(graph_.list(arg)[i]);
		encode_template_arg(data.a);
		return;
	case ABI_NODE_EXPRESSION_ARGUMENT:
		put('X');
		encode_expression(data.a);
		put('E');
		return;
	case ABI_NODE_ARGUMENT_PACK:
		put('J');
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_template_arg(graph_.list(arg)[i]);
		put('E');
		return;
	case ABI_NODE_ENTITY_ARGUMENT:
		if ((data.flags & ABI_FLAG_ADDRESS_OF) != 0)
			put("Xad");
		put('L');
		encode_entity_reference(data.a);
		put('E');
		if ((data.flags & ABI_FLAG_ADDRESS_OF) != 0)
			put('E');
		return;
	default:
		encode_type(arg);
		return;
	}
}

// <template-param-decl> ::= Ty | Tn <type> | Tt <template-param-decl>* E
//                       ::= Tp <template-param-decl>
// (the current ABI's 5.1.5.10, cxx-abi issue 47). A non-type parameter's
// type is a <type> like any other: its components are substitution
// candidates in order.
void AbiEncoder::encode_template_param_decl(AbiNode declaration)
{
	check_stack();
	const AbiNodeData& data = graph_.node(declaration);
	if (data.kind != ABI_NODE_TEMPLATE_PARAM_DECL)
		fail("invalid ABI template parameter declaration");
	if ((data.flags & ABI_FLAG_PACK) != 0)
		put("Tp");
	switch (data.code)
	{
	case ABI_PARAM_DECL_TYPE:
		put("Ty");
		return;
	case ABI_PARAM_DECL_NON_TYPE:
		put("Tn");
		encode_type(data.a);
		return;
	case ABI_PARAM_DECL_TEMPLATE:
		put("Tt");
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_template_param_decl(graph_.list(declaration)[i]);
		put('E');
		return;
	default:
		fail("invalid ABI template parameter declaration");
	}
}

// <expr-primary> ::= L <type> <value number> E; unsigned builtins (and
// literals marked unsigned: an enumeration's of an unsigned underlying
// type) print their unsigned value, other types a signed value with an 'n'
// sign (5.1.6.1); the nullptr keyword is L <nullptr type> E (LDnE), a
// template argument that is a null pointer its type's 0 (LPi0E, LDn0E).
void AbiEncoder::encode_literal(AbiNode literal)
{
	const AbiNodeData& data = graph_.node(literal);
	put('L');
	encode_type(data.a);
	const AbiNodeData& type = graph_.node(data.a);
	const long long value = graph_.value(literal);
	const bool builtin = type.kind == ABI_NODE_BUILTIN;
	if ((data.flags & ABI_FLAG_FLOATING) != 0)
	{
		// <value float>: the IEEE bits in lowercase hexadecimal, as many
		// digits as the type's width (5.1.6.1).
		static const char kDigits[] = "0123456789abcdef";
		const int digits = builtin && type.code == ABI_BUILTIN_TYPE_FLOAT ? 8 : 16;
		for (int i = digits - 1; i >= 0; --i)
			put(kDigits[(static_cast<unsigned long long>(value) >> (4 * i)) & 0xf]);
	}
	else if ((data.flags & ABI_FLAG_NO_VALUE) != 0)
	{
	}
	else if ((data.flags & ABI_FLAG_UNSIGNED_VALUE) != 0 ||
		(builtin && abi_builtin_type_is_unsigned(static_cast<AbiBuiltinTypeKind>(type.code))))
		put_number(static_cast<unsigned long long>(value));
	else
		put_signed(value);
	put('E');
}

// <template-param> (5.1.8): T_, T <index-1> _, or with a level (see
// AbiGraph::template_param) TL <level-1> _ and the index as T's.
void AbiEncoder::encode_template_param(const AbiNodeData& param)
{
	put('T');
	if (param.b != 0)
	{
		put('L');
		put_number(param.b - 1);
		put('_');
	}
	if (param.a != 0)
		put_number(param.a - 1);
	put('_');
}

// <expression> (5.1.6).
void AbiEncoder::encode_expression(AbiNode expression)
{
	check_stack();
	const AbiNodeData& data = graph_.node(expression);
	const AbiNode* items = graph_.list(expression);
	switch (data.kind)
	{
	case ABI_NODE_TEMPLATE_PARAM:
		encode_template_param(data);
		return;
	case ABI_NODE_FUNCTION_PARAM:
		// fpT; fp _ (L 0) or fL <L-1> p _ with the position after the
		// first (5.1.6).
		if ((data.flags & ABI_FLAG_THIS) != 0)
		{
			put("fpT");
			return;
		}
		if (data.b == 0)
			put("fp");
		else
		{
			put("fL");
			put_number(data.b - 1);
			put('p');
		}
		if (data.a != 0)
			put_number(data.a - 1);
		put('_');
		return;
	case ABI_NODE_LITERAL:
		encode_literal(expression);
		return;
	case ABI_NODE_OPERATION:
		put(static_cast<char>(data.code >> 8));
		put(static_cast<char>(data.code & 0xff));
		if ((data.flags & ABI_FLAG_PREFIX) != 0)
			put('_');
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_expression(items[i]);
		return;
	case ABI_NODE_CALL:
		put((data.flags & ABI_FLAG_PARENTHESIZED) != 0 ? "cp" : "cl");
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_expression(graph_.list(expression)[i]);
		put('E');
		return;
	case ABI_NODE_CAST:
		put(static_cast<char>(data.code >> 8));
		put(static_cast<char>(data.code & 0xff));
		encode_type(data.a);
		encode_expression(data.b);
		return;
	case ABI_NODE_UNRESOLVED_NAME:
		// <base-unresolved-name>: a source name, or on <operator-name>.
		if (graph_.kind(data.a) == ABI_NODE_OPERATOR_NAME)
		{
			put("on");
			put(abi_terminal_code(static_cast<AbiTerminalKind>(graph_.node(data.a).code)));
		}
		else
			put_source_name(data.a);
		if (data.list_size != 0)
			encode_template_args(expression);
		return;
	case ABI_NODE_VENDOR_EXPRESSION:
		put('u');
		put_source_name(data.a);
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_template_arg(graph_.list(expression)[i]);
		put('E');
		return;
	case ABI_NODE_SIZEOF_TYPE:
		if (data.code == 0)
			put("st");
		else
		{
			put(static_cast<char>(data.code >> 8));
			put(static_cast<char>(data.code & 0xff));
		}
		encode_type(data.a);
		return;
	case ABI_NODE_SCOPED_MEMBER:
		if (graph_.kind(data.a) == ABI_NODE_UNRESOLVED_PREFIX)
			encode_unresolved_prefix(data.a);
		else
		{
			put("sr");
			encode_type(data.a);
			if ((data.flags & ABI_FLAG_CLOSE_QUALIFIER) != 0)
				put('E');
		}
		put_source_name(data.b);
		if (data.list_size != 0)
			encode_template_args(expression);
		return;
	case ABI_NODE_MEMBER_ACCESS:
		put(static_cast<char>(data.code >> 8));
		put(static_cast<char>(data.code & 0xff));
		encode_expression(data.a);
		put_source_name(data.b);
		if (data.list_size != 0)
			encode_template_args(expression);
		return;
	case ABI_NODE_ENTITY_REFERENCE:
		put('L');
		encode_entity_reference(data.a);
		put('E');
		return;
	case ABI_NODE_EXPRESSION_LIST:
		put(static_cast<char>(data.code >> 8));
		put(static_cast<char>(data.code & 0xff));
		if (data.a != ABI_NONE)
			encode_type(data.a);
		if (data.code == abi_pack_operation_code('c', 'v'))
			put('_');
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_expression(items[i]);
		put('E');
		return;
	case ABI_NODE_ALLOCATION:
		// [gs] nw|na <expression>* _ <type> E, or with <initializer> (its
		// own E ending it) in place of the E; [gs] dl|da <expression> (5.1.6).
		if ((data.flags & ABI_FLAG_GLOBAL) != 0)
			put("gs");
		put(static_cast<char>(data.code >> 8));
		put(static_cast<char>(data.code & 0xff));
		for (std::uint32_t i = 0; i < data.list_size; ++i)
			encode_expression(items[i]);
		if (data.a == ABI_NONE)
			return;
		put('_');
		encode_type(data.a);
		if (data.b != ABI_NONE)
			encode_expression(data.b);
		else
			put('E');
		return;
	default:
		fail("ABI node is not an expression");
	}
}

// <unresolved-name>'s qualifier (5.1.6): gs alone, sr <unresolved-type>,
// srN <unresolved-type> <unresolved-qualifier-level>+ E, or [gs] sr
// <unresolved-qualifier-level>+ E. The unresolved type is a <type> like any
// other; a qualifier level, a <simple-id>, is no substitution candidate
// (as Clang mangles it), though its template arguments' components are.
void AbiEncoder::encode_unresolved_prefix(AbiNode prefix)
{
	const AbiNodeData& data = graph_.node(prefix);
	const bool global = (data.flags & ABI_FLAG_GLOBAL) != 0;
	if (global)
		put("gs");
	if (data.list_size == 0 && data.a == ABI_NONE)
	{
		if (!global)
			fail("empty ABI unresolved qualifier");
		return;
	}
	put("sr");
	if (data.list_size == 0)
	{
		encode_type(data.a);
		return;
	}
	if (data.a != ABI_NONE)
	{
		put('N');
		encode_type(data.a);
	}
	for (std::uint32_t i = 0; i < data.list_size; ++i)
	{
		const AbiNode level = graph_.list(prefix)[i];
		if (graph_.kind(level) != ABI_NODE_UNRESOLVED_NAME || graph_.kind(graph_.node(level).a) != ABI_NODE_STRING)
			fail("invalid ABI unresolved qualifier level");
		put_source_name(graph_.node(level).a);
		if (graph_.list_size(level) != 0)
			encode_template_args(level);
	}
	put('E');
}

void AbiEncoder::encode_encoding(AbiNode entity)
{
	const AbiNodeData& data = graph_.node(entity);
	if (data.kind == ABI_NODE_FUNCTION_ENCODING)
	{
		encode_name(data.a, data.code);
		encode_bare_function_type(entity);
		return;
	}
	if (data.kind != ABI_NODE_NAME && data.kind != ABI_NODE_TEMPLATE_ID)
		fail("ABI entity is neither a function nor a data name");
	encode_name(entity, 0);
}

// Function templates encode their result type, except constructors,
// destructors and conversion functions (5.1.5.3); an empty parameter list
// is `v`.
void AbiEncoder::encode_bare_function_type(AbiNode encoding)
{
	const AbiNodeData& data = graph_.node(encoding);
	if (data.b != ABI_NONE && graph_.kind(data.a) == ABI_NODE_TEMPLATE_ID)
	{
		const AbiNode final_name = graph_.node(data.a).a;
		const AbiNodeKind terminal = graph_.kind(final_name) == ABI_NODE_NAME ? graph_.kind(graph_.node(final_name).b)
																			  : ABI_NODE_NONE;
		if (terminal != ABI_NODE_CTOR_DTOR_NAME && terminal != ABI_NODE_CONVERSION_NAME)
			encode_type(data.b);
	}
	for (std::uint32_t i = 0; i < data.list_size; ++i)
		encode_type(graph_.list(encoding)[i]);
	if (data.list_size == 0 && (data.flags & ABI_FLAG_VARIADIC) == 0)
		put('v');
	if ((data.flags & ABI_FLAG_VARIADIC) != 0)
		put('z');
}

// <expr-primary> ::= L _Z <encoding> E: the nested encoding is part of the
// one mangled name, so its components are substitution sources and
// candidates like any other (5.1.10). An entity known only by its external
// symbol is opaque text.
void AbiEncoder::encode_entity_reference(AbiNode entity)
{
	if (stats_ != nullptr)
		++stats_->entity_references;
	if (graph_.kind(entity) == ABI_NODE_EXTERNAL_SYMBOL)
	{
		const AbiText text = graph_.text(graph_.node(entity).a);
		out_->append(text.text, text.length);
		return;
	}
	put("_Z");
	encode_encoding(entity);
}

void AbiEncoder::encode_context(AbiNode function_scope)
{
	const AbiNodeData& data = graph_.node(function_scope);
	if ((data.flags & ABI_FLAG_RAW) != 0)
	{
		const AbiText text = graph_.text(data.a);
		out_->append(text.text, text.length);
		return;
	}
	put('Z');
	encode_encoding(data.a);
	put('E');
	// A default argument's closures: d [<parameter number>] _ (5.1.8).
	if (data.b != 0)
	{
		put('d');
		if (data.b > 1)
			put_number(data.b - 2);
		put('_');
	}
}

// <call-offset> ::= h <nv-offset> _ | v <v-offset> _ (5.1.4.2).
void AbiEncoder::encode_call_offset(const AbiCallOffset& offset)
{
	put(offset.is_virtual ? 'v' : 'h');
	put_signed(offset.offset);
	put('_');
	if (offset.is_virtual)
	{
		put_signed(offset.vcall_offset);
		put('_');
	}
}

void AbiEncoder::encode_special(const AbiTarget& target)
{
	switch (target.kind)
	{
	case ABI_TARGET_TYPEINFO:
		put("_ZTI");
		encode_type(target.node);
		return;
	case ABI_TARGET_TYPEINFO_NAME:
		put("_ZTS");
		encode_type(target.node);
		return;
	case ABI_TARGET_VTABLE:
		put("_ZTV");
		encode_type(target.node);
		return;
	case ABI_TARGET_VTT:
		put("_ZTT");
		encode_type(target.node);
		return;
	case ABI_TARGET_CONSTRUCTION_VTABLE:
		put("_ZTC");
		encode_type(target.node);
		put_signed(target.offset);
		put('_');
		encode_type(target.base);
		return;
	case ABI_TARGET_TLS_WRAPPER:
		put("_ZTW");
		encode_name(target.node, 0);
		return;
	case ABI_TARGET_TLS_INIT:
		put("_ZTH");
		encode_name(target.node, 0);
		return;
	case ABI_TARGET_GUARD_VARIABLE:
		// 5.1.4: _ZGV <object name>, a local static's <local-name> included.
		put("_ZGV");
		encode_name(target.node, 0);
		return;
	case ABI_TARGET_THUNK:
		put("_ZT");
		if (target.has_result_adjustment)
		{
			put('c');
			encode_call_offset(target.this_adjustment);
			encode_call_offset(target.result_adjustment);
		}
		else
			encode_call_offset(target.this_adjustment);
		encode_encoding(target.node);
		return;
	default:
		fail("unsupported ABI target kind");
	}
}

void AbiEncoder::encode(const AbiTarget& target, std::string& out)
{
	char marker;
	stack_base_ = reinterpret_cast<std::uintptr_t>(&marker);
	out_ = &out;
	entries_.clear();
	chain_.clear();
	const std::size_t start = out.size();
	switch (target.kind)
	{
	case ABI_TARGET_TYPE:
		encode_type(target.node);
		break;
	case ABI_TARGET_ENTITY:
		put("_Z");
		encode_encoding(target.node);
		break;
	case ABI_TARGET_C_NAME:
	{
		const AbiText text = graph_.text(target.node);
		out.append(text.text, text.length);
		break;
	}
	default:
		encode_special(target);
		break;
	}
	if (stats_ != nullptr)
		stats_->output_bytes += out.size() - start;
}

} // namespace abi_mangle
