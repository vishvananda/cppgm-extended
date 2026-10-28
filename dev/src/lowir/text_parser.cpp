#include "lowir/text_parser.h"

#include <cmath>
#include <cstring>

#include "lowir/literals.h"

namespace lowir {
namespace text {

namespace {

bool is_section_char(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '.';
}

bool is_power_of_two(uint64_t value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

unsigned log2_of(uint64_t value)
{
	unsigned log = 0;
	while ((static_cast<uint64_t>(1) << log) < value)
		++log;
	return log;
}

} // namespace

Parser::Parser(Program& program)
	: program_(program), lexer_(nullptr), source_(0), parameter_epoch_(0), epoch_(0), terminated_(false)
{
	define_words();
}

void Parser::define_words()
{
	auto define = [this](const char* text, WordClass word_class, unsigned value) {
		const NameId name = program_.names.intern(text, static_cast<uint32_t>(std::strlen(text)));
		if (name >= words_.size())
			words_.resize(name + 1, WordInfo{ WordClass::None, 0 });
		words_[name] = WordInfo{ word_class, static_cast<uint8_t>(value) };
	};
	for (unsigned i = 0; i < kKeywordCount; ++i)
		define(lowir_model::spelling(static_cast<Keyword>(i)), WordClass::Keyword, i);
	for (unsigned i = 0; i < kMetadataKeyCount; ++i)
	{
		const char* text = ir_model::spelling(static_cast<MetadataKey>(i));
		const NameId name = program_.names.intern(text, static_cast<uint32_t>(std::strlen(text)));
		if (name >= metadata_keys_.size())
			metadata_keys_.resize(name + 1, 0);
		metadata_keys_[name] = static_cast<uint8_t>(i + 1);
	}
	for (unsigned i = 0; i < kOpcodeCount; ++i)
		define(lowir_model::spelling(static_cast<Opcode>(i)), WordClass::Opcode, i);
	for (unsigned i = 0; i < kUnaryOpCount; ++i)
		define(lowir_model::spelling(static_cast<UnaryOp>(i)), WordClass::Unary, i);
	for (unsigned i = 0; i < kBinaryOpCount; ++i)
		define(lowir_model::spelling(static_cast<BinaryOp>(i)), WordClass::Binary, i);
	for (unsigned i = 0; i < kComparePredicateCount; ++i)
		define(lowir_model::spelling(static_cast<ComparePredicate>(i)), WordClass::Compare, i);
	for (unsigned i = 0; i < kConvertOpCount; ++i)
		define(lowir_model::spelling(static_cast<ConvertOp>(i)), WordClass::Convert, i);
	for (const InstructionFlag fact : { IF_VOLATILE, IF_TYPED, IF_NO_WRAP, IF_EXACT })
		define(lowir_model::spelling(fact), WordClass::Fact, fact);
	// The compatibility spellings `readonly` and `thread_local` after a
	// global's name (lowir.md "Structured Global Data Initializers").
	for (unsigned i = ir_model::GSM_READONLY; i < ir_model::kGlobalStorageModeCount; ++i)
		define(ir_model::spelling(static_cast<ir_model::GlobalStorageMode>(i)), WordClass::Storage, i);
	for (unsigned kind = 0; kind <= static_cast<unsigned>(TypeKind::Ptr); ++kind)
		define(lowir_model::spelling(static_cast<TypeKind>(kind)), WordClass::Type, kind);
	// The floating words also take a floating suffix (`INFINITYL`, `nanf`);
	// the value records the suffix above the literal (kNamedSuffixShift).
	const char* const suffixes[] = { "", "f", "F", "l", "L" };
	const unsigned suffix_kinds[] = { 0, 1, 1, 2, 2 };
	for (unsigned literal = 0; literal < kNamedLiteralCount; ++literal)
	{
		const NamedLiteral named = static_cast<NamedLiteral>(literal);
		for (const char* word : { lowir_model::spelling(named), alternate_spelling(named) })
		{
			for (unsigned suffix = 0; word != nullptr && suffix < 5; ++suffix)
			{
				if (suffix != 0 && named == NamedLiteral::Nullptr)
					break;
				const std::string text = std::string(word) + suffixes[suffix];
				define(text.c_str(), WordClass::NamedLiteral, literal | suffix_kinds[suffix] << kNamedSuffixShift);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Tokens

WordInfo Parser::word(const Token& token) const
{
	if (token.kind != TokenKind::Word || token.name >= words_.size())
		return WordInfo{ WordClass::None, 0 };
	return words_[token.name];
}

bool Parser::at_keyword(Keyword keyword) const
{
	const WordInfo info = word(token());
	return info.word_class == WordClass::Keyword && info.value == static_cast<uint8_t>(keyword);
}

bool Parser::find_key(const Token& token, MetadataKey* key) const
{
	if (token.kind != TokenKind::Word || token.name >= metadata_keys_.size() || metadata_keys_[token.name] == 0)
		return false;
	*key = static_cast<MetadataKey>(metadata_keys_[token.name] - 1);
	return true;
}

bool Parser::accept_word(WordClass word_class, unsigned value)
{
	const WordInfo info = word(token());
	if (info.word_class != word_class || info.value != value)
		return false;
	advance();
	return true;
}

bool Parser::accept(TokenKind kind)
{
	if (!at(kind))
		return false;
	advance();
	return true;
}

bool Parser::accept_keyword(Keyword keyword)
{
	if (!at_keyword(keyword))
		return false;
	advance();
	return true;
}

std::string Parser::spelling(const Token& token) const
{
	if (token.kind == TokenKind::End)
		return "end of input";
	// A name token's text omits its sigil.
	const bool named = token.kind == TokenKind::Temp || token.kind == TokenKind::Slot ||
		token.kind == TokenKind::Global || token.kind == TokenKind::Block;
	return named ? std::string(token.text - 1, token.length + 1) : std::string(token.text, token.length);
}

void Parser::expect(TokenKind kind, const char* text)
{
	if (!accept(kind))
		fail(std::string("expected '") + text + "', got '" + spelling(token()) + "'");
}

void Parser::expect_keyword(Keyword keyword)
{
	if (!accept_keyword(keyword))
		fail(std::string("expected '") + lowir_model::spelling(keyword) + "', got '" + spelling(token()) + "'");
}

NameId Parser::expect_name(TokenKind kind, const char* what)
{
	if (!at(kind))
		fail(std::string("expected ") + what + ", got '" + spelling(token()) + "'");
	const NameId name = token().name;
	advance();
	return name;
}

Parser::Position Parser::here() const
{
	return Position{ source_, token().line };
}

void Parser::fail(const std::string& message) const
{
	if (at(TokenKind::Invalid))
		fail_at(here(), std::string(token().problem));
	fail_at(here(), message);
}

void Parser::fail_at(Position position, const std::string& message) const
{
	throw Failure{ message + " at " + sources_[position.source] + ":" + std::to_string(position.line) };
}

// ---------------------------------------------------------------------------
// Top level

void Parser::parse(const char* begin, const char* end, const std::string& source)
{
	debug_base_ = program_.debug ? static_cast<uint32_t>(program_.debug->types.size() - 1) : 0;
	scope_base_ = program_.debug ? static_cast<uint32_t>(program_.debug->scopes.size() - 1) : 0;
	source_ = static_cast<uint32_t>(sources_.size());
	sources_.push_back(source);
	Lexer lexer(program_.names, begin, end);
	lexer_ = &lexer;
	while (!at(TokenKind::End))
		parse_top_level_item();
	lexer_ = nullptr;
}

void Parser::parse_top_level_item()
{
	if (accept_keyword(Keyword::Declare))
	{
		if (accept_keyword(Keyword::Global))
			parse_global_declaration();
		else if (accept_keyword(Keyword::Function))
			parse_function(false);
		else
			fail("expected declaration kind");
	}
	else if (accept_keyword(Keyword::Global))
		parse_global_definition();
	else if (accept_keyword(Keyword::Function))
		parse_function(true);
	else if (accept_keyword(Keyword::Alias))
		parse_alias();
	else if (accept_keyword(Keyword::DebugType))
		parse_debug_type();
	else if (accept_keyword(Keyword::DebugScope))
		parse_debug_scope();
	else if (accept_keyword(Keyword::DebugLocal))
		parse_debug_local();
	else
		fail("expected top-level LowIR item");
}

SymbolId Parser::begin_symbol(NameId name, Position position)
{
	const SymbolId symbol = reference_symbol(name);
	if (program_.symbols[symbol].kind != SymbolKind::Unresolved)
		fail_at(position, std::string("duplicate top-level symbol: ") + program_.spelling(name));
	return symbol;
}

SymbolId Parser::reference_symbol(NameId name)
{
	const SymbolId symbol = program_.symbol_for(name);
	if (symbol >= symbol_uses_.size())
		symbol_uses_.resize(symbol + 1, here());
	return symbol;
}

void Parser::parse_storage_keyword(SymbolMetadata& metadata)
{
	// The compatibility spellings `readonly` and `thread_local` after the
	// name (lowir.md "Structured Global Data Initializers").
	const WordInfo info = word(token());
	if (info.word_class == WordClass::Storage)
	{
		metadata.storage = static_cast<ir_model::GlobalStorageMode>(info.value);
		advance();
	}
}

void Parser::parse_global_declaration()
{
	const Position position = here();
	const SymbolId symbol = begin_symbol(expect_name(TokenKind::Global, "global name"), position);
	SymbolMetadata metadata;
	parse_storage_keyword(metadata);
	Global global = Global();
	global.init = GlobalInit::None;
	global.type = Type::scalar(TypeKind::Void);
	global.address = kNoId;
	if (accept(TokenKind::Colon))
	{
		global.typed = true;
		global.type = parse_type();
	}
	parse_metadata(MetadataContext::GlobalDeclaration, &metadata, nullptr);
	program_.declare_global(symbol, SymbolKind::GlobalDeclaration, global);
	program_.symbols[symbol].metadata = metadata;
}

void Parser::parse_global_definition()
{
	const Position position = here();
	const SymbolId symbol = begin_symbol(expect_name(TokenKind::Global, "global name"), position);
	SymbolMetadata metadata;
	parse_storage_keyword(metadata);
	Global global = Global();
	global.type = Type::scalar(TypeKind::Void);
	global.address = kNoId;
	if (accept(TokenKind::Colon))
	{
		global.typed = true;
		global.type = parse_type();
		parse_metadata(MetadataContext::GlobalDefinition, &metadata, nullptr);
		expect(TokenKind::Equals, "=");
		if (at(TokenKind::LBrace))
		{
			// A structured global's storage type: its minimum alignment and
			// span (lowir.md "Structured Global Data Initializers").
			if (!global.type.is_object())
				fail("a structured global's storage type must be an object type");
			parse_global_data(global);
		}
		else if (accept_keyword(Keyword::Zero))
			global.init = GlobalInit::Zero;
		else if (accept_word(WordClass::Opcode, static_cast<unsigned>(Opcode::Addr)))
		{
			global.init = GlobalInit::Address;
			global.address = reference_symbol(expect_name(TokenKind::Global, "address initializer symbol"));
			if (at(TokenKind::Plus) || at(TokenKind::Minus))
			{
				const bool negative = at(TokenKind::Minus);
				advance();
				global.addend = negative ? -parse_small_integer("address addend")
										 : parse_small_integer("address addend");
			}
		}
		else
		{
			global.init = GlobalInit::Constant;
			global.constant = parse_constant(global.type.kind());
		}
	}
	else
	{
		parse_metadata(MetadataContext::GlobalDefinition, &metadata, nullptr);
		expect(TokenKind::Equals, "=");
		if (!at(TokenKind::LBrace))
			fail("scalar global requires type");
		parse_global_data(global);
	}
	if (global.init == GlobalInit::Data && global.item_count == 0 && metadata.storage != ir_model::GSM_METADATA &&
		metadata.storage != ir_model::GSM_METADATA_STRINGS)
		fail("structured global must contain data");
	program_.declare_global(symbol, SymbolKind::GlobalDefinition, global);
	program_.symbols[symbol].metadata = metadata;
}

void Parser::parse_global_data(Global& global)
{
	expect(TokenKind::LBrace, "{");
	global.init = GlobalInit::Data;
	global.first_item = static_cast<uint32_t>(program_.data_items.size());
	while (!accept(TokenKind::RBrace))
	{
		DataItem item = DataItem();
		item.address = kNoId;
		item.type = Type::scalar(TypeKind::Void);
		if (accept_keyword(Keyword::Zero))
		{
			item.kind = DataItemKind::Zero;
			if (!at(TokenKind::Number))
				fail("expected positive integer");
			item.amount = static_cast<int64_t>(parse_positive(token().text, token().length));
			advance();
		}
		else
		{
			item.type = parse_type();
			if (item.type.is_pointer() && accept_word(WordClass::Opcode, static_cast<unsigned>(Opcode::Addr)))
			{
				item.kind = DataItemKind::Address;
				item.address = reference_symbol(expect_name(TokenKind::Global, "address target"));
				if (at(TokenKind::Plus) || at(TokenKind::Minus))
				{
					const bool negative = at(TokenKind::Minus);
					advance();
					item.amount = negative ? -parse_small_integer("address addend")
										   : parse_small_integer("address addend");
				}
			}
			else
			{
				item.kind = DataItemKind::Scalar;
				item.constant = parse_constant(item.type.kind());
			}
		}
		program_.data_items.push_back(item);
	}
	global.item_count = static_cast<uint32_t>(program_.data_items.size()) - global.first_item;
}

void Parser::parse_function(bool definition)
{
	const Position position = here();
	const SymbolId symbol = begin_symbol(expect_name(TokenKind::Global, "function name"), position);
	SymbolMetadata metadata;
	const SignatureId signature = parse_signature(
		definition ? MetadataContext::FunctionDefinition : MetadataContext::FunctionDeclaration, &metadata);
	program_.declare_function(
		symbol, definition ? SymbolKind::FunctionDefinition : SymbolKind::FunctionDeclaration, signature);
	program_.symbols[symbol].metadata = metadata;
	if (!definition)
		return;
	if (at(TokenKind::Debug) || at(TokenKind::Invalid))
		program_.functions[program_.symbols[symbol].detail].location = parse_debug_location();
	expect(TokenKind::LBrace, "{");
	parse_body(symbol);
}

void Parser::parse_alias()
{
	expect_keyword(Keyword::Object);
	const Position position = here();
	if (!at(TokenKind::Word))
		fail("invalid object alias spelling");
	const NameId object = token().name;
	advance();
	if (object >= alias_defined_.size())
		alias_defined_.resize(program_.names.size(), false);
	if (alias_defined_[object])
		fail_at(position, "duplicate object alias");
	alias_defined_[object] = true;
	expect(TokenKind::Equals, "=");
	const SymbolId target = reference_symbol(expect_name(TokenKind::Global, "alias target"));
	program_.aliases.push_back(ObjectAlias{ object, target });
}

// ---------------------------------------------------------------------------
// Types and literals

Type Parser::parse_type()
{
	const WordInfo info = word(token());
	if (info.word_class == WordClass::Type)
	{
		advance();
		return Type::scalar(static_cast<TypeKind>(info.value));
	}
	if (at(TokenKind::Word) && token().length > 4 && std::memcmp(token().text, "obj<", 4) == 0)
	{
		const Type type = parse_object_type(token());
		advance();
		return type;
	}
	fail("unknown LowIR type: " + spelling(token()));
}

Type Parser::parse_object_type(const Token& token)
{
	// obj<bytes x align>
	const char* text = token.text + 4;
	const char* end = token.text + token.length;
	const char* close = static_cast<const char*>(std::memchr(text, '>', static_cast<size_t>(end - text)));
	const char* cross = static_cast<const char*>(std::memchr(text, 'x', static_cast<size_t>(end - text)));
	if (close == nullptr || close + 1 != end || cross == nullptr || cross > close)
		fail("unknown LowIR type: " + spelling(token));
	const uint64_t bytes = parse_positive(text, static_cast<uint32_t>(cross - text));
	const uint64_t align = parse_positive(cross + 1, static_cast<uint32_t>(close - cross - 1));
	if (!is_power_of_two(align))
		fail("object alignment is not a power of two");
	if (align > bytes)
		fail("object alignment exceeds size");
	if (bytes >= (static_cast<uint64_t>(1) << 48))
		fail("object size is too large");
	return Type::object(bytes, log2_of(align));
}

uint64_t Parser::parse_positive(const char* text, uint32_t length) const
{
	uint64_t value = 0;
	bool valid = length > 0 && text[0] != '0';
	for (uint32_t i = 0; valid && i < length; ++i)
	{
		if (text[i] < '0' || text[i] > '9' || value > (~static_cast<uint64_t>(0) - 9) / 10)
			valid = false;
		else
			value = value * 10 + static_cast<uint64_t>(text[i] - '0');
	}
	if (!valid)
		fail("invalid positive integer: " + std::string(text, length));
	return value;
}

uint64_t Parser::parse_natural(const char* text, uint32_t length) const
{
	return length == 1 && text[0] == '0' ? 0 : parse_positive(text, length);
}

void Parser::parse_span(uint64_t& bytes, unsigned& align_log2)
{
	// `<bytes>x<align>`, or `<bytes>` for alignment 1.
	if (!at(TokenKind::Number))
		fail("invalid bulk-memory span");
	const char* text = token().text;
	const uint32_t length = token().length;
	const char* cross = static_cast<const char*>(std::memchr(text, 'x', length));
	const uint32_t bytes_length = cross == nullptr ? length : static_cast<uint32_t>(cross - text);
	bytes = parse_positive(text, bytes_length);
	uint64_t align = 1;
	if (cross != nullptr)
		align = parse_positive(cross + 1, length - bytes_length - 1);
	if (!is_power_of_two(align))
		fail("object alignment is not a power of two");
	align_log2 = log2_of(align);
	advance();
}

Constant Parser::parse_constant(TypeKind context)
{
	Constant constant;
	constant.float_kind = TypeKind::Void;
	const bool negative = accept(TokenKind::Minus);
	const bool integer_context = integer_bits(context) != 0 || context == TypeKind::Ptr;
	const WordInfo info = word(token());
	if (info.word_class == WordClass::NamedLiteral)
	{
		advance();
		const NamedLiteral named = static_cast<NamedLiteral>(info.value & ((1u << kNamedSuffixShift) - 1));
		const unsigned suffix = info.value >> kNamedSuffixShift;
		if (named == NamedLiteral::Nullptr)
		{
			if (negative)
				fail("expected literal");
			constant.kind = ValueKind::Integer;
			constant.bits = integer_literal(0);
			return constant;
		}
		if (integer_context)
			fail("invalid floating literal");
		constant.kind = ValueKind::Float;
		constant.float_kind = float_rank(context) != 0 ? context
			: suffix == 1                              ? TypeKind::F32
			: suffix == 2                              ? TypeKind::F80
													   : TypeKind::F64;
		const SpecialFloat which = named == NamedLiteral::Infinity ? SpecialFloat::Infinity
			: named == NamedLiteral::QuietNaN                      ? SpecialFloat::QuietNaN
																   : SpecialFloat::SignalingNaN;
		constant.bits = special_float(constant.float_kind, which, negative);
		if (which != SpecialFloat::Infinity && program_.object_facts && accept(TokenKind::LParen))
		{
			// Host objects: a NaN's payload, `nan(0x55)` (append_float_literal).
			Literal payload;
			if (!at(TokenKind::Number) || is_floating_spelling(token().text, token().length) ||
				!scan_integer_literal(token().text, token().length, false, true, payload) || payload.high != 0 ||
				!nan_with_payload(constant.float_kind, which, negative, payload.low, constant.bits))
				fail("invalid NaN payload");
			advance();
			expect(TokenKind::RParen, ")");
		}
		return constant;
	}
	if (!at(TokenKind::Number))
		fail("expected literal");
	const char* text = token().text;
	const uint32_t length = token().length;
	if (is_floating_spelling(text, length))
	{
		constant.kind = ValueKind::Float;
		if (integer_context ||
			!scan_float_literal(text, length, negative, context, constant.float_kind, constant.bits, scratch_))
			fail("invalid floating literal");
	}
	else if (float_rank(context) != 0)
	{
		// In a floating context an integer spelling is the floating literal
		// of its value (`8` is `8.0`, `-0` is `-0.0`): one literal, however
		// spelled.
		Literal value;
		if (!scan_integer_literal(text, length, negative, true, value))
			fail("invalid integer literal: " + spelling(token()));
		constant.kind = ValueKind::Float;
		constant.float_kind = context;
		constant.bits = integer_as_float(context, value, negative);
		if (std::isinf(float_value(context, constant.bits)))
			fail("invalid floating literal");
	}
	else
	{
		// An i128 context, or one whose type the use decides later (call
		// arguments, switch values), keeps all 128 bits of the literal.
		constant.kind = ValueKind::Integer;
		if (!scan_integer_literal(
				text, length, negative, context == TypeKind::I128 || context == TypeKind::Void, constant.bits))
			fail("invalid integer literal: " + spelling(token()));
	}
	advance();
	return constant;
}

int64_t Parser::parse_small_integer(const char* what)
{
	// A signed integer that fits in 64 bits (addends, selectors).
	const bool negative = accept(TokenKind::Minus);
	Literal bits;
	if (!at(TokenKind::Number) || is_floating_spelling(token().text, token().length) ||
		!scan_integer_literal(token().text, token().length, negative, false, bits))
		fail(std::string("invalid ") + what);
	advance();
	return static_cast<int64_t>(bits.low);
}

DebugLocation Parser::parse_debug_location()
{
	if (!at(TokenKind::Debug))
		fail("malformed !dbg(file, line, column) location");
	const Token& debug = token();
	if (debug.debug_line == 0 || debug.debug_line > 0xffffffffu)
		fail("invalid positive integer: line of " + spelling(debug));
	if (debug.debug_column == 0 || debug.debug_column > 0xffffffffu)
		fail("invalid positive integer: column of " + spelling(debug));
	if (debug.debug_scope + scope_base_ > 0xffffffffu || debug.debug_scope > 0xffffffffu)
		fail("debug scope is out of range");
	if (debug.debug_scope != 0 && !program_.debug)
		program_.debug.reset(new DebugInfo());
	const DebugLocation location = { debug.debug_file, static_cast<uint32_t>(debug.debug_line),
		static_cast<uint32_t>(debug.debug_column),
		debug.debug_scope != 0 ? static_cast<uint32_t>(debug.debug_scope) + scope_base_ : 0 };
	advance();
	return location;
}

// ---------------------------------------------------------------------------
// Metadata

bool Parser::at_word(const char* spelling) const
{
	const size_t length = std::strlen(spelling);
	return at(TokenKind::Word) && token().length == length && std::memcmp(token().text, spelling, length) == 0;
}

uint8_t Parser::parse_object_classes(Type type)
{
	// lowir.md "Object ABI Conventions": a direct object of at most 16
	// bytes, one class for each of its eightbytes (in host objects also
	// another type's, and their own words).
	const uint64_t bytes = type.is_object()                             ? type.bytes
		: type.kind() == TypeKind::I128 || type.kind() == TypeKind::F80 ? 16
																		: (integer_bits(type.kind()) + 7) / 8;
	if (!program_.object_facts && (!type.is_object() || bytes > 16))
		fail("classes metadata requires a direct object of matching size");
	uint8_t classes = 0;
	if (at(TokenKind::Word))
		classes = ir_model::object_classes_from_spelling(token().text, token().length, bytes, program_.object_facts);
	else if (at(TokenKind::Number) && program_.object_facts)
	{
		const uint64_t value = parse_positive(token().text, token().length);
		classes = value < 256 ? static_cast<uint8_t>(value) : 0;
	}
	if (classes == 0)
		fail("invalid classes metadata");
	advance();
	return classes;
}

void Parser::parse_metadata(MetadataContext context, SymbolMetadata* symbol, BoundaryMetadata* boundary)
{
	// One bracket or several (lowir.md "Functions"); a key appears once.
	uint32_t seen = 0;
	while (accept(TokenKind::LBracket))
	{
		do
			parse_metadata_item(context, symbol, boundary, seen);
		while (accept(TokenKind::Comma));
		expect(TokenKind::RBracket, "]");
	}
	if (symbol != nullptr && (symbol->flags & SF_REPLACEABLE) != 0 && symbol->binding != ir_model::SBM_WEAK)
		fail("replaceable metadata requires binding=weak");
}

void Parser::parse_metadata_item(
	MetadataContext context, SymbolMetadata* symbol, BoundaryMetadata* boundary, uint32_t& seen)
{
	if (!at(TokenKind::Word))
		fail("expected metadata key, got '" + spelling(token()) + "'");
	MetadataKey key = MetadataKey::Arity;
	if (!find_key(token(), &key) || !(is_boundary_key(key) || is_symbol_key(key)))
		fail("unknown symbol metadata: " + spelling(token()));
	advance();
	expect(TokenKind::Equals, "=");
	const uint32_t bit = 1u << static_cast<unsigned>(key);
	if ((seen & bit) != 0)
		fail(std::string("duplicate metadata key: ") + ir_model::spelling(key));
	seen |= bit;
	const bool on_global = context == MetadataContext::GlobalDeclaration ||
		context == MetadataContext::GlobalDefinition;
	if (is_boundary_key(key))
	{
		if (on_global)
			fail("function metadata on non-function");
		if (key == MetadataKey::ResultClasses)
		{
			if (result_classes_ == nullptr)
				fail("result_classes metadata requires a boundary");
			*result_classes_ = parse_object_classes(*result_type_);
			if (*result_classes_ == (OC_STATED | OC_MEMORY) && !program_.object_facts)
				fail("a memory-class result is returned through an indirect_result parameter");
			return;
		}
		parse_boundary_value(key, *boundary);
		return;
	}
	if (context == MetadataContext::CallSignature)
		fail("symbol metadata on call signature");
	parse_symbol_value(key, context, *symbol);
}

void Parser::parse_boundary_value(MetadataKey key, BoundaryMetadata& boundary)
{
	const char* text = token().text;
	const uint32_t length = token().length;
	const bool word = at(TokenKind::Word);
	switch (key)
	{
	case MetadataKey::Arity:
		if (!word || !ir_model::from_spelling(text, length, &boundary.arity))
			fail("invalid arity metadata");
		break;
	case MetadataKey::Effects:
		if (!word || !ir_model::from_spelling(text, length, &boundary.effects))
			fail("invalid effects metadata");
		break;
	case MetadataKey::Unwind:
		if (!word || !ir_model::from_spelling(text, length, &boundary.unwind))
			fail("invalid unwind metadata");
		break;
	default:
		if (!word || !ir_model::from_spelling(text, length, &boundary.returns))
			fail("invalid return metadata");
		break;
	}
	advance();
}

void Parser::parse_symbol_value(MetadataKey key, MetadataContext context, SymbolMetadata& symbol)
{
	const bool on_global = context == MetadataContext::GlobalDeclaration ||
		context == MetadataContext::GlobalDefinition;
	switch (key)
	{
	case MetadataKey::Storage:
	case MetadataKey::Section:
		if (!on_global)
			fail(key == MetadataKey::Storage ? "storage metadata on non-global" : "section metadata requires a global");
		break;
	case MetadataKey::TlsFor:
	case MetadataKey::ForceInline:
	case MetadataKey::InlineHint:
	case MetadataKey::NoInline:
		if (on_global)
			fail(std::string(ir_model::spelling(key)) + " metadata requires a function");
		break;
	default:
		break;
	}
	if (key == MetadataKey::TlsFor)
	{
		symbol.tls_for = reference_symbol(expect_name(TokenKind::Global, "tls_for global"));
		return;
	}
	if (key == MetadataKey::Object && at(TokenKind::Global))
	{
		// An object spelling may be written as a global name; the spelling
		// keeps its `@`.
		symbol.object = program_.names.intern(token().text - 1, token().length + 1);
		advance();
		return;
	}
	if (!at(TokenKind::Word))
		fail(std::string("missing value after ") + ir_model::spelling(key));
	const Token& value = token();
	switch (key)
	{
	case MetadataKey::Storage:
		if (!ir_model::from_spelling(value.text, value.length, &symbol.storage))
			fail("invalid global storage metadata");
		break;
	case MetadataKey::Section:
		for (uint32_t i = 0; i < value.length; ++i)
		{
			if (!is_section_char(value.text[i]))
				fail("invalid global section name");
		}
		symbol.section = value.name;
		break;
	case MetadataKey::Role:
		if (!ir_model::from_spelling(value.text, value.length, &symbol.role))
			fail("invalid symbol role");
		if (is_global_role(symbol.role) != on_global)
			fail(on_global ? "function role on global" : "invalid symbol role");
		break;
	case MetadataKey::Linkage:
		if (!ir_model::from_spelling(value.text, value.length, &symbol.linkage))
			fail("invalid linkage metadata");
		break;
	case MetadataKey::Binding:
		if (!ir_model::from_spelling(value.text, value.length, &symbol.binding))
			fail("invalid binding metadata");
		break;
	case MetadataKey::Object:
		symbol.object = value.name;
		break;
	default:
	{
		// The flags: keep_alias, prefer_local, object_root, force_inline,
		// inline_hint, no_inline.
		if (value.length != 3 || std::memcmp(value.text, metadata_yes_spelling(), 3) != 0)
			fail("metadata flag must be yes");
		symbol.flags |= symbol_flag_of(key);
		break;
	}
	}
	advance();
}

// ---------------------------------------------------------------------------
// Signatures

SignatureId Parser::parse_signature(MetadataContext context, SymbolMetadata* symbol)
{
	const Position position = here();
	++parameter_epoch_;
	Signature signature;
	signature.first_parameter = static_cast<uint32_t>(program_.parameters.size());
	signature.parameter_count = 0;
	expect(TokenKind::LParen, "(");
	if (!at(TokenKind::RParen))
	{
		do
			parse_parameter(signature);
		while (accept(TokenKind::Comma));
	}
	expect(TokenKind::RParen, ")");
	expect(TokenKind::Arrow, "->");
	signature.result = parse_type();
	uint8_t* const saved_result_classes = result_classes_;
	const Type* const saved_result_type = result_type_;
	result_classes_ = &signature.boundary.result_classes;
	result_type_ = &signature.result;
	parse_metadata(context, symbol, &signature.boundary);
	result_classes_ = saved_result_classes;
	result_type_ = saved_result_type;
	check_signature(signature, position);
	return program_.add_signature(signature);
}

void Parser::parse_parameter(Signature& signature)
{
	Parameter parameter = Parameter();
	parameter.name = expect_name(TokenKind::Temp, "parameter name");
	if (parameter.name >= parameter_marks_.size())
		parameter_marks_.resize(program_.names.size(), 0);
	if (parameter_marks_[parameter.name] == parameter_epoch_)
		fail("duplicate parameter");
	parameter_marks_[parameter.name] = parameter_epoch_;
	expect(TokenKind::Colon, ":");
	parameter.type = parse_type();
	if (at(TokenKind::LBracket))
		parse_parameter_metadata(parameter);
	program_.parameters.push_back(parameter);
	++signature.parameter_count;
}

void Parser::parse_parameter_metadata(Parameter& parameter)
{
	advance();
	uint32_t seen = 0;
	do
	{
		if (!at(TokenKind::Word))
			fail("invalid parameter metadata");
		const Token key = token();
		advance();
		expect(TokenKind::Equals, "=");
		MetadataKey index = MetadataKey::Arity;
		if (!find_key(key, &index) || !is_parameter_key(index))
			fail("invalid parameter metadata");
		if (index == MetadataKey::Pass)
		{
			if (!at(TokenKind::Word) || !ir_model::from_spelling(token().text, token().length, &parameter.pass))
				fail("invalid parameter pass metadata");
			if (!parameter.type.is_pointer())
				fail("non-direct passing requires ptr");
		}
		else if (index == MetadataKey::Alias)
		{
			if (!at(TokenKind::Word) || !ir_model::from_spelling(token().text, token().length, &parameter.alias))
				fail("invalid parameter metadata");
			if (!parameter.type.is_pointer())
				fail("alias metadata requires ptr");
		}
		else if (index == MetadataKey::ObjectBytes)
		{
			if (!at(TokenKind::Number))
				fail("expected positive integer");
			parameter.object_bytes = parse_positive(token().text, token().length);
			if (!parameter.type.is_pointer())
				fail("object_bytes metadata requires ptr");
		}
		const uint32_t bit = 1u << static_cast<unsigned>(index);
		if (index == MetadataKey::Classes)
		{
			if ((seen & bit) != 0)
				fail("duplicate metadata key: classes");
			seen |= bit;
			parameter.classes = parse_object_classes(parameter.type);
			continue;
		}
		if ((seen & bit) != 0)
			fail("duplicate metadata key: " + std::string(key.text, key.length));
		seen |= bit;
		advance();
	} while (accept(TokenKind::Comma));
	expect(TokenKind::RBracket, "]");
}

void Parser::check_signature(const Signature& signature, Position position)
{
	const Parameter* parameters = program_.parameters_of(signature);
	for (uint32_t i = 0; i < signature.parameter_count; ++i)
	{
		if (parameters[i].pass == ir_model::PPM_INDIRECT_RESULT && (i != 0 || !signature.result.is_void()))
			fail_at(position, "invalid indirect result parameter");
	}
}

// ---------------------------------------------------------------------------
// Completion

void Parser::finish()
{
	finish_debug();
	for (SymbolId symbol = 0; symbol < program_.symbols.size(); ++symbol)
	{
		if (program_.symbols[symbol].kind == SymbolKind::Unresolved)
			fail_at(symbol_uses_[symbol],
				std::string("undefined top-level symbol: ") + program_.spelling(program_.symbols[symbol].name));
	}
	apply_legacy_roles(program_);
	Diagnostic diagnostic;
	if (validate_program(program_, diagnostic))
		return;
	std::string message = describe(program_, diagnostic);
	if (diagnostic.inst != kNoId)
	{
		for (const BodyLines& body : body_lines_)
		{
			if (body.function == diagnostic.function)
				fail_at(Position{ body.source, inst_lines_[body.first + diagnostic.inst] }, message);
		}
	}
	throw Failure{ message };
}

} // namespace text
} // namespace lowir
