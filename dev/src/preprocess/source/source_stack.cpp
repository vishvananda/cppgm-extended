#include "preprocess/source/source_stack.h"

#include "preprocess/preprocess_error.h"

namespace {

PPToken EndToken(uint32_t location)
{
	PPToken token = PPToken();
	token.kind = PPTokenKind::EndOfFile;
	token.spelling = "";
	token.begin = location;
	token.end = location;
	return token;
}

} // namespace

SourceStack::SourceStack(SourceFiles& files, IdentifierTable& identifiers, Arena& arena, const PreprocessorNames& names,
	SourceFrameObserver& observer)
	: files_(files), identifiers_(identifiers), arena_(arena), names_(names), observer_(observer), token_(EndToken(0)),
	  peeked_(EndToken(0)), has_peeked_(false), line_mode_(false), line_ended_(false), at_directive_(false),
	  at_end_(false), allow_va_args_(false), hosted_(false), directive_location_(0), tokens_read_(0)
{}

SourceStack::~SourceStack() = default;

void SourceStack::enter(SourceFile* file, std::string presumed_name, std::size_t conditional_depth,
	int32_t search_index, uint32_t included_at)
{
	const uint32_t parent = included_at != kNoLocation ? frames_.back()->inclusion : kNoInclusion;
	std::unique_ptr<SourceFrame> frame(new SourceFrame());
	frame->file = file;
	frame->inclusion = files_.enter(file, std::move(presumed_name), parent, included_at);
	frame->base = files_.inclusion(frame->inclusion).base;
	PPLexerOptions options;
	options.location_base = frame->base;
	options.header_names_on_request = true;
	options.binary_exponents = hosted_;
	options.legacy_comment_bytes = hosted_;
	options.lone_quotes = hosted_;
	options.crlf_splices = hosted_;
	frame->lexer.reset(new PPLexer(file->buffer(), options));
	frame->conditional_depth = conditional_depth;
	frame->search_index = search_index;
	frame->at_line_start = true;
	frame->guard_state = GuardState::Start;
	frame->guard = nullptr;
	frame->guard_depth = 0;
	frames_.push_back(std::move(frame));
}

// ---- ExpansionInput ------------------------------------------------------------------

const PPToken& SourceStack::next()
{
	if (has_peeked_)
	{
		has_peeked_ = false;
		token_ = peeked_;
		return token_;
	}
	if (line_mode_)
		lex_line(token_);
	else
		lex_text(token_);
	return token_;
}

const PPToken& SourceStack::peek()
{
	if (!has_peeked_)
	{
		if (line_mode_)
			lex_line(peeked_);
		else
			lex_text(peeked_);
		has_peeked_ = true;
	}
	return peeked_;
}

void SourceStack::lex_text(PPToken& token)
{
	if (at_directive_ || at_end_)
	{
		token = EndToken(directive_location_);
		return;
	}
	SourceFrame* frame = frames_.back().get();
	uint8_t flags = frame->at_line_start ? static_cast<uint8_t>(kStartOfLine | kLeadingSpace) : 0;
	for (;;)
	{
		const PPToken& lexed = frame->lexer->next();
		switch (lexed.kind)
		{
		case PPTokenKind::WhitespaceSequence:
			flags |= kLeadingSpace;
			continue;
		case PPTokenKind::NewLine:
			flags |= kLeadingSpace | kStartOfLine;
			frame->at_line_start = true;
			continue;
		case PPTokenKind::EndOfFile:
			if (frames_.size() == 1)
			{
				at_end_ = true;
				directive_location_ = lexed.begin;
				token = EndToken(lexed.begin);
				return;
			}
			// An included file's text runs on into its includer's, which
			// continues at the start of the line after the #include.
			leave_file();
			frame = frames_.back().get();
			flags |= kLeadingSpace | kStartOfLine;
			continue;
		default:
			break;
		}
		frame->at_line_start = false;
		if ((flags & kStartOfLine) != 0 && lexed.punctuator == kHashPunctuator)
		{
			at_directive_ = true;
			directive_location_ = lexed.begin;
			token = EndToken(lexed.begin);
			return;
		}
		// Text outside a guard's #ifndef group means the file has no guard.
		if (frame->guard_state != GuardState::Open)
			frame->guard_state = GuardState::None;
		token = lexed;
		token.flags = flags;
		stabilize(token, *frame);
		return;
	}
}

// ---- directive lines -------------------------------------------------------------------

const PPToken& SourceStack::next_in_line()
{
	read_line_token(token_);
	return token_;
}

void SourceStack::read_line_token(PPToken& token)
{
	SourceFrame* frame = frames_.back().get();
	uint8_t flags = 0;
	for (;;)
	{
		const PPToken& lexed = frame->lexer->next();
		if (lexed.kind == PPTokenKind::WhitespaceSequence)
		{
			flags |= kLeadingSpace;
			continue;
		}
		token = lexed;
		token.flags = flags;
		if (lexed.kind == PPTokenKind::NewLine || lexed.kind == PPTokenKind::EndOfFile)
		{
			frame->at_line_start = true;
			token.kind = PPTokenKind::NewLine;
			token.identifier = nullptr;
			token.hide_set = 0;
			return;
		}
		stabilize(token, *frame);
		return;
	}
}

void SourceStack::skip_line()
{
	if (line_mode_ && line_ended_)
		return;
	has_peeked_ = false;
	SourceFrame* frame = frames_.back().get();
	for (;;)
	{
		const PPTokenKind kind = frame->lexer->next().kind;
		if (kind == PPTokenKind::NewLine || kind == PPTokenKind::EndOfFile)
			break;
	}
	frame->at_line_start = true;
	line_ended_ = true;
}

void SourceStack::begin_line_input(const PPToken* first)
{
	line_mode_ = true;
	line_ended_ = false;
	has_peeked_ = first != nullptr;
	if (first != nullptr)
		peeked_ = *first;
}

void SourceStack::end_line_input()
{
	// Whatever the expansion left of the line is not part of the directive's
	// result, but the line still ends the directive.
	if (!line_ended_)
		skip_line();
	line_mode_ = false;
	has_peeked_ = false;
}

void SourceStack::lex_line(PPToken& token)
{
	if (line_ended_)
	{
		token = EndToken(directive_location_);
		return;
	}
	read_line_token(token);
	if (token.kind == PPTokenKind::NewLine)
	{
		line_ended_ = true;
		directive_location_ = token.begin;
		token = EndToken(token.begin);
	}
}

// ---- skipped groups -------------------------------------------------------------------

bool SourceStack::skip_to_directive()
{
	SourceFrame* frame = frames_.back().get();
	bool line_start = frame->at_line_start;
	for (;;)
	{
		const PPToken& lexed = frame->lexer->next();
		switch (lexed.kind)
		{
		case PPTokenKind::WhitespaceSequence:
			continue;
		case PPTokenKind::NewLine:
			line_start = true;
			continue;
		case PPTokenKind::EndOfFile:
			frame->at_line_start = true;
			return false;
		default:
			break;
		}
		if (line_start && lexed.punctuator == kHashPunctuator)
		{
			frame->at_line_start = false;
			directive_location_ = lexed.begin;
			return true;
		}
		line_start = false;
	}
}

// ---- tokens ----------------------------------------------------------------------------

void SourceStack::stabilize(PPToken& token, const SourceFrame& frame)
{
	++tokens_read_;
	token.hide_set = 0;
	if (token.kind == PPTokenKind::Identifier)
	{
		token.identifier = identifiers_.intern(token.spelling, token.length);
		token.spelling = token.identifier->spelling;
		// __VA_ARGS__ shall occur only in the replacement list of a variadic
		// macro ([cpp.replace] 16.3p5).
		if (token.identifier == names_.va_args && !allow_va_args_)
			throw PreprocessError(token.begin, "__VA_ARGS__ outside a variadic macro's replacement list");
		return;
	}
	// A clean spelling is a view of the immutable source image; a translated
	// one lives in lexer scratch storage until the next token.
	const char* image = frame.file->buffer().data();
	if (token.spelling < image || token.spelling >= image + frame.file->buffer().size())
		token.spelling = arena_.copy_string(token.spelling, token.length);
}

void SourceStack::leave_file()
{
	observer_.leaving_file(*frames_.back());
	frames_.pop_back();
}
