#include "preprocess/preprocessor.h"

#include <algorithm>
#include <cstring>
#include <sstream>

#include "lex/text_literal.h"
#include "preprocess/lexer/pp_lexer.h"

namespace {

bool IsPunctuatorToken(const PPToken& token, TokenKind kind)
{
	return token.kind == PPTokenKind::PreprocessingOpOrPunc && IsPunctuator(token.punctuator, kind);
}

void AppendQuoted(std::string& text, const std::string& contents)
{
	text.push_back('"');
	for (char c : contents)
	{
		if (c == '"' || c == '\\')
			text.push_back('\\');
		text.push_back(c);
	}
	text.push_back('"');
}

// The predefined macros with fixed values (directives.md), as #define
// directives the translation unit reads first. __x86_64__ and __linux__
// describe the target, as the course reference defines them.
std::string PredefinedText(const PreprocessorOptions& options)
{
	std::string text = "#define __CPPGM__ 201303L\n"
					   "#define __cplusplus " +
		options.cplusplus +
		"\n"
		"#define __STDC_HOSTED__ 1\n"
		"#define __x86_64__ 1\n"
		"#define __linux__ 1\n";
	text += "#define __CPPGM_AUTHOR__ ";
	AppendQuoted(text, options.author);
	text += "\n#define __DATE__ ";
	AppendQuoted(text, options.date);
	text += "\n#define __TIME__ ";
	AppendQuoted(text, options.time);
	text += "\n";
	return text;
}

// __FILE__'s spelling: the name as a character string literal, with `"`,
// `\` and control characters escaped.
std::string FileLiteral(const std::string& name)
{
	std::string text(1, '"');
	for (char c : name)
	{
		const unsigned char byte = static_cast<unsigned char>(c);
		if (c == '"' || c == '\\')
		{
			text.push_back('\\');
			text.push_back(c);
		}
		else if (byte < 0x20 || byte == 0x7F)
		{
			const char octal[] = { '\\', static_cast<char>('0' + (byte >> 6)),
				static_cast<char>('0' + ((byte >> 3) & 7)), static_cast<char>('0' + (byte & 7)) };
			text.append(octal, sizeof octal);
		}
		else
		{
			text.push_back(c);
		}
	}
	text.push_back('"');
	return text;
}

} // namespace

bool Preprocessor::MacroQuery::is_defined(const char* spelling, uint32_t length) const
{
	return identifiers_.intern(spelling, length)->macro != nullptr;
}

Preprocessor::Preprocessor(const PreprocessorOptions& options, const std::string& path)
	: identifiers_(arena_), names_(identifiers_), source_(files_, identifiers_, arena_, names_, *this),
	  expander_(arena_, identifiers_, names_, *this, macro_state_), directive_expander_(&expander_),
	  macro_query_(identifiers_), evaluator_(macro_query_, options.literals),
	  definition_parser_(arena_, names_.va_args, options.hosted), predefined_(nullptr), command_line_begin_(0),
	  command_line_end_(0), host_predefines_begin_(0), host_predefines_end_(0), builtin_begin_(0), builtin_end_(0),
	  hosted_(options.hosted), modes_(options.modes), directives_(0), skipped_directives_(0), includes_(0),
	  guarded_skips_(0), finished_(false), pack_(0)
{
	expander_.set_input(&source_);
	source_.set_binary_exponents(hosted_);
	expander_.set_binary_exponents(hosted_);
	expander_.set_hosted_recursion(hosted_);
	SourceFile* primary = files_.open(path);
	if (primary == nullptr)
		throw PreprocessError(0, "cannot open the source file");
	source_.enter(primary, path, 0);
	define_builtin_macros();
	// The predefined definitions, then the command line's, are read as if
	// included at the start of the primary source file.
	if (!options.command_line.empty())
	{
		source_.enter(files_.add_text("<command-line>", options.command_line), "<command-line>", 0);
		command_line_begin_ = source_.current_inclusion().base;
		command_line_end_ = command_line_begin_ + source_.current_inclusion().file->buffer().size() + 1;
	}
	if (!options.builtin_declarations.empty())
	{
		source_.enter(files_.add_text("<built-in>", options.builtin_declarations), "<built-in>", 0);
		builtin_begin_ = source_.current_inclusion().base;
		builtin_end_ = builtin_begin_ + static_cast<uint32_t>(source_.current_inclusion().file->buffer().size()) + 1;
	}
	const std::string own = PredefinedText(options);
	predefined_ = files_.add_text("<predefined>", own + options.host_predefines);
	source_.enter(predefined_, "<predefined>", 0);
	if (!options.host_predefines.empty())
	{
		host_predefines_begin_ = source_.current_inclusion().base + static_cast<uint32_t>(own.size());
		host_predefines_end_ = source_.current_inclusion().base + static_cast<uint32_t>(predefined_->buffer().size()) +
			1;
	}
	include_paths_ = options.include_paths;
	system_paths_ = include_paths_.size();
	include_paths_.insert(
		include_paths_.end(), options.system_include_paths.begin(), options.system_include_paths.end());
}

Preprocessor::~Preprocessor() = default;

void Preprocessor::define_builtin_macros()
{
	names_.file->macro = MakeBuiltinMacro(arena_, names_.file, BuiltinMacro::File);
	names_.line->macro = MakeBuiltinMacro(arena_, names_.line, BuiltinMacro::Line);
	names_.counter->macro = MakeBuiltinMacro(arena_, names_.counter, BuiltinMacro::Counter);
	names_.has_cpp_attribute->macro = MakeBuiltinMacro(arena_, names_.has_cpp_attribute, BuiltinMacro::HasCppAttribute);
	names_.has_attribute->macro = MakeBuiltinMacro(arena_, names_.has_attribute, BuiltinMacro::HasAttribute);
	if (!hosted_)
		return;
	static const struct
	{
		const char* spelling;
		BuiltinMacro probe;
	} kHostedProbes[] = { { "__has_include", BuiltinMacro::HasInclude },
		{ "__has_include_next", BuiltinMacro::HasIncludeNext }, { "__has_builtin", BuiltinMacro::HasBuiltin },
		{ "__has_feature", BuiltinMacro::HasFeature }, { "__has_extension", BuiltinMacro::HasExtension },
		{ "__has_warning", BuiltinMacro::HasWarning },
		{ "__has_declspec_attribute", BuiltinMacro::HasDeclspecAttribute },
		{ "__has_constexpr_builtin", BuiltinMacro::HasConstexprBuiltin },
		{ "__has_rmw_builtin", BuiltinMacro::HasRmwBuiltin }, { "__building_module", BuiltinMacro::BuildingModule },
		{ "__is_identifier", BuiltinMacro::IsIdentifier } };
	for (const auto& probe : kHostedProbes)
	{
		Identifier* name = identifiers_.intern(probe.spelling, static_cast<uint32_t>(std::strlen(probe.spelling)));
		name->macro = MakeBuiltinMacro(arena_, name, probe.probe);
	}
}

const PPToken& Preprocessor::next()
{
	for (;;)
	{
		const PPToken& token = expander_.next();
		if (token.kind == PPTokenKind::EndOfFile)
		{
			if (source_.at_directive())
			{
				run_directive();
				continue;
			}
			if (!finished_)
			{
				finished_ = true;
				if (!conditionals_.empty())
					throw PreprocessError(conditionals_.back().location, "unterminated conditional directive");
			}
			return token;
		}
		// `_Pragma ( string-literal )` is recognized in the text after all
		// macro replacement and removed once executed (directives.md).
		if (token.kind == PPTokenKind::Identifier && token.identifier == names_.pragma_operator)
		{
			run_pragma_operator(token);
			continue;
		}
		return token;
	}
}

void Preprocessor::run_pragma_operator(const PPToken& pragma)
{
	const uint32_t location = pragma.begin;
	if (!IsPunctuatorToken(expander_.next(), TokenKind::OP_LPAREN))
		throw PreprocessError(location, "_Pragma requires a parenthesized string literal");
	const PPToken literal = expander_.next();
	TextLiteralParts parts;
	if (literal.kind != PPTokenKind::StringLiteral || !SplitTextLiteral(literal.spelling, literal.length, parts))
		throw PreprocessError(location, "_Pragma requires a parenthesized string literal");
	if (!IsPunctuatorToken(expander_.next(), TokenKind::OP_RPAREN))
		throw PreprocessError(location, "_Pragma requires a parenthesized string literal");

	// Destringizing drops the prefix and quotes and replaces `\"` and `\\`
	// with the characters they escape ([cpp.pragma.op] 16.9p1).
	std::string text;
	for (uint32_t i = parts.body_begin; i < parts.body_end; ++i)
	{
		const char c = literal.spelling[i];
		if (!parts.raw && c == '\\' && i + 1 < parts.body_end &&
			(literal.spelling[i + 1] == '"' || literal.spelling[i + 1] == '\\'))
		{
			text.push_back(literal.spelling[++i]);
			continue;
		}
		text.push_back(c);
	}
	run_pragma(text.data(), static_cast<uint32_t>(text.size()));
}

void Preprocessor::run_pragma(const char* text, uint32_t length)
{
	// The characters are processed through translation phase 3 and executed
	// as a #pragma directive: `once` and `pack` are recognized, other
	// pragmas ignored ([cpp.pragma] 16.6).
	const SourceBuffer buffer = SourceBuffer::ForTokenText(std::string(text, length));
	PPLexerOptions options;
	options.header_names_on_request = true;
	options.token_text = true;
	PPLexer lexer(buffer, options);
	try
	{
		for (;;)
		{
			const PPToken& token = lexer.next();
			if (token.kind == PPTokenKind::WhitespaceSequence)
				continue;
			if (token.kind == PPTokenKind::Identifier && token.length == 4 &&
				std::memcmp(token.spelling, "once", 4) == 0)
				mark_once();
			else if (token.kind == PPTokenKind::Identifier && token.length == 4 &&
				std::memcmp(token.spelling, "pack", 4) == 0)
			{
				std::vector<PPToken> operands;
				for (const PPToken* operand = &lexer.next(); operand->kind != PPTokenKind::EndOfFile;
					operand = &lexer.next())
				{
					if (operand->kind != PPTokenKind::WhitespaceSequence && operand->kind != PPTokenKind::NewLine)
						operands.push_back(*operand);
				}
				apply_pack_pragma(operands);
			}
			return;
		}
	}
	catch (const SourceError&)
	{
		// An unrecognizable pragma is ignored.
	}
}

void Preprocessor::mark_once()
{
	// The file named by __FILE__ is not included again in this translation
	// unit, whatever path names it (directives.md).
	FileIdentity identity;
	if (GetFileIdentity(source_.current_inclusion().presumed_name, identity))
		once_files_.insert(identity);
}

// ---- ExpansionEnvironment and SourceFrameObserver ---------------------------------------

namespace {

// The line control state in force at a location of the inclusion (none:
// the latest).
const SourceLineState* EarlierState(const SourceInclusion& inclusion, uint32_t location)
{
	const auto after = std::upper_bound(inclusion.earlier.begin(), inclusion.earlier.end(), location,
		[](uint32_t value, const SourceLineState& state) { return value < state.until; });
	return after != inclusion.earlier.end() ? &*after : nullptr;
}

} // namespace

std::string Preprocessor::file_of(uint32_t location) const
{
	const SourceInclusion& inclusion = files_.inclusion_at(location);
	const SourceLineState* state = EarlierState(inclusion, location);
	return state != nullptr ? state->presumed_name : inclusion.presumed_name;
}

bool Preprocessor::in_system_header(uint32_t location) const
{
	return files_.inclusion_at(location).system;
}

uint64_t Preprocessor::line_of(uint32_t location) const
{
	const SourceInclusion& inclusion = files_.inclusion_at(location);
	const SourceLineState* state = EarlierState(inclusion, location);
	const int64_t line = inclusion.file->physical_line(location - inclusion.base) +
		(state != nullptr ? state->line_delta : inclusion.line_delta);
	return line < 1 ? 1 : static_cast<uint64_t>(line);
}

uint64_t Preprocessor::column_of(uint32_t location) const
{
	const SourceInclusion& inclusion = files_.inclusion_at(location);
	uint32_t offset = location - inclusion.base;
	if (offset > inclusion.file->buffer().size())
		offset = static_cast<uint32_t>(inclusion.file->buffer().size());
	return inclusion.file->physical_column(offset);
}

uint64_t Preprocessor::presumed_line(uint32_t location)
{
	const SourceInclusion& inclusion = files_.inclusion_at(location);
	const int64_t line = inclusion.file->physical_line(location - inclusion.base) + inclusion.line_delta;
	return line < 1 ? 1 : static_cast<uint64_t>(line);
}

TokenSpelling Preprocessor::presumed_file_literal(uint32_t location)
{
	SourceInclusion& inclusion = files_.inclusion_at(location);
	if (inclusion.file_literal == nullptr)
	{
		const std::string literal = FileLiteral(inclusion.presumed_name);
		inclusion.file_literal = arena_.copy_string(literal.data(), literal.size());
		inclusion.file_literal_length = static_cast<uint32_t>(literal.size());
	}
	return TokenSpelling{ inclusion.file_literal, inclusion.file_literal_length };
}

bool Preprocessor::has_feature(const char* spelling, uint32_t length)
{
	return IsProvidedFeature(spelling, length, modes_);
}

void Preprocessor::leaving_file(const SourceFrame& frame)
{
	// Each if-section is contained in one file ([cpp.cond] 16.1).
	if (conditionals_.size() != frame.conditional_depth)
		throw PreprocessError(conditionals_.back().location, "unterminated conditional directive");
	if (frame.guard_state == GuardState::Closed)
		frame.file->set_guard(frame.guard);
}

// ---- reporting ------------------------------------------------------------------------------

std::string Preprocessor::describe(const PreprocessError& error) const
{
	return describe(error.location(), error.what());
}

std::string Preprocessor::describe(uint32_t location, const char* message) const
{
	if (files_.inclusion_count() == 0)
		return message;
	// The presumed name and line in force there (16.4: what __FILE__ and
	// __LINE__ would give), and the byte column.
	const SourceInclusion* inclusion = &files_.inclusion_at(location);
	uint32_t offset = location - inclusion->base;
	if (offset > inclusion->file->buffer().size())
		offset = inclusion->file->buffer().size();
	location = inclusion->base + offset;
	std::ostringstream text;
	text << file_of(location) << ":" << line_of(location) << ":" << inclusion->file->physical_column(offset) << ": "
		 << message;
	// The #include directives that brought the location in, innermost
	// first.
	while (inclusion->parent != kNoInclusion)
	{
		const uint32_t directive = inclusion->included_at;
		text << "\nnote: included from " << file_of(directive) << ":" << line_of(directive);
		inclusion = &files_.inclusion(inclusion->parent);
	}
	return text.str();
}

std::string Preprocessor::describe(const SourceError& error) const
{
	// A lexical error comes from the file being read, at an offset in its
	// image.
	const SourceFrame& frame = source_.current();
	const uint32_t size = frame.file->buffer().size();
	return describe(frame.base + (error.offset() < size ? error.offset() : size), error.what());
}

PreprocessorCounters Preprocessor::counters() const
{
	PreprocessorCounters counters;
	counters.source_tokens = source_.tokens_read();
	counters.directives = directives_;
	counters.skipped_directives = skipped_directives_;
	counters.includes = includes_;
	counters.guarded_skips = guarded_skips_;
	counters.files = files_.file_count();
	counters.source_bytes = files_.bytes_read();
	counters.identifiers = identifiers_.size();
	counters.arena_bytes = arena_.bytes_reserved();
	counters.expansion = expander_.counters();
	return counters;
}
