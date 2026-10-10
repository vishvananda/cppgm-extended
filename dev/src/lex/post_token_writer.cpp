// Output format of DebugPostTokenOutputStream from the CPPGM PA2 starter code
// (C) 2013 CPPGM Foundation www.cppgm.org. See NOTICE.

#include "lex/post_token_writer.h"

#include <cstring>
#include <ostream>

namespace {

const std::size_t kFlushThreshold = 1 << 16;

const char* UserDefinedFormName(LiteralForm form)
{
	switch (form)
	{
	case LiteralForm::Integer:
		return "integer ";
	case LiteralForm::Floating:
		return "floating ";
	case LiteralForm::Character:
		return "character ";
	case LiteralForm::String:
		return "string ";
	}
	return "";
}

} // namespace

PostTokenWriter::PostTokenWriter(std::ostream& out) : out_(out)
{}

PostTokenWriter::~PostTokenWriter()
{
	flush();
}

void PostTokenWriter::write(const PostToken& token)
{
	switch (token.kind)
	{
	case PostTokenKind::Simple:
		append("simple ");
		append(token.source);
		append(" ");
		append(TokenKindName(token.simple));
		break;
	case PostTokenKind::Identifier:
		append("identifier ");
		append(token.source);
		break;
	case PostTokenKind::Literal:
		if (token.multicharacter)
		{
			// Multicharacter literals are outside PA2's literal set.
			append("invalid ");
			append(token.source);
			break;
		}
		append("literal ");
		append(token.source);
		append(" ");
		append_type_and_value(token);
		break;
	case PostTokenKind::UserDefinedLiteral:
		append("user-defined-literal ");
		append(token.source);
		append(" ");
		append(token.ud_suffix);
		append(" ");
		append(UserDefinedFormName(token.form));
		if (token.form == LiteralForm::Integer || token.form == LiteralForm::Floating)
			append_ud_prefix(token);
		else
			append_type_and_value(token);
		break;
	case PostTokenKind::Invalid:
		append("invalid ");
		append(token.source);
		break;
	case PostTokenKind::EndOfFile:
		append("eof");
		break;
	}
	buffer_.push_back('\n');
	if (buffer_.size() >= kFlushThreshold)
		flush();
}

void PostTokenWriter::write_line(const std::string& text)
{
	buffer_.append(text);
	buffer_.push_back('\n');
	if (buffer_.size() >= kFlushThreshold)
		flush();
}

void PostTokenWriter::flush()
{
	out_.write(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
	out_.flush();
	buffer_.clear();
}

void PostTokenWriter::append(const char* text)
{
	buffer_.append(text);
}

void PostTokenWriter::append(const char* text, uint32_t length)
{
	buffer_.append(text, length);
}

void PostTokenWriter::append_type_and_value(const PostToken& token)
{
	// `<type> <hexdump>` or `array of <n> <type> <hexdump>`.
	if (token.form == LiteralForm::String)
	{
		append("array of ");
		buffer_.append(std::to_string(token.elements));
		append(" ");
	}
	append(FundamentalTypeName(token.type));
	append(" ");
	static const char kHexDigits[] = "0123456789ABCDEF";
	const std::size_t start = buffer_.size();
	buffer_.resize(start + 2 * static_cast<std::size_t>(token.size));
	char* out = &buffer_[start];
	for (uint32_t i = 0; i < token.size; ++i)
	{
		*out++ = kHexDigits[token.data[i] >> 4];
		*out++ = kHexDigits[token.data[i] & 0x0F];
	}
}

void PostTokenWriter::append_ud_prefix(const PostToken& token)
{
	// The literal's text without its ud-suffix.
	append(token.source.text, token.source.length - token.ud_suffix.length);
}
