#include "syntax/parse_token_stream.h"

namespace syntax {

namespace {

// Consumed tokens are dropped once this many accumulate outside retained
// regions; the copy of the remaining lookahead is then amortized.
const uint32_t kCompactThreshold = 4096;

} // namespace

ParseTokenStream::ParseTokenStream(PostTokenizer& input, Arena& spellings)
	: input_(input), spellings_(spellings), position_(0), retained_(0), base_(0), finished_(false), end_(),
	  tokens_read_(0), peak_window_(0), pack_state_(nullptr), last_pack_(0), ignored_count_(0), literal_name_(nullptr),
	  literal_spelling_(TokenSpelling{ "", 0 })
{
	end_.kind = ParseTokenKind::EndOfFile;
	end_.spelling = TokenSpelling{ "", 0 };
	end_.identifier = nullptr;
}

void ParseTokenStream::advance()
{
	if (!replays_.empty())
	{
		if (position_ < replays_.back().run.size)
			++position_;
		return;
	}
	if (position_ < window_.size() && !window_[position_].is_end())
		++position_;
	if (retained_ == 0 && position_ >= kCompactThreshold)
	{
		window_.erase(window_.begin(), window_.begin() + position_);
		base_ += position_;
		position_ = 0;
	}
}

TokenRun ParseTokenStream::capture(uint32_t begin, uint32_t end, Arena& arena) const
{
	if (!replays_.empty())
	{
		const TokenRun& run = replays_.back().run;
		return TokenRun{ run.tokens + begin, end - begin, run.origin + begin };
	}
	return TokenRun{ arena.copy_array(window_.data() + begin, end - begin), end - begin, base_ + begin };
}

void ParseTokenStream::push_run(TokenRun run)
{
	replays_.push_back(Replay{ run, position_ });
	position_ = 0;
}

void ParseTokenStream::pop_run()
{
	position_ = replays_.back().saved_position;
	replays_.pop_back();
}

void ParseTokenStream::fill(uint32_t count)
{
	while (window_.size() < count)
	{
		if (finished_)
		{
			window_.push_back(end_);
			continue;
		}
		pull();
	}
	telemetry::note_peak(peak_window_, window_.size());
}

uint32_t ParseTokenStream::pack_at(uint64_t ordinal) const
{
	uint32_t pack = 0;
	size_t low = 0, high = pack_changes_.size();
	while (low < high)
	{
		const size_t middle = (low + high) / 2;
		if (pack_changes_[middle].ordinal <= ordinal)
			low = middle + 1;
		else
			high = middle;
	}
	if (low != 0)
		pack = pack_changes_[low - 1].pack;
	return pack;
}

void ParseTokenStream::pull()
{
	const PostToken& post = input_.next();
	if (pack_state_ != nullptr && *pack_state_ != last_pack_)
	{
		last_pack_ = *pack_state_;
		pack_changes_.push_back(PackChange{ base_ + window_.size(), last_pack_ });
	}
	if (!prelude_done_ && !(post.begin >= prelude_begin_ && post.begin < prelude_end_))
	{
		prelude_done_ = true;
		prelude_tokens_ = base_ + window_.size();
	}
	ParseToken token;
	token.flags = 0;
	token.location = post.begin;
	token.spelling = post.source;
	token.identifier = nullptr;
	token.simple = TokenKind::KW_ALIGNAS;
	++tokens_read_;
	switch (post.kind)
	{
	case PostTokenKind::Simple:
		token.kind = ParseTokenKind::Simple;
		token.simple = post.simple;
		if (post.simple == TokenKind::OP_RSHIFT)
		{
			// `>>` becomes two adjacent `>` halves (14.2p3).
			token.simple = TokenKind::OP_GT;
			token.flags = kFirstShiftHalf;
			token.spelling.length = 1;
			window_.push_back(token);
			token.flags = kSecondShiftHalf;
			token.location = post.begin + 1;
			token.spelling.text = post.source.text + 1;
		}
		break;
	case PostTokenKind::Identifier:
		for (uint32_t i = 0; i < ignored_count_; ++i)
		{
			if (post.identifier == ignored_[i])
				return;
		}
		if (post.identifier == literal_name_)
		{
			token.kind = ParseTokenKind::Literal;
			token.spelling = literal_spelling_;
			break;
		}
		token.kind = ParseTokenKind::Identifier;
		token.identifier = post.identifier;
		break;
	case PostTokenKind::Literal:
	case PostTokenKind::UserDefinedLiteral:
		token.kind = post.kind == PostTokenKind::Literal ? ParseTokenKind::Literal : ParseTokenKind::UserDefinedLiteral;
		if (post.form == LiteralForm::String)
		{
			// The source of a string literal sequence is the tokenizer's
			// scratch text.
			token.flags = kStringLiteral;
			token.spelling.text = spellings_.copy_string(post.source.text, post.source.length);
		}
		break;
	case PostTokenKind::EndOfFile:
		token = end_;
		token.location = post.begin;
		finished_ = true;
		break;
	case PostTokenKind::Invalid:
		throw ParseError(post.begin, "invalid token");
	}
	window_.push_back(token);
}

} // namespace syntax
