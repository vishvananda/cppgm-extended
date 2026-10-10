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
using abi_mangle::AbiFactDefinition;
using abi_mangle::AbiGraph;
using abi_mangle::AbiNode;
using abi_mangle::AbiTarget;

// The fact-file vocabulary families a word belongs to (abi_mangle.h's
// fact-word enums, and the builtin and terminal words of its Itanium
// tables), found with one lookup (classify_fact_word) so that a reader
// classifies each form word once and switches on the result. A word can
// belong to several families: `const` is a qualifier and a type form,
// `function` a target, a context form and an entity form. Each fact field
// holds the word's enumerator in that family's enum, or kNotFactWord.
const std::uint8_t kNotFactWord = 0xff;

struct FactWordClass
{
	std::uint8_t definition;                // AbiFactDefinition
	std::uint8_t target;                    // AbiFactTarget
	std::uint8_t function_form;             // AbiFactFunctionForm
	std::uint8_t record;                    // AbiFactRecord
	std::uint8_t qualifier;                 // AbiFactQualifier
	std::uint8_t special_terminal;          // AbiFactSpecialTerminal
	std::uint8_t type_form;                 // AbiFactTypeForm
	std::uint8_t prefix;                    // AbiFactTypePrefix, spelled with its colon
	std::uint8_t argument_form;             // AbiFactArgumentForm
	std::uint8_t expression_form;           // AbiFactExpressionForm
	std::uint8_t context_form;              // AbiFactContextForm
	std::uint8_t entity_form;               // AbiFactEntityForm
	std::uint8_t word;                      // AbiFactWord
	abi_mangle::AbiBuiltinTypeKind builtin; // ABI_BUILTIN_TYPE_NONE if none
	abi_mangle::AbiTerminalKind terminal;   // ABI_TERMINAL_NONE if none
};

// The fact words of abi_mangle.h's tables: each family's enumerators by
// their spellings, and the builtin and terminal words. abi_fact_reader.cpp
// fills the one instance before main.
class FactVocabulary
{
public:
	FactVocabulary();

	// The class of `length` bytes at `text`; every field "none" for a word
	// of no family.
	const FactWordClass& find(const char* text, std::uint32_t length) const
	{
		// Most words a reader classifies are names and ids, and no fact word
		// has their first byte and length.
		if (length - 1u >= kLongestWord ||
			((lengths_by_first_[static_cast<unsigned char>(text[0])] >> length) & 1) == 0)
			return none_;
		for (std::uint32_t slot = hash(text, length);; slot = (slot + 1) & (kSlots - 1))
		{
			const Entry& entry = slots_[slot];
			if (entry.length == length && same_bytes(entry.text, text, length))
				return entry.word_class;
			if (entry.length == 0)
				return none_;
		}
	}

	// The compact prefix (an AbiFactTypePrefix, spelled with its colon)
	// that `length` > 0 bytes at `text` start with, or kNotFactWord. The
	// prefixes are kept by first byte; no first byte starts more than a
	// few.
	std::uint8_t prefix_at(const char* text, std::uint32_t length) const
	{
		for (std::uint8_t at = first_prefix_[static_cast<unsigned char>(text[0])]; at != 0; at = prefixes_[at - 1].next)
		{
			const Prefix& prefix = prefixes_[at - 1];
			if (prefix.length <= length && same_bytes(prefix.text, text, prefix.length))
				return prefix.value;
		}
		return kNotFactWord;
	}
	// The length of `value`'s spelling, its colon included.
	std::uint32_t prefix_length(std::uint8_t value) const { return prefix_lengths_[value]; }

private:
	// At most one entry per spelling of the families.
	static const std::uint32_t kEntries = abi_mangle::ABI_FACT_DEFINITION_COUNT + abi_mangle::ABI_FACT_TARGET_COUNT +
		abi_mangle::ABI_FACT_FUNCTION_COUNT + abi_mangle::ABI_FACT_RECORD_COUNT + abi_mangle::ABI_FACT_QUALIFIER_COUNT +
		abi_mangle::ABI_FACT_TERMINAL_COUNT + abi_mangle::ABI_FACT_TYPE_COUNT + abi_mangle::ABI_FACT_PREFIX_COUNT +
		abi_mangle::ABI_FACT_ARGUMENT_COUNT + abi_mangle::ABI_FACT_EXPRESSION_COUNT +
		abi_mangle::ABI_FACT_CONTEXT_COUNT + abi_mangle::ABI_FACT_ENTITY_COUNT + abi_mangle::ABI_FACT_WORD_COUNT +
		abi_mangle::ABI_BUILTIN_TYPE_COUNT + abi_mangle::ABI_TERMINAL_COUNT;
	// A power of two at least twice kEntries, so probe chains stay short.
	static const std::uint32_t kSlots = 512;
	static const std::uint32_t kSlotShift = 23; // 32 - log2(kSlots)
	static_assert(kSlots >= 2 * kEntries && kSlots == 1u << (32 - kSlotShift), "fact word table size");
	// Words are shorter than this (lengths_by_first_ has a bit per length).
	static const std::uint32_t kLongestWord = 63;

	struct Entry
	{
		const char* text;
		std::uint32_t length;
		FactWordClass word_class;
	};
	struct Prefix
	{
		const char* text;
		std::uint32_t length;
		std::uint8_t value; // AbiFactTypePrefix
		std::uint8_t next;  // the next prefix with this first byte + 1, or 0
	};

	// Open addressing on the word's length and its first, middle and last
	// bytes (one multiplicative hash of the four): a lookup reads no more of
	// a word than its one comparison.
	static std::uint32_t hash(const char* text, std::uint32_t length)
	{
		const std::uint32_t key = static_cast<unsigned char>(text[0]) |
			static_cast<std::uint32_t>(static_cast<unsigned char>(text[length >> 1])) << 8 |
			static_cast<std::uint32_t>(static_cast<unsigned char>(text[length - 1])) << 16 | length << 24;
		return (key * 0x9e3779b1u) >> kSlotShift;
	}
	// Compares in overlapping 8- or 4-byte loads that stay inside both
	// words.
	static bool same_bytes(const char* left, const char* right, std::uint32_t length)
	{
		if (length >= 8)
		{
			for (std::uint32_t at = 0; at + 8 < length; at += 8)
			{
				if (load<std::uint64_t>(left + at) != load<std::uint64_t>(right + at))
					return false;
			}
			return load<std::uint64_t>(left + length - 8) == load<std::uint64_t>(right + length - 8);
		}
		if (length >= 4)
			return load<std::uint32_t>(left) == load<std::uint32_t>(right) &&
				load<std::uint32_t>(left + length - 4) == load<std::uint32_t>(right + length - 4);
		return left[0] == right[0] && left[length >> 1] == right[length >> 1] && left[length - 1] == right[length - 1];
	}
	template<typename T>
	static T load(const char* bytes)
	{
		T value;
		std::memcpy(&value, bytes, sizeof value);
		return value;
	}

	template<typename E>
	void add_family(unsigned count, std::uint8_t FactWordClass::* field);
	FactWordClass& add(const char* text);

	// Entries by hash position; an empty one has length 0.
	Entry slots_[kSlots];
	// Bit n of lengths_by_first_[c]: a fact word of n bytes starts with c.
	std::uint64_t lengths_by_first_[256];
	Prefix prefixes_[abi_mangle::ABI_FACT_PREFIX_COUNT];
	std::uint32_t prefix_lengths_[abi_mangle::ABI_FACT_PREFIX_COUNT];
	// The first prefix with each first byte + 1, or 0.
	std::uint8_t first_prefix_[256];
	FactWordClass none_;
};

extern const FactVocabulary kFactVocabulary;

inline const FactWordClass& classify_fact_word(const char* text, std::uint32_t length)
{
	return kFactVocabulary.find(text, length);
}

// A word of a fact line: a slice of the immutable file buffer.
struct FactWord
{
	const char* text;
	std::uint32_t length;

	// Whether this is the scaffold's spelling of `value`, a fact word of one
	// of abi_mangle.h's enums. Inline, so the spelling's length folds where
	// the word is named.
	template<typename E>
	bool is(E value) const
	{
		const char* spelled = abi_mangle::spelling(value);
		return std::strlen(spelled) == length && std::memcmp(text, spelled, length) == 0;
	}
	const FactWordClass& classify() const { return classify_fact_word(text, length); }
	std::string str() const { return std::string(text, length); }
};

// A line of a case. Its words are [first_word, the next line's first_word),
// so a case's lines end with a sentinel that holds only first_word.
struct FactLine
{
	std::uint32_t number;
	std::uint32_t first_word;
	// The class of the line's first word in the families that say what a
	// line is (kNotFactWord where it is none of their words).
	std::uint8_t definition; // AbiFactDefinition
	std::uint8_t target;     // AbiFactTarget
	std::uint8_t record;     // AbiFactRecord
};

class FactError : public std::runtime_error
{
public:
	explicit FactError(const std::string& message) : std::runtime_error(message) {}
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

	// Reads the case's records into its one target. `lines` ends with the
	// sentinel line.
	AbiTarget read_target();

	std::size_t definitions() const { return definitions_.size(); }

private:
	struct Definition
	{
		FactWord id;
		std::uint32_t line;
		AbiFactDefinition kind;
		std::uint8_t state; // 0 unresolved, 1 resolving, 2 resolved
		AbiNode node;
	};

	// abi_fact_reader.cpp: definitions, targets and function records.
	[[noreturn]] void fail(std::uint32_t line, const std::string& message) const;
	std::uint32_t line_count() const { return static_cast<std::uint32_t>(lines_.size() - 1); }
	const FactWord* line_words(std::uint32_t line) const { return &words_[lines_[line].first_word]; }
	std::uint32_t line_size(std::uint32_t line) const { return lines_[line + 1].first_word - lines_[line].first_word; }
	void index_definitions();
	int find_definition(const FactWord& id) const;
	AbiNode resolve(std::uint32_t definition);
	AbiNode reference(const FactWord& id, AbiFactDefinition kind, const char* what);
	bool is_defined_as(const FactWord& id, AbiFactDefinition kind) const;
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
	// `form` is the class of words[0].
	AbiNode type_form(std::uint32_t line, const FactWord* words, std::uint32_t count, const FactWordClass& form);
	// `word_class`, if not null, is the class of the whole word.
	AbiNode compact_type(
		std::uint32_t line, const char* text, std::uint32_t length, const FactWordClass* word_class = nullptr);
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
