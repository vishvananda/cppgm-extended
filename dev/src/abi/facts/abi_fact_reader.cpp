// The PA9 fact-file adapter: splits files into cases, resolves each case's
// definitions on demand, assembles its one target and hands it to the
// shared encoder. Definitions may be used before they appear; each is read
// once per case, and a definition that reaches itself is rejected.

#include "abi/facts/abi_fact_reader.h"

#include "abi/facts/abi_fact_case.h"
#include "support/file_handle.h"
#include "support/stack_budget.h"

#include <cstdio>
#include <cstring>
#include <sstream>

namespace abi_facts {

using namespace abi_mangle;

namespace {

std::uint32_t word_hash(const FactWord& word)
{
	std::uint32_t hash = 2166136261u;
	for (std::uint32_t i = 0; i < word.length; ++i)
	{
		hash ^= static_cast<unsigned char>(word.text[i]);
		hash *= 16777619u;
	}
	return hash;
}

bool same_word(const FactWord& left, const FactWord& right)
{
	return left.length == right.length && std::memcmp(left.text, right.text, left.length) == 0;
}

bool definition_kind(const FactWord& word, DefinitionKind* kind)
{
	static const struct
	{
		const char* word;
		DefinitionKind kind;
	} kForms[] = {
		{ "let-type", DEFINITION_TYPE },
		{ "let-arg", DEFINITION_ARGUMENT },
		{ "let-expr", DEFINITION_EXPRESSION },
		{ "let-context", DEFINITION_CONTEXT },
		{ "let-entity", DEFINITION_ENTITY },
	};
	for (const auto& form : kForms)
	{
		if (word.is(form.word))
		{
			*kind = form.kind;
			return true;
		}
	}
	return false;
}

bool is_target_word(const FactWord& word)
{
	static const char* const kTargets[] = {
		"type",
		"function",
		"c-function",
		"variable",
		"typeinfo",
		"typeinfo-name",
		"vtable",
		"vtt",
		"construction-vtable",
		"tls-wrapper",
		"thunk",
		"virtual-base-thunk",
	};
	for (const char* target : kTargets)
	{
		if (word.is(target))
			return true;
	}
	return false;
}

bool is_placeholder(const FactWord& word)
{
	return word.length == 0 || word.is("-") || word.is("operator");
}

bool is_member_scope(const AbiGraph& graph, AbiNode scope)
{
	return scope != ABI_NONE && graph.kind(scope) != ABI_NODE_STD;
}

} // namespace

CaseReader::CaseReader(AbiGraph& graph, const std::vector<FactWord>& words, const std::vector<FactLine>& lines)
	: graph_(graph), words_(words), lines_(lines), stack_base_(0), stack_budget_(stack_budget_bytes())
{}

void CaseReader::fail(std::uint32_t line, const std::string& message) const
{
	std::ostringstream text;
	text << "ABI fact line " << lines_[line].number << ": " << message;
	throw FactError(text.str());
}

void CaseReader::index_definitions()
{
	for (std::uint32_t line = 0; line < lines_.size(); ++line)
	{
		DefinitionKind kind;
		if (!definition_kind(line_words(line)[0], &kind))
			continue;
		if (line_size(line) < 3)
			fail(line, "incomplete definition");
		definitions_.push_back(Definition{ line_words(line)[1], line, kind, 0, ABI_NONE });
	}
	std::uint32_t size = 16;
	while (size < definitions_.size() * 2)
		size *= 2;
	definition_index_.assign(size, 0);
	for (std::uint32_t i = 0; i < definitions_.size(); ++i)
	{
		const FactWord& id = definitions_[i].id;
		std::uint32_t position = word_hash(id) & (size - 1);
		for (; definition_index_[position] != 0; position = (position + 1) & (size - 1))
		{
			if (same_word(definitions_[definition_index_[position] - 1].id, id))
				fail(definitions_[i].line, "duplicate ABI definition id '" + id.str() + "'");
		}
		definition_index_[position] = i + 1;
	}
}

int CaseReader::find_definition(const FactWord& id) const
{
	const std::uint32_t mask = static_cast<std::uint32_t>(definition_index_.size() - 1);
	for (std::uint32_t position = word_hash(id) & mask; definition_index_[position] != 0;
		position = (position + 1) & mask)
	{
		const std::uint32_t index = definition_index_[position] - 1;
		if (same_word(definitions_[index].id, id))
			return static_cast<int>(index);
	}
	return -1;
}

bool CaseReader::is_defined_as(const FactWord& id, DefinitionKind kind) const
{
	const int index = find_definition(id);
	return index >= 0 && definitions_[index].kind == kind;
}

AbiNode CaseReader::resolve(std::uint32_t index)
{
	Definition& definition = definitions_[index];
	if (definition.state == 2)
		return definition.node;
	if (definition.state == 1)
		fail(definition.line, "recursive ABI definition '" + definition.id.str() + "'");
	char marker;
	if (stack_bytes_used(stack_base_, &marker) > stack_budget_)
		fail(definition.line, "ABI definitions nested too deeply");
	definition.state = 1;
	const AbiNode node = read_definition(definition);
	definitions_[index].node = node;
	definitions_[index].state = 2;
	return node;
}

AbiNode CaseReader::reference(const FactWord& id, DefinitionKind kind, const char* what)
{
	const int index = find_definition(id);
	if (index < 0 || definitions_[index].kind != kind)
		throw FactError(std::string("unknown ABI ") + what + " '" + id.str() + "'");
	return resolve(static_cast<std::uint32_t>(index));
}

std::uint32_t CaseReader::index_value(std::uint32_t line, const FactWord& word) const
{
	if (word.length == 0 || word.length > 9)
		fail(line, "invalid ABI index '" + word.str() + "'");
	std::uint32_t value = 0;
	for (std::uint32_t i = 0; i < word.length; ++i)
	{
		if (word.text[i] < '0' || word.text[i] > '9')
			fail(line, "invalid ABI index '" + word.str() + "'");
		value = value * 10 + static_cast<std::uint32_t>(word.text[i] - '0');
	}
	return value;
}

// A decimal integer in [-2^63, 2^64 - 1]; values above 2^63 - 1 keep their
// unsigned bit pattern.
long long CaseReader::integer_value(std::uint32_t line, const FactWord& word) const
{
	const bool negative = word.length != 0 && word.text[0] == '-';
	const std::uint32_t start = negative ? 1 : 0;
	if (word.length == start)
		fail(line, "invalid ABI integer '" + word.str() + "'");
	unsigned long long magnitude = 0;
	for (std::uint32_t i = start; i < word.length; ++i)
	{
		const char c = word.text[i];
		if (c < '0' || c > '9')
			fail(line, "invalid ABI integer '" + word.str() + "'");
		const unsigned digit = static_cast<unsigned>(c - '0');
		if (magnitude > (~0ull - digit) / 10)
			fail(line, "ABI integer out of range '" + word.str() + "'");
		magnitude = magnitude * 10 + digit;
	}
	if (negative && magnitude > (1ull << 63))
		fail(line, "ABI integer out of range '" + word.str() + "'");
	return static_cast<long long>(negative ? 0ull - magnitude : magnitude);
}

bool CaseReader::flag_value(std::uint32_t line, const FactWord& word) const
{
	if (word.is("yes") || word.is("true") || word.is("1"))
		return true;
	if (word.is("no") || word.is("false") || word.is("0"))
		return false;
	fail(line, "expected boolean ABI word, got '" + word.str() + "'");
}

AbiNode CaseReader::qualified_name(std::uint32_t line, const char* text, std::uint32_t length)
{
	if (length >= 2 && text[0] == ':' && text[1] == ':')
	{
		text += 2;
		length -= 2;
	}
	if (length == 0)
		fail(line, "empty ABI qualified name");
	AbiNode scope = ABI_NONE;
	std::uint32_t start = 0;
	bool first = true;
	while (start <= length)
	{
		std::uint32_t end = start;
		while (end < length && !(text[end] == ':' && end + 1 < length && text[end + 1] == ':'))
			++end;
		if (end == start)
			fail(line, "empty component in ABI name '" + std::string(text, length) + "'");
		if (std::memchr(text + start, ':', end - start) != nullptr)
			fail(line, "stray ':' in ABI name '" + std::string(text, length) + "'");
		const bool last = end >= length;
		if (first && !last && end - start == 3 && std::memcmp(text + start, "std", 3) == 0)
			scope = graph_.std_scope();
		else
			scope = graph_.name(scope, graph_.string(text + start, end - start));
		first = false;
		start = end + 2;
	}
	return scope;
}

// Splits `path` into the scope of its last component and that component,
// which a terminal replaces when it is a placeholder.
void CaseReader::set_path(std::uint32_t line, const FactWord& path, FunctionFacts& function)
{
	const char* text = path.text;
	std::uint32_t length = path.length;
	if (length >= 2 && text[0] == ':' && text[1] == ':')
	{
		text += 2;
		length -= 2;
	}
	std::uint32_t split = length;
	for (std::uint32_t i = length; i >= 2; --i)
	{
		if (text[i - 1] == ':' && text[i - 2] == ':')
		{
			split = i - 2;
			break;
		}
	}
	const FactWord last = split == length ? FactWord{ text, length } : FactWord{ text + split + 2, length - split - 2 };
	if (last.length == 0)
		fail(line, "function path needs a name");
	if (std::memchr(last.text, ':', last.length) != nullptr)
		fail(line, "stray ':' in ABI name '" + path.str() + "'");
	function.scope = split == length ? ABI_NONE : qualified_name(line, text, split);
	if (split != length && split == 3 && std::memcmp(text, "std", 3) == 0)
		function.scope = graph_.std_scope();
	function.has_final_component = true;
	function.final_is_placeholder = is_placeholder(last);
	function.final_component = function.final_is_placeholder ? ABI_NONE : string(last);
}

// The function part of a target line from word `first`: `encoding`,
// `path <name> <operand>...`, `local ...`, `lambda ...`,
// `namespace-lambda ...`, or `<name> <parameter type>...`.
void CaseReader::read_function_target(std::uint32_t line, std::uint32_t first, FunctionFacts& function)
{
	const FactWord* words = line_words(line) + first;
	const std::uint32_t count = line_size(line) - first;
	if (count == 0)
		fail(line, "missing function target");
	const FactWord& form = words[0];
	if (form.is("encoding"))
	{
		if (count != 1)
			fail(line, "function encoding takes no operands");
		function.structured = true;
		return;
	}
	if (form.is("local") || form.is("lambda") || form.is("namespace-lambda"))
	{
		AbiTerminalKind terminal;
		const std::uint32_t terminal_word = form.is("namespace-lambda") ? 2 : 3;
		if (count <= terminal_word ||
			!abi_find_terminal_kind(words[terminal_word].text, words[terminal_word].length, &terminal))
			fail(line, form.str() + " function target has invalid operands");
		if (form.is("local"))
		{
			if (count != 5)
				fail(line, "local function target has invalid operands");
			function.scope = local_entity(line, words[1], string(words[2]), words[4]);
		}
		else if (form.is("lambda"))
			function.scope = closure_entity(line, words[1], words[2], words + 4, count - 4);
		else
			function.scope = graph_.name(namespace_path(line, words + 3, count - 3), string(words[1]));
		function.terminal = terminal;
		++function.terminal_records;
		++function.context_records;
		return;
	}
	const bool path = form.is("path");
	if (path && count < 2)
		fail(line, "function path needs a name");
	set_path(line, words[path ? 1 : 0], function);
	for (std::uint32_t i = path ? 2 : 1; i < count; ++i)
	{
		if (path && is_defined_as(words[i], DEFINITION_ARGUMENT))
			function.template_args.push_back(reference(words[i], DEFINITION_ARGUMENT, "template argument"));
		else
			function.params.push_back(compact_type(line, words[i].text, words[i].length));
	}
}

AbiNode CaseReader::function_entity(std::uint32_t line, std::uint32_t first)
{
	FunctionFacts function;
	if (line_words(line)[first].is("encoding"))
		fail(line, "a function entity needs a path");
	read_function_target(line, first, function);
	return finish_function(line, function);
}

void CaseReader::read_terminal_record(std::uint32_t line, FunctionFacts& function)
{
	const FactWord* words = line_words(line);
	const std::uint32_t count = line_size(line);
	if (++function.terminal_records > 1)
		fail(line, "multiple ABI function terminals");
	const FactWord& form = words[0];
	if (form.is("conversion-terminal"))
	{
		if (count < 2)
			fail(line, "conversion-terminal needs a type");
		function.terminal_unqualified = graph_.conversion_name(type_spec(line, words + 1, count - 1));
		return;
	}
	if (count < 2)
		fail(line, form.str() + " has invalid operands");
	if (form.is("terminal-source"))
	{
		if (count != 2)
			fail(line, "terminal-source takes one name");
		function.terminal_unqualified = string(words[1]);
		return;
	}
	if (form.is("operator-terminal") && words[1].is("literal"))
	{
		if (count != 3)
			fail(line, "operator-terminal has invalid operands");
		function.terminal_unqualified = graph_.literal_operator_name(string(words[2]));
		return;
	}
	// An inheriting constructor: CI1 or CI2, then the class that declared
	// the inherited constructor (5.1.4.3).
	const bool inherited_complete = words[1].is("constructor-inherited-complete");
	if (form.is("terminal") && (inherited_complete || words[1].is("constructor-inherited-base")))
	{
		if (count != 3)
			fail(line, "an inherited constructor terminal needs its base class type");
		function.terminal_unqualified = graph_.ctor_dtor_name(
			inherited_complete ? ABI_TERMINAL_CONSTRUCTOR_COMPLETE : ABI_TERMINAL_CONSTRUCTOR_BASE,
			compact_type(line, words[2].text, words[2].length));
		return;
	}
	AbiTerminalKind kind;
	if (count != 2 || !abi_find_terminal_kind(words[1].text, words[1].length, &kind))
		fail(line, "unknown ABI terminal '" + words[1].str() + "'");
	if (form.is("operator-terminal") && abi_terminal_is_constructor_or_destructor(kind))
		fail(line, "operator-terminal names an operator");
	function.terminal = kind;
}

// name-std, name-source and name-template: the components of a structured
// function name, in order. A component is attached to the scope when the
// next one arrives; the last one is the function's own name.
void CaseReader::read_name_record(std::uint32_t line, FunctionFacts& function)
{
	const FactWord* words = line_words(line);
	const std::uint32_t count = line_size(line);
	if (!function.structured)
		fail(line, "structured ABI name records require a function encoding target");
	if (function.context_records != 0)
		fail(line, "name components cannot follow a local context");
	if (words[0].is("name-std"))
	{
		if (count != 1)
			fail(line, "name-std takes no operands");
		if (function.has_final_component || function.scope != ABI_NONE)
			fail(line, "name-std must precede the name components");
		function.scope = graph_.std_scope();
		return;
	}
	if (words[0].is("component-abi-tag"))
	{
		if (count != 2)
			fail(line, "component-abi-tag takes one tag");
		if (!function.has_final_component || function.final_is_placeholder)
			fail(line, "component ABI tag has no preceding name component");
		function.component_tags.push_back(string(words[1]));
		return;
	}
	// Attach the previous component.
	if (function.has_final_component)
	{
		if (function.final_is_placeholder)
			fail(line, "a placeholder name component must be last");
		AbiNode component = function.final_component;
		if (graph_.kind(component) == ABI_NODE_STRING)
			component = graph_.name(
				function.scope, component, function.component_tags.data(), function.component_tags.size());
		else if (!function.component_tags.empty())
			component = graph_.with_tags(component, function.component_tags.data(), function.component_tags.size());
		function.scope = component;
		function.component_tags.clear();
		function.has_final_component = false;
	}
	if (words[0].is("name-source"))
	{
		if (count != 2 && count != 3)
			fail(line, "name-source has invalid operands");
		function.has_final_component = true;
		function.final_is_placeholder = is_placeholder(words[1]);
		function.final_component = function.final_is_placeholder ? ABI_NONE : string(words[1]);
		return;
	}
	// name-template <name> <template-key> <id-key> <std-code or -> <includes-args> <arg>...
	if (count < 7)
		fail(line, "name-template has invalid operands");
	argument_list(words + 6, count - 6, scratch_);
	AbiNode component;
	if (words[4].is("-"))
		component = graph_.template_id(graph_.name(function.scope, string(words[1])), scratch_.data(), scratch_.size());
	else
	{
		const AbiStandardSubstitutionKind kind = abi_standard_substitution_kind(words[4].text, words[4].length);
		if (kind == ABI_STANDARD_SUBSTITUTION_NONE)
			fail(line, "unknown standard substitution '" + words[4].str() + "'");
		if (function.scope != ABI_NONE && graph_.kind(function.scope) != ABI_NODE_STD)
			fail(line, "a standard substitution must be the first name component");
		const AbiNode abbreviation = graph_.std_abbreviation(kind);
		component = flag_value(line, words[5]) ? abbreviation
											   : graph_.template_id(abbreviation, scratch_.data(), scratch_.size());
		function.scope = ABI_NONE;
	}
	function.has_final_component = true;
	function.final_is_placeholder = false;
	function.final_component = component;
}

void CaseReader::read_context_record(std::uint32_t line, FunctionFacts& function)
{
	const FactWord* words = line_words(line);
	const std::uint32_t count = line_size(line);
	if (!function.structured)
		fail(line, "structured ABI name records require a function encoding target");
	if (++function.context_records > 1)
		fail(line, "multiple ABI function context kinds");
	if (function.has_final_component || function.scope != ABI_NONE)
		fail(line, "a local context cannot follow name components");
	if (words[0].is("local-context"))
	{
		if (count != 4)
			fail(line, "local-context has invalid operands");
		function.scope = local_entity(line, words[1], string(words[2]), words[3]);
		function.generated_lambda = is_generated_lambda_name(words[2]);
	}
	else if (words[0].is("lambda-context"))
	{
		if (count < 3)
			fail(line, "lambda-context has invalid operands");
		function.scope = closure_entity(line, words[1], words[2], words + 3, count - 3);
	}
	else
	{
		if (count < 2)
			fail(line, "namespace-lambda-context needs a source name");
		function.scope = graph_.name(namespace_path(line, words + 2, count - 2), string(words[1]));
	}
}

void CaseReader::read_function_record(std::uint32_t line, FunctionFacts& function)
{
	const FactWord* words = line_words(line);
	const std::uint32_t count = line_size(line);
	const FactWord& form = words[0];
	if (form.is("param"))
		function.params.push_back(type_spec(line, words + 1, count - 1));
	else if (form.is("result"))
	{
		if (function.result != ABI_NONE)
			fail(line, "multiple ABI function result types");
		function.result = type_spec(line, words + 1, count - 1);
	}
	else if (form.is("variadic"))
	{
		if (count != 1)
			fail(line, "variadic takes no operands");
		if (function.variadic)
			fail(line, "multiple ABI variadic markers");
		function.variadic = true;
	}
	else if (form.is("abi-tag"))
	{
		if (count != 2)
			fail(line, "abi-tag takes one tag");
		function.tags.push_back(string(words[1]));
	}
	else if (form.is("qualifier") || form.is("function-qualifier"))
	{
		if (count < 2)
			fail(line, "function qualifier needs at least one qualifier");
		for (std::uint32_t i = 1; i < count; ++i)
		{
			if (words[i].is("const"))
				function.qualifiers |= ABI_QUALIFIER_CONST;
			else if (words[i].is("volatile"))
				function.qualifiers |= ABI_QUALIFIER_VOLATILE;
			else if (words[i].is("restrict"))
				function.qualifiers |= ABI_QUALIFIER_RESTRICT;
			else if (words[i].is("lvalue-ref"))
				function.qualifiers |= ABI_QUALIFIER_LVALUE_REF;
			else if (words[i].is("rvalue-ref"))
				function.qualifiers |= ABI_QUALIFIER_RVALUE_REF;
			else
				fail(line, "unknown function qualifier '" + words[i].str() + "'");
		}
		if ((function.qualifiers & ABI_QUALIFIER_LVALUE_REF) != 0 &&
			(function.qualifiers & ABI_QUALIFIER_RVALUE_REF) != 0)
			fail(line, "conflicting ABI function reference qualifiers");
	}
	else if (form.is("function-template-arg"))
	{
		if (count != 2)
			fail(line, "function-template-arg takes one id");
		function.template_args.push_back(reference(words[1], DEFINITION_ARGUMENT, "template argument"));
	}
	else if (form.is("function-template-prefix"))
	{
		// The template prefix is a substitution candidate by the grammar;
		// the key names it for readers and is not an identity.
		if (count != 2)
			fail(line, "function-template-prefix takes one key");
	}
	else if (form.is("operator-terminal") || form.is("conversion-terminal") || form.is("terminal") ||
		form.is("terminal-source"))
		read_terminal_record(line, function);
	else if (form.is("name-source") || form.is("name-std") || form.is("name-template") || form.is("component-abi-tag"))
		read_name_record(line, function);
	else if (form.is("local-context") || form.is("lambda-context") || form.is("namespace-lambda-context"))
		read_context_record(line, function);
	else
		fail(line, "unknown ABI function record '" + form.str() + "'");
}

// Builds the function encoding: the final name component (a terminal, or
// the last name component), its ABI tags and template arguments, and the
// signature.
AbiNode CaseReader::finish_function(std::uint32_t line, FunctionFacts& function)
{
	drop_void_parameter(graph_, function.params);
	if (function.terminal == ABI_TERMINAL_NONE && function.terminal_unqualified == ABI_NONE &&
		function.generated_lambda && !function.has_final_component)
		function.terminal = ABI_TERMINAL_CALL;
	AbiNode scope = function.scope;
	AbiNode final_component = function.final_component;
	std::vector<AbiNode> component_tags = function.component_tags;
	if (function.terminal != ABI_TERMINAL_NONE || function.terminal_unqualified != ABI_NONE)
	{
		if (function.has_final_component && !function.final_is_placeholder)
		{
			if (graph_.kind(final_component) == ABI_NODE_STRING)
				final_component = graph_.name(scope, final_component, component_tags.data(), component_tags.size());
			scope = final_component;
			component_tags.clear();
		}
		if (function.terminal != ABI_TERMINAL_NONE)
		{
			const std::size_t operands = function.params.size() + (is_member_scope(graph_, scope) ? 1 : 0);
			const AbiTerminalKind kind = abi_terminal_for_operands(function.terminal, operands);
			final_component = abi_terminal_is_constructor_or_destructor(kind) ? graph_.ctor_dtor_name(kind)
																			  : graph_.operator_name(kind);
		}
		else
			final_component = function.terminal_unqualified;
	}
	else if (!function.has_final_component || function.final_is_placeholder)
		fail(line, "missing ABI function terminal");
	std::vector<AbiNode> args;
	AbiNode name;
	if (graph_.kind(final_component) == ABI_NODE_TEMPLATE_ID)
	{
		const AbiNodeData data = graph_.node(final_component);
		if (graph_.kind(data.a) != ABI_NODE_NAME)
			fail(line, "invalid final structured ABI name component");
		args.assign(graph_.list(final_component), graph_.list(final_component) + data.list_size);
		const AbiNodeData templ = graph_.node(data.a);
		component_tags.assign(graph_.list(data.a), graph_.list(data.a) + templ.list_size);
		scope = templ.a;
		final_component = templ.b;
	}
	else if (graph_.kind(final_component) == ABI_NODE_STD_ABBREVIATION)
		fail(line, "invalid final structured ABI name component");
	component_tags.insert(component_tags.end(), function.tags.begin(), function.tags.end());
	name = graph_.name(scope, final_component, component_tags.data(), component_tags.size());
	args.insert(args.end(), function.template_args.begin(), function.template_args.end());
	if (!args.empty())
		name = graph_.template_id(name, args.data(), args.size());
	return graph_.function_encoding(
		name, function.result, function.params.data(), function.params.size(), function.variadic, function.qualifiers);
}

AbiTarget CaseReader::read_target_line(std::uint32_t line, FunctionFacts& function, bool& is_function)
{
	const FactWord* words = line_words(line);
	const std::uint32_t count = line_size(line);
	const FactWord& form = words[0];
	AbiTarget target;
	is_function = false;
	if (form.is("type"))
	{
		target.kind = ABI_TARGET_TYPE;
		target.node = type_spec(line, words + 1, count - 1);
	}
	else if (form.is("typeinfo") || form.is("typeinfo-name") || form.is("vtable") || form.is("vtt"))
	{
		target.kind = form.is("typeinfo") ? ABI_TARGET_TYPEINFO
			: form.is("typeinfo-name")    ? ABI_TARGET_TYPEINFO_NAME
			: form.is("vtable")           ? ABI_TARGET_VTABLE
										  : ABI_TARGET_VTT;
		target.node = type_spec(line, words + 1, count - 1);
	}
	else if (form.is("variable") || form.is("tls-wrapper"))
	{
		const bool wrapper = form.is("tls-wrapper");
		if (count != (wrapper ? 3u : 2u) || (wrapper && !words[1].is("variable")))
			fail(line, wrapper ? "TLS wrapper must name a variable" : "variable target takes one name");
		target.kind = wrapper ? ABI_TARGET_TLS_WRAPPER : ABI_TARGET_ENTITY;
		target.node = named_type(line, words[count - 1].text, words[count - 1].length);
		// 5.1.2: an ordinary variable of the global namespace is not
		// mangled (its plain source name, without ABI tags).
		const AbiNodeData data = graph_.node(target.node);
		if (!wrapper && data.kind == ABI_NODE_NAME && data.a == ABI_NONE && data.list_size == 0 &&
			(data.flags & ABI_FLAG_INTERNAL) == 0 && graph_.kind(data.b) == ABI_NODE_STRING)
		{
			target.kind = ABI_TARGET_C_NAME;
			target.node = data.b;
		}
	}
	else if (form.is("c-function"))
	{
		if (count < 2)
			fail(line, "empty C-linkage ABI function name");
		const FactWord& path = words[1];
		std::uint32_t start = path.length;
		while (start > 0 && path.text[start - 1] != ':')
			--start;
		qualified_name(line, path.text, path.length);
		for (std::uint32_t i = 2; i < count; ++i)
			compact_type(line, words[i].text, words[i].length);
		target.kind = ABI_TARGET_C_NAME;
		target.node = graph_.string(path.text + start, path.length - start);
	}
	else if (form.is("construction-vtable"))
	{
		if (count != 4)
			fail(line, "construction vtable needs derived, offset, and base");
		target.kind = ABI_TARGET_CONSTRUCTION_VTABLE;
		target.node = compact_type(line, words[1].text, words[1].length);
		target.offset = integer_value(line, words[2]);
		target.base = compact_type(line, words[3].text, words[3].length);
	}
	else if (form.is("thunk") || form.is("virtual-base-thunk"))
	{
		// thunk <this> [<result> | virtual-result <fixed> <vcall>] function ...
		// virtual-base-thunk <vcall> function ...
		std::uint32_t next = 2;
		target.kind = ABI_TARGET_THUNK;
		if (count < 3)
			fail(line, "thunk is missing function target");
		if (form.is("virtual-base-thunk"))
		{
			target.this_adjustment.is_virtual = true;
			target.this_adjustment.vcall_offset = integer_value(line, words[1]);
		}
		else
		{
			target.this_adjustment.offset = integer_value(line, words[1]);
			if (words[2].is("virtual-result"))
			{
				if (count < 6)
					fail(line, "virtual result thunk has invalid offsets");
				target.has_result_adjustment = true;
				target.result_adjustment.is_virtual = true;
				target.result_adjustment.offset = integer_value(line, words[3]);
				target.result_adjustment.vcall_offset = integer_value(line, words[4]);
				next = 5;
			}
			else if (!words[2].is("function"))
			{
				target.has_result_adjustment = true;
				target.result_adjustment.offset = integer_value(line, words[2]);
				next = 3;
			}
		}
		if (next >= count || !words[next].is("function"))
			fail(line, "thunk target has invalid operands");
		read_function_target(line, next + 1, function);
		is_function = true;
	}
	else
	{
		target.kind = ABI_TARGET_ENTITY;
		read_function_target(line, 1, function);
		is_function = true;
	}
	return target;
}

// Definitions are read in file order first, so a definition written before
// its uses (the usual layout) finds its operands already built and reading
// never nests deeper than the forward references require.
AbiTarget CaseReader::read_target()
{
	char marker;
	stack_base_ = reinterpret_cast<std::uintptr_t>(&marker);
	index_definitions();
	for (std::uint32_t i = 0; i < definitions_.size(); ++i)
		resolve(i);
	int target_line = -1;
	for (std::uint32_t line = 0; line < lines_.size(); ++line)
	{
		if (!is_target_word(line_words(line)[0]))
			continue;
		if (target_line >= 0)
			fail(line, "ABI case has more than one target");
		target_line = static_cast<int>(line);
	}
	if (target_line < 0)
		throw FactError("ABI case has no target");
	FunctionFacts function;
	bool is_function = false;
	AbiTarget target = read_target_line(static_cast<std::uint32_t>(target_line), function, is_function);
	const bool c_function = line_words(static_cast<std::uint32_t>(target_line))[0].is("c-function");
	for (std::uint32_t line = 0; line < lines_.size(); ++line)
	{
		DefinitionKind kind;
		if (static_cast<int>(line) == target_line || definition_kind(line_words(line)[0], &kind))
			continue;
		if (!is_function)
			fail(line,
				c_function ? "C-linkage function cannot have ABI function records"
						   : "function records attached to a non-function ABI target");
		read_function_record(line, function);
	}
	if (is_function)
		target.node = finish_function(static_cast<std::uint32_t>(target_line), function);
	return target;
}

namespace {

bool is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}

const std::size_t kReadChunk = 64 * 1024;
const std::size_t kOutputFlush = 64 * 1024;

// Streams fact files case by case. A `case <label>` line starts a case;
// records before the first one form an implicit case. Blank lines and
// lines whose first word begins with `#` are ignored. Only the current
// case's text is held (its definitions may be used before they appear);
// each case's names are written as soon as it is encoded, and the graph is
// reset between cases, which share no facts.
class FileReader
{
public:
	FileReader(AbiGraph& graph, AbiEncoder& encoder, std::FILE* out, AbiMangleStats* stats)
		: graph_(graph), encoder_(encoder), out_(out), stats_(stats)
	{}

	void read(const std::string& path)
	{
		FileHandle input(std::fopen(path.c_str(), "rb"));
		if (input.get() == nullptr)
			throw FactError("unable to open ABI fact file '" + path + "'");
		std::string partial;
		std::vector<char> chunk(kReadChunk);
		cases_ = 0;
		open_case_ = false;
		line_number_ = 0;
		for (;;)
		{
			const std::size_t count = std::fread(chunk.data(), 1, chunk.size(), input.get());
			if (count == 0)
				break;
			if (stats_ != nullptr)
				stats_->source_bytes += count;
			const char* cursor = chunk.data();
			const char* end = cursor + count;
			while (cursor < end)
			{
				const char* newline = static_cast<const char*>(std::memchr(cursor, '\n', end - cursor));
				if (newline == nullptr)
				{
					partial.append(cursor, end);
					break;
				}
				if (partial.empty())
					line(cursor, newline);
				else
				{
					partial.append(cursor, newline);
					line(partial.data(), partial.data() + partial.size());
					partial.clear();
				}
				cursor = newline + 1;
			}
		}
		if (std::ferror(input.get()))
			throw FactError("unable to read ABI fact file '" + path + "'");
		if (!partial.empty())
			line(partial.data(), partial.data() + partial.size());
		if (open_case_)
			finish_case();
		input.close();
		if (stats_ != nullptr)
			++stats_->source_files;
		if (cases_ == 0)
			throw FactError("ABI fact file contains no cases");
	}

	void flush()
	{
		if (!output_.empty() && std::fwrite(output_.data(), 1, output_.size(), out_) != output_.size())
			throw FactError("unable to write mangled ABI output");
		output_.clear();
	}

private:
	void line(const char* begin, const char* end)
	{
		++line_number_;
		const char* first = begin;
		while (first < end && is_space(*first))
			++first;
		if (first == end || *first == '#')
			return;
		const char* first_end = first;
		while (first_end < end && !is_space(*first_end))
			++first_end;
		const FactWord keyword{ first, static_cast<std::uint32_t>(first_end - first) };
		if (keyword.is("case"))
		{
			const char* label = first_end;
			while (label < end && is_space(*label))
				++label;
			const char* label_end = label;
			while (label_end < end && !is_space(*label_end))
				++label_end;
			const char* rest = label_end;
			while (rest < end && is_space(*rest))
				++rest;
			if (label == label_end || rest != end)
				throw FactError("ABI fact line " + std::to_string(line_number_) + ": case takes one label");
			if (open_case_)
				finish_case();
			open_case_ = true;
			return;
		}
		case_text_.append(first, end);
		case_text_.push_back('\n');
		line_numbers_.push_back(line_number_);
		open_case_ = true;
	}

	void split_case()
	{
		words_.clear();
		lines_.clear();
		const char* cursor = case_text_.data();
		for (std::uint32_t number : line_numbers_)
		{
			FactLine line{ number, static_cast<std::uint32_t>(words_.size()), 0 };
			for (;;)
			{
				while (is_space(*cursor))
					++cursor;
				if (*cursor == '\n')
					break;
				const char* start = cursor;
				while (*cursor != '\n' && !is_space(*cursor))
					++cursor;
				words_.push_back(FactWord{ start, static_cast<std::uint32_t>(cursor - start) });
			}
			++cursor;
			line.word_count = static_cast<std::uint32_t>(words_.size()) - line.first_word;
			lines_.push_back(line);
		}
	}

	void finish_case()
	{
		split_case();
		CaseReader reader(graph_, words_, lines_);
		const AbiTarget target = reader.read_target();
		encoder_.encode(target, output_);
		output_.push_back('\n');
		++cases_;
		if (stats_ != nullptr)
		{
			++stats_->cases;
			++stats_->targets;
			stats_->records += lines_.size();
			stats_->definitions += reader.definitions();
			telemetry::note_peak(stats_->peak_graph_nodes, graph_.size());
			telemetry::note_peak(stats_->peak_graph_bytes, graph_.bytes_reserved());
		}
		graph_.reset();
		case_text_.clear();
		line_numbers_.clear();
		if (output_.size() >= kOutputFlush)
			flush();
	}

	AbiGraph& graph_;
	AbiEncoder& encoder_;
	std::FILE* out_;
	AbiMangleStats* stats_;
	std::string case_text_;
	std::vector<std::uint32_t> line_numbers_;
	std::vector<FactWord> words_;
	std::vector<FactLine> lines_;
	std::string output_;
	std::size_t cases_ = 0;
	bool open_case_ = false;
	std::uint32_t line_number_ = 0;
};

} // namespace

void mangle_fact_files(const std::vector<std::string>& paths, std::FILE* out, AbiMangleStats* stats)
{
	AbiGraph graph;
	AbiEncoder encoder(graph, stats);
	FileReader reader(graph, encoder, out, stats);
	for (const std::string& path : paths)
		reader.read(path);
	reader.flush();
}

} // namespace abi_facts
