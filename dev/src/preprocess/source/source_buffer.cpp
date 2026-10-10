#include "preprocess/source/source_buffer.h"

#include <cstring>
#include <istream>
#include <limits>
#include <sstream>
#include <utility>

#include "support/tool_errors.h"

SourceBuffer::SourceBuffer(std::string bytes) : bytes_(std::move(bytes)), begin_(0)
{
	// Offsets are 32-bit; keep one position free for the appended new-line.
	if (bytes_.size() >= std::numeric_limits<uint32_t>::max())
		throw LimitError("source file is too large");

	if (bytes_.compare(0, 3, "\xEF\xBB\xBF") == 0)
		begin_ = 3;

	if (bytes_.size() > begin_ && (bytes_[bytes_.size() - 1] != '\n' || ends_with_spliced_new_line()))
	{
		bytes_.push_back('\n');
	}
}

SourceBuffer::SourceBuffer(std::string bytes, TokenTextTag) : bytes_(std::move(bytes)), begin_(0)
{
	if (bytes_.size() >= std::numeric_limits<uint32_t>::max())
		throw LimitError("token text is too large");
	bytes_.push_back('\n');
}

SourceBuffer SourceBuffer::ForTokenText(std::string text)
{
	return SourceBuffer(std::move(text), TokenTextTag());
}

bool SourceBuffer::ends_with_spliced_new_line() const
{
	const std::size_t length = bytes_.size() - begin_;
	const char* end = bytes_.data() + bytes_.size();
	if (length >= 2 && end[-2] == '\\')
		return true;
	return length >= 4 && std::memcmp(end - 4, "?\?/", 3) == 0;
}

SourceBuffer SourceBuffer::ReadFrom(std::istream& in)
{
	std::string bytes;
	std::streambuf* buffer = in.rdbuf();

	// Seekable input (a regular file) is read into storage of its exact size
	// plus the possible appended new-line, so peak memory is one image rather
	// than the old and new blocks of a geometric regrowth.
	const std::streampos start = buffer->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
	const std::streampos end = buffer->pubseekoff(0, std::ios_base::end, std::ios_base::in);
	if (start != std::streampos(-1) && end != std::streampos(-1) && end >= start)
	{
		if (buffer->pubseekpos(start, std::ios_base::in) != start)
			throw InputOutputError("cannot rewind source input");
		bytes.reserve(static_cast<std::size_t>(end - start) + 1);
	}

	char chunk[1 << 16];
	for (;;)
	{
		std::streamsize count = buffer->sgetn(chunk, sizeof chunk);
		if (count <= 0)
			break;
		bytes.append(chunk, static_cast<std::size_t>(count));
	}
	return SourceBuffer(std::move(bytes));
}

SourceLineCursor::SourceLineCursor(const SourceBuffer& source)
	: source_(source), offset_(source.begin_offset()), line_start_(source.begin_offset()), line_(1)
{}

SourcePosition SourceLineCursor::advance_to(uint32_t offset)
{
	const char* data = source_.data();
	while (offset_ < offset)
	{
		const void* found = std::memchr(data + offset_, '\n', offset - offset_);
		if (found == nullptr)
		{
			offset_ = offset;
			break;
		}
		offset_ = static_cast<uint32_t>(static_cast<const char*>(found) - data) + 1;
		line_start_ = offset_;
		++line_;
	}
	SourcePosition position = { line_, static_cast<std::size_t>(offset - line_start_) + 1 };
	return position;
}

std::string FormatSourceError(const SourceBuffer& source, const SourceError& error)
{
	SourceLineCursor cursor(source);
	uint32_t offset = error.offset() < source.size() ? error.offset() : source.size();
	if (offset < source.begin_offset())
		offset = source.begin_offset();
	SourcePosition position = cursor.advance_to(offset);
	std::ostringstream text;
	text << position.line << ":" << position.column << ": " << error.what();
	return text.str();
}
