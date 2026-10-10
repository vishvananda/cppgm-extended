#pragma once

// Internal to the PA9 fact adapter: one case's records, read into typed
// graph nodes and one target. The adapter is the only place fact words are
// interpreted; the graph and encoder never see fact text.

#include "abi/itanium/abi_encoder.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace abi_facts {

using abi_mangle::ABI_NONE;
using abi_mangle::AbiGraph;
using abi_mangle::AbiNode;
using abi_mangle::AbiTarget;

// A word of a fact line: a slice of the immutable file buffer.
struct FactWord
{
	const char* text;
	std::uint32_t length;

	// Inline, so a literal's length folds where the keyword is written.
	bool is(const char* literal) const
	{
		return std::strlen(literal) == length && std::memcmp(text, literal, length) == 0;
	}
	std::string str() const { return std::string(text, length); }
};

struct FactLine
{
	std::uint32_t number;
	std::uint32_t first_word;
	std::uint32_t word_count;
};

class FactError : public std::runtime_error
{
public:
	explicit FactError(const std::string& message) : std::runtime_error(message) {}
};

enum DefinitionKind : std::uint8_t
{
	DEFINITION_TYPE,
	DEFINITION_ARGUMENT,
	DEFINITION_EXPRESSION,
	DEFINITION_CONTEXT,
	DEFINITION_ENTITY
};

// The facts that make up a function name and signature, gathered from a
// function target line and the case's function records.
struct FunctionFacts
{
	bool structured = false;            // `function encoding`: named by name records
	AbiNode scope = ABI_NONE;           // parent of the final name component
	bool has_final_component = false;   // the last name component read so far
	AbiNode final_component = ABI_NONE; // source-name STRING or template-id
	bool final_is_placeholder = false;  // "-" or "operator": replaced by a terminal
	bool generated_lambda = false;      // a $_N local class: operator() by default
	abi_mangle::AbiTerminalKind terminal = abi_mangle::ABI_TERMINAL_NONE;
	AbiNode terminal_unqualified = ABI_NONE; // conversion, literal or source terminal
	std::vector<AbiNode> component_tags;     // tags of the last name component
	std::vector<AbiNode> tags;               // abi-tag: tags of the function's own name
	std::vector<AbiNode> template_args;
	unsigned qualifiers = 0;
	AbiNode result = ABI_NONE;
	std::vector<AbiNode> params;
	bool variadic = false;
	unsigned context_records = 0;
	unsigned terminal_records = 0;
};

class CaseReader
{
public:
	CaseReader(AbiGraph& graph, const std::vector<FactWord>& words, const std::vector<FactLine>& lines);

	// Reads the case's records into its one target.
	AbiTarget read_target();

	std::size_t definitions() const { return definitions_.size(); }

private:
	struct Definition
	{
		FactWord id;
		std::uint32_t line;
		DefinitionKind kind;
		std::uint8_t state; // 0 unresolved, 1 resolving, 2 resolved
		AbiNode node;
	};

	// abi_fact_reader.cpp: definitions, targets and function records.
	[[noreturn]] void fail(std::uint32_t line, const std::string& message) const;
	const FactWord* line_words(std::uint32_t line) const { return &words_[lines_[line].first_word]; }
	std::uint32_t line_size(std::uint32_t line) const { return lines_[line].word_count; }
	void index_definitions();
	int find_definition(const FactWord& id) const;
	AbiNode resolve(std::uint32_t definition);
	AbiNode reference(const FactWord& id, DefinitionKind kind, const char* what);
	bool is_defined_as(const FactWord& id, DefinitionKind kind) const;
	AbiTarget read_target_line(std::uint32_t line, FunctionFacts& function, bool& is_function);
	void read_function_target(std::uint32_t line, std::uint32_t first, FunctionFacts& function);
	void read_function_record(std::uint32_t line, FunctionFacts& function);
	void read_name_record(std::uint32_t line, FunctionFacts& function);
	void read_context_record(std::uint32_t line, FunctionFacts& function);
	void read_terminal_record(std::uint32_t line, FunctionFacts& function);
	void set_path(std::uint32_t line, const FactWord& path, FunctionFacts& function);
	AbiNode finish_function(std::uint32_t line, FunctionFacts& function);
	AbiNode function_entity(std::uint32_t line, std::uint32_t first);

	// abi_fact_definitions.cpp: definition bodies and type specifications.
	AbiNode read_definition(const Definition& definition);
	AbiNode read_argument_definition(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode read_member_address(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode read_expression_definition(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode read_context_definition(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode read_entity_definition(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode type_spec(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode type_form(std::uint32_t line, const FactWord* words, std::uint32_t count);
	AbiNode compact_type(std::uint32_t line, const char* text, std::uint32_t length);
	AbiNode compact_member_pointer(std::uint32_t line, const char* text, std::uint32_t length);
	AbiNode named_type(std::uint32_t line, const char* text, std::uint32_t length);
	void argument_list(const FactWord* words, std::uint32_t count, std::vector<AbiNode>& args);
	AbiNode typed_literal(std::uint32_t line, AbiNode type, const FactWord& word);
	AbiNode local_entity(
		std::uint32_t line, const FactWord& context, AbiNode unqualified, const FactWord& discriminator);
	AbiNode closure_entity(std::uint32_t line, const FactWord& context, const FactWord& number,
		const FactWord* signature, std::uint32_t count);
	AbiNode namespace_path(std::uint32_t line, const FactWord* names, std::uint32_t count);

	// Shared helpers.
	AbiNode string(const FactWord& word) { return graph_.string(word.text, word.length); }
	// A qualified name `a::b::c` as a chain of names under `root` (the
	// global scope, or ::std when the first component is `std`).
	AbiNode qualified_name(std::uint32_t line, const char* text, std::uint32_t length);
	std::uint32_t index_value(std::uint32_t line, const FactWord& word) const;
	long long integer_value(std::uint32_t line, const FactWord& word) const;
	bool flag_value(std::uint32_t line, const FactWord& word) const;

	AbiGraph& graph_;
	const std::vector<FactWord>& words_;
	const std::vector<FactLine>& lines_;
	std::vector<Definition> definitions_;
	// Definition indices + 1 by hash position of the id; at most half full.
	std::vector<std::uint32_t> definition_index_;
	std::vector<AbiNode> scratch_;
	std::uintptr_t stack_base_;
	std::uintptr_t stack_budget_;
};

bool is_generated_lambda_name(const FactWord& word);
// An empty parameter list may be spelled as a single void parameter.
void drop_void_parameter(const AbiGraph& graph, std::vector<AbiNode>& params);

} // namespace abi_facts
