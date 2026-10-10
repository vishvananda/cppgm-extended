#pragma once

// The LowIR text parser behind read_program_files (lowir/text_reader.h).
// Internal to the reader: text_parser.cpp holds the top level (symbols,
// types, metadata, signatures, globals) and text_parser_body.cpp the
// function bodies.

#include <string>
#include <vector>

#include "lowir/builder.h"
#include "ir_symbol_model.h"
#include "lowir/program.h"
#include "lowir/text_lexer.h"
#include "lowir/validator.h"

namespace lowir {
namespace text {

// The class of a word the lexer interned; WordInfo::value is the word's
// enumerator in its class's lowir_model.h (or ir_symbol_model.h) enum.
enum class WordClass : uint8_t
{
	None,
	Keyword, // lowir_model::Keyword
	Type,
	Unary,
	Binary,
	Compare,
	Convert,
	NamedLiteral,
	Opcode, // an instruction head
	Fact,   // an InstructionFlag word: volatile, typed, nowrap, exact
	Storage // a GlobalStorageMode word after a global's name
};

// A NamedLiteral word's value holds the literal in its low bits and its
// floating suffix (0 none, 1 f32, 2 f80) from this bit up.
const unsigned kNamedSuffixShift = 4;

struct WordInfo
{
	WordClass word_class;
	uint8_t value;
};

enum class MetadataContext : uint8_t
{
	GlobalDeclaration,
	GlobalDefinition,
	FunctionDeclaration,
	FunctionDefinition,
	CallSignature
};

// A reader failure: the message with its source position.
struct Failure
{
	std::string message;
};

class Parser
{
public:
	explicit Parser(Program& program);

	// Parses one source buffer into the program; throws Failure.
	void parse(const char* begin, const char* end, const std::string& source);
	// After the last buffer: undefined symbols, legacy roles, validation;
	// throws Failure.
	void finish();

private:
	struct Position
	{
		uint32_t source;
		uint32_t line;
	};

	// Per-function name bindings, valid when `epoch` is the current body's.
	struct LocalName
	{
		uint32_t epoch;
		TempId temp;
		SlotId slot;
		BlockId block;
	};

	// Tokens (text_parser.cpp).
	const Token& token() const { return lexer_->current(); }
	void advance() { lexer_->advance(); }
	bool at(TokenKind kind) const { return token().kind == kind; }
	bool at_word(const char* spelling) const;
	// Object mode: an ObjectClassBits word (or number) at the token;
	// the program then states object facts.
	// Classes of a value of `type` (lowir.md "Object ABI Conventions").
	uint8_t parse_object_classes(Type type);
	WordInfo word(const Token& token) const;
	bool at_keyword(Keyword keyword) const;
	bool accept(TokenKind kind);
	bool accept_keyword(Keyword keyword);
	// Accepts the word `value` of `word_class` (an Opcode, an
	// InstructionFlag, ...).
	bool accept_word(WordClass word_class, unsigned value);
	void expect(TokenKind kind, const char* spelling);
	void expect_keyword(Keyword keyword);
	NameId expect_name(TokenKind kind, const char* what);
	std::string spelling(const Token& token) const;
	[[noreturn]] void fail(const std::string& message) const;
	[[noreturn]] void fail_at(Position position, const std::string& message) const;
	Position here() const;
	void define_words();

	// Top level (text_parser.cpp).
	void parse_top_level_item();
	SymbolId begin_symbol(NameId name, Position position);
	SymbolId reference_symbol(NameId name);
	void parse_global_declaration();
	void parse_global_definition();
	void parse_global_data(Global& global);
	void parse_function(bool definition);
	void parse_alias();
	void parse_storage_keyword(SymbolMetadata& metadata);
	Type parse_type();
	Type parse_object_type(const Token& token);
	uint64_t parse_positive(const char* text, uint32_t length) const;
	// `0` or a positive integer (a 0-based index).
	uint64_t parse_natural(const char* text, uint32_t length) const;
	void parse_span(uint64_t& bytes, unsigned& align_log2);
	Constant parse_constant(TypeKind context);
	int64_t parse_small_integer(const char* what);
	void parse_metadata(MetadataContext context, SymbolMetadata* symbol, BoundaryMetadata* boundary);
	void parse_metadata_item(
		MetadataContext context, SymbolMetadata* symbol, BoundaryMetadata* boundary, uint32_t& seen);
	void parse_symbol_value(MetadataKey key, MetadataContext context, SymbolMetadata& symbol);
	void parse_boundary_value(MetadataKey key, BoundaryMetadata& boundary);
	SignatureId parse_signature(MetadataContext context, SymbolMetadata* symbol);
	void parse_parameter(Signature& signature);
	void parse_parameter_metadata(Parameter& parameter);
	void check_signature(const Signature& signature, Position position);
	DebugLocation parse_debug_location();

	// Function bodies (text_parser_body.cpp).
	void parse_body(SymbolId function);
	void parse_slot(FunctionBuilder& builder);
	void parse_block_label(FunctionBuilder& builder);
	bool lacks_end(const FunctionBody& body) const;
	void finish_body(FunctionBuilder& builder, SymbolId function);
	LocalName& local(NameId name);
	TempId use_temp(FunctionBuilder& builder, NameId name);
	TempId define_temp(FunctionBuilder& builder, NameId name);
	SlotId use_slot(NameId name);
	BlockId use_block(FunctionBuilder& builder, NameId name);
	BlockId expect_block(FunctionBuilder& builder);
	Value parse_value(FunctionBuilder& builder, TypeKind context);
	Value parse_storage(FunctionBuilder& builder);
	Value parse_callee(FunctionBuilder& builder);
	bool at_value_start() const;
	MemoryOrder parse_order();
	void parse_instruction(FunctionBuilder& builder);
	void parse_rvalue(FunctionBuilder& builder, TempId result);
	bool parse_value_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst, Type& result);
	bool parse_memory_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst, Type& result);
	void parse_effect(FunctionBuilder& builder);
	bool parse_control_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst);
	void parse_eh_instruction(FunctionBuilder& builder, Opcode keyword, Instruction& inst);
	void parse_call(FunctionBuilder& builder, Instruction& inst);
	void parse_phi(FunctionBuilder& builder, Instruction& inst);
	void parse_switch(FunctionBuilder& builder, Instruction& inst);
	void parse_index_metadata(Instruction& inst);

	Program& program_;
	Lexer* lexer_;
	uint32_t source_;
	std::vector<std::string> sources_;
	std::vector<WordInfo> words_;
	// MetadataKey + 1 of an interned word that spells one (0: none); a key
	// may also be another class's word (`return`, `alias`, `object`).
	std::vector<uint8_t> metadata_keys_;
	bool find_key(const Token& token, MetadataKey* key) const;
	std::string scratch_;
	// First reference of each symbol, for undefined-symbol diagnostics.
	std::vector<Position> symbol_uses_;
	// Object alias spellings already defined, by name.
	std::vector<bool> alias_defined_;
	// Parameter names of the signature being read, by name.
	std::vector<uint32_t> parameter_marks_;
	uint32_t parameter_epoch_;
	// Where the metadata being parsed puts a boundary's result classes, and
	// the result's type (null outside one).
	uint8_t* result_classes_ = nullptr;
	const Type* result_type_ = nullptr;
	// Per-body state.
	std::vector<LocalName> locals_;
	uint32_t epoch_;
	std::vector<uint32_t> temp_uses_;  // first-use line of each temporary
	std::vector<uint32_t> block_uses_; // first-use line of each block
	std::vector<Value> operand_scratch_;
	bool terminated_;
	// Source position of each instruction, for validator diagnostics: the
	// lines of body `i`'s instructions start at body_lines_[i].
	std::vector<uint32_t> inst_lines_;
	struct BodyLines
	{
		SymbolId function;
		uint32_t source;
		uint32_t first;
	};
	std::vector<BodyLines> body_lines_;
};

} // namespace text
} // namespace lowir
